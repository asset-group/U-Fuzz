#ifndef __RT8812AUDriver__
#define __RT8812AUDriver__

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
    lni::fast_vector<uint8_t> &data;
    uint8_t *buffer;
    uint16_t buffer_size;
} driver_event;

class WifiRT8812AUDriver {
private:
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
    const char *TAG = "[Driver(RT8812AU)] ";

    bool init(uint16_t dev_id = 0, bool enable_dhcp = false)
    {
        _dev_id = dev_id;

        // Prepare buffers
        memcpy(send_buffer + 1, pkt_radio_tap_hdr, sizeof(pkt_radio_tap_hdr));
        // Init RX buffer
        msg_buffer.reserve(NETLINK_MAX_DATA_LEN);
        msg_buffer.resize(NETLINK_MAX_DATA_LEN);
        nl_init_msg(&nl_msg_recv, &msg_buffer[0], NETLINK_MAX_DATA_LEN);
        msg_buffer.apply_offset(1);
        // Init TX buffer
        nl_init_msg(&nl_msg_send, NULL, 0);

        sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER_FAMILY);
        if (sock_fd < 0) {
            GL1M(TAG, "Netlink Socket Error, trying to load driver rt8812au now...");
            system("./src/drivers/wifi/rtl8812au/load.sh");
            sleep(1);
            sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER_FAMILY);
            if (sock_fd < 0) {
                GL1R(TAG, "Netlink Socket Error, check if driver 88XXau is built and inserted");
                return false;
            }
        }

        // Attempt netlink socker connection
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid(); /* self pid */
        src_addr.nl_groups = NETLINK_GROUP;
        if (!bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr))) {
            GL1G(TAG, "Connected to Netlink socket at group=", NETLINK_GROUP, ", with PID=", getpid(), " and dev_id=", dev_id);
        }
        else {
            GL1R(TAG, "Netlink binding failed, check if driver 88XXau is inserted");
            return false;
        }

        return EnableDHCP(enable_dhcp);
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

    void send(uint8_t *pkt_buf, uint16_t pkt_len)
    {
        // Update sequence
        *((uint16_t *)(pkt_buf + 22)) = (++seq_num) << 4;
        send_buffer[0] = NETLINK_CMD_SEND_DATA;
        memcpy(send_buffer + 1 + sizeof(pkt_radio_tap_hdr), pkt_buf, pkt_len);
        nl_send_msg(&nl_msg_send, send_buffer, pkt_len + 1 + sizeof(pkt_radio_tap_hdr));
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
        int len;
        while ((len = recvmsg(sock_fd, &nl_msg_recv, MSG_DONTWAIT)) <= 0) {
            usleep(10);
        }

        int nl_command = msg_buffer.header()[0];

        if (unlikely(len <= 3)) {
            if (nl_command == NETLINK_EVT_TIMEOUT)
                return driver_event({NETLINK_EVT_TIMEOUT, msg_buffer, NULL, 0});
            // Unkown events
            return driver_event({NETLINK_NULL, msg_buffer, NULL, 0});
        }

        msg_buffer.resize(len - 1);

        return driver_event({nl_command, msg_buffer, &msg_buffer[0], (uint16_t)(len - 1)});
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

            send(pkt_deauth, sizeof(pkt_deauth));
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

    ~WifiRT8812AUDriver()
    {
        if (sock_fd) {
            close(sock_fd);
        }
    }
};

#endif