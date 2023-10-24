/**************************************************************************************************
  Filename:       mac_backoff_timer.c
  Revised:        $Date: 2013-05-17 11:25:11 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34355 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2012 Texas Instruments Incorporated. All rights reserved.

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

/* high-level specific */
#include "mac_spec.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_backoff_timer.h"
#include "mac_tx.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define COMPARE_STATE_ROLLOVER_BV                 BV(0)
#define COMPARE_STATE_TRIGGER_BV                  BV(1)
#define COMPARE_STATE_ARM_BV                      BV(2)

#define COMPARE_STATE_TRIGGER                     COMPARE_STATE_TRIGGER_BV
#define COMPARE_STATE_ROLLOVER                    COMPARE_STATE_ROLLOVER_BV
#define COMPARE_STATE_ROLLOVER_AND_TRIGGER        (COMPARE_STATE_ROLLOVER_BV | COMPARE_STATE_TRIGGER_BV)
#define COMPARE_STATE_ROLLOVER_AND_ARM_TRIGGER    (COMPARE_STATE_ROLLOVER_BV | COMPARE_STATE_ARM_BV)


/*
 *  The datasheet mentions a small delay on both receive and transmit side of approximately
 *  two microseconds.  The precise characterization is given below.
 *  (This data is not given in rev 1.03 datasheet)
 */
#define RX_TX_PROP_DELAY_AVG_USEC         ((MAC_RADIO_RX_TX_PROP_DELAY_MIN_USEC + MAC_RADIO_RX_TX_PROP_DELAY_MAX_USEC) / 2)
#define RX_TX_PROP_DELAY_AVG_TIMER_TICKS  ((uint16)(MAC_RADIO_TIMER_TICKS_PER_USEC() * RX_TX_PROP_DELAY_AVG_USEC + 0.5))

/*
 *  For slotted receives, the SFD signal is expected to occur on a specifc symbol boundary.
 *  This does *not* correspond to the backoff boundary.  The SFD signal occurs at an
 *  offset from the backoff boundary.  This is done for efficiency of related algorithms.
 *
 *  Once transmit is strobed there is a fixed delay until the SFD signal occurs.  The frame
 *  does not start over-the-air transmit until after an internal radio delay of 12 symbols.
 *  Once transmitting over-the-air, the preamble is sent (8 symbols) followed by the
 *  SFD field (2 symbols). After the SFD field completes, the SFD signal occurs.  This
 *  adds up to a total of 22 symbols from strobe to SFD signal.
 *
 *  Since 22 symbols spans more than a backoff (20 symbols) the modulus operation is used
 *  to find the symbol offset which is 2 symbols.
 *
 *  This math is derived formally via the pre-processor.
 */
#define SYMBOLS_FROM_STROBE_TO_PREAMBLE   12 /* from datasheet */
#define SYMBOLS_FROM_PREAMBLE_TO_SFD      (MAC_SPEC_PREAMBLE_FIELD_LENGTH + MAC_SPEC_SFD_FIELD_LENGTH)
#define SYMBOLS_FROM_STROBE_TO_SFD        (SYMBOLS_FROM_STROBE_TO_PREAMBLE + SYMBOLS_FROM_PREAMBLE_TO_SFD)
#define SYMBOLS_EXPECTED_AT_SFD           (SYMBOLS_FROM_STROBE_TO_SFD % MAC_A_UNIT_BACKOFF_PERIOD)

/* after all that formal math, make sure the result is as expected */
#if (SYMBOLS_EXPECTED_AT_SFD != 2)
#error "ERROR! Internal problem with pre-processor math of slotted alignment."
#endif


/*
 *  The expected SFD signal occurs at the symbol offset *plus* a small internal propagation delay
 *  internal to the radio.  This delay is given as the sum of a receive side delay and a transmit
 *  side delay.  When this delay is subtracted from the internal timer, the internal time base
 *  actually becomes the actual receive time *minus* the transmit delay.  This works out though.
 *  The transmit logic does *not* take into account this delay.  Since the timer is skewed by the
 *  transmit delay already, the transmits go out precisely on time.
 */
#define TIMER_TICKS_EXPECTED_AT_SFD   ((SYMBOLS_EXPECTED_AT_SFD * MAC_RADIO_TIMER_TICKS_PER_SYMBOL()) \
                                          + RX_TX_PROP_DELAY_AVG_TIMER_TICKS)


/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint32 backoffTimerRollover;
static uint32 backoffTimerTrigger;


/**************************************************************************************************
 * @fn          macBackoffTimerInit
 *
 * @brief       Intializes backoff timer.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerInit(void)
{
  MAC_RADIO_BACKOFF_SET_COUNT(0);
  macBackoffTimerSetRollover(MAC_BACKOFF_TIMER_DEFAULT_NONBEACON_ROLLOVER);
  MAC_RADIO_BACKOFF_PERIOD_CLEAR_INTERRUPT();
  MAC_RADIO_BACKOFF_PERIOD_ENABLE_INTERRUPT();
  MAC_RADIO_BACKOFF_COMPARE_CLEAR_INTERRUPT();
  MAC_RADIO_BACKOFF_COMPARE_ENABLE_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macBackoffTimerReset
 *
 * @brief       Resets backoff timer.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerReset(void)
{
  MAC_RADIO_BACKOFF_COMPARE_DISABLE_INTERRUPT();
  MAC_RADIO_BACKOFF_PERIOD_DISABLE_INTERRUPT();
  macBackoffTimerInit();
}


/**************************************************************************************************
 * @fn          macBackoffTimerSetRollover
 *
 * @brief       Set rollover count of backoff timer.
 *
 * @param       rolloverBackoff - backoff count where count is reset to zero
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerSetRollover(uint32 rolloverBackoff)
{
  halIntState_t  s;

  MAC_ASSERT(rolloverBackoff > MAC_RADIO_BACKOFF_COUNT());  /* rollover value must be greater than count */

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffTimerRollover = rolloverBackoff;
  MAC_RADIO_BACKOFF_SET_PERIOD(rolloverBackoff);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macBackoffTimerSetCount
 *
 * @brief       Sets the count of the backoff timer.
 *
 * @param       backoff - new count
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerSetCount(uint32 backoff)
{
  halIntState_t  s;

  MAC_ASSERT(backoff < backoffTimerRollover);  /* count must be less than rollover value */
  MAC_ASSERT(!(backoff & 0x80000000));  /* count must not represent negative value for int32 */

  HAL_ENTER_CRITICAL_SECTION(s);
  MAC_RADIO_BACKOFF_SET_COUNT(backoff);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macBackoffTimerCount
 *
 * @brief       Returns the current backoff count.
 *
 * @param       none
 *
 * @return      current backoff count
 **************************************************************************************************
 */
MAC_INTERNAL_API uint32 macBackoffTimerCount(void)
{
  halIntState_t  s;
  uint32 backoffCount;

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffCount = MAC_RADIO_BACKOFF_COUNT();
  HAL_EXIT_CRITICAL_SECTION(s);
  
#ifdef FEATURE_MAC_RADIO_HARDWARE_OVERFLOW_NO_ROLLOVER
  /*
   *  Extra processing is required if the radio has a special hardware overflow
   *  count feature.  Unfortunately this feature does not provide for setting a
   *  rollover value.  This must be done manually.
   *
   *  This means there is a small window in time when reading the hardware count
   *  will be inaccurate.  It's possible it could be one more than the allowable
   *  count.  This happens if the count has just incremented beyond the maximum
   *  and is queried before the ISR has a chance to run and reset the backoff
   *  count back to zero.  (Pure software implementation of backoff count does
   *  not have this problem.)
   *
   *  To solve this, before returning a value for the backoff count, the value
   *  must be tested to see if it is beyond the maximum value.  If so, a rollover
   *  interrupt that will set backoff count to zero is imminent.  In that case,
   *  the correct backoff count of zero is returned.
   */
  if (backoffCount >= backoffTimerRollover)
  {
    return(0);
  }
#endif
  
  return(backoffCount);
}


/**************************************************************************************************
 * @fn          macBackoffTimerCapture
 *
 * @brief       Returns the most recently captured backoff count
 *
 * @param       none
 *
 * @return      last backoff count that was captured
 **************************************************************************************************
 */
MAC_INTERNAL_API uint32 macBackoffTimerCapture(void)
{
  halIntState_t  s;
  uint32 backoffCapture;

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffCapture = MAC_RADIO_BACKOFF_CAPTURE();
  HAL_EXIT_CRITICAL_SECTION(s);

#ifdef FEATURE_MAC_RADIO_HARDWARE_OVERFLOW_NO_ROLLOVER
  /*
   *  See other instance of this #ifdef for detailed comments.
   *  Those comments apply to the backoff capture value too.
   */
  if (backoffCapture >= backoffTimerRollover)
  {
    return(0);
  }
#endif
  
  return(backoffCapture);
}


/**************************************************************************************************
 * @fn          macBackoffTimerGetTrigger
 *
 * @brief       Returns the trigger set for the backoff timer.
 *
 * @param       none
 *
 * @return      backoff count of trigger
 **************************************************************************************************
 */
MAC_INTERNAL_API uint32 macBackoffTimerGetTrigger(void)
{
  return(backoffTimerTrigger);
}


/**************************************************************************************************
 * @fn          macBackoffTimerSetTrigger
 *
 * @brief       Sets the trigger count for the backoff counter.  A callback is exectuted when
 *              the backoff count reaches the trigger
 *
 * @param       triggerBackoff - backoff count for new trigger
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerSetTrigger(uint32 triggerBackoff)
{
  halIntState_t  s;

  MAC_ASSERT(triggerBackoff < backoffTimerRollover); /* trigger backoff must be less than rollover backoff */

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffTimerTrigger = triggerBackoff;
  MAC_RADIO_BACKOFF_SET_COMPARE(triggerBackoff);
  if (triggerBackoff == MAC_RADIO_BACKOFF_COUNT())
  {
    /* Clear the interrupt and fire it manually */
    MAC_RADIO_BACKOFF_COMPARE_CLEAR_INTERRUPT();
    HAL_EXIT_CRITICAL_SECTION(s);
    macBackoffTimerTriggerCallback();
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}


/**************************************************************************************************
 * @fn          macBackoffTimerCancelTrigger
 *
 * @brief       Cancels the trigger for the backoff counter - obselete for CC2530.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerCancelTrigger(void)
{
  /* Stub for high level MAC */
}


/**************************************************************************************************
 * @fn          macBackoffTimerRealign
 *
 * @brief       
 *
 *  Realignment is accomplished by adjusting the internal time base to align with the expected
 *  reception time of an incoming frame.  The difference between the expected reception time and
 *  the actual reception time is computed and this difference is used to adjust the hardware
 *  timer count and backoff count.
 *
 *  The realignment is based on the SFD signal for the incoming frame.  The timer is aligned
 *  by adjusting it with the difference between the expected SFD time and the actual SFD time.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API int32 macBackoffTimerRealign(macRx_t *pMsg)
{
  uint16 timerDelayTicks;
  int32 backoffDelta;
  int32 backoffCount;

  MAC_ASSERT(!MAC_TX_IS_PHYSICALLY_ACTIVE()); /* realignment during actual transmit corrupts timing */

  /*-------------------------------------------------------------------------------
   *  Calculate the delta backoff difference between expected backoff count,
   *  which is zero, and the backoff count of the received frame.
   */

  /* since expected receive time is zero, the delta is simply the receive time */
  backoffDelta = pMsg->mac.timestamp;

  /* if the frame was received more than halfway to the rollover count, use a negative delta value */
  if (((uint32) backoffDelta) > (backoffTimerRollover / 2))
  {
    backoffDelta = backoffDelta - backoffTimerRollover;    /* result will be negative */
  }

  /*-------------------------------------------------------------------------------
   *  Calculate the number of timer ticks to delay that will align the internal
   *  time base with the received frame.
   */

  /* retrieve the timer count when frame was received */
  timerDelayTicks = pMsg->mac.timestamp2;

  /*
   *  Subtract the expected SFD time from the actual SFD time to find the needed
   *  timer adjustment. If subtracting the offset would result in a negative value,
   *  the tick delay must wrap around.
   */
  if (timerDelayTicks >= TIMER_TICKS_EXPECTED_AT_SFD)
  {
    /* since delay count is greater than or equal to offset, subtract it directly */
    timerDelayTicks = timerDelayTicks - TIMER_TICKS_EXPECTED_AT_SFD;
  }
  else
  {
    /*
     *  The expected time is greater that actualy time so it cannot be subtracted directly.
     *  The tick count per backoff is added to wrap around within the backoff.
     *  Since a wrap around did happen, the backoff delta is adjusted by one.
     */
    timerDelayTicks = timerDelayTicks - TIMER_TICKS_EXPECTED_AT_SFD + MAC_RADIO_TIMER_TICKS_PER_BACKOFF();
    backoffDelta--;
  }

  /*-------------------------------------------------------------------------------
   *  Calculate the new backoff count.
   */

  backoffCount = MAC_RADIO_BACKOFF_COUNT() - backoffDelta;

  if (backoffCount >= ((int32) backoffTimerRollover))
  {
    backoffCount -= backoffTimerRollover;
  }
  else if (backoffCount < 0)
  {
    backoffCount += backoffTimerRollover;
  }

  MAC_RADIO_TIMER_FORCE_DELAY(timerDelayTicks);
  MAC_RADIO_BACKOFF_SET_COUNT(backoffCount);

  return(backoffDelta);
}


/**************************************************************************************************
 * @fn          macBackoffTimerCompareIsr
 *
 * @brief       Interrupt service routine that fires when the backoff count is equal
 *              to the trigger count.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerCompareIsr(void)
{
  macBackoffTimerTriggerCallback();
}

/**************************************************************************************************
 * @fn          macBackoffTimerPeriodIsr
 *
 * @brief       Interrupt service routine that fires when the backoff count rolls over on
 *              overflow period.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBackoffTimerPeriodIsr(void)
{
  macMcuAccumulatedOverFlow();
  macBackoffTimerRolloverCallback();
}

/**************************************************************************************************
 * @fn          macGetBackOffTimerRollover
 *
 * @brief       Function to get the timer 2 rollover value
 *
 * @param       none
 *
 * @return      timer 2 rollover value
 **************************************************************************************************
 */
MAC_INTERNAL_API uint32 macGetBackOffTimerRollover(void)
{
  return backoffTimerRollover;
}

/**************************************************************************************************
*/
