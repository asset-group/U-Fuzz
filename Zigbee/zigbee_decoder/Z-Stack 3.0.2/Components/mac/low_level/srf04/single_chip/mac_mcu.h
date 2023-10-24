/**************************************************************************************************
  Filename:       mac_mcu.h
  Revised:        $Date: 2015-02-17 14:17:44 -0800 (Tue, 17 Feb 2015) $
  Revision:       $Revision: 42683 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef MAC_MCU_H
#define MAC_MCU_H

/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_board.h"
#include "mac_high_level.h"


/* ------------------------------------------------------------------------------------------------
 *                                    Target Specific Defines
 * ------------------------------------------------------------------------------------------------
 */
/* IP0, IP1 */
#define IPX_0                 BV(0)
#define IPX_1                 BV(1)
#define IPX_2                 BV(2)
#define IP_RFERR_RF_DMA_BV    IPX_0
#define IP_RXTX0_T2_BV        IPX_2

/* T2CTRL */
#define LATCH_MODE            BV(3)
#define TIMER2_STATE          BV(2)
#define TIMER2_SYNC           BV(1)
#define TIMER2_RUN            BV(0)

/* T2IRQF */
#define TIMER2_OVF_COMPARE2F  BV(5)
#define TIMER2_OVF_COMPARE1F  BV(4)
#define TIMER2_OVF_PERF       BV(3)
#define TIMER2_COMPARE2F      BV(2)
#define TIMER2_COMPARE1F      BV(1)
#define TIMER2_PERF           BV(0)

/* T2IRQM */
#define TIMER2_OVF_COMPARE2M  BV(5)
#define TIMER2_OVF_COMPARE1M  BV(4)
#define TIMER2_OVF_PERM       BV(3)
#define TIMER2_COMPARE2M      BV(2)
#define TIMER2_COMPARE1M      BV(1)
#define TIMER2_PERM           BV(0)

/* T2PEROF2 */
#define CMPIM           BV(7)
#define PERIM           BV(6)
#define OFCMPIM         BV(5)
#define PEROF2_BITS     (BV(3) | BV(2) | BV(1) | BV(0))

/* RFIRQF0 */
#define IRQ_SFD         BV(1)
#define IRQ_FIFOP       BV(2)
   
/* RFIRQF1 */
#define IRQ_TXACKDONE   BV(0)
#define IRQ_TXDONE      BV(1)
#define IRQ_CSP_MANINT  BV(3)
#define IRQ_CSP_STOP    BV(4)

/* RFIRQM0 */
#define IM_SFD          BV(1)
#define IM_FIFOP        BV(2)

/* RFIRQM1 */
#define IM_TXACKDONE    BV(0)
#define IM_TXDONE       BV(1)
#define IM_CSP_MANINT   BV(3)
#define IM_CSP_STOP     BV(4)

/* IRQSRC */
#define TXACK           BV(0)

/* RFERRM and RFERRF */
#define RFERR_RXOVERF   BV(2)


/* ------------------------------------------------------------------------------------------------
 *                                       Interrupt Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_MCU_WRITE_RFIRQF0(x)      HAL_CRITICAL_STATEMENT( S1CON = 0x00; RFIRQF0 = x; )
#define MAC_MCU_WRITE_RFIRQF1(x)      HAL_CRITICAL_STATEMENT( S1CON = 0x00; RFIRQF1 = x; )
#define MAC_MCU_OR_RFIRQM0(x)         st( RFIRQM0 |= x; )  /* compiler must use atomic ORL instruction */
#define MAC_MCU_AND_RFIRQM0(x)        st( RFIRQM0 &= x; )  /* compiler must use atomic ANL instruction */
#define MAC_MCU_OR_RFIRQM1(x)         st( RFIRQM1 |= x; )  /* compiler must use atomic ORL instruction */
#define MAC_MCU_AND_RFIRQM1(x)        st( RFIRQM1 &= x; )  /* compiler must use atomic ANL instruction */

#define MAC_MCU_FIFOP_ENABLE_INTERRUPT()              MAC_MCU_OR_RFIRQM0(IM_FIFOP)
#define MAC_MCU_FIFOP_DISABLE_INTERRUPT()             MAC_MCU_AND_RFIRQM0((IM_FIFOP ^ 0xFF))
#define MAC_MCU_FIFOP_CLEAR_INTERRUPT()               MAC_MCU_WRITE_RFIRQF0((IRQ_FIFOP ^ 0xFF))

#define MAC_MCU_TXACKDONE_ENABLE_INTERRUPT()          MAC_MCU_OR_RFIRQM1(IM_TXACKDONE)
#define MAC_MCU_TXACKDONE_DISABLE_INTERRUPT()         MAC_MCU_AND_RFIRQM1((IM_TXACKDONE ^ 0xFF))
#define MAC_MCU_TXACKDONE_CLEAR_INTERRUPT()           MAC_MCU_WRITE_RFIRQF1((IRQ_TXACKDONE ^ 0xFF))

#define MAC_MCU_CSP_STOP_ENABLE_INTERRUPT()           MAC_MCU_OR_RFIRQM1(IM_CSP_STOP)
#define MAC_MCU_CSP_STOP_DISABLE_INTERRUPT()          MAC_MCU_AND_RFIRQM1((IM_CSP_STOP ^ 0xFF))
#define MAC_MCU_CSP_STOP_CLEAR_INTERRUPT()            MAC_MCU_WRITE_RFIRQF1((IRQ_CSP_STOP ^ 0xFF))
#define MAC_MCU_CSP_STOP_INTERRUPT_IS_ENABLED()       (RFIRQM1 & IM_CSP_STOP)

#define MAC_MCU_CSP_INT_ENABLE_INTERRUPT()            MAC_MCU_OR_RFIRQM1(IM_CSP_MANINT)
#define MAC_MCU_CSP_INT_DISABLE_INTERRUPT()           MAC_MCU_AND_RFIRQM1((IM_CSP_MANINT ^ 0xFF))
#define MAC_MCU_CSP_INT_CLEAR_INTERRUPT()             MAC_MCU_WRITE_RFIRQF1((IRQ_CSP_MANINT ^ 0xFF))
#define MAC_MCU_CSP_INT_INTERRUPT_IS_ENABLED()        (RFIRQM1 & IM_CSP_MANINT)

#define MAC_MCU_RFERR_ENABLE_INTERRUPT()              st( RFERRM |=  RFERR_RXOVERF; )
#define MAC_MCU_RFERR_DISABLE_INTERRUPT()             st( RFERRM &= (RFERR_RXOVERF ^ 0xFF); )

/* ------------------------------------------------------------------------------------------------
 *                                       MAC Timer Macros
 * ------------------------------------------------------------------------------------------------
 */
#define T2M_OVF_BITS    (BV(6) | BV(5) | BV(4))
#define T2M_BITS        (BV(2) | BV(1) | BV(0))

#define T2M_OVFSEL(x)   ((x) << 4)
#define T2M_SEL(x)      (x)

#define T2M_T2OVF       T2M_OVFSEL(0)
#define T2M_T2OVF_CAP   T2M_OVFSEL(1)
#define T2M_T2OVF_PER   T2M_OVFSEL(2)
#define T2M_T2OVF_CMP1  T2M_OVFSEL(3)
#define T2M_T2OVF_CMP2  T2M_OVFSEL(4)

#define T2M_T2TIM       T2M_SEL(0)
#define T2M_T2_CAP      T2M_SEL(1)
#define T2M_T2_PER      T2M_SEL(2)
#define T2M_T2_CMP1     T2M_SEL(3)
#define T2M_T2_CMP2     T2M_SEL(4)

#define MAC_MCU_T2_ACCESS_OVF_COUNT_VALUE()   st( T2MSEL = T2M_T2OVF; )
#define MAC_MCU_T2_ACCESS_OVF_CAPTURE_VALUE() st( T2MSEL = T2M_T2OVF_CAP; )
#define MAC_MCU_T2_ACCESS_OVF_PERIOD_VALUE()  st( T2MSEL = T2M_T2OVF_PER; )
#define MAC_MCU_T2_ACCESS_OVF_CMP1_VALUE()    st( T2MSEL = T2M_T2OVF_CMP1; )
#define MAC_MCU_T2_ACCESS_OVF_CMP2_VALUE()    st( T2MSEL = T2M_T2OVF_CMP2; )

#define MAC_MCU_T2_ACCESS_COUNT_VALUE()       st( T2MSEL = T2M_T2TIM; )
#define MAC_MCU_T2_ACCESS_CAPTURE_VALUE()     st( T2MSEL = T2M_T2_CAP; )
#define MAC_MCU_T2_ACCESS_PERIOD_VALUE()      st( T2MSEL = T2M_T2_PER; )
#define MAC_MCU_T2_ACCESS_CMP1_VALUE()        st( T2MSEL = T2M_T2_CMP1; )
#define MAC_MCU_T2_ACCESS_CMP2_VALUE()        st( T2MSEL = T2M_T2_CMP2; )

#define MAC_MCU_CONFIG_CSP_EVENT1()           st( T2CSPCFG = 1; )

/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macChipVersion;

/* ------------------------------------------------------------------------------------------------
 *                                       Prototypes
 * ------------------------------------------------------------------------------------------------
 */
MAC_INTERNAL_API void macMcuInit(void);
MAC_INTERNAL_API uint8 macMcuRandomByte(void);
MAC_INTERNAL_API uint16 macMcuRandomWord(void);
MAC_INTERNAL_API void macMcuTimerForceDelay(uint16 count);
MAC_INTERNAL_API uint16 macMcuTimerCapture(void);
MAC_INTERNAL_API uint32 macMcuOverflowCount(void);
MAC_INTERNAL_API uint32 macMcuOverflowCapture(void);
MAC_INTERNAL_API void macMcuOverflowSetCount(uint32 count);
MAC_INTERNAL_API void macMcuOverflowSetCompare(uint32 count);
MAC_INTERNAL_API void macMcuOverflowSetPeriod(uint32 count);
MAC_INTERNAL_API void macMcuRecordMaxRssiStart(void);
MAC_INTERNAL_API int8 macMcuRecordMaxRssiStop(void);
MAC_INTERNAL_API void macMcuRecordMaxRssiIsr(void);
MAC_INTERNAL_API void macMcuAccumulatedOverFlow(void);
uint32 macMcuPrecisionCount(void);
void macMcuTimer2OverflowWorkaround(void);


/**************************************************************************************************
 */
#endif
