/***************************************************************************************************
  Filename:       MT_GP.c
  Revised:        $Date: 2016-06-21 01:06:52 -0700 (Thu, 21 July 2016) $
  Revision:       $Revision:  $

  Description:    MonitorTest functions GP interface.

  Copyright 2007-2013 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED ?AS IS? WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

 ***************************************************************************************************/

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
 
 
#include "ZComDef.h"
#include "MT.h"
#include "MT_GP.h"
#include "MT_UART.h"
#include "nwk_util.h"   


/***************************************************************************************************
* CONST
***************************************************************************************************/
#define GP_DATA_REQ_PAYLOAD_LEN_POS   17   
#define GP_DATA_REQ_APP_ID_POS         2

#define SEC_KEY_LEN                   16
 
/***************************************************************************************************
* LOCAL FUNCTIONs
***************************************************************************************************/

#ifdef MT_GP_CB_FUNC
static void MT_GpDataReq(uint8* pBuf);
static void MT_GpSecRsp(uint8* pBuf);
#endif

void MT_GPDataCnf(gp_DataCnf_t* gp_DataCnf);
void MT_GPSecReq(gp_SecReq_t* gp_SecReq);
void MT_GPDataInd(gp_DataInd_t* gp_DataInd);


/***************************************************************************************************
* External variables
***************************************************************************************************/

#ifdef MT_GP_CB_FUNC  
uint8 MT_GpCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_GP_DATA_REQ:
      MT_GpDataReq(pBuf);
    break;
    case MT_GP_SEC_RSP:
      MT_GpSecRsp(pBuf);
    break;
    case MT_GP_ADDRESS_CONFLICT:
      MT_GPAddressConflict(pBuf);
    break;
  }
  return status;
}


 /***************************************************************************************************
 * @fn      MT_GpDataReq
 *
 * @brief   GP data request interface
 *
 * @param   pBuf - gp data request
 *
 * @return  void
 ***************************************************************************************************/
 static void MT_GpDataReq(uint8* pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  gp_DataReq_t *gp_DataReq;
  uint8  payloadLen;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  //Get the payload length
  payloadLen = pBuf[GP_DATA_REQ_PAYLOAD_LEN_POS];
  
  gp_DataReq = (gp_DataReq_t*)osal_msg_allocate(sizeof(gp_DataReq_t) + payloadLen);  

  //No memory
  if(gp_DataReq == NULL)
  {
    retValue = FAILURE;
  }
  //Invalid application ID
  if( (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_DEFAULT) && (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_GP ) )
  {
    retValue = INVALIDPARAMETER;
  }
  //Return fail/InvalidParameter
  if(retValue)
  {
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_GP), cmdId, 1, &retValue);
    return;
  }

  gp_DataReq->Action = *pBuf++;
  gp_DataReq->TxOptions = *pBuf++;
  gp_DataReq->gpd_ID.AppID = *pBuf++;
  if(gp_DataReq->gpd_ID.AppID == GP_APP_ID_DEFAULT)
  {
    gp_DataReq->gpd_ID.GPDId.SrcID = osal_build_uint32(pBuf,4);
    pBuf += sizeof(uint32) + Z_EXTADDR_LEN;
  }
  else
  {
    pBuf += sizeof(uint32);
    osal_memcpy(gp_DataReq->gpd_ID.GPDId.GPDExtAddr,pBuf,Z_EXTADDR_LEN);
    pBuf += Z_EXTADDR_LEN;
  }

  gp_DataReq->EndPoint = *pBuf++;
  gp_DataReq->GPDCmmdId = *pBuf++;
  gp_DataReq->GPDasduLength = *pBuf++;
  osal_memcpy(gp_DataReq->GPDasdu,pBuf,payloadLen);
  pBuf += payloadLen;
  gp_DataReq->GPEPhandle = *pBuf++;
  gp_DataReq->gpTxQueueEntryLifeTime[2] = *pBuf++;
  gp_DataReq->gpTxQueueEntryLifeTime[1] = *pBuf++;
  gp_DataReq->gpTxQueueEntryLifeTime[0] = *pBuf++;
  
  gp_DataReq->hdr.event = GP_DATA_REQ;
  gp_DataReq->hdr.status = 0;

  osal_msg_send(gp_TaskID,(uint8*)gp_DataReq);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_GP), cmdId, 1, &retValue);
}



/***************************************************************************************************
* @fn      MT_GpSecRsp
*
* @brief   GP Sec Response interface
*
* @param   pBuf - gp sec rsp
*
* @return  void
***************************************************************************************************/

void MT_GpSecRsp(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  gp_SecRsp_t *gp_SecRsp;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  gp_SecRsp = (gp_SecRsp_t*)osal_msg_allocate(sizeof(gp_SecRsp_t));
  
    //No memory
  if(gp_SecRsp == NULL)
  {
    retValue = FAILURE;
  }
  //Invalid application ID
  if( (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_DEFAULT) && (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_GP ) )
  {
    retValue = INVALIDPARAMETER;
  }
  //Return fail/InvalidParameter
  if(retValue)
  {
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_GP), cmdId, 1, &retValue);
    return;
  }
  
  gp_SecRsp->Status = *pBuf++;
  gp_SecRsp->dGPStubHandle = *pBuf++;
  gp_SecRsp->gpd_ID.AppID = *pBuf++;
  if(gp_SecRsp->gpd_ID.AppID == GP_APP_ID_DEFAULT)
  {
    gp_SecRsp->gpd_ID.GPDId.SrcID = osal_build_uint32(pBuf,4);
    pBuf += sizeof(uint32) + Z_EXTADDR_LEN;
  }
  else
  {
    pBuf += sizeof(uint32);
    osal_memcpy(gp_SecRsp->gpd_ID.GPDId.GPDExtAddr,pBuf,Z_EXTADDR_LEN);
    pBuf += Z_EXTADDR_LEN;
  }
  gp_SecRsp->EndPoint = *pBuf++;
  gp_SecRsp->gp_SecData.GPDFSecLvl = *pBuf++;
  gp_SecRsp->gp_SecData.GPDFKeyType = *pBuf++; 
  osal_memcpy(gp_SecRsp->GPDKey,pBuf,SEC_KEY_LEN);
  pBuf += SEC_KEY_LEN;
  gp_SecRsp->gp_SecData.GPDSecFrameCounter = osal_build_uint32(pBuf,4);
  
  gp_SecRsp->hdr.event = GP_SEC_RSP;
  gp_SecRsp->hdr.status = 0;
  
  osal_msg_send(gp_TaskID,(uint8*)gp_SecRsp);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_GP), cmdId, 1, &retValue);
}

#endif

/***************************************************************************************************
* @fn      MT_GPDataInd
*
* @brief   Send GP Data Ind to Host Processor
*
* @param   gp_DataInd
*
* @return  void
***************************************************************************************************/
void MT_GPDataInd(gp_DataInd_t* gp_DataInd)
{
#ifdef MT_GP_CB_FUNC  
  uint8 *pBuf = NULL;
  uint8 *tempBuf = NULL;
  uint8 bufLen = sizeof(gp_DataInd_t) + gp_DataInd->GPDasduLength - 1;
  
  pBuf = osal_mem_alloc(bufLen);

  if(pBuf == NULL)
  {
    return;
  }

  tempBuf = pBuf;
  
  *pBuf++ = gp_DataInd->status;
  *pBuf++ = gp_DataInd->Rssi;
  *pBuf++ = gp_DataInd->LinkQuality;  
  *pBuf++ = gp_DataInd->SeqNumber;  
  *pBuf++ = gp_DataInd->srcAddr.addrMode;
  *pBuf++ = LO_UINT16(gp_DataInd->srcPanID);
  *pBuf++ = HI_UINT16(gp_DataInd->srcPanID);
  
   pBuf   = osal_cpyExtAddr( pBuf,gp_DataInd->srcAddr.addr.extAddr);
  *pBuf++ = gp_DataInd->appID;
  *pBuf++ = gp_DataInd->GPDFSecLvl;
  *pBuf++ = gp_DataInd->GPDFKeyType;
  *pBuf++ = gp_DataInd->AutoCommissioning;
  *pBuf++ = gp_DataInd->RxAfterTx;

  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_DataInd->SrcId);
  *pBuf++ = gp_DataInd->EndPoint;
  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_DataInd->GPDSecFrameCounter);
  *pBuf++ = gp_DataInd->GPDCmmdID;
  *pBuf++ = gp_DataInd->GPDasduLength;
  osal_memcpy(pBuf,gp_DataInd->GPDasdu,gp_DataInd->GPDasduLength);
  pBuf += gp_DataInd->GPDasduLength;
  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_DataInd->MIC);  
  
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_GP), MT_GP_DATA_IND, 36 + gp_DataInd->GPDasduLength - 1, tempBuf);

  osal_mem_free(tempBuf);
#else
  (void)gp_DataInd;
#endif
}




/***************************************************************************************************
* @fn      MT_GPDataCnf
*
* @brief   Send GP Data Cnf to Host Processor
*
* @param   gp_DataCnf
*
* @return  void
***************************************************************************************************/
void MT_GPDataCnf(gp_DataCnf_t* gp_DataCnf)
{
#ifdef MT_GP_CB_FUNC
  uint8 buf[2];
  uint8 *pBuf = buf;

  *pBuf++ = gp_DataCnf->status;
  *pBuf++ = gp_DataCnf->GPEPhandle;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_GP), MT_GP_DATA_CNF, 2, buf);
#else
  (void)gp_DataCnf;
#endif
}

/***************************************************************************************************
* @fn      MT_GPSecReq
*
* @brief   Send GP Sec Req to Host Processor
*
* @param   gp_SecReq
*
* @return  void
***************************************************************************************************/
void MT_GPSecReq(gp_SecReq_t* gp_SecReq)
{
#ifdef MT_GP_CB_FUNC
  uint8 buf[21];
  uint8 *pBuf = buf;

  *pBuf++ = gp_SecReq->gpd_ID.AppID;

  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_SecReq->gpd_ID.GPDId.SrcID);
  pBuf   = osal_cpyExtAddr( pBuf,gp_SecReq->gpd_ID.GPDId.GPDExtAddr);
  *pBuf++ = gp_SecReq->EndPoint;  
  *pBuf++ = gp_SecReq->gp_SecData.GPDFSecLvl;
  *pBuf++ = gp_SecReq->gp_SecData.GPDFKeyType;

  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_SecReq->gp_SecData.GPDSecFrameCounter);

  *pBuf++ = gp_SecReq->dGPStubHandle;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_GP), MT_GP_SEC_REQ, 21, buf);
#else
  (void)gp_SecReq;
#endif
}

/***************************************************************************************************
* @fn      MT_GPAddressConflict
*
* @brief   Send Network Status with address conflict
*
* @param   pBuf
*
* @return  void
****************************************************************************************************/
void MT_GPAddressConflict(uint8* pBuf)
{
  uint16 addr;
  
  pBuf += MT_RPC_FRAME_HDR_SZ;
  addr = osal_build_uint16(pBuf);
  
  // Do address conflict resolution
  NLME_SendNetworkStatus( NWK_BROADCAST_SHORTADDR_DEVRXON,
                      addr, NWKSTAT_ADDRESS_CONFLICT, TRUE );
}

