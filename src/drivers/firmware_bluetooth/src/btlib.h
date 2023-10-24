#ifndef __BTLIB__
#include "rompatcher.h"
// BT Macros
// TX
#define BT_HEADER_BASE(x) (*((uint32_t *)(0x3FFB23BC + x)))
#define ACL_HEADER_BASE(x) (*((uint16_t *)(0x3FFB23BE + x)))
#define ACL_PDU_CONTROL_BASE(x) ((uint8_t *)(*((uint16_t *)(0x3FFB23C2 + x)) + 0x3FFB0000))
#define ACL_PDU_DATA_BASE(x) ((uint8_t *)(*((uint16_t *)(0x3FFB23C0 + x)) + 0x3FFB0000))
#define PATCH_ADDR_LD_PAGE_EM_INIT 0x4003cdeb
// RX
#define ACL_BUFFER_BASE (0x3FFB238EU + (ld_env[426] * 0xe))
#define ACL_BUFFER_LMP_VALUE (*((uint16_t *)ACL_BUFFER_BASE))
#define ACL_BUFFER_LMP_PAYLOAD ((uint8_t *)(ACL_BUFFER_LMP_VALUE + 0x3FFB0000))
#define ACL_BUFFER_DATA_VALUE (*((uint16_t *)(ACL_BUFFER_BASE - 2)))
#define ACL_BUFFER_DATA_PAYLOAD ((uint8_t *)(ACL_BUFFER_DATA_VALUE + 0x3FFB0000))
#define ACL_BUFFER_HEADER (*((uint16_t *)(ACL_BUFFER_BASE - 6))) // 6 bytes down
#define BT_HEADER (*((uint16_t *)(ACL_BUFFER_BASE - 8)))         // 8 bytes down
#define BT_FULL_HEADER (*((uint32_t *)(ACL_BUFFER_BASE - 8)))    // 8 bytes down, includes ACL header
#define ACL_BUFFER_LMP_BASE(x) ((uint8_t *)(*((uint16_t *)(0x3FFB238EU + x)) + 0x3FFB0000U))
#define ACL_BUFFER_DATA_BASE(x) ((uint8_t *)(*((uint16_t *)(0x3FFB238CU + x)) + 0x3FFB0000U))
#define ACL_RX_RF_STATS (*((uint16_t *)(ACL_BUFFER_BASE - 4)))
#define ACL_RX_RF_STATS_CHANNEL ((ACL_RX_RF_STATS >> 8) & 0x7F)

// Misc Macros
#define UPDATE_ADDRESS_HOOK(a, b) (*((uint32_t *)a) = &b)
#define GET_VALUE_FROM_ARRAY8_SAFE(ptr_addr, addr_offset, array_offset) ((*ptr_addr > 0 ? (*(uint8_t **)ptr_addr + addr_offset)[array_offset] : 0))
#define GET_PTT_ACL(offset) GET_VALUE_FROM_ARRAY8_SAFE(ld_acl_env, offset, 0xc1)
#define GET_FLOW_ACL(offset) GET_VALUE_FROM_ARRAY8_SAFE(ld_acl_env, offset, 0xc2)
#define GET_RX_ENCRYPTED(offset) (((*((uint16_t *)(0x3ffb1e06 + (offset * 0x66))) >> 8) & 0b111) > 0)
#define GET_TX_ENCRYPTED(offset) ((*((uint16_t *)(0x3ffb1e06 + (offset * 0x66))) & 0b111) > 0)

#define CONCAT11(a, b) ((uint16_t)((uint16_t)a << 8) | b)

// Misc Defines
#define CUSTOM_LMP_MAGIC 0x12345678
#define CUSTOM_LMP_RETRY_MAGIC 0x12345689
#define MAX_PKTS_IN 128 // Maximum packets in the queue

// Reversed defines
#define _DAT_ram_3ff71064 (*((uint32_t *)0x3ff71064))
#define _DAT_ram_3ff7102c (*((uint32_t *)0x3ff7102c))
#define _DAT_ram_3ff71024 (*((uint32_t *)0x3ff71024))
#define _DAT_ram_3ffb96c8 (*((uint32_t *)0x3ffb96c8))
#define DAT_ram_3ffb2384 (*((uint32_t *)0x3ffb2384))
#define DAT_ram_3ffb2382 (*((uint32_t *)0x3ffb2382))
#define DAT_ram_3ffb1e44 (*((uint32_t *)0x3ffb1e44))
#define DAT_ram_3ff71064 (*((uint32_t *)0x3ff71064))
#define DAT_ram_3ffb23c2 (*((uint32_t *)0x3ffb23c2))
#define DAT_ram_3ffb23ba (*((uint32_t *)0x3ffb23ba))
#define DAT_ram_3ffb23c0 (*((uint32_t *)0x3ffb23c0))
#define DAT_ram_3ffb23be (*((uint32_t *)0x3ffb23be))
#define DAT_ram_3ffb23bc (*((uint32_t *)0x3ffb23bc))
#define DAT_ram_3ffb1e0a (*((uint32_t *)0x3ffb1e0a))
#define DAT_ram_3ffb1e50 (*((uint32_t *)0x3ffb1e50))
#define DAT_ram_3ffb1e52 (*((uint32_t *)0x3ffb1e52))
#define DAT_ram_3ffb1dee (*((uint32_t *)0x3ffb1dee))
#define DAT_ram_3ffb1e04 (*((uint32_t *)0x3ffb1e04))
#define DAT_ram_3ffb1df8 (*((uint32_t *)0x3ffb1df8))
#define DAT_ram_3ffb1dfa (*((uint32_t *)0x3ffb1dfa))
#define DAT_ram_3ffb1dfc (*((uint32_t *)0x3ffb1dfc))
#define DAT_ram_3ffb1dfe (*((uint32_t *)0x3ffb1dfe))
#define DAT_ram_3ffb1e00 (*((uint32_t *)0x3ffb1e00))
#define DAT_ram_3ffb1e02 (*((uint32_t *)0x3ffb1e02))
#define DAT_ram_3ffb1e06 (*((uint32_t *)0x3ffb1e06))
#define DAT_ram_3ffb1e4c (*((uint32_t *)0x3ffb1e4c))
#define DAT_ram_3ffb1e08 (*((uint32_t *)0x3ffb1e08))
#define DAT_ram_3ffb1df0 (*((uint32_t *)0x3ffb1df0))
#define DAT_ram_3ffb1df2 (*((uint32_t *)0x3ffb1df2))
#define DAT_ram_3ffb1df4 (*((uint32_t *)0x3ffb1df4))
#define DAT_ram_3ffb1df6 (*((uint32_t *)0x3ffb1df6))
#define DAT_ram_3ffb1e4e (*((uint32_t *)0x3ffb1e4e))

// ROM Function Pointers
#define r_ke_state_get_ptr 0x3ffafe78
#define r_ke_state_set_ptr 0x3ffafe74
#define r_lmp_unpack_ptr 0x3FFAE764
#define r_ld_acl_lmp_tx_ptr 0x3FFAEC50
#define r_co_list_push_back_ptr 0x3FFAFD74
#define r_co_slot_to_duration_ptr 0x3FFAFDA8
#define r_ke_timer_set_ptr 0x3FFAFE90
#define r_ld_fm_prog_enable_ptr 0x3FFAED60
#define r_ld_fm_prog_push_ptr 0x3ffaed68
#define r_lc_release_ptr 0x3ffaeb04
#define r_ld_acl_rx_enc_ptr 0x3ffaec80
#define r_ld_acl_tx_enc_ptr 0x3ffaec7c
#define r_lc_start_enc_ptr 0x3ffaea64
#define r_ld_acl_edr_set_ptr 0x3ffaeca4
#define r_ld_page_start_ptr 0x3ffaedd8
#define r_ld_util_fhs_pk_ptr 0x3ffaee94
#define r_ld_util_fhs_unpk_ptr 0x3ffaee8c
#define r_bt_util_buf_lmp_tx_alloc_ptr 0x3ffae714
#define r_bt_util_buf_lmp_tx_free_ptr 0x3ffae718
#define r_bt_util_buf_acl_tx_alloc_ptr 0x3ffae724
#define r_bt_util_buf_acl_tx_free_ptr 0x3ffae728
#define r_lc_send_lmp_ptr 0x3ffae918
#define r_ld_acl_allowed_tx_packet_types_set_ptr 0x3ffaeca8
// ROM Functions frame isr callbacks
#define lc_acl_frm_cbk 0x40034414
#define ld_page_frm_cbk 0x4003d280
#define ld_acl_clk_isr_addr 0x40030cf8
#define ld_page_evt_start_cbk_addr 0x4003cf40
#define ld_page_evt_canceled_cbk_addr 0x4003d0f0

// Global pointers
static uint8_t *sw_to_hw = 0x3FFB8D40;
static uint8_t *ld_acl_env = 0x3FFB8258;
// static uint32_t *r_plf_funcs_p = 0x3FFB8360;
static uint32_t *r_ip_funcs_p = 0x3FFAE70C;
static uint8_t *ld_env = 0x3FFB9510;
static uint8_t *ld_sched_env = 0x3ffb830c;
static uint8_t *rwip_rf = 0x3ffbdb28;
static uint8_t *co_default_bdaddr = 0x3ffae704;
extern uint32_t *r_modules_funcs_p;

// Global pointers macros
#define ld_page_env *((uint32_t *)0x3ffb82f0)
#define r_plf_funcs_p *((uint32_t *)0x3ffb8360)

// Pre defined packets
uint8_t pkt_lmp_not_accepted_switch_req[] = {0x1f, 0x00, 0x08, 0x27, 0x1A};
uint8_t pkt_hci_evt_reset[] = {0x04, 0x03, 0x0c, 00};

// Structs
typedef struct
{
    uint32_t *next;
    uint16_t buf_idx;
    uint16_t length;
} lmp_packet_t;

typedef struct
{
    uint16_t buf_idx;
    uint16_t length; // Double check if this is 2 bytes
} acl_packet_t;

typedef union {
    uint16_t raw_header;
    struct __attribute__((packed)) {
        uint8_t llid : 2;
        uint8_t flow : 1;
        uint16_t length : 10;
        uint8_t rfu : 3;
    } fields;

} acl_header_t;

typedef union {
    uint16_t raw_header;
    struct __attribute__((packed)) {
        uint8_t lt_address : 3;
        uint8_t type : 4;
        uint8_t flow : 1;
        uint8_t arqn : 1;
        uint8_t seqn : 1;
        uint16_t hec : 8;
    } fields;

} bt_header_t;

typedef union {
    uint32_t raw_header;
    struct __attribute__((packed)) {
        bt_header_t bt_header;
        acl_header_t acl_header;
    } fields;

} bt_acl_header_t;

typedef struct
{
    uint32_t p1;
    uint32_t p2;
    uint32_t *callback_fcn;
} rx_isr_struct;

typedef struct __attribute__((packed)) {
    uint32_t bt_clock;
    uint8_t channel : 7;
    uint32_t bt_header;
    uint8_t ptt : 1;
    uint8_t role : 1;
    uint8_t custom_lmp : 1;
    uint8_t retry_flag : 1;
    uint8_t direction : 1;
    uint8_t tx_encrypted : 1;
    uint8_t rx_encrypted : 1;
    uint8_t fast_tx : 1;
    uint8_t is_fhs : 1;
    uint8_t intercept_req : 1;
    uint8_t is_eir : 1;
} bt_info_t;

typedef struct __attribute__((packed)) {
    uint16_t size[MAX_PKTS_IN];
    uint8_t *pkt_buffer[MAX_PKTS_IN];
    uint8_t idx;
    bt_info_t bt_info[MAX_PKTS_IN];
    volatile uint8_t queue_spinlock;
} bt_packets_t;

typedef struct
{
    uint16_t size[MAX_PKTS_IN];
    uint8_t *pkt_buffer[MAX_PKTS_IN];
    WORD_ALIGNED_ATTR uint8_t buf_pool[MAX_PKTS_IN][2048];
    uint8_t idx;
    uint8_t idx_pool;
    uint8_t sent;
    uint8_t long_transfer[MAX_PKTS_IN];
    volatile uint queue_spinlock;
    bt_info_t bt_info[MAX_PKTS_IN];
} bt_packets_dma_t;

enum KE_STATE {
    KE_STATE_START = 0x02,
    KE_STATE_VERSION_REQ_TX = 0x16,
    KE_STATE_FEATURE_TX = 0x17,
    KE_STATE_FEATURE_EXT_TX = 0x18,
    KE_STATE_PACKET_TYPE = 0x3C,
    KE_STATE_DETACH_TX = 0x19,
    KE_STATE_DETACH_RX = 0x1C,
    KE_STATE_RELEASE = 0x1B,
    KE_STATE_START_ENC_TX = 0x2C,
    KE_STATE_START_ENC_KEY_SIZE_TX = 0x2A,
    KE_STATE_CLK_OFFSET_REQ_TX = 0x08,
};

// https://github.com/greatscottgadgets/ubertooth/blob/master/firmware/btbr/src/codec/bitstream_proc.c#L295
const uint8_t hec_tbl[32] = {
    0x00,
    0xce,
    0x57,
    0x99,
    0xae,
    0x60,
    0xf9,
    0x37,
    0x97,
    0x59,
    0xc0,
    0x0e,
    0x39,
    0xf7,
    0x6e,
    0xa0,
    0xe5,
    0x2b,
    0xb2,
    0x7c,
    0x4b,
    0x85,
    0x1c,
    0xd2,
    0x72,
    0xbc,
    0x25,
    0xeb,
    0xdc,
    0x12,
    0x8b,
    0x45,
};

// Extern Options
uint8_t enable_intercept_tx = 0;
uint8_t enable_lmp_sniffing = 0;
uint8_t enable_rx_bypass = 0;
uint8_t enable_bypass_on_demand = 0;
uint8_t disable_role_switch = 0;
uint8_t disable_poll_null = 0;

// Extern
extern TaskHandle_t uart_tx_task_handle;
extern TaskHandle_t uart_rx_task_handle;
extern volatile bt_packets_t queue_hci_rx_packets;
extern volatile bt_packets_t queue_hci_tx_packets;
extern volatile bt_packets_t queue_acl_rx_packets;
extern volatile bt_packets_t queue_acl_tx_packets;
extern volatile uint8_t custom_lmp_req;
extern volatile uint8_t spinlock_uart_busy;
extern volatile uint8_t spinlock_tx_intercept_lmp;
extern volatile uint8_t spinlock_tx_intercept_data;
extern uint IRAM_ATTR dma_write_acl_tx(bt_info_t bt_info, uint8_t *dst_array, uint8_t *src_array, uint16_t src_len);

// Hook Functions
uint32_t hook_tx_acl_control(uint16_t header_offset, lmp_packet_t *lmp_packet, uint32_t interrupt_ctx, uint8_t custom_flag, uint8_t retry_flag, uint param_1);
void hook_tx_acl_data(uint16_t offset, uint16_t *len);
uint32_t hook_lc_lmp_rx_handler(uint32_t param_1, uint8_t *param_2, uint32_t param_3);
void hook_ld_page_em_init(uint16_t offset);

// Prototypes
uint32_t reversed_r_ld_acl_lmp_tx(uint param_1, uint param_2, uint32_t custom_flag);

// ROM Functions
extern uint16_t *lc_default_state_tab_p_get();
extern uint ld_acl_tx_packet_type_select(int param_1, uint param_2, uint8_t *param_3);
extern uint32_t r_ld_acl_lmp_tx(uint32_t param1, lmp_packet_t *lmp_packet);
extern uint32_t r_global_int_disable(void);
extern uint32_t r_global_int_restore(uint32_t param_1);
extern int *r_co_list_push_back(int param_1, uint8_t *list_hdr);
extern int *r_co_list_pop_front(int **param_1);
extern int ld_acl_sched(int param_1);
extern void *r_llm_util_set_public_addr(void *bd_addr);
extern int ld_acl_sniff_sched(int param_1, int param_2);
extern uint lc_op_loc_unsniff_req_handler(uint32_t param_1, uint32_t param_2, uint32_t param_3);
extern uint r_ld_read_clock(void);
extern uint lc_afh_update_ind_handler(uint32_t param_1, uint32_t param_2, uint32_t param_3);
extern uint r_ea_elt_remove(int param_1);
extern uint r_lmp_unpack(int *param_1, uint8_t *param_2, uint8_t *param_3);
extern uint lc_op_loc_switch_req_handler(uint32_t param_1, uint32_t param_2, uint32_t param_3);
extern uint lc_lmp_rx_handler(uint32_t param_1, uint8_t *param_2, uint32_t param_3);
extern uint32_t lc_sync_rx_ind_handler(uint32_t param_1, uint16_t *param_2, uint32_t param_3);
extern uint32_t lc_pca_sscan_start_req_handler(uint32_t param_1, uint16_t *param_2, uint32_t param_3);
extern uint32_t lc_lmp_tx_cfm_handler(uint32_t param_1, uint16_t *param_2, uint32_t param_3);
extern uint r_ld_acl_data_tx(uint param_1, char *param_2);
extern uart_tx_one_char(uint8_t c);
extern r_bt_util_buf_lmp_tx_alloc();
extern r_bt_util_buf_lmp_tx_free(uint16_t idx);
extern r_bt_util_buf_acl_tx_alloc();
extern r_bt_util_buf_acl_tx_free(uint16_t idx);
extern r_ke_state_get(uint param1);
extern r_ke_state_set(uint param1, char param_2);
extern uint r_ld_acl_rssi_delta_get(uint param_1);
extern uint8_t *r_lc_release(uint param_1);
extern ushort *r_ld_acl_rx_enc(uint param_1, uint param_2);
extern ushort *r_ld_acl_tx_enc(uint param_1, uint param_2);
extern uint8_t *r_lc_start_enc(uint param_1);
extern ushort *r_ld_fm_prog_push(uint param_1, uint param_2, char param_3, uint param_4, int callback);
extern uint r_ld_acl_t_poll_get(uint param_1);
extern uint r_co_slot_to_duration(uint param_1);
extern uint32_t *r_ke_timer_set(ushort param_1, uint param_2, uint param_3);
extern uint32_t *r_ke_msg_alloc(uint16_t param_1, uint16_t param_2, uint16_t param_3, uint param_4);
extern uint32_t r_ld_acl_lmp_flush(uint param_1);
extern uint32_t r_ld_acl_flow_on(uint param_1);
extern uint r_ld_fm_prog_enable(uint32_t param_1, uint8_t param_2);
extern uint8_t *r_ld_acl_edr_set(uint param_1, uint param_2);
extern uint r_ld_acl_role_get(uint param_1);
extern uint32_t r_ld_page_start(void *param_1);
extern uint32_t r_ea_elt_insert(int **param_1);
extern ushort *r_ke_malloc(int param_1, uint param_2);
extern uint8_t *r_ld_util_bch_create(uint8_t *bd_address, int param_2);
extern uint32_t *r_ld_acl_allowed_tx_packet_types_set(uint param1, ushort param2);
extern uint r_ld_util_fhs_pk(uint param_1, uint8_t *param_2, uint param_3, uint param_4, uint8_t *param_5, uint8_t *param_6, uint8_t param_7, uint8_t param_8);
extern uint r_ld_util_fhs_unpk(uint param_1, uint8_t *param_2, uint8_t *param_3, uint8_t *param_4, uint8_t *param_5, uint *param_6, uint8_t *param_7);
static uint (*ld_acl_resched)(uint param_1, int *param_2) = 0x40033814;
static uint8_t *(*ld_acl_test_mode_update)(uint8_t *param_1) = 0x40032050;
static uint32_t (*ld_acl_end)(uint param1) = 0x40033140;
static uint32_t (*ld_acl_rsw_end)(int param_1, int param_2) = 0x40032bc0;
static uint8_t *(*ld_acl_rx_isr)(uint param_1) = 0x40033aa8;
static uint16_t *(*ld_acl_frm_cbk)(uint16_t *param_1, uint param_2, uint32_t param_3, uint param_4) = 0x40034414;
static uint8_t *(*ld_page_evt_start_cbk)(int param_1) = 0x4003cf40;
static uint8_t *(*ld_page_evt_canceled_cbk)(int **param_1) = 0x4003d0f0;
static uint32_t (*ld_page_em_init)(void) = 0x4003cb34;
static uint (*ld_page_end)(uint8_t param_1) = 0x4003cea8;

// Dummy function pointers
typedef uint32_t(code6)(int, int, int, int, int, int);
typedef uint32_t(code5)(int, int, int, int, int);
typedef uint32_t(code4)(int, int, int, int);
typedef uint32_t(code3)(int, int, int);
typedef uint32_t(code2)(int, int);
typedef uint32_t(code1)(int);
typedef uint32_t(code0)(void);

// Misc vars
uint16_t custom_lmp_header = 0;
uint16_t custom_bt_header = 0;

// -------- BT Functions ------------

uint8_t custom_send_lmp(uint8_t *buf, uint16_t buf_size, uint32_t conn_handler, uint8_t retry_flag)
{

    // Get Current BT Status
    uint32_t status = r_ke_state_get(1);
    int iVar5 = *(int *)(ld_acl_env + (conn_handler * 4));
    // ets_printf("%d\n", *(uint8_t *)(iVar5 + 0xc5));
    // ets_printf("%d\n", status);
    if (status != 0 && (*(uint8_t *)(iVar5 + 0xc5) < 2)) {
        // if (status != 0)
        // {
        static lmp_packet_t *pkt_buf = NULL;
        if (pkt_buf = r_bt_util_buf_lmp_tx_alloc()) {
            // 52248
            // r_ld_acl_allowed_tx_packet_types_set(0, 52248);
            bt_header_t *bt_hdr = (bt_header_t *)buf;
            acl_header_t *acl_hdr = (acl_header_t *)(buf + 2);
            // ets_printf("l:%d\n", acl_hdr->fields.length);
            // DM1
            // Fix lmp max length
            // if (acl_hdr->fields.length > 17)
            //     acl_hdr->fields.length = 17;
            custom_bt_header = bt_hdr->raw_header;
            custom_lmp_header = acl_hdr->raw_header;
            // util_print_buffer(buf, buf_size);
            // Fill lmp_packet_t structure
            pkt_buf->length = acl_hdr->fields.length;
            if (pkt_buf->length > 17)
                pkt_buf->length = 17;
            memcpy(pkt_buf->buf_idx + 0x3FFB0000, buf + 4, pkt_buf->length);
            // uart_tx_one_char('D');
            // uart_tx_one_char('\n');
            // r_ld_acl_flow_on(conn_handler);
            // int poll_ = r_ld_acl_t_poll_get(conn_handler);
            // ets_printf("%d,%d\n", poll_, r_co_slot_to_duration(poll_));
            // ets_printf("l:%d,buf_idx:%d\n", pkt_buf->length, pkt_buf->buf_idx);
            // util_print_buffer(buf, buf_size);
            // puts("----");
            if (reversed_r_ld_acl_lmp_tx(conn_handler, pkt_buf, (retry_flag == 0 ? CUSTOM_LMP_MAGIC : CUSTOM_LMP_RETRY_MAGIC)) == 0) {
                // r_bt_util_buf_lmp_tx_free(pkt_buf->buf_idx);
            }

            return 1;

            // r_ld_acl_lmp_flush(conn_handler);
        }
        else {
            return 0;
        }
    }

    return 0;
}

// Send LMP Only (No BT Header present)
uint8_t internal_custom_send_lmp(uint8_t *buf, uint16_t buf_size, uint32_t conn_handler)
{
    // Get Current BT Status
    uint32_t status = r_ke_state_get(1);
    int iVar5 = *(int *)(ld_acl_env + (conn_handler * 4));
    if (status != 0 && (*(uint8_t *)(iVar5 + 0xc5) < 2)) {
        static lmp_packet_t *pkt_buf = NULL;
        if (pkt_buf = r_bt_util_buf_lmp_tx_alloc()) {
            acl_header_t *acl_hdr = (acl_header_t *)buf;
            // Fix lmp max length
            if (acl_hdr->fields.length > 17)
                acl_hdr->fields.length = 17;

            // custom_lmp_header = acl_hdr->raw_header;
            // Fill lmp_packet_t structure
            pkt_buf->length = acl_hdr->fields.length;
            memcpy(pkt_buf->buf_idx + 0x3FFB0000, buf + 2, pkt_buf->length);

            reversed_r_ld_acl_lmp_tx(conn_handler, pkt_buf, 0);
        }
        return 1;
    }

    return 0;
}

// ----------------------- Utils ---------------------------
uint8_t IRAM_ATTR pkt_queue_push(bt_packets_t *pkt_queue, uint8_t *r_buf, uint16_t r_size, bt_info_t *bt_info)
{

    if (pkt_queue->idx < MAX_PKTS_IN) {
        uint8_t *buffer_queue = NULL;

        if (r_size) {
            buffer_queue = malloc(r_size);
            if (!buffer_queue)
                return 0;
            memcpy(buffer_queue, r_buf, r_size);
        }

        while (pkt_queue->queue_spinlock)
            ;
        pkt_queue->queue_spinlock = 1;
        pkt_queue->size[pkt_queue->idx] = r_size;
        pkt_queue->pkt_buffer[pkt_queue->idx] = buffer_queue;
        if (bt_info) {
            pkt_queue->bt_info[pkt_queue->idx] = *bt_info;
        }
        pkt_queue->idx = pkt_queue->idx + 1;
        pkt_queue->queue_spinlock = 0;
        return 1;
    }
    return 0;
}

static uint8_t IRAM_ATTR pkt_queue_push_dma(bt_packets_dma_t *pkt_queue, uint8_t *r_buf, uint16_t r_size, bt_info_t *bt_info)
{
    uint16_t o_size = r_size;
    uint8_t offset = 2; // 2B headroom for short packets

    if (bt_info != NULL)
        r_size += 13; // BT header (11) + 2 (SPI byte flags)

    if (r_size > 32) {
        r_size -= (32 - 4 - 1); // exclude spi hdr (4B) + last block seq (1B)
        uint32_t bits;
        __asm__ __volatile__("nsau %1,%0"
                             : "=r"(bits)
                             : "r"(r_size));
        bits = (32 - bits);
        r_size = (1 << bits);
        offset = 4; // 4B Headroom for long packets
    }
    else {
        r_size = 32;
    }

    if (pkt_queue->idx <= MAX_PKTS_IN) {
        while (pkt_queue->queue_spinlock)
            ;
        pkt_queue->queue_spinlock = 1;

        uint8_t c_idx = pkt_queue->idx;
        pkt_queue->sent = false;
        pkt_queue->size[c_idx] = r_size;
        pkt_queue->pkt_buffer[c_idx] = (uint8_t *)pkt_queue->buf_pool[pkt_queue->idx_pool];

        if (bt_info != NULL) {
            // ADD bt_info to array
            dma_write_acl_tx(*bt_info, pkt_queue->pkt_buffer[c_idx] + offset, r_buf, o_size);
            // puts("bt_info");
            // uart_tx_one_char('B');
            // uart_tx_one_char('\n');
            // ets_printf("2.bt_ifo:%d,%d\n", r_size, o_size);
        }
        else {
            // util_print_buffer(r_buf, o_size);
            memcpy(pkt_queue->pkt_buffer[c_idx] + offset, r_buf, o_size);
            // ets_printf("2.memcpy:%d,%d\n", r_size, o_size);
        }

        if (offset > 2) {
            pkt_queue->long_transfer[c_idx] = 1;
            pkt_queue->pkt_buffer[c_idx][1] = 0x55; // Long
            *((uint16_t *)(pkt_queue->pkt_buffer[c_idx] + 2)) = r_size;
        }
        else {
            pkt_queue->long_transfer[c_idx] = 0;
            pkt_queue->pkt_buffer[c_idx][1] = 0xAA; // Short
        }

        pkt_queue->idx_pool = (pkt_queue->idx_pool + 1) % (MAX_PKTS_IN - 1);
        pkt_queue->idx++;
        pkt_queue->queue_spinlock = 0;

        return 1;
    }
    return 0;
}

void IRAM_ATTR pkt_queue_pop(bt_packets_t *pkt_queue)
{
    while (pkt_queue->queue_spinlock)
        ;
    pkt_queue->queue_spinlock = 1;
    if (pkt_queue->idx) {
        if (pkt_queue->size[0])
            free(pkt_queue->pkt_buffer[0]);
        for (uint8_t i = 0; i < pkt_queue->idx - 1; i++) {
            pkt_queue->pkt_buffer[i] = pkt_queue->pkt_buffer[i + 1];
            pkt_queue->size[i] = pkt_queue->size[i + 1];
            pkt_queue->bt_info[i] = pkt_queue->bt_info[i + 1];
        }
        pkt_queue->idx--;
    }
    pkt_queue->queue_spinlock = 0;
}

void IRAM_ATTR pkt_queue_pop_dma(bt_packets_dma_t *pkt_queue)
{
    while (pkt_queue->queue_spinlock)
        ;
    pkt_queue->queue_spinlock = 1;
    if (pkt_queue->idx) {
        for (uint8_t i = 0; i < pkt_queue->idx - 1; i++) {
            pkt_queue->pkt_buffer[i] = pkt_queue->pkt_buffer[i + 1];
            pkt_queue->size[i] = pkt_queue->size[i + 1];
            pkt_queue->bt_info[i] = pkt_queue->bt_info[i + 1];
            pkt_queue->long_transfer[i] = pkt_queue->long_transfer[i + 1];
        }
        pkt_queue->sent = false;
        pkt_queue->idx--;
    }
    pkt_queue->queue_spinlock = 0;
}

void IRAM_ATTR pkt_queue_clear(bt_packets_t *pkt_queue)
{
    while (pkt_queue->idx) {
        pkt_queue_pop(pkt_queue);
    }
}

void IRAM_ATTR pkt_queue_clear_dma(bt_packets_t *pkt_queue)
{
    while (pkt_queue->idx) {
        pkt_queue_pop_dma(pkt_queue);
    }
}

void print_bt_header(uint16_t bt_header)
{
    uint8_t lt_addr = bt_header & 0b111;
    uint8_t pkt_type = ((bt_header >> 3) & 0b1111);
    uint8_t flow = ((bt_header >> 7) & 0b1);
    uint8_t arqn = ((bt_header >> 8) & 0b1);
    uint8_t seqn = ((bt_header >> 9) & 0b1);
    uint8_t hec = bt_header >> 10;

    const char *type_str = NULL;

    switch (pkt_type) {
    case 0:
        type_str = "NULL";
        break;

    case 1:
        type_str = "POLL";
        break;

    case 2:
        type_str = "FHS";
        break;

    case 3:
        type_str = "DM1";
        break;

    case 4:
        type_str = "DH1";
        break;

    case 8:
        type_str = "3-DH1";
        break;

    case 9:
        type_str = "AUX";
        break;

    case 0xA:
        type_str = "DM3";
        break;

    case 0xE:
        type_str = "DM5";
        break;

    case 0xF:
        type_str = "DH5";
        break;

    default:
        type_str = "Ukn";
        break;
    }

    ets_printf("%d,%s,%d,%d,%d,%d\n", lt_addr, type_str, flow, arqn, seqn, hec);
}

static inline uint8_t reverse8(uint8_t data)
{
    uint8_t reversed = 0;

    for (size_t i = 0; i < 8; i++) {
        reversed |= ((data >> i) & 0x01) << (7 - i);
    }

    return reversed;
}

static inline uint8_t hec_compute(uint16_t data, uint8_t hec)
{
    extern const uint8_t hec_tbl[32];

    hec = reverse8(hec);
    hec = (hec >> 5) ^ hec_tbl[(data ^ hec) & 0x1f];
    hec = (hec >> 5) ^ hec_tbl[((data >> 5) ^ hec) & 0x1f];
    return hec;
}

// -------- Reversed Functions -----------

// Custom callback handler
uint32_t custom_lc_reset_lc_default_state_funcs(void)
{
    // static uint8_t disable = 1;
    uint16_t uVar1;
    code3 *pcVar2;
    uint16_t *puVar3;

    // Do not execute function at first (function to be first called at main)
    // if (disable == 1)
    // {
    //     disable = 0;
    //     return 0;
    // }

    puVar3 = lc_default_state_tab_p_get();
    do {
        uVar1 = *puVar3;
        if (uVar1 == 0x522) {
            return 0x525;
        }
        if (uVar1 == 0x50e) {
            *(code3 **)(puVar3 + 2) = lc_op_loc_unsniff_req_handler;
        }
        else {
            if (uVar1 < 0x50f) {
                if (uVar1 == 0x508) {
                    pcVar2 = lc_afh_update_ind_handler;
                }
                else {
                    if (uVar1 == 0x50c) {
                        pcVar2 = lc_op_loc_switch_req_handler;
                    }
                    else {
                        if (uVar1 != 0x501)
                            goto LAB_ram_40128ec0;
                        pcVar2 = hook_lc_lmp_rx_handler;
                    }
                }
                *(code3 **)(puVar3 + 2) = pcVar2;
            }
            else {
                if (uVar1 == 0x52f) {
                    *(code3 **)(puVar3 + 2) = lc_sync_rx_ind_handler;
                }
                else {
                    if (uVar1 == 0x533) {
                        pcVar2 = lc_pca_sscan_start_req_handler;
                    }
                    else {
                        if (uVar1 != 0x525)
                            goto LAB_ram_40128ec0;
                        pcVar2 = lc_lmp_tx_cfm_handler;
                    }
                    *(code3 **)(puVar3 + 2) = pcVar2;
                }
            }
        }
    LAB_ram_40128ec0:
        puVar3 = puVar3 + 4;
    } while (true);
}

uint32_t reversed_r_ld_acl_lmp_tx(uint param_1, uint param_2, uint32_t custom_flag)

{
    bool bVar1;
    uint32_t uVar2;
    int iVar3;
    uint clock;
    int iVar5;
    uint8_t bVar6;
    uint uVar7;
    uint8_t bVar8;
    uint unaff_a12;
    uint unaff_a13;
    uint32_t uVar9;

    if (!param_2)
        return;

    uint8_t retry_flag = (custom_flag == CUSTOM_LMP_RETRY_MAGIC);
    uint8_t custom_lmp_req = (custom_flag == CUSTOM_LMP_MAGIC || retry_flag);

    if ((enable_rx_bypass) && !custom_lmp_req) {
        // Disable any packet form ESP32 controller to be sent.
        // This allow complete control over Link Manager Protocol.
        return 0;
    }

    param_1 = param_1 & 0xff;

    if (sw_to_hw[22] != 0xff) {
        clock = 1 << 0x20 - ((sw_to_hw[22] & 3) * -8 + ' ');

        *((uint32_t *)0x3ff71064) = (clock ^ 0xffffffff) & *((uint32_t *)0x3ff71064) | clock;
    }
    if (sw_to_hw[21] != 0xff) {
        clock = 1 << 0x20 - ((sw_to_hw[21] & 3) * -8 + ' ');

        *((uint32_t *)0x3ff71064) = (clock ^ 0xffffffff) & *((uint32_t *)0x3ff71064) | clock;
    }
    // uVar9 = r_global_int_disable();
    iVar5 = *(int *)(ld_acl_env + param_1 * 4);
    uVar2 = 0xc;
    if (iVar5 != 0) {
        // ets_printf("%d\n", *(uint8_t *)(iVar5 + 0xc5));
        uVar2 = 0;
        if (*(uint8_t *)(iVar5 + 0xc5) < 2) {

            iVar3 = ((uint) * (uint8_t *)(iVar5 + 0xb2) * 2 + (uint) * (uint8_t *)(iVar5 + 0xc4) & 0xff) * 10;
            unaff_a13 = 7;

            // Overwrite ACL header
            if (custom_lmp_req) {
                *(ushort *)(0x3ffb23bc + iVar3) = custom_bt_header;
                *(ushort *)(0x3ffb23be + iVar3) = custom_lmp_header;
            }
            else {
                *(ushort *)(0x3ffb23bc + iVar3) = *(ushort *)(0x3ffb23bc + iVar3) & 0xff87 | 0x18;
                *(ushort *)(0x3ffb23be + iVar3) = (ushort) * (uint8_t *)(param_2 + 6) << 3 | 7;
            }

            *(ushort *)(0x3ffb23c2 + iVar3) = *(ushort *)(param_2 + 4);
            // Hook TX packet here to capture ACL Header
            hook_tx_acl_control(iVar3, param_2, uVar9, custom_lmp_req, retry_flag, param_1);
            // Update packet type
            // ld_acl_tx_packet_type_select(param_1,)
            // Enable TX
            *(ushort *)(0x3ffb23ba + iVar3) = *(ushort *)(0x3ffb23ba + iVar3) & 0x7fff;
            // uVar9 = r_global_int_disable();
            // r_global_int_restore(uVar9);

            bVar8 = sw_to_hw[21];
            bVar1 = *(char *)(iVar5 + 0xc4) == '\0';
            param_2 = (uint)sw_to_hw[21];
            *(bool *)(iVar5 + 0xc4) = bVar1;
            bVar6 = sw_to_hw[21];
            if (bVar8 != 0xff) {
                bVar8 = bVar8 << 3;

                unaff_a13 = 0xffffffff;
                param_2 = (4 << 0x20 - (' ' - (bVar8 & 0x1f)) ^ 0xffffffffU) & *((uint32_t *)0x3ff71064);
                *((uint32_t *)0x3ff71064) = ((uint)bVar1 << 2) << 0x20 - (' ' - (bVar8 & 0x1f)) | param_2;
            }
            clock = (uint) * (uint8_t *)(iVar5 + 0xc5) + 1;
            unaff_a12 = clock & 0xff;
            *(uint8_t *)(iVar5 + 0xc5) = (uint8_t)unaff_a12;
            if (bVar6 != 0xff) {
                bVar6 = bVar6 << 3;

                unaff_a12 = ((clock & 3) << 4) << 0x20 - (' ' - (bVar6 & 0x1f));
                param_2 = 0xffffffff;

                unaff_a13 = *((uint32_t *)0x3ff71064);
                *((uint32_t *)0x3ff71064) =
                    unaff_a12 | (0x30 << 0x20 - (' ' - (bVar6 & 0x1f)) ^ 0xffffffffU) & *((uint32_t *)0x3ff71064);
            }
            *(uint8_t *)(iVar5 + 0xbe) = 1;
            if ((*(char *)(iVar5 + 0xb4) == '\0') &&
                ((*(char *)(iVar5 + 0xb5) == '\0' ||
                  ((*(char *)(iVar5 + 0xb5) == '\x02' && (*(char *)(iVar5 + 0xf1) != '\0')))))) {
                iVar3 = *(int *)(ld_acl_env + param_1 * 4);
                clock = r_ld_read_clock();
                uVar7 = *(int *)(iVar3 + 8) - clock & 0x7ffffff;
                param_2 = 0x4000000;
                if ((uVar7 < 0x4000001) && (param_2 = 10, 10 < uVar7)) {
                    r_ea_elt_remove(iVar3);
                    *(uint *)(iVar3 + 8) = clock;
                    if (*(char *)(iVar5 + 0xb5) == '\0') {
                        // uart_tx_one_char('S');
                        // uart_tx_one_char('\n');
                        ld_acl_sched(param_1);
                    }
                    else {
                        ld_acl_sniff_sched(param_1, clock);
                        param_2 = clock;
                    }
                }
            }
            // uart_tx_one_char('N');
            // uart_tx_one_char('\n');
            // Normal Path
        }
        else {
            // uart_tx_one_char('P');
            // uart_tx_one_char('\n');
            // uVar2 = 1;
            r_co_list_push_back(iVar5 + 0x40, param_2);
        }
    }
    // r_global_int_restore(uVar9);
    if (sw_to_hw[22] != 0xff) {

        *((uint32_t *)0x3ff71064) =
            (uint)(0xfffffffefffffffe >> (sw_to_hw[22] & 3) * -8 + 0x20) & *((uint32_t *)0x3ff71064);
    }
    if (sw_to_hw[21] != 0xff) {

        *((uint32_t *)0x3ff71064) =
            (uint)(0xfffffffefffffffe >> (sw_to_hw[21] & 3) * -8 + 0x20) & *((uint32_t *)0x3ff71064);
    }
    return uVar2;
}

uint32_t reversed_ld_page_em_init(void)

{
    uint8_t bVar1;
    char cVar2;
    int iVar3;
    ushort *puVar4;
    int iVar5;
    ushort *puVar6;
    ushort *puVar7;
    ushort *puVar8;

    iVar5 = ld_page_env;
    bVar1 = *(uint8_t *)(ld_page_env + 0x41);
    iVar3 = (uint)bVar1 * 0x66;
    puVar4 = (ushort *)(&DAT_ram_3ffb1dee + iVar3);
    puVar8 = (ushort *)(&DAT_ram_3ffb1e04 + iVar3);

    *puVar4 = *puVar4 & 0xffe0 | 4;

    *(uint16_t *)(&DAT_ram_3ffb1df8 + iVar3) = *(uint16_t *)(iVar5 + 0x28);

    *(uint16_t *)(&DAT_ram_3ffb1dfa + iVar3) = *(uint16_t *)(iVar5 + 0x2a);

    *(uint16_t *)(&DAT_ram_3ffb1dfc + iVar3) = *(uint16_t *)(iVar5 + 0x2c);

    *(uint16_t *)(&DAT_ram_3ffb1dfe + iVar3) = *(uint16_t *)(iVar5 + 0x44);

    *(uint16_t *)(&DAT_ram_3ffb1e00 + iVar3) = *(uint16_t *)(iVar5 + 0x46);

    *(ushort *)(&DAT_ram_3ffb1e02 + iVar3) = (ushort) * (uint8_t *)(iVar5 + 0x48) & 3;

    *puVar8 = (ushort)rwip_rf[44] | *puVar8 & 0xff00;
    puVar6 = (ushort *)(&DAT_ram_3ffb1e06 + iVar3);

    *puVar6 = *puVar6 & 0xfff7;
    puVar7 = (ushort *)(&DAT_ram_3ffb1e4c + iVar3);

    *puVar7 = *puVar7 & 0x7fff;

    *puVar7 = *puVar7 & 0xfbff;

    *puVar7 = *puVar7 & 0xfffe;

    *puVar7 = *puVar7 & 0xfffd;

    *puVar7 = *puVar7 & 0xdfff;

    *puVar7 = *puVar7 & 0xefff;

    *puVar8 = *puVar8 & 0x7fff | 0x8000;

    *(uint16_t *)(&DAT_ram_3ffb1e08 + iVar3) = 0x30;

    *(uint16_t *)(&DAT_ram_3ffb1df0 + iVar3) = 0;

    *(uint16_t *)(&DAT_ram_3ffb1df2 + iVar3) = *(uint16_t *)(iVar5 + 0x3c);

    *(uint16_t *)(&DAT_ram_3ffb1df4 + iVar3) = 0;

    *puVar6 = *puVar6 & 0x1fff;
    puVar7 = &DAT_ram_3ffb1df6 + (uint)bVar1 * 0x33;

    *puVar7 = *puVar7 & 0x7fff;

    *puVar7 = *puVar7 & 0xbfff;

    *puVar7 = *puVar7 & 0xefff;

    *puVar6 = *puVar6 & 0xfffc;

    *puVar6 = *puVar6 & 0xfcff;

    *puVar6 = *puVar6 & 0xfffb;

    *puVar6 = *puVar6 & 0xfbff;

    *puVar6 = *puVar6 & 0xff7f;
    cVar2 = *(char *)(iVar5 + 0x54);

    *puVar7 = *puVar7 & 0xf7ff;
    if (cVar2 == '\0') {
        iVar5 = ((uint) * (uint8_t *)(iVar5 + 0x41) & 0x7f) * 0x14;

        *(uint16_t *)(&DAT_ram_3ffb23bc + iVar5) = 0x390;

        *(uint16_t *)(&DAT_ram_3ffb23be + iVar5) = 0x92;

        *(uint16_t *)(&DAT_ram_3ffb23c0 + iVar5) = 0;

        *(uint16_t *)(&DAT_ram_3ffb23c2 + iVar5) = 0x262c;

        *(uint16_t *)(&DAT_ram_3ffb23ba + iVar5) = 0;

        hook_ld_page_em_init(iVar5);

        *(short *)(&DAT_ram_3ffb1e0a + iVar3) = (short)iVar5 + 0x23ba;
    }
    else {

        *(uint16_t *)(&DAT_ram_3ffb1e0a + iVar3) = 0;
    }
    if (false) {
        (**(code3 **)(r_plf_funcs_p + 0x14))("(((uint16_t)aclincpriostep << 13) & ~((uint16_t)0x0000E000)) == 0", "ld_page.c", 0xa33);
    }
    if (false) {
        (**(code3 **)(r_plf_funcs_p + 0x14))("(((uint16_t)aclminprio << 8) & ~((uint16_t)0x00001F00)) == 0", "ld_page.c", 0xa34);
    }
    if (false) {
        (**(code3 **)(r_plf_funcs_p + 0x14))("(((uint16_t)accurrentprio << 0) & ~((uint16_t)0x0000001F)) == 0", "ld_page.c", 0xa36);
    }

    *(uint16_t *)(&DAT_ram_3ffb1e50 + iVar3) = 0x2808;

    *(uint16_t *)(&DAT_ram_3ffb1e52 + iVar3) = 0;

    *puVar4 = 0x5704;
    return 0x5704;
}

uint32_t reversed_r_ld_page_start(void *param_1)

{
    uint8_t uVar1;
    int **ppiVar2;
    uint16_t uVar3;
    uint32_t uVar4;
    int *piVar5;
    int **__dest;
    uint uVar6;
    uint uVar7;
    uint uVar8;
    uint8_t *unaff_a14;
    uint32_t unaff_a15;
    int iVar9;

    uVar4 = 0xc;
    if (ld_page_env == (int **)0x0) {
        ld_page_env = (int **)r_ke_malloc(0x58, 0);
        if (ld_page_env != (int **)0x0) {
            piVar5 = (int *)r_ld_read_clock();
            ppiVar2 = ld_page_env;
            uVar8 = (int)piVar5 + (uint) * (ushort *)((int)param_1 + 0xc) & 0x7ffffff;
            uVar7 = uVar8 - (int)piVar5 & 0x7ffffff;
            if (0x4000000 < uVar7) {
                uVar7 = -((uint)((int)piVar5 - uVar8) & 0x7ffffff);
            }
            memset(ld_page_env, 0, 0x58);
            *(code1 **)(ppiVar2 + 9) = ld_page_evt_canceled_cbk;
            *(code1 **)(ppiVar2 + 7) = ld_page_evt_start_cbk;
            *(uint8_t *)((int)ppiVar2 + 0x19) = 2;
            *(uint8_t *)((int)ppiVar2 + 0x16) = 8;
            uVar3 = _DAT_ram_3ffb96c8;
            ppiVar2[2] = piVar5;
            *(uint16_t *)(ppiVar2 + 4) = 0x4841;
            __dest = ppiVar2 + 10;
            *(uint16_t *)((int)ppiVar2 + 0x12) = uVar3;
            memcpy(__dest, param_1, 6);
            *(uint8_t *)((int)ppiVar2 + 0x41) = *(uint8_t *)((int)param_1 + 0x11);
            ppiVar2[0xc] = *(int **)((int)param_1 + 8);
            *(uint16_t *)(ppiVar2 + 0xf) = *(uint16_t *)((int)param_1 + 0xc);
            *(uint8_t *)(ppiVar2 + 0x10) = *(uint8_t *)((int)param_1 + 0x12);
            uVar1 = *(uint8_t *)((int)param_1 + 0x13);
            *(uint16_t *)((int)ppiVar2 + 0x3e) = *(uint16_t *)((int)param_1 + 0xe);
            *(uint8_t *)(ppiVar2 + 0x15) = uVar1;
            *(uint8_t *)((int)ppiVar2 + 0x43) = 0;
            if (sw_to_hw[17] != 0xff) {

                _DAT_ram_3ff71064 =
                    (0x80 << 0x20 - ((sw_to_hw[17] & 3) * -8 + ' ') ^ 0xffffffffU) & _DAT_ram_3ff71064;
            }
            ppiVar2[0xd] = (int *)0xffffffff;
            r_ld_util_bch_create((uint8_t *)__dest, (int)(ppiVar2 + 0x11));
            uVar8 = (uint) * (uint8_t *)(ppiVar2 + 0x15);
            if (*(uint8_t *)(ppiVar2 + 0x15) == 0) {
                unaff_a14 = ld_env;
                unaff_a15 = 0x3ffb9516;
                r_ld_util_fhs_pk(0x262c, ld_env + 0x1a2, uVar8, uVar8, ld_env, ld_env + 6,
                                 *(uint8_t *)(ppiVar2 + 0x10), 0);
            }

            reversed_ld_page_em_init();

            _DAT_ram_3ff71024 = _DAT_ram_3ff71024 & 0x7fffffff;

            uVar6 = (uint) * (ushort *)((int)ppiVar2 + 0x3e) << 0x10;
            if ((uVar6 & 0xf800ffff) != 0) {
                (**(code6 **)(r_plf_funcs_p + 0x14))("(((uint32_t)abtpagetime << 16) & ~((uint32_t)0x07FF0000)) == 0", "ld_page.c",
                                                     0x847, uVar8, unaff_a14, unaff_a15);
            }

            _DAT_ram_3ff71024 =
                (uint)(0x7800 < uVar7 + 0x4000) << 0x1e |
                (uVar6 | _DAT_ram_3ff71024 & 0xf800ffff) & 0x2fffffff | 0x90000000;

            iVar9 = r_ea_elt_insert(ppiVar2);
            if (iVar9 == 0) {
                *(uint8_t *)((int)ppiVar2 + 0x42) = 0;
                return 0;
            }
            (**(code6 **)(r_plf_funcs_p + 0x14))("0", "ld_page.c", 0x328, uVar8, unaff_a14, unaff_a15);
            return 0xc;
        }
        (**(code3 **)(r_plf_funcs_p + 0x14))("0", "ld_page.c", 0x32d);
    }
    return uVar4;
}

int **reversed_ld_page_frm_cbk(uint32_t param_1, uint32_t param_2, uint32_t param_3, uint32_t param_4)

{
    uint8_t bVar1;
    ushort uVar2;
    int **ppiVar3;
    uint uVar4;
    ushort *puVar5;
    ushort *puVar6;
    short *psVar7;
    uint8_t uVar8;
    uint32_t unaff_a13;
    int *piVar9;
    int iVar10;
    int iVar11;

    param_4 = param_4 & 0xff;
    if (param_4 == 0) {
        ppiVar3 = ld_page_env;
        r_ea_elt_remove((int *)ld_page_env);
        *(uint8_t *)((int)ppiVar3 + 0x42) = 0;
        ld_page_evt_canceled_cbk(ppiVar3);
        return ppiVar3;
    }
    if (param_4 != 1) {
        ppiVar3 = *(int ***)(r_plf_funcs_p + 0x18);
        (*(code4 *)ppiVar3)(param_2 & 0xff, param_4, "ld_page.c", 0x2b6);
        return ppiVar3;
    }
    if (sw_to_hw[17] != 0xff) {
        uVar4 = 0x40 << 0x20 - ((sw_to_hw[17] & 3) * -8 + ' ');

        _DAT_ram_3ff71064 = (uVar4 ^ 0xffffffff) & _DAT_ram_3ff71064 | uVar4;
    }
    if (ld_page_env == (int **)0x0) {
        (**(code3 **)(r_plf_funcs_p + 0x14))("0", "ld_page.c", 0x292);
        goto LAB_ram_4003d7ac;
    }
    ppiVar3 = ld_page_env;
    r_ea_elt_remove((int *)ld_page_env);
    if (*(char *)((int)ppiVar3 + 0x42) == '\x02') {
        ld_page_end(0x16);
        if ((short)(&DAT_ram_3ffb2382)[(uint)ld_env[426] * 7] < 0) {
            if ((uint)(*(ushort *)(&DAT_ram_3ffb2384 + (uint)ld_env[426] * 0xe) >> 0xb) !=
                (uint) * (uint8_t *)((int)ppiVar3 + 0x41)) {
                unaff_a13 = 0x201;
                (**(code4 **)(r_plf_funcs_p + 0x18))((uint) * (uint8_t *)((int)ppiVar3 + 0x41),
                                                     (uint)(*(ushort *)(&DAT_ram_3ffb2384 + (uint)ld_env[426] * 0xe) >> 0xb),
                                                     "ld_page.c", 0x201);
            }

            (&DAT_ram_3ffb2382)[(uint)ld_env[426] * 7] =
                (&DAT_ram_3ffb2382)[(uint)ld_env[426] * 7] & 0x7fff;
            uVar4 = (uint)ld_env[426] + 1 & 3;
            ld_env[426] = (uint8_t)uVar4;

            if (uVar4 * 0xe + 0x2382 != (_DAT_ram_3ff7102c & 0x7fff)) {
                (**(code4 **)(r_plf_funcs_p + 0x14))(
                    "(EM_BT_RXDESC_OFFSET + ld_env.curr_rxdesc_index * REG_EM_BT_RXDESC_SIZE) ==bt_et_currentrxdescptr_currentrxdescptr_getf()", "ld_page.c", 0x209, unaff_a13);
            }
            psVar7 = &DAT_ram_3ffb2382;
            iVar11 = 0;
            do {
                if (*psVar7 < 0) {
                    (**(code4 **)(r_plf_funcs_p + 0x18))((uint)ld_env[426], iVar11, "ld_page.c", 0x20c);
                }
                iVar11 = iVar11 + 1;
                psVar7 = psVar7 + 7;
            } while (iVar11 != 4);
        }
        goto LAB_ram_4003d7ac;
    }
    piVar9 = (int *)r_ld_read_clock();
    *(uint8_t *)((int)ppiVar3 + 0x42) = 0;
    bVar1 = *(uint8_t *)((int)ppiVar3 + 0x43);
    ppiVar3[2] = piVar9;
    uVar4 = (uint) * (uint8_t *)((int)ppiVar3 + 0x41);
    if (bVar1 == 0) {
        if ((*(char *)(ppiVar3 + 0x15) == '\0') || (-1 < *(short *)(&DAT_ram_3ffb1e44 + uVar4 * 0x66))) {
            iVar11 = uVar4 * 0x66;
            if ((*(short *)(&DAT_ram_3ffb1e44 + iVar11) < 0) &&
                (((*(ushort *)(&DAT_ram_3ffb1e4e + iVar11) & 0x2000) != 0 &&
                  (puVar5 = (ushort *)(&DAT_ram_3ffb1dee + iVar11), (*puVar5 & 0x1f) == 6)))) {
                piVar9 = (int *)((int)piVar9 + 0x21U & 0x7ffffff);
                *(uint8_t *)((int)ppiVar3 + 0x16) = 0x14;
                ppiVar3[0xe] = piVar9;
                *(uint16_t *)((int)ppiVar3 + 0x12) = 0xe2e;
                ppiVar3[3] = piVar9;
                *(uint16_t *)(ppiVar3 + 4) = 0x8842;
                iVar10 = r_ea_elt_insert(ppiVar3);
                if (iVar10 == 0) {

                    *puVar5 = *puVar5 & 0xffe0 | 2;

                    *(ushort *)(&DAT_ram_3ffb1df8 + iVar11) = CONCAT11(ld_env[1], ld_env[0]);

                    *(ushort *)(&DAT_ram_3ffb1dfa + iVar11) = CONCAT11(ld_env[3], ld_env[2]);

                    *(ushort *)(&DAT_ram_3ffb1dfc + iVar11) = CONCAT11(ld_env[5], ld_env[4]);

                    *(ushort *)(&DAT_ram_3ffb1dfe + iVar11) = CONCAT11(ld_env[419], ld_env[418]);

                    *(ushort *)(&DAT_ram_3ffb1e00 + iVar11) = CONCAT11(ld_env[421], ld_env[420]);

                    *(ushort *)(&DAT_ram_3ffb1e02 + iVar11) = (ushort)ld_env[422] & 3;
                    puVar5 = &DAT_ram_3ffb1df6 + uVar4 * 0x33;

                    *puVar5 = *puVar5 & 0xbfff;
                    bVar1 = *(uint8_t *)(ppiVar3 + 0x10);
                    if (((uint)bVar1 << 8 & 0xfffff8ff) != 0) {
                        (**(code3 **)(r_plf_funcs_p + 0x14))("(((uint16_t)aclltaddr << 8) & ~((uint16_t)0x00000700)) == 0", "ld_page.c",
                                                             0x1c5);
                    }

                    *puVar5 = *puVar5 & 0xf8ff | (ushort)((uint)bVar1 << 8);

                    *(ushort *)(&DAT_ram_3ffb1e04 + iVar11) =
                        *(ushort *)(&DAT_ram_3ffb1e04 + iVar11) & 0x7fff | 0x8000;

                    *(ushort *)(&DAT_ram_3ffb1e06 + iVar11) = *(ushort *)(&DAT_ram_3ffb1e06 + iVar11) & 0xfff7;

                    *(uint16_t *)(&DAT_ram_3ffb1e0a + iVar11) = 0;
                    puVar5 = (ushort *)(&DAT_ram_3ffb1e4c + iVar11);

                    *puVar5 = *puVar5 & 0xefff | 0x1000;

                    *puVar5 = *puVar5 & 0xdfff | 0x2000;

                    *(uint16_t *)(&DAT_ram_3ffb1df2 + iVar11) = 0;

                    *(uint16_t *)(&DAT_ram_3ffb1df4 + iVar11) = 0;
                    *(uint8_t *)((int)ppiVar3 + 0x43) = 1;
                    if (sw_to_hw[17] != 0xff) {
                        uVar4 = 0x80 << 0x20 - ((sw_to_hw[17] & 3) * -8 + ' ');

                        _DAT_ram_3ff71064 = (uVar4 ^ 0xffffffff) & _DAT_ram_3ff71064 | uVar4;
                    }
                    goto LAB_ram_4003d7ac;
                }
            }
            *(uint8_t *)((int)ppiVar3 + 0x16) = 8;
            *(uint16_t *)((int)ppiVar3 + 0x12) = _DAT_ram_3ffb96c8;
            *(uint16_t *)(ppiVar3 + 4) = 0x8841;
            ppiVar3[3] = ppiVar3[0xd];
            iVar11 = r_ea_elt_insert(ppiVar3);
            if (iVar11 == 0)
                goto LAB_ram_4003d7ac;
            goto LAB_ram_4003d640;
        }
    LAB_ram_4003d3e4:
        uVar8 = 0;
    }
    else {
        if (bVar1 != 1) {
            (**(code4 **)(r_plf_funcs_p + 0x18))((uint)bVar1, 0, "ld_page.c", 0x28a);
            goto LAB_ram_4003d7ac;
        }
        puVar5 = &DAT_ram_3ffb2382 + (uint)ld_env[426] * 7;
        if ((short)*puVar5 < 0) {
            puVar6 = (ushort *)(&DAT_ram_3ffb2384 + (uint)ld_env[426] * 0xe);
            ld_env[426] = ld_env[426] + 1 & 3;
            uVar2 = *puVar6;
            if ((uint)(*puVar6 >> 0xb) != (uint) * (uint8_t *)((int)ld_page_env + 0x41)) {
                unaff_a13 = 0x103;
                (**(code5 **)(r_plf_funcs_p + 0x18))((uint) * (uint8_t *)((int)ld_page_env + 0x41), (uint)(*puVar6 >> 0xb), "ld_page.c", 0x103, (uint)uVar2);
            }

            *puVar5 = *puVar5 & 0x7fff;

            if ((uint)ld_env[426] * 0xe + 0x2382 != (_DAT_ram_3ff7102c & 0x7fff)) {
                (**(code5 **)(r_plf_funcs_p + 0x14))(
                    "(EM_BT_RXDESC_OFFSET + ld_env.curr_rxdesc_index * REG_EM_BT_RXDESC_SIZE) ==bt_et_currentrxdescptr_currentrxdescptr_getf()", "ld_page.c", 0x110, unaff_a13, (uint)uVar2);
            }
            psVar7 = &DAT_ram_3ffb2382;
            iVar11 = 0;
            do {
                if (*psVar7 < 0) {
                    (**(code5 **)(r_plf_funcs_p + 0x18))((uint)ld_env[426], iVar11, "ld_page.c", 0x113, *(code1 **)(r_plf_funcs_p + 0x18));
                }
                iVar11 = iVar11 + 1;
                psVar7 = psVar7 + 7;
            } while (iVar11 != 4);
            if ((uVar2 & 3) == 0)
                goto LAB_ram_4003d3e4;
        }
        iVar11 = r_ea_elt_insert(ppiVar3);
        bVar1 = sw_to_hw[17];
        if (iVar11 == 0)
            goto LAB_ram_4003d7ac;
        *(uint8_t *)((int)ppiVar3 + 0x43) = 0;
        if (bVar1 != 0xff) {

            _DAT_ram_3ff71064 =
                (0x80 << 0x20 - ((bVar1 & 3) * -8 + ' ') ^ 0xffffffffU) & _DAT_ram_3ff71064;
        }
        *(uint8_t *)((int)ppiVar3 + 0x16) = 8;
        *(uint16_t *)((int)ppiVar3 + 0x12) = _DAT_ram_3ffb96c8;
        *(uint16_t *)(ppiVar3 + 4) = 0x8841;
        ppiVar3[3] = ppiVar3[0xd];
        iVar11 = r_ea_elt_insert(ppiVar3);
        if (iVar11 == 0) {
            ld_page_em_init();
            goto LAB_ram_4003d7ac;
        }
    LAB_ram_4003d640:
        uVar8 = 4;
    }
    ld_page_end(uVar8);
LAB_ram_4003d7ac:
    if (sw_to_hw[17] == 0xff) {
        return (int **)0xff;
    }

    _DAT_ram_3ff71064 =
        (0x40 << 0x20 - ((sw_to_hw[17] & 3) * -8 + ' ') ^ 0xffffffffU) & _DAT_ram_3ff71064;
    return (int **)&DAT_ram_3ff71064;
}

int *reversed_r_co_list_push_back(int *param_1, uint32_t *list_hdr)
{
    uint uVar1;

    if (list_hdr == NULL) {
        (**(code3 **)(r_plf_funcs_p + 0x14))("list_hdr != NULL", "co_list.c", 0x6f);
    }
    if (*param_1 == 0) {
        *(uint32_t **)param_1 = list_hdr;
    }
    else {
        *(uint32_t **)param_1[1] = list_hdr;
    }
    *(uint32_t **)(param_1 + 1) = list_hdr;
    *list_hdr = 0;
    uVar1 = param_1[2] + 1;
    param_1[2] = uVar1;
    if ((uint)param_1[3] < uVar1) {
        param_1[3] = uVar1;
    }
    return param_1;
}

uint reversed_r_co_slot_to_duration(uint slots)
{
    slots = slots & 0xffff;
    if (slots < 0x20) {
        slots = 0x20;
    }
    return slots >> 4;
}

uint reversed_r_ld_acl_role_get(uint param_1)

{
    uint uVar1;
    uVar1 = 0xff;
    if ((&ld_acl_env)[param_1 & 0xff] != (uint8_t *)0x0) {
        uVar1 = (uint)(&ld_acl_env)[param_1 & 0xff][0xb3];
    }
    return uVar1;
}

#endif
