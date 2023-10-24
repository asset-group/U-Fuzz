/**************************************************************************************************
  Filename:       NLMEDE.h
  Revised:        $Date: 2015-06-02 15:55:43 -0700 (Tue, 02 Jun 2015) $
  Revision:       $Revision: 43961 $

  Description:    Network layer interface NLME and NLDE.


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

#ifndef NLMEDE_H
#define NLMEDE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZMAC.h"
#include "AssocList.h"
#include "nwk_bufs.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Tx Options (bitmap values)
#define TX_OPTIONS_GTS              0x01
#define TX_OPTIONS_SECURITY_ENABLE  0x02

// Route Discovery Options
#define DISC_ROUTE_NONE     0x00  // Don't discover route
#define DISC_ROUTE_NETWORK  0x01  // If a route is needed, the device (also
                                  // intermediate router) will issue  a route
                                  // disc request.
#define DISC_ROUTE_INITIATE 0x04  // Only the source router initiates route req.

// Beacon Order Values
#define BEACON_ORDER_NO_BEACONS     15
#define BEACON_ORDER_4_MINUTES      14  // 245760 milliseconds
#define BEACON_ORDER_2_MINUTES      13  // 122880 milliseconds
#define BEACON_ORDER_1_MINUTE       12  //  61440 milliseconds
#define BEACON_ORDER_31_SECONDS     11  //  30720 milliseconds
#define BEACON_ORDER_15_SECONDS     10  //  15360 MSecs
#define BEACON_ORDER_7_5_SECONDS     9  //   7680 MSecs
#define BEACON_ORDER_4_SECONDS       8  //   3840 MSecs
#define BEACON_ORDER_2_SECONDS       7  //   1920 MSecs
#define BEACON_ORDER_1_SECOND        6  //    960 MSecs
#define BEACON_ORDER_480_MSEC        5
#define BEACON_ORDER_240_MSEC        4
#define BEACON_ORDER_120_MSEC        3
#define BEACON_ORDER_60_MSEC         2
#define BEACON_ORDER_30_MSEC         1
#define BEACON_ORDER_15_MSEC                 0

#define STARTING_SCAN_DURATION       5
#define MAX_SCAN_DURATION           15
#define ENERGY_SCAN_INCREMENT       16

/* Definition of scan application */
#define NLME_ED_SCAN                 0
#define NLME_DISC_SCAN               1
#define NLME_PID_SCAN                2

// CapabilityFlags Bitmap values
#define CAPINFO_ALTPANCOORD           0x01
#define CAPINFO_DEVICETYPE_FFD        0x02
#define CAPINFO_DEVICETYPE_RFD        0x00
#define CAPINFO_POWER_AC              0x04
#define CAPINFO_RCVR_ON_IDLE          0x08
#define CAPINFO_SECURITY_CAPABLE      0x40
#define CAPINFO_ALLOC_ADDR            0x80

// ***********************   BROADCAST SUPPORT  **********************************
// Broadcast address definitions

enum  bcast_addr_e {
  NWK_BROADCAST_SHORTADDR_RESRVD_F8  = 0xFFF8,
  NWK_BROADCAST_SHORTADDR_RESRVD_F9,
  NWK_BROADCAST_SHORTADDR_RESRVD_FA,
  NWK_BROADCAST_SHORTADDR_RESRVD_FB,
  NWK_BROADCAST_SHORTADDR_DEVZCZR,            // 0xFFFC: Routers and Coordinators
  NWK_BROADCAST_SHORTADDR_DEVRXON,            // 0xFFFD: Everyone with RxOnWhenIdle == TRUE
                                              // 0xFFFE: Reserved (legacy: used for 'invalid address')
  NWK_BROADCAST_SHORTADDR_DEVALL     = 0xFFFF
};
typedef enum bcast_addr_e bcast_addr_t;
#define NWK_BROADCAST_SHORTADDR     NWK_BROADCAST_SHORTADDR_DEVALL

// Broadcast filter support
#define NWK_BROADCAST_FILTER_DEVALL   ((uint8)0x01)
#define NWK_BROADCAST_FILTER_DEVRXON  ((uint8)0x02)
#define NWK_BROADCAST_FILTER_DEVZCZR  ((uint8)0x04)
#define NWK_BROADCAST_FILTER_RESRVD   ((uint8)0x08)
#define NWK_BROADCAST_FILTER_ANY      ( \
                                       NWK_BROADCAST_FILTER_DEVALL  | \
                                       NWK_BROADCAST_FILTER_DEVRXON | \
                                       NWK_BROADCAST_FILTER_DEVZCZR | \
                                       NWK_BROADCAST_FILTER_RESRVD    \
                                      )
enum addr_filter_e  {
  ADDR_NOT_BCAST,     // not a broadcast address
  ADDR_BCAST_NOT_ME,  // broadcast address but not for me based on capabilities
  ADDR_BCAST_FOR_ME   // broadcast for me based on capabilities
};
typedef enum addr_filter_e addr_filter_t;

// Join indication type - MAC associate or rejoin
#define NWK_ASSOC_JOIN                  0
#define NWK_ASSOC_REJOIN_UNSECURE       1
#define NWK_ASSOC_REJOIN_SECURE         2

#define NWK_FRAME_TYPE_MASK         0x03
#define NWK_FRAMETYPE_FORCE_SEQ     0x80

// ***********************   END BROADCAST SUPPORT  **********************************

// NV Enables - for use when saving NV [NLME_UpdateNV()]
#define NWK_NV_NIB_ENABLE             0x01
#define NWK_NV_DEVICELIST_ENABLE      0x02
#define NWK_NV_BINDING_ENABLE         0x04
#define NWK_NV_ADDRMGR_ENABLE         0x08

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  nwkSequenceNum      = 0x81,
  nwkPassiveAckTimeout,
  nwkMaxBroadcastRetries,
  nwkMaxChildren,
  nwkMaxDepth,
  nwkMaxRouters,
  nwkNeighborTable,
  nwkBroadcastDeliveryTime,
  nwkReportConstantCost,
  nwkRouteDiscRetries,    // 0x8a
  nwkRoutingTable,
  nwkSecureAllFrames,
  nwkSecurityLevel,
  nwkSymLink,
  nwkCapabilityInfo,      // 0x8f

  // next 5 attributes are only needed for alternate addressing...
  //nwkUseTreeAddrAlloc,             // boolean
  //nwkUseTreeRouting,               // boolean
  //nwkNextAddress,                  // 16 bit
  //nwkAvailableAddresses,           // 16 bit
  //nwkAddressIncrement,             // 16 bit

  nwkTransactionPersistenceTime = 0x95,   // 16 bit

  //nwkShortAddress,                      // 16 bit
  //nwkStackProfile,
  nwkProtocolVersion = 0x98,
  //nwkAllowAddressReuse,                 // Boolean
  //nwkGroupIDTable,

  // non-standard items
  nwkRouteDiscoveryTime = 0x9B,
  nwkNumNeighborTableEntries,
  nwkNumRoutingTableEntries,
  nwkNwkState,
  nwkNwkPollTimeOut = 0xD8,
  nwkMAX_NIB_ITEMS            // Must be the last entry
}ZNwkAttributes_t;

typedef struct
{
  uint16 panId;
  byte logicalChannel;
  byte routerCapacity;
  byte deviceCapacity;
  byte version;
  byte stackProfile;
  uint16 chosenRouter;
  uint8 chosenRouterLinkQuality;
  uint8 chosenRouterDepth;
  uint8 extendedPANID[Z_EXTADDR_LEN];
  byte updateId;
  void *nextDesc;
} networkDesc_t;

// Src route subframe format
typedef struct
{
  uint8   relayCnt;
  uint8   relayIdx;
  uint16* relayList;
} NLDE_SrcFrameFormat_t;

typedef struct
{
  uint8  bufLength;
  uint8  hdrLen;
  uint8  frameType;
  uint8  protocolVersion;
  uint8  discoverRoute;
  uint8  multicast;
  uint8  secure;
  uint8  dstExtAddrSet;
  uint8  srcExtAddrSet;
  uint16 dstAddr;
  uint16 srcAddr;
  uint16 macDstAddr;
  uint16 transID;     // Only used in local messaging
  uint8  radius;
  uint8  broadcastId;
  uint8* dstExtAddr;
  uint8* srcExtAddr;
  uint8  nsduLength;
  uint8  srcRouteSet;    //If this flag is set, srcfd shall present
  NLDE_SrcFrameFormat_t srcfd;          //Source route frame data
  uint8* nsdu;
  uint16 macSrcAddr;
  uint16  txOptions;
  uint8   apsRetries;
  uint8   endDevInitiator;
  uint8   nsduHandle;
} NLDE_FrameFormat_t;

typedef struct
{
  uint8     LinkQuality;      /* The link quality of the received data frame */
  uint8     correlation;      /* The raw correlation value of the received data frame */
  int8      rssi;             /* The received RF power in units dBm */
} NLDE_Signal_t;

#define NLME_SCAN_FIELDS_RES_SIZE 1

typedef struct
{
  uint8  frameType;
  uint8  hdrLen;
  uint16 dstAddr;
  uint16 srcAddr;
  uint8  srcRouteSet;   // If this flag is set, srcfd shall present
  NLDE_SrcFrameFormat_t srcfd;         // Source route frame data
  uint8* nsdu;
  uint8  nsduLen;
  uint8  nsduHandle;
  uint16 nsduHandleOptions;
  uint8  secure;
  uint8  discoverRoute;
  uint8  radius;
  uint8  seqNum;
  uint8  multicast;
  uint8  dstExtAddrSet;
  uint8  srcExtAddrSet;
  uint8* dstExtAddr;
  uint8* srcExtAddr;
  uint16 transID;     // Only used for local messaging
  uint16 txOptions;
  uint8  apsRetries;
  void*  fd;
  uint8  endDevInitiator;
} NLDE_FrameData_t;

typedef struct
{
//ZMacDataReq_t    mfd;
  NLDE_FrameData_t nfd;
} NLDE_DataReq_t;

typedef struct
{
  uint8 overhead;
  uint8 nsduLen;
  uint8 secure;
} NLDE_DataReqAlloc_t;

typedef struct
{
  uint32 channels;
  uint8  duration;
  uint8  scanType;
  uint8  scanApp;
} NLME_ScanFields_t;

typedef struct
{
  nwkDB_t*  db;
  ZStatus_t status;
  uint8     retries;
} NLDE_DataCnf_t;

typedef struct
{
  uint8* extAddr;
  uint8  removeChildren;
  uint8  rejoin;
  uint8  silent;
} NLME_LeaveReq_t;

typedef struct
{
  uint8 removeChildren;
  uint8 rejoin;
} NLME_LeaveRsp_t;

typedef struct
{
  uint16 dstAddr;
  uint8  extAddr[Z_EXTADDR_LEN];
  uint8  removeChildren;
  uint8  rejoin;
  uint8  status;
} NLME_LeaveCnf_t;

typedef struct
{
  uint16 srcAddr;
  uint8  extAddr[Z_EXTADDR_LEN];
  uint8  request;
  uint8  removeChildren;
  uint8  rejoin;
} NLME_LeaveInd_t;

typedef struct
{
  uint16 sourceAddr;
  uint16 panID;
  uint8  logicalChannel;
  uint8	 permitJoining;
  uint8	 routerCapacity;
  uint8	 deviceCapacity;
  uint8  protocolVersion;
  uint8  stackProfile ;
  uint8	 LQI ;
  uint8  depth ;
  uint8  updateID;
  uint8  extendedPanID[Z_EXTADDR_LEN];
} NLME_beaconInd_t;
/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte NLME_PermitJoining;
extern byte NLME_AssocPermission;
extern uint16 savedResponseRate;     // Backed response rate for rejoin request
extern uint16 savedQueuedPollRate;   // Backed queued poll rate

// network discovery scan fields
extern NLME_ScanFields_t* NLME_ScanFields;

/*********************************************************************
 * NWK Data Service
 *   NLDE-DATA
 */

/*
 * This function requests the transfer of data using the NWK layer
 * data service.
 *
 * @MT SPI_CMD_NLDE_DATA_REQ
 *
 */
extern ZStatus_t NLDE_DataReq( NLDE_DataReq_t* req );

/*
 * This function allocates a request buffer for use with the NWK layer
 * data service.
 *
 */
extern NLDE_DataReq_t* NLDE_DataReqAlloc( NLDE_DataReqAlloc_t* dra );

/*
 * This function reports the results of a request to transfer a data
 * PDU (NSDU) from a local APS sub-layer entity to a single peer APS
 * sub-layer entity.
 *
 * @MT SPI_CB_NLDE_DATA_CNF
 *
 */
extern void NLDE_DataCnf( NLDE_DataCnf_t* cnf );

/*
 * This function indicates the transfer of a data PDU (NSDU) from the
 * NWK layer to the local APS sub-layer entity.
 *
 * @MT SPI_CB_NLDE_DATA_IND
 */
extern void NLDE_DataIndication( NLDE_FrameFormat_t *ff,  NLDE_Signal_t *sig, uint32 timestamp );

/*********************************************************************
 * NWK Management Service
 *   NLME-NETWORK-FORMATION
 *   NLME-NETWORK-DISCOVERY
 *   NLME-PERMIT-JOINING
 *   NLME-JOIN
 *   NLME-DIRECT-JOIN
 *   NLME-ORPHAN-JOIN
 *   NLME-START-ROUTER
 *   NLME-SYNC
 *   NLME-LEAVE
 *   NLME-RESET
 *   NLME-GET
 *   NLME-SET
 */

/*
 * This function allows the next higher layer to request that the device
 * form a new network and become the ZigBee Coordinator for that network.
 *
 * @MT SPI_CMD_NLME_INIT_COORD_REQ
 * (uint16 PanId,
 *  uint32 ScanChannels,
 *  byte BeaconOrder,
 *  byte ScanDuration,
 *  byte SuperFrameOrder,
 *  byte BatteryLifeExtension,
 *  bool DistributedNetwork,
 *  int16 DistributedNetworkAddress)
 *
 */
extern ZStatus_t NLME_NetworkFormationRequest( uint16 PanId,  uint8* ExtendedPANID, uint32 ScanChannels,
                                               byte ScanDuration, byte BeaconOrder,
                                               byte SuperframeOrder, byte BatteryLifeExtension, bool DistributedNetwork, 
                                               uint16 DistributedNetworkAddress );

/*
 * This function reports the results of the request to form a new
 * network.
 *
 * @MT SPI_CB_NLME_INITCOORD_CNF
 *
 */
extern void NLME_NetworkFormationConfirm( ZStatus_t Status );

/* This function requests the NWK layer to discover neighboring routers
 *
 * @MT SPI_CMD_NLME_NWK_DISC_REQ
 *
 */
extern ZStatus_t NLME_NetworkDiscoveryRequest( uint32 ScanChannels,
                                               uint8  scanDuration);

/* This function allows the NWK layer to discover neighboring routers
 * without affecting the current nwk state
 */
extern ZStatus_t NLME_NwkDiscReq2( NLME_ScanFields_t* fields );

/* This function cleans up the NWK layer after a call to
 * NLME_NwkDiscReq2
 */
extern void NLME_NwkDiscTerm( void );

/* This function returns network discovery confirmation
 *
 * @MT SPI_CB_NLME_NWK_DISC_CNF
 *
 */
extern void NLME_NetworkDiscoveryConfirm(uint8 status);

extern uint8 NLME_GetRemainingPermitJoiningDuration( void );

/*
 * This function defines how the next higher layer of a coordinator device
 * to permit devices to join its network for a fixed period.
 *
 * @MT SPI_CMD_NLME_PERMIT_JOINING_REQ
 *
 */
extern ZStatus_t NLME_PermitJoiningRequest( byte PermitDuration );

/*
 * This function handles the NWK_PERMITJOIN_EVT event.
 *
 */
extern void NLME_PermitJoiningEvent( void );

/*
 * This function allows the next higher layer to request the device to join
 * itself to a specific network.
 *
 * @MT SPI_CMD_NLME_JOIN_REQ
 *
 */
extern ZStatus_t NLME_JoinRequest( uint8 *extendedPANID, uint16 PanId,
                             uint8 channel, uint8 CapabilityFlags,
                             uint16 chosenParent, uint8 parentDepth );
/*
 * This function allows the next higher layer to request to directly join
 * another device to this device
 * The result is contained in the return value and there is no confirm primitive
 *
 * @MT SPI_CMD_NLME_DIRECT_JOIN_REQ
 *
 */
extern ZStatus_t NLME_DirectJoinRequest( byte *DevExtAddress, byte capInfo );

/*
 * This function allows the next higher layer to request to directly join
 * another device, specified by the short address, to this device
 * The result is contained in the return value and there is no confirm primitive
 */
extern ZStatus_t NLME_DirectJoinRequestWithAddr( byte *DevExtAddress, uint16 shortAddress, uint8 capInfo );

/*
 * This function allows the next higher layer to request the device
 * to search for its parent.
 *
 * @MT SPI_CMD_NLME_ORPHAN_JOIN_REQ
 *
 */
extern ZStatus_t NLME_OrphanJoinRequest( uint32 ScanChannels, byte ScanDuration );



/*
 * This function allows the next higher layer to set the nwk state to parent lost.
 */
extern void NLME_OrphanStateSet(void);



/*
 * This function allows the next higher layer to request the device
 * to rejoin the network.
 */
extern ZStatus_t NLME_ReJoinRequest( uint8 *ExtendedPANID, uint8 channel );

/*
 * This function allows the next higher layer to request the device
 * to rejoin the network "non-securely".
 */
extern ZStatus_t NLME_ReJoinRequestUnsecure( uint8 *ExtendedPANID, uint8 channel );

/*
 * This function allows the next higher layer to be notified of the
 * results of its request to join itself to a network.
 *
 * @MT SPI_CB_NLME_JOIN_CNF
 * (byte *DeviceAddress,
 *  uint16 PanId,
 *  byte Status)
 *
 */
extern void NLME_JoinConfirm( uint16 PanId, ZStatus_t Status );

/*
 * This function allows the next higher layer of a coordinator to be
 * notified of a remote join request.
 *
 * @MT SPI_CB_NLME_JOIN_IND
 *
 */
extern ZStatus_t NLME_JoinIndication( uint16 ShortAddress,
                                      uint8 *ExtendedAddress,
                                      uint8 CapabilityFlags,
                                      uint8 type );

/*
 * This function allows the next higher layer to request a device to function
 * as a router. NOTE: the BeaconOrder and SuperframeOrder parameters are not
 *  used in this version -- the values obtained during network formation or
 * joining are used instead, ensuring commonality with the ZDO COORDINATOR.
 *
 * @MT SPI_CMD_NLME_START_ROUTER_REQ
 *
 */
extern ZStatus_t NLME_StartRouterRequest( byte BeaconOrder,
                                          byte SuperframeOrder,
                                          byte BatteryLifeExtension  );

/*
 * This function reports the results of the request  to start
 * functioning as a router.
 *
 * @MT SPI_CB_NLME_START_ROUTER_CNF
 *
 */
extern void NLME_StartRouterConfirm( ZStatus_t Status );

/*
 * This function reports the beacon notification received due
 * to network discovery
 *
 */
extern void NLME_beaconNotifyInd(NLME_beaconInd_t *pBeacon);

/*
 * This function allows the next higher layer to request that itself
 * or another device leave the network.
 *
 * @MT SPI_CMD_NLME_LEAVE_REQ
 *
 */
extern ZStatus_t NLME_LeaveReq( NLME_LeaveReq_t* req );

/*
 * This function allows the next higher layer to be notified of the
 * results of its request for itself or another device to leave the
 * network.
 *
 * @MT SPI_CB_NLME_LEAVE_CNF
 *
 */
extern void NLME_LeaveCnf( NLME_LeaveCnf_t* cnf );

/*
 * This function allows the next higher layer of a device to be
 * notified of a remote leave request.
 *
 * @MT SPI_CB_NLME_LEAVE_IND
 *
 */
extern void NLME_LeaveInd( NLME_LeaveInd_t* ind );

/*
 * This function allows the next higher layer to respond to a leave
 * indication.
 */
extern ZStatus_t NLME_LeaveRsp( NLME_LeaveRsp_t* rsp );

/*
 * This function allows the next higher layer to request that the NWK layer
 * perform a reset operation.
 *
 * @MT SPI_CMD_NLME_RESET_REQ
 *
 */
extern ZStatus_t NLME_ResetRequest( void );

/*
 * This function allows the next higher layer to request
 * synchronization with its parent and extract data
 *
 * @MT SPI_CMD_NLME_SYNC_REQ
 */

extern ZStatus_t NLME_SyncRequest( byte Track );

/*
 * This function allows the next higher layer to be notified of the
 * loss of synchronization at the MAC sub-layer.
 *
 * @MT SPI_CB_NLME_SYNC_IND
 * (byte Status)
 *
 */
extern void NLME_SyncIndication( byte type, uint16 shortAddr );

/*
 * This function stub allows the next higher layer to be notified of
 * a permit joining timeout.
 */
extern void NLME_PermitJoiningTimeout( void );

/*
 * This function allows the next higher layer to be notified of a
 * Poll Confirm from the MAC sub-layer.
 *
 * @MT SPI_CB_NLME_POLL_CNF
 * (byte Status)
 *
 */
extern void NLME_PollConfirm( byte status );

/*
 * This function allows the next higher layer to read the value of
 * an attribute from the NIB.
 *
 * @MT SPI_CMD_NLME_GET_REQ
 *
 */
extern ZStatus_t NLME_GetRequest( ZNwkAttributes_t NIBAttribute, uint16 Index,
                                    void *Value );

/*
 * This function allows the next higher layer to write the value of an
 * attribute into the NIB.
 *
 * @MT SPI_CMD_NLME_SET_REQ
 *
 */
extern ZStatus_t NLME_SetRequest( ZNwkAttributes_t NIBAttribute,
                                  uint16 Index,
                                  void *Value );
/*
 * This function allows the higher layers to initiate route discovery
 * to a given destination address
 *
 * @MT SPI_CMD_NLME_ROUTE_DISC_REQ
 *
 */
extern ZStatus_t NLME_RouteDiscoveryRequest( uint16 DstAddress, byte options, uint8 radius );


/*
 * This function allow to indicate to higher layer the existence of
 * concentrator and its nwk address
 */
extern void NLME_ConcentratorIndication( uint16 nwkAddr, uint8 *extAddr, uint8 pktCost );

/*
 * This function allows the next higher layer to request an energy scan
 * to evaluate channels in the local area.
 */
extern ZStatus_t NLME_EDScanRequest( uint32 ScanChannels, uint8 scanDuration);

/*
 * This function returns list of energy measurements.
 */
extern void NLME_EDScanConfirm( uint8 status, uint32 scannedChannels, uint8 *energyDetectList );

/*********************************************************************
 * NWK Helper Functions
 */

/*
 * This function will return a pointer to the device's IEEE 64 bit address
 *
 * This function resides in nwk_util.c.
 */
extern byte *NLME_GetExtAddr( void );

/*
 * This function will return this device's 16 bit short address
 *
 * This function resides in nwk_util.c.
 */
extern uint16 NLME_GetShortAddr( void );

/*
 * This function will return the MAC's Coordinator's short (16bit) address
 * which is this device's parent, not necessarily the Zigbee coordinator.
 *
 * This function resides in nwk_util.c.
 */
extern uint16 NLME_GetCoordShortAddr( void );

/*
 * This function will return the MAC's Coordinator's Extented (64bit) address
 * which is this device's parent, not necessarily the Zigbee coordinator.
 *
 * This function resides in nwk_util.c.
 */
extern void NLME_GetCoordExtAddr( byte * );

/*
 * Use this function to request a single MAC data request.
 */
extern ZMacStatus_t NwkPollReq( byte securityEnable );

/*
 * Use this function to set/change the Network Poll Rate. If the
 * newRate is set to 0, it will turn off the auto polling, 1 will do a
 * one time poll.
 */
extern void NLME_SetPollRate( uint32 newRate );

/*
 * Use this function to set/change the Network Queued Poll Rate.
 * This is used after receiving a data indication to poll immediately
 * for queued messages.
 */
extern void NLME_SetQueuedPollRate( uint16 newRate );

/*
 * Use this function to set/change the Network Queued Poll Rate.
 * This is used after receiving a data confirmation to poll immediately
 * for response messages.
 */
extern void NLME_SetResponseRate( uint16 newRate );

/*
 * Initialize the Nwk, Assoc device list, and binding NV Items
 *   returns SUCCESS if successful otherwise an error bitmask.
 */
extern byte NLME_InitNV( void );

/*
 * Set defaults for the Nwk, Assoc device list, and binding NV Items
 */
extern void NLME_SetDefaultNV( void );

/*
 * Restore network memory items from NV.
 */
extern byte NLME_RestoreFromNV( void );

/*
 * Write network items to NV.
 *        enables - bit mask of items to save:
 *                     NWK_NV_NIB_ENABLE
 *                     NWK_NV_DEVICELIST_ENABLE
 *                     NWK_NV_BINDING_ENABLE
 *                     NWK_NV_ADDRMGR_ENABLE
 */
void NLME_UpdateNV( byte enables );

/*
 * NLME_CheckNewAddrSet
 *
 * We have a new address pair (short and extended) - check our database.
 *     dontChange - Don't change our address just issue conflict (It was taken
 *                  out since the Spec was changed again. All devices will
 *                  change address upon any circumstances.
 *
 * Returns      ZSuccess if in data base and matches
 *              ZUnsupportedMode if not in database
 *              ZFailure if short address is in database,
 *                   but extended address doesn't match database
 *
 * If ZFailure is returned, the stack already sent out an address conflict
 * route error - already called NLME_ReportAddressConflict().
 */
extern ZStatus_t NLME_CheckNewAddrSet( uint16 shortAddr, uint8 *extAddr );

/*
 * Issues a Router Error with Address conflict and handles the
 * conflict locally for itself and children (RFDs).
 */
extern void NLME_ReportAddressConflict( uint16 shortAddr, uint8 forceSpecialMode );


extern void NLME_CoordinatorInit( void );
extern void NLME_DeviceJoiningInit( void );

extern void (*pnwk_ScanPANChanSelect)( ZMacScanCnf_t *param );
extern void (*pnwk_ScanPANChanVerify)( ZMacScanCnf_t *param );
extern void (*pNLME_NetworkFormationConfirm)( ZStatus_t Status );

extern ZStatus_t NLME_ReadNwkKeyInfo(uint16 index, uint16 len, void *keyinfo, uint16 NvId);

/****************************************************************************
****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* NLMEDE_H */


