/**************************************************************************************************
  Filename:       zcl_cc.c
  Revised:        $Date: 2013-06-11 13:53:09 -0700 (Tue, 11 Jun 2013) $
  Revision:       $Revision: 34523 $

  Description:    Zigbee Cluster Library - Commissioning Cluster


  Copyright 2011 Texas Instruments Incorporated. All rights reserved.

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


/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_cc.h"

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
typedef struct zclCCCBRec
{
  struct zclCCCBRec    *next;
  uint8                endpoint; // Used to link it into the endpoint descriptor
  zclCC_AppCallbacks_t *CBs;     // Pointer to Callback function
} zclCCCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclCCCBRec_t *zclCCCBs = (zclCCCBRec_t *)NULL;
static uint8 zclCCPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*
 * Callback from ZCL to process incoming Commands specific to this cluster library
 */
static ZStatus_t zclCC_HdlIncoming(  zclIncoming_t *pInMsg );

/* 
 * Function to process incoming Commands specific to this cluster library
 */
static ZStatus_t zclCC_HdlInSpecificCommands( zclIncoming_t *pInMsg );

/*
 * Process received Restart Device Command
 */
static ZStatus_t zclCC_ProcessInCmd_RestartDevice( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Save Startup Parameters Command
 */
static ZStatus_t zclCC_ProcessInCmd_SaveStartupParams( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Restore Startup Parameters Command
 */
static ZStatus_t zclCC_ProcessInCmd_RestoreStartupParams( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Reset Startup Parameters Command
 */
static ZStatus_t zclCC_ProcessInCmd_ResetStartupParams( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Restart Device Response
 */
static ZStatus_t zclCC_ProcessInCmd_RestartDeviceRsp( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Save Startup Parameters Response
 */
static ZStatus_t zclCC_ProcessInCmd_SaveStartupParamsRsp( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Restore Startup Parameters Response
 */
static ZStatus_t zclCC_ProcessInCmd_RestoreStartupParamsRsp( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );

/*
 * Process received Reset Startup Parameters Response
 */
static ZStatus_t zclCC_ProcessInCmd_ResetStartupParamsRsp( zclIncoming_t *pInMsg, zclCC_AppCallbacks_t *pCBs );


/*********************************************************************
 * @fn      zclCC_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZSuccess if successful. ZMemError if not able to allocate
 */
ZStatus_t zclCC_RegisterCmdCallbacks( uint8 endpoint, zclCC_AppCallbacks_t *callbacks )
{
  zclCCCBRec_t *pNewItem;
  zclCCCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclCCPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_GEN_COMMISSIONING,
                        ZCL_CLUSTER_ID_GEN_COMMISSIONING,
                        zclCC_HdlIncoming );
    zclCCPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclCCCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclCCCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclCCCBs == NULL )
  {
    zclCCCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclCCCBs;
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
 * @fn      zclCC_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclCC_AppCallbacks_t *zclCC_FindCallbacks( uint8 endpoint )
{
  zclCCCBRec_t *pCBs;
  
  pCBs = zclCCCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }

    pCBs = pCBs->next;
  }

  return ( (zclCC_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclCC_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclCC_HdlIncoming(  zclIncoming_t *pInMsg )
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
      stat = zclCC_HdlInSpecificCommands( pInMsg );
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
 * @fn      zclCC_HdlInSpecificCommands
 *
 * @brief   Function to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclCC_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat;
  zclCC_AppCallbacks_t *pCBs;
  
  // Make sure endpoint exists
  pCBs = zclCC_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
  {
    return ( ZFailure );
  }

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )  
  {
    // Process Client commands, received by server
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_CC_RESTART_DEVICE:
        stat = zclCC_ProcessInCmd_RestartDevice( pInMsg, pCBs );
        break;

      case COMMAND_CC_SAVE_STARTUP_PARAMS:
        stat = zclCC_ProcessInCmd_SaveStartupParams( pInMsg, pCBs );
        break;

      case COMMAND_CC_RESTORE_STARTUP_PARAMS:
        stat = zclCC_ProcessInCmd_RestoreStartupParams( pInMsg, pCBs );
        break;

      case COMMAND_CC_RESET_STARTUP_PARAMS:
        stat = zclCC_ProcessInCmd_ResetStartupParams( pInMsg, pCBs );
        break;

      default:
        stat = ZFailure;
        break;
    }
  }
  else
  {
    switch ( pInMsg->hdr.commandID )
    {
      case COMMAND_CC_RESTART_DEVICE_RSP:
        stat = zclCC_ProcessInCmd_RestartDeviceRsp( pInMsg, pCBs );
        break;

      case COMMAND_CC_SAVE_STARTUP_PARAMS_RSP:
        stat = zclCC_ProcessInCmd_SaveStartupParamsRsp( pInMsg, pCBs );
        break;

      case COMMAND_CC_RESTORE_STARTUP_PARAMS_RSP:
        stat = zclCC_ProcessInCmd_RestoreStartupParamsRsp( pInMsg, pCBs );
        break;

      case COMMAND_CC_RESET_STARTUP_PARAMS_RSP:
        stat = zclCC_ProcessInCmd_ResetStartupParamsRsp( pInMsg, pCBs );
        break;

      default:
        stat = ZFailure;
        break;
    }
  }
 
  return ( stat );
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_RestartDevice
 *
 * @brief   Process in the received Restart Device Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                
 */
static ZStatus_t zclCC_ProcessInCmd_RestartDevice( zclIncoming_t *pInMsg, 
                                                   zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnRestart_Device )
  {
    zclCCRestartDevice_t cmd;

    cmd.options = pInMsg->pData[0];
    cmd.delay = pInMsg->pData[1];
    cmd.jitter = pInMsg->pData[2];

    pCBs->pfnRestart_Device( &cmd, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZCL_STATUS_CMD_HAS_RSP;
  }

  return ZFailure;
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_SaveStartupParams
 *
 * @brief   Process in the received Save Startup Parameters Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                     
 */
static ZStatus_t zclCC_ProcessInCmd_SaveStartupParams( zclIncoming_t *pInMsg, 
                                                       zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnSave_StartupParams )
  {
    zclCCStartupParams_t cmd;

    cmd.options = pInMsg->pData[0];
    cmd.index = pInMsg->pData[1];

    pCBs->pfnSave_StartupParams( &cmd, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZCL_STATUS_CMD_HAS_RSP;
  }

  return ZFailure;
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_RestoreStartupParams
 *
 * @brief   Process in the received Restore Startup Parameters Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                   
 */
static ZStatus_t zclCC_ProcessInCmd_RestoreStartupParams( zclIncoming_t *pInMsg,
                                                          zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnRestore_StartupParams )
  {
    zclCCStartupParams_t cmd;

    cmd.options = pInMsg->pData[0];
    cmd.index = pInMsg->pData[1];

    pCBs->pfnRestore_StartupParams( &cmd, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZCL_STATUS_CMD_HAS_RSP;
  }

  return ZFailure;
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_ResetStartupParams
 *
 * @brief   Process in the received Reset Startup Parameters Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                    
 */
static ZStatus_t zclCC_ProcessInCmd_ResetStartupParams( zclIncoming_t *pInMsg,
                                                        zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnReset_StartupParams )
  {
    zclCCStartupParams_t cmd;

    cmd.options = pInMsg->pData[0];
    cmd.index = pInMsg->pData[1];

    pCBs->pfnReset_StartupParams( &cmd, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZCL_STATUS_CMD_HAS_RSP;
  }

  return ZFailure; 
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_RestartDeviceRsp
 *
 * @brief   Process in the received Restart Device Response
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                    
 */
static ZStatus_t zclCC_ProcessInCmd_RestartDeviceRsp( zclIncoming_t *pInMsg,
                                                      zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnRestart_DeviceRsp )
  {
    zclCCServerParamsRsp_t rsp;

    rsp.status = pInMsg->pData[0];

    pCBs->pfnRestart_DeviceRsp( &rsp, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZSuccess;
  }

  return ZFailure; 
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_SaveStartupParamsRsp
 *
 * @brief   Process in the received Save Startup Parameters Response
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                 
 */
static ZStatus_t zclCC_ProcessInCmd_SaveStartupParamsRsp( zclIncoming_t *pInMsg,
                                                          zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnSave_StartupParamsRsp )
  {
    zclCCServerParamsRsp_t rsp;

    rsp.status = pInMsg->pData[0];

    pCBs->pfnSave_StartupParamsRsp( &rsp, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZSuccess;
  }

  return ZFailure; 
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_RestoreStartupParamsRsp
 *
 * @brief   Process in the received Restore Startup Parameters Response
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing                  
 */
static ZStatus_t zclCC_ProcessInCmd_RestoreStartupParamsRsp( zclIncoming_t *pInMsg,
                                                             zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnRestore_StartupParamsRsp )
  {
    zclCCServerParamsRsp_t rsp;

    rsp.status = pInMsg->pData[0];

    pCBs->pfnRestore_StartupParamsRsp( &rsp, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZSuccess;
  }

  return ZFailure; 
}

/*********************************************************************
 * @fn      zclCC_ProcessInCmd_ResetStartupParamsRsp
 *
 * @brief   Process in the received Reset Startup Parameters Response
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback
 *
 * @return  ZStatus_t - status of the command processing              
 */
static ZStatus_t zclCC_ProcessInCmd_ResetStartupParamsRsp( zclIncoming_t *pInMsg,
                                                           zclCC_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnReset_StartupParamsRsp )
  {
    zclCCServerParamsRsp_t rsp;

    rsp.status = pInMsg->pData[0];

    pCBs->pfnReset_StartupParamsRsp( &rsp, &(pInMsg->msg->srcAddr), pInMsg->hdr.transSeqNum );

    return ZSuccess;
  }

  return ZFailure; 
}
 
/*********************************************************************
 * @fn      zclCC_Send_RestartDevice
 *
 * @brief   Call to send out a Restart Device command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Restart Command data structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclCC_Send_RestartDevice( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclCCRestartDevice_t *pCmd, 
                                    uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[CC_PACKET_LEN_RESTART_DEVICE];
  
  buf[0] = pCmd->options;
  buf[1] = pCmd->delay;
  buf[2] = pCmd->jitter;
  
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_COMMISSIONING,
                          COMMAND_CC_RESTART_DEVICE, TRUE, 
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, CC_PACKET_LEN_RESTART_DEVICE, buf );
}

/*********************************************************************
 * @fn      zclCC_Send_StartupParamsCmd
 *
 * @brief   Call to send out a Startup parameter command (Restore, Save, Reset)
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Startup Parameter Command data structure
 * @param   cmdId - Command type ( Restore, Save or Reset)
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclCC_Send_StartupParamsCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                       zclCCStartupParams_t *pCmd, uint8 cmdId,
                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[CC_PACKET_LEN_STARTUP_PARAMS_CMD];
  
  buf[0] = pCmd->options;
  buf[1] = pCmd->index;
  
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_COMMISSIONING,
                          cmdId, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 
                          CC_PACKET_LEN_STARTUP_PARAMS_CMD, buf );
}

/*********************************************************************
 * @fn      zclCC_Send_ServerParamsRsp
 *
 * @brief   Call to send out a Server Parameters Response to a client request
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - pointer to Startup Parameter Response data structure
 * @param   cmdId - Command type ( Restore, Save or Reset)
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclCC_Send_ServerParamsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                      zclCCServerParamsRsp_t *pRsp, uint8 cmdId,
                                      uint8 disableDefaultRsp, uint8 seqNum )
{ 
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_GEN_COMMISSIONING,
                          cmdId, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, 
                          CC_PACKET_LEN_SERVER_RSP, &(pRsp->status) );
}


/********************************************************************************************
*********************************************************************************************/
