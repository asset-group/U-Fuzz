#pragma once

#ifndef __MODULES_WD_INC__
#define __MODULES_WD_INC__

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/byteorder/little_endian.h>
#include <linux/const.h>

#include "wdissector.h"
#ifdef FUZZ_LTE
#include "configs/eNBConfig.hpp"
#elif FUZZ_BT
#include "configs/BTConfig.hpp"
#elif FUZZ_BTHOST
#include "configs/BTHostConfig.hpp"
#elif FUZZ_ZNP
#include "configs/BTHostConfig.hpp"
#elif FUZZ_WIFI_AP
#include "configs/WifiAPConfig.hpp"
#endif

// using namespace quicktype;

// Useful macros to be used in modules

// ---------------- Bluetooth Classic (BR/EDR) ----------------
#define IS_FHS(p_buffer) (((p_buffer[0] >> 3) & 0b1111) == 0x02)
#define IS_LMP(p_buffer) ((p_buffer[2] & 0b11) == 0x03)
#define IS_L2CAP(p_buffer) ((p_buffer[2] & 0b11) == 0x02)
#define IS_L2CAP_FRAGMENT(p_buffer) ((p_buffer[2] & 0b11) == 0x01)
#define IS_L2CAP_CMD(p_buffer, cmd_val) (IS_L2CAP(p_buffer) && (p_buffer[8] == cmd_val))
#define IS_LMP_OPCODE(p_buffer, opcode) (((p_buffer[4] >> 1) & 0x7F) == opcode)
#define IS_LMP_EXT_OPCODE(p_buffer, ext_opcode) (((p_buffer[4] >> 1) == 127) && (p_buffer[5] == ext_opcode))

// ---------------- WiFi 802.11 ----------------
#define _UL(x) (_AC(x, UL))
#define _ULL(x) (_AC(x, ULL))
#define UL(x) (_UL(x))
#define ULL(x) (_ULL(x))
#define BIT(nr) (UL(1) << (nr))

#define WIFI_Get_FrameType(pbuf) (__le16_to_cpu(*(unsigned short *)(pbuf)) & (BIT(3) | BIT(2)))
#define WIFI_Get_FrameSubType(pbuf) (__cpu_to_le16(*(unsigned short *)(pbuf)) & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))
#define WIFI_Get_Sequence(pbuf) (*((unsigned short *)(pbuf + 22)) >> 4)
#define WIFI_Get_FragNum(pbuf) (__cpu_to_le16(*(unsigned short *)((pbuf + 22)) & 0x0f)

#define WIFI_Is_Broadcast(addr)                                     \
    ((addr[4] == 0xff) && (addr[5] == 0xff) && (addr[6] == 0xff) && \
     (addr[7] == 0xff) && (addr[8] == 0xff) && (addr[9] == 0xff))

enum WIFI_FRAME_TYPE {
    WIFI_MGT_TYPE = (0),
    WIFI_CTRL_TYPE = (BIT(2)),
    WIFI_DATA_TYPE = (BIT(3)),
    WIFI_QOS_DATA_TYPE = (BIT(7) | BIT(3)), /* !< QoS Data	 */
};

enum WIFI_FRAME_SUBTYPE {
    /* below is for mgt frame */
    WIFI_ASSOCREQ = (0 | WIFI_MGT_TYPE),
    WIFI_ASSOCRSP = (BIT(4) | WIFI_MGT_TYPE),
    WIFI_REASSOCREQ = (BIT(5) | WIFI_MGT_TYPE),
    WIFI_REASSOCRSP = (BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_PROBEREQ = (BIT(6) | WIFI_MGT_TYPE),
    WIFI_PROBERSP = (BIT(6) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_BEACON = (BIT(7) | WIFI_MGT_TYPE),
    WIFI_ATIM = (BIT(7) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DISASSOC = (BIT(7) | BIT(5) | WIFI_MGT_TYPE),
    WIFI_AUTH = (BIT(7) | BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DEAUTH = (BIT(7) | BIT(6) | WIFI_MGT_TYPE),
    WIFI_ACTION = (BIT(7) | BIT(6) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_ACTION_NOACK = (BIT(7) | BIT(6) | BIT(5) | WIFI_MGT_TYPE),

    /* below is for control frame */
    WIFI_BF_REPORT_POLL = (BIT(6) | WIFI_CTRL_TYPE),
    WIFI_NDPA = (BIT(6) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_BAR = (BIT(7) | WIFI_CTRL_TYPE),
    WIFI_PSPOLL = (BIT(7) | BIT(5) | WIFI_CTRL_TYPE),
    WIFI_RTS = (BIT(7) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_CTS = (BIT(7) | BIT(6) | WIFI_CTRL_TYPE),
    WIFI_ACK = (BIT(7) | BIT(6) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_CFEND = (BIT(7) | BIT(6) | BIT(5) | WIFI_CTRL_TYPE),
    WIFI_CFEND_CFACK = (BIT(7) | BIT(6) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),

    /* below is for data frame */
    WIFI_DATA = (0 | WIFI_DATA_TYPE),
    WIFI_DATA_CFACK = (BIT(4) | WIFI_DATA_TYPE),
    WIFI_DATA_CFPOLL = (BIT(5) | WIFI_DATA_TYPE),
    WIFI_DATA_CFACKPOLL = (BIT(5) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_DATA_NULL = (BIT(6) | WIFI_DATA_TYPE),
    WIFI_CF_ACK = (BIT(6) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_CF_POLL = (BIT(6) | BIT(5) | WIFI_DATA_TYPE),
    WIFI_CF_ACKPOLL = (BIT(6) | BIT(5) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_QOS_DATA_NULL = (BIT(6) | WIFI_QOS_DATA_TYPE),
};

// ---------------- Main modules argument structure ----------------
typedef struct ModulesInclude {
    uint8_t *pkt_buf;
    uint16_t pkt_len;
    uint32_t tx_count;
    uint8_t disconnect;
    uint16_t period;
    uint8_t stop;
    uint8_t stop_on_crash;

} module_request_t;

// ---------------- Main API (declarations) ----------------
// Name
extern const char *module_name();
// Setup
extern int setup(void *p);
// TX Pre
extern int tx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p);
// TX Ppost
extern int tx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p);
// RX Pre
extern int rx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p);
// RX Post
extern int rx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p);

// ---------------- Helper functions ----------------
static inline void m_set_period_ms(void *m, uint16_t period_ms)
{
    module_request_t *ctx = (module_request_t *)m;
    ctx->period = period_ms;
}

static inline void send_packet(void *m, uint8_t *pkt_buf, uint16_t pkt_len, uint32_t tx_count = 1)
{
    if (!m)
        return;
    module_request_t *ctx = (module_request_t *)m;
    ctx->tx_count = tx_count;
    ctx->pkt_buf = pkt_buf;
    ctx->pkt_len = pkt_len;
}

static inline void m_conf_stop_on_crash(void *m, uint8_t val)
{
    if (!m)
        return;
    module_request_t *ctx = (module_request_t *)m;
    ctx->stop_on_crash = val;
}

static inline void m_stop(void *m)
{
    if (!m)
        return;
    module_request_t *ctx = (module_request_t *)m;
    ctx->stop = 1;
}

static inline void m_disconnect(void *m)
{
    if (!m)
        return;
    module_request_t *ctx = (module_request_t *)m;
    ctx->disconnect = 1;
}

#endif