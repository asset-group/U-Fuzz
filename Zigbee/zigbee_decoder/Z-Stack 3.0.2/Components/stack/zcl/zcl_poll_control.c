/**************************************************************************************************
  Filename:       zcl_poll_control.c
  Revised:        $Date: 2014-01-15 11:31:23 -0800 (Wed, 15 Jan 2014) $
  Revision:       $Revision: 36854 $

  Description:    Zigbee Cluster Library - Poll Control


  Copyright 2013 - 2014 Texas Instruments Incorporated. All rights reserved.

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
#include "zcl_poll_control.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

#ifdef ZCL_POLL_CONTROL

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclPollControlCBRec
{
  struct zclPollControlCBRec    *next;
  uint8                         endpoint; // Used to link it into the endpoint descriptor
  zclPollControl_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclPollControlCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclPollControlCBRec_t *zclPollControlCBs = (zclPollControlCBRec_t *)NULL;
static uint8 zclPollControlPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclPollControl_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclPollControl_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclPollControl_AppCallbacks_t *zclPollControl_FindCallbacks( uint8 endpoint );
static ZStatus_t zclPollControl_ProcessInCmds( zclIncoming_t *pInMsg, zclPollControl_AppCallbacks_t *pCBs );

static ZStatus_t zclPollControl_ProcessInCmd_CheckIn( zclIncoming_t *pInMsg, zclPollControl_AppCallbacks_t *pCBs );
static ZStatus_t zclPollControl_ProcessInCmd_CheckInRsp( zclIncoming_t *pInMsg, zclPollControl_AppCallbacks_t *pCBs );
static ZStatus_t zclPollControl_ProcessInCmd_FastPollStop( zclIncoming_t *pInMsg, zclPollControl_AppCallbacks_t *pCBs );
static ZStatus_t zclPollControl_ProcessInCmd_SetLongPollInterval( zclIncoming_t *pInMsg, zclPollControl_AppCallbacks_t *pCBs );
static ZStatus_t zclPollControl_ProcessInCmd_SetShortPollInterval( zclIncoming_t *pInMsg, zclPollControl_AppCallbacks_t *pCBs );


/*********************************************************************
 * @fn      zclPollControl_RegisterCmdCallbacks
 *
 * @brief   Register applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclPollControl_RegisterCmdCallbacks( uint8 endpoint, zclPollControl_AppCallbacks_t *callbacks )
{
  zclPollControlCBRec_t *pNewItem;
  zclPollControlCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclPollControlPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                        ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                        zclPollControl_HdlIncoming );
    zclPollControlPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclPollControlCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclPollControlCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclPollControlCBs == NULL )
  {
    zclPollControlCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclPollControlCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclPollControl_Send_CheckIn
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED
 *          (server) to ZR/ZC (client) via binding table. The Rsp will
 *          indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPollControl_Send_CheckIn( uint8 srcEP, afAddrType_t *dstAddr,
                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  // send, no paylod
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                          COMMAND_POLL_CONTROL_CHECK_IN, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, 0, NULL);
}

/*********************************************************************
 * @fn      zclPollControl_Send_CheckInRsp
 *
 * @brief   Call to send out Poll Control CheckInRsp. Sent from ZC/ZR
 *          (client) to ZED (server) This will tell the ZED (server)
 *          who just checked in to stay awake or to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   startFastPolling - tell ZED to stay awake and poll at fast rate
 * @param   fastPollTimeout - number of 1/4 seconds for ZED to stay awake and fast poll
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPollControl_Send_CheckInRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                          uint8 startFastPolling, uint16 fastPollTimeout,
                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[3];   // 3 byte payload

  buf[0] = startFastPolling;
  buf[1] = LO_UINT16(fastPollTimeout);
  buf[2] = HI_UINT16(fastPollTimeout);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                          COMMAND_POLL_CONTROL_CHECK_IN_RSP, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, sizeof(buf), buf );
}

/*********************************************************************
 * @fn      zclPollControl_Send_FastPollStop
 *
 * @brief   Call to send out Poll Control FastPollStop. This will tell the ZED
 *          to stop fast polling and go back to sleep at the long poll rate.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPollControl_Send_FastPollStop( uint8 srcEP, afAddrType_t *dstAddr,
                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                          COMMAND_POLL_CONTROL_FAST_POLL_STOP, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 0, NULL );
}

/*********************************************************************
 * @fn      zclPollControl_Send_SetLongPollInterval
 *
 * @brief   Call to send out a Poll Control Set Long Poll Interval command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   newLongPollInterval - new long poll interval in 1/4 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPollControl_Send_SetLongPollInterval( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint32 newLongPollInterval,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4];

  buf[0] = BREAK_UINT32(newLongPollInterval, 0);
  buf[1] = BREAK_UINT32(newLongPollInterval, 1);
  buf[2] = BREAK_UINT32(newLongPollInterval, 2);
  buf[3] = BREAK_UINT32(newLongPollInterval, 3);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                          COMMAND_POLL_CONTROL_SET_LONG_POLL_INTERVAL, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof(buf), buf );
}

/*********************************************************************
 * @fn      zclPollControl_Send_SetShortPollInterval
 *
 * @brief   Call to send out a Poll Control Set Long Poll Interval command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   newLongPollInterval - new long poll interval in 1/4 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclPollControl_Send_SetShortPollInterval( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint16 newShortPollInterval,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[2];

  buf[0] = LO_UINT16(newShortPollInterval);
  buf[1] = HI_UINT16(newShortPollInterval);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_POLL_CONTROL,
                          COMMAND_POLL_CONTROL_SET_SHORT_POLL_INTERVAL, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, sizeof(buf), buf );
}


/*********************************************************************
 * @fn      zclPollControl_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclPollControl_AppCallbacks_t *zclPollControl_FindCallbacks( uint8 endpoint )
{
  zclPollControlCBRec_t *pCBs;

  pCBs = zclPollControlCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclPollControl_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclPollControl_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclPollControl_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclPollControl_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclPollControl_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclPollControl_FindCallbacks( pInMsg->msg->endPoint );
  if (pCBs == NULL )
  {
    return ( ZFailure );
  }

  stat = zclPollControl_ProcessInCmds( pInMsg, pCBs );

  return ( stat );
}

/*********************************************************************
 * @fn      zclPollControl_ProcessInCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_ProcessInCmds( zclIncoming_t *pInMsg,
                                               zclPollControl_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_POLL_CONTROL_CHECK_IN_RSP:
        stat = zclPollControl_ProcessInCmd_CheckInRsp( pInMsg, pCBs );
        break;

      case COMMAND_POLL_CONTROL_FAST_POLL_STOP:
        stat = zclPollControl_ProcessInCmd_FastPollStop( pInMsg, pCBs );
        break;

      case COMMAND_POLL_CONTROL_SET_LONG_POLL_INTERVAL:
        stat = zclPollControl_ProcessInCmd_SetLongPollInterval( pInMsg, pCBs );
        break;

      case COMMAND_POLL_CONTROL_SET_SHORT_POLL_INTERVAL:
        stat = zclPollControl_ProcessInCmd_SetShortPollInterval( pInMsg, pCBs );
        break;

      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }
  // Sent Server-to-Client
  else
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_POLL_CONTROL_CHECK_IN:
        stat = zclPollControl_ProcessInCmd_CheckIn( pInMsg, pCBs );
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
 * @fn      zclPollControl_ProcessInCmd_CheckIn
 *
 * @brief   Process in the received Poll Control CheckIn cmd (sent to ZC/ZR)
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_ProcessInCmd_CheckIn( zclIncoming_t *pInMsg,
                                                      zclPollControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPollControl_CheckIn )
  {
    zclPollControlCheckIn_t cmd;

    cmd.srcAddr = &(pInMsg->msg->srcAddr);

    return ( pCBs->pfnPollControl_CheckIn( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPollControl_ProcessInCmd_CheckInRsp
 *
 * @brief   Process in the received Poll Control CheckInRsp cmd (sent ZED)
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_ProcessInCmd_CheckInRsp( zclIncoming_t *pInMsg,
                                                         zclPollControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPollControl_CheckInRsp )
  {
    zclPollControlCheckInRsp_t cmd;

    cmd.startFastPolling = pInMsg->pData[0];
    cmd.fastPollTimeOut = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

    return ( pCBs->pfnPollControl_CheckInRsp( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPollControl_ProcessInCmd_FastPollStop
 *
 * @brief   Process in the received Poll Control FastPollStop
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_ProcessInCmd_FastPollStop( zclIncoming_t *pInMsg,
                                                           zclPollControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPollControl_FastPollStop )
  {
    return ( pCBs->pfnPollControl_FastPollStop( ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPollControl_ProcessInCmd_SetLongPollInterval
 *
 * @brief   Process in the received Poll Control SetLongPollInterval
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_ProcessInCmd_SetLongPollInterval( zclIncoming_t *pInMsg,
                                                                  zclPollControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPollControl_SetLongPollInterval )
  {
    zclPollControlSetLongPollInterval_t cmd;

    cmd.newLongPollInterval = BUILD_UINT32( pInMsg->pData[0], pInMsg->pData[1], pInMsg->pData[2], pInMsg->pData[3] );

    return ( pCBs->pfnPollControl_SetLongPollInterval( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclPollControl_ProcessInCmd_SetShortPollInterval
 *
 * @brief   Process in the received Poll Control SetShortPollInterval
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclPollControl_ProcessInCmd_SetShortPollInterval( zclIncoming_t *pInMsg,
                                                                   zclPollControl_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnPollControl_SetShortPollInterval )
  {
    zclPollControlSetShortPollInterval_t cmd;

    cmd.newShortPollInterval = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

    return ( pCBs->pfnPollControl_SetShortPollInterval( &cmd ) );
  }

  return ( ZFailure );
}

/****************************************************************************
****************************************************************************/

#endif // ZCL_POLL_CONTROL