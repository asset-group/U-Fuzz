/**************************************************************************************************
  Filename:       mac_rx_onoff.c
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
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_defs.h"
#include "hal_types.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_rx_onoff.h"
#include "mac_rx.h"
#include "mac_tx.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macRxOnFlag;
uint8 macRxEnableFlags;


/**************************************************************************************************
 * @fn          macRxOnOffInit
 *
 * @brief       Initialize variables for rx on/off module.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOnOffInit(void)
{
  macRxEnableFlags = 0;
  macRxOnFlag = 0;
}


/**************************************************************************************************
 * @fn          macRxEnable
 *
 * @brief       Set enable flags and then turn on receiver.
 *
 * @param       flags - byte containing rx enable flags to set
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxEnable(uint8 flags)
{
  halIntState_t  s;

  MAC_ASSERT(flags != 0); /* rx flags not affected */

  /* set enable flags and then turn on receiver */
  HAL_ENTER_CRITICAL_SECTION(s);
  macRxEnableFlags |= flags;
  macRxOn();
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxSoftEnable
 *
 * @brief       Set enable flags but don't turn on the receiver.  Useful to leave the receiver
 *              on after a transmit, but without turning it on immediately.
 *
 * @param       flags - byte containing rx enable flags to set
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxSoftEnable(uint8 flags)
{
  halIntState_t  s;

  MAC_ASSERT(flags != 0); /* rx flags not affected */

  /* set the enable flags but do not turn on the receiver */
  HAL_ENTER_CRITICAL_SECTION(s);
  macRxEnableFlags |= flags;
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxDisable
 *
 * @brief       Clear indicated rx enable flags.  If all flags are clear, turn off receiver
 *              unless there is an active receive or transmit.
 *
 * @param       flags - byte containg rx enable flags to clear
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxDisable(uint8 flags)
{
  halIntState_t  s;

  MAC_ASSERT(flags != 0); /* rx flags not affected */

  /* clear the indicated flags */
  HAL_ENTER_CRITICAL_SECTION(s);
  macRxEnableFlags &= (flags ^ 0xFF);
  HAL_EXIT_CRITICAL_SECTION(s);

  /* turn off the radio if it is allowed */
  macRxOffRequest();
}


/**************************************************************************************************
 * @fn          macRxHardDisable
 *
 * @brief       Clear all enable flags and turn off receiver.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxHardDisable(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);

  macRxEnableFlags = 0;
  macRxOnFlag = 0;

  /* force receiver off */
  MAC_RADIO_RXTX_OFF();
  MAC_RADIO_FLUSH_RX_FIFO();
  MAC_DEBUG_TURN_OFF_RX_LED();

  HAL_EXIT_CRITICAL_SECTION(s);

  /* clean up after being forced off */
  macRxHaltCleanup();
}

/**************************************************************************************************
 * @fn          macRxOnRequest
 *
 * @brief       Turn on the receiver if any rx enable flag is set.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOnRequest(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (macRxEnableFlags)
  {
    macRxOn();
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxOffRequest
 *
 * @brief       Turn off receiver if permitted.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOffRequest(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (!macRxEnableFlags)
  {
    if (!MAC_RX_IS_PHYSICALLY_ACTIVE() && !MAC_TX_IS_PHYSICALLY_ACTIVE())
    {
      macRxOff();
    }
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxOn
 *
 * @brief       Turn on the receiver if it's not already on.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOn(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (!macRxOnFlag)
  {
    macRxOnFlag = 1;
    MAC_RADIO_RX_ON();
    MAC_DEBUG_TURN_ON_RX_LED();
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxOff
 *
 * @brief       Turn off the receiver if it's not already off.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOff(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (macRxOnFlag)
  {
    macRxOnFlag = 0;
    MAC_RADIO_RXTX_OFF();
    MAC_DEBUG_TURN_OFF_RX_LED();
    
    /* just in case a receive was about to start, flush the receive FIFO */
    MAC_RADIO_FLUSH_RX_FIFO();

    /* clear any receive interrupt that happened to squeak through */
    MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
*/
