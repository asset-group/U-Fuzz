/**************************************************************************************************
  Filename:       nwk.h
  Revised:        $Date: 2014-12-01 14:58:34 -0800 (Mon, 01 Dec 2014) $
  Revision:       $Revision: 41287 $

  Description:    Network layer logic component interface.


  Copyright 2004-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef NWK_H
#define NWK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "ZMAC.h"
#include "nwk_bufs.h"
#include "NLMEDE.h"
#include "ssp.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

//NWK event identifiers
#define MAC_SCAN_REQ          0x01
#define NWK_NETWORKSTART_REQ  0x02
#define MAC_ASSOCIATE_REQ     0x03
#define NWK_REMOTE_GET_REQ    0x04
#define NWK_REMOTE_SET_REQ    0x05
#define NWK_ASSOCIATE_RESP    0x06
#define NWK_DISASSOCIATE_REQ  0x07

#define NWK_AUTO_POLL_EVT         0x0001
#define NWK_NOT_EXPECTING_EVT     0x0004
#define RTG_TIMER_EVENT           0x0010
#define NWK_DATABUF_SEND          0x0020
#define NWK_BCAST_TIMER_EVT       0x0040
#define NWK_PERMITJOIN_EVT        0x0080
#define NWK_LINK_STATUS_EVT       0x0100
#define NWK_PID_UPDATE_EVT        0x0200
#define NWK_REJOIN_TIMEOUT_EVT    0x0400
#define NWK_MTO_RTG_REQ_EVT       0x0800
#define NWK_MTO_RTG_REQ_DELAY_EVT 0x1000
#define NWK_BROADCAST_MSG_EVT     0x2000
#define NWK_CHILD_AGE_TIMER_EVT   0x4000
// Event 0x8000 is Reserved for SYS_EVENT_MSG

//NWK PACKET: FIELD IDENTIFIERS
#define NWK_CMD_ID                  0
#define NWK_PARAMS_ID               1
#define NWK_REQ_ATTR_ID             1
#define NWK_REQ_ATTR                2
#define NWK_CMD_PYLD_BEGIN          NWK_HEADER_LEN
#define NWK_DEVICE_LIST_LEN_FIELD   NWK_HEADER_LEN + 1

// This value needs to be set or read from somewhere
#define ED_SCAN_MAXCHANNELS 27

// Max length of packet that can be sent to the MAC
#define MAX_DATA_PACKET_LEN MAC_MAX_FRAME_SIZE

#define NWK_TASK_ID              0
#define ASSOC_CAPABILITY_INFO    0
#define ASSOC_SECURITY_EN        0

#define DEF_DEST_EP              2
#define DEVICE_APPLICATION       0

#define MAC_ADDR_LEN             8

#define NWK_TXOPTIONS_ACK        0x01
#define NWK_TXOPTIONS_INDIRECT   0x04

// TxOptions for a data request - Indirect data and ACK required
#define NWK_TXOPTIONS_COORD      (NWK_TXOPTIONS_ACK | NWK_TXOPTIONS_INDIRECT)

// TxOptions for a data request - Direct data and ACK required
//#define NWK_TXOPTIONS_COORD       (NWK_TXOPTIONS_ACK)

//Assume value until defined By SuperApp or design spec
#define DEF_MAX_NUM_COORDINATORS 15        //Default value
#define DEF_CHANNEL_SCAN_BITMAP  MAX_CHANNELS_24GHZ
#define SOFT_SCAN_DURATION       1         //in secs

#define DEF_SCAN_DURATION        2

#define NO_BEACONS              15

#define DEF_BEACON_ORDER         NO_BEACONS
//#define DEF_BEACON_ORDER         10   // 15 seconds
//#define DEF_BEACON_ORDER         9    // 7.5 seconds
//#define DEF_BEACON_ORDER         8    // 3.75 seconds
//#define DEF_BEACON_ORDER         6    // 1 second
//#define DEF_BEACON_ORDER         1    // 30 millisecond

//#define DEF_SUPERFRAMEORDER      2
#define DEF_SUPERFRAMEORDER      DEF_BEACON_ORDER
#define NWK_SECURITY_ENABLE      FALSE
#define NWK_MAC_ASSOC_CNF_LEN    4
#define FIXED_SIZ_MAC_DATA_CNF   4         //Length of all fixed params except data
#define FIXED_SIZ_MAC_DATA_IND   26
#define FIXED_SIZ_MAC_SCAN_CNF   7

#define ALL_PAIRING_TABLE_ENTRIES   0
#define SIZE_OF_PAIRING_TABLE_ENTRY 6 //Two short addr and two endpts
#define SIZE_OF_DEVICE_LIST_ENTRY   2 //short addr in dev list is 2 bytes

#define NWK_SEND_TIMER_INTERVAL         2
#define NWK_BCAST_TIMER_INTERVAL        100 // NWK_BCAST_TIMER_EVT duration
#define NWK_CHILD_AGE_TIMER_INTERVAL    1000  // One Second 1(s) * 1000(ms)

#define INVALID_NODE_ADDR                           0xFFFE

// Link cost constants
#define DEF_LINK_COST              1   // Default link cost
#define MAX_LINK_COST              7   // max link cost
#define LINK_DOWN_COST             0   // link is down if txCost is equal to LINK_DOWN_COST
#define LINK_AGEOUT_COST           0   // Set link cost to zero if the neighbor age out

#define DEF_LQI                    170   // Default lqi
#define LOWEST_LQI_STILL_ACTIVE    1
#define LINK_AGEOUT_LQI            0   // Set lqi to zero if the neighbor age out

// Link counter constants
#define DEF_LINK_COUNTER           ((gLINK_DOWN_TRIGGER+1) / 2)   // Starting tx counter
#define LINK_ACTIVE_TRIGGER        2   // link is up if txCounter goes below this

//NWK Callback subscription IDs
#define CB_ID_APP_ANNOUNCE_CNF          0x00
#define CB_ID_APP_ASSOCIATE_CNF         0x01
#define CB_ID_APP_ASSOCIATE_IND         0x02
#define CB_ID_APP_DATA_CNF              0x03
#define CB_ID_APP_DATA_IND              0x04
#define CB_ID_APP_DISASSOCIATE_CNF      0x05
#define CB_ID_APP_DISASSOCIATE_IND      0x06
#define CB_ID_APP_NETWORK_DETECT_CNF    0x07
#define CB_ID_APP_REMOTE_GET_CNF        0x08
#define SPI_CB_APP_REMOTE_SET_CNF       0x09
#define CB_ID_APP_SERVICE_CNF           0x0a
#define CB_ID_APP_SERVICE_IND           0x0b
#define CB_ID_APP_START_CNF             0x0c

#define NUM_PING_ROUTE_ADDRS            12
#define PING_ROUTE_ADDRS_INDEX          8

#define NWK_GetNodeDepth()              (_NIB.nodeDepth)
#define NWK_GetTreeDepth()              (0)

#define BEACON_MAX_DEPTH                0x0F

// The value of this event should larger than the maximum value of the MAC events
#define NWK_CHILD_TABLE_MGMT            100

// Status of child device
#define NWK_CHILD_NOT_IN_TABLE          1

// Router parent capabilities information bitmask
// Bits   Value    Description
//   0    0x01     MAC Data Poll Keepalive Supported
#define NWK_PARENT_INFO_UNDEFINED             0x00
#define NWK_PARENT_INFO_MAC_DATA_POLL         0x01
   
#define PARENT_INFO_MAC_DATA_POLL_BIT         0x00
   
/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  NWK_INIT,
  NWK_JOINING_ORPHAN,
  NWK_DISC,
  NWK_JOINING,
  NWK_ENDDEVICE,
  PAN_CHNL_SELECTION,
  PAN_CHNL_VERIFY,
  PAN_STARTING,
  NWK_ROUTER,
  NWK_REJOINING
} nwk_states_t;

// MAC Command Buffer types
typedef enum
{
  MACCMDBUF_NONE,
  MACCMDBUF_ASSOC_REQ,
  MACCMDBUF_DISASSOC_REQ
} nwkMacCmds_t;

typedef struct
{
  byte  SequenceNum;
  byte  PassiveAckTimeout;
  byte  MaxBroadcastRetries;
  byte  MaxChildren;
  byte  MaxDepth;
  byte  MaxRouters;

  byte  dummyNeighborTable;     // to make everything a byte!!

  byte  BroadcastDeliveryTime;
  byte  ReportConstantCost;
  byte  RouteDiscRetries;

  byte  dummyRoutingTable;      // to make everything a byte!!

  byte  SecureAllFrames;
  byte  SecurityLevel;
#if defined ( COMPATIBILITY_221 )   // Obsolete - do not use
  byte  nwkAllFresh;
#endif
  byte  SymLink;
  byte  CapabilityFlags;

  uint16 TransactionPersistenceTime;

  byte   nwkProtocolVersion;

  // non-standard attributes
  byte  RouteDiscoveryTime;
  byte  RouteExpiryTime;        // set to 0 to turn off expiration of routes

  // non-settable
  uint16  nwkDevAddress;
  byte    nwkLogicalChannel;
  uint16  nwkCoordAddress;
  byte    nwkCoordExtAddress[Z_EXTADDR_LEN];
  uint16  nwkPanId;

  // Other global items - non-settable
  nwk_states_t  nwkState;
  uint32        channelList;
  byte          beaconOrder;
  byte          superFrameOrder;
  byte          scanDuration;
  byte          battLifeExt;
  uint32        allocatedRouterAddresses;
  uint32        allocatedEndDeviceAddresses;
  byte          nodeDepth;

  // Version 1.1 - extended PAN ID
  uint8         extendedPANID[Z_EXTADDR_LEN];

  // Network key flag
  uint8      nwkKeyLoaded;
  // Key information - Moved out of the NIB structure after ZStack 2.3.0
  // If these elements are going to be reused make sure to consider the size
  // of the structures and padding specific to the target where the stack is
  // going to be running.
  nwkKeyDesc spare1;    // Not used
  nwkKeyDesc spare2;    // Not used

  // Zigbee Pro extensions
  uint8      spare3;                // nwkAddrAlloc deprecated - not used anymore
  uint8      spare4;                // nwkUniqueAddr deprecated - not used anymore
  uint8      nwkLinkStatusPeriod;   // The time in seconds betwee link status
                                    // command frames
  uint8      nwkRouterAgeLimit;     // The number of missed link status
                                    // command frames before resetting the
                                    // link cost to zero
  uint8      nwkUseMultiCast;
  // ZigBee Pro extentions: MTO routing
  uint8      nwkIsConcentrator;             // If set, then the device is concentrator
  uint8      nwkConcentratorDiscoveryTime;  // Time period between two consecutive MTO route discovery
  uint8      nwkConcentratorRadius;         // Broadcast radius of the MTO route discovery

#if defined ( COMPATIBILITY_221 )   // Obsolete - do not use
  uint8      nwkMaxSourceRoute;
  uint8      nwkSrcRtgExpiryTime;
#else
  uint8      nwkAllFresh;
#endif

  uint16     nwkManagerAddr;        // Network Manager Address
  uint16     nwkTotalTransmissions;
  uint8      nwkUpdateId;
} nwkIB_t;

// Scanned PAN IDs to be used for Network Report command
typedef struct
{
  uint16 panId;
  void   *next;
} nwkPanId_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern nwkIB_t _NIB;
extern byte NWK_TaskID;
extern networkDesc_t *NwkDescList;
extern byte nwkExpectingMsgs;
extern byte nwk_beaconPayload[ZMAC_MAX_BEACON_PAYLOAD_LEN];
extern byte nwk_beaconPayloadSize;

extern uint8 nwkSendMTOReq;

/*********************************************************************
 * FUNCTIONS
 */

 /*
 * NWK Task Initialization
 */
extern void nwk_init( byte task_id );

 /*
 * Calls mac_data_req then handles the buffering
 */
extern ZStatus_t nwk_data_req_send( nwkDB_t* db );

 /*
 * NWK Event Loop
 */
extern UINT16 nwk_event_loop( byte task_id, UINT16 events );

 /*
 * Process incoming command packet
 */
//extern void CommandPktProcessing( NLDE_FrameFormat_t *ff );

/*
* Start a coordinator
*/
extern ZStatus_t nwk_start_coord( void );

/*
 * Free any network discovery data
 */
extern void nwk_desc_list_free( void );

/*
 * This function sets to null the discovery nwk list
 */
extern void nwk_desc_list_release(void);

extern networkDesc_t *nwk_getNetworkDesc( uint8 *ExtendedPANID, uint16 PanId, byte Channel );
extern networkDesc_t *nwk_getNwkDescList( void );

extern void nwk_BeaconFromNative(byte* buff, byte size, beaconPayload_t* beacon);
extern void nwk_BeaconToNative(beaconPayload_t* beacon, byte* buff, byte size);

/*
 * Set NWK task's state
 */
extern void nwk_setStateIdle( uint8 idle );

/*
 * Returns TRUE if NWK state is idle, FALSE otherwise.
 */
extern uint8 nwk_stateIdle( void );

/*********************************************************************
 * Functionality - not to be called directly.
 */
extern void nwk_ScanPANChanSelect( ZMacScanCnf_t *param );
extern void nwk_ScanPANChanVerify( ZMacScanCnf_t *param );

/*
 *  Special Send Leave Posts the message directly to MAC without buffering it
 */
extern ZStatus_t nwk_send_direct_leave_req( nwkDB_t* db );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* NWK_H */
