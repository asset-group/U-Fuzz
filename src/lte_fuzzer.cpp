

#include "Fuzzing.hpp"
#include "Machine.hpp"
#include "Modules.hpp"
#include "Process.hpp"
#include "drivers/OAICommunication.hpp"
#include "libs/log_misc_utils.hpp"
#include "libs/oai_tracing/T_IDs.h"
#include "libs/refl.hpp"
#include "libs/termcolor.hpp"
#include <algorithm>
#include <csignal>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#define CONFIG_FILE_PATH "configs/enb_config.json"
#define FUZZ_PDCP_FIELD_EN 0
#define FUZZ_PDCP_FIELD_NAME "lte-rrc.integrityProtAlgorithm"
#define FILTER_TEST_FIELD 0

extern "C" {
#include "libs/logs.h"
#include "libs/profiling.h"
#include "libs/shared_memory.h"
#include "wdissector.h"
}

using namespace std;
using namespace refl;

// Summary
uint32_t counter_rx_packets = 0;
uint32_t counter_tx_packets = 0;
uint32_t counter_tx_downlink = 0;
uint32_t counter_tx_sib = 0;
uint32_t counter_tx_mib = 0;
uint32_t counter_tx_pdcp = 0;
uint32_t time_processing_dlsch = 0;
uint32_t time_processing_uplink = 0;

// Variables
// list_data<uint8_t> stored_packets;
unordered_map<int, shared_ptr<TimeoutWatcher>> dup_timeout_list;
pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t recv_cond = PTHREAD_COND_INITIALIZER;
std::mutex mutex_pdcp;
uint8_t recv_summary[2048];
uint8_t enable_logger = 0;
bool mutate_downlink = false;

// Misc
bool enable_state_mapper = true;

// Module handling
module_request_t module_request = {0};

// --------- Instances ---------
ProcessRunner Open5GSProcess;
ProcessRunner OpenAirInterfaceENB;
ProcessRunner OpenAirInterfaceUE;

void signal_handler(int signal)
{
    switch (signal) {
    case SIGUSR1:
        LOGY("GUI Closed");

        // Ensure open5gs is killed
        system("pkill open5gs");
        system("pkill lte-softmodem");
        StateMachine.ClearCallbacks(); // Avoid callbacks being executed during exit
        Open5GSProcess.stop(true);
        OpenAirInterfaceENB.stop(true);
        StateMachine.save(CONFIG_FILE_PATH);
        exit(0);
        break;

    default:
        break;
    }
}

// Update summary each 100ms
inline void gui_summary()
{
    // clang-format off
    GL5G(ICON_FA_ARROW_RIGHT  " Transmitted (TX): ", counter_tx_packets);
    GL5G(ICON_FA_ARROW_LEFT   " Received (RX):    ", counter_tx_packets);
    GL5G(ICON_FA_CLONE        " Broadcasts (SIB): ", counter_tx_sib);
    GL5G(ICON_FA_CLONE        " Broadcasts (MIB): ", counter_tx_mib);
    GL5G(ICON_FA_CLOCK        " DL-SCH Time (us): ", time_processing_dlsch);
    GL5G(ICON_FA_CLOCK        " UL Time (us):     ", time_processing_uplink);
    GL5G(ICON_FA_ARROW_RIGHT  " PDCP packets (TX):", counter_tx_pdcp);
    GL5G(ICON_FA_SITEMAP      " Total States:     ", StateMachine.TotalStates());
    GL5G(ICON_FA_ARROWS_ALT   " Total Transitions:", StateMachine.TotalTransitions());
    GL5Y(ICON_FA_COMPRESS_ALT " Current State:    ", StateMachine.GetCurrentStateName());
    GL5Y(ICON_FA_REPLY        " Previous State:   ", StateMachine.GetPreviousStateName());
    // clang-format on
}

string fuzz_field;

void LTEControls()
{
    static ImVec2 btn_size = ImVec2(80, 0);
    Config &conf = StateMachine.config;

    static int selected_fuzz_field = 0;
    static int last_selected_fuzz_field = -1;

    ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::Separator();
    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
        // if (ImGui::BeginTabItem(ICON_FA_BROADCAST_TOWER " LTE Controls"))
        // {
        //     // ImGui::TextUnformatted("EPS Status:");
        //     // ImGui::SameLine();
        //     // ImGui::LEDStatus(OAICommunication::OAISHMComm.zmq_connected, "Connected", "Offline");
        //     // ImGui::Separator();

        //     if (ImGui::CheckboxInputDouble("Packet Mutation   ",
        //                                    "Chance ",
        //                                    &conf.lte.test_field_enable,
        //                                    &conf.fuzzing.default_mutation_probability))
        //     {
        //         if (conf.lte.test_field_enable)
        //         {
        //             selected_fuzz_field = conf.lte.test_field;
        //             fuzz_field = conf.lte.default_test_fields[conf.lte.test_field];
        //             if (fuzz_field.find("nas_eps") != string::npos)
        //             {
        //                 mutate_downlink = false;
        //                 OAICommunication::OAISHMComm.mutate_nas_field(fuzz_field.c_str());
        //                 last_selected_fuzz_field = selected_fuzz_field;
        //             }
        //             else
        //             {
        //                 // OAICommunication::OAISHMComm.disable_mutate_nas_fields();
        //                 LOG2("Selected field: ", fuzz_field);
        //                 packet_register_set_field(fuzz_field.c_str(), FILTER_TEST_FIELD);
        //                 mutate_downlink = true;
        //                 last_selected_fuzz_field = selected_fuzz_field;
        //             }
        //         }
        //         else
        //         {
        //             mutate_downlink = false;
        //         }
        //     }

        //     // LOG1(fuzz_field);

        //     if (ImGui::ComboString("Field", &conf.lte.default_test_fields, (int *)&conf.lte.test_field), &fuzz_field)
        //     {

        //         if (conf.lte.test_field_enable)
        //         {
        //             selected_fuzz_field = conf.lte.test_field;
        //             if (selected_fuzz_field != last_selected_fuzz_field)
        //             {
        //                 fuzz_field = conf.lte.default_test_fields[conf.lte.test_field];
        //                 if (fuzz_field.find("nas_eps") != string::npos)
        //                 {
        //                     mutate_downlink = false;
        //                     OAICommunication::OAISHMComm.mutate_nas_field(fuzz_field.c_str());
        //                     last_selected_fuzz_field = selected_fuzz_field;
        //                 }
        //                 else
        //                 {
        //                     // OAICommunication::OAISHMComm.disable_mutate_nas_fields();
        //                     LOG2("Selected field: ", fuzz_field);
        //                     packet_register_set_field(fuzz_field.c_str(), FILTER_TEST_FIELD);
        //                     mutate_downlink = true;
        //                     last_selected_fuzz_field = selected_fuzz_field;
        //                 }
        //             }
        //         }
        //     }

        //     ImGui::Separator();

        //     ImGui::EndTabItem();
        // }

        // Fuzzing -------
        if (ImGui::BeginTabItem(ICON_FA_RANDOM " Fuzzing")) {
            ImGui::Text("Fuzzing Options");
            ImGui::Separator();

            ImGui::Checkbox("Optimization", &conf.fuzzing.enable_optimization);

            ImGui::CheckboxInputInt("Packet Retry      ",
                                    "Timeout (ms)",
                                    &conf.fuzzing.packet_retry,
                                    (int *)&conf.fuzzing.packet_retry_timeout_ms,
                                    0, 10000);

            ImGui::CheckboxInputDouble("Packet Duplication",
                                       "Chance",
                                       &conf.fuzzing.enable_duplication,
                                       &conf.fuzzing.default_duplication_probability);

            ImGui::CheckboxInputDouble("Packet Mutation   ",
                                       "Chance ",
                                       &conf.fuzzing.enable_mutation,
                                       &conf.fuzzing.default_mutation_probability);

            ImGui::CheckboxInputDouble("Field Mutation    ",
                                       "Chance  ",
                                       &conf.fuzzing.enable_mutation,
                                       &conf.fuzzing.default_mutation_field_probability);

            ImGui::PushItemWidth(123);
            ImGui::InputInt("Max Duplication Timeout", (int *)&conf.fuzzing.max_duplication_time, 100);
            conf.fuzzing.max_duplication_time = constrain((int)conf.fuzzing.max_duplication_time, 0, 10000);
            ImGui::PopItemWidth();

            ImGui::Separator();
            ImGui::ComboString("Mutator", &conf.fuzzing.default_mutators, (int *)&conf.fuzzing.mutator);
            ImGui::ComboString("Selector", &conf.fuzzing.default_selectors, (int *)&conf.fuzzing.selector);

            ImGui::Text("Stats");
            ImGui::Separator();
            ImGui::Text("Transitions: %d", StateMachine.stats_transitions);

            ImGui::EndTabItem();
        }

        // Model -------
        if (ImGui::BeginTabItem(ICON_FA_SITEMAP " Model")) {
            ImGui::Text("State Machine Model Options");
            ImGui::Separator();

            ImGui::Dummy(ImVec2((ImGui::GetWindowWidth() - 370) / 2.0f, 0));
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER " Import Model")) {
                string program_name;
                if (conf.options.default_programs.size())
                    program_name = string_split(conf.options.default_programs[conf.options.program], "/").back();
                else
                    program_name = conf.name;

                ImGui::FileDialog("LoadModel", "Import Model File", "Model or Capture (*.json *.pcap *.pcapng){.json,.pcap,.pcapng}", conf.state_mapper.save_folder, program_name);
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_SAVE " Save model")) {
                string program_name;
                if (conf.options.default_programs.size())
                    program_name = string_split(conf.options.default_programs[conf.options.program], "/").back();
                else
                    program_name = conf.name;

                ImGui::FileDialog("SaveModel", "Save Model File", ".json", conf.state_mapper.save_folder, program_name);
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_OBJECT_GROUP " Merge model")) {
                string program_name;
                if (conf.options.default_programs.size())
                    program_name = string_split(conf.options.default_programs[conf.options.program], "/").back();
                else
                    program_name = conf.name;

                ImGui::FileDialog("MergeModel", "Load Models to merge into current model", "Model or Capture (*.json *.pcap *.pcapng){.json,.pcap,.pcapng}", conf.state_mapper.save_folder, program_name, 5);
            }

            ImGui::Separator();
            ImGui::Text("Visual");
            if (ImGui::Checkbox("Show All States", &conf.state_mapper.show_all_states)) {
                gui_set_graph_dot(StateMachine.get_graph());
            }

            ImGui::Separator();
            ImGui::Text("State Mapper Options");
            ImGui::Checkbox("Enable Mapper", &conf.state_mapper.enable);
            ImGui::SameLine();
            HelpMarker("Create new states and transitions\nby analysing state mapping definition.");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    if (ImGui::CheckFileDialog("SaveModel", ImGuiWindowFlags_NoDocking) == 1) {
        string file_path = ImGui::CheckFileDialogPath();
        StateMachine.SaveModel(file_path.c_str());
        GL1G("Saved " + file_path);
    }

    if (ImGui::CheckFileDialog("LoadModel", ImGuiWindowFlags_NoDocking) == 1) {
        GL1Y("Loading Model...");
        string file_path = ImGui::CheckFileDialogPath();
        StateMachine.LoadModel(file_path.c_str());
        GL1G("Loaded " + file_path);
    }

    if (ImGui::CheckFileDialog("MergeModel", ImGuiWindowFlags_NoDocking) == 1) {
        GL1Y("Loading Model...");
        for (auto &file_path : ImGui::CheckFileDialogPaths()) {
            StateMachine.LoadModel(file_path.c_str(), false, true);
            GL1G("Loaded " + file_path);
        }
        GL1G("All models merged");
    }
}

void gui_open5gs_program(const char *bytes, size_t n)
{
    string output = string(bytes, n);
    vector<string> lines;
    strtk::parse(output, "\n", lines);

    for (string &line : lines) {
        if (line.size()) {
            gui_log7.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, remove_colors((char *)line.c_str()));
        }
    }
}

void gui_oai_enb_program(const char *bytes, size_t n)
{
    string output = string(bytes, n);
    vector<string> lines;
    strtk::parse(output, "\n", lines);

    for (string &line : lines) {
        if (line.size()) {
            gui_log8.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, remove_colors((char *)line.c_str()));
        }
    }
}

void gui_oai_ue_program(const char *bytes, size_t n)
{
    string output = string(bytes, n);
    vector<string> lines;
    strtk::parse(output, "\n", lines);

    for (string &line : lines) {
        if (line.size()) {
            gui_log9.add_msg_color(ImGuiAl::Crt::CGA::BrightWhite, remove_colors((char *)line.c_str()));
        }
    }
}

uint fuzz_offset = 0;
uint fuzz_value = 0;
bool fuzz_pkt(uint8_t *buf_data, uint16_t buf_size)
{
    bool ret = false;
    double chance = g_random_double_range(0.0, 1.0);

    // printf("chance=%03f\n", chance);
    // static char info_text[256];
    if (chance < StateMachine.config.fuzzing.default_mutation_probability) {
        ret = true;

        // field_info *field = packet_read_field(FILTER_TEST_FIELD); // Search and get fields info array by cached index 0
        // if (field)
        // {
        //     ret = true;
        //     uint64_t mask, offset;
        //     offset = packet_read_field_offset(field);
        //     mask = packet_read_field_bitmask(field);
        //     uint64_t field_value = *((uint64_t *)&buf_data[offset]);
        //     uint64_t field_fuzzed = (field_value & (~mask)) | (rand() & (~mask));

        //     uint64_t *x = (uint64_t *)&buf_data[offset];
        //     *x = field_fuzzed;
        //     sprintf(info_text, "Before:%04X,After:%04X,Offset:%d,Mask:%04X", field_value, field_fuzzed, offset, mask);
        //     GL1M(info_text);
        // }

        packet_navigate(1, 0, [](proto_tree *subnode, uint8_t field_type, uint8_t *pkt_buffer) -> uint8_t {
            uint64_t mask = packet_read_field_bitmask(subnode->finfo);
            uint field_format = subnode->finfo->value.ftype->ftype;

            switch (field_type) {
            case WD_TYPE_FIELD: {
                uint byte_offset = packet_read_field_offset(subnode->finfo);
                uint bit_offset = packet_read_field_bitmask_offset(mask);
                fuzz_offset = byte_offset;

                switch (field_format) {
                case FT_BOOLEAN: {
                    // Toggle bits
                    printf("Before:%02X\n", pkt_buffer[byte_offset]);
                    uint8_t val = ~pkt_buffer[byte_offset] & mask;
                    pkt_buffer[byte_offset] = (pkt_buffer[byte_offset] & ~mask) | val;
                    printf("After:%02X\n", pkt_buffer[byte_offset]);
                    fuzz_value = pkt_buffer[byte_offset];
                    break;
                }

                case FT_UINT8: {
                    // Random value
                    printf("Before:%02X\n", pkt_buffer[byte_offset]);
                    uint8_t o_value = pkt_buffer[byte_offset] & mask;
                    uint8_t val;
                    // do
                    // {
                    //     val = (g_random_int_range(0, 255) << bit_offset) & mask;
                    // } while (val == o_value);
                    val = (g_random_int_range(0, 255) << bit_offset) & mask;

                    pkt_buffer[byte_offset] = (pkt_buffer[byte_offset] & ~mask) | val;
                    fuzz_value = pkt_buffer[byte_offset];
                    printf("After:%02X\n", pkt_buffer[byte_offset]);
                }

                default:
                    break;
                }
                break;
            }

            case WD_TYPE_GROUP: {

                break;
            }

            case WD_TYPE_LAYER: {
                break;
            }

            default:
                break;
            }

            return 0;
        });

        buf_data[fuzz_offset] = fuzz_value;
    }

    return ret;
}

uint layer_count = 0;
uint field_count = 0;
static uint8_t test_navigate(proto_tree *subnode, uint8_t field_type, uint8_t *pkt_buffer)
{
    uint64_t mask = packet_read_field_bitmask(subnode->finfo);
    uint field_format = subnode->finfo->value.ftype->ftype;

    switch (field_type) {
    case WD_TYPE_FIELD:
        // printf("     Field: %s, Size=%d, Type=%s, Offset=%d, Mask=%02X, Bit=%d\n",
        //        subnode->finfo->hfinfo->name, subnode->finfo->length,
        //        subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo),
        //        mask, packet_read_field_bitmask_offset(mask));
        if (layer_count == 1 && field_count == 0 && mask) {
            // LOGY("Fuzzing first field, lets see what happens");
            uint byte_offset = packet_read_field_offset(subnode->finfo);
            uint bit_offset = packet_read_field_bitmask_offset(mask);
            // uint8_t val = (pkt_buffer[byte_offset] & mask) >> bit_offset;
            // printf("     Field: %s, Size=%d, Type=%s, Offset=%d, Mask=%02X, Bit=%d\n",
            //        subnode->finfo->hfinfo->name, subnode->finfo->length,
            //        subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo),
            //        mask, packet_read_field_bitmask_offset(mask));

            switch (field_format) {
            case FT_BOOLEAN: {
                // Toggle bits
                uint8_t val = ~pkt_buffer[byte_offset] & mask;
                pkt_buffer[byte_offset] = (pkt_buffer[byte_offset] & ~mask) | val;
                break;
            }

            case FT_UINT8: {
                // Random value
                uint8_t o_value = pkt_buffer[byte_offset] & mask;
                uint8_t val;
                do {
                    val = (g_random_int_range(0, 255) << bit_offset) & mask;
                } while (val == o_value);

                pkt_buffer[byte_offset] = (pkt_buffer[byte_offset] & ~mask) | val;
            }

            default:
                break;
            }
        }
        field_count++;
        break;
    case WD_TYPE_GROUP:
        // printf("\033[36m"
        //        "---> Group: %s, Type=%s, Size=%d\n"
        //        "\033[00m",
        //        subnode->finfo->hfinfo->name, subnode->finfo->value.ftype->name,
        //        subnode->finfo->length);
        break;
    case WD_TYPE_LAYER:
        // printf("\033[33m"
        //        "==== Layer: %s, Type=%s, Size=%d\n"
        //        "\033[00m",
        //        subnode->finfo->hfinfo->name,
        //        subnode->finfo->value.ftype->name, subnode->finfo->length);

        layer_count++;
        break;

    default:
        break;
    }

    return 0;
}

static inline bool packet_fuzzing(uint8_t *pkt, int offset = 0, int *sent_dup_num = NULL)
{
    bool ret = false;
    uint8_t fuzz_fields = 0;

    if (StateMachine.CurrentExclude == Machine::EXCLUDE_MUTATION || StateMachine.CurrentExclude == Machine::EXCLUDE_ALL)
        return false;

    if (StateMachine.config.fuzzing.enable_mutation) {
        // acl_header_t acl_header;
        // acl_header.raw_header = *((uint16_t *)&pkt[offset + 2]);
        // if (acl_header.fields.llid == 0x02)
        // {
        // LOG1("acl_header.fields.llid");
        float r = ((float)rand()) / ((float)RAND_MAX);
        if (r <= StateMachine.config.fuzzing.default_mutation_probability) {
            // GL1M("Fuzzed packet ", stored_packets.slices.size(), " offset:", r_idx, " : ", packet_summary());
            // // ret = true;
            struct timespec start_time;
            struct timespec end_time;
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            packet_navigate_cpp(1, 1, [&](proto_tree *subnode, uint8_t field_type, uint8_t *pkt_buf) -> uint8_t {
                switch (field_type) {
                case WD_TYPE_FIELD: {
                    uint64_t mask = packet_read_field_bitmask(subnode->finfo);
                    printf("     Field: %s, Size=%d, Type=%s, Offset=%d, Mask=%02X, Bit=%d\n",
                           subnode->finfo->hfinfo->name, subnode->finfo->length,
                           subnode->finfo->value.ftype->name, packet_read_field_offset(subnode->finfo),
                           mask, packet_read_field_bitmask_offset(mask));
                    double r = ((float)rand()) / ((float)RAND_MAX);
                    if (r <= StateMachine.config.fuzzing.default_mutation_field_probability) {
                        // LOG1("[fuzz]");
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
                case WD_TYPE_LAYER:
                    // printf("\033[33m"
                    //        "==== Layer: %s, Type=%s, Size=%d\n"
                    //        "\033[00m",
                    //        subnode->finfo->hfinfo->name,
                    //        subnode->finfo->value.ftype->name, subnode->finfo->length);
                    break;

                default:
                    break;
                }

                return 0;
            });
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            long measured_latency_us = ((end_time.tv_sec - start_time.tv_sec) * 1000000000UL) + (end_time.tv_nsec - start_time.tv_nsec) / 1000;
            LOG2("time=", measured_latency_us);
        }
        // }
        if (ret)
            GL1M("Fuzzed ", (int)fuzz_fields, " fields : ", packet_summary());
    }

    return ret;
}

static inline bool packet_duplication(uint8_t *pkt_buf, uint16_t pkt_len, int offset = 0, int *sent_dup_num = NULL)
{
    static int id = 0;
    bool ret = false;
    vector<uint8_t> pkt(pkt_buf, pkt_buf + pkt_len);

    if (StateMachine.config.fuzzing.enable_duplication) {
        // Ignore set_afh packets for now
        if (StateMachine.CurrentExclude == Machine::EXCLUDE_DUPLICATION || StateMachine.CurrentExclude == Machine::EXCLUDE_ALL)
            return false;

        double r = ((float)rand()) / ((float)RAND_MAX);

        if (r <= StateMachine.config.fuzzing.default_duplication_probability) {
            if (dup_timeout_list.size() > 30) // Limit duplication
                return false;
            ret = true;
            string dup_summary = std::move(packet_summary());
            int dup_time = g_random_int_range(1, StateMachine.config.fuzzing.max_duplication_time);
            int dup_id = id;
            int pkt_n = counter_tx_downlink;
            dup_timeout_list[dup_id] = loop.onTimeoutMS(dup_time,
                                                        [dup_time, dup_summary, pkt, dup_id, pkt_n, sent_dup_num, offset]() {
                                                            if (sent_dup_num)
                                                                *sent_dup_num = pkt_n;
                                                            // TODO: Add as callback
                                                            GL1Y("[", pkt_n, " - DUP:", dup_time / 1000.0, "] ", dup_summary);
                                                            OAICommunication::OAISHMComm.send_packet((uint8_t *)&pkt[0], pkt.size(), offset);

                                                            dup_timeout_list.erase(dup_id);
                                                        });
            id++;
            LOG3Y("Dup in ", dup_time, " MS");
        }
    }

    return ret;
}

int main(int argc, char **argv)
{

    if (getuid()) {
        LOGY("Not running as root.");
        exit(1);
    }

    GUI_Init(argc, argv);

    pthread_setname_np(pthread_self(), "main_thread"); // Set main thread name
    set_affinity_no_hyperthreading();
    enable_rt_scheduler(1);

    // Configure signals
    std::signal(SIGUSR1, signal_handler);

    LOGG("Ready");

    gui_add_user_fcn(LTEControls);
    GL1G("----------LTE Fuzzer----------");

    // Initialize main classes

    if (!StateMachine.init(CONFIG_FILE_PATH))
        exit(-1);

    Config &conf = StateMachine.config;
    Options &options = conf.options;
    Fuzzing &fuzzing = conf.fuzzing;

    if (StateMachine.config.options.file_logger) {
        gui_log1.enableLogToFile(true, "logs/" + conf.name, "events", options.main_thread_core);
        gui_log7.enableLogToFile(true, "logs/" + conf.name, "open5gs", options.main_thread_core);
        gui_log8.enableLogToFile(true, "logs/" + conf.name, "oai_enb", options.main_thread_core);
        gui_log9.enableLogToFile(true, "logs/" + conf.name, "oai_ue", options.main_thread_core);
    }

    gui_summary();

    // Call graph view on event
    StateMachine.OnTransition([](transition_t *transition) { gui_set_graph_dot(StateMachine.get_graph().c_str()); });

    // Process pseudoheader when loading pcap files (set direction)
    StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
        if (pkt_len >= 49 + 4) {
            // in mac-lte-framed, direction is on second byte of pseusoheader (+ 49 bytes of udp)
            // Set packet direction
            packet_set_direction(!(int)pkt[49 + 1]);
            // Return pseudoheader offset
            offset = 0;
        }
    });

    // StateMachine.LoadModel("captures/lte_rrc_nas.pcap");
    // Try to load program model
    GL1Y("Loading Model...");
    // Load .json model according to default program
    if (StateMachine.LoadModel("lte-softmodem.json", true))
        GL1G("Model Loaded!");
    else
        GL1R("Failed to load Model!");

    if (conf.options.default_programs.size() < 2) {
        LOGR("Open5GS or OpenAirInterface paths not present");
        exit(1);
    }

    // Start Core Network
    if (conf.lte.auto_start_core_network) {
        system("pkill open5gs"); // TODO: improve
        Open5GSProcess.init(conf.options.default_programs[0], "2>&1", gui_open5gs_program, true);
    }
    else {
        GL1R("Open5GS (Core Network) not autostarted!");
    }

    // Start OAI (eNB)
    if (conf.lte.auto_start_base_station) {
        system("pkill lte-softmodem"); // TODO: improve

        string config_file = (conf.lte.enable_simulator ? "lte-fdd-basic-sim" : conf.lte.band);
        string prg_name = (conf.options.launch_program_with_gdb ? "gdb" : conf.options.default_programs[1]);
        string prg_args = "";
        if (conf.options.launch_program_with_gdb)
            prg_args += "-q -ex='set cwd " + GetPathDirName(conf.options.default_programs[1]) + "' -ex=r -ex=bt --args " + conf.options.default_programs[1] + " ";

        prg_args += "-O "s + g_get_current_dir() + "/configs/lte_enb/" + config_file + ".conf";

        if (conf.lte.enable_simulator) {
            // Add eNB simulation environment variable and basicsim args
            g_setenv("ENODEB", "1", NULL);
            if (conf.lte.simulator_delay_us > 0)
                g_setenv("BASICSIM_DELAY_US", std::to_string(conf.lte.simulator_delay_us).c_str(), NULL);
            prg_args += " --basicsim";
        }

        // Redirect stderr (errors) to stdout
        prg_args += " 2>&1";

        OpenAirInterfaceENB.init(prg_name, prg_args, gui_oai_enb_program, true);
    }
    else {
        GL1R("OpenAirInterface (eNB) not autostarted!");
    }

    // Start OAI (UE Simulator)
    if (conf.lte.enable_simulator) {
        system("pkill lte-uesoftmodem"); // TODO: improve
        GL1Y("Starting OAI UE Simulator (basicsim)");
        string prg_name = (conf.options.launch_program_with_gdb ? "gdb" : conf.options.default_programs[2]);
        string prg_args = (conf.options.launch_program_with_gdb ? "-q -ex='set cwd " + GetPathDirName(conf.options.default_programs[2]) + "' -ex=r -ex=bt --args " + conf.options.default_programs[2] : "");
        prg_args += " "s + conf.lte.simulator_ue_arguments;
        prg_args += " 2>&1";

        g_unsetenv("ENODEB");

        OpenAirInterfaceUE.init(prg_name, prg_args, gui_oai_ue_program, true);
    }

    OAICommunication OAICommMAC(SHM_SERVER, 0);
    OAICommunication OAICommPDCP(SHM_SERVER, 1);

    // if (conf.fuzzing.enable_mutation)
    // {
    //     string fuzz_field = conf.lte.default_test_fields[conf.lte.test_field];
    //     if (fuzz_field.find("nas_eps") != string::npos)
    //     {
    //         mutate_downlink = false;
    //         OAICommunication::OAISHMComm.mutate_nas_field(fuzz_field.c_str());
    //     }
    //     else
    //     {
    //         LOG2("Selected field: ", fuzz_field);
    //         // OAICommunication::OAISHMComm.disable_mutate_nas_fields();
    //         packet_register_set_field(fuzz_field.c_str(), FILTER_TEST_FIELD);
    //         mutate_downlink = true;
    //     }
    // }
    // else
    // {
    //     // OAICommunication::OAISHMComm.disable_mutate_nas_fields();
    // }

    std::thread *thread_pdcp = new thread([&]() {
        uint64_t mask, offset;
        field_info *field;
        GPtrArray *fields;

        GL1G("[Main] PDCP Thread started");
        while (1) {
            bool fuzzed = false;
            uint8_t *pdu = OAICommPDCP.receive();
            if (OAICommPDCP.isValid()) {
                // Switch to dissect PDCP

                mutex_pdcp.lock();
                uint8_t plane_type = pdu[1];
                const char *plane_type_str = (plane_type == 2 ? "[ USER ] " : "[SIGNAL] ");
                packet_set_protocol_fast("pdcp-lte-framed");
                // printf("PDU Type: %d\n", OAICommPDCP.pdu_type());
                switch (OAICommPDCP.pdu_type()) {
                case T_ENB_PDCP_PLAIN:
                    // wdissector_enable_fast_full_dissection(1);
                    // if (StateMachine.config.lte.test_field_enable)
                    //     packet_set_field(FILTER_TEST_FIELD);
                    // if (FUZZ_PDCP_FIELD_EN)
                    // {
                    //     packet_register_set_field(FUZZ_PDCP_FIELD_NAME, 0);
                    // }
                    packet_dissect(pdu, OAICommPDCP.pdu_size());

                    // if (StateMachine.config.lte.test_field_enable)
                    //     fuzzed = fuzz_pkt(pdu, OAICommPDCP.pdu_size());

                    // if (StateMachine.config.lte.test_field_enable)
                    // {
                    //     field = packet_read_field(0);
                    //     if (field)
                    //     {
                    //         offset = packet_read_field_offset(field);
                    //         mask = packet_read_field_bitmask(field);
                    //         uint16_t *value = (uint16_t *)&pdu[offset];
                    //         *value = *value | mask;
                    //     }
                    // }

                    // if (FUZZ_PDCP_FIELD_EN)
                    // {
                    //     fields = packet_read_fields(0);
                    //     if (fields)
                    //     {

                    //         for (int i = 0; i < fields->len; i++)
                    //         {
                    //             field = packet_read_field_at(fields, i);
                    //             offset = packet_read_field_offset(field);
                    //             mask = packet_read_field_bitmask(field);
                    //             uint16_t *value = (uint16_t *)&pdu[offset];
                    //             *value = *value | mask;
                    //         }
                    //     }
                    // }
                    // layer_count = 0;
                    // field_count = 0;
                    // packet_navigate(1, 0, test_navigate);
                    // printf("buf addr:%08X\n", (uint64_t)pdu);

                    fuzzed |= packet_fuzzing(pdu, 0, 0);

                    OAICommPDCP.notify();
                    // wdissector_enable_fast_full_dissection(0);
                    if (fuzzed)
                        GL6M(plane_type_str, "PLAIN --> ", packet_summary()); // TX Fuzzed
                    else
                        GL6C(plane_type_str, "PLAIN --> ", packet_summary()); // TX Normal
                    counter_tx_pdcp += 1;
                    break;
                case T_ENB_PDCP_ENC:
                    // wdissector_enable_fast_full_dissection(1);
                    packet_dissect(pdu, OAICommPDCP.pdu_size());
                    OAICommPDCP.notify();
                    // wdissector_enable_fast_full_dissection(0);
                    GL6C(plane_type_str, "ENC   --> ", packet_summary());
                    break;
                }
                // Switch back to MAC-lte
                packet_set_protocol_fast("mac-lte-framed");
                mutex_pdcp.unlock();
            }
        }
    });
    pthread_setname_np(thread_pdcp->native_handle(), "thread_pdcp");

    std::thread *thread_mac = new thread([&]() {
        GL1G("[Main] MAC Thread started");
        bool fuzzed = false;
        int dup_pkt_id = 0;
        while (1) {
            gui_summary();

            uint8_t *pdu = OAICommMAC.receive();
            uint16 pdu_len = OAICommMAC.pdu_size();
            if (OAICommMAC.isValid()) {
                mutex_pdcp.lock();
                switch (OAICommMAC.pdu_type()) {
                case T_ENB_MAC_UE_DL_PDU_WITH_DATA:
                    profiling_timer_start(0);
                    if (StateMachine.config.lte.test_field_enable)
                        packet_set_field(FILTER_TEST_FIELD);
                    StateMachine.PrepareStateMapper();
                    // wdissector_enable_fast_full_dissection(1);
                    packet_dissect(pdu, pdu_len);

                    if (StateMachine.config.lte.test_field_enable)
                        fuzzed = fuzz_pkt(pdu, pdu_len);

                    // memset(pdu, 0, OAICommMAC.pdu_size());
                    // printf("%02x %02x %02x %02x %02x %02x \n", pdu[0], pdu[1], pdu[2], pdu[3], pdu[4], pdu[5]);

                    OAICommMAC.notify(); // Notify end of pdu processing
                    // wdissector_enable_fast_full_dissection(0);

                    packet_duplication(pdu, pdu_len, 11, &dup_pkt_id);

                    StateMachine.RunStateMapper(enable_state_mapper);
                    time_processing_dlsch = profiling_timer_end(0) / 1000;
                    counter_tx_packets += 1;
                    counter_tx_downlink += 1;
                    if (!fuzzed)
                        GL2C("TX --> ", packet_summary());
                    else
                        GL2M("F TX --> ", packet_summary());
                    break;

                case T_ENB_PHY_INITIATE_RA_PROCEDURE:
                    StateMachine.PrepareStateMapper();
                    packet_dissect(pdu, OAICommMAC.pdu_size());
                    OAICommMAC.notify();
                    StateMachine.RunStateMapper(enable_state_mapper);
                    counter_rx_packets += 1;
                    // TODO: RA handled on validation
                    GL2G("RX <-- ", packet_summary());
                    break;

                case T_ENB_MAC_UE_DL_RAR_PDU_WITH_DATA:
                    if (StateMachine.config.lte.test_field_enable)
                        packet_set_field(FILTER_TEST_FIELD);
                    StateMachine.PrepareStateMapper();
                    // wdissector_enable_fast_full_dissection(1);
                    packet_dissect(pdu, OAICommMAC.pdu_size());
                    StateMachine.RunStateMapper(enable_state_mapper);
                    if (StateMachine.config.lte.test_field_enable)
                        fuzzed = fuzz_pkt(pdu, OAICommMAC.pdu_size());
                    // TODO: Fuzz RAR
                    OAICommMAC.notify(); // Notify end of pdu processing
                    // wdissector_enable_fast_full_dissection(0);
                    StateMachine.RunStateMapper(enable_state_mapper);
                    counter_tx_packets += 1;
                    if (!fuzzed)
                        GL2C("TX --> ", packet_summary());
                    else
                        GL2M("F TX --> ", packet_summary());
                    break;

                case T_ENB_MAC_UE_UL_PDU_WITH_DATA:
                    profiling_timer_start(0);
                    StateMachine.PrepareStateMapper();
                    packet_dissect(pdu, OAICommMAC.pdu_size());
                    StateMachine.RunStateMapper(enable_state_mapper);
                    time_processing_uplink = profiling_timer_end(0) / 1000;
                    counter_rx_packets += 1;
                    GL2G("RX <-- ", packet_summary());
                    break;

                case T_ENB_MAC_UE_DL_SIB:
                    // StateMachine.PrepareStateMapper();
                    packet_dissect(pdu, OAICommMAC.pdu_size());
                    OAICommMAC.notify(); // Notify end of pdu processing
                    // StateMachine.RunStateMapper(enable_state_mapper);
                    counter_tx_sib += 1;
                    GL3C("TX --> ", packet_summary());
                    break;

                case T_ENB_PHY_MIB:
                    // StateMachine.PrepareStateMapper();
                    packet_dissect(pdu, OAICommMAC.pdu_size());
                    OAICommMAC.notify(); // Notify end of pdu processing
                    // StateMachine.RunStateMapper(enable_state_mapper);
                    counter_tx_mib += 1;
                    GL4C("TX --> ", packet_summary());
                    break;
                }
                mutex_pdcp.unlock();
            }
            fuzzed = false;
        }
    });
    pthread_setname_np(thread_mac->native_handle(), "thread_mac");

    thread_mac->join(); // Wait here

    return 0;
}
