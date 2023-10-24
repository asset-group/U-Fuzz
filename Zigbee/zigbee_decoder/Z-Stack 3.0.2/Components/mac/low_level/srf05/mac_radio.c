/**************************************************************************************************
  Filename:       mac_radio.c
  Revised:        $Date: 2014-10-07 12:55:35 -0700 (Tue, 07 Oct 2014) $
  Revision:       $Revision: 40482 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac_sleep.h"
#include "mac_backoff_timer.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#define ED_RF_POWER_MIN_DBM   (MAC_RADIO_RECEIVER_SENSITIVITY_DBM + MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY)
#define ED_RF_POWER_MAX_DBM   MAC_RADIO_RECEIVER_SATURATION_DBM


/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macPhyTxPower;
uint8 macPhyChannel;


/* ------------------------------------------------------------------------------------------------
 *                                        Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8 reqChannel;
static uint8 reqTxPower;


/* ------------------------------------------------------------------------------------------------
 *                                        Local Functions
 * ------------------------------------------------------------------------------------------------
 */
static uint8 radioComputeED(int8 rssiDbm);


/**************************************************************************************************
 * @fn          macRadioInit
 *
 * @brief       Initialize radio software.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioInit(void)
{
  /* variable initialization for this module */
  reqChannel    = MAC_RADIO_CHANNEL_DEFAULT;
  macPhyChannel = MAC_RADIO_CHANNEL_INVALID;
  reqTxPower    = MAC_RADIO_TX_POWER_INVALID;
  macPhyTxPower = MAC_RADIO_TX_POWER_INVALID;
}


/**************************************************************************************************
 * @fn          macRadioReset
 *
 * @brief       Resets the radio module.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioReset(void)
{
  macRadioStopScan();
  macRadioEnergyDetectStop();
}


/**************************************************************************************************
 * @fn          macRadioRandomByte
 *
 * @brief       Return a random byte derived from previously set random seed.
 *
 * @param       none
 *
 * @return      a random byte
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macRadioRandomByte(void)
{
  return(MAC_RADIO_RANDOM_BYTE());
}


/**************************************************************************************************
 * @fn          macRadioSetPanCoordinator
 *
 * @brief       Configure the pan coordinator status of the radio
 *
 * @param       panCoordFlag - non-zero to configure radio to be pan coordinator
 *                             zero to configure radio as NON pan coordinator
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetPanCoordinator(uint8 panCoordFlag)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_PAN_COORDINATOR(panCoordFlag);
}


/**************************************************************************************************
 * @fn          macRadioSetPanID
 *
 * @brief       Set the pan ID on the radio.
 *
 * @param       panID - 16 bit PAN identifier
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetPanID(uint16 panID)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_PAN_ID(panID);
}


/**************************************************************************************************
 * @fn          macRadioSetShortAddr
 *
 * @brief       Set the short addrss on the radio.
 *
 * @param       shortAddr - 16 bit short address
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetShortAddr(uint16 shortAddr)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_SHORT_ADDR(shortAddr);
}


/**************************************************************************************************
 * @fn          macRadioSetIEEEAddr
 *
 * @brief       Set the IEEE address on the radio.
 *
 * @param       pIEEEAddr - pointer to array holding 64 bit IEEE address; array must be little
 *                          endian format (starts with lowest signficant byte)
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetIEEEAddr(uint8 * pIEEEAddr)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_IEEE_ADDR(pIEEEAddr);
}


/**************************************************************************************************
 * @fn          macRadioSetTxPower
 *
 * @brief       Set transmitter power of the radio.
 *
 * @param       txPower - the minus dBm for power but as a postive integer (or if configured
 *                        for it, txPower is the raw register value). If PA/LNA is installed
 *                        then txPower becomes positive dBm.
 *
 * @return      The minus dBm for power actually set according to what is possible according to
                the build and run-time configuration set.
 **************************************************************************************************
 */
#ifndef HAL_MAC_USE_REGISTER_POWER_VALUES
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

MAC_INTERNAL_API uint8 macRadioSetTxPower(uint8 txPower)
{
  halIntState_t  s;
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
  defined MAC_RUNTIME_CC2592
  const uint8 CODE *pTable = macRadioDefsTxPwrTables[macRadioDefsRefTableId >> 4];
#elif defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
  defined HAL_PA_LNA_CC2592
  const uint8 CODE *pTable = macRadioDefsTxPwrTables[0];
#else
  const uint8 CODE *pTable = macRadioDefsTxPwrBare;
#endif

  /* if the selected dBm is out of range, use the closest available */
  if ((int8)txPower > (int8)pTable[MAC_RADIO_DEFS_TBL_TXPWR_FIRST_ENTRY])
  {
    /* greater than base value -- out of table range */
    txPower = pTable[MAC_RADIO_DEFS_TBL_TXPWR_FIRST_ENTRY];
  }
  else if ((int8)txPower < (int8)pTable[MAC_RADIO_DEFS_TBL_TXPWR_LAST_ENTRY])
  {
    /* smaller than the lowest power level -- out of table range */
    txPower = pTable[MAC_RADIO_DEFS_TBL_TXPWR_LAST_ENTRY];
  }

  /*
   *  Set the global variable reqTxPower.  This variable is referenced
   *  by the function macRadioUpdateTxPower() to write the radio register.
   *
   *  A lookup table is used to translate the power level to the register
   *  value.
   */
  HAL_ENTER_CRITICAL_SECTION(s);
  /* When calculating index to the power register value table,
   * either txPower (of uint8 type) has to be explicitly type-casted to int8
   * or the subtraction expression has to be type-casted to uint8 to work
   * with the integral promotions.
   * The latter is more code size efficient and hence the latter is used.
   */
  {
    uint8 index = pTable[MAC_RADIO_DEFS_TBL_TXPWR_FIRST_ENTRY] - txPower
      + MAC_RADIO_DEFS_TBL_TXPWR_ENTRIES;
    reqTxPower = pTable[index];
  }
  HAL_EXIT_CRITICAL_SECTION(s);

  /* update the radio power setting */
  macRadioUpdateTxPower();
  return txPower;
}

#else
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

MAC_INTERNAL_API uint8 macRadioSetTxPower(uint8 txPower)
{
  halIntState_t  s;

  /* same as above but with no lookup table, use raw register value */
  HAL_ENTER_CRITICAL_SECTION(s);
  reqTxPower = txPower;
  HAL_EXIT_CRITICAL_SECTION(s);

  /* update the radio power setting */
  macRadioUpdateTxPower();
  return txPower;
}

#endif


/**************************************************************************************************
 * @fn          macRadioUpdateTxPower
 *
 * @brief       Update the radio's transmit power if a new power level has been requested
 *
 * @param       reqTxPower - file scope variable that holds the last request power level
 *              macPhyTxPower - global variable that holds radio's set power level
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioUpdateTxPower(void)
{
  halIntState_t  s;

  /*
   *  If the requested power setting is different from the actual radio setting,
   *  attempt to udpate to the new power setting.
   */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (reqTxPower != macPhyTxPower)
  {
    /*
     *  Radio power cannot be updated when the radio is physically transmitting.
     *  If there is a possibility radio is transmitting, do not change the power
     *  setting.  This function will be called again after the current transmit
     *  completes.
     */
    if (!macRxOutgoingAckFlag && !MAC_TX_IS_PHYSICALLY_ACTIVE())
    {
      /*
       *  Set new power level;  update the shadow value and write
       *  the new value to the radio hardware.
       */
      macPhyTxPower = reqTxPower;
      MAC_RADIO_SET_TX_POWER(macPhyTxPower);
    }
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRadioSetChannel
 *
 * @brief       Set radio channel.
 *
 * @param       channel - channel number, valid range is 11 through 26. Allow
 *              channels 27 and 28 for some Japanese customers.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSetChannel(uint8 channel)
{
  halIntState_t  s;

  MAC_ASSERT((channel >= 11) && (channel <= 28));  /* illegal channel */

  /* critical section to make sure transmit does not start while updating channel */
  HAL_ENTER_CRITICAL_SECTION(s);

  /* set requested channel */
  reqChannel = channel;

  /*
   *  If transmit is not active, update the radio hardware immediately.  If transmit is active,
   *  the channel will be updated at the end of the current transmit.
   */
  if (!macTxActive)
  {
    macRadioUpdateChannel();
  }

  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRadioUpdateChannel
 *
 * @brief       Update the radio channel if a new channel has been requested.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioUpdateChannel(void)
{
  halIntState_t  s;

  MAC_ASSERT(!macTxActive); /* cannot change channel during a transmit */

  /* if the channel has changed, set the radio to the new channel */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (reqChannel != macPhyChannel)
  {
    macPhyChannel = reqChannel;
    HAL_EXIT_CRITICAL_SECTION(s);

    /* changing the channel stops any receive in progress */
    macRxOff();
    MAC_RADIO_SET_CHANNEL(macPhyChannel);

    /* If the channel is updated in the middle of receiving a frame, we must
     * clean up the Rx logic.
     */
    macRxHaltCleanup();

    macRxOnRequest();
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}


/**************************************************************************************************
 * @fn          macRadioStartScan
 *
 * @brief       Puts radio into selected scan mode.
 *
 * @param       scanMode - scan mode, see #defines in .h file
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioStartScan(uint8 scanMode)
{
  MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_AWAKE); /* radio must be awake */
  MAC_ASSERT(macRxFilter == RX_FILTER_OFF); /* all filtering must be off to start scan */

  /* set the receive filter based on the selected scan mode */
  if (scanMode == MAC_SCAN_ED)
  {
    macRxFilter = RX_FILTER_ALL;
  }
  else if (scanMode == MAC_SCAN_ORPHAN)
  {
    macRxFilter = RX_FILTER_NON_COMMAND_FRAMES;
  }
  else
  {
#ifdef FEATURE_ENHANCED_BEACON
    MAC_ASSERT((scanMode == MAC_SCAN_ACTIVE_ENHANCED) || (scanMode == MAC_SCAN_ACTIVE) ||
               (scanMode == MAC_SCAN_PASSIVE)); /* invalid scan type */
#else
    MAC_ASSERT((scanMode == MAC_SCAN_ACTIVE) ||
               (scanMode == MAC_SCAN_PASSIVE)); /* invalid scan type */
#endif

    macRxFilter = RX_FILTER_NON_BEACON_FRAMES;

    /* for active and passive scans, per spec the pan ID must be 0xFFFF */
    MAC_RADIO_SET_PAN_ID(0xFFFF);
  }
}


/**************************************************************************************************
 * @fn          macRadioStopScan
 *
 * @brief       Takes radio out of scan mode.  Note can be called if
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioStopScan(void)
{
  macRxFilter = RX_FILTER_OFF;

  /* restore the pan ID (passive and active scans set pan ID to 0xFFFF) */
  MAC_RADIO_SET_PAN_ID(pMacPib->panId);
}


/**************************************************************************************************
 * @fn          macRadioEnergyDetectStart
 *
 * @brief       Initiates energy detect.  The highest energy detected is recorded from the time
 *              when this function is called until the energy detect is stopped.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioEnergyDetectStart(void)
{
  MAC_RADIO_RECORD_MAX_RSSI_START();
}


/**************************************************************************************************
 * @fn          macRadioEnergyDetectStop
 *
 * @brief       Called at completion of an energy detect.  Note: can be called even if energy
 *              detect is already stopped (needed by reset).
 *
 * @param       none
 *
 * @return      highest energy detect measurement
 **************************************************************************************************
 */
uint8 macRadioEnergyDetectStop(void)
{
  uint8 rssiDbm;
  uint8 energyDetectMeasurement;

  rssiDbm = MAC_RADIO_RECORD_MAX_RSSI_STOP() + MAC_RADIO_RSSI_OFFSET;
  MAC_RADIO_RSSI_LNA_OFFSET(rssiDbm);
  energyDetectMeasurement = radioComputeED(rssiDbm);

  return(energyDetectMeasurement);
}

/*=================================================================================================
 * @fn          radioComputeED
 *
 * @brief       Compute energy detect measurement.
 *
 * @param       rssi - raw RSSI value from radio hardware
 *
 * @return      energy detect measurement in the range of 0x00-0xFF
 *=================================================================================================
 */
static uint8 radioComputeED(int8 rssiDbm)
{
  uint8 ed;

  /*
   *  Keep RF power between minimum and maximum values.
   *  This min/max range is derived from datasheet and specification.
   */
  if (rssiDbm < ED_RF_POWER_MIN_DBM)
  {
    rssiDbm = ED_RF_POWER_MIN_DBM;
  }
  else if (rssiDbm > ED_RF_POWER_MAX_DBM)
  {
    rssiDbm = ED_RF_POWER_MAX_DBM;
  }

  /*
   *  Create energy detect measurement by normalizing and scaling RF power level.
   *
   *  Note : The division operation below is designed for maximum accuracy and
   *         best granularity.  This is done by grouping the math operations to
   *         compute the entire numerator before doing any division.
   */
  ed = (MAC_SPEC_ED_MAX * (rssiDbm - ED_RF_POWER_MIN_DBM)) / (ED_RF_POWER_MAX_DBM - ED_RF_POWER_MIN_DBM);

  return(ed);
}


/**************************************************************************************************
 * @fn          macRadioComputeLQI
 *
 * @brief       Compute link quality indication.
 *
 * @param       rssi - raw RSSI value from radio hardware
 *              corr - correlation value from radio hardware
 *
 * @return      link quality indicator value
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macRadioComputeLQI(int8 rssiDbm, uint8 corr)
{
  (void) corr; /* suppress compiler warning of unused parameter */

  /*
   *  Note : Currently the LQI value is simply the energy detect measurement.
   *         A more accurate value could be derived by using the correlation
   *         value along with the RSSI value.
   */
  return(radioComputeED(rssiDbm));
}


/**************************************************************************************************
*/
