/**************************************************************************************************
  Filename:       mac_sleep.c
  Revised:        $Date: 2007-10-29 19:11:44 -0700 (Mon, 29 Oct 2007) $
  Revision:       $Revision: 15810 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2009 Texas Instruments Incorporated. All rights reserved.

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
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"

/* high-level */
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_sleep.h"
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macSleepState = MAC_SLEEP_STATE_RADIO_OFF;


/**************************************************************************************************
 * @fn          macSleepWakeUp
 *
 * @brief       Wake up the radio from sleep mode.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macSleepWakeUp(void)
{
  /* don't wake up radio if it's already awake */
  if (macSleepState == MAC_SLEEP_STATE_AWAKE)
  {
    return;
  }

  /* wake up MAC timer */
  MAC_RADIO_TIMER_WAKE_UP();

  /* if radio was completely off, restore from that state first */
  if (macSleepState == MAC_SLEEP_STATE_RADIO_OFF)
  {
    /* turn on radio power (turns on oscillator too) */
    MAC_RADIO_TURN_ON_POWER();

    /* power-up initialization of receive logic */
    macRxRadioPowerUpInit();
  }
  else
  {
    MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_OSC_OFF);

    /* turn on the oscillator */
    MAC_RADIO_TURN_ON_OSC();
  }

  /* update sleep state here before requesting to turn on receiver */
  macSleepState = MAC_SLEEP_STATE_AWAKE;

  /* turn on the receiver if enabled */
  macRxOnRequest();
}


/**************************************************************************************************
 * @fn          macSleep
 *
 * @brief       Puts radio into the selected sleep mode.
 *
 * @param       sleepState - selected sleep level, see #defines in .h file
 *
 * @return      TRUE if radio was successfully put into selected sleep mode.
 *              FALSE if it was not safe for radio to go to sleep.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macSleep(uint8 sleepState)
{
  halIntState_t  s;

  /* disable interrupts until macSleepState can be set */
  HAL_ENTER_CRITICAL_SECTION(s);

  /* assert checks */
  MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_AWAKE); /* radio must be awake to put it to sleep */
  MAC_ASSERT(macRxFilter == RX_FILTER_OFF); /* do not sleep when scanning or in promiscuous mode */

  /* if either RX or TX is active or any RX enable flags are set, it's not OK to sleep */
  if (macRxActive || macRxOutgoingAckFlag || macTxActive || macRxEnableFlags)
  {
    HAL_EXIT_CRITICAL_SECTION(s);
    return(FALSE);
  }

  /* turn off the receiver */
  macRxOff();

  /* update sleep state variable */
  macSleepState = sleepState;

  /* macSleepState is now set, re-enable interrupts */
  HAL_EXIT_CRITICAL_SECTION(s);

  /* put MAC timer to sleep */
  MAC_RADIO_TIMER_SLEEP();

  /* put radio in selected sleep mode */
  if (sleepState == MAC_SLEEP_STATE_OSC_OFF)
  {
    MAC_RADIO_TURN_OFF_OSC();
  }
  else
  {
    MAC_ASSERT(sleepState == MAC_SLEEP_STATE_RADIO_OFF); /* unknown sleep state */
    MAC_RADIO_TURN_OFF_POWER();
  }

  /* radio successfully entered sleep mode */
  return(TRUE);
}



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if ((MAC_SLEEP_STATE_AWAKE == MAC_SLEEP_STATE_OSC_OFF) ||  \
     (MAC_SLEEP_STATE_AWAKE == MAC_SLEEP_STATE_RADIO_OFF))
#error "ERROR!  Non-unique state values."
#endif


/**************************************************************************************************
*/
