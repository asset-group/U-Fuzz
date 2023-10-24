/**************************************************************************************************
  Filename:       zcl_partition.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Partition Cluster definitions. The
  Partition Cluster allows up to 100K packets.

  Note: both side must agree on the following 4 attributes:
  PartitionedFrameSize (e.g. 0x40)
  LargeFrameSize (e.g. 1280 - an MTU for IPv6 for example)
  NumberOfAckFrame (e.g. 4)
  InterfaceDelay (e.g. 0x32)

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

#ifndef ZCL_PARTITION_H
#define ZCL_PARTITION_H

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

/*****************************************/
/***  Partition Cluster Attributes ***/
/*****************************************/

// Server Attributes, Section A.1.3.2 of 10-5685-00 ZigBee Telecomunications Applications Profile
#define ATTRID_PARTITION_MAX_INCOMING_TRANSFER_SIZE      0x0000   // M, R,  UINT16
#define ATTRID_PARTITION_MAX_OUTGOING_TRANSFER_SIZE      0x0001   // M, R,  UINT16
#define ATTRID_PARTITION_PARTITIONED_FRAME_SIZE          0x0002   // M, RW,  UINT8
#define ATTRID_PARTITION_LARGE_FRAME_SIZE                0x0003   // M, RW, UINT16
#define ATTRID_PARTITION_NUMBER_OF_ACK_FRAMES            0x0004   // M, RW,  UINT8
#define ATTRID_PARTITION_NACK_TIMEOUT                    0x0005   // M, R,  UINT16
#define ATTRID_PARTITION_INTERFRAME_DELAY                0x0006   // M, RW,  UINT8
#define ATTRID_PARTITION_NUMBER_OF_SEND_RETRIES          0x0007   // M, R,   UINT8
#define ATTRID_PARTITION_SENDER_TIMEOUT                  0x0008   // M, R,  UINT16
#define ATTRID_PARTITION_RECEIVER_TIMEOUT                0x0009   // M, R,  UINT16


// Server Attribute Defaults, Section A.1.3.2 of 10-5685-00 ZigBee Telecomunications Applications Profile
#define ATTR_DEFAULT_PARTITION_MAX_INCOMING_TRANSFER_SIZE      0x0500   // UINT16,
#define ATTR_DEFAULT_PARTITION_MAX_OUTGOING_TRANSFER_SIZE      0x0500   // UINT16,
#define ATTR_DEFAULT_PARTITION_PARTITIONED_FRAME_SIZE            0x50   //  UINT8,
#define ATTR_DEFAULT_PARTITION_LARGE_FRAME_SIZE                0x0500   // UINT16,
#define ATTR_DEFAULT_PARTITION_NUMBER_OF_ACK_FRAMES              0x64   //  UINT8,
#define ATTR_DEFAULT_PARTITION_NACK_TIMEOUT                     15000   // UINT16, 0x64 * 200ms * 5 = 186a0 (too large). Spec issue.
#define ATTR_DEFAULT_PARTITION_INTERFRAME_DELAY                  0x10   //  UINT8, 10ms, see A.1.3.2.7
#define ATTR_DEFAULT_PARTITION_NUMBER_OF_SEND_RETRIES            0x03   //  UINT8, 3 retries
#define ATTR_DEFAULT_PARTITION_SENDER_TIMEOUT                   60000   // UINT16, 2 * 0x64 * 200ms * 5 = 30D40 (too large). Spec issue.
#define ATTR_DEFAULT_PARTITION_RECEIVER_TIMEOUT                 60000   // UINT16, time to wait before timing out (1 minute)

// Commands received by Server
#define COMMAND_PARTITION_TRANSFER_PARTITIONED_FRAME             0x00     // M, zclCmdTransferPartitionedFrame_t
#define COMMAND_PARTITION_READ_HANDSHAKE_PARAM                   0x01     // M, zclCmdReadHandshakeFrame_t
#define COMMAND_PARTITION_WRITE_HANDSHAKE_PARAM                  0x02     // M, zclCmdWriteHandshakeFrame_t

// Commands generated by Server
#define COMMAND_PARTITION_MULTIPLE_ACK                           0x00     // M, zclCmdMultipleAckFrame_t
#define COMMAND_PARTITION_READ_HANDSHAKE_PARAM_RSP               0x01     // M, zclCmdReadHandshakeFrame_t


/*******************************************************************************
 * TYPEDEFS
 */

/*** ZCL Partition: Transfer Partitioned Frame ***/
typedef struct
{
  uint8   fragmentationOptions;
  uint16  partitionIndicator;   // total length on first block, else block index starting from 1
  uint8   frameLen;
  uint8   *pFrame;
} zclCmdTransferPartitionedFrame_t;

#define PAYLOAD_LEN_TRANSFER_PARTITIONED_FRAME    4 // not including pFrame

#define ZCL_PARTITION_OPTIONS_FIRSTBLOCK       0x01 // set true for 1st block
#define ZCL_PARTITION_OPTIONS_INDICATOR_16BIT  0x02 // use this for options to have 16-bit indicator field
#define ZCL_PARTITION_OPTIONS_INDICATOR_8BIT   0x00 // use this for options to have 8-bit indicator field

/*** ZCL Partition: Read Handshake Parameters ***/
typedef struct
{
  uint16    clusterID;
  uint8     seqNum;              // sequence # for response
  uint8     numAttrs;            // # of attributes to read
  uint16    *pAttrID;            // array of attribute IDs to read (usually 2,3,4,6)
} zclCmdReadHandshakeParam_t;

typedef struct
{
  uint16 attrID;             // attribute ID
  uint8  dataType;           // attribute data type
  uint16 attr;               // attribute data is here (8 or 16 bits)
} zclPartitionWriteRec_t;

#define PAYLOAD_LEN_WRITE_REC   5

/*** ZCL Partition: Write Handshake Parameters ***/
typedef struct
{
  uint16                  clusterID;
  uint8                   numRecords;     // # of records to write
  zclPartitionWriteRec_t *pWriteRecord;   // array of write records to write (attrs 2,3,4,6)
} zclCmdWriteHandshakeParam_t;

#define PAYLOAD_LEN_WRITE_HANDSHAKE_PARAM   2 // not including pWriteRecord or numRecords

/*** ZCL Partition: Multiple ACK ***/
typedef struct
{
  uint8    options;
  uint16   firstFrameID;      // corresponds to partitionIndicator on zclCmdTransferPartitionedFrame_t
  uint8    numNAcks;
  uint16   *pNAckID;           // array of NACK IDs
} zclCmdMultipleAck_t;

#define ZCL_PARTITION_OPTIONS_NACK_8BIT   0x00 // use this to indicate NACK IDs will fit in 8 bits
#define ZCL_PARTITION_OPTIONS_NACK_16BIT  0x01 // use this to indicate NACK IDs need 16-bits

typedef struct
{
  uint16  attrID;
  uint8   status;
  uint8   dataType;   // see ZCL_DATATYPE_UINT8 in zcl.h
  uint16  attr;       // attribute data, either uint8 or uint16
} zclPartitionReadRec_t;

/*** ZCL Partition: Read Handshake Response ***/
#define COMMAND_PARTITION_READ_HANDSHAKE_PARAM_RSP               0x01     // M, zclCmdReadHandshakeFrame_t
typedef struct
{
  uint16                  clusterID;      // cluster ID
  uint8                   numRecords;     // # of records in array below (may be 0)
  zclPartitionReadRec_t  *pReadRecord;    // array of Read Attribute Response Status records
} zclCmdReadHandshakeParamRsp_t;

// Partition Cluster callback types
typedef ZStatus_t (*zclPartition_TransferPartitionedFrame_t)( afAddrType_t *srcAddr, zclCmdTransferPartitionedFrame_t *pCmd );
typedef ZStatus_t (*zclPartition_ReadHandshakeParam_t)( afAddrType_t *srcAddr, zclCmdReadHandshakeParam_t *pCmd );
typedef ZStatus_t (*zclPartition_WriteHandshakeParam_t)( zclCmdWriteHandshakeParam_t *pCmd );
typedef ZStatus_t (*zclPartition_MultipleAck_t)( afAddrType_t *srcAddr, zclCmdMultipleAck_t *pCmd );
typedef ZStatus_t (*zclPartition_ReadHandshakeParamRsp_t)( zclCmdReadHandshakeParamRsp_t *pCmd );


// Partition Cluster callback types
typedef struct
{
  zclPartition_TransferPartitionedFrame_t   pfnPartition_TransferPartitionedFrame;
  zclPartition_ReadHandshakeParam_t         pfnPartition_ReadHandshakeParam;
  zclPartition_WriteHandshakeParam_t        pfnPartition_WriteHandshakeParam;
  zclPartition_MultipleAck_t                pfnPartition_MultipleAck;
  zclPartition_ReadHandshakeParamRsp_t      pfnPartition_ReadHandshakeParamRsp;
} zclPartition_AppCallbacks_t;


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
extern ZStatus_t zclPartition_RegisterCmdCallbacks( uint8 endpoint, zclPartition_AppCallbacks_t *callbacks );


// helper functions for ProcessIn functions and when interacting with SWAT or the MT interface
// All these helper functions returns ZSuccess if they worked (no problems with frame/buffer/memory allocation)
// All these helper functions allocate memory for their lists (e.g. pCmd->pWriteRecord). Make sure to free the list!
extern ZStatus_t zclPartition_ConvertOtaToNative_TransferPartitionedFrame( zclCmdTransferPartitionedFrame_t *pCmd, uint8 *buf, uint8 buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_ReadHandshakeParam( zclCmdReadHandshakeParam_t *pCmd, uint8 *buf, uint8 buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_WriteHandshakeParam( zclCmdWriteHandshakeParam_t *pCmd, uint8 *buf, uint8 buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_MultipleAck( zclCmdMultipleAck_t *pCmd, uint8 *buf, uint8 buflen );
extern ZStatus_t zclPartition_ConvertOtaToNative_ReadHandshakeParamRsp( zclCmdReadHandshakeParamRsp_t *pCmd, uint8 *buf, uint8 buflen );

/*********************************************************************
 * @fn      zclPartition_Send_TransferPartitionedFrame
 *
 * @brief   send a single block (partitioned frame) to a remote receiver
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - the partitioned frame
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_TransferPartitionedFrame( uint8 srcEP, afAddrType_t *dstAddr,
                                                             zclCmdTransferPartitionedFrame_t *pCmd,
                                                             uint8 disableDefaultRsp, uint8 seqNum );


/*********************************************************************
 * @fn      zclPartition_Send_ReadHandshakeParam
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - what parameters to read
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_ReadHandshakeParam( uint8 srcEP, afAddrType_t *dstAddr,
                                                       zclCmdReadHandshakeParam_t *pCmd,
                                                       uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPartition_Send_WriteHandshakeParam
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - what parameters to write
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_WriteHandshakeParam( uint8 srcEP, afAddrType_t *dstAddr,
                                                        zclCmdWriteHandshakeParam_t *pCmd,
                                                        uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPartition_Send_MultipleAck
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - multi ack response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_MultipleAck( uint8 srcEP, afAddrType_t *dstAddr,
                                                zclCmdMultipleAck_t *pCmd,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPartition_Send_ReadHandshakeParamRsp
 *
 * @brief   Call to send out Poll Control CheckIn command from ZED to ZR/ZC. The Rsp
 *          will indicate whether to stay awake or go back to sleep.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - read multi-attribues response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPartition_Send_ReadHandshakeParamRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                          zclCmdReadHandshakeParamRsp_t *pCmd,
                                                          uint8 disableDefaultRsp, uint8 seqNum );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_PARTITION_H */

