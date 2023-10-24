#pragma once

#ifndef __ESP32BTDriver__
#define __ESP32BTDriver__

#include "Machine.hpp"
#include "gui/GUI_Bluetooth.hpp"
#include "libs/libmpsse/libusb.h"
#include "libs/libmpsse/mpsse.h"
#include "serial/serial.h"
#include <array>
#include <cassert>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <errno.h>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <pty.h>
#include <queue>
#include <sched.h>
#include <string>
#include <termios.h>
#include <unordered_map>

#define DIRECTION_RX 1
#define DIRECTION_TX 0
#define N_LATENCY_TEST 100

using namespace serial;
using namespace std;

// ENUMs
enum ESP32_SERIAL_COMMANDS {
    ESP32_EVT_POLL = 0xA5,                    // Continously indicates connection and BT clock
    ESP32_EVT_STARTUP = 0xA6,                 // Used to indicate startup
    ESP32_EVT_HW_ERROR = 0xA4,                // Used to indicate HW error
    ESP32_CMD_DATA_RX = 0xA7,                 // Used to receive or send packets in any radio channel
    ESP32_CMD_DATA_TX = 0xBB,                 // Used to receive packets that were automatically transmitted,
    ESP32_CMD_DATA_LMP = 0xBC,                // Used to send arbitrary ACL Packets
    ESP32_CMD_FIFO_FULL = 0xA1,               // Indicates when tranmission data FIFO is full
    ESP32_CMD_CHECKSUM_ERROR = 0xA8,          // Indicates serial reception error.
    ESP32_CMD_CHECKSUM_OK = 0xA9,             // Indicates serial reception success.
    ESP32_CMD_LOG = 0x7F,                     // Print logs through python driver library
    ESP32_CMD_VERSION = 0xEE,                 // Returns the firmware version
    ESP32_CMD_PING = 0x80,                    // Used to test latency between PC and ESP32
    ESP32_CMD_ENABLE_INTERCEPT_TX = 0xEF,     // Enable ACL TX interception (for fuzzing)
    ESP32_CMD_ENABLE_LMP_SNIFFING = 0x81,     // Enable LMP RX/TX Sniffing
    ESP32_CMD_ENABLE_RX_BYPASS = 0x82,        // Enable LMP RX Bypass (Ignore received packets)
    ESP32_CMD_ENABLE_BYPASS_ON_DEMAND = 0x83, // Enable LMP Bypass when receiving response from CMD_DATA_LMP
    ESP32_CMD_DEFAULT_SETTINGS = 0x84,        // Reset Default Settings
    ESP32_CMD_DISABLE_ROLE_SWITCH = 0x85,     // Disable Switch Role to force original role of connection
    ESP32_CMD_RESET = 0x86,                   // Reset ESP32
    ESP32_CMD_SET_MAC = 0x87,                 // Retry last packet
};

enum H4_TYPES {
    H4_CMD = 0x01,
    H4_ACL = 0x02,
    H4_SCO = 0x03,
    H4_EVT = 0x04
};

typedef struct __attribute__((packed)) {
    uint32_t bt_clock;
    uint8_t channel;
    uint8_t ptt : 1;
    uint8_t role : 1;
    uint8_t custom_lmp : 1;
    uint8_t retry_flag : 1;
    uint8_t intercept_req : 1;

} bt_info_t;

typedef union __attribute__((packed)) {
    uint16_t raw_header;
    struct __attribute__((packed)) {
        uint8_t lt_address : 3;
        uint8_t type : 4;
        uint8_t flow : 1;
        uint8_t arqn : 1;
        uint8_t seqn : 1;
        uint16_t hec : 8;
    } fields;

} bt_header_t;

typedef union __attribute__((packed)) {
    uint16_t raw_header;
    struct __attribute__((packed)) {
        uint8_t llid : 2;
        uint8_t flow : 1;
        uint16_t length : 10;
        uint8_t rfu : 3;
    } fields;

} acl_header_t;

typedef union __attribute__((packed)) {
    uint32_t raw_header;
    struct __attribute__((packed)) {
        bt_header_t bt_header;
        acl_header_t acl_header;
    } fields;

} bt_acl_header_t;

// HCI Commands strcutures
typedef struct __attribute__((packed)) {
    uint8_t hci_type = 1;
    uint16_t cmd_opcode = 0x0c01;
    uint8_t length = 8;
    uint8_t event_mask[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f}; // All enabled

} hci_set_event_mask_t;

typedef struct __attribute__((packed)) {
    uint8_t hci_type = 1;
    uint16_t cmd_opcode = 0x0c45;
    uint8_t length = 1;
    uint8_t inquiry_mode = 2; // Results with RSSI or Extended Results (EIR)

} hci_write_inquiry_mode_t;

typedef struct __attribute__((packed)) {
    uint8_t hci_type = 1;
    uint16_t cmd_opcode = 0x0401;
    uint8_t length = 5;
    uint8_t lap[3] = {0x33, 0x8b, 0x9e};
    uint8_t enquiry_length = 8; // Defaults to 10.24 seconds
    uint8_t num_responses = 0;

} hci_inquiry_t;

// Main packet event structure
typedef struct
{
    uint8_t event;
    bt_info_t bt_info;
    vector<uint8_t> data;
    uint8_t *buffer;
    uint16_t buffer_size;
} driver_event;

typedef struct
{
    string bd_addr;
    string name;
    int8_t rssi;
    string device_class;
} scan_result_t;

class priority_mutex {
    std::condition_variable cv_;
    std::mutex gate_;
    bool locked_;
    std::thread::id pr_tid_; // priority thread
public:
    priority_mutex() : locked_(false) {}
    ~priority_mutex() { assert(!locked_); }
    priority_mutex(priority_mutex &) = delete;
    priority_mutex operator=(priority_mutex &) = delete;

    void lock(bool privileged = false)
    {
        const std::thread::id tid = std::this_thread::get_id();
        std::unique_lock<decltype(gate_)> lk(gate_);

        if (privileged)
            pr_tid_ = tid;
        cv_.wait(lk, [&] {
            return !locked_ && (pr_tid_ == std::thread::id() || pr_tid_ == tid);
        });
        locked_ = true;
    }

    void unlock()
    {
        std::lock_guard<decltype(gate_)> lk(gate_);
        if (pr_tid_ == std::this_thread::get_id())
            pr_tid_ = std::thread::id();
        locked_ = false;
        cv_.notify_all();
    }
};

class ESP32BTDriver {
private:
    std::mutex mutex_uart_cmd;
    priority_mutex mutex_uart_cmd_read;
    std::mutex mutex_hci_bridge;
    struct timespec epoll_start_time;

    function<void(uint8_t *, uint16_t)> callback_hci_tx = nullptr;
    function<void(void)> callback_hci_post_tx = nullptr;
    function<void(uint8_t *, uint16_t)> callback_hci_rx = nullptr;
    function<void(string bdaddress, string name, int8_t rssi, string device_class)> callback_scan;
    function<void(unordered_map<string, scan_result_t> &)> callback_scan_complete;

    static void print_buffer(uint8_t *buf, uint16_t size)
    {
        for (size_t i = 0; i < size; i++) {
            printf("%02X", buf[i]);
        }
        printf("\n");
    }

    void process_inquiry_response(uint8_t *hci_buf, uint16_t hci_size)
    {
        // Look for Extended Inquiry Result (0x2f) or Inquiry Result (0x2) or Inquiry complete (0x1)
        if (!(hci_buf[1] == 0x2f || hci_buf[1] == 0x02) || hci_size < 18 || !callback_scan) {
            if (hci_buf[1] == 0x01) {
                GL1G(TAG, "BT Scanning Finished, got ", scan_results.size(), " result(s).");
                is_scanning = false;
                if (callback_scan_complete)
                    callback_scan_complete(this->scan_results);
            }
            return;
        }

        // Prune interesting fields
        header_field_info *hfi_bd_addr = packet_register_set_field_hfinfo("bthci_evt.bd_addr");
        header_field_info *hfi_device_name = packet_register_set_field_hfinfo("btcommon.eir_ad.entry.device_name");
        header_field_info *hfi_rssi = packet_register_set_field_hfinfo("bthci_evt.rssi");
        header_field_info *hfi_device_class = packet_register_set_field_hfinfo("btcommon.cod.minor_device_class");

        // TODO: Move this inside set_field
        // Prune all fields with the same name, same_name_prev_id returns -1 when no mode is found
        header_field_info *next_hfi = hfi_device_class;
        while (next_hfi->same_name_prev_id != -1) {
            next_hfi = proto_registrar_get_nth(next_hfi->same_name_prev_id);
            packet_set_field_hfinfo(next_hfi);
        }

        // This must be HCI RX packet (from driver to host)
        packet_set_direction(DIRECTION_RX);
        // Dissect packet here
        packet_dissect(hci_buf, hci_size);
        field_info *bd_addr = packet_read_field_hfinfo(hfi_bd_addr);
        if (!bdaddr) // Check if bd_addr is really on the packet
            return;
        field_info *device_name = packet_read_field_hfinfo(hfi_device_name);
        field_info *rssi = packet_read_field_hfinfo(hfi_rssi);
        field_info *device_class = NULL;

        // TODO: Move this inside read_field_hfinfo
        // Iterate over all duplicated fields name and check if they were present on packet dissection
        next_hfi = hfi_device_class;
        while (!(device_class = packet_read_field_hfinfo(next_hfi)) && (next_hfi->same_name_prev_id != -1)) {
            next_hfi = proto_registrar_get_nth(next_hfi->same_name_prev_id);
        }

        // Read bdaddr as string for parsed field
        string str_bd_addr = packet_read_field_string(bd_addr);

        // Here, only add new results for unique bdaddres in the array "scan_results", otherwise we may receive duplicate results
        if (scan_results.find(str_bd_addr) == scan_results.end()) {
            // Get class name of device
            const char *str_device_class = packet_read_value_to_string(device_class->value.value.uinteger, device_class->hfinfo);
            // Add scan result to "scan_results" array. Don't mind the weird syntax
            scan_result_t &result = scan_results[str_bd_addr] = scan_result_t({str_bd_addr,
                                                                               (device_name ? packet_read_field_string(device_name) : ""),
                                                                               (int8_t)rssi->value.value.sinteger,
                                                                               (str_device_class ? str_device_class : "Unkown")});
            // Finally call user scan result callback
            callback_scan(result.bd_addr,
                          result.name,
                          result.rssi,
                          result.device_class);
        }
    }

public:
    const char *TAG = "[ESP32BT] ";
    Serial *uart = nullptr;
    // ESP32SPI *uart;
    string uart_name;
    int uart_baud;
    bool isOpen = false;
    bool ready = false;
    long latency;
    bool latency_status = true;
    long poll_time = 0;
    bool enabled_spi = false;
    bool latency_done = false;
    uint8_t bdaddr[6];
    string *bdaddr_str = nullptr;
    bool is_scanning = false;
    unordered_map<string, scan_result_t> scan_results;

    string fw_version;
    bool enable_uart_debug = false;
    bool enable_uart_debug_hci = false;
    bool enable_intercept_tx = false;
    bool enable_bridge_hci = false;
    bool enable_lmp_sniffing = false;
    bool enable_rx_bypass = false;
    bool enable_bypass_on_demand = false;
    bool disable_role_switch = false;

    int counter_hci_from_driver = 0;
    int counter_hci_to_driver = 0;

    string hci_bridge_name;
    int pty_master = 0;
    int pty_slave = 0;
    thread *hci_bridge_thread = nullptr;

    bool init(Config &config)
    {
        this->ready = false;
        is_scanning = false;

        if (config.bluetooth.serial_port.capacity() < 16)
            config.bluetooth.serial_port.reserve(16);

        // if (!this->latency_done) {
        //     // Force kill processes attached to same Serial port
        //     ::system(("sudo fuser -k "s + config.bluetooth.serial_port + " >/dev/null 2>&1").c_str());
        //     ::system(("sudo fuser -k "s + config.bluetooth.serial_port + " >/dev/null 2>&1").c_str());
        //     ::system(("sudo fuser -k "s + config.bluetooth.serial_port + " >/dev/null 2>&1").c_str());
        // }

        // Configure HCI Bridge
        if (config.bluetooth.bridge_hci) {
            int retries = 0;
            while (!enable_hci_bridge(config.bluetooth.bridge_hci)) {
                this_thread::sleep_for(10ms);
                if (retries++ >= 5) {
                    GL1R("[HCI] enable_hci_bridge failed");
                    break;
                }
            }
        }

        // Auto detect port name
        // if (config.bluetooth.serial_auto_discovery)
        //     DiscoverDriver(config.bluetooth.serial_port, config.bluetooth.serial_baud_rate);

        this->uart_name = config.bluetooth.serial_port;
        this->uart_baud = config.bluetooth.serial_baud_rate;
        this->bdaddr_str = &config.bluetooth.own_bd_address;

        try {
            if (this->uart) {
                delete this->uart;
                this->uart = NULL;
            }

            this->uart = new Serial(config.bluetooth.serial_port,
                                    config.bluetooth.serial_baud_rate,
                                    Timeout::simpleTimeout(20), // Read timeout
                                    false,                      // Enable pooling
                                    false);                     // Enable low latency (required)

            this->isOpen = uart->isOpen();
            this->uart->setRTS(true);
            this->uart->setDTR(true);
            this->uart->setRTS(false);
            this->uart->setDTR(false);
        }
        catch (const std::exception &e) {
            this->isOpen = false;
        }

        if (this->isOpen) {
            int status = true;

            // Check driver firmware version
            // usleep(20000);
            // if (string_contains(this->fw_version, ".")) {
            //     GL1Y(TAG, "Firmware version: ", fw_version);
            // }
            // else {
            //     GL1R(TAG, "Firmware version not detected");
            //     this->ready = false;
            //     this->fw_version = "";
            //     this->close();

            //     return false;
            // };

            // if (set_enable_lmp_sniffing(config.bluetooth.lmp_sniffing)) {
            //     if (this->enable_lmp_sniffing)
            //         GL1G(TAG, "LMP Sniffing ENABLED");
            //     else
            //         GL1Y(TAG, "[!] LMP Sniffing DISABLED");
            // }
            // else {
            //     GL1R(TAG, "Could not disable LMP Sniffing");
            //     this->ready = false;
            //     this->close();
            //     return false;
            // }

            // if (set_enable_intercept_tx(config.bluetooth.intercept_tx)) {
            //     if (this->enable_intercept_tx)
            //         GL1G(TAG, "TX Packet interception ENABLED");
            //     else
            //         GL1Y(TAG, "[!] TX Packet interception DISABLED");
            // }
            // else {
            //     GL1R(TAG, "Could not disable TX Packet interception");
            //     this->ready = false;
            //     this->close();
            //     return false;
            // }

            // if (set_enable_rx_bypass(config.bluetooth.rx_bypass)) {
            //     if (this->enable_rx_bypass)
            //         GL1G(TAG, "RX Bypass ENABLED");
            //     else
            //         GL1Y(TAG, "[!] RX Bypass DISABLED");
            // }
            // else {
            //     GL1R(TAG, "Could not disable RX Bypass");
            //     this->ready = false;
            //     this->close();
            //     return false;
            // }

            // if (set_enable_bypass_on_demand(config.bluetooth.rx_bypass_on_demand)) {
            //     if (this->enable_bypass_on_demand)
            //         GL1G(TAG, "Bypass on Demand ENABLED");
            //     else
            //         GL1Y(TAG, "[!] Bypass on Demand DISABLED");
            // }
            // else {
            //     GL1R(TAG, "Could not disable Bypass on Demand");
            //     this->ready = false;
            //     this->close();
            //     return false;
            // }

            // if (set_disable_role_witch(config.bluetooth.disable_role_switch)) {
            //     if (this->disable_role_switch)
            //         GL1G(TAG, "Role Switch DISABLED");
            //     else
            //         GL1Y(TAG, "[!] Role Switch ENABLED");
            // }
            // else {
            //     GL1R(TAG, "Could not change Role Switch");
            //     this->ready = false;
            //     this->close();
            //     return false;
            // }

            // if (set_bdaddress(config.bluetooth.own_bd_address)) {
            //     GL1G(TAG, "Own BDADDR set to ", config.bluetooth.own_bd_address);
            // }
            // else {
            //     GL1R(TAG, "Could not set own BDADDR to ", config.bluetooth.own_bd_address);
            //     this->ready = false;
            //     this->close();
            //     return false;
            // }

            // if (!this->latency_done) {
            //     GL1Y(TAG, "Measuring UART Latency...");
            //     GL1G(TAG, "USB Latency:", get_latency(), " us [OK]");
            //     this->latency_done = true;
            // }

            if (this->enable_uart_debug = config.bluetooth.serial_enable_debug)
                GL1Y(TAG, "UART debugging enabled");

            if (this->enable_uart_debug_hci = config.bluetooth.serial_enable_debug_hci)
                GL1Y(TAG, "HCI debugging enabled");

            this->ready = true;
            return true;
        }
        else {
            this->ready = false;
            return false;
        }
    };

    void hci_bridge_write(uint8_t *buf, size_t buf_len)
    {
        if (this->enable_bridge_hci && this->pty_master) {
            if (callback_hci_rx)
                callback_hci_rx(buf, buf_len);

            // Make sure to flush previous data (this also avoids write freeze due to pty_slave being closed)
            if (is_scanning)
                tcflush(this->pty_master, TCOFLUSH);
            // Write to the bridge
            ::write(this->pty_master, buf, buf_len);

            this->counter_hci_from_driver++;

            if (this->enable_uart_debug_hci)
                LOG3("[HCI] ", "HCI FROM ESP32:", strtk::convert_bin_to_hex(string((char *)buf, buf_len)));
        }
    }

    void ClearHCIBridge()
    {
        if (this->enable_bridge_hci && this->pty_master) {
            tcflush(this->pty_master, TCOFLUSH);
            ::system(("sudo fuser -k "s + this->hci_bridge_name + " >/dev/null 2>&1").c_str());
            ::system(("sudo fuser -k "s + this->hci_bridge_name + " >/dev/null 2>&1").c_str());
            ::system(("sudo fuser -k "s + this->hci_bridge_name + " >/dev/null 2>&1").c_str());
        }
    }

    void SetHCICallbackTX(function<void(uint8_t *, uint16_t)> fcn)
    {
        callback_hci_tx = fcn;
    };

    void SetHCICallbackPostTX(function<void(void)> fcn)
    {
        callback_hci_post_tx = fcn;
    };

    void SetHCICallbackRX(function<void(uint8_t *, uint16_t)> fcn)
    {
        callback_hci_rx = fcn;
    };

    void hci_bridge_read()
    {
        enable_idle_scheduler();
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

        auto buffer = std::unique_ptr<char[]>(new char[4096]);
        ssize_t n;

        while (this->enable_bridge_hci) {
            n = ::read(this->pty_master, buffer.get(), 4096);

            if (n > 0) {
                // cout << "hci:lock" << endl;
                this->mutex_hci_bridge.lock();
                if (callback_hci_tx)
                    callback_hci_tx((uint8_t *)buffer.get(), n);

                // LOG1("<--");

                send_raw((uint8_t *)buffer.get(), n);
                this->counter_hci_to_driver++;

                if (callback_hci_post_tx)
                    callback_hci_post_tx();

                this->mutex_hci_bridge.unlock();

                // cout << "hci:unlock" << endl;

                if (this->enable_uart_debug_hci) {
                    // LOG2("Size: ", n);
                    string hex = strtk::convert_bin_to_hex(string(buffer.get(), static_cast<size_t>(n)));
                    LOG3("[HCI] ", "HCI From Host:", hex);
                }
            }
            else {
                // LOG1("[HCI] sleep");
                this_thread::sleep_for(20ms);
            }
        }
    }

    void hci_bridge_close()
    {
        this->enable_bridge_hci = false;
        // Reset mutex
        (this->mutex_hci_bridge.try_lock() ? this->mutex_hci_bridge.unlock() : this->mutex_hci_bridge.unlock());
        if (hci_bridge_thread != nullptr) {

            // printf("1:pthread_cancel ok\n");
            pthread_cancel(hci_bridge_thread->native_handle());
            // printf("2:pthread_cancel ok\n");
            hci_bridge_thread->join();
            // printf("3:hci_bridge_thread->join ok\n");
            delete hci_bridge_thread;
            hci_bridge_thread = nullptr;
        }

        if (this->pty_master != 0) {
            ::close(this->pty_master);
            this->pty_master = 0;
        }

        if (this->pty_slave != 0) {
            ::close(this->pty_slave);
            this->pty_slave = 0;
        }
    }

    bool enable_hci_bridge(bool en)
    {
        static char pty_name[64];

        if (en && !this->enable_bridge_hci) {
            if (openpty(&pty_master, &pty_slave, pty_name, NULL, NULL) == 0) {
                fcntl(pty_master, F_SETFD, FD_CLOEXEC); // Close on exec
                fcntl(pty_slave, F_SETFD, FD_CLOEXEC);  // Close on exec
                // LOG2("enable_hci_bridge:", en);
                // termios options
                struct termios params_master, params_slave;
                tcgetattr(this->pty_master, &params_master);
                tcgetattr(pty_slave, &params_slave);

                // Set to raw mode
                cfmakeraw(&params_master);
                cfmakeraw(&params_slave);
                // CREAD - Enable port to read data
                // CLOCAL - Ignore modem control lines
                params_master.c_cflag |= (B4000000 | CS8 | CLOCAL | CREAD);
                params_slave.c_cflag |= (B4000000 | CS8 | CLOCAL | CREAD);

                tcsetattr(pty_slave, TCSANOW, &params_slave);
                ::close(pty_slave);
                // pty_slave = 0;

                if (tcsetattr(this->pty_master, TCSANOW, &params_master) < 0) {
                    hci_bridge_close();
                }
                else {
                    // Flush data
                    tcflush(this->pty_master, TCIOFLUSH);
                    // Start HCI READ bridge thread here
                    // Unlock mutex
                    (this->mutex_hci_bridge.try_lock() ? this->mutex_hci_bridge.unlock() : this->mutex_hci_bridge.unlock());

                    this->hci_bridge_name = string(pty_name);
                    this->enable_bridge_hci = true;
                    hci_bridge_thread = new std::thread(&ESP32BTDriver::hci_bridge_read, this);
                    pthread_setname_np(hci_bridge_thread->native_handle(), "hci_bridge");
                    GL1G(TAG, "HCI Bridge ON: ", this->hci_bridge_name);
                }
            }
            else {
                hci_bridge_close();
                GL1R(TAG, "HCI Bridge failed, run with sudo");
            }
        }
        else if (!en && this->enable_bridge_hci) {
            hci_bridge_close();
            GL1Y(TAG, "HCI Bridge OFF");
        }

        return this->enable_bridge_hci;
    }

    driver_event receive()
    {
        uint8_t cmd;
        uint16_t cmd_len;
        uint8_t checksum;
        uint32_t counter_bt_clock;
        uint8_t evt;
        static uint8_t hci_buff[2048];
        uint16_t hci_size;

        // cout << "receive:unlock" << endl;
        mutex_hci_bridge.unlock();

        while (this->ready) {
            // Lock read command mutex
            std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read);

            if (uart->read(&cmd, 1) > 0) {
                hci_size = 1;
                evt = cmd;
                mutex_hci_bridge.lock();

                switch (cmd) {
                case ESP32_EVT_STARTUP:
                    return driver_event({evt});
                    break;

                case ESP32_EVT_POLL:
                    if ((uart->read((uint8_t *)&counter_bt_clock, 4)) == 4) {
                        struct timespec end_time;
                        clock_gettime(CLOCK_MONOTONIC, &end_time);
                        long measured_latency_ns = ((end_time.tv_sec - epoll_start_time.tv_sec) * 1000000000UL) + (end_time.tv_nsec - epoll_start_time.tv_nsec) / 1000;
                        clock_gettime(CLOCK_MONOTONIC, &epoll_start_time);
                        if (measured_latency_ns < 26000)
                            poll_time = measured_latency_ns;

                        return driver_event({evt, counter_bt_clock});
                    }
                    break;

                case ESP32_CMD_DATA_LMP:
                case ESP32_CMD_DATA_RX:
                case ESP32_CMD_DATA_TX:
                    // Lock HCI bridge until event is processed and returns here again
                    // puts("ESP32_CMD");
                    // Receive payload length
                    if ((uart->read((uint8_t *)&cmd_len, 2) == 2)) {
                        // Receive payload + checksum
                        vector<uint8_t> rx_buffer;
                        if (uart->read(rx_buffer, cmd_len + 1) == cmd_len + 1) {
                            // Calculate checksum
                            checksum = 0;
                            for (int16_t i = 0; i < cmd_len; i++) {
                                checksum += rx_buffer[i];
                            }

                            if ((checksum == rx_buffer[cmd_len])) {
                                bt_info_t *bt_info = (bt_info_t *)&rx_buffer[0];
                                // if (evt == ESP32_CMD_DATA_TX)
                                //     printf("H:%d\n", bt_info->intercept_req);
                                // puts("2.recv");
                                // printf("CMD, size: %d\n", cmd_len);
                                // print_buffer(&rx_buffer[0], cmd_len);
                                driver_event de = driver_event({evt, *bt_info, std::move(rx_buffer), nullptr, cmd_len});
                                de.buffer = &de.data[0];
                                return de;
                            }
                            else {
                                printf("chksum error. len=%d,calc=%d,received=%d\n", cmd_len, checksum, rx_buffer[cmd_len]);
                                cmd = ESP32_CMD_CHECKSUM_ERROR;
                                uart->write(&cmd, 1);
                            }
                        }
                    }
                    break;

                case H4_CMD:
                    hci_buff[0] = cmd;
                    if (uart->read(&hci_buff[1], 3) == 3) {
                        // Receive 3 bytes (opcode (2) + length (1))
                        hci_size += 3;
                        uint8_t h4_cmd_size = hci_buff[3];
                        if (!h4_cmd_size || (uart->read(&hci_buff[4], h4_cmd_size) == h4_cmd_size)) {
                            hci_size += h4_cmd_size;
                            // Lock HCI bridge until event is processed and returns here again
                            // hci_bridge_write(hci_buff, hci_size);
                            return {evt, {0}, vector<uint8_t>(hci_buff, hci_buff + hci_size)};
                        }
                    }
                    break;
                case H4_ACL:
                    hci_buff[0] = cmd;
                    // LOG1("H4_ACL");
                    if (uart->read(&hci_buff[1], 4) == 4) {
                        // Receive 4 bytes (opcode (2) + length (2))
                        hci_size += 4;
                        uint16_t h4_acl_size = *((uint16_t *)(hci_buff + 3));
                        // LOG2("1)h4_acl_size=", h4_acl_size);
                        if (!h4_acl_size || (uart->read(&hci_buff[5], h4_acl_size) == h4_acl_size)) {
                            hci_size += h4_acl_size;
                            // Lock HCI bridge until event is processed and returns here again
                            hci_bridge_write(hci_buff, hci_size);
                            return {evt, {0}, vector<uint8_t>(hci_buff, hci_buff + hci_size)};
                        }
                    }
                    break;

                case H4_EVT:
                    hci_buff[0] = cmd;
                    // print_buffer(hci_buff, 4);

                    if (uart->read(&hci_buff[1], 2) == 2) {
                        // Receive 2 bytes (event code (1) + length (1))
                        hci_size += 2;
                        uint8_t h4_evt_size = hci_buff[2];
                        if (!h4_evt_size || (uart->read(&hci_buff[3], h4_evt_size) == h4_evt_size)) {
                            // print_buffer(hci_buff, hci_size);
                            // printf("evt:%d,%d\n", h4_evt_size, hci_size);
                            hci_size += h4_evt_size;
                            // Lock HCI bridge until event is processed and returns here again
                            hci_bridge_write(hci_buff, hci_size);
                            // Check for HCI Inquiry Result
                            if (is_scanning) {
                                // If scan is in progress, process hci packets
                                process_inquiry_response(hci_buff, hci_size);
                            }
                            return {evt, {0}, vector<uint8_t>(hci_buff, hci_buff + hci_size)};
                        }
                    }
                    break;

                default:
                    if (enable_uart_debug) {
                        printf("%c", cmd);
                    }
                    break;
                }

                mutex_hci_bridge.unlock();
            }

            if (!(this->isOpen = this->uart->isOpen()))
                this->ready = false;
        }
        return driver_event({NULL});
    }

    void set_default()
    {
        if (isOpen && this->uart && this->uart->isOpen()) {
            lock_guard<mutex> lock(mutex_uart_cmd);
            const uint8_t cmd_buf[] = {ESP32_CMD_DEFAULT_SETTINGS};

            uart->write(cmd_buf, sizeof(cmd_buf));
        }
    }

    string get_version()
    {
        if (!this->isOpen)
            return ""s;
        lock_guard<mutex> lock(mutex_uart_cmd);
        mutex_uart_cmd_read.lock(true);
        std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);

        uint8_t cmd = ESP32_CMD_VERSION;
        uart->write(&cmd, 1);
        // usleep(10000);
        return uart->readline(6, "\n", false);
    }

    bool set_enable_intercept_tx(bool val, bool ignore_return = false)
    {
        if (!this->isOpen)
            return false;
        if (!ignore_return) {
            lock_guard<mutex> lock(mutex_uart_cmd);
            mutex_uart_cmd_read.lock(true);
            std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);
        }
        bool res = val;
        const uint8_t cmd_buf[] = {ESP32_CMD_ENABLE_INTERCEPT_TX, val};
        uart->write(cmd_buf, sizeof(cmd_buf));

        if (ignore_return == false) {
            if (uart->read((uint8_t *)&res, 1) < 1)
                return false;
        }

        this->enable_intercept_tx = res;
        return true;
    }

    bool parse_mac(uint8_t *out, std::string const &in)
    {
        uint8_t bytes[6];
        if (std::sscanf(in.c_str(),
                        "%02x:%02x:%02x:%02x:%02x:%02x",
                        &bytes[0], &bytes[1], &bytes[2],
                        &bytes[3], &bytes[4], &bytes[5]) != 6) {
            return false;
        }
        memcpy(out, bytes, 6);
        return true;
    }

    bool randomize_bdaddr()
    {
        if (bdaddr_str->size() > 15) {
            const char *hex_digits = "0123456789abcdef";
            uint8_t a1 = g_random_int_range(0, 16);
            uint8_t a2 = g_random_int_range(0, 16);
            uint8_t a3 = g_random_int_range(0, 16);
            uint8_t a4 = g_random_int_range(0, 16);
            uint8_t a5 = g_random_int_range(0, 16);
            uint8_t a6 = g_random_int_range(0, 16);
            uint8_t a7 = g_random_int_range(0, 16);
            uint8_t a8 = g_random_int_range(0, 16);
            uint8_t a9 = g_random_int_range(0, 16);
            uint8_t a10 = g_random_int_range(0, 16);
            uint8_t a11 = g_random_int_range(0, 16);
            uint8_t a12 = g_random_int_range(0, 16);
            (*bdaddr_str)[0] = hex_digits[a1];
            (*bdaddr_str)[1] = hex_digits[a2];
            (*bdaddr_str)[3] = hex_digits[a3];
            (*bdaddr_str)[4] = hex_digits[a4];
            (*bdaddr_str)[6] = hex_digits[a5];
            (*bdaddr_str)[7] = hex_digits[a6];
            (*bdaddr_str)[9] = hex_digits[a7];
            (*bdaddr_str)[10] = hex_digits[a8];
            (*bdaddr_str)[12] = hex_digits[a9];
            (*bdaddr_str)[13] = hex_digits[a10];
            (*bdaddr_str)[15] = hex_digits[a11];
            (*bdaddr_str)[16] = hex_digits[a12];
            return set_bdaddress(*bdaddr_str);
        }
        else
            return false;
    }

    bool update_bdaddress()
    {
        if (bdaddr_str)
            return set_bdaddress(*bdaddr_str);
        else
            return false;
    }

    bool set_bdaddress(string bdaddr)
    {
        uint8_t parsed_bdaddr[6];
        if (parse_mac(parsed_bdaddr, bdaddr))
            return set_bdaddress(parsed_bdaddr);
        else
            return false;
    }

    bool set_bdaddress(uint8_t *val)
    {
        if (!this->isOpen)
            return false;

        lock_guard<mutex> lock(mutex_uart_cmd);

        const uint8_t cmd_buf[] = {ESP32_CMD_SET_MAC, val[5], val[4], val[3], val[2], val[1], val[0]};
        uart->write(cmd_buf, sizeof(cmd_buf));

        memcpy(this->bdaddr, val, 6);

        usleep(1000);
        return true;
    }

    bool set_enable_lmp_sniffing(bool val, bool ignore_return = false)
    {
        if (!this->isOpen)
            return false;
        if (!ignore_return) {
            lock_guard<mutex> lock(mutex_uart_cmd);
            mutex_uart_cmd_read.lock(true);
            std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);
        }
        bool res = val;
        const uint8_t cmd_buf[] = {ESP32_CMD_ENABLE_LMP_SNIFFING, val};
        uart->write(cmd_buf, sizeof(cmd_buf));

        if (ignore_return == false) {
            if (uart->read((uint8_t *)&res, 1) < 1)
                return false;
        }

        this->enable_lmp_sniffing = res;
        return true;
    }

    bool set_enable_rx_bypass(bool val, bool ignore_return = false)
    {
        if (!this->isOpen)
            return false;
        if (!ignore_return) {
            lock_guard<mutex> lock(mutex_uart_cmd);
            mutex_uart_cmd_read.lock(true);
            std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);
        }

        bool res = val;
        const uint8_t cmd_buf[] = {ESP32_CMD_ENABLE_RX_BYPASS, val};
        uart->write(cmd_buf, sizeof(cmd_buf));
        // uart->write(cmd_buf, sizeof(cmd_buf));

        if (ignore_return == false) {
            if (uart->read((uint8_t *)&res, 1) < 1)
                return false;
        }

        this->enable_rx_bypass = res;
        return true;
    }

    bool set_disable_role_witch(bool val, bool ignore_return = false)
    {
        if (!this->isOpen)
            return false;
        if (!ignore_return) {
            lock_guard<mutex> lock(mutex_uart_cmd);
            mutex_uart_cmd_read.lock(true);
            std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);
        }

        bool res = val;
        const uint8_t cmd_buf[] = {ESP32_CMD_DISABLE_ROLE_SWITCH, val};
        uart->write(cmd_buf, sizeof(cmd_buf));

        if (ignore_return == false) {
            if (uart->read((uint8_t *)&res, 1) < 1) {
                mutex_uart_cmd_read.unlock();
                return false;
            }
        }

        this->disable_role_switch = res;
        return true;
    }

    bool set_enable_bypass_on_demand(bool val, bool ignore_return = false)
    {
        if (!this->isOpen)
            return false;
        if (!ignore_return) {
            lock_guard<mutex> lock(mutex_uart_cmd);
            mutex_uart_cmd_read.lock(true);
            std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);
        }
        bool res = val;
        const uint8_t cmd_buf[] = {ESP32_CMD_ENABLE_BYPASS_ON_DEMAND, val};
        uart->write(cmd_buf, sizeof(cmd_buf));

        if (ignore_return == false) {
            if (uart->read((uint8_t *)&res, 1) < 1)
                return false;
        }

        this->enable_bypass_on_demand = res;
        return true;
    }

    uint32_t get_latency()
    {
        if (!this->isOpen)
            return 0;
        lock_guard<mutex> lock(mutex_uart_cmd);
        mutex_uart_cmd_read.lock(true);
        std::unique_lock<priority_mutex> lk(mutex_uart_cmd_read, std::adopt_lock);

        uint8_t cmd = ESP32_CMD_PING;
        uint8_t tmp = 0;
        long measured_latency_ns = 0;
        long pings_completed = 0;

        struct timespec start_time = {0};
        struct timespec end_time = {0};
        // Dummy reads to avoid errors
        uart->read(&tmp, 1);
        uart->read(&tmp, 1);
        uart->read(&tmp, 1);

        for (size_t i = 0; i < N_LATENCY_TEST; i++) {
            clock_gettime(CLOCK_MONOTONIC, &start_time);

            uart->write(&cmd, 1);
            if ((uart->read(&tmp, 1) > 0) && (tmp == (uint8_t)ESP32_CMD_PING + 1)) {
                clock_gettime(CLOCK_MONOTONIC, &end_time);
                measured_latency_ns += ((end_time.tv_sec - start_time.tv_sec) * 1000000000UL) + (end_time.tv_nsec - start_time.tv_nsec);
                pings_completed += 1;
            }
        }

        if (pings_completed > 0) {
            this->latency = (measured_latency_ns / pings_completed) / 1000;
        }
        else {
            this->latency = 0;
        }

        return this->latency;
    }

    void OnScanComplete(function<void(unordered_map<string, scan_result_t> &)> fcn)
    {
        callback_scan_complete = fcn;
    }

    void ScanBT(function<void(string bdaddress, string name, int8_t rssi, string device_class)> fcn_callback)
    {
        static shared_ptr<React::TimeoutWatcher> scan_timeout = nullptr;
        hci_set_event_mask_t hci_pkt1;     // Enable all HCI events
        hci_write_inquiry_mode_t hci_pkt2; // Enable RSSI and EIR
        hci_inquiry_t hci_pkt3;            // Enquiry (BT Scan)

        // Call hci tx callbacks previously register by user
        if (callback_hci_tx) {
            callback_hci_tx((uint8_t *)&hci_pkt1, sizeof(hci_pkt1));
            callback_hci_tx((uint8_t *)&hci_pkt2, sizeof(hci_pkt2));
            callback_hci_tx((uint8_t *)&hci_pkt3, sizeof(hci_pkt3));
        }
        mutex_hci_bridge.lock();
        // Send HCI_SET_EVT_MASK to enable ESP32 to send us inquiry result events
        send_raw((uint8_t *)&hci_pkt1, sizeof(hci_pkt1));
        // Send HCI_WRITE_INQUIRY_MODE to enable extended inquiry results to we can get device name
        send_raw((uint8_t *)&hci_pkt2, sizeof(hci_pkt2));
        // Finally send HCI_INQUIRY to start scanning for 10.25 seconds.
        send_raw((uint8_t *)&hci_pkt3, sizeof(hci_pkt3));
        this->counter_hci_to_driver += 3;
        mutex_hci_bridge.unlock();
        // Clear previous scan results
        scan_results.clear();
        // Clear scan timeout timer
        if (scan_timeout != nullptr)
            scan_timeout->cancel();

        // Set user callback
        callback_scan = fcn_callback;
        // Set flag to indicate that scanning is in progress
        is_scanning = true;

        GL1Y("BT Scanning Started (Inquiry)...");
        // Start scan timeout timer
        scan_timeout = loop.onTimeoutMS(10250, [&]() {
            // About 10.25 seconds
            if (is_scanning) {
                // Force finish scanning if we didn't receive a inquiry complete previously via hci
                is_scanning = false;
                GL1Y("BT Scanning Finished, got ", scan_results.size(), " result(s).");
                if (callback_scan_complete)
                    callback_scan_complete(this->scan_results);
            }
        });
    }

    void send_raw(uint8_t *src_array, uint16_t src_len)
    {
        lock_guard<mutex> lock(mutex_uart_cmd);

        if (this->uart) {
            this->uart->write(src_array, src_len);
        }
    }

    void send(uint8_t event, const vector<uint8_t> &data, int offset = 0)
    {
        send(event, (uint8_t *)&data[offset], data.size() - offset);
    }

    void disconnect()
    {
        // Force a disconnection
        uint8_t lmp_detach[] = {0x99, 0x03, 0x17, 0x00, 0xe, 0x13};
        send(ESP32_CMD_DATA_LMP, lmp_detach, sizeof(lmp_detach));
        static uint8_t hci_cmd_disconnect[] = {0x01, 0x3, 0xc, 0x0};
        send_raw(hci_cmd_disconnect, sizeof(hci_cmd_disconnect));
    }

    void send(uint8_t event, uint8_t *src_array, uint16_t src_len, int flags = 0)
    {
        static uint8_t uart_buffer[2048];

        // Check if src_array is valid

        uint16_t i = 0;
        uint16_t uart_pos = 0;
        uint8_t checksum = 0;
        uint16_t payload_len = src_len;

        // printf("Host:%d\n", src_len);

        lock_guard<mutex> lock(mutex_uart_cmd);
        if (this->isOpen && this->ready) {
            uart_buffer[uart_pos++] = event;
            if (event == ESP32_CMD_DATA_LMP)
                payload_len++;
            uart_buffer[uart_pos++] = payload_len & 0xFF;
            uart_buffer[uart_pos++] = payload_len >> 8;

            // Serial Payload
            uint8_t *payload = uart_buffer + uart_pos;
            if (event == ESP32_CMD_DATA_LMP) {
                payload[0] = flags;
                checksum += (uint8_t)flags;
                payload++;
            }

            // ACL Payload
            for (i = 0; i < src_len; i++) {
                payload[i] = src_array[i];
                checksum += payload[i];
            }
            payload[i] = checksum;

            // Write to serial
            int final_size = uart_pos + payload_len + 1;
            uart->write(uart_buffer, final_size);
            // printf("CHK: %d, i=%d, payload_len=%d,payload[i]=%d, final_size=%d\n", checksum, i, payload_len, payload[i], final_size);
            // printf("To ESP32:");
            // print_buffer(uart_buffer, final_size);
        }
    }

    void ResetHardware(Serial *probe_serial = nullptr)
    {

        if (probe_serial == nullptr)
            probe_serial = this->uart;

        if (probe_serial) {
            probe_serial->setRTS(true);
            probe_serial->setDTR(true);
            probe_serial->setRTS(false);
            probe_serial->setDTR(false);
            // Reset
            probe_serial->setRTS(true);
            probe_serial->flushInput();
            probe_serial->flush();
            usleep(10000);
            if (probe_serial) {
                probe_serial->setRTS(false);
                usleep(30000);
            }
        }
    }

    bool ConsumeBootBytes(Serial *probe_serial)
    {
        if (probe_serial == nullptr)
            probe_serial = this->uart;

        if (probe_serial) {
            // Flush invalid bytes received after esp32 reset
            int c = 0;
            probe_serial->read();
            probe_serial->available();
            if (probe_serial->available() == 0)
                return false;
            while (probe_serial->available() && (c++ < 20)) {
                probe_serial->available();
                probe_serial->read(probe_serial->available());
                usleep(50000);
                probe_serial->read();
            }
            if (c >= 20)
                return false;
            // Flush additional bytes
            probe_serial->read(1024);
            probe_serial->available();
            return true;
        }
        return false;
    }

    bool DiscoverDriver(string &port_name, int64_t &port_baud)
    {
        static string last_port;
        uint32_t scan_timeout = 200;

        auto ports = serial::list_ports();

        GL1Y(TAG, "Discovering Serial Port");

        for (auto &port : ports) {
            if (string_contains(port.port, "USB")) {
                uint64_t default_port_baud = 4000000U;
                // for (size_t i = 0; i < 2; i++)
                // {
                Serial *probe_serial = nullptr;
                try {
                    GL1Y(TAG, "Probing ", port.port, " at ", default_port_baud, " baudrate...");
                    // Open the port without changing it to low_latency
                    probe_serial = new Serial(port.port, default_port_baud, Timeout::simpleTimeout(scan_timeout), false, false);
                    ResetHardware(probe_serial);
                    // Flush invalid bytes received after esp32 reset
                    if (!ConsumeBootBytes(probe_serial))
                        goto close_serial;

                    // Probe version
                    uint8_t cmd = ESP32_CMD_VERSION;
                    probe_serial->write(&cmd, 1);
                    string ret = probe_serial->readline(10);
                    if (string_contains(ret, "1.")) {
                        port_name = port.port;
                        port_baud = default_port_baud;
                        last_port = port.port;
                        probe_serial->close();
                        delete probe_serial;
                        GL1G(TAG, "Got valid response from ", port.port);
                        return true;
                    }
                }
                catch (const std::exception &e) {
                    // Ignore error
                }
            close_serial:
                GL1R(TAG, "No response");
                if (probe_serial != nullptr) {
                    probe_serial->close();
                    delete probe_serial;
                }

                // Try again with 4000000
                // default_port_baud = 4000000;
                // }
            }
        }

        return false;
    }

    void close()
    {
        if (this->uart) {
            this->ready = false;
            this->uart->close();
        }
    }

    ~ESP32BTDriver()
    {
        if (this->isOpen) {
            // this->uart->close();
            if (this->pty_master > 0) {
                ::close(this->pty_master);
            }

            if (this->pty_slave > 0) {
                ::close(this->pty_slave);
            }
        }
    }
};

#endif