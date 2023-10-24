/**************************************************************************************************
  Filename:       mac_pib.h
  Revised:        $Date: 2014-11-06 11:03:55 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41021 $

  Description:    Internal interface file for the MAC PIB module.


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

#ifndef MAC_PIB_H
#define MAC_PIB_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "mac_api.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* MAC PIB type */
typedef struct
{
  uint8             ackWaitDuration;
  bool              associationPermit;
  bool              autoRequest;
  bool              battLifeExt;
  uint8             battLifeExtPeriods;

  uint8             *pBeaconPayload;
  uint8             beaconPayloadLength;
  uint8             beaconOrder;
  uint32            beaconTxTime;
  uint8             bsn;

  sAddr_t           coordExtendedAddress;
  uint16            coordShortAddress;
  uint8             dsn;
  bool              gtsPermit;
  uint8             maxCsmaBackoffs;

  uint8             minBe;
  uint16            panId;
  bool              promiscuousMode;
  bool              rxOnWhenIdle;
  uint16            shortAddress;

  uint8             superframeOrder;
  uint16            transactionPersistenceTime;
  bool              associatedPanCoord;
  uint8             maxBe;
  uint16            maxFrameTotalWaitTime;

  uint8             maxFrameRetries;
  uint8             responseWaitTime;
  uint8             syncSymbolOffset;
  bool              timeStampSupported;
  bool              securityEnabled;

  /* Proprietary */
  uint8             phyTransmitPower;
  uint8             logicalChannel;
  sAddr_t           extendedAddress;
  uint8             altBe;
  uint8             deviceBeaconOrder;
  uint8             rf4cepowerSavings;
  uint8             frameVersionSupport;
  
  /* Diagnostics */
  uint32            diagsRxCrcPass;
  uint32            diagsRxCrcFail;
  uint32            diagsRxBcast;
  uint32            diagsTxBcast;
  uint32            diagsRxUcast;
  uint32            diagsTxUcast;
  uint32            diagsTxUcastRetry;
  uint32            diagsTxUcastFail;
} macPib_t;


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* MAC PIB */
extern macPib_t macPib;

/* Pointer to the MAC PIB */
#if defined( FEATURE_MAC_PIB_PTR )
extern macPib_t* pMacPib;
#else
#define pMacPib (&macPib)
#endif /* FEATURE_DUAL_MAC_PIB */

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macPibReset(void);
MAC_INTERNAL_API uint8 macPibIndex(uint8 pibAttribute);

#if defined (FEATURE_MAC_PIB_PTR) && defined (CC26XX)
#error "ERROR! CC26XX does not support FEATURE_MAC_PIB_PTR."
#endif

/**************************************************************************************************
*/

#endif /* MAC_PIB_H */

