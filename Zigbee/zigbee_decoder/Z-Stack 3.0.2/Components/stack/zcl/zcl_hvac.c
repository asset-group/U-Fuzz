/**************************************************************************************************
  Filename:       zcl_hvac.c
  Revised:        $Date: 2013-10-16 16:08:27 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35699 $

  Description:    Zigbee Cluster Library - HVAC


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifdef ZCL_HVAC_CLUSTER

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_hvac.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclHVACCBRec
{
  struct zclHVACCBRec     *next;
  uint8                   endpoint; // Used to link it into the endpoint descriptor
  zclHVAC_AppCallbacks_t  *CBs;     // Pointer to Callback function
} zclHVACCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclHVACCBRec_t *zclHVACCBs = (zclHVACCBRec_t *)NULL;
static uint8 zclHVACPluginRegisted = FALSE;


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclHVAC_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclHVAC_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclHVAC_AppCallbacks_t *zclHVAC_FindCallbacks( uint8 endpoint );

static ZStatus_t zclHVAC_ProcessInPumpCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclHVAC_ProcessInThermostatCmds( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_SetpointRaiseLower( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_SetWeeklySchedule( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_GetWeeklySchedule( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_ClearWeeklySchedule( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_GetRelayStatusLog( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_GetWeeklyScheduleRsp( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );
static ZStatus_t zclThermostat_ProcessInCmd_GetRelayStatusLogRsp( zclIncoming_t *pInMsg, zclHVAC_AppCallbacks_t *pCBs );

/*********************************************************************
 * @fn      zclHVAC_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclHVAC_RegisterCmdCallbacks( uint8 endpoint, zclHVAC_AppCallbacks_t *callbacks )
{
  zclHVACCBRec_t *pNewItem;
  zclHVACCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclHVACPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL,
                        ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG,
                        zclHVAC_HdlIncoming );
    zclHVACPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclHVACCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclHVACCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclHVACCBs == NULL )
  {
    zclHVACCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclHVACCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclHVAC_SendSetpointRaiseLower
 *
 * @brief   Call to send out a Setpoint Raise/Lower Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   mode - which setpoint is to be configured
 * @param   amount - amount setpoint(s) are to be increased (or decreased) by,
 *                   in steps of 0.1°C
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendSetpointRaiseLower( uint8 srcEP, afAddrType_t *dstAddr,
                                          uint8 mode, int8 amount,
                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_SETPOINT_RAISE_LOWER];

  buf[0] = mode;
  buf[1] = amount;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                          COMMAND_THERMOSTAT_SETPOINT_RAISE_LOWER, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_SETPOINT_RAISE_LOWER, buf );
}

/*********************************************************************
 * @fn      zclHVAC_SendSetWeeklySchedule
 *
 * @brief   Call to send out a Set Weekly Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload - payload for Set Weekly Schedule cmd
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendSetWeeklySchedule( uint8 srcEP, afAddrType_t *dstAddr,
                                         zclThermostatWeeklySchedule_t *pPayload,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 modeBuff;   // buffer to determine how to incorporate heat/cool set points
  uint8 *pBuf;    // variable length payload
  uint8 offset;
  uint16 arrayRecordSize;    // number of bytes in record array
  uint16 calculatedBufSize;
  ZStatus_t status;


  modeBuff = pPayload->modeForSequence;    // store the modeForSequence in buffer

  // if either the heat or cool set points
  if( ( modeBuff == HVAC_THERMOSTAT_MODE_HEAT ) || ( modeBuff == HVAC_THERMOSTAT_MODE_COOL ) )
  {
    // calculate the number of transitions multiplied by size of transitionTime and heatSetPoint/coolSetPoint
    arrayRecordSize = ( pPayload->numberOfTransitionsForSequence * PAYLOAD_LEN_WEEKLY_SCHEDULE_COOL_HEAT_MODE );
  }
  // if both cool and heat set points
  else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )
  {
    // calculate the number of transitions multiplied by size of transitionTime, heatSetPoint, and coolSetPoint
    arrayRecordSize = ( pPayload->numberOfTransitionsForSequence * PAYLOAD_LEN_WEEKLY_SCHEDULE_BOTH_MODES );
  }
  else
  {
    return ( ZFailure );    // invalid data
  }

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = ( PAYLOAD_LEN_WEEKLY_SCHEDULE + arrayRecordSize );

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if( !pBuf )
  {
    return ( ZFailure );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->numberOfTransitionsForSequence;
  pBuf[1] = pPayload->dayOfWeekForSequence;
  pBuf[2] = pPayload->modeForSequence;
  offset = 3;
  for( i = 0; i < ( pPayload->numberOfTransitionsForSequence ); i++ )
  {
    if( modeBuff == HVAC_THERMOSTAT_MODE_HEAT )   // heat set point
    {
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].transitionTime );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].transitionTime );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].heatSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].heatSetPoint );
    }
    else if( modeBuff == HVAC_THERMOSTAT_MODE_COOL )    // cool set point
    {
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].transitionTime );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].transitionTime );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].coolSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].coolSetPoint );
    }
    else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )    // both cool and heat set points
    {
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].transitionTime );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].transitionTime );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].heatSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].heatSetPoint );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].coolSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].coolSetPoint );
    }
    else
    {
      return ( ZFailure );    // unsupported mode
    }
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                           COMMAND_THERMOSTAT_SET_WEEKLY_SCHEDULE, TRUE,
                           ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return status;
}

/*********************************************************************
 * @fn      zclHVAC_SendGetWeeklySchedule
 *
 * @brief   Call to send out a Get Weekly Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   daysToReturn - indicates number of days client would like to return the set point values for
 * @param   modeToReturn - indicates the mode the client would like to return the set point values for
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendGetWeeklySchedule( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint8 daysToReturn, uint8 modeToReturn,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_GET_WEEKLY_SCHEDULE];

  buf[0] = daysToReturn;
  buf[1] = modeToReturn;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                          COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_GET_WEEKLY_SCHEDULE, buf );
}

/*********************************************************************
 * @fn      zclHVAC_SendClearWeeklySchedule
 *
 * @brief   Call to send out a Clear Weekly Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendClearWeeklySchedule( uint8 srcEP, afAddrType_t *dstAddr,
                                           uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                          COMMAND_THERMOSTAT_CLEAR_WEEKLY_SCHEDULE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclHVAC_SendGetRelayStatusLog
 *
 * @brief   Call to send out a Get Relay Status Log Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendGetRelayStatusLog( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                          COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclHVAC_SendGetWeeklyScheduleRsp
 *
 * @brief   Call to send out a Get Weekly Schedule Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload - payload for Get Weekly Schedule rsp
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendGetWeeklyScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclThermostatWeeklySchedule_t *pPayload,
                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 modeBuff;   // buffer to determine how to incorporate heat/cool set points
  uint8 *pBuf;    // variable length payload
  uint8 offset;
  uint16 arrayRecordSize;    // number of bytes in record array
  uint16 calculatedBufSize;
  ZStatus_t status;


  modeBuff = pPayload->modeForSequence;    // store the modeForSequence in buffer

  // if either the heat or cool set points
  if( ( modeBuff == HVAC_THERMOSTAT_MODE_HEAT ) || ( modeBuff == HVAC_THERMOSTAT_MODE_COOL ) )
  {
    // calculate the number of transitions multiplied by size of transitionTime and heatSetPoint/coolSetPoint
    arrayRecordSize = ( pPayload->numberOfTransitionsForSequence * PAYLOAD_LEN_WEEKLY_SCHEDULE_COOL_HEAT_MODE );
  }
  // if both cool and heat set points
  else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )
  {
    // calculate the number of transitions multiplied by size of transitionTime, heatSetPoint, and coolSetPoint
    arrayRecordSize = ( pPayload->numberOfTransitionsForSequence * PAYLOAD_LEN_WEEKLY_SCHEDULE_BOTH_MODES );
  }
  else
  {
    return ( ZFailure );    // invalid data
  }

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = ( PAYLOAD_LEN_WEEKLY_SCHEDULE + arrayRecordSize );

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if( !pBuf )
  {
    return ( ZFailure );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->numberOfTransitionsForSequence;
  pBuf[1] = pPayload->dayOfWeekForSequence;
  pBuf[2] = pPayload->modeForSequence;
  offset = 3;
  for( i = 0; i < ( pPayload->numberOfTransitionsForSequence ); i++ )
  {
    if( modeBuff == HVAC_THERMOSTAT_MODE_HEAT )   // heat set point
    {
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].transitionTime );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].transitionTime );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].heatSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeHeat[i].heatSetPoint );
    }
    else if( modeBuff == HVAC_THERMOSTAT_MODE_COOL )    // cool set point
    {
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].transitionTime );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].transitionTime );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].coolSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeCool[i].coolSetPoint );
    }
    else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )    // both cool and heat set points
    {
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].transitionTime );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].transitionTime );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].heatSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].heatSetPoint );
      pBuf[offset++] = LO_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].coolSetPoint );
      pBuf[offset++] = HI_UINT16( pPayload->sThermostateSequenceMode.psThermostatModeBoth[i].coolSetPoint );
    }
    else
    {
      return ( ZFailure );    // unsupported mode
    }
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                           COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE_RSP, TRUE,
                           ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return status;
}

/*********************************************************************
 * @fn      zclHVAC_SendGetRelayStatusLogRsp
 *
 * @brief   Call to send out a Get Relay Status Log Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload - payload for Get Relay Status Log rsp
 * @param   seqNum - transaction sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclHVAC_SendGetRelayStatusLogRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclThermostatGetRelayStatusLogRsp_t *pPayload,
                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_GET_RELAY_STATUS_LOG_RSP];

  buf[0] = LO_UINT16( pPayload->timeOfDay );
  buf[1] = HI_UINT16( pPayload->timeOfDay );
  buf[2] = LO_UINT16( pPayload->relayStatus );
  buf[3] = HI_UINT16( pPayload->relayStatus );
  buf[4] = LO_UINT16( pPayload->localTemperature );
  buf[5] = HI_UINT16( pPayload->localTemperature );
  buf[6] = pPayload->humidity;
  buf[7] = LO_UINT16( pPayload->setPoint );
  buf[8] = HI_UINT16( pPayload->setPoint );
  buf[9] = LO_UINT16( pPayload->unreadEntries );
  buf[10] = HI_UINT16( pPayload->unreadEntries );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HVAC_THERMOSTAT,
                          COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_GET_RELAY_STATUS_LOG_RSP, buf );
}

/*********************************************************************
 * @fn      zclHVAC_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclHVAC_AppCallbacks_t *zclHVAC_FindCallbacks( uint8 endpoint )
{
  zclHVACCBRec_t *pCBs;
  pCBs = zclHVACCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }
  return ( (zclHVAC_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclHVAC_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclHVAC_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
    return ( stat ); // Cluster not supported thru Inter-PAN
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclHVAC_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command -- ignore it.
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
 * @fn      zclHVAC_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclHVAC_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclHVAC_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = (void*)zclHVAC_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL:
      stat = zclHVAC_ProcessInPumpCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_HVAC_THERMOSTAT:
      stat = zclHVAC_ProcessInThermostatCmds( pInMsg, pCBs );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclHVAC_ProcessInPumpCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclHVAC_ProcessInPumpCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZFailure;

  // there are no specific command for this cluster yet.
  // instead of suppressing a compiler warnings( for a code porting reasons )
  // fake unused call here and keep the code skeleton intact
 (void)pInMsg;
  if ( stat != ZFailure )
    zclHVAC_FindCallbacks( 0 );

  return ( stat );
}

/*********************************************************************
 * @fn      zclHVAC_ProcessInThermostatCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclHVAC_ProcessInThermostatCmds( zclIncoming_t *pInMsg,
                                                  zclHVAC_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  if  ( pInMsg->hdr.commandID > COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG )
  {
    return (ZFailure);   // Error ignore the command
  }

  // Client-to-Server
  if( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_THERMOSTAT_SETPOINT_RAISE_LOWER:
        stat = zclThermostat_ProcessInCmd_SetpointRaiseLower( pInMsg, pCBs);
        break;

      case COMMAND_THERMOSTAT_SET_WEEKLY_SCHEDULE:
        stat = zclThermostat_ProcessInCmd_SetWeeklySchedule( pInMsg, pCBs );
        break;

      case COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE:
        stat = zclThermostat_ProcessInCmd_GetWeeklySchedule( pInMsg, pCBs );
        break;

      case COMMAND_THERMOSTAT_CLEAR_WEEKLY_SCHEDULE:
        stat = zclThermostat_ProcessInCmd_ClearWeeklySchedule( pInMsg, pCBs );
        break;

      case COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG:
        stat = zclThermostat_ProcessInCmd_GetRelayStatusLog( pInMsg, pCBs );
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
      case COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE_RSP:
        stat = zclThermostat_ProcessInCmd_GetWeeklyScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG_RSP:
        stat = zclThermostat_ProcessInCmd_GetRelayStatusLogRsp( pInMsg, pCBs );
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
 * @fn      zclThermostat_ProcessInCmd_SetpointRaiseLower
 *
 * @brief   Process in the received Setpoint Raise/Lower cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_SetpointRaiseLower( zclIncoming_t *pInMsg,
                                                               zclHVAC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnHVAC_SetpointRaiseLower )
  {
    zclCmdThermostatSetpointRaiseLowerPayload_t cmd;

    cmd.mode = pInMsg->pData[0];
    cmd.amount = pInMsg->pData[1];

    return ( pCBs->pfnHVAC_SetpointRaiseLower( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclThermostat_ProcessInCmd_SetWeeklySchedule
 *
 * @brief   Process in the received Set Weekly Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_SetWeeklySchedule( zclIncoming_t *pInMsg,
                                                               zclHVAC_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;
  uint8 modeBuff;   // buffer to determine how to incorporate heat/cool set points
  uint8 numTransitionsBuf;  // buffer for number of transitions for sequence
  uint16 arrayRecordSize;
  zclThermostatWeeklySchedule_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnHVAC_SetWeeklySchedule )
  {
    numTransitionsBuf = pInMsg->pData[0];
    modeBuff = pInMsg->pData[2];    // store the modeForSequence in buffer

    // if either the heat or cool set points
    if( ( modeBuff == HVAC_THERMOSTAT_MODE_HEAT ) || ( modeBuff == HVAC_THERMOSTAT_MODE_COOL ) )
    {
      // calculate the number of transitions multiplied by size of transitionTime and heatSetPoint/coolSetPoint
      arrayRecordSize = ( numTransitionsBuf * 4 );
    }
    // if both cool and heat set points
    else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )
    {
      // calculate the number of transitions multiplied by size of transitionTime, heatSetPoint, and coolSetPoint
      arrayRecordSize = ( numTransitionsBuf * 6 );
    }
    else
    {
      return ( ZFailure );    // invalid mode field
    }

    // allocate memory
    cmd.sThermostateSequenceMode.psThermostatModeBoth = zcl_mem_alloc( arrayRecordSize );
    if( !cmd.sThermostateSequenceMode.psThermostatModeBoth )
    {
      return ZMemError; // no memory
    }

    cmd.numberOfTransitionsForSequence = numTransitionsBuf;
    cmd.dayOfWeekForSequence = pInMsg->pData[1];
    cmd.modeForSequence = modeBuff;
    offset = 3;
    for ( i = 0; i < numTransitionsBuf; i++ )
    {
      if ( modeBuff == HVAC_THERMOSTAT_MODE_HEAT )   // heat set point
      {
        cmd.sThermostateSequenceMode.psThermostatModeHeat[i].transitionTime = BUILD_UINT16( pInMsg->pData[offset], pInMsg->pData[offset + 1] );
        cmd.sThermostateSequenceMode.psThermostatModeHeat[i].heatSetPoint = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
        offset += 4;
      }
      else if ( modeBuff == HVAC_THERMOSTAT_MODE_COOL )    // cool set point
      {
        cmd.sThermostateSequenceMode.psThermostatModeCool[i].transitionTime = BUILD_UINT16( pInMsg->pData[offset], pInMsg->pData[offset + 1] );
        cmd.sThermostateSequenceMode.psThermostatModeCool[i].coolSetPoint = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
        offset += 4;
      }
      else if ( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )    // both cool and heat set points
      {
        cmd.sThermostateSequenceMode.psThermostatModeBoth[i].transitionTime = BUILD_UINT16( pInMsg->pData[offset], pInMsg->pData[offset + 1] );
        cmd.sThermostateSequenceMode.psThermostatModeBoth[i].heatSetPoint = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
        cmd.sThermostateSequenceMode.psThermostatModeBoth[i].coolSetPoint = BUILD_UINT16( pInMsg->pData[offset + 4], pInMsg->pData[offset + 5] );
        offset += 6;
      }
      else
      {
        return ( ZFailure );    // unsupported mode
      }
    }


    status = ( pCBs->pfnHVAC_SetWeeklySchedule( &cmd ) );
    zcl_mem_free( cmd.sThermostateSequenceMode.psThermostatModeBoth );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclThermostat_ProcessInCmd_GetWeeklySchedule
 *
 * @brief   Process in the received Get Weekly Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_GetWeeklySchedule( zclIncoming_t *pInMsg,
                                                               zclHVAC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnHVAC_GetWeeklySchedule )
  {
    zclThermostatGetWeeklySchedule_t cmd;

    cmd.daysToReturn = pInMsg->pData[0];
    cmd.modeToReturn = pInMsg->pData[1];

    return ( pCBs->pfnHVAC_GetWeeklySchedule( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclThermostat_ProcessInCmd_ClearWeeklySchedule
 *
 * @brief   Process in the received Clear Weekly Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_ClearWeeklySchedule( zclIncoming_t *pInMsg,
                                                                 zclHVAC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnHVAC_ClearWeeklySchedule )
  {
    // no payload

    return ( pCBs->pfnHVAC_ClearWeeklySchedule(  ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclThermostat_ProcessInCmd_GetRelayStatusLog
 *
 * @brief   Process in the received Get Relay Status Log cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_GetRelayStatusLog( zclIncoming_t *pInMsg,
                                                               zclHVAC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnHVAC_GetRelayStatusLog )
  {
    // no payload

    return ( pCBs->pfnHVAC_GetRelayStatusLog(  ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclThermostat_ProcessInCmd_GetWeeklyScheduleRsp
 *
 * @brief   Process in the received Get Weekly Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_GetWeeklyScheduleRsp( zclIncoming_t *pInMsg,
                                                                  zclHVAC_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;
  uint8 modeBuff;   // buffer to determine how to incorporate heat/cool set points
  uint8 numTransitions; // buffer for number of transitions for sequence
  uint16 arrayRecordSize;
  zclThermostatWeeklySchedule_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnHVAC_GetWeeklyScheduleRsp )
  {
    numTransitions = pInMsg->pData[0];
    modeBuff = pInMsg->pData[2];    // store the modeForSequence in buffer

    // if either the heat or cool set points
    if( ( modeBuff == HVAC_THERMOSTAT_MODE_HEAT ) || ( modeBuff == HVAC_THERMOSTAT_MODE_COOL ) )
    {
      // calculate the number of transitions multiplied by size of transitionTime and heatSetPoint/coolSetPoint
      arrayRecordSize = ( numTransitions * 4 );
    }
    // if both cool and heat set points
    else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )
    {
      // calculate the number of transitions multiplied by size of transitionTime, heatSetPoint, and coolSetPoint
      arrayRecordSize = ( numTransitions * 6 );
    }
    else
    {
      return ( ZFailure );    // invalid mode field
    }

    // allocate memory
    cmd.sThermostateSequenceMode.psThermostatModeBoth = zcl_mem_alloc( arrayRecordSize );
    if( !cmd.sThermostateSequenceMode.psThermostatModeBoth )
    {
      return ( ZMemError ); // no memory
    }

    cmd.numberOfTransitionsForSequence = numTransitions;
    cmd.dayOfWeekForSequence = pInMsg->pData[1];
    cmd.modeForSequence = modeBuff;
    offset = 3;
    for( i = 0; i < numTransitions; i++ )
    {
      if( modeBuff == HVAC_THERMOSTAT_MODE_HEAT )   // heat set point
      {
        cmd.sThermostateSequenceMode.psThermostatModeHeat[i].transitionTime = BUILD_UINT16( pInMsg->pData[offset], pInMsg->pData[offset + 1] );
        cmd.sThermostateSequenceMode.psThermostatModeHeat[i].heatSetPoint = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
        offset += 4;
      }
      else if( modeBuff == HVAC_THERMOSTAT_MODE_COOL )    // cool set point
      {
        cmd.sThermostateSequenceMode.psThermostatModeCool[i].transitionTime = BUILD_UINT16( pInMsg->pData[offset], pInMsg->pData[offset + 1] );
        cmd.sThermostateSequenceMode.psThermostatModeCool[i].coolSetPoint = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
        offset += 4;
      }
      else if( modeBuff == HVAC_THERMOSTAT_MODE_BOTH )    // both cool and heat set points
      {
        cmd.sThermostateSequenceMode.psThermostatModeBoth[i].transitionTime = BUILD_UINT16( pInMsg->pData[offset], pInMsg->pData[offset + 1] );
        cmd.sThermostateSequenceMode.psThermostatModeBoth[i].heatSetPoint = BUILD_UINT16( pInMsg->pData[offset + 2], pInMsg->pData[offset + 3] );
        cmd.sThermostateSequenceMode.psThermostatModeBoth[i].coolSetPoint = BUILD_UINT16( pInMsg->pData[offset + 4], pInMsg->pData[offset + 5] );
        offset += 6;
      }
      else
      {
        return ( ZFailure );    // unsupported mode
      }
    }


    status = ( pCBs->pfnHVAC_GetWeeklyScheduleRsp( &cmd ) );
    zcl_mem_free( cmd.sThermostateSequenceMode.psThermostatModeBoth );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclThermostat_ProcessInCmd_GetRelayStatusLogRsp
 *
 * @brief   Process in the received Get Relay Status Log Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclThermostat_ProcessInCmd_GetRelayStatusLogRsp( zclIncoming_t *pInMsg,
                                                                  zclHVAC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnHVAC_GetRelayStatusLogRsp )
  {
    zclThermostatGetRelayStatusLogRsp_t cmd;

    cmd.timeOfDay = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.relayStatus = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
    cmd.localTemperature = BUILD_UINT16( pInMsg->pData[4], pInMsg->pData[5] );
    cmd.humidity = pInMsg->pData[6];
    cmd.setPoint = BUILD_UINT16( pInMsg->pData[7], pInMsg->pData[8] );
    cmd.unreadEntries = BUILD_UINT16( pInMsg->pData[9], pInMsg->pData[10] );

    return ( pCBs->pfnHVAC_GetRelayStatusLogRsp( &cmd ) );
  }

  return ( ZFailure );
}

#endif //ZCL_HVAC_CLUSTER
/****************************************************************************
****************************************************************************/

