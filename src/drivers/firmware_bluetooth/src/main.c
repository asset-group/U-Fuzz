
// SDK Includes
#include <driver/gpio.h>
#include <driver/spi_slave.h>
#include <driver/uart.h>
#include <esp_bt.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
// Project Includes
#include "btlib.h"

// Defines
#define VERSION_STRING "1.4.1"

#define UART_BAUD_RATE 4000000U
#define UART_BUF_SIZE (2048)
#define UART_RD_BUF_SIZE (BUF_SIZE)
#define UART_TIMEOUT_MS 10 / portTICK_RATE_MS
#define ENABLE_EXPERIMENTAL_SPI 0
#define DIRECTION_RX 1
#define DIRECTION_TX 0
#define H4_NONE 0x00
#define H4_CMD 0x01
#define H4_ACL 0x02
#define H4_SCO 0x03
#define H4_EVT 0x04
#define H4_EVT_HW_ERROR 0x10

#define GPIO_HANDSHAKE 2
#define GPIO_MOSI 12
#define GPIO_MISO 15
#define GPIO_SCLK 13
#define GPIO_CS 14

// Macros
#define delay_ms(x) vTaskDelay(x / portTICK_RATE_MS)
#define millis() (xthal_get_ccount() / 240000U)
#define micros() (xthal_get_ccount() / 240U)

#define BT_DEVICE_NAME "ESP32BTFuzz"

// Protocol codes. If this appear to be too random, it's because they are :-)
enum SERIAL_COMMANDS {
    ESP32_EVT_POLL = 0xA5,                    // Continously indicates connection and BT clock
    ESP32_EVT_STARTUP = 0xA6,                 // Used to indicate startup
    ESP32_EVT_HW_ERROR = 0xA4,                // Used to indicate HW error
    ESP32_CMD_DATA_RX = 0xA7,                 // Used to receive or send packets in any radio channel
    ESP32_CMD_DATA_TX = 0xBB,                 // Used to receive packets that were automatically transmitted,
    ESP32_CMD_DATA_LMP = 0xBC,                // Used to send arbitrary ACL Packets
    ESP32_CMD_FIFO_FULL = 0xA1,               // Indicates when tranmission data FIFO is full
    ESP32_CMD_CHECKSUM_ERROR = 0xA8,          // Indicates serial reception error.
    ESP32_CMD_CHECKSUM_OK = 0xA9,             // Indicates serial reception success.
    ESP32_CMD_LOG = 0x7F,                     // Print logs through python driver library
    ESP32_CMD_VERSION = 0xEE,                 // Returns the firmware version
    ESP32_CMD_PING = 0x80,                    // Used to test latency between PC and ESP32
    ESP32_CMD_ENABLE_INTERCEPT_TX = 0xEF,     // Enable ACL TX interception (for fuzzing)
    ESP32_CMD_ENABLE_LMP_SNIFFING = 0x81,     // Enable LMP RX/TX Sniffing
    ESP32_CMD_ENABLE_RX_BYPASS = 0x82,        // Enable LMP RX Bypass (Ignore received packets)
    ESP32_CMD_ENABLE_BYPASS_ON_DEMAND = 0x83, // Enable LMP Bypass when receiving response from CMD_DATA_LMP
    ESP32_CMD_DEFAULT_SETTINGS = 0x84,        // Reset Default Settings
    ESP32_CMD_DISABLE_ROLE_SWITCH = 0x85,     // Disable Switch Role to force original role of connection
    ESP32_CMD_RESET = 0x86,                   // Reset ESP32
    ESP32_CMD_SET_MAC = 0x87,                 // Retry last packet
    ESP32_CMD_HEARTBEAT = 0x88,               // Indicate Host presence (perform software reset if heartbeat timesout)
    ESP32_CMD_DISABLE_POLL_NULL = 0x89,
    ESP32_SPI_SHORT = 0xAA, // Indicate fixed 32 Byte SPI payload
    ESP32_SPI_LONG = 0x55   // Indicate long SPI payload (variable SPI transfer size)
};

// Internal Buffers
uint8_t buffer_uart_rx[2048];
uint8_t buffer_lmp_interception[2048]; // Buffer used to store intercepted LMP packets
uint8_t buffer_uart_tx[2048];
uint8_t default_bt_address[6] = {0x24, 0x0A, 0xC4, 0x9A, 0x58, 0x20}; // 24:0A:C4:9A:58:20

#if ENABLE_EXPERIMENTAL_SPI
WORD_ALIGNED_ATTR char spi_sendbuf[2048];
WORD_ALIGNED_ATTR char spi_recvbuf[2048];
#endif
// Spinlocks
volatile uint8_t spinlock_uart_busy = 0;
volatile uint8_t spinlock_tx_intercept_lmp = 0;
volatile uint8_t spinlock_tx_intercept_data = 0;

// Packet Queues
volatile bt_packets_t queue_hci_rx_packets = {0};
volatile bt_packets_t queue_hci_tx_packets = {0};
volatile bt_packets_t queue_acl_rx_packets = {0};
volatile bt_packets_t queue_acl_tx_packets = {0};
volatile bt_packets_t queue_dup_tx_packets = {0};
#if ENABLE_EXPERIMENTAL_SPI
volatile bt_packets_dma_t queue_spi_tx = {0};
volatile bt_packets_dma_t queue_spi_tx_low = {0};
#endif

// Flags
rx_isr_struct *saved_list_hdr = NULL;
uint8_t custom_lmp_sent = 0;
uint8_t tx_encrypted = 0;
uint8_t rx_encrypted = 0;
// Tasks
TaskHandle_t uart_tx_task_handle;
TaskHandle_t uart_rx_task_handle;
TaskHandle_t spi_hci_task_handle;

// ----------------------- UART Protocol Commands ---------------------------

void util_print_buffer(uint8_t *buf, uint16_t size)
{
    for (size_t i = 0; i < size; i++) {
        ets_printf("%02X", buf[i]);
    }
    ets_write_char_uart('\n');
}

void IRAM_ATTR uart_write_evt_poll()
{
    if (disable_poll_null)
        return;

    if (!spinlock_uart_busy) {
        spinlock_uart_busy = 1;
        uint32_t bt_clock = r_ld_read_clock();
        uart_tx_one_char(ESP32_EVT_POLL);
        uart_tx_one_char(bt_clock & 0xFF);
        uart_tx_one_char((bt_clock >> 8) & 0xFF);
        uart_tx_one_char((bt_clock >> 16) & 0xFF);
        uart_tx_one_char((bt_clock >> 24) & 0xFF);
        spinlock_uart_busy = 0;
    }
}

void IRAM_ATTR uart_write_acl_tx(bt_info_t bt_info, uint8_t *src_array, uint16_t src_len)
{

    uint16_t uart_pos = 0;
    uint16_t payload_pos = 0;
    uint8_t checksum = 0;

    while (spinlock_uart_busy)
        portYIELD();
    spinlock_uart_busy = 1;

    if (bt_info.direction == DIRECTION_TX)
        buffer_uart_tx[uart_pos++] = (bt_info.custom_lmp == 1 ? ESP32_CMD_DATA_LMP : ESP32_CMD_DATA_TX);
    else
        buffer_uart_tx[uart_pos++] = ESP32_CMD_DATA_RX;

    uart_pos += 2; // Reserve for size at index 1
    uint8_t *payload = buffer_uart_tx + uart_pos;
    // Meta Data
    *(uint32_t *)&payload[payload_pos] = bt_info.bt_clock;
    payload_pos += 4;
    payload[payload_pos] = bt_info.channel;
    payload_pos += 1;
    payload[payload_pos] = bt_info.is_eir << 7 | bt_info.rx_encrypted << 6 | bt_info.tx_encrypted << 5 | bt_info.intercept_req << 4 | bt_info.retry_flag << 3 | bt_info.custom_lmp << 2 | bt_info.role << 1 | bt_info.ptt;
    payload_pos += 1;
    // BT Header
    if (!bt_info.is_fhs && src_len) {
        // ACL Packet
        *(uint32_t *)&payload[payload_pos] = bt_info.bt_header;
        payload_pos += 4;
    }
    else {
        // NULL / POLL / FHS Packet
        *(uint16_t *)&payload[payload_pos] = bt_info.bt_header;
        payload_pos += 2;
    }
    // Checksum for metadata & BT Header
    for (uint16_t i = 0; i < payload_pos; i++) {
        checksum += payload[i];
    }
    // ACL Payload
    for (uint16_t i = 0; i < src_len; i++) {
        checksum += payload[payload_pos] = src_array[i];
        payload_pos++;
    }
    payload[payload_pos] = checksum;

    // Update size
    buffer_uart_tx[1] = payload_pos & 0xFF;
    buffer_uart_tx[2] = payload_pos >> 8;
    // Write to serial using ets rom function
    int final_size = uart_pos + payload_pos + 1;

    for (size_t i = 0; i < final_size; i++) {
        uart_tx_one_char(buffer_uart_tx[i]);
    }
    spinlock_uart_busy = 0;

    // uart_write_bytes does not work on interrupts due to IRAM requirement
    // uart_write_bytes(UART_NUM_0, buffer_uart_tx, uart_pos + payload_len + 1);
}

uint IRAM_ATTR dma_write_acl_tx(bt_info_t bt_info, uint8_t *dst_array, uint8_t *src_array, uint16_t src_len)
{
    uint16_t hdr_pos = 0;
    uint16_t payload_pos = 0;

    if (bt_info.direction == DIRECTION_TX)
        dst_array[hdr_pos++] = (bt_info.custom_lmp == 1 ? ESP32_CMD_DATA_LMP : ESP32_CMD_DATA_TX);
    else
        dst_array[hdr_pos++] = ESP32_CMD_DATA_RX;

    hdr_pos += 2; // Reserve for size at index 1
    uint8_t *payload = dst_array + hdr_pos;
    // Meta Data
    *(uint32_t *)&payload[payload_pos] = bt_info.bt_clock;
    payload_pos += 4;
    payload[payload_pos] = bt_info.channel;
    payload_pos += 1;
    payload[payload_pos] = bt_info.rx_encrypted << 6 | bt_info.tx_encrypted << 5 | bt_info.intercept_req << 4 | bt_info.retry_flag << 3 | bt_info.custom_lmp << 2 | bt_info.role << 1 | bt_info.ptt;
    payload_pos += 1;
    // BT Header
    if (!bt_info.is_fhs && src_len) {
        // ACL Packet
        *(uint32_t *)&payload[payload_pos] = bt_info.bt_header;
        payload_pos += 4;
    }
    else {
        // NULL / POLL / FHS Packet
        *(uint16_t *)&payload[payload_pos] = bt_info.bt_header;
        payload_pos += 2;
    }

    // ACL Payload
    memcpy(payload + payload_pos, src_array, src_len);
    payload_pos += src_len;

    // Update size
    *(uint16_t *)&dst_array[1] = payload_pos;

    return hdr_pos + payload_pos;
}

// ----------------------- Hooks ---------------------------

uint32_t hook_tx_acl_control(uint16_t offset, lmp_packet_t *lmp_packet, uint32_t interrupt_ctx, uint8_t custom_lmp, uint8_t retry_flag, uint param_1)
{
    // Check if LMP sniffing is enabled and RX Bypass is disabled
    if (!enable_lmp_sniffing)
        return;

    if (lmp_packet) {
        if (custom_lmp && !retry_flag)
            custom_lmp_sent = 1;
        // Extract ACL Header
        acl_header_t hdr = {
            .raw_header = ACL_HEADER_BASE(offset),
        };
        // Extract ACL PDU
        uint8_t *pkt_buffer = (uint8_t *)(0x3FFB0000 + lmp_packet->buf_idx);

        // uint8_t opcode = ((*pkt_buffer >> 1) & 0x7F);

        if (disable_role_switch) {
            uint8_t opcode = ((*pkt_buffer >> 1) & 0x7F);
            if (opcode == 39) {
                // Modify LMP_features_req to disable role switching
                pkt_buffer[1] = pkt_buffer[1] & 0b11011111;
                // pkt_buffer[1] = pkt_buffer[1] | 0b00100000;
            }
        }

        // if (opcode == 39)
        // {
        //     // ets_printf("foii\n");
        //     // *((uint16_t *)0x3ffb9508) = 1;
        //     r_ke_state_set(1, 0x17);
        //     // *((uint16_t *)0x3ffb9508) = 2;
        // }

        // Fill bt_info
        bt_info_t bt_info = {0};
        bt_info.direction = DIRECTION_TX;
        bt_info.bt_header = BT_HEADER_BASE(offset); // BT Packet Header + ACL Header
        bt_info.bt_clock = r_ld_read_clock();
        bt_info.custom_lmp = custom_lmp;
        bt_info.retry_flag = retry_flag;
        bt_info.ptt = GET_PTT_ACL(param_1);
        bt_info.role = r_ld_acl_role_get(param_1);
        bt_info.intercept_req = 1;
        bt_info.tx_encrypted = GET_TX_ENCRYPTED(param_1);
        bt_info.rx_encrypted = GET_RX_ENCRYPTED(param_1);
        // uint32_t time_start = micros();

        pkt_queue_push(&queue_acl_tx_packets, pkt_buffer, hdr.fields.length, &bt_info);
        xTaskNotify(uart_tx_task_handle, 0, eIncrement);
        // portYIELD_FROM_ISR();

        if (enable_intercept_tx && bt_info.intercept_req && !custom_lmp) {
            // Cancel packet
            BT_HEADER_BASE(offset) = 0;

            while (spinlock_tx_intercept_lmp)
                ;
            spinlock_tx_intercept_lmp = 1;
            while (spinlock_tx_intercept_lmp)
                ;
            BT_HEADER_BASE(offset) = *(uint32_t *)&buffer_lmp_interception[0];
            if (lmp_packet->length > 17)
                lmp_packet->length = 17;
            memcpy(pkt_buffer, buffer_lmp_interception + 4, lmp_packet->length);
            // if (((bt_info.bt_header >> 3) & 0b1111) != 3)
            // {
            //     // lmp_packet_t *jj = (lmp_packet_t *)r_bt_util_buf_acl_tx_alloc();
            //     // jj->length = lmp_packet->length;
            //     *(ushort *)(0x3ffb23c2 + offset) = jj->buf_idx;
            //     *(ushort *)(0x3ffb23c0 + offset) = jj->buf_idx;
            // }
            // r_ld_acl_allowed_tx_packet_types_set(param_1, 0xFFFF);
            // int res;
            // int a = ld_acl_tx_packet_type_select(param_1, hdr.fields.length, &res);
            // ets_printf("%d,%d\n", a, res);

            // ets_printf("%d\n", *(ushort *)(0x3ffb23ba + offset));
            // ets_printf("%d\n", r_ke_state_get(1));
        }
    }
}

// LMP TX (Task)
void hook_tx_acl_data(uint16_t offset, uint16_t *len)
{

    // Check if LMP sniffing is enabled and RX Bypass is disabled
    if (!enable_lmp_sniffing)
        return;

    if (len) {
        // Extract ACL Header
        bt_acl_header_t *bt_acl_hdr = (bt_acl_header_t *)&BT_HEADER_BASE(offset);
        acl_header_t *acl_hdr = (acl_header_t *)&ACL_HEADER_BASE(offset);
        uint8_t *data_pkt = ACL_PDU_DATA_BASE(offset);
        uint16_t acl_len = acl_hdr->fields.length;
        register uint32_t *stack_addr asm("a1");
        uint8_t conn_index = stack_addr[0x10];
        // uint8_t conn_index = 1;

        // ets_printf("%04X\n", (*((uint16_t *)(0x3FFB23BC + offset))));
        // *(uint16_t *)(0x3FFB23BC + offset) = 0b1011011; // DH3

        // Fill bt_info
        bt_info_t bt_info = {0};
        bt_info.direction = DIRECTION_TX;
        bt_info.bt_header = bt_acl_hdr->raw_header; // BT Packet Header + ACL Header
        bt_info.bt_clock = r_ld_read_clock();
        bt_info.ptt = GET_PTT_ACL(conn_index);
        bt_info.role = r_ld_acl_role_get(conn_index);
        bt_info.intercept_req = 1;
        bt_info.tx_encrypted = GET_TX_ENCRYPTED(conn_index);
        bt_info.rx_encrypted = GET_RX_ENCRYPTED(conn_index);

        pkt_queue_push(&queue_acl_tx_packets, data_pkt, acl_len, &bt_info);
        xTaskNotify(uart_tx_task_handle, 0, eIncrement);
        portYIELD_FROM_ISR();
        if (enable_intercept_tx && bt_info.intercept_req) {
            // uart_tx_one_char('D');
            // uart_tx_one_char('\n');

            // hdr->fields.length = 0x01;
            while (spinlock_tx_intercept_lmp)
                // portYIELD_FROM_ISR();
                ;

            spinlock_tx_intercept_lmp = 1;
            while (spinlock_tx_intercept_lmp)
                // portYIELD_FROM_ISR();
                ;

            bt_acl_hdr->raw_header = *((uint32_t *)&buffer_lmp_interception[0]);
            memcpy(data_pkt, buffer_lmp_interception + 4, acl_len);
            // ets_printf("%d\n", *(ushort *)(0x3ffb23ba + offset));
        }
    }
}

// LMP TX (ISR)
void IRAM_ATTR patch_ld_acl_tx_lmp(XtExcFrame *frame)
{
    // a4 = &DAT_ram_3ffb23ba + iVar9
    register uint32_t *stack_addr asm("a1");
    uint8_t conn_index = ((stack_addr[0x10] >> 2) > 0 ? 1 : 0);

    bt_info_t bt_info = {0};
    bt_info.direction = DIRECTION_TX;                 // DIRECTION_TX
    bt_info.bt_header = *(uint32_t *)(frame->a4 + 2); // 0x3ffb23bc = BT Packet Header + ACL Header
    bt_info.bt_clock = r_ld_read_clock();
    bt_info.ptt = GET_PTT_ACL(conn_index);
    bt_info.fast_tx = 1;
    bt_info.role = r_ld_acl_role_get(conn_index);
    bt_info.intercept_req = 1;

    bt_info.tx_encrypted = GET_TX_ENCRYPTED(conn_index);
    bt_info.rx_encrypted = GET_RX_ENCRYPTED(conn_index);

    uint8_t *pkt_buffer = 0x3ffb0000 + *((uint16_t *)(frame->a4 + 8));
    uint16_t acl_length = (bt_info.bt_header >> 19) & 0x3FF;
    uint8_t bt_type = (bt_info.bt_header >> 3) & 0xF;
    // Disable interception for afh map (TODO: fix this)
    // if ((bt_type > 2 && ((pkt_buffer[0] >> 1) & 0x7F) == 60))
    //     bt_info.intercept_req = 0;

    pkt_queue_push(&queue_acl_tx_packets, pkt_buffer, acl_length, &bt_info);
    xTaskNotifyFromISR(uart_tx_task_handle, 0, eIncrement, NULL);
    portYIELD_FROM_ISR();

    if (enable_intercept_tx && bt_info.intercept_req) {

        while (spinlock_tx_intercept_lmp)
            // portYIELD_FROM_ISR();
            ;

        spinlock_tx_intercept_lmp = 1;
        while (spinlock_tx_intercept_lmp)
            // portYIELD_FROM_ISR();
            ;

        // Update header and LMP buffer
        // r_ld_acl_allowed_tx_packet_types_set(0, 0xFFFF);
        *(uint32_t *)(frame->a4 + 2) = *((uint32_t *)&buffer_lmp_interception[0]);
        memcpy(pkt_buffer, buffer_lmp_interception + 4, acl_length);
    }
}

void IRAM_ATTR patch_empty_tx_buf(XtExcFrame *frame)
{
    // Set TX idx to 0, somewhat tx descriptor is pointing to invalid idx
    frame->a4 = 0;
    frame->a7 = 0x3ffb23c2 + frame->a4;
    r_ke_state_set(1, 0);
    r_ld_acl_lmp_flush(0);
}

uint16_t *hook_ld_acl_frm_cbk(uint16_t *param_1, uint param_2, uint32_t param_3, uint param_4)
{

    // TX lmp
    rompatcher_add(0, 0x40030681, 0, &patch_ld_acl_tx_lmp, ROMPATCHER_LEVEL_DEFAULT);
    // 0x400309b6

    // Fix empty TX buf (ignore and return)
    // rompatcher_add(1, 0x400301ae, -0xFE, &patch_empty_tx_buf, ROMPATCHER_LEVEL_DEFAULT);
    // esp_intr_alloc(ETS_INTERNAL_INTR_SOURCE_OFF)
    uint16_t *ret = ld_acl_frm_cbk(param_1, param_2, param_3, param_4);

    // if (spinlock_tx_intercept_lmp)
    // {
    //     while (spinlock_tx_intercept_lmp)
    //         ;
    // }

    // POLL here
    // uint8_t puVar4 = (uint8_t *)(param_2 & 0xff);
    // int **ppiStack48 = (int **)((int)puVar4 * 4);
    // int iVar9 = *(int *)(ld_acl_env + (int)ppiStack48);
    // if (iVar9)
    // {
    //     ppiStack48 = (int **)((uint) * (uint8_t *)(iVar9 + 0xb2) * 2 + (uint) * (uint8_t *)(iVar9 + 0xc4) & 0xff);
    //     int iVar7 = (int)ppiStack48 * 10;
    //     // Extract ACL Header
    //     acl_header_t hdr = {
    //         .raw_header = ACL_HEADER_BASE(iVar7),
    //     };

    //     bt_info_t bt_info;
    //     bt_info.direction = 0;                     // DIRECTION_TX
    //     bt_info.bt_header = BT_HEADER_BASE(iVar7); // BT Packet Header + ACL Header
    //     bt_info.bt_clock = r_ld_read_clock();
    //     bt_info.ptt = GET_PTT_ACL(0);
    //     bt_info.custom_lmp = 0;
    //     bt_info.fast_tx = 1;
    //     bt_info.role = r_ld_acl_role_get(0);
    //     bt_info.is_fhs = 0;

    //     pkt_queue_push(&queue_acl_tx_packets, ACL_PDU_CONTROL_BASE(iVar7), hdr.fields.length, &bt_info);
    //     xTaskNotifyFromISR(uart_tx_task_handle, 0, eIncrement, NULL);
    //     portYIELD_FROM_ISR();

    //     if (hdr.fields.length && hdr.fields.llid == 0x03)
    //     {
    //     }
    // }

    return ret;
}

int *IRAM_ATTR hook_frame_isr(int param_1, rx_isr_struct *list_hdr)
{
    static uint8_t last_channel;

    acl_header_t acl_hdr;

    saved_list_hdr = list_hdr;

    if (enable_lmp_sniffing) {
        if (list_hdr->callback_fcn == lc_acl_frm_cbk) {
            list_hdr->callback_fcn = hook_ld_acl_frm_cbk;

            bt_info_t bt_info = {0};
            bt_info.channel = ACL_RX_RF_STATS_CHANNEL;

            if ((last_channel == bt_info.channel) || !enable_lmp_sniffing)
                goto r_end;

            uint8_t conn_index = list_hdr[1].p1 & 0xFF;

            last_channel = bt_info.channel;
            bt_info.direction = DIRECTION_RX;
            bt_info.bt_header = BT_FULL_HEADER; // BT Packet Header + ACL Header
            bt_info.bt_clock = r_ld_read_clock();
            bt_info.ptt = GET_PTT_ACL(conn_index);
            bt_info.role = r_ld_acl_role_get(conn_index);

            acl_hdr.raw_header = ACL_BUFFER_HEADER;

            if (acl_hdr.fields.length) {
                // ACL Packet
                // acl_hdr.raw_header = *(uint16_t *)(0x3FFB2388U + (ld_env[426] * 0xe));

                // uart_tx_one_char('O');
                // uart_tx_one_char('\n');

                bt_info.tx_encrypted = GET_TX_ENCRYPTED(conn_index);
                bt_info.rx_encrypted = GET_RX_ENCRYPTED(conn_index);

                switch (acl_hdr.fields.llid) {
                case 1:
                case 2:
                    // ets_printf("%d:%d\n", acl_hdr.fields.llid, acl_hdr.fields.length);
                    pkt_queue_push(&queue_acl_rx_packets, ACL_BUFFER_DATA_PAYLOAD, acl_hdr.fields.length, &bt_info);
                    xTaskNotifyFromISR(uart_tx_task_handle, 0, eIncrement, NULL);
                    break;
                case 3:
                    pkt_queue_push(&queue_acl_rx_packets, (acl_hdr.fields.length <= 17 ? ACL_BUFFER_LMP_PAYLOAD : ACL_BUFFER_DATA_PAYLOAD), acl_hdr.fields.length, &bt_info);
                    xTaskNotifyFromISR(uart_tx_task_handle, 0, eIncrement, NULL);
#if ENABLE_EXPERIMENTAL_SPI
                    else
                    {
                        pkt_queue_push_dma(&queue_spi_tx, ACL_BUFFER_LMP_PAYLOAD, acl_hdr.fields.length, &bt_info);
                    }
#endif
                    break;
                default:
                    break;
                }

                // portYIELD_FROM_ISR();

                // uart_tx_one_char('C');
                // uart_tx_one_char('\n');
            }
            else if (((bt_info.bt_header >> 3) & 0x0F) < 2) {
                // NULL / POLL BT Packet
                if (disable_poll_null)
                    goto r_end;
                bt_info.tx_encrypted = 0;
                bt_info.rx_encrypted = 0;
                pkt_queue_push(&queue_acl_rx_packets, NULL, 0, &bt_info);
                xTaskNotifyFromISR(uart_tx_task_handle, 0, eIncrement, NULL);
                portYIELD_FROM_ISR();
#if ENABLE_EXPERIMENTAL_SPI
                else
                {
                    pkt_queue_push_dma(&queue_spi_tx, NULL, 0, &bt_info);
                }
#endif
                // uart_write_evt_poll();
            }
        }
        else if (list_hdr->callback_fcn == ld_page_frm_cbk) {
            // uart_tx_one_char('P');
            // uart_tx_one_char('\n');
            // rompatcher_add(0, PATCH_ADDR_LD_PAGE_EM_INIT, 0, patch_ld_page_em_init, ROMPATCHER_LEVEL_DEFAULT);
            // list_hdr->callback_fcn = reversed_ld_page_frm_cbk;
            // reversed_ld_page_frm_cbk
            // Repeated FHS
            // hook_ld_page_em_init();
        }
    }
r_end:
    return r_co_list_push_back(param_1, list_hdr);
}

// Handler to execute or reject frame_isr callbacks
void *IRAM_ATTR hook_frame_isr_callback(int param_1, int param_2, int param_3, int param_4)
{
    // TODO: disable poll reception to play with retries
    // Process Radio TX / RX ISR
    ((code4 *)saved_list_hdr->callback_fcn)(param_1, param_2, param_3, param_4);
}

uint32_t hook_lc_lmp_rx_handler(uint32_t param_1, uint8_t *param_2, uint32_t param_3)
{

    // acl_header_t acl_hdr;
    // if ((enable_lmp_sniffing) && (acl_hdr.raw_header = ACL_BUFFER_HEADER))
    // {
    //     pkt_queue_push(ACL_BUFFER_LMP_PAYLOAD, acl_hdr.fields.length, &queue_acl_rx_packets, acl_hdr.raw_header);
    //     xTaskNotify(uart_tx_task_handle, 0, eIncrement);
    // }

    // ets_printf("R:%d\n", param_3);
    // uint8_t opcode = ((*param_2 >> 1) & 0x7F);
    if (disable_role_switch) {
        uint8_t opcode = ((*param_2 >> 1) & 0x7F);

        if (opcode == 19) {
            internal_custom_send_lmp(pkt_lmp_not_accepted_switch_req, 3, 0);
            return 0;
        }
    }

    if (enable_bypass_on_demand && custom_lmp_sent) {
        custom_lmp_sent = 0;
        return 0;
    }

    if (enable_rx_bypass)
        return 0;

    // if (opcode == 40)
    // {
    //     // ets_printf("foii\n");
    //     r_ke_state_set(1, 0x17);
    //     // *((uint16_t *)0x3ffb9508) = 2;
    // }

    uint res = lc_lmp_rx_handler(param_1, param_2, param_3);

    // if (opcode == 40)
    // {
    //     ets_printf("foii\n");
    //     r_ke_state_set(1, '\x02');
    //     *((uint16_t *)0x3ffb9508) = 2;
    // }

    return res;
}

uint32_t IRAM_ATTR *hook_r_ke_timer_set(ushort param_1, uint param_2, uint param_3)
{
    return r_ke_timer_set(param_1, param_2, param_3);
}

uint hook_r_ld_fm_prog_enable(uint32_t param_1, uint8_t param_2)
{
    return r_ld_fm_prog_enable(param_1, param_2);
}

ushort *hook_r_ld_fm_prog_push(uint param_1, uint param_2, char param_3, uint param_4, int callback)
{
    return r_ld_fm_prog_push(param_1, param_2, param_3, param_4, callback);
}

// Called when disconnection received (not timeout)
uint8_t *hook_r_lc_release(uint param_1)
{
    // uart_tx_one_char('D');
    // uart_tx_one_char('\n');
    return r_lc_release(param_1);
}

uint8_t *hook_r_lc_start_enc(uint param_1)
{
    // uart_tx_one_char('S');
    // uart_tx_one_char('\n');
    uint8_t ret = r_lc_start_enc(param_1);
    return ret;
}

ushort *hook_r_ld_acl_rx_enc(uint param_1, uint param_2)
{
    // uart_tx_one_char('R');
    // uart_tx_one_char('\n');

    ushort ret = r_ld_acl_rx_enc(param_1, param_2);
    // ets_printf("R:%d\n", GET_RX_ENCRYPTED(param_1));
    // ets_printf("%d\n", ret);
    return ret;
}

ushort *hook_r_ld_acl_tx_enc(uint param_1, uint param_2)
{
    // uart_tx_one_char('T');
    // uart_tx_one_char('\n');
    ushort ret = r_ld_acl_tx_enc(param_1, param_2);
    // ets_printf("T:%d\n", GET_TX_ENCRYPTED(param_1));
    // ets_printf("%d\n", ret);
    return ret;
}

// Enable Enhanced Data Rate
uint8_t *hook_r_ld_acl_edr_set(uint param_1, uint param_2)
{
    return r_ld_acl_edr_set(param_1, param_2);
}

// Intercept FHS packet generate during paging
void hook_ld_page_em_init(uint16_t offset)
{
    if (!enable_lmp_sniffing)
        return;

    // Get Paging offset
    // uint32_t offset = ((uint) * (uint8_t *)(ld_page_env + 0x41) & 0x7f) * 0x14;
    // Fill bt_info
    bt_info_t bt_info = {0};
    bt_info.direction = DIRECTION_TX;
    bt_info.bt_header = BT_HEADER_BASE(offset); // BT Packet Header + ACL Header
    bt_info.bt_clock = r_ld_read_clock();
    bt_info.custom_lmp = 0;
    bt_info.retry_flag = 0;
    bt_info.fast_tx = 0;
    bt_info.ptt = 0;
    bt_info.role = 0;
    bt_info.is_fhs = 1;
    // FHS has fixed size of 18 bytes
    // ACL_PDU_CONTROL_BASE(offset)
    // [1] = 0;
    // ACL_PDU_CONTROL_BASE(offset)
    // [2] = 0;
    pkt_queue_push(&queue_acl_tx_packets, ACL_PDU_CONTROL_BASE(offset), 18, &bt_info);
    xTaskNotify(uart_tx_task_handle, 0, eIncrement);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR handle_fhs(uint32_t dir, uint32_t *hdr_addr, uint8_t *buf_addr)
{
    if (!enable_lmp_sniffing)
        return;

    bt_info_t bt_info = {0};
    bt_info.direction = dir;
    bt_info.bt_header = *hdr_addr; // BT Packet Header + ACL Header
    bt_info.bt_clock = r_ld_read_clock();
    bt_info.ptt = GET_PTT_ACL(0);
    bt_info.is_fhs = 1;

    // FHS has fixed size of 18 bytes
    // ACL_PDU_CONTROL_BASE(offset)
    // [1] = 0;
    // ACL_PDU_CONTROL_BASE(offset)
    // [2] = 0;
    // uint32_t ret_addr;
    // READ_RETURN_ADDRESS(ret_addr);

    pkt_queue_push(&queue_acl_tx_packets, buf_addr, 18, &bt_info);
    xTaskNotify(uart_tx_task_handle, 0, eIncrement);
    // portYIELD_FROM_ISR();
}

// FHS RX
void hook_r_ld_util_fhs_unpk(uint param_1, uint8_t *param_2, uint8_t *param_3, uint8_t *param_4, uint8_t *param_5, uint *param_6, uint8_t *param_7)
{
    // uart_tx_one_char('U');
    // uart_tx_one_char('\n');
    uint ret = r_ld_util_fhs_unpk(param_1, param_2, param_3, param_4, param_5, param_6, param_7);
    if (!enable_lmp_sniffing)
        return ret;

    bt_info_t bt_info = {0};
    bt_info.direction = DIRECTION_RX;
    bt_info.bt_header = 0x00920390; // BT Packet Header + ACL Header
    bt_info.bt_clock = r_ld_read_clock();
    bt_info.channel = ACL_RX_RF_STATS_CHANNEL;
    bt_info.is_fhs = 1;
    // FHS has fixed size of 18 bytes
    // ACL_PDU_CONTROL_BASE(offset)
    // [1] = 0;
    // ACL_PDU_CONTROL_BASE(offset)
    // [2] = 0;
    pkt_queue_push(&queue_acl_rx_packets, (uint8_t *)(0x3ffb0000 + param_1), 18, &bt_info);
    xTaskNotify(uart_tx_task_handle, 0, eIncrement);
    portYIELD_FROM_ISR();
    return ret;
}

void IRAM_ATTR handle_eir(uint32_t dir, uint32_t *hdr_addr, uint8_t *buf_addr)
{
    if (!enable_lmp_sniffing)
        return;

    bt_info_t bt_info = {0};
    acl_header_t acl_hdr;
    bt_info.channel = ACL_RX_RF_STATS_CHANNEL;
    bt_info.direction = dir;
    bt_info.bt_header = *hdr_addr; // BT Packet Header + ACL Header
    bt_info.bt_clock = r_ld_read_clock();
    bt_info.ptt = GET_PTT_ACL(0);
    bt_info.role = r_ld_acl_role_get(0);
    bt_info.is_eir = 1;

    acl_hdr.raw_header = (uint16_t)(bt_info.bt_header >> 16);

    pkt_queue_push(&queue_acl_rx_packets, buf_addr, acl_hdr.fields.length, &bt_info);
    xTaskNotifyFromISR(uart_tx_task_handle, 0, eIncrement, NULL);
}

void IRAM_ATTR patch_ld_page_start(XtExcFrame *frame)
{
    handle_fhs(DIRECTION_TX, 0x3ffb23bc, 0x3ffb262c);
}

void IRAM_ATTR patch_ld_iscan_start(XtExcFrame *frame)
{

    handle_fhs(DIRECTION_TX, 0x3ffb2452, 0x3ffb2618);

    if (frame->a6 != 0) {
        // EIR is set,
        handle_eir(DIRECTION_TX, 0x3ffb245c, 0x3ffb2640);
    }
}

void IRAM_ATTR patch_ld_acl_rsw_start(XtExcFrame *frame)
{
    handle_fhs(DIRECTION_TX, frame->a3 - 6, frame->a3);
}

// FHS RX
uint IRAM_ATTR hook_r_ld_util_fhs_pk(uint param_1, uint8_t *param_2, uint param_3, uint param_4, uint8_t *param_5, uint8_t *param_6, uint8_t param_7, uint8_t param_8)
{
    uint ret = r_ld_util_fhs_pk(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8);
    register uint32_t ret_addr asm("a0");
    ret_addr -= 0x40000000;

    if (ret_addr == 0x4003BC14) {
        // called from r_ld_iscan_start
        // patch enquiry response + eir (if set)
        rompatcher_add(0, 0x4003bce5, 0, patch_ld_iscan_start, ROMPATCHER_LEVEL_DEFAULT);
    }
    else if (ret_addr == 0x4003D975) {
        // called from r_ld_page_start
        // patch r_ld_page_em_init
        rompatcher_add(0, 0x4003cdfa, 0, patch_ld_page_start, ROMPATCHER_LEVEL_DEFAULT);
    }
    else if (ret_addr == 0x40035f28) {
        rompatcher_add(1, 0x40033120, 0, patch_ld_acl_rsw_start, ROMPATCHER_LEVEL_DEFAULT);
        // rompatcher_add(1, 0x40032e9b, 0, patch_ld_acl_rsw_start, ROMPATCHER_LEVEL_DEFAULT);
    }
    // else if (ret_addr == 0x40035f28)
    // {
    //     //     // //     // called from r_ld_acl_rsw_req
    //     //     // // }
    //     //     // else
    //     //     // {
    //     bt_info_t bt_info = {0};
    //     bt_info.direction = DIRECTION_TX;
    //     bt_info.bt_header = 0x00920390; // BT Packet Header + ACL Header
    //     bt_info.bt_clock = r_ld_read_clock();
    //     bt_info.ptt = GET_PTT_ACL(0);
    //     bt_info.role = r_ld_acl_role_get(0);
    //     bt_info.is_fhs = 1;

    //     // FHS has fixed size of 18 bytes
    //     // ACL_PDU_CONTROL_BASE(offset)
    //     // [1] = 0;
    //     // ACL_PDU_CONTROL_BASE(offset)
    //     // [2] = 0;
    //     // uint32_t ret_addr;
    //     // READ_RETURN_ADDRESS(ret_addr);

    //     pkt_queue_push(&queue_acl_tx_packets, (uint8_t *)(0x3ffb0000 + param_1), 18, &bt_info);
    //     xTaskNotify(uart_tx_task_handle, 0, eIncrement);
    //     portYIELD_FROM_ISR();
    // }

    return ret;
}

uint32_t *hook_r_ld_acl_allowed_tx_packet_types_set(uint param1, uint param2)
{
    // ets_printf("%d\n", param1);
    return r_ld_acl_allowed_tx_packet_types_set(param1, param2);
}

IRAM_ATTR uint hook_r_ke_state_set(uint param1, char param_2)
{
    // static uint flag = 0;
    // ets_printf("%d\n", param_2);
    // if (param_2 == 24)
    //     flag = 1;
    // if (flag && param_2 == 2)
    // {
    //     flag = 0;
    //     ets_printf("B\n");
    // }
    // ets_printf("%d\n", *((uint16_t *)0x3ffb9508));
    // *((uint16_t *)0x3ffb9508) = 2;
    // if (param_2 == 24)
    // {
    //     ets_printf("B\n");
    // }
    uint res = r_ke_state_set(param1, param_2);
    return res;
}

IRAM_ATTR uint hook_r_ke_state_get(uint param1)
{
    static uint flag = 0;

    // uint res = r_ke_state_get(param1);
    // ets_printf("%d\n", res);
    // if (res == 24)
    //     flag++;
    // if (flag > 9 && res == 24)
    // {

    //     flag = 0;
    //     ets_printf("B\n");
    // }
    // if (res == 24)
    // {
    //     ets_printf("B\n");
    // }
    // return res;
}

void bt_apply_rom_hooks()
{
    // Hook r_ld_acl_lmp_tx

    UPDATE_ADDRESS_HOOK(r_ld_acl_lmp_tx_ptr, reversed_r_ld_acl_lmp_tx);
    // UPDATE_ADDRESS_HOOK(r_ld_acl_lmp_tx_ptr, r_ld_acl_data_tx);
    // UPDATE_ADDRESS_HOOK(r_bt_util_buf_lmp_tx_alloc_ptr, r_bt_util_buf_acl_tx_alloc);
    // UPDATE_ADDRESS_HOOK(r_bt_util_buf_lmp_tx_free_ptr, r_bt_util_buf_acl_tx_free);
    // Hook r_co_list_push_back
    UPDATE_ADDRESS_HOOK(r_co_list_push_back_ptr, reversed_r_co_list_push_back);
    // Hook r_co_slot_to_duration
    UPDATE_ADDRESS_HOOK(r_co_slot_to_duration_ptr, reversed_r_co_slot_to_duration);
    // Hook r_ke_timer_set
    UPDATE_ADDRESS_HOOK(r_ke_timer_set_ptr, hook_r_ke_timer_set);
    // Hook r_ld_fm_prog_enable
    UPDATE_ADDRESS_HOOK(r_ld_fm_prog_enable_ptr, hook_r_ld_fm_prog_enable);
    // Hook r_ld_fm_prog_push
    UPDATE_ADDRESS_HOOK(r_ld_fm_prog_push_ptr, hook_r_ld_fm_prog_push);
    // Hook r_lc_release
    UPDATE_ADDRESS_HOOK(r_lc_release_ptr, hook_r_lc_release);
    // Hook r_lc_start_enc
    UPDATE_ADDRESS_HOOK(r_lc_start_enc_ptr, hook_r_lc_start_enc);
    // Hook r_ld_acl_rx_enc
    UPDATE_ADDRESS_HOOK(r_ld_acl_rx_enc_ptr, hook_r_ld_acl_rx_enc);
    // Hook r_ld_acl_tx_enc
    UPDATE_ADDRESS_HOOK(r_ld_acl_tx_enc_ptr, hook_r_ld_acl_tx_enc);
    // Hook r_ld_acl_edr_set
    UPDATE_ADDRESS_HOOK(r_ld_acl_edr_set_ptr, hook_r_ld_acl_edr_set);
    // Hook r_ld_util_fhs_pk (Paging TX FHS Packet)
    UPDATE_ADDRESS_HOOK(r_ld_util_fhs_pk_ptr, hook_r_ld_util_fhs_pk);
    // Hook r_ld_util_fhs_pk (Paging RX FHS Packet)
    UPDATE_ADDRESS_HOOK(r_ld_util_fhs_unpk_ptr, hook_r_ld_util_fhs_unpk);

    // UPDATE_ADDRESS_HOOK(r_ke_state_set_ptr, hook_r_ke_state_set);
    // UPDATE_ADDRESS_HOOK(r_ke_state_get_ptr, hook_r_ke_state_get);

    // UPDATE_ADDRESS_HOOK(r_ld_acl_allowed_tx_packet_types_set_ptr, hook_r_ld_acl_allowed_tx_packet_types_set);
    // UPDATE_ADDRESS_HOOK(r_lc_send_lmp_ptr, r_ld_acl_data_tx);
}

// ----------------------- UART Tasks ---------------------------

static void IRAM_ATTR uart_tx_task(void *pvParameters)
{
    while (true) {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

        // Send ACL TX Packets
        while (queue_acl_tx_packets.idx) {
            uart_write_acl_tx(queue_acl_tx_packets.bt_info[0], queue_acl_tx_packets.pkt_buffer[0], queue_acl_tx_packets.size[0]);
            pkt_queue_pop(&queue_acl_tx_packets);
        }
        // Send ACL RX Packets
        if (queue_acl_rx_packets.idx) {
            // Spinlock if waiting for interception
            while (spinlock_tx_intercept_lmp)
                portYIELD();

            uart_write_acl_tx(queue_acl_rx_packets.bt_info[0], queue_acl_rx_packets.pkt_buffer[0], queue_acl_rx_packets.size[0]);
            pkt_queue_pop(&queue_acl_rx_packets);
        }
        // Send HCI TX Packets
        if (queue_hci_tx_packets.idx) {
            // Spinlock if waiting for interception
            while (spinlock_tx_intercept_lmp)
                portYIELD();
            while (spinlock_uart_busy)
                portYIELD();
            spinlock_uart_busy = 1;
            for (size_t i = 0; i < queue_hci_tx_packets.size[0]; i++) {
                uart_tx_one_char(queue_hci_tx_packets.pkt_buffer[0][i]);
            }
            spinlock_uart_busy = 0;
            pkt_queue_pop(&queue_hci_tx_packets);
        }
    }

    vTaskDelete(NULL);
}

static void uart_hci_task()
{
    while (1) {
        // Process HCI to Controller event here
        delay_ms(1);
        while (queue_hci_rx_packets.idx && !spinlock_tx_intercept_lmp && esp_vhci_host_check_send_available()) {
            esp_vhci_host_send_packet(queue_hci_rx_packets.pkt_buffer[0], queue_hci_rx_packets.size[0]);
            pkt_queue_pop(&queue_hci_rx_packets);
        }
    }

    vTaskDelete(NULL);
}

static void IRAM_ATTR uart_dup_lmp_task()
{
    while (1) {
        // Process LMP injection messages here
        if (queue_dup_tx_packets.idx && !spinlock_tx_intercept_lmp) {
            if (custom_send_lmp(queue_dup_tx_packets.pkt_buffer[0], queue_dup_tx_packets.size[0], 0, *(uint8_t *)&queue_dup_tx_packets.bt_info)) {
                // Packet was confirmed to be sent
                pkt_queue_pop(&queue_dup_tx_packets);
            };
        }
        delay_ms(1);
    }

    vTaskDelete(NULL);
}

static void IRAM_ATTR uart_rx_task(void *pvParameters)
{
    static uint8_t r_buff[2048];
    uint16_t r_size;
    uint16_t data_len;
    uint16_t cmd_len;
    uint8_t checksum;
    int len;
    int pkt_counter = 0;
    uint8_t cmd;
    uint8_t ping_n;

    // Stat HCI RX Task
    xTaskCreatePinnedToCore(uart_hci_task, "uart_rx_hci", 1024, NULL, 1, NULL, 0);
    // Stat LMP DUP Task
    xTaskCreatePinnedToCore(uart_dup_lmp_task, "uart_dup_lmp", 1024, NULL, 1, NULL, 0);

    // H4 handling using uart timeout and inline conditions
    while (true) {

        // Handle UART Commands
        if (len = uart_read_bytes(UART_NUM_0, &r_buff[0], 1, UART_TIMEOUT_MS)) {

            r_size = 1;
            // HCI first byte (type)
            switch (r_buff[0]) {
            case H4_CMD:
                if ((len = uart_read_bytes(UART_NUM_0, &r_buff[1], 3, UART_TIMEOUT_MS)) == 3) {
                    // Receive 3 bytes (opcode (2) + length (1))
                    r_size += 3;
                    uint16_t h4_cmd_size = r_buff[3];
                    if (!h4_cmd_size || ((len = uart_read_bytes(UART_NUM_0, &r_buff[4], h4_cmd_size, UART_TIMEOUT_MS)) == h4_cmd_size)) {
                        r_size += h4_cmd_size;

                        // if (spinlock_tx_intercept_lmp)
                        // {
                        //     ets_printf("BUG HERE 1\n");
                        // }
                        uint16_t cmd_opcode = *((uint16_t *)&r_buff[1]);
                        if (cmd_opcode == 0x0C03) {
                            // HCI Reset
                            // ets_printf("%d,%d,%d\n", spinlock_tx_intercept_lmp, queue_hci_rx_packets.idx, esp_vhci_host_check_send_available());
                            // enable_bypass_on_demand = 0;
                            // esp_restart();

                            spinlock_tx_intercept_lmp = 0;
                            pkt_queue_clear(&queue_dup_tx_packets);
                            pkt_queue_clear(&queue_hci_rx_packets);
                            // r_ke_state_set(1, KE_STATE_DETACH_TX);
                            r_ld_acl_lmp_flush(0);
                            r_ke_state_set(1, 0);
                        }

                        // puts("HCI OK");
                        pkt_queue_push(&queue_hci_rx_packets, r_buff, r_size, 0);
                    }
                }
                break;
            case H4_ACL:

                if (uart_read_bytes(UART_NUM_0, &r_buff[1], 4, UART_TIMEOUT_MS) == 4) {
                    // Receive 4 bytes (opcode (2) + length (2))
                    r_size += 4;
                    uint16_t h4_acl_size = *((uint16_t *)(r_buff + 3));
                    if (!h4_acl_size || (uart_read_bytes(UART_NUM_0, &r_buff[5], h4_acl_size, UART_TIMEOUT_MS) == h4_acl_size)) {
                        r_size += h4_acl_size;
                        pkt_queue_push(&queue_hci_rx_packets, r_buff, r_size, 0);
                    }
                }
                break;

            case H4_EVT:
                if (uart_read_bytes(UART_NUM_0, &r_buff[1], 2, UART_TIMEOUT_MS) == 2) {
                    // Receive 2 bytes (event code (1) + length (1))
                    r_size += 2;
                    uint16_t h4_evt_size = r_buff[2];
                    if (!h4_evt_size || (uart_read_bytes(UART_NUM_0, &r_buff[3], h4_evt_size, UART_TIMEOUT_MS) == h4_evt_size)) {
                        r_size += h4_evt_size;
                        pkt_queue_push(&queue_hci_rx_packets, r_buff, r_size, 0);
                    }
                }
                break;

            case ESP32_CMD_DATA_LMP:
                if ((len = uart_read_bytes(UART_NUM_0, &cmd_len, 2, UART_TIMEOUT_MS)) && cmd_len < 2000) {
                    if (uart_read_bytes(UART_NUM_0, buffer_uart_rx, cmd_len + 1, UART_TIMEOUT_MS) == (cmd_len + 1)) {
                        // Calculate checksum
                        checksum = 0;
                        for (int16_t i = 0; i < cmd_len; i++) {
                            checksum += buffer_uart_rx[i];
                        }
                        // printf("checksum: %02X, received:%02X\n", checksum, buffer_uart_rx[cmd_len]);
                        // Verify received checksum against calculated checksum
                        if (checksum == buffer_uart_rx[cmd_len]) {
                            if (custom_send_lmp(buffer_uart_rx + 1, cmd_len - 1, 0, buffer_uart_rx[0]) == 0) {
                                // Could not send now, queue packet
                                pkt_queue_push(&queue_dup_tx_packets, buffer_uart_rx + 1, cmd_len - 1, (bt_info_t *)buffer_uart_rx);
                            };
                        }
                        else {
                            // printf("CHK Error: %d!=%d\n", checksum, buffer_uart_rx[cmd_len]);
                            // puts("CHK Error");
                            // If a error is received, send a checksum error so the host can retransmit the packet;
                            uart_tx_one_char(ESP32_CMD_CHECKSUM_ERROR);
                        }
                    }
                }
                break;

            case ESP32_CMD_DATA_TX:
                // puts("ESP32_CMD_DATA_TX");
                if ((len = uart_read_bytes(UART_NUM_0, &cmd_len, 2, UART_TIMEOUT_MS)) && cmd_len < 2000) {
                    // Data + Checksum (1B)
                    if (len = uart_read_bytes(UART_NUM_0, buffer_uart_rx, cmd_len + 1, UART_TIMEOUT_MS)) {
                        // Calculate checksum
                        checksum = 0;
                        for (int16_t i = 0; i < cmd_len; i++) {
                            checksum += buffer_uart_rx[i];
                        }
                        // Verify received checksum against calculated checksum
                        if (checksum == buffer_uart_rx[cmd_len]) {
                            if (enable_intercept_tx) {

                                // puts("CHK OK");
                                memcpy(buffer_lmp_interception, buffer_uart_rx, cmd_len);

                                // if ((buffer_lmp_interception[2] & 0x03) == 0x03)
                                // {
                                //     uart_tx_one_char('R');
                                //     uart_tx_one_char('\n');
                                // }
                                // puts("S\n");
                                spinlock_tx_intercept_lmp = 0;
                            }
                        }
                        else {
                            // puts("CHK Error");
                            // If a error is received, send a checksum error so the host can retransmit the packet;
                            uart_tx_one_char(ESP32_CMD_CHECKSUM_ERROR);
                        }
                    }
                }
                break;
            case ESP32_CMD_VERSION:
                uart_write_bytes(UART_NUM_0, VERSION_STRING "\n", sizeof(VERSION_STRING));
                break;

            case ESP32_CMD_PING:
                uart_tx_one_char(ESP32_CMD_PING + 1);
                break;

            case ESP32_CMD_ENABLE_INTERCEPT_TX:
                if (uart_read_bytes(UART_NUM_0, &enable_intercept_tx, 1, UART_TIMEOUT_MS) == 1)
                    uart_tx_one_char(enable_intercept_tx);
                break;

            case ESP32_CMD_ENABLE_LMP_SNIFFING:
                if (uart_read_bytes(UART_NUM_0, &enable_lmp_sniffing, 1, UART_TIMEOUT_MS) == 1)
                    uart_tx_one_char(enable_lmp_sniffing);
                break;

            case ESP32_CMD_ENABLE_RX_BYPASS:
                if (uart_read_bytes(UART_NUM_0, &enable_rx_bypass, 1, UART_TIMEOUT_MS) == 1)
                    uart_tx_one_char(enable_rx_bypass);
                break;

            case ESP32_CMD_ENABLE_BYPASS_ON_DEMAND:
                if (uart_read_bytes(UART_NUM_0, &enable_bypass_on_demand, 1, UART_TIMEOUT_MS) == 1)
                    uart_tx_one_char(enable_bypass_on_demand);
                break;

            case ESP32_CMD_DISABLE_ROLE_SWITCH:
                if (uart_read_bytes(UART_NUM_0, &disable_role_switch, 1, UART_TIMEOUT_MS) == 1)
                    uart_tx_one_char(disable_role_switch);
                break;

            case ESP32_CMD_DISABLE_POLL_NULL:
                if (uart_read_bytes(UART_NUM_0, &disable_poll_null, 1, UART_TIMEOUT_MS) == 1)
                    uart_tx_one_char(disable_poll_null);
                break;

            case ESP32_CMD_RESET: {
                // Reset upon receiving [0x86, 0xAA] bytes
                uint8_t reset_confirm;
                if (uart_read_bytes(UART_NUM_0, &reset_confirm, 1, UART_TIMEOUT_MS) == 1) {
                    if (reset_confirm == 0xAA)
                        esp_restart();
                }
            } break;
            case ESP32_CMD_SET_MAC: {
                // base BT address
                if (uart_read_bytes(UART_NUM_0, &default_bt_address, 6, UART_TIMEOUT_MS) == 6) {
                    memcpy(co_default_bdaddr, default_bt_address, 6); // Set host BDADDRESS
                }
            } break;

            case ESP32_CMD_DEFAULT_SETTINGS:
                enable_intercept_tx = 0;
                enable_lmp_sniffing = 0;
                enable_rx_bypass = 0;
                enable_bypass_on_demand = 0;
                disable_role_switch = 0;
                break;

            default:
                break;
            }
        }
    }

    vTaskDelete(NULL);
}

// ----------------------- VHCI Callbacks ---------------------------

static void vhci_to_uart_notify(void)
{
    asm("nop");
}

static int vhci_to_uart(uint8_t *data, uint16_t len)
{
    if (data[0] == H4_EVT) {
        if (data[1] == H4_EVT_HW_ERROR) {
            pkt_queue_push(&queue_hci_rx_packets, pkt_hci_evt_reset, sizeof(pkt_hci_evt_reset), 0);
        }
    }
    // Queue HCI packet and notify UART TX Task
    pkt_queue_push(&queue_hci_tx_packets, data, len, 0);
    xTaskNotify(uart_tx_task_handle, 0, eIncrement);
    return 0;
}

// ----------------------- Setup Functions ---------------------------

void setup_uart0()
{
    static const char *TAG = "uart_events";
    esp_log_level_set(TAG, ESP_LOG_INFO);
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // uart_intr_config_t uc = {

    // }
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_0, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    // Configure UART Interrupt latency
    // uart_intr_config_t uart_intr = {
    //     .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M | UART_RXFIFO_OVF_INT_ENA_M | UART_BRK_DET_INT_ENA_M | UART_PARITY_ERR_INT_ENA_M | UART_FRM_ERR_INT_ENA_M,
    //     .rxfifo_full_thresh = 5,
    //     .rx_timeout_thresh = 10,
    //     .txfifo_empty_intr_thresh = 5};
    // uart_intr_config(UART_NUM_0, &uart_intr);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // Create UART tasks
    xTaskCreatePinnedToCore(uart_rx_task, "uart_rx_hci", 4096, NULL, configMAX_PRIORITIES - 1, &uart_rx_task_handle, 1);
    xTaskCreatePinnedToCore(uart_tx_task, "uart_tx_hci", 4096, NULL, configMAX_PRIORITIES - 1, &uart_tx_task_handle, 1);
    // Indicate Startup
    ets_write_char_uart(ESP32_EVT_STARTUP);

    vTaskDelete(NULL);
}

void setup_bt()
{
    static esp_vhci_host_callback_t vhci_host_cb = {
        vhci_to_uart_notify,
        vhci_to_uart};

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        printf("Bluetooth controller initialize failed\n");
        return;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        printf("Bluetooth controller enable failed\n");
        return;
    }

    esp_vhci_host_register_callback(&vhci_host_cb);
}

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
IRAM_ATTR void my_post_setup_cb(spi_slave_transaction_t *trans)
{
    WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1 << GPIO_HANDSHAKE));
}

//Called after transaction is sent/received. We use this to set the handshake line low.
IRAM_ATTR void my_post_trans_cb(spi_slave_transaction_t *trans)
{
    WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1 << GPIO_HANDSHAKE));
}

void spi_hci_task()
{
    while (1) {
        delay_ms(1);
        // Process HCI to Controller event here
        while (queue_hci_rx_packets.idx && !spinlock_tx_intercept_lmp && esp_vhci_host_check_send_available()) {
            // puts("HCI OK");
            // util_print_buffer(queue_hci_rx_packets.pkt_buffer[0], queue_hci_rx_packets.size[0]);
            esp_vhci_host_send_packet(queue_hci_rx_packets.pkt_buffer[0], queue_hci_rx_packets.size[0]);
            pkt_queue_pop(&queue_hci_rx_packets);
        }
    }
}

// ------------------------- Main -----------------------------

void app_main()
{
    // Workaround for debug exception hang after firmware flash (Disable this during debugging)
    // debug_exception_workaround();

    // #### Init functions ####
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Setup default BT mac address
    default_bt_address[5] -= 2; // Compase bt mac addr offset
    // esp_base_mac_addr_set(default_bt_address); // Set own bt address
    // Setup Bluetooth and configure VHCI to UART callback
    setup_bt();
    // Setup UART and start RX handler thread
    xTaskCreatePinnedToCore(setup_uart0, "setup_uart0", 1024, NULL, configMAX_PRIORITIES - 1, NULL, 1);
    // Setup SPI and start TX/RX handler thread
#if ENABLE_EXPERIMENTAL_SPI
    xTaskCreatePinnedToCore(spi_task, "spi_task", 4096, NULL, configMAX_PRIORITIES - 1, NULL, 1);
#endif
    // #### Reverse Engineering ####
    volatile uint8_t detached = 0;
    if (detached) {
        // Force hooks linkage
        hook_tx_acl_control(0, 0, 0, 0, 0, 0);
        hook_tx_acl_data(0, NULL);
        hook_frame_isr(0, 0);
        hook_frame_isr_callback(0, 0, 0, 0);
        reversed_r_ld_acl_lmp_tx(0, 0, 0);
        custom_lc_reset_lc_default_state_funcs();
    }

    // Patch exported pointers
    bt_apply_rom_hooks();
}
