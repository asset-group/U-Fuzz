/**************************************************************************************************
  Filename:       mac_low_level.h
  Revised:        $Date: 2014-03-13 10:50:10 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37663 $

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

#ifndef MAC_LOW_LEVEL_H
#define MAC_LOW_LEVEL_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "mac_high_level.h"


 /* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* identifies low-level code as specific to Chipcon SmartRF03(tm) technology */
#define MAC_LOW_LEVEL_SMARTRF04

#define MAC_BACKOFF_TIMER_DEFAULT_ROLLOVER  (((uint32) MAC_A_BASE_SUPERFRAME_DURATION) << 14)

/*
 * Since T2MOVF is 24 bit so the value can't exceed 24 bit.
 * so choosing 0xFF0000 a  value which is a 4 byte aligned
 */
#define MAC_BACKOFF_TIMER_DEFAULT_NONBEACON_ROLLOVER  0xFF0000

/* macTxFrame() parameter values for txType */
#define MAC_TX_TYPE_SLOTTED_CSMA            0x00
#define MAC_TX_TYPE_UNSLOTTED_CSMA          0x01
#define MAC_TX_TYPE_SLOTTED                 0x02
#define MAC_TX_TYPE_GREEN_POWER             0x03

/* macSleep() parameter values for sleepState */
#define MAC_SLEEP_STATE_OSC_OFF             0x01
#define MAC_SLEEP_STATE_RADIO_OFF           0x02

/* macRxPromiscuousMode() parameter values */
#define MAC_PROMISCUOUS_MODE_OFF            0x00  /* must be zero; reserved for boolean use */
#define MAC_PROMISCUOUS_MODE_COMPLIANT      0x01
#define MAC_PROMISCUOUS_MODE_WITH_BAD_CRC   0x02


/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 const macTxSlottedDelay;

/* beacon interval margin */
extern uint16 macBeaconMargin[];


/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* mac_low_level.c */
MAC_INTERNAL_API void macLowLevelInit(void);
MAC_INTERNAL_API void macLowLevelReset(void);
MAC_INTERNAL_API void macLowLevelResume(void);
MAC_INTERNAL_API bool macLowLevelYield(void);
MAC_INTERNAL_API void macLowLevelDiags(uint8 pibAttribute);

/* mac_sleep.c */
MAC_INTERNAL_API void macSleepWakeUp(void);
MAC_INTERNAL_API uint8 macSleep(uint8 sleepState);

/* mac_radio.c */
MAC_INTERNAL_API uint8 macRadioRandomByte(void);
MAC_INTERNAL_API void macRadioSetPanCoordinator(uint8 panCoordinator);
MAC_INTERNAL_API void macRadioSetPanID(uint16 panID);
MAC_INTERNAL_API void macRadioSetShortAddr(uint16 shortAddr);
MAC_INTERNAL_API void macRadioSetIEEEAddr(uint8 * pIEEEAddr);
MAC_INTERNAL_API uint8 macRadioSetTxPower(uint8 txPower);
MAC_INTERNAL_API void macRadioSetChannel(uint8 channel);
MAC_INTERNAL_API void macRadioStartScan(uint8 scanType);
MAC_INTERNAL_API void macRadioStopScan(void);
void macRadioEnergyDetectStart(void);
uint8 macRadioEnergyDetectStop(void);

/* mac_backoff_timer.c */
MAC_INTERNAL_API void macBackoffTimerSetRollover(uint32 rolloverBackoff);
MAC_INTERNAL_API void macBackoffTimerSetCount(uint32 backoff);
MAC_INTERNAL_API uint32 macBackoffTimerCount(void);
MAC_INTERNAL_API uint32 macBackoffTimerGetTrigger(void);
MAC_INTERNAL_API void macBackoffTimerSetTrigger(uint32 triggerBackoff);
MAC_INTERNAL_API void macBackoffTimerCancelTrigger(void);
MAC_INTERNAL_API void macBackoffTimerTriggerCallback(void);
MAC_INTERNAL_API void macBackoffTimerRolloverCallback(void);
MAC_INTERNAL_API int32 macBackoffTimerRealign(macRx_t *pMsg);

/* mac_tx.c */
MAC_INTERNAL_API void macTxFrame(uint8 txType);
MAC_INTERNAL_API void macTxFrameRetransmit(void);
MAC_INTERNAL_API void macTxCompleteCallback(uint8 status);

/* mac_rx.c */
MAC_INTERNAL_API bool macRxCheckPendingCallback(void);
MAC_INTERNAL_API bool macRxCheckMACPendingCallback(void);
MAC_INTERNAL_API void macRxCompleteCallback(macRx_t * pMsg);
MAC_INTERNAL_API void macRxPromiscuousMode(uint8 mode);

/* mac_rx_onoff.c */
MAC_INTERNAL_API void macRxEnable(uint8 flags);
MAC_INTERNAL_API void macRxSoftEnable(uint8 flags);
MAC_INTERNAL_API void macRxDisable(uint8 flags);
MAC_INTERNAL_API void macRxHardDisable(void);

/**************************************************************************************************
 */
#endif
