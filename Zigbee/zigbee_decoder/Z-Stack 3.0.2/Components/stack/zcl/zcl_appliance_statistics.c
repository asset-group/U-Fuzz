/**************************************************************************************************
  Filename:       zcl_appliance_statistics.c
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    Zigbee Cluster Library - Appliance Statistics


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
#include "zcl_appliance_statistics.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

#ifdef ZCL_APPLIANCE_STATISTICS
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclApplianceStatisticsCBRec
{
  struct zclApplianceStatisticsCBRec *next;
  uint8 endpoint;                                 // Used to link it into the endpoint descriptor
  zclApplianceStatistics_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclApplianceStatisticsCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclApplianceStatisticsCBRec_t *zclApplianceStatisticsCBs = (zclApplianceStatisticsCBRec_t *)NULL;
static uint8 zclApplianceStatisticsPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclApplianceStatistics_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclApplianceStatistics_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclApplianceStatistics_AppCallbacks_t *zclApplianceStatistics_FindCallbacks( uint8 endpoint );
static ZStatus_t zclApplianceStatistics_ProcessInCmds( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );

// helper functions
uint8 *zclApplianceStatistics_LogNotification_NativeToOta( zclCmdApplianceStatisticsLogNotificationPayload_t *pPayload , uint16 * pZclPayloadLen );
uint8 *zclApplianceStatistics_LogQueueRsp_NativeToOta( zclCmdApplianceStatisticsLogQueueRspPayload_t *pPayload , uint16 * pZclPayloadLen );
void zclApplianceStatistics_LogNotification_OtaToNative( zclCmdApplianceStatisticsLogNotificationPayload_t *pCmd, uint8 *pData );
ZStatus_t zclApplianceStatistics_LogQueueRsp_OtaToNative( zclCmdApplianceStatisticsLogQueueRspPayload_t *pCmd, uint8 *pData );

// server-side commands
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogNotification( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogRsp( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogQueueRsp( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceStatistics_ProcessInCmd_StastisticsAvailable( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );

// client-side commands
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogReq( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogQueueReq( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs );


/*********************************************************************
 * @fn      zclApplianceStatistics_RegisterCmdCallbacks
 *
 * @brief   Register applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclApplianceStatistics_RegisterCmdCallbacks( uint8 endpoint, zclApplianceStatistics_AppCallbacks_t *callbacks )
{
  zclApplianceStatisticsCBRec_t *pNewItem;
  zclApplianceStatisticsCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclApplianceStatisticsPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                        ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                        zclApplianceStatistics_HdlIncoming );
    zclApplianceStatisticsPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclApplianceStatisticsCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclApplianceStatisticsCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclApplianceStatisticsCBs == NULL )
  {
    zclApplianceStatisticsCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclApplianceStatisticsCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_LogNotification_NativeToOta
 *
 * @brief   Converts from native to OTA format.
 *
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   pZclPayloadLen - returns zcl payload len
 *
 * @return  pointer to zcl payload
*/
uint8 *zclApplianceStatistics_LogNotification_NativeToOta( zclCmdApplianceStatisticsLogNotificationPayload_t *pPayload , uint16 * pZclPayloadLen )
{
  uint8 *pZclPayload;      // OTA ZCL payload
  uint16 zclPayloadLen;    // OTA ZCL payload length
  uint16 i;

  // get a buffer large enough to hold the whole packet
  zclPayloadLen = 12 + (uint16)( pPayload->logLength );   // 12=sizeof(TimeStamp) + sizeof(LogID) + sizeof(LogLength)
  pZclPayload = zcl_mem_alloc( zclPayloadLen );
  if( !pZclPayload )
    return NULL;  // no memory

  // convert from Native to OTA format
  pZclPayload[0] = BREAK_UINT32(pPayload->timeStamp, 0);
  pZclPayload[1] = BREAK_UINT32(pPayload->timeStamp, 1);
  pZclPayload[2] = BREAK_UINT32(pPayload->timeStamp, 2);
  pZclPayload[3] = BREAK_UINT32(pPayload->timeStamp, 3);
  pZclPayload[4] = BREAK_UINT32(pPayload->logID, 0);
  pZclPayload[5] = BREAK_UINT32(pPayload->logID, 1);
  pZclPayload[6] = BREAK_UINT32(pPayload->logID, 2);
  pZclPayload[7] = BREAK_UINT32(pPayload->logID, 3);
  pZclPayload[8] = BREAK_UINT32(pPayload->logLength, 0);
  pZclPayload[9] = BREAK_UINT32(pPayload->logLength, 1);
  pZclPayload[10] = BREAK_UINT32(pPayload->logLength, 2);
  pZclPayload[11] = BREAK_UINT32(pPayload->logLength, 3);
  for( i = 0; i < pPayload->logLength; ++i )
    pZclPayload[12 + i] = pPayload->pLogPayload[i];

  // return payload and length
  *pZclPayloadLen = zclPayloadLen;
  return pZclPayload;
}

/*********************************************************************
 * @fn      zclApplianceStatistics_LogQueueRsp_NativeToOta
 *
 * @brief   Converts from native to OTA format.
 *
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   pZclPayloadLen - returns zcl payload len
 *
 * @return  pointer to zcl payload
*/
uint8 *zclApplianceStatistics_LogQueueRsp_NativeToOta( zclCmdApplianceStatisticsLogQueueRspPayload_t *pPayload , uint16 * pZclPayloadLen )
{
  uint8 *pZclPayload;      // OTA ZCL payload
  uint16 zclPayloadLen;    // OTA ZCL payload length
  uint16 i;
  uint16 offset;

  // allocate some memory for the ZCL payload
  zclPayloadLen = 1 + (pPayload->logQueueSize * sizeof(uint32));    // # of LogQueue IDs
  pZclPayload = zcl_mem_alloc( zclPayloadLen );
  if( !pZclPayload )
  {
    return NULL;  // no memory
  }

  // fill in payload
  pZclPayload[0] = pPayload->logQueueSize;
  offset = 1;
  for( i = 0; i < pPayload->logQueueSize; ++i )
  {
    pZclPayload[offset]   = BREAK_UINT32(pPayload->pLogID[i], 0);
    pZclPayload[offset+1] = BREAK_UINT32(pPayload->pLogID[i], 1);
    pZclPayload[offset+2] = BREAK_UINT32(pPayload->pLogID[i], 2);
    pZclPayload[offset+3] = BREAK_UINT32(pPayload->pLogID[i], 3);
    offset += sizeof( uint32 );
  }

  // return OTA payload and length
  *pZclPayloadLen = zclPayloadLen;
  return pZclPayload;
}

/*********************************************************************
 * @fn      zclApplianceStatistics_LogQueueRsp_OtaToNative
 *
 * @brief   Converts from native to OTA format. Make sure to free pCmd->pLogID if ZSuccess.
 *
 * @param   pCmd - native format (destination)
 * @param   pZclPayloadLen - OTA ZCL payload (source)
 *
 * @return  ZSuccess if worked
*/
ZStatus_t zclApplianceStatistics_LogQueueRsp_OtaToNative( zclCmdApplianceStatisticsLogQueueRspPayload_t *pCmd, uint8 *pData )
{
  uint8  offset;
  uint8  i;

  // convert from OTA (stream) format to native format
  pCmd->logQueueSize = pData[0];
  pCmd->pLogID = zcl_mem_alloc( pCmd->logQueueSize * sizeof(uint32) );
  if( !pCmd->pLogID )
  {
    return ( ZMemError );
  }

  offset = 1;
  for( i = 0; i < pCmd->logQueueSize; ++i )
  {
    pCmd->pLogID[i] = BUILD_UINT32( pData[offset], pData[offset+1], pData[offset+2], pData[offset+3] );
    offset += sizeof(uint32);
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_LogNotification_OtaToNative
 *
 * @brief   Converts from native to OTA format.
 *
 * @param   pCmd - native format (destination)
 * @param   pZclPayloadLen - OTA ZCL payload (source)
 *
 * @return  none
*/
void zclApplianceStatistics_LogNotification_OtaToNative( zclCmdApplianceStatisticsLogNotificationPayload_t *pCmd, uint8 *pData )
{
  pCmd->timeStamp = BUILD_UINT32( pData[0], pData[1], pData[2], pData[3] );
  pCmd->logID     = BUILD_UINT32( pData[4], pData[5], pData[6], pData[7] );
  pCmd->logLength = BUILD_UINT32( pData[8], pData[9], pData[10], pData[11] );
  pCmd->pLogPayload = &pData[12];
}

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogNotification
 *
 * @brief   Request sent to client for Log Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          timeStamp - timestamp of the notification
 *          logID - identifies uniquely the log information contained in log payload
 *          logLength - indicates the length in bytes of log payload
 *          logPayload - variable length payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_LogNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                       zclCmdApplianceStatisticsLogNotificationPayload_t *pPayload,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pZclPayload;      // OTA ZCL Payload
  uint16 zclPayloadLen;
  ZStatus_t status;

  // convert from Native to OTA format
  pZclPayload = zclApplianceStatistics_LogNotification_NativeToOta( pPayload , &zclPayloadLen );
  if( !pZclPayload )
  {
    return ZFailure;  // no memory
  }

  // sent over the air
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                          COMMAND_APPLIANCE_STATISTICS_LOG_NOTIFICATION, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, zclPayloadLen, pZclPayload );

  // done with packet
  zcl_mem_free( pZclPayload );

  return status;
}

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogRsp
 *
 * @brief   Response sent to client from LogReq cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          timeStamp - timestamp of the notification
 *          logID - identifies uniquely the log information contained in log payload
 *          logLength - indicates the length in bytes of log payload
 *          logPayload - variable length payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_LogRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                     zclCmdApplianceStatisticsLogRspPayload_t *pPayload,
                                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pZclPayload;      // OTA ZCL Payload
  uint16 zclPayloadLen;
  ZStatus_t status;

  // convert from Native to OTA format
  // note: zclCmdApplianceStatisticsLogRspPayload_t is same as zclCmdApplianceStatisticsLogNotificationPayload_t
  pZclPayload = zclApplianceStatistics_LogNotification_NativeToOta( (void *)pPayload , &zclPayloadLen );
  if( !pZclPayload )
  {
    return ZFailure;  // no memory
  }

  // send over the air
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                          COMMAND_APPLIANCE_STATISTICS_LOG_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, zclPayloadLen, pZclPayload );
  zcl_mem_free( pZclPayload );

  return status;
}


/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogQueueRsp
 *
 * @brief   Response sent to client from LogQueueReq cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          logQueueSize - defines the # of logID records
 *          logID - variable length list of logIDs
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_LogQueueRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                   zclCmdApplianceStatisticsLogQueueRspPayload_t *pPayload,
                                                   uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pZclPayload;      // OTA ZCL Payload
  uint16 zclPayloadLen;
  ZStatus_t status;

  // convert from native to OTA format
  pZclPayload = zclApplianceStatistics_LogQueueRsp_NativeToOta( pPayload , &zclPayloadLen );

  // send over the air
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                          COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_RSP, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, zclPayloadLen, pZclPayload );
  zcl_mem_free( pZclPayload );

  return status;
}

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_StatisticsAvailable
 *
 * @brief   Request sent to client for Statistics Available.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          logQueueSize - # elements in the logID list
 *          logID - variable length list of logIDs
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_StatisticsAvailable( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  zclCmdApplianceStatisticsStatisticsAvailablePayload_t *pPayload,
                                                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pZclPayload;      // OTA ZCL Payload
  uint16 zclPayloadLen;
  ZStatus_t status;

  // convert from native to OTA format
  // note: zclCmdApplianceStatisticsStatisticsAvailablePayload_t and zclCmdApplianceStatisticsLogQueueRspPayload_t are the same format
  pZclPayload = zclApplianceStatistics_LogQueueRsp_NativeToOta( (void *)pPayload , &zclPayloadLen );

  // send over the air
  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                          COMMAND_APPLIANCE_STATISTICS_STATISTICS_AVAILABLE, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, zclPayloadLen, pZclPayload );
  zcl_mem_free( pZclPayload );

  return status;
}

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogReq
 *
 * @brief   Request sent to server for Log Request.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   logID - identifies uniquely the log information contained in log payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_LogReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                     uint32 logID,
                                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4];   // 4 byte payload

  buf[0] = BREAK_UINT32(logID, 0);
  buf[1] = BREAK_UINT32(logID, 1);
  buf[2] = BREAK_UINT32(logID, 2);
  buf[3] = BREAK_UINT32(logID, 3);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                          COMMAND_APPLIANCE_STATISTICS_LOG_REQ, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogQueueReq
 *
 * @brief   Request sent to server for Log Queue Request.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_LogQueueReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS,
                          COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_REQ, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclApplianceStatistics_AppCallbacks_t *zclApplianceStatistics_FindCallbacks( uint8 endpoint )
{
  zclApplianceStatisticsCBRec_t *pCBs;

  pCBs = zclApplianceStatisticsCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclApplianceStatistics_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclApplianceStatistics_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclApplianceStatistics_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclApplianceStatistics_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclApplianceStatistics_FindCallbacks( pInMsg->msg->endPoint );
  if (pCBs == NULL )
  {
    return ( ZFailure );
  }

  stat = zclApplianceStatistics_ProcessInCmds( pInMsg, pCBs );

  return ( stat );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_ProcessInCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmds( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_APPLIANCE_STATISTICS_LOG_REQ:
        stat = zclApplianceStatistics_ProcessInCmd_LogReq( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_REQ:
        stat = zclApplianceStatistics_ProcessInCmd_LogQueueReq( pInMsg, pCBs );
        break;

      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }
  // sent Server-to-Client
  else
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_APPLIANCE_STATISTICS_LOG_NOTIFICATION:
        stat = zclApplianceStatistics_ProcessInCmd_LogNotification( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_STATISTICS_LOG_RSP:
        stat = zclApplianceStatistics_ProcessInCmd_LogRsp( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_RSP:
        stat = zclApplianceStatistics_ProcessInCmd_LogQueueRsp( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_STATISTICS_STATISTICS_AVAILABLE:
        stat = zclApplianceStatistics_ProcessInCmd_StastisticsAvailable( pInMsg, pCBs );
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
 * @fn      zclApplianceStatistics_ProcessInCmd_LogNotification
 *
 * @brief   Process in the received Appliance Statistics Log Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callback
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogNotification( zclIncoming_t *pInMsg,
                                                                      zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  zclCmdApplianceStatisticsLogNotificationPayload_t cmd;

  if ( pCBs->pfnApplianceStatistics_LogNotification )
  {
    // convert from OTA format to native format (note: log data is not converted)
    zclApplianceStatistics_LogNotification_OtaToNative( &cmd, &pInMsg->pData[0] );

    // call user callback
    return ( pCBs->pfnApplianceStatistics_LogNotification( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_ProcessInCmd_LogRsp
 *
 * @brief   Process in the received Appliance Statistics Log Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogRsp( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  zclCmdApplianceStatisticsLogRspPayload_t cmd;

  if ( pCBs->pfnApplianceStatistics_LogRsp )
  {
    // convert from OTA format to native format (note: log data is not converted)
    // note: LogRsp is same format as Log Notification
    zclApplianceStatistics_LogNotification_OtaToNative( (void *)(&cmd), &pInMsg->pData[0] );

    // call user callback
    return ( pCBs->pfnApplianceStatistics_LogRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_ProcessInCmd_LogQueueRsp
 *
 * @brief   Process in the received Appliance Statistics Log Queue Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callback
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogQueueRsp( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  zclCmdApplianceStatisticsLogQueueRspPayload_t cmd;
  ZStatus_t status = ZFailure;

  if ( pCBs->pfnApplianceStatistics_LogQueueRsp )
  {
    // convert from OTA format to native format
    status = zclApplianceStatistics_LogQueueRsp_OtaToNative( &cmd, &pInMsg->pData[0] );

    // successfully converted (and log allocated)
    if( status  == ZSuccess )
    {
      // call user callback
      status = pCBs->pfnApplianceStatistics_LogQueueRsp( &cmd );
      zcl_mem_free( cmd.pLogID );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_ProcessInCmd_StastisticsAvailable
 *
 * @brief   Process in the received Appliance Statistics Available cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callback
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmd_StastisticsAvailable( zclIncoming_t *pInMsg,
                                                                      zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  zclCmdApplianceStatisticsStatisticsAvailablePayload_t cmd;
  ZStatus_t status = ZFailure;

  if ( pCBs->pfnApplianceStatistics_StatisticsAvailable )
  {
    // convert from OTA format to native format
    status = zclApplianceStatistics_LogQueueRsp_OtaToNative( (void *)(&cmd), &pInMsg->pData[0] );

    // successfully converted (and log allocated)
    if( status  == ZSuccess )
    {
      // call user callback
      status = pCBs->pfnApplianceStatistics_StatisticsAvailable( &cmd );
      zcl_mem_free( cmd.pLogID );
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_ProcessInCmd_LogReq
 *
 * @brief   Process in the received Appliance Statistics Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callback
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogReq( zclIncoming_t *pInMsg,
                                                             zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  zclApplianceStatisticsLogReq_t cmd;

  if ( pCBs->pfnApplianceStatistics_LogReq )
  {
    // convert from OTA format to native format (note: log data is not converted)
    cmd.logID = BUILD_UINT32( pInMsg->pData[0], pInMsg->pData[1], pInMsg->pData[2], pInMsg->pData[3] );

    // call user callback
    return ( pCBs->pfnApplianceStatistics_LogReq( pInMsg, &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceStatistics_ProcessInCmd_LogQueueReq
 *
 * @brief   Process in the received Appliance Log Queue Request cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceStatistics_ProcessInCmd_LogQueueReq( zclIncoming_t *pInMsg, zclApplianceStatistics_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnApplianceStatistics_LogQueueReq )
  {
    // call user callback
    return ( pCBs->pfnApplianceStatistics_LogQueueReq( pInMsg ) );
  }

  return ( ZFailure );
}


/****************************************************************************
****************************************************************************/

#endif // ZCL_APPLIANCE_STATISTICS
