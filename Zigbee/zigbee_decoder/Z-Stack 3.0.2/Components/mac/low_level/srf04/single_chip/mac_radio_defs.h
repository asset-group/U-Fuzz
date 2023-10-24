/**************************************************************************************************
  Filename:       mac_radio_def.h
  Revised:        $Date: 2014-06-05 16:56:45 -0700 (Thu, 05 Jun 2014) $
  Revision:       $Revision: 38844 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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

#ifndef MAC_RADIO_DEFS_H
#define MAC_RADIO_DEFS_H

/* ------------------------------------------------------------------------------------------------
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_defs.h"
#include "hal_board_cfg.h"
#include "hal_mac_cfg.h"
#include "mac_spec.h"
#include "mac_mcu.h"
#include "mac_mem.h"
#include "mac_csp_tx.h"
#include "mac_assert.h"
#include "mac_high_level.h"
#include "hal_sleep.h"
#include "mac_radio.h"

/* ------------------------------------------------------------------------------------------------
 *                                      Target Specific Defines
 * ------------------------------------------------------------------------------------------------
 */

/* PA LNA chip used */
#define PA_LNA_NONE    0  
#define PA_LNA_CC2591  1 
#define PA_LNA_CC2590  2
#define PA_LNA_SE2431L 3
#define PA_LNA_CC2592  4
   
/* immediate strobe commands */
#define ISTXCAL       0xEC
#define ISRXON        0xE3
#define ISTXON        0xE9
#define ISTXONCCA     0xEA
#define ISRFOFF       0xEF
#define ISFLUSHRX     0xED
#define ISFLUSHTX     0xEE
#define ISACK         0xE6
#define ISACKPEND     0xE7
#define ISNACK        0xE8

/* FSCTRLL */
#define FREQ_2405MHZ                  0x0B

/* FSMSTAT1 */
#define TX_ACTIVE                     BV(1)
#define CCA                           BV(4)
#define SFD                           BV(5)
#define FIFOP                         BV(6)
#define FIFO                          BV(7)

/* IEN0 */
#define RFERRIE                       BV(0)

/* IEN2 */
#define RFIE                          BV(0)

/* FRMCTRL0 */
#define FRMCTRL0_RESET_VALUE          0x40
#define ENERGY_SCAN                   BV(4)
#define AUTOACK                       BV(5)
#define RX_MODE(x)                    ((x) << 2)
#define RX_MODE_INFINITE_RECEPTION    RX_MODE(2)
#define RX_MODE_NORMAL_OPERATION      RX_MODE(0)

/* FRMCTRL1 */
#define PENDING_OR                    BV(2)

/* FRMFILT0 */
#define PAN_COORDINATOR               BV(1)
#define FRAME_FILTER_EN               BV(0)

#define FRAME_VERSION(x)              ((x) << 3)
#define FRAME_FILTER_MAX_VERSION      FRAME_VERSION(1)

/* SRCMATCH */
#define PEND_DATAREQ_ONLY             BV(2)
#define AUTOPEND                      BV(1)
#define SRC_MATCH_EN                  BV(0)

/* SRCRESINDEX */
#define AUTOPEND_RES                  BV(6)

/* MDMCTRL1 */
#define CORR_THR_SFD                  BV(5)
#define CORR_THR                      0x14

/* CCACTRL0 */
#define CCA_THR                       0xFC   /* -4-76=-80dBm when CC2530 operated alone or with CC2591 in LGM */
#define CCA_THR_HGM                   0x06   /* 6-76=-70dBm when CC2530 operated with CC2591 in HGM */
#define CCA_THR_MINUS_20              0x38
#define CCA_THR_CC2533                0xF8

/* CCACTRL1 */
#define CCA_OTHER_MODE_11             0x1A
#define CCA_OTHER_MODE_01             0x0A

/* FSMSTATE */
#define FSM_FFCTRL_STATE_RX_INF       31      /* infinite reception state - not documented in datasheet */

/* ADCCON1 */
#define RCTRL1                        BV(3)
#define RCTRL0                        BV(2)
#define RCTRL_BITS                    (RCTRL1 | RCTRL0)
#define RCTRL_CLOCK_LFSR              RCTRL0

/* CSPCTRL */
#define CPU_CTRL                      BV(0)
#define CPU_CTRL_ON                   CPU_CTRL
#define CPU_CTRL_OFF                  (!(CPU_CTRL))

/* TXFILTCFG */
#define TXFILTCFG                     XREG( 0x61FA )
#define TXFILTCFG_RESET_VALUE         0x09

/* ------------------------------------------------------------------------------------------------
 *                                    Unique Radio Define
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_CC2530
#define FEATURE_MAC_RADIO_HARDWARE_OVERFLOW_NO_ROLLOVER


/* ------------------------------------------------------------------------------------------------
 *                                    Common Radio Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_CHANNEL_DEFAULT               11
#define MAC_RADIO_CHANNEL_INVALID               0xFF
#define MAC_RADIO_TX_POWER_INVALID              0xFF

#define MAC_RADIO_RECEIVER_SENSITIVITY_DBM      -97 /* dBm */
#define MAC_RADIO_RECEIVER_SATURATION_DBM       10  /* dBm */

/* Reduce RX power consumption current to 20mA at the cost of some sensitivity
 * Note: This feature can be applied to CC2530 and CC2533 only.
 */
#ifdef FEATURE_CC253X_LOW_POWER_RX
#undef  HAL_MAC_RSSI_OFFSET
#define HAL_MAC_RSSI_OFFSET                     -61 /* no unit */
#endif
   
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592
# define MAC_RADIO_SENSITIVITY_OFFSET(x)         st(  x += macRadioDefsSensitivityAdj[macRadioDefsRefTableId>> 4]; )
# define MAC_RADIO_SATURATION_OFFSET(x)          st(  x += macRadioDefsSaturationAdj[macRadioDefsRefTableId>> 4]; )

#elif defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
      defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592
# define MAC_RADIO_SENSITIVITY_OFFSET(x)         st(  x += macRadioDefsSensitivityAdj[0]; )
# define MAC_RADIO_SATURATION_OFFSET(x)          st(  x += macRadioDefsSaturationAdj[0]; )

#else
# define MAC_RADIO_SENSITIVITY_OFFSET(x)
# define MAC_RADIO_SATURATION_OFFSET(x) 

#endif

/* offset applied to hardware RSSI value to get RF power level in dBm units */
#define MAC_RADIO_RSSI_OFFSET                   HAL_MAC_RSSI_OFFSET

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L ||  defined MAC_RUNTIME_CC2592 || \
    defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
    defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592
# define MAC_RADIO_RSSI_LNA_OFFSET(x)            st(  x += macRadioDefsRssiAdj[macRadioDefsRefTableId&0x0F]; )
#else
# define MAC_RADIO_RSSI_LNA_OFFSET(x)
#endif
      


/* precise values for small delay on both receive and transmit side */
#define MAC_RADIO_RX_TX_PROP_DELAY_MIN_USEC     3.076  /* usec */
#define MAC_RADIO_RX_TX_PROP_DELAY_MAX_USEC     3.284  /* usec */

/* register value table offset */
#define MAC_RADIO_DEFS_TBL_TXPWR_FIRST_ENTRY   0
#define MAC_RADIO_DEFS_TBL_TXPWR_LAST_ENTRY    1
#define MAC_RADIO_DEFS_TBL_TXPWR_ENTRIES       2

/* RF observable control register value to output PA signal */
#define RFC_OBS_CTRL_PA_PD_INV        0x68

/* RF observable control register value to output LNA signal */
#define RFC_OBS_CTRL_LNAMIX_PD_INV    0x6A

/* RF observable control register value to output LNA signal
 * for CC2591 compression workaround.
 */
#define RFC_OBS_CTRL_DEMOD_CCA        0x0D

/* OBSSELn register value to select RF observable 0 */
#define OBSSEL_OBS_CTRL0             0xFB

/* OBSSELn register value to select RF observable 1 */
#define OBSSEL_OBS_CTRL1             0xFC

#define MAC_RADIO_CHANNEL_INVALID               0xFF


/* ------------------------------------------------------------------------------------------------
 *                                      Common Radio Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_MCU_INIT()                          macMcuInit()

#define MAC_RADIO_TURN_ON_POWER()                     macRadioTurnOnPower()
#define MAC_RADIO_TURN_OFF_POWER()                    macRadioTurnOffPower()
#define MAC_RADIO_TURN_ON_OSC()                       MAC_ASSERT(SLEEPSTA & XOSC_STB)/* oscillator should already be stable, it's mcu oscillator too */
#define MAC_RADIO_TURN_OFF_OSC()                      /* don't do anything, oscillator is also mcu oscillator */

#define MAC_RADIO_RX_FIFO_HAS_OVERFLOWED()            ((FSMSTAT1 & FIFOP) && !(FSMSTAT1 & FIFO))
#define MAC_RADIO_RX_FIFO_IS_EMPTY()                  (!(FSMSTAT1 & FIFO) && !(FSMSTAT1 & FIFOP))

#define MAC_RADIO_SET_RX_THRESHOLD(x)                 st( FIFOPCTRL = ((x)-1); )
#define MAC_RADIO_RX_IS_AT_THRESHOLD()                (FSMSTAT1 & FIFOP)
#define MAC_RADIO_ENABLE_RX_THRESHOLD_INTERRUPT()     MAC_MCU_FIFOP_ENABLE_INTERRUPT()
#define MAC_RADIO_DISABLE_RX_THRESHOLD_INTERRUPT()    MAC_MCU_FIFOP_DISABLE_INTERRUPT()
#define MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG() MAC_MCU_FIFOP_CLEAR_INTERRUPT()

#define MAC_RADIO_TX_ACK()                            MAC_RADIO_TURN_OFF_PENDING_OR()
#define MAC_RADIO_TX_ACK_PEND()                       MAC_RADIO_TURN_ON_PENDING_OR()

#define MAC_RADIO_RX_ON()                             st( RFST = ISRXON;    )
#define MAC_RADIO_RXTX_OFF()                          st( RFST = ISRFOFF;   )
#define MAC_RADIO_FLUSH_RX_FIFO()                     st( RFST = ISFLUSHRX; RFST = ISFLUSHRX; )
#define MAC_RADIO_FLUSH_TX_FIFO()                     st( RFST = ISFLUSHTX; )

#define MAC_RADIO_READ_RX_FIFO(pData,len)             macMemReadRxFifo((pData),(uint8)(len))
#define MAC_RADIO_WRITE_TX_FIFO(pData,len)            macMemWriteTxFifo((pData),(uint8)(len))

#define MAC_RADIO_SET_PAN_COORDINATOR(b)              st( FRMFILT0 = (FRMFILT0 & ~PAN_COORDINATOR) | (PAN_COORDINATOR * (b!=0)); )
#define MAC_RADIO_SET_CHANNEL(x)                      st( FREQCTRL = FREQ_2405MHZ + 5 * ((x) - 11); )

#define MAC_RADIO_SELECT_BOOST_MODE(x)                st( \
  if ((*(uint8 *)(P_INFOPAGE+0x03) == 0x95) || (*(uint8 *)(P_INFOPAGE+0x03) == 0x34)) \
  { \
    /* If boost mode is enabled, it is used for specific higher power levels. \
     * Thus, the code below must be in sync with which entries of pTable utilize boost mode. \
     * Currently, boost mode is only for >= 5dBm which corresponds to register setting 0xFD. \
     */ \
    reqTxBoost = ((x) == 0xFD) ? TRUE : FALSE; \
  } \
)

#define MAC_RADIO_SELECT_PTABLE(PTABLE)               st( \
  if ((*(uint8 *)(P_INFOPAGE+0x03) == 0x95) || (*(uint8 *)(P_INFOPAGE+0x03) == 0x34)) \
  { \
    MAC_RADIO_SELECT_PTABLE_CC2533((PTABLE)); \
  } \
  else \
  { \
    MAC_RADIO_SELECT_PTABLE_CC2530((PTABLE)); \
  } \
)

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_CC2592 || defined MAC_RUNTIME_SE2431L

#define MAC_RADIO_SELECT_PTABLE_CC2530(PTABLE) \
  st( (PTABLE) = macRadioDefsTxPwrTables[macRadioDefsRefTableId >> 4]; )

#define MAC_RADIO_SELECT_PTABLE_CC2533(PTABLE) \
  st( (PTABLE) = macRadioDefsTxPwrTables0x95[macRadioDefsRefTableId >> 4]; )

#elif defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
    defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592

#define MAC_RADIO_SELECT_PTABLE_CC2530(PTABLE) \
  st( (PTABLE) = macRadioDefsTxPwrTables[0]; )

#define MAC_RADIO_SELECT_PTABLE_CC2533(PTABLE) \
  st( (PTABLE) = macRadioDefsTxPwrTables0x95[0]; )

#else

#define MAC_RADIO_SELECT_PTABLE_CC2530(PTABLE) \
  st( (PTABLE) = macRadioDefsTxPwrBare; )

#define MAC_RADIO_SELECT_PTABLE_CC2533(PTABLE) \
  st( (PTABLE) = macRadioDefsTxPwrBare0x95; )

#endif

#define MAC_RADIO_SET_TX_POWER(x)                     st( \
  if ((*(uint8 *)(P_INFOPAGE+0x03) == 0x95) || (*(uint8 *)(P_INFOPAGE+0x03) == 0x34)) \
  { \
     TXPOWER = macPhyTxPower; \
    if (reqTxBoost)  /* Configure the specific Boost Mode Tx register settings. */\
    { \
      FSCTRL = 0xF5; \
      TXCTRL = 0x74; \
    } \
    else             /* Restore non-boosted/default values. */\
    { \
      FSCTRL = 0x55; \
      TXCTRL = 0x69; \
    } \
  } \
  else \
  { \
     TXPOWER = x;\
  }\
 )\

#define MAC_RADIO_SET_PAN_ID(x)                       st( PAN_ID0 = (x) & 0xFF; PAN_ID1 = (x) >> 8; )
#define MAC_RADIO_SET_SHORT_ADDR(x)                   st( SHORT_ADDR0 = (x) & 0xFF; SHORT_ADDR1 = (x) >> 8; )
#define MAC_RADIO_SET_IEEE_ADDR(p)                    macMemWriteRam((macRam_t *) &EXT_ADDR0, p, 8)

#define MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK()      st( MAC_MCU_TXACKDONE_CLEAR_INTERRUPT(); MAC_MCU_TXACKDONE_ENABLE_INTERRUPT(); )
#define MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK()       MAC_MCU_TXACKDONE_DISABLE_INTERRUPT()

#define MAC_RADIO_RANDOM_BYTE()                       macMcuRandomByte()
#define MAC_RADIO_RANDOM_WORD()                       macMcuRandomWord()

#define MAC_RADIO_TX_RESET()                          macCspTxReset()
#define MAC_RADIO_TX_PREP_CSMA_UNSLOTTED()            macCspTxPrepCsmaUnslotted()
#define MAC_RADIO_TX_PREP_CSMA_SLOTTED()              macCspTxPrepCsmaSlotted()
#define MAC_RADIO_TX_PREP_SLOTTED()                   macCspTxPrepSlotted()
#define MAC_RADIO_TX_PREP_GREEN_POWER()               macCspTxPrepGreenPower()
#define MAC_RADIO_TX_GO_CSMA()                        macCspTxGoCsma()
#define MAC_RADIO_TX_GO_SLOTTED()                     macCspTxGoSlotted()
#define MAC_RADIO_TX_GO_SLOTTED_CSMA()                macCspTxGoCsma()
#define MAC_RADIO_TX_GO_GREEN_POWER()                 macCspTxGoGreenPower()

#define MAC_RADIO_FORCE_TX_DONE_IF_PENDING()          macCspForceTxDoneIfPending()

#define MAC_RADIO_TX_REQUEST_ACK_TIMEOUT_CALLBACK()   macCspTxRequestAckTimeoutCallback()
#define MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK()    macCspTxCancelAckTimeoutCallback()

#define MAC_RADIO_TIMER_TICKS_PER_USEC()              HAL_CPU_CLOCK_MHZ /* never fractional */
#define MAC_RADIO_TIMER_TICKS_PER_BACKOFF()           (HAL_CPU_CLOCK_MHZ * MAC_SPEC_USECS_PER_BACKOFF)
#define MAC_RADIO_TIMER_TICKS_PER_SYMBOL()            (HAL_CPU_CLOCK_MHZ * MAC_SPEC_USECS_PER_SYMBOL)

#define MAC_RADIO_TIMER_CAPTURE()                     macMcuTimerCapture()
#define MAC_RADIO_TIMER_FORCE_DELAY(x)                macMcuTimerForceDelay(x)

#define MAC_RADIO_TIMER_SLEEP()                       st( T2CTRL &= ~TIMER2_RUN; while(T2CTRL & TIMER2_STATE); )
#define MAC_RADIO_TIMER_WAKE_UP()                     st( st( while (CLKCONSTA != (CLKCONCMD_32MHZ | OSC_32KHZ)); ); \
                                                          T2CTRL |= (TIMER2_RUN | TIMER2_SYNC); \
                                                          while(!(T2CTRL & TIMER2_STATE)); )

#define MAC_RADIO_BACKOFF_COUNT()                     macMcuOverflowCount()
#define MAC_RADIO_BACKOFF_CAPTURE()                   macMcuOverflowCapture()
#define MAC_RADIO_BACKOFF_SET_COUNT(x)                macMcuOverflowSetCount(x)
#define MAC_RADIO_BACKOFF_SET_COMPARE(x)              macMcuOverflowSetCompare(x)

#define MAC_RADIO_BACKOFF_COMPARE_CLEAR_INTERRUPT()    st( T2IRQF  = (TIMER2_OVF_COMPARE1F ^ 0xFF); )
#define MAC_RADIO_BACKOFF_COMPARE_ENABLE_INTERRUPT()   st( T2IRQM |= TIMER2_OVF_COMPARE1M; )
#define MAC_RADIO_BACKOFF_COMPARE_DISABLE_INTERRUPT()  st( T2IRQM &= ~TIMER2_OVF_COMPARE1M; )

#define MAC_RADIO_BACKOFF_SET_PERIOD(x)               macMcuOverflowSetPeriod(x)
#define MAC_RADIO_BACKOFF_PERIOD_CLEAR_INTERRUPT()    st( T2IRQF  = (TIMER2_OVF_PERF ^ 0xFF); )
#define MAC_RADIO_BACKOFF_PERIOD_ENABLE_INTERRUPT()   st( T2IRQM |= TIMER2_OVF_PERM; )
#define MAC_RADIO_BACKOFF_PERIOD_DISABLE_INTERRUPT()  st( T2IRQM &= ~TIMER2_OVF_PERM; )

#define MAC_RADIO_RECORD_MAX_RSSI_START()             macMcuRecordMaxRssiStart()
#define MAC_RADIO_RECORD_MAX_RSSI_STOP()              macMcuRecordMaxRssiStop()

#define MAC_RADIO_TURN_ON_RX_FRAME_FILTERING()        st( FRMFILT0  = 0; \
                                                          FRMFILT0 |= (FRAME_FILTER_EN | FRAME_FILTER_MAX_VERSION); )
#define MAC_RADIO_TURN_OFF_RX_FRAME_FILTERING()       st( FRMFILT0 &= ~FRAME_FILTER_EN; )

/*--------Source Matching------------*/

#define MAC_RADIO_TURN_ON_AUTO_ACK()                  st( FRMCTRL0 |= AUTOACK; )
#define MAC_RADIO_CANCEL_TX_ACK()                     st( RFST = ISNACK; )

#define MAC_RADIO_TURN_ON_AUTOPEND_DATAREQ_ONLY()     st( SRCMATCH |= PEND_DATAREQ_ONLY; )

#define MAC_RADIO_TURN_ON_SRC_MATCH()                 st( SRCMATCH |= SRC_MATCH_EN; )
#define MAC_RADIO_TURN_ON_AUTOPEND()                  st( SRCMATCH |= AUTOPEND; )
#define MAC_RADIO_IS_AUTOPEND_ON()                    ( SRCMATCH & AUTOPEND )
#define MAC_RADIO_SRC_MATCH_RESINDEX(p)               st( p = SRCRESINDEX; )

#define MAC_RADIO_TURN_ON_PENDING_OR()                st( FRMCTRL1 |= PENDING_OR; )
#define MAC_RADIO_TURN_OFF_PENDING_OR()               st( FRMCTRL1 &= ~PENDING_OR; )

#define MAC_RADIO_SRC_MATCH_GET_SHORTADDR_EN()        macSrcMatchGetShortAddrEnableBit()
#define MAC_RADIO_SRC_MATCH_GET_EXTADDR_EN()          macSrcMatchGetExtAddrEnableBit()
#define MAC_RADIO_SRC_MATCH_GET_SHORTADDR_PENDEN()    macSrcMatchGetShortAddrPendEnBit()
#define MAC_RADIO_SRC_MATCH_GET_EXTADDR_PENDEN()      macSrcMatchGetExtAddrPendEnBit()

#define MAC_RADIO_GET_SRC_SHORTPENDEN(p)              macMemReadRam( (uint8*)&SRCSHORTPENDEN0, (p), 3 )
#define MAC_RADIO_GET_SRC_EXTENPEND(p)                macMemReadRam( (uint8*)&SRCEXTPENDEN0, (p), 3 )
#define MAC_RADIO_GET_SRC_SHORTEN(p)                  macMemReadRam( (uint8*)&SRCSHORTEN0, (p), 3 )
#define MAC_RADIO_GET_SRC_EXTEN(p)                    macMemReadRam( (uint8*)&SRCEXTEN0, (p), 3 )

#define MAC_RADIO_SRC_MATCH_SET_SHORTPENDEN(p)        macMemWriteRam( (uint8*)&SRCSHORTPENDEN0, (p), 3 )
#define MAC_RADIO_SRC_MATCH_SET_EXTPENDEN(p)          macMemWriteRam( (uint8*)&SRCEXTPENDEN0, (p), 3 )
#define MAC_RADIO_SRC_MATCH_SET_SHORTEN(x)            osal_buffer_uint24( (uint8*)&SRCSHORTEN0, (x) )
#define MAC_RADIO_SRC_MATCH_SET_EXTEN(x)              osal_buffer_uint24( (uint8*)&SRCEXTEN0, (x) )
#define MAC_RADIO_SRC_MATCH_RESULT()                  MAC_SrcMatchCheckResult()

#define MAC_RADIO_SRC_MATCH_INIT_EXTPENDEN()          st( SRCEXTPENDEN0 = 0; \
                                                          SRCEXTPENDEN1 = 0; \
                                                          SRCEXTPENDEN2 = 0; )
#define MAC_RADIO_SRC_MATCH_INIT_SHORTPENDEN()        st( SRCSHORTPENDEN0 = 0; \
                                                          SRCSHORTPENDEN1 = 0; \
                                                          SRCSHORTPENDEN2 = 0; )

#define MAC_RADIO_SRC_MATCH_TABLE_WRITE(offset, p, len)    macMemWriteRam( (macRam_t *)(SRC_ADDR_TABLE + (offset)), (p), (len) )
#define MAC_RADIO_SRC_MATCH_TABLE_READ(offset, p, len)     macMemReadRam( SRC_ADDR_TABLE + (offset), (p), (len))


/* ----------- PA/LNA control ---------- */
/* For Skyworks parts only CPS maps closely to the HGM line */
#define HAL_PA_LNA_RX_HGM()                           st( P0_7 = 1; )
#define HAL_PA_LNA_RX_LGM()                           st( P0_7 = 0; )


/* ----------- PA/LNA SKE2431L control ---------- */
/* CSD Line high maps closely to EN which is controlled by hardware */ 
#define HAL_PA_LNA_RX_CSD_HIGH()                          st( P1_3 = 1; ) 

/* case of CC2590 CC2591 and CC2592. Non Skyworks part */
#define HAL_PA_LNA_RX_CSD_LOW()                           st( P1_3 = 0; ) 

/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */

extern uint8 reqTxBoost;  /* CC2533/34-specific register settings to achieve highest Tx power. */
extern uint8 paLnaChip;

/* ------------------------------------------------------------------------------------------------
 *                                    Common Radio Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macRadioDefsRefTableId;
extern const uint8 CODE *const CODE macRadioDefsTxPwrTables[];
extern const uint8 CODE *const CODE macRadioDefsTxPwrTables0x95[];
extern const int8 CODE macRadioDefsRssiAdj[];
extern const int8 CODE macRadioDefsSensitivityAdj[];
extern const int8 CODE macRadioDefsSaturationAdj[];
extern const uint8 CODE macRadioDefsTxPwrBare[];
extern const uint8 CODE macRadioDefsTxPwrBare0x95[];
extern int8 macRadioRecieverSensitivity;
extern int8 macRadioRecieverSaturation; 



MAC_INTERNAL_API void macRadioTurnOnPower(void);
MAC_INTERNAL_API void macRadioTurnOffPower(void);


/* ------------------------------------------------------------------------------------------------
 *                              Transmit Power Setting Configuration
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  To use actual register values for setting power level, uncomment the following #define.
 *  In this case, the value passed to the set power function will be written directly to TXCTRLL.
 *  Characterized values for this register can be found in the datasheet in the "power settings" table.
 */
//#define HAL_MAC_USE_REGISTER_POWER_VALUES


/**************************************************************************************************
 */
#endif
