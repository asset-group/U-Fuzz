/**************************************************************************************************
  Filename:       zcl_ll.h
  Revised:        $Date: 2012-10-25 15:25:21 -0700 (Thu, 25 Oct 2012) $
  Revision:       $Revision: 31911 $

  Description:    This file contains the ZCL Light Link (ZLL) commissioning
                  cluster definitions.


  Copyright 2011-2012 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_LL_H
#define ZCL_LL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */

// Zigbee Light Link Profile Identification
#define ZLL_PROFILE_ID                                     0xc05e

// ZLL Basic Lighting Device IDs
#define ZLL_DEVICEID_ON_OFF_LIGHT                          0x0000
#define ZLL_DEVICEID_ON_OFF_PLUG_IN_UNIT                   0x0010
#define ZLL_DEVICEID_DIMMABLE_LIGHT                        0x0100
#define ZLL_DEVICEID_DIMMABLE_PLUG_IN_UNIT                 0x0110

// ZLL Color Lighting Device IDs
#define ZLL_DEVICEID_COLOR_LIGHT                           0x0200
#define ZLL_DEVICEID_EXTENDED_COLOR_LIGHT                  0x0210
#define ZLL_DEVICEID_COLOR_TEMPERATURE_LIGHT               0x0220

// ZLL Lighting Remotes Device IDs
#define ZLL_DEVICEID_COLOR_CONTORLLER                      0x0800
#define ZLL_DEVICEID_COLOR_SCENE_CONTROLLER                0x0810
#define ZLL_DEVICEID_NON_COLOR_CONTORLLER                  0x0820
#define ZLL_DEVICEID_NON_COLOR_SCENE_CONTROLLER            0x0830
#define ZLL_DEVICEID_CONTROL_BRIDGE                        0x0840
#define ZLL_DEVICEID_ON_OFF_SENSOR                         0x0850

// ZLL Device Version
#define ZLL_DEVICE_VERSION                                 2

/**********************************************/
/*** ZLL Commissioning Cluster Commands     ***/
/**********************************************/

// Commands received by ZLL Commissioning Cluster Server
#define COMMAND_ZLL_SCAN_REQ                               0x00
#define COMMAND_ZLL_DEVICE_INFO_REQ                        0x02
#define COMMAND_ZLL_IDENTIFY_REQ                           0x06
#define COMMAND_ZLL_RESET_TO_FN_REQ                        0x07
#define COMMAND_ZLL_NWK_START_REQ                          0x10
#define COMMAND_ZLL_NWK_JOIN_RTR_REQ                       0x12
#define COMMAND_ZLL_NWK_JOIN_ED_REQ                        0x14
#define COMMAND_ZLL_NWK_UPDATE_REQ                         0x16
#define COMMAND_ZLL_GET_GRP_IDS_REQ                        0x41
#define COMMAND_ZLL_GET_EP_LIST_REQ                        0x42

// Commands received by ZLL Commissioning Cluster Client
#define COMMAND_ZLL_SCAN_RSP                               0x01
#define COMMAND_ZLL_DEVICE_INFO_RSP                        0x03
#define COMMAND_ZLL_NWK_START_RSP                          0x11
#define COMMAND_ZLL_NWK_JOIN_RTR_RSP                       0x13
#define COMMAND_ZLL_NWK_JOIN_ED_RSP                        0x15
#define COMMAND_ZLL_EP_INFO                                0x40
#define COMMAND_ZLL_GET_GRP_IDS_RSP                        0x41
#define COMMAND_ZLL_GET_EP_LIST_RSP                        0x42

// Request command lengths
#define ZLL_CMDLEN_SCAN_REQ                                6
#define ZLL_CMDLEN_DEVICE_INFO_REQ                         5
#define ZLL_CMDLEN_IDENTIFY_REQ                            6
#define ZLL_CMDLEN_RESET_TO_FN_REQ                         4
#define ZLL_CMDLEN_NWK_START_REQ                           56
#define ZLL_CMDLEN_NWK_JOIN_REQ                            47
#define ZLL_CMDLEN_NWK_UPDATE_REQ                          18
#define ZLL_CMDLEN_GET_GRP_IDS_REQ                         1
#define ZLL_CMDLEN_GET_EP_LIST_REQ                         1

// Response command lengths
#define ZLL_CMDLEN_SCAN_RSP                                29
#define ZLL_CMDLENOPTIONAL_SCAN_RSP                        7
#define ZLL_CMDLEN_DEVICE_INFO_RSP                         7
#define ZLL_CMDLENOPTIONAL_DEVICE_INFO_RSP                 16
#define ZLL_CMDLEN_NWK_START_RSP                           17
#define ZLL_CMDLEN_NWK_JOIN_RSP                            5
#define ZLL_CMDLEN_EP_INFO                                 16
#define ZLL_CMDLEN_GET_GRP_IDS_RSP                         3
#define ZLL_CMDLENOPTIONAL_GET_GRP_IDS_RSP                 3
#define ZLL_CMDLEN_GET_EP_LIST_RSP                         3
#define ZLL_CMDLENOPTIONAL_GET_EP_LIST_RSP                 8

// ZigBee information bit mask
#define ZLL_ZINFO_LOGICAL_TYPE                             0x03
#define ZLL_ZINFO_RX_ON_WHILE_IDLE                         0x04

// ZLL information bit mask
#define ZLL_ZLLINFO_FACTORY_NEW                            0x01
#define ZLL_ZLLINFO_ADDR_ASSIGN                            0x02
#define ZLL_ZLLINFO_LINK_INITIATOR                         0x10
#define ZLL_ZLLINFO_LINK_TIME_WINDOW                       0x20

#define ZLL_DEVICE_INFO_RSP_REC_COUNT_MAX                  5

#define ZLL_NETWORK_START_RSP_STATUS_SUCCESS               0x00
#define ZLL_NETWORK_START_RSP_STATUS_FAILURE               0x01
#define ZLL_NETWORK_JOIN_RSP_STATUS_SUCCESS                0x00
#define ZLL_NETWORK_JOIN_RSP_STATUS_FAILURE                0x01

/*********************************************************************
 * TYPEDEFS
 */

// Scan Request/Response "ZigBee information" field bitmap
typedef struct
{
  unsigned int logicalType:2;
  unsigned int rxOnWhenIdle:1;
  unsigned int reserved:5;
} zInfoBits_t;

// Scan Request/Response "ZigBee information" field
typedef union
{
  zInfoBits_t zInfoBits;
  uint8 zInfoByte;
} zInfo_t;

// Scan Request/Response "ZLL information" field bitmap
typedef struct
{
  unsigned int factoryNew:1;
  unsigned int addressAssignment:1;
  unsigned int reserved:2;
  unsigned int linkInitiator:1;
  unsigned int linkPriority:1;
  unsigned int reserved2:2;  	   	
} zllInfoBits_t;

// Scan Request/Response "ZLL information" field
typedef union
{
  zllInfoBits_t zllInfoBits;
  uint8 zllInfoByte;
} zllInfo_t;

// Scan Request command format
typedef struct
{
  uint32 transID;    // Inter-PAN transaction idententifier
  zInfo_t zInfo;     // ZigBee information
  zllInfo_t zllInfo; // ZLL information

  // shorthand "zInfo" access
#define zLogicalType          zInfo.zInfoBits.logicalType
#define zRxOnWhenIdle         zInfo.zInfoBits.rxOnWhenIdle

  // shorthand "zllInfo" access
#define zllFactoryNew         zllInfo.zllInfoBits.factoryNew
#define zllAddressAssignment  zllInfo.zllInfoBits.addressAssignment
#define zllLinkInitiator      zllInfo.zllInfoBits.linkInitiator
} zclLLScanReq_t;

// Device Information Request command format
typedef struct
{
  uint32 transID;   // Inter-PAN transaction idententifier
  uint8 startIndex; // Start index
} zclLLDeviceInfoReq_t;

// Identify Request command format
typedef struct
{
  uint32 transID;    // Inter-PAN transaction idententifier
  uint16 IdDuration; // Identify duration
} zclLLIdentifyReq_t;

// Reset to factory new request command frame
typedef struct
{
  uint32 transID;    // Inter-PAN transaction idententifier
} zclLLResetToFNReq_t;

// ZLL Network parameters
typedef struct
{
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 keyIndex;                     // Key index
  uint8 nwkKey[SEC_KEY_LEN];          // Encrypted network key
  uint8 logicalChannel;               // Logical channel
  uint16 panId;                       // PAN identifier
  uint16 nwkAddr;                     // Network address
  uint16 grpIDsBegin;                 // Group identifiers begin
  uint16 grpIDsEnd;                   // Group identifiers end
  uint16 freeNwkAddrBegin;            // Free network address range begin
  uint16 freeNwkAddrEnd;              // Free network address range end
  uint16 freeGrpIDBegin;              // Free group identifier range begin
  uint16 freeGrpIDEnd;                // Free group identifier range end
} zclLLNwkParams_t;

// Network start request command frame
typedef struct
{
  uint32 transID;                         // Inter-PAN transaction idententifier
  zclLLNwkParams_t nwkParams;             // Network parameters
  uint8 initiatorIeeeAddr[Z_EXTADDR_LEN]; // Initiator IEEE address
  uint16 initiatorNwkAddr;                // Initiator network address
} zclLLNwkStartReq_t;

// Network join router/end device request command frame
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  zclLLNwkParams_t nwkParams;         // Network parameters
  uint8 nwkUpdateId;                  // Network update identifier
} zclLLNwkJoinReq_t;

// Network update request command frame
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 nwkUpdateId;                  // Network update identifier
  uint8 logicalChannel;               // Logical channel
  uint16 PANID;                       // PAN identifier
  uint16 nwkAddr;                     // Network address
} zclLLNwkUpdateReq_t;

// Get group identifiers command format
typedef struct
{
  uint8 startIndex; // Start index
} zclLLGetGrpIDsReq_t;

// Get endpoint list command format
typedef struct
{
  uint8 startIndex; // Start index
} zclLLGetEPListReq_t;

// Device Info
typedef struct
{
  uint8 endpoint;                     // Endpoint identifier
  uint16 profileID;                   // Profile identifier
  uint16 deviceID;                    // Device identifier
  uint8 version;                      // Version
  uint8 grpIdCnt;                     // Group identifier count
} zclLLDeviceInfo_t;

// Scan Response command format
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  uint8 rssiCorrection;               // RSSI correction
  zInfo_t zInfo;                      // ZigBee information
  zllInfo_t zllInfo;                  // ZLL information
  uint16 keyBitmask;                  // Key bitmask
  uint32 responseID;                  // Response idententifier
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 nwkUpdateId;                  // Network update identifier
  uint8 logicalChannel;               // Logical channel
  uint16 PANID;                       // PAN identifier
  uint16 nwkAddr;                     // Network address
  uint8 numSubDevices;                // Number of sub-devices
  uint8 totalGrpIDs;                  // Total group identifiers

  // The followings are only present when numSubDevices is equal to 1
  zclLLDeviceInfo_t deviceInfo;       // Device info

  // shorthand "zInfo" access
#define zLogicalType          zInfo.zInfoBits.logicalType
#define zRxOnWhenIdle         zInfo.zInfoBits.rxOnWhenIdle

  // shorthand "zllInfo" access
#define zllFactoryNew         zllInfo.zllInfoBits.factoryNew
#define zllAddressAssignment  zllInfo.zllInfoBits.addressAssignment
#define zllLinkInitiator      zllInfo.zllInfoBits.linkInitiator
#define zllLinklinkPriority   zllInfo.zllInfoBits.linkPriority
} zclLLScanRsp_t;

// Device information record
typedef struct
{
  uint8 ieeeAddr[Z_EXTADDR_LEN]; // IEEE address
  zclLLDeviceInfo_t deviceInfo;  // Device info
  uint8 sort;                    // Sort
} devInfoRec_t;

// Device information response command frame
typedef struct
{
  uint32 transID;            // Inter-PAN transaction idententifier
  uint8 numSubDevices;       // Number of sub-devices
  uint8 startIndex;          // Start index
  uint8 cnt;                 // Device information record count
  devInfoRec_t *devInfoRec; // Device information record
} zclLLDeviceInfoRsp_t;

// Network start response command frame
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  uint8 status;                       // Status
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 nwkUpdateId;                  // Network update identifier
  uint8 logicalChannel;               // Logical channel
  uint16 panId;                       // PAN identifier
} zclLLNwkStartRsp_t;

// Network join router/end device response command frame
typedef struct
{
  uint32 transID; // Inter-PAN transaction idententifier
  uint8 status;   // Status
} zclLLNwkJoinRsp_t;

// Endpoint information command format
typedef struct
{
  uint8 ieeeAddr[Z_EXTADDR_LEN]; // IEEE address
  uint16 nwkAddr;                // Network address
  uint8 endpoint;                // Endpoint identifier
  uint16 profileID;              // Profile identifier
  uint16 deviceID;               // Device identifier
  uint8 version;                 // Version
} zclLLEndpointInfo_t;

// Group information record
typedef struct
{
  uint16 grpID;  // Group identifier
  uint8 grpType; // Group type
} grpInfoRec_t;

// Get group identifiers response command frame
typedef struct
{
  uint8 total;               // total number of group ids supported by device
  uint8 startIndex;          // Start index
  uint8 cnt;                 // Number of entries in the group info record
  grpInfoRec_t *grpInfoRec; // Group information record
} zclLLGetGrpIDsRsp_t;

// Endpoint information record entry
typedef struct
{
  uint16 nwkAddr;   // Network address
  uint8 endpoint;   // Endpoint identifier
  uint16 profileID; // Profile identifier
  uint16 deviceID;  // Device identifier
  uint8 version;    // Version
} epInfoRec_t;

// Get endpoint list response command format
typedef struct
{
  uint8 total;             // total number of endpoints supported by device
  uint8 startIndex;        // Start index
  uint8 cnt;               // Number of entries in the endpoint info record
  epInfoRec_t *epInfoRec; // Endpoint information record
} zclLLGetEPListRsp_t;

/* Request Commands */

// This callback is called to process a Scan Request command
typedef ZStatus_t (*zclLL_ScanReqCB_t)( afAddrType_t *srcAddr, zclLLScanReq_t *pReq, uint8 seqNum );

// This callback is called to process a Device Information Request command
typedef ZStatus_t (*zclLL_DeviceInfoReqCB_t)( afAddrType_t *srcAddr, zclLLDeviceInfoReq_t *pReq, uint8 seqNum );

// This callback is called to process an Identify Request command
typedef ZStatus_t (*zclLL_IdentifyReqCB_t)( afAddrType_t *srcAddr, zclLLIdentifyReq_t *pReq );

// This callback is called to process a Reset to Factory New Request command
typedef ZStatus_t (*zclLL_ResetToFNReqCB_t)( afAddrType_t *srcAddr, zclLLResetToFNReq_t *pReq );

// This callback is called to process a Network Start Request command
typedef ZStatus_t (*zclLL_NwkStartReqCB_t)( afAddrType_t *srcAddr, zclLLNwkStartReq_t *pReq, uint8 seqNum );

// This callback is called to process a Network Join Router Request command
typedef ZStatus_t (*zclLL_NwkJoinRtrReqCB_t)( afAddrType_t *srcAddr, zclLLNwkJoinReq_t *pReq, uint8 seqNum );

// This callback is called to process a Network Join End Device Request command
typedef ZStatus_t (*zclLL_NwkJoinEDReqCB_t)( afAddrType_t *srcAddr, zclLLNwkJoinReq_t *pReq, uint8 seqNum );

// This callback is called to process a Network Update Request command
typedef ZStatus_t (*zclLL_NwkUpdateReqCB_t)( afAddrType_t *srcAddr, zclLLNwkUpdateReq_t *pReq );

// This callback is called to process a Get Group Identifiers Request command
typedef ZStatus_t (*zclLL_GetGrpIDsReqCB_t)( afAddrType_t *srcAddr, zclLLGetGrpIDsReq_t *pReq, uint8 seqNum );

// This callback is called to process a SGet Endpoint List Request command
typedef ZStatus_t (*zclLL_GetEPListReqCB_t)( afAddrType_t *srcAddr, zclLLGetEPListReq_t *pReq, uint8 SeqNum );

/* Response Commands */

// This callback is called to process a Scan Response command
typedef ZStatus_t (*zclLL_ScanRspCB_t)( afAddrType_t *srcAddr, zclLLScanRsp_t *pRsp );

// This callback is called to process a Device Information Response command
typedef ZStatus_t (*zclLL_DeviceInfoRspCB_t)( afAddrType_t *srcAddr, zclLLDeviceInfoRsp_t *pRsp );

// This callback is called to process a Network Start Response command
typedef ZStatus_t (*zclLL_NwkStartRspCB_t)( afAddrType_t *srcAddr, zclLLNwkStartRsp_t *pRsp );

// This callback is called to process a Network Join Router Response command
typedef ZStatus_t (*zclLL_NwkJoinRtrRspCB_t)( afAddrType_t *srcAddr, zclLLNwkJoinRsp_t *pRsp );

// This callback is called to process a Network Join End Device Response command
typedef ZStatus_t (*zclLL_NwkJoinEDRspCB_t)( afAddrType_t *srcAddr, zclLLNwkJoinRsp_t *pRsp );

// This callback is called to process a Endpoint Information command
typedef ZStatus_t (*zclLL_EndpointInfoCB_t)( afAddrType_t *srcAddr, zclLLEndpointInfo_t *pRsp );

// This callback is called to process a Get Group Identifiers command
typedef ZStatus_t (*zclLL_GetGrpIDsRspCB_t)( afAddrType_t *srcAddr, zclLLGetGrpIDsRsp_t *pRsp );

// This callback is called to process a Get Endpoint List Response command
typedef ZStatus_t (*zclLL_GetEPListRspCB_t)( afAddrType_t *srcAddr, zclLLGetEPListRsp_t *pRsp );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct			
{
  // Received Server Commands
  zclLL_GetGrpIDsReqCB_t   pfnGetGrpIDsReq;  // Get Group Identifiers Request command
  zclLL_GetEPListReqCB_t   pfnGetEPListReq;  // Get Endpoint List Request command

  // Received Client Commands
  zclLL_EndpointInfoCB_t   pfnEndpointInfo;  // Endpoint Information command
  zclLL_GetGrpIDsRspCB_t   pfnGetGrpIDsRsp;  // Get Group Identifiers Response command
  zclLL_GetEPListRspCB_t   pfnGetEPListRsp;  // Get Endpoint List Response command
} zclLL_AppCallbacks_t;

// Register Inter-PAN Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct			
{
  // Received Server Commands
  zclLL_ScanReqCB_t        pfnScanReq;       // Scan Request command
  zclLL_DeviceInfoReqCB_t  pfnDeviceInfoReq; // Device Information Request command
  zclLL_IdentifyReqCB_t    pfnIdentifyReq;   // Identify Request command
  zclLL_ResetToFNReqCB_t   pfnResetToFNReq;  // Reset to Factory New Request command
  zclLL_NwkStartReqCB_t    pfnNwkStartReq;   // Network Start Request command
  zclLL_NwkJoinRtrReqCB_t  pfnNwkJoinRtrReq; // Network Join Router Request command
  zclLL_NwkJoinEDReqCB_t   pfnNwkJoinEDReq;  // Network Join End Device Request command
  zclLL_NwkUpdateReqCB_t   pfnNwkUpdateReq;  // Network Update Request command

  // Received Client Commands
  zclLL_ScanRspCB_t        pfnScanRsp;       // Scan Response command
  zclLL_DeviceInfoRspCB_t  pfnDeviceInfoRsp; // Device Information Response command
  zclLL_NwkStartRspCB_t    pfnNwkStartRsp;   // Network Start Response command
  zclLL_NwkJoinRtrRspCB_t  pfnNwkJoinRtrRsp; // Network Join Router Response command
  zclLL_NwkJoinEDRspCB_t   pfnNwkJoinEDRsp;  // Network Join End Device Response command
} zclLL_InterPANCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclLL_RegisterCmdCallbacks( uint8 endpoint, zclLL_AppCallbacks_t *callbacks );

 /*
  * Register for callbacks from this cluster library for Inter-PAN commands
  */
extern ZStatus_t zclLL_RegisterInterPANCmdCallbacks( zclLL_InterPANCallbacks_t *callbacks );

/*
 * Call to send out an Scan Request
 */
extern ZStatus_t zclLL_Send_ScanReq( uint8 srcEP, afAddrType_t *dstAddr,
                                     zclLLScanReq_t *pReq, uint8 seqNum );
/*
 * Call to send out a Device Information Request
 */
ZStatus_t zclLL_Send_DeviceInfoReq( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclLLDeviceInfoReq_t *pReq, uint8 seqNum );
/*
 * Call to send out an Identify Request
 */
extern ZStatus_t zclLL_Send_IndentifyReq( uint8 srcEP, afAddrType_t *dstAddr,
                                          zclLLIdentifyReq_t *pReq, uint8 seqNum );
/*
 * Call to send out a Reset to Factory New Request
 */
extern ZStatus_t zclLL_Send_ResetToFNReq( uint8 srcEP, afAddrType_t *dstAddr,
                                          zclLLResetToFNReq_t *pReq, uint8 seqNum );
/*
 * Call to send out a Network Start Request
 */
extern ZStatus_t zclLL_Send_NwkStartReq( uint8 srcEP, afAddrType_t *dstAddr,
                                         zclLLNwkStartReq_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Network Join Router/End Device Request
 */
extern ZStatus_t zclLL_Send_NwkJoinReq( uint8 srcEP, afAddrType_t *dstAddr,
                                        zclLLNwkJoinReq_t *pRsp, uint8 cmd, uint8 seqNum );
/*
 * Call to send out a Network Update Request
 */
extern ZStatus_t zclLL_Send_NwkUpdateReq( uint8 srcEP, afAddrType_t *dstAddr,
                                          zclLLNwkUpdateReq_t *pReq, uint8 seqNum );
/*
 * Call to send out an Scan Response
 */
extern ZStatus_t zclLL_Send_ScanRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                     zclLLScanRsp_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Device Information Response
 */
extern ZStatus_t zclLL_Send_DeviceInfoRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                           zclLLDeviceInfoRsp_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Network Start Response
 */
extern ZStatus_t zclLL_Send_NwkStartRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                         zclLLNwkStartRsp_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Network Join Router/End Device Response
 */
extern ZStatus_t zclLL_Send_NwkJoinRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                        zclLLNwkJoinRsp_t *pRsp, uint8 cmd, uint8 seqNum );
/*
 * Call to send out an Endpoint Information
 */
extern ZStatus_t zclLL_Send_EndpointInfo( uint8 srcEP, afAddrType_t *dstAddr,
                                          zclLLEndpointInfo_t *pCmd,
                                          uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send out a Get Group Identifiers Response
 */
extern ZStatus_t zclLL_Send_GetGrpIDsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                          zclLLGetGrpIDsRsp_t *pRsp,
                                          uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send out a Get Endpoint List Response
 */
extern ZStatus_t zclLL_Send_GetEPListRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                          zclLLGetEPListRsp_t *pRsp,
                                          uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send ZLL Profile and Cluster Specific Inter-PAN Command messages
 */
extern ZStatus_t zclLL_SendInterPANCommand( uint8 srcEP, afAddrType_t *destAddr, uint8 cmd,
                                            uint8 direction, uint8 seqNum, uint16 cmdFormatLen, uint8 *cmdFormat );

/*********************************************************************
 * @fn      zclLL_Send_NwkJoinRtrReq
 *
 * @brief   Call to send out a Network Join Router Request command
 *
 *          Use like: zclLL_Send_NwkJoinRtrReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                              zclLLNwkJoinReq_t *pReq, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define zclLL_Send_NwkJoinRtrReq( srcEP, dstAddr, pReq, seqNum ) \
                                  zclLL_Send_NwkJoinReq( (srcEP), (dstAddr), (pReq), \
                                                         COMMAND_ZLL_NWK_JOIN_RTR_REQ, \
                                                         (seqNum) )

/*********************************************************************
 * @fn      zclLL_Send_NwkJoinEDReq
 *
 * @brief   Call to send out a Network Join End Device Request command
 *
 *          Use like: zclLL_Send_NwkJoinEDReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             zclLLNwkJoinReq_t *pReq, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define zclLL_Send_NwkJoinEDReq( srcEP, dstAddr, pReq, seqNum ) \
                                 zclLL_Send_NwkJoinReq( (srcEP), (dstAddr), (pReq), \
                                                        COMMAND_ZLL_NWK_JOIN_ED_REQ, \
                                                        (seqNum) )

/*********************************************************************
 * @fn      zclLL_Send_GetGrpIDsReq
 *
 * @brief   Call to send out a Get Group Identifiers Request command
 *
 *          Use like: zclLL_Send_GetGrpIDsReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             zclLLGetGrpIDsReq_t *pReq,
 *                                             uint8 disableDefaultRsp, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - request parameters
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define zclLL_Send_GetGrpIDsReq( srcEP, dstAddr, pReq, disableDefaultRsp, seqNum ) \
                                 zcl_SendCommand( (srcEP), (dstAddr), ZCL_CLUSTER_ID_LIGHT_LINK, \
                                                  COMMAND_ZLL_GET_GRP_IDS_REQ, TRUE, \
                                                  ZCL_FRAME_CLIENT_SERVER_DIR, (disableDefaultRsp), \
                                                  0, (seqNum), 1, &((pReq)->startIndex) )

/*********************************************************************
 * @fn      zclLL_Send_GetEPListReq
 *
 * @brief   Call to send out a Get Endpoint List Request command
 *
 *          Use like: zclLL_Send_GetEPListReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             zclLLGetEPListReq_t *pReq,
 *                                             uint8 disableDefaultRsp, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - request parameters
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define zclLL_Send_GetEPListReq( srcEP, dstAddr, pReq, disableDefaultRsp, seqNum ) \
                                 zcl_SendCommand( (srcEP), (dstAddr), ZCL_CLUSTER_ID_LIGHT_LINK, \
                                                  COMMAND_ZLL_GET_EP_LIST_REQ, TRUE, \
                                                  ZCL_FRAME_CLIENT_SERVER_DIR, (disableDefaultRsp), \
                                                  0, (seqNum), 1, &((pReq)->startIndex) )

/*********************************************************************
 * @fn      zclLL_Send_NwkJoinRtrRsp
 *
 * @brief   Call to send out a Network Join Router Response command
 *
 *          Use like: zclLL_Send_NwkJoinRtrRsp( uint8 srcEP, afAddrType_t *dstAddr,
 *                                              zclLLNwkJoinRsp_t *pRsp, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define zclLL_Send_NwkJoinRtrRsp( srcEP, dstAddr, pRsp, seqNum ) \
                                  zclLL_Send_NwkJoinRsp( (srcEP), (dstAddr), (pRsp), \
                                                         COMMAND_ZLL_NWK_JOIN_RTR_RSP, \
                                                         (seqNum) )

/*********************************************************************
 * @fn      zclLL_Send_NwkJoinEDRsp
 *
 * @brief   Call to send out a Network Join End Device Response command
 *
 *          Use like: zclLL_Send_NwkJoinEDRsp( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             zclLLNwkJoinReq_t *pRsp, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define zclLL_Send_NwkJoinEDRsp( srcEP, dstAddr, pRsp, seqNum ) \
                                 zclLL_Send_NwkJoinRsp( (srcEP), (dstAddr), (pRsp), \
                                                        COMMAND_ZLL_NWK_JOIN_ED_RSP, \
                                                        (seqNum) )


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_LL_H */
