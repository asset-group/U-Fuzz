#ifndef __znpPcapDriver__
#define __znpPcapDriver__

#include <PcapFileDevice.h>
#include <RawPacket.h>

#include "Machine.hpp"
#include "libs/fast_vector.hpp"
#include "libs/strtk.hpp"
#include <Process.hpp>
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

#define IPTABLES_WRAPPER "./3rd-party/hostapd/idemptables"
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

class znpPcapDriver {
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

    bool init(const char *pcap_input, uint16_t dev_id = 0, bool enable_dhcp = false)
    {
        // create a pcap file reader
        pcpp::PcapNgFileReaderDevice pcapReader(pcap_input);
        pcapReader.open();

        // create a pcapng file writer
        // pcpp::PcapNgFileWriterDevice pcapNgWriter("output.pcapng");
        // pcapNgWriter.open();

        // raw packet object
        pcpp::RawPacket rawPacket;

        // read packets from pcap reader and write pcapng writer
        while (pcapReader.getNextPacket(rawPacket)) {
            std::cout << rawPacket.getRawDataLen() << std::endl;
            packet_list.push_back(rawPacket);
        }

        return true;
    }

    bool start(bool set_core_affinity = false)
    {
        // Required to set main thread core, such core must be isolated from any kthread
        // Set isolcpus=StateMachine.config.options.main_thread_core
        // if (set_core_affinity) {
        //     if (set_affinity(StateMachine.config.options.main_thread_core)) {
        //         GL1Y("[!] Main thread running on CPU ", StateMachine.config.options.main_thread_core);
        //         // Enable RR scheduler (low latency)
        //         enable_rt_scheduler(1);
        //     }
        //     else {
        //         // GL1Y("No CPU to shield selected. driver WILL be unstable");
        //         GL1R("set_affinity failed. driver WILL be unstable");
        //         return false;
        //     }
        // }

        // set_interception_enable(1);
        // set_interrupt_rx_tx_enable(1);

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
        // cmd_buffer[0] = NETLINK_CMD_INTERCEPT_TX;
        // memcpy(cmd_buffer + 1, pkt_buf, pkt_len);
        // nl_send_msg(&nl_msg_send, cmd_buffer, pkt_len + 1);
        // if (pkt_len > 22 && (WIFI_Get_FrameType(pkt_buf) == WIFI_MGT_TYPE)) {
        //     uint16_t seq = WIFI_Get_Sequence(pkt_buf);
        //     if (seq) {
        //         seq_num = seq;
        //         // LOG1(seq_num);
        //     }
        // }
    }

    void send(uint8_t *pkt_buf, uint16_t pkt_len)
    {
        // Update sequence
        // *((uint16_t *)(pkt_buf + 22)) = (++seq_num) << 4;
        // send_buffer[0] = NETLINK_CMD_SEND_DATA;
        // memcpy(send_buffer + 1 + sizeof(pkt_radio_tap_hdr), pkt_buf, pkt_len);
        // nl_send_msg(&nl_msg_send, send_buffer, pkt_len + 1 + sizeof(pkt_radio_tap_hdr));
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
        auto direction = 0;
        pcpp::RawPacket *rawPacket = &packet_list[packet_counter];
        if (packet_counter >= packet_list.size()) {
            exit(0);
            return {NULL};
        }
        // std::cout << packet_list.size() << std::endl;
        packet_counter++;

        std::vector<uint8_t> pcap_buffer_0(rawPacket->getRawData(), rawPacket->getRawData() + rawPacket->getRawDataLen());
        // std::cout << "This is the rawdata_0" << (int)pcap_buffer_0[0] << std::endl;
        lni::fast_vector<uint8_t> pcap_buffer(pcap_buffer_0);
        // std::cout << "This is the rawdata" << (int)pcap_buffer[0] << std::endl;
        if (pcap_buffer.size() > 8) {
            // std::cout << "This is the byte 7-8 " << (int)pcap_buffer[7] << " and " << (int)pcap_buffer[8] << std::endl;
            if (pcap_buffer[8] == 0) {
                direction = NETLINK_EVT_TX;
            }
            else {
                direction = NETLINK_EVT_RX;
            }
        }
        else {
            direction = NETLINK_EVT_RX;
        }
        // auto direction = pcap_buffer.header();
        // std::cout << "This is the direction" << typeid(direction).name() << std::endl;
        return driver_event({direction, pcap_buffer, &pcap_buffer[0], (uint16_t)rawPacket->getRawDataLen()});
    }

    // Disconnect all clients/station
    bool disconnect()
    {
        int ret = 0;
        // try {
        //     string stations = popen_exec((HOSTAPD_CLI " -i "s + StateMachine.config.wifi.wifi_interface + " list_sta").c_str());
        //     vector<string> station_list;
        //     strtk::parse(stations, "\n", station_list);

        //     for (auto &station : station_list) {
        //         if (!station.size())
        //             continue;
        //         GL1Y(TAG, "Disconnecting ", station);
        //         ret |= system((HOSTAPD_CLI " -i "s + StateMachine.config.wifi.wifi_interface + " deauthenticate " + station).c_str());
        //     }
        // }
        // catch (const std::exception &e) {
        //     LOGR(e.what());
        //     return false;
        // }

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

            send(pkt_deauth, sizeof(pkt_deauth));
        }
    }

    bool EnableDHCP(bool enable_dhcp)
    {
        // if (enable_dhcp && !dhcp_started) {

        //     int ret;
        //     Wifi &conf = StateMachine.config.wifi;
        //     std::string dhcp_gw_addr = conf.wifi_dhcp_gateway_address;

        //     std::vector<std::string> ip_list;
        //     strtk::parse(dhcp_gw_addr, ".", ip_list);
        //     if (ip_list.size() < 3) {
        //         LOG2R(TAG, "DHCP IP format error");
        //         return false;
        //     }
        //     std::string dhcp_net = ip_list[0] + "." + ip_list[1] + "." + ip_list[2] + ".";
        //     std::string dhcp_range = dhcp_net + std::to_string((stoi(ip_list[3]) + 1));
        //     dhcp_range += "," + dhcp_net + "149";

        //     GL1Y(TAG, "DHCP Interface: ", conf.wifi_interface);
        //     GL1Y(TAG, "DHCP Gateway: ", dhcp_gw_addr);
        //     GL1Y(TAG, "DHCP Range: ", dhcp_range);
        //     GL1Y(TAG, "Bringing up interface ", conf.wifi_interface, "...");
        //     // Unblock wifi
        //     system("rfkill unblock all");
        //     if (ret = system(("ifconfig " + conf.wifi_interface + " up " + dhcp_gw_addr + " netmask 255.255.255.0").c_str())) {
        //         LOG2R(TAG, "ifconfig error, check if interface is operational");
        //         return false;
        //     }

        //     if (conf.wifi_allow_internet) {
        //         // Enable internet by configuring iptables postrouting
        //         GL1Y(TAG, "Allowing Internet on interface ", conf.wifi_interface);
        //         ret = 0;
        //         // 1. iptables need to accept interface input
        //         ret |= system((IPTABLES_WRAPPER " -I INPUT -i " + conf.wifi_interface + " -j ACCEPT").c_str());
        //         // 2. iptables need to accept forwarding packet to and from interface respectivelly
        //         ret |= system((IPTABLES_WRAPPER " -A FORWARD ! -i " + conf.wifi_interface + " -o " + conf.wifi_interface + " -j ACCEPT").c_str());
        //         ret |= system((IPTABLES_WRAPPER " -A FORWARD -i " + conf.wifi_interface + " ! -o " + conf.wifi_interface + " -j ACCEPT").c_str());
        //         // 3. iptables need to masquerade packets going from interface to external addresses
        //         ret |= system((IPTABLES_WRAPPER " -t nat -A POSTROUTING -s " + dhcp_net + "0/24 ! -o " + conf.wifi_interface + " -j MASQUERADE").c_str());
        //         if (ret) {
        //             LOG2R(TAG, "iptables error, check if iptables service is running");
        //             return false;
        //         }
        //     }

        //     thread_dhcp.setup("/usr/sbin/dnsmasq",
        //                       "--no-daemon --listen-address=" + dhcp_gw_addr +
        //                           " --bind-dynamic --interface=" + conf.wifi_interface +
        //                           " --dhcp-range=" + dhcp_range + " --except-interface=lo",
        //                       [&](const char *bytes, size_t n) {});
        //     GL1(TAG, "/usr/sbin/dnsmasq ", thread_dhcp.process_args);
        //     thread_dhcp.init();
        //     usleep(100000); // 100ms
        //     int pid = thread_dhcp.GetPID();
        //     if (pid != -1) {
        //         dhcp_started = true;
        //         GL1G(TAG, "DHCP (dnsmasq) started with pid ", thread_dhcp.GetPID());
        //     }
        //     else {
        //         dhcp_started = false;
        //         GL1R(TAG, "DHCP (dnsmasq) error, check if interface is operational");
        //     }
        //     return dhcp_started;
        // }
        // else if (!enable_dhcp && dhcp_started) {
        //     dhcp_started = false;
        //     thread_dhcp.stop();
        //     GL1Y(TAG, "DHCP (dnsmasq) stopped");
        //     return true;
        // }

        return true;
    }

    ~znpPcapDriver()
    {
        if (sock_fd) {
            close(sock_fd);
        }
    }
};

#endif