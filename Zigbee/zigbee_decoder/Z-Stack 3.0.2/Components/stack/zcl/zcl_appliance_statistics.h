/**************************************************************************************************
  Filename:       zcl_appliance_statistics.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Statistics definitions.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_APPLIANCE_STATISTICS_H
#define ZCL_APPLIANCE_STATISTICS_H

#ifdef ZCL_APPLIANCE_STATISTICS

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"

/******************************************************************************
 * CONSTANTS
 */

/************************************************/
/***  Appliance Statistics Cluster Attributes ***/
/************************************************/

// Server Attributes
#define ATTRID_APPLIANCE_STATISTICS_LOG_MAX_SIZE                             0x0000  // M, R, UINT32
#define ATTRID_APPLIANCE_STATISTICS_LOG_QUEUE_MAX_SIZE                       0x0001  // M, R, UINT8


// Server Attribute Defaults
#define ATTR_DEFAULT_APPLIANCE_STATISTICS_LOG_MAX_SIZE                       0x0000003C
#define ATTR_DEFAULT_APPLIANCE_STATISTICS_LOG_QUEUE_MAX_SIZE                 0x01

// Server commands received (Server-to-Client in ZCL Header)
#define COMMAND_APPLIANCE_STATISTICS_LOG_NOTIFICATION                   0x00  // M, zclApplianceStatisticsLogNotification_t
#define COMMAND_APPLIANCE_STATISTICS_LOG_RSP                            0x01  // M, zclApplianceStatisticsLogRsp_t
#define COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_RSP                      0x02  // M, zclApplianceStatisticsLogQueueRsp_t
#define COMMAND_APPLIANCE_STATISTICS_STATISTICS_AVAILABLE               0x03  // M, zclApplianceStatisticsStatisticsAvailable_t

// Client commands generated (Client-to-Server in ZCL Header)
#define COMMAND_APPLIANCE_STATISTICS_LOG_REQ                            0x00  // M, zclApplianceStatisticsLogReq_t
#define COMMAND_APPLIANCE_STATISTICS_LOG_QUEUE_REQ                      0x01  // M, no payload

/*******************************************************************************
 * TYPEDEFS
 */

/*** SERVER COMMANDS GENERATED ***/

/*** ZCL Appliance Statistics Log Notification payload ***/
typedef struct
{
  uint32 timeStamp;
  uint32 logID;
  uint32 logLength;
  uint8 *pLogPayload;
} zclCmdApplianceStatisticsLogNotificationPayload_t;

typedef struct
{
  uint32 timeStamp;
  uint32 logID;
  uint32 logLength;
  uint8 *pLogPayload;
} zclCmdApplianceStatisticsLogRspPayload_t;

typedef struct
{
  uint8 logQueueSize;
  uint32 *pLogID;
} zclCmdApplianceStatisticsLogQueueRspPayload_t;

typedef struct
{
  uint8 logQueueSize;
  uint32 *pLogID;
} zclCmdApplianceStatisticsStatisticsAvailablePayload_t;

/*** CLIENT COMMANDS GENERATED ***/
typedef struct
{
  uint32 logID;
} zclApplianceStatisticsLogReq_t;

// prototypes for each command
typedef ZStatus_t (*zclAppliance_Statistics_LogNotification_t)( zclCmdApplianceStatisticsLogNotificationPayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogRsp_t)( zclCmdApplianceStatisticsLogRspPayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogQueueRsp_t)( zclCmdApplianceStatisticsLogQueueRspPayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_StatisticsAvailable_t)( zclCmdApplianceStatisticsStatisticsAvailablePayload_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogReq_t)( zclIncoming_t *pInMsg, zclApplianceStatisticsLogReq_t *pCmd );
typedef ZStatus_t (*zclAppliance_Statistics_LogQueueReq_t)( zclIncoming_t *pInMsg );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive

typedef struct
{
  zclAppliance_Statistics_LogNotification_t               pfnApplianceStatistics_LogNotification;   // server-side functions
  zclAppliance_Statistics_LogRsp_t                        pfnApplianceStatistics_LogRsp;
  zclAppliance_Statistics_LogQueueRsp_t                   pfnApplianceStatistics_LogQueueRsp;
  zclAppliance_Statistics_StatisticsAvailable_t           pfnApplianceStatistics_StatisticsAvailable;
  zclAppliance_Statistics_LogReq_t                        pfnApplianceStatistics_LogReq;            // client-side functions
  zclAppliance_Statistics_LogQueueReq_t                   pfnApplianceStatistics_LogQueueReq;
} zclApplianceStatistics_AppCallbacks_t;

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */
/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclApplianceStatistics_RegisterCmdCallbacks( uint8 endpoint, zclApplianceStatistics_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogNotification
 *
 * @brief   Request sent to client for Log Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload - command payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                              zclCmdApplianceStatisticsLogNotificationPayload_t *pPayload,
                                                              uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogRsp
 *
 * @brief   Response sent to client from LogReq cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          timeStamp - timestamp of the notification
 *          logID - identifies uniquely the log information contained in log payload
 *          logLength - indicates the length in bytes of log payload
 *          logPayload - variable length payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                     zclCmdApplianceStatisticsLogRspPayload_t *pPayload,
                                                     uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogQueueRsp
 *
 * @brief   Response sent to client from LogQueueReq cmd.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          logQueueSize - defines the # of logID records
 *          logID - variable length list of logIDs
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogQueueRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                          zclCmdApplianceStatisticsLogQueueRspPayload_t *pPayload,
                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_StatisticsAvailable
 *
 * @brief   Request sent to client for Statistics Available.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          logQueueSize - # elements in the logID list
 *          logID - variable length list of logIDs
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclApplianceStatistics_Send_StatisticsAvailable( uint8 srcEP, afAddrType_t *dstAddr,
                                                           zclCmdApplianceStatisticsStatisticsAvailablePayload_t *pPayload,
                                                           uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogReq
 *
 * @brief   Request sent to server for Log Request.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   logID - identifies uniquely the log information contained in log payload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                     uint32 logID,
                                                     uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceStatistics_Send_LogQueueReq
 *
 * @brief   Request sent to server for Log Queue Request.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceStatistics_Send_LogQueueReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif
#endif // ZCL_APPLIANCE_STATISTICS
#endif /* ZCL_APPLIANCE_STATISTICS_H */
