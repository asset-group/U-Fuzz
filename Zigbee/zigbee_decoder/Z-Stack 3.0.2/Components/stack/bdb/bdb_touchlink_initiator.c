/**************************************************************************************************
  Filename:       bdb_touchlink_initiator.c
  Revised:        $Date: 2013-11-22 16:17:23 -0800 (Fri, 22 Nov 2013) $
  Revision:       $Revision: 36220 $

  Description:    Zigbee Cluster Library - Light Link Initiator.


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
#include "OSAL_Tasks.h"
#include "OSAL_Nv.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDSecMgr.h"
#include "ZDObject.h"
#include "nwk_util.h"
#include "ZGlobals.h"
#include "AddrMgr.h"

#if defined ( POWER_SAVING )
#include "OSAL_PwrMgr.h"
#endif

#include "stub_aps.h"

#include "zcl.h"
#include "zcl_general.h"
#include "bdb.h"
#include "bdb_interface.h"
#include "bdb_tlCommissioning.h"
#include "bdb_touchlink.h"

#include "bdb_touchlink_initiator.h"
#include "hal_lcd.h"
#include "hal_led.h"

#if defined ( BDB_TL_INITIATOR )
   
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define TOUCHLINK_INITIATOR_REJOIN_TIMEOUT             2500 // 2.5 sec

// for non-polling end-devices only
#define TOUCHLINK_INITIATOR_TEMP_POST_TL_POLL_RATE     1000

#define DEV_INFO_INVALID_EP                            0xFE

#define TOUCHLINK_INITIATOR_NUM_SCAN_REQ_PRIMARY       8  // 5 times on 1st channel, plus once for each remianing primary channel
#define TOUCHLINK_INITIATOR_NUM_SCAN_REQ_EXTENDED      20 // (TOUCHLINK_NUM_SCAN_REQ_PRIMARY + sizeof(TOUCHLINK_SECONDARY_CHANNELS_SET))

/*********************************************************************
 * TYPEDEFS
 */
typedef union
{
  bdbTLNwkStartRsp_t nwkStartRsp;
  bdbTLNwkJoinRsp_t nwkJoinRsp;
} bdbTLRsp_t;

typedef struct
{
  bdbTLScanRsp_t scanRsp;
  afAddrType_t srcAddr;
  uint16 newNwkAddr;
  uint8 rxChannel;  // channel scan response was heard on
  int8 lastRssi;    // receieved RSSI
} targetCandidate_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 touchLinkInitiator_TaskID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern devStartModes_t devStartMode;
extern uint8 _tmpRejoinState;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void bdb_setNodeIsOnANetwork(bool isOnANetwork);

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 initiatorSeqNum;

// Touch Link channel tracking
static uint8 numScanReqSent;
static uint8 scanReqChannels;

// Network key sent to the target to start the network with
static uint8 keyIndexSent;
static uint8 encKeySent[SEC_KEY_LEN];
static uint32 responseIDSent;

// Info related to the received request
#if ZSTACK_END_DEVICE_BUILD
static bdbTLNwkJoinReq_t joinReq;
#endif

// Info related to the received response
static targetCandidate_t selectedTarget;
static uint16 selectedTargetNwkAddr;
static ZLongAddr_t selectedTargetIEEEAddr;
static bdbTLRsp_t rxRsp; // network start or join response

static bdbTLNwkParams_t initiatorNwkParams = {0};

// Addresses used for sending/receiving messages
static afAddrType_t bcastAddr;

static uint16 savedPollRate;
static uint16 savedQueuedPollRate;
static uint16 savedResponsePollRate;
static uint8 savedRxOnIdle;

// Application callback
static touchLink_NotifyAppTLCB_t pfnNotifyAppCB = NULL;
static touchLink_SelectDiscDevCB_t pfnSelectDiscDevCB = NULL;

static uint8 initiatorRegisteredMsgAppTaskID = TASK_NO_TASK;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
extern void touchLink_ItemInit( uint16 id, uint16 len, void *pBuf );

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static ZStatus_t initiatorScanReqCB( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8 seqNum );
static ZStatus_t initiatorDeviceInfoReqCB( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8 seqNum );
static ZStatus_t initiatorIdentifyReqCB( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq );
static ZStatus_t initiatorNwkJoinReqCB( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8 seqNum );
static ZStatus_t initiatorNwkUpdateReqCB( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq );
static ZStatus_t initiatorScanRspCB( afAddrType_t *srcAddr, bdbTLScanRsp_t *pRsp );
static ZStatus_t initiatorDeviceInfoRspCB( afAddrType_t *srcAddr, bdbTLDeviceInfoRsp_t *pRsp );
static ZStatus_t initiatorNwkStartRspCB( afAddrType_t *srcAddr, bdbTLNwkStartRsp_t *pRsp );
static ZStatus_t initiatorNwkJoinRtrRspCB( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp );
static ZStatus_t initiatorNwkJoinEDRspCB( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp );
static void *initiatorZdoLeaveCnfCB( void *pParam );
static ZStatus_t touchLink_InitiatorSendScanRsp( uint8 srcEP, afAddrType_t *dstAddr, uint32 transID, uint8 seqNum );
static void initiatorProcessStateChange( devStates_t state );
static void initiatorSetNwkToInitState( void );
#if ( ZSTACK_ROUTER_BUILD )
static void initiatorJoinNwk( void );
#endif
static void initiatorReJoinNwk( devStartModes_t startMode );
static void initiatorSendScanReq( bool freshScan );
static ZStatus_t initiatorSendNwkStartReq( bdbTLScanRsp_t *pRsp );
static ZStatus_t initiatorSendNwkJoinReq( bdbTLScanRsp_t *pRsp );
static ZStatus_t initiatorSendNwkUpdateReq( bdbTLScanRsp_t *pRsp );
static void initiatorClearSelectedTarget( void );

/*********************************************************************
 * TOUCHLINK Initiator Callback Table
 */
// Initiator Command Callbacks table
static bdbTL_InterPANCallbacks_t touchLinkInitiator_CmdCBs =
{
  // Received Server Commands
  initiatorScanReqCB,       // Scan Request command
  initiatorDeviceInfoReqCB, // Device Information Request command
  initiatorIdentifyReqCB,   // Identify Request command
  NULL,                     // Reset to Factory New Request command
  NULL,                     // Network Start Request command
#if ( ZSTACK_ROUTER_BUILD )
  initiatorNwkJoinReqCB,    // Network Join Router Request command
  NULL,                     // Network Join End Device Request command
#else
  NULL,                     // Network Join Router Request command
  initiatorNwkJoinReqCB,    // Network Join End Device Request command
#endif
  initiatorNwkUpdateReqCB,  // Network Update Request command

  // Received Client Commands
  initiatorScanRspCB,       // Scan Response command
  initiatorDeviceInfoRspCB, // Device Information Response command
  initiatorNwkStartRspCB,   // Network Start Response command
  initiatorNwkJoinRtrRspCB, // Network Join Router Response command
  initiatorNwkJoinEDRspCB   // Network Join End Device Response command
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLinkInitiator_InitDevice
 *
 * @brief   Start the TOUCHLINK Initiator device in the network if it's not
 *          factory new. Otherwise, determine the network parameters
 *          and wait for a touchlink command.
 *
 * @param   none
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_InitDevice( void )
{
  ZDO_Config_Node_Descriptor.LogicalType = zgDeviceLogicalType;
  
  // Is not factory new?
  if ( bdbAttributes.bdbNodeIsOnANetwork == TRUE )
  {
    // Resume ZigBee functionality based on the info stored in NV
    initiatorReJoinNwk( MODE_RESUME );
  }
  else
  {
    initiatorSelectNwkParams();
  }

#if defined ( POWER_SAVING )
  osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

#if ( ZSTACK_ROUTER_BUILD )
  // Enable our receiver
  savedRxOnIdle = TRUE;
  ZMacSetReq( ZMacRxOnIdle, &savedRxOnIdle );
  touchLink_PermitJoin(0);
#endif

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      touchLinkInitiator_RegisterNotifyTLCB
 *
 * @brief   Register an Application's Touch-Link Notify callback function.
 *
 * @param   pfnNotifyApp - application callback
 *
 * @return  none
 */
void touchLinkInitiator_RegisterNotifyTLCB( touchLink_NotifyAppTLCB_t pfnNotifyApp )
{
  pfnNotifyAppCB = pfnNotifyApp;
}

/*********************************************************************
 * @fn      touchLinkInitiator_RegisterSelectDiscDevCB
 *
 * @brief   Register an Application's Selection callback function, to select
 *          a target from the discovered devices during a Touch-link scan.
 *
 * @param   pfnSelectDiscDev - application callback
 *
 * @return  none
 */
void touchLinkInitiator_RegisterSelectDiscDevCB( touchLink_SelectDiscDevCB_t pfnSelectDiscDev )
{
  pfnSelectDiscDevCB = pfnSelectDiscDev;
}

/*********************************************************************
 * @fn      touchLinkInitiator_StartDevDisc
 *
 * @brief   Start device discovery, scanning for other devices in the vicinity
 *          of the originator (initiating first part of the Touch-Link process).
 *          Device discovery shall only be initiated by address assignment capable
 *          devices. To perform device discovery, the initiator shall broadcast
 *          inter-PAN Scan Requests, spaced at an interval of
 *          BDBCTL_SCAN_TIME_BASE_DURATION seconds.
 *
 * @param   none
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_StartDevDisc( void )
{
  osal_clear_event( ZDAppTaskID, ZDO_NETWORK_INIT ); // in case orphaned rejoin was called
  ZDApp_StopJoiningCycle();

  //abort any touchlink in progress and start the new dev discovery.
  touchLinkInitiator_AbortTL();

  // To perform device discovery, switch to channel 11 and broadcast five
  // consecutive inter-PAN Scan Requests. Then switch to each remaining
  // TOUCHLINK channels in turn (i.e., 15, 20, and 25) and broadcast a single
  // inter-PAN Scan Request on each channel.
  if ( !osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT ) )
  {
    uint8 x = TRUE;

    // Generate a new Transaction Id
    touchLinkTransID = ( ( (uint32)osal_rand() ) << 16 ) + osal_rand();
    osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT,
                        BDBCTL_INTER_PAN_TRANS_ID_LIFETIME );

    if ( bdbAttributes.bdbNodeIsOnANetwork == TRUE )
    {
      // Turn off polling during touch-link procedure
      savedPollRate = zgPollRate;
      savedQueuedPollRate = zgQueuedPollRate;
      savedResponsePollRate = zgResponsePollRate;

      NLME_SetPollRate( 0 );
      NLME_SetQueuedPollRate( 0 );
      NLME_SetResponseRate( 0 );
    }

    // Remember current rx state
    ZMacGetReq( ZMacRxOnIdle, &savedRxOnIdle );

    // MAC receiver should be on during touch-link procedure
    ZMacSetReq( ZMacRxOnIdle, &x );

    scanReqChannels = TOUCHLINK_SCAN_PRIMARY_CHANNELS;
    numScanReqSent = 0;

    // Send out the first Scan Request
    initiatorSendScanReq( TRUE );

    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      bdbTL_Send_IndentifyReq
 *
 * @brief   Call to send out a scan request for factory new procedure
 *
 * @param   
 *
 * @return 
 */
void touchLinkInitiator_ResetToFNProcedure( void )
{
  if ( ( bdbCommissioningProcedureState.bdbCommissioningState == 0 ) || ( osal_get_timeoutEx( bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE ) == 0 ) )
  {
    touchlinkFNReset = TRUE;
    touchLinkInitiator_StartDevDisc( );
  }
}

/*********************************************************************
 * @fn      touchLinkInitiator_AbortTL
 *
 * @brief   Abort Touch-link device discovery.
 *          Successful execution could be done before Network Start/Join
 *          commands are sent. Until then, since no device parameters
 *          such as network settings are altered, the Touch-Link is
 *          still reversible.
 *
 * @param   none
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_AbortTL( void )
{
  if ( ( osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT ) == SUCCESS )
       || ( osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT ) == SUCCESS )
       || ( osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT ) == SUCCESS )
       || ( osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT ) == SUCCESS ) )
  {
    initiatorSetNwkToInitState();
    touchLinkTransID = 0;
    numScanReqSent = 0;
    initiatorClearSelectedTarget();
    selectedTargetNwkAddr = 0;

    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn          touchLinkInitiator_Init
 *
 * @brief       Initialization function for the TOUCHLINK Initiator task.
 *
 * @param       task_id - TOUCHLINK Initiator task id
 *
 * @return      none
 */
void touchLinkInitiator_Init( uint8 task_id )
{
  // Save our own Task ID
  touchLinkInitiator_TaskID = task_id;

  touchLink_SetTouchLinkTaskId( touchLinkInitiator_TaskID );


  // Build a broadcast address for the Scan Request
  bcastAddr.addrMode = afAddrBroadcast;
  bcastAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVALL;
  bcastAddr.panId = 0xFFFF;
  bcastAddr.endPoint = STUBAPS_INTER_PAN_EP;

  // Initialize TOUCHLINK common variables
  touchLink_InitVariables( TRUE );

  savedPollRate = POLL_RATE;
  savedQueuedPollRate = QUEUED_POLL_RATE;
  savedResponsePollRate = RESPONSE_POLL_RATE;

  numScanReqSent = 0;
  initiatorClearSelectedTarget();
  scanReqChannels = TOUCHLINK_SCAN_PRIMARY_CHANNELS;

  initiatorSeqNum = 0;

  // Register to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( touchLinkInitiator_TaskID );

  // Register for TOUCHLINK Initiator callbacks (for Inter-PAN commands)
  bdbTL_RegisterInterPANCmdCallbacks( &touchLinkInitiator_CmdCBs );

  // Register for Initiator to receive Leave Confirm
  ZDO_RegisterForZdoCB( ZDO_LEAVE_CNF_CBID, initiatorZdoLeaveCnfCB );

#if (ZSTACK_ROUTER_BUILD)
  // Register to process ZDO messages
  ZDO_RegisterForZDOMsg( touchLinkInitiator_TaskID, Mgmt_Permit_Join_req );
  ZDO_RegisterForZDOMsg( touchLinkInitiator_TaskID, Device_annce );
#endif

}

/*********************************************************************
 * @fn      initiatorSelectNwkParams
 *
 * @brief   Select a unique PAN ID and Extended PAN ID when compared to
 *          the PAN IDs and Extended PAN IDs of the networks detected
 *          on the TOUCHLINK channels. The selected Extended PAN ID must be
 *          a random number (and not equal to our IEEE address).
 *
 * @param   void
 *
 * @return  void
 */
void initiatorSelectNwkParams( void )
{
  // Set our group ID range
  touchLink_PopGrpIDRange( touchLink_GetNumGrpIDs(), &touchLinkGrpIDsBegin, &touchLinkGrpIDsEnd );

  // Select a random Extended PAN ID
  touchLink_GenerateRandNum( _NIB.extendedPANID, Z_EXTADDR_LEN );

  // Select a random PAN ID
  _NIB.nwkPanId = osal_rand();

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
  
  selectedTargetNwkAddr = 0;

  if ( devState != DEV_INIT )
  {
    // Let's assume we're the first initiator
    _NIB.nwkDevAddress = touchLink_PopNwkAddress();
  }

  // Configure MAC with our network parameters
  touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );
}

/*********************************************************************
 * @fn      touchLinkInitiator_RegisterForMsg
 *
 * @brief   Register application task to receive unprocessed messages
 *          received by the initiator endpoint.
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 *
 * @return  ZSuccess if task registration successful
 *********************************************************************/
ZStatus_t touchLinkInitiator_RegisterForMsg( uint8 taskId )
{
  if ( initiatorRegisteredMsgAppTaskID == TASK_NO_TASK )
  {
    initiatorRegisteredMsgAppTaskID = taskId;
    return ( ZSuccess );
  }
  return ( ZFailure );
}

/*********************************************************************
 * @fn          touchLinkInitiator_event_loop
 *
 * @brief       Event Loop Processor for TOUCHLINK Initiator.
 *
 * @param       task_id - task id
 * @param       events - event bitmap
 *
 * @return      unprocessed events
 */
uint16 touchLinkInitiator_event_loop( uint8 task_id, uint16 events )
{
  if ( events & SYS_EVENT_MSG )
  {
    osal_event_hdr_t *pMsg;
    ZStatus_t stat = ZFailure;

    if ( (pMsg = (osal_event_hdr_t *)osal_msg_receive( task_id )) != NULL )
    {
      switch ( pMsg->event )
      {

#if (ZSTACK_ROUTER_BUILD)
        case ZDO_CB_MSG:
          // ZDO sends the message that we registered for
          touchLink_RouterProcessZDOMsg( (zdoIncomingMsg_t *)pMsg );
          stat = ZSuccess;
          break;
#endif
        case ZDO_STATE_CHANGE:
          initiatorProcessStateChange( (devStates_t)pMsg->status );
          stat = ZSuccess;
          break;

        default:
          break;
      }

      if ( stat == ZSuccess )
      {
        // Release the OSAL message
        VOID osal_msg_deallocate( (uint8 *)pMsg );
      }
      else
      {
        // forward to the application
        osal_msg_send( initiatorRegisteredMsgAppTaskID, (uint8 *)pMsg );
      }
    }

    // return unprocessed events
    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & TOUCHLINK_TL_SCAN_BASE_EVT )
  {
    if ( ( ( scanReqChannels == TOUCHLINK_SCAN_PRIMARY_CHANNELS ) && ( numScanReqSent < TOUCHLINK_INITIATOR_NUM_SCAN_REQ_PRIMARY  ) ) ||
         ( ( scanReqChannels == TOUCHLINK_SCAN_SECONDARY_CHANNELS ) && ( numScanReqSent < TOUCHLINK_INITIATOR_NUM_SCAN_REQ_EXTENDED ) ) )
    {
      // Send another Scan Request on the next channel
      initiatorSendScanReq( FALSE );
    }
    else // Channels scan is complete
    {
      if ( ( scanReqChannels == TOUCHLINK_SCAN_PRIMARY_CHANNELS ) && ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) )
      {
        // Extended scan is required, lets scan secondary channels
        scanReqChannels = TOUCHLINK_SCAN_SECONDARY_CHANNELS;

        // Send another Scan Request on the next channel
        initiatorSendScanReq( FALSE );
      }
      // See if we've received any Scan Responses back
      else if ( ( selectedTarget.lastRssi != TOUCHLINK_WORST_RSSI )
               && ( selectedTarget.scanRsp.deviceInfo.endpoint != DEV_INFO_INVALID_EP ) )
      {
        // Make sure the responder is not a factory new initiator if this device is also 
        // factory new
        if ( ( selectedTarget.scanRsp.touchLinkInitiator == FALSE ) ||
             ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) )
        {
          bdbTLIdentifyReq_t req;

          // Tune to the channel that the Scan Response was heard on
          touchLink_SetChannel( selectedTarget.rxChannel );

          req.transID = selectedTarget.scanRsp.transID;
          req.IdDuration = BDB_TL_IDENTIFY_TIME;
          
          if ( touchlinkFNReset == TRUE )
          {
            osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT );
            return ( events ^ TOUCHLINK_TL_SCAN_BASE_EVT );
          }
          
          bdbTL_Send_IndentifyReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), &req, initiatorSeqNum++ );

#if ZSTACK_ROUTER_BUILD
          uint8 i = 0;
            
          while ( !CHECK_BIT ( selectedTarget.scanRsp.keyBitmask , i ) ) 
          {
            i++;
          }
    
          initiatorNwkParams.keyIndex = i;
          zTouchLinkNwkStartRtr = TRUE;
          // Disabe other TouchLink events
          osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT );
#endif
          osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT, TOUCHLINK_INITIATOR_IDENTIFY_INTERVAL );
        }
        // else wait for touch-link commands from the other initiator
      }
      else
      {
        // We did not manage to select any target
        // Let's just go back to our initial configuration
        osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT );
        bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_NO_SCAN_RESPONSE;
        bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
      }
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_TL_SCAN_BASE_EVT );
  }

  if ( events & TOUCHLINK_CFG_TARGET_EVT )
  {
    ZStatus_t status = ZFailure;

    bdbTLIdentifyReq_t req;

    req.transID = selectedTarget.scanRsp.transID;
    req.IdDuration = 0x00;
    uint8 tcExtAddr[Z_EXTADDR_LEN];
    uint8 DistributedSecurityNwkAddress[Z_EXTADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        
    if ( zTouchLinkNwkStartRtr == FALSE )
    {
      // Send an Identify stop Request
      bdbTL_Send_IndentifyReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), &req, initiatorSeqNum++ );
    }
    
   // See if the target is part of our network
    if ( !TOUCHLINK_SAME_NWK( selectedTarget.scanRsp.PANID, selectedTarget.scanRsp.extendedPANID ) )
    {
      // If the local device is not the trust center, always return TRUE
      status = APSME_GetRequest( apsTrustCenterAddress, 0, tcExtAddr );
        
      // Notify BDB state machine
      if ( bdbAttributes.bdbNodeIsOnANetwork == TRUE )
      {
        if ( ( status == ZSuccess ) && ( !osal_ExtAddrEqual( DistributedSecurityNwkAddress, tcExtAddr ) ) )
        {
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_NOT_PERMITTED;
          bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
          return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
        }
      }
            
      // verify address ranges split possible if required
      if ( !touchLink_IsValidSplitFreeRanges( selectedTarget.scanRsp.totalGrpIDs ) )
      {
        bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_NOT_AA_CAPABLE;
        bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
        return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
      }
      
      if ( touchlinkFNReset == TRUE )
      {
        touchlinkFNReset = FALSE;
        touchLinkInitiator_ResetToFNSelectedTarget( );
        osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT );
        
        return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
      }
        
      if ( bdbAttributes.bdbNodeIsOnANetwork == TRUE )
      {
        // Ask the target to join our network
        osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT, 50);
        zTouchLinkNwkStartRtr = FALSE;
        return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
      }
      else if ( ZSTACK_ROUTER_BUILD == TRUE )
      {
        // Tune to the channel that the Scan Response was heard on
        touchLink_SetChannel( selectedTarget.rxChannel );
        
        // Try to form a new network
        osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_NWK_RTR_START_EVT);
        return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
      }
      else if ( ZSTACK_ROUTER_BUILD == FALSE )
      {
        if ( selectedTarget.scanRsp.zLogicalType == ZG_DEVICETYPE_ROUTER )
        {
          if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
          {
            _NIB.nwkDevAddress = APL_FREE_NWK_ADDR_RANGE_BEGIN;
          }
          
          // Must be the first light; ask the light to start the network
          status = initiatorSendNwkStartReq( &(selectedTarget.scanRsp) );
        }
        else
        {
          // Notify the BDB state machine 
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
          bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
          return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
        }
      }
    }
    
    else if ( _NIB.nwkUpdateId != selectedTarget.scanRsp.nwkUpdateId )
    {
      // Set NWK task to run
      nwk_setStateIdle( TRUE );
      
      // Target is already part of our network
      if ( selectedTarget.scanRsp.nwkUpdateId > _NIB.nwkUpdateId )
      {
        // Update our network update id and logical channel
        touchLink_ProcessNwkUpdate( selectedTarget.scanRsp.nwkUpdateId, selectedTarget.scanRsp.logicalChannel );

        // We're done here
        status = ZSuccess;
      }
      else if ( selectedTarget.scanRsp.nwkUpdateId < _NIB.nwkUpdateId )
      {
        // Inform the target to update its network update id and logical channel
        initiatorSendNwkUpdateReq( &(selectedTarget.scanRsp) ); // there's no corresponding response!

        // Notify the application about this device
        osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_NOTIFY_APP_EVT );
      }
      
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, TRUE );
      return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
    }
    
    //we are touchlinking to a light in our network, just send application the device info
    else if ( selectedTarget.scanRsp.touchLinkInitiator == FALSE )
    {
      epInfoRec_t rec;
      rec.nwkAddr = selectedTarget.scanRsp.nwkAddr;
      rec.endpoint = selectedTarget.scanRsp.deviceInfo.endpoint;
      rec.profileID = selectedTarget.scanRsp.deviceInfo.profileID;
      rec.deviceID = selectedTarget.scanRsp.deviceInfo.deviceID;
      rec.version = selectedTarget.scanRsp.deviceInfo.version;
      // Notify the application
      if ( pfnNotifyAppCB )
      {
        (*pfnNotifyAppCB)( &rec );
      }
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, TRUE );
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_CFG_TARGET_EVT );
  }

  if ( events & TOUCHLINK_W4_NWK_START_RSP_EVT )
  {
    bdbTLNwkStartRsp_t *pRsp = &(rxRsp.nwkStartRsp);

    // Look if we have a valid response
    if ( ( pRsp->status == TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS )
       && ( ( nwk_ExtPANIDValid( pRsp->extendedPANID ) ) && ( touchLink_IsValidTransID( pRsp->transID ) ) ) )
    {
      // Copy the new network parameters to NIB
      touchLink_SetNIB( ( ZSTACK_ROUTER_BUILD ? NWK_ROUTER : NWK_REJOINING ),
                  _NIB.nwkDevAddress, pRsp->extendedPANID,
                  pRsp->logicalChannel, pRsp->panId, pRsp->nwkUpdateId );

      // Apply the received network key
      touchLink_DecryptNwkKey( encKeySent, keyIndexSent, pRsp->transID, responseIDSent );

      // This is not a usual Trust Center protected network
      ZDSecMgrUpdateTCAddress( NULL );

      // Notify the application about this device
      osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_NOTIFY_APP_EVT );

      // Wait at least BDBCTL_MIN_STARTUP_DELAY_TIME seconds to allow the
      // target to start the network correctly. Join the target afterwards.
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_START_NWK_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME );
    }
    else
    {
      // Notify the BDB state machine 
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );        
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_W4_NWK_START_RSP_EVT );
  }

  if ( events & TOUCHLINK_START_NWK_EVT )
  {
    // Rejoins without NWK scan

    bdbTLNwkStartRsp_t *pRsp = &(rxRsp.nwkStartRsp);
    bdbTLNwkRejoin_t rejoinInf;
    
    rejoinInf.panId = pRsp->panId;
    rejoinInf.logicalChannel = pRsp->logicalChannel;
    osal_memcpy( rejoinInf.extendedPANID, pRsp->extendedPANID, Z_EXTADDR_LEN);
    rejoinInf.nwkAddr = selectedTargetNwkAddr;
    rejoinInf.nwkUpdateId = pRsp->nwkUpdateId;
      
    touchLink_DevRejoin( &rejoinInf );

    // return unprocessed events
    return ( events ^ TOUCHLINK_START_NWK_EVT );
  }

  if ( events & TOUCHLINK_W4_NWK_JOIN_RSP_EVT )
  {
    bdbTLNwkJoinRsp_t *pRsp = &(rxRsp.nwkJoinRsp);

    if ( pRsp->status == TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS )
    {
      // Wait at least BDBCTL_MIN_STARTUP_DELAY_TIME seconds to allow the
      // target to start operating on the network correctly. Notify the
      // application afterwards.
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_NOTIFY_APP_EVT,
                          BDBCTL_MIN_STARTUP_DELAY_TIME );
      
      // Establish bind links
      if ( pRespondentHead != NULL )
      {
        AddrMgrEntry_t entry;

        // add the device's address information
        entry.user    = ADDRMGR_USER_BINDING;
        entry.nwkAddr = selectedTargetNwkAddr;
        osal_cpyExtAddr( entry.extAddr, selectedTargetIEEEAddr );
        AddrMgrEntryUpdate( &entry );
        osal_start_timerEx( bdb_TaskID, BDB_RESPONDENT_PROCESS_TIMEOUT, SIMPLEDESC_RESPONSE_TIMEOUT );
      }
      
      // We're done with touch-link procedure here
      initiatorSetNwkToInitState();
      //bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
      //bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, TRUE );

      touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

      if ( ( POLL_RATE == 0 ) && ( selectedTarget.scanRsp.zLogicalType == ZG_DEVICETYPE_ENDDEVICE ) )
      {
        //allow to respond to TOUCHLINK commission utility commands after TL
        NLME_SetPollRate( TOUCHLINK_INITIATOR_TEMP_POST_TL_POLL_RATE );
        //polling should reset when TL life time expires
      }
    }
    else 
    {
      touchLink_SendLeaveReq( );
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_TARGET_FAILURE;
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_W4_NWK_JOIN_RSP_EVT );
  }

  if ( events & TOUCHLINK_NWK_JOIN_IND_EVT )
  {
    // If not factory new, perform a Leave on our old network
    if ( ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) && ( touchLink_SendLeaveReq( ) == ZSuccess ) )
    {
      // Wait for Leave confirmation before joining the new network
      touchLinkLeaveInitiated = TOUCHLINK_LEAVE_TO_JOIN_NWK;
    }
    else
    {
#if ( ZSTACK_ROUTER_BUILD )
      // Notify our task to join this network
      osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_JOIN_NWK_ATTEMPT_EVT );
#else
      bdbTLNwkParams_t *pParams = &(joinReq.nwkParams);
      
      // Notify our task to join this network
      // Perform Network Discovery to verify our new network parameters uniqeness
      touchLink_PerformNetworkDisc( (uint32)1 << pParams->logicalChannel );
#endif
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_JOIN_IND_EVT );
  }

  if ( events & TOUCHLINK_JOIN_NWK_ATTEMPT_EVT )
  {
    // Join the network
#if ( ZSTACK_ROUTER_BUILD )
    initiatorJoinNwk();
#else
    bdbTLNwkParams_t *pParams = &(joinReq.nwkParams);
    
    bdbTLNwkRejoin_t rejoinInf;
         
    rejoinInf.panId = pParams->panId;
    rejoinInf.logicalChannel = pParams->logicalChannel;
    osal_memcpy( rejoinInf.extendedPANID, pParams->extendedPANID, Z_EXTADDR_LEN);
    rejoinInf.nwkAddr = pDiscoveredNwkParamList->chosenRouter;
    rejoinInf.nwkUpdateId = joinReq.nwkUpdateId;
      
    touchLink_DevRejoin( &rejoinInf );
#endif
    touchLink_FreeNwkParamList();

    // return unprocessed events
    return ( events ^ TOUCHLINK_JOIN_NWK_ATTEMPT_EVT );
  }

  if ( events & TOUCHLINK_DISABLE_RX_EVT )
  {
    // We're not asked to join a network
    initiatorSetNwkToInitState();

    scanReqChannels = TOUCHLINK_SCAN_PRIMARY_CHANNELS;
    numScanReqSent = 0;
    // Reset selected target
    if ( zTouchLinkNwkStartRtr == FALSE )
    {
      initiatorClearSelectedTarget();
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_DISABLE_RX_EVT );
  }

  if ( events & TOUCHLINK_W4_REJOIN_EVT )
  {
    // Stop joining cycle
    ZDApp_StopJoiningCycle();

    // return unprocessed events
    return ( events ^ TOUCHLINK_W4_REJOIN_EVT );
  }

  if ( events & TOUCHLINK_NOTIFY_APP_EVT )
  {
    ZDP_DeviceAnnce( NLME_GetShortAddr(), NLME_GetExtAddr(),
                     ZDO_Config_Node_Descriptor.CapabilityFlags, 0 );
    
    if ( selectedTarget.lastRssi > TOUCHLINK_WORST_RSSI )
    {
      epInfoRec_t rec;
      rec.nwkAddr = selectedTarget.newNwkAddr; // newly assigned network address
      rec.endpoint = selectedTarget.scanRsp.deviceInfo.endpoint;
      rec.profileID = selectedTarget.scanRsp.deviceInfo.profileID;
      rec.deviceID = selectedTarget.scanRsp.deviceInfo.deviceID;
      rec.version = selectedTarget.scanRsp.deviceInfo.version;
      // Notify the application
      if ( pfnNotifyAppCB )
      {
        (*pfnNotifyAppCB)( &rec );
      }
    }
    // return unprocessed events
    return ( events ^ TOUCHLINK_NOTIFY_APP_EVT );
  }
  
  if ( events & TOUCHLINK_NWK_RTR_START_EVT )
  {
    uint16 nwkAddr = INVALID_NODE_ADDR;
    
    osal_nv_write( ZCD_NV_NIB, osal_offsetof( nwkIB_t, nwkDevAddress ),
                   sizeof( uint16), &nwkAddr );
    
    // If the PAN Id, Extended PAN Id or Logical Channel are zero then
    // determine each of these parameters
    if ( !nwk_ExtPANIDValid( initiatorNwkParams.extendedPANID ) )
    {
      touchLink_GenerateRandNum( initiatorNwkParams.extendedPANID, Z_EXTADDR_LEN );
    }

    if ( initiatorNwkParams.panId == 0 )
    {
      initiatorNwkParams.panId = osal_rand();
    }

    if ( initiatorNwkParams.logicalChannel == 0 )
    {
      initiatorNwkParams.logicalChannel = touchLink_GetRandPrimaryChannel();
    }

    if ( selectedTarget.scanRsp.touchLinkAddressAssignment)
    {
      touchLink_GerFreeRanges( &initiatorNwkParams );
    }

    // Perform Network Discovery to verify our new network parameters uniqeness
    touchLink_PerformNetworkDisc( (uint32)1 << initiatorNwkParams.logicalChannel );

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_RTR_START_EVT );
  }
  
  if ( events & TOUCHLINK_NWK_FORMATION_SUCCESS_EVT )
  {
    if( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
    {
      bdbTLNwkJoinRsp_t *pRsp = &(rxRsp.nwkJoinRsp);
      pRsp->status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_FAILURE;
      
      // Tune to the channel that the Scan Response was heard on
      touchLink_SetChannel( selectedTarget.rxChannel );
      
      if ( _NIB.nwkUpdateId <= selectedTarget.scanRsp.nwkUpdateId )
      {
        NLME_SetUpdateID( selectedTarget.scanRsp.nwkUpdateId );
      }
      // Ask the target to join our network
      bdb_Initiator_SendNwkJoinReq( );
      zTouchLinkNwkStartRtr = FALSE;
    }
    return ( events ^ TOUCHLINK_NWK_FORMATION_SUCCESS_EVT );
  }

  if ( events & TOUCHLINK_NWK_DISC_CNF_EVT )
  {
#if ZSTACK_ROUTER_BUILD
    if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
    {
      initiatorNwkParams.nwkAddr = APL_FREE_NWK_ADDR_RANGE_BEGIN;
    }
    // Copy in the encrypted network key
    touchLink_EncryptNwkKey( initiatorNwkParams.nwkKey, initiatorNwkParams.keyIndex, touchLinkTransID, touchLinkResponseID );
    // Start operating on the new network
    touchLinkStartRtr( &initiatorNwkParams, touchLinkTransID );
#else
    // Join to the chosen network
    osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_JOIN_NWK_ATTEMPT_EVT );
#endif
    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_DISC_CNF_EVT );
  }

  if ( events & TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT )
  {
    touchLinkTransID = 0;
    initiatorClearSelectedTarget();
    initiatorSetNwkToInitState();

    // return unprocessed events
    return ( events ^ TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT );
  }

  // If reach here, the events are unknown
  // Discard or make more handlers
  return 0;
}

/*********************************************************************
 * @fn      touchLinkInitiator_ChannelChange
 *
 * @brief   Change channel to supprot Frequency agility.
 *
 * @param   targetChannel - channel to
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_ChannelChange( uint8 targetChannel )
{
    uint32 channelMask;
    zAddrType_t dstAddr = {0};
    if ( ( targetChannel < 11 ) || targetChannel > 26 )
    {
      if (TOUCHLINK_PRIMARY_CHANNEL (_NIB.nwkLogicalChannel))
      {
        switch (_NIB.nwkLogicalChannel)
        {
        case TOUCHLINK_FIRST_CHANNEL:
          targetChannel = TOUCHLINK_SECOND_CHANNEL;
          break;
        case TOUCHLINK_SECOND_CHANNEL:
          targetChannel = TOUCHLINK_THIRD_CHANNEL;
          break;
        case TOUCHLINK_THIRD_CHANNEL:
          targetChannel = TOUCHLINK_FOURTH_CHANNEL;
          break;
        case TOUCHLINK_FOURTH_CHANNEL:
          targetChannel = TOUCHLINK_FIRST_CHANNEL;
        }
      }
      else
      {
        targetChannel = _NIB.nwkLogicalChannel + 1;
        if ( _NIB.nwkLogicalChannel > 26 )
          targetChannel = 11;
      }
    }

    dstAddr.addrMode = AddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
    channelMask = (uint32)1 << targetChannel;

    // Increment the nwkUpdateId parameter and set the updateID in the beacon
    NLME_SetUpdateID(_NIB.nwkUpdateId + 1);

    ZDP_MgmtNwkUpdateReq( &dstAddr, channelMask, 0xfe, 0, _NIB.nwkUpdateId, 0 );

    return ZSuccess;
}

/*********************************************************************
 * @fn      touchLinkSampleRemote_SendEPInfo
 *
 * @brief   Send Endpoint info command.
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   seqNum - transaction sequnece number
 *
 * @return  ZStatus_t
 */
ZStatus_t touchLinkInitiator_SendEPInfo( uint8 srcEP, afAddrType_t *dstAddr, uint8 seqNum)
{
    bdbTLEndpointInfo_t bdbTLEndpointInfoCmd;
    bdbTLDeviceInfo_t  bdbTLDeviceInfo;
      //send Epinfo cmd
    touchLink_GetSubDeviceInfo( 0, &bdbTLDeviceInfo );
    bdbTLEndpointInfoCmd.endpoint = bdbTLDeviceInfo.endpoint;
    bdbTLEndpointInfoCmd.profileID = bdbTLDeviceInfo.profileID;
    bdbTLEndpointInfoCmd.deviceID = bdbTLDeviceInfo.deviceID;
    bdbTLEndpointInfoCmd.version = bdbTLDeviceInfo.version;

    osal_cpyExtAddr( bdbTLEndpointInfoCmd.ieeeAddr, NLME_GetExtAddr() );
    bdbTLEndpointInfoCmd.nwkAddr = NLME_GetShortAddr();

    dstAddr->panId = _NIB.nwkPanId;
    return bdbTL_Send_EndpointInfo( srcEP, dstAddr, &bdbTLEndpointInfoCmd,
                                          0, seqNum );
}

/*********************************************************************
 * @fn      touchLinkInitiator_ResetToFNSelectedTarget
 *
 * @brief   Send Reset to Factory New Request command to the selected
 *          target of the current Touch-Link transaction.
 *          Note - this function should be called within no later than
 *          BDBCTL_INTER_PAN_TRANS_ID_LIFETIME ms from the Scan Request.
 *
 * @param   none
 *
 * @return  status - failure is returned due to invalid selected target or
 *          expired Touch-Link transaction.
 */
ZStatus_t touchLinkInitiator_ResetToFNSelectedTarget( void )
{
  bdbTLResetToFNReq_t req;
  req.transID = touchLinkTransID;

  // Cancel further touch-link commissioning (if called during identify interval)
  osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT );

  touchLink_SetChannel( selectedTarget.rxChannel );
  return bdbTL_Send_ResetToFNReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), &req, initiatorSeqNum++ );
}

/*********************************************************************
 * @fn      touchLink_InitiatorSendScanRsp
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
static ZStatus_t touchLink_InitiatorSendScanRsp( uint8 srcEP, afAddrType_t *dstAddr, uint32 transID, uint8 seqNum )
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
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT,
                          BDBCTL_INTER_PAN_TRANS_ID_LIFETIME );

      pRsp->transID = transID;
      pRsp->rssiCorrection = TOUCHLINK_RSSI_CORRECTION;
      pRsp->zLogicalType = zgDeviceLogicalType;
      pRsp->touchLinkAddressAssignment = touchLink_IsValidSplitFreeRanges( 0 );
      pRsp->touchLinkInitiator = TRUE;
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
 * @fn      bdb_Initiator_SendNwkJoinReq
 *
 * @brief   Send out a Network Join Router or End Device Request command.
 *          using the selected Target.
 *
 * @param   -
 *
 * @return  ZStatus_t
 */
ZStatus_t bdb_Initiator_SendNwkJoinReq( void )
{
  // Set NWK task to idle
  nwk_setStateIdle( TRUE );
  return initiatorSendNwkJoinReq( &(selectedTarget.scanRsp) );
}  

#if (ZSTACK_ROUTER_BUILD)
/*********************************************************************
 * @fn      touchLinkInitiator_PermitJoin
 *
 * @brief   Set the router permit join flag, to allow or deny classical
 *          commissioning by other ZigBee devices.
 *
 * @param   duration - enable up to aplcMaxPermitJoinDuration seconds,
 *                     0 to disable
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_PermitJoin( uint8 duration )
{
  return touchLink_PermitJoin( duration );
}  
#endif //(ZSTACK_ROUTER_BUILD)

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      initiatorScanReqCB
 *
 * @brief   This callback is called to process a Scan Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorScanReqCB( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8 seqNum )
{
  int8 rssi;
  rssi = touchLink_GetMsgRssi();
  if( ( rssi > TOUCHLINK_WORST_RSSI ) && ( pReq->touchLinkInitiator == TRUE ) )
  {
    // response to the originator, but switch to dst PAN 0xFFFF
    afAddrType_t dstAddr;
    osal_memcpy(&dstAddr, srcAddr, sizeof(afAddrType_t));
    dstAddr.panId = 0xFFFF;

    // If we are factory new and revice a Scan Request from other factory new
    // device then drop the request.
    if ( ( pReq->touchLinkFactoryNew == TRUE ) && ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) )
    {
      return ( ZSuccess );
    }
    // If, during its scan, a non factory new initiator receives another scan
    // request inter-PAN command frame from a factory new target, it shall be ignored.
    if ( ( pReq->touchLinkFactoryNew == TRUE ) && ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) &&
         osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT ) )
    {
      return ( ZSuccess );
    }

    // Send a Scan Response back
    if ( touchLink_InitiatorSendScanRsp( TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, pReq->transID, seqNum ) == ZSuccess )
    {
      // If we're a factory new initiator and are in the middle of a Device
      // Discovery, stop the procedure and wait for subsequent configuration
      // information from the non-factory new initiator that we just responded to.
      if ( ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) && !pReq->touchLinkFactoryNew )
      {
        osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT );
      }
    }
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorDeviceInfoReqCB
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
static ZStatus_t initiatorDeviceInfoReqCB( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8 seqNum )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }
  return ( touchLink_SendDeviceInfoRsp( TOUCHLINK_INTERNAL_ENDPOINT, srcAddr,
                                  pReq->startIndex, pReq->transID, seqNum ) );
}

/*********************************************************************
 * @fn      initiatorIdentifyReqCB
 *
 * @brief   This callback is called to process an Identify Request command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorIdentifyReqCB( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq )
{
  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

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
 * @fn      initiatorNwkJoinReqCB
 *
 * @brief   This callback is called to process Network Join
 *          Request and Network Join End Device Request commands.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkJoinReqCB( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8 seqNum )
{
  bdbTLNwkJoinRsp_t rsp;
  afAddrType_t dstAddr;
  nwk_states_t nwkState;

  if ( touchLink_IsValidTransID( pReq->transID ) == FALSE )
  {
    return ( ZFailure );
  }

  rsp.transID = pReq->transID;

  if ( nwk_ExtPANIDValid( pReq->nwkParams.extendedPANID ) )
    //NOTE: additional nwk params verification may be added here, e.g. ranges.
  {
#if ( ZSTACK_ROUTER_BUILD )
    nwkState = NWK_ROUTER;
#else
    // Save the request for later
    joinReq = *pReq;
    nwkState = NWK_ENDDEVICE;
#endif

    // Notify our task to join the new network
    osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_NWK_JOIN_IND_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME );

    osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT );
    osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT );

    rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS;
    
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

  dstAddr = *srcAddr;
  dstAddr.panId = 0xFFFF;

  // Send a response back
#if ( ZSTACK_ROUTER_BUILD )
  bdbTL_Send_NwkJoinRtrRsp( TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, &rsp, seqNum );
#else
  bdbTL_Send_NwkJoinEDRsp( TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, &rsp, seqNum );
#endif

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorNwkUpdateReqCB
 *
 * @brief   This callback is called to process a Network Update Request
 *          command.
 *
 * @param   srcAddr - sender's address
 * @param   pReq - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkUpdateReqCB( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq )
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
      touchLink_ProcessNwkUpdate( pReq->nwkUpdateId, pReq->logicalChannel );
    }
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorScanRspCB
 *
 * @brief   This callback is called to process a Scan Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorScanRspCB( afAddrType_t *srcAddr, bdbTLScanRsp_t *pRsp )
{
  bdbFindingBindingRespondent_t *pCurr;
  
  if ( osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT )
       && ( touchLink_IsValidTransID( pRsp->transID ) )
       && ( pRsp->keyBitmask & touchLink_GetNwkKeyBitmask() ) )
  {

    uint8 selectThisTarget = FALSE;
    int8 rssi = touchLink_GetMsgRssi();
    if ( pfnSelectDiscDevCB != NULL )
    {
      selectThisTarget = pfnSelectDiscDevCB( pRsp, rssi );
    }
    // Default selection - according to RSSI
    else if ( rssi > TOUCHLINK_WORST_RSSI )
    {
      if ( ( rssi + pRsp->rssiCorrection ) > selectedTarget.lastRssi )
      {
        // Better RSSI, select this target
        selectThisTarget = TRUE;
      }
    }

    if ( selectThisTarget )
    {
      selectedTarget.scanRsp = *pRsp;
      selectedTarget.lastRssi = rssi;
      selectedTarget.srcAddr = *srcAddr;
      selectedTarget.srcAddr.panId = 0xFFFF;
      touchLinkResponseID = pRsp->responseID;
      touchLinkTransID = pRsp->transID;

      // Remember channel we heard back this scan response on
      ZMacGetReq( ZMacChannel, &(selectedTarget.rxChannel));

      if ( pRsp->numSubDevices > 1 )
      {
        selectedTarget.scanRsp.deviceInfo.endpoint = DEV_INFO_INVALID_EP;

        bdbTLDeviceInfoReq_t devInfoReq;
        devInfoReq.transID = pRsp->transID;
        devInfoReq.startIndex = 0;

        return bdbTL_Send_DeviceInfoReq( TOUCHLINK_INTERNAL_ENDPOINT, srcAddr,
                                    &devInfoReq, initiatorSeqNum++ );
      }
        
      // add new node to the list
      pCurr = bdb_AddRespondentNode( &pRespondentHead, NULL );
      
      if ( pCurr != NULL )
      {
        pCurr->data.endPoint = pRsp->deviceInfo.endpoint;
        pCurr->data.panId = srcAddr->panId;
      }
    }
    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      initiatorDeviceInfoRspCB
 *
 * @brief   This callback is called to process a Device Information
 *          Response command.
 *          If sub-device is selected, selectedTarget data is updated.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorDeviceInfoRspCB( afAddrType_t *srcAddr, bdbTLDeviceInfoRsp_t *pRsp )
{
  bdbFindingBindingRespondent_t *pCurr;
  
  if ( touchLink_IsValidTransID( pRsp->transID )
       && ( srcAddr->addr.shortAddr == selectedTarget.srcAddr.addr.shortAddr ) )
  {
    uint8 i;
    uint8 selectedIdx = pRsp->cnt;
    bdbTLScanRsp_t tmpInfo = selectedTarget.scanRsp;

    for ( i = 0; i < pRsp->cnt; ++i )
    {
      if ( pfnSelectDiscDevCB != NULL )
      {
        tmpInfo.deviceInfo = pRsp->devInfoRec[i].deviceInfo;
        if ( pfnSelectDiscDevCB( &tmpInfo, selectedTarget.lastRssi ) )
        {
          selectedIdx = i;
          // no break here to allow cycling through all sub-devices
        }
      }
      else
      {
        if ( ( pRsp->devInfoRec[i].deviceInfo.profileID == TOUCHLINK_PROFILE_ID ) ||
             ( pRsp->devInfoRec[i].deviceInfo.profileID == Z3_PROFILE_ID ) )
        {
          selectedIdx = i;
          break; // first match
        }
      }
    }
    if ( selectedIdx < pRsp->cnt )
    {
      // NOTE - the original scan response device info is overwritten with the
      // selected sub-device info, to complete the data required for the application.
      selectedTarget.scanRsp.deviceInfo = pRsp->devInfoRec[selectedIdx].deviceInfo;
      
      for ( i = 0; i < pRsp->cnt; ++i )
      {
        if ( ( pRsp->devInfoRec[i].deviceInfo.profileID == TOUCHLINK_PROFILE_ID ) ||
             ( pRsp->devInfoRec[i].deviceInfo.profileID == Z3_PROFILE_ID ) )
        {
          // add new node to the list
          pCurr = bdb_AddRespondentNode( &pRespondentHead, NULL );
          
          if ( pCurr != NULL )
          {
            osal_memset(pCurr, 0x00, sizeof(bdbFindingBindingRespondent_t));
            pCurr->data.endPoint = pRsp->devInfoRec[i].deviceInfo.endpoint;
            pCurr->data.panId = srcAddr->panId;
            osal_cpyExtAddr(selectedTargetIEEEAddr, pRsp->devInfoRec[i].ieeeAddr);
          }
        }
      }
    }
    else
    {
      // no sub-device of the currently selected device was selected.
      // clear selection
      initiatorClearSelectedTarget();
      bdb_zclRespondentListClean( &pRespondentHead );
    }
    return ( ZSuccess );
  }
  return ( ZFailure );
}

/*********************************************************************
 * @fn      initiatorNwkStartRspCB
 *
 * @brief   This callback is called to process a Network Start Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkStartRspCB( afAddrType_t *srcAddr, bdbTLNwkStartRsp_t *pRsp )
{
  if ( touchLink_IsValidTransID( pRsp->transID ) == FALSE )
  {
    return ( ZFailure );
  }
  // Make sure we didn't timeout waiting for this response
  if ( osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT ) )
  {
    // Save the Network Start Response for later
    rxRsp.nwkStartRsp = *pRsp;

    // No need to wait longer
    osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT );  
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorNwkJoinRtrRspCB
 *
 * @brief   This callback is called to process a Network Join Router
 *          Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkJoinRtrRspCB( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp )
{
  if ( ( touchLink_IsValidTransID( pRsp->transID ) == FALSE ) || 
     ( ( srcAddr->addrMode != afAddr64Bit ) || ( !osal_memcmp( selectedTarget.srcAddr.addr.extAddr, srcAddr->addr.extAddr, Z_EXTADDR_LEN ) ) ) )
  {
    return ( ZFailure );
  }
  
  // Make sure we didn't timeout waiting for this response
  if ( osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT ) )
  {
    // Save the Network Start Response for later
    rxRsp.nwkJoinRsp = *pRsp;

    // No need to wait longer
    osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT );
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      initiatorNwkJoinEDRspCB
 *
 * @brief   This callback is called to process a Network Join End Device
 *          Response command.
 *
 * @param   srcAddr - sender's address
 * @param   pRsp - parsed command
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorNwkJoinEDRspCB( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp )
{
  if ( ( touchLink_IsValidTransID( pRsp->transID ) == FALSE ) || 
     ( ( srcAddr->addrMode != afAddr64Bit ) || ( !osal_memcmp( selectedTarget.srcAddr.addr.extAddr, srcAddr->addr.extAddr, Z_EXTADDR_LEN ) ) ) )
  {
    return ( ZFailure );
  }
  
  // Make sure we didn't timeout waiting for this response
  if ( osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT ) )
  {
    // Save the Network Start Response for later
    rxRsp.nwkJoinRsp = *pRsp;

    // No need to wait longer
    osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT );

  }
  else
  {
    rxRsp.nwkJoinRsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_FAILURE;
  }

  return ( ZSuccess );
}

/******************************************************************************
 * @fn      initiatorZdoLeaveCnfCB
 *
 * @brief   This callback is called to process a Leave Confirmation message.
 *
 *          Note: this callback function returns a pointer if it has handled
 *                the confirmation message and no further action should be
 *                taken with it. It returns NULL if it has not handled the
 *                confirmation message and normal processing should take place.
 *
 * @param   pParam - received message
 *
 * @return  Pointer if message processed. NULL, otherwise.
 */
static void *initiatorZdoLeaveCnfCB( void *pParam )
{
  (void)pParam;

  // Did we initiate the leave?
  if ( touchLinkLeaveInitiated == FALSE )
  {
    return ( NULL );
  }

  if ( touchLinkLeaveInitiated == TOUCHLINK_LEAVE_TO_JOIN_NWK )
  {
    // Notify our task to join the new network
    osal_set_event( touchLinkInitiator_TaskID, TOUCHLINK_JOIN_NWK_ATTEMPT_EVT );
  }

  return ( (void *)&touchLinkLeaveInitiated );
}

/*********************************************************************
 * @fn      initiatorProcessStateChange
 *
 * @brief   Process ZDO device state change
 *
 * @param   state - The device's network state
 *
 * @return  none
 */
static void initiatorProcessStateChange( devStates_t state )
{
  if ( ( ( devState == DEV_ROUTER ) || ( devState == DEV_END_DEVICE ) ) && ( touchlinkDistNwk == TRUE ) )
  {
    // Set touchlink flag to false after joining is complete
    touchlinkDistNwk = FALSE;
    
    // Save the latest NIB to update our parent's address
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );
   
    if ( !_NIB.CapabilityFlags )
    {
      _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;
    }

#if (ZSTACK_ROUTER_BUILD)
      linkInfo_t *linkInfo;
      
      // check if entry exists
      linkInfo = nwkNeighborGetLinkInfo( selectedTargetNwkAddr, _NIB.nwkPanId );

      // if not, look for a vacant entry to add this node...
      if ( linkInfo == NULL )
      {
        nwkNeighborAdd( selectedTargetNwkAddr, _NIB.nwkPanId, DEF_LQI );
        linkInfo = nwkNeighborGetLinkInfo( selectedTargetNwkAddr, _NIB.nwkPanId );
        linkInfo->txCost = DEF_LINK_COST;
        linkInfo->rxLqi = MIN_LQI_COST_1; 
      }
      
      bdb_setNodeIsOnANetwork(TRUE);
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT, 500);
#endif

    // We found our parent
    osal_stop_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_REJOIN_EVT );
  }
  
  else if ( ( state == DEV_NWK_ORPHAN ) || ( state == DEV_NWK_DISC ) )
  {
    // Device has lost information about its parent; give it some time to rejoin
    if ( !osal_get_timeoutEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_REJOIN_EVT ) )
    {
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_REJOIN_EVT,
                          (NUM_DISC_ATTEMPTS + 1) * TOUCHLINK_INITIATOR_REJOIN_TIMEOUT );
    }
  }
}

/*********************************************************************
 * @fn      initiatorSetNwkToInitState
 *
 * @brief   Set our network state to its original state.
 *
 * @param   void
 *
 * @return  void
 */
static void initiatorSetNwkToInitState()
{
  // Turn MAC receiver back to its old state
  ZMacSetReq( ZMacRxOnIdle, &savedRxOnIdle );

  // Tune back to our channel
  touchLink_SetChannel( _NIB.nwkLogicalChannel );

  // Set NWK task to run
  nwk_setStateIdle( FALSE );

  if ( savedPollRate != zgPollRate )
  {
    NLME_SetPollRate( savedPollRate );
    savedPollRate = POLL_RATE;
  }

  if ( savedQueuedPollRate != zgQueuedPollRate )
  {
    NLME_SetQueuedPollRate( savedQueuedPollRate );
    savedQueuedPollRate = QUEUED_POLL_RATE;
  }

  if ( savedResponsePollRate != zgResponsePollRate )
  {
    NLME_SetResponseRate( savedResponsePollRate );
    savedResponsePollRate = RESPONSE_POLL_RATE;
  }
}

#if ( ZSTACK_ROUTER_BUILD )
/*********************************************************************
 * @fn      initiatorJoinNwk
 *
 * @brief   Initiate a network join request.
 *
 * @param   void
 *
 * @return  void
 */
static void initiatorJoinNwk( void )
{
  // Save free ranges
  touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

  // In case we're here after a leave
  touchLinkLeaveInitiated = FALSE;

  // Clear leave control logic
  ZDApp_LeaveCtrlReset();

  if ( POLL_RATE == 0 )
  {
    //allow to respond to TOUCHLINK commission utility commands after TL
    NLME_SetPollRate( TOUCHLINK_INITIATOR_TEMP_POST_TL_POLL_RATE );
    //polling should reset when TL life time expires
  }

  touchLinkInitiator_PermitJoin( 0 );

  ZDOInitDevice( 0 );
  // Perform a network rejoin request
  _NIB.nwkState = NWK_REJOINING;
  initiatorReJoinNwk( MODE_REJOIN );
}
#endif

/*********************************************************************
 * @fn      initiatorReJoinNwk
 *
 * @brief   Send out an Rejoin Request.
 *
 * @param   startMode - MODE_REJOIN or MODE_RESUME
 *
 * @return  none
 */
static void initiatorReJoinNwk( devStartModes_t startMode )
{
  // Set NWK task to run
  nwk_setStateIdle( FALSE );

  // Configure MAC with our network parameters
  touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );

  // Use the new network paramters
  zgConfigPANID = _NIB.nwkPanId;
  zgDefaultChannelList = _NIB.channelList;
  osal_cpyExtAddr( ZDO_UseExtendedPANID, _NIB.extendedPANID );

  devStartMode = startMode;

  _tmpRejoinState = TRUE;

  // Start the network joining process
  osal_set_event( ZDAppTaskID, ZDO_NETWORK_INIT );
}

/*********************************************************************
 * @fn      initiatorSendScanReq
 *
 * @brief   Send out an Scan Request command on one of the TOUCHLINK channels.
 *
 * @param   freshScan - TRUE to start fresh scan, FALSE to resume existing process.
 *
 * @return  void
 */
static void initiatorSendScanReq( bool freshScan )
{
  bdbTLScanReq_t req;
  uint8 newChannel;
  uint8 secondaryChList[] = TOUCHLINK_SECONDARY_CHANNELS_SET;
  static uint8 channelIndex = 0;
  
  // Set the device as initiator of touchlink commissioning
  touchLink_DeviceIsInitiator( TRUE );

  if ( freshScan )
  {
    channelIndex = 0;
  }

  // First figure out the channel
  if ( scanReqChannels == TOUCHLINK_SCAN_PRIMARY_CHANNELS )
  {
    if ( numScanReqSent < 5 )
    {
      // First five consecutive requests are sent on channel 11
      newChannel = TOUCHLINK_FIRST_CHANNEL;
    }
    else if ( numScanReqSent == 5 )
    {
      // Sixth request is sent on channel 15
      newChannel = TOUCHLINK_SECOND_CHANNEL;
    }
    else if ( numScanReqSent == 6 )
    {
      // Seventh request is sent on channel 20
      newChannel = TOUCHLINK_THIRD_CHANNEL;
    }
    else
    {
      // Last request is sent on channel 25
      newChannel = TOUCHLINK_FOURTH_CHANNEL;
    }
  }
  else
  {
    // scan secondary channel list
    if ( channelIndex < sizeof(secondaryChList) )
    {
       newChannel = secondaryChList[channelIndex++];
    }
    else
    {
      // set it to initial value for next discovery process
      channelIndex = 0;
      return;
    }
  }

  if ( touchLinkTransID != 0 )
  {
    // Build the request
    req.transID = touchLinkTransID;
    touchLinkLastAcceptedTransID = touchLinkTransID;

    req.zInfo.zInfoByte = 0;
    req.zLogicalType = zgDeviceLogicalType;
    if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
    {
      req.zRxOnWhenIdle = TRUE;
    }

    req.touchLinkInfo.touchLinkInfoByte = 0;
    req.touchLinkFactoryNew = !bdbAttributes.bdbNodeIsOnANetwork;
    req.touchLinkAddressAssignment = TRUE;
    req.touchLinkInitiator = TRUE;

    // First switch to the right channel
    touchLink_SetChannel( newChannel );

    // Broadcast the request
    bdbTL_Send_ScanReq( TOUCHLINK_INTERNAL_ENDPOINT, &bcastAddr, &req, initiatorSeqNum++ );

    numScanReqSent++;

    // After each transmission, wait BDBCTL_SCAN_TIME_BASE_DURATION seconds
    // to receive any responses.
    osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT, BDBCTL_SCAN_TIME_BASE_DURATION );
  }
  else
  {
    touchLinkInitiator_AbortTL();
  }
}

/*********************************************************************
 * @fn      initiatorSendNwkStartReq
 *
 * @brief   Send out a Network Start Request command.
 *
 * @param   pRsp - received Scan Response
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorSendNwkStartReq( bdbTLScanRsp_t *pRsp )
{
  bdbTLNwkStartReq_t *pReq;
  ZStatus_t status;

  pReq = (bdbTLNwkStartReq_t *)osal_mem_alloc( sizeof( bdbTLNwkStartReq_t ) );
  if ( pReq != NULL )
  {
    uint16 i;
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);

    osal_memset( pReq, 0, sizeof( bdbTLNwkStartReq_t ) );

    // Build the request
    pReq->transID = selectedTarget.scanRsp.transID;

    // Find out key index (prefer highest)
    for ( i = 15; i > 0; i-- )
    {
      if ( ( (uint16)1 << i ) & pRsp->keyBitmask )
      {
        break;
      }
    }
    pParams->keyIndex = i;

    // Copy in the encrypted network key
    touchLink_EncryptNwkKey( pParams->nwkKey, i, pRsp->transID, pRsp->responseID );

    pParams->nwkAddr = touchLink_PopNwkAddress();
    if ( pParams->nwkAddr == 0 )
    {
      pParams->nwkAddr = osal_rand();
    }
    // update address for app notification
    selectedTarget.newNwkAddr = pParams->nwkAddr;
    selectedTargetNwkAddr = pParams->nwkAddr;

    // Set group ID range
    if ( pRsp->totalGrpIDs > 0 )
    {
      touchLink_PopGrpIDRange( pRsp->totalGrpIDs, &(pParams->grpIDsBegin), &(pParams->grpIDsEnd) );
    }

    if ( pRsp->touchLinkAddressAssignment )
    {
      touchLink_SplitFreeRanges( &(pParams->freeNwkAddrBegin), &(pParams->freeNwkAddrEnd),
                           &(pParams->freeGrpIDBegin), &(pParams->freeGrpIDEnd) );
    }

#ifdef TOUCHLINK_INITIATOR_SET_NEW_NWK_PARAMS
    pParams->logicalChannel = _NIB.nwkLogicalChannel;
    pParams->panId = _NIB.nwkPanId;
    osal_memcpy( pParams->extendedPANID, _NIB.extendedPANID ,Z_EXTADDR_LEN);
#endif

    osal_cpyExtAddr( pReq->initiatorIeeeAddr, NLME_GetExtAddr() );
    pReq->initiatorNwkAddr = _NIB.nwkDevAddress;

    status = bdbTL_Send_NwkStartReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), pReq, initiatorSeqNum++ );
    if ( status == ZSuccess )
    {
      // Keep a copy of the encryted network key sent to the target
      keyIndexSent = i;
      osal_memcpy( encKeySent, pParams->nwkKey, SEC_KEY_LEN );
      responseIDSent = pRsp->responseID;

      // After the transmission, wait BDBCTL_RX_WINDOW_DURATION seconds to
      // receive a response.
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT, BDBCTL_RX_WINDOW_DURATION );
    }

    osal_mem_free( pReq );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      initiatorSendNwkJoinReq
 *
 * @brief   Send out a Network Join Router or End Device Request command.
 *
 * @param   pRsp - received Scan Response
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorSendNwkJoinReq( bdbTLScanRsp_t *pRsp )
{
  bdbTLNwkJoinReq_t *pReq;
  ZStatus_t status;
  bdbFindingBindingRespondent_t *pCurr;

  pReq = (bdbTLNwkJoinReq_t *)osal_mem_alloc( sizeof( bdbTLNwkJoinReq_t ) );
  if ( pReq != NULL )
  {
    uint16 i;
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);

    osal_memset( pReq, 0, sizeof( bdbTLNwkJoinReq_t ) );

    // Build the request
    pReq->transID = selectedTarget.scanRsp.transID;

    // Find out key index (prefer highest)
    for ( i = 15; i > 0; i-- )
    {
      if ( ( (uint16)1 << i ) & pRsp->keyBitmask )
      {
        break;
      }
    }
    pParams->keyIndex = i;

    // Copy in the encrypted network key
    touchLink_EncryptNwkKey( pParams->nwkKey, i, pRsp->transID, pRsp->responseID );

    pParams->nwkAddr = touchLink_PopNwkAddress();
    if ( pParams->nwkAddr == 0 )
    {
      pParams->nwkAddr = osal_rand();
    }
    // update address for app notification
    selectedTarget.newNwkAddr = pParams->nwkAddr;
    selectedTargetNwkAddr = pParams->nwkAddr;
    
    // Set group ID range
    if ( pRsp->totalGrpIDs > 0 )
    {
      touchLink_PopGrpIDRange( pRsp->totalGrpIDs, &(pParams->grpIDsBegin), &(pParams->grpIDsEnd) );
    }

    if ( pRsp->touchLinkAddressAssignment )
    {
      touchLink_SplitFreeRanges( &(pParams->freeNwkAddrBegin), &(pParams->freeNwkAddrEnd),
                           &(pParams->freeGrpIDBegin), &(pParams->freeGrpIDEnd) );
    }
    // update 
    pCurr = pRespondentHead;
    while( pCurr != NULL )
    {
      pCurr->data.addr.shortAddr = pParams->nwkAddr;
      pCurr->data.addrMode = afAddr16Bit;
      pCurr->data.panId = _NIB.nwkPanId;
      pCurr = pCurr->pNext;
    }

    pParams->logicalChannel = _NIB.nwkLogicalChannel;
    pParams->panId = _NIB.nwkPanId;
    osal_cpyExtAddr( pParams->extendedPANID, _NIB.extendedPANID );
    pReq->nwkUpdateId = _NIB.nwkUpdateId;

    // Let PAN ID, Extended PAN ID and Logical Channel to be determined by the target
    if ( pRsp->zLogicalType == ZG_DEVICETYPE_ROUTER )
    {
      // It's a light
      status = bdbTL_Send_NwkJoinRtrReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), pReq, initiatorSeqNum++ );
    }
    else // another controller
    {
      status = bdbTL_Send_NwkJoinEDReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), pReq, initiatorSeqNum++ );
    }

    if ( status == ZSuccess )
    {
      // After the transmission, wait BDBCTL_RX_WINDOW_DURATION seconds to
      // receive a response.
      osal_start_timerEx( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT, BDBCTL_RX_WINDOW_DURATION );
    }

    osal_mem_free( pReq );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      initiatorSendNwkUpdateReq
 *
 * @brief   Send out a Network Update Request command.
 *
 * @param   pRsp - received Scan Response
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorSendNwkUpdateReq( bdbTLScanRsp_t *pRsp )
{
  bdbTLNwkUpdateReq_t *pReq;
  ZStatus_t status;

  pReq = (bdbTLNwkUpdateReq_t *)osal_mem_alloc( sizeof( bdbTLNwkUpdateReq_t ) );
  if ( pReq!= NULL )
  {
    // Build the request
    pReq->transID = pRsp->transID;
    osal_cpyExtAddr( pReq->extendedPANID, _NIB.extendedPANID );
    pReq->nwkUpdateId = _NIB.nwkUpdateId;
    pReq->logicalChannel = _NIB.nwkLogicalChannel;
    pReq->PANID = _NIB.nwkPanId;
    pReq->nwkAddr = pRsp->nwkAddr;

    status = bdbTL_Send_NwkUpdateReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), pReq, initiatorSeqNum++ );

    osal_mem_free( pReq );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      initiatorClearSelectedTarget
 *
 * @brief   clear selected target variable.
 *
 * @param   none
 *
 * @return  none
 */
static void initiatorClearSelectedTarget( void )
{
  osal_memset( &selectedTarget, 0x00, sizeof(targetCandidate_t) );
  selectedTarget.lastRssi = TOUCHLINK_WORST_RSSI;
}

#endif //BDB_TL_INITIATOR

/*********************************************************************
*********************************************************************/
