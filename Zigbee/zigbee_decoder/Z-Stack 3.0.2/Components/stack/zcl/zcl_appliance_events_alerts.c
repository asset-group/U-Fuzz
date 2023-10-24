/**************************************************************************************************
  Filename:       zcl_appliance_events_alerts.c
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    Zigbee Cluster Library - Appliance Events & Alerts


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
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_appliance_events_alerts.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

#ifdef ZCL_APPLIANCE_EVENTS_ALERTS
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclApplianceEventsAlertsCBRec
{
  struct zclApplianceEventsAlertsCBRec *next;
  uint8 endpoint;                                   // Used to link it into the endpoint descriptor
  zclApplianceEventsAlerts_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclApplianceEventsAlertsCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclApplianceEventsAlertsCBRec_t *zclApplianceEventsAlertsCBs = (zclApplianceEventsAlertsCBRec_t *)NULL;
static uint8 zclApplianceEventsAlertsPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclApplianceEventsAlerts_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclApplianceEventsAlerts_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclApplianceEventsAlerts_AppCallbacks_t *zclApplianceEventsAlerts_FindCallbacks( uint8 endpoint );
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmds( zclIncoming_t *pInMsg, zclApplianceEventsAlerts_AppCallbacks_t *pCBs );

static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_GetAlerts( zclIncoming_t *pInMsg, zclApplianceEventsAlerts_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_GetAlertsRsp( zclIncoming_t *pInMsg, zclApplianceEventsAlerts_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_AlertsNotification( zclIncoming_t *pInMsg, zclApplianceEventsAlerts_AppCallbacks_t *pCBs );
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_EventNotification( zclIncoming_t *pInMsg, zclApplianceEventsAlerts_AppCallbacks_t *pCBs );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_RegisterCmdCallbacks
 *
 * @brief   Register applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclApplianceEventsAlerts_RegisterCmdCallbacks( uint8 endpoint, zclApplianceEventsAlerts_AppCallbacks_t *callbacks )
{
  zclApplianceEventsAlertsCBRec_t *pNewItem;
  zclApplianceEventsAlertsCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclApplianceEventsAlertsPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,
                        ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,
                        zclApplianceEventsAlerts_HdlIncoming );
    zclApplianceEventsAlertsPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclApplianceEventsAlertsCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclApplianceEventsAlertsCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclApplianceEventsAlertsCBs == NULL )
  {
    zclApplianceEventsAlertsCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclApplianceEventsAlertsCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_GetAlerts
 *
 * @brief   Request sent to server for Get Alerts info.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_GetAlerts( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,
                          COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 0, 0 );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_GetAlertsRsp
 *
 * @brief   Response sent to client due to GetAlertsRsp cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          alertsCount - Contains the length of the alert structure array
 *          aAlert - Contains the information of the Alert
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_GetAlertsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                             zclApplianceEventsAlerts_t *pPayload,
                                                             uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 j;
  uint8 *pBuf;    // variable length payload
  uint8 offset;
  uint16 calculatedBufSize;
  ZStatus_t status;

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = ( (pPayload->alertsCount) * sizeof(alertStructureRecord_t) + 1 );  // size of variable array plus alertsCount

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->alertsCount;
  offset = 1;
  for( i = 0; i < ( pPayload->alertsCount ); i++ )
  {
    for( j = 0; j < 3; j++ )
    {
      pBuf[offset++] = pPayload->pAlertStructureRecord[i].aAlert[j];
    }
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,
                           COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS_RSP, TRUE,
                           ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_AlertsNotification
 *
 * @brief   Response sent to client due to AlertsNotification cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          alertsCount - Contains the length of the alert structure array
 *          aAlert - Contains the information of the Alert
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_AlertsNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                   zclApplianceEventsAlerts_t *pPayload,
                                                                   uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 j;
  uint8 *pBuf;    // variable length payload
  uint8 offset;
  uint16 calculatedBufSize;
  ZStatus_t status;

  // get a buffer large enough to hold the whole packet
  calculatedBufSize = ( (pPayload->alertsCount) * sizeof(alertStructureRecord_t) + 1 );  // size of variable array plus alertsCount

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->alertsCount;
  offset = 1;
  for( i = 0; i < ( pPayload->alertsCount ); i++ )
  {
    for( j = 0; j < 3; j++ )
    {
      pBuf[offset++] = pPayload->pAlertStructureRecord[i].aAlert[j];
    }
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,
                           COMMAND_APPLIANCE_EVENTS_ALERTS_ALERTS_NOTIFICATION, TRUE,
                           ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );

  return ( status );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_EventNotification
 *
 * @brief   Response sent to client for Event Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   eventHeader - a reserved field set to 0
 * @param   eventID - Identifies the event to be notified
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_EventNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint8 eventHeader, uint8 eventID,
                                                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[2]; // 2 byte payload

  buf[0] = eventHeader;
  buf[1] = eventID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS,
                          COMMAND_APPLIANCE_EVENTS_ALERTS_EVENT_NOTIFICATION, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclApplianceEventsAlerts_AppCallbacks_t *zclApplianceEventsAlerts_FindCallbacks( uint8 endpoint )
{
  zclApplianceEventsAlertsCBRec_t *pCBs;

  pCBs = zclApplianceEventsAlertsCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclApplianceEventsAlerts_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclApplianceEventsAlerts_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclApplianceEventsAlerts_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclApplianceEventsAlerts_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclApplianceEventsAlerts_FindCallbacks( pInMsg->msg->endPoint );
  if (pCBs == NULL )
  {
    return ( ZFailure );
  }

  stat = zclApplianceEventsAlerts_ProcessInCmds( pInMsg, pCBs );

  return ( stat );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_ProcessInCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmds( zclIncoming_t *pInMsg, zclApplianceEventsAlerts_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch( pInMsg->hdr.commandID )
    {
      case COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS:
        stat = zclApplianceEventsAlerts_ProcessInCmd_GetAlerts( pInMsg, pCBs );
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
      case COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS_RSP:
        stat = zclApplianceEventsAlerts_ProcessInCmd_GetAlertsRsp( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_EVENTS_ALERTS_ALERTS_NOTIFICATION:
        stat = zclApplianceEventsAlerts_ProcessInCmd_AlertsNotification( pInMsg, pCBs );
        break;

      case COMMAND_APPLIANCE_EVENTS_ALERTS_EVENT_NOTIFICATION:
        stat = zclApplianceEventsAlerts_ProcessInCmd_EventNotification( pInMsg, pCBs );
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
 * @fn      zclApplianceEventsAlerts_ProcessInCmd_GetAlerts
 *
 * @brief   Process in the received Get Alerts cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_GetAlerts( zclIncoming_t *pInMsg,
                                                                  zclApplianceEventsAlerts_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnApplianceEventsAlerts_GetAlerts )
  {
    // no payload

    return ( pCBs->pfnApplianceEventsAlerts_GetAlerts( ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_ProcessInCmd_GetAlertsRsp
 *
 * @brief   Process in the received Get Alerts Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_GetAlertsRsp( zclIncoming_t *pInMsg,
                                                                     zclApplianceEventsAlerts_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 j;
  uint8 offset;
  uint16 arrayRecordSize;
  zclApplianceEventsAlerts_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnApplianceEventsAlerts_GetAlertsRsp )
  {
    // calculate size of variable array, accounting for size of aAlert being a uint24
    arrayRecordSize = 3 * pInMsg->pData[0];

    cmd.pAlertStructureRecord = zcl_mem_alloc( arrayRecordSize );
    if ( !cmd.pAlertStructureRecord )
    {
      return ( ZMemError );  // no memory, return failure
    }

    cmd.alertsCount = pInMsg->pData[0];
    offset = 1;
    for( i = 0; i < pInMsg->pData[0]; i++ )
    {
      for( j = 0; j < 3; j++ )
      {
        cmd.pAlertStructureRecord[i].aAlert[j] = pInMsg->pData[offset++];
      }
    }

    status = ( pCBs->pfnApplianceEventsAlerts_GetAlertsRsp( &cmd ) );
    zcl_mem_free( cmd.pAlertStructureRecord );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_ProcessInCmd_AlertsNotification
 *
 * @brief   Process in the received Alerts Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_AlertsNotification( zclIncoming_t *pInMsg,
                                                                           zclApplianceEventsAlerts_AppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 j;
  uint8 offset;
  uint16 arrayRecordSize;
  zclApplianceEventsAlerts_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnApplianceEventsAlerts_AlertsNotification )
  {
    // calculate size of variable array, accounting for size of aAlert being a uint24
    arrayRecordSize = 3 * pInMsg->pData[0];

    cmd.pAlertStructureRecord = zcl_mem_alloc( arrayRecordSize );
    if ( !cmd.pAlertStructureRecord )
    {
      return ( ZMemError );  // no memory, return failure
    }

    cmd.alertsCount = pInMsg->pData[0];
    offset = 1;
    for( i = 0; i < pInMsg->pData[0]; i++ )
    {
      for( j = 0; j < 3; j++ )
      {
        cmd.pAlertStructureRecord[i].aAlert[j] = pInMsg->pData[offset++];
      }
    }

    status = ( pCBs->pfnApplianceEventsAlerts_AlertsNotification( &cmd ) );
    zcl_mem_free( cmd.pAlertStructureRecord );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_ProcessInCmd_EventNotification
 *
 * @brief   Process in the received Event Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclApplianceEventsAlerts_ProcessInCmd_EventNotification( zclIncoming_t *pInMsg,
                                                                          zclApplianceEventsAlerts_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnApplianceEventsAlerts_EventNotification )
  {
    zclApplianceEventsAlertsEventNotification_t cmd;

    cmd.eventHeader = pInMsg->pData[0];
    cmd.eventID = pInMsg->pData[1];

    return ( pCBs->pfnApplianceEventsAlerts_EventNotification( &cmd ) );
  }

  return ( ZFailure );
}

/****************************************************************************
****************************************************************************/

#endif // ZCL_APPLIANCE_EVENTS_ALERTS