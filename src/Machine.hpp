#pragma once

#ifndef __MACHINE__
#define __MACHINE__
#include <array>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sched.h>
#include <semaphore.h>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>

#include "MiscUtils.hpp"
#include "libs/gvpp/gvpp.hpp"
#include "libs/react-cpp/reactcpp.h"
#include <Packet.h>
#include <PcapFileDevice.h>
#include <RawPacket.h>

#ifdef FUZZ_LTE
#include "configs/eNBConfig.hpp"
#include "gui/GUI_LTE.hpp"
#elif FUZZ_BT
#include "configs/BTConfig.hpp"
#include "gui/GUI_Bluetooth.hpp"
#elif FUZZ_BTHOST
#include "configs/BTHostConfig.hpp"
#include "gui/GUI_BluetoothHost.hpp"
#elif FUZZ_WIFI_AP
#include "configs/WifiAPConfig.hpp"
#include "gui/GUI_WifiAP.hpp"
#elif FUZZ_ZNP
#include "configs/BTHostConfig.hpp"
#include "gui/GUI_WifiAP.hpp"
#elif FUZZ_COAP
#include "configs/CoapConfig.hpp"
#include "gui/GUI_WifiAP.hpp"

#else
#include "configs/GenericConfig.hpp"
#define GL1 misc_utils::lgs
#define GL1Y(...) (cout << termcolor::yellow, misc_utils::lgs(__VA_ARGS__), cout << termcolor::reset)
#define GL1G(...) (cout << termcolor::green, misc_utils::lgs(__VA_ARGS__), cout << termcolor::reset)
#define GL1R(...) (cout << termcolor::red, misc_utils::lgs(__VA_ARGS__), cout << termcolor::reset)
#define GL1M(...) (cout << termcolor::magenta, misc_utils::lgs(__VA_ARGS__), cout << termcolor::reset)
#define GL1C(...) (cout << termcolor::cyan, misc_utils::lgs(__VA_ARGS__), cout << termcolor::reset)
#endif

extern "C" {
#include "libs/profiling.h"
#include "wdissector.h"
}

using namespace quicktype;
using namespace gvpp;
using namespace std;
using namespace React;

/**
 * @brief State mapper rules structure
 *
 */
typedef struct
{
    string layer_name;
    string filter;
    const char *filter_compiled = nullptr;
    vector<string> state_field_name;
    vector<header_field_info *> state_field_header;
    uint8_t append_summary;
    uint32_t total_mapped_states;
    uint32_t total_mapped_transitions;
    uint32_t idx;
} state_mapper_t;

typedef struct
{
    Exclude *entry;
    const char *filter_compiled = nullptr;
    uint32_t exclude_flag;
} exclude_t;

/**
 * @brief State structure
 *
 */
typedef struct
{
    string name;
    Node<> *node;
    uint32_t node_number;
    timeval time;
    uint32_t type_value;
    uint32_t timeout;
    uint8_t direction;
    uint8_t layers_count;
    string on_timeout;
    state_mapper_t *smt;
    uint32_t mapping_idx;
    uint32_t global_layer_offset;
    void (*on_enter_state)() = nullptr;
    void (*on_exit_state)() = nullptr;
    void (*on_timeout_fcn)() = nullptr;

} state_t;

/**
 * @brief Transition structure
 *
 */
typedef struct
{
    string trigger;
    state_t *src_state;
    state_t *dst_state;
    string src_state_name;
    string dst_state_name;
    timeval time;
    Edge<> *edge;

} transition_t;

typedef struct
{
    unordered_map<state_t *, transition_t> to_state;
    unordered_map<state_t *, transition_t *> from_state;
} transition_table_t;

// Register JSON struct vars
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(timeval, tv_sec, tv_usec);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(state_t, node_number, time, type_value, timeout, on_timeout, direction, layers_count);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(transition_t, trigger, src_state_name, dst_state_name, time);

MainLoop loop;
/**
 * @brief State Machine Class
 *
 */
class Machine : public MachineConfig {
private:
    const char *TAG = "[Machine] ";
    // State Mapping vars
    uint16_t filter_index_mapping[256];
    string loaded_config_ref;
    shared_ptr<thread> event_loop;
    uint32_t layer_cumulative_count = 0;
    uint32_t layer_count_padding = 0;

    std::string current_state;
    vector<state_mapper_t> StateMap;
    unordered_map<string, state_t> StatesNodes;
    unordered_map<state_t *, transition_table_t> StatesTransitions;
    vector<pair<timeval, transition_t *>> TransitionsRecord;
    uint32_t unique_transitions = 0;
    state_t *PreviousStateNode = nullptr;
    state_t *CurrentStateNode = nullptr;
    transition_t *CurrentTransition = nullptr;
    transition_t *PreviousTransition = nullptr;
    string node_shape = "rectangle";
    vector<exclude_t> ExcludesMapping;

    // Vars
    void (*on_transition)(transition_t *transition) = nullptr;
    void (*on_process_pkt_pseudoheader)(uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) = nullptr;
    void (*on_loop_detected)(uint8_t) = nullptr;
    bool evt_on_transition = false;
    sem_t mutex_on_event;
    mutex mutex_graph;
    bool color_nodes = true;

    shared_ptr<thread> events_thread;

    // Loop detection vars
    bool loop_detection_enable = false;
    header_field_info *loop_detection_field_hfi = NULL;
    uint32_t loop_detection_malformed_count = 0;
    uint32_t loop_detection_same_state_count = 0;
    uint16_t loop_detection_max_count = 0;

    /**
     * @brief Thread that handles state machine event callbacks
     *
     */
    void event_thread()
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
        while (true) {
            sem_wait(&mutex_on_event);
            if (evt_on_transition) {
                evt_on_transition = false;
                if (on_transition != nullptr)
                    on_transition(this->CurrentTransition);
            }
        }
    }

public:
    Machine() = default;

    /**
     * @brief Exclusion structure
     *
     */
    enum EXCLUDES {
        EXCLUDE_ALL = 1,         // (A) Excludes packet from all fuzzing actions
        EXCLUDE_MUTATION = 2,    // (M) Excludes packet from being mutated
        EXCLUDE_DUPLICATION = 4, // (D) Excludes packet from being duplicated
        EXCLUDE_VALIDATION = 8,  // (V) Exclude packet from being validated
        EXCLUDE_MAPPING = 16,    // (S) Exclude packet from being mapped
        EXCLUDE_RETRY = 32       // (R) Exclude packet from being retried
    };

    /**
     * @brief Loop detection reason structure
     *
     */
    enum LOOP_DETECTION_REASON {
        LD_MALFORMED_RECEPTION = 0,
        LD_SAME_STATE_LOOP,
        LD_DEADLOCK_CYCLE
    };

    // Stats
    uint32_t stats_transitions = 0;
    uint32_t stats_known_transitions = 0;
    string DissectedStateName;

    uint32_t CurrentExclude;
    Graph<> MachineGraph;

    /**
     * @brief Initialize protocol State machine according to the configuration file
     *
     * @param config_file Path of the .json configuration file
     * @param _load_default_config load default configuration
     * @return boolean
     */
    inline bool init(const char *config_file, bool _load_default_config = false)
    {
        sem_init(&mutex_on_event, 0, 0);

        // Initialize main classes
        if (!_load_default_config) {
            try {

                std::ifstream in_file(config_file); // Open file
                json j;
                in_file >> j;                  // Convert file to json object
                nlohmann::from_json(j, *this); // convert json object to this instance
                loaded_config_ref = j.dump();

                GL1Y(TAG, "Config Loaded: ", config_file);
                // Sanity checks
                if (config.options.program >= config.options.default_programs.size())
                    config.options.program = 0;

                UpdateConfig();
            }
            catch (const std::exception &e) {
                LOGR(e.what());
                return false;
            }
        }
        else {
            if (ResetConfig())
                GL1G(TAG, "Default Config Loaded!");
        }

        // DTL 201
        // wdissector_init("encap:BLUETOOTH_HCI_H4_WITH_PHDR");
        // wdissector_init("proto:hci_h4");
        wdissector_enable_fast_full_dissection(1);
        // wdissector_init(config.options.default_protocol.c_str());
        // need to update the protocol here is the wdmapper is not working
        wdissector_init("proto:eth");
        // printf("check whether the init is correct or not", config.options.default_protocol.c_str());
        GL1(wdissector_version_info());
        GL1("Profile \"", wdissector_profile_info(), "\" loaded");

        // Initialize State Machine
        ResetModel();
        InitStateMapper();
        InitExcludes();
        // printf("Does it even reach here? \n");

        // Event Loop
        event_loop = make_shared<thread>([&]() {
            printf("Enter loop");
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGTERM);
            sigaddset(&set, SIGABRT);
            pthread_sigmask(SIG_BLOCK, &set, NULL);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
            pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
            GL1Y(TAG, "Event loop started");
            loop.run(true);
        });
        pthread_setname_np(event_loop->native_handle(), "event_loop");
        event_loop->detach();

        // Events Thread
        events_thread = make_shared<thread>(&Machine::event_thread, this);
        pthread_setname_np(events_thread->native_handle(), "events_thread");
        events_thread->detach();

        return true;
    }

    /**
     * @brief Initialize state mapper structure (used internally by constructor)
     *
     * @return true
     * @return false
     */
    bool InitStateMapper()
    {
        // Fill the State Mapper structure
        int idx = 0;
        // printf("Entered Init State Mapper funtion");
        for (auto &SM : this->config.state_mapper.mapping) {
            state_mapper_t smt;
            smt.idx = idx;
            smt.total_mapped_states = 0;
            smt.total_mapped_transitions = 0;
            smt.append_summary = SM.append_summary;
            smt.layer_name = SM.layer_name;
            smt.filter = SM.filter;
            smt.filter_compiled = packet_register_filter(smt.filter.c_str());
            if (smt.filter_compiled == NULL) {
                LOGR("Error compiling filter \"" + smt.filter + "\"");
                return false;
            }

            if (holds_alternative<string>(SM.state_name_field)) {
                // Single string field
                string &field_name = get<string>(SM.state_name_field);
                header_field_info *hf = packet_get_header_info(field_name.c_str());
                if (hf == NULL) {
                    LOGR("Error validating field \"" + field_name + "\"");
                    return false;
                }

                smt.state_field_name.emplace_back(field_name);
                smt.state_field_header.emplace_back(hf);
            }
            else {
                // Multiple string field
                auto &fields_name = get<vector<string>>(SM.state_name_field);
                for (auto &field_name : fields_name) {
                    header_field_info *hf = packet_get_header_info(field_name.c_str());
                    if (hf == NULL) {
                        LOGR("Error validating field \"" + field_name + "\"");
                        exit(0);
                    }
                    smt.state_field_name.emplace_back(field_name);
                    smt.state_field_header.emplace_back(hf);
                }
            }

            StateMap.push_back(smt);
            idx++;
        }
        return true;
    }

    /**
     * @brief Initialize exclusion structure (used internally)
     *
     * @return true
     * @return false
     */
    bool InitExcludes()
    {

        bool no_errors = true;

        for (auto &exclude : this->config.fuzzing.excludes) {
            uint32_t exclude_flag = 0;

            for (size_t i = 0; i < exclude.apply_to.size(); i++) {
                switch (exclude.apply_to[i]) {
                case 'A':
                    exclude_flag |= EXCLUDE_ALL;
                    break;
                case 'M':
                    exclude_flag |= EXCLUDE_MUTATION;
                    break;
                case 'D':
                    exclude_flag |= EXCLUDE_DUPLICATION;
                    break;
                case 'V':
                    exclude_flag |= EXCLUDE_VALIDATION;
                    break;
                case 'S':
                    exclude_flag |= EXCLUDE_MAPPING;
                    break;
                case 'R':
                    exclude_flag |= EXCLUDE_RETRY;
                    break;
                default:
                    exclude_flag |= EXCLUDE_ALL;
                }
            }

            const char *filter_compiled = packet_register_filter(exclude.filter.c_str());
            if (filter_compiled) {
                ExcludesMapping.push_back({&exclude, filter_compiled, exclude_flag});
            }
            else if (exclude.filter.size()) {
                no_errors = false;
                LOG3R("Exclude filter \"", exclude.filter, "\" failed to compile (check syntax)");
            }
        }
        // printf("Is here any errors? ", no_errors);
        return no_errors;
    }

    /**
     * @brief Install state exclusion filters
     *
     */
    void PrepareExcludes()
    {
        this->CurrentExclude = 0;
        for (auto &exclude : ExcludesMapping) {
            packet_set_filter(exclude.filter_compiled);
        }
    }

    /**
     * @brief Check if current packet is to be exluded according to the exclusion rules
     *
     * @return uint32_t
     */
    uint32_t RunExcludes()
    {
        for (auto &exclude : ExcludesMapping) {
            // LOG1(exclude.entry->filter);
            if (packet_read_filter(exclude.filter_compiled)) {
                // LOG1("OK");
                this->CurrentExclude |= exclude.exclude_flag;
            };
        }
        return this->CurrentExclude;
    }

    /**
     * @brief Add state object (used internally)
     *
     * @param state_name
     * @param value
     * @param direction
     * @param layers_count
     * @param smt
     * @param timeout
     * @param enter_cb
     * @param exit_cb
     * @param timeout_cb
     * @return state_t *
     */
    state_t *AddState(const char *state_name,
                      uint32_t value,
                      uint8_t direction = 0,
                      uint8_t layers_count = 0,
                      state_mapper_t *smt = nullptr,
                      uint32_t timeout = 0,
                      void (*enter_cb)() = nullptr,
                      void (*exit_cb)() = nullptr,
                      void (*timeout_cb)() = nullptr)
    {
        state_t new_state;
        mutex_graph.lock();
        uint32_t node_number = StatesNodes.size();
        Node<> &n = MachineGraph.addNode("n"s + to_string(node_number), state_name);
        n.set("shape", node_shape);
        mutex_graph.unlock();
        // Add node information to State structure
        gettimeofday(&new_state.time, NULL);
        new_state.name = state_name;
        new_state.node = &n;
        new_state.node_number = node_number;
        new_state.type_value = value;
        new_state.timeout = timeout;
        new_state.direction = direction;
        new_state.layers_count = layers_count;
        new_state.global_layer_offset = layer_cumulative_count;
        new_state.smt = smt;
        // Update mapping structure info
        if (unlikely(smt != nullptr)) {
            // Save mapping reference and index
            new_state.mapping_idx = smt->idx;
            // Add mapped states count per mapped layers
            smt->total_mapped_states++;
        }
        // Add callbacks
        new_state.on_enter_state = enter_cb;
        new_state.on_exit_state = exit_cb;
        new_state.on_timeout_fcn = timeout_cb;
        // Add state
        StatesNodes[state_name] = new_state;
        // printf("This is the new state", state_name);

        // Update global layer count
        layer_cumulative_count += (layers_count - config.state_mapper.packet_layer_offset) + layer_count_padding;

        return &StatesNodes[state_name];
    }

    /**
     * @brief  Add transition by states name (used internally)
     *
     * @param src_state_name
     * @param dst_state_name
     * @param trigger_name
     * @return boolean
     */
    bool AddTransition(const char *src_state_name, const char *dst_state_name, const char *trigger_name = NULL)
    {
        auto state_not_found = StatesNodes.end();
        auto src = StatesNodes.find(src_state_name);
        // printf("This is src", src);
        auto dst = StatesNodes.find(dst_state_name);
        // printf("This is dst\n", dst);
        if ((src == state_not_found) || (dst == state_not_found))
            return false;

        // StatesNodes
        return AddTransition(&src->second, &dst->second);
    }

    /**
     * @brief Add transition by states reference (Used internally)
     *
     * @param src_state
     * @param dst_state
     * @param trigger_name
     * @return boolean
     */
    bool AddTransition(state_t *src_state, state_t *dst_state, const char *trigger_name = NULL)
    {
        timeval time;
        gettimeofday(&time, NULL);

        auto table_not_found = StatesTransitions.end();
        auto src_table = StatesTransitions.find(src_state);
        auto dst_table = StatesTransitions.find(dst_state);

        transition_table_t *src_table_ptr;
        transition_table_t *dst_table_ptr;

        // Create tables entry if not existent
        if (src_table == table_not_found) {
            StatesTransitions[src_state] = transition_table_t();
            src_table_ptr = &StatesTransitions[src_state];
        }
        else {
            src_table_ptr = &src_table->second;
        }

        if (dst_table == table_not_found) {
            StatesTransitions[dst_state] = transition_table_t();
            dst_table_ptr = &StatesTransitions[dst_state];
        }
        else {
            dst_table_ptr = &dst_table->second;
        }

        // Increment transitions count by layer
        if (src_state->smt != nullptr)
            src_state->smt->total_mapped_transitions++;

        // Create to_state table of src state
        src_table_ptr->to_state[dst_state] = transition_t();
        transition_t &new_transition = src_table_ptr->to_state[dst_state];
        new_transition.src_state = src_state;
        new_transition.src_state_name = src_state->name;
        new_transition.dst_state = dst_state;
        new_transition.dst_state_name = dst_state->name;
        new_transition.time = time;
        if (trigger_name)
            new_transition.trigger = trigger_name;
        mutex_graph.lock();
        Edge<> &edge = MachineGraph.addEdge(*src_state->node, *dst_state->node, new_transition.trigger);
        mutex_graph.unlock();
        new_transition.edge = &edge;

        // Update from_state table of dst state
        dst_table_ptr->from_state[src_state] = &new_transition;
        unique_transitions += 1;

        return true;
    }

    /**
     * @brief Perform transition, given the names of source and destination states
     *
     * @param dst_state_name
     * @param auto_add_transition
     * @param no_events
     * @param no_color_change
     * @return boolean
     */
    bool Transition(const char *dst_state_name, bool auto_add_transition = false,
                    bool no_events = false, bool no_color_change = false)
    {
        timeval time;
        gettimeofday(&time, NULL);

        auto state_not_found = StatesNodes.end();
        auto dst_state = StatesNodes.find(dst_state_name);

        // Check if src and dst states exist
        if ((dst_state == state_not_found) || (CurrentStateNode == nullptr))
            return false;

        auto table_not_found = StatesTransitions.end();
        auto src_table = StatesTransitions.find(CurrentStateNode);
        transition_table_t *src_table_ptr;
        // Check if src and dst tables exist
        if (src_table == table_not_found) {
            // Table not found
            if (auto_add_transition) {
                // Create table
                StatesTransitions[CurrentStateNode] = transition_table_t();
                src_table_ptr = &StatesTransitions[CurrentStateNode];
            }
            else {
                return false;
            }
        }
        else {
            // Table found
            src_table_ptr = &src_table->second;
        }

        // Check if transition exists
        auto transition_not_found = src_table_ptr->to_state.end();
        auto transition = src_table_ptr->to_state.find(&dst_state->second);
        if (transition == transition_not_found) {
            if (auto_add_transition)
                AddTransition(CurrentStateNode, &dst_state->second);
            else
                return false;
        }

        if (color_nodes) {
            mutex_graph.lock();
            if (PreviousStateNode != nullptr)
                PreviousStateNode->node->unset("color");
            if (PreviousTransition != nullptr)
                PreviousTransition->edge->unset("color");
            mutex_graph.unlock();
        }

        // Update Current State
        PreviousStateNode = CurrentStateNode;
        CurrentStateNode = &dst_state->second;
        PreviousTransition = CurrentTransition;
        CurrentTransition = &src_table_ptr->to_state[&dst_state->second];

        if (color_nodes && !no_color_change) {
            mutex_graph.lock();
            if (PreviousStateNode != nullptr)
                PreviousStateNode->node->set("color", "red");
            CurrentStateNode->node->set("color", "blue");

            if (PreviousTransition != nullptr)
                PreviousTransition->edge->set("color", "red");
            CurrentTransition->edge->set("color", "blue");
            mutex_graph.unlock();
        }

        // Add transition to transitions record
        // TransitionsRecord.push_back(make_pair(time, CurrentTransition));

        // Notify event
        if (!no_events) {
            NotifyOnTransition();
        }

        stats_transitions++;

        return true;
    }

    /**
     * @brief Must be called before RunStateMapper and packet_dissect to install filters and
     * fields for the state mapping
     *
     */
    __attribute__((always_inline)) void PrepareStateMapper()
    {

        for (auto &smt : this->StateMap) {
            packet_set_filter(smt.filter_compiled);
            // Iterate over all fields headers
            for (auto *state_field_header : smt.state_field_header) {
                // packet_set_field_hfinfo(state_field_header);
                // TODO: measure impact of packet_set_field_hfinfo_all and use as default on wdissector
                packet_set_field_hfinfo_all(state_field_header);
            }
        }

        // Loop detection
        if ((loop_detection_enable) && (packet_direction() == P2P_DIR_RECV)) {
            // Look for general expert info messages.
            // The first strategy is to simply look for malformed packets
            packet_set_field_hfinfo(loop_detection_field_hfi);
        }
    }

    /**
     * @brief Run Loop Detection (used internally)
     *
     */
    __attribute__((always_inline)) void RunLoopDetection()
    {
        // Perform simple loop detection for RX
        if ((loop_detection_enable) && (packet_direction() == P2P_DIR_RECV)) {
            field_info *fi = packet_read_field_hfinfo(loop_detection_field_hfi);
            if ((fi) && (fi->value.value.uinteger == PI_MALFORMED)) {
                if (++loop_detection_malformed_count >= loop_detection_max_count) {
                    loop_detection_malformed_count = 0;
                    if (on_loop_detected != nullptr)
                        on_loop_detected(LD_MALFORMED_RECEPTION);
                }
            }

            // Same state loop detection
            if (CurrentStateNode == PreviousStateNode) {
                if (++loop_detection_same_state_count >= loop_detection_max_count) {
                    loop_detection_same_state_count = 0;
                    if (on_loop_detected != nullptr)
                        on_loop_detected(LD_SAME_STATE_LOOP);
                }
            }
        }

        // TODO: add second order state detection
    }

    /**
     * @brief Run State Mapper. The function returns if the state transition is
     * valid (true) or not (false). Before calling this function, PrepareStateMapper
     * must be called to initialize filters and mapping fields
     *
     * @param new_mapping Add state to model
     * @param no_events Do not trigger any event while performing state transition
     * @param no_color_change Does not change color of the state machine nodes visualization
     * @return boolean
     */
    __attribute__((always_inline)) bool RunStateMapper(bool new_mapping = false, bool no_events = false,
                                                       bool no_color_change = false)
    {
        for (auto &smt : this->StateMap) {
            if (smt.filter_compiled && packet_read_filter(smt.filter_compiled)) {

                RunLoopDetection();
                // LOG1(smt.filter);
                for (header_field_info *state_field_header : smt.state_field_header) {
                    int i = 0;
                    LOG2("-->", smt.state_field_name[i]);
                    if (field_info *fi = packet_read_field_hfinfo(state_field_header)) {
                        // LOG1("OK");
                        uint32_t field_value = fi->value.value.uinteger;
                        const char *ps = packet_read_value_to_string(field_value, fi->hfinfo);

                        if (likely(ps != NULL)) {

                            string state_name;
                            if (!smt.append_summary)
                                state_name = p2p_dir_to_string(packet_direction()) + " / " +
                                             smt.layer_name + " / " + ps;
                            else
                                state_name = p2p_dir_to_string(packet_direction()) + " / " +
                                             smt.layer_name + " / " + ps + " / " + packet_summary();

                            this->DissectedStateName = state_name;

                            if (StatesNodes.find(state_name) == StatesNodes.end()) {
                                // New state
                                if (new_mapping) {
                                    // Add new state
                                    AddState(state_name.c_str(), field_value, packet_direction(), packet_layers_count(), &smt);
                                }
                                else {
                                    return false;
                                }
                            }

                            return Transition(state_name.c_str(), new_mapping, no_events, no_color_change);
                        }
                        i++;
                    }
                }
            }
        }

        return false;
    }

    /**
     * @brief User callback that is called for every state transition
     *
     * @param transition_cb
     */
    void OnTransition(void (*transition_cb)(transition_t *transition))
    {
        on_transition = transition_cb;
    }

    /**
     * @brief User callback that is called to process packet pseudoheader before loading a model.
     * This can be used to set the direction of the dissected packet
     *
     * @param evt_callback
     */
    void OnProcessPseudoHeader(void (*evt_callback)(uint8_t *pkt, uint32_t pkt_len, uint32_t &offset))
    {
        on_process_pkt_pseudoheader = evt_callback;
    }

    /**
     * @brief User callback that is called whenever a loop condition has been detected
     *
     * @param evt_callback
     */
    void OnLoopDetected(void (*evt_callback)(uint8_t))
    {
        on_loop_detected = evt_callback;
    }

    /**
     * @brief Get the Config object in string
     *
     * @return string
     */
    string GetConfig()
    {
        json j;
        nlohmann::to_json(j, *this); // convert json object to this instance
        return j.dump(4);
    }

    // Update internal Machine core settings if config changes
    void UpdateConfig()
    {
        // Enable/Disable Core dump
        enable_coredump(config.options.core_dump);
    }

    /**
     * @brief Set the Config object
     *
     * @param conf
     * @return boolean
     */
    bool SetConfig(string conf)
    {
        try {
            json patch = json::parse(conf, nullptr, false, true);
            json new_config;

            nlohmann::to_json(new_config, *this);
            // Merge json path to existing configuration
            new_config.merge_patch(patch);
            nlohmann::from_json(new_config, *this);
            UpdateConfig();
            return true;
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
        return false;
    }

    inline bool save(const char *config_file)
    {
        try {
            json j;
            nlohmann::to_json(j, *this); // convert json object to this instance
            string new_config = j.dump();
            if (new_config.compare(loaded_config_ref) != 0) {
                std::ofstream out_file(config_file); // Open file
                out_file << j.dump(4);               // Convert file to json object
                LOG3G(TAG, "Config Saved: ", config_file);
            }

            return true;
        }
        catch (const std::exception &e) {
            LOGR(e.what());
            return false;
        }
    }

    /**
     * @brief Clear State Machine event callbacks
     *
     */
    void ClearCallbacks()
    {
        this->on_transition = nullptr;
        this->on_process_pkt_pseudoheader = nullptr;
    }

    inline const string get_graph()
    {
        stringstream ss;
        mutex_graph.lock();
        if (config.state_mapper.show_all_states) {
            ss << MachineGraph;
            mutex_graph.unlock();
            return ss.str();
        }
        else if (CurrentStateNode != nullptr) {
            int nodes = 0;
            Graph<> LocalGraph;
            Node<> &MainNode = LocalGraph.addNode("n0", CurrentStateNode->name);
            MainNode.set("color", "blue");
            nodes++;

            auto table = StatesTransitions.find(CurrentStateNode);
            if (table != StatesTransitions.end()) {
                auto to_states = table->second.to_state;
                auto from_states = table->second.from_state;

                for (auto &to_state : to_states) {
                    Node<> &to_node = LocalGraph.addNode("n"s + to_string(nodes), to_state.first->name);
                    LocalGraph.addEdge(MainNode, to_node);
                    nodes++;
                }

                for (auto &from_state : from_states) {
                    Node<> &from_node = LocalGraph.addNode("n"s + to_string(nodes), from_state.first->name);
                    Edge<> &from_edge = LocalGraph.addEdge(from_node, MainNode);
                    if (PreviousStateNode == from_state.first) {
                        from_node.set("color", "red");
                        from_edge.set("color", "red");
                    }
                    nodes++;
                }
            }

            ss << LocalGraph;
            mutex_graph.unlock();
            return ss.str();
        }

        return "";
    }

    /**
     * @brief Set the Layer Count Padding number.
     * This is used to return the global state offset adjusted to extra (padded) positions
     *
     * @param value number of padding
     */
    void SetStateGlobalOffsetPadding(uint32_t value)
    {
        layer_count_padding = value;
    }

    /**
     * @brief Get the States object
     *
     * @return unordered_map<string, state_t>&
     */

    unordered_map<string, state_t> &GetStates()
    {
        return StatesNodes;
    }

    /**
     * @brief Get the State Map object
     *
     * @return vector<state_mapper_t>&
     */

    vector<state_mapper_t> &GetStateMap()
    {
        return StateMap;
    }

    /**
     * @brief Return total number of transitions in the loaded model
     *
     * @return uint32_t
     */
    uint32_t TotalTransitions()
    {
        return unique_transitions;
    }

    /**
     * @brief Return total number of states in the loaded model
     *
     * @return uint32_t
     */
    uint32_t TotalStates()
    {
        return StatesNodes.size();
    }

    /**
     * @brief Return the total number of layers across all states in the model
     *
     * @param sum_offset offsets the sum to each state layer count in the model
     * @return uint32_t
     */
    uint32_t TotalStatesLayers(uint32_t sum_offset = 0)
    {
        return layer_cumulative_count + (StatesNodes.size() * sum_offset);
    }

    /**
     * @brief Load State Machine Model.
     * The model can be a .json file or a capture file (.pcapng)
     *
     * @param file_path Path of the model file to be loaded
     * @param trim_to_name Loads the model file relative to SaveFolder parameter
     * @param merge Merges model into previous loaded model
     * @return true/false
     */
    bool LoadModel(const char *file_path = NULL, bool trim_to_name = false, bool merge = false,
                   bool ignore_tx = false)
    {
        string load_file_path;
        json json_root;

        if (file_path == NULL) {
            load_file_path = this->config.state_mapper.save_folder + "/" + this->config.name + ".json";
        }
        else {
            string fn;
            if (trim_to_name) {
                fn = string_split(file_path, "/").back();
                fn = config.state_mapper.save_folder + "/" + string_split(fn, ".").front() + ".json";
            }
            else {
                fn = string(file_path);
            }
            load_file_path = fn;
        }

        gboolean h = packet_set_protocol(config.options.default_pcap_protocol.c_str());
        // puts(config.options.default_pcap_protocol.c_str());
        // LOG1(h);

        string file_extension = string_file_extension(load_file_path);

        if (!file_extension.compare("json")) {
            // Parse json
            try {
                // LOG1(load_file_path);
                std::ifstream in_file(load_file_path); // Open file
                in_file >> json_root;                  // Convert file to json object
            }
            catch (const std::exception &e) {
                LOGR(e.what());
                return false;
            }

            if (merge == false) {
                ResetModel();
            }

            json &json_states = json_root["StatesNodes"];
            for (auto &state : json_states.items()) {
                if (StatesNodes.find(state.key()) != StatesNodes.end())
                    continue; // Ignore existent states;

                // Recover state (Add new state from json file)
                auto new_state = state.value().get<state_t>();
                new_state.name = state.key(); // Load most properties here
                new_state.global_layer_offset = layer_cumulative_count;
                // Recover state mapping index (TODO: Search by string for greater precision)
                if (new_state.mapping_idx < StateMap.size())
                    new_state.smt = &StateMap[new_state.mapping_idx];
                else
                    new_state.smt = nullptr;
                // Update misc
                layer_cumulative_count += (new_state.layers_count - config.state_mapper.packet_layer_offset) + layer_count_padding;

                // Create state node
                Node<> &n = MachineGraph.addNode("n"s + to_string(StatesNodes.size()), state.key());
                n.set("shape", node_shape);
                new_state.node = &n;

                // Update hash maps references
                StatesNodes[state.key()] = new_state;
            }

            json &json_tables = json_root["StatesTransitions"];
            for (auto &table : json_tables.items()) {
                state_t *src_state_ref = &StatesNodes[table.key()];
                // Create table for src state
                if (StatesTransitions.find(src_state_ref) == StatesTransitions.end())
                    StatesTransitions[src_state_ref] = transition_table_t();
                // Fill to_state transitions
                for (auto &transition : table.value()["to_state"].items()) {
                    transition_t new_transition = transition.value().get<transition_t>();
                    new_transition.dst_state = &StatesNodes[new_transition.dst_state_name];

                    // Check if transition already exists and ignore if so
                    auto &to_state = StatesTransitions[src_state_ref].to_state;
                    if (to_state.find(new_transition.dst_state) != to_state.end())
                        continue;

                    new_transition.src_state = &StatesNodes[table.key()];
                    Edge<> &edge = MachineGraph.addEdge(*new_transition.src_state->node, *new_transition.dst_state->node);
                    new_transition.edge = &edge;

                    StatesTransitions[src_state_ref].to_state[new_transition.dst_state] = new_transition;
                    unique_transitions++;
                }
            }
            for (auto &table : json_tables.items()) {
                state_t *src_state_ref = &StatesNodes[table.key()];
                // Fill from_state transitions pointer
                for (auto &transition : table.value()["from_state"]) {
                    state_t *from_state_ref = &StatesNodes[transition];
                    StatesTransitions[src_state_ref].from_state[from_state_ref] = &StatesTransitions[from_state_ref].to_state[src_state_ref];
                }
            }

            NotifyOnTransition();

            packet_set_protocol(config.options.default_protocol.c_str());
            return true;
        }
        else {
            // Parse PCAP / PCAPNG
            pcpp::IFileReaderDevice *pcapReader = pcpp::IFileReaderDevice::getReader(load_file_path.c_str());

            if (pcapReader != NULL) {
                if (!pcapReader->open())
                    return false;

                if (merge == false) {
                    ResetModel();
                }

                // raw packet object
                pcpp::RawPacket rawPacket;
                uint32_t offset;
                uint p = 1;

                while (pcapReader->getNextPacket(rawPacket)) {
                    offset = 0;
                    // ------------ State Mapping / Transitions ------------
                    if (on_process_pkt_pseudoheader != nullptr)
                        on_process_pkt_pseudoheader((uint8_t *)rawPacket.getRawData(), rawPacket.getRawDataLen(), offset);
                    if (offset >= rawPacket.getRawDataLen())
                        continue;
                    PrepareStateMapper();
                    // print_buffer((uint8_t *)rawPacket.getRawData(), rawPacket.getRawDataLen() - offset);
                    packet_dissect(((uint8_t *)rawPacket.getRawData()) + offset, rawPacket.getRawDataLen() - offset);
                    LOG3(p, ": ", packet_summary());
                    if ((ignore_tx) && (packet_direction() == P2P_DIR_SENT))
                        continue;
                    RunStateMapper(true, true, true);
                    LOG3(p++, ": ", GetCurrentStateName());
                }

                GoToInitialState();
                NotifyOnTransition();
                packet_set_protocol(config.options.default_protocol.c_str());
                return true;
            }
        }

        return false;
    }

    /**
     * @brief File/Folder path to save loaded model into .json file
     *
     * @param file_path
     * @return true
     * @return false
     */
    bool SaveModel(const char *file_path = NULL)
    {
        string save_file_path;
        if (file_path == NULL) {
            save_file_path = this->config.state_mapper.save_folder + "/" + this->config.name + ".json";
        }
        else {
            save_file_path = string(file_path);
        }

        json json_root;
        json &json_states = json_root["StatesNodes"];
        for (auto &state : StatesNodes) {
            json_states[state.first] = state.second;
        }
        json &json_tables = json_root["StatesTransitions"];
        for (auto &table : StatesTransitions) {
            // fill to_state map
            json &json_transition_to = json_tables[table.first->name]["to_state"];
            for (auto &transition : table.second.to_state) {
                json_transition_to[transition.first->name] = transition.second;
            }
            // fill from_state (only string keys)
            json &json_transition_from = json_tables[table.first->name]["from_state"];
            int idx = 0;
            for (auto &transition : table.second.from_state) {
                json_transition_from[idx] = transition.first->name;
                idx++;
            }
        }

        try {
            std::ofstream json_file(save_file_path); // Open file
            json_file << json_root.dump(4);          // Convert file to json object

            string dot_file_path = string_split(save_file_path, ".").front() + ".dot";
            std::ofstream dot_file(dot_file_path); // Open file
            dot_file << get_graph();               // Save dot file
        }
        catch (const std::exception &e) {
            LOGR(e.what());
            packet_set_protocol(config.options.default_protocol.c_str());
            return false;
        }

        packet_set_protocol(config.options.default_protocol.c_str());
        return true;
    }

    /**
     * @brief Clear the current loaded model and resets the state machine to initial state
     *
     */
    void ResetModel()
    {

        if (CurrentStateNode != nullptr) {
            CurrentStateNode->node->unset("color");
        }
        CurrentStateNode = nullptr;
        if (PreviousStateNode != nullptr) {
            PreviousStateNode->node->unset("color");
        }
        PreviousStateNode = nullptr;
        if (CurrentTransition != nullptr) {
            CurrentTransition->edge->unset("color");
        }
        CurrentTransition = nullptr;
        if (PreviousTransition != nullptr) {
            PreviousTransition->edge->unset("color");
        }
        PreviousTransition = nullptr;

        unique_transitions = 0;
        StatesNodes.clear();
        StatesTransitions.clear();
        MachineGraph.clear();

        CurrentStateNode = AddState(this->config.validation.initial_state.c_str(), 0); // Add initial state
        CurrentStateNode->node->set("color", "blue");

        ResetLoopDetector();
    }

    /**
     * @brief Get the Current State Name as string
     *
     * @return string
     */
    string GetCurrentStateName()
    {
        if (CurrentStateNode != nullptr)
            return CurrentStateNode->name;
        else
            return "None";
    }

    /**
     * @brief Get the Current State Node object
     *
     * @return state_t*
     */
    state_t *GetCurrentStateNode()
    {
        return CurrentStateNode;
    }

    /**
     * @brief Get the Next State Names object
     *
     * @return vector<string>&
     */
    vector<string> &GetNextStateNames(state_t *C_StateNode)
    {
        static vector<string> next_states;

        next_states.clear();

        if (CurrentStateNode != nullptr) {
            auto table = StatesTransitions.find(C_StateNode);
            if (table != StatesTransitions.end()) {
                auto to_states = table->second.to_state;

                for (auto &to_state : to_states) {
                    next_states.push_back(to_state.first->name);
                }
            }
        }

        return next_states;
    }

    /**
     * @brief Get the current state number
     *
     * @return uint
     */
    inline uint GetCurrentStateNumber()
    {
        return CurrentStateNode->node_number;
    }

    uint GetCurrentStateLayersCount()
    {
        if (CurrentStateNode != nullptr)
            return CurrentStateNode->layers_count - config.state_mapper.packet_layer_offset;

        return 0;
    }

    /**
     * @brief Get the global offset of the current state
     *
     * @return uint
     */
    uint GetCurrentStateGlobalOffset()
    {
        if (CurrentStateNode != nullptr)
            return CurrentStateNode->global_layer_offset;

        return 0;
    }

    /**
     * @brief Get name of previous state
     *
     * @return string
     */
    string GetPreviousStateName()
    {
        if (PreviousStateNode != nullptr)
            return PreviousStateNode->name;
        else
            return "None";
    }

    /**
     * @brief Go to initial state
     *
     */
    void GoToInitialState()
    {
        stats_transitions = 0;

        mutex_graph.lock();
        if (CurrentStateNode != nullptr) {
            CurrentStateNode->node->unset("color");
        }

        if (PreviousStateNode != nullptr) {
            PreviousStateNode->node->unset("color");
        }

        if (CurrentTransition != nullptr) {
            CurrentTransition->edge->unset("color");
        }

        if (PreviousTransition != nullptr) {
            PreviousTransition->edge->unset("color");
        }

        auto initial_state = StatesNodes.find(this->config.validation.initial_state.c_str());
        if (initial_state != StatesNodes.end()) {
            CurrentStateNode = &initial_state->second;
            CurrentStateNode->node->set("color", "blue");
        }

        mutex_graph.unlock();

        ResetLoopDetector();

        NotifyOnTransition();
    }

    /**
     * @brief Notify the event handler worker
     *
     */
    void NotifyOnTransition()
    {
        evt_on_transition = true;
        sem_post(&mutex_on_event);
    }

    /**
     * @brief Get the default configuration
     *
     * @return string
     */
    string GetDefaultConfig()
    {
        return default_config;
    }

    /**
     * @brief Reset configuration to default values
     *
     * @return boolean
     */
    bool ResetConfig()
    {
        try {
            json default_config = json::parse(GetDefaultConfig(), nullptr, false, true);
            nlohmann::from_json(default_config, *this);
            loaded_config_ref = default_config.dump();
            // Sanity checks
            if (config.options.program >= config.options.default_programs.size())
                config.options.program = 0;

            UpdateConfig();
            return true;
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
        return false;
    }

    /**
     * @brief Enable/Disable State Loop detection.
     * Enable this to detect target unresponsiveness
     *
     * @param value
     */
    void SetLoopDetector(bool en, uint16_t max_count = 5)
    {
        loop_detection_field_hfi = packet_get_header_info("_ws.expert.group");
        loop_detection_enable = en;
        loop_detection_max_count = max_count;
    }

    /**
     * @brief Reset State Loop Detector
     *
     */
    void ResetLoopDetector()
    {
        loop_detection_malformed_count = 0;
        loop_detection_same_state_count = 0;
    }
};

/**
 * @brief State Machine Singleton.
 * There should exist only one instance of the program
 * TODO: Implement multithread version
 */
Machine StateMachine; // Machine Singleton

#endif