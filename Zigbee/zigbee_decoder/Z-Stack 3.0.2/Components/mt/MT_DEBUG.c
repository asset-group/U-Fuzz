/***************************************************************************************************
  Filename:       MT_DEBUG.c
  Revised:        $Date: 2013-07-29 05:42:52 -0700 (Mon, 29 Jul 2013) $
  Revision:       $Revision: 34794 $

  Description:    MonitorTest functions for debug utilities.

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
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
#include "MT_DEBUG.h"
#include "MT_UART.h"
#include "mac_main.h"
#include "mac_data.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "mac_radio_defs.h"
#include "OSAL_Nv.h"

#include "bdb.h"
#if (ZG_BUILD_COORDINATOR_TYPE)
#include "ssp_hash.h"
#include "APSMEDE.h"
#endif

#include "ZGlobals.h"
#include "nwk_util.h"
#ifdef APP_TP2
#include "ZDConfig.h"
#endif


/***************************************************************************************************
 * LOCAL FUNCTIONs
 ***************************************************************************************************/

#if defined (MT_DEBUG_FUNC)
static void MT_DebugSetThreshold(uint8 *pBuf);
#if defined ( APP_TP2 )
  extern uint8 TP2_securityEnabled;
  extern uint8 zgAllowRejoins;
  static void MT_TP2_EnableApsSecurity(uint8 *pBuf);
  static void MT_TP2_SetR20NodeDesc(uint8 *pBuf);
  #define EN_SECURITY                     0x40
#endif


static void MT_DebugMacDataDump(void);
#endif


#if defined (MT_DEBUG_FUNC)
/***************************************************************************************************
 * @fn      MT_DebugProcessing
 *
 * @brief   Process all the DEBUG commands that are issued by test tool
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_DebugCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_DEBUG_SET_THRESHOLD:
      MT_DebugSetThreshold(pBuf);
      break;

      

#if defined ( APP_TP2 )
  case MT_DEBUG_TP2_ENABLEAPSSECURITY:
    MT_TP2_EnableApsSecurity(pBuf);
  break;
  case MT_DEBUG_TP2_SET_NODE_R20:
    MT_TP2_SetR20NodeDesc(pBuf);
  break;
#endif


    case MT_DEBUG_MAC_DATA_DUMP:
      MT_DebugMacDataDump();
      break;

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_DebugSetThreshold
 *
 * @brief   Process Debug Set Threshold
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_DebugSetThreshold(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Populate info */
  debugCompId = *pBuf++;
  debugThreshold = *pBuf;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

#if defined ( APP_TP2 )
/***************************************************************************************************
 * @fn      MT_TP2_EnableApsSecurity
 *
 * @brief   Set the APS security on TP2 messages
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_EnableApsSecurity(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
 
  if(*pBuf)
  {
    TP2_securityEnabled = EN_SECURITY;
  }
  else
  {
    TP2_securityEnabled = 0;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}

/***************************************************************************************************
 * @fn      MT_TP2_SetR20NodeDesc
 *
 * @brief   Set the device rev R20
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_TP2_SetR20NodeDesc(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  uint8 revPositionBit = 0x09;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  
  ZDO_Config_Node_Descriptor.ServerMask &= ~(0x01 << revPositionBit);
   
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_DBG), cmdId, 1, &retValue);
}
#endif


/***************************************************************************************************
 * @fn      MT_DebugMacDataDump
 *
 * @brief   Process the debug MAC Data Dump request.
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_DebugMacDataDump(void)
{
  uint8 buf[sizeof(mtDebugMacDataDump_t)];
  uint8 *pBuf = buf;

#ifdef FEATURE_PACKET_FILTER_STATS
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 0);
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 1);
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 2);
  *pBuf++ = BREAK_UINT32(nwkInvalidPackets, 3);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 0);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 1);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 2);
  *pBuf++ = BREAK_UINT32(rxCrcFailure, 3);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 0);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 1);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 2);
  *pBuf++ = BREAK_UINT32(rxCrcSuccess, 3);
#endif
#if defined MAC_RADIO_CC2520
  *pBuf++ = macSpiReadReg(FSMSTAT0);
  *pBuf++ = macSpiReadReg(FSMSTAT1);
#else
  *pBuf++ = FSMSTAT0;
  *pBuf++ = FSMSTAT1;
#endif
  *pBuf++ = macData.rxCount;
  *pBuf++ = macData.directCount;
  *pBuf++ = macMain.state;
  *pBuf++ = macRxActive;
  *pBuf   = macTxActive;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_DBG),
                                       MT_DEBUG_MAC_DATA_DUMP, sizeof(buf), buf);
}
#endif

/***************************************************************************************************
 * @fn      MT_ProcessDebugMsg
 *
 * @brief   Build and send a debug message.
 *
 * @param   byte *data - pointer to the data portion of the debug message
 *
 * @return  void
 ***************************************************************************************************/
void MT_ProcessDebugMsg( mtDebugMsg_t *msg )
{
  byte *msg_ptr;
  byte dataLen;
  uint8 buf[11];
  uint8 *pBuf;

  /* Calculate the data length based */
  dataLen = 5 + (msg->numParams * sizeof ( uint16 ));

  /* Get a message buffer to build the debug message */
  msg_ptr = osal_msg_allocate( (byte)(SPI_0DATA_MSG_LEN + dataLen + 1) );
  if ( msg_ptr )
  {
    /* Build the message */
    pBuf = buf;
    *pBuf++ = msg->compID;
    *pBuf++ = msg->severity;
    *pBuf++ = msg->numParams;

    if ( msg->numParams >= 1 )
    {
      *pBuf++ = LO_UINT16( msg->param1 );
      *pBuf++ = HI_UINT16( msg->param1 );
    }

    if ( msg->numParams >= 2 )
    {
      *pBuf++ = LO_UINT16( msg->param2 );
      *pBuf++ = HI_UINT16( msg->param2 );
    }

    if ( msg->numParams == 3 )
    {
      *pBuf++ = LO_UINT16( msg->param3 );
      *pBuf++ = HI_UINT16( msg->param3 );
    }

    *pBuf++ = LO_UINT16( msg->timestamp );
    *pBuf = HI_UINT16( msg->timestamp );

#ifdef MT_UART_DEFAULT_PORT
    /* Debug message is set to AREQ CMD 0x80 for now */
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_DBG), 0x80, dataLen, buf);
#endif
    osal_msg_deallocate( msg_ptr );
  }
}

/***************************************************************************************************
 * @fn      MT_ProcessDebugStr
 *
 * @brief   Build and send a debug string.
 *
 * @param   byte *dstr - pointer to the data portion of the debug message
 *
 * @return  void
 ***************************************************************************************************/
void MT_ProcessDebugStr(mtDebugStr_t *dstr)
{
  byte *msg_ptr;

  /* Get a message buffer to build the debug message */
  msg_ptr = osal_mem_alloc( (byte)(SPI_0DATA_MSG_LEN + dstr->strLen) );
  if ( msg_ptr )
  {
#ifdef MT_UART_DEFAULT_PORT
    /* Debug message is set to AREQ CMD 0x80 for now */
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_DBG), MT_DEBUG_MSG, dstr->strLen, dstr->pString);
#endif
    osal_mem_free( msg_ptr );
  }
}

/**************************************************************************************************
 */
