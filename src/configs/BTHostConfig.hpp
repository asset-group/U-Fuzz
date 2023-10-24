//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     MachineConfig data = nlohmann::json::parse(jsonString);

#pragma once

#include <variant>
#include "json.hpp"

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json & j, const std::shared_ptr<T> & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::shared_ptr<T> from_json(const json & j) {
            if (j.is_null()) return std::unique_ptr<T>(); else return std::unique_ptr<T>(new T(j.get<T>()));
        }
    };
}
#endif

namespace quicktype {
    using nlohmann::json;

    inline json get_untyped(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json & j, std::string property) {
        return get_untyped(j, property.data());
    }

    template <typename T>
    inline std::shared_ptr<T> get_optional(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> get_optional(const json & j, std::string property) {
        return get_optional<T>(j, property.data());
    }

    struct Bluetooth {
        int64_t auth_req;
        bool bridge_hci;
        bool disable_role_switch;
        bool enable_bounding;
        int64_t io_cap;
        bool intercept_tx;
        bool lmp_sniffing;
        std::string own_bd_address;
        std::string pin;
        bool rx_bypass;
        bool rx_bypass_on_demand;
        bool randomize_own_bt_address;
        bool serial_auto_discovery;
        int64_t serial_baud_rate;
        bool serial_enable_debug;
        bool serial_enable_debug_hci;
        std::string serial_port;
        bool show_null_poll_packets;
        std::string target_bd_address;
        std::vector<std::string> target_bd_address_list;
    };

    struct Exclude {
        std::string apply_to;
        std::string description;
        std::string filter;
    };

    struct StopCondition {
        bool stop_on_max_iterations;
        int64_t max_iterations;
        bool stop_on_max_time_minutes;
        int64_t max_time_minutes;
    };

    struct Fuzzing {
        double default_duplication_probability;
        double default_mutation_field_probability;
        double default_mutation_probability;
        std::vector<std::string> default_mutators;
        std::vector<std::string> default_selectors;
        std::vector<Exclude> excludes;
        double field_mutation_backoff_multipler;
        int64_t max_duplication_time;
        int64_t max_fields_mutation;
        int64_t mutator;
        bool packet_retry;
        int64_t packet_retry_timeout_ms;
        int64_t selector;
        bool enable_duplication;
        bool enable_mutation;
        bool enable_optimization;
        bool restore_session;
        bool save_session;
        int64_t random_seed;
        StopCondition stop_condition;
    };

    struct Adb {
        std::string adb_device;
        std::string adb_filter;
        std::vector<std::string> adb_magic_words;
        std::string adb_program;
    };

    struct Microphone {
        double microphone_detection_sensitivity;
        int64_t microphone_device_id;
    };

    struct SerialUart {
        int64_t serial_baud_rate;
        std::vector<std::string> serial_magic_words;
        std::string serial_port_name;
    };

    struct Ssh {
        std::string ssh_command;
        bool ssh_enable_pre_commands;
        std::string ssh_host_address;
        std::vector<std::string> ssh_magic_words;
        std::string ssh_password;
        int64_t ssh_port;
        std::vector<std::string> ssh_pre_commands;
        std::string ssh_username;
    };

    struct Monitor {
        Adb adb;
        bool enable;
        Microphone microphone;
        int64_t monitor_type;
        std::vector<std::string> monitor_type_list;
        bool print_to_stdout;
        Ssh ssh;
        SerialUart serial_uart;
    };

    struct Options {
        bool auto_restart;
        bool auto_start;
        bool core_dump;
        std::string default_pcap_protocol;
        std::vector<std::string> default_programs;
        std::string default_protocol;
        bool file_logger;
        int64_t global_timeout;
        bool live_capture;
        int64_t main_thread_core;
        int64_t program;
        bool save_capture;
    };

    struct ServerOptions {
        std::string api_namespace;
        bool enable;
        bool enable_events;
        std::string listen_address;
        bool logging;
        int64_t port;
        int64_t server_module;
        std::vector<std::string> server_modules_list;
    };

    using StateNameField = std::variant<std::vector<std::string>, std::string>;

    struct Mapping {
        bool append_summary;
        std::string filter;
        std::string layer_name;
        StateNameField state_name_field;
    };

    struct Overrides {
    };

    struct StateMapper {
        bool enable;
        std::vector<Mapping> mapping;
        Overrides overrides;
        int64_t packet_layer_offset;
        std::string save_folder;
        bool show_all_states;
    };

    struct CommonRejection {
        std::string description;
        std::string filter;
    };

    struct Validation {
        std::vector<CommonRejection> common_rejections;
        std::string default_fragments_layer;
        std::string default_packet_layer;
        std::string initial_state;
    };

    struct Config {
        Bluetooth bluetooth;
        Fuzzing fuzzing;
        Monitor monitor;
        std::string name;
        Options options;
        ServerOptions server_options;
        StateMapper state_mapper;
        Validation validation;
    };

    struct MachineConfig {
        Config config;
    };
}

namespace nlohmann {
    void from_json(const json & j, quicktype::Bluetooth & x);
    void to_json(json & j, const quicktype::Bluetooth & x);

    void from_json(const json & j, quicktype::Exclude & x);
    void to_json(json & j, const quicktype::Exclude & x);

    void from_json(const json & j, quicktype::StopCondition & x);
    void to_json(json & j, const quicktype::StopCondition & x);

    void from_json(const json & j, quicktype::Fuzzing & x);
    void to_json(json & j, const quicktype::Fuzzing & x);

    void from_json(const json & j, quicktype::Adb & x);
    void to_json(json & j, const quicktype::Adb & x);

    void from_json(const json & j, quicktype::Microphone & x);
    void to_json(json & j, const quicktype::Microphone & x);

    void from_json(const json & j, quicktype::SerialUart & x);
    void to_json(json & j, const quicktype::SerialUart & x);

    void from_json(const json & j, quicktype::Ssh & x);
    void to_json(json & j, const quicktype::Ssh & x);

    void from_json(const json & j, quicktype::Monitor & x);
    void to_json(json & j, const quicktype::Monitor & x);

    void from_json(const json & j, quicktype::Options & x);
    void to_json(json & j, const quicktype::Options & x);

    void from_json(const json & j, quicktype::ServerOptions & x);
    void to_json(json & j, const quicktype::ServerOptions & x);

    void from_json(const json & j, quicktype::Mapping & x);
    void to_json(json & j, const quicktype::Mapping & x);

    void from_json(const json & j, quicktype::Overrides & x);
    void to_json(json & j, const quicktype::Overrides & x);

    void from_json(const json & j, quicktype::StateMapper & x);
    void to_json(json & j, const quicktype::StateMapper & x);

    void from_json(const json & j, quicktype::CommonRejection & x);
    void to_json(json & j, const quicktype::CommonRejection & x);

    void from_json(const json & j, quicktype::Validation & x);
    void to_json(json & j, const quicktype::Validation & x);

    void from_json(const json & j, quicktype::Config & x);
    void to_json(json & j, const quicktype::Config & x);

    void from_json(const json & j, quicktype::MachineConfig & x);
    void to_json(json & j, const quicktype::MachineConfig & x);

    void from_json(const json & j, std::variant<std::vector<std::string>, std::string> & x);
    void to_json(json & j, const std::variant<std::vector<std::string>, std::string> & x);

    inline void from_json(const json & j, quicktype::Bluetooth& x) {
        x.auth_req = j.at("AuthReq").get<int64_t>();
        x.bridge_hci = j.at("BridgeHCI").get<bool>();
        x.disable_role_switch = j.at("DisableRoleSwitch").get<bool>();
        x.enable_bounding = j.at("EnableBounding").get<bool>();
        x.io_cap = j.at("IOCap").get<int64_t>();
        x.intercept_tx = j.at("InterceptTX").get<bool>();
        x.lmp_sniffing = j.at("LMPSniffing").get<bool>();
        x.own_bd_address = j.at("OwnBDAddress").get<std::string>();
        x.pin = j.at("Pin").get<std::string>();
        x.rx_bypass = j.at("RXBypass").get<bool>();
        x.rx_bypass_on_demand = j.at("RXBypassOnDemand").get<bool>();
        x.randomize_own_bt_address = j.at("RandomizeOwnBTAddress").get<bool>();
        x.serial_auto_discovery = j.at("SerialAutoDiscovery").get<bool>();
        x.serial_baud_rate = j.at("SerialBaudRate").get<int64_t>();
        x.serial_enable_debug = j.at("SerialEnableDebug").get<bool>();
        x.serial_enable_debug_hci = j.at("SerialEnableDebugHCI").get<bool>();
        x.serial_port = j.at("SerialPort").get<std::string>();
        x.show_null_poll_packets = j.at("ShowNullPollPackets").get<bool>();
        x.target_bd_address = j.at("TargetBDAddress").get<std::string>();
        x.target_bd_address_list = j.at("TargetBDAddressList").get<std::vector<std::string>>();
    }

    inline void to_json(json & j, const quicktype::Bluetooth & x) {
        j = json::object();
        j["AuthReq"] = x.auth_req;
        j["BridgeHCI"] = x.bridge_hci;
        j["DisableRoleSwitch"] = x.disable_role_switch;
        j["EnableBounding"] = x.enable_bounding;
        j["IOCap"] = x.io_cap;
        j["InterceptTX"] = x.intercept_tx;
        j["LMPSniffing"] = x.lmp_sniffing;
        j["OwnBDAddress"] = x.own_bd_address;
        j["Pin"] = x.pin;
        j["RXBypass"] = x.rx_bypass;
        j["RXBypassOnDemand"] = x.rx_bypass_on_demand;
        j["RandomizeOwnBTAddress"] = x.randomize_own_bt_address;
        j["SerialAutoDiscovery"] = x.serial_auto_discovery;
        j["SerialBaudRate"] = x.serial_baud_rate;
        j["SerialEnableDebug"] = x.serial_enable_debug;
        j["SerialEnableDebugHCI"] = x.serial_enable_debug_hci;
        j["SerialPort"] = x.serial_port;
        j["ShowNullPollPackets"] = x.show_null_poll_packets;
        j["TargetBDAddress"] = x.target_bd_address;
        j["TargetBDAddressList"] = x.target_bd_address_list;
    }

    inline void from_json(const json & j, quicktype::Exclude& x) {
        x.apply_to = j.at("ApplyTo").get<std::string>();
        x.description = j.at("Description").get<std::string>();
        x.filter = j.at("Filter").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::Exclude & x) {
        j = json::object();
        j["ApplyTo"] = x.apply_to;
        j["Description"] = x.description;
        j["Filter"] = x.filter;
    }

    inline void from_json(const json & j, quicktype::StopCondition& x) {
        x.stop_on_max_iterations = j.at("StopOnMaxIterations").get<bool>();
        x.max_iterations = j.at("MaxIterations").get<int64_t>();
        x.stop_on_max_time_minutes = j.at("StopOnMaxTimeMinutes").get<bool>();
        x.max_time_minutes = j.at("MaxTimeMinutes").get<int64_t>();
    }

    inline void to_json(json & j, const quicktype::StopCondition & x) {
        j = json::object();
        j["StopOnMaxIterations"] = x.stop_on_max_iterations;
        j["MaxIterations"] = x.max_iterations;
        j["StopOnMaxTimeMinutes"] = x.stop_on_max_time_minutes;
        j["MaxTimeMinutes"] = x.max_time_minutes;
    }

    inline void from_json(const json & j, quicktype::Fuzzing& x) {
        x.default_duplication_probability = j.at("DefaultDuplicationProbability").get<double>();
        x.default_mutation_field_probability = j.at("DefaultMutationFieldProbability").get<double>();
        x.default_mutation_probability = j.at("DefaultMutationProbability").get<double>();
        x.default_mutators = j.at("DefaultMutators").get<std::vector<std::string>>();
        x.default_selectors = j.at("DefaultSelectors").get<std::vector<std::string>>();
        x.excludes = j.at("Excludes").get<std::vector<quicktype::Exclude>>();
        x.field_mutation_backoff_multipler = j.at("FieldMutationBackoffMultipler").get<double>();
        x.max_duplication_time = j.at("MaxDuplicationTime").get<int64_t>();
        x.max_fields_mutation = j.at("MaxFieldsMutation").get<int64_t>();
        x.mutator = j.at("Mutator").get<int64_t>();
        x.packet_retry = j.at("PacketRetry").get<bool>();
        x.packet_retry_timeout_ms = j.at("PacketRetryTimeoutMS").get<int64_t>();
        x.selector = j.at("Selector").get<int64_t>();
        x.enable_duplication = j.at("enable_duplication").get<bool>();
        x.enable_mutation = j.at("enable_mutation").get<bool>();
        x.enable_optimization = j.at("enable_optimization").get<bool>();
        x.restore_session = j.at("RestoreSession").get<bool>();
        x.save_session = j.at("SaveSession").get<bool>();
        x.random_seed = j.at("RandomSeed").get<int64_t>();
        x.stop_condition = j.at("StopCondition").get<quicktype::StopCondition>();
    }

    inline void to_json(json & j, const quicktype::Fuzzing & x) {
        j = json::object();
        j["DefaultDuplicationProbability"] = x.default_duplication_probability;
        j["DefaultMutationFieldProbability"] = x.default_mutation_field_probability;
        j["DefaultMutationProbability"] = x.default_mutation_probability;
        j["DefaultMutators"] = x.default_mutators;
        j["DefaultSelectors"] = x.default_selectors;
        j["Excludes"] = x.excludes;
        j["FieldMutationBackoffMultipler"] = x.field_mutation_backoff_multipler;
        j["MaxDuplicationTime"] = x.max_duplication_time;
        j["MaxFieldsMutation"] = x.max_fields_mutation;
        j["Mutator"] = x.mutator;
        j["PacketRetry"] = x.packet_retry;
        j["PacketRetryTimeoutMS"] = x.packet_retry_timeout_ms;
        j["Selector"] = x.selector;
        j["enable_duplication"] = x.enable_duplication;
        j["enable_mutation"] = x.enable_mutation;
        j["enable_optimization"] = x.enable_optimization;
        j["RestoreSession"] = x.restore_session;
        j["SaveSession"] = x.save_session;
        j["RandomSeed"] = x.random_seed;
        j["StopCondition"] = x.stop_condition;
    }

    inline void from_json(const json & j, quicktype::Adb& x) {
        x.adb_device = j.at("ADBDevice").get<std::string>();
        x.adb_filter = j.at("ADBFilter").get<std::string>();
        x.adb_magic_words = j.at("ADBMagicWords").get<std::vector<std::string>>();
        x.adb_program = j.at("ADBProgram").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::Adb & x) {
        j = json::object();
        j["ADBDevice"] = x.adb_device;
        j["ADBFilter"] = x.adb_filter;
        j["ADBMagicWords"] = x.adb_magic_words;
        j["ADBProgram"] = x.adb_program;
    }

    inline void from_json(const json & j, quicktype::Microphone& x) {
        x.microphone_detection_sensitivity = j.at("MicrophoneDetectionSensitivity").get<double>();
        x.microphone_device_id = j.at("MicrophoneDeviceId").get<int64_t>();
    }

    inline void to_json(json & j, const quicktype::Microphone & x) {
        j = json::object();
        j["MicrophoneDetectionSensitivity"] = x.microphone_detection_sensitivity;
        j["MicrophoneDeviceId"] = x.microphone_device_id;
    }

    inline void from_json(const json & j, quicktype::SerialUart& x) {
        x.serial_baud_rate = j.at("SerialBaudRate").get<int64_t>();
        x.serial_magic_words = j.at("SerialMagicWords").get<std::vector<std::string>>();
        x.serial_port_name = j.at("SerialPortName").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::SerialUart & x) {
        j = json::object();
        j["SerialBaudRate"] = x.serial_baud_rate;
        j["SerialMagicWords"] = x.serial_magic_words;
        j["SerialPortName"] = x.serial_port_name;
    }

    inline void from_json(const json & j, quicktype::Ssh& x) {
        x.ssh_command = j.at("SSHCommand").get<std::string>();
        x.ssh_enable_pre_commands = j.at("SSHEnablePreCommands").get<bool>();
        x.ssh_host_address = j.at("SSHHostAddress").get<std::string>();
        x.ssh_magic_words = j.at("SSHMagicWords").get<std::vector<std::string>>();
        x.ssh_password = j.at("SSHPassword").get<std::string>();
        x.ssh_port = j.at("SSHPort").get<int64_t>();
        x.ssh_pre_commands = j.at("SSHPreCommands").get<std::vector<std::string>>();
        x.ssh_username = j.at("SSHUsername").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::Ssh & x) {
        j = json::object();
        j["SSHCommand"] = x.ssh_command;
        j["SSHEnablePreCommands"] = x.ssh_enable_pre_commands;
        j["SSHHostAddress"] = x.ssh_host_address;
        j["SSHMagicWords"] = x.ssh_magic_words;
        j["SSHPassword"] = x.ssh_password;
        j["SSHPort"] = x.ssh_port;
        j["SSHPreCommands"] = x.ssh_pre_commands;
        j["SSHUsername"] = x.ssh_username;
    }

    inline void from_json(const json & j, quicktype::Monitor& x) {
        x.adb = j.at("ADB").get<quicktype::Adb>();
        x.enable = j.at("Enable").get<bool>();
        x.microphone = j.at("Microphone").get<quicktype::Microphone>();
        x.monitor_type = j.at("MonitorType").get<int64_t>();
        x.monitor_type_list = j.at("MonitorTypeList").get<std::vector<std::string>>();
        x.print_to_stdout = j.at("PrintToStdout").get<bool>();
        x.ssh = j.at("SSH").get<quicktype::Ssh>();
        x.serial_uart = j.at("SerialUART").get<quicktype::SerialUart>();
    }

    inline void to_json(json & j, const quicktype::Monitor & x) {
        j = json::object();
        j["ADB"] = x.adb;
        j["Enable"] = x.enable;
        j["Microphone"] = x.microphone;
        j["MonitorType"] = x.monitor_type;
        j["MonitorTypeList"] = x.monitor_type_list;
        j["PrintToStdout"] = x.print_to_stdout;
        j["SSH"] = x.ssh;
        j["SerialUART"] = x.serial_uart;
    }

    inline void from_json(const json & j, quicktype::Options& x) {
        x.auto_restart = j.at("AutoRestart").get<bool>();
        x.auto_start = j.at("AutoStart").get<bool>();
        x.core_dump = j.at("CoreDump").get<bool>();
        x.default_pcap_protocol = j.at("DefaultPCAPProtocol").get<std::string>();
        x.default_programs = j.at("DefaultPrograms").get<std::vector<std::string>>();
        x.default_protocol = j.at("DefaultProtocol").get<std::string>();
        x.file_logger = j.at("FileLogger").get<bool>();
        x.global_timeout = j.at("GlobalTimeout").get<int64_t>();
        x.live_capture = j.at("LiveCapture").get<bool>();
        x.main_thread_core = j.at("MainThreadCore").get<int64_t>();
        x.program = j.at("Program").get<int64_t>();
        x.save_capture = j.at("SaveCapture").get<bool>();
    }

    inline void to_json(json & j, const quicktype::Options & x) {
        j = json::object();
        j["AutoRestart"] = x.auto_restart;
        j["AutoStart"] = x.auto_start;
        j["CoreDump"] = x.core_dump;
        j["DefaultPCAPProtocol"] = x.default_pcap_protocol;
        j["DefaultPrograms"] = x.default_programs;
        j["DefaultProtocol"] = x.default_protocol;
        j["FileLogger"] = x.file_logger;
        j["GlobalTimeout"] = x.global_timeout;
        j["LiveCapture"] = x.live_capture;
        j["MainThreadCore"] = x.main_thread_core;
        j["Program"] = x.program;
        j["SaveCapture"] = x.save_capture;
    }

    inline void from_json(const json & j, quicktype::ServerOptions& x) {
        x.api_namespace = j.at("APINamespace").get<std::string>();
        x.enable = j.at("Enable").get<bool>();
        x.enable_events = j.at("EnableEvents").get<bool>();
        x.listen_address = j.at("ListenAddress").get<std::string>();
        x.logging = j.at("Logging").get<bool>();
        x.port = j.at("Port").get<int64_t>();
        x.server_module = j.at("ServerModule").get<int64_t>();
        x.server_modules_list = j.at("ServerModulesList").get<std::vector<std::string>>();
    }

    inline void to_json(json & j, const quicktype::ServerOptions & x) {
        j = json::object();
        j["APINamespace"] = x.api_namespace;
        j["Enable"] = x.enable;
        j["EnableEvents"] = x.enable_events;
        j["ListenAddress"] = x.listen_address;
        j["Logging"] = x.logging;
        j["Port"] = x.port;
        j["ServerModule"] = x.server_module;
        j["ServerModulesList"] = x.server_modules_list;
    }

    inline void from_json(const json & j, quicktype::Mapping& x) {
        x.append_summary = j.at("AppendSummary").get<bool>();
        x.filter = j.at("Filter").get<std::string>();
        x.layer_name = j.at("LayerName").get<std::string>();
        x.state_name_field = j.at("StateNameField").get<quicktype::StateNameField>();
    }

    inline void to_json(json & j, const quicktype::Mapping & x) {
        j = json::object();
        j["AppendSummary"] = x.append_summary;
        j["Filter"] = x.filter;
        j["LayerName"] = x.layer_name;
        j["StateNameField"] = x.state_name_field;
    }

    inline void from_json(const json & j, quicktype::Overrides& x) {
    }

    inline void to_json(json & j, const quicktype::Overrides & x) {
        j = json::object();
    }

    inline void from_json(const json & j, quicktype::StateMapper& x) {
        x.enable = j.at("Enable").get<bool>();
        x.mapping = j.at("Mapping").get<std::vector<quicktype::Mapping>>();
        x.overrides = j.at("Overrides").get<quicktype::Overrides>();
        x.packet_layer_offset = j.at("PacketLayerOffset").get<int64_t>();
        x.save_folder = j.at("SaveFolder").get<std::string>();
        x.show_all_states = j.at("ShowAllStates").get<bool>();
    }

    inline void to_json(json & j, const quicktype::StateMapper & x) {
        j = json::object();
        j["Enable"] = x.enable;
        j["Mapping"] = x.mapping;
        j["Overrides"] = x.overrides;
        j["PacketLayerOffset"] = x.packet_layer_offset;
        j["SaveFolder"] = x.save_folder;
        j["ShowAllStates"] = x.show_all_states;
    }

    inline void from_json(const json & j, quicktype::CommonRejection& x) {
        x.description = j.at("Description").get<std::string>();
        x.filter = j.at("Filter").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::CommonRejection & x) {
        j = json::object();
        j["Description"] = x.description;
        j["Filter"] = x.filter;
    }

    inline void from_json(const json & j, quicktype::Validation& x) {
        x.common_rejections = j.at("CommonRejections").get<std::vector<quicktype::CommonRejection>>();
        x.default_fragments_layer = j.at("DefaultFragmentsLayer").get<std::string>();
        x.default_packet_layer = j.at("DefaultPacketLayer").get<std::string>();
        x.initial_state = j.at("InitialState").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::Validation & x) {
        j = json::object();
        j["CommonRejections"] = x.common_rejections;
        j["DefaultFragmentsLayer"] = x.default_fragments_layer;
        j["DefaultPacketLayer"] = x.default_packet_layer;
        j["InitialState"] = x.initial_state;
    }

    inline void from_json(const json & j, quicktype::Config& x) {
        x.bluetooth = j.at("Bluetooth").get<quicktype::Bluetooth>();
        x.fuzzing = j.at("Fuzzing").get<quicktype::Fuzzing>();
        x.monitor = j.at("Monitor").get<quicktype::Monitor>();
        x.name = j.at("Name").get<std::string>();
        x.options = j.at("Options").get<quicktype::Options>();
        x.server_options = j.at("ServerOptions").get<quicktype::ServerOptions>();
        x.state_mapper = j.at("StateMapper").get<quicktype::StateMapper>();
        x.validation = j.at("Validation").get<quicktype::Validation>();
    }

    inline void to_json(json & j, const quicktype::Config & x) {
        j = json::object();
        j["Bluetooth"] = x.bluetooth;
        j["Fuzzing"] = x.fuzzing;
        j["Monitor"] = x.monitor;
        j["Name"] = x.name;
        j["Options"] = x.options;
        j["ServerOptions"] = x.server_options;
        j["StateMapper"] = x.state_mapper;
        j["Validation"] = x.validation;
    }

    inline void from_json(const json & j, quicktype::MachineConfig& x) {
        x.config = j.at("config").get<quicktype::Config>();
    }

    inline void to_json(json & j, const quicktype::MachineConfig & x) {
        j = json::object();
        j["config"] = x.config;
    }
    inline void from_json(const json & j, std::variant<std::vector<std::string>, std::string> & x) {
        if (j.is_string())
            x = j.get<std::string>();
        else if (j.is_array())
            x = j.get<std::vector<std::string>>();
        else throw "Could not deserialize";
    }

    inline void to_json(json & j, const std::variant<std::vector<std::string>, std::string> & x) {
        switch (x.index()) {
            case 0:
                j = std::get<std::vector<std::string>>(x);
                break;
            case 1:
                j = std::get<std::string>(x);
                break;
            default: throw "Input JSON does not conform to schema";
        }
    }
}


const char *default_config = R"/({
    "config": {
        "Bluetooth": {
            "AuthReq": 3,
            "BridgeHCI": true,
            "DisableRoleSwitch": false,
            "EnableBounding": true,
            "IOCap": 3,
            "InterceptTX": false,
            "LMPSniffing": false,
            "OwnBDAddress": "bc:62:6f:b1:82:a1",
            "Pin": "0000",
            "RXBypass": false,
            "RXBypassOnDemand": false,
            "RandomizeOwnBTAddress": false,
            "SerialAutoDiscovery": false,
            "SerialBaudRate": 4000000,
            "SerialEnableDebug": false,
            "SerialEnableDebugHCI": false,
            "SerialPort": "/dev/ttyUSB1",
            "ShowNullPollPackets": false,
            "TargetBDAddress": "a4:c1:38:d8:ad:a9",
            "TargetBDAddressList": [
                "4E:8B:89:59:ED:BE",
                "24:0A:C4:61:1C:1A",
                "E0:D4:E8:19:C7:69"
            ]
        },
        "Fuzzing": {
            "DefaultDuplicationProbability": 0.5,
            "DefaultMutationFieldProbability": 0.1,
            "DefaultMutationProbability": 0.15,
            "DefaultMutators": [
                "Random",
                "RandomBit",
                "RandomZeroByte",
                "RandomFullByte",
                "ToggleBit"
            ],
            "DefaultSelectors": [
                "Random",
                "Sequential",
                "Overlap"
            ],
            "Excludes": [{
                "ApplyTo": "V",
                "Description": "L2CAP Command Reject",
                "Filter": "btl2cap.rej_reason"
            }],
            "FieldMutationBackoffMultipler": 0.7,
            "MaxDuplicationTime": 6000,
            "MaxFieldsMutation": 6,
            "Mutator": 1,
            "PacketRetry": false,
            "PacketRetryTimeoutMS": 500,
            "Selector": 0,
            "enable_duplication": false,
            "enable_mutation": false,
            "enable_optimization": false,
            "RestoreSession": true,
            "SaveSession": true,
            "RandomSeed": 123456789,
            "StopCondition": {
                "StopOnMaxIterations": true,
                "MaxIterations": 1000,
                "StopOnMaxTimeMinutes": true,
                "MaxTimeMinutes": 240
            }
        },
        "Monitor": {
            "ADB": {
                "ADBDevice": "66766e89",
                "ADBFilter": "",
                "ADBMagicWords": [
                    "SOC crashed",
                    "Unable to wake SOC"
                ],
                "ADBProgram": "logcat"
            },
            "Enable": false,
            "Microphone": {
                "MicrophoneDetectionSensitivity": 0.7,
                "MicrophoneDeviceId": -1
            },
            "MonitorType": 0,
            "MonitorTypeList": [
                "Serial",
                "SSH",
                "Microphone",
                "ADB"
            ],
            "PrintToStdout": true,
            "SSH": {
                "SSHCommand": "sudo dmesg -C && sudo dmesg -e -w",
                "SSHEnablePreCommands": true,
                "SSHHostAddress": "127.0.0.1",
                "SSHMagicWords": [
                    "Backtrace:",
                    "Oops:",
                    "BUG:",
                    "RIP:",
                    "Call Trace:"
                ],
                "SSHPassword": "megaman500",
                "SSHPort": 22,
                "SSHPreCommands": [
                    "sudo sh -c \"echo 'module btusb +p' > /sys/kernel/debug/dynamic_debug/control\"",
                    "sudo sh -c \"echo 'module btintel +p' > /sys/kernel/debug/dynamic_debug/control\"",
                    "sudo sh -c \"echo 'module bluetooth +p' > /sys/kernel/debug/dynamic_debug/control\"",
                    "sudo sh -c \"echo 'module rfcomm +p' > /sys/kernel/debug/dynamic_debug/control\"",
                    "sudo hciconfig hci0 inqparms 18:18",
                    "sudo hciconfig hci0 pageparms 18:18"
                ],
                "SSHUsername": "matheus"
            },
            "SerialUART": {
                "SerialBaudRate": 115200,
                "SerialMagicWords": [
                    "v1.0 Start",
                    "OpenStack()",
                    "DBFW_ASSERT_TYPE_FATAL!!!",
                    "WRAP THOR AI",
                    "BT Started!",
                    "Guru Meditation Error",
                    "Abort()"
                ],
                "SerialPortName": "/dev/ttyUSB0"
            }
        },
        "Name": "BTHost",
        "Options": {
            "AutoRestart": true,
            "AutoStart": true,
            "CoreDump": true,
            "DefaultPCAPProtocol": "encap:BLUETOOTH_HCI_H4",
            "DefaultPrograms": [
                "bin/gatt_browser"
            ],
            "DefaultProtocol": "encap:BLUETOOTH_HCI_H4",
            "FileLogger": true,
            "GlobalTimeout": 10,
            "LiveCapture": false,
            "MainThreadCore": -1,
            "Program": 0,
            "SaveCapture": true
        },
        "ServerOptions": {
            "APINamespace": "/",
            "Enable": true,
            "EnableEvents": false,
            "ListenAddress": "127.0.0.1",
            "Logging": false,
            "Port": 3000,
            "ServerModule": 1,
            "ServerModulesList": [
                "SocketIOServer",
                "RESTServer"
            ]
        },
        "StateMapper": {
            "Enable": false,
            "Mapping": [{
                    "AppendSummary": false,
                    "Filter": "btatt.uuid16",
                    "LayerName": "GATT",
                    "StateNameField": "btatt.uuid16"
                },
                {
                    "AppendSummary": false,
                    "Filter": "btatt.error_code",
                    "LayerName": "ATT Error",
                    "StateNameField": "btatt.error_code"
                },
                {
                    "AppendSummary": false,
                    "Filter": "btatt",
                    "LayerName": "ATT",
                    "StateNameField": "btatt.opcode.method"
                },
                {
                    "AppendSummary": false,
                    "Filter": "btsmp",
                    "LayerName": "SMP",
                    "StateNameField": "btsmp.opcode"
                },
                {
                    "AppendSummary": false,
                    "Filter": "btl2cap",
                    "LayerName": "L2CAP",
                    "StateNameField": "btl2cap.cmd_code"
                },
                {
                    "AppendSummary": false,
                    "Filter": "",
                    "LayerName": "",
                    "StateNameField": [
                        ""
                    ]
                }
            ],
            "Overrides": {},
            "PacketLayerOffset": 1,
            "SaveFolder": "configs/models/bthost/",
            "ShowAllStates": false
        },
        "Validation": {
            "CommonRejections": [{
                "Description": "L2CAP Command Reject",
                "Filter": "btl2cap.rej_reason"
            }],
            "DefaultFragmentsLayer": "btl2cap",
            "DefaultPacketLayer": "btl2cap",
            "InitialState": "IDLE"
        }
    }
})/";
