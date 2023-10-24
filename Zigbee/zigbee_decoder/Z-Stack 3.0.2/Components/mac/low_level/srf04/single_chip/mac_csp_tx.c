/**************************************************************************************************
  Filename:       mac_csp_tx.c
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

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"
#include "hal_mcu.h"

/* high-level */
#include "mac_spec.h"
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_csp_tx.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"

/* ------------------------------------------------------------------------------------------------
 *                                   CSP Defines / Macros
 * ------------------------------------------------------------------------------------------------
 */
/* immediate strobe commands */
#define ISSTART     0xE1
#define ISSTOP      0xE2
#define ISCLEAR     0xFF

/* strobe processor instructions */
#define SKIP(s,c)   (0x00 | (((s) & 0x07) << 4) | ((c) & 0x0F))   /* skip 's' instructions if 'c' is true  */
#define WHILE(c)    SKIP(0,c)              /* pend while 'c' is true (derived instruction)        */
#define WAITW(w)    (0x80 | ((w) & 0x1F))  /* wait for 'w' number of MAC timer overflows          */
#define WEVENT1     (0xB8)                 /* wait for MAC timer compare                          */
#define WAITX       (0xBC)                 /* wait for CSPX number of MAC timer overflows         */
#define LABEL       (0xBB)                 /* set next instruction as start of loop               */
#define RPT(c)      (0xA0 | ((c) & 0x0F))  /* if condition is true jump to last label             */
#define INT         (0xBA)                 /* assert IRQ_CSP_INT interrupt                        */
#define INCY        (0xC1)                 /* increment CSPY                                      */
#define INCMAXY(m)  (0xC8 | ((m) & 0x07))  /* increment CSPY but not above maximum value of 'm'   */
#define DECX        (0xC3)                 /* decrement CSPX                                      */
#define DECY        (0xC4)                 /* decrement CSPY                                      */
#define DECZ        (0xC5)                 /* decrement CSPZ                                      */
#define RANDXY      (0xBD)                 /* load the lower CSPY bits of CSPX with random value  */

/* strobe processor command instructions */
#define SSTOP       (0xD2)    /* stop program execution                                      */
#define SNOP        (0xD0)    /* no operation                                                */
#define STXCAL      (0xDC)    /* enable and calibrate frequency synthesizer for TX           */
#define SRXON       (0xD3)    /* turn on receiver                                            */
#define STXON       (0xD9)    /* transmit after calibration                                  */
#define STXONCCA    (0xDA)    /* transmit after calibration if CCA indicates clear channel   */
#define SRFOFF      (0xDF)    /* turn off RX/TX                                              */
#define SFLUSHRX    (0xDD)    /* flush receive FIFO                                          */
#define SFLUSHTX    (0xDE)    /* flush transmit FIFO                                         */
#define SACK        (0xD6)    /* send ACK frame                                              */
#define SACKPEND    (0xD7)    /* send ACK frame with pending bit set                         */

/* conditions for use with instructions SKIP and RPT */
#define C_CCA_IS_VALID        0x00
#define C_SFD_IS_ACTIVE       0x01
#define C_CPU_CTRL_IS_ON      0x02
#define C_END_INSTR_MEM       0x03
#define C_CSPX_IS_ZERO        0x04
#define C_CSPY_IS_ZERO        0x05
#define C_CSPZ_IS_ZERO        0x06
#define C_RSSI_IS_VALID       0x07

/* negated conditions for use with instructions SKIP and RPT */
#define C_NEGATE(c)   ((c) | 0x08)
#define C_CCA_IS_INVALID      C_NEGATE(C_CCA_IS_VALID)
#define C_SFD_IS_INACTIVE     C_NEGATE(C_SFD_IS_ACTIVE)
#define C_CPU_CTRL_IS_OFF     C_NEGATE(C_CPU_CTRL_IS_ON)
#define C_NOT_END_INSTR_MEM   C_NEGATE(C_END_INSTR_MEM)
#define C_CSPX_IS_NON_ZERO    C_NEGATE(C_CSPX_IS_ZERO)
#define C_CSPY_IS_NON_ZERO    C_NEGATE(C_CSPY_IS_ZERO)
#define C_CSPZ_IS_NON_ZERO    C_NEGATE(C_CSPZ_IS_ZERO)
#define C_RSSI_IS_INVALID     C_NEGATE(C_RSSI_IS_VALID)


/* ------------------------------------------------------------------------------------------------
 *                                         Defines
 * ------------------------------------------------------------------------------------------------
 */

/* CSPZ return values from CSP program */
#define CSPZ_CODE_TX_DONE           0
#define CSPZ_CODE_CHANNEL_BUSY      1
#define CSPZ_CODE_TX_ACK_TIME_OUT   2


/* ------------------------------------------------------------------------------------------------
 *                                     Local Programs
 * ------------------------------------------------------------------------------------------------
 */
static void  cspPrepForTxProgram(void);
static void  cspWeventSetTriggerNow(void);
static void  cspWeventSetTriggerSymbols(uint8 symbols);
static uint8 cspReadCountSymbols(void);



/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
#define CSP_STOP_AND_CLEAR_PROGRAM()          st( RFST = ISSTOP; RFST = ISCLEAR; )
#define CSP_START_PROGRAM()                   st( RFST = ISSTART; )

/*
 *  These macros improve readability of using T2CMP in conjunction with WEVENT.
 *
 *  The timer2 compare, T2CMP, only compares one byte of the 16-bit timer register.
 *  It is configurable and has been set to compare against the upper byte of the timer value.
 *  The CSP instruction WEVENT waits for the timer value to be greater than or equal
 *  the value of T2CMP.
 *
 *  Reading the timer value is done by reading the low byte first.  This latches the
 *  high byte.  A trick with the ternary operator is used by a macro below to force a
 *  read of the low byte when returning the value of the high byte.
 *
 *  CSP_WEVENT_SET_TRIGGER_NOW()      - sets the WEVENT1 trigger point at the current timer count
 *  CSP_WEVENT_SET_TRIGGER_SYMBOLS(x) - sets the WEVENT1 trigger point in symbols
 *  CSP_WEVENT_READ_COUNT_SYMBOLS()   - reads the current timer count in symbols
 */
#define T2THD_TICKS_PER_SYMBOL                (MAC_RADIO_TIMER_TICKS_PER_SYMBOL() >> 8)

#define CSP_WEVENT_CLEAR_TRIGGER()            st( T2IRQF = (TIMER2_COMPARE1F ^ 0xFF); )
#define CSP_WEVENT_SET_TRIGGER_NOW()          cspWeventSetTriggerNow()
#define CSP_WEVENT_SET_TRIGGER_SYMBOLS(x)     cspWeventSetTriggerSymbols(x)
#define CSP_WEVENT_READ_COUNT_SYMBOLS()       cspReadCountSymbols()

/*
 *  Number of bits used for aligning a slotted transmit to the backoff count (plus
 *  derived values).  There are restrictions on this value.  Compile time integrity
 *  checks will catch an illegal setting of this value.  A full explanation accompanies
 *  this compile time check (see bottom of this file).
 */
#define SLOTTED_TX_MAX_BACKOFF_COUNTDOWN_NUM_BITS     4
#define SLOTTED_TX_MAX_BACKOFF_COUNTDOWN              (1 << SLOTTED_TX_MAX_BACKOFF_COUNTDOWN_NUM_BITS)
#define SLOTTED_TX_BACKOFF_COUNT_ALIGN_BIT_MASK       (SLOTTED_TX_MAX_BACKOFF_COUNTDOWN - 1)


/**************************************************************************************************
 * @fn          macCspTxReset
 *
 * @brief       Reset the CSP.  Immediately halts any running program.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxReset(void)
{
  MAC_MCU_CSP_STOP_DISABLE_INTERRUPT();
  MAC_MCU_CSP_INT_DISABLE_INTERRUPT();
  CSP_STOP_AND_CLEAR_PROGRAM();
}


/*=================================================================================================
 * @fn          cspWeventSetTriggerNow
 *
 * @brief       sets the WEVENT1 trigger point at the current timer count
 *
 * @param       none
 *
 * @return      symbols
 *=================================================================================================
 */
static void cspWeventSetTriggerNow(void)
{
  halIntState_t  s;
  uint8          temp0, temp1;

  /* Clear the compare interrupt flag for debugging purpose. */
  CSP_WEVENT_CLEAR_TRIGGER();

  /* copy current timer count to compare */
  HAL_ENTER_CRITICAL_SECTION(s);
  MAC_MCU_T2_ACCESS_COUNT_VALUE();
  temp0 = T2M0;
  temp1 = T2M1;

  /* MAC timer bug on the cc2530 PG1 made it impossible to use
   * compare = 0 for both the timer and the overflow counter.
   */
  if ((macChipVersion <= REV_B) && (temp0 == 0) && (temp1 == 0))
  {
    temp0++;
  }

  MAC_MCU_T2_ACCESS_CMP1_VALUE();
  T2M0 = temp0;
  T2M1 = temp1;
  HAL_EXIT_CRITICAL_SECTION(s);
}


/*=================================================================================================
 * @fn          cspWeventSetTriggerSymbols
 *
 * @brief       sets the WEVENT1 trigger point in symbols
 *
 * @param       symbols
 *
 * @return      none
 *=================================================================================================
 */
static void cspWeventSetTriggerSymbols(uint8 symbols)
{
  halIntState_t  s;
  uint16         cmp;

  MAC_ASSERT(symbols <= MAC_A_UNIT_BACKOFF_PERIOD);

  /* Clear the compare interrupt flag for debugging purpose. */
  CSP_WEVENT_CLEAR_TRIGGER();

  HAL_ENTER_CRITICAL_SECTION(s);
  MAC_MCU_T2_ACCESS_CMP1_VALUE();
  cmp  = (symbols) * MAC_RADIO_TIMER_TICKS_PER_SYMBOL();

  /* MAC timer bug on the cc2530 PG1 made it impossible to use
   * compare = 0 for both the timer and the overflow counter.
   */
  if ((macChipVersion <= REV_B) && (cmp == 0))
  {
    cmp++;
  }
  T2M0 = (cmp & 0xFF);
  T2M1 = (cmp >> 8);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/*=================================================================================================
 * @fn          cspReadCountSymbols
 *
 * @brief       reads the current timer count in symbols
 *
 * @param       none
 *
 * @return      symbols
 *=================================================================================================
 */
static uint8 cspReadCountSymbols(void)
{
  uint8          countLow, countHigh;
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  MAC_MCU_T2_ACCESS_COUNT_VALUE();
  countLow  = T2M0;
  countHigh = T2M1;
  HAL_EXIT_CRITICAL_SECTION(s);

  return (((countHigh << 8) | countLow) / MAC_RADIO_TIMER_TICKS_PER_SYMBOL());
}


/*=================================================================================================
 * @fn          cspPrepForTxProgram
 *
 * @brief       Prepare and initialize for transmit CSP program.
 *              Call *before* loading the CSP program!
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void cspPrepForTxProgram(void)
{
  MAC_ASSERT(!(RFIRQM1 & IM_CSP_STOP)); /* already an active CSP program */

  /* set CSP EVENT1 to T2 CMP1 */
  MAC_MCU_CONFIG_CSP_EVENT1();

  /* set up parameters for CSP transmit program */
  CSPZ = CSPZ_CODE_CHANNEL_BUSY;

  /* clear the currently loaded CSP, this generates a stop interrupt which must be cleared */
  CSP_STOP_AND_CLEAR_PROGRAM();
  MAC_MCU_CSP_STOP_CLEAR_INTERRUPT();
  MAC_MCU_CSP_INT_CLEAR_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macCspTxPrepCsmaUnslotted
 *
 * @brief       Prepare CSP for "Unslotted CSMA" transmit.  Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxPrepCsmaUnslotted(void)
{
  cspPrepForTxProgram();

  /*----------------------------------------------------------------------
   *  Load CSP program :  Unslotted CSMA transmit
   */

  /*
   *  Wait for X number of backoffs, then wait for intra-backoff count
   *  to reach value set for WEVENT1.
   */
  RFST = WAITX;
  RFST = WEVENT1;

  /* wait until RSSI is valid */
  RFST = WHILE(C_RSSI_IS_INVALID);

  /* Note that the CCA signal is updated four clock cycles (system clock) 
   * after the RSSI_VALID signal has been set.
   */
  RFST = SNOP;
  RFST = SNOP;
  RFST = SNOP;
  RFST = SNOP;

  /* sample CCA, if it fails exit from here, CSPZ indicates result */
  RFST = SKIP(1, C_CCA_IS_VALID);
  RFST = SSTOP;
  
  /* CSMA has passed so transmit (actual frame starts one backoff from when strobe is sent) */
  RFST = STXON;

  /*
   *  Wait for the start of frame delimiter of the transmitted frame.  If SFD happens to
   *  already be active when STXON is strobed, it gets forced low.  How long this takes
   *  though, is not certain.  For bulletproof operation, the first step is to wait
   *  until SFD is inactive (which should be very fast if even necessary), and then wait
   *  for it to go active.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = WHILE(C_SFD_IS_INACTIVE);

  /*
   *  Record the timestamp.  The INT instruction causes an interrupt to fire.
   *  The ISR for this interrupt records the timestamp (which was just captured
   *  when SFD went high).
   */
  RFST = INT;

  /*
   *  Wait for SFD to go inactive which is the end of transmit.  Decrement CSPZ to indicate
   *  the transmit was successful.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = DECZ;

  /*
   * CC2530 requires SSTOP to generate CSP_STOP interrupt.
   */
  RFST = SSTOP;
}


/**************************************************************************************************
 * @fn          macCspTxPrepCsmaSlotted
 *
 * @brief       Prepare CSP for "Slotted CSMA" transmit.  Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxPrepCsmaSlotted(void)
{
  cspPrepForTxProgram();

  /*----------------------------------------------------------------------
   *  Load CSP program :  Slotted CSMA transmit
   */

  /* wait for X number of backoffs */
  RFST = WAITX;
  
  /* sample RSSI, if it is valid then skip one extra backoff. */
  RFST = SKIP(1, C_RSSI_IS_VALID);
  
  /* wait for one backoff to guarantee receiver has been on at least that long */
  RFST = WAITW(1);

  /* sample CCA, if it fails exit from here, CSPZ indicates result */
  RFST = SKIP(1, C_CCA_IS_VALID);
  RFST = SSTOP;

  /* per slotted CSMA-CCA in specification, wait one backoff */
  RFST = WAITW(1);

  /* sample CCA again, if it fails exit from here, CSPZ indicates result */
  RFST = SKIP(1, C_CCA_IS_VALID);
  RFST = SSTOP;

  /* CSMA has passed so transmit */
  RFST = STXON;

  /*
   *  Wait for the start of frame delimiter of the transmitted frame.  If SFD happens to
   *  already be active when STXON is strobed, it gets forced low.  How long this takes
   *  though, is not certain.  For bulletproof operation, the first step is to wait
   *  until SFD is inactive (which should be very fast if even necessary), and then wait
   *  for it to go active.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = WHILE(C_SFD_IS_INACTIVE);

  /*
   *  Record the timestamp.  The INT instruction causes an interrupt to fire.
   *  The ISR for this interrupt records the timestamp (which was just captured
   *  when SFD went high).
   */
  RFST = INT;

  /*
   *  Wait for SFD to go inactive which is the end of transmit.  Decrement CSPZ to indicate
   *  the transmit was successful.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = DECZ;

  /*
   * CC2530 requires SSTOP to generate CSP_STOP interrupt.
   */
  RFST = SSTOP;
}


/**************************************************************************************************
 * @fn          macCspTxGoCsma
 *
 * @brief       Run previously loaded CSP program for CSMA transmit.  Handles either
 *              slotted or unslotted CSMA transmits.  When CSP program has finished,
 *              an interrupt occurs and macCspTxStopIsr() is called.  This ISR will in
 *              turn call macTxDoneCallback().
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxGoCsma(void)
{
  /*
   *  Set CSPX with the countdown time of the CSMA delay.
   */
  CSPX = macTxCsmaBackoffDelay;

  /*
   *  Set WEVENT to trigger at the current value of the timer.  This allows
   *  unslotted CSMA to transmit just a little bit sooner.
   */
  CSP_WEVENT_SET_TRIGGER_NOW();

  /*
   *  Enable interrupt that fires when CSP program stops.
   *  Also enable interrupt that fires when INT instruction
   *  is executed.
   */
  MAC_MCU_CSP_STOP_ENABLE_INTERRUPT();
  MAC_MCU_CSP_INT_ENABLE_INTERRUPT();

  /*
   *  Turn on the receiver if it is not already on.  Receiver must be 'on' for at
   *  least one backoff before performing clear channel assessment (CCA).
   */
  macRxOn();

  /* start the CSP program */
  CSP_START_PROGRAM();
}


/**************************************************************************************************
 * @fn          macCspTxPrepSlotted
 *
 * @brief       Prepare CSP for "Slotted" (non-CSMA) transmit.
 *              Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxPrepSlotted(void)
{
  cspPrepForTxProgram();

  /*----------------------------------------------------------------------
   *  Load CSP program :  Slotted transmit (no CSMA)
   */

  /* wait for X number of backoffs */
  RFST = WAITX;

  /* just transmit, no CSMA required */
  RFST = STXON;

  /*
   *  Wait for the start of frame delimiter of the transmitted frame.  If SFD happens to
   *  already be active when STXON is strobed, it gets forced low.  How long this takes
   *  though, is not certain.  For bulletproof operation, the first step is to wait
   *  until SFD is inactive (which should be very fast if even necessary), and then wait
   *  for it to go active.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = WHILE(C_SFD_IS_INACTIVE);

  /*
   *  Record the timestamp.  The INT instruction causes an interrupt to fire.
   *  The ISR for this interrupt records the timestamp (which was just captured
   *  when SFD went high).
   */
  RFST = INT;

  /*
   *  Wait for SFD to go inactive which is the end of transmit.  Decrement CSPZ to indicate
   *  the transmit was successful.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = DECZ;

  /*
   * CC2530 requires SSTOP to generate CSP_STOP interrupt.
   */
  RFST = SSTOP;

}


/**************************************************************************************************
 * @fn          macCspTxGoSlotted
 *
 * @brief       Run previously loaded CSP program for non-CSMA slotted transmit.   When CSP
 *              program has finished, an interrupt occurs and macCspTxStopIsr() is called.
 *              This ISR will in turn call macTxDoneCallback().
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxGoSlotted(void)
{
  halIntState_t  s;
  uint8 lowByteOfBackoffCount;
  uint8 backoffCountdown;

  /*
   *  Enable interrupt that fires when CSP program stops.
   *  Also enable interrupt that fires when INT instruction
   *  is executed.
   */
  MAC_MCU_CSP_STOP_ENABLE_INTERRUPT();
  MAC_MCU_CSP_INT_ENABLE_INTERRUPT();

  /* critical section needed for timer accesses */
  HAL_ENTER_CRITICAL_SECTION(s);

  /* store lowest byte of backoff count (same as lowest byte of overflow count) */
  MAC_MCU_T2_ACCESS_OVF_COUNT_VALUE();

  /* Latch T2MOVFx */
  T2M0;
  lowByteOfBackoffCount = T2MOVF0;

  /*
   *  Compute the number of backoffs until time to strobe transmit.  The strobe should
   *  occur one backoff before the SFD pin is expected to go high.  So, the forumla for the
   *  countdown value is to determine when the lower bits would rollover and become zero,
   *  and then subtract one.
   */
  backoffCountdown = SLOTTED_TX_MAX_BACKOFF_COUNTDOWN - (lowByteOfBackoffCount & SLOTTED_TX_BACKOFF_COUNT_ALIGN_BIT_MASK) - 1;

  /*
   *  Store backoff countdown value into CSPX.
   *
   *  Note: it is OK if this value is zero.  The WAITX instruction at the top of the
   *  CSP program will immediately continue if CSPX is zero when executed.  However,
   *  if the countdown is zero, it means the transmit function was not called early
   *  enough for a properly timed slotted transmit.  The transmit will be late.
   */
  CSPX = backoffCountdown;

  /* Disable Rx and flush RXFIFO due to chip bug #1546 */
  macRxHardDisable();

  /*
   *  The receiver will be turned on during CSP execution, guaranteed.
   *  Since it is not possible to update C variables within the CSP,
   *  the new "on" state of the receiver must be set a little early
   *  here before the CSP is started.
   */
  MAC_RX_WAS_FORCED_ON();

  /* start the CSP program */
  CSP_START_PROGRAM();

  /*
   *  If the previous stored low byte of the backoff count is no longer equal to
   *  the current value, a rollover has occurred.  This means the backoff countdown
   *  stored in CSPX may not be correct.
   *
   *  In this case, the value of CSPX is reloaded to reflect the correct backoff
   *  countdown value (this is one less than what was just used as a rollover has
   *  occurred).  Since it is certain a rollover *just* occurred, there is no danger
   *  of another rollover occurring.  This means the value written to CSPX is guaranteed
   *  to be accurate.
   *
   *  Also, the logic below ensures that the value written to CSPX is at least one.
   *  This is needed for correct operation of the WAITX instruction.  As with an
   *  initial backoff countdown value of zero, if this case does occur, it means the
   *  transmit function was not called early enough for a properly timed slotted transmit.
   *  The transmit will be late.
   *
   *  Finally, worth noting, writes to CSPX may not work if the CSP is executing the WAITX
   *  instruction and a timer rollover occurs.  In this case, however, there is no possibility
   *  of that happening.  If CSPX is updated here, a rollover has just occurred so a
   *  collision is not possible (still within a critical section here too).
   */
  MAC_MCU_T2_ACCESS_OVF_COUNT_VALUE();

  /* Latch T2MOVFx */
  T2M0;
  if ((lowByteOfBackoffCount != T2MOVF0) && (backoffCountdown > 1))
  {
    CSPX = backoffCountdown - 1;
  }

  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macCspTxPrepGreenPower
 *
 * @brief       Prepare CSP for "Green Power" transmit.  Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxPrepGreenPower(void)
{
  cspPrepForTxProgram();

  /*----------------------------------------------------------------------
   *  Load CSP program :  Green Power transmit
   */
  
  /* Set the next STXON as a lable */
  RFST = LABEL;
  
  /* wait for number of backoffs for interframe spacing */
  RFST = WAITW(macTxGpInterframeDelay);
  
  /* Transmit (actual frame starts one backoff from when strobe is sent) */
  RFST = STXON;

  /*
   *  Wait for the start of frame delimiter of the transmitted frame.  If SFD happens to
   *  already be active when STXON is strobed, it gets forced low.  How long this takes
   *  though, is not certain.  For bulletproof operation, the first step is to wait
   *  until SFD is inactive (which should be very fast if even necessary), and then wait
   *  for it to go active.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = WHILE(C_SFD_IS_INACTIVE);

  /*
   *  Record the timestamp.  The INT instruction causes an interrupt to fire.
   *  The ISR for this interrupt records the timestamp (which was just captured
   *  when SFD went high).
   */
  RFST = INT;

  /*
   *  Wait for SFD to go inactive which is the end of transmit.  Decrement CSPZ to indicate
   *  the transmit was successful.
   */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = DECX;

  /* Go to the STXON label for repeat transmissions */
  RFST = RPT(C_CSPX_IS_NON_ZERO);
  
  /* Clear the busy status */
  RFST = DECZ;
  
  /*
   * CC2530 requires SSTOP to generate CSP_STOP interrupt.
   */
  RFST = SSTOP;
}


/**************************************************************************************************
 * @fn          macCspTxGoGreenPower
 *
 * @brief       Run previously loaded CSP program for Green Power transmit.  When CSP program 
 *              has finished, an interrupt occurs and macCspTxStopIsr() is called.  This ISR 
 *              will in turn call macTxDoneCallback().
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxGoGreenPower(void)
{
  /*
   *  Set CSPX with the countdown of number of Green Power transmissions.
   */
  CSPX = macTxCsmaBackoffDelay;

  /*
   *  Enable interrupt that fires when CSP program stops.
   *  Also enable interrupt that fires when INT instruction
   *  is executed.
   */
  MAC_MCU_CSP_STOP_ENABLE_INTERRUPT();
  MAC_MCU_CSP_INT_ENABLE_INTERRUPT();

  /*
   *  Turn on the receiver if it is not already on.  Receiver must be 'on' for at
   *  least one backoff before performing clear channel assessment (CCA).
   */
  macRxOn();

  /* start the CSP program */
  CSP_START_PROGRAM();
}


/**************************************************************************************************
 * @fn          macCspForceTxDoneIfPending
 *
 * @brief       The function clears out any pending TX done logic.  Used by receive logic
 *              to make sure its ISR does not prevent transmit from completing in a reasonable
 *              amount of time.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspForceTxDoneIfPending(void)
{
  if ((CSPZ == CSPZ_CODE_TX_DONE) &&  MAC_MCU_CSP_STOP_INTERRUPT_IS_ENABLED())
  {
    MAC_MCU_CSP_STOP_DISABLE_INTERRUPT();
    if (MAC_MCU_CSP_INT_INTERRUPT_IS_ENABLED())
    {
      macCspTxIntIsr();
    }
    macTxDoneCallback();
  }
}


/**************************************************************************************************
 * @fn          macCspTxRequestAckTimeoutCallback
 *
 * @brief       Requests a callback after the ACK timeout period has expired.  At that point,
 *              the function macCspTxStopIsr() is called via an interrupt.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxRequestAckTimeoutCallback(void)
{
  uint8 startSymbol;
  uint8 symbols;
  uint8 rollovers;

  MAC_ASSERT(!(RFIRQM1 & IM_CSP_STOP)); /* already an active CSP program */

  /* record current symbol count */
  startSymbol = CSP_WEVENT_READ_COUNT_SYMBOLS();

  /* set symbol timeout from PIB */
  symbols = pMacPib->ackWaitDuration;

  /* make sure delay value is not too small for logic to handle */
  MAC_ASSERT(symbols > MAC_A_UNIT_BACKOFF_PERIOD);  /* symbols timeout period must be great than a backoff */

  /* subtract out symbols left in current backoff period */
  symbols = symbols - (MAC_A_UNIT_BACKOFF_PERIOD - startSymbol);

  /* calculate rollovers needed for remaining symbols */
  rollovers = symbols / MAC_A_UNIT_BACKOFF_PERIOD;

  /* calculate symbols that still need counted after last rollover */
  symbols = symbols - (rollovers * MAC_A_UNIT_BACKOFF_PERIOD);

  /* add one to rollovers to account for symbols remaining in the current backoff period */
  rollovers++;

  /* set up parameters for CSP program */
  CSPZ = CSPZ_CODE_TX_ACK_TIME_OUT;
  CSPX = rollovers;
  CSP_WEVENT_SET_TRIGGER_SYMBOLS(symbols);

  /* clear the currently loaded CSP, this generates a stop interrupt which must be cleared */
  CSP_STOP_AND_CLEAR_PROGRAM();
  MAC_MCU_CSP_STOP_CLEAR_INTERRUPT();

  /*--------------------------
   * load CSP program
   */
  RFST = WAITX;
  RFST = WEVENT1;
  RFST = SSTOP;

  /*--------------------------
   */

  /* run CSP program */
  MAC_MCU_CSP_STOP_ENABLE_INTERRUPT();
  CSP_START_PROGRAM();

  /*
   *  For bullet proof operation, must account for the boundary condition
   *  where a rollover occurs after count was read but before CSP program
   *  was started.
   *
   *  If current symbol count is less that the symbol count recorded at the
   *  start of this function, a rollover has occurred.
   */
  if (CSP_WEVENT_READ_COUNT_SYMBOLS() < startSymbol)
  {
    /* a rollover has occurred, make sure it was accounted for */
    if (CSPX == rollovers)
    {
      /*
       *  Rollover event missed, manually decrement CSPX to adjust.
       *
       *  Note : there is a very small chance that CSPX does not
       *  get decremented.  This would occur if CSPX were written
       *  at exactly the same time a timer overflow is occurring (which
       *  causes the CSP instruction WAITX to decrement CSPX).  This
       *  would be extremely rare, but if it does happen, the only
       *  consequence is that the ACK timeout period is extended
       *  by one backoff.
       */
      CSPX--;
    }
  }
}


/**************************************************************************************************
 * @fn          macCspTxCancelAckTimeoutCallback
 *
 * @brief       Cancels previous request for ACK timeout callback.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxCancelAckTimeoutCallback(void)
{
  MAC_MCU_CSP_STOP_DISABLE_INTERRUPT();
  CSP_STOP_AND_CLEAR_PROGRAM();
}


/**************************************************************************************************
 * @fn          macCspTxIntIsr
 *
 * @brief       Interrupt service routine for handling INT type interrupts from CSP.
 *              This interrupt happens when the CSP instruction INT is executed.  It occurs
 *              once the SFD signal goes high indicating that transmit has successfully
 *              started.  The timer value has been captured at this point and timestamp
 *              can be stored.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxIntIsr(void)
{
  MAC_MCU_CSP_INT_DISABLE_INTERRUPT();

  /* execute callback function that records transmit timestamp */
  macTxTimestampCallback();
}


/**************************************************************************************************
 * @fn          macCspTxStopIsr
 *
 * @brief       Interrupt service routine for handling STOP type interrupts from CSP.
 *              This interrupt occurs when the CSP program stops by 1) reaching the end of the
 *              program, 2) executing SSTOP within the program, 3) executing immediate
 *              instruction ISSTOP.
 *
 *              The value of CSPZ indicates if interrupt is being used for ACK timeout or
 *              is the end of a transmit.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCspTxStopIsr(void)
{
  MAC_MCU_CSP_STOP_DISABLE_INTERRUPT();

  if (CSPZ == CSPZ_CODE_TX_DONE)
  {
    macTxDoneCallback();   
  }
  else if (CSPZ == CSPZ_CODE_CHANNEL_BUSY)
  {
    macTxChannelBusyCallback();
  }
  else
  {
    MAC_ASSERT(CSPZ == CSPZ_CODE_TX_ACK_TIME_OUT); /* unexpected CSPZ value */
    macTxAckNotReceivedCallback();
  }
}



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

#if ((CSPZ_CODE_TX_DONE != 0) || (CSPZ_CODE_CHANNEL_BUSY != 1))
#error "ERROR!  The CSPZ return values are very specific and tied into the actual CSP program."
#endif

#if (MAC_TX_TYPE_SLOTTED_CSMA != 0)
#error "WARNING!  This define value changed.  It was selected for optimum performance."
#endif

#if (T2THD_TICKS_PER_SYMBOL == 0)
#error "ERROR!  Timer compare will not work on high byte.  Clock speed is probably too slow."
#endif

#define BACKOFFS_PER_BASE_SUPERFRAME  (MAC_A_BASE_SLOT_DURATION * MAC_A_NUM_SUPERFRAME_SLOTS)
#if (((BACKOFFS_PER_BASE_SUPERFRAME - 1) & SLOTTED_TX_BACKOFF_COUNT_ALIGN_BIT_MASK) != SLOTTED_TX_BACKOFF_COUNT_ALIGN_BIT_MASK)
#error "ERROR!  The specified bit mask for backoff alignment of slotted transmit does not rollover 'cleanly'."
/*
 *  In other words, the backoff count for the number of superframe rolls over before the
 *  specified number of bits rollover.  For example, if backoff count for a superframe
 *  rolls over at 48, the binary number immediately before a rollover is 00101111.
 *  In this case four bits would work as an alignment mask.  Five would not work though as
 *  the lower five bits would go from 01111 to 00000 (instead of the value 10000 which
 *  would be expected) because it a new superframe is starting.
 */
#endif
#if (SLOTTED_TX_MAX_BACKOFF_COUNTDOWN_NUM_BITS < 2)
#error "ERROR!  Not enough backoff countdown bits to be practical."
#endif


/**************************************************************************************************
*/
