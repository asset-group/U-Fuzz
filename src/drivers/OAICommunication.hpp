#ifndef __OAICOMMUNICATION__
#define __OAICOMMUNICATION__

#include "Machine.hpp"
#include "MiscUtils.hpp"
#include "gui/GUI_LTE.hpp" // Terminal User Interface
#include "libs/json.hpp"
#include "libs/oai_tracing/T_IDs.h"
#include "libs/refl.hpp"
#include "libs/zmq.hpp"
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

extern "C" {
#include "libs/logs.h"
#include "libs/shared_memory.h"
#include "wdissector.h"
}

using namespace nlohmann;

// ZMQ uses ipc by default (overrided by config file)
#define ZMQ_CONNECT_TIMEOUT_MS 1000
#define ZMQ_CMD_FAILED "update_failed"
#define ZMQ_MAX_JSON_SIZE 8192
#define OAI_MAX_BUFFER_SIZE 8192 * 2

using namespace std;
using namespace std::chrono_literals;
using namespace refl;

inline static char *
z_recv(void *socket, int flags = 0)
{
    zmq_msg_t message;
    zmq_msg_init(&message);

    int rc = zmq_msg_recv(&message, socket, flags);

    if (rc < 0)
        return nullptr; //  Context terminated, exit

    size_t size = zmq_msg_size(&message);
    char *string = (char *)malloc(size + 1);
    memcpy(string, zmq_msg_data(&message), size);
    zmq_msg_close(&message);
    string[size] = 0;
    return (string);
}

// PDU events accepted
struct pdu_types_struct {
    const int DL_PDU_WITH_DATA = T_ENB_MAC_UE_DL_PDU_WITH_DATA;
    const int UP_INITIATE_RA_PROCEDURE = T_ENB_PHY_INITIATE_RA_PROCEDURE;
    const int DL_RAR_PDU_WITH_DATA = T_ENB_MAC_UE_DL_RAR_PDU_WITH_DATA;
    const int UL_PDU_WITH_DATA = T_ENB_MAC_UE_UL_PDU_WITH_DATA;
    const int DL_SIB = T_ENB_MAC_UE_DL_SIB;     // Custom event code to receive SIB packets
    const int DL_MIB = T_ENB_PHY_MIB;           // Custom event code to receibe MIB packets
    const int DL_PDCP_PLAIN = T_ENB_PDCP_PLAIN; // Custom event code to receibe MIB packets
    const int DL_PDCP_ENC = T_ENB_PDCP_ENC;
} pdu_types;

REFL_AUTO(
    type(pdu_types_struct),
    field(DL_PDU_WITH_DATA),
    field(UP_INITIATE_RA_PROCEDURE),
    field(DL_RAR_PDU_WITH_DATA),
    field(UL_PDU_WITH_DATA),
    field(DL_SIB),
    field(DL_MIB),
    field(DL_PDCP_PLAIN),
    field(DL_PDCP_ENC))

class OAISHMCommunication {
private:
    const char *TAG = "[OAIComm] ";
    unordered_map<uint16_t, string> pdu_types_name;
    zmq::socket_t *zmq_sock_events;
    zmq::socket_t *zmq_sock_cmd;
    zmq::socket_t *zmq_sock_duplication;
    uint32_t zmq_cmd_requested = 0;
    uint32_t zmq_duplication_requested = 0;
    thread *zmq_thread_events;
    thread *zmq_thread_cmd;
    thread *zmq_thread_duplication;
    Config *config;
    zmq::context_t zmq_ctx;
    uint8_t zmq_running = 1;

public:
    uint8_t *oai_buffer[MAX_MUTEXES];
    uint16_t pdu_size[MAX_MUTEXES];
    uint8_t pdu_type[MAX_MUTEXES];
    uint8_t isValid[MAX_MUTEXES];
    string eps_submodel;
    string eps_state;
    bool zmq_connected = false;

    inline void init(uint8_t mode, int32_t timeout = -1)
    {
        ios_base::sync_with_stdio(false); // Alows unsync of console print (faster)
        shm_init(mode, OAI_MAX_BUFFER_SIZE, "hello");
        // // Allocate local buffers
        // for (size_t i = 0; i < MAX_MUTEXES; i++)
        // {
        //     oai_buffer[i] = (uint8_t *)malloc(OAI_MAX_BUFFER_SIZE);
        // }

        shm_timeout(timeout);
        // Initialize types struct (Uses reflection)
        GL1Y(TAG, "Enabled OAI Events:");
        for_each(reflect(pdu_types).members, [&](auto member) {
            // Map OAI event ids to their respective names
            pdu_types_name[member(pdu_types)] = member.name.str();
            GL1Y(TAG, member.name.str().c_str(), " = ", member(pdu_types));
        });
        // Get Global Config
        config = &StateMachine.config;

        // Initialize ZMQ Sockets and Threads

        //-----> Core Network Commands REQ socket
        zmq_sock_cmd = new zmq::socket_t(zmq_ctx, ZMQ_REQ);
        zmq_sock_cmd->set(zmq::sockopt::connect_timeout, ZMQ_CONNECT_TIMEOUT_MS);
        zmq_sock_cmd->connect(config->lte.zmq_address_cmd);
        //-----> Core Network Events SUB socket
        zmq_sock_events = new zmq::socket_t(zmq_ctx, ZMQ_SUB);
        zmq_sock_events->set(zmq::sockopt::connect_timeout, ZMQ_CONNECT_TIMEOUT_MS);
        zmq_sock_events->set(zmq::sockopt::subscribe, "");
        zmq_sock_events->connect(config->lte.zmq_address_events);
        //-----> Base Station Duplication REQ socket
        zmq_sock_duplication = new zmq::socket_t(zmq_ctx, ZMQ_REQ);
        zmq_sock_duplication->set(zmq::sockopt::connect_timeout, ZMQ_CONNECT_TIMEOUT_MS);
        zmq_sock_duplication->connect(config->lte.zmq_address_duplication);

        //-----> ZMQ Events thread
        zmq_thread_events = new thread([&]() {
            enable_idle_scheduler();
            try {
                string last_event;

                while (zmq_running) {
                    if (zmq_sock_events->connected()) {
                        zmq::message_t msg;

                        (void)zmq_sock_events->recv(msg);
                        string evt_str = msg.to_string();
                        if (last_event.compare(evt_str) && json::accept(evt_str)) // Validate json and filter same event
                        {
                            zmq_connected = true;
                            last_event = evt_str;
                            json event = json::parse(evt_str);
                            GL1G("[FUZZ_EPS] EVENT Received");
                            for (auto obj : event.items()) {
                                GL1Y("[FUZZ_EPS] Submodel: ", obj.key(), ", State: ", obj.value());
                            }
                            eps_state = event["mme"];
                            // TODO: callback here
                        }
                    }
                    else {
                        zmq_connected = false;
                        this_thread::sleep_for(10ms);
                    }
                }
                zmq_connected = false;
            }
            catch (const std::exception &e) {
                LOG2Y("[zmq_thread_events] ", e.what());
            }
            // Silently exit thread
            pthread_kill(zmq_thread_events->native_handle(), SIGINT);
        });
        // Name thread (for debugging)
        pthread_setname_np(zmq_thread_events->native_handle(), "zmq_thread_events");

        //-----> ZMQ Commands thread
        zmq_thread_cmd = new thread([&]() {
            enable_idle_scheduler();
            try {
                while (zmq_running) {
                    if (zmq_sock_cmd->connected() && zmq_cmd_requested) {
                        zmq::message_t msg;
                        zmq_cmd_requested -= 1;
                        (void)zmq_sock_cmd->recv(msg);
                        if (!msg.to_string().compare(ZMQ_CMD_FAILED)) {
                            GL1Y("[FUZZ_EPS] CMD Failed");
                        }
                    }
                    else {
                        this_thread::sleep_for(10ms);
                    }
                }
            }
            catch (const std::exception &e) {
                LOG2Y("[zmq_thread_cmd] ", e.what());
            }
            // Silently exit thread
            pthread_kill(zmq_thread_cmd->native_handle(), SIGINT);
        });
        // Name thread (for debugging)
        pthread_setname_np(zmq_thread_cmd->native_handle(), "zmq_thread_cmd");

        //-----> ZMQ Duplication thread
        zmq_thread_duplication = new thread([&]() {
            enable_idle_scheduler();
            try {
                while (zmq_running) {
                    if (zmq_sock_duplication->connected() && zmq_duplication_requested) {
                        zmq::message_t msg;
                        zmq_duplication_requested -= 1;
                        (void)zmq_sock_duplication->recv(msg);
                        LOG1(msg);
                        if (!msg.to_string().compare(ZMQ_CMD_FAILED)) {
                            GL1Y("[Duplication] CMD Failed");
                        }
                    }
                    else {
                        this_thread::sleep_for(1ms);
                    }
                }
            }
            catch (const std::exception &e) {
                LOG2Y("[zmq_thread_duplication] ", e.what());
            }
            // Silently exit thread
            pthread_kill(zmq_thread_duplication->native_handle(), SIGINT);
        });
        // Name thread (for debugging)
        pthread_setname_np(zmq_thread_duplication->native_handle(), "zmq_thread_duplication");

        GL1G(TAG, "ZMQ Events Initialized on ", config->lte.zmq_address_events);
        GL1G(TAG, "ZMQ CMD Initialized on ", config->lte.zmq_address_cmd);
        GL1G(TAG, "ZMQ Duplication Initialized on ", config->lte.zmq_address_duplication);
    }

    inline void mutate_nas_field(const char *field_name)
    {
        if (zmq_sock_cmd->connected()) {
            json cmd;
            cmd["fields"][0] = field_name;
            cmd["mutators"][0] = 1;
            send_nas_command(cmd.dump());
        }
    }

    inline void disable_mutate_nas_fields()
    {
        if (zmq_sock_cmd->connected()) {
            json cmd;
            cmd["fields"][0] = "";
            cmd["mutators"][0] = 1;
            send_nas_command(cmd.dump());
        }
    }

    ~OAISHMCommunication()
    {
        zmq_running = 0;
        zmq_sock_cmd->disconnect(config->lte.zmq_address_cmd);
        zmq_sock_events->disconnect(config->lte.zmq_address_events);
        zmq_sock_cmd->close();
        zmq_sock_events->close();
        zmq_ctx.close();
        delete zmq_sock_events;
        delete zmq_sock_cmd;
    }

    inline void wait(uint16_t mutex_num)
    {
        shm_wait(mutex_num);
    }

    inline void notify(uint16_t mutex_num)
    {
        shm_notify(mutex_num);
    }

    inline void send(uint8_t *pdu, uint16_t pdu_size, uint16_t mutex_num)
    {
        memcpy(local_sync.shared_memory[mutex_num] + 2, pdu, pdu_size);
        local_sync.shared_memory[mutex_num][0] = pdu_size & 0xFF;
        local_sync.shared_memory[mutex_num][1] = (pdu_size >> 8) & 0xFF;
        shm_notify(mutex_num);
    }

    inline uint8_t *receive(uint16_t mutex_num)
    {
        // Reset PDU type
        pdu_type[mutex_num] = 0;
        uint8_t status = shm_wait(mutex_num);
        if (status) {
            pdu_size[mutex_num] = *((uint16_t *)local_sync.shared_memory[mutex_num]) - 3; // subtract length (2) and type (1);
            pdu_type[mutex_num] = local_sync.shared_memory[mutex_num][2];                 // Get event type from second position
            // skip size and type
            // memcpy(oai_buffer[mutex_num], local_sync.shared_memory[mutex_num] + 3, pdu_size[mutex_num]);
            oai_buffer[mutex_num] = &local_sync.shared_memory[mutex_num][3];
            isValid[mutex_num] = 1;
            // return local_sync.shared_memory + 3;
            return &local_sync.shared_memory[mutex_num][3];
        }
        isValid[mutex_num] = 0;
        return NULL;
    }

    inline const char *pdu_type_name_by_event(uint16_t event_id)
    {
        return pdu_types_name[event_id].c_str();
    }

    inline const char *pdu_type_name(uint16_t mutex_num)
    {
        return pdu_types_name[pdu_type[mutex_num]].c_str();
    }

    inline bool isEvent(uint16_t mutex_num)
    {
        return pdu_types_name.count(pdu_type[mutex_num]);
    }

    inline void send_nas_command(string cmd_string)
    {
        if (zmq_sock_cmd->connected()) {
            zmq_sock_cmd->send(zmq::buffer(cmd_string), zmq::send_flags::none);
            zmq_cmd_requested += 1;
        }
    }

    inline void send_packet(uint8_t *pkt, uint16_t pkt_len, uint16_t pkt_offset)
    {
        assert((pkt_offset < pkt_len) && (pkt_len > 0));
        if (zmq_sock_duplication->connected()) {
            json j;
            j["sdu_len"] = pkt_len;
            j["pdu_buf"] = vector<uint8_t>(pkt + pkt_offset, pkt + pkt_len - pkt_offset);
            try {
                zmq_sock_duplication->send(zmq::buffer(j.dump()), zmq::send_flags::none);
                zmq_duplication_requested += 1;
            }
            catch (const std::exception &e) {
                LOGR("OAI Busy");
            }
        }
    }
};

class OAICommunication {
public:
    uint16_t mutex_number;
    static OAISHMCommunication OAISHMComm; // OAI Singleton
    static uint8_t OAISHM_initialized;

    OAICommunication(uint8_t mode, uint16_t mutex_number)
    {
        this->mutex_number = mutex_number;

        // Initialize first instance
        if (!OAICommunication::OAISHM_initialized) {
            OAICommunication::OAISHMComm.init(mode);
            OAICommunication::OAISHM_initialized = 1;
        }
    };

    inline void wait()
    {
        OAICommunication::OAISHMComm.wait(mutex_number);
    }

    inline void notify()
    {
        OAICommunication::OAISHMComm.notify(mutex_number);
    }

    inline void send(uint8_t *pdu, uint16_t pdu_size)
    {
        OAICommunication::OAISHMComm.send(pdu, pdu_size, mutex_number);
    }

    inline uint8_t *receive()
    {
        return OAICommunication::OAISHMComm.receive(mutex_number);
    }

    inline uint16_t pdu_size()
    {
        return OAICommunication::OAISHMComm.pdu_size[mutex_number];
    }

    inline uint8_t pdu_type()
    {
        return OAICommunication::OAISHMComm.pdu_type[mutex_number];
    }

    inline uint8_t isValid()
    {
        return OAICommunication::OAISHMComm.isValid[mutex_number];
    }

    inline bool isEvent()
    {
        return OAICommunication::OAISHMComm.isEvent(mutex_number);
    }

    inline const char *pdu_type_name_by_event(uint16_t event_id)
    {
        return OAICommunication::OAISHMComm.pdu_type_name_by_event(event_id);
    }

    inline const char *pdu_type_name()
    {
        return OAICommunication::OAISHMComm.pdu_type_name(mutex_number);
    }
};

uint8_t OAICommunication::OAISHM_initialized{0};  // Initialize static member
OAISHMCommunication OAICommunication::OAISHMComm; // Initialize static member

#endif