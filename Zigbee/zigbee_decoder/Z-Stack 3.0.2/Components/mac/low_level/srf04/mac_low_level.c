/**************************************************************************************************
  Filename:       mac_low_level.c
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
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"
#include "hal_mcu.h"

#include "mac_main.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_radio.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_backoff_timer.h"
#include "mac_sleep.h"

/* high level */
#include "mac_pib.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"

/* turn on additional power savings */
#define MAC_EXTRA_POWER_SAVINGS  1

#if !defined ( HAL_CLOCK_CRYSTAL )
/* DCO has a frequency error of 2% */
const uint16 CODE macBeaconMarginDCO[] =
{
  3,
  3,
  4,
  8,
  16,
  31,
  62,
  123,
  246,
  492,
  983,
  1967,
  3933,
  7865,
  15729,
  0
};
#endif

/**************************************************************************************************
 * @fn          macLowLevelInit
 *
 * @brief       Initialize low-level MAC.  Called only once on system power-up.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelInit(void)
{
  /* initialize mcu before anything else */
  MAC_RADIO_MCU_INIT();

  /* initialize (override) beacon margin timing table */
#if !defined ( HAL_CLOCK_CRYSTAL )
  {
    uint8 i;

    for (i = 0; i < sizeof(macBeaconMarginDCO)/sizeof(macBeaconMarginDCO[0]); i++)
    {
      macBeaconMargin[i] = macBeaconMarginDCO[i];
    }
  }
#endif

  /* software initialziation */
  macRadioInit();
  macRxOnOffInit();
  macRxInit();
  macTxInit();
  macBackoffTimerInit();
}


/**************************************************************************************************
 * @fn          macLowLevelReset
 *
 * @brief       Reset low-level MAC.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelReset(void)
{
  MAC_ASSERT(!HAL_INTERRUPTS_ARE_ENABLED());   /* interrupts must be disabled */

  /* reset radio modules;  if not awake, skip this */
  if (macSleepState == MAC_SLEEP_STATE_AWAKE)
  {
    macRxTxReset();
    macRadioReset();
  }

  /* reset timer */
  macBackoffTimerReset();

  /* power up the radio */
  macSleepWakeUp();
  
  /* Turn on extra power savings*/
  pMacPib->rf4cepowerSavings = MAC_EXTRA_POWER_SAVINGS;
}


/**************************************************************************************************
 * @fn          macLowLevelResume
 *
 * @brief       Resume the low-level MAC after a successful return from macLowLevelYield.
 *              Note: assuming interrupts are disabled.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelResume(void)
{
  macRadioInit();
}


/**************************************************************************************************
 * @fn          macLowLevelYield
 *
 * @brief       Check whether or not the low-level MAC is ready to yield.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE or FALSE whether the low-level MAC is ready to yield.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macLowLevelYield(void)
{
  bool rtrn = TRUE;
  halIntState_t  s;
  HAL_ENTER_CRITICAL_SECTION(s);

  // If RX or TX is active or any RX enable flags are set, it's not OK to yield.
  if (macRxActive || macRxOutgoingAckFlag || macTxActive || (macRxEnableFlags & ~MAC_RX_WHEN_IDLE))
  {
    rtrn = FALSE;
  }

  HAL_EXIT_CRITICAL_SECTION(s);
  return rtrn;
}


/**************************************************************************************************
 * @fn          macLowLevelDiags
 *
 * @brief       Increments a specified diagnostic counter (stored in the PIB).
 *
 * @param       pibAttribute - PIB attribute to be incremented.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelDiags( uint8 pibAttribute )
{
#if defined ( FEATURE_SYSTEM_STATS )
  if ( ( pibAttribute >= MAC_DIAGS_RX_CRC_PASS ) &&
       ( pibAttribute <= MAC_DIAGS_TX_UCAST_FAIL ) )
  {
    uint32 value;

    /* Update Diagnostics counter */
    MAC_MlmeGetReq( pibAttribute, &value );
    value++;
    MAC_MlmeSetReq( pibAttribute, &value );
  }
#endif
}


/**************************************************************************************************
*/
