/**************************************************************************************************
  Filename:       bdb_touchlink_target.c
  Revised:        $Date: 2013-11-26 15:12:49 -0800 (Tue, 26 Nov 2013) $
  Revision:       $Revision: 36298 $

  Description:    Zigbee Cluster Library - Light Link Target.


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


/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_Nv.h"
#include "AF.h"
#include "ZDApp.h"
#include "nwk_util.h"
#include "AddrMgr.h"
#include "ZDSecMgr.h"

#if defined( INTER_PAN )
  #include "stub_aps.h"
#endif

#include "zcl.h"
#include "zcl_general.h"
#include "bdb.h"
#include "bdb_interface.h"
#include "bdb_tlCommissioning.h"
#include "bdb_touchLink.h"

#include "bdb_touchlink_target.h"

#include "OSAL_PwrMgr.h"

#if defined ( BDB_TL_TARGET )
   
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 touchLinkTarget_TaskID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Info related to the received request
static afAddrType_t dstAddr;
static bdbTLReq_t rxReq; // network start or join request
static uint8 rxSeqNum;
static uint8 initiatorRxChannel;
static bool targetJoinedNwk;
static bool touchlinkAllowStealing = BDB_ALLOW_TL_STEALING;
tlGCB_TargetEnable_t pfnTargetEnableChangeCB = NULL;

bool touchlink_target_perpetual_operation;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static ZStatus_t targetScanReqCB( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8 seqNum );
static ZStatus_t targetDeviceInfoReqCB( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8 seqNum );
static ZStatus_t targetIdentifyReqCB( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq );
static ZStatus_t targetResetToFNReqCB( afAddrType_t *srcAddr, bdbTLResetToFNReq_t *pReq );
static ZStatus_t targetNwkStartReqCB( afAddrType_t *srcAddr, bdbTLNwkStartReq_t *pReq, uint8 seqNum );
static ZStatus_t targetNwkJoinReqCB( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8 seqNum );
static ZStatus_t targetNwkUpdateReqCB( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq );
static void *targetZdoLeaveCnfCB( void *pParam );
static void targetProcessStateChange( devStates_t devState );
static ZStatus_t touchLink_TargetSendScanRsp( uint8 srcEP, afAddrType_t *dstAddr, uint32 transID, uint8 seqNum );
static void targetSelectNwkParams( void );
static ZStatus_t targetVerifyNwkParams( uint16 PANID, uint8 *pExtendedPANID );
#if (ZSTACK_ROUTER_BUILD)
static void targetSendNwkStartRsp( afAddrType_t *dstAddr, uint32 transID, uint8 status,
                                   bdbTLNwkParams_t *pNwkParams, uint8 nwkUpdateId, uint8 seqNum );
#endif  //  ZSTACK_ROUTER_BUILD

/*********************************************************************
 * TouchLink Target Callback Table
 */
// Target Command Callbacks table
static bdbTL_InterPANCallbacks_t touchLinkTarget_CmdCBs =
{
  // Received Server Commands
  targetScanReqCB,          // Scan Request command
  targetDeviceInfoReqCB,    // Device Information Request command
  targetIdentifyReqCB,      // Identify Request command
  targetResetToFNReqCB,     // Reset to Factory New Request command
  targetNwkStartReqCB,      // Network Start Request command
  #if ( ZSTACK_ROUTER_BUILD )
  targetNwkJoinReqCB,       // Network Join Router Request command
  NULL,                     // Network Join End Device Request command
#else
  NULL,                     // Network Join Router Request command
  targetNwkJoinReqCB,       // Network Join End Device Request command
#endif
  targetNwkUpdateReqCB,     // Network Update Request command

  // Received Client Commands
  NULL,                     // Scan Response command
  NULL,                     // Device Information Response command
  NULL,                     // Network Start Response command
  NULL,                     // Network Join Router Response command
  NULL                      // Network Join End Device Response command
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLinkTarget_InitDevice
 *
 * @brief   Start the TouchLink Target device in the network if it's not
 *          factory new. Otherwise, determine the network parameters
 *          and wait for a touchlink command.
 *
 * @param   none
 *
 * @return  status
 */
ZStatus_t touchLinkTarget_InitDevice( void )
{
  ZDO_Config_Node_Descriptor.LogicalType = zgDeviceLogicalType;
  
  uint8 x = TRUE;

  // Enable our receiver
  ZMacSetReq( ZMacRxOnIdle, &x );
  
  if( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    targetSelectNwkParams( );
  }

  // Wait for a touchlink command
  touchLinkTarget_PermitJoin(0);

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      touchLinkTarget_Init
 *
 * @brief   Initialization function for the TouchLink Target task.
 *
 * @param   task_id - TouchLink Target task id
 *
 * @return  none
 */
void touchLinkTarget_Init( uint8 task_id )
{
  // Save our own Task ID
  touchLinkTarget_TaskID = task_id;

  touchLink_SetTouchLinkTaskId( touchLinkTarget_TaskID );

  // Initialize TouchLink common variables
  touchLink_InitVariables( FALSE );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( touchLinkTarget_TaskID );

  // Register for TouchLink Target callbacks (for Inter-PAN commands)
  bdbTL_RegisterInterPANCmdCallbacks( &touchLinkTarget_CmdCBs );

  // Register for Initiator to receive Leave Confirm
  ZDO_RegisterForZdoCB( ZDO_LEAVE_CNF_CBID, targetZdoLeaveCnfCB );

  // Register to process ZDO messages
  ZDO_RegisterForZDOMsg( touchLinkTarget_TaskID, Mgmt_Permit_Join_req );
  ZDO_RegisterForZDOMsg( touchLinkTarget_TaskID, Device_annce );
}

/*********************************************************************
 * @fn      touchLinkTarget_PermitJoin
 *
 * @brief   Set the router permit join flag, to allow or deny classical
 *          commissioning by other ZigBee devices.
 *
 * @param   duration - enable up to aplcMaxPermitJoinDuration seconds,
 *                     0 to disable
 *
 * @return  status
 */
ZStatus_t touchLinkTarget_PermitJoin( uint8 duration )
{
  return touchLink_PermitJoin( duration );
}

/*********************************************************************
 * @fn      touchLinkTarget_event_loop
 *
 * @brief   Event Loop Processor for TouchLink Target.
 *
 * @param   task_id - task id
 * @param   events - event bitmap
 *
 * @return  unprocessed events
 */
uint16 touchLinkTarget_event_loop( uint8 task_id, uint16 events )
{ 
  if ( events & SYS_EVENT_MSG )
  {
    osal_event_hdr_t *pMsg;

    if ( (pMsg = (osal_event_hdr_t *)osal_msg_receive( task_id )) != NULL )
    {
      switch (pMsg->event )
      {
        case ZDO_CB_MSG:
          // ZDO sends the message that we registered for
          touchLink_RouterProcessZDOMsg( (zdoIncomingMsg_t *)pMsg );
          break;

        case ZDO_STATE_CHANGE:
          targetProcessStateChange( (devStates_t)pMsg->status );
          break;

        default:
          break;
      }

      // Release the OSAL message
      VOID osal_msg_deallocate( (uint8 *)pMsg );
    }

    // return unprocessed events
    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & TOUCHLINK_NWK_START_EVT )
  {
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);

    // If the PAN Id, Extended PAN Id or Logical Channel are zero then
    // determine each of these parameters
    if ( !nwk_ExtPANIDValid( pReq->nwkParams.extendedPANID ) )
    {
      touchLink_GenerateRandNum( pReq->nwkParams.extendedPANID, Z_EXTADDR_LEN );
    }

    if ( pReq->nwkParams.panId == 0 )
    {
      pReq->nwkParams.panId = osal_rand();
    }

    if ( pReq->nwkParams.logicalChannel == 0 )
    {
      pReq->nwkParams.logicalChannel = touchLink_GetRandPrimaryChannel();
    }

    if ( pReq->nwkParams.nwkAddr == 0 )
    {
      pReq->nwkParams.nwkAddr = osal_rand();
    }

    // Perform Network Discovery to verify our new network parameters uniqeness
    touchLink_PerformNetworkDisc( (uint32)1 << pReq->nwkParams.logicalChannel );
    
    initiatorRxChannel = _NIB.nwkLogicalChannel;

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_START_EVT );
  }
  
  if ( events & TOUCHLINK_NWK_FORMATION_SUCCESS_EVT )
  {
#if (ZSTACK_ROUTER_BUILD)
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);
    
    if( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
    {
      if ( targetJoinedNwk == FALSE )
      {
        // Tune to the channel that the Scan Response was heard on
        touchLink_SetChannel( initiatorRxChannel );
      
        touchLinkTarget_PermitJoin(APLC_MAX_PERMIT_JOIN_DURATION);

        // Send a response back
        targetSendNwkStartRsp( &dstAddr, pReq->transID, TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS, pParams, _NIB.nwkUpdateId, rxSeqNum );
      }
      
      zTouchLinkNwkStartRtr = FALSE;
      
      osal_start_timerEx( touchLinkTarget_TaskID, TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT, 50 );
    }
#else
    (void)targetJoinedNwk;
    (void)initiatorRxChannel;
#endif
    return ( events ^ TOUCHLINK_NWK_FORMATION_SUCCESS_EVT );
  }
      
  if ( events & TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT )
  {
#if (ZSTACK_ROUTER_BUILD)
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);
    
    if ( osal_get_timeoutEx( touchLinkTarget_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT ) )
    {
      osal_stop_timerEx( touchLinkTarget_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT );
    }
    
    if( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
    {
      // Tune back to our channel
      touchLink_SetChannel( pParams->logicalChannel );
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, TRUE );
    }
#endif
    return ( events ^ TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT );
  }

  if ( events & TOUCHLINK_NWK_DISC_CNF_EVT )
  {
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);
    uint8 status;
    
    // Verify the received Network Parameters
    if ( targetVerifyNwkParams( pParams->panId, pParams->extendedPANID ) == ZSuccess )
    {
      status = TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS;
    }
    else
    {
      status = TOUCHLINK_NETWORK_START_RSP_STATUS_FAILURE;
    }
    
    
    if ( status == TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS )
    {
      touchLink_FreeNwkParamList();
      // If not factory new, perform a Leave on our old network
      if ( ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) && ( touchLink_SendLeaveReq() == ZSuccess ) )
      {
        // Wait for Leave confirmation before joining the new network
        touchLinkLeaveInitiated = TOUCHLINK_LEAVE_TO_START_NWK;
      }
      else
      {
        // Notify our task to start the network
        osal_set_event( touchLinkTarget_TaskID, TOUCHLINK_START_NWK_EVT );
      }
    }
    else
    {
      // Join to the chosen network
      osal_set_event( touchLinkTarget_TaskID, TOUCHLINK_JOIN_ATTEMPT_EVT );
    }
    
    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_DISC_CNF_EVT );
  }

  if ( events & TOUCHLINK_NWK_JOIN_IND_EVT )
  {
    bdbTLNwkJoinReq_t *pReq = &(rxReq.nwkJoinReq);
    
    initiatorRxChannel = pReq->nwkParams.logicalChannel;
    
    // If not factory new, perform a Leave on our old network
    if ( ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) && ( touchLink_SendLeaveReq() == ZSuccess ) )
    {
      // Wait for Leave confirmation before joining the new network
      touchLinkLeaveInitiated = TOUCHLINK_LEAVE_TO_JOIN_NWK;
    }
    else
    {
      // Notify our task to join this network
      // Perform Network Discovery to verify our new network parameters uniqeness
      touchLink_PerformNetworkDisc( (uint32)1 << initiatorRxChannel );
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_JOIN_IND_EVT );
  }

  if ( events & TOUCHLINK_START_NWK_EVT )
  {
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_TL;

    // Start operating on the new network
    touchLinkStartRtr( &(pReq->nwkParams), pReq->transID );

    // Perform a ZigBee Direct Join in order to allow direct communication
    // via the ZigBee network between the Initiator and the Target (i.e.,
    // create an entry in the neighbor table with the IEEE address and the
    // network address of the Initiator).
    NLME_DirectJoinRequestWithAddr( pReq->initiatorIeeeAddr, pReq->initiatorNwkAddr,
                                    CAPINFO_DEVICETYPE_RFD );

    // return unprocessed events
    return ( events ^ TOUCHLINK_START_NWK_EVT );
  }

  if ( events & TOUCHLINK_JOIN_ATTEMPT_EVT )
  {
    bdbTLNwkJoinReq_t *pReq = &(rxReq.nwkJoinReq);
    
    initiatorRxChannel = pReq->nwkParams.logicalChannel;
    
    // Copy the new network parameters to NIB
    touchLink_SetNIB( ( ZSTACK_ROUTER_BUILD ? NWK_ROUTER : NWK_REJOINING ),
                pReq->nwkParams.nwkAddr, pReq->nwkParams.extendedPANID,
                pReq->nwkParams.logicalChannel, pReq->nwkParams.panId, pReq->nwkUpdateId );

    // Apply the received network key
    touchLink_DecryptNwkKey( pReq->nwkParams.nwkKey, pReq->nwkParams.keyIndex, pReq->transID, touchLinkResponseID );

    // This is not a usual Trust Center protected network
    ZDSecMgrUpdateTCAddress( NULL );
    
#if ( ZSTACK_ROUTER_BUILD )
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_TL;
    
    // Start operating on the new network
    touchLinkStartRtr( &(pReq->nwkParams), pReq->transID );
#else
    bdbTLNwkRejoin_t rejoinInf;
         
    rejoinInf.panId = pReq->nwkParams.panId;
    rejoinInf.logicalChannel = pReq->nwkParams.logicalChannel;
    osal_memcpy( rejoinInf.extendedPANID, pReq->nwkParams.extendedPANID, Z_EXTADDR_LEN);
    rejoinInf.nwkAddr = pDiscoveredNwkParamList->chosenRouter;
    rejoinInf.nwkUpdateId = pReq->nwkUpdateId;
      
    touchLink_DevRejoin( &rejoinInf );
#endif
    touchLink_FreeNwkParamList();
    
    // return unprocessed events
    return ( events ^ TOUCHLINK_JOIN_ATTEMPT_EVT );
  }

  if ( events & TOUCHLINK_RESET_TO_FN_EVT )
  {
    bdb_resetLocalAction();
    
    // return unprocessed events
    return ( events ^ TOUCHLINK_RESET_TO_FN_EVT );
  }

  if ( events & TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT )
  {
    touchLinkTransID = 0;
    // return unprocessed events
    return ( events ^ TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT );
  }

  if(events & TOUCHLINK_TARGET_ENABLE_TIMEOUT)
  {
    touchLinkTargetEnabled = FALSE;
    touchlink_target_perpetual_operation = FALSE;
    pfnTargetEnableChangeCB( touchLinkTargetEnabled );  
    (void)osal_pwrmgr_task_state(touchLinkTarget_TaskID, PWRMGR_CONSERVE);
    
    bdb_ClearNetworkParams();    
    
    return (events ^ TOUCHLINK_TARGET_ENABLE_TIMEOUT);
  }
  // If reach here, the events are unknown
  // Discard or make more handlers
  return 0;
}

/*********************************************************************
 * @fn      touchLinkTarget_EnableCommissioning
 *
 * @brief   Enable the reception of TL Commissioning commands. Refer to 
 *          bdb_RegisterTouchlinkTargetEnableCB to get enable/disable notifications
 *
 * @param   timeoutTime - Enable timeout in ms
 *
 * @return  status
 */
void touchLinkTarget_EnableCommissioning( uint32 timeoutTime )
{
  touchLinkTargetEnabled = TRUE;

  touchLinkTarget_InitDevice( );
  
  // if time == 0xFFFF set target active forever, otherwise disable it in
  // the timeout given by timeoutTime in ms
  if ( timeoutTime < TOUCHLINK_TARGET_PERPETUAL )
  {
    osal_start_timerEx(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT, timeoutTime);
    touchlink_target_perpetual_operation = FALSE;
  }
  else
  {
    touchlink_target_perpetual_operation = TRUE;
  }
  pfnTargetEnableChangeCB( touchLinkTargetEnabled );
  (void)osal_pwrmgr_task_state(touchLinkTarget_TaskID, PWRMGR_HOLD);
}

/*********************************************************************
 * @fn      touchLinkTarget_DisableCommissioning
 *
 * @brief   Disable TouchLink on a target device
 *
 * @param   none
 *
 * @return  none
 */
void touchLinkTarget_DisableCommissioning( void )
{
  osal_stop_timerEx(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT);
  osal_set_event(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT);
}

/*********************************************************************
 * @fn      touchLinkTarget_GetTimer
 *
 * @brief   Get remaining touchlink duration on a target device
 *
 * @param   none
 *
 * @return  Remaining touchlink duration in milliseconds
 */
uint32 touchLinkTarget_GetTimer( void )
{
  if (!touchLinkTargetEnabled)
  {
  	return 0;
  }
  else if (touchlink_target_perpetual_operation)
  {
  	return TOUCHLINK_TARGET_PERPETUAL;
  }

  return osal_get_timeoutEx(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT);
}

/*********************************************************************
 * @fn      bdb_RegisterTouchlinkTargetEnableCB
 *
 * @brief   Register an Application's Enable/Disable callback function. 
 *          Refer to touchLinkTarget_EnableCommissioning to enable/disable TL as target
 *
 * @param   pfnIdentify - application callback
 *
 * @return  none
 */
void bdb_RegisterTouchlinkTargetEnableCB( tlGCB_TargetEnable_t pfnTargetEnableChange )
{
  pfnTargetEnableChangeCB = pfnTargetEnableChange;
}

/*********************************************************************
 * @fn      bdb_TouchlinkSetAllowStealing
 *
 * @brief   General function to allow stealing when performing TL as target
 *
 * @param   allow - allow stealling if TRUE, deny if FALSE
 *
 * @return  none
 */
void bdb_TouchlinkSetAllowStealing( bool allow )
{
  touchlinkAllowStealing = allow;
}

/*********************************************************************
 * @fn      bdb_TouchlinkGetAllowStealing
 *
 * @brief   General function to get the allow stealing value
 *
 * @param   
 *
 * @return  return TRUE if allowed, FALSE if not allowed
 */
bool bdb_TouchlinkGetAllowStealing( void )
{
  return touchlinkAllowStealing;
}
/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      targetScanReqCB
 *
 * @brief   This callback is called to process a Scan Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetScanReqCB( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8 seqNum )
{
  ZStatus_t ret = ZSuccess;
  int8 rssi;
  
  if( pReq->touchLinkInitiator == FALSE )
  {
    return ZFailure;
  }
  
  rssi = touchLink_GetMsgRssi();
  if( rssi > TOUCHLINK_WORST_RSSI )
  {
    if ( pDiscoveredNwkParamList == NULL )
    {
      dstAddr = *srcAddr;
      dstAddr.panId = 0xFFFF;

      ret = touchLink_TargetSendScanRsp( TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, pReq->transID, seqNum );
      if ( ret == ZSuccess )
      {
        touchLinkTransID = pReq->transID;
      }
    }
  }

  return ( ret );
}

/*********************************************************************
 * @fn      targetDeviceInfoReqCB
 *
 * @brief   This callback is called to process a Device Information
 *          Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetDeviceInfoReqCB( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8 seqNum )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }
  return ( touchLink_SendDeviceInfoRsp( TOUCHLINK_INTERNAL_ENDPOINT, srcAddr,
                                  pReq->startIndex, pReq->transID, seqNum ) );
}

/*********************************************************************
 * @fn      targetIdentifyReqCB
 *
 * @brief   This callback is called to process an Identify Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetIdentifyReqCB( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  // The target should identify itself
  uint16 identifyTime;
  endPointDesc_t * bdb_CurrEpDescriptor;

  // Values of the Identify Duration field:
  // - Exit identify mode: 0x0000
  // - Length of time to remain in identify mode: 0x0001-0xfffe
  // - Remain in identify mode for a default time known by the receiver: 0xffff
  if ( pReq->IdDuration == 0xffff )
  {
    identifyTime = TOUCHLINK_DEFAULT_IDENTIFY_TIME;
  }
  else
  {
    identifyTime = pReq->IdDuration;
  }
  
  bdb_CurrEpDescriptor = bdb_setEpDescListToActiveEndpoint( );
  
  bdb_ZclIdentifyCmdInd( identifyTime, bdb_CurrEpDescriptor->endPoint );

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      targetResetToFNReqCB
 *
 * @brief   This callback is called to process a Reset to Factory New
 *          Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetResetToFNReqCB( afAddrType_t *srcAddr, bdbTLResetToFNReq_t *pReq )
{
  // If factory new, discard the request
  if ( ( touchLink_IsValidTransID( pReq->transID ) == FALSE ) )
  {
    return ( ZFailure );
  }

  osal_set_event( touchLinkTarget_TaskID, TOUCHLINK_RESET_TO_FN_EVT );

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      targetNwkStartReqCB
 *
 * @brief   This callback is called to process a Network Start Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetNwkStartReqCB( afAddrType_t *srcAddr, bdbTLNwkStartReq_t *pReq, uint8 seqNum )
{
#if ZSTACK_END_DEVICE_BUILD
  (void)rxSeqNum;
  return ( ZFailure );
#else
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }
  
  dstAddr = *srcAddr;
  dstAddr.panId = 0xFFFF;
  targetJoinedNwk = FALSE;

  if ( ( touchlinkAllowStealing == TRUE ) || ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) )
  {
    // Save the request for later
    rxReq.nwkStartReq = *pReq;
    rxSeqNum = seqNum;
        
    osal_set_event( touchLinkTarget_TaskID, TOUCHLINK_NWK_START_EVT );
  }
  else
  {
    targetSendNwkStartRsp( &dstAddr, pReq->transID, TOUCHLINK_NETWORK_START_RSP_STATUS_FAILURE,
                           NULL, 0, seqNum );
  }

  return ( ZSuccess );
#endif
}

/*********************************************************************
 * @fn      targetNwkJoinReqCB
 *
 * @brief   This callback is called to process a Network Join
 *          Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetNwkJoinReqCB( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8 seqNum )
{ 
  bdbTLNwkJoinRsp_t rsp;
  nwk_states_t nwkState;
  
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  dstAddr = *srcAddr;
  dstAddr.panId = 0xFFFF;
  targetJoinedNwk = TRUE;

  if ( ( touchlinkAllowStealing == TRUE ) || ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) )
  {
    // Save the request for later
    rxReq.nwkJoinReq = *pReq;
    
    // Wait at least BDBCTL_MIN_STARTUP_DELAY_TIME seconds to allow the
    // initiator to start the network correctly. Join the initiator afterwards.
    osal_start_timerEx( touchLinkTarget_TaskID, TOUCHLINK_NWK_JOIN_IND_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME );

    rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS;
    
#if ( ZG_DEVICE_RTRONLY_TYPE )
    nwkState = NWK_ROUTER;
#else
    nwkState = NWK_ENDDEVICE;
#endif
    
    bdb_setNodeIsOnANetwork(TRUE);
    
    // Apply the received network key
    touchLink_DecryptNwkKey( pReq->nwkParams.nwkKey, pReq->nwkParams.keyIndex, pReq->transID, touchLinkResponseID );

    // This is not a usual Trust Center protected network
    ZDSecMgrUpdateTCAddress( NULL );
    
    // Configure MAC with our network parameters
    NLME_InitNV();
    touchLink_SetNIB( nwkState, pReq->nwkParams.nwkAddr, pReq->nwkParams.extendedPANID,
                 _NIB.nwkLogicalChannel, pReq->nwkParams.panId, pReq->nwkParams.panId );
    
    touchLink_SetChannel( _NIB.nwkLogicalChannel );
    touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );
  }
  else
  {
    rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_FAILURE;
  }

  rsp.transID = pReq->transID;

  // Send a response back
#if ( ZSTACK_ROUTER_BUILD )
  bdbTL_Send_NwkJoinRtrRsp( TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, &rsp, seqNum );
#else
  bdbTL_Send_NwkJoinEDRsp( TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, &rsp, seqNum );
#endif  //ZSTACK_ROUTER_BUILD

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      targetNwkUpdateReqCB
 *
 * @brief   This callback is called to process a Network Update Request
 *          command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t targetNwkUpdateReqCB( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }
  // Discard the request if the Extended PAN ID and PAN ID are not
  // identical with our corresponding stored values
  if ( TOUCHLINK_SAME_NWK( pReq->PANID, pReq->extendedPANID ) )
  {
    uint8 newUpdateId = touchLink_NewNwkUpdateId( pReq->nwkUpdateId, _NIB.nwkUpdateId);
    if ( _NIB.nwkUpdateId != newUpdateId )
    {
      // Update the network update id and logical channel
      touchLink_ProcessNwkUpdate( newUpdateId, pReq->logicalChannel );
    }
  }

  return ( ZSuccess );
}

/******************************************************************************
 * @fn      targetZdoLeaveCnfCB
 *
 * @brief   This callback is called to process a Leave Confirmation message.
 *
 *          Note: this callback function returns a pointer if it has handled
 *                the confirmation message and no further action should be
 *                taken with it. It returns NULL if it has not handled the
 *                confirmation message and normal processing should take place.
 *
 * @param       pParam - received message
 *
 * @return      Pointer if message processed. NULL, otherwise.
 */
static void *targetZdoLeaveCnfCB( void *pParam )
{
  // Did we initiate the leave?
  if ( touchLinkLeaveInitiated == FALSE )
  {
    return ( NULL );
  }

  if ( touchLinkLeaveInitiated == TOUCHLINK_LEAVE_TO_START_NWK )
  {
    // Notify our task to start the network
    osal_set_event( touchLinkTarget_TaskID, TOUCHLINK_START_NWK_EVT );
  }
  else // TOUCHLINK_LEAVE_TO_JOIN_NWK
  {
    AssocReset();
    nwkNeighborInitTable();
    AddrMgrSetDefaultNV();
    // Immediately store empty tables in NV
    osal_set_event( ZDAppTaskID, ZDO_NWK_UPDATE_NV );
    // Notify our task to join the new network
    osal_start_timerEx( touchLinkTarget_TaskID, TOUCHLINK_JOIN_ATTEMPT_EVT, 100 );
  }

  return ( (void *)&touchLinkLeaveInitiated );
}

/*********************************************************************
 * @fn      targetProcessStateChange
 *
 * @brief   Process ZDO device state change
 *
 * @param   devState - The device's network state
 *
 * @return  none
 */
static void targetProcessStateChange( devStates_t devState )
{
  if ( ( ( devState == DEV_ROUTER ) || ( devState == DEV_END_DEVICE ) ) && ( touchlinkDistNwk == TRUE ) )
  {
    // Set touchlink flag to false after joining is complete
    touchlinkDistNwk = FALSE;
    
    if ( !_NIB.CapabilityFlags )
    {
      _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;
    }
    // Initialize the security for type of device
    ZDApp_SecInit( ZDO_INITDEV_RESTORED_NETWORK_STATE );
    
    ZDP_DeviceAnnce( NLME_GetShortAddr(), NLME_GetExtAddr(),
                     ZDO_Config_Node_Descriptor.CapabilityFlags, 0 );
    
    bdb_setNodeIsOnANetwork(TRUE);
    
    osal_start_timerEx( touchLinkTarget_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT, 500);
    
  }
}

/*********************************************************************
 * @fn      touchLink_TargetSendScanRsp
 *
 * @brief   Send out a Scan Response command.
 *
 * @param   srcEP - sender's endpoint
 * @param   dstAddr - pointer to destination address struct
 * @param   transID - received transaction id
 * @param   seqNum - received sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t touchLink_TargetSendScanRsp( uint8 srcEP, afAddrType_t *dstAddr, uint32 transID, uint8 seqNum )
{
  ZStatus_t status = ZSuccess;

  // Make sure we respond only once during a Device Discovery
  if ( touchLinkLastAcceptedTransID != transID )
  {
    bdbTLScanRsp_t *pRsp;    

    pRsp = (bdbTLScanRsp_t *)osal_mem_alloc( sizeof( bdbTLScanRsp_t ) );
    if ( pRsp )
    {
      osal_memset( pRsp, 0, sizeof( bdbTLScanRsp_t ) );

      // Save transaction id
      touchLinkLastAcceptedTransID = transID;
      osal_start_timerEx( touchLinkTarget_TaskID, TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT,
                          BDBCTL_INTER_PAN_TRANS_ID_LIFETIME );

      pRsp->transID = transID;
      pRsp->rssiCorrection = TOUCHLINK_RSSI_CORRECTION;
      pRsp->zLogicalType = zgDeviceLogicalType;
      pRsp->touchLinkAddressAssignment = touchLink_IsValidSplitFreeRanges( 0 );
      pRsp->touchLinkInitiator = FALSE;
      pRsp->touchLinkProfileInterop = TRUE;

      if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
      {
        pRsp->zRxOnWhenIdle = TRUE;
      }

      pRsp->touchLinklinkPriority = FALSE;
      pRsp->keyBitmask = touchLink_GetNwkKeyBitmask();

      // Generate a new Response ID
      touchLinkResponseID = ( ((uint32)osal_rand()) << 16 ) + osal_rand();
      pRsp->responseID = touchLinkResponseID;

      pRsp->touchLinkFactoryNew = !bdbAttributes.bdbNodeIsOnANetwork;
      if ( pRsp->touchLinkFactoryNew )
      {
        pRsp->nwkAddr = 0xFFFF;
        pRsp->nwkUpdateId = 0;
      }
      else
      {
        pRsp->nwkAddr = _NIB.nwkDevAddress;
        pRsp->nwkUpdateId = _NIB.nwkUpdateId;
      }
      pRsp->PANID = _NIB.nwkPanId;
      pRsp->logicalChannel = _NIB.nwkLogicalChannel;
      osal_cpyExtAddr( pRsp->extendedPANID, _NIB.extendedPANID );

      pRsp->numSubDevices = touchLink_GetNumSubDevices( 0 );
      if ( pRsp->numSubDevices == 1 )
      {
        touchLink_GetSubDeviceInfo( 0, &(pRsp->deviceInfo) );
      }

      pRsp->totalGrpIDs = touchLink_GetNumGrpIDs();

      // Send a response back
      status = bdbTL_Send_ScanRsp( srcEP, dstAddr, pRsp, seqNum );

      osal_mem_free( pRsp );
    }
    else
    {
      status = ZMemError;
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      targetSelectNwkParams
 *
 * @brief   Select a unique PAN ID and Extended PAN ID when compared to
 *          the PAN IDs and Extended PAN IDs of the networks detected
 *          on the TouchLink channels. The selected Extended PAN ID must be
 *          a random number (and not equal to our IEEE address).
 *
 * @param   void
 *
 * @return  void
 */
static void targetSelectNwkParams( void )
{
  uint8 status = ZFailure;

  while ( status == ZFailure )
  {
    // Select a random Extended PAN ID
    touchLink_GenerateRandNum( _NIB.extendedPANID, Z_EXTADDR_LEN );

    // Select a random PAN ID
    _NIB.nwkPanId = osal_rand( );

    // Make sure they're unique
    status = targetVerifyNwkParams( _NIB.nwkPanId, _NIB.extendedPANID );
  }

  if ( _NIB.nwkLogicalChannel == 0 )
  {
    if (TOUCHLINK_FIXED_CHANNEL_ENABLE == TRUE )
    {
      // Use the fixed channel defined in bdb_interface
      _NIB.nwkLogicalChannel = TOUCHLINK_FIXED_CHANNEL;
    }
    else
    {
      // Select randomly one of the TouchLink channels as our logical channel
      _NIB.nwkLogicalChannel = touchLink_GetRandPrimaryChannel( );
    }
  }

  _NIB.nwkDevAddress = osal_rand( );

  // Configure MAC with our network parameters
  touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );
}

/*********************************************************************
 * @fn      targetVerifyNwkParams
 *
 * @brief   Verify that the PAN ID and Extended PAN ID are unique.
 *
 * @param   PANID - PAN Identifier
 * @param   pExtendedPANID - extended PAN Identifier
 *
 * @return  status
 */
static ZStatus_t targetVerifyNwkParams( uint16 PANID, uint8 *pExtendedPANID )
{
  touchLinkDiscoveredNwkParam_t *pParam = pDiscoveredNwkParamList;

  // Add for our network parameters in the Network Parameter List
  while ( pParam != NULL )
  {
    if ( ( pParam->PANID == PANID ) &&
         ( osal_ExtAddrEqual( pParam->extendedPANID, pExtendedPANID ) ) )
    {
      return ( ZFailure );
    }

    pParam = pParam->nextParam;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      targetSendNwkStartRsp
 *
 * @brief   Send out a Network Start Response command.
 *
 * @param   dstAddr - destination's address
 * @param   transID - touch link transaction identifier
 * @param   status - Network Start Response command status field
 * @param   pNwkParams - network parameters
 * @param   nwkUpdateId - network update identifier
 * @param   seqNum
 *
 * @return  none
 */
#if (ZSTACK_ROUTER_BUILD)
static void targetSendNwkStartRsp( afAddrType_t *dstAddr, uint32 transID, uint8 status,
                                   bdbTLNwkParams_t *pNwkParams, uint8 nwkUpdateId, uint8 seqNum )
{
  bdbTLNwkStartRsp_t *pRsp;

  // Send out a response
  pRsp = (bdbTLNwkStartRsp_t *)osal_mem_alloc( sizeof( bdbTLNwkStartRsp_t ) );
  if ( pRsp )
  {
    pRsp->transID = transID;
    pRsp->status = status;

    if ( pNwkParams != NULL )
    {
      osal_cpyExtAddr( pRsp->extendedPANID, pNwkParams->extendedPANID );
      pRsp->logicalChannel = pNwkParams->logicalChannel;
      pRsp->panId = pNwkParams->panId;
    }
    else
    {
      osal_memset( pRsp->extendedPANID, 0, Z_EXTADDR_LEN );
      pRsp->logicalChannel = 0;
      pRsp->panId = 0;
    }

    pRsp->nwkUpdateId = nwkUpdateId;

    bdbTL_Send_NwkStartRsp( TOUCHLINK_INTERNAL_ENDPOINT, dstAddr, pRsp, seqNum );

    osal_mem_free( pRsp );
  }
}
#endif  // ZSTACK_ROUTER_BUILD


#endif // BDB_TL_TARGET

/*********************************************************************
*********************************************************************/
