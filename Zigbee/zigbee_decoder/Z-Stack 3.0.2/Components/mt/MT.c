/***************************************************************************************************
  Filename:       MT.c
  Revised:        $Date: 2015-01-18 19:44:10 -0800 (Sun, 18 Jan 2015) $
  Revision:       $Revision: 41896 $

  Description:    MonitorTest Event Loop functions.
                  Everything in the MonitorTest Task (except the serial driver).


  Copyright 2007-2014 Texas Instruments Incorporated. All rights reserved.

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

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/

#include "ZComDef.h"
#include "MT.h"
#include "MT_APP.h"
#include "MT_DEBUG.h"
#include "MT_UTIL.h"
#include "MT_SYS.h"

#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Memory.h"
#include "OSAL_Nv.h"

#include "DebugTrace.h"
#include "ZMAC.h"

#if !defined ( NONWK )
  #include "NLMEDE.h"
  #include "nwk_bufs.h"
  #include "ZDObject.h"
  #include "ssp.h"
  #include "nwk_util.h"
  #include "AF.h"
  #include "MT_SAPI.h"
#endif

#if defined( MT_MAC_FUNC ) || defined( MT_MAC_CB_FUNC )
  #include "MT_MAC.h"
#endif
#if defined( MT_NWK_FUNC ) || defined( MT_NWK_CB_FUNC )
  #include "MT_NWK.h"
  #include "nwk.h"
  #include "nwk_bufs.h"
#endif
#if defined( MT_AF_FUNC ) || defined( MT_AF_CB_FUNC )
  #include "MT_AF.h"
#endif
#if defined( MT_USER_TEST_FUNC )
  #include "AF.h"
#endif
#if defined( MT_ZDO_FUNC )
  #include "MT_ZDO.h"
#endif
#if defined (MT_SAPI_FUNC)
	#include "MT_SAPI.h"
#endif
#if defined (MT_OTA_FUNC)
  #include "MT_OTA.h"
#endif

#if defined( APP_TP )
 #include "TestProfile.h"
#endif
#if defined( APP_TP2 )
 #include "TestProfile2.h"
#endif

#if defined(APP_TGEN)
  #include "TrafficGenApp.h"
#endif
#if defined(APP_DEBUG)
	#include "DebugApp.h"
#endif
#if defined (NWK_TEST)
	#include "HWTTApp.h"
#endif
#if defined (MT_UBL_FUNC)
  extern uint8 MT_UblCommandProcessing(uint8 *pBuf);
#endif
#if defined (MT_ZNP_FUNC)
  #include "MT_ZNP.h"
#endif
#if defined (MT_MAC_PROTOBUF_FUNC)
  #include "mtmacpb.h"
#endif
#if defined (MT_GP_CB_FUNC)
  #include "MT_GP.h"
#endif

#if defined (MT_APP_CNF_FUNC)
  #include "MT_APP_CONFIG.h"
#endif

#include "hal_uart.h"
#include "hal_led.h"
#include "hal_key.h"
#include "MT_UART.h"

#if defined (FEATURE_DUAL_MAC)
  #include "dmmgr.h"
#endif 

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

mtProcessMsg_t mtProcessIncoming[] =
{
  NULL,                               // MT_RPC_SYS_RES0

#if defined (MT_SYS_FUNC)
  MT_SysCommandProcessing,            // MT_RPC_SYS_SYS
#else
  NULL,
#endif

#if defined (MT_MAC_FUNC)
  MT_MacCommandProcessing,            // MT_RPC_SYS_MAC
#else
  NULL,
#endif

#if defined (MT_NWK_FUNC)
  MT_NwkCommandProcessing,            // MT_RPC_SYS_NWK
#else
  NULL,
#endif

#if defined (MT_AF_FUNC)
  MT_AfCommandProcessing,             // MT_RPC_SYS_AF
#else
  NULL,
#endif

#if defined (MT_ZDO_FUNC)
  MT_ZdoCommandProcessing,            // MT_RPC_SYS_ZDO
#else
  NULL,
#endif

#if defined (MT_SAPI_FUNC)
  MT_SapiCommandProcessing,           // MT_RPC_SYS_SAPI
#else
  NULL,
#endif

#if defined (MT_UTIL_FUNC)
  MT_UtilCommandProcessing,           // MT_RPC_SYS_UTIL
#else
  NULL,
#endif

#if defined (MT_DEBUG_FUNC)
  MT_DebugCommandProcessing,          // MT_RPC_SYS_DBG
#else
  NULL,
#endif

#if defined (MT_APP_FUNC)
  MT_AppCommandProcessing,            // MT_RPC_SYS_APP
#else
  NULL,
#endif

#if defined (MT_OTA_FUNC)
  MT_OtaCommandProcessing,            // MT_RPC_SYS_OTA
#else
  NULL,
#endif

#if defined (MT_ZNP_FUNC)
  MT_ZnpCommandProcessing,
#else
  NULL,
#endif

  NULL,  // Spare sub-system 12.

#if defined (MT_UBL_FUNC)
  MT_UblCommandProcessing,
#else
  NULL,
#endif

  NULL,                               // MT_RPC_SYS_RESERVED14
  
#if defined (MT_APP_CNF_FUNC) 
  MT_AppCnfCommandProcessing,        // MT_RPC_SYS_APP_CNF
#else
  NULL,
#endif
  
  NULL,                               // MT_RPC_SYS_RESERVED16
#if defined (MT_MAC_PROTOBUF_FUNC)
  MT_MacPBCmdProc,                    // MT_RPC_SYS_PROTOBUF
#else
  NULL,
#endif
  NULL,                               // MT_RPC_SYS_RES18
  NULL,                               // MT_RPC_SYS_RES19
  NULL,                               // MT_RPC_SYS_RES20
#if defined (MT_GP_CB_FUNC)
  MT_GpCommandProcessing,
#else
  NULL,
#endif
};

/**************************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************************/

byte debugThreshold;
byte debugCompId;

/**************************************************************************************************
 * LOCAL FUNCTIONS
 **************************************************************************************************/

void MT_MsgQueueInit( void );
void MT_ResetMsgQueue( void );
byte MT_QueueMsg( byte *msg , byte len );
void MT_ProcessQueue( void );

#if defined ( MT_USER_TEST_FUNC )
void MT_ProcessAppUserCmd( byte *pData );
#endif

/**************************************************************************************************
 * @fn         MT_Init
 *
 * @brief      This function is the secondary initialization that resolves conflicts during
 *             osalInitTasks(). For example, since MT is the highest priority task, and
 *             specifically because the MT task is initialized before the ZDApp task, if MT_Init()
 *             registers anything with ZDO_RegisterForZdoCB(), it is wiped out when ZDApp task
 *             initialization invokes ZDApp_InitZdoCBFunc().
 *             There may be other existing or future such races, so try to do all possible
 *             MT initialization here vice in MT_TaskInit().
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
void MT_Init(uint8 taskID)
{
  MT_TaskID = taskID;
  debugThreshold = 0;
  debugCompId = 0;

#if defined (MT_ZDO_FUNC)
  MT_ZdoInit();
#endif
#ifndef ZBIT
  MT_SysResetInd();
#endif /* !ZBIT */
}

/***************************************************************************************************
 * @fn      MT_BuildAndSendZToolResponse
 *
 * @brief   Build and send a ZTOOL msg
 * @param   uint8 cmdType - include type and subsystem
 *          uint8 cmdId - command ID
 *          byte dataLen
 *          byte *pData
 *
 * @return  void
 ***************************************************************************************************/
#if !defined(NPI)
void MT_BuildAndSendZToolResponse(uint8 cmdType, uint8 cmdId, uint8 dataLen, uint8 *pData)
{
  uint8 *msg_ptr;

#ifdef FEATURE_DUAL_MAC
  msg_ptr = DMMGR_BuildRspMsg( cmdType, cmdId, dataLen, pData );

  if ( msg_ptr )
  {
    MT_TransportSend(msg_ptr);
  }
#else
  if ((msg_ptr = MT_TransportAlloc((mtRpcCmdType_t)(cmdType & 0xE0), dataLen)) != NULL)
  {
    msg_ptr[MT_RPC_POS_LEN] = dataLen;
    msg_ptr[MT_RPC_POS_CMD0] = cmdType;
    msg_ptr[MT_RPC_POS_CMD1] = cmdId;
    (void)osal_memcpy(msg_ptr+MT_RPC_POS_DAT0, pData, dataLen);

    MT_TransportSend(msg_ptr);
  }
#endif /* FEATURE_DUAL_MAC */
}
#endif /* NPI */
/***************************************************************************************************
 * @fn      MT_ProcessIncoming
 *
 * @brief  Process Incoming Message.
 *
 * @param   byte *pBuf - pointer to event message
 *
 * @return  void
 ***************************************************************************************************/
void MT_ProcessIncoming(uint8 *pBuf)
{
  mtProcessMsg_t func;
  uint8 rsp[MT_RPC_FRAME_HDR_SZ];

  /* pre-build response message:  | status | cmd0 | cmd1 | */
  rsp[1] = pBuf[MT_RPC_POS_CMD0];
  rsp[2] = pBuf[MT_RPC_POS_CMD1];

  /* check length */
  if (pBuf[MT_RPC_POS_LEN] > MT_RPC_DATA_MAX)
  {
    rsp[0] = MT_RPC_ERR_LENGTH;
  }
  /* check subsystem range */
  else if ((rsp[1] & MT_RPC_SUBSYSTEM_MASK) < MT_RPC_SYS_MAX)
  {
    /* look up processing function */
    func = mtProcessIncoming[rsp[1] & MT_RPC_SUBSYSTEM_MASK];
    if (func)
    {
      /* execute processing function */
      rsp[0] = (*func)(pBuf);
    }
    else
    {
      rsp[0] = MT_RPC_ERR_SUBSYSTEM;
    }
  }
  else
  {
    rsp[0] = MT_RPC_ERR_SUBSYSTEM;
  }

  /* if error and this was an SREQ, send error message */
  if ((rsp[0] != MT_RPC_SUCCESS) && ((rsp[1] & MT_RPC_CMD_TYPE_MASK) == MT_RPC_CMD_SREQ))
  {
    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_RES0), 0,
                                                                  MT_RPC_FRAME_HDR_SZ, rsp);
  }
}

/***************************************************************************************************
 * @fn      MTProcessAppRspMsg
 *
 * @brief   Process the User App Response Message
 *
 * @param   data - output serial buffer.  The first byte must be the
 *          endpoint that send this message.
 * @param   len - data length
 *
 * @return  none
 ***************************************************************************************************/
void MTProcessAppRspMsg( byte *pData, byte len )
{
  /* Send out Reset Response message */
  MT_BuildAndSendZToolResponse( ((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP), MT_APP_RSP, len, pData );
}


/***************************************************************************************************
 * @fn      MT_ReverseBytes
 *
 * @brief
 *
 *   Reverses bytes within an array
 *
 * @param   data - ptr to data buffer to reverse
 * @param    len - number of bytes in buffer
 *
 * @return  void
 ***************************************************************************************************/
void MT_ReverseBytes( byte *pData, byte len )
{
  byte i,j;
  byte temp;

  for ( i = 0, j = len-1; len > 1; len-=2 )
  {
    temp = pData[i];
    pData[i++] = pData[j];
    pData[j--] = temp;
  }
}


/***************************************************************************************************
 * @fn      MT_Word2Buf
 *
 * @brief   Copy a uint16 array to a byte array, little endian.
 *
 * @param   pBuf - byte array
 * @param   pWord - uint16 array
 * @param   len - length of uint16 array
 *
 * @return  pointer to end of byte array
 ***************************************************************************************************/
uint8 *MT_Word2Buf( uint8 *pBuf, uint16 *pWord, uint8 len )
{
  while ( len-- > 0 )
  {
    *pBuf++ = LO_UINT16( *pWord );
    *pBuf++ = HI_UINT16( *pWord );
    pWord++;
  }

  return pBuf;
}
#if !defined(NONWK)
/***************************************************************************************************
 * @fn      MT_BuildEndpointDesc
 *
 * @brief   Build endpoint descriptor and simple descriptor structure from incoming buffer
 *
 * @param   pBuf - byte array
 *
 * @return  epDesc - pointer to the endpoint descriptor
 ***************************************************************************************************/
uint8 MT_BuildEndpointDesc( uint8 *pBuf, void *param )
{
  uint8 i;
  uint8 ret = ZSuccess;
  endPointDesc_t *epDesc;

  epDesc = (endPointDesc_t *)param;
  /* check if this endpoint is already registered */
  if ( afFindEndPointDesc( *pBuf ) != NULL )
  {
    ret = ZApsDuplicateEntry;
  }
  else if ( epDesc )
  {
    epDesc->endPoint = *pBuf;

    /* Ignore the latency reqs */
    epDesc->latencyReq = noLatencyReqs;

    /* allocate memory for the simple descriptor */
    epDesc->simpleDesc = (SimpleDescriptionFormat_t *) osal_mem_alloc(sizeof(SimpleDescriptionFormat_t));
    if (epDesc->simpleDesc)
    {
      /* Endpoint */
      epDesc->simpleDesc->EndPoint = *pBuf++;

      /* AppProfId */
      epDesc->simpleDesc->AppProfId = BUILD_UINT16(pBuf[0], pBuf[1]);
      pBuf += sizeof(uint16);

      /* AppDeviceId */
      epDesc->simpleDesc->AppDeviceId = BUILD_UINT16(pBuf[0],pBuf[1]);
      pBuf += sizeof(uint16);

      /* AppDevVer */
      epDesc->simpleDesc->AppDevVer = (*pBuf++) & AF_APP_DEV_VER_MASK ;

      /* LatencyReq */
      pBuf++;

      /* AppNumInClusters */
      epDesc->simpleDesc->AppNumInClusters = *pBuf++;
      if (epDesc->simpleDesc->AppNumInClusters)
      {
        epDesc->simpleDesc->pAppInClusterList = (uint16 *)
                  osal_mem_alloc((epDesc->simpleDesc->AppNumInClusters)*sizeof(uint16));
        if ( epDesc->simpleDesc->pAppInClusterList )
        {
          for (i=0; i<(epDesc->simpleDesc->AppNumInClusters); i++)
          {
            epDesc->simpleDesc->pAppInClusterList[i] = BUILD_UINT16(*pBuf, *(pBuf+1));
            pBuf += 2;
          }
        }
        else
        {
          ret = ZMemError;
        }
      }

      /* AppNumOutClusters */
      epDesc->simpleDesc->AppNumOutClusters = *pBuf++;
      if (epDesc->simpleDesc->AppNumOutClusters)
      {
        epDesc->simpleDesc->pAppOutClusterList = (uint16 *)
                          osal_mem_alloc((epDesc->simpleDesc->AppNumOutClusters)*sizeof(uint16));
        if (epDesc->simpleDesc->pAppOutClusterList)
        {
          for (i=0; i<(epDesc->simpleDesc->AppNumOutClusters); i++)
          {
            epDesc->simpleDesc->pAppOutClusterList[i] = BUILD_UINT16(*pBuf, *(pBuf+1));
            pBuf += 2;
          }
        }
        else
        {
          ret = ZMemError;
        }
      }

      /* if any list cannot be allocated...free all */
      if ( ret == ZMemError )
      {
        if (epDesc->simpleDesc->pAppInClusterList)
        {
          osal_mem_free(epDesc->simpleDesc->pAppInClusterList);
        }

        if (epDesc->simpleDesc->AppNumOutClusters)
        {
          osal_mem_free(epDesc->simpleDesc->pAppOutClusterList);
        }

        osal_mem_free(epDesc->simpleDesc);
      }
    }
    else
    {
      ret = ZMemError;
    }
  }

  return ret;
}
#endif
/***************************************************************************************************
***************************************************************************************************/
