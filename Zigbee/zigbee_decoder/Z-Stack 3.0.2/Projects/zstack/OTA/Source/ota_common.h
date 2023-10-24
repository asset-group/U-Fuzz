/******************************************************************************
  Filename:       ota_common.h
  Revised:        $Date: 2013-12-10 07:42:48 -0800 (Tue, 10 Dec 2013) $
  Revision:       $Revision: 36527 $

  Description:    This file contains code common to the OTA server,
                  client, and dongle.


  Copyright 2010-2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
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
******************************************************************************/

#ifndef OTA_COMMON_H
#define OTA_COMMON_H

#if !defined HAL_OTA_BOOT_CODE
#include "af.h"
#endif

#ifndef _MSC_VER
#include "hal_mcu.h"
#endif

// Endpoint for SYS App messages
#define OTA_SYSAPP_ENDPOINT                 20

#define OTA_APP_MAX_ATTRIBUTES              7

// SYS App message format byte positions
#define MT_APP_ENDPOINT_POS                 0
#define MT_APP_COMMAND_POS                  1
#define MT_APP_DATA_POS                     2

// SYS APP Commands for dongle communication with OTA Console
#define OTA_APP_READ_ATTRIBUTE_REQ          0
#define OTA_APP_IMAGE_NOTIFY_REQ            1
#define OTA_APP_DISCOVERY_REQ               2
#define OTA_APP_JOIN_REQ                    3
#define OTA_APP_LEAVE_REQ                   4

#define OTA_APP_READ_ATTRIBUTE_IND          0x80
#define OTA_APP_IMAGE_NOTIFY_RSP            0x81
#define OTA_APP_DEVICE_IND                  0x82
#define OTA_APP_JOIN_IND                    0x83
#define OTA_APP_ENDPOINT_IND                0x85
#define OTA_APP_DONGLE_IND                  0x8A

// Sys App Message Lengths
#define OTA_APP_READ_ATTRIBUTE_REQ_LEN      (8 + OTA_APP_MAX_ATTRIBUTES*2)
#define OTA_APP_IMAGE_NOTIFY_REQ_LEN        15
#define OTA_APP_DISCOVERY_REQ_LEN           2
#define OTA_APP_JOIN_REQ_LEN                5
#define OTA_APP_LEAVE_REQ_LEN               2

#define OTA_APP_READ_ATTRIBUTE_IND_LEN      21
#define OTA_APP_IMAGE_NOTIFY_RSP_LEN        16
#define OTA_APP_DEVICE_IND_LEN              6
#define OTA_APP_JOIN_IND_LEN                4
#define OTA_APP_ENDPOINT_IND_LEN            7
#define OTA_APP_DONGLE_IND_LEN              10

#define OTA_INVALID_ID                      0xFF

// OTA Header constants
#define OTA_HDR_MAGIC_NUMBER                0x0BEEF11E
#define OTA_HDR_BLOCK_SIZE                  128
#define OTA_HDR_STACK_VERSION               2
#define OTA_HDR_HEADER_VERSION              0x0100
#define OTA_HDR_FIELD_CTRL                  0

#define OTA_HEADER_LEN_MIN                  56
#define OTA_HEADER_LEN_MAX                  69
#define OTA_HEADER_LEN_MIN_ECDSA            166
#define OTA_HEADER_STR_LEN                  32

#define OTA_HEADER_IMAGE_SIZE_POS           52
// OTA_HEADER_FILE_ID_POS is needed for windows tools
#define OTA_HEADER_FILE_ID_POS              10

#define OTA_FC_SCV_PRESENT                  (0x1 << 0)
#define OTA_FC_DSF_PRESENT                  (0x1 << 1)
#define OTA_FC_HWV_PRESENT                  (0x1 << 2)

#define OTA_SUB_ELEMENT_HDR_LEN             6

#define OTA_UPGRADE_IMAGE_TAG_ID            0
#define OTA_ECDSA_SIGNATURE_TAG_ID          1
#define OTA_EDCSA_CERTIFICATE_TAG_ID        2

// MT_OtaGeImage options
#define MT_OTA_HW_VER_PRESENT_OPTION        0x01
#define MT_OTA_QUERY_SPECIFIC_OPTION        0x02

// MT OTA Status Indication Types
#define MT_OTA_DL_COMPLETE                  0

#if defined HAL_MCU_CC2538
#pragma pack(2)
#endif
typedef struct
{
  uint16 manufacturer;
  uint16 type;
  uint32 version;
} zclOTA_FileID_t;

typedef struct
{
  uint16 tag;
  uint32 length;
} OTA_SubElementHdr_t;

typedef struct
{
  uint32 magicNumber;
  uint16 headerVersion;
  uint16 headerLength;
  uint16 fieldControl;
  zclOTA_FileID_t fileId;
  uint16 stackVersion;
  uint8 headerString[OTA_HEADER_STR_LEN];
  uint32 imageSize;
  uint8 secCredentialVer;
  uint8 destIEEE[8];
  uint16 minHwVer;
  uint16 maxHwVer;
} OTA_ImageHeader_t;
#if defined HAL_MCU_CC2538
#pragma pack()
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8 *OTA_WriteHeader(OTA_ImageHeader_t *pHdr, uint8 *pBuf);
extern uint8 *OTA_ParseHeader(OTA_ImageHeader_t *pHdr, uint8 *pBuf);

extern void OTA_GetFileName(char *pName, zclOTA_FileID_t *pFileId, char *text);
extern void OTA_SplitFileName(char *pName, zclOTA_FileID_t *pFileId);

extern uint8 *OTA_StreamToFileId(zclOTA_FileID_t *pFileId, uint8 *pStream);
extern uint8 *OTA_FileIdToStream(zclOTA_FileID_t *pFileId, uint8 *pStream);

#if !defined HAL_OTA_BOOT_CODE
extern uint8 *OTA_AfAddrToStream(afAddrType_t *pAddr, uint8 *pStream);
extern uint8 *OTA_StreamToAfAddr(afAddrType_t *pAddr, uint8 *pStream);
#endif

#ifdef __cplusplus
}
#endif

#endif // OTA_COMMON_H
