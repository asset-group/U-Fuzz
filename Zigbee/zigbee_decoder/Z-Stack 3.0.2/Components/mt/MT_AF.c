/**************************************************************************************************
  Filename:       MT_AF.c
  Revised:        $Date: 2015-01-26 08:25:50 -0800 (Mon, 26 Jan 2015) $
  Revision:       $Revision: 42025 $

  Description:    MonitorTest functions for the AF layer.

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_AF.h"
#include "MT_ZDO.h"
#include "nwk.h"
#include "OnBoard.h"
#include "MT_UART.h"

#if defined INTER_PAN
#include "stub_aps.h"
#endif


/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#if !defined MT_AF_EXEC_CNT
#define MT_AF_EXEC_CNT  15
#endif

#if !defined MT_AF_EXEC_DLY
#define MT_AF_EXEC_DLY  1000
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct
{
  uint8 *data;
  afAddrType_t dstAddr;
  endPointDesc_t *epDesc;
  uint16 cId;
  uint16 dataLen;
  uint8 transId;
  uint8 txOpts;
  uint8 radius;
  uint8 tick;
} mtAfDataReq_t;

typedef struct _mtAfInMsgList_t
{
  struct _mtAfInMsgList_t *next;
  uint8 *data;
  uint32 timestamp;         // Receipt timestamp from MAC.
  uint8 tick;
} mtAfInMsgList_t;

/* ------------------------------------------------------------------------------------------------
 *                                        Local Variables
 * ------------------------------------------------------------------------------------------------
 */

mtAfInMsgList_t *pMtAfInMsgList = NULL;
mtAfDataReq_t *pMtAfDataReq = NULL;

/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */

#if defined ( MT_AF_CB_FUNC )
uint16 _afCallbackSub;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                        Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void MT_AfRegister(uint8 *pBuf);
static void MT_AfDelete(uint8 *pBuf);
static void MT_AfDataRequest(uint8 *pBuf);

#if defined ( ZIGBEEPRO )
static void MT_AfDataRequestSrcRtg(uint8 *pBuf);
#endif

#if defined INTER_PAN
static void MT_AfInterPanCtl(uint8 *pBuf);
#endif

static void MT_AfDataRetrieve(uint8 *pBuf);
static void MT_AfDataStore(uint8 *pBuf);
static void MT_AfAPSF_ConfigSet(uint8 *pBuf);
static void MT_AfAPSF_ConfigGet(uint8 *pBuf);


/**************************************************************************************************
 * @fn          MT_AfExec
 *
 * @brief       This function is invoked by an MT timer event.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MT_AfExec(void)
{
  mtAfInMsgList_t *pPrev, *pItem = pMtAfInMsgList;

  while (pItem != NULL)
  {
    if (--(pItem->tick) == 0)
    {
      if (pMtAfInMsgList == pItem)
      {
        pMtAfInMsgList = pItem->next;
        (void)osal_mem_free(pItem);
        pItem = pMtAfInMsgList;
      }
      else
      {
        pPrev->next = pItem->next;
        (void)osal_mem_free(pItem);
        pItem = pPrev->next;
      }
    }
    else
    {
      pPrev = pItem;
      pItem = pItem->next;
    }
  }

  if (pMtAfDataReq != NULL)
  {
    if (--(pMtAfDataReq->tick) == 0)
    {
      (void)osal_mem_free(pMtAfDataReq);
      pMtAfDataReq = NULL;
    }
  }

  if ((pMtAfInMsgList != NULL) || (pMtAfDataReq != NULL))
  {
    if (ZSuccess != osal_start_timerEx(MT_TaskID, MT_AF_EXEC_EVT, MT_AF_EXEC_DLY))
    {
      osal_set_event(MT_TaskID, MT_AF_EXEC_EVT);
    }
  }
}

/***************************************************************************************************
 * @fn      MT_AfCommandProcessing
 *
 * @brief   Process all the AF commands that are issued by test tool
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_AfCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_AF_REGISTER:
      MT_AfRegister(pBuf);
      break;

    case MT_AF_DELETE:
      MT_AfDelete( pBuf );
      break;

    case MT_AF_DATA_REQUEST:
    case MT_AF_DATA_REQUEST_EXT:
      MT_AfDataRequest(pBuf);
      break;

#if defined( ZIGBEEPRO )
    case MT_AF_DATA_REQUEST_SRCRTG:
      MT_AfDataRequestSrcRtg(pBuf);
      break;
#endif

#if defined INTER_PAN
    case MT_AF_INTER_PAN_CTL:
      MT_AfInterPanCtl(pBuf);
      break;
#endif

    case MT_AF_DATA_RETRIEVE:
      MT_AfDataRetrieve(pBuf);
      break;

    case MT_AF_DATA_STORE:
      MT_AfDataStore(pBuf);
      break;

    case MT_AF_APSF_CONFIG_SET:
      MT_AfAPSF_ConfigSet(pBuf);
      break;

    case MT_AF_APSF_CONFIG_GET:
      MT_AfAPSF_ConfigGet(pBuf);
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_AfRegister
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfRegister(uint8 *pBuf)
{
  uint8 cmdId;
  uint8 retValue = ZMemError;
  endPointDesc_t *epDesc;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  epDesc = (endPointDesc_t *)osal_mem_alloc(sizeof(endPointDesc_t));
  if ( epDesc )
  {
    epDesc->task_id = &MT_TaskID;
    retValue = MT_BuildEndpointDesc( pBuf, epDesc );
    if ( retValue == ZSuccess )
    {
      retValue = afRegister( epDesc );
    }

    if ( retValue != ZSuccess )
    {
      osal_mem_free( epDesc );
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AfDelete
 *
 * @brief   Process AF Delete Endpoint command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfDelete(uint8 *pBuf)
{
  uint8 cmdId;
  uint8 retValue = ZMemError;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  retValue = afDelete( *pBuf );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_AfDataRequest
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfDataRequest(uint8 *pBuf)
{
  #define MT_AF_REQ_MSG_LEN  10
  #define MT_AF_REQ_MSG_EXT  10

  endPointDesc_t *epDesc;
  afAddrType_t dstAddr;
  cId_t cId;
  uint8 transId, txOpts, radius;
  uint8 cmd0, cmd1;
  uint8 retValue = ZFailure;
  uint16 dataLen, tempLen;

  /* Parse header */
  cmd0 = pBuf[MT_RPC_POS_CMD0];
  cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (cmd1 == MT_AF_DATA_REQUEST_EXT)
  {
    dstAddr.addrMode = (afAddrMode_t)*pBuf++;

    if (dstAddr.addrMode == afAddr64Bit)
    {
      (void)osal_memcpy(dstAddr.addr.extAddr, pBuf, Z_EXTADDR_LEN);
    }
    else
    {
      dstAddr.addr.shortAddr = osal_build_uint16( pBuf );
    }
    pBuf += Z_EXTADDR_LEN;

    dstAddr.endPoint = *pBuf++;
    dstAddr.panId = osal_build_uint16( pBuf );
    pBuf += 2;
  }
  else
  {
    /* Destination address */
    dstAddr.addrMode = afAddr16Bit;
    dstAddr.addr.shortAddr = osal_build_uint16( pBuf );
    pBuf += 2;

    /* Destination endpoint */
    dstAddr.endPoint = *pBuf++;
    dstAddr.panId = 0;
  }

  /* Source endpoint */
  epDesc = afFindEndPointDesc(*pBuf++);

  /* ClusterId */
  cId = osal_build_uint16( pBuf );
  pBuf +=2;

  /* TransId */
  transId = *pBuf++;

  /* TxOption */
  txOpts = *pBuf++;

  /* Radius */
  radius = *pBuf++;

  /* Length */
  if (cmd1 == MT_AF_DATA_REQUEST_EXT)
  {
    dataLen = osal_build_uint16( pBuf );
    tempLen = dataLen + MT_AF_REQ_MSG_LEN + MT_AF_REQ_MSG_EXT;
    pBuf += 2;
  }
  else
  {
    dataLen = *pBuf++;
    tempLen = dataLen + MT_AF_REQ_MSG_LEN;
  }

  if ( epDesc == NULL )
  {
    retValue = afStatus_INVALID_PARAMETER;
  }
  else if (tempLen > (uint16)MT_RPC_DATA_MAX)
  {
    if (pMtAfDataReq != NULL)
    {
      retValue = afStatus_INVALID_PARAMETER;
    }
    else if ((pMtAfDataReq = osal_mem_alloc(sizeof(mtAfDataReq_t) + dataLen)) == NULL)
    {
      retValue = afStatus_MEM_FAIL;
    }
    else
    {
      retValue = afStatus_SUCCESS;

      pMtAfDataReq->data = (uint8 *)(pMtAfDataReq+1);
      (void)osal_memcpy(&(pMtAfDataReq->dstAddr), &dstAddr, sizeof(afAddrType_t));
      pMtAfDataReq->epDesc = epDesc;
      pMtAfDataReq->cId = cId;
      pMtAfDataReq->dataLen = dataLen;
      pMtAfDataReq->transId = transId;
      pMtAfDataReq->txOpts = txOpts;
      pMtAfDataReq->radius = radius;

      // Setup to time-out the huge outgoing item if host does not MT_AF_DATA_STORE it.
      pMtAfDataReq->tick = MT_AF_EXEC_CNT;
      if (ZSuccess != osal_start_timerEx(MT_TaskID, MT_AF_EXEC_EVT, MT_AF_EXEC_DLY))
      {
        (void)osal_set_event(MT_TaskID, MT_AF_EXEC_EVT);
      }
    }
  }
  else
  {
    retValue = AF_DataRequest(&dstAddr, epDesc, cId, dataLen, pBuf, &transId, txOpts, radius);
  }

  if (MT_RPC_CMD_SREQ == (cmd0 & MT_RPC_CMD_TYPE_MASK))
  {
    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP|(uint8)MT_RPC_SYS_AF), cmd1, 1, &retValue);
  }
}

#if defined( ZIGBEEPRO )
/***************************************************************************************************
 * @fn      MT_AfDataRequestSrcRtg
 *
 * @brief   Process AF Register command
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfDataRequestSrcRtg(uint8 *pBuf)
{
  uint8 cmdId, dataLen = 0;
  uint8 retValue = ZFailure;
  endPointDesc_t *epDesc;
  byte transId;
  afAddrType_t dstAddr;
  cId_t cId;
  byte txOpts, radius, srcEP, relayCnt;
  uint16 *pRelayList;
  uint8 i;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Destination address */
  /* Initialize the panID field to zero to avoid inter-pan */
  osal_memset( &dstAddr, 0, sizeof(afAddrType_t) );
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.addr.shortAddr = osal_build_uint16( pBuf );
  pBuf += 2;

  /* Destination endpoint */
  dstAddr.endPoint = *pBuf++;

  /* Source endpoint */
  srcEP = *pBuf++;
  epDesc = afFindEndPointDesc( srcEP );

  /* ClusterId */
  cId = osal_build_uint16( pBuf );
  pBuf +=2;

  /* TransId */
  transId = *pBuf++;

  /* TxOption */
  txOpts = *pBuf++;

  /* Radius */
  radius = *pBuf++;

  /* Source route relay count */
  relayCnt = *pBuf++;

  /* Convert the source route relay list */
  if( (pRelayList = osal_mem_alloc( relayCnt * sizeof( uint16 ))) != NULL )
  {
    for( i = 0; i < relayCnt; i++ )
    {
      pRelayList[i] = osal_build_uint16( pBuf );
      pBuf += 2;
    }

    /* Data payload Length */
    dataLen = *pBuf++;

    if ( epDesc == NULL )
    {
      retValue = afStatus_INVALID_PARAMETER;
    }
    else
    {
      retValue = AF_DataRequestSrcRtg( &dstAddr, epDesc, cId, dataLen, pBuf,
                                     &transId, txOpts, radius, relayCnt, pRelayList );
    }

    /* Free the memory allocated */
    osal_mem_free( pRelayList );
  }
  else
  {
    retValue = afStatus_MEM_FAIL;
  }


  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF), cmdId, 1, &retValue);
}
#endif

#if defined INTER_PAN
/***************************************************************************************************
 * @fn      MT_AfInterPanCtl
 *
 * @brief   Process the AF Inter Pan control command.
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
static void MT_AfInterPanCtl(uint8 *pBuf)
{
  uint8 cmd, rtrn;
  uint16 panId;
  endPointDesc_t *pEP;

  cmd = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  switch (*pBuf++)  // Inter-pan request parameter.
  {
  case InterPanClr:
    rtrn = StubAPS_SetIntraPanChannel();           // Switch channel back to the NIB channel.
    break;

  case InterPanSet:
    rtrn = StubAPS_SetInterPanChannel(*pBuf);      // Set channel for inter-pan communication.
    break;

  case InterPanReg:
    if ((pEP = afFindEndPointDesc(*pBuf)))
    {
      StubAPS_RegisterApp(pEP);
      rtrn = SUCCESS;
    }
    else
    {
      rtrn = FAILURE;
    }
    break;

  case InterPanChk:
    panId = osal_build_uint16( pBuf );
    rtrn = (StubAPS_InterPan(panId, pBuf[2])) ? ZSuccess : ZFailure;
    break;

  default:
    rtrn = afStatus_INVALID_PARAMETER;
    break;
  }

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF), cmd, 1, &rtrn);
}
#endif

/***************************************************************************************************
 * @fn      MT_AfDataConfirm
 *
 * @brief   Process
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfDataConfirm(afDataConfirm_t *pMsg)
{
  uint8 retArray[3];

  retArray[0] = pMsg->hdr.status;
  retArray[1] = pMsg->endpoint;
  retArray[2] = pMsg->transID;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_AF), MT_AF_DATA_CONFIRM, 3, retArray);
}

/***************************************************************************************************
 * @fn      MT_AfReflectError
 *
 * @brief   Process
 *
 * @param   pBuf - pointer to the received buffer
 *
 * @return  none
 ***************************************************************************************************/
void MT_AfReflectError(afReflectError_t *pMsg)
{
  uint8 retArray[6];

  retArray[0] = pMsg->hdr.status;
  retArray[1] = pMsg->endpoint;
  retArray[2] = pMsg->transID;
  retArray[3] = pMsg->dstAddrMode;
  retArray[4] = LO_UINT16( pMsg->dstAddr );
  retArray[5] = HI_UINT16( pMsg->dstAddr );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_AF), MT_AF_REFLECT_ERROR, 6, retArray);
}

/***************************************************************************************************
 * @fn          MT_AfIncomingMsg
 *
 * @brief       Process the callback subscription for AF Incoming data.
 *
 * @param       pkt - Incoming AF data.
 *
 * @return      none
 ***************************************************************************************************/
void MT_AfIncomingMsg(afIncomingMSGPacket_t *pMsg)
{
  #define MT_AF_INC_MSG_LEN  20
  #define MT_AF_INC_MSG_EXT  10

  uint16 dataLen = pMsg->cmd.DataLength;  // Length of the data section in the response packet.
  uint16 respLen = MT_AF_INC_MSG_LEN + dataLen;
  uint8 cmd = MT_AF_INCOMING_MSG;
  uint8 *pRsp, *pTmp;
  mtAfInMsgList_t *pItem = NULL;

#if defined INTER_PAN
  if (StubAPS_InterPan(pMsg->srcAddr.panId, pMsg->srcAddr.endPoint))
  {
    cmd = MT_AF_INCOMING_MSG_EXT;
  }
  else
#endif
  if ((pMsg->srcAddr.addrMode == afAddr64Bit) ||
      (respLen > (uint16)(MT_RPC_DATA_MAX - MT_AF_INC_MSG_EXT)))
  {
    cmd = MT_AF_INCOMING_MSG_EXT;
  }

  if (cmd == MT_AF_INCOMING_MSG_EXT)
  {
    respLen += MT_AF_INC_MSG_EXT;
  }

  if (respLen > (uint16)MT_RPC_DATA_MAX)
  {
    if ((pItem = (mtAfInMsgList_t *)osal_mem_alloc(sizeof(mtAfInMsgList_t) + dataLen)) == NULL)
    {
      return;  // If cannot hold a huge message, cannot give indication at all.
    }

    pItem->data = (uint8 *)(pItem+1);
    respLen -= dataLen;  // Zero data bytes are sent with an over-sized incoming indication.
  }

  // Attempt to allocate memory for the response packet.
  if ((pRsp = osal_mem_alloc(respLen)) == NULL)
  {
    if (pItem != NULL)
    {
      (void)osal_mem_free(pItem);
    }
    return;
  }
  pTmp = pRsp;

  /* Group ID */
  *pTmp++ = LO_UINT16(pMsg->groupId);
  *pTmp++ = HI_UINT16(pMsg->groupId);

  /* Cluster ID */
  *pTmp++ = LO_UINT16(pMsg->clusterId);
  *pTmp++ = HI_UINT16(pMsg->clusterId);

  if (cmd == MT_AF_INCOMING_MSG_EXT)
  {
    *pTmp++ = pMsg->srcAddr.addrMode;

    if (pMsg->srcAddr.addrMode == afAddr64Bit)
    {
      (void)osal_memcpy(pTmp, pMsg->srcAddr.addr.extAddr, Z_EXTADDR_LEN);
    }
    else
    {
      pTmp[0] = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
      pTmp[1] = HI_UINT16(pMsg->srcAddr.addr.shortAddr);
    }
    pTmp += Z_EXTADDR_LEN;

    *pTmp++ = pMsg->srcAddr.endPoint;
#if defined INTER_PAN
    *pTmp++ = LO_UINT16(pMsg->srcAddr.panId);
    *pTmp++ = HI_UINT16(pMsg->srcAddr.panId);
#else
    *pTmp++ = 0;
    *pTmp++ = 0;
#endif
  }
  else
  {
    /* Source Address */
    *pTmp++ = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
    *pTmp++ = HI_UINT16(pMsg->srcAddr.addr.shortAddr);

    /* Source EP */
    *pTmp++ = pMsg->srcAddr.endPoint;
  }

  /* Destination EP */
  *pTmp++ = pMsg->endPoint;

  /* WasBroadCast */
  *pTmp++ = pMsg->wasBroadcast;

  /* LinkQuality */
  *pTmp++ = pMsg->LinkQuality;

  /* SecurityUse */
  *pTmp++ = pMsg->SecurityUse;

  /* Timestamp */
  osal_buffer_uint32( pTmp, pMsg->timestamp );
  pTmp += 4;

  /* Data Length */
  if (cmd == MT_AF_INCOMING_MSG_EXT)
  {
    /* Z-Tool apparently takes the last Byte before the data buffer as the dynamic length and
     * ignores the bigger UInt16 length of an EXT incoming message. But no data bytes will be sent
     * with a huge message, so it's necessary to work-around and fake-out Z-Tool with a zero here.
     */
    *pTmp++ = 0;  // TODO - workaround Z-Tool shortcoming; should be: = pMsg->cmd.TransSeqNumber;
    *pTmp++ = LO_UINT16(dataLen);
    *pTmp++ = HI_UINT16(dataLen);
  }
  else
  {
    *pTmp++ = pMsg->cmd.TransSeqNumber;
    *pTmp++ = dataLen;
  }

  /* Data */
  if (pItem != NULL)
  {
    // Enqueue the new huge incoming item.
    pItem->next = pMtAfInMsgList;
    pMtAfInMsgList = pItem;

    // Setup to time-out the huge incoming item if host does not MT_AF_DATA_RETRIEVE it.
    pItem->tick = MT_AF_EXEC_CNT;
    if (ZSuccess != osal_start_timerEx(MT_TaskID, MT_AF_EXEC_EVT, MT_AF_EXEC_DLY))
    {
      (void)osal_set_event(MT_TaskID, MT_AF_EXEC_EVT);
    }

    pItem->timestamp = pMsg->timestamp;
    (void)osal_memcpy(pItem->data, pMsg->cmd.Data, dataLen);
  }
  else
  {
    (void)osal_memcpy(pTmp, pMsg->cmd.Data, dataLen);
    pTmp += dataLen;
  }

  // MAC Source address
  *pTmp++ = LO_UINT16(pMsg->macSrcAddr);
  *pTmp++ = HI_UINT16(pMsg->macSrcAddr);

  // messages result radius
  *pTmp = pMsg->radius;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ|(uint8)MT_RPC_SYS_AF), cmd, respLen, pRsp);

  (void)osal_mem_free(pRsp);
}

/**************************************************************************************************
 * @fn          MT_AfDataRetrieve
 *
 * @brief   Process AF Data Retrieve command to incrementally read out a very large
 *          incoming AF message.
 *
 * input parameters
 *
 * @param pBuf - pointer to the received buffer
 *
 * output parameters
 *
 * @param rtrn - AF-Status of the operation.
 *
 * @return      None.
 **************************************************************************************************
 */
static void MT_AfDataRetrieve(uint8 *pBuf)
{
  #define MT_AF_RTV_HDR_SZ  2

  uint32 timestamp;
  mtAfInMsgList_t *pPrev, *pItem = pMtAfInMsgList;
  uint8 rtrn = afStatus_FAILED;
  uint8 len = 0;

  pBuf += MT_RPC_FRAME_HDR_SZ;
  timestamp = osal_build_uint32( pBuf, 4 );

  while (pItem != NULL)
  {
    pPrev = pItem;
    if (pItem->timestamp == timestamp)
    {
      break;
    }
    pItem = pItem->next;
  }

  if (pItem != NULL)
  {
    uint16 idx;
    uint8 *pRsp;

    pBuf += 4;
    idx = osal_build_uint16( pBuf );
    len = pBuf[2];

    if (len == 0)  // Indication to delete the afIncomingMSGPacket.
    {
      if (pMtAfInMsgList == pItem)
      {
        pMtAfInMsgList = pItem->next;
      }
      else
      {
        pPrev->next = pItem->next;
      }
      (void)osal_mem_free(pItem);
      rtrn = afStatus_SUCCESS;
    }
    else if ((pRsp = osal_mem_alloc(len + MT_AF_RTV_HDR_SZ)) == NULL)
    {
      rtrn = afStatus_MEM_FAIL;
      len = 0;
    }
    else
    {
      pRsp[0] = ZSuccess;
      pRsp[1] = len;
      (void)osal_memcpy(pRsp + MT_AF_RTV_HDR_SZ, pItem->data+idx, len);
      MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF),
                                           MT_AF_DATA_RETRIEVE, len + MT_AF_RTV_HDR_SZ, pRsp);
      (void)osal_mem_free(pRsp);
      return;
    }
  }

  pBuf[0] = rtrn;
  pBuf[1] = len;
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF),
                                       MT_AF_DATA_RETRIEVE, MT_AF_RTV_HDR_SZ, pBuf);
}

/**************************************************************************************************
 * @fn          MT_AfDataStore
 *
 * @brief   Process AF Data Store command to incrementally store the data buffer for very large
 *          outgoing AF message.
 *
 * input parameters
 *
 * @param pBuf - pointer to the received buffer
 *
 * output parameters
 *
 * @param rtrn - AF-Status of the operation.
 *
 * @return      None.
 **************************************************************************************************
 */
static void MT_AfDataStore(uint8 *pBuf)
{
  uint16 idx;
  uint8 len, rtrn = afStatus_FAILED;

  pBuf += MT_RPC_FRAME_HDR_SZ;
  idx = osal_build_uint16( pBuf );
  len = pBuf[2];
  pBuf += 3;

  if (pMtAfDataReq == NULL)
  {
    rtrn = afStatus_MEM_FAIL;
  }
  else if (len == 0)  // Indication to send the message.
  {
    rtrn = AF_DataRequest(&(pMtAfDataReq->dstAddr), pMtAfDataReq->epDesc, pMtAfDataReq->cId,
                            pMtAfDataReq->dataLen,  pMtAfDataReq->data,
                          &(pMtAfDataReq->transId), pMtAfDataReq->txOpts, pMtAfDataReq->radius);
    (void)osal_mem_free(pMtAfDataReq);
    pMtAfDataReq = NULL;
  }
  else
  {
    (void)osal_memcpy(pMtAfDataReq->data+idx, pBuf, len);
    rtrn = afStatus_SUCCESS;
  }

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF),
                                                                MT_AF_DATA_STORE, 1, &rtrn);
}

/**************************************************************************************************
 * @fn          MT_AfAPSF_ConfigSet
 *
 * @brief       This function is the MT proxy for afAPSF_ConfigSet().
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the received buffer.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void MT_AfAPSF_ConfigSet(uint8 *pBuf)
{
  afAPSF_Config_t cfg = { pBuf[MT_RPC_POS_DAT0+1], pBuf[MT_RPC_POS_DAT0+2] };
  afStatus_t rtrn = afAPSF_ConfigSet(pBuf[MT_RPC_POS_DAT0], &cfg);

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF),
                                       MT_AF_APSF_CONFIG_SET, 1, (uint8 *)&rtrn);
}

/**************************************************************************************************
 * @fn          MT_AfAPSF_ConfigGet
 *
 * @brief       This function is the MT proxy for afAPSF_ConfigGet().
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the received buffer.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void MT_AfAPSF_ConfigGet(uint8 *pBuf)
{
  afAPSF_Config_t cfg = { 0, 0 };
  uint8 buf[2];

  afAPSF_ConfigGet( pBuf[MT_RPC_POS_DAT0], &cfg );

  buf[0] = cfg.frameDelay;
  buf[1] = cfg.windowSize;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_AF),
                                       MT_AF_APSF_CONFIG_GET, 3, buf );
}

/***************************************************************************************************
***************************************************************************************************/
