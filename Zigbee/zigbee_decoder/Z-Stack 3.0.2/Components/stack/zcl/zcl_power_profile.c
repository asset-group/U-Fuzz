/**************************************************************************************************
  Filename:       zcl_power_profile.c
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    Zigbee Cluster Library - Power Profile


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_power_profile.h"

#if defined ( INTER_PAN )
#include "stub_aps.h"
#endif

#ifdef ZCL_POWER_PROFILE
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclPowerProfileCBRec
{
  struct zclPowerProfileCBRec *next;
  uint8 endpoint;                          // Used to link it into the endpoint descriptor
  zclPowerProfile_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclPowerProfileCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclPowerProfileCBRec_t *zclPowerProfileCBs = (zclPowerProfileCBRec_t *)NULL;
static uint8 zclPowerProfilePluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclPowerProfile_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclPowerProfile_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclPowerProfile_AppCallbacks_t *zclPowerProfile_FindCallbacks( uint8 endpoint );
static ZStatus_t zclPowerProfile_ProcessInCmds( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );

static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileReq( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileStateReq( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePriceRsp( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_GetOverallSchedulePriceRsp( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_EnergyPhasesSchedule( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsReq( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleStateReq( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExtRsp( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileNotification( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileRsp( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileStateRsp( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePrice( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileStateNotification( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_GetOverallSchedulePrice( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleReq( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsNotification( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsRsp( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExt( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs );

/*********************************************************************
 * @fn      zclPowerProfile_RegisterCmdCallbacks
 *
 * @brief   Register applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclPowerProfile_RegisterCmdCallbacks( uint8 endpoint, zclPowerProfile_AppCallbacks_t *callbacks )
{
  zclPowerProfileCBRec_t *pNewItem;
  zclPowerProfileCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclPowerProfilePluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                        ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                        zclPowerProfile_HdlIncoming );
    zclPowerProfilePluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclPowerProfileCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError ); // memory error
  }

  pNewItem->next = (zclPowerProfileCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclPowerProfileCBs == NULL )
  {
    zclPowerProfileCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclPowerProfileCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileReq
 *
 * @brief   Request sent to server for Power Profile info.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                       uint8 powerProfileID,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];   // 1 byte payload

  buf[0] = powerProfileID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_POWER_PROFILE_REQ, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateReq
 *
 * @brief   Generated in order to retrieve the identifiers of current Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_REQ, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceRsp
 *
 * @brief   Allows a client to communicate the cost associated with a defined
 *          Power Profile to a server requesting it.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 powerProfileID, uint16 currency,
                                                               uint32 price, uint8 priceTrailingDigit,
                                                               uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[8];   // 8 byte payload

  buf[0] = powerProfileID;
  buf[1] = LO_UINT16( currency );
  buf[2] = HI_UINT16( currency );
  buf[3] = BREAK_UINT32( price, 0 );
  buf[4] = BREAK_UINT32( price, 1 );
  buf[5] = BREAK_UINT32( price, 2 );
  buf[6] = BREAK_UINT32( price, 3 );
  buf[7] = priceTrailingDigit;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_RSP, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetOverallSchedulePriceRsp
 *
 * @brief   Allows a client to communicate the cost associated with all
 *          Power Profiles to a server requesting it.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetOverallSchedulePriceRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint16 currency, uint32 price, uint8 priceTrailingDigit,
                                                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[7];   // 7 byte payload

  buf[0] = LO_UINT16( currency );
  buf[1] = HI_UINT16( currency );
  buf[2] = BREAK_UINT32( price, 0 );
  buf[3] = BREAK_UINT32( price, 1 );
  buf[4] = BREAK_UINT32( price, 2 );
  buf[5] = BREAK_UINT32( price, 3 );
  buf[6] = priceTrailingDigit;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE_RSP, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesSchedule
 *
 * @brief   Generated by server to notify the Energy Phases Schedule.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmdID - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION
 * @param   pCmd - parameters for the Energy Phases Schedule commands
 * @param   direction - send command client-to-server or server-to-client
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesSchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmdID,
                                                            zclPowerProfileEnergyPhasesSchedule_t *pCmd,
                                                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 offset;   // used to offset record when breaking payload into bytes
  uint8 *pBuf;              // variable length payload
  uint8 status;
  uint16 calculatedBufSize;

  // get a buffer large enough to hold the whole packet
  // size of scheduledPhasesRecord_t and zclPowerProfileEnergyPhasesSchedule_t
  calculatedBufSize = ( ( pCmd->numOfScheduledPhases * 3 ) + 2 );

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pCmd->powerProfileID;
  pBuf[1] = pCmd->numOfScheduledPhases;

  if ( pCmd->numOfScheduledPhases > 0 )
  {
    offset = 2;
    for ( i = 0; i < pCmd->numOfScheduledPhases; ++i )
    {
      pBuf[offset++] = pCmd->pScheduledPhasesRecord[i].energyPhaseID;
      pBuf[offset++] = LO_UINT16( pCmd->pScheduledPhasesRecord[i].scheduledTime );
      pBuf[offset++] = HI_UINT16( pCmd->pScheduledPhasesRecord[i].scheduledTime );
    }
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                            cmdID, TRUE, direction, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );

  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsReq
 *
 * @brief   Request sent to server to request constraints of the Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                                          uint8 powerProfileID,
                                                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];   // 1 byte payload

  buf[0] = powerProfileID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_REQ, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesScheduleStateReq
 *
 * @brief   Request sent to server to check the states of the scheduling of a Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                                    uint8 powerProfileID,
                                                                    uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];   // 1 byte payload

  buf[0] = powerProfileID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_REQ, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceExtRsp
 *
 * @brief   Allows a client to communicate the cost associated with all
 *          Power Profiles scheduled to a server.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceExtRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint8 powerProfileID, uint16 currency,
                                                                  uint32 price, uint8 priceTrailingDigit,
                                                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[8];   // 8 byte payload

  buf[0] = powerProfileID;
  buf[1] = LO_UINT16( currency );
  buf[2] = HI_UINT16( currency );
  buf[3] = BREAK_UINT32( price, 0 );
  buf[4] = BREAK_UINT32( price, 1 );
  buf[5] = BREAK_UINT32( price, 2 );
  buf[6] = BREAK_UINT32( price, 3 );
  buf[7] = priceTrailingDigit;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXT_RSP, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileNotification
 *
 * @brief   Server sends information of specific parameters belonging to each phase.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          totalProfileNum - total number of profiles supported by the device
 *          powerProfileID - identifier of the specific Power Profile
 *          numOfTransferredPhases - number of phases transferred
 *          energyPhaseID - identifier of the specific Power Profile energy phase
 *          macroPhaseID - identifier of the specific Power Profile phase
 *          expectedDuration - estimated duration of the specific phase
 *          peakPower - estimated power of the specific phase
 *          energy - estimated energy consumption for the accounted phase
 *          maxActivationDelay - maximum interruption time between end of previous phase and start of next phase
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                zclPowerProfileNotification_t *pPayload,
                                                                uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pBuf;   // variable length payload
  uint8 i;
  uint8 offset;   // used to offset record when breaking payload into bytes
  uint16 calculatedBufSize;
  ZStatus_t status;

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = pPayload->numOfTransferredPhases * 10 + 3;   // size of variable array plus size of structure

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->totalProfileNum;
  pBuf[1] = pPayload->powerProfileID;
  pBuf[2] = pPayload->numOfTransferredPhases;
  offset = 3;
  for ( i = 0; i < pPayload->numOfTransferredPhases; ++i )
  {
    pBuf[offset++] = pPayload->pTransferredPhasesRecord[i].energyPhaseID;
    pBuf[offset++] = pPayload->pTransferredPhasesRecord[i].macroPhaseID;
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].expectedDuration );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].expectedDuration );
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].peakPower );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].peakPower );
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].energy );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].energy );
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].maxActivationDelay );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].maxActivationDelay );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                            COMMAND_POWER_PROFILE_POWER_PROFILE_NOTIFICATION, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileRsp
 *
 * @brief   A response from the server to the PowerProfileReq command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          totalProfileNum - total number of profiles supported by the device
 *          powerProfileID - identifier of the specific Power Profile
 *          numOfTransferredPhases - number of phases transferred
 *          energyPhaseID - identifier of the specific Power Profile energy phase
 *          macroPhaseID - identifier of the specific Power Profile phase
 *          expectedDuration - estimated duration of the specific phase
 *          peakPower - estimated power of the specific phase
 *          energy - estimated energy consumption for the accounted phase
 *          maxActivationDelay - maximum interruption time between end of previous phase and start of next phase
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                       zclPowerProfileRsp_t *pPayload,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pBuf;   // variable length payload
  uint8 i;
  uint8 offset;   // used to offset record when breaking payload into bytes
  uint16 calculatedBufSize;
  ZStatus_t status;

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = pPayload->numOfTransferredPhases * 10 + 3;   // size of variable array plus size of structure

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->totalProfileNum;
  pBuf[1] = pPayload->powerProfileID;
  pBuf[2] = pPayload->numOfTransferredPhases;
  offset = 3;
  for ( i = 0; i < ( pPayload->numOfTransferredPhases ); ++i )
  {
    pBuf[offset++] = pPayload->pTransferredPhasesRecord[i].energyPhaseID;
    pBuf[offset++] = pPayload->pTransferredPhasesRecord[i].macroPhaseID;
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].expectedDuration );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].expectedDuration );
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].peakPower );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].peakPower );
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].energy );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].energy );
    pBuf[offset++] = LO_UINT16( pPayload->pTransferredPhasesRecord[i].maxActivationDelay );
    pBuf[offset++] = HI_UINT16( pPayload->pTransferredPhasesRecord[i].maxActivationDelay );
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                            COMMAND_POWER_PROFILE_POWER_PROFILE_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateRsp
 *
 * @brief   Server communicates its current Power Profile(s) to requesting client.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          powerProfileCount - number of Power Profile Records that follow in the message
 *          powerProfileRecord - includes: powerProfileID, energyPhaseID, powerProfileRemoteControl, powerProfileState
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                            zclPowerProfileStateRsp_t *pPayload,
                                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pBuf;   // variable length payload
  uint8 i;
  uint8 offset;   // used to offset record when breaking payload into bytes
  uint16 calculatedBufSize;
  ZStatus_t status;

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = pPayload->powerProfileCount * 4 + 1;   // size of variable array plus size of structure

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->powerProfileCount;
  offset = 1;
  for ( i = 0; i < pPayload->powerProfileCount; i++ )
  {
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].powerProfileID;
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].energyPhaseID;
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].powerProfileRemoteControl;
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].powerProfileState;
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                            COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_RSP, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePrice
 *
 * @brief   Used by server to retrieve the cost associated to a specific Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePrice( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 powerProfileID,
                                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];   // 1 byte payload

  buf[0] = powerProfileID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateNotification
 *
 * @brief   Generated by server to update the state of the power profile and
 *          current energy phase.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          powerProfileCount - number of Power Profile Records that follow in the message
 *          powerProfileRecord - includes: powerProfileID, energyPhaseID, powerProfileRemoteControl, powerProfileState
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                     zclPowerProfileStateNotification_t *pPayload,
                                                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pBuf;   // variable length payload
  uint8 i;
  uint8 offset;   // used to offset record when breaking payload into bytes
  uint16 calculatedBufSize;
  ZStatus_t status;

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = pPayload->powerProfileCount * 4 + 1;   // size of variable array plus size of structure

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->powerProfileCount;
  offset = 1;
  for ( i = 0; i < ( pPayload->powerProfileCount ); i++ )
  {
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].powerProfileID;
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].energyPhaseID;
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].powerProfileRemoteControl;
    pBuf[offset++] = pPayload->pPowerProfileStateRecord[i].powerProfileState;
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                            COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_NOTIFICATION, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
 zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetOverallSchedulePrice
 *
 * @brief   Generated by server to retrieve the overall cost associated to
 *          all Power Profiles scheduled by the scheduler for the next 24 hrs.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetOverallSchedulePrice( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesScheduleReq
 *
 * @brief   Generated by server to retrieve from scheduler the schedule of
 *          specific Power Profile carried in the payload.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 powerProfileID,
                                                               uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];   // 1 byte payload

  buf[0] = powerProfileID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_REQ, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsNotification
 *
 * @brief   Generated by server to notify client of imposed constraints and
 *          allow scheduler to set proper boundaries for scheduler.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Schedule Constraints Notification cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                                   zclPowerProfileScheduleConstraintsNotification_t *pCmd,
                                                                                   uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[5];   // 5 byte payload

  buf[0] = pCmd->powerProfileID;
  buf[1] = LO_UINT16( pCmd->startAfter );
  buf[2] = HI_UINT16( pCmd->startAfter );
  buf[3] = LO_UINT16( pCmd->stopBefore );
  buf[4] = HI_UINT16( pCmd->stopBefore );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_NOTIFICATION, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsRsp
 *
 * @brief   Generated by server in response to PowerProfileScheduleConstraintsReq.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Schedule Constraints Rsp cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                          zclPowerProfileScheduleConstraintsRsp_t *pCmd,
                                                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[5];   // 5 byte payload

  buf[0] = pCmd->powerProfileID;
  buf[1] = LO_UINT16( pCmd->startAfter );
  buf[2] = HI_UINT16( pCmd->startAfter );
  buf[3] = LO_UINT16( pCmd->stopBefore );
  buf[4] = HI_UINT16( pCmd->stopBefore );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceExt
 *
 * @brief   Generated by server to retrieve cost associated to a specific
 *          Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Get Power Profile Price Extended cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceExt( uint8 srcEP, afAddrType_t *dstAddr,
                                                               zclPowerProfileGetPowerProfilePriceExt_t *pCmd,
                                                               uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4]; // maximum payload size
  uint8 calculatedSize = 2; // minimum payload size

  buf[0] = pCmd->options;
  buf[1] = pCmd->powerProfileID;

  if ( pCmd->options & 0x01 )
  {
    calculatedSize = 4;
    buf[2] = LO_UINT16( pCmd->powerProfileStartTime );
    buf[3] = HI_UINT16( pCmd->powerProfileStartTime );
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POWER_PROFILE,
                          COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXT, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedSize, buf );
}

/*********************************************************************
 * @fn      zclPowerProfile_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclPowerProfile_AppCallbacks_t *zclPowerProfile_FindCallbacks( uint8 endpoint )
{
  zclPowerProfileCBRec_t *pCBs;

  pCBs = zclPowerProfileCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclPowerProfile_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclPowerProfile_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
  {
    return ( stat ); // Cluster not supported thru Inter-PAN
  }
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclPowerProfile_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    stat = ZFailure;
  }
  return ( stat );
}

/*********************************************************************
 * @fn      zclPowerProfile_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclPowerProfile_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclPowerProfile_FindCallbacks( pInMsg->msg->endPoint );
  if (pCBs == NULL )
  {
    return ( ZFailure );
  }

  stat = zclPowerProfile_ProcessInCmds( pInMsg, pCBs );

  return ( stat );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmds( zclIncoming_t *pInMsg, zclPowerProfile_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_POWER_PROFILE_POWER_PROFILE_REQ:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileReq( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_REQ:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileStateReq( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_RSP:
        stat = zclPowerProfile_ProcessInCmd_GetPowerProfilePriceRsp( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE_RSP:
        stat = zclPowerProfile_ProcessInCmd_GetOverallSchedulePriceRsp( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION:
      case COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP:
        stat = zclPowerProfile_ProcessInCmd_EnergyPhasesSchedule( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_REQ:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsReq( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_REQ:
        stat = zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleStateReq( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXT_RSP:
        stat = zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExtRsp( pInMsg, pCBs );
        break;

      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }
  // Server-to-Client
  else
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_POWER_PROFILE_POWER_PROFILE_NOTIFICATION:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileNotification( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_RSP:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileRsp( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_RSP:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileStateRsp( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE:
        stat = zclPowerProfile_ProcessInCmd_GetPowerProfilePrice( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_NOTIFICATION:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileStateNotification( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE:
        stat = zclPowerProfile_ProcessInCmd_GetOverallSchedulePrice( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_REQ:
        stat = zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleReq( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP:
      case COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION:
        stat = zclPowerProfile_ProcessInCmd_EnergyPhasesSchedule( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_NOTIFICATION:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsNotification( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_RSP:
        stat = zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsRsp( pInMsg, pCBs );
        break;

      case COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXT:
        stat = zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExt( pInMsg, pCBs );
        break;

      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileReq
 *
 * @brief   Process in the received Power Profile Request cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileReq( zclIncoming_t *pInMsg,
                                                               zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_PowerProfileReq )
  {
    return ( pCBs->pfnPowerProfile_PowerProfileReq( pInMsg->pData[0], &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileStateReq
 *
 * @brief   Process in the received Power Profile Power Profile State Request cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileStateReq( zclIncoming_t *pInMsg,
                                                                       zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_PowerProfileStateReq )
  {
    // no payload
    return ( pCBs->pfnPowerProfile_PowerProfileStateReq( &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_GetPowerProfilePriceRsp
 *
 * @brief   Process in the received Power Profile Get Power Profile Price Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePriceRsp( zclIncoming_t *pInMsg,
                                                                       zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_GetPowerProfilePriceRsp )
  {
    zclPowerProfileGetPowerProfilePriceRsp_t cmd;

    cmd.powerProfileID = pInMsg->pData[0];
    cmd.currency = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
    cmd.price = BUILD_UINT32( pInMsg->pData[3], pInMsg->pData[4], pInMsg->pData[5], pInMsg->pData[6] );
    cmd.priceTrailingDigit = pInMsg->pData[7];

    return ( pCBs->pfnPowerProfile_GetPowerProfilePriceRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_GetOverallSchedulePriceRsp
 *
 * @brief   Process in the received Power Profile Get Overall Schedule Price Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_GetOverallSchedulePriceRsp( zclIncoming_t *pInMsg,
                                                                          zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_GetOverallSchedulePriceRsp )
  {
    zclPowerProfileGetOverallSchedulePriceRsp_t cmd;

    cmd.currency = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.price = BUILD_UINT32( pInMsg->pData[2], pInMsg->pData[3], pInMsg->pData[4], pInMsg->pData[5] );
    cmd.priceTrailingDigit = pInMsg->pData[6];

    return ( pCBs->pfnPowerProfile_GetOverallSchedulePriceRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_EnergyPhasesSchedule
 *
 * @brief   Process in the received Power Profile Energy Phases Schedule commmands
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_EnergyPhasesSchedule( zclIncoming_t *pInMsg,
                                                                    zclPowerProfile_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;     // used for creating variable length structures
  uint16 calculatedArraySize;
  zclPowerProfileEnergyPhasesSchedule_t cmd;
  ZStatus_t status = ZFailure;

  // calculate the array size multiplying numOfScheduledPhases by size of variable array
  calculatedArraySize = ( pInMsg->pData[1] * sizeof( scheduledPhasesRecord_t ) );

  // allocate memory for variable array
  cmd.pScheduledPhasesRecord = zcl_mem_alloc( calculatedArraySize );
  if ( !cmd.pScheduledPhasesRecord )
  {
    return ( ZMemError );  // no memory, return failure
  }

  cmd.powerProfileID = pInMsg->pData[0];
  cmd.numOfScheduledPhases = pInMsg->pData[1];

  if ( pInMsg->pData[1] > 0 )
  {
    offset = 2;
    for ( i = 0; i < pInMsg->pData[1]; ++i)
    {
      cmd.pScheduledPhasesRecord[i].energyPhaseID = pInMsg->pData[offset];
      cmd.pScheduledPhasesRecord[i].scheduledTime = BUILD_UINT16( pInMsg->pData[offset + 1], pInMsg->pData[offset + 2] );
      offset += 3;
    }
  }


  if ( pInMsg->hdr.commandID == COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION )
  {
    if ( pCBs->pfnPowerProfile_EnergyPhasesScheduleNotification )
    {
      status = ( pCBs->pfnPowerProfile_EnergyPhasesScheduleNotification( &cmd ) );
    }
  }
  else if ( pInMsg->hdr.commandID == COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP )
  {
    if ( pCBs->pfnPowerProfile_EnergyPhasesScheduleRsp )
    {
      status = ( pCBs->pfnPowerProfile_EnergyPhasesScheduleRsp( &cmd ) );
    }
  }
  else if ( pInMsg->hdr.commandID == COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP )
  {
    if ( pCBs->pfnPowerProfile_EnergyPhasesScheduleStateRsp )
    {
      status = ( pCBs->pfnPowerProfile_EnergyPhasesScheduleStateRsp( &cmd ) );
    }
  }
  else if ( pInMsg->hdr.commandID == COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION )
  {
    if ( pCBs->pfnPowerProfile_EnergyPhasesScheduleStateNotification )
    {
      status = ( pCBs->pfnPowerProfile_EnergyPhasesScheduleStateNotification( &cmd ) );
    }
  }

  zcl_mem_free( cmd.pScheduledPhasesRecord );

  return status;
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsReq
 *
 * @brief   Process in the received Power Profile Schedule Constraints Request cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsReq( zclIncoming_t *pInMsg,
                                                                                  zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_PowerProfileScheduleConstraintsReq )
  {
    return ( pCBs->pfnPowerProfile_PowerProfileScheduleConstraintsReq( pInMsg->pData[0], &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleStateReq
 *
 * @brief   Process in the received Power Profile Energy Phases Schedule State Request cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleStateReq( zclIncoming_t *pInMsg,
                                                                            zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_EnergyPhasesScheduleStateReq )
  {
    return ( pCBs->pfnPowerProfile_EnergyPhasesScheduleStateReq( pInMsg->pData[0], &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExtRsp
 *
 * @brief   Process in the received Power Profile Get Power Profile Price Extended Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExtRsp( zclIncoming_t *pInMsg,
                                                                          zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_GetPowerProfilePriceExtRsp )
  {
    zclPowerProfileGetPowerProfilePriceExtRsp_t cmd;

    cmd.powerProfileID = pInMsg->pData[0];
    cmd.currency = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
    cmd.price = BUILD_UINT32( pInMsg->pData[3], pInMsg->pData[4], pInMsg->pData[5], pInMsg->pData[6] );
    cmd.priceTrailingDigit = pInMsg->pData[7];

    return ( pCBs->pfnPowerProfile_GetPowerProfilePriceExtRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileNotification
 *
 * @brief   Process in the received Power Profile Power Profile Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileNotification( zclIncoming_t *pInMsg,
                                                                        zclPowerProfile_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;     // used for creating variable length structures
  uint16 calculatedArraySize;
  zclPowerProfileNotification_t cmd;

  if ( pCBs->pfnPowerProfile_PowerProfileNotification )
  {
    // calculate array size multiplying numOfTransferredPhases by size of variable array
    calculatedArraySize = pInMsg->pData[2] * 10;

    // allocate memory for variable array
    cmd.pTransferredPhasesRecord = zcl_mem_alloc( calculatedArraySize );
    if ( !cmd.pTransferredPhasesRecord )
    {
      return ( ZMemError );  // no memory, return failure
    }

    cmd.totalProfileNum = pInMsg->pData[0];
    cmd.powerProfileID = pInMsg->pData[1];
    cmd.numOfTransferredPhases = pInMsg->pData[2];
    offset = 3;
    for ( i = 0; i < pInMsg->pData[2]; ++i )
    {
      cmd.pTransferredPhasesRecord[i].energyPhaseID = pInMsg->pData[offset];
      cmd.pTransferredPhasesRecord[i].macroPhaseID = pInMsg->pData[offset + 1];
      cmd.pTransferredPhasesRecord[i].expectedDuration = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
      cmd.pTransferredPhasesRecord[i].peakPower = BUILD_UINT16( pInMsg->pData[offset + 4], pInMsg->pData[offset + 5] );
      cmd.pTransferredPhasesRecord[i].energy = BUILD_UINT16( pInMsg->pData[offset + 6], pInMsg->pData[offset + 7] );
      cmd.pTransferredPhasesRecord[i].maxActivationDelay = BUILD_UINT16( pInMsg->pData[offset + 8], pInMsg->pData[offset + 9] );
      offset += 10;
    }

    zcl_mem_free( cmd.pTransferredPhasesRecord );
    return ( pCBs->pfnPowerProfile_PowerProfileNotification( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileRsp
 *
 * @brief   Process in the received Power Profile Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileRsp( zclIncoming_t *pInMsg,
                                                               zclPowerProfile_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;     // used for creating variable length structures
  uint16 calculatedArraySize;
  zclPowerProfileRsp_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnPowerProfile_PowerProfileRsp )
  {
    // calculate size of array multiplying numOfTransferredPhases by size of variable array
    calculatedArraySize = pInMsg->pData[2] * 10;

    // allocate memory for variable array
    cmd.pTransferredPhasesRecord = zcl_mem_alloc( calculatedArraySize );
    if ( !cmd.pTransferredPhasesRecord )
    {
      return ( ZMemError );  // no memory, return failure
    }

    cmd.totalProfileNum = pInMsg->pData[0];
    cmd.powerProfileID = pInMsg->pData[1];
    cmd.numOfTransferredPhases = pInMsg->pData[2];
    offset = 3;
    for ( i = 0; i < pInMsg->pData[2]; ++i )
    {
      cmd.pTransferredPhasesRecord[i].energyPhaseID = pInMsg->pData[offset];
      cmd.pTransferredPhasesRecord[i].macroPhaseID = pInMsg->pData[offset + 1];
      cmd.pTransferredPhasesRecord[i].expectedDuration = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
      cmd.pTransferredPhasesRecord[i].peakPower = BUILD_UINT16( pInMsg->pData[offset + 4], pInMsg->pData[offset + 5] );
      cmd.pTransferredPhasesRecord[i].energy = BUILD_UINT16( pInMsg->pData[offset + 6], pInMsg->pData[offset + 7] );
      cmd.pTransferredPhasesRecord[i].maxActivationDelay = BUILD_UINT16( pInMsg->pData[offset + 8], pInMsg->pData[offset + 9] );
      offset += 10;
    }

    status = ( pCBs->pfnPowerProfile_PowerProfileRsp( &cmd ) );
    zcl_mem_free( cmd.pTransferredPhasesRecord );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileStateRsp
 *
 * @brief   Process in the received Power Profile State Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileStateRsp( zclIncoming_t *pInMsg,
                                                                    zclPowerProfile_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;     // used for creating variable length structures
  uint16 calculatedArraySize;
  zclPowerProfileStateRsp_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnPowerProfile_PowerProfileStateRsp )
  {
    // calculate size of array multiplying powerProfileCount by size of variable array
    calculatedArraySize = pInMsg->pData[0] * 4;

    // allocate memory for variable array
    cmd.pPowerProfileStateRecord = zcl_mem_alloc( calculatedArraySize );
    if ( !cmd.pPowerProfileStateRecord )
    {
      return ( ZMemError );  // no memory, return failure
    }

    cmd.powerProfileCount = pInMsg->pData[0];
    offset = 1;
    for ( i = 0; i < pInMsg->pData[0]; ++i )
    {
      cmd.pPowerProfileStateRecord[i].powerProfileID = pInMsg->pData[offset++];
      cmd.pPowerProfileStateRecord[i].energyPhaseID = pInMsg->pData[offset++];
      cmd.pPowerProfileStateRecord[i].powerProfileRemoteControl = pInMsg->pData[offset++];
      cmd.pPowerProfileStateRecord[i].powerProfileState = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnPowerProfile_PowerProfileStateRsp( &cmd ) );
    zcl_mem_free( cmd.pPowerProfileStateRecord );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_GetPowerProfilePrice
 *
 * @brief   Process in the received Power Profile Get Power Profile Price cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePrice( zclIncoming_t *pInMsg,
                                                                    zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_GetPowerProfilePrice )
  {
    return ( pCBs->pfnPowerProfile_GetPowerProfilePrice( pInMsg->pData[0], &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileStateNotification
 *
 * @brief   Process in the received Power Profile State Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileStateNotification( zclIncoming_t *pInMsg,
                                                                             zclPowerProfile_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;     // used for creating variable length structures
  uint16 calculatedArraySize;
  zclPowerProfileStateNotification_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnPowerProfile_PowerProfileStateNotification )
  {
    // calculate array size multiplying powerProfileCount by size of variable array
    calculatedArraySize = pInMsg->pData[0] * 4;

    // allocate memory for variable array
    cmd.pPowerProfileStateRecord = zcl_mem_alloc( calculatedArraySize );
    if ( !cmd.pPowerProfileStateRecord )
    {
      return ( ZMemError );  // no memory, return failure
    }

    cmd.powerProfileCount = pInMsg->pData[0];
    offset = 1;
    for ( i = 0; i < pInMsg->pData[0]; ++i )
    {
      cmd.pPowerProfileStateRecord[i].powerProfileID = pInMsg->pData[offset++];
      cmd.pPowerProfileStateRecord[i].energyPhaseID = pInMsg->pData[offset++];
      cmd.pPowerProfileStateRecord[i].powerProfileRemoteControl = pInMsg->pData[offset++];
      cmd.pPowerProfileStateRecord[i].powerProfileState = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnPowerProfile_PowerProfileStateNotification( &cmd ) );
    zcl_mem_free( cmd.pPowerProfileStateRecord );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_GetOverallSchedulePrice
 *
 * @brief   Process in the received Power Profile Get Overall Schedule Price cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_GetOverallSchedulePrice( zclIncoming_t *pInMsg,
                                                                       zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_GetOverallSchedulePrice )
  {
    // no payload
    return ( pCBs->pfnPowerProfile_GetOverallSchedulePrice( &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleReq
 *
 * @brief   Process in the received Power Profile Energy Phases Schedule Request cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_EnergyPhasesScheduleReq( zclIncoming_t *pInMsg,
                                                                       zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_EnergyPhasesScheduleReq )
  {
    return ( pCBs->pfnPowerProfile_EnergyPhasesScheduleReq( pInMsg->pData[0], &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsNotification
 *
 * @brief   Process in the received Power Profile Schedule Constraints Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsNotification( zclIncoming_t *pInMsg,
                                                                                           zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_PowerProfileScheduleConstraintsNotification )
  {
    zclPowerProfileScheduleConstraintsNotification_t cmd;

    cmd.powerProfileID = pInMsg->pData[0];
    cmd.startAfter = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
    cmd.stopBefore = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );

    return ( pCBs->pfnPowerProfile_PowerProfileScheduleConstraintsNotification( &cmd ) );
  }
  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsRsp
 *
 * @brief   Process in the received Power Profile Schedule Constraints Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_PowerProfileScheduleConstraintsRsp( zclIncoming_t *pInMsg,
                                                                                  zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_PowerProfileScheduleConstraintsRsp )
  {
    zclPowerProfileScheduleConstraintsRsp_t cmd;

    cmd.powerProfileID = pInMsg->pData[0];
    cmd.startAfter = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
    cmd.stopBefore = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );

    return ( pCBs->pfnPowerProfile_PowerProfileScheduleConstraintsRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExt
 *
 * @brief   Process in the received Power Profile Get Power Profile Price Extended cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPowerProfile_ProcessInCmd_GetPowerProfilePriceExt( zclIncoming_t *pInMsg,
                                                                       zclPowerProfile_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPowerProfile_GetPowerProfilePriceExt )
  {
    zclPowerProfileGetPowerProfilePriceExt_t cmd;

    cmd.options = pInMsg->pData[0];
    cmd.powerProfileID = pInMsg->pData[1];

    if ( cmd.options & 0x01 )
    {
      cmd.powerProfileStartTime = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
    }

    return ( pCBs->pfnPowerProfile_GetPowerProfilePriceExt( &cmd, &pInMsg->msg->srcAddr, pInMsg->hdr.transSeqNum ) );
  }

  return ( ZFailure );
}

#endif // ZCL_POWER_PROFILE

/****************************************************************************
****************************************************************************/

