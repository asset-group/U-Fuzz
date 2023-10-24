/**************************************************************************************************
  Filename:       stub_aps.c
  Revised:        $Date: 2014-03-26 10:01:05 -0700 (Wed, 26 Mar 2014) $
  Revision:       $Revision: 37899 $

  Description:    Stub APS processing functions


  Copyright 2008 - 2014 Texas Instruments Incorporated. All rights reserved.

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
#include "OSAL.h"
#include "mac_spec.h"
#include "nwk_util.h"
#include "AF.h"

#include "stub_aps.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Stub NWK header length
#define STUB_NWK_HDR_LEN                2

// Start of the Stub APS header in the Inter-PAN frame
#define STUB_APS_HDR_FRAME_CTRL         STUB_NWK_HDR_LEN

// Stub APS event identifiers
#define CHANNEL_CHANGE_EVT              0x0001

#define CHANNEL_CHANGE_RETRY_TIMEOUT    100

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  zAddrType_t addr;
  uint16 panId;
} pan_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8 StubAPS_TaskID = 0xFF;    // Task ID for internal task/event processing

/*********************************************************************
 * EXTERNAL VARIABLES
 */


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 newChannel;
static uint8 channelChangeInProgress = FALSE;

// Application info
static uint8 appTaskID = 0xFF;  // Application task id
uint8 appEndPoint = 0;   // Application endpoint


/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void StubNWK_ParseMsg( uint8 *buf, uint8 bufLength, NLDE_FrameFormat_t *snff );
static void StubAPS_ParseMsg( NLDE_FrameFormat_t *snff, aps_FrameFormat_t *saff );
static void StubNWK_BuildMsg( uint8 *nwkHdr );
static void StubAPS_BuildMsg( uint8 *apsHdr, uint8 frmCtrl, uint16 groupID, APSDE_DataReq_t *req );
static ZStatus_t StubAPS_BuildFrameControl( uint8 *frmCtrl, zAddrType_t *dstAddr,
                                            uint16 *groupID, APSDE_DataReq_t *req );
static ZStatus_t StubAPS_SetNewChannel( uint8 channel );
static void StubAPS_NotifyApp( uint8 status );

uint8 StubAPS_ZMacCallback( uint8 *msgPtr );

/*********************************************************************
 * @fn      StubAPS_Init()
 *
 * @brief   Initialize stub APS layer
 *
 * @param   task_id - Task identifier for the desired task
 *
 * @return  none
 */
void StubAPS_Init( uint8 task_id )
{
  StubAPS_TaskID = task_id;

  // register with ZMAC
  pZMac_AppCallback = StubAPS_ZMacCallback;

} /* StubAPS_Init() */

/*********************************************************************
 * @fn      StubAPS_ProcessEvent()
 *
 * @brief   Main event loop for Stub APS task. This function should be called
 *          at periodic intervals when event occur.
 *
 * @param   task_id - Task ID
 * @param   events  - Bitmap of events
 *
 * @return  none
 */
UINT16 StubAPS_ProcessEvent( uint8 task_id, uint16 events )
{
  (void)task_id; // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    osal_event_hdr_t *msg_ptr;

    while ( (msg_ptr = (osal_event_hdr_t *)osal_msg_receive( StubAPS_TaskID )) != NULL )
    {
      if ( msg_ptr->event == MAC_MCPS_DATA_CNF )
      {
        INTERP_DataConfirm( (ZMacDataCnf_t *)msg_ptr );
      }
      else if ( msg_ptr->event == MAC_MCPS_DATA_IND )
      {
        INTERP_DataIndication( (macMcpsDataInd_t *)msg_ptr );
      }

      osal_msg_deallocate( (uint8 *)msg_ptr );
    }

    // Return unproccessed events
    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & CHANNEL_CHANGE_EVT )
  {
    // try to change to the new channel
    ZStatus_t status = StubAPS_SetNewChannel( newChannel );
    if ( status != ZSuccess )
    {
      // turn MAC receiver back on
      uint8 rxOnIdle = true;
      ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

      // set NWK task to run
      nwk_setStateIdle( FALSE );

      channelChangeInProgress = FALSE;
    }

    // notify the application
    StubAPS_NotifyApp( status );

    return ( events ^ CHANNEL_CHANGE_EVT );
  }

  // If reach here, the events are unknown
  // Discard or make more handlers
  return 0;

} /* StubAPS_ProcessEvent() */


/*********************************************************************
 * @fn          StubNWK_ParseMsg
 *
 * @brief       Call this function to parse an incoming Stub NWK frame.
 *
 * @param       buf - pointer incoming message buffer
 * @param       bufLength - length of incoming message
 * @param       snff  - pointer Frame Format Parameters
 *
 * @return      pointer to network packet, NULL if error
 */
static void StubNWK_ParseMsg( uint8 *buf, uint8 bufLength, NLDE_FrameFormat_t *snff )
{
  uint16 fc;

  osal_memset( snff, 0, sizeof(NLDE_FrameFormat_t) );

  snff->bufLength = bufLength;

  // get the frame control
  fc = BUILD_UINT16( buf[NWK_HDR_FRAME_CTRL_LSB], buf[NWK_HDR_FRAME_CTRL_MSB] );

  // parse the frame control
  NLDE_ParseFrameControl( fc, snff );

  snff->hdrLen = STUB_NWK_HDR_LEN;

  // Stub NWK payload
  snff->nsdu = buf + snff->hdrLen;
  snff->nsduLength = snff->bufLength - snff->hdrLen;

} /* StubNWK_ParseMsg */

/*********************************************************************
 * @fn          StubAPS_ParseMsg
 *
 * @brief       Call this function to parse an incoming Stub APS frame.
 *
 * @param       naff  - pointer Stub NWK Frame Format Parameters
 * @param       saff  - pointer Stub APS Format Parameters
 *
 * @return      none
 */
static void StubAPS_ParseMsg( NLDE_FrameFormat_t *snff, aps_FrameFormat_t *saff )
{
  uint8 fcb;
  uint8 *asdu;

  osal_memset( saff, 0, sizeof(aps_FrameFormat_t) );

  saff->asduLength = snff->nsduLength;
  asdu = snff->nsdu;
  saff->macDestAddr = snff->macDstAddr;

  // First byte is Frame Control.
  saff->FrmCtrl = *asdu++;

  fcb = saff->FrmCtrl & APS_FRAME_TYPE_MASK;
  if ( fcb == STUB_APS_FRAME )
  {
    fcb = saff->FrmCtrl & APS_DELIVERYMODE_MASK;
    if ( fcb == APS_FC_DM_BROADCAST )
      saff->wasBroadcast = true;
    else
      saff->wasBroadcast = false;

    if ( fcb == APS_FC_DM_GROUP )
    {
      saff->GroupID = BUILD_UINT16( asdu[0], asdu[1] );
      asdu += sizeof( uint16 );
    }

    // Pull out the Cluster ID
    saff->ClusterID = BUILD_UINT16( asdu[0], asdu[1] );
    asdu += sizeof( uint16 );

    // Pull out the profile ID
    saff->ProfileID = BUILD_UINT16( asdu[0], asdu[1] );
    asdu += 2;
  }

  saff->asdu = asdu;
  saff->asduLength -= (uint8) (asdu - snff->nsdu);
  saff->apsHdrLen = snff->nsduLength - saff->asduLength;

} /* StubAPS_ParseMsg */

/******************************************************************************
 * @fn          StubAPS_BuildFrameControl
 *
 * @brief       This function builds Stub APS Frame Control and the destination
 *              address parameter for the MCPS-DATA Request.
 *
 * @param       frmCtrl - frame control
 * @param       dstAddr - destination address for MCPS-DATA Request
 * @param       groupID - group id
 * @param       req - APSDE_DataReq_t
 *
 * @return      ZStatus_t
 */
static ZStatus_t StubAPS_BuildFrameControl( uint8 *frmCtrl, zAddrType_t *dstAddr,
                                            uint16 *groupID, APSDE_DataReq_t *req )
{
  // Security
  if ( req->txOptions & APS_TX_OPTIONS_SECURITY_ENABLE )
    return ( ZApsNotSupported );

  // Ack request
  if ( req->txOptions & APS_TX_OPTIONS_ACK )
    return ( ZApsNotSupported );

   // Fragmentation
  if ( req->txOptions & APS_TX_OPTIONS_PERMIT_FRAGMENT )
    return ( ZApsNotSupported );

  // set delivery mode
  if ( req->dstAddr.addrMode == AddrNotPresent )
    return ( ZApsNotSupported ); // No REFLECTOR

  // set frame type
  *frmCtrl = STUB_APS_FRAME;

  // set DstAddrMode of MCPS-DATA Request to DstAddrMode of INTERP-Data Request
  dstAddr->addrMode = req->dstAddr.addrMode;

  // set DstAddr of MCPS-DATA Request to DstAddr of INTERP-Data Request
  if ( req->dstAddr.addrMode == AddrBroadcast )
  {
    *frmCtrl |= APS_FC_DM_BROADCAST;

    // set DstAddrMode of MCPS-DATA Request to short address
    dstAddr->addrMode = Addr16Bit;
    dstAddr->addr.shortAddr = req->dstAddr.addr.shortAddr;
  }
  else if ( req->dstAddr.addrMode == Addr16Bit )
  {
    *frmCtrl |= APS_FC_DM_UNICAST;
    dstAddr->addr.shortAddr = req->dstAddr.addr.shortAddr;
  }
  else if ( req->dstAddr.addrMode == Addr64Bit )
  {
    *frmCtrl |= APS_FC_DM_UNICAST;
    osal_cpyExtAddr( dstAddr->addr.extAddr, req->dstAddr.addr.extAddr );
  }
  else if ( req->dstAddr.addrMode == AddrGroup )
  {
    *frmCtrl |= APS_FC_DM_GROUP;

    // set DstAddrMode of MCPS-DATA Request to short address
    dstAddr->addrMode = Addr16Bit;

    // set DstAddr of MCPS-DATA Request to 0xFFFF
    dstAddr->addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVALL;

    // set Group ID to DstAddr of INTERP-Data Request
    *groupID = req->dstAddr.addr.shortAddr;
  }

  return ( ZSuccess );

} /* StubAPS_BuildFrameControl */

/******************************************************************************
 * @fn          StubNWK_BuildMsg
 *
 * @brief       This function builds a Stub NWK frame.
 *
 * @param       nwkHdr - stub NWK header
 *
 * @return      none
 */
static void StubNWK_BuildMsg( uint8 *nwkHdr )
{
  uint16 frmCtrl = 0;
  uint8  protoVer = NLME_GetProtocolVersion();

  // frame type
  frmCtrl |= (STUB_NWK_FRAME_TYPE << NWK_FC_FRAME_TYPE);

  // protocol version
  frmCtrl |= (protoVer << NWK_FC_PROT_VERSION);

  // set Stub NWK header
  *nwkHdr++ = LO_UINT16( frmCtrl );
  *nwkHdr++ = HI_UINT16( frmCtrl );

} /* StubNWK_BuildMsg */

/******************************************************************************
 * @fn          StubAPS_BuildMsg
 *
 * @brief       This function builds a Stub APS frame.
 *
 * @param       apsHdr - stub APS header
 * @param       frmCtrl - stub APS frame control
 * @param       groupID - group id
 * @param       req - APSDE_DataReq_t
 *
 * @return      none
 */
static void StubAPS_BuildMsg( uint8 *apsHdr, uint8 frmCtrl, uint16 groupID, APSDE_DataReq_t *req )
{
  // add frame type
  *apsHdr++ = frmCtrl;

  // add Group ID
  if ( ( frmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_GROUP )
  {
    *apsHdr++ = LO_UINT16( groupID );
    *apsHdr++ = HI_UINT16( groupID );
  }

  // add clusterID
  *apsHdr++ = LO_UINT16( req->clusterID );
  *apsHdr++ = HI_UINT16( req->clusterID );

  // add profile ID
  *apsHdr++ = LO_UINT16( req->profileID );
  *apsHdr++ = HI_UINT16( req->profileID );

  // copy ASDU data into frame
  osal_memcpy ( apsHdr, req->asdu, req->asduLen );

} /* StubAPS_BuildMsg */

/******************************************************************************
 * @fn          StubAPS_setNewChannel
 *
 * @brief       This function changes the device's channel.
 *
 * @param       none
 *
 * @return      ZStatus_t
 */
static ZStatus_t StubAPS_SetNewChannel( uint8 channel )
{
  uint8 rxOnIdle;

  // make sure MAC has nothing to transmit
  if ( ( nwkDB_CountTypes( NWK_DATABUF_SENT ) == 0 ) && ZMacStateIdle() )
  {
    // set the new channel
    ZMacSetReq( ZMacChannel, &channel );

    // turn MAC receiver back on
    rxOnIdle = true;
    ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

    channelChangeInProgress = FALSE;

    return ( ZSuccess );
  }

  return ( ZFailure );

} /* StubAPS_setNewChannel */


/******************************************************************************
 * @fn          StubAPS_NotifyApp
 *
 * @brief       This function sends an OSAL message to the Application task.
 *
 * @param       status - command status
 *
 * @return      none
 */
static void StubAPS_NotifyApp( uint8 status )
{
  osal_event_hdr_t *msgPtr;

  // Notify the application task
  msgPtr = (osal_event_hdr_t *)osal_msg_allocate( sizeof(osal_event_hdr_t) );
  if ( msgPtr )
  {
    msgPtr->event = SAPS_CHANNEL_CHANGE;
    msgPtr->status = status;

    osal_msg_send( appTaskID, (uint8 *)msgPtr );
  }

} /* StubAPS_NotifyApp */

/******************************************************************************
 *
 *  External APIs provided to the Application.
 */

/******************************************************************************
 * @fn          StubAPS_SetInterPanChannel
 *
 * @brief       This function changes the device's channel for inter-PAN communication.
 *
 * @param       channel - new channel
 *
 * @return      ZStatus_t
 */
ZStatus_t StubAPS_SetInterPanChannel( uint8 channel )
{
  uint8 currChannel;
  uint8 rxOnIdle;

  if ( channelChangeInProgress )
    return ( ZFailure );

  ZMacGetReq( ZMacChannel, &currChannel );
  if ( currChannel == channel )
  {
    // inter PANs communication within the same channel
    return ( ZSuccess );
  }

  // go into channel transition state
  channelChangeInProgress = TRUE;

  // set NWK task to idle
  nwk_setStateIdle( TRUE );

  // turn MAC receiver off
  rxOnIdle = false;
  ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

  // try to change to the new channel
  if ( StubAPS_SetNewChannel( channel ) == ZSuccess )
    return ( ZSuccess );

  // save the new channel for retry
  newChannel = channel;

  // ask StubAPS task to retry it later
  osal_start_timerEx( StubAPS_TaskID, CHANNEL_CHANGE_EVT, CHANNEL_CHANGE_RETRY_TIMEOUT );

  return ( ZApsNotAllowed );

} /* StubAPS_SetInterPanChannel */

/******************************************************************************
 * @fn          StubAPS_SetIntraPanChannel
 *
 * @brief       This function sets the device's channel back to the NIB channel.
 *
 * @param       none
 *
 * @return      ZStatus_t
 */
ZStatus_t StubAPS_SetIntraPanChannel( void )
{
  uint8 currChannel;
  uint8 rxOnIdle;

  if ( channelChangeInProgress )
    return ( ZFailure );

  ZMacGetReq( ZMacChannel, &currChannel );
  if ( currChannel == _NIB.nwkLogicalChannel )
    return ( ZSuccess );

  channelChangeInProgress = TRUE;

  // turn MAC receiver off
  rxOnIdle = false;
  ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

  // set the NIB channel
  ZMacSetReq( ZMacChannel, &(_NIB.nwkLogicalChannel) );

  // turn MAC receiver back on
  rxOnIdle = true;
  ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

  // set NWK task to run
  nwk_setStateIdle( FALSE );

  channelChangeInProgress = FALSE;

  return ( ZSuccess );

} /* StubAPS_SetIntraPanChannel */

/******************************************************************************
 * @fn          StubAPS_InterPan
 *
 * @brief       This function checks to see if a PAN is an Inter-PAN.
 *
 * @param       panId - PAN ID
 * @param       endPoint - endpoint
 *
 * @return      TRUE if PAN is Inter-PAN, FALSE otherwise
 */
uint8 StubAPS_InterPan( uint16 panId, uint8 endPoint )
{
  (void)panId; // Intentionally unreferenced parameter

  // No need to check the MAC/NIB Channels or Source/Destination PAN IDs
  // since it's possible to send Inter-PAN messages within the same network.
  if ( endPoint == STUBAPS_INTER_PAN_EP )
  {
    // Inter-PAN endpoint
    return ( TRUE );
  }

  return ( FALSE );

} /* StubAPS_InterPan */

/******************************************************************************
 * @fn          StubAPS_RegisterApp
 *
 * @brief       This function registers the Application with the Stub APS layer.
 *
 *              NOTE: Since Stub APS messages don't include the application
 *                    endpoint, the application has to register its endpoint
 *                    with Stub APS.
 *
 * @param       epDesc - application's endpoint descriptor
 *
 * @return      none
 */
void StubAPS_RegisterApp( endPointDesc_t *epDesc )
{
  appTaskID = *epDesc->task_id;
  appEndPoint = epDesc->endPoint;

} /* StubAPS_RegisterApp */

/******************************************************************************
 * @fn          StubAPS_ZMacCallback
 *
 * @brief       This function accepts an inter-PAN message from ZMac.
 *
 * @param       msgPtr - received message
 *
 * @return      TRUE if message is processed. FALSE otherwise.
 */
uint8 StubAPS_ZMacCallback( uint8 *msgPtr )
{
  uint16 nwk_fc;
  uint8  aps_fc;
  uint8  frameType;
  uint8 *buf = NULL;
  uint8  event = ((osal_event_hdr_t *)msgPtr)->event;

  if ( event == MAC_MCPS_DATA_IND )
  {
    buf = ((macMcpsDataInd_t *)msgPtr)->msdu.p;
  }
  else if ( event == MAC_MCPS_DATA_CNF )
  {
    buf = ((macMcpsDataCnf_t *)msgPtr)->pDataReq->msdu.p;
  }

  if ( buf )
  {
    // get the NWK frame control
    nwk_fc = BUILD_UINT16( buf[NWK_HDR_FRAME_CTRL_LSB], buf[NWK_HDR_FRAME_CTRL_MSB] );

    // frame type
    frameType = (uint8)((nwk_fc >> NWK_FC_FRAME_TYPE) & NWK_FC_FRAME_TYPE_MASK);

    // check if incoming frame is of the right type
    if ( frameType != STUB_NWK_FRAME_TYPE )
    {
      // message doesn't belong to Stub APS
      return ( FALSE );
    }

    // get the APS frame control
    aps_fc = buf[STUB_APS_HDR_FRAME_CTRL];

    // frame type
    frameType = aps_fc & APS_FRAME_TYPE_MASK;

    // check if incoming frame is of the right type
    if ( frameType != STUB_APS_FRAME )
    {
      // message doesn't belong to Stub APS
      return ( FALSE );
    }

    // message belongs to Stub APS
    osal_msg_send( StubAPS_TaskID, (uint8 *)msgPtr );

    return ( TRUE );
  }

  // message doesn't belong to Stub APS
  return ( FALSE );

} /* StubAPS_ZMacCallback */

/******************************************************************************
 *
 *  Stub APS Inter-PAN interface INTERP and its callbacks.
 */

/******************************************************************************
 * @fn          INTERP_DataReq
 *
 * @brief       This function requests the transfer of data from the next
 *              higher layer to a single peer entity.
 *
 * @param       req - APSDE_DataReq_t
 *
 * @return      ZStatus_t
 */
ZStatus_t INTERP_DataReq( APSDE_DataReq_t *req )
{
  uint8 apsFrmCtrl;
  uint16 groupID = 0;
  uint8 *buf;
  uint8 hdrLen;
  ZMacDataReq_t dataReq;
  ZStatus_t status;

  if ( channelChangeInProgress || !StubAPS_InterPan( req->dstPanId, req->dstEP ) )
    return ( ZFailure );

  osal_memset( &dataReq, 0, sizeof( ZMacDataReq_t ) );

  // Build Stub APS header
  status = StubAPS_BuildFrameControl( &apsFrmCtrl, &(dataReq.DstAddr), &groupID, req );
  if ( status != ZSuccess )
    return ( status );

  // set default Stub APS header length
  hdrLen = APS_FRAME_CTRL_FIELD_LEN;

  // add group ID length
  if ( ( apsFrmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_GROUP )
    hdrLen += APS_GROUP_ID_FIELD_LEN;

  // add cluster ID length
  hdrLen += APS_CLUSTERID_FIELD_LEN;

  // add profile ID length
  hdrLen += APS_PROFILEID_FIELD_LEN;

  // add default Stub NWK header length
  hdrLen += STUB_NWK_HDR_LEN;

  // calculate MSDU length
  dataReq.msduLength = hdrLen + req->asduLen;

  // allocate buffer
  buf = osal_mem_alloc( dataReq.msduLength );
  if ( buf != NULL )
  {
    dataReq.msdu = buf;

    // Add Stub APS header and data
    StubAPS_BuildMsg( &buf[STUB_APS_HDR_FRAME_CTRL], apsFrmCtrl, groupID, req );

    // Add Stub NWK header
    StubNWK_BuildMsg( buf );

    // Set ZMac data request
    dataReq.DstPANId = req->dstPanId;
    dataReq.SrcAddrMode = Addr64Bit;
    dataReq.Handle = req->transID;

    if ( ( apsFrmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_UNICAST )
      dataReq.TxOptions = NWK_TXOPTIONS_ACK;
    else
      dataReq.TxOptions = 0;

    // send the frame
    status = ZMacDataReq( &dataReq );

    // free the frame
    osal_mem_free( buf );
  }
  else
  {
    // flag a memory error
    status = ZMemError;
  }

  return ( status );

} /* INTERP_DataReq */

/******************************************************************************
 * @fn          INTERP_DataReqMTU
 *
 * @brief       This function requests the MTU (Max Transport Unit) of the
 *              Inter-PAN Data Service.
 *
 * @param       none
 *
 * @return      uint8 - MTU
 */
uint8 INTERP_DataReqMTU( void )
{
  uint8 mtu;
  uint8 hdrLen;

  // Use maximum header size for Stub APS header
  hdrLen = APS_FRAME_CTRL_FIELD_LEN +
           APS_GROUP_ID_FIELD_LEN   +
           APS_CLUSTERID_FIELD_LEN  +
           APS_PROFILEID_FIELD_LEN;

  mtu = MAC_A_MAX_FRAME_SIZE - STUB_NWK_HDR_LEN - hdrLen;

  return ( mtu );

} /* INTERP_DataReqMTU */

/****************************************************************************
 * @fn          INTERP_DataConfirm
 *
 * @brief       This function processes the data confirm from the MAC layer.
 *
 * @param       dataCnf - data confirm primitive
 *
 * @return      none
 */
void INTERP_DataConfirm( ZMacDataCnf_t *dataCnf )
{
  afDataConfirm( appEndPoint, dataCnf->msduHandle, dataCnf->hdr.Status );

} /* INTERP_DataConfirm */

/****************************************************************************
 * @fn          INTERP_DataIndication
 *
 * @brief       This function indicates the transfer of a data SPDU (MSDU)
 *              from the MAC layer to the local application layer entity.
 *
 * @param       dataInd - data indicate primitive
 *
 * @return      none
 */
void INTERP_DataIndication( macMcpsDataInd_t *dataInd )
{
  NLDE_FrameFormat_t snff;
  aps_FrameFormat_t saff;
  zAddrType_t srcAddr;
  NLDE_Signal_t sig;

  // parse the Stub NWK header
  StubNWK_ParseMsg( dataInd->msdu.p, dataInd->msdu.len, &snff );

  // Fill in MAC destination address
  snff.macDstAddr = dataInd->mac.dstAddr.addr.shortAddr;

  // fill in MAC source address (Stub NWK frame doesn't have address fields)
  osal_copyAddress( &srcAddr, (zAddrType_t *)&(dataInd->mac.srcAddr) );

  // check if incoming frame is of the right type
  if ( snff.frameType != STUB_NWK_FRAME_TYPE )
    return;

  // check if incoming frame is of the right version
  if ( snff.protocolVersion != NLME_GetProtocolVersion() )
    return;

  // check if the remaining sun-fields are zero
  if ( ( snff.discoverRoute != 0 ) || ( snff.multicast != 0 )   ||
       ( snff.secure != 0 )        || ( snff.srcRouteSet != 0 ) ||
       ( snff.dstExtAddrSet != 0 ) || ( snff.srcExtAddrSet != 0 ) )
  {
    return;
  }

  // parse the Stub APS header
  StubAPS_ParseMsg( &snff, &saff );

  // check if incoming frame is of the right type
  if ( ( saff.FrmCtrl & APS_FRAME_TYPE_MASK ) != STUB_APS_FRAME )
    return;

  // check if delivery mode is of the right type
  if ( ( saff.FrmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_INDIRECT )
    return;

  // check if incoming frame is unsecured
  if ( saff.FrmCtrl & APS_FC_SECURITY )
    return;

  // check if there's no extended header
  if ( saff.FrmCtrl & APS_FC_EXTENDED )
      return;

  // Set the endpoints
  saff.DstEndPoint = appEndPoint;
  saff.SrcEndPoint = STUBAPS_INTER_PAN_EP;

  // Set the signal strength information
  sig.LinkQuality = dataInd->mac.mpduLinkQuality;
  sig.correlation = dataInd->mac.correlation;
  sig.rssi = dataInd->mac.rssi;

  APSDE_DataIndication( &saff, &srcAddr, dataInd->mac.srcPanId,
                        &sig, snff.broadcastId, FALSE, dataInd->mac.timestamp, 0 );

} /* INTERP_DataIndication */


/*********************************************************************
*********************************************************************/
