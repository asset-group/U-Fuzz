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

    struct Fuzzing {
        std::vector<Exclude> excludes;
    };

    struct Options {
        std::string default_pcap_protocol;
        std::string default_protocol;
        std::vector<std::string> default_programs;
        int64_t program;
        bool core_dump;
    };

    using StateNameField = std::variant<std::vector<std::string>, std::string>;

    struct Mapping {
        std::string filter;
        std::string layer_name;
        StateNameField state_name_field;
        bool append_summary;
    };

    struct Overrides {
    };

    struct StateMapper {
        std::vector<Mapping> mapping;
        Overrides overrides;
        std::string save_folder;
        bool show_all_states;
        int64_t packet_layer_offset;
    };

    struct CommonRejection {
        std::string description;
        std::string filter;
    };

    struct Validation {
        std::vector<CommonRejection> common_rejections;
        std::string initial_state;
    };

    struct Config {
        std::string name;
        Options options;
        StateMapper state_mapper;
        Fuzzing fuzzing;
        Validation validation;
    };

    struct MachineConfig {
        Config config;
    };
}

namespace nlohmann {
    void from_json(const json & j, quicktype::Exclude & x);
    void to_json(json & j, const quicktype::Exclude & x);

    void from_json(const json & j, quicktype::Fuzzing & x);
    void to_json(json & j, const quicktype::Fuzzing & x);

    void from_json(const json & j, quicktype::Options & x);
    void to_json(json & j, const quicktype::Options & x);

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

    inline void from_json(const json & j, quicktype::Fuzzing& x) {
        x.excludes = j.at("Excludes").get<std::vector<quicktype::Exclude>>();
    }

    inline void to_json(json & j, const quicktype::Fuzzing & x) {
        j = json::object();
        j["Excludes"] = x.excludes;
    }

    inline void from_json(const json & j, quicktype::Options& x) {
        x.default_pcap_protocol = j.at("DefaultPCAPProtocol").get<std::string>();
        x.default_protocol = j.at("DefaultProtocol").get<std::string>();
        x.default_programs = j.at("DefaultPrograms").get<std::vector<std::string>>();
        x.program = j.at("Program").get<int64_t>();
        x.core_dump = j.at("CoreDump").get<bool>();
    }

    inline void to_json(json & j, const quicktype::Options & x) {
        j = json::object();
        j["DefaultPCAPProtocol"] = x.default_pcap_protocol;
        j["DefaultProtocol"] = x.default_protocol;
        j["DefaultPrograms"] = x.default_programs;
        j["Program"] = x.program;
        j["CoreDump"] = x.core_dump;
    }

    inline void from_json(const json & j, quicktype::Mapping& x) {
        x.filter = j.at("Filter").get<std::string>();
        x.layer_name = j.at("LayerName").get<std::string>();
        x.state_name_field = j.at("StateNameField").get<quicktype::StateNameField>();
        x.append_summary = j.at("AppendSummary").get<bool>();
    }

    inline void to_json(json & j, const quicktype::Mapping & x) {
        j = json::object();
        j["Filter"] = x.filter;
        j["LayerName"] = x.layer_name;
        j["StateNameField"] = x.state_name_field;
        j["AppendSummary"] = x.append_summary;
    }

    inline void from_json(const json & j, quicktype::Overrides& x) {
    }

    inline void to_json(json & j, const quicktype::Overrides & x) {
        j = json::object();
    }

    inline void from_json(const json & j, quicktype::StateMapper& x) {
        x.mapping = j.at("Mapping").get<std::vector<quicktype::Mapping>>();
        x.overrides = j.at("Overrides").get<quicktype::Overrides>();
        x.save_folder = j.at("SaveFolder").get<std::string>();
        x.show_all_states = j.at("ShowAllStates").get<bool>();
        x.packet_layer_offset = j.at("PacketLayerOffset").get<int64_t>();
    }

    inline void to_json(json & j, const quicktype::StateMapper & x) {
        j = json::object();
        j["Mapping"] = x.mapping;
        j["Overrides"] = x.overrides;
        j["SaveFolder"] = x.save_folder;
        j["ShowAllStates"] = x.show_all_states;
        j["PacketLayerOffset"] = x.packet_layer_offset;
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
        x.initial_state = j.at("InitialState").get<std::string>();
    }

    inline void to_json(json & j, const quicktype::Validation & x) {
        j = json::object();
        j["CommonRejections"] = x.common_rejections;
        j["InitialState"] = x.initial_state;
    }

    inline void from_json(const json & j, quicktype::Config& x) {
        x.name = j.at("Name").get<std::string>();
        x.options = j.at("Options").get<quicktype::Options>();
        x.state_mapper = j.at("StateMapper").get<quicktype::StateMapper>();
        x.fuzzing = j.at("Fuzzing").get<quicktype::Fuzzing>();
        x.validation = j.at("Validation").get<quicktype::Validation>();
    }

    inline void to_json(json & j, const quicktype::Config & x) {
        j = json::object();
        j["Name"] = x.name;
        j["Options"] = x.options;
        j["StateMapper"] = x.state_mapper;
        j["Fuzzing"] = x.fuzzing;
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
        "Name": "Bluetooth",
        "Options": {
            "DefaultPCAPProtocol": "encap:BLUETOOTH_HCI_H4",
            "DefaultProtocol": "proto:hci_h4",
            "DefaultPrograms": [
                ""
            ],
            "Program": 0,
            "CoreDump": true
        },
        "StateMapper": {
            "Mapping": [
                {
                    "Filter": "btsdp",
                    "LayerName": "SDP",
                    "StateNameField": "btsdp.pdu",
                    "AppendSummary": false
                },
                {
                    "Filter": "bta2dp",
                    "LayerName": "A2DP",
                    "StateNameField": "bta2dp.codec",
                    "AppendSummary": false
                },
                {
                    "Filter": "btavrcp",
                    "LayerName": "AVRCP",
                    "StateNameField": "btavrcp.notification.event_id",
                    "AppendSummary": false
                },
                {
                    "Filter": "btavdtp",
                    "LayerName": "AVDTP",
                    "StateNameField": "btavdtp.signal_id",
                    "AppendSummary": false
                },
                {
                    "Filter": "btrfcomm",
                    "LayerName": "RFCOMM",
                    "StateNameField": "btrfcomm.frame_type",
                    "AppendSummary": false
                },
                {
                    "Filter": "btl2cap",
                    "LayerName": "L2CAP",
                    "StateNameField": "btl2cap.cmd_code",
                    "AppendSummary": false
                },
                {
                    "Filter": "btlmp",
                    "LayerName": "LMP",
                    "StateNameField": [
                        "btbrlmp.eop",
                        "btbrlmp.op"
                    ],
                    "AppendSummary": false
                }
            ],
            "Overrides": {},
            "SaveFolder": "configs/models/generic/",
            "ShowAllStates": true,
            "PacketLayerOffset": 1
        },
        "Fuzzing": {
            "Excludes": [
                {
                    "ApplyTo": "",
                    "Description": "",
                    "Filter": ""
                }
            ]
        },
        "Validation": {
            "CommonRejections": [
                {
                    "Description": "LMP_not_accepted",
                    "Filter": "btbrlmp.op == 4"
                }
            ],
            "InitialState": "IDLE"
        }
    }
})/";
