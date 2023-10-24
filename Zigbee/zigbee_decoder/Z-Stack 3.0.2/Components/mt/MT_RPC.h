/**************************************************************************************************
  Filename:       mt_rpc.h
  Revised:        $Date: 2014-06-20 15:25:38 -0700 (Fri, 20 Jun 2014) $
  Revision:       $Revision: 39136 $

  Description:    Public interface file for the RPC Transport Protocol Design.

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

#ifndef MT_RPC_H
#define MT_RPC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/

#include "hal_types.h"

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/

/* 1st byte is the length of the data field, 2nd/3rd bytes are command field. */
#define MT_RPC_FRAME_HDR_SZ   3

/* Maximum length of data in the general frame format. The upper limit is 255 because of the
 * 1-byte length protocol. But the operation limit is lower for code size and ram savings so that
 * the uart driver can use 256 byte rx/tx queues and so
 * (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + MT_UART_FRAME_OVHD) < 256
 */
#define MT_RPC_DATA_MAX       250

/* The 3 MSB's of the 1st command field byte are for command type. */
#define MT_RPC_CMD_TYPE_MASK  0xE0

/* The 5 LSB's of the 1st command field byte are for the subsystem. */
#define MT_RPC_SUBSYSTEM_MASK 0x1F

/* position of fields in the general format frame */
#define MT_RPC_POS_LEN        0
#define MT_RPC_POS_CMD0       1
#define MT_RPC_POS_CMD1       2
#define MT_RPC_POS_DAT0       3

/* Error codes */
#define MT_RPC_SUCCESS        0     /* success */
#define MT_RPC_ERR_SUBSYSTEM  1     /* invalid subsystem */
#define MT_RPC_ERR_COMMAND_ID 2     /* invalid command ID */
#define MT_RPC_ERR_PARAMETER  3     /* invalid parameter */
#define MT_RPC_ERR_LENGTH     4     /* invalid length */

#ifdef FEATURE_DUAL_MAC
#define MT_RPC_ERR_VER_UNSUPP 5     /* unsupported mt extension version */
#define MT_RPC_ERR_STACK_ID   6     /* Missing or invalid stack id */
#define MT_RPC_ERR_BUSY       7     /* System is busy */
#endif /* FEATURE_DUAL_MAC */

/***************************************************************************************************
 * TYPEDEF
 ***************************************************************************************************/

typedef enum {
  MT_RPC_CMD_POLL = 0x00,
  MT_RPC_CMD_SREQ = 0x20,
  MT_RPC_CMD_AREQ = 0x40,
  MT_RPC_CMD_SRSP = 0x60,
  MT_RPC_CMD_RES4 = 0x80,
  MT_RPC_CMD_RES5 = 0xA0,
  MT_RPC_CMD_RES6 = 0xC0,
  MT_RPC_CMD_RES7 = 0xE0
} mtRpcCmdType_t;

typedef enum {
  MT_RPC_SYS_RES0,   /* Reserved. */
  MT_RPC_SYS_SYS,
  MT_RPC_SYS_MAC,
  MT_RPC_SYS_NWK,
  MT_RPC_SYS_AF,
  MT_RPC_SYS_ZDO,
  MT_RPC_SYS_SAPI,   /* Simple API. */
  MT_RPC_SYS_UTIL,
  MT_RPC_SYS_DBG,
  MT_RPC_SYS_APP,
  MT_RPC_SYS_OTA,
  MT_RPC_SYS_ZNP,
  MT_RPC_SYS_SPARE_12,
  MT_RPC_SYS_UBL = 13,  // 13 to be compatible with existing RemoTI.
  MT_RPC_SYS_RES14,
  MT_RPC_SYS_APP_CNF,
  MT_RPC_SYS_RES16,
  MT_RPC_SYS_PROTOBUF,
  MT_RPC_SYS_RES18,  // RPC_SYS_PB_NWK_MGR
  MT_RPC_SYS_RES19,  // RPC_SYS_PB_GW
  MT_RPC_SYS_RES20,  // RPC_SYS_PB_OTA_MGR
  MT_RPC_SYS_GP = 21,
  MT_RPC_SYS_MAX     /* Maximum value, must be last */
  /* 22-32 available, not yet assigned. */
} mtRpcSysType_t;

typedef struct
{
  uint8 *(*alloc)(mtRpcCmdType_t type, uint8 len);
  void (*send)(uint8 *pBuf);
} mtTransport_t;

typedef uint8 (*mtProcessMsg_t)(uint8 *pBuf);

/***************************************************************************************************
***************************************************************************************************/

#ifdef __cplusplus
};
#endif

#endif /* MT_RPC_H */
