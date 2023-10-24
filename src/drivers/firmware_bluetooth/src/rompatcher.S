#include <xtensa/coreasm.h>
#include <xtensa/corebits.h>
#include <xtensa/config/system.h>
#include <freertos/xtensa_context.h>
#include <freertos/xtensa_rtos.h>
#include <esp_private/panic_reason.h>
#include <soc/soc.h>
#include "sdkconfig.h"


// https://github.com/espressif/esp-idf/blob/master/components/esp_system/port/soc/esp32/dport_panic_highint_hdl.S#L90-L105
    .section .iram1,"ax"
    .global     xt_debugexception
    .type       xt_debugexception,@function
    .align      4
xt_debugexception:

    /* Start saving context */
    mov     a0, sp
    addi    sp, sp, -XT_STK_FRMSZ
    s32i    a0, sp, XT_STK_A1
    #if XCHAL_HAVE_WINDOWED
    s32e    a0, sp, -12                     /* for debug backtrace */
    #endif
    rsr     a0, PS                          /* save interruptee's PS */
    s32i    a0, sp, XT_STK_PS
    rsr     a0, EPC_6                       /* save interruptee's PC */
    s32i    a0, sp, XT_STK_PC
    #if XCHAL_HAVE_WINDOWED
    s32e    a0, sp, -16                     /* for debug backtrace */
    #endif
    s32i    a12, sp, XT_STK_A12             /* _xt_context_save requires a12,a13 */
    s32i    a13, sp, XT_STK_A13             
    call0   _xt_context_save                /* Save xtensa context */
    rsr     a0, EXCSAVE_6                   
    s32i    a0, sp, XT_STK_A0               /* save interruptee's a0 */


    /* Set up PS for C, enable debug and NMI interrupts and clear EXCM. */
    movi    a0, PS_INTLEVEL(5) | PS_UM | PS_WOE
    wsr     a0, PS
    /* User function here (must use call4) */
    mov     a6,sp 
    call4   rompatcher_handler           
    /* User function end */

    call0   _xt_context_restore             /* Restore xtensa context */

    l32i    a0, sp, XT_STK_PS               /* retrieve interruptee's PS */
    wsr     a0, PS
    l32i    a0, sp, XT_STK_PC               /* retrieve interruptee's PC */
    wsr     a0, EPC_6
    l32i    sp, sp, XT_STK_A1               /* remove exception frame */               
    rsync                                   /* ensure PS and EPC written */

    rsr     a0, EXCSAVE_6                   /* restore a0 */
    rfi     6



