/**************************************************************************************************
  Filename:       APSMEDE.h
  Revised:        $Date: 2015-06-02 15:55:43 -0700 (Tue, 02 Jun 2015) $
  Revision:       $Revision: 43961 $

  Description:    Primitives of the Application Support Sub Layer Data Entity (APSDE) and
                  Management Entity (APSME).


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

#ifndef APSMEDE_H
#define APSMEDE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "nwk_globals.h"
#include "AssocList.h"
#include "nwk_bufs.h"
#include "BindingTable.h"
#include "ssp.h"

/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS
 */
// Frame control fields
#define APS_FRAME_TYPE_MASK         0x03
#define APS_DATA_FRAME              0x00
#define APS_CMD_FRAME               0x01
#define APS_ACK_FRAME               0x02
#define STUB_APS_FRAME              0x03

#define APS_DELIVERYMODE_MASK       0x0C
#define APS_FC_DM_UNICAST           0x00
#define APS_FC_DM_INDIRECT          0x04
#define APS_FC_DM_BROADCAST         0x08
#define APS_FC_DM_GROUP             0x0C

#define APS_FC_ACK_FORMAT           0x10
#define APS_FC_SECURITY             0x20
#define APS_FC_ACK_REQ              0x40
#define APS_FC_EXTENDED             0x80

#define APS_XFC_FRAG_MASK           0x03
#define APS_XFC_FIRST_FRAG          0x01
#define APS_XFC_FRAGMENT            0x02
#define APS_XFC_RESERVED            0xFC

#define APS_FRAME_CTRL_FIELD_LEN     0x01
#define APS_DSTEP_ID_FIELD_LEN       0x01
#define APS_GROUP_ID_FIELD_LEN       0x02
#define APS_SRCEP_ID_FIELD_LEN       0x01
#define APS_CLUSTERID_FIELD_LEN_V1_0 0x01
#define APS_CLUSTERID_FIELD_LEN      0x02
#define APS_PROFILEID_FIELD_LEN      0x02
#define APS_FRAME_CNT_FIELD_LEN      0x01
#define APS_XFRAME_CTRL_FIELD_LEN    0x01
#define APS_BLOCK_CNT_FIELD_LEN      0x01
#define APS_ACK_BITS_FIELD_LEN       0x01

// Tx Options (bitmap values)
#define APS_TX_OPTIONS_SECURITY_ENABLE  0x0001u
//#define APS_TX_OPTIONS_USE_NWK_KEY    0x0002u remove from spec
#define APS_TX_OPTIONS_ACK              0x0004u
#define APS_TX_OPTIONS_PERMIT_FRAGMENT  0x0008u
#define APS_TX_OPTIONS_SKIP_ROUTING     0x0010u
#define APS_TX_OPTIONS_FIRST_FRAGMENT   0x0020u
#define APS_TX_OPTIONS_PREPROCESS       0x0040u
#define APS_TX_OPTIONS_RETRY_MSG        0x0080u
#define APS_TX_OPTIONS_REFLECTED_MSG    0x0100u

// APSDE header fields
#define APS_HDR_FC 0

// APSME CMD id index
#define APSME_CMD_ID 0

// APS commands
#define APSME_CMD_TRANSPORT_KEY    0x05
#define APSME_CMD_UPDATE_DEVICE    0x06
#define APSME_CMD_REMOVE_DEVICE    0x07
#define APSME_CMD_REQUEST_KEY      0x08
#define APSME_CMD_SWITCH_KEY       0x09
#define APSME_CMD_TUNNEL           0x0E
#define APSME_CMD_VERIFY_KEY       0x0F
#define APSME_CMD_CONFIRM_KEY      0x10
   
// APSME CMD packet fields (APSME_CMD_TRANSPORT_KEY)
#define APSME_TK_KEY_TYPE      1
#define APSME_TK_KEY           2
#define APSME_TK_COMMON_LEN    (uint8)         \
                               (APSME_TK_KEY + \
                                SEC_KEY_LEN   )
#define APSME_TK_KEY_SEQ_LEN   1
#define APSME_TK_INITIATOR_LEN 1

#define APSME_TK_TC_DST_ADDR  18
#define APSME_TK_TC_SRC_ADDR  26
#define APSME_TK_TC_KEY_LEN   34

#define APSME_TK_NWK_KEY_SEQ      18
#define APSME_TK_NWK_DST_ADDR     19
#define APSME_TK_NWK_SRC_ADDR     27
#define APSME_TK_NWK_KEY_LEN      35

#define APSME_TK_APP_PARTNER_ADDR 18
#define APSME_TK_APP_INITIATOR    26
#define APSME_TK_APP_KEY_LEN      27
                                 
#define APSME_TK_APP_SOURCE_ADDR_OFFSET  42                                 
#define APSME_REQ_KEY_CMD_OFFSET         15
                         
   
   
// APSME CMD packet fields (APSME_CMD_UPDATE_DEVICE)
#define APSME_UD_STANDARD_SECURED_REJOIN        0
#define APSME_UD_STANDARD_UNSECURED_JOIN        1
#define APSME_UD_DEVICE_LEFT                    2
#define APSME_UD_STANDARD_TRUST_CENTER_REJOIN   3
#define APSME_UD_HIGH_SECURED_REJOIN            4
#define APSME_UD_HIGH_UNSECURED_JOIN            5
#define APSME_UD_HIGH_UNSECURED_REJOIN          7

#define APSME_UD_EADDR     1
#define APSME_UD_SADDR_LSB 9
#define APSME_UD_SADDR_MSB 10
#define APSME_UD_STATUS    11
#define APSME_UD_LEN       12

// APSME CMD packet fields (APSME_CMD_REMOVE_DEVICE)
#define APSME_RD_LEN   9
#define APSME_RD_EADDR 1

// APSME CMD packet fields (APSME_CMD_REQUEST_KEY)
#define APSME_RK_KEY_TYPE 1
#define APSME_RK_EADDR    2
#define APSME_RK_SYS_LEN  2
#define APSME_RK_APP_LEN  10

// APSME CMD packet fields (APSME_CMD_SWITCH_KEY)
#define APSME_SK_SEQ_NUM 1
#define APSME_SK_LEN     2

// APSME CMD packet fields (APSME_CMD_TUNNEL)
#define APSME_TUNNEL_DEA  1 //destination extended address
//devtag.pro.security.remove
//      APSME_TUNNEL_AUX  9 //auxillary header(obsolete)
#define APSME_TUNNEL_TCMD 9 //tunnelled command
#define APSME_TUNNEL_LEN  9

// APSME CMD packet fields (APSME_CMD_VERIFY_KEY)
#define APSME_VK_KEY_TYPE 1
#define APSME_VK_EADDR    2
#define APSME_VK_HASH     10
#define APSME_VK_APP_LEN  26

// APSME CMD packet fields (APSME_CMD_CONFIRM_KEY)
#define APSME_CK_STATUS   1
#define APSME_CK_KEY_TYPE 2
#define APSME_CK_EADDR    3
#define APSME_CK_APP_LEN  11   

// APSME Coordinator/Trust Center NWK addresses
#define APSME_TRUSTCENTER_NWKADDR  NWK_PAN_COORD_ADDR

#if !defined( MAX_APS_FRAMECOUNTER_CHANGES )
  // The number of times the frame counter can change before
  // saving to NV
  #define MAX_APS_FRAMECOUNTER_CHANGES    1000
#endif

#if !defined( MAX_TCLK_FRAMECOUNTER_CHANGES )
  // The number of times the frame counter can change before
  // saving to NV
  #define MAX_TCLK_FRAMECOUNTER_CHANGES    10
#endif

/******************************************************************************
 * TYPEDEFS
 */

// AIB item Ids
typedef enum
{
  apsAddressMap = 0xA0,

  // Proprietary Items
  apsMaxBindingTime,
  apsBindingTable,
  apsNumBindingTableEntries,
  apsUseExtendedPANID,
  apsUseInsecureJoin,
  apsTrustCenterAddress = 0xAB,
  apsMAX_AIB_ITEMS  // Must be the last entry
} ZApsAttributes_t;

// Type of information being queried
typedef enum
{
  NWK_ADDR_LIST,
  EXT_ADDRESS,
  SIMPLE_DESC,
  NODE_DESC,
  POWER_DESC,
  SVC_MATCH
} APSME_query_t;

#define APS_ILLEGAL_DEVICES             0x02

// Structure returned from APSME_GetRequest for apsBindingTable
typedef struct
{
  uint8 srcAddr[Z_EXTADDR_LEN]; // Src address
  byte srcEP;                   // Endpoint/interface of source device
  uint16 clusterID;             // Cluster ID
  zAddrType_t dstAddr;          // Destination address
  byte dstEP;                   // Endpoint/interface of dest device
} apsBindingItem_t;

typedef struct
{
  uint8 FrmCtrl;
  uint8 XtndFrmCtrl;
  uint8 DstEndPoint;
  uint8 SrcEndPoint;
  uint16 GroupID;
  uint16 ClusterID;
  uint16 ProfileID;
  uint16 macDestAddr;
  uint8 wasBroadcast;
  uint8 apsHdrLen;
  uint8 *asdu;
  uint8 asduLength;
  uint8 ApsCounter;
  uint8 transID;
  uint8 BlkCount;
  uint8 AckBits;
  uint16 macSrcAddr;
} aps_FrameFormat_t;

typedef struct
{
  uint16 tna; // tunnel network address
  uint8* dea; // destination extended address
} APSDE_FrameTunnel_t;

// APS Data Service Primitives
typedef struct
{
  zAddrType_t dstAddr;
  uint8       srcEP;
  uint8       dstEP;
  uint16      dstPanId;
  uint16      clusterID;
  uint16      profileID;
  uint16      asduLen;
  uint8*      asdu;
  uint16      txOptions;
  uint8       transID;
  uint8       discoverRoute;
  uint8       radiusCounter;
  uint8       apsCount;
  uint8       blkCount;
  uint8       apsRetries;
  uint8       nsduHandle;
} APSDE_DataReq_t;

typedef struct
{
  uint16 dstAddr;
  uint8  dstEP;
  uint8  srcEP;
  uint8  transID;
  uint8  status;
} APSDE_DataCnf_t;

typedef struct
{
  uint8 secure;
  uint8 addressingMode; // Helps to identify the exact length of the payload.
} APSDE_DataReqMTU_t;

// APS Security Related Primitives
typedef struct
{
  uint16               dstAddr;
  uint8                keyType;
  uint8                keySeqNum;
  uint8*               key;
  uint8*               extAddr;
  uint8                initiator;
  uint8                apsSecure;
  uint8                nwkSecure;
  APSDE_FrameTunnel_t* tunnel;
} APSME_TransportKeyReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8  keyType;
  uint8  keySeqNum;
  uint8* key;
  uint8* dstExtAddr;
  uint8* srcExtAddr;
  uint8  initiator;
  uint8  secure;
} APSME_TransportKeyInd_t;

typedef struct
{
  uint16 dstAddr;
  uint16 devAddr;
  uint8* devExtAddr;
  uint8  status;
  uint8  apsSecure;
} APSME_UpdateDeviceReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8* devExtAddr;
  uint16 devAddr;
  uint8  status;
} APSME_UpdateDeviceInd_t;

typedef struct
{
  uint16 parentAddr;
  uint8* childExtAddr;
  uint8  apsSecure;
} APSME_RemoveDeviceReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8* childExtAddr;
} APSME_RemoveDeviceInd_t;

typedef struct
{
  uint8  dstAddr;
  uint8  keyType;
  uint8* partExtAddr;
} APSME_RequestKeyReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8  keyType;
  uint8* partExtAddr;
} APSME_RequestKeyInd_t;

typedef struct
{
  uint16 dstAddr;
  uint8  keySeqNum;
  uint8  apsSecure;
} APSME_SwitchKeyReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8  keySeqNum;
} APSME_SwitchKeyInd_t;

typedef struct
{
  uint8* tcExtAddr;
  uint8  keyType;
} APSME_VerifyKeyReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8  keyType;
  uint8* partExtAddr;
  uint8* receivedInitiatorHashValue;
} APSME_VerifyKeyInd_t;

typedef struct
{
  uint16 dstAddr;
  uint8  status;
  uint8* dstExtAddr;
  uint8  keyType;
} APSME_ConfirmKeyReq_t;

typedef struct
{
  uint16 srcAddr;
  uint8  status;
  uint8* srcExtAddr;
  uint8  keyType;
} APSME_ConfirmKeyInd_t;

// APS Incoming Command Packet
typedef struct
{
  osal_event_hdr_t hdr;
  uint8*           asdu;
  uint8            asduLen;
  uint8            secure;
  uint16           nwkAddr;
  uint8            nwkSecure;
} APSME_CmdPkt_t;

typedef struct
{
  uint8  key[SEC_KEY_LEN];
  uint32 txFrmCntr;
  uint32 rxFrmCntr;
} APSME_LinkKeyData_t;

typedef struct
{
  uint8   frmCtrl;
  uint8   xtndFrmCtrl;
  uint8   srcEP;
  uint8   dstEP;
  uint16  groupID;
  uint16  clusterID;
  uint16  profileID;
  uint8   asduLen;
  uint8*  asdu;
  uint8   hdrLen;
  uint8   apsCounter;
  uint8   transID;
  uint8   blkCount;
  uint8   ackBits;
} APSDE_FrameData_t;

typedef struct
{
  uint8  frmCtrl;
  uint8  xtndFrmCtrl;
  uint8  srcEP;
  uint8  dstEP;
  uint16 clusterID;
  uint16 profileID;
  uint8  asduLen;
  uint16 dstAddr;
  uint8  transID;
  uint8  apsCounter;
} APSDE_StoredFrameData_t;

typedef struct
{
//ZMacDataReq_t     mfd;
  NLDE_FrameData_t  nfd;
  APSDE_FrameData_t afd;
} APSDE_FrameFormat_t;

typedef struct
{
  uint16               dstAddr;
  uint8                frmCtrl;
  uint8                xtndFrmCtrl;
  uint8                asduLen;
  uint8                nwkSecure;
  APSDE_FrameTunnel_t* tunnel;
} APSDE_FrameAlloc_t;

typedef struct
{
  //input
  APSDE_FrameAlloc_t   fa;

  //output
  APSDE_FrameFormat_t* aff;
  SSP_Info_t*          si;
  uint8                status;
} APSDE_FrameBlk_t;

typedef struct
{
  uint32 txFrmCntr;
  uint32 rxFrmCntr;
  uint8  extAddr[Z_EXTADDR_LEN];
  uint8  keyAttributes;
  uint8  keyType;
  uint8  SeedShift_IcIndex;    //For Unique key this is the number of shifts, for IC this is the offset on the NvId index
} APSME_TCLKDevEntry_t;

typedef struct
{
  uint32 txFrmCntr;
  uint32 rxFrmCntr;
  uint8  pendingFlag;
} APSME_ApsLinkKeyFrmCntr_t;

typedef struct
{
  uint32 txFrmCntr;
  uint32 rxFrmCntr;
  uint8  pendingFlag;
} APSME_TCLinkKeyFrmCntr_t;

// Function pointer prototype to preprocess messages before calling NWK layer
typedef void (*apsPreProcessDataReq_t)( APSDE_FrameBlk_t *blk );

/******************************************************************************
 * GLOBAL VARIABLES
 */
// Store Frame Counters in RAM and update NV periodically
extern APSME_ApsLinkKeyFrmCntr_t ApsLinkKeyFrmCntr[];
extern APSME_TCLinkKeyFrmCntr_t TCLinkKeyFrmCntr[];

/******************************************************************************
 * APS Data Service
 *   APSDE-DATA
 */

/*
 * This function requests the transfer of data from the next higher layer
 * to a single peer entity.
 */
extern ZStatus_t APSDE_DataReq( APSDE_DataReq_t* req );

/*
 * This function requests the MTU(Max Transport Unit) of the APS Data Service
 */
extern uint8 APSDE_DataReqMTU( APSDE_DataReqMTU_t* fields );

/*
 * This function reports the results of a request to transfer a data
 * PDU (ASDU) from a local higher layer entity to another single peer entity.
 */
extern void APSDE_DataConfirm( nwkDB_t *rec, ZStatus_t Status );
extern void APSDE_DataCnf( APSDE_DataCnf_t* cnf );

/*
 * This function indicates the transfer of a data PDU (ASDU) from the
 * APS sub-layer to the local application layer entity.
 */
extern void APSDE_DataIndication( aps_FrameFormat_t *aff, zAddrType_t *SrcAddress,
                                uint16 SrcPanId, NLDE_Signal_t *sig, uint8 nwkSeqNum,
                                byte SecurityUse, uint32 timestamp, uint8 radius  );

/******************************************************************************
 * APS Management Service
 *   APSME-BIND
 *   APSME-UNBIND
 */

/*
 * This function allows the next higher layer to request to bind two devices together
 * either by proxy or directly, if issued on the coordinator.
 *
 * NOTE: The APSME-BIND.confirm is returned by this function and is not a
 *       seperate callback function.
 */
extern ZStatus_t APSME_BindRequest( byte SrcEndpInt, uint16 ClusterId,
                                   zAddrType_t *DstAddr, byte DstEndpInt);

/*
 * This function allows the next higher layer to request to unbind two devices
 * either by proxy or directly, if issued on the coordinator.
 *
 * NOTE: The APSME-UNBIND.confirm is returned by this function and is not a
 *       seperate callback function.
 */
extern ZStatus_t APSME_UnBindRequest( byte SrcEndpInt,
                            uint16 ClusterId, zAddrType_t *DstAddr, byte DstEndpInt);

/*
 * This function allows the next higher layer to read the value of an attribute
 * from the AIB (APS Information Base)
 */
extern ZStatus_t APSME_GetRequest( ZApsAttributes_t AIBAttribute,
                                    uint16 Index, byte *AttributeValue );

/*
 * This function allows the next higher layer to write the value of an attribute
 * into the AIB (APS Information Base)
 */
extern ZStatus_t APSME_SetRequest( ZApsAttributes_t AIBAttribute,
                                    uint16 Index, byte *AttributeValue );

/*
 * This function gets the EXT address based on the NWK address.
 */
extern uint8 APSME_LookupExtAddr( uint16 nwkAddr, uint8* extAddr );

/*
 * This function gets the NWK address based on the EXT address.
 */
extern uint8 APSME_LookupNwkAddr( uint8* extAddr, uint16* nwkAddr );

#if 0     // NOT IMPLEMENTED
/*
 * This function allows the next higher layer to be notified of the results of its
 * request to unbind two devices directly or by proxy.
 */
extern void APSME_UnbindConfirm( zAddrType_t CoorAddr,ZStatus_t Status,
                           uint16 SrcAddr, byte SrcEndpInt, byte ObjectId,
                           uint16 DstAddr, byte DstEndpInt);
/*
 * This function allows the next higher layer to be notified of the results of its
 * request to bind two devices directly or by proxy.
 */
extern void APSME_BindConfirm( zAddrType_t CoorAddr,ZStatus_t Status,
                           uint16 SrcAddr, byte SrcEndpInt, byte ObjectId,
                           uint16 DstAddr, byte DstEndpInt);
#endif  // NOT IMPLEMENTED

/*
 * Set the Preprocess function pointer.  The APS Layer will call this function
 * right before calling APSDE_FrameSend() [setup function that calls NLDE_DataReq()].
 */
extern void APSDE_SetPreProcessFnp( apsPreProcessDataReq_t pfnCB );


/******************************************************************************
 * APS Incoming Command Packet Handler
 */

/*
 * APSME_CmdPkt handles APS CMD packets.
 */
extern void APSME_CmdPkt( APSME_CmdPkt_t* pkt );

/******************************************************************************
 * APS Frame Allocation And Routing
 */

/*
 * APSDE_FrameAlloc allocates an APS frame.
 */
extern void APSDE_FrameAlloc( APSDE_FrameBlk_t* blk );

/*
 * APSDE_FrameSend sends an APS frame.
 */
extern void APSDE_FrameSend( APSDE_FrameBlk_t* blk );

/*
 * APSME_HoldDataRequests holds all data request for a timeout.
 */
void APSME_HoldDataRequests( uint16 holdTime );

/******************************************************************************
 * APS Security Related Functions
 */

/*
 * APSME_FrameSecurityRemove removes security from APS frame.
 */
extern ZStatus_t APSME_FrameSecurityRemove(uint16             srcAddr,
                                           aps_FrameFormat_t* aff);

/*
 * APSME_FrameSecurityApply applies security to APS frame.
 */
extern ZStatus_t APSME_FrameSecurityApply(uint16             dstAddr,
                                          aps_FrameFormat_t* aff);

/*
 * Configure APS security mode
 */
extern void APSME_SecurityNM( void );   // NULL MODE        - NO SECURITY
extern void APSME_SecurityRM_ED( void );// RESIDENTIAL MODE - END DEVICE
extern void APSME_SecurityRM_RD( void );// RESIDENTIAL MODE - ROUTER DEVICE
extern void APSME_SecurityRM_CD( void );// RESIDENTIAL MODE - COORD DEVICE
extern void APSME_SecurityCM_ED( void );// COMMERCIAL MODE  - END DEVICE
extern void APSME_SecurityCM_RD( void );// COMMERCIAL MODE  - ROUTER DEVICE
extern void APSME_SecurityCM_CD( void );// COMMERCIAL MODE  - COORD DEVICE

/******************************************************************************
 * APS Security Service Primitives - API, NHLE Calls Routines
 *
 *   APSME_TransportKeyReq
 *   APSME_UpdateDeviceReq
 *   APSME_RemoveDeviceReq
 *   APSME_RequestKeyReq
 *   APSME_SwitchKeyReq
 *   APSME_ConfirmKeyReq    // added for confirm key service
 */

/*
 * APSME_TransportKeyReq primitive.
 */
extern ZStatus_t APSME_TransportKeyReq( APSME_TransportKeyReq_t* req );

/*
 * APSME_UpdateDeviceReq primitive.
 */
extern ZStatus_t APSME_UpdateDeviceReq( APSME_UpdateDeviceReq_t* req );

/*
 * APSME_RemoveDeviceReq primitive.
 */
extern ZStatus_t APSME_RemoveDeviceReq( APSME_RemoveDeviceReq_t* req );

/*
 * APSME_RequestKeyReq primitive.
 */
extern ZStatus_t APSME_RequestKeyReq( APSME_RequestKeyReq_t* req );

/*
 * APSME_SwitchKeyReq primitive.
 */
extern ZStatus_t APSME_SwitchKeyReq( APSME_SwitchKeyReq_t* req );

/*
 * APSME_VerifyKeyReq_t primitive.
 */
extern ZStatus_t APSME_VerifyKeyReq( APSME_VerifyKeyReq_t* req );

/*
 * APSME_SwitchKeyReq primitive.
 */
extern ZStatus_t APSME_ConfirmKeyReq( APSME_ConfirmKeyReq_t* req );

/******************************************************************************
 * APS Security Primitive Stubs - API, NHLE Implements Callback Stubs
 *
 *   APSME_TransportKeyInd
 *   APSME_UpdateDeviceInd
 *   APSME_RemoveDeviceInd
 *   APSME_RequestKeyInd
 *   APSME_SwitchKeyInd
 */

/*
 * APSME_TransportKeyInd primitive.
 */
extern void APSME_TransportKeyInd( APSME_TransportKeyInd_t* ind );

/*
 * APSME_UpdateDeviceInd primitive.
 */
extern void APSME_UpdateDeviceInd( APSME_UpdateDeviceInd_t* ind );

/*
 * APSME_RemoveDeviceInd primitive.
 */
extern void APSME_RemoveDeviceInd( APSME_RemoveDeviceInd_t* ind );

/*
 * APSME_RequestKeyInd primitive.
 */
extern void APSME_RequestKeyInd( APSME_RequestKeyInd_t* ind );

/*
 * APSME_SwitchKeyInd primitive.
 */
extern void APSME_SwitchKeyInd( APSME_SwitchKeyInd_t* ind );

/*
 * APSME_VerifyKeyInd primitive.
 */
extern void APSME_VerifyKeyInd( APSME_VerifyKeyInd_t* ind );

/*
 * APSME_ConfirmKeyInd primitive.
 */
extern void APSME_ConfirmKeyInd( APSME_ConfirmKeyInd_t* apsmeInd );


/*
 * APSME_EraseICEntry
 */
extern void APSME_EraseICEntry(uint8 *IcIndex);

/*
 * APSME_AddTCLinkKey Interface to add TC link key derived from install codes.
 */
extern ZStatus_t APSME_AddTCLinkKey(uint8* pTCLinkKey, uint8* pExt);

/*
 * APSME_SetDefaultKey Interface to set the centralized default key to defaultTCLinkKey
 */
extern ZStatus_t APSME_SetDefaultKey(void);

/*
 * APSME_SearchTCLinkKeyEntry Interface search for the TCLK entry
 */
extern uint16 APSME_SearchTCLinkKeyEntry(uint8 *pExt,uint8* found, APSME_TCLKDevEntry_t* tcLinkKeyAddrEntry);
/******************************************************************************
 * APS Security Support - NHLE Implements Callback Stubs
 *
 *   APSME_LinkKeySet
 *   APSME_LinkKeyNVIdGet
 *   APSME_KeyFwdToChild
 */

/*
 * APSME_LinkKeySet stub.
 */
extern ZStatus_t APSME_LinkKeySet( uint8* extAddr, uint8* key );


/*
 * APSME_LinkKeyNVIdGet stub.
 */
extern ZStatus_t APSME_LinkKeyNVIdGet(uint8* extAddr, uint16 *pKeyNvId);

/*
 * APSME_IsLinkKeyValid stub.
 */
extern uint8 APSME_IsLinkKeyValid(uint8* extAddr);

/*
 * APSME_KeyFwdToChild stub.
 */
extern uint8 APSME_KeyFwdToChild( APSME_TransportKeyInd_t* ind );

/*
 * APSME_IsDistributedSecurity - Is APS configured for distributed secrity network
 * (not Trust Center protected).
 *    Use with ZG_SECURITY_SE_STANDARD
 */
extern uint8 APSME_IsDistributedSecurity( void );



/******************************************************************************
******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* APSMEDE_H */


