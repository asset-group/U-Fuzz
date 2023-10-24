#include "WDMapper.h"

using namespace std;
using namespace misc_utils;

typedef struct
{
    uint8_t *buf;
    unsigned int buf_len;
} graphviz_output_t;

void ConfigurePseudoHeader()
{
    string default_pcap_protocol = StateMachine.config.options.default_pcap_protocol;
    string default_protocol = StateMachine.config.options.default_protocol;

    string_to_lowercase(default_pcap_protocol);
    // printf("This is the default_pcap_protocol", default_pcap_protocol);
    std::cout << "This is the default_pcap_protocol" << default_pcap_protocol << std::endl;
    string_to_lowercase(default_protocol);
    // printf("This is the default_protocol", default_protocol);
    std::cout << "This is the default_protocol" << default_protocol << std::endl;

    // Process pseudoheader when loading pcap files (set direction)
    if (string_contains(default_pcap_protocol, "_h4") || string_contains(default_protocol, "_h4")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
            if (pkt_len >= 4) {
                // Set packet direction
                packet_set_direction((int)pkt[3]);
                // Return pseudoheader offset
                offset = 4;
            }
        });
    }
    else if (string_contains(default_pcap_protocol, "sysdig")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) { offset = 0; });
    }
    else if (string_contains(default_protocol, "mac-lte-framed")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
            packet_set_direction(!(int)pkt[49 + 1]);
            // offset = 49;
        });
    }
    else if (string_contains(default_pcap_protocol, "ieee802_11_radiotap")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
            packet_set_direction((int)pkt[8]);
            offset = 9;
        });
    }
    else if (string_contains(default_pcap_protocol, "wpan")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
            // std::cout << "This is the pkt_len" << pkt_len << std::endl;
            if (pkt_len > 8) {
                // std::cout << "This is the byte 7-8 " << (int)pcap_buffer[7] << " and " << (int)pcap_buffer[8] << std::endl;
                if (pkt[8] == 0) {
                    packet_set_direction(0);
                }
                else {
                    packet_set_direction(1);
                }
            }
            else {
                packet_set_direction(1);
            }
        });
    }
    // This is for MQTT
    // else if (string_contains(default_pcap_protocol, "eth")) {
    //     StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
    //         // std::cout << "This is the pkt_len" << pkt_len << std::endl;
    //         if (pkt_len > 36) {
    //             // std::cout << "This is the byte 7-8 " << (int)pcap_buffer[7] << " and " << (int)pcap_buffer[8] << std::endl;
    //             if (pkt[36] == 0x07 && pkt[37] == 0x5b) {
    //                 packet_set_direction(0);
    //             }
    //             else {
    //                 packet_set_direction(1);
    //             }
    //         }
    //         else {
    //             packet_set_direction(1);
    //         }
    //     });
    // }
    // This is for Coap
    else if (string_contains(default_pcap_protocol, "eth")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
            // std::cout << "This is the pkt_len" << pkt_len << std::endl;
            if (pkt_len > 33) {
                // std::cout << "This is the byte 7-8 " << (int)pcap_buffer[7] << " and " << (int)pcap_buffer[8] << std::endl;
                // this condition means the src port is 5683 which should be TX
                std::cout << "This is the byte 34: " << pkt[34] << " and 35: " << pkt[35] << std::endl;
                if (pkt[34] == 0x16 && pkt[35] == 0x33) {
                    packet_set_direction(0);
                }
                else {
                    packet_set_direction(1);
                }
            }
            else {
                packet_set_direction(1);
            }
        });
    }
    else if (string_contains(default_pcap_protocol, "znp")) {
        StateMachine.OnProcessPseudoHeader([](uint8_t *pkt, uint32_t pkt_len, uint32_t &offset) {
            std::cout << "This is the default proto" << std::endl;
            if (pkt[2] >= 0x60) {
                std::cout << "This is the byte 0 " << pkt[2] << " and " << pkt[3] << std::endl;
                packet_set_direction(1);
            }
            else {
                packet_set_direction(0);
            }
        });
    }
}

graphviz_output_t graphviz_dot_to_format(const char *dot_graph, const char *output_format)
{
    static GVC_t *gvc = gvContext();
    static std::mutex dot_mutex;
    graphviz_output_t output;
    lock_guard<mutex> m(dot_mutex);
    Agraph_t *g;
    char *buf;
    unsigned int buf_len;

    output.buf = NULL;
    output.buf_len = 0;

    profiling_timer_start(11);
    g = agmemread(dot_graph);

    if (g == NULL) {
        return output;
    }

    gvLayout(gvc, g, "dot");
    // agset(g, "splines", "line");
    gvRenderData(gvc, g, output_format, &buf, &buf_len);
    gvFreeLayout(gvc, g);
    agclose(g);

    if (buf) {
        output.buf = (uint8_t *)buf;
        output.buf_len = buf_len;
    }

    return output;
}

void graphviz_free_data(graphviz_output_t &graphviz_output)
{
    if (graphviz_output.buf) {
        gvFreeRenderData((char *)graphviz_output.buf);
        graphviz_output.buf_len = 0;
    }
}

int main(int argc, char **argv)
{
    bool file_saved = false;

    // clang-format off
    cxxopts::Options options("WDMapper", "Creates a State Machine from wireshark protocol fields especification");
    options.add_options()
    ("help", "Print help")
    ("c,config", "Configuration File", cxxopts::value<string>()->default_value("configs/generic_config.json"))
    ("i,input", "Input File (*.json,*.pcap,*.pcapng)", cxxopts::value<vector<std::string>>())
    ("o,output", "Output File (*.dot,.svg,*.png,*.json)", cxxopts::value<string>()->default_value("wdmapper.svg"))
    ("fast", "Generate low quality graph (faster)")
    ("ignore_tx", "Ignores TX Mapping");
    // clang-format on

    try {
        auto result = options.parse(argc, argv);

        if (!result.arguments().size() || result.count("help")) {
            cout << options.help({"", "Group"}) << endl;
            exit(0);
        }

        if (result.count("input")) {
            vector<string> model_paths = result["input"].as<vector<string>>();
            int models_number = model_paths.size();

            // Initialize Generic Model Configuration
            if (!StateMachine.init(result["config"].as<string>().c_str())) {
                LOGR("Configuration file could not be open.");
                exit(1);
            }

            // Set wireshark log level
            wdissector_set_log_level(LOG_LEVEL_CRITICAL);

            // Configure dissection depending on proto type
            ConfigurePseudoHeader();

            // Load input files
            if (models_number == 1)
                LOGY("Loading Model...");
            else
                LOGY("Merging Models...");

            for (size_t i = 0; i < models_number; i++) {
                if (file_saved = StateMachine.LoadModel(model_paths[i].c_str(),
                                                        false,
                                                        (models_number > 1),
                                                        result["ignore_tx"].as<bool>())) {
                    LOGG("Loaded " + model_paths[i]);
                    // Print total states and transitions per layer
                    for (auto &layer : StateMachine.GetStateMap()) {
                        LOG3Y("Layer:\"", layer.layer_name, "\"");
                        LOG4C("--> States:", layer.total_mapped_states, ", Transitions:", layer.total_mapped_transitions);
                    }
                    LOG1("------------------");
                    LOG2Y("Total States: ", StateMachine.TotalStates());
                    LOG2Y("Total Transitions: ", StateMachine.TotalTransitions());
                }
                else
                    LOGR("Error loading " + model_paths[i]);
            }

            // Save State Machine graph (*.png,*.dot,*.svg)
            string original_output_file = result["output"].as<string>();
            string output_file = original_output_file;
            string file_extension = string_file_extension(output_file);
            int save_format;

            if (file_extension == "dot") {
                // dot/json
                auto o_list = string_split(output_file, ".");
                output_file = o_list[o_list.size() - 2] + ".json";
                save_format = 0;
            }
            else if (file_extension == "svg") {
                // svg
                save_format = 1;
            }
            else if (file_extension == "png") {
                // png
                save_format = 2;
            }
            else if (file_extension == "json") {
                // json
                save_format = 0;
            }
            else if (file_extension == "") {
                // dot/json
                output_file += ".json";
                original_output_file = output_file;
                save_format = 0;
            }
            else {
                LOGR("Save format not recognized. Valid extensions are *.dot,*.svg,*.png,*.json");
            }

            // Run save format
            LOGY("Saving file...");

            if (result["fast"].as<bool>()) {
                // Set Faster Rendering for graphviz
                // StateMachine.MachineGraph.set(gvpp::AttrType::GRAPH, "splines", "line");
                StateMachine.MachineGraph.set(gvpp::AttrType::GRAPH, "maxiter", "1");
                StateMachine.MachineGraph.set(gvpp::AttrType::GRAPH, "mclimit", "0.01");
            }

            // Force showing all states
            StateMachine.config.state_mapper.show_all_states = true;

            switch (save_format) {
            case 0:
                file_saved = StateMachine.SaveModel(output_file.c_str());
                break;
            case 1:
            case 2:
                graphviz_output_t graphviz_data = graphviz_dot_to_format(StateMachine.get_graph().c_str(), file_extension.c_str());
                if (!graphviz_data.buf || !graphviz_data.buf_len)
                    break;

                ofstream of(output_file, ios::out | ios::binary);
                if (!of.good())
                    break;

                of.write((char *)graphviz_data.buf, graphviz_data.buf_len);
                of.close();
                graphviz_free_data(graphviz_data);
                file_saved = true;
                break;
            }

            // Save Success
            if (file_saved)
                LOGG("Saved " + original_output_file);
            else
                LOGR("Error saving " + output_file);
        }
    }
    catch (const cxxopts::OptionException &e) {
        cout << "error parsing options: " << e.what() << endl;
        exit(1);
    }
}