/**************************************************************************************************
  Filename:       MT_OTA.c
  Revised:        $Date: 2013-07-18 12:30:24 -0700 (Thu, 18 Jul 2013) $
  Revision:       $Revision: 34729 $

  Description:    MonitorTest functions for the ZCL OTA Upgrade.


  Copyright 2004-2007 Texas Instruments Incorporated. All rights reserved.

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

#ifdef MT_OTA_FUNC

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "ZComDef.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_OTA.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/**************************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************************/
uint8 OTA_Task = 0xFF;

/**************************************************************************************************
 * LOCAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 * LOCAL FUNCTIONS
 **************************************************************************************************/

/***************************************************************************************************
 * @fn      MT_OtaRegister
 *
 * @brief   Called to set the task to receive callbacks from the MT OTA.
 *
 * @param   taskId - task identifier
 *
 * @return  void
 ***************************************************************************************************/
void MT_OtaRegister(uint8 taskId)
{
  OTA_Task = taskId;
}

/***************************************************************************************************
 * @fn      MT_OtaCommandProcessing
 *
 * @brief   Process all the MT OTA commands that are issued by the OTA Console tool
 *
 * @param   pBuf - pointer to the msg buffer
 *
 *          | LEN  | CMD0  | CMD1  |  DATA  |
 *          |  1   |   1   |   1   |  0-255 |
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_OtaCommandProcessing(uint8* pBuf)
{
  uint8 status = MT_RPC_SUCCESS;
  uint8 len;
  OTA_MtMsg_t *pMsg;
  uint8 cmd = pBuf[MT_RPC_POS_CMD1];

  if (cmd == MT_OTA_FILE_READ_RSP || cmd == MT_OTA_NEXT_IMG_RSP)
  {
    // Forward the message to the task
    if (OTA_Task != 0xff)
    {
      len = pBuf[MT_RPC_POS_LEN];
      pMsg = (OTA_MtMsg_t*) osal_msg_allocate(len + sizeof(OTA_MtMsg_t));
        
      if (pMsg)
      {
        pMsg->hdr.event = MT_SYS_OTA_MSG;
        pMsg->cmd = cmd;
        
        osal_memcpy(pMsg->data, &pBuf[MT_RPC_POS_DAT0], len);        
        osal_msg_send(OTA_Task, (uint8*) pMsg);
      }
    }
  }
  else
  {
    status = MT_RPC_ERR_COMMAND_ID;
  }

  return status;
}

/***************************************************************************************************
 * @fn      MT_OtaFileReadReq
 *
 * @brief   Requests a block of a file be read from the remote.
 *
 * @param   pAddr - The addres of the device requsting the data
 * @param   pFileId - Teh id of the image to read from
 * @param       len - Amount of data to read (must be smaller than the max MT message payload len)
 * @param    offset - The offset into the image to start reading from
 *
 * @return  status
 ***************************************************************************************************/
uint8 MT_OtaFileReadReq(afAddrType_t *pAddr, zclOTA_FileID_t *pFileId, uint8 len, uint32 offset)
{
  uint8   msgLen;
  uint8   *pBuf;
  uint8   *p;

  // Check if the requested length is longer than the RX receive buffer
  if (len + MT_OTA_FILE_READ_RSP_LEN + SPI_0DATA_MSG_LEN > MT_UART_RX_BUFF_MAX)
    return 0;
  
  // Get length
  msgLen = MT_OTA_FILE_READ_REQ_LEN;
  
  // Allocate a buffer
  if ((p = pBuf = MT_TransportAlloc(0, msgLen)) != NULL)
  {
    /* build header */
    *p++ = msgLen;
    *p++ = (uint8) MT_RPC_CMD_AREQ | (uint8) MT_RPC_SYS_OTA;
    *p++ = MT_OTA_FILE_READ_REQ;
    
    // Add the file ID
    p = OTA_FileIdToStream(pFileId, p);

    // Add the device address 
    p = OTA_AfAddrToStream(pAddr, p);

    // File ofset to read from
    *p++ = BREAK_UINT32(offset, 0);
    *p++ = BREAK_UINT32(offset, 1);
    *p++ = BREAK_UINT32(offset, 2);
    *p++ = BREAK_UINT32(offset, 3);

    *p = len;
    
    // Send command to server
    MT_TransportSend(pBuf);
    
    return ZSuccess;
  }
  
  return ZMemError;
}

/***************************************************************************************************
 * @fn      MT_OtaGetImage
 *
 * @brief   Requests the next OTA image for a given device.
 *
 * @param   pAddr - Address of the device requesting the image
 * @param   pFileId - The file ID of the image currently on the device
 * @param   hwVer - The hardware version of the device (optional)
 * @param   ieee - The IEEE address of the device (optional)
 * @param   options - The get image options
 *
 * @return  Status
 *
 ***************************************************************************************************/
uint8 MT_OtaGetImage(afAddrType_t *pAddr, zclOTA_FileID_t *pFileId, uint16 hwVer, 
                     uint8 *ieee, uint8 options)
{
  uint8   msgLen;
  uint8   *pBuf;
  uint8   *p;

  // Get length
  msgLen = MT_OTA_GET_IMG_MSG_LEN;
  
  // Allocate a buffer
  if ((p = pBuf = MT_TransportAlloc(0, msgLen)) != NULL)
  {
    // build header
    *p++ = msgLen;
    *p++ = (uint8) MT_RPC_CMD_AREQ | (uint8) MT_RPC_SYS_OTA;
    *p++ = MT_OTA_NEXT_IMG_REQ;
    
    // Add the file ID
    p = OTA_FileIdToStream(pFileId, p);

    // Add the device address 
    p = OTA_AfAddrToStream(pAddr, p);

    // Add the options
    *p++ = options;

    // Add the hardware ID (optional)
    *p++ = LO_UINT16(hwVer);
    *p = HI_UINT16(hwVer);
    
    if (ieee)
      osal_memcpy(p, ieee, Z_EXTADDR_LEN);
  
    // Send command to server
    MT_TransportSend(pBuf);
    
    return ZSuccess;
  }
  
  return ZMemError;
}

/***************************************************************************************************
 * @fn      MT_OtaSendStatus
 *
 * @brief   Sends the status of the OTA transfer. 
 *          eg. "MT_OTA_DL_COMPLETE" to the PC OTA Console tool
 *
 * @param   shortAddr - Short Address of the device the status relates to
 * @param   type - The type of status being reported
 * @param   status - The status value
 * @param   optional - Optional type specific additional information
 *
 * @return  status
 *
 ***************************************************************************************************/
uint8 MT_OtaSendStatus(uint16 shortAddr, uint8 type, uint8 status, uint8 optional)
{
  uint8   msgLen;
  uint8   *pBuf;
  uint8   *p;

  // Get length
  msgLen = 7;
  
  // Allocate a buffer
  if ((p = pBuf = MT_TransportAlloc(0, msgLen)) != NULL)
  {
    // build header
    *p++ = msgLen;
    *p++ = (uint8) MT_RPC_CMD_AREQ | (uint8) MT_RPC_SYS_OTA;
    *p++ = MT_OTA_STATUS_IND;
    
    // Add message parameters
    *p++ = LO_UINT16(_NIB.nwkPanId);
    *p++ = HI_UINT16(_NIB.nwkPanId);
    *p++ = LO_UINT16(shortAddr);
    *p++ = HI_UINT16(shortAddr);
    *p++ = type;
    *p++ = status;
    *p = optional;
  
    // Send command to server
    MT_TransportSend(pBuf);
    
    return ZSuccess;
  }
  
  return ZMemError;  
}

#endif   // MT_OTA_FUNC
