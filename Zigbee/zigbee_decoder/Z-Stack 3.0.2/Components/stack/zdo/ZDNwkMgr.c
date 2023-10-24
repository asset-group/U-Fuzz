/**************************************************************************************************
  Filename:       ZDNwkMgr.c
  Revised:        $Date: 2007-10-17 15:38:45 -0700 (Wed, 17 Oct 2007) $
  Revision:       $Revision: 15716 $

  Description:    The ZigBee Network Manager.


  Copyright 2007-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "nwk_util.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZGlobals.h"
#include "ZDNwkMgr.h"

#if defined( MT_ZDO_FUNC )
  #include "MT_ZDO.h"
#endif
  
#if defined ( LCD_SUPPORTED )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
  
/******************************************************************************
 * CONSTANTS
 */

#define ONE_MINUTE             60000  // 1(m) * 60(s) * 1000(ms)

#if defined ( LCD_SUPPORTED )
  const char NwkMgrStr_1[]     = "NM-fail not hi";
  const char NwkMgrStr_2[]     = "NM-cur<last fail";
  const char NwkMgrStr_3[]     = "NM-energy too hi";
  const char NwkMgrStr_4[]     = "NM-energy not up";
#endif
  
/******************************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
  
// Task ID for internal task/event processing. This variable will be
// received when ZDNwkMgr_Init() is called.
uint8 ZDNwkMgr_TaskID = 0;

/******************************************************************************
 * LOCAL VARIABLES
 */

// Frequency Agility variables
uint8 ZDNwkMgr_MgmtNwkUpdateNotifyTransSeq = 0;
zAddrType_t ZDNwkMgr_MgmtNwkUpdateNotifyAddr;
uint16 ZDNwkMgr_UpdateNotifyTimer = 0;
uint8  ZDNwkMgr_NumUpdateNotifySent = 0;
uint8  ZDNwkMgr_WaitingForNotifyConfirm = FALSE;
uint16 ZDNwkMgr_TotalTransmissions;
uint16 ZDNwkMgr_TxFailures;

ZDO_MgmtNwkUpdateReq_t ZDNwkMgr_MgmtNwkUpdateReq;
  
#if defined ( NWK_MANAGER )
uint16 ZDNwkMgr_UpdateRequestTimer = 0;
uint8  ZDNwkMgr_LastChannelEnergy = 0;
uint16 ZDNwkMgr_LastChannelFailureRate = 0;
#endif // NWK_MANAGER

uint8 ZDNwkMgr_NewChannel;

// PAN ID Conflict variables
#if defined ( NWK_MANAGER )
uint8 ZDNwkMgr_PanIdUpdateInProgress = FALSE;
#endif // NWK_MANAGER

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
// Freguency Agility functions
void (*pZDNwkMgr_ReportChannelInterference)( NLME_ChanInterference_t *chanInterference ) = NULL;
void (*pZDNwkMgr_ProcessDataConfirm)( afDataConfirm_t *afDataConfirm ) = NULL;
void (*pZDNwkMgr_EDScanConfirmCB)( NLME_EDScanConfirm_t *EDScanConfirm ) = NULL;

// PAN ID Conflict functions
void (*pZDNwkMgr_NetworkReportCB)( ZDNwkMgr_NetworkReport_t *pReport ) = NULL;
void (*pZDNwkMgr_NetworkUpdateCB)( ZDNwkMgr_NetworkUpdate_t *pUpdate ) = NULL;

/******************************************************************************
 * LOCAL FUNCTIONS
 */

void ZDNwkMgr_ProcessServerDiscRsp( zdoIncomingMsg_t *inMsg );
void ZDNwkMgr_SetNwkManagerAddr( uint16 nwkManagerAddr );

// Frequency Agility functions
static void ZDNwkMgr_ProcessMsgCBs( zdoIncomingMsg_t *inMsg );

static void ZDNwkMgr_ProcessMgmtNwkUpdateReq( zdoIncomingMsg_t *inMsg );
static void ZDNwkMgr_ProcessChannelInterference( ZDNwkMgr_ChanInterference_t *pChanInterference );
static void ZDNwkMgr_ProcessEDScanConfirm( ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm );
static void ZDNwkMgr_CheckForChannelInterference( ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm );
static void ZDNwkMgr_BuildAndSendUpdateNotify( uint8 TransSeq, zAddrType_t *dstAddr,
                                               uint16 totalTransmissions, uint16 txFailures,
                                               ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm, uint8 txOptions );
void ZDNwkMgr_EDScanConfirmCB( NLME_EDScanConfirm_t *EDScanConfirm );
void ZDNwkMgr_ProcessDataConfirm( afDataConfirm_t *afDataConfirm );
void ZDNwkMgr_ReportChannelInterference( NLME_ChanInterference_t *chanInterference );

#if defined ( NWK_MANAGER )
static void ZDNwkMgr_ProcessMgmtNwkUpdateNotify( zdoIncomingMsg_t *inMsg );
static void ZDNwkMgr_CheckForChannelChange( ZDO_MgmtNwkUpdateNotify_t *pNotify );
#endif // NWK_MANAGER

// PAN ID Conflict functions
#if defined ( NWK_MANAGER )
void ZDNwkMgr_NetworkReportCB( ZDNwkMgr_NetworkReport_t *pReport );
void ZDNwkMgr_NetworkUpdateCB( ZDNwkMgr_NetworkUpdate_t *pUpdate );

void ZDNwkMgr_ProcessNetworkReport( ZDNwkMgr_NetworkReport_t *pNetworkReport );
void ZDNwkMgr_ProcessNetworkUpdate( ZDNwkMgr_NetworkUpdate_t *pNetworkUpdate );
#endif // NWK_MANAGER

/*********************************************************************
 * @fn      ZDNwkMgr_Init
 *
 * @brief   Initialization function for the Network Manager Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void ZDNwkMgr_Init( byte task_id )
{
  // Save the task ID
  ZDNwkMgr_TaskID = task_id;

  ZDO_RegisterForZDOMsg( ZDNwkMgr_TaskID, Server_Discovery_rsp );

  // Frequecy Agility initialization
  ZDO_RegisterForZDOMsg( ZDNwkMgr_TaskID, Mgmt_NWK_Update_req );
#if defined ( NWK_MANAGER )
  ZDO_RegisterForZDOMsg( ZDNwkMgr_TaskID, Mgmt_NWK_Update_notify );
#endif // NWK_MANAGER

  pZDNwkMgr_EDScanConfirmCB = ZDNwkMgr_EDScanConfirmCB;
  pZDNwkMgr_ProcessDataConfirm = ZDNwkMgr_ProcessDataConfirm;
  pZDNwkMgr_ReportChannelInterference = ZDNwkMgr_ReportChannelInterference;
  
  // PAN ID Conflict initialization
#if defined ( NWK_MANAGER )
  pZDNwkMgr_NetworkReportCB = ZDNwkMgr_NetworkReportCB;
  pZDNwkMgr_NetworkUpdateCB = ZDNwkMgr_NetworkUpdateCB;
#endif // NWK_MANAGER
  
  ZDNwkMgr_MgmtNwkUpdateNotifyAddr.addrMode = Addr16Bit;
  ZDNwkMgr_MgmtNwkUpdateNotifyAddr.addr.shortAddr = INVALID_NODE_ADDR;
}

/*********************************************************************
 * @fn      ZDNwkMgr_event_loop
 *
 * @brief   Main event loop for the Network Manager task. This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
UINT16 ZDNwkMgr_event_loop( byte task_id, UINT16 events )
{
  osal_event_hdr_t *msgPtr;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    msgPtr = (osal_event_hdr_t *)osal_msg_receive( ZDNwkMgr_TaskID );
    while ( msgPtr )
    {
      switch ( msgPtr->event )
      {
        case ZDO_CB_MSG:
          // ZDO sends the message that we registered for
          ZDNwkMgr_ProcessMsgCBs( (zdoIncomingMsg_t *)msgPtr );
          break;
         
        case NM_CHANNEL_INTERFERE:
          // NWK layer sends the message when it detectes Channel Interference
          ZDNwkMgr_ProcessChannelInterference( (ZDNwkMgr_ChanInterference_t *)msgPtr );
          break;
   
        case NM_ED_SCAN_CONFIRM:
          // NWK layer sends the message when it receives an ED scan confirmation
          ZDNwkMgr_ProcessEDScanConfirm( (ZDNwkMgr_EDScanConfirm_t *)msgPtr );
          break;
#if defined ( NWK_MANAGER )
        case ZDO_NETWORK_REPORT:
          // NWK layer sends this message when it receives a Network Report message
          ZDNwkMgr_ProcessNetworkReport( (ZDNwkMgr_NetworkReport_t *)msgPtr );
          break;
       
        case ZDO_NETWORK_UPDATE:
          // NKW layer sends this message when it receives a Network Update message
          ZDNwkMgr_ProcessNetworkUpdate( (ZDNwkMgr_NetworkUpdate_t *)msgPtr );
          break;
#endif // NWK_MANAGER         
        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)msgPtr );

      // Next
      msgPtr = (osal_event_hdr_t *)osal_msg_receive( ZDNwkMgr_TaskID );
    }
    
    // Return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & ZDNWKMGR_CHANNEL_CHANGE_EVT )
  {       
    // Switch channel
    _NIB.nwkLogicalChannel = ZDNwkMgr_NewChannel;
    ZMacSetReq( ZMacChannel, &ZDNwkMgr_NewChannel );
 
    // Our Channel has been changed -- notify to save info into NV
    ZDApp_NwkStateUpdateCB();
    
    // Reset the total transmit count and the transmit failure counters
    _NIB.nwkTotalTransmissions = 0;
    nwkTransmissionFailures( TRUE );
    
    return ( events ^ ZDNWKMGR_CHANNEL_CHANGE_EVT );
  }

  if ( events & ZDNWKMGR_UPDATE_NOTIFY_EVT )
  {
    // Update the Update Notify timer
    if ( ZDNwkMgr_UpdateNotifyTimer > 0 )
    {
      ZDNwkMgr_UpdateNotifyTimer--;
      osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_UPDATE_NOTIFY_EVT, ONE_MINUTE );
    }
    else
    {
      ZDNwkMgr_NumUpdateNotifySent = 0;
    }
    
    return ( events ^ ZDNWKMGR_UPDATE_NOTIFY_EVT );
  }
  
#if defined ( NWK_MANAGER )
  if ( events & ZDNWKMGR_UPDATE_REQUEST_EVT )
  {
    // Update the Update Request timer
    if ( ZDNwkMgr_UpdateRequestTimer > 0 )
    {
      ZDNwkMgr_UpdateRequestTimer--;
      osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_UPDATE_REQUEST_EVT, ONE_MINUTE );
    }
    
    return ( events ^ ZDNWKMGR_UPDATE_REQUEST_EVT );
  }
#endif // NWK_MANAGER
  
  if ( events & ZDNWKMGR_SCAN_REQUEST_EVT )
  {  
    if ( ZDNwkMgr_MgmtNwkUpdateReq.scanCount > 0 )
    {
      if (  NLME_EDScanRequest( ZDNwkMgr_MgmtNwkUpdateReq.channelMask, 
                                ZDNwkMgr_MgmtNwkUpdateReq.scanDuration ) == ZSuccess )
      {
        ZDNwkMgr_MgmtNwkUpdateReq.scanCount--;
      }
    }
      
    return ( events ^ ZDNWKMGR_SCAN_REQUEST_EVT );
  }
  
  // Discard or make more handlers
  return 0;
}

/*********************************************************************
 * @fn      ZDNwkMgr_ProcessMsgCBs
 *
 * @brief   Process the incoming messages.
 *
 * @param   msgPtr - message to process
 *
 * @return  TRUE if message to be freed. FALSE otherwise.
 */
static void ZDNwkMgr_ProcessMsgCBs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {   
    case Mgmt_NWK_Update_req:
      ZDNwkMgr_ProcessMgmtNwkUpdateReq( inMsg );
      break;    
#if defined ( NWK_MANAGER )  
    case Mgmt_NWK_Update_notify:
      ZDNwkMgr_ProcessMgmtNwkUpdateNotify( inMsg );
      break;
#endif // NWK_MANAGER
    case Server_Discovery_rsp:
      ZDNwkMgr_ProcessServerDiscRsp( inMsg );
      break;
      
    default:
      // Unknown message
      break;
  }
}

/*********************************************************************
 * Frequency Agility Routines
 */
#if defined ( NWK_MANAGER )
/*********************************************************************
 * @fn          ZDNwkMgr_ProcessMgmtNwkUpdateNotify
 *
 * @brief       This function processes the incoming Management
 *              Network Update notify.
 *
 * @param       pUpdateNotify - notify message
 *
 * @return      TRUE if message to be freed. FALSE otherwise.
 */
static void ZDNwkMgr_ProcessMgmtNwkUpdateNotify( zdoIncomingMsg_t *inMsg )
{
  if ( zgNwkMgrMode == ZDNWKMGR_ENABLE )
  {
    ZDO_MgmtNwkUpdateNotify_t *pNotify = ZDO_ParseMgmtNwkUpdateNotify( inMsg ); 
    if ( pNotify )
    {
      ZDNwkMgr_CheckForChannelChange( pNotify );

      osal_mem_free( pNotify );
    }
  }
}

/*********************************************************************
 * @fn          ZDNwkMgr_CheckForChannelChange
 *
 * @brief       This function processes the incoming Management Network
 *              Update notify and starts an Update Request if a channel
 *              change is needed.
 *
 * @param       pUpdateNotify - notify message
 *
 * @return      none
 */
static void ZDNwkMgr_CheckForChannelChange( ZDO_MgmtNwkUpdateNotify_t *pNotify )
{
  uint8  i;
  uint16 failureRate;
  uint8  lowestEnergyIndex;
  uint8  lowestEnergyValue = 0xFF;
      
  // If any device has more than 50% transmission failures, a channel
  // change should be considered
  failureRate = ( pNotify->transmissionFailures * 100 ) / pNotify->totalTransmissions;
  if ( failureRate < ZDNWKMGR_CC_TX_FAILURE )
  {
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( (char*)NwkMgrStr_1, HAL_LCD_LINE_1 );
    HalLcdWriteStringValueValue( ": ", failureRate, 10, ZDNWKMGR_CC_TX_FAILURE, 10, HAL_LCD_LINE_2 );
#endif
    return;
  }

  // If the current failure rate is higher than the last failure rate,
  // a channel change should be considered
  if ( failureRate < ZDNwkMgr_LastChannelFailureRate )
  {
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( (char*)NwkMgrStr_2, HAL_LCD_LINE_1 );
    HalLcdWriteStringValueValue( ": ", failureRate, 10, 
                                 ZDNwkMgr_LastChannelFailureRate, 10, HAL_LCD_LINE_2 );
#endif
    return;
  }
  
  // Select a single channel based on the Mgmt_NWK_Update_notify based on
  // the lowest energy. This is the proposed new channel. 
  for ( i = 0; i < pNotify->listCount; i++ )
  {
    if ( pNotify->energyValues[i] < lowestEnergyValue )
    {
      lowestEnergyIndex = i;
      lowestEnergyValue = pNotify->energyValues[i];
    }
  }
      
  // If this new channel does not have an energy level below an acceptable
  // threshold, a channel change should not be done.
  if ( lowestEnergyValue > ZDNWKMGR_ACCEPTABLE_ENERGY_LEVEL )
  {
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( (char*)NwkMgrStr_3, HAL_LCD_LINE_1 );
    HalLcdWriteStringValueValue( ": ", lowestEnergyValue, 10, 
                                 ZDNWKMGR_ACCEPTABLE_ENERGY_LEVEL, 10, HAL_LCD_LINE_2 );
#endif
    return;
  }

  // Channel change should be done -- find out the new active channel
  for ( i = 0; i < ED_SCAN_MAXCHANNELS; i++ )
  {
    if ( ( (uint32)1 << i ) & pNotify->scannedChannels )
    {
      if ( lowestEnergyIndex == 0 )
        break;
      lowestEnergyIndex--;
    }
  }
  
  if ( ( _NIB.nwkLogicalChannel != i ) && ( ZDNwkMgr_UpdateRequestTimer == 0 ) )
  {
    uint32 channelMask;
    zAddrType_t dstAddr;
    
    // The new channel
    ZDNwkMgr_NewChannel = i;
        
    // Prior to changing channels, the network manager should store the 
    // energy scan value as the last energy scan value and the failure 
    // rate from the existing channel as the last failure rate.  These 
    // values are useful to allow comparison of the failure rate and energy
    // level on the previous channel to evaluate if the network is causing
    // its own interference.
    ZDNwkMgr_LastChannelEnergy = lowestEnergyValue;
    ZDNwkMgr_LastChannelFailureRate = failureRate;
       
    // The network manager should broadcast a Mgmt_NWK_Update_req notifying
    // devices of the new channel.  The broadcast shall be to all routers 
    // and coordinator.
    dstAddr.addrMode = AddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
    channelMask = (uint32)1 << i;
        
    // Increment the nwkUpdateId parameter and set the updateID in the beacon
    NLME_SetUpdateID(_NIB.nwkUpdateId + 1); 
    
    ZDP_MgmtNwkUpdateReq( &dstAddr, channelMask, 0xfe, 0, _NIB.nwkUpdateId, 0 );
        
    // The network manager shall set a timer based on the value of 
    // apsChannelTimer upon issue of a Mgmt_NWK_Update_req that changes 
    // channels and shall not issue another such command until this 
    // timer expires.  
    ZDNwkMgr_UpdateRequestTimer = ZDNWKMGR_UPDATE_REQUEST_TIMER;
    osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_UPDATE_REQUEST_EVT, ONE_MINUTE );
                  
    // Upon receipt of a Mgmt_NWK_Update_req with a change of channels, 
    // the local network manager shall set a timer equal to the 
    // nwkNetworkBroadcastDeliveryTime and shall switch channels upon 
    // expiration of this timer.  NOTE: since we won't recevied our own
    // broadcasted Update Request, we start the channel change timer here.  
    osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_CHANNEL_CHANGE_EVT, 
                        ZDNWKMGR_BCAST_DELIVERY_TIME );
  }
}
#endif  // NWK_MANAGER

/*********************************************************************
 * @fn          ZDNwkMgr_ProcessMgmtNwkUpdateReq
 *
 * @brief       This function processes the incoming Management
 *              Network Update request and starts the request (if needed).
 *
 * @param       Request message
 *
 * @return      none
 */
static void ZDNwkMgr_ProcessMgmtNwkUpdateReq( zdoIncomingMsg_t *inMsg )
{
  ZDO_MgmtNwkUpdateReq_t Req;
  
  ZDO_ParseMgmtNwkUpdateReq( inMsg, &Req );
   
  if ( Req.scanDuration <= 0x05 )
  {
    // Request is to scan over channelMask. The result will be reported by Confirm   
    if ( ( !inMsg->wasBroadcast )                     && 
         ( Req.scanCount >  ZDNWKMGR_MIN_SCAN_COUNT ) && 
         ( Req.scanCount <= ZDNWKMGR_MAX_SCAN_COUNT ) )
    {
      if ( NLME_EDScanRequest( Req.channelMask, Req.scanDuration ) == ZSuccess )
      {
        // Save off the information to be used for the notify
        ZDNwkMgr_MgmtNwkUpdateNotifyTransSeq            = inMsg->TransSeq;
        ZDNwkMgr_MgmtNwkUpdateNotifyAddr.addr.shortAddr = inMsg->srcAddr.addr.shortAddr;
        
        Req.scanCount--;
        
        // Save off scan info for the subsequent scans
        ZDNwkMgr_MgmtNwkUpdateReq = Req;
      }
    }
  }
  else if ( Req.scanDuration == 0xFE )
  {
    // Request is to change Channel. The command provide a new active
    // channel as a single channel in the channelMask.
    if ( Req.nwkUpdateId > _NIB.nwkUpdateId )
    {
      uint8 i;
      
      // Set update ID in the Beacon
      NLME_SetUpdateID(Req.nwkUpdateId); 
      
      // Find out the new active channel
      for ( i = 0; i < ED_SCAN_MAXCHANNELS; i++ )
      {
        if ( ( (uint32)1 << i ) & Req.channelMask )
        {
          break;
        }
      }

      if ( _NIB.nwkLogicalChannel != i )
      {
        ZDNwkMgr_NewChannel = i;
          
        // Upon receipt of a Mgmt_NWK_Update_req with a change of channels, 
        // the local network manager shall set a timer equal to the 
        // nwkNetworkBroadcastDeliveryTime and shall switch channels upon 
        // expiration of this timer.  Each node shall also increment the 
        // nwkUpdateId parameter and also reset the total transmit count 
        // and the transmit failure counters.  
        osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_CHANNEL_CHANGE_EVT, 
                            ZDNWKMGR_BCAST_DELIVERY_TIME );
      }
    }
  }
  else if ( Req.scanDuration == 0xFF )
  {
    // Request is to change apsChannelMask and nwkManagerAddr
    if ( Req.nwkUpdateId > _NIB.nwkUpdateId )
    {
      NLME_SetUpdateID(Req.nwkUpdateId); // Set the updateID in the beacon
       
      if ( ( Req.channelMask != 0 ) && ( _NIB.channelList != Req.channelMask ) )
      {
        _NIB.channelList = Req.channelMask;
      
        // Our Channel List has been changed -- notify to save info into NV
        ZDApp_NwkStateUpdateCB();
      }
    
      ZDNwkMgr_SetNwkManagerAddr( Req.nwkManagerAddr );
    }
  }
  else // 0x06-0xFD
  {
    // Request is invalid
    if ( !inMsg->wasBroadcast )
    {
      ZDNwkMgr_MgmtNwkUpdateNotifyAddr.addr.shortAddr = inMsg->srcAddr.addr.shortAddr;
      ZDP_MgmtNwkUpdateNotify( inMsg->TransSeq, &ZDNwkMgr_MgmtNwkUpdateNotifyAddr,
                               ZDP_INVALID_REQTYPE, 0, 0, 0, 0, NULL, AF_TX_OPTIONS_NONE, false );
    }
  }
}

/*********************************************************************
 * @fn      ZDNwkMgr_ProcessServerDiscRsp
 *
 * @brief   Process the incoming System Server Discovery Response
 *
 * @param   pRsp - Structure containing Server Discovery response
 *
 * @return  none
 */
void ZDNwkMgr_ProcessServerDiscRsp( zdoIncomingMsg_t *inMsg )
{
  ZDO_ServerDiscRsp_t Rsp;
  
  ZDO_ParseServerDiscRsp( inMsg, &Rsp );
  
  if ( Rsp.status == ZSuccess )
  {
    // Is the Network Manager bit set in the response?
    if ( Rsp.serverMask & NETWORK_MANAGER )
    {
      // Set the Remote Device's NWK Address as the Network Manager Address
      ZDNwkMgr_SetNwkManagerAddr( inMsg->srcAddr.addr.shortAddr );
    }
  }
}

/*********************************************************************
 * @fn          ZDNwkMgr_ProcessChannelInterference
 *
 * @brief       This function processes the incoming Channel Interference
 *              detection message and sends out a notify (if needed).
 *
 * @param       pChannelInterference - interference message
 *
 * @return      none
 */
static void ZDNwkMgr_ProcessChannelInterference( ZDNwkMgr_ChanInterference_t *pChanInterference )
{
  // To avoid a device with communication problems from constantly 
  // sending reports to the network manager, the device should not 
  // send a Mgmt_NWK_Update_notify more than 4 times per hour.
  if ( ZDNwkMgr_NumUpdateNotifySent < 4 )
  {
    // Conduct an energy scan on all channels.
    if ( NLME_EDScanRequest( MAX_CHANNELS_24GHZ, _NIB.scanDuration ) == ZSuccess )
    {
      // Save the counters for the Update Notify message to be sent
      ZDNwkMgr_TotalTransmissions = pChanInterference->totalTransmissions;
      ZDNwkMgr_TxFailures = pChanInterference->txFailures;

      // Mark scan as channel inetrference check
      ZDNwkMgr_MgmtNwkUpdateReq.scanCount = 0xFF;
    }
  }
}

/*********************************************************************
 * @fn          ZDNwkMgr_ProcessEDScanConfirm
 *
 * @brief       This function processes the incoming ED Scan Confirm
 *              message and sends out a notify (if needed).
 *
 * @param       pEDScanConfirm - SD Scan Confirmation message
 *
 * @return      none
 */
static void ZDNwkMgr_ProcessEDScanConfirm( ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm )
{ 
  if ( ZDNwkMgr_MgmtNwkUpdateReq.scanCount == 0xFF )
  {
    // Confirm to scan all channels for channel interference check
    ZDNwkMgr_CheckForChannelInterference( pEDScanConfirm ); 
    
    ZDNwkMgr_MgmtNwkUpdateReq.scanCount = 0;
  }
  else
  {
    // Confirm to the requested scan
    uint16 txFailures = nwkTransmissionFailures( FALSE );
    
    ZDNwkMgr_BuildAndSendUpdateNotify( ZDNwkMgr_MgmtNwkUpdateNotifyTransSeq,
                                       &ZDNwkMgr_MgmtNwkUpdateNotifyAddr, 
                                       _NIB.nwkTotalTransmissions, txFailures, 
                                       pEDScanConfirm, AF_TX_OPTIONS_NONE );
    // More scans needed?
    if ( ZDNwkMgr_MgmtNwkUpdateReq.scanCount > 0 )
    {
      osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_SCAN_REQUEST_EVT, 50 );
    }
  }
}

/*********************************************************************
 * @fn          ZDNwkMgr_CheckForChannelInterference
 *
 * @brief       This function processes the incoming ED Scan Confirm
 *              message and sends out an Update Notify (if needed).
 *
 * @param       pEDScanConfirm - SD Scan Confirmation message
 *
 * @return      none
 */
static void ZDNwkMgr_CheckForChannelInterference( ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm )
{
  uint8 i;
  uint8 channelEnergy = 0;
  uint8 energyIncreased = FALSE;
    
  // Get the current channel energy
  if ( ( (uint32)1 << _NIB.nwkLogicalChannel ) & pEDScanConfirm->scannedChannels )
  {
    channelEnergy = pEDScanConfirm->energyDetectList[_NIB.nwkLogicalChannel];
  }
    
  // If this energy scan does not indicate higher energy on the current 
  // channel then other channels, no action is taken. The device should 
  // continue to operate as normal and the message counters are not reset.
  for ( i = 0; i < ED_SCAN_MAXCHANNELS; i++ )
  {
    if ( ( ( (uint32)1 << i ) & pEDScanConfirm->scannedChannels ) && 
         ( channelEnergy > pEDScanConfirm->energyDetectList[i] ) )
    {
      energyIncreased = TRUE;
      break;
    }
  }
    
  // If the energy scan does indicate increased energy on the channel
  // in use, a Mgmt_NWK_Update_notify should be sent to the Network 
  // Manager to indicate interference is present.
  if ( energyIncreased )
  {
    // Send a Management Network Update notify to the Network Manager
    ZDNwkMgr_MgmtNwkUpdateNotifyAddr.addr.shortAddr = _NIB.nwkManagerAddr;
    ZDNwkMgr_BuildAndSendUpdateNotify( 0, &ZDNwkMgr_MgmtNwkUpdateNotifyAddr, 
                                       ZDNwkMgr_TotalTransmissions, ZDNwkMgr_TxFailures,
                                       pEDScanConfirm, AF_MSG_ACK_REQUEST );
    ZDNwkMgr_WaitingForNotifyConfirm = TRUE; // Confirm will clear the counters
      
    if ( ZDNwkMgr_NumUpdateNotifySent == 0 )
    {
      // First notify message sent within this hour. Start the Update Notify timer.
      ZDNwkMgr_UpdateNotifyTimer = ZDNWKMGR_UPDATE_NOTIFY_TIMER;
      osal_start_timerEx( ZDNwkMgr_TaskID, ZDNWKMGR_UPDATE_NOTIFY_EVT, ONE_MINUTE );
    }
    
    ZDNwkMgr_NumUpdateNotifySent++;
  }
#if defined ( LCD_SUPPORTED )
  else
  {
    HalLcdWriteString( (char*)NwkMgrStr_4, HAL_LCD_LINE_1 );
    HalLcdWriteStringValueValue( ": ", _NIB.nwkLogicalChannel, 10, channelEnergy, 10, HAL_LCD_LINE_2 );
  }
#endif
}

/*********************************************************************
 * @fn          ZDNwkMgr_BuildAndSendUpdateNotify
 *
 * @brief       This builds and send a Mgmt_NWK_Update_notify message. This
 *              function sends a unicast message.
 *
 * @param       TransSeq - transaction sequence number
 * @param       dstAddr - destination address of the message
 * @param       pEDScanConfirm - update notify info
 *
 * @return      afStatus_t
 */
static void ZDNwkMgr_BuildAndSendUpdateNotify( uint8 TransSeq, zAddrType_t *dstAddr,
                                               uint16 totalTransmissions, uint16 txFailures,
                                               ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm,
                                               uint8 txOptions )
{
  uint8 i;
  uint8 listCount = 0;
  uint8 *energyValues = NULL;
  
  // Count number of energy detects
  for ( i = 0; i < ED_SCAN_MAXCHANNELS; i++ )
  {
    if ( ( (uint32)1 << i ) & pEDScanConfirm->scannedChannels )
      listCount++;
  }
  
  if ( listCount > 0 )
  {
    energyValues = (uint8 *)osal_mem_alloc( listCount );
    if ( energyValues )
    {
      uint8 j = 0;

      for ( i = 0; i < ED_SCAN_MAXCHANNELS; i++ )
      {
        if ( ( (uint32)1 << i ) & pEDScanConfirm->scannedChannels )
          energyValues[j++] = pEDScanConfirm->energyDetectList[i];
      }
    }
  }
    
  // Send a Management Network Update notify back
  ZDP_MgmtNwkUpdateNotify( TransSeq, dstAddr, pEDScanConfirm->status, 
                           pEDScanConfirm->scannedChannels,
                           totalTransmissions, txFailures,
                           listCount, energyValues, txOptions, false );
  if ( energyValues )
    osal_mem_free( energyValues );
}

#if defined ( NWK_MANAGER )
/*********************************************************************
 * @fn      NwkMgr_SetNwkManager
 *
 * @brief   Set the local device as the Network Manager
 *
 * @param   none
 *
 * @return  none
 */
void NwkMgr_SetNwkManager( void )
{
  if ( zgNwkMgrMode == ZDNWKMGR_ENABLE )
  {
    // We're the Network Manager. Set our address as the Network Manager Address
    ZDNwkMgr_SetNwkManagerAddr( _NIB.nwkDevAddress );
    
    // Set the Network Manager bit of the Server Mask
    ZDO_Config_Node_Descriptor.ServerMask |= NETWORK_MANAGER;
  }
}
#endif // NWK_MANAGER

/*********************************************************************
 * @fn      ZDApp_SetNwkManagerAddr()
 *
 * @brief   Sets the nwkManagerAddr in NIB.
 *
 * @param   nwkManagerAddr
 *
 * @return  none
 */
void ZDNwkMgr_SetNwkManagerAddr( uint16 nwkManagerAddr )
{
  if ( _NIB.nwkManagerAddr != nwkManagerAddr )
  {
    // Update the Network Manager Address
    _NIB.nwkManagerAddr = nwkManagerAddr;
  
    // Our Network Manger Address has been changed -- notify to save info into NV
    ZDApp_NwkStateUpdateCB();
  }
}

/*********************************************************************
 * @fn          ZDNwkMgr_ReportChannelInterference
 *
 * @brief       This function builds a Channel Interference detection
 *              message and then forwards it to the Network Manager.
 *
 * @param       chanInterference
 *
 * @return      none
 */
void ZDNwkMgr_ReportChannelInterference(  NLME_ChanInterference_t *chanInterference  )
{
  ZDNwkMgr_ChanInterference_t *pChanInterference;

  // Send Channel Interference message to the Network Manager task
  pChanInterference = (ZDNwkMgr_ChanInterference_t *)osal_msg_allocate( sizeof( ZDNwkMgr_ChanInterference_t ) );
  if ( pChanInterference )
  {
    pChanInterference->hdr.event = NM_CHANNEL_INTERFERE;
      
    // Build the structure
    pChanInterference->totalTransmissions = chanInterference->totalTransmissions;
    pChanInterference->txFailures = chanInterference->txFailures;
              
    osal_msg_send( ZDNwkMgr_TaskID, (uint8 *)pChanInterference );
  }
}

/*********************************************************************
 * @fn          ZDNwkMgr_EDScanConfirmCB
 *
 * @brief       Handle Energy Scan confirm callback
 *
 * @param       scannedChannels  - scanned channels
 * @param       energyDetectList - measured energy for channels
 *
 * @return      none
 */
void ZDNwkMgr_EDScanConfirmCB( NLME_EDScanConfirm_t *EDScanConfirm )
{
  ZDNwkMgr_EDScanConfirm_t *pEDScanConfirm;

  // Send ED Confirm to the Network Manager task
  pEDScanConfirm = (ZDNwkMgr_EDScanConfirm_t *)osal_msg_allocate( sizeof( ZDNwkMgr_EDScanConfirm_t ) );
  if ( pEDScanConfirm )
  {
    pEDScanConfirm->hdr.event = NM_ED_SCAN_CONFIRM;
      
    // Build the structure
    pEDScanConfirm->status = EDScanConfirm->status;
    pEDScanConfirm->scannedChannels = EDScanConfirm->scannedChannels;
    osal_memcpy( pEDScanConfirm->energyDetectList, EDScanConfirm->energyDetectList, ED_SCAN_MAXCHANNELS );
      
    osal_msg_send( ZDNwkMgr_TaskID, (uint8 *)pEDScanConfirm );
  }
}

/*********************************************************************
 * @fn      ZDNwkMgr_ProcessDataConfirm
 *
 * @brief   Process received Confirmation for Mgmt NWK Update Notify message
 *
 * @param   none
 *
 * @return  none
 */
void ZDNwkMgr_ProcessDataConfirm( afDataConfirm_t *afDataConfirm )
{
  if (   ZDNwkMgr_WaitingForNotifyConfirm  && 
       ( afDataConfirm->transID == 0 )     && 
       ( afDataConfirm->hdr.status == ZSuccess ) )
  {
    // The Mgmt NWK Update Notify was sent as an APS Unicast with  
    // acknowledgement and once the acknowledgment is received the 
    // total transmit and transmit failure counters are reset to zero.  
    _NIB.nwkTotalTransmissions = 0;
    nwkTransmissionFailures( TRUE );
    
    ZDNwkMgr_WaitingForNotifyConfirm = FALSE;
  }
}

/*********************************************************************
 * PAN ID Conflict Routines
 */
#if defined ( NWK_MANAGER )
/*********************************************************************
 * @fn          ZDNwkMgr_NetworkReportCB
 *
 * @brief       Handle the Network Report Command
 *
 * @param       srcAddr     - Source Address of the message.
 * @param       status      - ZSuccess.
 * @param       serverMask  - Bit mask of services matching the req serverMask.
 * @param       securityUse -
 *
 * @return      none
 */
void ZDNwkMgr_NetworkReportCB( ZDNwkMgr_NetworkReport_t *pReport )
{ 
  // Send Network Report message to the Network Manager task
  osal_msg_send( ZDNwkMgr_TaskID, (uint8 *)pReport );
}

/*********************************************************************
 * @fn          ZDNwkMgr_NetworkUpdateCB
 *
 * @brief       Handle the Network Update Command
 *
 * @param       srcAddr     - Source Address of the message.
 * @param       status      - ZSuccess.
 * @param       serverMask  - Bit mask of services matching the req serverMask.
 * @param       securityUse -
 *
 * @return      none
 */
void ZDNwkMgr_NetworkUpdateCB( ZDNwkMgr_NetworkUpdate_t *pUpdate )
{
  // Send Network Update message to the Network Manager task
  osal_msg_send( ZDNwkMgr_TaskID, (uint8 *)pUpdate );
}

/*********************************************************************
 * @fn      ZDNwkMgr_ProcessNetworkReport
 *
 * @brief   Process the incoming Network Report message
 *
 * @param   pNetworkReport - Structure containing Network Report message
 *
 * @return  none
 */
void ZDNwkMgr_ProcessNetworkReport( ZDNwkMgr_NetworkReport_t *pNetworkReport )
{
  uint8 i;
  uint16 newPID;
  uint8 unique = TRUE;

  if ( pNetworkReport->reportType == NWKREPORT_PANID_CONFLICT )
  {
    if ( ZDNwkMgr_PanIdUpdateInProgress == FALSE )
    {
      do
      {
        // select a new PAN ID
        newPID = (uint16)osal_rand();
      
        // Make sure that the chosen PAN ID is not already in use in the
        // local neighborhood and also not contained within the Report 
        // Information field of the Network Report Command frame
        for ( i = 0; i < pNetworkReport->reportInfoCnt; i++ )
        {
          if ( pNetworkReport->panIDs[i] == newPID )
          {
            unique = FALSE;
            break;
          }
        }
      } while ( !unique );
         
      // Send out a Network Update command.
      NLME_SendNetworkUpdate( NWK_BROADCAST_SHORTADDR, NWKUPDATE_PANID_UPDATE,
                              _NIB.extendedPANID, _NIB.nwkUpdateId+1, newPID );
    
      ZDNwkMgr_PanIdUpdateInProgress = TRUE;
    }
  }
}

/*********************************************************************
 * @fn      ZDNwkMgr_ProcessNetworkUpdate
 *
 * @brief   Process the incoming Network Update message
 *
 * @param   pNetworkReport - Structure containing Network Update message
 *
 * @return  none
 */
void ZDNwkMgr_ProcessNetworkUpdate( ZDNwkMgr_NetworkUpdate_t *pNetworkUpdate )
{
  if ( pNetworkUpdate->updateType == NWKUPDATE_PANID_UPDATE )
  { 
    // Our PAN ID has been changed -- notify to save info into NV
    ZDApp_NwkStateUpdateCB();
    
    ZDNwkMgr_PanIdUpdateInProgress = FALSE;
  }
}
#endif // NWK_MANAGER


/*********************************************************************
*********************************************************************/
