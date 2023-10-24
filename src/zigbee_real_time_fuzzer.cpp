#include <algorithm>
#include <csignal>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "libs/cxxopts/cxxopts.hpp"
#include "libs/log_misc_utils.hpp"
#include "libs/strtk.hpp"
#include "libs/termcolor.hpp"

#include "Machine.hpp"
#include "MiscUtils.hpp"
#include "Modules.hpp"
#include "PacketLogger.hpp"
#include "Process.hpp"
#include "drivers/ZnpRealTimeDriver.hpp"
#include "monitors/Monitors.hpp"
#ifndef BUILD_EXPLOITER
#include "Fitness.hpp"
#include "Fuzzing.hpp"
#include "python_modules/PythonRuntime.hpp"
#include "python_modules/PythonServer.hpp"
#endif

extern "C" {
#include "libs/logs.h"
#include "libs/profiling.h"
#include "wdissector.h"
}

#define CONFIG_FILE_PATH "configs/znp_real.json"
#define CAPTURE_LMP_FILE "logs/BTHost/capture_bluetooth.pcapng"
#define CAPTURE_LMP_FIFO_FILE "/tmp/bthost_fuzzer.pcapng"
#define MODULES_PATH "modules/exploits/zigbee/"

using namespace std;
using namespace React;

// --------- Variables ---------
// Summary
uint32_t counter_tx_packets;
uint32_t counter_rx_packets;
uint32_t counter_driver_reboots;
uint32_t counter_bt_clock;
uint32_t counter_crashes;
uint32_t counter_anomalies;
bool status_connection;

// Packet Handling
list_data<uint8_t> stored_packets;
unordered_map<int, shared_ptr<TimeoutWatcher>> dup_timeout_list;
shared_ptr<React::IntervalWatcher> timer_pkt_retry = nullptr;
shared_ptr<React::IntervalWatcher> timer_global = nullptr;
string selected_bt_program;
mutex hci_processing;
int dup_pkt_id = 0;
uint logs_count = 0;
uint duplicated_count = 0;
bool load_default_config = false;
bool scan_only = false;
bool no_gui = true;

// Module handling
module_request_t module_request = {0};

// --------- Instances ---------
ZnpRealTimeDriver ESP32Driver;
ProcessRunner BTProcess;
ProcessRunner WSProcess;
PacketLogger BTPacketLogger;
PacketLogger WSPacketLogger;
Monitors MonitorTarget;
WDModules ModulesExploits;
#ifndef BUILD_EXPLOITER
PythonServer Server;
Fitness WDFitness;
#endif

// --------- Prototypes ---------
void CrashCallback(bool is_timeout = false);
void GlobalTimoutCallback(uint global_timeout_param);
static void gui_summary();
static void BTControls();
static void gui_bt_program(const char *bytes, size_t n);
static void start_scan();

// Functions
struct host
{
    char *node;
    char *port;
    char *msg;
};

int create_socket(char *host, char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_protocol = 0;          /* Any protocol */

    if (host != NULL)
    {
        hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    }
    else
    {
        hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
        hints.ai_protocol = 0;       /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
    }

    int s = getaddrinfo(host, port, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully connect(2).
        If socket(2) (or connect(2)/bind(2)) fails, we (close the socket
        and) try the next address. */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        // printf("this is the socketaddr data %s\n", rp->ai_addr->sa_data[]);
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (host == NULL)
        {
            printf("%s\n", "host is null");
            if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) /* server socket */
                break;                                       /* Success */
        }
        else
        {
            if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) /* client socket */
                break;                                           /* Success */
        }

        close(sfd);
    }

    if (rp == NULL)
    { /* No address succeeded */
        fprintf(stderr, "Could not bind/connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result); /* No longer needed */

    return sfd;
}

void *
recv_msg(void *arg)
{
    int s;
    ssize_t nread;
    char buf[500];

    socklen_t peer_addr_len;
    struct sockaddr_storage peer_addr;
    struct host *me = (struct host *)arg;

    int sfd = create_socket(me->node, me->port);

    printf("receive sfd = %d\n", sfd);
    for (;;)
    {
        printf("ready to receive message\n");

        peer_addr_len = sizeof(struct sockaddr_storage);

        nread = recvfrom(sfd, buf, 500, 0,
                         (struct sockaddr *)&peer_addr, &peer_addr_len);

        printf(">>> %d\n", nread);
        if (nread == -1)
            continue; /* Ignore failed request */

        char host[NI_MAXHOST], service[NI_MAXSERV];

        s = getnameinfo((struct sockaddr *)&peer_addr,
                        peer_addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0){
            printf("Received %zd bytes from %s:%s\n",
                   nread, host, service);
            StateMachine.config.fuzzing.enable_mutation=false;
            loop.onTimeoutMS(60000, [](){
                std::cout << "timeout.............." << std::endl;
                StateMachine.config.fuzzing.enable_mutation=true;
        });}
        else
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
    }

    pthread_exit(NULL);
}

void create_threads(struct host *me)
{
    pthread_t tids[2];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // pthread_create(&tids[0], &attr, send_msg, receiver);
    pthread_create(&tids[1], &attr, recv_msg, me);

    // pthread_join(tids[0], NULL);
    // pthread_join(tids[1], NULL);
}
void signal_handler(int signal)
{
    static bool received_sigusr1 = false;
    switch (signal) {
    case SIGINT:
    case SIGUSR1:
        if (!received_sigusr1) {
            received_sigusr1 = true;
            // Avoid callbacks being executed during exit
            StateMachine.ClearCallbacks();
#ifndef BUILD_EXPLOITER
            Server.EnableEvents(false);
            WDFitness.SetEnable(false);
#endif
            // BTProcess.stop();
            // MonitorTarget.
            ESP32Driver.close();
            StateMachine.save(CONFIG_FILE_PATH);
            SaveAllLogs("logs/" + StateMachine.config.name);
            LOGY("Fuzzer Closed");
            exit(0);
        }

        break;

    default:
        break;
    }
}

// Call BT Scan function
void start_scan()
{
    ESP32Driver.ScanBT([&](string bdaddress, string name, int8_t rssi, string device_class) {
        loop.onTimeoutMS(1, [bdaddress, name, rssi, device_class]() {
            if (name.size())
                GL1M(ESP32Driver.TAG, "BDAddress: ", bdaddress, ", Name: ", name, ", RSSI: ", (int)rssi, ", Class: ", device_class);
            else
                GL1M(ESP32Driver.TAG, "BDAddress: ", bdaddress, ", RSSI: ", (int)rssi, ", Class: ", device_class);
#ifndef BUILD_EXPLOITER
            if (Server.clients && Server.enable_events) {
                json j;
                j["BDAddress"] = bdaddress;
                j["Name"] = name;
                j["RSSI"] = (int)rssi;
                j["Class"] = device_class;
                Server.SendEvent("Scan", j.dump());
            }
#endif
        });
    });
}

void start_live_capture()
{
    if (WSProcess.isRunning())
        WSProcess.stop(true, true);

    WSPacketLogger.SetPreInitCallback([&](const string file_path) {
        WSProcess.setDetached(true);
        WSProcess.init("bin/wireshark", "-k -i "s + file_path + " >/dev/null 2>&1", nullptr);
    });
    WSPacketLogger.init(CAPTURE_LMP_FIFO_FILE, pcpp::LinkLayerType::LINKTYPE_BLUETOOTH_HCI_H4_WITH_PHDR, false, true);
}

void setup_bt_program(bool start = false)
{
    // Config &conf = StateMachine.config;
    // string program_name = conf.options.default_programs[conf.options.program];
    // BTProcess.setup(program_name, "-u " + ESP32Driver.hci_bridge_name + " -a " + conf.bluetooth.target_bd_address + " --iocap " + to_string(conf.bluetooth.io_cap) + " --authreq " + to_string(conf.bluetooth.auth_req) + " --bounding " + to_string((conf.bluetooth.enable_bounding ? 1 : 0)), gui_bt_program);

    // if (start) {
    //     if (BTProcess.init())
    //         GL1Y("[BT Program] ", "Starting program ", program_name + " " + BTProcess.process_args);
    //     else
    //         GL1R("[BT Program] Error starting program ", program_name + " " + BTProcess.process_args);
    // }
}

inline void gui_driver_status(int d_status, const char *on_text, const char *off_text)
{
    int chb0 = 0;
    ImU32 d_color = (d_status ? ImGuiAl::Crt::CGA::Green : ImGuiAl::Crt::CGA::Red);

    ImGui::PushStyleColor(ImGuiCol_CheckMark, d_color);
    ImGui::PushStyleColor(ImGuiCol_Text, d_color);
    ImGui::SameLine();
    ImGui::RadioButton((d_status ? on_text : off_text), &chb0, 0);
    ImGui::PopStyleColor(2);
}

static void gui_bt_program(const char *bytes, size_t n)
{
    if (bytes[0] == '\n')
        return;

    string output = string(bytes, n);
    vector<string> lines;
    strtk::parse(output, "\n", lines);

    for (string &line : lines) {
        // TODO: expose filters and timeout on conf file
        if (line.size() && line[0] != '\n' && line[0] != '\0') {
            GL4(line);
            if (StateMachine.config.options.auto_restart && !BTProcess.stopping) {
                if ((line.find("GATT browser - DISCONNECTED") != string::npos)) {
                    if (BTProcess.process_started && !BTProcess.stopping) {
                        const char *r_msg = "[BT Program] Restart Triggered";
                        GL1Y(r_msg);
                        if (StateMachine.config.options.save_capture)
                            BTPacketLogger.writeLog(r_msg);
                        // Restart BT Program via another thread
                        thread *t = new thread([]() {
                            BTProcess.restart();
                        });
                        t->detach();
                    }
                }
            }
        }
    }
}

#ifndef BUILD_EXPLOITER
static void BTControls()
{
    //     static ImVec2 btn_size = ImVec2(80, 0);
    //     Config &conf = StateMachine.config;

    //     ImGui::Begin(ICON_FA_COGS " Options", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar);

    //     ImGui::Separator();
    //     if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {

    //         // BT Controls -------
    //         if (ImGui::BeginTabItem(ICON_FA_SLIDERS_H " BT Controls")) {
    //             bool disable_program_controls = BTProcess.process_started;

    //             ImGui::Text("BT Options");
    //             ImGui::Separator();

    //             if (ImGui::Checkbox("Save LMP Capture", &conf.options.save_capture)) {
    //                 if (!conf.options.save_capture)
    //                     BTPacketLogger.Close();
    //                 else
    //                     BTPacketLogger.init(CAPTURE_LMP_FILE, pcpp::LinkLayerType::LINKTYPE_BLUETOOTH_HCI_H4_WITH_PHDR);
    //             };
    //             ImGui::SameLine();

    //             if (ImGui::Checkbox("Live Capture", &conf.options.live_capture)) {
    //                 if (!conf.options.live_capture) {
    //                     WSPacketLogger.Close();
    //                 }
    //                 else {
    //                     static bool ws_opening_thread = false;
    //                     if (!ws_opening_thread) {
    //                         thread *ws_t = new thread([]() {
    //                             ws_opening_thread = true;
    //                             start_live_capture();
    //                             ws_opening_thread = false;
    //                         });
    //                     }
    //                 }
    //             };

    //             // gui_disable_items(disable_program_controls);
    //             // ImGui::ComboString("BT Program", &conf.options.default_programs, (int *)&conf.options.program);
    //             // ImGui::Separator();
    //             // gui_disable_items(!ESP32Driver.enable_bridge_hci);

    //             // char *own_bdaddr = (char *)conf.bluetooth.own_bd_address.c_str();
    //             // ImGui::PushItemWidth(130);
    //             // ImGui::InputText("Host BDADDR", own_bdaddr, conf.bluetooth.own_bd_address.capacity());
    //             // ImGui::PopItemWidth();
    //             // ImGui::SameLine();
    //             // ImGui::Checkbox("Randomize Host BDADDR", &conf.bluetooth.randomize_own_bt_address);

    //             // Buttons
    //             gui_disable_items(!ESP32Driver.ready);
    //             if (ImGui::Button(ICON_FA_SEARCH " Scan", btn_size)) {
    //                 start_scan();
    //             }
    //             gui_disable_items(false);

    //             char *bdaddr = (char *)conf.bluetooth.target_bd_address.c_str();
    //             ImGui::SameLine();
    //             ImGui::PushItemWidth(130);
    //             ImGui::InputText("Target BDADDR", bdaddr, conf.bluetooth.target_bd_address.capacity());
    //             ImGui::PopItemWidth();
    //             ImGui::SameLine();

    //             gui_driver_status(BTProcess.process_started, "RUNNING", "STOPPED");

    //             gui_disable_items(disable_program_controls || !ESP32Driver.ready);
    //             if (ImGui::Button(ICON_FA_PLAY " Start", btn_size)) {
    //                 setup_bt_program(true);
    //             }

    //             gui_disable_items(!disable_program_controls); // Disable stop if process is not running
    //             ImGui::SameLine();
    //             if (ImGui::Button(ICON_FA_STOP " Stop", btn_size)) {
    //                 GL1Y("[BT Controls] ", "Stopping program");
    //                 BTProcess.stop();
    //             }
    //             gui_disable_items(false);

    //             ImGui::Checkbox("Auto Start", &StateMachine.config.options.auto_start);
    //             ImGui::SameLine();
    //             ImGui::Checkbox("Auto Restart", &conf.options.auto_restart);

    //             ImGui::Separator();
    //             ImGui::Text("Security Options");

    //             ImGui::Checkbox("Enable Bounding", &conf.bluetooth.enable_bounding);
    //             ImGui::SameLine();
    //             if (ImGui::Checkbox("Disable Role Switch", &conf.bluetooth.disable_role_switch)) {
    //                 loop.onTimeoutMS(10, [&conf]() {
    //                     ESP32Driver.set_disable_role_witch(conf.bluetooth.disable_role_switch, true);
    //                 });
    //             }
    //             ImGui::PushItemWidth(20);
    //             ImGui::InputInt("IO Capabilities", (int *)&conf.bluetooth.io_cap, 0);
    //             ImGui::SameLine();
    //             HelpMarker("Display Only = 0\nDisplay Yes No = 1\nKeyboard Only = 2\nNo Input No Output = 3\nUnknown = 256");
    //             ImGui::SameLine();

    //             ImGui::InputInt("Auth. Requirements", (int *)&conf.bluetooth.auth_req, 0);
    //             ImGui::SameLine();
    //             HelpMarker("No MitM, No Bouding = 0\nMitM, No Bouding = 1\nNo MitM, Dedicated Bouding = 2\nMitM, Dedicated Bouding = 3\nNo MitM, General Bouding = 4\nMitM, General Bouding = 5");
    //             ImGui::PopItemWidth();
    //             ImGui::PushItemWidth(35);
    //             ImGui::InputText("PIN", (char *)conf.bluetooth.pin.c_str(), 5);
    //             ImGui::SameLine();
    //             HelpMarker("For Bluetooth v2.0 or older");
    //             ImGui::PopItemWidth();

    //             if (ImGui::CollapsingHeader(ICON_FA_BIOHAZARD " Exploits")) {
    //                 // Enable external C modules here
    //                 for (auto &m : ModulesExploits.modules_ptr) {
    //                     if (ImGui::RadioButton(m->name.c_str(), m->enable)) {
    //                         ModulesExploits.DisableAllModules();
    //                         ModulesExploits.enable_module(m->name, true);
    //                         // Force driver restart
    //                         ESP32Driver.close();
    //                     }
    //                 }
    //             }

    //             if (ImGui::CollapsingHeader(ICON_FA_VIAL " LMP Tests")) {
    //                 static ImVec2 btn_size = ImVec2(170, 0);

    //                 if (ImGui::Button("LMP_features_req", btn_size)) {
    //                     // Send LMP_features_req
    //                     uint8_t packet[] = {0x99, 0x03, 0x4f, 0x00, 0x4e, 0xbf, 0xfe, 0xf, 0xfe, 0xdb, 0xff, 0x7b, 0x87};
    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_features_res", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x4f, 0x00, 0x50, 0xbf, 0xee, 0xcd, 0xfe, 0xdb, 0xff, 0x7b, 0x87};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_features_req_ext", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x67, 0x00, 0xfe, 0x3, 0x1, 0x2, 0xb, 0x0, 0x0, 0x0, 0x0,
    //                                         0x0, 0x0, 0x0};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_features_req_ext2", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x67, 0x0, 0xfe, 0x3, 0x2, 0x2, 0x5f, 0x3, 0x0,
    //                                         0x0, 0x0, 0x0, 0x0, 0x0};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_features_res_ext", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x67, 0x00, 0xfe, 0x4, 0x1, 0x2, 0x1, 0x0, 0x0, 0x0, 0x0,
    //                                         0x0, 0x0, 0x0};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }

    //                 if (ImGui::Button("LMP_host_connection_req", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x0f, 0x00, 0x66};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_accepted", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x17, 0x00, 0x6, 0x33};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_setup_complete", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x0f, 0x00, 0x62};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_set_AFH", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x87, 0x00, 0x78, 0x48, 0x27, 0x59, 0x6, 0x1, 0xfe, 0xff, 0xff,
    //                                         0xfe, 0x8, 0xff, 0xff, 0xff, 0xff, 0x3f};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }

    //                 if (ImGui::Button("LMP_name_req", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x17, 0x00, 0x03, 0x00}; // Name request
    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_DHkey_Check", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x8f, 0x00, 0x82, 0xad, 0xb0, 0xf5, 0x9e, 0xfb, 0x98, 0x4f, 0x36,
    //                                         0x3a, 0x77, 0x17, 0x46, 0xd7, 0x1, 0xce, 0x2e};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_detach", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x17, 0x00, 0xe, 0x13};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_AU_RAND", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x3,
    //                                         0x8f, 0x0, 0x16, 0x9e, 0x63, 0x7a, 0x9d, 0xf5,
    //                                         0x47, 0x58, 0xb2, 0xd1, 0x80, 0x2c, 0xd7, 0xf2,
    //                                         0x65, 0x25, 0x2c, 0x20};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }

    //                 if (ImGui::Button("LMP_version_req", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x37, 0x00, 0x4b, 0x8, 0x60, 0x0, 0xe, 0x3};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_version_res", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x37, 0x00, 0x4d, 0x9, 0x2, 0x0, 0x0, 0x1};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_features_req = 5", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x47, 0x00, 0x4e, 0xbf, 0xfe, 0xf, 0xfe, 0xdb, 0xff, 0x7b, 0x87};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_features_req = 17", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x8f, 0x00, 0x4e, 0xbf, 0xfe, 0xf, 0xfe, 0xdb, 0xff, 0x7b, 0x87};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }

    //                 if (ImGui::Button("LMP_features_req (data)", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x03, 0x8e, 0x00, 0x4e, 0xbf, 0xfe, 0xf, 0xfe, 0xdb, 0xff, 0x7b, 0x87};

    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //                 ImGui::SameLine();
    //                 if (ImGui::Button("LMP_IO_Capability_req", btn_size)) {
    //                     uint8_t packet[] = {0x99, 0x3, 0x2f, 0x0, 0xfe, 0x19, 0x3, 0x0, 0x2, 0x7c};
    //                     ESP32Driver.send(ESP32_CMD_DATA_LMP, packet, sizeof(packet));
    //                 }
    //             }

    //             if (!ESP32Driver.enable_bridge_hci) {
    //                 ImGui::SameLine();
    //                 ImGui::PushStyleColor(ImGuiCol_Text, ImGuiAl::Crt::CGA::Yellow);
    //                 ImGui::TextUnformatted("[!] HCI Bridge Disabled");
    //                 ImGui::PopStyleColor();
    //             }

    //             ImGui::EndTabItem();
    //         }
    //         // Fuzzing -------
    //         if (ImGui::BeginTabItem(ICON_FA_RANDOM " Fuzzing")) {
    //             ImGui::Text("Fuzzing Options");
    //             ImGui::Separator();

    //             ImGui::Checkbox("Optimization", &conf.fuzzing.enable_optimization);

    //             ImGui::CheckboxInputInt("Packet Retry      ",
    //                                     "Timeout (ms)",
    //                                     &conf.fuzzing.packet_retry,
    //                                     (int *)&conf.fuzzing.packet_retry_timeout_ms,
    //                                     0, 10000);

    //             ImGui::CheckboxInputDouble("Packet Duplication",
    //                                        "Chance",
    //                                        &conf.fuzzing.enable_duplication,
    //                                        &conf.fuzzing.default_duplication_probability);

    //             ImGui::CheckboxInputDouble("Packet Mutation   ",
    //                                        "Chance ",
    //                                        &conf.fuzzing.enable_mutation,
    //                                        &conf.fuzzing.default_mutation_probability);

    //             ImGui::CheckboxInputDouble("Field Mutation    ",
    //                                        "Chance ",
    //                                        &conf.fuzzing.enable_mutation,
    //                                        &conf.fuzzing.default_mutation_field_probability);

    //             ImGui::PushItemWidth(123);
    //             ImGui::InputInt("Max Duplication Timeout", (int *)&conf.fuzzing.max_duplication_time, 100);
    //             conf.fuzzing.max_duplication_time = constrain((int)conf.fuzzing.max_duplication_time, 0, 10000);
    //             ImGui::PopItemWidth();

    //             ImGui::Separator();
    //             ImGui::ComboString("Mutator", &conf.fuzzing.default_mutators, (int *)&conf.fuzzing.mutator);
    //             ImGui::ComboString("Selector", &conf.fuzzing.default_selectors, (int *)&conf.fuzzing.selector);

    //             ImGui::Text("Stats");
    //             ImGui::Separator();
    //             ImGui::Text("Transitions: %d", StateMachine.stats_transitions);

    //             ImGui::EndTabItem();
    //         }

    //         // Model -------
    //         if (ImGui::BeginTabItem(ICON_FA_SITEMAP " Model")) {
    //             ImGui::Text("State Machine Model Options");
    //             ImGui::Separator();

    //             ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - 370) / 2.0f, 0));
    //             ImGui::SameLine();
    //             if (ImGui::Button(ICON_FA_FOLDER " Import Model")) {
    //                 string program_name;
    //                 if (conf.options.default_programs.size())
    //                     program_name = string_split(conf.options.default_programs[conf.options.program], "/").back();
    //                 else
    //                     program_name = conf.name;

    //                 ImGui::FileDialog("LoadModel", "Import Model File", "Model or Capture (*.json *.pcap *.pcapng){.json,.pcap,.pcapng}", conf.state_mapper.save_folder, program_name);
    //             }

    //             ImGui::SameLine();
    //             if (ImGui::Button(ICON_FA_SAVE " Save model")) {
    //                 string program_name;
    //                 if (conf.options.default_programs.size())
    //                     program_name = string_split(conf.options.default_programs[conf.options.program], "/").back();
    //                 else
    //                     program_name = conf.name;

    //                 ImGui::FileDialog("SaveModel", "Save Model File", ".json", conf.state_mapper.save_folder, program_name);
    //             }

    //             ImGui::SameLine();
    //             if (ImGui::Button(ICON_FA_OBJECT_GROUP " Merge model")) {
    //                 string program_name;
    //                 if (conf.options.default_programs.size())
    //                     program_name = string_split(conf.options.default_programs[conf.options.program], "/").back();
    //                 else
    //                     program_name = conf.name;

    //                 ImGui::FileDialog("MergeModel", "Load Models to merge into current model", "Model or Capture (*.json *.pcap *.pcapng){.json,.pcap,.pcapng}", conf.state_mapper.save_folder, program_name, 5);
    //             }

    //             ImGui::Separator();
    //             ImGui::Text("Visual");
    //             if (ImGui::Checkbox("Show All States", &conf.state_mapper.show_all_states)) {
    //                 gui_set_graph_dot(StateMachine.get_graph());
    //             }

    //             ImGui::Separator();
    //             ImGui::Text("State Mapper Options");
    //             ImGui::Checkbox("Enable Mapper", &conf.state_mapper.enable);
    //             ImGui::SameLine();
    //             HelpMarker("Create new states and transitions\nby analysing state mapping definition.");

    //             ImGui::EndTabItem();
    //         }
    //         // Monitor -------
    //         if (ImGui::BeginTabItem(ICON_FA_HEARTBEAT " Monitor")) {
    //             ImGui::Text("Monitor Options");
    //             ImGui::Separator();

    //             ImGui::Checkbox("Enable", &StateMachine.config.monitor.enable);
    //             ImGui::SameLine();
    //             ImGui::Text("Status:");
    //             ImGui::SameLine();
    //             gui_driver_status(MonitorTarget.IsOpen(), "ONLINE", "OFFLINE");

    //             ImGui::ComboString("Connection Type", &conf.monitor.monitor_type_list, (int *)&conf.monitor.monitor_type);
    //             ImGui::Separator();

    //             switch (conf.monitor.monitor_type) {
    //                 // Serial port
    //             case 0: {
    //                 string &d_port = conf.monitor.serial_uart.serial_port_name;
    //                 ImGui::PushItemWidth(100);
    //                 ImGui::InputText("Serial Port", (char *)d_port.c_str(), d_port.capacity());
    //                 ImGui::InputInt("Baud Rate   ", (int *)&conf.monitor.serial_uart.serial_baud_rate, 0);
    //                 ImGui::PopItemWidth();
    //             } break;
    //                 // SSH
    //             case 1: {
    //                 string &h_address = conf.monitor.ssh.ssh_host_address;
    //                 string &h_user = conf.monitor.ssh.ssh_username;
    //                 string &h_pass = conf.monitor.ssh.ssh_password;
    //                 string &h_cmd = conf.monitor.ssh.ssh_command;
    //                 ImGui::PushItemWidth(100);
    //                 ImGui::InputText("SSH Address", (char *)h_address.c_str(), h_address.capacity());
    //                 ImGui::InputInt("SSH Port   ", (int *)&conf.monitor.ssh.ssh_port, 0);
    //                 ImGui::InputText("SSH User", (char *)h_user.c_str(), h_user.capacity());
    //                 ImGui::InputText("SSH Password", (char *)h_pass.c_str(), h_pass.capacity());
    //                 ImGui::PopItemWidth();
    //                 ImGui::PushItemWidth(400);
    //                 ImGui::InputText("SSH Command", (char *)h_cmd.c_str(), h_cmd.capacity());
    //                 ImGui::PopItemWidth();
    //             } break;
    //             // Microphone
    //             case 2: {
    //             } break;
    //             }

    //             if (ImGui::Button("Clear Log", ImVec2(100, 0))) {
    //                 gui_log5.clear();
    //             }

    //             ImGui::EndTabItem();
    //         }
    //         // Server -------
    //         if (ImGui::BeginTabItem(ICON_FA_SERVER " Server")) {
    //             ImGui::Text("Server Options");
    //             ImGui::Separator();
    //             ImGui::Text("Status:");
    //             ImGui::SameLine();
    //             gui_driver_status(Server.started, "ONLINE", "OFFLINE");

    //             ImGui::Text("Current Setting: %s:%d", Server.listen_address.c_str(), Server.port);

    //             if (ImGui::Checkbox("Enable Server", &conf.server_options.enable)) {
    //                 if (conf.server_options.enable)
    //                     Server.init(conf, conf.options.main_thread_core);
    //                 else
    //                     Server.Stop();
    //             }
    //             ImGui::Checkbox("Enable Events", &StateMachine.config.server_options.enable_events);
    //             ImGui::Checkbox("Enable Logging", &conf.server_options.logging);

    //             ImGui::Separator();

    //             string &d_address = StateMachine.config.server_options.listen_address;
    //             ImGui::PushItemWidth(150);
    //             ImGui::ComboString(ICON_FA_PYTHON " Server Module",
    //                                &conf.server_options.server_modules_list,
    //                                (int *)&conf.server_options.server_module);
    //             ImGui::InputText(ICON_FA_GLOBE " Listen Address", (char *)d_address.c_str(), d_address.capacity());
    //             ImGui::InputInt(ICON_FA_IOXHOST " Listen Port", (int *)&conf.server_options.port, 0);
    //             ImGui::PopItemWidth();

    //             if (ImGui::Button("Reload", ImVec2(100, 0))) {
    //                 Server.Restart();
    //             }

    //             ImGui::EndTabItem();
    //         }

    //         // ESP32 Driver -------
    //         if (ImGui::BeginTabItem(ICON_FA_MICROCHIP " Driver")) {
    //             gui_driver_status(ESP32Driver.ready, "ONLINE", "OFFLINE");

    //             ImGui::Text("ESP32 Driver Options");
    //             ImGui::Separator();
    //             ImGui::Checkbox("Auto Discovery", &conf.bluetooth.serial_auto_discovery);

    //             gui_disable_items(conf.bluetooth.serial_auto_discovery);
    //             ImGui::PushItemWidth(100);
    //             string &d_port = StateMachine.config.bluetooth.serial_port;
    //             ImGui::InputText("Serial Port", (char *)d_port.c_str(), d_port.capacity());
    //             ImGui::PopItemWidth();

    //             ImGui::PushItemWidth(100);
    //             ImGui::InputInt("Baud Rate   ", (int *)&conf.bluetooth.serial_baud_rate, 0);
    //             ImGui::PopItemWidth();

    //             gui_disable_items(BTProcess.isRunning());
    //             if (ImGui::Button("Reconnect", ImVec2(100, 0))) {
    //                 ESP32Driver.close();
    //             }

    //             ImGui::Separator();

    //             if (ImGui::Checkbox("Bridge HCI  ", &StateMachine.config.bluetooth.bridge_hci)) {
    //                 loop.onTimeoutMS(10, []() {
    //                     ESP32Driver.enable_hci_bridge(StateMachine.config.bluetooth.bridge_hci);
    //                 });
    //             }
    //             ImGui::SameLine();
    //             if (ImGui::Checkbox("Intercept TX", &StateMachine.config.bluetooth.intercept_tx)) {
    //                 loop.onTimeoutMS(10, []() {
    //                     ESP32Driver.set_enable_intercept_tx(StateMachine.config.bluetooth.intercept_tx);
    //                 });
    //             }

    //             if (ImGui::Checkbox("LMP Sniffing", &StateMachine.config.bluetooth.lmp_sniffing)) {
    //                 loop.onTimeoutMS(10, []() {
    //                     ESP32Driver.set_enable_lmp_sniffing(StateMachine.config.bluetooth.lmp_sniffing);
    //                 });
    //             }
    //             ImGui::SameLine();
    //             if (ImGui::Checkbox("RX Bypass   ", &StateMachine.config.bluetooth.rx_bypass)) {
    //                 loop.onTimeoutMS(10, []() {
    //                     ESP32Driver.set_enable_rx_bypass(StateMachine.config.bluetooth.rx_bypass);
    //                 });
    //             }

    //             if (ImGui::Checkbox("Debug UART  ", &StateMachine.config.bluetooth.serial_enable_debug)) {
    //                 ESP32Driver.enable_uart_debug = StateMachine.config.bluetooth.serial_enable_debug;
    //             }
    //             ImGui::SameLine();
    //             if (ImGui::Checkbox("Bypass on Demand ", &StateMachine.config.bluetooth.rx_bypass_on_demand)) {
    //                 loop.onTimeoutMS(10, []() {
    //                     ESP32Driver.set_enable_bypass_on_demand(StateMachine.config.bluetooth.rx_bypass_on_demand);
    //                 });
    //             }

    //             if (ImGui::Checkbox("Debug HCI   ", &StateMachine.config.bluetooth.serial_enable_debug_hci)) {
    //                 ESP32Driver.enable_uart_debug_hci = StateMachine.config.bluetooth.serial_enable_debug_hci;
    //             }
    //             ImGui::SameLine();
    //             gui_disable_items(false);
    //             ImGui::Checkbox("Show NULL/POLL", &StateMachine.config.bluetooth.show_null_poll_packets);

    //             ImGui::EndTabItem();
    //         }
    //         else {
    //             gui_driver_status(ESP32Driver.ready, "ONLINE", "OFFLINE");
    //         }
    //         ImGui::EndTabBar();
    //     }

    //     ImGui::End();

    //     if (ImGui::CheckFileDialog("SaveModel", ImGuiWindowFlags_NoDocking) == 1) {
    //         string file_path = ImGui::CheckFileDialogPath();
    //         StateMachine.SaveModel(file_path.c_str());
    //         GL1G("Saved " + file_path);
    //     }

    //     if (ImGui::CheckFileDialog("LoadModel", ImGuiWindowFlags_NoDocking) == 1) {
    //         GL1Y("Loading Model...");
    //         string file_path = ImGui::CheckFileDialogPath();
    //         StateMachine.LoadModel(file_path.c_str());
    //         WebView.ExecuteFunction("SVGReset");
    //         GL1G("Loaded " + file_path);
    // #ifndef BUILD_EXPLOITER
    //         gui_summary();
    // #endif
    //     }

    //     if (ImGui::CheckFileDialog("MergeModel", ImGuiWindowFlags_NoDocking) == 1) {
    //         GL1Y("Loading Model...");
    //         for (auto &file_path : ImGui::CheckFileDialogPaths()) {
    //             StateMachine.LoadModel(file_path.c_str(), false, true);
    //             WebView.ExecuteFunction("SVGReset");
    //             GL1G("Loaded " + file_path);
    //         }
    //         GL1G("All models merged");

    // #ifndef BUILD_EXPLOITER
    //         gui_summary();
    // #endif
    //     }
}

void PacketViewer()
{
    static int last_pkt_line = -1;

    // GuiMemEditor.DrawWindow(ICON_FA_SIGN_IN_ALT " Raw Packets", (void *)&stored_packets[0], stored_packets.size());

    int pkt_line_n = gui_log2._line_selected;
    if (pkt_line_n == -1) {
        // Highlight the last packet bytes received
        // GuiMemEditor.GotoAddrAndHighlight(stored_packets.slice_last_offset(), stored_packets.size());
    }
    else {
        // Set autoscroll when selected new packet line
        if (last_pkt_line != pkt_line_n) {
            // GuiMemEditor.autoscrool = true;
            // last_pkt_line = pkt_line_n;
        }

        // Highlit bytes fo selected line
        int offset = stored_packets.slice_offset(pkt_line_n);
        // GuiMemEditor.GotoAddrAndHighlight(offset, offset + stored_packets.slice_size(pkt_line_n));
    }
}

static void gui_summary()
{
    // if (gui_enabled) {
    if (true) {

        GL3(ICON_FA_MICROCHIP " Driver Version:     ", ESP32Driver.fw_version);
        GL3(ICON_FA_POWER_OFF " Driver Reboots:     ", counter_driver_reboots);
        GL3(ICON_FA_USB " USB Latency:       ", ESP32Driver.latency, " us");

        if (Server.clients > 0)
            GL3G(ICON_FA_USER " Remote Clients:     ", Server.clients);
        else
            GL3Y(ICON_FA_USER_TIMES " Remote Clients:    ", Server.clients);

        if (status_connection)
            GL3G(ICON_FA_WIFI " BT Connection:     Connected");
        else
            GL3Y(ICON_FA_WIFI " BT Connection:     Disconnected");
        GL3G(ICON_FA_CLOCK " BT Clock:           ", counter_bt_clock);
        GL3G(ICON_FA_ARROW_RIGHT " TX Packets:         ", counter_tx_packets);
        GL3G(ICON_FA_ARROW_LEFT " RX Packets:         ", counter_rx_packets);

        GL3G(ICON_FA_ARROW_RIGHT " HCI To Driver:      ", ESP32Driver.counter_hci_to_driver);
        GL3G(ICON_FA_ARROW_LEFT " HCI From Driver:    ", ESP32Driver.counter_hci_from_driver);

        GL3G(ICON_FA_SITEMAP " Total States:      ", StateMachine.TotalStates());
        GL3G(ICON_FA_ARROWS_ALT " Total Transitions:  ", StateMachine.TotalTransitions());
        GL3C(ICON_FA_COMPRESS_ALT " Current State:      ", StateMachine.GetCurrentStateName());
        GL3C(ICON_FA_REPLY " Previous State:     ", StateMachine.GetPreviousStateName());

        GL3G(ICON_FA_CLONE " Dup. Packets Queue: ", dup_timeout_list.size());
        GL3Y(ICON_FA_EXCLAMATION_CIRCLE " Target Anomalies:   ", counter_anomalies);
        GL3R(ICON_FA_TIMES_CIRCLE " Target Crashes:     ", counter_crashes);
    }
}
#endif

static inline int prepare_packet(vector<uint8_t> &raw_pkt, int dir)
{
    // Insert code 0x09 to indicate to wireshark the correct payload via HCI
    // print_buffer(&raw_pkt[0], raw_pkt.size());
    raw_pkt.insert(raw_pkt.begin(), 0x09);
    packet_set_direction(dir);
    return 1 + 6;
}

static inline void save_packet(vector<uint8_t> ws_packet,
                               int direction, bool fuzzed = false, bool duplicated = false,
                               int packet_number = -1, bool valid = true)
{
    timeval capture_time;
    gettimeofday(&capture_time, NULL);
    bool live_capture = StateMachine.config.options.live_capture;
    bool save_capture = StateMachine.config.options.save_capture;

    // TODO: Register Pseudoheader externally
    if (save_capture)
        BTPacketLogger.SetPseudoHeader((uint32_t)(direction << 24));
    if (live_capture)
        WSPacketLogger.SetPseudoHeader((uint32_t)(direction << 24));

    if (duplicated) {
        const string txt_dup = "Duplicated from "s + to_string(packet_number);
        if (save_capture)
            BTPacketLogger.write(ws_packet, capture_time, txt_dup.c_str());
        if (live_capture)
            WSPacketLogger.write(ws_packet, capture_time, txt_dup.c_str());
    }
    else if (fuzzed) {
        // Store both the original and fuzzed packet
        int slice_idx = stored_packets.slices.size() - 1;
        int slice_offset = stored_packets.slices[slice_idx];
        int slice_size = stored_packets.slice_size(slice_idx);

        if (save_capture) {
            BTPacketLogger.write(&stored_packets[slice_offset], slice_size, capture_time);
            BTPacketLogger.write(ws_packet, capture_time, "Fuzzed from previous");
        }
        if (live_capture) {
            WSPacketLogger.write(&stored_packets[slice_offset], slice_size, capture_time);
            WSPacketLogger.write(ws_packet, capture_time, "Fuzzed from previous");
        }
    }
    else if (!valid) {
        string txt_invalid = "Invalid response at "s + StateMachine.GetCurrentStateName();
        if (save_capture)
            BTPacketLogger.write(ws_packet, capture_time, txt_invalid.c_str());
        if (live_capture)
            WSPacketLogger.write(ws_packet, capture_time, txt_invalid.c_str());
    }
    else {
        if (save_capture)
            BTPacketLogger.write(ws_packet, capture_time);
        if (live_capture)
            WSPacketLogger.write(ws_packet, capture_time);
    }
}

static bool process_packet(vector<uint8_t> &raw_pkt, int offset, uint32_t dir,
                           bool enable_state_mapper = false,
                           bool duplicated = false,
                           int *sent_dup_num = NULL)
{
    bool valid = false;
    static int retry_count = 0;

    if (raw_pkt.size()) {

        // ------------ State Mapping / Transitions ------------
        StateMachine.PrepareStateMapper();
        StateMachine.PrepareExcludes();
        // struct timespec start_time;
        // struct timespec end_time;
        // clock_gettime(CLOCK_MONOTONIC, &start_time);
        packet_dissect(&raw_pkt[0], raw_pkt.size());
        // clock_gettime(CLOCK_MONOTONIC, &end_time);
        // long measured_latency_ns = ((end_time.tv_sec - start_time.tv_sec) * 1000000000UL) + (end_time.tv_nsec - start_time.tv_nsec) / 1000;
        // LOG4("d:", dir, ",time=", measured_latency_ns);
        valid = StateMachine.RunStateMapper(StateMachine.config.state_mapper.enable || (!dir));
        StateMachine.RunExcludes();
        stored_packets.append_slice(&raw_pkt[0], raw_pkt.size());
        // Ignore excludes
        if ((StateMachine.CurrentExclude & Machine::EXCLUDE_DUPLICATION) ||
            (StateMachine.CurrentExclude & Machine::EXCLUDE_ALL))
            return valid;

        // TODO: Loop detection and reconnect
        if (timer_pkt_retry != nullptr) {

            if (duplicated || dir == DIRECTION_RX) {
                timer_pkt_retry->setMS(StateMachine.config.fuzzing.packet_retry_timeout_ms);
            }
            else {
                timer_pkt_retry->cancel();
                timer_pkt_retry = nullptr;
            }
        }

        if (duplicated)
            duplicated_count++;

        if (timer_global != nullptr && dir == DIRECTION_RX) {
            timer_global->set(StateMachine.config.options.global_timeout);
        }

        if (!duplicated && dir == DIRECTION_TX && StateMachine.config.fuzzing.packet_retry) {
            // Prepare retry packet
            uint8_t *buf = (uint8_t *)&stored_packets[stored_packets.slice_last_offset() + offset];
            vector<uint8_t> pkt_buf = raw_pkt;

            int buf_size = raw_pkt.size() - offset - 1; // - 1 for stored crc
            string pkt_summary = std::move(packet_summary());
            retry_count = 0;
            timer_pkt_retry = loop.onIntervalMS(
                StateMachine.config.fuzzing.packet_retry_timeout_ms, [pkt_buf, buf, buf_size, sent_dup_num, offset, pkt_summary]() -> bool {
                    // TODO: Expose as callback

                    if (retry_count >= 2) {
                        if (StateMachine.config.options.save_capture)
                            BTPacketLogger.writeLog("Retry Timeout");

                        GL1Y("[!] Retry Timeout! Reconnecting...");
                        BTProcess.restart();
                        return false;
                    }
                    else {
                        GL1Y("[!] Retry");

                        if (sent_dup_num)
                            *sent_dup_num = stored_packets.slices.size() + logs_count + duplicated_count;
                        if (TestPtrAccess(buf, buf_size) == 0) // Ensure buf addr is valid
                        {
                            hci_processing.lock();
                            // GL1Y is yellow whcih print on the terminal
                            // GL2Y is yellow which print on the gui
                            GL1Y(" TX --> ", pkt_summary);
                            save_packet(pkt_buf, DIRECTION_TX, false, true);
                            hci_processing.unlock();
                            ESP32Driver.send_raw((uint8_t *)&pkt_buf[0], pkt_buf.size());
                        }
                        else {
                            char txt[32];
                            snprintf(txt, sizeof(txt), "0x%08X", buf);
                            GL1R("ERROR: Retry buffer pointer corruption, addr=", txt);
                        }
                        retry_count++;
                        return true;
                    }
                });
        }
    }

    return valid;
}

#ifndef BUILD_EXPLOITER
static inline bool packet_fuzzing(vector<uint8_t> &pkt, int offset = 0, int *sent_dup_num = NULL)
{
    bool ret = false;
    uint8_t fuzz_fields = 0;
    uint8_t fuzz_layers = 0;
    bool fuzz_layer = false;

    if ((StateMachine.CurrentExclude & Machine::EXCLUDE_MUTATION) ||
        (StateMachine.CurrentExclude & Machine::EXCLUDE_ALL))
        return false;


    if (StateMachine.config.fuzzing.enable_mutation) {

        // GL1M("Fuzzed packet ", stored_packets.slices.size(), " offset:", r_idx, " : ", packet_summary());
        // // ret = true;
        // struct timespec start_time;
        // struct timespec end_time;
        // clock_gettime(CLOCK_MONOTONIC, &start_time);
        // this function will navigate to all fields then decides whether need to fuzzer it or not
        packet_navigate_cpp(0, 0, [&](proto_tree *subnode, uint8_t field_type, uint8_t *pkt_buf) -> uint8_t {
            switch (field_type) {
            case WD_TYPE_FIELD: {
                if (fuzz_layer == false)
                    break;
                // uint64_t mask = packet_read_field_bitmask(subnode->finfo);
                // printf("     Field: %s, Size=%d, Type=%s, Offset=%d, Mask=%02X, Bit=%d\n",
                //        subnode->finfo->hfinfo->name, subnode->finfo->length,
                //        subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo),
                //        mask, packet_read_field_bitmask_offset(mask));
                float r = ((float)rand()) / ((float)RAND_MAX);
                // LOG1(r);
                // LOG1(WDFitness.x[StateMachine.GetCurrentStateNumber()]);
                float chanceThreshold;
                if (!fuzz_fields) // Apply probability backoff for m
                    chanceThreshold = StateMachine.config.fuzzing.default_mutation_field_probability;
                else
                    chanceThreshold = StateMachine.config.fuzzing.default_mutation_field_probability *
                                      StateMachine.config.fuzzing.field_mutation_backoff_multipler;
                // float chanceThreshold = WDFitness.x[StateMachine.GetCurrentStateNumber()];
                // std::cout<<"this is the result"<<r << " and " << chanceThreshold <<std::endl;
                if (r <= chanceThreshold){
                // LOG1(WDFitness.x[StateMachine.GetCurrentStateNumber()]);
                // uint32_t decision_offset = StateMachine.GetCurrentStateGlobalOffset();

                // if (decision_offset >= WDFitness.x.size())
                //     // Discard unmapped packets to be fuzzed (TODO: switch to a different criteria)
                //     return 0;
                // std::cout<<"This is the value: "<< WDFitness.x[decision_offset]<<std::endl;
                // if (r <= WDFitness.x[decision_offset]) {
                ret = true;
                uint8_t r_value = (uint8_t)g_random_int_range(0, 255);

                pkt[packet_read_field_offset(subnode->finfo)] = r_value;
                fuzz_fields++;
                }
            } break;
            case WD_TYPE_GROUP:
                // printf("\033[36m"
                //        "---> Group: %s, Type=%s, Size=%d\n"
                //        "\033[00m",
                //        subnode->finfo->hfinfo->name, subnode->finfo->value.ftype->name,
                //        subnode->finfo->length);
                break;
            case WD_TYPE_LAYER: {
                // printf("\033[33m"
                //        "==== Layer: %s, Type=%s, Size=%d\n"
                //        "\033[00m",
                //        subnode->finfo->hfinfo->name,
                //        subnode->finfo->value.ftype->name, subnode->finfo->length);
                fuzz_layers++;
                // float r = ((float)rand()) / ((float)RAND_MAX);
                // LOG1(WDFitness.x[StateMachine.GetCurrentStateNumber() + fuzz_layers]);
                // std::cout<<"This is the layer value"<<WDFitness.x[StateMachine.GetCurrentStateGlobalOffset() + fuzz_layers]<<std::endl;
                // if (r <= WDFitness.x[StateMachine.GetCurrentStateGlobalOffset() + fuzz_layers])
                fuzz_layer = true;
                // else
                //     fuzz_layer = false;
                break;
            }
            default:
                break;
            }

            // Stop iterating packet if maximum number of fuzzed fields is reached.
            if (fuzz_fields >= StateMachine.config.fuzzing.max_fields_mutation)
                return 0;
            else
                return 1;
        });
        // clock_gettime(CLOCK_MONOTONIC, &end_time);
        // long measured_latency_us = ((end_time.tv_sec - start_time.tv_sec) * 1000000000UL) + (end_time.tv_nsec - start_time.tv_nsec) / 1000;
        // LOG2("time=", measured_latency_us);
        // }
        // }
    }
    if (ret) {
        GL1M("Fuzzed ", (int)fuzz_fields, " fields : ", packet_summary());
    }

    return ret;
}
static inline bool packet_duplication(vector<uint8_t> &pkt, int offset = 0, int *sent_dup_num = NULL)
{
    static int id = 0;
    bool ret = false;

    if (StateMachine.config.fuzzing.enable_duplication) {
        // Ignore set_afh packets for now
        if ((StateMachine.CurrentExclude & Machine::EXCLUDE_DUPLICATION) ||
            (StateMachine.CurrentExclude & Machine::EXCLUDE_ALL))
            return false;

        float r = ((float)rand()) / ((float)RAND_MAX);

        if (r <= StateMachine.config.fuzzing.default_duplication_probability) {

            ret = true;
            string dup_summary = std::move(packet_summary());
            int dup_time = g_random_int_range(1, StateMachine.config.fuzzing.max_duplication_time);

            int dup_id = id;
            int pkt_n = stored_packets.slices.size() + logs_count;
            dup_timeout_list[dup_id] = loop.onTimeoutMS(dup_time,
                                                        [dup_time, dup_summary, pkt, dup_id, pkt_n, sent_dup_num, offset]() {
                                                            if (sent_dup_num)
                                                                *sent_dup_num = pkt_n;
                                                            // TODO: Add as callback

                                                            // ESP32Driver.send((uint8_t)ESP32_CMD_DATA_LMP, (uint8_t *)&pkt[offset], pkt.size() - offset - 1, 0);
                                                            hci_processing.lock();
                                                            packet_set_direction(DIRECTION_TX);
                                                            vector<uint8_t> tmp_pkt = pkt;
                                                            process_packet(tmp_pkt, 0, DIRECTION_TX, StateMachine.config.state_mapper.enable, true, &dup_pkt_id);
                                                            ESP32Driver.send_raw((uint8_t *)&pkt[0], pkt.size());
                                                            GL2Y(dup_pkt_id, ":[", packet_protocol(), "] ", packet_summary());
                                                            save_packet(pkt, DIRECTION_TX, false, true, dup_pkt_id);
                                                            hci_processing.unlock();
                                                            dup_timeout_list.erase(dup_id);
                                                        });
            id++;
        }
    }

    return ret;
}
#endif

static inline void packet_defragmenter(vector<uint8_t> ws_packet, int direction)
{
}

void MonitorCallback(string line)
{
    gui_log5.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, remove_colors((char *)line.c_str()));
}

void GlobalTimeoutCallback(uint initial_tx_packets)
{
    if (initial_tx_packets == counter_tx_packets) {
        GL1R("ERROR: Process hang detected, restarting BT Driver and BT program...");
        ESP32Driver.ResetHardware();
        BTProcess.restart(true);
    }
    else {
        GL1R("[Timeout] No Response received for ", StateMachine.config.options.global_timeout, " seconds");
        CrashCallback(true);
    }
}

void CrashCallback(bool is_timeout)
{
    counter_crashes += 1;
    string crash_msg;
    if (!is_timeout)
        crash_msg = "[Crash] Crash detected at state " + StateMachine.GetCurrentStateName();
    else
        crash_msg = "[Timeout] Target is not responding, check if target is still alive...";

    GL1R(crash_msg); // Log in events
    if (StateMachine.config.options.save_capture)
        BTPacketLogger.writeLog(crash_msg, true); // Log message to capture file
#ifndef BUILD_EXPLOITER
    Server.SendAnomaly("Crash", crash_msg, true);
    // gui_summary();
#endif

    if (module_request.stop_on_crash) {
        GL1Y("Exploit stopped, press CTRL+C to exit!");
        BTProcess.stop();
    }
    else {
        BTProcess.restart();
    }
}

void process_module_request(module_request_t &module_request)
{
    // Process modules request for TX injection
    while (module_request.tx_count) {
        module_request.tx_count--;
        vector<uint8_t> d_pkt_buf(module_request.pkt_buf, module_request.pkt_buf + module_request.pkt_len);
        packet_set_direction(DIRECTION_TX);
        process_packet(d_pkt_buf, 0, DIRECTION_TX, false, true);
        GL2Y("[", packet_protocol(), "] TX --> ", packet_summary()); // Dup TX
        ESP32Driver.send_raw(module_request.pkt_buf, module_request.pkt_len);
        save_packet(d_pkt_buf, DIRECTION_TX, false, true);
    }

    if (module_request.stop) {
        module_request.stop = 0;
        BTProcess.stop();
        GL1Y(ModulesExploits.TAG, "Stop requested");
    }
    else if (module_request.disconnect) {
        module_request.disconnect = 0;
        BTProcess.restart(true);
        GL1Y(ModulesExploits.TAG, "Disconnection requested");
    }
}

bool argsHasHelp(int argc, char **argv)
{
    for (size_t i = 0; i < argc; i++) {
        if (string(argv[i]) == "--help")
            return true;
        else if (string(argv[i]) == "--list-exploits")
            return true;
        else if (string(argv[i]) == "--scan")
            return true;
        else if (string(argv[i]) == "--default-config")
            load_default_config = true;
    }
    return false;
}

void parseArgs(int argc, char **argv)
{

    // clang-format off
#ifndef BUILD_EXPLOITER
    cxxopts::Options options("BT Fuzzer", "Bluetooth Classic Fuzzer (Baseband, LMP, L2CAP, etc)");
#else
    cxxopts::Options options("BT Exploiter", "Bluetooth Classic Exploiter (Baseband, LMP, L2CAP, etc)");
#endif
    options.add_options()
    ("help", "Print help")
    ("default-config", "Start with default config", cxxopts::value<bool>())
    ("autostart", "Automatically start", cxxopts::value<bool>()->default_value("true"))
#ifndef BUILD_EXPLOITER
    ("no-gui", "Start without GUI")
    ("test-webview", "Test GUI webview performance (requires internet)")
#endif
    ("live-capture", "Open wireshark in live capture mode", cxxopts::value<bool>()->implicit_value("true"))
    ("exploit", "Exploit Name", cxxopts::value<string>()->implicit_value(""))
    ("list-exploits", "List all exploits", cxxopts::value<bool>())
    ("host", "Host BDAddress", cxxopts::value<string>())
    ("host-port", "Host serial port name of BT Interface (ESP-WROVER-KIT)", cxxopts::value<string>())
    ("random_bdaddress", "Enable/Disable host BDAddress randomization", cxxopts::value<bool>()->implicit_value("true"))
    ("target", "Target BDAddress", cxxopts::value<string>()->default_value("/dev/ttyUSB1"))
    ("target-port", "Target serial port name to detect crashes", cxxopts::value<string>()->default_value("/dev/ttyUSB2"))
    ("target-baud", "Target baud rate", cxxopts::value<int>()->default_value("115200"))
    ("bounding", "Enable/Disable Bounding", cxxopts::value<bool>()->default_value("true"))
    ("iocap", "IO Capabilities", cxxopts::value<int>()->default_value("3"))
    ("authreq", "Authentication Request flag", cxxopts::value<int>()->default_value("3"))
    ("scan", "Scan BT Targets")
    ("mutation", "Enable/Disable mutation", cxxopts::value<bool>()->default_value("true"))
    ("duplication", "Enable/Disable duplication", cxxopts::value<bool>()->default_value("true"))
    ("optimization", "Enable/Disable optimization", cxxopts::value<bool>()->default_value("true"))
    ("max-iterations", "Max number of iterations to stop fuzzing", cxxopts::value<int>()->default_value("1000"))
    ("max-time", "Max time in minutes to stop fuzzing", cxxopts::value<int>()->default_value("240"))
    ;
    // clang-format on

    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            cout << options.help({"", "Group"}) << endl;
            exit(0);
        }

        if (result.count("exploit")) {
            string module_name = result["exploit"].as<string>();
            if (!module_name.size()) {
                LOGR("No exploit name");
                ModulesExploits.printAvailableModules();
                exit(1);
            }

            if (!ModulesExploits.enable_module(module_name, true)) {
                LOG2R("Exploit not found: ", module_name);
                ModulesExploits.printAvailableModules();
                exit(1);
            }
        }
        // if (result.count("list-exploits")) {
        //     ModulesExploits.printAvailableModules();
        //     exit(0);
        // }
        // if (result.count("autostart")) {
        //     StateMachine.config.options.auto_start = result["autostart"].as<bool>();
        // }
        // if (result.count("live-capture")) {
        //     StateMachine.config.options.live_capture = result["live-capture"].as<bool>();
        // }
        // if (result.count("host")) {
        //     StateMachine.config.bluetooth.own_bd_address = result["host"].as<string>();
        // }
        // if (result.count("host-port")) {
        //     StateMachine.config.bluetooth.serial_port = result["host-port"].as<string>();
        // }
        // if (result.count("target")) {
        //     StateMachine.config.bluetooth.target_bd_address = result["target"].as<string>();
        // }
        // if (result.count("target-port")) {
        //     StateMachine.config.monitor.serial_uart.serial_port_name = result["target-port"].as<string>();
        // }
        // if (result.count("target-baud")) {
        //     StateMachine.config.monitor.serial_uart.serial_baud_rate = result["target-baud"].as<int>();
        // }
        // if (result.count("random_bdaddress")) {
        //     StateMachine.config.bluetooth.randomize_own_bt_address = result["random_bdaddress"].as<bool>();
        // }
        // if (result.count("bounding")) {
        //     StateMachine.config.bluetooth.enable_bounding = result["bounding"].as<bool>();
        // }
        // if (result.count("iocap")) {
        //     StateMachine.config.bluetooth.io_cap = result["iocap"].as<int>();
        // }
        // if (result.count("authreq")) {
        //     StateMachine.config.bluetooth.io_cap = result["authreq"].as<int>();
        // }
        // if (result.count("scan")) {
        //     scan_only = true;
        // }
        // if (result.count("no-gui")) {
        //     no_gui = true;
        // }
    }
    catch (const cxxopts::OptionException &e) {
        LOG2R("error parsing option: ", e.what());
        exit(1);
    }
}

int main(int argc, char **argv)
{
    bool hasHelp = argsHasHelp(argc, argv);

    // Check root
    if (getuid() && !hasHelp) {
        LOGY("Not running as root.");
        exit(1);
    }
    no_gui = true;

// Initialize and configure GUI
#ifndef BUILD_EXPLOITER
    // GUI_Init(argc, argv, false);
    // gui_summary();
    // // Add custom GUI views
    // GuiMemEditor.ReadOnly = true;
    // GuiMemEditor.OptAddrDigitsCount = 8;
    // gui_add_user_fcn(BTControls);
    // gui_add_user_fcn(PacketViewer);
#else
    // Disable gui for exploiter build
    no_gui = true;
    GUI_Init(argc, argv, false);
#endif

    // Configure main process pthread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
    pthread_setname_np(pthread_self(), "main_thread"); // Set main thread name
    // Enable RR scheduler (low latency)
    if (!hasHelp) {
        set_affinity_no_hyperthreading();
        enable_rt_scheduler(1);
    }

    // Configure signals
    std::signal(SIGUSR1, signal_handler); // Received from GUI application to request closing
    std::signal(SIGINT, signal_handler);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGABRT);
    pthread_sigmask(SIG_BLOCK, &set, NULL); // Ignore SIGTERM (USE SIGINT instead)

    // Initialize State Machine and wdissector according to DefaultProtocol on config. file.
    if (!StateMachine.init(CONFIG_FILE_PATH, load_default_config)) {
        LOGR("Failure to load configuration.");
        exit(1);
    }

    Config &conf = StateMachine.config;
    Options &options = conf.options;
    Fuzzing &fuzzing = conf.fuzzing;

    // Enable Logs
    if (options.file_logger) {
        gui_log1.enableLogToFile(true, "logs/" + conf.name, "events", options.main_thread_core);
        gui_log2.enableLogToFile(true, "logs/" + conf.name, "session", options.main_thread_core);
        gui_log4.enableLogToFile(true, "logs/" + conf.name, "stack", options.main_thread_core);
        gui_log5.enableLogToFile(true, "logs/" + conf.name, "monitor", options.main_thread_core);
    }

    // Print to cout if GUI is disabled
    gui_log1.disableGUI(true);
    // gui_log2.disableGUI(!gui_enabled);
    // gui_log4.disableGUI(!gui_enabled);
    // if (conf.monitor.print_to_stdout)
    //     gui_log5.disableGUI(!gui_enabled);

    // Try to load program model
    GL1Y("Loading Model...");
    StateMachine.SetStateGlobalOffsetPadding(1);
    // Process pseudoheader when loading pcap files (set direction)
    StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
        if (pkt_len >= 4) {
            // Set packet direction
            packet_set_direction((int)pkt[3]);
            // Return pseudoheader offset
            offset = 4;
        }
    });
#ifndef BUILD_EXPLOITER
    // Call graph view on event
    StateMachine.OnTransition([](transition_t *transition) {
        // const string graph_str = StateMachine.get_graph();
        // Server.SendGraph(StateMachine.GetCurrentStateName(), graph_str);
        // gui_set_graph_dot(graph_str);
    });
#endif
    // Load .json model according to default program
    // if (StateMachine.LoadModel(options.default_programs[options.program].c_str(), true))
    if (StateMachine.LoadModel("znp_model", true))
        GL1G("Model Loaded. Total States:", StateMachine.TotalStates(),
             "  Total Transistions:", StateMachine.TotalTransitions(),
             "Model name",options.default_programs[options.program].c_str());
    else
        GL1R("Failed to load Model!");

    // Load / Compile extern exploit modules
    if (ModulesExploits.init(MODULES_PATH, &conf))
        GL1G("[Modules] ", ModulesExploits.modules_loaded, "/", ModulesExploits.modules_count, " Modules Compiled / Loaded");
    else {
        LOG5R("[Modules] ", ModulesExploits.modules_loaded, "/", ModulesExploits.modules_count, " Error when loading or compiling C Modules");
        exit(1);
    }
    // -> Set modules log callback (save to captures)
    set_wd_log_g([](const char *msg) {
        GL1G("[Modules] ", msg);
        BTPacketLogger.writeLog(msg);
#ifndef BUILD_EXPLOITER
        // Log to client on loop thread (we don't want to block main thread sending fancy messages from python)
        if (Server.clients && Server.enable_events) {
            int str_s = strnlen(msg, 1024);
            char *clean_string = new char[str_s];
            memcpy(clean_string, msg, str_s);

            loop.onTimeoutMS(5, [clean_string] {
                json j;
                j["level"] = "G";
                j["msg"] = remove_colors(clean_string);
                delete[] clean_string;
                Server.SendEvent("Modules", j.dump());
            });
        }
#endif
    });

    set_wd_log_y([](const char *msg) {
        GL1Y("[Modules] ", msg);
        BTPacketLogger.writeLog(msg);
#ifndef BUILD_EXPLOITER
        // Log to client on loop thread (we don't want to block main thread sending fancy messages from python)
        if (Server.clients && Server.enable_events) {
            int str_s = strnlen(msg, 1024);
            char *clean_string = new char[str_s];
            memcpy(clean_string, msg, str_s);

            loop.onTimeoutMS(5, [clean_string] {
                json j;
                j["level"] = "Y";
                j["msg"] = remove_colors(clean_string);
                delete[] clean_string;
                Server.SendEvent("Modules", j.dump());
            });
        }
#endif
    });

    set_wd_log_r([](const char *msg) {
        GL1R("[Modules] ", msg);
        BTPacketLogger.writeLog(msg, true);
#ifndef BUILD_EXPLOITER
        // Log to client on loop thread (we don't want to block main thread sending fancy messages from python)
        if (Server.clients && Server.enable_events) {
            int str_s = strnlen(msg, 1024);
            char *clean_string = new char[str_s];
            memcpy(clean_string, msg, str_s);

            loop.onTimeoutMS(5, [clean_string] {
                json j;
                j["level"] = "R";
                j["msg"] = remove_colors(clean_string);
                delete[] clean_string;
                Server.SendEvent("Modules", j.dump());
            });
        }
#endif
    });

    // Parse command line options
    parseArgs(argc, argv);
#ifndef BUILD_EXPLOITER
    // Initialize Python Modules (PythonServer)
    if (PythonCore.init()) {
        GL1Y(PythonCore.TAG, "Version ", PythonCore.version, " initialized");

        if (Server.init(conf)) {
            GL1G(Server.TAG, "Server initialized at ", Server.listen_address, ":", Server.port);

            Server.OnConnectionChange([](bool connection) {
                if (connection)
                    GL1C(Server.TAG, "Remote Client Connected");
                else
                    GL1C(Server.TAG, "Remote Client Disconnected");

#ifndef BUILD_EXPLOITER
                    // gui_summary();
#endif
            });

            Server.RegisterEventCallback(
                "Start", [&](py::list args) {
                    setup_bt_program(true);
                    return "OK";
                },
                "Start Fuzzer Session");

            Server.RegisterEventCallback(
                "Restart", [&](py::list args) {
                    BTProcess.restart(true);
                    return "OK";
                },
                "Restart Fuzzing Session");

            Server.RegisterEventCallback(
                "Stop", [&](py::list args) {
                    BTProcess.stop();
                    return "OK";
                },
                "Stop Fuzzing Session");

            Server.RegisterEventCallback(
                "Scan", [&](py::list args) {
                    start_scan();
                    return "OK";
                },
                "Scan for Bluetooth Classic targets");

            Server.RegisterEventCallback(
                "GetScanResults", [&](py::list args) -> string {
                    string res = "";
                    json j;
                    int i = 0;
                    for (auto &scan_result : ESP32Driver.scan_results) {
                        j[i]["BDAddress"] = scan_result.second.bd_addr;
                        j[i]["Name"] = scan_result.second.name;
                        j[i]["RSSI"] = (int)scan_result.second.rssi;
                        j[i]["Class"] = scan_result.second.device_class;
                        i++;
                    }

                    if (j.size())
                        res = j.dump();

                    return res;
                },
                "Returns last scanning results in JSON format");

            Server.RegisterEventCallback(
                "StartExploit", [&](py::list args) -> string {
                    bool ret = false;
                    if (args.size() > 0) {
                        auto module_name = args[0].cast<string>();
                        // ModulesExploits.DisableAllModules();
                        ret = ModulesExploits.enable_module(module_name, true);
                        // Force driver restart
                        ESP32Driver.close();
                    }

                    return (ret ? "OK" : "ERROR");
                },
                "Start an exploit by its name");

            Server.RegisterEventCallback(
                "StopExploit", [&](py::list args) {
                    // ModulesExploits.DisableAllModules();
                    BTProcess.stop();
                    return "OK";
                },
                "Stops currently running exploit");

            Server.RegisterEventCallback(
                "Summary", [&](py::list args) -> string {
                    json j;
                    j["FirmwareVersion"] = ESP32Driver.fw_version;
                    j["DriverReboots"] = counter_driver_reboots;
                    j["USBLatency"] = ESP32Driver.latency;
                    j["RemoteClients"] = Server.clients;
                    j["BTConnection"] = status_connection;
                    j["BTClock"] = counter_bt_clock;
                    j["TXPackets"] = counter_tx_packets;
                    j["RXPackets"] = counter_rx_packets;
                    j["HCIToDriver"] = ESP32Driver.counter_hci_to_driver;
                    j["HCIFromDriver"] = ESP32Driver.counter_hci_from_driver;
                    j["TotalStates"] = StateMachine.TotalStates();
                    j["TotalTransitions"] = StateMachine.TotalTransitions();
                    j["CurrentState"] = StateMachine.GetCurrentStateName();
                    j["PreviousState"] = StateMachine.GetPreviousStateName();
                    j["Dup.PacketsQueue"] = dup_timeout_list.size();
                    j["TargetAnomalies"] = counter_anomalies;
                    j["TargetCrashes"] = counter_crashes;
                    j["Iteration"] = WDFitness.GetIterations();
                    return j.dump();
                },
                "Returns summary of Fuzzing session");
        }
        else {
            GL1R(Server.TAG, "Server failed to initialize");
        }
    }

    // Init Fitness engine and register iteration callback
    WDFitness.init(StateMachine.TotalStatesLayers(1), 5);
    WDFitness.SetEnable(conf.fuzzing.enable_optimization);
    WDFitness.SetIterationCallback([&](const vector_double x) {
        if (x.size() >= 3) {
            // fuzzing.default_mutation_probability = x[0];
            // fuzzing.default_duplication_probability = x[1];
            // fuzzing.max_duplication_time = (int64_t)(x[2] * 4000.0);
            GL1Y("Mutation Probability: ", fuzzing.default_mutation_probability);
            GL1Y("Duplication Probability: ", fuzzing.default_duplication_probability);
            GL1Y("Max Duplication Time: ", fuzzing.max_duplication_time);
        }
    });
#endif

    // Save capture if enabled on config. file
    BTPacketLogger.SetPostLogCallback([&](const string msg, bool error) {
        logs_count++;
        if (conf.options.live_capture)
            WSPacketLogger.writeLog(msg, error);
    });
    if (options.save_capture) {
        BTPacketLogger.init(CAPTURE_LMP_FILE, pcpp::LinkLayerType::LINKTYPE_BLUETOOTH_HCI_H4_WITH_PHDR);
    }

    // Start live capture if enabled on config. file
    if (options.live_capture) {
        GL1Y("Opening Wireshark...");
        start_live_capture();
    }

#ifndef BUILD_EXPLOITER
    LOGC("Access Documentation:\nhttps://asset-sutd.gitlab.io/software/wireless-deep-fuzzer/");
#endif
    StateMachine.config.fuzzing.enable_mutation = false;
    struct host me;

    me.node = NULL;
    me.port = "9000";

    create_threads(&me);
    // Initialize ESP32 Driver
    bool driver_retry = !ESP32Driver.init(StateMachine.config);
    printf("Does it reach here 1\n");
    std::cout << driver_retry << std::endl;
    if (driver_retry)
        // printf("hello");
        LOGR("Serial port could not open.");
    else
        LOG5("Serial port ", ESP32Driver.uart_name, "@", ESP32Driver.uart_baud, " opened");

    // Save HCI TX Packets
    ESP32Driver.SetHCICallbackTX([&](uint8_t *pkt_buf, uint16_t pkt_len) {
        vector<uint8_t> v_pkt_buf(pkt_buf, pkt_buf + pkt_len);
        hci_processing.lock();
        packet_set_direction(DIRECTION_TX);
        // if (pkt_buf[0] == H4_ACL) {
        if (1) {
            bool pkt_fuzzed = ModulesExploits.run_tx_pre_dissection(&v_pkt_buf[0], v_pkt_buf.size(), &module_request);
            process_packet(v_pkt_buf, 0, DIRECTION_TX, StateMachine.config.state_mapper.enable);
#ifndef BUILD_EXPLOITER
            bool packet_duplicated = packet_duplication(v_pkt_buf, 0, &dup_pkt_id);
#endif
            pkt_fuzzed |= ModulesExploits.run_tx_post_dissection(&v_pkt_buf[0], v_pkt_buf.size(), &module_request);
#ifndef BUILD_EXPLOITER
            pkt_fuzzed |= packet_fuzzing(v_pkt_buf, 0);
#endif
            memcpy(pkt_buf, &v_pkt_buf[0], pkt_len);
            // string hex = strtk::convert_bin_to_hex(string(pkt_buf.get(), static_cast<size_t>(n)));
            // LOG3("[HCI] ", "HCI From Host:", hex);
            // uint8_t buf_crash[] = {0xfe,0x04,0x25,0x45,0x77,0x88,0x00,0x1e,0x85};
            if (pkt_fuzzed)
                GL1M("[", packet_protocol(), "] TX --> ", packet_summary()); // Fuzzed TX
            else
                GL1C("[", packet_protocol(), "] TX --> ", packet_summary()); // Normal TX
            GL1C("TX------> This is the statename: ", StateMachine.GetCurrentStateName());

            // if (memcmp(buf_crash,pkt_buf,sizeof(buf_crash))==0){
            //     CrashCallback(true);
            // }

            if((counter_tx_packets%10)==0){
                GL1C("Reset Iteration!!!!!!!!!!");
                WDFitness.Iteration(StateMachine.stats_transitions);
            }
            save_packet(v_pkt_buf, DIRECTION_TX, pkt_fuzzed, false);
        }
        // else {
        //     // Other HCI types
        //     ModulesExploits.run_tx_pre_dissection(&v_pkt_buf[0], v_pkt_buf.size(), &module_request);
        //     packet_dissect(&v_pkt_buf[0], v_pkt_buf.size());
        //     ModulesExploits.run_tx_post_dissection(&v_pkt_buf[0], v_pkt_buf.size(), &module_request);
        //     save_packet(v_pkt_buf, DIRECTION_TX);
        // }
        ++counter_tx_packets;
        hci_processing.unlock();
    });

    ESP32Driver.SetHCICallbackPostTX([]() {
        hci_processing.lock();
        process_module_request(module_request);
        hci_processing.unlock();
    });

    // Save HCI RX Packets
    ESP32Driver.SetHCICallbackRX([&](uint8_t *pkt_buf, uint16_t pkt_len) {
        bool pkt_valid = true;
        vector<uint8_t> v_pkt_buf(pkt_buf, pkt_buf + pkt_len);
        hci_processing.lock();
        packet_set_direction(DIRECTION_RX);
        ModulesExploits.run_rx_pre_dissection(&v_pkt_buf[0], v_pkt_buf.size(), &module_request);

        // if (pkt_buf[0] == H4_ACL) {
        pkt_valid = process_packet(v_pkt_buf, 0, DIRECTION_RX, StateMachine.config.state_mapper.enable, false, &dup_pkt_id);
        uint8_t buf_crash[] = {0xfe,0x01,0x65,0x45,0x00,0x21};
        if (memcmp(buf_crash,pkt_buf,sizeof(buf_crash))==0){
            CrashCallback(true);
        }
        GL1G("[", packet_protocol(), "] RX <-- ", packet_summary());
        // }
        // else {
        // Other HCI Events
        // packet_dissect(&v_pkt_buf[0], v_pkt_buf.size());
        // }
        pkt_valid &= !ModulesExploits.run_rx_post_dissection(&v_pkt_buf[0], v_pkt_buf.size(), &module_request);

        process_module_request(module_request);
        save_packet(v_pkt_buf, DIRECTION_RX, false, false, -1, pkt_valid);

        ++counter_rx_packets;
        hci_processing.unlock();
    });

    // If CLI requested only BT scanning, scan then exit
    if (scan_only) {
        ESP32Driver.OnScanComplete([](auto &scan_results) {
            kill(getpid(), SIGUSR1); // Notify main thread of closed window
        });
        start_scan();
    }
    else {
        // Initialize Monitor (UART / SSH / Microphone / ADB)
        if (MonitorTarget.setup(conf)) {
            MonitorTarget.SetCallback(MonitorCallback);
            MonitorTarget.SetCrashCallback(CrashCallback);
            MonitorTarget.init(conf);
            MonitorTarget.printStatus();
        }

        // Initialize BT Program (Do not start yet)
        BTProcess.SetStartCallback([&]() {
            if (timer_pkt_retry) {
                timer_pkt_retry->cancel();
                timer_pkt_retry = nullptr;
            }

            ESP32Driver.ClearHCIBridge(); // Kill process connected to slave pty
            // if (conf.bluetooth.randomize_own_bt_address) {
            //     ESP32Driver.randomize_bdaddr(); // Send new BDAddress to ESP32
            //     GL1Y("Host BDAddress randomized to ", conf.bluetooth.own_bd_address);
            // }

            if (conf.options.save_capture)
                BTPacketLogger.writeLog("------------------- BT Process Started -------------------");

            // Start global timeout
            if (timer_global == nullptr) {
                GL1Y("[!] Global timeout started with ", StateMachine.config.options.global_timeout, " seconds");
                uint global_timeout_param = counter_tx_packets;
                timer_global = loop.onInterval(StateMachine.config.options.global_timeout, [global_timeout_param]() {
                    GlobalTimeoutCallback(global_timeout_param);
                    return false;
                });
            }
        });
        BTProcess.SetStopCallback([&]() {
            ESP32Driver.disconnect();
            if (timer_pkt_retry) {
                timer_pkt_retry->cancel();
                timer_pkt_retry = nullptr;
            }
            if (timer_global) {
                timer_global->cancel();
                timer_global = nullptr;
            }
#ifndef BUILD_EXPLOITER
            if (conf.fuzzing.enable_optimization)
                WDFitness.Iteration(StateMachine.stats_transitions);
#endif
            StateMachine.GoToInitialState();
        });

        setup_bt_program(options.auto_start && !driver_retry);
        // hci_processing.lock();
        // packet_cleanup();
        // hci_processing.unlock();
    }

    set_affinity_core(StateMachine.config.options.main_thread_core);

    bool pkt_duplicated = false;
    bool pkt_fuzzed = false;
    bool pkt_valid = false;

    int count = 0;
    int pkt_offset = 0;
    int pkt_llid = 0;

    // unsigned long time_in_micros1 = 0;
    // unsigned long time_in_micros2 = 0;
    // struct timeval tv;
    // struct timeval tvf;
    // usleep(10000000);
    loop.onTimeoutMS(20000, [](){
         std::cout << "timeout.............." << std::endl;
         StateMachine.config.fuzzing.enable_mutation=true;
    });

    // StateMachine.config.fuzzing.enable_mutation=true;
    while (true) {
#ifndef BUILD_EXPLOITER
        // gui_summary();
#endif

        driver_event evt = ESP32Driver.receive();

        switch (evt.event) {
        case ESP32_CMD_DATA_TX: // Outgoing BT Packets
            // Normal TX
            counter_tx_packets += 1;
            counter_bt_clock = evt.bt_info.bt_clock;

            pkt_offset = prepare_packet(evt.data, DIRECTION_TX);
            // ModulesExploits.run_tx_pre_dissection(&evt.data[pkt_offset], evt.data.size() - pkt_offset, &module_request);

            process_packet(evt.data, pkt_offset, DIRECTION_TX, StateMachine.config.state_mapper.enable, false, &dup_pkt_id);
#ifndef BUILD_EXPLOITER
            packet_duplication(evt.data, pkt_offset, &dup_pkt_id);
#endif
            // printf("Intercept=%d\n", evt.bt_info.intercept_req);
            if (evt.bt_info.intercept_req) {

                // pkt_fuzzed = ModulesExploits.run_tx_post_dissection(&evt.data[pkt_offset], evt.data.size() - pkt_offset, &module_request);
#ifndef BUILD_EXPLOITER
                pkt_fuzzed = packet_fuzzing(evt.data, pkt_offset);
#endif
            }
            else {
                pkt_fuzzed = 0;
            }

            // Send back Bluetooth packet
            if (ESP32Driver.enable_intercept_tx && evt.bt_info.intercept_req) {
                ESP32Driver.send(ESP32_CMD_DATA_TX, evt.data, pkt_offset);
            }

            // Process modules request for TX injection
            // process_module_request(module_request);

            save_packet(evt.data, DIRECTION_TX, pkt_fuzzed, false);

            if (pkt_fuzzed)
                GL2M("[", packet_protocol(), "] ", packet_summary()); // Fuzzed TX
            else
                GL2C("[", packet_protocol(), "] ", packet_summary()); // Normal TX

            break;

        case ESP32_CMD_DATA_LMP: // Duplicated Packets
            // Duplicated TX
            counter_tx_packets += 1;
            counter_bt_clock = evt.bt_info.bt_clock;

            pkt_offset = prepare_packet(evt.data, DIRECTION_TX);

            process_packet(evt.data, pkt_offset, DIRECTION_TX, false, true);

            save_packet(evt.data, DIRECTION_TX, false, true, dup_pkt_id);

            GL2Y(dup_pkt_id, ":[", packet_protocol(), "] ", packet_summary());
            break;

        case ESP32_CMD_DATA_RX:
            // Normal RX
            counter_rx_packets += 1;
            counter_bt_clock = evt.bt_info.bt_clock;

            pkt_offset = prepare_packet(evt.data, DIRECTION_RX);
            // ModulesExploits.run_rx_pre_dissection(&evt.data[pkt_offset], evt.data.size() - pkt_offset, &module_request);

            // Only receive packets above NUll & POLL (BT Type > 1)
            // if (!StateMachine.config.bluetooth.show_null_poll_packets && ((evt.data[pkt_offset] >> 3) & 0xF) < 2) {
            //     // Process modules request
            //     process_module_request(module_request);
            //     break;
            // }

            pkt_valid = process_packet(evt.data, pkt_offset, DIRECTION_RX, StateMachine.config.state_mapper.enable, false);
            // pkt_valid &= !ModulesExploits.run_rx_post_dissection(&evt.data[pkt_offset], evt.data.size() - pkt_offset, &module_request);

            if (!pkt_valid)
                counter_anomalies += 1;

            // Process modules request
            // process_module_request(module_request);

            save_packet(evt.data, DIRECTION_RX, false, false, dup_pkt_id, pkt_valid);
            if (!scan_only) // Do not show RX during CLI Scanning
                GL2G("[", packet_protocol(), "] ", packet_summary());
            break;

        case ESP32_EVT_POLL: // Connection Poll
            counter_bt_clock = evt.bt_info.bt_clock;
            break;

        case ESP32_EVT_HW_ERROR:
        case ESP32_EVT_STARTUP: // Driver Startup / Reboot

            // Cancel any retry timeout
            if (timer_pkt_retry) {
                timer_pkt_retry->cancel();
                timer_pkt_retry = nullptr;
            }
            GL1Y("[!] ESP32 Driver Startup/Restart");
            counter_driver_reboots += 1;
            ESP32Driver.init(StateMachine.config);
            BTProcess.restart(conf.options.auto_start);
            break;

        case NULL: // Driver Error
            // Cancel any retry timeout
            if (timer_pkt_retry) {
                timer_pkt_retry->cancel();
                timer_pkt_retry = nullptr;
            }

            // Retry to open port on error
            if (!driver_retry)
                GL1R("[ESP32BT] Serial port ", ESP32Driver.uart_name, " error, retrying to open...");

            std::this_thread::sleep_for(1s);

            driver_retry = !ESP32Driver.init(StateMachine.config);

            if (driver_retry == false) {
                GL1G("[ESP32BT] Serial port ", ESP32Driver.uart_name, "@", ESP32Driver.uart_baud, " opened");
                BTProcess.restart(options.auto_start);
            }
            break;
        }
    }
}
