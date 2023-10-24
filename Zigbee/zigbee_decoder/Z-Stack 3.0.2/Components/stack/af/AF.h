/**************************************************************************************************
  Filename:       AF.h
  Revised:        $Date: 2014-11-04 10:53:36 -0800 (Tue, 04 Nov 2014) $
  Revision:       $Revision: 40974 $

  Description:    This file contains the Application Framework definitions.


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
#ifndef AF_H
#define AF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "nwk.h"
#include "APSMEDE.h"

/*********************************************************************
 * CONSTANTS
 */

#define AF_BROADCAST_ENDPOINT              0xFF

#define AF_WILDCARD_PROFILEID              0x02   // Will force the message to use Wildcard ProfileID
#define AF_PREPROCESS                      0x04   // Will force APS to callback to preprocess before calling NWK layer
#define AF_LIMIT_CONCENTRATOR              0x08
#define AF_ACK_REQUEST                     0x10
#define AF_SUPRESS_ROUTE_DISC_NETWORK      0x20   // Supress Route Discovery for intermediate routes
                                                  // (route discovery preformed for initiating device)
#define AF_EN_SECURITY                     0x40
#define AF_SKIP_ROUTING                    0x80

#define AF_DISCV_ROUTE                     0x00   // This option is no longer available, and is included for backwards compatibility

// Backwards support for afAddOrSendMessage / afFillAndSendMessage.
#define AF_TX_OPTIONS_NONE                 0
#define AF_MSG_ACK_REQUEST                 AF_ACK_REQUEST

// Default Radius Count value
#define AF_DEFAULT_RADIUS                  DEF_NWK_RADIUS

/*********************************************************************
 * Node Descriptor
 */
#define AF_MAX_USER_DESCRIPTOR_LEN         16
#define AF_USER_DESCRIPTOR_FILL          0x20
typedef struct
{
  uint8 len;     // Length of string descriptor
  uint8 desc[AF_MAX_USER_DESCRIPTOR_LEN];
} UserDescriptorFormat_t;

// Node Logical Types
#define NODETYPE_COORDINATOR    0x00
#define NODETYPE_ROUTER         0x01
#define NODETYPE_DEVICE         0x02

// Node Frequency Band - bit map
#define NODEFREQ_800            0x01    // 868 - 868.6 MHz
#define NODEFREQ_900            0x04    // 902 - 928 MHz
#define NODEFREQ_2400           0x08    // 2400 - 2483.5 MHz

// Node MAC Capabilities - bit map
//   Use CAPINFO_ALTPANCOORD, CAPINFO_DEVICETYPE_FFD,
//       CAPINFO_DEVICETYPE_RFD, CAPINFO_POWER_AC,
//       and CAPINFO_RCVR_ON_IDLE from NLMEDE.h

// Node Descriptor format structure
typedef struct
{
  uint8 LogicalType:3;
  uint8 ComplexDescAvail:1;  /* AF_V1_SUPPORT - reserved bit. */
  uint8 UserDescAvail:1;     /* AF_V1_SUPPORT - reserved bit. */
  uint8 Reserved:3;
  uint8 APSFlags:3;
  uint8 FrequencyBand:5;
  uint8 CapabilityFlags;
  uint8 ManufacturerCode[2];
  uint8 MaxBufferSize;
  uint8 MaxInTransferSize[2];
  uint16 ServerMask;
  uint8 MaxOutTransferSize[2];
  uint8 DescriptorCapability;
} NodeDescriptorFormat_t;

// Bit masks for the ServerMask.
#define PRIM_TRUST_CENTER        0x01
#define BKUP_TRUST_CENTER        0x02
#define PRIM_BIND_TABLE          0x04
#define BKUP_BIND_TABLE          0x08
#define PRIM_DISC_TABLE          0x10
#define BKUP_DISC_TABLE          0x20
#define NETWORK_MANAGER          0x40


/*********************************************************************
 * Node Power Descriptor
 */

// Node Current Power Modes (CURPWR)
// Receiver permanently on or sync with coordinator beacon.
#define NODECURPWR_RCVR_ALWAYS_ON   0x00
// Receiver automatically comes on periodically as defined by the
// Node Power Descriptor.
#define NODECURPWR_RCVR_AUTO        0x01
// Receiver comes on when simulated, eg by a user pressing a button.
#define NODECURPWR_RCVR_STIM        0x02

// Node Available Power Sources (AVAILPWR) - bit map
//   Can be used for AvailablePowerSources or CurrentPowerSource
#define NODEAVAILPWR_MAINS          0x01  // Constant (Mains) power
#define NODEAVAILPWR_RECHARGE       0x02  // Rechargeable Battery
#define NODEAVAILPWR_DISPOSE        0x04  // Disposable Battery

// Power Level
#define NODEPOWER_LEVEL_CRITICAL    0x00  // Critical
#define NODEPOWER_LEVEL_33          0x04  // 33%
#define NODEPOWER_LEVEL_66          0x08  // 66%
#define NODEPOWER_LEVEL_100         0x0C  // 100%

// Node Power Descriptor format structure
typedef struct
{
  unsigned int PowerMode:4;
  unsigned int AvailablePowerSources:4;
  unsigned int CurrentPowerSource:4;
  unsigned int CurrentPowerSourceLevel:4;
} NodePowerDescriptorFormat_t;

/*********************************************************************
 * Simple Descriptor
 */

// AppDevVer values
#define APPDEVVER_1               0x01

// AF_V1_SUPPORT AppFlags - bit map
#define APPFLAG_NONE                0x00  // Backwards compatibility to AF_V1.

// AF-AppFlags - bit map
#define AF_APPFLAG_NONE             0x00
#define AF_APPFLAG_COMPLEXDESC      0x01  // Complex Descriptor Available
#define AF_APPFLAG_USERDESC         0x02  // User Descriptor Available

typedef uint16  cId_t;
// Simple Description Format Structure
typedef struct
{
  uint8          EndPoint;
  uint16         AppProfId;
  uint16         AppDeviceId;
  uint8          AppDevVer:4;
  uint8          Reserved:4;             // AF_V1_SUPPORT uses for AppFlags:4.
  uint8          AppNumInClusters;
  cId_t         *pAppInClusterList;
  uint8          AppNumOutClusters;
  cId_t         *pAppOutClusterList;
} SimpleDescriptionFormat_t;

/*********************************************************************
 * AF Message Format
 */

// Frame Types
#define FRAMETYPE_KVP          0x01     // 0001
#define FRAMETYPE_MSG          0x02     // 0010

#define ERRORCODE_SUCCESS               0x00

#define AF_HDR_KVP_MAX_LEN   0x08  // Max possible AF KVP header.
#define AF_HDR_V1_0_MAX_LEN  0x03  // Max possible AF Ver 1.0 header.
#define AF_HDR_V1_1_MAX_LEN  0x00  // Max possible AF Ver 1.1 header.

// Generalized MSG Command Format
typedef struct
{
  uint8   TransSeqNumber;
  uint16  DataLength;              // Number of bytes in TransData
  uint8  *Data;
} afMSGCommandFormat_t;

typedef enum
{
  noLatencyReqs,
  fastBeacons,
  slowBeacons
} afNetworkLatencyReq_t;

/*********************************************************************
 * Endpoint  Descriptions
 */

typedef enum
{
  afAddrNotPresent = AddrNotPresent,
  afAddr16Bit      = Addr16Bit,
  afAddr64Bit      = Addr64Bit,
  afAddrGroup      = AddrGroup,
  afAddrBroadcast  = AddrBroadcast
} afAddrMode_t;

typedef struct
{
  union
  {
    uint16      shortAddr;
    ZLongAddr_t extAddr;
  } addr;
  afAddrMode_t addrMode;
  uint8 endPoint;
  uint16 panId;  // used for the INTER_PAN feature
}  afAddrType_t;


typedef struct
{
  osal_event_hdr_t hdr;     /* OSAL Message header */
  uint16 groupId;           /* Message's group ID - 0 if not set */
  uint16 clusterId;         /* Message's cluster ID */
  afAddrType_t srcAddr;     /* Source Address, if endpoint is STUBAPS_INTER_PAN_EP,
                               it's an InterPAN message */
  uint16 macDestAddr;       /* MAC header destination short address */
  uint8 endPoint;           /* destination endpoint */
  uint8 wasBroadcast;       /* TRUE if network destination was a broadcast address */
  uint8 LinkQuality;        /* The link quality of the received data frame */
  uint8 correlation;        /* The raw correlation value of the received data frame */
  int8  rssi;               /* The received RF power in units dBm */
  uint8 SecurityUse;        /* deprecated */
  uint32 timestamp;         /* receipt timestamp from MAC */
  uint8 nwkSeqNum;          /* network header frame sequence number */
  afMSGCommandFormat_t cmd; /* Application Data */
  uint16 macSrcAddr;        /* MAC header source short address */
  uint8 radius;
} afIncomingMSGPacket_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint8 endpoint;
  uint8 transID;
} afDataConfirm_t;

// Reflect Error Message - sent when there is an error occurs
// during a reflected message.
typedef struct
{
  osal_event_hdr_t hdr;  // hdr.status contains the error indication (ie. ZApsNoAck)
  uint8 endpoint;        // destination endpoint
  uint8 transID;         // transaction ID of sent message
  uint8 dstAddrMode;     // destination address type: 0 - short address, 1 - group address
  uint16 dstAddr;        // destination address - depends on dstAddrMode
} afReflectError_t;

// Endpoint Table - this table is the device description
// or application registration.
// There will be one entry in this table for every
// endpoint defined.
typedef struct
{
  uint8 endPoint;
  uint8 epType;
  uint8 *task_id;  // Pointer to location of the Application task ID.
  SimpleDescriptionFormat_t *simpleDesc;
  afNetworkLatencyReq_t latencyReq;
} endPointDesc_t;

// Typedef for callback function to retrieve an endpoints
//   descriptors, contained in the endpoint descriptor.
//   This will allow an application to dynamically change
//   the descriptor and not use the RAM/ROM.
typedef void *(*pDescCB)( uint8 type, uint8 endpoint );

// Typedef for callback function to control the AF transaction ID
//   used when sending messages.
//   This allows the application to verify if the transaction ID
//   is not duplicated of a pending message.
typedef void (*pApplCB)( APSDE_DataReq_t *req );

// Descriptor types used in the above callback
#define AF_DESCRIPTOR_SIMPLE            1
#define AF_DESCRIPTOR_PROFILE_ID        2

// Bit definitions for epList_t flags.
typedef enum
{
  eEP_AllowMatch = 1,
  eEP_NotUsed
} eEP_Flags;

typedef struct {
  uint8 frameDelay;
  uint8 windowSize;
} afAPSF_Config_t;

typedef struct _epList_t {
  struct _epList_t *nextDesc;
  endPointDesc_t *epDesc;
  pDescCB  pfnDescCB;     // Don't use if this function pointer is NULL.
  afAPSF_Config_t apsfCfg;
  eEP_Flags flags;
  pApplCB pfnApplCB;    // Don't use it if it has not been set to a valid function pointer by the application
} epList_t;

/*********************************************************************
 * TYPEDEFS
 */

#define afStatus_SUCCESS            ZSuccess           /* 0x00 */
#define afStatus_FAILED             ZFailure           /* 0x01 */
#define afStatus_INVALID_PARAMETER  ZInvalidParameter  /* 0x02 */
#define afStatus_MEM_FAIL           ZMemError          /* 0x10 */
#define afStatus_NO_ROUTE           ZNwkNoRoute        /* 0xCD */

typedef ZStatus_t afStatus_t;

typedef struct
{
  uint8              kvp;
  APSDE_DataReqMTU_t aps;
} afDataReqMTU_t;

/*********************************************************************
 * Globals
 */

extern epList_t *epList;

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * afInit - Initialize the AF.
  */
  //extern void afInit( void );
  #define afInit()  // No work to do for now.

 /*
  * afRegisterExtended - Register an Application's EndPoint description
  *           with a callback function for descriptors and
  *           with an Application callback function to control
  *           the AF transaction ID.
  *
  */
  extern epList_t *afRegisterExtended( endPointDesc_t *epDesc, pDescCB descFn, pApplCB applFn );

 /*
  * afRegister - Register an Application's EndPoint description.
  *
  */
  extern afStatus_t afRegister( endPointDesc_t *epDesc );

 /*
  * afDelete - Delete an Application's EndPoint descriptor and frees the memory.
  *
  */
  extern afStatus_t afDelete( uint8 EndPoint );

 /*
  * afDataConfirm - APS will call this function after a data message
  *                 has been sent.
  */
  extern void afDataConfirm( uint8 endPoint, uint8 transID, ZStatus_t status );

 /*
  * afReflectError - APS will call this function for an error with a reflected data message.
  */
  extern void afReflectError( uint8 srcEP, uint8 dstAddrMode, uint16 dstAddr, uint8 dstEP,
                              uint8 transID, ZStatus_t status );

 /*
  * afIncomingData - APS will call this function when an incoming
  *                   message is received.
  */
  extern void afIncomingData( aps_FrameFormat_t *aff, zAddrType_t *SrcAddress, uint16 SrcPanId,
                       NLDE_Signal_t *sig, uint8 nwkSeqNum, uint8 SecurityUse, uint32 timestamp, uint8 radius );

  afStatus_t AF_DataRequest( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                             uint16 cID, uint16 len, uint8 *buf, uint8 *transID,
                             uint8 options, uint8 radius );


/*********************************************************************
 * @fn      AF_DataRequestSrcRtg
 *
 * @brief   Common functionality for invoking APSDE_DataReq() for both
 *          SendMulti and MSG-Send.
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param  *srcEP - Origination (i.e. respond to or ack to) End Point Descr.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   len - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 * @param   options - Valid bit mask of Tx options.
 * @param   radius - Normally set to AF_DEFAULT_RADIUS.
 * @param   relayCnt - Number of devices in the relay list
 * @param   pRelayList - Pointer to the relay list
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */

afStatus_t AF_DataRequestSrcRtg( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                           uint16 cID, uint16 len, uint8 *buf, uint8 *transID,
                           uint8 options, uint8 radius, uint8 relayCnt,
                           uint16* pRelayList );

/*********************************************************************
 * Direct Access Functions - ZigBee Device Object
 */

 /*
  *	afFindEndPointDesc - Find the endpoint description entry from the
  *                      endpoint number.
  */
  extern endPointDesc_t *afFindEndPointDesc( uint8 endPoint );

 /*
  *	afFindSimpleDesc - Find the Simple Descriptor from the endpoint number.
  *   	  If return value is not zero, the descriptor memory must be freed.
  */
  extern uint8 afFindSimpleDesc( SimpleDescriptionFormat_t **ppDesc, uint8 EP );

 /*
  *	afDataReqMTU - Get the Data Request MTU(Max Transport Unit)
  */
  extern uint8 afDataReqMTU( afDataReqMTU_t* fields );

 /*
  *	afGetMatch - Get the action for the Match Descriptor Response
  *             TRUE allow match descriptor response
  */
  extern uint8 afGetMatch( uint8 ep );

 /*
  *	afSetMatch - Set the action for the Match Descriptor Response
  *             TRUE allow match descriptor response
  */
  extern uint8 afSetMatch( uint8 ep, uint8 action );

 /*
  *	afNumEndPoints - returns the number of endpoints defined.
  */
  extern uint8 afNumEndPoints( void );

 /*
  *	afEndPoints - builds an array of endpoints.
  */
  extern void afEndPoints( uint8 *epBuf, uint8 skipZDO );

 /*
  * afCopyAddress
  */
extern void afCopyAddress (afAddrType_t *afAddr, zAddrType_t *zAddr);

 /*
  *	afAPSF_ConfigGet - ascertain the fragmentation configuration for the specified EndPoint.
  */
void afAPSF_ConfigGet(uint8 endPoint, afAPSF_Config_t *pCfg);

 /*
  *	afAPSF_ConfigSet - set the fragmentation configuration for the specified EndPoint.
  */
afStatus_t afAPSF_ConfigSet(uint8 endPoint, afAPSF_Config_t *pCfg);

 /*
  *	afSetApplCB - Sets the pointer to the Application Callback function for a
  *               specific EndPoint.
  */
uint8 afSetApplCB( uint8 endPoint, pApplCB pApplFn );

#ifdef __cplusplus
}
#endif
#endif
/**************************************************************************************************
*/
