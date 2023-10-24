/**************************************************************************************************
  Filename:       zmac_internal.h
  Revised:        $Date: 2014-07-07 17:47:52 -0700 (Mon, 07 Jul 2014) $
  Revision:       $Revision: 39365 $

  Description:    This file contains the ZStack MAC Porting Layer.


  Copyright 2005-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZMAC_INTERNAL_H
#define ZMAC_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/********************************************************************************************************
 *                                               INCLUDES
 ********************************************************************************************************/

#include "mac_api.h"

/********************************************************************************************************
 *                                                DEFINES
 ********************************************************************************************************/

// MAC Type Indication
#define ZMAC_F8W

// PHY transiver output power values
#define OUTPUT_POWER_0DBM       0x00
#define OUTPUT_POWER_N1DBM      0x21
#define OUTPUT_POWER_N3DBM      0x23
#define OUTPUT_POWER_N5DBM      0x25
#define OUTPUT_POWER_N7DBM      0x27
#define OUTPUT_POWER_N10DBM     0x2A
#define OUTPUT_POWER_N15DBM     0x2F
#define OUTPUT_POWER_N25DBM     0x39

// MAC PIB Attributes
enum
{
  ZMacAckWaitDuration                   = MAC_ACK_WAIT_DURATION,
  ZMacAssociationPermit                 = MAC_ASSOCIATION_PERMIT,
  ZMacAutoRequest                       = MAC_AUTO_REQUEST,
  ZMacBattLifeExt                       = MAC_BATT_LIFE_EXT,
  ZMacBattLeftExtPeriods                = MAC_BATT_LIFE_EXT_PERIODS,

  ZMacBeaconMSDU                        = MAC_BEACON_PAYLOAD,
  ZMacBeaconMSDULength                  = MAC_BEACON_PAYLOAD_LENGTH,
  ZMacBeaconOrder                       = MAC_BEACON_ORDER,
  ZMacBeaconTxTime                      = MAC_BEACON_TX_TIME,
  ZMacBSN                               = MAC_BSN,

  ZMacCoordExtendedAddress              = MAC_COORD_EXTENDED_ADDRESS,
  ZMacCoordShortAddress                 = MAC_COORD_SHORT_ADDRESS,
  ZMacDSN                               = MAC_DSN,
  ZMacGTSPermit                         = MAC_GTS_PERMIT,
  ZMacMaxCSMABackoffs                   = MAC_MAX_CSMA_BACKOFFS,

  ZMacMinBE                             = MAC_MIN_BE,
  ZMacPanId                             = MAC_PAN_ID,
  ZMacPromiscuousMode                   = MAC_PROMISCUOUS_MODE,
  ZMacRxOnIdle                          = MAC_RX_ON_WHEN_IDLE,
  ZMacShortAddress                      = MAC_SHORT_ADDRESS,

  ZMacSuperframeOrder                   = MAC_SUPERFRAME_ORDER,
  ZMacTransactionPersistenceTime        = MAC_TRANSACTION_PERSISTENCE_TIME,
  ZMacAssociatedPanCoord                = MAC_ASSOCIATED_PAN_COORD,
  ZMacMaxBE                             = MAC_MAX_BE,
  ZMacMaxFrameTotalWaitTime             = MAC_MAX_FRAME_TOTAL_WAIT_TIME,

  ZMacMaxFrameRetries                   = MAC_MAX_FRAME_RETRIES,
  ZMacResponseWaitTime                  = MAC_RESPONSE_WAIT_TIME,
  ZMacSyncSymbolOffset                  = MAC_SYNC_SYMBOL_OFFSET,
  ZMacTimestampSupported                = MAC_TIMESTAMP_SUPPORTED,
  ZMacSecurityEnabled                   = MAC_SECURITY_ENABLED,

  // Proprietary Items
  ZMacPhyTransmitPowerSigned            = MAC_PHY_TRANSMIT_POWER_SIGNED,
  ZMacChannel                           = MAC_LOGICAL_CHANNEL,
  ZMacExtAddr                           = MAC_EXTENDED_ADDRESS,
  ZMacAltBE                             = MAC_ALT_BE,
  ZMacDeviceBeaconOrder                 = MAC_DEVICE_BEACON_ORDER,
  ZMacRf4cePowerSavings                 = MAC_RF4CE_POWER_SAVINGS,
  ZMacFrameVersionSupport               = MAC_FRAME_VERSION_SUPPORT,
    
  // Diagnostics Items
  ZMacDiagsRxCrcPass                    = MAC_DIAGS_RX_CRC_PASS,
  ZMacDiagsRxCrcFail                    = MAC_DIAGS_RX_CRC_FAIL,
  ZMacDiagsRxBcast                      = MAC_DIAGS_RX_BCAST,
  ZMacDiagsTxBcast                      = MAC_DIAGS_TX_BCAST,
  ZMacDiagsRxUcast                      = MAC_DIAGS_RX_UCAST,
  ZMacDiagsTxUcast                      = MAC_DIAGS_TX_UCAST,
  ZMacDiagsTxUcastRetry                 = MAC_DIAGS_TX_UCAST_RETRY,
  ZMacDiagsTxUcastFail                  = MAC_DIAGS_TX_UCAST_FAIL,

#ifdef FEATURE_MAC_SECURITY
  ZMacKeyTable                          = MAC_KEY_TABLE,
  ZMacKeyTableEntries                   = MAC_KEY_TABLE_ENTRIES,
  ZMacDeviceTable                       = MAC_DEVICE_TABLE,
  ZMacDeviceTableEntries                = MAC_DEVICE_TABLE_ENTRIES,
  ZMacSecurityLevelTable                = MAC_SECURITY_LEVEL_TABLE,

  ZMacSecurityLevelTableEntries         = MAC_SECURITY_LEVEL_TABLE_ENTRIES,
  ZMacFrameCounter                      = MAC_FRAME_COUNTER,
  ZMacAutoRequestSecurityLevel          = MAC_AUTO_REQUEST_SECURITY_LEVEL,
  ZMacAutoRequestKeyIdMode              = MAC_AUTO_REQUEST_KEY_ID_MODE,
  ZMacAutoRequestKeySource              = MAC_AUTO_REQUEST_KEY_SOURCE,

  ZMacAutoRequestKeyIndex               = MAC_AUTO_REQUEST_KEY_INDEX,
  ZMacDefaultKeySource                  = MAC_DEFAULT_KEY_SOURCE,
  ZMacPanCoordExtendedAddress           = MAC_PAN_COORD_EXTENDED_ADDRESS,
  ZMacPanCoordShortAddress              = MAC_PAN_COORD_SHORT_ADDRESS,
  
  ZMacKeyIdLookupEntry                  = MAC_KEY_ID_LOOKUP_ENTRY,
  ZMacKeyDeviceEntry                    = MAC_KEY_DEVICE_ENTRY,
  ZMacKeyUsageEntry                     = MAC_KEY_USAGE_ENTRY,
  ZMacKeyEntry                          = MAC_KEY_ENTRY,
  ZMacDeviceEntry                       = MAC_DEVICE_ENTRY,
  ZMacSecurityLevelEntry                = MAC_SECURITY_LEVEL_ENTRY,
#endif /* FEATURE_MAC_SECURITY */

  // Junk
  ZMacACLDefaultSecurityMaterialLength  = 0,     // not implemented
  ZMacTxGTSId                           = 1,     // not implemented
  ZMacUpperLayerType                    = 2,     // not implemented
  ZMacRxGTSId                           = 3,     // not implemented
  ZMacSnoozePermit                      = 4      // not implemented
};

typedef uint8 ZMacAttributes_t;

// Status type
typedef uint8 ZMacStatus_t;

/* Definition of scan type */
#define ZMAC_ED_SCAN                   MAC_SCAN_ED
#define ZMAC_ACTIVE_SCAN               MAC_SCAN_ACTIVE
#define ZMAC_PASSIVE_SCAN              MAC_SCAN_PASSIVE
#define ZMAC_ORPHAN_SCAN               MAC_SCAN_ORPHAN

/* Adding Enhanced Active Scan request/ Enhanced beacon request */ 
#define ZMAC_ENHANCED_ACTIVE_SCAN      MAC_SCAN_ACTIVE_ENHANCED
// Association Status Field Values
#define ZMAC_SUCCESSFUL_ASSOCIATION    0x00
#define ZMAC_PAN_AT_CAPACITY           0x01
#define ZMAC_PAN_ACCESS_DENIED         0x02

// Disassociation Reason Codes
#define ZMAC_COORD_INITIATE            MAC_DISASSOC_COORD
#define ZMAC_DEVICE_INITIATE           MAC_DISASSOC_DEVICE

#define ZMAC_SECURITY_USE              0x01
#define ZMAC_ACL_ENTRY                 0x02
#define ZMAC_SECURITY_FAILURE          0x04

// TX Option flags
#define ZMAC_TXOPTION_ACK              MAC_TXOPTION_ACK
#define ZMAC_TXOPTION_GTS              MAC_TXOPTION_GTS
#define ZMAC_TXOPTION_INDIRECT         MAC_TXOPTION_INDIRECT
#define ZMAC_TXOPTION_SECURITY_ENABLE  MAC_TXOPTION_SECURITY
#define ZMAC_TXOPTION_NO_RETRANS       MAC_TXOPTION_NO_RETRANS
#define ZMAC_TXOPTION_NO_CNF           MAC_TXOPTION_NO_CNF
#define ZMAC_TXOPTION_GREEN_PWR        MAC_TXOPTION_GREEN_PWR

#define ZMAC_ASSOC_CAPINFO_FFD_TYPE    MAC_CAPABLE_FFD
#define ZMAC_ASSOC_CAPINFO_RX_ON_IDLE  MAC_CAPABLE_RX_ON_IDLE
#define ZMAC_ASSOC_CAPINFO_SECURITY    MAC_CAPABLE_SECURITY

// LQI to Cost mapping
#define MIN_LQI_COST_1  12     //24
#define MIN_LQI_COST_2  9      //20
#define MIN_LQI_COST_3  6      //16
#define MIN_LQI_COST_4  4      //12
#define MIN_LQI_COST_5  2      //8
#define MIN_LQI_COST_6  1      //4
#define MIN_LQI_COST_7  0      //0

/* Number of bytes to allocate for ED scan; matches ED_SCAN_MAXCHANNELS in nwk.h */
#define ZMAC_ED_SCAN_MAXCHANNELS       27

#define ZMAC_SUCCESS                   MAC_SUCCESS
#define ZMAC_TRANSACTION_OVERFLOW      MAC_TRANSACTION_OVERFLOW
#define ZMAC_TRANSACTION_EXPIRED       MAC_TRANSACTION_EXPIRED
#define ZMAC_CHANNEL_ACCESS_FAILURE    MAC_CHANNEL_ACCESS_FAILURE
#define ZMAC_NO_RESOURCES              MAC_NO_RESOURCES
#define ZMAC_NO_ACK                    MAC_NO_ACK
#define ZMAC_COUNTER_ERROR             MAC_COUNTER_ERROR
#define ZMAC_INVALID_PARAMETER         MAC_INVALID_PARAMETER

#define ZMAC_KEY_SOURCE_MAX_LEN        MAC_KEY_SOURCE_MAX_LEN

/********************************************************************************************************
 *                                            TYPE DEFINITIONS
 ********************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZMAC_INTERNAL_H */
