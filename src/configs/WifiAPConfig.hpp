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
        bool launch_program_with_gdb;
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

    struct Wifi {
        bool wifi802_11_w;
        bool wifi_allow_internet;
        int64_t wifi_channel;
        std::string wifi_country_code;
        bool wifi_dhcp;
        std::string wifi_dhcp_gateway_address;
        int64_t wifi_eap_method;
        std::vector<std::string> wifi_eap_method_list;
        std::string wifi_interface;
        int64_t wifi_key_auth;
        std::vector<std::string> wifi_key_auth_list;
        std::string wifi_password;
        int64_t wifi_rsn_crypto;
        std::vector<std::string> wifi_rsn_crypto_list;
        std::string wifi_ssid;
        std::string wifi_username;
    };

    struct Config {
        Fuzzing fuzzing;
        Monitor monitor;
        std::string name;
        Options options;
        ServerOptions server_options;
        StateMapper state_mapper;
        Validation validation;
        Wifi wifi;
    };

    struct MachineConfig {
        Config config;
    };
}

namespace nlohmann {
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

    void from_json(const json & j, quicktype::Wifi & x);
    void to_json(json & j, const quicktype::Wifi & x);

    void from_json(const json & j, quicktype::Config & x);
    void to_json(json & j, const quicktype::Config & x);

    void from_json(const json & j, quicktype::MachineConfig & x);
    void to_json(json & j, const quicktype::MachineConfig & x);

    void from_json(const json & j, std::variant<std::vector<std::string>, std::string> & x);
    void to_json(json & j, const std::variant<std::vector<std::string>, std::string> & x);

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
        x.launch_program_with_gdb = j.at("LaunchProgramWithGDB").get<bool>();
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
        j["LaunchProgramWithGDB"] = x.launch_program_with_gdb;
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

    inline void from_json(const json & j, quicktype::Wifi& x) {
        x.wifi802_11_w = j.at("Wifi802.11w").get<bool>();
        x.wifi_allow_internet = j.at("WifiAllowInternet").get<bool>();
        x.wifi_channel = j.at("WifiChannel").get<int64_t>();
        x.wifi_country_code = j.at("WifiCountryCode").get<std::string>();
        x.wifi_dhcp = j.at("WifiDHCP").get<bool>();
        x.wifi_dhcp_gateway_address = j.at("WifiDHCPGatewayAddress").get<std::string>();
        x.wifi_eap_method = j.at("WifiEAPMethod").get<int64_t>();
        x.wifi_eap_method_list = j.at("WifiEAPMethodList").get<std::vector<std::string>>();
        x.wifi_interface = j.at("WifiInterface").get<std::string>();
        x.wifi_key_auth = j.at("WifiKeyAuth").get<int64_t>();
        x.wifi_key_auth_list = j.at("WifiKeyAuthList").get<std::vector<std::string>>();
        x.wifi_password = j.at("WifiPassword").get<std::string>();
        x.wifi_rsn_crypto = j.at("WifiRSNCrypto").get<int64_t>();
        x.wifi_rsn_crypto_list = j.at("WifiRSNCryptoList").get<std::vector<std::string>>();
        x.wifi_ssid = j.at("WifiSSID").get<std::string>();
        x.wifi_username = j.at("WifiUsername").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::Wifi & x) {
        j = json::object();
        j["Wifi802.11w"] = x.wifi802_11_w;
        j["WifiAllowInternet"] = x.wifi_allow_internet;
        j["WifiChannel"] = x.wifi_channel;
        j["WifiCountryCode"] = x.wifi_country_code;
        j["WifiDHCP"] = x.wifi_dhcp;
        j["WifiDHCPGatewayAddress"] = x.wifi_dhcp_gateway_address;
        j["WifiEAPMethod"] = x.wifi_eap_method;
        j["WifiEAPMethodList"] = x.wifi_eap_method_list;
        j["WifiInterface"] = x.wifi_interface;
        j["WifiKeyAuth"] = x.wifi_key_auth;
        j["WifiKeyAuthList"] = x.wifi_key_auth_list;
        j["WifiPassword"] = x.wifi_password;
        j["WifiRSNCrypto"] = x.wifi_rsn_crypto;
        j["WifiRSNCryptoList"] = x.wifi_rsn_crypto_list;
        j["WifiSSID"] = x.wifi_ssid;
        j["WifiUsername"] = x.wifi_username;
    }

    inline void from_json(const json & j, quicktype::Config& x) {
        x.fuzzing = j.at("Fuzzing").get<quicktype::Fuzzing>();
        x.monitor = j.at("Monitor").get<quicktype::Monitor>();
        x.name = j.at("Name").get<std::string>();
        x.options = j.at("Options").get<quicktype::Options>();
        x.server_options = j.at("ServerOptions").get<quicktype::ServerOptions>();
        x.state_mapper = j.at("StateMapper").get<quicktype::StateMapper>();
        x.validation = j.at("Validation").get<quicktype::Validation>();
        x.wifi = j.at("Wifi").get<quicktype::Wifi>();
    }

    inline void to_json(json & j, const quicktype::Config & x) {
        j = json::object();
        j["Fuzzing"] = x.fuzzing;
        j["Monitor"] = x.monitor;
        j["Name"] = x.name;
        j["Options"] = x.options;
        j["ServerOptions"] = x.server_options;
        j["StateMapper"] = x.state_mapper;
        j["Validation"] = x.validation;
        j["Wifi"] = x.wifi;
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
        "Fuzzing": {
            "DefaultDuplicationProbability": 0.3,
            "DefaultMutationFieldProbability": 0.05000000000000002,
            "DefaultMutationProbability": 0.9000000000000002,
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
                    "ApplyTo": "D",
                    "Description": "Probe Response",
                    "Filter": "(wlan.fc.type_subtype == 0x0005)"
                },
                {
                    "ApplyTo": "R",
                    "Description": "Deauth",
                    "Filter": "wlan.fc.type_subtype == 0x000c"
                },
                {
                    "ApplyTo": "V",
                    "Description": "Probe Request",
                    "Filter": "wlan.fc.type_subtype == 0x0004"
                },
                {
                    "ApplyTo": "V",
                    "Description": "Null Data",
                    "Filter": "wlan.fc.type_subtype == 0x0024"
                }
            ],
            "FieldMutationBackoffMultipler": 0.7,
            "MaxDuplicationTime": 6000,
            "MaxFieldsMutation": 4,
            "Mutator": 1,
            "PacketRetry": false,
            "PacketRetryTimeoutMS": 1500,
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
                "ADBDevice": "3ffd4d9a",
                "ADBFilter": "",
                "ADBMagicWords": [
                    "brcmf_fw_crashed",
                    "SOC crashed",
                    "Unable to wake SOC",
                    "crash"
                ],
                "ADBProgram": "logcat"
            },
            "Enable": true,
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
                "SSHHostAddress": "10.42.0.220",
                "SSHMagicWords": [
                    "brcmf_fw_crashed",
                    "Backtrace:",
                    "Oops:",
                    "BUG:",
                    "RIP:",
                    "Call Trace:"
                ],
                "SSHPassword": "raspberry",
                "SSHPort": 22,
                "SSHPreCommands": [""],
                "SSHUsername": "pi"
            },
            "SerialUART": {
                "SerialBaudRate": 115200,
                "SerialMagicWords": [
                    "brcmf_fw_crashed",
                    "Backtrace:",
                    "Oops:",
                    "BUG:",
                    "RIP:",
                    "Call Trace:"
                ],
                "SerialPortName": "/dev/ttyUSB0"
            }
        },
        "Name": "wifi_ap",
        "Options": {
            "AutoRestart": false,
            "AutoStart": true,
            "CoreDump": true,
            "DefaultPCAPProtocol": "encap:IEEE802_11_RADIOTAP",
            "DefaultPrograms": [
                "3rd-party/hostapd/hostapd",
                "3rd-party/hostapd/hostapd_cli"
            ],
            "DefaultProtocol": "encap:IEEE802_11",
            "FileLogger": true,
            "GlobalTimeout": 6,
            "LaunchProgramWithGDB": false,
            "LiveCapture": false,
            "MainThreadCore": 2,
            "Program": 0,
            "SaveCapture": true
        },
        "ServerOptions": {
            "APINamespace": "/",
            "Enable": true,
            "EnableEvents": false,
            "ListenAddress": "0.0.0.0",
            "Logging": false,
            "Port": 3000,
            "ServerModule": 0,
            "ServerModulesList": [
                "SocketIOServer",
                "RESTServer"
            ]
        },
        "StateMapper": {
            "Enable": false,
            "Mapping": [{
                    "AppendSummary": false,
                    "Filter": "eap",
                    "LayerName": "EAP",
                    "StateNameField": [
                        "eap.type",
                        "eap.code"
                    ]
                },
                {
                    "AppendSummary": true,
                    "Filter": "eapol",
                    "LayerName": "802.1X",
                    "StateNameField": [
                        "eapol.keydes.type",
                        "eapol.type"
                    ]
                },
                {
                    "AppendSummary": false,
                    "Filter": "wlan.fixed.action_code",
                    "LayerName": "Action",
                    "StateNameField": "wlan.fixed.action_code"
                },
                {
                    "AppendSummary": false,
                    "Filter": "wlan",
                    "LayerName": "802.11",
                    "StateNameField": "wlan.fc.type_subtype"
                }
            ],
            "Overrides": {},
            "PacketLayerOffset": 0,
            "SaveFolder": "configs/models/wifi_ap/",
            "ShowAllStates": true
        },
        "Validation": {
            "CommonRejections": [{
                "Description": "LMP_not_accepted",
                "Filter": "btbrlmp.op == 4"
            }],
            "DefaultFragmentsLayer": "wlan",
            "DefaultPacketLayer": "wlan",
            "InitialState": "IDLE"
        },
        "Wifi": {
            "Wifi802.11w": false,
            "WifiAllowInternet": true,
            "WifiChannel": 9,
            "WifiCountryCode": "US",
            "WifiDHCP": true,
            "WifiDHCPGatewayAddress": "192.172.42.1",
            "WifiEAPMethod": 0,
            "WifiEAPMethodList": [
                "PEAP",
                "PWD",
                "TTLS",
                "TLS"
            ],
            "WifiInterface": "wlan1",
            "WifiKeyAuth": 0,
            "WifiKeyAuthList": [
                "WPA-EAP",
                "WPA-PSK",
                "SAE"
            ],
            "WifiPassword": "testtest",
            "WifiRSNCrypto": 0,
            "WifiRSNCryptoList": [
                "CCMP",
                "TKIP"
            ],
            "WifiSSID": "TEST_KRA",
            "WifiUsername": "matheus_garbelini"
        }
    }
})/";
