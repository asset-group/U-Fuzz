/**************************************************************************************************
  Filename:       bdb_touchlink.h
  Revised:        $Date: 2013-08-30 16:09:11 -0700 (Fri, 30 Aug 2013) $
  Revision:       $Revision: 35197 $

  Description:    This file contains the Zigbee Cluster Library: Light Link
                  Profile definitions.


  Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef TOUCHLINK_H
#define TOUCHLINK_H

#ifdef __cplusplus
extern "C"
{
#endif

#if ( SECURE != TRUE )
#error SECURE must be globally defined to TRUE.
#endif
/*********************************************************************
 * INCLUDES
 */

#include "bdb_tlCommissioning.h"
#include "ZDProfile.h"
   
/*********************************************************************
 * CONSTANTS
 */

// Reason for Leave command initiation
#define TOUCHLINK_LEAVE_TO_JOIN_NWK                              1
#define TOUCHLINK_LEAVE_TO_START_NWK                             2

//Scan modes
#define TOUCHLINK_SCAN_PRIMARY_CHANNELS                          1
#define TOUCHLINK_SCAN_SECONDARY_CHANNELS                        2
#define TOUCHLINK_SCAN_FOUND_NOTHING                             3

#define APLC_MAX_PERMIT_JOIN_DURATION                            60
   
#define APL_FREE_NWK_ADDR_RANGE_BEGIN                            0x0001
#define APL_FREE_NWK_ADDR_RANGE_END                              0xFFF7
#define APL_FREE_ADDR_THRESHOLD                                  10

#define APL_FREE_GROUP_ID_RANGE_BEGIN                            0x0001
#define APL_FREE_GROUP_ID_RANGE_END                              0xFEFF
#define APL_FREE_GROUP_ID_THRESHOLD                              10

#define TOUCHLINK_UPDATE_NV_NIB                                  0x01
#define TOUCHLINK_UPDATE_NV_RANGES                               0x02


// Only for development
#define No_Ch_offset                                             0 
#define Ch_Plus_1                                                1
#define Ch_Plus_2                                                2
#define Ch_Plus_3                                                3  

/*********************************************************************
 * MACROS
 */
#define TOUCHLINK_PRIMARY_CHANNEL( ch )              ( (ch) == TOUCHLINK_FIRST_CHANNEL  || \
                                                 (ch) == TOUCHLINK_SECOND_CHANNEL || \
                                                 (ch) == TOUCHLINK_THIRD_CHANNEL  || \
                                                 (ch) == TOUCHLINK_FOURTH_CHANNEL )

#define TOUCHLINK_SAME_NWK( panId, ePanId )          ( ( _NIB.nwkPanId == (panId) ) && \
                                                 osal_ExtAddrEqual( _NIB.extendedPANID, (ePanId) ) )

/*********************************************************************
 * TYPEDEFS
 */
typedef struct nwkParam
{
  struct nwkParam *nextParam;
  uint16 PANID;
  uint8 logicalChannel;
  uint8 extendedPANID[Z_EXTADDR_LEN];
  uint16 chosenRouter;
  uint8 chosenRouterLinkQuality;
  uint8 chosenRouterDepth;
  uint8 routerCapacity;
  uint8 deviceCapacity;
} touchLinkDiscoveredNwkParam_t;

typedef union
{
  bdbTLNwkStartReq_t nwkStartReq;
  bdbTLNwkJoinReq_t nwkJoinReq;
} bdbTLReq_t;

/*********************************************************************
 * VARIABLES
 */
extern uint32 touchLinkLastAcceptedTransID;

extern uint32 touchLinkResponseID;
extern uint32 touchLinkTransID;

extern uint16 touchLinkGrpIDsBegin;
extern uint16 touchLinkGrpIDsEnd;

extern uint8 touchLinkLeaveInitiated;

extern touchLinkDiscoveredNwkParam_t *pDiscoveredNwkParamList;

extern bool touchlinkDistNwk;
extern bool touchlinkFNReset;

/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Initialize the TOUCHLINK global and local variables
 */
void touchLink_InitVariables( bool initiator );

/*
 * Get the total number of sub-devices (endpoints) registered
 */
uint8 touchLink_GetNumSubDevices( uint8 startIndex );

/*
 * Get the total number of group IDs required by this device
 */
uint8 touchLink_GetNumGrpIDs( void );

/*
 * Get the sub-device information
 */
void touchLink_GetSubDeviceInfo( uint8 index, bdbTLDeviceInfo_t *pInfo );

/*
 * Copy new Network Parameters to the NIB
 */
void touchLink_SetNIB( nwk_states_t nwkState, uint16 nwkAddr, uint8 *pExtendedPANID,
                        uint8 logicalChannel, uint16 panId, uint8 nwkUpdateId );

/*
 * Updates NV with NIB and free ranges items
 */
void touchLink_UpdateNV( uint8 enables );

/*
 * Set our channel
 */
void touchLink_SetChannel( uint8 newChannel );

/*
 * Encrypt the network key to be sent to the Target
 */
void touchLink_EncryptNwkKey( uint8 *pNwkKey, uint8 keyIndex, uint32 transID, uint32 responseID );

/*
 * Decrypt the network key received from the Initiator
 */
void touchLink_DecryptNwkKey( uint8 *pNwkKey, uint8 keyIndex, uint32 transID, uint32 responseID );

/*
 * Fill buffer with random bytes
 */
void touchLink_GenerateRandNum( uint8 *pNum, uint8 numSize );

/*
 * Get randomly chosen TOUCHLINK primary channel
 */
uint8 touchLink_GetRandPrimaryChannel( void );

/*
 * Get the supported network key bitmask
 */
uint16 touchLink_GetNwkKeyBitmask( void );

/*
 * Update our local network update id and logical channel
 */
void touchLink_ProcessNwkUpdate( uint8 nwkUpdateId, uint8 logicalChannel );

/*
 * Configure MAC with our Network Parameters
 */
void touchLink_SetMacNwkParams( uint16 nwkAddr, uint16 panId, uint8 channel );

/*
 * Send out a Device Information Response command
 */
uint8 touchLink_SendDeviceInfoRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 startIndex,
                                    uint32 transID, uint8 seqNum );

/*
 * Send out a Leave Request command
 */
ZStatus_t touchLink_SendLeaveReq( void );

/*
 * Pop an avaialble short address out of the free network addresses range
 */
uint16 touchLink_PopNwkAddress( void );

/*
 * Update the TOUCHLINK free range global variables
 */
void touchLink_UpdateFreeRanges( bdbTLNwkParams_t *pParams );

/*
 * Get the TOUCHLINK free range global variables
 */
void touchLink_GerFreeRanges( bdbTLNwkParams_t *pParams );

/*
 * Checks to see if the free ranges can be split
 */
bool touchLink_IsValidSplitFreeRanges( uint8 totalGrpIDs );

/*
 * Split our own free network address and group ID ranges
 */
void touchLink_SplitFreeRanges( uint16 *pAddrBegin, uint16 *pAddrEnd,
                               uint16 *pGrpIdBegin, uint16 *pGrpIdEnd );

/*
 * Pop the requested number of group IDs out of the free group IDs range.
 */
void touchLink_PopGrpIDRange( uint8 numGrpIDs, uint16 *pGrpIDsBegin, uint16 *pGrpIDsEnd );

/*
 * Get the RSSI of the message just received through a ZCL callback
 */
int8 touchLink_GetMsgRssi( void );

/*
 * Determine the new network update id.
 */
uint8 touchLink_NewNwkUpdateId( uint8 ID1, uint8 ID2 );

/*
 * Update the network key.
 */
void touchLink_UpdateNwkKey( uint8 *pNwkKey, uint8 keyIndex );

/*
 * Register Target/Initiator taskID for commissioning events
 */
void touchLink_SetTouchLinkTaskId( uint8 taskID );

/*
 * Perform a Network Discovery scan
 */
void touchLink_PerformNetworkDisc( uint32 scanChannelList );

/*
 * Free any network discovery data
 */
void touchLink_FreeNwkParamList( void );

/*
 * Transaction ID Filter for Touch-Link received commands
 */
bool touchLink_IsValidTransID( uint32 transID );

/*
 * Process incoming ZDO messages (for routers)
 */
void touchLink_RouterProcessZDOMsg( zdoIncomingMsg_t *inMsg );

/*
 * Set the router permit join flag
 */
ZStatus_t touchLink_PermitJoin( uint8 duration );

/*
 * @brief   Perform a network rejoin
 */
void touchLink_DevRejoin( bdbTLNwkRejoin_t *rejoinInf );

/*
 * To request a network start
 */
void touchLinkStartRtr( bdbTLNwkParams_t *pParams, uint32 transID );

/*
 * Set the touchlink initiator flag
 */
void touchLink_DeviceIsInitiator( bool initiator );

/*
 * Get the touchlink initiator flag
 */
bool touchLink_GetDeviceInitiator( void );

/*
 * @brief   Initialize the TOUCHLINK free range global variables.
 */
void touchLink_InitFreeRanges( bool initiator );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* TOUCHLINK_H */
