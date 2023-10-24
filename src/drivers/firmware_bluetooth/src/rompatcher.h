#ifndef __ROMPATCHER__
#define __ROMPATCHER__

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "xtensa/specreg.h"

#define SET_INTERRUPT_LEVEL(x) WSR(PS, PS_INTLEVEL(x) | PS_UM | PS_WOE)
#define ROMPATCHER_LEVEL_DEFAULT -1
#define READ_RETURN_ADDRESS(val) asm volatile("mov %0, a8" \
                                              : "=r"(val));

// ----------------------- RomPatcher -----------------------

typedef struct
{
    uint32_t target_addr;
    int16_t return_offset;
    uint32_t (*callback)(XtExcFrame *frame);
    uint8_t rearm_patch;
    uint32_t interrupt_level;
} rompatcher_config_t;

rompatcher_config_t rompatcher_config[2] = {0};
RTC_NOINIT_ATTR uint32_t noinit_reset_reason;

static inline void IRAM_ATTR debug_exception_workaround()
{
    // TODO: Properly detect debugging (fix rompatcher to include default debug exception)
    if (noinit_reset_reason != 3 && esp_reset_reason() == 3)
    {
        noinit_reset_reason = 3;
        esp_restart();
    }
    noinit_reset_reason = 0;
}

static inline void IRAM_ATTR breakpoint_add(unsigned id, uint32_t addr)
{
    // https://github.com/espressif/esp-idf/blob/master/components/hal/esp32/include/hal/cpu_ll.h#L64
    uint32_t en;

    // Set the break address register to the appropriate PC
    if (!id)
    {
        WSR(128, addr);
    }
    else
    {
        WSR(129, addr);
    }

    // Enable the breakpoint using the break enable register
    RSR(IBREAKENABLE, en);
    en |= BIT(id);
    WSR(IBREAKENABLE, en);
}

static inline void IRAM_ATTR breakpoint_del(uint8_t id)
{
    uint32_t en;

    // Disable the breakpoint using the break enable register
    RSR(IBREAKENABLE, en);
    en &= ~BIT(id);
    WSR(IBREAKENABLE, en);
}

static inline void IRAM_ATTR rompatcher_add(uint8_t id, uint32_t target_addr, int16_t return_offset,
                                            uint32_t (*callback)(XtExcFrame *frame), int interrupt_level)
{
    if (id > 1)
        id = 1;
    rompatcher_config[id].target_addr = target_addr;
    rompatcher_config[id].return_offset = return_offset;
    rompatcher_config[id].callback = callback;
    rompatcher_config[id].interrupt_level = interrupt_level;
    breakpoint_add(id, target_addr);
}

static inline void IRAM_ATTR rompatcher_remove(uint8_t id)
{
    breakpoint_del(id);
}

static inline void IRAM_ATTR rompatcher_clear()
{
    breakpoint_del(0);
    breakpoint_del(1);
}

void IRAM_ATTR rompatcher_handler(XtExcFrame *frame)
{
    uint8_t id = 0;
    uint32_t brk_addr1;

    RSR(128, brk_addr1);
    if (frame->pc == brk_addr1)
    {
        breakpoint_del(0);
    }
    else
    {
        breakpoint_del(1);
        id = 1;
    };

    rompatcher_config_t *rc = &rompatcher_config[id];

    if (rc->interrupt_level != ROMPATCHER_LEVEL_DEFAULT)
        SET_INTERRUPT_LEVEL(rc->interrupt_level);

    if (rc->callback)
    {
        rc->callback(frame);
        frame->pc += rc->return_offset;
    }
}

#endif