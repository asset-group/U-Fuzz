#ifndef __FgRealTimeDriver__
#define __FgRealTimeDriver__

#include <PcapFileDevice.h>
#include <RawPacket.h>

#include "Machine.hpp"
#include "libs/fast_vector.hpp"
#include "libs/strtk.hpp"
#include <array>
#include <linux/byteorder/little_endian.h>
#include <linux/const.h>
#include <linux/netlink.h>
#include <queue>
#include <semaphore.h>
#include <string>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <thread>
#include <typeinfo>
#include <unistd.h>

// Add the interface library
#include "Process.hpp"
#include "libs/profiling.h"
#include <libs/libvifaced/include/viface/viface.hpp>
extern "C" {
#include <libs/shared_memory.h>
}
#define IPTABLES_WRAPPER "./idemptables"
// #define IPTABLES_WRAPPER "./3rd-party/hostapd/idemptables"
#define HOSTAPD_CLI "./3rd-party/hostapd/hostapd_cli"

#define NETLINK_MAX_DATA_LEN 2048 /* maximum payload size*/
#define NETLINK_USER_FAMILY 25    // Netlink port
#define NETLINK_GROUP 1           // Group ID (must be 1 or higher for multicast)

// Commands received from user space
#define NETLINK_CMD_SELECT_DEVICE 0
#define NETLINK_CMD_READ_ADDR 1
#define NETLINK_CMD_WRITE_ADDR 2
#define NETLINK_CMD_MULTIWRITE_ADDR 3
#define NETLINK_CMD_MULTIREAD_ADDR 4
#define NETLINK_CMD_INTERRUPT_RX_TX_ENABLE 5
#define NETLINK_CMD_INTERCEPTION_TX_ENABLE 6
#define NETLINK_CMD_FORCE_FLAGS_ENABLE 7
#define NETLINK_CMD_FORCE_FLAGS_RETRY 8
#define NETLINK_CMD_SEND_DATA 9
#define NETLINK_CMD_INTERCEPT_TX 10

// Events sent to user space
#define NETLINK_NULL -1
#define NETLINK_EVT_TX 0
#define NETLINK_EVT_RX 1
#define NETLINK_EVT_TX_INJECTED 2
#define NETLINK_EVT_TIMEOUT 3

#define NETLINK_STATUS_OK 0
#define NETLINK_STATUS_FAIL -1

#define DIRECTION_RX 1
#define DIRECTION_TX 0
// Start modify the zigbee_driver
#define SHM_BUFFER_LEN 4096
#define SHM_MUTEX_DIR_RX 0
#define SHM_MUTEX_DIR_TX 1
// #define IPTABLES_WRAPPER "./idemptables"
// #define LOG_BRIDGE_PACKETS 0

#define nl_send_msg(msg, data_buf, data_len)   \
    {                                          \
        (msg)->msg_iov->iov_base = (data_buf); \
        (msg)->msg_iov->iov_len = (data_len);  \
        sendmsg(sock_fd, (msg), 0);            \
    }

typedef struct
{
    int event;
    lni::fast_vector<uint8_t> data;
    uint8_t *buffer;
    uint16_t buffer_size;
} driver_event;

static const char *filter;

// created the thread lock
mutex mutex_dissection;

// create the event queue which contains all the RX and TX packets with driver event type, the event is the direction
queue<driver_event> event_queue;

// create the interface here
set<viface::VIface *> ifaces;
set<string> ipv6_addr;
viface::VIface *iface;

ProcessRunner VETHClient;

class FgRealTimeDriver {
private:
    vector<pcpp::RawPacket> packet_list;
    uint64_t packet_counter = 0;
    // Threads
    bool dhcp_started = false;
    ProcessRunner thread_dhcp;

    // Netlink Socket Vars
    struct sockaddr_nl src_addr;
    struct iovec iov;
    struct msghdr nl_msg_recv, nl_msg_send;
    int sock_fd;
    uint16_t seq_num;

    // Paket Vars
    uint8_t cmd_buffer[NETLINK_MAX_DATA_LEN + 1];
    uint8_t send_buffer[NETLINK_MAX_DATA_LEN + 1];
    uint8_t pkt_radio_tap_hdr[9] = {0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    lni::fast_vector<uint8_t> msg_buffer;

    // Driver vars
    uint16_t _dev_id = 0;

    int nl_init_msg(struct msghdr *msg, uint8_t *data_buf, int max_len)
    {
        static struct sockaddr_nl dest_addr;
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0;    /* For Linux Kernel */
        dest_addr.nl_groups = 0; /* unicast */
        msg->msg_name = (void *)&dest_addr;
        msg->msg_namelen = sizeof(dest_addr);
        msg->msg_iovlen = 1;
        if (!msg->msg_iov)
            msg->msg_iov = (struct iovec *)malloc(sizeof(struct iovec));
        if (!msg->msg_iov)
            return -1;
        msg->msg_iov->iov_base = data_buf;
        msg->msg_iov->iov_len = max_len;
        return 0;
    }

public:
    const char *TAG = "[Driver(Zigbee)] ";
    // able to compile after modify the init function for new zigbee driver
    static bool dispatcher(string const &name, uint id, vector<uint8_t> &packet)
    {
        static uint count = 0;
#if LOG_BRIDGE_PACKETS == 1
        cout << "[Master VETH] --> TX " << count;
        cout << " from interface " << name;
        cout << " of size " << packet.size() << endl;
#endif
        count++;

        if (packet.size() >= SHM_BUFFER_LEN - 2)
            return 1;

        // -------- Wdissector TX interception here --------
        mutex_dissection.lock();
        lni::fast_vector<uint8_t> packet_buffer(packet);
        event_queue.push(driver_event({NETLINK_EVT_TX, packet_buffer, &packet_buffer[0], (uint16_t)packet_buffer.size()}));
        mutex_dissection.unlock();
        // -------------------------------------------------
        return 1;
    }
    static void veth_client_stdout(const char *bytes, size_t n)
    {
        cout << "[Slave VETH] " << string(bytes, n);
    }
    bool init(string prefix, string ip_address, string intercept_filter, string mac_address)
    {
        if (intercept_filter.empty()) {
            intercept_filter = "eth";
        }

        filter = packet_register_filter(intercept_filter.c_str());
        if (filter == NULL) {
            cout << "Filter cannot be compiled" << endl;
            return -1;
        }

        cout << "Starting viface daemon " VIFACE_VERSION " ..." << endl;
        shm_init(SHM_SERVER, SHM_BUFFER_LEN, ("/tmp/wshm_" + prefix).c_str());
        shm_timeout(-1); // Wait for a long time

        // set<viface::VIface *> ifaces;
        // set<string> ipv6_addr;
        try {
            iface = new viface::VIface(prefix);
            ifaces.insert(iface);
            iface->setMAC(mac_address);
            ipv6_addr.insert("fe80::eef1:f8ff:fed5:476b"); // Not used
            try {
                iface->up();
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
            }

            cout << "Interface " << iface->getName() << " up!" << endl;
            // cout << "IPv4 Address: " << iface.getIPv4() << endl;
            cout << "MAC Address:  " << iface->getMAC() << endl;

            cout << "Allowing Internet on interface " << iface->getName() << endl;
            std::vector<std::string> ip_list;
            strtk::parse(ip_address, ".", ip_list);
            if (ip_list.size() < 3) {
                cout << "IP format error" << endl;
                exit(1);
            }
            std::string ip_network = ip_list[0] + "." + ip_list[1] + "." + ip_list[2] + ".";
            int ret = 0;
            // 1. iptables need to accept interface input
            ret |= system((IPTABLES_WRAPPER " -I INPUT -i " + prefix + " -j ACCEPT").c_str());
            // 2. iptables need to accept forwarding packet to and from interface respectivelly
            ret |= system((IPTABLES_WRAPPER " -A FORWARD ! -i " + prefix + " -o " + prefix + " -j ACCEPT").c_str());
            ret |= system((IPTABLES_WRAPPER " -A FORWARD -i " + prefix + " ! -o " + prefix + " -j ACCEPT").c_str());
            // 3. iptables need to masquerade packets going from interface to external addresses
            ret |= system((IPTABLES_WRAPPER " -t nat -A POSTROUTING -s " + ip_network + "0/24 ! -o " + prefix + " -j MASQUERADE").c_str());
            if (ret) {
                cout << "iptables error, check if iptables service is running" << endl;
                // exit(1);
            }

            system(("ip link set dev " + prefix + " multicast on").c_str());
            system(("ip route del " + ip_network + "0/24").c_str());
            system(("ip route add " + ip_network + "0/24 dev " + prefix).c_str());
        }
        catch (exception const &ex) {
            cerr << ex.what() << endl;
            return -1;
        }
        thread *thread_veth_rx = new thread([&]() {
            uint count = 0;
            while (true)
            {
                shm_wait(SHM_MUTEX_DIR_RX);
                cout<<"After receive RX from the tunnel"<<endl;
                uint8_t * pkt_buf = local_sync.shared_memory[SHM_MUTEX_DIR_RX];
                uint16_t pkt_len = *((uint16_t *)pkt_buf);
                pkt_buf += 2; // Skip length bytes
                vector<uint8_t> packet = vector<uint8_t>(pkt_buf, pkt_buf + pkt_len);
                count++;

                // -------- Wdissector RX interception here --------
                mutex_dissection.lock();
                // insert it to the queue
                lni::fast_vector<uint8_t> packet_buffer(packet);

                event_queue.push(driver_event({NETLINK_EVT_RX, packet_buffer, &packet_buffer[0], (uint16_t)packet_buffer.size()}));
                cout<<"pushed the rx to the queue"<<endl;

                // cout<<"This is the rx pkt size"<<packet_buffer.size()<<endl;


                // packet_set_direction(P2P_DIR_RECV);
                // packet_set_filter(filter);
                // packet_dissect(packet.data(), packet.size());
                // if (packet_read_filter(filter))
                // {
                //     // Do mutation here
                //     cout << "[" << count << "] " << termcolor::green
                //         << "RX <-- [" << packet_protocol() << "] "
                //         << termcolor::cyan << packet_summary()
                //         << termcolor::reset << endl;
                // }
                mutex_dissection.unlock();
                // -------------------------------------------------
#if LOG_BRIDGE_PACKETS == 1
                cout << "[Master VETH] <-- RX Packet with Len = " << packet.size() << endl;
#endif
            } });
        pthread_setname_np(thread_veth_rx->native_handle(), "thread_veth_rx");
        usleep(1000); // Wait 1ms for thread to start;

        // Start client process in new network namespace;
        system(("ip netns add " + prefix).c_str());
        VETHClient.init("/sbin/ip",
                        ("netns exec " + prefix + " " +
                         "./veth_tunnel " + prefix + " " + ip_address + " " + mac_address),
                        veth_client_stdout);

        sleep(1);

        // Usage Instructions
        cout << termcolor::green << "VETH Bridge is ready via " << termcolor::yellow << ip_address << termcolor::green << "/24" << termcolor::reset << endl;
        cout << termcolor::green << "Using Protocol Filter: " << termcolor::magenta << "\"" << intercept_filter << "\"" << termcolor::reset << endl;
        cout << "Execute your program as follows:" << endl;
        cout << termcolor::cyan << "sudo ip netns exec " << termcolor::red << prefix << termcolor::yellow << " <program path and args>" << termcolor::reset << endl;
        cout << "example 1: " << termcolor::cyan << "sudo ip netns exec " << termcolor::red << prefix << termcolor::magenta << " ping 8.8.8.8" << termcolor::reset << endl;
        cout << "example 2: " << termcolor::cyan << "sudo ip netns exec " << termcolor::red << prefix << termcolor::magenta << " http-server --address 10.47.0.1" << termcolor::reset << endl;
        thread *thread_veth_tx = new thread([&]() {
            while (true) {
                viface::dispatch(ifaces, dispatcher);
            }
        });

        cout << "Bye!" << endl;

        // // create a pcap file reader
        // pcpp::PcapFileReaderDevice pcapReader(pcap_input);
        // pcapReader.open();

        // // create a pcapng file writer
        // // pcpp::PcapNgFileWriterDevice pcapNgWriter("output.pcapng");
        // // pcapNgWriter.open();

        // // raw packet object
        // pcpp::RawPacket rawPacket;

        // // read packets from pcap reader and write pcapng writer
        // while (pcapReader.getNextPacket(rawPacket)) {
        //     std::cout << rawPacket.getRawDataLen() << std::endl;
        //     packet_list.push_back(rawPacket);
        // }

        return true;
    }

    bool start(bool set_core_affinity = false)
    {
        // Required to set main thread core, such core must be isolated from any kthread
        // Set isolcpus=StateMachine.config.options.main_thread_core
        if (set_core_affinity) {
            if (set_affinity(StateMachine.config.options.main_thread_core)) {
                GL1Y("[!] Main thread running on CPU ", StateMachine.config.options.main_thread_core);
                // Enable RR scheduler (low latency)
                enable_rt_scheduler(1);
            }
            else {
                // GL1Y("No CPU to shield selected. driver WILL be unstable");
                GL1R("set_affinity failed. driver WILL be unstable");
                return false;
            }
        }

        set_interception_enable(1);
        set_interrupt_rx_tx_enable(1);

        return true;
    }

    void stop()
    {
        set_interrupt_rx_tx_enable(0);
        set_interception_enable(0);
    }

    void set_interrupt_rx_tx_enable(uint8_t enable)
    {
        uint8_t nl_cmd[] = {NETLINK_CMD_INTERRUPT_RX_TX_ENABLE, enable};
        nl_send_msg(&nl_msg_send, nl_cmd, sizeof(nl_cmd));
        recvmsg(sock_fd, &nl_msg_recv, 0);
    }

    void set_interception_enable(uint8_t enable)
    {
        uint8_t nl_cmd[] = {NETLINK_CMD_INTERCEPTION_TX_ENABLE, enable};
        nl_send_msg(&nl_msg_send, nl_cmd, sizeof(nl_cmd));
        recvmsg(sock_fd, &nl_msg_recv, 0);
    }

    void intercept_tx(uint8_t *pkt_buf, uint16_t pkt_len)
    {
        cmd_buffer[0] = NETLINK_CMD_INTERCEPT_TX;
        memcpy(cmd_buffer + 1, pkt_buf, pkt_len);
        nl_send_msg(&nl_msg_send, cmd_buffer, pkt_len + 1);
        if (pkt_len > 22 && (WIFI_Get_FrameType(pkt_buf) == WIFI_MGT_TYPE)) {
            uint16_t seq = WIFI_Get_Sequence(pkt_buf);
            if (seq) {
                seq_num = seq;
                // LOG1(seq_num);
            }
        }
    }

    void send_rx(uint8_t *pkt_buf, uint16_t pkt_len)
    {
        // Update sequence
        // *((uint16_t *)(pkt_buf + 22)) = (++seq_num) << 4;
        // send_buffer[0] = NETLINK_CMD_SEND_DATA;
        // memcpy(send_buffer + 1 + sizeof(pkt_radio_tap_hdr), pkt_buf, pkt_len);
        // nl_send_msg(&nl_msg_send, send_buffer, pkt_len + 1 + sizeof(pkt_radio_tap_hdr));
        // cout << "I tried to send rx but faild" << endl;
        vector<uint8_t> packet = vector<uint8_t>(pkt_buf, pkt_buf + pkt_len);
        iface->send(packet);
        shm_notify(SHM_MUTEX_DIR_RX);
    }
    void send_tx(uint8_t *pkt_buf, uint16_t pkt_len)
    {
        // Update sequence
        // *((uint16_t *)(pkt_buf + 22)) = (++seq_num) << 4;
        // send_buffer[0] = NETLINK_CMD_SEND_DATA;
        // memcpy(send_buffer + 1 + sizeof(pkt_radio_tap_hdr), pkt_buf, pkt_len);
        // nl_send_msg(&nl_msg_send, send_buffer, pkt_len + 1 + sizeof(pkt_radio_tap_hdr));
        vector<uint8_t> packet = vector<uint8_t>(pkt_buf, pkt_buf + pkt_len);
        *((uint16_t *)local_sync.shared_memory[SHM_MUTEX_DIR_TX]) = packet.size();
        memcpy(local_sync.shared_memory[SHM_MUTEX_DIR_TX] + 2, packet.data(), packet.size());
        shm_notify(SHM_MUTEX_DIR_TX); // Notify server of new packet
        shm_wait(SHM_MUTEX_DIR_TX);   // Wait server to receive packet
    }

    void add_radio_tap(lni::fast_vector<uint8_t> &pkt_buf, uint8_t direction)
    {
        // add pkt_radio_tap_hdr
        pkt_buf.insert(pkt_buf.begin(), pkt_radio_tap_hdr, pkt_radio_tap_hdr + sizeof(pkt_radio_tap_hdr));
        // add direction to last byte of undecoded radiotap
        pkt_buf[8] = direction;
    }

    driver_event receive()
    {
        // shm_wait(SHM_MUTEX_DIR_RX);
        // uint8_t *pkt_buf = local_sync.shared_memory[SHM_MUTEX_DIR_RX];
        // uint16_t pkt_len = *((uint16_t *)pkt_buf);
        // pkt_buf += 2; // Skip length bytes
        // vector<uint8_t> packet = vector<uint8_t>(pkt_buf, pkt_buf + pkt_len);
        // count++;
        // auto direction = 0;
        // pcpp::RawPacket *rawPacket = &packet_list[packet_counter];
        // if (packet_counter >= packet_list.size()) {
        //     exit(0);
        //     return {NULL};
        // }
        // // std::cout << packet_list.size() << std::endl;
        // packet_counter++;

        // std::vector<uint8_t> pcap_buffer_0(rawPacket->getRawData(), rawPacket->getRawData() + rawPacket->getRawDataLen());
        // // std::cout << "This is the rawdata_0" << (int)pcap_buffer_0[0] << std::endl;
        // lni::fast_vector<uint8_t> pcap_buffer(pcap_buffer_0);
        // // std::cout << "This is the rawdata" << (int)pcap_buffer[0] << std::endl;
        // if (pcap_buffer.size() > 8) {
        //     // std::cout << "This is the byte 7-8 " << (int)pcap_buffer[7] << " and " << (int)pcap_buffer[8] << std::endl;
        //     if (pcap_buffer[8] == 0) {
        //         direction = NETLINK_EVT_TX;
        //     }
        //     else {
        //         direction = NETLINK_EVT_RX;
        //     }
        // }
        // else {
        //     direction = NETLINK_EVT_RX;
        // }
        // auto direction = pcap_buffer.header();
        // std::cout << "This is the direction" << typeid(direction).name() << std::endl;

        // return driver_event({P2P_DIR_RECV, packet.data(), &packet[0], packet.size()});
        if (event_queue.size() == 0) {
            return {NULL};
        }

        driver_event x = event_queue.front();
        // GL1Y("This is the packet: ", x.buffer_size);

        event_queue.pop();
        return x;

        // return driver_event({direction, pcap_buffer, &pcap_buffer[0], (uint16_t)rawPacket->getRawDataLen()});
    }

    // Disconnect all clients/station
    bool disconnect()
    {
        int ret = 0;
        try {
            string stations = popen_exec((HOSTAPD_CLI " -i "s + StateMachine.config.wifi.wifi_interface + " list_sta").c_str());
            vector<string> station_list;
            strtk::parse(stations, "\n", station_list);

            for (auto &station : station_list) {
                if (!station.size())
                    continue;
                GL1Y(TAG, "Disconnecting ", station);
                ret |= system((HOSTAPD_CLI " -i "s + StateMachine.config.wifi.wifi_interface + " deauthenticate " + station).c_str());
            }
        }
        catch (const std::exception &e) {
            LOGR(e.what());
            return false;
        }

        return (ret == 0);
    }

    void Deauth(uint16_t deauth_count = 1)
    {
        uint8_t pkt_deauth[] = {0xc0, 0x0, 0x0, 0x0, 0x80, 0x7d, 0x3a, 0x69,
                                0x2d, 0x1f, 0x0, 0xc0, 0xca, 0xac, 0xf1, 0x9b,
                                0x0, 0xc0, 0xca, 0xac, 0xf1, 0x9b, 0x50, 0x61,
                                0x6, 0x0};

        // uint8_t pkt_delete_block_ack[] = {0xd0, 0x0, 0x0, 0x0, 0xa8, 0x3, 0x2a, 0xeb,
        //                                   0xf5, 0x20, 0x0, 0xc0, 0xca, 0xac, 0xf1, 0x9b,
        //                                   0x0, 0xc0, 0xca, 0xac, 0xf1, 0x9b, 0x30, 0x4e,
        //                                   0x3, 0x2, 0x0, 0x0, 0x25, 0x0};
        uint8_t pkt_delete_block_ack[] = {0xd0, 0x0, 0x0, 0x0, 0x08, 0x3a, 0xf2, 0x31,
                                          0x1c, 0xb0, 0x0, 0xc0, 0xca, 0xac, 0xf1, 0x9b,
                                          0x0, 0xc0, 0xca, 0xac, 0xf1, 0x9b, 0x30, 0x4e,
                                          0x3, 0x2, 0x0, 0x0, 0x25, 0x0};

        // Update sequence number
        for (size_t i = 0; i < deauth_count; i++) {
            // *((uint16_t *)(pkt_delete_block_ack + 22)) = (++seq_num) << 4;
            // send(pkt_delete_block_ack, sizeof(pkt_delete_block_ack));

            // send(pkt_deauth, sizeof(pkt_deauth));
        }
    }

    bool EnableDHCP(bool enable_dhcp)
    {
        if (enable_dhcp && !dhcp_started) {

            int ret;
            Wifi &conf = StateMachine.config.wifi;
            std::string dhcp_gw_addr = conf.wifi_dhcp_gateway_address;

            std::vector<std::string> ip_list;
            strtk::parse(dhcp_gw_addr, ".", ip_list);
            if (ip_list.size() < 3) {
                LOG2R(TAG, "DHCP IP format error");
                return false;
            }
            std::string dhcp_net = ip_list[0] + "." + ip_list[1] + "." + ip_list[2] + ".";
            std::string dhcp_range = dhcp_net + std::to_string((stoi(ip_list[3]) + 1));
            dhcp_range += "," + dhcp_net + "149";

            GL1Y(TAG, "DHCP Interface: ", conf.wifi_interface);
            GL1Y(TAG, "DHCP Gateway: ", dhcp_gw_addr);
            GL1Y(TAG, "DHCP Range: ", dhcp_range);
            GL1Y(TAG, "Bringing up interface ", conf.wifi_interface, "...");
            // Unblock wifi
            system("rfkill unblock all");
            if (ret = system(("ifconfig " + conf.wifi_interface + " up " + dhcp_gw_addr + " netmask 255.255.255.0").c_str())) {
                LOG2R(TAG, "ifconfig error, check if interface is operational");
                return false;
            }

            if (conf.wifi_allow_internet) {
                // Enable internet by configuring iptables postrouting
                GL1Y(TAG, "Allowing Internet on interface ", conf.wifi_interface);
                ret = 0;
                // 1. iptables need to accept interface input
                ret |= system((IPTABLES_WRAPPER " -I INPUT -i " + conf.wifi_interface + " -j ACCEPT").c_str());
                // 2. iptables need to accept forwarding packet to and from interface respectivelly
                ret |= system((IPTABLES_WRAPPER " -A FORWARD ! -i " + conf.wifi_interface + " -o " + conf.wifi_interface + " -j ACCEPT").c_str());
                ret |= system((IPTABLES_WRAPPER " -A FORWARD -i " + conf.wifi_interface + " ! -o " + conf.wifi_interface + " -j ACCEPT").c_str());
                // 3. iptables need to masquerade packets going from interface to external addresses
                ret |= system((IPTABLES_WRAPPER " -t nat -A POSTROUTING -s " + dhcp_net + "0/24 ! -o " + conf.wifi_interface + " -j MASQUERADE").c_str());
                if (ret) {
                    LOG2R(TAG, "iptables error, check if iptables service is running");
                    return false;
                }
            }

            thread_dhcp.setup("/usr/sbin/dnsmasq",
                              "--no-daemon --listen-address=" + dhcp_gw_addr +
                                  " --bind-dynamic --interface=" + conf.wifi_interface +
                                  " --dhcp-range=" + dhcp_range + " --except-interface=lo",
                              [&](const char *bytes, size_t n) {});
            GL1(TAG, "/usr/sbin/dnsmasq ", thread_dhcp.process_args);
            thread_dhcp.init();
            usleep(100000); // 100ms
            int pid = thread_dhcp.GetPID();
            if (pid != -1) {
                dhcp_started = true;
                GL1G(TAG, "DHCP (dnsmasq) started with pid ", thread_dhcp.GetPID());
            }
            else {
                dhcp_started = false;
                GL1R(TAG, "DHCP (dnsmasq) error, check if interface is operational");
            }
            return dhcp_started;
        }
        else if (!enable_dhcp && dhcp_started) {
            dhcp_started = false;
            thread_dhcp.stop();
            GL1Y(TAG, "DHCP (dnsmasq) stopped");
            return true;
        }

        return true;
    }

    ~FgRealTimeDriver()
    {
        if (sock_fd) {
            close(sock_fd);
        }
    }
};

#endif