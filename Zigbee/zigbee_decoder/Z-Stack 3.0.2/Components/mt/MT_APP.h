/***************************************************************************************************
  Filename:       MT_APP.h
  Revised:        $Date: 2014-11-19 13:29:24 -0800 (Wed, 19 Nov 2014) $
  Revision:       $Revision: 41175 $

  Description:

  Copyright 2008-2014 Texas Instruments Incorporated. All rights reserved.

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

 ***************************************************************************************************/
#ifndef MT_APP_H
#define MT_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "OSAL.h"
#include "AF.h"


/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/

#define MT_APP_PB_ZCL_CMD_MSG 0x00 // MT_APP_PB_ZCL_CMD message
#define MT_APP_PB_ZCL_CMD_CFG 0x01 // MT_APP_PB_ZCL_CMD config

#define MT_APP_PB_ZCL_MSG_HDR_LEN 13 // sizeof (uint8)  + // AppEndPoint
                                     // sizeof (uint16) + // DestAddress
                                     // sizeof (uint8)  + // DestEndpoint
                                     // sizeof (uint16) + // ClusterID
                                     // sizeof (uint8)  + // CommandID
                                     // sizeof (uint8)  + // Specific
                                     // sizeof (uint8)  + // Direction
                                     // sizeof (uint8)  + // DisableDefaultRsp
                                     // sizeof (uint16) + // ManuCode
                                     // sizeof (uint8)  + // TransSeqNum

#define MT_APP_PB_ZCL_IND_HDR_LEN 13 // sizeof (uint8)  + // AppEndPoint
                                     // sizeof (uint16) + // SrcAddress
                                     // sizeof (uint8)  + // SrcEndpoint
                                     // sizeof (uint16) + // ClusterID
                                     // sizeof (uint8)  + // CommandID
                                     // sizeof (uint8)  + // Specific
                                     // sizeof (uint8)  + // Direction
                                     // sizeof (uint8)  + // DisableDefaultRsp
                                     // sizeof (uint16) + // ManuCode
                                     // sizeof (uint8)  + // TransSeqNum

#define MT_APP_PB_ZCL_CFG_HDR_LEN  2 // sizeof (uint8)  + // AppEndPoint
                                     // sizeof (uint8)  + // Mode


/***************************************************************************************************
 * TYPEDEF
 ***************************************************************************************************/

typedef struct
{
  osal_event_hdr_t  hdr;
  uint8             endpoint;
  uint8             appDataLen;
  uint8             *appData;
} mtSysAppMsg_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint8            type;
} mtAppPB_ZCLCmd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint8            type;
  uint8            appEP;
  afAddrType_t     dstAddr;
  uint16           clusterID;
  uint8            commandID;
  uint8            specific;
  uint8            direction;
  uint8            disableDefRsp;
  uint16           manuCode;
  uint8            transSeqNum;
  uint8            appPBDataLen;
  uint8            *appPBData;
} mtAppPB_ZCLMsg_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint8            type;
  uint8            mode;
} mtAppPB_ZCLCfg_t;

typedef struct
{
  uint8  appEP;
  uint16 srcAddr;
  uint8  srcEP;
  uint16 clusterID;
  uint8  commandID;
  uint8  specific;
  uint8  direction;
  uint8  disableDefRsp;
  uint16 manuCode;
  uint8  transSeqNum;
  uint8  appPBDataLen;
  uint8  *appPBData;
} mtAppPB_ZCLInd_t;


/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/
#if defined (MT_APP_FUNC)
/*
 * Process MT_APP commands
 */
extern uint8 MT_AppCommandProcessing(uint8 *pBuf);
#endif

/*
 * Send an MT_APP_PB_ZCL_IND command
 */
extern void MT_AppPB_ZCLInd( mtAppPB_ZCLInd_t *ind );


#ifdef __cplusplus
}
#endif

#endif /* MTEL_H */

/***************************************************************************************************
 ***************************************************************************************************/
