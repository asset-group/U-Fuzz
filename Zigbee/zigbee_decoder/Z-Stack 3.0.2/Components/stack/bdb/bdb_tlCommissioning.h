/**************************************************************************************************
  Filename:       bdb_tlCommissioning.h
  Revised:        $Date: 2012-10-25 15:25:21 -0700 (Thu, 25 Oct 2012) $
  Revision:       $Revision: 31911 $

  Description:    This file contains the Touch Linkcommissioning
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

#ifndef BDB_TLCOMMISSIONING_H
#define BDB_TLCOMMISSIONING_H

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
#define TOUCHLINK_PROFILE_ID                                     0xc05e

// TouchLink Basic Lighting Device IDs
#define TOUCHLINK_DEVICEID_ON_OFF_LIGHT                          0x0000
#define TOUCHLINK_DEVICEID_ON_OFF_PLUG_IN_UNIT                   0x0010
#define TOUCHLINK_DEVICEID_DIMMABLE_LIGHT                        0x0100
#define TOUCHLINK_DEVICEID_DIMMABLE_PLUG_IN_UNIT                 0x0110

// TouchLink Color Lighting Device IDs
#define TOUCHLINK_DEVICEID_COLOR_LIGHT                           0x0200
#define TOUCHLINK_DEVICEID_EXTENDED_COLOR_LIGHT                  0x0210
#define TOUCHLINK_DEVICEID_COLOR_TEMPERATURE_LIGHT               0x0220

// TouchLink Lighting Remotes Device IDs
#define TOUCHLINK_DEVICEID_COLOR_CONTORLLER                      0x0800
#define TOUCHLINK_DEVICEID_COLOR_SCENE_CONTROLLER                0x0810
#define TOUCHLINK_DEVICEID_NON_COLOR_CONTORLLER                  0x0820
#define TOUCHLINK_DEVICEID_NON_COLOR_SCENE_CONTROLLER            0x0830
#define TOUCHLINK_DEVICEID_CONTROL_BRIDGE                        0x0840
#define TOUCHLINK_DEVICEID_ON_OFF_SENSOR                         0x0850

// TouchLink Device Version
#define TOUCHLINK_DEVICE_VERSION                                 2

/**********************************************/
/*** TouchLink Commissioning Cluster Commands     ***/
/**********************************************/

// Commands received by TouchLink Commissioning Cluster Server
#define COMMAND_TOUCHLINK_SCAN_REQ                               0x00
#define COMMAND_TOUCHLINK_DEVICE_INFO_REQ                        0x02
#define COMMAND_TOUCHLINK_IDENTIFY_REQ                           0x06
#define COMMAND_TOUCHLINK_RESET_TO_FN_REQ                        0x07
#define COMMAND_TOUCHLINK_NWK_START_REQ                          0x10
#define COMMAND_TOUCHLINK_NWK_JOIN_RTR_REQ                       0x12
#define COMMAND_TOUCHLINK_NWK_JOIN_ED_REQ                        0x14
#define COMMAND_TOUCHLINK_NWK_UPDATE_REQ                         0x16
#define COMMAND_TOUCHLINK_GET_GRP_IDS_REQ                        0x41
#define COMMAND_TOUCHLINK_GET_EP_LIST_REQ                        0x42

// Commands received by TouchLink Commissioning Cluster Client
#define COMMAND_TOUCHLINK_SCAN_RSP                               0x01
#define COMMAND_TOUCHLINK_DEVICE_INFO_RSP                        0x03
#define COMMAND_TOUCHLINK_NWK_START_RSP                          0x11
#define COMMAND_TOUCHLINK_NWK_JOIN_RTR_RSP                       0x13
#define COMMAND_TOUCHLINK_NWK_JOIN_ED_RSP                        0x15
#define COMMAND_TOUCHLINK_EP_INFO                                0x40
#define COMMAND_TOUCHLINK_GET_GRP_IDS_RSP                        0x41
#define COMMAND_TOUCHLINK_GET_EP_LIST_RSP                        0x42

// Request command lengths
#define TOUCHLINK_CMDLEN_SCAN_REQ                                6
#define TOUCHLINK_CMDLEN_DEVICE_INFO_REQ                         5
#define TOUCHLINK_CMDLEN_IDENTIFY_REQ                            6
#define TOUCHLINK_CMDLEN_RESET_TO_FN_REQ                         4
#define TOUCHLINK_CMDLEN_NWK_START_REQ                           56
#define TOUCHLINK_CMDLEN_NWK_JOIN_REQ                            47
#define TOUCHLINK_CMDLEN_NWK_UPDATE_REQ                          18
#define TOUCHLINK_CMDLEN_GET_GRP_IDS_REQ                         1
#define TOUCHLINK_CMDLEN_GET_EP_LIST_REQ                         1

// Response command lengths
#define TOUCHLINK_CMDLEN_SCAN_RSP                                29
#define TOUCHLINK_CMDLENOPTIONAL_SCAN_RSP                        7
#define TOUCHLINK_CMDLEN_DEVICE_INFO_RSP                         7
#define TOUCHLINK_CMDLENOPTIONAL_DEVICE_INFO_RSP                 16
#define TOUCHLINK_CMDLEN_NWK_START_RSP                           17
#define TOUCHLINK_CMDLEN_NWK_JOIN_RSP                            5
#define TOUCHLINK_CMDLEN_EP_INFO                                 16
#define TOUCHLINK_CMDLEN_GET_GRP_IDS_RSP                         3
#define TOUCHLINK_CMDLENOPTIONAL_GET_GRP_IDS_RSP                 3
#define TOUCHLINK_CMDLEN_GET_EP_LIST_RSP                         3
#define TOUCHLINK_CMDLENOPTIONAL_GET_EP_LIST_RSP                 8

// ZigBee information bit mask
#define TOUCHLINK_ZINFO_LOGICAL_TYPE                             0x03
#define TOUCHLINK_ZINFO_RX_ON_WHILE_IDLE                         0x04

// TouchLink information bit mask
#define TOUCHLINK_INFO_FACTORY_NEW                            0x01
#define TOUCHLINK_INFO_ADDR_ASSIGN                            0x02
#define TOUCHLINK_INFO_LINK_INITIATOR                         0x10
#define TOUCHLINK_INFO_LINK_TIME_WINDOW                       0x20

#define TOUCHLINK_DEVICE_INFO_RSP_REC_COUNT_MAX                  5

#define TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS               0x00
#define TOUCHLINK_NETWORK_START_RSP_STATUS_FAILURE               0x01
#define TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS                0x00
#define TOUCHLINK_NETWORK_JOIN_RSP_STATUS_FAILURE                0x01

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

// Scan Request/Response "TouchLink information" field bitmap
typedef struct
{
  unsigned int factoryNew:1;
  unsigned int addressAssignment:1;
  unsigned int reserved:2;
  unsigned int linkInitiator:1;
  unsigned int linkPriority:1;
  unsigned int reserved1:1;  
  unsigned int profileInterop:1;  
} touchLinkInfoBits_t;

// Scan Request/Response "TouchLink information" field
typedef union
{
  touchLinkInfoBits_t touchLinkInfoBits;
  uint8 touchLinkInfoByte;
} touchLinkInfo_t;

// Scan Request command format
typedef struct
{
  uint32 transID;    // Inter-PAN transaction idententifier
  zInfo_t zInfo;     // ZigBee information
  touchLinkInfo_t touchLinkInfo; // TouchLink information

  // shorthand "zInfo" access
#define zLogicalType          zInfo.zInfoBits.logicalType
#define zRxOnWhenIdle         zInfo.zInfoBits.rxOnWhenIdle

  // shorthand "touchLinkInfo" access
#define touchLinkFactoryNew         touchLinkInfo.touchLinkInfoBits.factoryNew
#define touchLinkAddressAssignment  touchLinkInfo.touchLinkInfoBits.addressAssignment
#define touchLinkInitiator          touchLinkInfo.touchLinkInfoBits.linkInitiator
} bdbTLScanReq_t;

// Device Information Request command format
typedef struct
{
  uint32 transID;   // Inter-PAN transaction idententifier
  uint8 startIndex; // Start index
} bdbTLDeviceInfoReq_t;

// Identify Request command format
typedef struct
{
  uint32 transID;    // Inter-PAN transaction idententifier
  uint16 IdDuration; // Identify duration
} bdbTLIdentifyReq_t;

// Reset to factory new request command frame
typedef struct
{
  uint32 transID;    // Inter-PAN transaction idententifier
} bdbTLResetToFNReq_t;

// TouchLink Network parameters
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
} bdbTLNwkParams_t;

// Network start request command frame
typedef struct
{
  uint32 transID;                         // Inter-PAN transaction idententifier
  bdbTLNwkParams_t nwkParams;             // Network parameters
  uint8 initiatorIeeeAddr[Z_EXTADDR_LEN]; // Initiator IEEE address
  uint16 initiatorNwkAddr;                // Initiator network address
} bdbTLNwkStartReq_t;

// Network join router/end device request command frame
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  bdbTLNwkParams_t nwkParams;         // Network parameters
  uint8 nwkUpdateId;                  // Network update identifier
} bdbTLNwkJoinReq_t;

// Network update request command frame
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 nwkUpdateId;                  // Network update identifier
  uint8 logicalChannel;               // Logical channel
  uint16 PANID;                       // PAN identifier
  uint16 nwkAddr;                     // Network address
} bdbTLNwkUpdateReq_t;
  
// Network update request command frame
typedef struct
{
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 nwkUpdateId;                  // Network update identifier
  uint8 logicalChannel;               // Logical channel
  uint16 panId;                       // PAN identifier
  uint16 nwkAddr;                     // Network address
} bdbTLNwkRejoin_t;

// Get group identifiers command format
typedef struct
{
  uint8 startIndex; // Start index
} bdbTLGetGrpIDsReq_t;

// Get endpoint list command format
typedef struct
{
  uint8 startIndex; // Start index
} bdbTLGetEPListReq_t;

// Device Info
typedef struct
{
  uint8 endpoint;                     // Endpoint identifier
  uint16 profileID;                   // Profile identifier
  uint16 deviceID;                    // Device identifier
  uint8 version;                      // Version
  uint8 grpIdCnt;                     // Group identifier count
} bdbTLDeviceInfo_t;

// Scan Response command format
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  uint8 rssiCorrection;               // RSSI correction
  zInfo_t zInfo;                      // ZigBee information
  touchLinkInfo_t touchLinkInfo;      // TouchLink information
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
  bdbTLDeviceInfo_t deviceInfo;       // Device info

  // shorthand "zInfo" access
#define zLogicalType          zInfo.zInfoBits.logicalType
#define zRxOnWhenIdle         zInfo.zInfoBits.rxOnWhenIdle

  // shorthand "touchLinkInfo" access
#define touchLinkFactoryNew         touchLinkInfo.touchLinkInfoBits.factoryNew
#define touchLinkAddressAssignment  touchLinkInfo.touchLinkInfoBits.addressAssignment
#define touchLinkInitiator          touchLinkInfo.touchLinkInfoBits.linkInitiator
#define touchLinklinkPriority       touchLinkInfo.touchLinkInfoBits.linkPriority
#define touchLinkProfileInterop     touchLinkInfo.touchLinkInfoBits.profileInterop
} bdbTLScanRsp_t;

// Device information record
typedef struct
{
  uint8 ieeeAddr[Z_EXTADDR_LEN]; // IEEE address
  bdbTLDeviceInfo_t deviceInfo;  // Device info
  uint8 sort;                    // Sort
} devInfoRec_t;

// Device information response command frame
typedef struct
{
  uint32 transID;            // Inter-PAN transaction idententifier
  uint8 numSubDevices;       // Number of sub-devices
  uint8 startIndex;          // Start index
  uint8 cnt;                 // Device information record count
  devInfoRec_t devInfoRec[1]; // Device information record
} bdbTLDeviceInfoRsp_t;

// Network start response command frame
typedef struct
{
  uint32 transID;                     // Inter-PAN transaction idententifier
  uint8 status;                       // Status
  uint8 extendedPANID[Z_EXTADDR_LEN]; // Extended PAN identifier
  uint8 nwkUpdateId;                  // Network update identifier
  uint8 logicalChannel;               // Logical channel
  uint16 panId;                       // PAN identifier
} bdbTLNwkStartRsp_t;

// Network join router/end device response command frame
typedef struct
{
  uint32 transID; // Inter-PAN transaction idententifier
  uint8 status;   // Status
} bdbTLNwkJoinRsp_t;

// Endpoint information command format
typedef struct
{
  uint8 ieeeAddr[Z_EXTADDR_LEN]; // IEEE address
  uint16 nwkAddr;                // Network address
  uint8 endpoint;                // Endpoint identifier
  uint16 profileID;              // Profile identifier
  uint16 deviceID;               // Device identifier
  uint8 version;                 // Version
} bdbTLEndpointInfo_t;

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
} bdbTLGetGrpIDsRsp_t;

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
} bdbTLGetEPListRsp_t;

/* Request Commands */

// This callback is called to process a Scan Request command
typedef ZStatus_t (*bdbTL_ScanReqCB_t)( afAddrType_t *srcAddr, bdbTLScanReq_t *pReq, uint8 seqNum );

// This callback is called to process a Device Information Request command
typedef ZStatus_t (*bdbTL_DeviceInfoReqCB_t)( afAddrType_t *srcAddr, bdbTLDeviceInfoReq_t *pReq, uint8 seqNum );

// This callback is called to process an Identify Request command
typedef ZStatus_t (*bdbTL_IdentifyReqCB_t)( afAddrType_t *srcAddr, bdbTLIdentifyReq_t *pReq );

// This callback is called to process a Reset to Factory New Request command
typedef ZStatus_t (*bdbTL_ResetToFNReqCB_t)( afAddrType_t *srcAddr, bdbTLResetToFNReq_t *pReq );

// This callback is called to process a Network Start Request command
typedef ZStatus_t (*bdbTL_NwkStartReqCB_t)( afAddrType_t *srcAddr, bdbTLNwkStartReq_t *pReq, uint8 seqNum );

// This callback is called to process a Network Join Router Request command
typedef ZStatus_t (*bdbTL_NwkJoinRtrReqCB_t)( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8 seqNum );

// This callback is called to process a Network Join End Device Request command
typedef ZStatus_t (*bdbTL_NwkJoinEDReqCB_t)( afAddrType_t *srcAddr, bdbTLNwkJoinReq_t *pReq, uint8 seqNum );

// This callback is called to process a Network Update Request command
typedef ZStatus_t (*bdbTL_NwkUpdateReqCB_t)( afAddrType_t *srcAddr, bdbTLNwkUpdateReq_t *pReq );

// This callback is called to process a Get Group Identifiers Request command
typedef ZStatus_t (*bdbTL_GetGrpIDsReqCB_t)( afAddrType_t *srcAddr, bdbTLGetGrpIDsReq_t *pReq, uint8 seqNum );

// This callback is called to process a SGet Endpoint List Request command
typedef ZStatus_t (*bdbTL_GetEPListReqCB_t)( afAddrType_t *srcAddr, bdbTLGetEPListReq_t *pReq, uint8 SeqNum );

/* Response Commands */

// This callback is called to process a Scan Response command
typedef ZStatus_t (*bdbTL_ScanRspCB_t)( afAddrType_t *srcAddr, bdbTLScanRsp_t *pRsp );

// This callback is called to process a Device Information Response command
typedef ZStatus_t (*bdbTL_DeviceInfoRspCB_t)( afAddrType_t *srcAddr, bdbTLDeviceInfoRsp_t *pRsp );

// This callback is called to process a Network Start Response command
typedef ZStatus_t (*bdbTL_NwkStartRspCB_t)( afAddrType_t *srcAddr, bdbTLNwkStartRsp_t *pRsp );

// This callback is called to process a Network Join Router Response command
typedef ZStatus_t (*bdbTL_NwkJoinRtrRspCB_t)( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp );

// This callback is called to process a Network Join End Device Response command
typedef ZStatus_t (*bdbTL_NwkJoinEDRspCB_t)( afAddrType_t *srcAddr, bdbTLNwkJoinRsp_t *pRsp );

// This callback is called to process a Endpoint Information command
typedef ZStatus_t (*bdbTL_EndpointInfoCB_t)( afAddrType_t *srcAddr, bdbTLEndpointInfo_t *pRsp );

// This callback is called to process a Get Group Identifiers command
typedef ZStatus_t (*bdbTL_GetGrpIDsRspCB_t)( afAddrType_t *srcAddr, bdbTLGetGrpIDsRsp_t *pRsp );

// This callback is called to process a Get Endpoint List Response command
typedef ZStatus_t (*bdbTL_GetEPListRspCB_t)( afAddrType_t *srcAddr, bdbTLGetEPListRsp_t *pRsp );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct			
{
  // Received Server Commands
  bdbTL_GetGrpIDsReqCB_t   pfnGetGrpIDsReq;  // Get Group Identifiers Request command
  bdbTL_GetEPListReqCB_t   pfnGetEPListReq;  // Get Endpoint List Request command

  // Received Client Commands
  bdbTL_EndpointInfoCB_t   pfnEndpointInfo;  // Endpoint Information command
  bdbTL_GetGrpIDsRspCB_t   pfnGetGrpIDsRsp;  // Get Group Identifiers Response command
  bdbTL_GetEPListRspCB_t   pfnGetEPListRsp;  // Get Endpoint List Response command
} bdbTL_AppCallbacks_t;

// Register Inter-PAN Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct			
{
  // Received Server Commands
  bdbTL_ScanReqCB_t        pfnScanReq;       // Scan Request command
  bdbTL_DeviceInfoReqCB_t  pfnDeviceInfoReq; // Device Information Request command
  bdbTL_IdentifyReqCB_t    pfnIdentifyReq;   // Identify Request command
  bdbTL_ResetToFNReqCB_t   pfnResetToFNReq;  // Reset to Factory New Request command
  bdbTL_NwkStartReqCB_t    pfnNwkStartReq;   // Network Start Request command
  bdbTL_NwkJoinRtrReqCB_t  pfnNwkJoinRtrReq; // Network Join Router Request command
  bdbTL_NwkJoinEDReqCB_t   pfnNwkJoinEDReq;  // Network Join End Device Request command
  bdbTL_NwkUpdateReqCB_t   pfnNwkUpdateReq;  // Network Update Request command

  // Received Client Commands
  bdbTL_ScanRspCB_t        pfnScanRsp;       // Scan Response command
  bdbTL_DeviceInfoRspCB_t  pfnDeviceInfoRsp; // Device Information Response command
  bdbTL_NwkStartRspCB_t    pfnNwkStartRsp;   // Network Start Response command
  bdbTL_NwkJoinRtrRspCB_t  pfnNwkJoinRtrRsp; // Network Join Router Response command
  bdbTL_NwkJoinEDRspCB_t   pfnNwkJoinEDRsp;  // Network Join End Device Response command
} bdbTL_InterPANCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Device Information Table
extern bdbTLDeviceInfo_t *touchLinkSubDevicesTbl[5];

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t bdbTL_RegisterCmdCallbacks( uint8 endpoint, bdbTL_AppCallbacks_t *callbacks );

 /*
  * Register for callbacks from this cluster library for Inter-PAN commands
  */
extern ZStatus_t bdbTL_RegisterInterPANCmdCallbacks( bdbTL_InterPANCallbacks_t *callbacks );

/*
 * Call to send out an Scan Request
 */
extern ZStatus_t bdbTL_Send_ScanReq( uint8 srcEP, afAddrType_t *dstAddr,
                                     bdbTLScanReq_t *pReq, uint8 seqNum );
/*
 * Call to send out a Device Information Request
 */
ZStatus_t bdbTL_Send_DeviceInfoReq( uint8 srcEP, afAddrType_t *dstAddr,
                                    bdbTLDeviceInfoReq_t *pReq, uint8 seqNum );
/*
 * Call to send out an Identify Request
 */
extern ZStatus_t bdbTL_Send_IndentifyReq( uint8 srcEP, afAddrType_t *dstAddr,
                                          bdbTLIdentifyReq_t *pReq, uint8 seqNum );

/*
 * Call to send out a Reset to Factory New Request
 */
extern ZStatus_t bdbTL_Send_ResetToFNReq( uint8 srcEP, afAddrType_t *dstAddr,
                                          bdbTLResetToFNReq_t *pReq, uint8 seqNum );
/*
 * Call to send out a Network Start Request
 */
extern ZStatus_t bdbTL_Send_NwkStartReq( uint8 srcEP, afAddrType_t *dstAddr,
                                         bdbTLNwkStartReq_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Network Join Router/End Device Request
 */
extern ZStatus_t bdbTL_Send_NwkJoinReq( uint8 srcEP, afAddrType_t *dstAddr,
                                        bdbTLNwkJoinReq_t *pReq, uint8 cmd, uint8 seqNum );
/*
 * Call to send out a Network Update Request
 */
extern ZStatus_t bdbTL_Send_NwkUpdateReq( uint8 srcEP, afAddrType_t *dstAddr,
                                          bdbTLNwkUpdateReq_t *pReq, uint8 seqNum );
/*
 * Call to send out an Scan Response
 */
extern ZStatus_t bdbTL_Send_ScanRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                     bdbTLScanRsp_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Device Information Response
 */
extern ZStatus_t bdbTL_Send_DeviceInfoRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                           bdbTLDeviceInfoRsp_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Network Start Response
 */
extern ZStatus_t bdbTL_Send_NwkStartRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                         bdbTLNwkStartRsp_t *pRsp, uint8 seqNum );
/*
 * Call to send out a Network Join Router/End Device Response
 */
extern ZStatus_t bdbTL_Send_NwkJoinRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                        bdbTLNwkJoinRsp_t *pRsp, uint8 cmd, uint8 seqNum );
/*
 * Call to send out an Endpoint Information Response
 */
extern ZStatus_t bdbTL_Send_EndpointInfo( uint8 srcEP, afAddrType_t *dstAddr,
                                          bdbTLEndpointInfo_t *pCmd,
                                          uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send out a Get Group Identifiers Response
 */
extern ZStatus_t bdbTL_Send_GetGrpIDsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                          bdbTLGetGrpIDsRsp_t *pRsp,
                                          uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send out a Get Endpoint List Response
 */
extern ZStatus_t bdbTL_Send_GetEPListRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                          bdbTLGetEPListRsp_t *pRsp,
                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      bdbTL_Send_NwkJoinRtrReq
 *
 * @brief   Call to send out a Network Join Router Request command
 *
 *          Use like: bdbTL_Send_NwkJoinRtrReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                              bdbTLNwkJoinReq_t *pReq, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define bdbTL_Send_NwkJoinRtrReq( srcEP, dstAddr, pReq, seqNum ) \
                                  bdbTL_Send_NwkJoinReq( (srcEP), (dstAddr), (pReq), \
                                                         COMMAND_TOUCHLINK_NWK_JOIN_RTR_REQ, \
                                                         (seqNum) )

/*********************************************************************
 * @fn      bdbTL_Send_NwkJoinEDReq
 *
 * @brief   Call to send out a Network Join End Device Request command
 *
 *          Use like: bdbTL_Send_NwkJoinEDReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             bdbTLNwkJoinReq_t *pReq, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pReq - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define bdbTL_Send_NwkJoinEDReq( srcEP, dstAddr, pReq, seqNum ) \
                                 bdbTL_Send_NwkJoinReq( (srcEP), (dstAddr), (pReq), \
                                                        COMMAND_TOUCHLINK_NWK_JOIN_ED_REQ, \
                                                        (seqNum) )

/*********************************************************************
 * @fn      bdbTL_Send_GetGrpIDsReq
 *
 * @brief   Call to send out a Get Group Identifiers Request command
 *
 *          Use like: bdbTL_Send_GetGrpIDsReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             bdbTLGetGrpIDsReq_t *pReq,
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
#define bdbTL_Send_GetGrpIDsReq( srcEP, dstAddr, pReq, disableDefaultRsp, seqNum ) \
                                 zcl_SendCommand( (srcEP), (dstAddr), ZCL_CLUSTER_ID_LIGHT_LINK, \
                                                  COMMAND_TOUCHLINK_GET_GRP_IDS_REQ, TRUE, \
                                                  ZCL_FRAME_CLIENT_SERVER_DIR, (disableDefaultRsp), \
                                                  0, (seqNum), 1, &((pReq)->startIndex) )

/*********************************************************************
 * @fn      bdbTL_Send_GetEPListReq
 *
 * @brief   Call to send out a Get Endpoint List Request command
 *
 *          Use like: bdbTL_Send_GetEPListReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             bdbTLGetEPListReq_t *pReq,
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
#define bdbTL_Send_GetEPListReq( srcEP, dstAddr, pReq, disableDefaultRsp, seqNum ) \
                                 zcl_SendCommand( (srcEP), (dstAddr), ZCL_CLUSTER_ID_LIGHT_LINK, \
                                                  COMMAND_TOUCHLINK_GET_EP_LIST_REQ, TRUE, \
                                                  ZCL_FRAME_CLIENT_SERVER_DIR, (disableDefaultRsp), \
                                                  0, (seqNum), 1, &((pReq)->startIndex) )

/*********************************************************************
 * @fn      bdbTL_Send_NwkJoinRtrRsp
 *
 * @brief   Call to send out a Network Join Router Response command
 *
 *          Use like: bdbTL_Send_NwkJoinRtrRsp( uint8 srcEP, afAddrType_t *dstAddr,
 *                                              bdbTLNwkJoinRsp_t *pRsp, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define bdbTL_Send_NwkJoinRtrRsp( srcEP, dstAddr, pRsp, seqNum ) \
                                  bdbTL_Send_NwkJoinRsp( (srcEP), (dstAddr), (pRsp), \
                                                         COMMAND_TOUCHLINK_NWK_JOIN_RTR_RSP, \
                                                         (seqNum) )

/*********************************************************************
 * @fn      bdbTL_Send_NwkJoinEDRsp
 *
 * @brief   Call to send out a Network Join End Device Response command
 *
 *          Use like: bdbTL_Send_NwkJoinEDRsp( uint8 srcEP, afAddrType_t *dstAddr,
 *                                             bdbTLNwkJoinReq_t *pRsp, uint8 seqNum );
 *
 * @param   srcEP - sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pRsp - response parameters
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
#define bdbTL_Send_NwkJoinEDRsp( srcEP, dstAddr, pRsp, seqNum ) \
                                 bdbTL_Send_NwkJoinRsp( (srcEP), (dstAddr), (pRsp), \
                                                        COMMAND_TOUCHLINK_NWK_JOIN_ED_RSP, \
                                                        (seqNum) )


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BDB_TLCOMMISSIONING_H */
