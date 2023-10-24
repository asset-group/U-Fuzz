#include <inttypes.h>

// ---------------- Main API (declarations) ----------------

// Name
extern const char *__attribute__((weak)) module_name()
{
    return 0;
}

// Setup
int __attribute__((weak)) setup(void *p)
{
    return 0;
}

// TX PRE
int __attribute__((weak)) tx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}
// TX POST
extern int __attribute__((weak)) tx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}

// RX PRE
extern int __attribute__((weak)) rx_pre_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}
// RX POST
extern int __attribute__((weak)) rx_post_dissection(uint8_t *pkt_buf, int pkt_length, void *p)
{
    return 0;
}