/**************************************************************************************************
  Filename:       MT_NWK.c
  Revised:        $Date: 2015-01-26 08:25:50 -0800 (Mon, 26 Jan 2015) $
  Revision:       $Revision: 42025 $

  Description:    MonitorTest functions for the NWK layer.

  Copyright 2007-2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "ZComDef.h"
#include "MT.h"
#include "MT_NWK.h"
#include "NLMEDE.h"
#include "nwk.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "OSAL.h"
#include "ZDApp.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
uint16 _nwkCallbackSub;

/*********************************************************************
 * TYPEDEFS
 */

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/
#if defined (MT_NWK_FUNC)
static void MT_NldeDataRequest(uint8 *pBuf);
static void MT_NlmeNetworkFormationRequest(uint8 *pBuf);
static void MT_NlmePermitJoiningRequest(uint8 *pBuf);
static void MT_NlmeStartRouterRequest(uint8 *pBuf);
static void MT_NlmeJoinRequest(uint8 *pBuf);
static void MT_NlmeLeaveRequest(uint8 *pBuf);
static void MT_NlmeResetRequest(uint8 *pBuf);
static void MT_NlmeGetRequest(uint8 *pBuf);
static void MT_NlmeSetRequest(uint8 *pBuf);
static void MT_NlmeNetworkDiscoveryRequest(uint8 *pBuf);
static void MT_NlmeRouteDiscoveryRequest(uint8 *pBuf);
static void MT_NlmeDirectJoinRequest(uint8 *pBuf);
static void MT_NlmeOrphanJoinRequest(uint8 *pBuf);

static uint8 MT_Nwk_DataRequest( uint16 dstAddr, uint8 nsduLen, uint8* nsdu,
                                 uint8 nsduHandle, uint16 nsduHandleOptions,
                                 uint8 secure, uint8 discoverRoute,
                                 uint8 radius);
#endif /* MT_NWK_FUNC */

#if defined (MT_NWK_FUNC)
/***************************************************************************************************
 * @fn      MT_NwkCommandProcessing
 *
 * @brief
 *
 *   Process all the NWK commands that are issued by test tool
 *
 * @param   cmd_id - Command ID
 * @param   len    - Length of received SPI data message
 * @param   pData  - pointer to received SPI data message
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_NwkCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_NWK_INIT:
      nwk_init(NWK_TaskID);
      break;

    case MT_NLDE_DATA_REQ:
      MT_NldeDataRequest(pBuf);
      break;

    case MT_NLME_NETWORK_FORMATION_REQ:
      MT_NlmeNetworkFormationRequest(pBuf);
      break;

    case MT_NLME_PERMIT_JOINING_REQ:
       MT_NlmePermitJoiningRequest(pBuf);
      break;

     case MT_NLME_JOIN_REQ:
       MT_NlmeJoinRequest(pBuf);
       break;

     case MT_NLME_LEAVE_REQ:
       MT_NlmeLeaveRequest(pBuf);
       break;

     case MT_NLME_RESET_REQ:
       MT_NlmeResetRequest(pBuf);
       break;

     case MT_NLME_GET_REQ:
       MT_NlmeGetRequest(pBuf);
       break;

     case MT_NLME_SET_REQ:
       MT_NlmeSetRequest(pBuf);
       break;

     case MT_NLME_NETWORK_DISCOVERY_REQ:
       MT_NlmeNetworkDiscoveryRequest(pBuf);
       break;

     case MT_NLME_ROUTE_DISCOVERY_REQ:
       MT_NlmeRouteDiscoveryRequest(pBuf);
       break;

     case MT_NLME_DIRECT_JOIN_REQ:
       MT_NlmeDirectJoinRequest(pBuf);
       break;

     case MT_NLME_ORPHAN_JOIN_REQ:
       MT_NlmeOrphanJoinRequest(pBuf);
       break;

    case MT_NLME_START_ROUTER_REQ:
      MT_NlmeStartRouterRequest(pBuf);
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_NldeDataReq
 *
 * @brief   Process NLDE Data Request commands
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NldeDataRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint16 dstAddr;
  uint8 dataLen = 0;
  uint8 *dataPtr;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* First read the DstAddr */
  dstAddr = osal_build_uint16( pBuf );
  pBuf += sizeof( dstAddr );

  /* Get the NSDU details */
  dataLen = *pBuf++;
  dataPtr = pBuf;

  /* Skip a length of ZTEST_DEFAULT_DATA_LEN */
  pBuf += dataLen;

  /* Send out Data Request */
  retValue = MT_Nwk_DataRequest(dstAddr, dataLen, dataPtr, pBuf[0], osal_build_uint16( &pBuf[1] ),
                                pBuf[3], pBuf[4], pBuf[5]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}


/***************************************************************************************************
 * @fn      MT_NlmeNetworkFormationRequest
 *
 * @brief   Network Formation Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeNetworkFormationRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint16 panId;
  uint32 channelList;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Build panId */
  panId = osal_build_uint16( pBuf );
  pBuf += sizeof(uint16);

  /* Build the channel list */
  channelList = osal_build_uint32(pBuf, 4);
  pBuf += sizeof(uint32);

  if ( ZG_BUILD_RTR_TYPE )
  {
    retValue = NLME_NetworkFormationRequest( panId, NULL, channelList, pBuf[0], pBuf[1], pBuf[2], pBuf[3],pBuf[4],pBuf[5] );
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmePermitJoiningRequest
 *
 * @brief   Permit Joining Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmePermitJoiningRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (ZSTACK_ROUTER_BUILD)
  {
    retValue = NLME_PermitJoiningRequest(*pBuf);
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeStartRouterRequest
 *
 * @brief   Start Router Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeStartRouterRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZSTACK_ROUTER_BUILD )
  {
    retValue = (uint8)NLME_StartRouterRequest(pBuf[0], pBuf[1], pBuf[2]);
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeJoinRequest
 *
 * @brief   Join Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeJoinRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 dummyExPANID[Z_EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint16 panID;
  uint8 cmdId;
  networkDesc_t *pNwkDesc;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  panID = osal_build_uint16( pBuf );

  if((pNwkDesc = nwk_getNetworkDesc(dummyExPANID,panID, pBuf[2])) != NULL )
  {
    if (pNwkDesc->chosenRouter == INVALID_NODE_ADDR )
    {
      retValue = ZNwkNotPermitted;
    }
    else
    {
      retValue = NLME_JoinRequest( dummyExPANID, panID, pBuf[2], pBuf[3],
                                   pNwkDesc->chosenRouter, pNwkDesc->chosenRouterDepth );
    }
  }
  else
  {
    retValue = ZNwkNotPermitted;
  }

  if ( pBuf[3] & CAPINFO_RCVR_ON_IDLE )
  {
    /* The receiver is on, turn network layer polling off. */
    NLME_SetPollRate( 0 );
    NLME_SetQueuedPollRate( 0 );
    NLME_SetResponseRate( 0 );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeLeaveRequest
 *
 * @brief   Leave Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeLeaveRequest(uint8 *pBuf)
{
  NLME_LeaveReq_t req;
  uint8 retValue = ZFailure;
  uint8 index, cmdId, len;

  /* parse header */
  len =  pBuf[MT_RPC_POS_LEN];
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* If extAddr is all zeros, it means null pointer */
  for(index=0;((index < Z_EXTADDR_LEN) && (pBuf[index] == 0));index++);

  if (index == Z_EXTADDR_LEN)
  {
    req.extAddr = NULL;
  }
  else
  {
    req.extAddr = pBuf;
  }

  /* Increment the pointer */
  pBuf += Z_EXTADDR_LEN;
  if ( len > Z_EXTADDR_LEN )
  {
    req.removeChildren = *pBuf++;
    req.rejoin         = *pBuf++;
  }
  else
  {
    req.removeChildren = FALSE;
    req.rejoin         = FALSE;
  }
  req.silent         = FALSE;

  retValue = NLME_LeaveReq(&req);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}


/***************************************************************************************************
 * @fn      MT_NlmeResetRequest
 *
 * @brief   Leave Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeResetRequest(uint8 *pBuf)
{
  uint8 retValue = NLME_ResetRequest();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), MT_NLME_RESET_REQ, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeGetRequest
 *
 * @brief   Get Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeGetRequest(uint8 *pBuf)
{
  uint8 dataBuf[11];
  uint8 attr, index, cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  attr = *pBuf++;
  index = *pBuf;

  dataBuf[0] = NLME_GetRequest((ZNwkAttributes_t )attr, index, &dataBuf[1]);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId,
                               11, dataBuf );
}

/***************************************************************************************************
 * @fn      MT_NlmeSetRequest
 *
 * @brief   Set Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeSetRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = NLME_SetRequest((ZNwkAttributes_t)pBuf[0], pBuf[1], &pBuf[2]);

  /* Update NV */
  ZDApp_NVUpdate();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_NlmeNetworkDiscoveryRequest
 *
 * @brief   Network Discovery Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeNetworkDiscoveryRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;
  uint32 scanChannels;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Scan channels */
  scanChannels = osal_build_uint32(pBuf, 4);
  pBuf += sizeof(uint32);

  retValue = NLME_NetworkDiscoveryRequest(scanChannels, *pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue );
}

/***************************************************************************************************
 * @fn      MT_NlmeRouteDiscoveryRequest
 *
 * @brief   Route Discovery Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeRouteDiscoveryRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZSTACK_ROUTER_BUILD )
  {
    retValue = NLME_RouteDiscoveryRequest(osal_build_uint16( pBuf ), pBuf[2], pBuf[3]);
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeDirectJoinRequest
 *
 * @brief   Direct Join Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeDirectJoinRequest(uint8 *pBuf)
{
  uint8 retValue = ZFailure;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZSTACK_ROUTER_BUILD )
  {
    retValue = NLME_DirectJoinRequest( pBuf, pBuf[8] );
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_NlmeOrphanJoinRequest
 *
 * @brief   Orphan Join Request
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_NlmeOrphanJoinRequest(uint8 *pBuf)
{
  uint8 i, j, attr;
  uint8 retValue = ZFailure;
  uint32 channelList;
  uint8 cmdId = pBuf[MT_RPC_POS_CMD1];

  if ( ZSTACK_END_DEVICE_BUILD )
  {
    /* parse header */
    pBuf += MT_RPC_FRAME_HDR_SZ;

    /* Channel list bit mask */
    channelList = osal_build_uint32(pBuf, 4);
    pBuf += sizeof(uint32);

    /* Count number of channels */
    j = attr = 0;

    for (i = 0; i < ED_SCAN_MAXCHANNELS; i++)
    {
      if (channelList & (1 << i))
      {
        j++;
        attr = i;
      }
    }

    /* If only one channel specified */
    if (j == 1)
    {
      _NIB.scanDuration = *pBuf;
      _NIB.nwkLogicalChannel = attr;
      _NIB.channelList = channelList;
      if ( !_NIB.CapabilityFlags )
      {
        _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;
      }

      devState = DEV_NWK_ORPHAN;
      retValue = (uint8)NLME_OrphanJoinRequest(channelList, *pBuf);
    }
    else
    {
      retValue = ZNwkInvalidParam;
    }
  }
  else
  {
    retValue = ZUnsupportedMode;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_NWK), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_Nwk_DataRequest
 *
 * @brief   Nwk Data Request
 *
 * @param   dstAddr, nsduLen, nsdu, nsduHandle, nsduHandleOptions, secure, discoverRoute, radius
 *
 * @return  void
 ***************************************************************************************************/
static uint8 MT_Nwk_DataRequest(uint16 dstAddr, uint8 nsduLen, uint8* nsdu,
                                uint8 nsduHandle, uint16 nsduHandleOptions,
                                uint8 secure, uint8 discoverRoute,
                                uint8 radius)
{
    uint8               status;
    NLDE_DataReqAlloc_t dra;
    NLDE_DataReq_t*     req;


    dra.overhead = sizeof(NLDE_DataReq_t);
    dra.nsduLen  = nsduLen;
    dra.secure   = secure;

    req = NLDE_DataReqAlloc(&dra);

    if ( req != NULL )
    {
      osal_memcpy(req->nfd.nsdu, nsdu, nsduLen);

      req->nfd.dstAddr           = dstAddr;
      req->nfd.nsduHandleOptions = nsduHandleOptions;
      req->nfd.discoverRoute     = discoverRoute;
      req->nfd.radius            = radius;

      status = NLDE_DataReq( req );
    }
    else
    {
      status = ZMemError;
    }

    return status;
}
#endif /* MT_NWK_FUNC */

#if defined ( MT_NWK_CB_FUNC )             //NWK callback commands
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubDataConfirm
 *
 * @brief       Process the callback subscription for NLDE-DATA.confirm
 *
 * @param       nsduHandle  - APS handle
 * @param       Status      - result of data request
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubDataConfirm(uint8 nsduHandle, ZStatus_t status)
{
  uint8 buf[2];

  buf[0] = nsduHandle;
  buf[1] = (uint8)status;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLDE_DATA_CONF, 2, buf );
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubDataIndication
 *
 * @brief       Process the callback subscription for NLDE-DATA.indication
 *
 * @param       SrcAddress      - 16 bit address
 * @param       nsduLength      - Length of incoming data
 * @param       nsdu            - Pointer to incoming data
 * @param       LinkQuality     - Link quality measured during
 *                                reception.
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubDataIndication(uint16 SrcAddress, int16 nsduLength, uint8 *nsdu, uint8 LinkQuality)
{
  uint8 *msgPtr;
  uint8 *msg;
  uint8 msgLen;

  msgLen = sizeof( uint16 ) + sizeof( uint8 ) + ZTEST_DEFAULT_DATA_LEN
            + sizeof( uint8);

  msgPtr = osal_mem_alloc( msgLen );
  if ( msgPtr )
  {
    //Fill up the data bytes
    msg = msgPtr;

    //First fill in details
    *msg++ = LO_UINT16( SrcAddress );
    *msg++ = HI_UINT16( SrcAddress );

    //Since the max packet size is less than 255 bytes, a byte is enough
    //to represent nsdu length
    *msg++ = ( uint8 ) nsduLength;

    osal_memset( msg, NULL, ZTEST_DEFAULT_DATA_LEN ); // Clear the mem
    osal_memcpy( msg, nsdu, nsduLength );
    msg += ZTEST_DEFAULT_DATA_LEN;

    *msg++ = LinkQuality;

    MT_BuildAndSendZToolResponse( ((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLDE_DATA_IND, msgLen, msgPtr );

    osal_mem_free( msgPtr );
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubInitCoordConfirm
 *
 * @brief       Process the callback subscription for NLME-INIT-COORD.confirm
 *
 * @param       Status - Result of NLME_InitCoordinatorRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubInitCoordConfirm( ZStatus_t Status )
{
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_NETWORK_FORMATION_CONF,
                          sizeof(uint8), (uint8*)&Status);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubStartRouterConfirm
 *
 * @brief       Process the callback subscription for NLME-START-ROUTER.confirm
 *
 * @param       Status - Result of NLME_StartRouterRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubStartRouterConfirm( ZStatus_t Status )
{
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_START_ROUTER_CONF,
                          sizeof(uint8), (uint8*)&Status);
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubJoinConfirm
 *
 * @brief       Process the callback subscription for NLME-JOIN.confirm
 *
 * @param       Status - Result of NLME_JoinRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubJoinConfirm(uint16 PanId, ZStatus_t Status)
{
  uint8 msg[Z_EXTADDR_LEN + 3];

  /* This device's 64-bit address */
  ZMacGetReq( ZMacExtAddr, msg );

  msg[Z_EXTADDR_LEN + 0] = LO_UINT16(PanId);
  msg[Z_EXTADDR_LEN + 1] = HI_UINT16(PanId);
  msg[Z_EXTADDR_LEN + 2] = (uint8)Status;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_JOIN_CONF, Z_EXTADDR_LEN + 3, msg );
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubNetworkDiscoveryConfirm
 *
 * @brief       Process the callback subscription for NLME-NWK_DISC.confirm
 *
 * @param       ResultCount			- number of networks discovered
 * @param				NetworkList			- pointer to list of network descriptors
 *
 * @return      void
 ***************************************************************************************************/
void nwk_MTCallbackSubNetworkDiscoveryConfirm( uint8 ResultCount, networkDesc_t *NetworkList )
{
	uint8 len;
	uint8 *msgPtr;
	uint8 *msg;
	uint8 i;

  // The message cannot be bigger then SPI_TX_BUFF_MAX.  Reduce resultCount if necessary
  if (ResultCount * sizeof(networkDesc_t) > MT_UART_TX_BUFF_MAX - (1 + SPI_0DATA_MSG_LEN))
  {
    ResultCount = (MT_UART_TX_BUFF_MAX - (1 + SPI_0DATA_MSG_LEN)) / sizeof(networkDesc_t);
  }

	len = 1 + ResultCount * sizeof(networkDesc_t);
  msgPtr = osal_mem_alloc( len );
	if ( msgPtr )
	{
	  /* Fill up the data bytes */
    msg = msgPtr;

		*msg++ = ResultCount;

		for ( i = 0; i < ResultCount; i++ )
		{
		  *msg++ = LO_UINT16( NetworkList->panId );
		  *msg++ = HI_UINT16( NetworkList->panId );
		  *msg++ = NetworkList->logicalChannel;
		  *msg++ = BEACON_ORDER_NO_BEACONS;
		  *msg++ = BEACON_ORDER_NO_BEACONS;
		  *msg++ = NetworkList->routerCapacity;
		  *msg++ = NetworkList->deviceCapacity;
		  *msg++ = NetworkList->version;
		  *msg++ = NetworkList->stackProfile;
		  //*msg++ = NetworkList->securityLevel;

			NetworkList = (networkDesc_t*)NetworkList->nextDesc;
		}

    MT_BuildAndSendZToolResponse (((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_NETWORK_DISCOVERY_CONF, len, msgPtr);

    osal_mem_free( msgPtr );
	}
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubJoinIndication
 *
 * @brief       Process the callback subscription for NLME-INIT-COORD.indication
 *
 * @param       ShortAddress - 16-bit address
 * @param       ExtendedAddress - IEEE (64-bit) address
 * @param       CapabilityFlags - Association Capability Information
 *
 * @return      ZStatus_t
 ***************************************************************************************************/
void nwk_MTCallbackSubJoinIndication( uint16 ShortAddress, uint8 *ExtendedAddress,
                                      uint8 CapabilityFlags )
{
  uint8 *msgPtr;
  uint8 *msg;
  uint8 len;

  len = sizeof( uint16 ) + Z_EXTADDR_LEN + sizeof( uint8 );
  msgPtr = osal_mem_alloc( len );

  if ( msgPtr )
  {
    /* Fill up the data bytes */
    msg = msgPtr;

    /* First fill in details */
    *msg++ = LO_UINT16( ShortAddress );
    *msg++ = HI_UINT16( ShortAddress );

    osal_cpyExtAddr( msg, ExtendedAddress );
    msg += Z_EXTADDR_LEN;

    *msg = CapabilityFlags;

    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_JOIN_IND, len, msgPtr );

    osal_mem_free( msgPtr );
  }
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubLeaveConfirm
 *
 * @brief       Process the callback subscription for NLME-LEAVE.confirm
 *
 * @param       DeviceAddress - IEEE (64-bit) address
 * @param       Status - Result of NLME_LeaveRequest()
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubLeaveConfirm( uint8 *DeviceAddress, ZStatus_t Status )
{
  uint8 *msgPtr;
  uint8 *msg;

  msgPtr = osal_mem_alloc( Z_EXTADDR_LEN + sizeof( uint8 ) );
  if ( msgPtr )
  {
    /* Fill up the data bytes */
    msg = msgPtr;

    /* First fill in details */
    osal_cpyExtAddr( msg, DeviceAddress );
    msg += Z_EXTADDR_LEN;

    *msg = (uint8)Status;

    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_LEAVE_CONF,
                            Z_EXTADDR_LEN + sizeof( uint8 ), msgPtr );

    osal_mem_free( msgPtr );
  }
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubLeaveIndication
 *
 * @brief       Process the callback subscription for NLME-LEAVE.indication
 *
 * @param       DeviceAddress - IEEE (64-bit) address
 *
 * @return      NULL
 ***************************************************************************************************/
void nwk_MTCallbackSubLeaveIndication( uint8 *DeviceAddress )
{
  uint8 msg[Z_EXTADDR_LEN+1];

  /* First fill in details */
  if ( DeviceAddress )
  {
    osal_cpyExtAddr( msg, DeviceAddress );
  }
  else
  {
    osal_memset( msg, 0, Z_EXTADDR_LEN );
  }

  /* Status, assume good if we get this far */
  msg[Z_EXTADDR_LEN] = 0;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_LEAVE_IND, Z_EXTADDR_LEN+1, msg );
}
/***************************************************************************************************
 * @fn          nwk_MTCallbackSubSyncIndication
 *
 * @brief       Process the callback subscription for NLME-SYNC.indication
 *
 * @param       none
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubSyncIndication( void )
{
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_SYNC_IND, 0, NULL );
}

/***************************************************************************************************
 * @fn          nwk_MTCallbackSubPollConfirm
 *
 * @brief       Process the callback subscription for NLME-POLL.confirm
 *
 * @param       status - status of the poll operation
 *
 * @return      none
 ***************************************************************************************************/
void nwk_MTCallbackSubPollConfirm( uint8 status )
{
  uint8 msg = status;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_NWK), MT_NLME_POLL_CONF, 1, &msg );
}

#endif /* NWK Callback commands */

/***************************************************************************************************
 ***************************************************************************************************/

