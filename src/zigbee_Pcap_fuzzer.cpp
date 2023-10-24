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

#include "libs/cxxopts/cxxopts.hpp"
#include "libs/log_misc_utils.hpp"
#include "libs/strtk.hpp"
#include "libs/termcolor.hpp"

#include "Fitness.hpp"
#include "Fuzzing.hpp"
#include "Machine.hpp"
#include "MiscUtils.hpp"
#include "Modules.hpp"
#include "PacketLogger.hpp"
#include "Process.hpp"
// #include "drivers/TestzigbeeDriver.hpp"
#include "drivers/znpPcapDriver.hpp"
#include "monitors/Monitors.hpp"
#include "python_modules/PythonHostAPConfig.hpp"
#include "python_modules/PythonRuntime.hpp"
#include "python_modules/PythonServer.hpp"
#include "wdissector.h"
extern "C" {
#include "libs/logs.h"
#include "libs/profiling.h"
}

#ifndef BUILD_EXPLOITER
#define FUZZER_NAME "Wi-Fi AP Fuzzer"
#define FUZZER_DESCRIPTION "Wi-Fi AP 802.11 Fuzzer (MAC, LLC, SNAP, EAPoL, etc)"
#else
#define FUZZER_NAME "Wi-Fi AP Exploiter"
#define FUZZER_DESCRIPTION "Wi-Fi AP 802.11 Exploiter (MAC, LLC, SNAP, EAPoL, etc)"
#endif

// change it for mqtt
// #define CONFIG_FILE_PATH "configs/NewMqtt.json"
#define CONFIG_FILE_PATH "configs/znp.json"
// #define CONFIG_FILE_PATH "configs/zigbee_final_config.json"
// #define CONFIG_FILE_PATH "configs/wifi_ap_config.json"
#define CAPTURE_FILE "logs/wifi_ap/capture_wifi_ap.pcapng"
#define CAPTURE_FIFO_FILE "/tmp/wifi_ap_fuzzer.pcapng"
#define CAPTURE_LINKTYPE pcpp::LinkLayerType::LINKTYPE_IEEE802_11_RADIOTAP
#define CONFIG_HOSTAPD_FILE "configs/wifi_ap/hostapd.conf"
#define MODULES_PATH "modules/exploits/wifi_ap/"

// --------- Instances ---------
znpPcapDriver WiFiDriver;
// TestzigbeeDriver WiFiDriver;
// ProcessRunner WiFiProcess;
ProcessRunner WSProcess;
PacketLogger WiFiPacketLogger;
PacketLogger WSPacketLogger;
Monitors MonitorTarget;
WDModules ModulesExploits;
PythonServer Server;
PythonHostAPDConfig HostAPDConfig;
Fitness WDFitness;

// Summary Variables
uint32_t counter_tx_packets;
uint32_t counter_rx_packets;
uint32_t counter_driver_reboots;
uint32_t counter_crashes;
uint32_t counter_anomalies;

// Packet Handling
list_data<uint8_t> stored_packets;
unordered_map<int, shared_ptr<TimeoutWatcher>> dup_timeout_list;
shared_ptr<React::IntervalWatcher> timer_pkt_retry = nullptr;
shared_ptr<React::IntervalWatcher> timer_global = nullptr;
uint logs_count = 0;
uint duplicated_count = 0;
bool load_default_config = false;

// Module handling
module_request_t module_request = {0};

void process_module_request(module_request_t &module_request)
{
    // LOG1(module_request.period);
    // Process modules request for TX injection
    if (!module_request.period) {
        while (module_request.tx_count) {
            module_request.tx_count--;
            // WiFiDriver.send(module_request.pkt_buf, module_request.pkt_len);
        }

        if (module_request.stop) {
            module_request.stop = 0;
            // WiFiDriver.stop();
            GL1Y(ModulesExploits.TAG, "Stop requested");
        }
        else if (module_request.disconnect) {
            module_request.disconnect = 0;
            // WiFiDriver.disconnect();
            GL1Y(ModulesExploits.TAG, "Disconnection requested");
        }
    }
    else {
        module_request_t delayed_m_request = module_request;
        module_request = {0};
        loop.onTimeoutMS(delayed_m_request.period, [delayed_m_request]() {
            for (size_t i = 0; i < delayed_m_request.tx_count; i++) {
                // WiFiDriver.send(delayed_m_request.pkt_buf, delayed_m_request.pkt_len);
            }

            if (delayed_m_request.stop) {
                // WiFiDriver.stop();
                GL1Y(ModulesExploits.TAG, "Stop requested");
            }
            else if (delayed_m_request.disconnect) {
                // WiFiDriver.disconnect();
                // WiFiDriver.Deauth();
                GL1Y(ModulesExploits.TAG, "Disconnection requested");
            }
        });
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
    cxxopts::Options options(FUZZER_NAME, FUZZER_DESCRIPTION);
    options.add_options()
    ("help", "Print help")
    ("default-config", "Start with default config", cxxopts::value<bool>())
    ("autostart", "Automatically start", cxxopts::value<bool>()->default_value("true"))
    ("exploit", "Exploit Name", cxxopts::value<string>()->implicit_value(""))
    ("fuzz", "Enable/Disable fuzzing", cxxopts::value<bool>()->default_value("true"))
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

        if (result.count("autostart")) {
            StateMachine.config.options.auto_start = result["autostart"].as<bool>();
        }

        if (result.count("fuzz")) {
            StateMachine.config.fuzzing.enable_mutation = result["fuzz"].as<bool>();
            StateMachine.config.fuzzing.enable_duplication = result["fuzz"].as<bool>();
            StateMachine.config.fuzzing.enable_optimization = result["fuzz"].as<bool>();
        }
    }
    catch (const cxxopts::OptionException &e) {
        LOG2R("error parsing option: ", e.what());
        exit(1);
    }
}

// Functions
void signal_handler(int signal)
{
    static bool received_sigusr1 = false;
    switch (signal) {
    case SIGINT:
    case SIGUSR1:
        LOG1("Received");
        if (!received_sigusr1) {
            received_sigusr1 = true;
            // WiFiDriver.stop();
            // Avoid callbacks being executed during exit
            StateMachine.ClearCallbacks();
            Server.EnableEvents(false);
            WDFitness.SetEnable(false);
            // WiFiProcess.stop();
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

static inline bool process_packet(fast_vector<uint8_t> &raw_pkt, int offset, uint32_t dir,
                                  bool duplicated = false,
                                  int *sent_dup_num = NULL,
                                  bool skip_mapping = false,
                                  bool skip_retry = false)
{
    bool valid = false;
    static int retry_count = 0;

    // Reset global timer on RX reception
    if (timer_global != nullptr && dir == DIRECTION_RX) {
        timer_global->set(StateMachine.config.options.global_timeout);
    }

    // ------------ State Mapping / Transitions ------------
    StateMachine.PrepareStateMapper();
    StateMachine.PrepareExcludes();
    // struct timespec start_time;
    // struct timespec end_time;
    // clock_gettime(CLOCK_MONOTONIC, &start_time);
    packet_set_direction(dir);
    packet_dissect(&raw_pkt[0], raw_pkt.size());
    // clock_gettime(CLOCK_MONOTONIC, &end_time);
    // long measured_latency_ns = ((end_time.tv_sec - start_time.tv_sec) * 1000000000UL) + (end_time.tv_nsec - start_time.tv_nsec) / 1000;
    // LOG4("d:", dir, ",time=", measured_latency_ns);
    stored_packets.slice();
    stored_packets.append((uint8_t[9]){0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 9);
    stored_packets.append(&raw_pkt[0], raw_pkt.size());

    if (duplicated)
        duplicated_count++;

    // Run Excludes filters
    StateMachine.RunExcludes();

    if (unlikely(!(StateMachine.CurrentExclude & Machine::EXCLUDE_MAPPING)))
        valid = StateMachine.RunStateMapper((!skip_mapping && (!dir)) || StateMachine.config.state_mapper.enable);

    if (unlikely(StateMachine.CurrentExclude & Machine::EXCLUDE_VALIDATION))
        valid = true;

    if (skip_retry || (!StateMachine.config.fuzzing.packet_retry))
        return valid;

    if (unlikely((StateMachine.CurrentExclude & Machine::EXCLUDE_RETRY) ||
                 (StateMachine.CurrentExclude & Machine::EXCLUDE_DUPLICATION) ||
                 (StateMachine.CurrentExclude & Machine::EXCLUDE_ALL)))
        return valid;

    if (timer_pkt_retry != nullptr) {
        if (duplicated || (dir == DIRECTION_RX)) {
            timer_pkt_retry->setMS(StateMachine.config.fuzzing.packet_retry_timeout_ms);
            return valid;
        }
        else {
            timer_pkt_retry->cancel();
            timer_pkt_retry = nullptr;
        }
    }

    // ------------ RETRY ------------
    if (dir == DIRECTION_TX) {
        // Prepare retry packet
        uint8_t *buf = (uint8_t *)&stored_packets[stored_packets.slice_last_offset() + offset];

        int buf_size = raw_pkt.size() - offset;
        retry_count = 0;
        // LOG3("dup in ", StateMachine.config.fuzzing.packet_retry_timeout_ms, " ms");

        timer_pkt_retry = loop.onIntervalMS(
            StateMachine.config.fuzzing.packet_retry_timeout_ms, [buf, buf_size, sent_dup_num, offset]() -> bool {
                // TODO: Expose as callback

                if (retry_count >= 2) {
                    if (StateMachine.config.options.save_capture)
                        WiFiPacketLogger.writeLog("Retry Timeout");

                    GL1Y("[!] Retry Timeout!");
                    // WiFiDriver.disconnect();
                    // WiFiDriver.Deauth();
                    // WiFiDriver.Deauth(100);
                    // WiFiDriver.Deauth(100);
                    timer_pkt_retry = nullptr;
                    return false;
                }
                else {
                    GL1Y("[!] Retry");

                    if (sent_dup_num)
                        *sent_dup_num = stored_packets.slices.size() + logs_count + duplicated_count;
                    if (TestPtrAccess(buf, buf_size) == 0) // Ensure buf addr is valid
                    {
                        // WiFiDriver.send(buf, buf_size);
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

    return valid;
}

static inline void save_packet(fast_vector<uint8_t> ws_packet,
                               int direction, bool fuzzed = false, bool duplicated = false,
                               int packet_number = -1, bool valid = true)
{
    timeval capture_time;
    gettimeofday(&capture_time, NULL);
    bool live_capture = StateMachine.config.options.live_capture;
    bool save_capture = StateMachine.config.options.save_capture;

    if (duplicated) {
        const string txt_dup = "Duplicated from "s + to_string(packet_number);
        if (save_capture)
            WiFiPacketLogger.write(ws_packet, capture_time, txt_dup.c_str());
        if (live_capture)
            WSPacketLogger.write(ws_packet, capture_time, txt_dup.c_str());
    }
    else if (fuzzed) {
        // Store both the original and fuzzed packet
        int slice_idx = stored_packets.slices.size() - 1;
        int slice_offset = stored_packets.slices[slice_idx];
        int slice_size = stored_packets.slice_size(slice_idx);

        if (save_capture) {
            WiFiPacketLogger.write(&stored_packets[slice_offset], slice_size, capture_time);
            WiFiPacketLogger.write(ws_packet, capture_time, "Fuzzed from previous");
        }
        if (live_capture) {
            WSPacketLogger.write(&stored_packets[slice_offset], slice_size, capture_time);
            WSPacketLogger.write(ws_packet, capture_time, "Fuzzed from previous");
        }
    }
    else if (!valid) {
        string txt_invalid = "Invalid response at "s + StateMachine.GetCurrentStateName();
        if (save_capture)
            WiFiPacketLogger.write(ws_packet, capture_time, txt_invalid.c_str());
        if (live_capture)
            WSPacketLogger.write(ws_packet, capture_time, txt_invalid.c_str());
    }
    else {
        if (save_capture)
            WiFiPacketLogger.write(ws_packet, capture_time);
        if (live_capture)
            WSPacketLogger.write(ws_packet, capture_time);
    }
}

static void gui_wifi_program(const char *bytes, size_t n)
{
    if (bytes[0] == '\n')
        return;

    string output = string(bytes, n);
    vector<string> lines;
    strtk::parse(output, "\n", lines);

    for (string &line : lines) {
        if (line.size() && line[0] != '\n' && line[0] != '\0') {
            GL4(line);
        }
    }
}

static inline bool packet_fuzzing(fast_vector<uint8_t> &pkt, int offset = 0, int *sent_dup_num = NULL, bool ignore_packet = false)
{
    if (ignore_packet)
        return false;

    bool ret = false;
    uint8_t fuzz_fields = 0;
    uint8_t fuzz_layers = 0;
    bool fuzz_layer = false;

    if ((StateMachine.CurrentExclude & Machine::EXCLUDE_MUTATION) || (StateMachine.CurrentExclude & Machine::EXCLUDE_ALL))
        return false;

    if (StateMachine.config.fuzzing.enable_mutation) {
        // acl_header_t acl_header;
        // acl_header.raw_header = *((uint16_t *)&pkt[offset + 2]);
        // if (acl_header.fields.llid == 0x02)
        // {
        // LOG1("acl_header.fields.llid");
        // float chance = ((float)rand()) / ((float)RAND_MAX);
        // if (chance <= StateMachine.config.fuzzing.default_mutation_probability)
        // {
        // GL1M("Fuzzed packet ", stored_packets.slices.size(), " offset:", r_idx, " : ", packet_summary());
        // // ret = true;
        // struct timespec start_time;
        // struct timespec end_time;
        // clock_gettime(CLOCK_MONOTONIC, &start_time);
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
                // float chanceThreshold;
                // if (!fuzz_fields) // Apply probability backoff for m
                //     chanceThreshold = StateMachine.config.fuzzing.default_mutation_field_probability;
                // else
                //     chanceThreshold = StateMachine.config.fuzzing.default_mutation_field_probability *
                //                       StateMachine.config.fuzzing.field_mutation_backoff_multipler;
                // float chanceThreshold = WDFitness.x[StateMachine.GetCurrentStateNumber()];

                // if (r <= chanceThreshold)
                // LOG1(WDFitness.x[StateMachine.GetCurrentStateNumber()]);
                uint32_t decision_offset = StateMachine.GetCurrentStateGlobalOffset();

                // if (decision_offset >= WDFitness.x.size())
                //     // Discard unmapped packets to be fuzzed (TODO: switch to a different criteria)
                //     return 0;

                // if (r <= WDFitness.x[decision_offset]) {
                if (r <= 0.8) {

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
                float r = ((float)rand()) / ((float)RAND_MAX);
                // LOG1(WDFitness.x[StateMachine.GetCurrentStateNumber() + fuzz_layers]);
                // if (r <= WDFitness.x[StateMachine.GetCurrentStateGlobalOffset() + fuzz_layers])
                if (r <= 0.8)

                    fuzz_layer = true;
                else
                    fuzz_layer = false;
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

static inline bool packet_duplication(fast_vector<uint8_t> &pkt, int offset = 0, int *sent_dup_num = NULL, bool skip_duplication = false)
{

    if (skip_duplication)
        return false;

    static int id = 0;
    bool ret = false;

    if (StateMachine.config.fuzzing.enable_duplication) {
        // Ignore set_afh packets for now
        if ((StateMachine.CurrentExclude & Machine::EXCLUDE_DUPLICATION) || (StateMachine.CurrentExclude & Machine::EXCLUDE_ALL))
            return false;

        float r = ((float)rand()) / ((float)RAND_MAX);

        if (r <= StateMachine.config.fuzzing.default_duplication_probability) {
            // LOG1((uint)acl_header.fields.llid);
            ret = true;
            string dup_summary = std::move(packet_summary());
            int dup_time = g_random_int_range(1, StateMachine.config.fuzzing.max_duplication_time);
            int dup_id = id;
            int pkt_n = stored_packets.slices.size() + logs_count;
            // LOG3("dup in ", dup_time, " ms");
            dup_timeout_list[dup_id] = loop.onTimeoutMS(dup_time,
                                                        [dup_time, dup_summary, pkt, dup_id, pkt_n, sent_dup_num, offset]() {
                                                            if (sent_dup_num)
                                                                *sent_dup_num = pkt_n;
                                                            // TODO: Add as callback
                                                            // WiFiDriver.send((uint8_t *)&pkt[0], pkt.size());
                                                            dup_timeout_list.erase(dup_id);
                                                        });
            id++;
        }
    }

    return ret;
}

void setup_wifi_program(bool start = false)
{
    Config &conf = StateMachine.config;
    string program_name = conf.options.default_programs[conf.options.program];
    // WiFiProcess.setup(program_name, "hostapd.conf", gui_wifi_program, true);

    // if (start) {
    //     if (WiFiProcess.init())
    //         GL1Y("[WiFi Program] ", "Starting program ", program_name + " " + HostAPDConfig.conf_file_path);
    //     else
    //         GL1R("[WiFi Program] Error starting program ", program_name + " " + HostAPDConfig.conf_file_path);
    // }
}

void MonitorCallback(string line)
{
    gui_log5.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, remove_colors((char *)line.c_str()));
}

void CrashCallback(bool is_timeout)
{
    string crash_msg;
    if (!is_timeout)
        crash_msg = "[Crash] Crash detected at state " + StateMachine.GetCurrentStateName();
    else
        crash_msg = "[Timeout] Target is not responding, check if target is still alive...";

    GL1R(crash_msg); // Log in events
    if (StateMachine.config.options.save_capture)
        WiFiPacketLogger.writeLog(crash_msg, true); // Log message to capture file
    Server.SendAnomaly("Crash", crash_msg, true);
}

int main(int argc, char **argv)
{
    bool hasHelp = argsHasHelp(argc, argv);

    // Check root
    if (!hasHelp && getuid()) {
        LOGY("Not running as root.");
        exit(1);
    }

    // Initialize and configure GUI
    GUI_Init(argc, argv, !hasHelp);

    // Configure main process pthread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
    pthread_setname_np(pthread_self(), "main_thread"); // Set main thread name

    // Configure signals
    std::signal(SIGUSR1, signal_handler); // Received from GUI application to request closing
    std::signal(SIGINT, signal_handler);
    // sigset_t set;
    // sigemptyset(&set);
    // sigaddset(&set, SIGTERM);
    // sigaddset(&set, SIGABRT);
    // pthread_sigmask(SIG_BLOCK, &set, NULL); // Ignore SIGTERM (USE SIGINT instead)

    // Initialize State Machine and wdissector according to DefaultProtocol on config. file.
    if (!StateMachine.init(CONFIG_FILE_PATH, load_default_config)) {
        LOGR("Failure to load configuration.");
        exit(1);
    }

    Config &conf = StateMachine.config;
    Options &options = conf.options;
    Fuzzing &fuzzing = conf.fuzzing;

    if (options.file_logger) {
        gui_log1.enableLogToFile(true, "logs/" + conf.name, "events");
        gui_log2.enableLogToFile(true, "logs/" + conf.name, "session");
        gui_log4.enableLogToFile(true, "logs/" + conf.name, "stack");
        gui_log5.enableLogToFile(true, "logs/" + conf.name, "monitor");
    }

    // Print to cout if GUI is disabled
    gui_log1.disableGUI(true);
    gui_log2.disableGUI(true);
    gui_log4.disableGUI(true);
    if (conf.monitor.print_to_stdout)
        gui_log5.disableGUI(true);

    // Try to load program model
    GL1Y("Loading Model...");
    StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
        packet_set_direction((int)pkt[8]);
        offset = 9;
    });
    // Call graph view on event
    StateMachine.OnTransition([](transition_t *transition) {
        const string graph_str = StateMachine.get_graph();
        Server.SendGraph(StateMachine.GetCurrentStateName(), graph_str);
    });
    // Load .json model according to default program
    if (StateMachine.LoadModel("mqttGenerated", true))
        // change it for mqtt to check it
        // if (StateMachine.LoadModel("mqtt", true))
        GL1G("Model Loaded. Total States:", StateMachine.TotalStates(),
             "  Total Transistions:", StateMachine.TotalTransitions());
    else
        GL1R("Failed to load Model!");
    // packet_set_protocol("encap:1");

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
        WiFiPacketLogger.writeLog(msg);
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
        WiFiPacketLogger.writeLog(msg);
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
        WiFiPacketLogger.writeLog(msg, true);
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

    // Init Wi-Fi Netlink Diver
    // update for new driver
    if (!WiFiDriver.init("./logs/znp/znp_converted.pcapng"))
        // if (!WiFiDriver.init("veth5", "10.47.0.1", "eth", "ec:f1:f8:d5:47:6b"))
        exit(1);

    // Initialize Monitor (UART / SSH / Microphone / ADB)
    if (MonitorTarget.setup(conf)) {
        MonitorTarget.SetCallback(MonitorCallback);
        MonitorTarget.SetCrashCallback(CrashCallback);
        MonitorTarget.init(conf);
        MonitorTarget.printStatus();
    }

    // Init Fitness engine and register iteration callback
    // WDFitness.init(StateMachine.TotalStatesLayers(1), 5);
    // WDFitness.SetEnable(conf.fuzzing.enable_optimization);
    WDFitness.SetIterationCallback([&](const vector_double x) {
        // TODO: Force disconnection here

        if (x.size() >= 3) {
            // fuzzing.default_mutation_probability = x[0];
            // fuzzing.default_duplication_probability = x[1];
            // fuzzing.max_duplication_time = (int64_t)(x[2] * 4000.0);
            // GL1Y("Mutation Probability: ", fuzzing.default_mutation_probability);
            // GL1Y("Duplication Probability: ", fuzzing.default_duplication_probability);
            GL1Y("Max Duplication Time: ", fuzzing.max_duplication_time);
        }
    });

    // Save capture if enabled on config. file
    WiFiPacketLogger.SetPostLogCallback([&](const string msg, bool error) {
        if (conf.options.live_capture)
            WSPacketLogger.writeLog(msg, error);
        logs_count++;
    });
    if (options.save_capture) {
        WiFiPacketLogger.init(CAPTURE_FILE, CAPTURE_LINKTYPE);
    }

    // Initialize Python Modules (PythonServer)
    if (PythonCore.init()) {
        GL1Y(PythonCore.TAG, "Version ", PythonCore.version, " initialized");

        HostAPDConfig.init(CONFIG_HOSTAPD_FILE);

        if (Server.init(conf)) {
            GL1G(Server.TAG, "Server initialized at ", Server.listen_address, ":", Server.port);

            Server.OnConnectionChange([](bool connection) {
                if (connection)
                    GL1C(Server.TAG, "Remote Client Connected");
                else
                    GL1C(Server.TAG, "Remote Client Disconnected");
            });

            Server.RegisterEventCallback(
                "Start", [&](py::list args) {
                    GL1Y("Start requested by remote client");
                    setup_wifi_program(true);
                    return "OK";
                },
                "Start Fuzzer Session");

            Server.RegisterEventCallback(
                "Restart", [&](py::list args) {
                    GL1Y("Restart requested by remote client");
                    // WiFiProcess.restart(true);
                    return "OK";
                },
                "Restart Fuzzing Session");

            Server.RegisterEventCallback(
                "Stop", [&](py::list args) {
                    GL1Y("Stop requested by remote client");
                    // WiFiProcess.stop();
                    return "OK";
                },
                "Stop Fuzzing Session");
        }
        else {
            GL1R(Server.TAG, "Server failed to initialize");
        }
    }

    LOGC("Access Documentation:\nhttps://asset-sutd.gitlab.io/software/wireless-deep-fuzzer/");
    GL1Y("Waiting Wi-Fi Communication to start on SSID=\"", conf.wifi.wifi_ssid, "\", Channel=", conf.wifi.wifi_channel);

    // Initialize Wi-Fi program
    setup_wifi_program(options.auto_start);

    // Start Wi-Fi TX/RX interrupt and TX interception
    // if (!WiFiDriver.start(true)) {
    //     WiFiProcess.stop();
    //     exit(1);
    // }

    bool pkt_duplicated = false;
    bool pkt_fuzzed = false;
    bool pkt_valid = false;
    int dup_pkt_id = 0;
    bool fuzzing_ready = false;
    bool flag_client_connected = false;

    while (true) {
        driver_event evt = WiFiDriver.receive();

        if (evt.buffer_size == 0) {
            usleep(100000);
            continue;
        }
        // std::cout << "This is the buffer size" << evt.buffer_size << std::endl;

        switch (evt.event) {
        // Wi-Fi Transmission
        case NETLINK_EVT_TX: {
            ++counter_tx_packets;
            uint8_t ignore_packet = 0;

            // if ((WIFI_Get_FrameSubType(evt.buffer) == WIFI_FRAME_SUBTYPE::WIFI_BEACON) ||
            //     (WIFI_Get_FrameSubType(evt.buffer) == WIFI_FRAME_SUBTYPE::WIFI_DATA_NULL))
            //     ignore_packet = 1;
            // StateMachine.GetCurrentStateName();
            // printf("THis is the current state" + StateMachine.GetCurrentStateName());

            // Main Fuzzing Loop
            // pkt_fuzzed = ModulesExploits.run_tx_pre_dissection(&evt.data[0], evt.data.size(), &module_request);
            process_packet(evt.data, 0, DIRECTION_TX, false, &dup_pkt_id, ignore_packet, ignore_packet);
            std::cout << "This is the current state name " << StateMachine.GetCurrentStateName() << std::endl;

            // packet_summary()
            // packet_layers()
            // pkt_duplicated = packet_duplication(evt.data, 0, &dup_pkt_id, ignore_packet);
            // pkt_fuzzed |= ModulesExploits.run_tx_post_dissection(&evt.data[0], evt.data.size(), &module_request);
            pkt_fuzzed |= packet_fuzzing(evt.data, 0, NULL, ignore_packet);
            // WiFiDriver.intercept_tx(evt.buffer, evt.buffer_size);

            // Do not display beacons or null data packets
            // if (ignore_packet)
            //     goto TX_END;

            if (pkt_fuzzed)
                GL2M("[", packet_protocol(), "] TX --> ", packet_summary()); // Fuzzed TX
            else
                GL2C("[", packet_protocol(), "] TX --> ", packet_summary()); // Normal TX

            GL2Y(StateMachine.GetCurrentStateName());

            // TX_END:
            // Process modules request for TX
            // process_module_request(module_request);
            // WiFiDriver.send_tx(&evt.data[0], evt.data.size());
            // WiFiDriver.send(&evt.data[0], evt.data.size());

            // Save packet
            // WiFiDriver.add_radio_tap(evt.data, DIRECTION_TX);
            // save_packet(evt.data, DIRECTION_TX, pkt_fuzzed, false);
        } break;

        // Wi-Fi Reception
        case NETLINK_EVT_RX: {
            // Do not process broadcast packets, except for probe requests
            // if (WIFI_Is_Broadcast(evt.buffer) &&
            //     (WIFI_Get_FrameSubType(evt.buffer) != WIFI_FRAME_SUBTYPE::WIFI_PROBEREQ))
            //     continue;

            ++counter_rx_packets;

            // Reception Loop
            // ModulesExploits.run_rx_pre_dissection(&evt.data[0], evt.data.size(), &module_request);

            pkt_valid = process_packet(evt.data, 0, DIRECTION_RX, false);
            // pkt_valid &= !ModulesExploits.run_rx_post_dissection(&evt.data[0], evt.data.size(), &module_request);

            if (!pkt_valid)
                counter_anomalies += 1;

            // Do not display received data null packets
            // if (WIFI_Get_FrameSubType(evt.buffer) == WIFI_FRAME_SUBTYPE::WIFI_QOS_DATA_NULL)
            //     goto RX_END;

            GL2G("[", packet_protocol(), "] RX <-- ", packet_summary()); // Normal TX

            GL2Y(StateMachine.GetCurrentStateName());
            // WiFiDriver.send_rx(&evt.data[0], evt.data.size());

            // Detect end of iteration
            // if ((WIFI_Get_FrameSubType(evt.buffer) == WIFI_FRAME_SUBTYPE::WIFI_DEAUTH ||
            //      WIFI_Get_FrameSubType(evt.buffer) == WIFI_FRAME_SUBTYPE::WIFI_DISASSOC) &&
            //     (flag_client_connected)) {
            //     flag_client_connected = false;
            //     WDFitness.Iteration(StateMachine.stats_transitions);
            // }
            // else if (WIFI_Get_FrameSubType(evt.buffer) == WIFI_FRAME_SUBTYPE::WIFI_AUTH) {
            //     if (flag_client_connected)
            //         WDFitness.Iteration(StateMachine.stats_transitions);
            //     flag_client_connected = true;
            // }

            // RX_END:
            // Process modules request
            // process_module_request(module_request);
            // cout << "prepare to send rx" << endl;
            // real_time
            // WiFiDriver.send_rx(&evt.data[0], evt.data.size());
            // cout << "send rs already" << endl;

            // Save packets
            // WiFiDriver.add_radio_tap(evt.data, DIRECTION_RX);
            // save_packet(evt.data, DIRECTION_RX, false, false, 0, pkt_valid);
        } break;

        case NETLINK_EVT_TX_INJECTED: {
            ++counter_tx_packets;
            // Do not display transmitted beacons
            if ((WIFI_Get_FrameSubType(evt.buffer) != WIFI_FRAME_SUBTYPE::WIFI_BEACON)) {
                process_packet(evt.data, 0, DIRECTION_TX, false, &dup_pkt_id, 1);
                GL2Y("[", packet_protocol(), "] TX --> ", packet_summary()); // Fuzzed TX
            }

            // Save packets
            // WiFiDriver.add_radio_tap(evt.data, DIRECTION_TX);
            // save_packet(evt.data, DIRECTION_TX, false, true, dup_pkt_id);
        } break;

        case NETLINK_EVT_TIMEOUT: {
            ++counter_driver_reboots;
            // Do not display transmitted beacons
            GL1R("Netlink connection timeout, restarting connection...");
            // WiFiDriver.start();
        } break;

        default:
            break;
        }
    }
}