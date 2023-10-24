/**************************************************************************************************
  Filename:       ZDProfile.h
  Revised:        $Date: 2015-02-10 15:42:13 -0800 (Tue, 10 Feb 2015) $
  Revision:       $Revision: 42485 $

  Description:    This file contains the interface to the Zigbee Device Object.


  Copyright 2004-2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZDPROFILE_H
#define ZDPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "NLMEDE.h"
#include "AF.h"
#include "ZDConfig.h"

/*********************************************************************
 * CONSTANTS
 */

#define ZDO_EP 0   // Endpoint of ZDO
#define ZDO_PROFILE_ID            0
#define ZDO_WILDCARD_PROFILE_ID   0xFFFF

// IEEE_addr_req request types
#define ZDP_ADDR_REQTYPE_SINGLE     0
#define ZDP_ADDR_REQTYPE_EXTENDED   1

// ZDO Status Values
#define ZDP_SUCCESS            0x00  // Operation completed successfully
#define ZDP_INVALID_REQTYPE    0x80  // The supplied request type was invalid
#define ZDP_DEVICE_NOT_FOUND   0x81  // Reserved
#define ZDP_INVALID_EP         0x82  // Invalid endpoint value
#define ZDP_NOT_ACTIVE         0x83  // Endpoint not described by a simple desc.
#define ZDP_NOT_SUPPORTED      0x84  // Optional feature not supported
#define ZDP_TIMEOUT            0x85  // Operation has timed out
#define ZDP_NO_MATCH           0x86  // No match for end device bind
#define ZDP_NO_ENTRY           0x88  // Unbind request failed, no entry
#define ZDP_NO_DESCRIPTOR      0x89  // Child descriptor not available
#define ZDP_INSUFFICIENT_SPACE 0x8a  // Insufficient space to support operation
#define ZDP_NOT_PERMITTED      0x8b  // Not in proper state to support operation
#define ZDP_TABLE_FULL         0x8c  // No table space to support operation
#define ZDP_NOT_AUTHORIZED     0x8d  // Permissions indicate request not authorized
#define ZDP_BINDING_TABLE_FULL 0x8e  // No binding table space to support operation

#define ZDP_NETWORK_DISCRIPTOR_SIZE  8
#define ZDP_NETWORK_EXTENDED_DISCRIPTOR_SIZE  14
#define ZDP_RTG_DISCRIPTOR_SIZE      5
#define ZDP_BIND_DISCRIPTOR_SIZE  19

// Mgmt_Permit_Join_req fields
#define ZDP_MGMT_PERMIT_JOIN_REQ_DURATION 0
#define ZDP_MGMT_PERMIT_JOIN_REQ_TC_SIG   1
#define ZDP_MGMT_PERMIT_JOIN_REQ_SIZE     2

// Mgmt_Leave_req fields
#define ZDP_MGMT_LEAVE_REQ_REJOIN      1 << 7
#define ZDP_MGMT_LEAVE_REQ_RC          1 << 6   // Remove Children

// Mgmt_Lqi_rsp - (neighbor table) device type
#define ZDP_MGMT_DT_COORD  0x0
#define ZDP_MGMT_DT_ROUTER 0x1
#define ZDP_MGMT_DT_ENDDEV 0x2

// Mgmt_Lqi_rsp - (neighbor table) relationship
#define ZDP_MGMT_REL_PARENT  0x0
#define ZDP_MGMT_REL_CHILD   0x1
#define ZDP_MGMT_REL_SIBLING 0x2
#define ZDP_MGMT_REL_UNKNOWN 0x3

// Mgmt_Lqi_rsp - (neighbor table) unknown boolean value
#define ZDP_MGMT_BOOL_RECEIVER_ON  0x00
#define ZDP_MGMT_BOOL_RECEIVER_OFF 0x01  
#define ZDP_MGMT_BOOL_UNKNOWN      0x02

// Status fields used by ZDO_ProcessMgmtRtgReq
#define ZDO_MGMT_RTG_ENTRY_ACTIVE                 0x00
#define ZDO_MGMT_RTG_ENTRY_DISCOVERY_UNDERWAY     0x01
#define ZDO_MGMT_RTG_ENTRY_DISCOVERY_FAILED       0x02
#define ZDO_MGMT_RTG_ENTRY_INACTIVE               0x03

#define ZDO_MGMT_RTG_ENTRY_MEMORY_CONSTRAINED     0x01
#define ZDO_MGMT_RTG_ENTRY_MANYTOONE              0x02
#define ZDO_MGMT_RTG_ENTRY_ROUTE_RECORD_REQUIRED  0x04

/*********************************************************************
 * Message/Cluster IDS
 */

// ZDO Cluster IDs
#define ZDO_RESPONSE_BIT_V1_0   ((uint8)0x80)
#define ZDO_RESPONSE_BIT        ((uint16)0x8000)

#define NWK_addr_req            ((uint16)0x0000)
#define IEEE_addr_req           ((uint16)0x0001)
#define Node_Desc_req           ((uint16)0x0002)
#define Power_Desc_req          ((uint16)0x0003)
#define Simple_Desc_req         ((uint16)0x0004)
#define Active_EP_req           ((uint16)0x0005)
#define Match_Desc_req          ((uint16)0x0006)
#define NWK_addr_rsp            (NWK_addr_req | ZDO_RESPONSE_BIT)
#define IEEE_addr_rsp           (IEEE_addr_req | ZDO_RESPONSE_BIT)
#define Node_Desc_rsp           (Node_Desc_req | ZDO_RESPONSE_BIT)
#define Power_Desc_rsp          (Power_Desc_req | ZDO_RESPONSE_BIT)
#define Simple_Desc_rsp         (Simple_Desc_req | ZDO_RESPONSE_BIT)
#define Active_EP_rsp           (Active_EP_req | ZDO_RESPONSE_BIT)
#define Match_Desc_rsp          (Match_Desc_req | ZDO_RESPONSE_BIT)

#define Complex_Desc_req        ((uint16)0x0010)
#define User_Desc_req           ((uint16)0x0011)
#define Discovery_Cache_req     ((uint16)0x0012)
#define Device_annce            ((uint16)0x0013)
#define User_Desc_set           ((uint16)0x0014)
#define Server_Discovery_req    ((uint16)0x0015)
#define Parent_annce            ((uint16)0x001F)
#define Complex_Desc_rsp        (Complex_Desc_req | ZDO_RESPONSE_BIT)
#define User_Desc_rsp           (User_Desc_req | ZDO_RESPONSE_BIT)
#define Discovery_Cache_rsp     (Discovery_Cache_req | ZDO_RESPONSE_BIT)
#define User_Desc_conf          (User_Desc_set | ZDO_RESPONSE_BIT)
#define Server_Discovery_rsp    (Server_Discovery_req | ZDO_RESPONSE_BIT)
#define Parent_annce_rsp        (Parent_annce | ZDO_RESPONSE_BIT)

#define End_Device_Bind_req     ((uint16)0x0020)
#define Bind_req                ((uint16)0x0021)
#define Unbind_req              ((uint16)0x0022)
#define Bind_rsp                (Bind_req | ZDO_RESPONSE_BIT)
#define End_Device_Bind_rsp     (End_Device_Bind_req | ZDO_RESPONSE_BIT)
#define Unbind_rsp              (Unbind_req | ZDO_RESPONSE_BIT)

#define Mgmt_NWK_Disc_req       ((uint16)0x0030)
#define Mgmt_Lqi_req            ((uint16)0x0031)
#define Mgmt_Rtg_req            ((uint16)0x0032)
#define Mgmt_Bind_req           ((uint16)0x0033)
#define Mgmt_Leave_req          ((uint16)0x0034)
#define Mgmt_Direct_Join_req    ((uint16)0x0035)
#define Mgmt_Permit_Join_req    ((uint16)0x0036)
#define Mgmt_NWK_Update_req     ((uint16)0x0038)
#define Mgmt_NWK_Disc_rsp       (Mgmt_NWK_Disc_req | ZDO_RESPONSE_BIT)
#define Mgmt_Lqi_rsp            (Mgmt_Lqi_req | ZDO_RESPONSE_BIT)
#define Mgmt_Rtg_rsp            (Mgmt_Rtg_req | ZDO_RESPONSE_BIT)
#define Mgmt_Bind_rsp           (Mgmt_Bind_req | ZDO_RESPONSE_BIT)
#define Mgmt_Leave_rsp          (Mgmt_Leave_req | ZDO_RESPONSE_BIT)
#define Mgmt_Direct_Join_rsp    (Mgmt_Direct_Join_req | ZDO_RESPONSE_BIT)
#define Mgmt_Permit_Join_rsp    (Mgmt_Permit_Join_req | ZDO_RESPONSE_BIT)
#define Mgmt_NWK_Update_notify  (Mgmt_NWK_Update_req | ZDO_RESPONSE_BIT)

#define ZDO_ALL_MSGS_CLUSTERID  0xFFFF

/*********************************************************************
 * TYPEDEFS
 */

#define ZDP_BINDINGENTRY_SIZE   19

typedef struct
{
  osal_event_hdr_t hdr;
  zAddrType_t      srcAddr;
  uint8            wasBroadcast;
  cId_t            clusterID;
  uint8            SecurityUse;
  uint8            TransSeq;
  uint8            asduLen;
  uint16           macDestAddr;
  uint8            *asdu;
  uint16           macSrcAddr;
} zdoIncomingMsg_t;

// This structure is used to build the Mgmt Network Discover Response
typedef struct
{
  uint8 extendedPANID[Z_EXTADDR_LEN];   // The extended PAN ID
  uint16 PANId;            // The network PAN ID
  uint8   logicalChannel;  // Network's channel
  uint8   stackProfile;    // Network's profile
  uint8   version;         // Network's Zigbee version
  uint8   beaconOrder;     // Beacon Order
  uint8   superFrameOrder;
  uint8   permitJoining;   // PermitJoining. 1 or 0
} mgmtNwkDiscItem_t;

// This structure is used to build the Mgmt LQI Response
typedef struct
{
  uint16 nwkAddr;         // device's short address
  uint16 PANId;           // The neighbor device's PAN ID
  uint8  extPANId[Z_EXTADDR_LEN]; // The neighbor device's Extended PanID
  uint8   txQuality;       // Transmit quality
  uint8   rxLqi;           // Receive LQI
} neighborLqiItem_t;
#define ZDP_NEIGHBORLQI_SIZE    12

// This structure is used to build the Mgmt_Lqi_rsp
typedef struct
{
  uint16 panID;                  // PAN Id
  uint8  extPanID[Z_EXTADDR_LEN];// Extended Pan ID
  uint8  extAddr[Z_EXTADDR_LEN]; // Extended address
  uint16 nwkAddr;                // Network address
  uint8  devType;                // Device type
  uint8  rxOnIdle;               // RxOnWhenIdle
  uint8  relation;               // Relationship
  uint8  permit;                 // Permit joining
  uint8  depth;                  // Depth
  uint8  lqi;                    // LQI
} ZDP_MgmtLqiItem_t;
// devType, rxOnIdle, relation, are all packed into 1 byte: 18-2=16.
#define ZDP_MGMTLQI_SIZE           15
#define ZDP_MGMTLQI_EXTENDED_SIZE  22   // One extra byte for permitJointing, also with extended PanID instead of PanID 15+8-2+1 = 22

// This structure is used to build the Mgmt Routing Response
// NOTICE: this structure must match "rtgEntry_t" in rtg.h
typedef struct
{
  uint16 dstAddress;     // Destination short address
  uint16 nextHopAddress; // next hop short address
  uint8  expiryTime;     // expiration time - not used for response
  uint8  status;         // route entry status
  uint8  options;
} rtgItem_t;
// expiryTime is not packed & sent OTA.
#define ZDP_ROUTINGENTRY_SIZE   5

typedef struct
{
  uint8  TransSeq;
  byte SecurityUse;
  uint16 srcAddr;
  uint16 localCoordinator;
  uint8  ieeeAddr[Z_EXTADDR_LEN];
  uint8  endpoint;
  uint16 profileID;
  uint8  numInClusters;
  uint16 *inClusters;
  uint8  numOutClusters;
  uint16 *outClusters;
} ZDEndDeviceBind_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern byte ZDP_TransID;
extern byte ZDP_TxOptions;

/*********************************************************************
 * MACROS
 */

/*
 * Generic data send function
 */
extern afStatus_t ZDP_SendData( uint8 *transSeq, zAddrType_t *dstAddr, uint16 cmd, byte len,
                                              uint8 *buf, byte SecurityEnable );

/*
 * ZDP_NodeDescReq - Request a Node Description
 *
 * @MT SPI_CMD_ZDO_NODE_DESC_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_NodeDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                              NWKAddrOfInterest, Node_Desc_req, SecurityEnable )

/*
 * ZDP_PowerDescReq - Request a Power Description
 *
 * @MT  SPI_CMD_ZDO_POWER_DESC_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte SecuritySuit)
 *
 */
#define ZDP_PowerDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                              NWKAddrOfInterest, Power_Desc_req, SecurityEnable )

/*
 * ZDP_ActiveEPReq - Request a device's endpoint list
 *
 * @MT SPI_CMD_ZDO_ACTIVE_EPINT_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_ActiveEPReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                            NWKAddrOfInterest, Active_EP_req, SecurityEnable )

/*
 * ZDP_ComplexDescReq - Request a device's complex description
 *
 * @MT SPI_CMD_ZDO_COMPLEX_DESC_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_ComplexDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                            NWKAddrOfInterest, Complex_Desc_req, SecurityEnable )

/*
 * ZDP_UserDescReq - Request a device's user description
 *
 * @MT SPI_CMD_ZDO_USER_DESC_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte SecuritySuite)
 *
 */
#define ZDP_UserDescReq( dstAddr, NWKAddrOfInterest, SecurityEnable ) \
                          ZDP_NWKAddrOfInterestReq(  dstAddr, \
                            NWKAddrOfInterest, User_Desc_req, SecurityEnable )

/*
 * ZDP_BindReq - bind request
 *
 * @MT SPI_CMD_ZDO_BIND_REQ
 * (UInt16 DstAddr,
 *  UInt64 SrcAddress,
 *  byte SrcEndpoint,
 *  uint16 ClusterID,
 *  zAddrType_t *DstAddress,
 *  byte DstEndpoint,
 *  byte SecuritySuite)
 *
 */
#define ZDP_BindReq( dstAddr, SourceAddr, SrcEP, \
              ClusterID, DestinationAddr, DstEP, SecurityEnable ) \
                       ZDP_BindUnbindReq( Bind_req, dstAddr, \
                            SourceAddr, SrcEP, ClusterID, \
                            DestinationAddr, DstEP, SecurityEnable )

/*
 * ZDP_UnbindReq - Unbind request
 *
 * @MT SPI_CMD_ZDO_UNBIND_REQ
 * (UInt16 DstAddr,
 *  UInt64 SrcAddress,
 *  byte SrcEndpoint,
 *  uint16 ClusterID,
 *  zAddrType_t DestinationAddr,
 *  byte DstEndpoint,
 *  byte SecuritySuite)
 *
 */
#define ZDP_UnbindReq( dstAddr, SourceAddr, SrcEP, \
              ClusterID, DestinationAddr, DstEP, SecurityEnable ) \
                       ZDP_BindUnbindReq( Unbind_req, dstAddr, \
                            SourceAddr, SrcEP, ClusterID, \
                            DestinationAddr, DstEP, SecurityEnable )

/*
 * ZDP_MgmtLqiReq - Send a Management LQI Request
 *
 * @MT SPI_CMD_ZDO_MGMT_LQI_REQ
 * (UInt16 DstAddr,
 *  byte StartIndex)
 *
 */
#define ZDP_MgmtLqiReq( dstAddr, StartIndex, SecurityEnable ) \
          ZDP_SendData( &ZDP_TransID, dstAddr, Mgmt_Lqi_req, 1, &StartIndex, SecurityEnable )

/*
 * ZDP_MgmtRtgReq - Send a Management Routing Table Request
 *
 * @MT SPI_CMD_ZDO_MGMT_RTG_REQ
 * (UInt16 DstAddr,
 *  byte StartIndex)
 *
 */
#define ZDP_MgmtRtgReq( dstAddr, StartIndex, SecurityEnable ) \
          ZDP_SendData( &ZDP_TransID, dstAddr, Mgmt_Rtg_req, 1, &StartIndex, SecurityEnable )

/*
 * ZDP_MgmtBindReq - Send a Management Binding Table Request
 *
 * @MT SPI_CMD_ZDO_MGMT_BIND_REQ
 * (UInt16 DstAddr,
 *  byte StartIndex)
 *
 */
#define ZDP_MgmtBindReq( dstAddr, StartIndex, SecurityEnable ) \
         ZDP_SendData( &ZDP_TransID, dstAddr, Mgmt_Bind_req, 1, &StartIndex, SecurityEnable )

/*
 * ZDP_ParentAnnceReq - Send a ParentAnnce Request
 */
#define ZDP_ParentAnnceReq( dstAddr, numberOfChildren, childInfo, SecurityEnable ) \
           ZDP_ParentAnnce( &ZDP_TransID, &dstAddr, numberOfChildren, childInfo, \
                            Parent_annce, SecurityEnable )

/*
 * ZDP_ActiveEPRsp - Send an list of active endpoint
 */
#define ZDP_ActiveEPRsp( TransSeq, dstAddr, Status, nwkAddr, Count, \
                  pEPList, SecurityEnable ) \
                      ZDP_EPRsp( Active_EP_rsp, TransSeq, dstAddr, Status, \
                           nwkAddr, Count, pEPList, SecurityEnable )

/*
 * ZDP_MatchDescRsp - Send an list of endpoint that match
 */
#define ZDP_MatchDescRsp( TransSeq, dstAddr, Status, nwkAddr, Count, \
                  pEPList, SecurityEnable ) \
                      ZDP_EPRsp( Match_Desc_rsp, TransSeq, dstAddr, Status, \
                           nwkAddr, Count, pEPList, SecurityEnable )

/*
 * ZDP_ComplexDescRsp - This message isn't supported until we fix it.
 */
#define ZDP_ComplexDescRsp( dstAddr, SecurityEnable ) \
        ZDP_GenericRsp( dstAddr, Complex_Desc_rsp, SecurityEnable )

/*
 * ZDP_UserDescConf - Send a User Descriptor Set Response
 */
#define ZDP_UserDescConf( TransSeq, dstAddr, Status, SecurityEnable ) \
            ZDP_SendData( &TransSeq, dstAddr, User_Desc_conf, 1, &Status, SecurityEnable )

/*
 * ZDP_EndDeviceBindRsp - Send a End Device Bind Response
 */
#define ZDP_EndDeviceBindRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
       ZDP_SendData( &TransSeq, dstAddr, End_Device_Bind_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_BindRsp - Send a Bind Response
 */
#define ZDP_BindRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
                  ZDP_SendData( &TransSeq, dstAddr, Bind_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_UnbindRsp - Send an Unbind Response
 */
#define ZDP_UnbindRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
                ZDP_SendData( &TransSeq, dstAddr, Unbind_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_MgmtLeaveRsp - Send a Management Leave Response
 */
#define ZDP_MgmtLeaveRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
            ZDP_SendData( &TransSeq, dstAddr, Mgmt_Leave_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_MgmtPermitJoinRsp - Send a Management Permit Join Response
 */
#define ZDP_MgmtPermitJoinRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
      ZDP_SendData( &TransSeq, dstAddr, Mgmt_Permit_Join_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_MgmtDirectJoinRsp - Send a Mgmt_DirectJoining_Rsp Response
 */
#define ZDP_MgmtDirectJoinRsp( TransSeq, dstAddr, Status, SecurityEnable ) \
      ZDP_SendData( &TransSeq, dstAddr, Mgmt_Direct_Join_rsp, 1, &Status, SecurityEnable )

/*
 * ZDP_ParentAnnceRsp - Send a ParentAnnceRsp Response
 */
#define ZDP_ParentAnnceRsp( TransSeq, dstAddr, numberOfChildren, childInfo, SecurityEnable ) \
           ZDP_ParentAnnce( &TransSeq, &dstAddr, numberOfChildren, childInfo, \
                            Parent_annce_rsp, SecurityEnable )

/*********************************************************************
 * FUNCTIONS - API
 */

/*
 * ZDP_NWKAddrOfInterestReq - Send request with NWKAddrOfInterest as parameter
 */
extern afStatus_t ZDP_NWKAddrOfInterestReq( zAddrType_t *dstAddr,
                              uint16 nwkAddr, byte cmd, byte SecurityEnable );
/*
 * ZDP_NwkAddrReq - Request a Network address
 *
 * @MT SPI_CMD_ZDO_NWK_ADDR_REQ
 * (UInt64 IEEEAddress,
 *  byte ReqType,
 *  byte StarIndex,
 *  byte SecurityEnable)
 *
 */
extern afStatus_t ZDP_NwkAddrReq( uint8 *IEEEAddress, byte ReqType,
                               byte StartIndex, byte SecurityEnable );

/*
 * ZDP_IEEEAddrReq - Request an IEEE address
 *
 * @MT SPI_CMD_ZDO_IEEE_ADDR_REQ
 * (UInt16 shortAddr,
 *  byte ReqType,
 *  byte StartIndex,
 *  byte SecurityEnable)
 *
 */
extern afStatus_t ZDP_IEEEAddrReq( uint16 shortAddr, byte ReqType,
                                byte StartIndex, byte SecurityEnable );

/*
 * ZDP_MatchDescReq - Request matching device's endpoint list
 *
 * @MT SPI_CMD_ZDO_MATCH_DESC_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  UInt16 ProfileID,
 *  byte NumInClusters,
 *  uint16 InClusterList[15],
 *  byte NumOutClusters,
 *  uint16 OutClusterList[15],
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_MatchDescReq( zAddrType_t *dstAddr, uint16 nwkAddr,
                                uint16 ProfileID,
                                byte NumInClusters, uint16 *InClusterList,
                                byte NumOutClusters, uint16 *OutClusterList,
                                byte SecurityEnable );

/*
 * ZDP_SimpleDescReq - Request Simple Descriptor
 *
 * @MT SPI_CMD_ZDO_SIMPLE_DESC_REQ
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte Endpoint,
 *  byte Security)
 *
 */
extern afStatus_t ZDP_SimpleDescReq( zAddrType_t *dstAddr, uint16 nwkAddr,
                                    byte ep, byte SecurityEnable );

/*
 * ZDP_UserDescSet - Set the User Descriptor
 *
 * @MT SPI_CMD_ZDO_USER_DESC_SET
 * (UInt16 DstAddr,
 *  UInt16 NWKAddrOfInterest,
 *  byte DescLen,
 *  byte Descriptor[15],
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_UserDescSet( zAddrType_t *dstAddr, uint16 nwkAddr,
                          UserDescriptorFormat_t *UserDescriptor,
                          byte SecurityEnable );

/*
 * ZDP_ServerDiscReq - Build and send a Server_Discovery_req request message.
 */
afStatus_t ZDP_ServerDiscReq( uint16 serverMask, byte SecurityEnable );

/*
 * ZDP_DeviceAnnce - Device Announce
 *
 * @MT SPI_CMD_ZDO_DEV_ANNCE
 * (UInt16 DevAddr,
 *  byte DeviceAddress,
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_DeviceAnnce( uint16 nwkAddr, uint8 *IEEEAddr,
                         byte capabilities, byte SecurityEnable );

/*
 * ZDP_ParentAnnce - Parent Announce and Parent Announce Rsp
 */
extern afStatus_t ZDP_ParentAnnce( uint8 *TransSeq,
                                   zAddrType_t *dstAddr,
                                   uint8 numberOfChildren,
                                   uint8 *childInfo,
                                   cId_t clusterID,
                                   uint8 SecurityEnable );

/*
 * ZDP_EndDeviceBindReq - End Device (hand) bind request
 *
 * @MT SPI_CMD_ZDO_END_DEV_BIND_REQ
 * (UInt16 DstAddr,
 *  UInt16 LocalCoordinator,
 *  byte Endpoint,
 *  UInt16 Profile,
 *  byte NumInClusters,
 *  uint16 InClusterList[15],
 *  byte NumOutClusters,
 *  uint16 OutClusterList[15],
 *  byte SecuritySuite)
 *
 */
extern afStatus_t ZDP_EndDeviceBindReq( zAddrType_t *dstAddr,
                              uint16 LocalCoordinator,
                              byte ep,
                              uint16 ProfileID,
                              byte NumInClusters, uint16 *InClusterList,
                              byte NumOutClusters, uint16 *OutClusterList,
                              byte SecurityEnable );

/*
 * ZDP_BindUnbindReq - bind request
 */
extern afStatus_t ZDP_BindUnbindReq( uint16 BindOrUnbind, zAddrType_t *dstAddr,
                            uint8 *SourceAddr, byte SrcEP,
                            cId_t  ClusterID,
                            zAddrType_t *DestinationAddr, byte DstEP,
                            byte SecurityEnable );

/*
 * ZDP_MgmtNwkDiscReq - Send a Management Network Discovery Request
 *
 * @MT SPI_CMD_ZDO_MGMT_NWKDISC_REQ
 * (UInt16 DstAddr,
 *  UInt32 ScanChannels,
 *  byte StartIndex)
 *
 */
extern afStatus_t ZDP_MgmtNwkDiscReq( zAddrType_t *dstAddr,
                            uint32 ScanChannels,
                            byte ScanDuration,
                            byte StartIndex,
                            byte SecurityEnable );

/*
 * ZDP_MgmtDirectJoinReq - Send a Management Direct Join Request
 *
 * @MT SPI_CMD_ZDO_MGMT_DIRECT_JOIN_REQ
 * (UInt16 DstAddr,
 *  UInt64 DeviceAddress,
 *  byte CapInfo)
 *
 */
extern afStatus_t ZDP_MgmtDirectJoinReq( zAddrType_t *dstAddr,
                               uint8 *deviceAddr,
                               byte capInfo,
                               byte SecurityEnable );

/*
 * ZDP_MgmtLeaveReq - Send a Management Leave Request
 *
 * @MT SPI_CMD_ZDO_MGMT_LEAVE_REQ
 * (UInt16 DstAddr,
 *  UInt64 DeviceAddress
 *  uint8 RemoveChildren
 *  uint8 Rejoin
 *  uint8 SecurityEnable)
 */
extern afStatus_t ZDP_MgmtLeaveReq( zAddrType_t *dstAddr,
                                   uint8 *IEEEAddr,
                                   uint8 RemoveChildren,
                                   uint8 Rejoin,
                                   uint8 SecurityEnable );
/*
 * ZDP_MgmtPermitJoinReq - Send a Management Permit Join Request
 *
 * @MT SPI_CMD_ZDO_MGMT_PERMIT_JOIN_REQ
 * (UInt16 DstAddr,
 *  byte duration,
 *  byte TcSignificance)
 *
 */
extern afStatus_t ZDP_MgmtPermitJoinReq( zAddrType_t *dstAddr,
                               byte duration,
                               byte TcSignificance,
                               byte SecurityEnable );

/*
 * ZDP_MgmtNwkUpdateReq - Send a Management NWK Update Request
 *
 * @MT SPI_CMD_ZDO_MGMT_NWK_UPDATE_REQ
 * (uint16 dstAddr,
 *  uint32 ChannelMask,
 *  uint8 ScanDuration,
 *  uint8 ScanCount,
 *  uint16 NwkManagerAddr )
 *
 */
extern afStatus_t ZDP_MgmtNwkUpdateReq( zAddrType_t *dstAddr,
                                        uint32 ChannelMask,
                                        uint8 ScanDuration,
                                        uint8 ScanCount,
                                        uint8 NwkUpdateId,
                                        uint16 NwkManagerAddr );

/*********************************************************************
 * @fn      ZDP_AddrRsp
 *
 * @brief   Build and send a NWK_addr_rsp or IEEE_addr_rsp message.
 *
 * @param   cId - ClusterID of the rsp, either NWK_addr_rsp or IEEE_addr_rsp.
 * @param   seq - Message sequence number of the corresponding request.
 * @param   dst - Destination address for the response.
 * @param   stat - Response status: ZDP_SUCCESS or other value from ZDProfile.h
 * @param   ieee - 64-bit IEEE address of the response.
 * @param   reqType - Type of response requested (single, extended, etc.)
 * @param   nwkAddr - 16-bit network address of the response.
 * @param   devCnt  - Number of associated devices in the device address list.
 * @param   strtIdx - Starting index into the dev addr array if extended rsp.
 * @param   devAddr - Array of 16-bit network addresses of the associated devs.
 * @param   secOpt  - Security Enable Options.
 *
 * @return  afStatus_t
 */
afStatus_t ZDP_AddrRsp( byte cId, byte seq, zAddrType_t *dst, byte stat,
  uint8 *ieee, byte reqType, uint16 nwkAddr, byte devCnt, byte strtIdx,
  uint16 *devAddr, byte secOpt );

/*
 * ZDP_NodeDescMsg - Send a Node Descriptor message.
 */
extern afStatus_t ZDP_NodeDescMsg( zdoIncomingMsg_t *inMsg,
                    uint16 nwkAddr, NodeDescriptorFormat_t *pNodeDesc );

/*
 * ZDP_PowerDescMsg - Send a Node Power Descriptor message.
 */
extern afStatus_t ZDP_PowerDescMsg( zdoIncomingMsg_t *inMsg,
 uint16 nwkAddr, NodePowerDescriptorFormat_t *pPowerDesc );

/*
 * ZDP_SimpleDescMsg - Send a Simple Descriptor message.
 */
extern afStatus_t ZDP_SimpleDescMsg( zdoIncomingMsg_t *inMsg,
                     byte Status, SimpleDescriptionFormat_t *pSimpleDesc );

/*
 * ZDP_EPRsp - Send a list of endpoint
 */
extern afStatus_t ZDP_EPRsp( uint16 MsgType, byte TransSeq, zAddrType_t *dstAddr, byte Status,
                                uint16 nwkAddr, byte Count, uint8 *pEPList,
                                byte SecurityEnable );

/*
 * ZDP_GenericRsp - Sends a response message with only the parameter response
 *                                     byte and the addr of interest for data.
 */
extern afStatus_t ZDP_GenericRsp( byte TransSeq, zAddrType_t *dstAddr,
                    byte status, uint16 aoi, uint16 rspID, byte SecurityEnable );

/*
 * ZDP_MgmtNwkDiscRsp - Sends the Management Network Discovery Response.
 */
extern afStatus_t ZDP_MgmtNwkDiscRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte NetworkCount,
                            byte StartIndex,
                            byte NetworkCountList,
                            networkDesc_t *NetworkList,
                            byte SecurityEnable );

/*
 * ZDP_MgmtLqiRsp - Sends the Management LQI Response.
 */
extern ZStatus_t ZDP_MgmtLqiRsp( byte TransSeq, zAddrType_t *dstAddr,
                          byte Status,
                          byte NeighborLqiEntries,
                          byte StartIndex,
                          byte NeighborLqiCount,
                          ZDP_MgmtLqiItem_t* NeighborList,
                          byte SecurityEnable );

/*
 * ZDP_MgmtRtgRsp - Sends the Management Routing Response.
 */
extern ZStatus_t ZDP_MgmtRtgRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte RoutingTableEntries,
                            byte StartIndex,
                            byte RoutingListCount,
                            rtgItem_t *RoutingTableList,
                            byte SecurityEnable );

/*
 * ZDP_MgmtBindRsp - Sends the Management Binding Response.
 */
extern ZStatus_t ZDP_MgmtBindRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte BindingTableEntries,
                            byte StartIndex,
                            byte BindingTableListCount,
                            apsBindingItem_t *BindingTableList,
                            byte SecurityEnable );
/*
 * ZDP_MgmtNwkUpdateNotify - Sends the Management Netwotk Update Notify.
 */
extern afStatus_t ZDP_MgmtNwkUpdateNotify( uint8 TransSeq, zAddrType_t *dstAddr,
                                    uint8 status, uint32 scannedChannels,
                                    uint16 totalTransmissions, uint16 transmissionFailures,
                                    uint8 listCount, uint8 *energyValues, uint8 txOptions,
                                    uint8 securityEnable );

/*
 * ZDP_UserDescRsp - Sends the user descriptor response message.
 */
extern ZStatus_t ZDP_UserDescRsp( byte TransSeq, zAddrType_t *dstAddr,
                uint16 nwkAddrOfInterest, UserDescriptorFormat_t *userDesc,
                byte SecurityEnable );

/*
 * ZDP_ServerDiscRsp - Build and send the User Decriptor Response.
 */
ZStatus_t ZDP_ServerDiscRsp( byte transID, zAddrType_t *dstAddr, byte status,
                           uint16 aoi, uint16 serverMask, byte SecurityEnable );

/*
 * ZDP_IncomingData - Incoming data callback from AF layer
 */
extern void ZDP_IncomingData( afIncomingMSGPacket_t *pData );

extern ZStatus_t ZDO_RegisterForZDOMsg( uint8 taskID, uint16 clusterID );
extern ZStatus_t ZDO_RemoveRegisteredCB( uint8 taskID, uint16 clusterID );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDPROFILE_H */
