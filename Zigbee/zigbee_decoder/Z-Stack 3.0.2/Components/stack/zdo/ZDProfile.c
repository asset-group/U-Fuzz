/**************************************************************************************************
  Filename:       ZDProfile.c
  Revised:        $Date: 2015-10-14 11:48:06 -0700 (Wed, 14 Oct 2015) $
  Revision:       $Revision: 44530 $

  Description:    This is the Zigbee Device Profile.


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

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "NLMEDE.h"
#include "nwk_util.h"
#include "APS.h"

#include "AddrMgr.h"
#include "ZDConfig.h"
#include "ZDProfile.h"
#include "ZDObject.h"
#include "ZDNwkMgr.h"

#if defined( LCD_SUPPORTED )
  #include "OnBoard.h"
#endif

#include "nwk_util.h"

#if defined( MT_ZDO_FUNC )
  #include "MT_ZDO.h"
#endif

/*********************************************************************
 * MACROS
 */

#define ZADDR_TO_AFADDR( pZADDR, AFADDR ) {                            \
  (AFADDR).endPoint = ZDP_AF_ENDPOINT;                                 \
  (AFADDR).addrMode = (afAddrMode_t)(pZADDR)->addrMode;                \
  (AFADDR).addr.shortAddr = (pZADDR)->addr.shortAddr;                  \
}

#define FillAndSendBuffer( TRANSSEQ, ADDR, ID, LEN, BUF ) {     \
  afStatus_t stat;                                    \
  ZDP_TmpBuf = (BUF)+1;                               \
  stat = fillAndSend( (TRANSSEQ), (ADDR), (ID), (LEN) );          \
  osal_mem_free( (BUF) );                             \
  ZDP_TmpBuf = ZDP_Buf+1;                             \
  return stat;                                        \
}

#define FillAndSendTxOptions( TRANSSEQ, ADDR, ID, LEN, TxO ) {  \
  afStatus_t stat;                                    \
  ZDP_TxOptions = (TxO);                              \
  stat = fillAndSend( (TRANSSEQ), (ADDR), (ID), (LEN) );          \
  ZDP_TxOptions = AF_TX_OPTIONS_NONE;                 \
  return stat;                                        \
}

#define FillAndSendBufferTxOptions( TRANSSEQ, ADDR, ID, LEN, BUF, TxO ) { \
  afStatus_t stat;                                    \
  ZDP_TmpBuf = (BUF)+1;                               \
  ZDP_TxOptions = (TxO);                              \
  stat = fillAndSend( (TRANSSEQ), (ADDR), (ID), (LEN) );          \
  osal_mem_free( (BUF) );                             \
  ZDP_TmpBuf = ZDP_Buf+1;                             \
  ZDP_TxOptions = AF_TX_OPTIONS_NONE;                 \
  return stat;                                        \
}

/*********************************************************************
 * CONSTANTS
 */

#define ZDP_BUF_SZ          80

CONST byte ZDP_AF_ENDPOINT = 0;

// Routing table options
#define ZP_RTG_RECORD       0x04       // Routing table indication that a route record is needed
#define ZP_MTO_ROUTE_RC     0x08       // Concentrator with routing cache
#define ZP_MTO_ROUTE_NRC    0x10       // Concentrator with limited cache


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  void *next;
  uint8 taskID;
  uint16 clusterID;
} ZDO_MsgCB_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */

byte ZDP_TransID = 0;
uint8 childIndex = 0;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

extern endPointDesc_t ZDApp_epDesc;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void ZDApp_SetParentAnnceTimer( void );

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static afStatus_t fillAndSend( uint8 *transSeq, zAddrType_t *addr, cId_t clusterID, byte len );
uint8 ZDO_SendMsgCBs( zdoIncomingMsg_t *inMsg );
void zdpProcessAddrReq( zdoIncomingMsg_t *inMsg );

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8  ZDP_Buf[ ZDP_BUF_SZ ];
static uint8 *ZDP_TmpBuf = ZDP_Buf+1;

byte ZDP_TxOptions = AF_TX_OPTIONS_NONE;
ZDO_MsgCB_t *zdoMsgCBs = (ZDO_MsgCB_t *)NULL;

/*********************************************************************
 * ZDO Message Processing table
 */

typedef void (*pfnZDPMsgProcessor)( zdoIncomingMsg_t *inMsg );

typedef struct
{
  uint16                clusterID;
  pfnZDPMsgProcessor    pFn;
} zdpMsgProcItem_t;

CONST zdpMsgProcItem_t zdpMsgProcs[] =
{
#if ( RFD_RCVC_ALWAYS_ON==TRUE ) || ( ZG_BUILD_RTR_TYPE )
  // These aren't processed by sleeping end devices.
  { Device_annce,           ZDO_ProcessDeviceAnnce },
#endif
#if ( ZG_BUILD_RTR_TYPE )
  // These aren't processed by end devices.
  { Parent_annce,           ZDO_ProcessParentAnnce },
  { Parent_annce_rsp,       ZDO_ProcessParentAnnceRsp },
#endif
  { NWK_addr_req,           zdpProcessAddrReq },
  { IEEE_addr_req,          zdpProcessAddrReq },
  { Node_Desc_req,          ZDO_ProcessNodeDescReq },
  { Node_Desc_rsp,          ZDO_ProcessNodeDescRsp },
  { Power_Desc_req,         ZDO_ProcessPowerDescReq },
  { Simple_Desc_req,        ZDO_ProcessSimpleDescReq },
  { Simple_Desc_rsp,        ZDO_ProcessSimpleDescRsp },
  { Active_EP_req,          ZDO_ProcessActiveEPReq },
  { Match_Desc_req,         ZDO_ProcessMatchDescReq },
#if defined ( ZDO_MGMT_NWKDISC_RESPONSE )
  { Mgmt_NWK_Disc_req,      ZDO_ProcessMgmtNwkDiscReq },
#endif
#if defined ( ZDO_MGMT_LQI_RESPONSE ) && ( ZG_BUILD_RTR_TYPE || ZG_BUILD_ENDDEVICE_TYPE )
  { Mgmt_Lqi_req,           ZDO_ProcessMgmtLqiReq },
#endif
#if defined ( ZDO_MGMT_RTG_RESPONSE ) && ( ZG_BUILD_RTR_TYPE )
  { Mgmt_Rtg_req,           ZDO_ProcessMgmtRtgReq },
#endif
#if defined ( ZDO_MGMT_BIND_RESPONSE ) && defined ( REFLECTOR )
  { Mgmt_Bind_req,          ZDO_ProcessMgmtBindReq },
#endif
#if defined ( ZDO_MGMT_JOINDIRECT_RESPONSE ) && ( ZG_BUILD_RTR_TYPE )
  { Mgmt_Direct_Join_req,   ZDO_ProcessMgmtDirectJoinReq },
#endif
#if defined ( ZDO_MGMT_LEAVE_RESPONSE )
  { Mgmt_Leave_req,         ZDO_ProcessMgmtLeaveReq },
#endif
#if defined ( ZDO_MGMT_PERMIT_JOIN_RESPONSE )  && ( ZG_BUILD_RTR_TYPE )
  { Mgmt_Permit_Join_req,   ZDO_ProcessMgmtPermitJoinReq },
#endif
#if defined ( ZDO_USERDESC_RESPONSE )
  { User_Desc_req,          ZDO_ProcessUserDescReq },
#endif
#if defined ( ZDO_USERDESCSET_RESPONSE )
  { User_Desc_set,          ZDO_ProcessUserDescSet },
#endif
#if defined ( ZDO_SERVERDISC_RESPONSE )
  { Server_Discovery_req,   ZDO_ProcessServerDiscReq },
#endif
  {0xFFFF, NULL} // Last
};

/*********************************************************************
 * @fn          fillAndSend
 *
 * @brief       Combined to reduce space
 *
 * @param
 * @param
 *
 * @return      afStatus_t
 */
static afStatus_t fillAndSend( uint8 *transSeq, zAddrType_t *addr, cId_t clusterID, byte len )
{
  afAddrType_t afAddr;

  osal_memset( &afAddr, 0, sizeof(afAddrType_t) );
  ZADDR_TO_AFADDR( addr, afAddr );

  *(ZDP_TmpBuf-1) = *transSeq;

  return AF_DataRequest( &afAddr, &ZDApp_epDesc, clusterID,
                           (uint16)(len+1), (uint8*)(ZDP_TmpBuf-1),
                           transSeq, ZDP_TxOptions,  AF_DEFAULT_RADIUS );

}

/*********************************************************************
 * @fn          ZDP_SendData
 *
 * @brief       This builds and send a request message that has
 *              NWKAddrOfInterest as its only parameter.
 *
 * @param       dstAddr - destination address
 * @param       cmd - clusterID
 * @param       dataLen - number of bytes of data
 * @param       data - pointer to the data
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_SendData( uint8 *TransSeq, zAddrType_t *dstAddr, uint16 cmd,
                        byte len, uint8 *buf, byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte cnt = len;

  while ( cnt-- )
  {
    *pBuf++ = *buf++;
  }

  FillAndSendTxOptions( TransSeq, dstAddr, cmd, len, ((SecurityEnable) ? AF_EN_SECURITY : 0) );
}

/*********************************************************************
 * @fn          ZDP_NWKAddrOfInterestReq
 *
 * @brief       This builds and send a request message that has
 *              NWKAddrOfInterest as its only parameter.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - 16 bit address
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_NWKAddrOfInterestReq( zAddrType_t *dstAddr, uint16 nwkAddr,
                                     byte cmd, byte SecurityEnable )
{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  ZDP_TmpBuf[0] = LO_UINT16( nwkAddr );
  ZDP_TmpBuf[1] = HI_UINT16( nwkAddr );

  return fillAndSend( &ZDP_TransID, dstAddr, cmd, 2 );
}

/*********************************************************************
 * Address Requests
 */

/*********************************************************************
 * @fn          ZDP_NwkAddrReq
 *
 * @brief       This builds and send a NWK_addr_req message.  This
 *              function sends a broadcast message looking for a 16
 *              bit address with a 64 bit address as bait.
 *
 * @param       IEEEAddress - looking for this device
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_NwkAddrReq( uint8 *IEEEAddress, byte ReqType,
                           byte StartIndex, byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = Z_EXTADDR_LEN + 1 + 1;  // IEEEAddress + ReqType + StartIndex.
  zAddrType_t dstAddr;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  if ( osal_ExtAddrEqual( saveExtAddr, IEEEAddress ) == FALSE )
  {
    dstAddr.addrMode = AddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
  }
  else
  {
    dstAddr.addrMode = Addr16Bit;
    dstAddr.addr.shortAddr = ZDAppNwkAddr.addr.shortAddr;
  }

  pBuf = osal_cpyExtAddr( pBuf, IEEEAddress );

  *pBuf++ = ReqType;
  *pBuf++ = StartIndex;

  return fillAndSend( &ZDP_TransID, &dstAddr, NWK_addr_req, len );
}

/*********************************************************************
 * @fn          ZDP_IEEEAddrReq
 *
 * @brief       This builds and send a IEEE_addr_req message.  This
 *              function sends a unicast message looking for a 64
 *              bit IEEE address with a 16 bit address as bait.
 *
 * @param       ReqType - ZDP_IEEEADDR_REQTYPE_SINGLE or
 *                        ZDP_IEEEADDR_REQTYPE_EXTENDED
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_IEEEAddrReq( uint16 shortAddr, byte ReqType,
                            byte StartIndex, byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = 2 + 1 + 1;  // shortAddr + ReqType + StartIndex.
  zAddrType_t dstAddr;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  dstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  dstAddr.addr.shortAddr = shortAddr;

  *pBuf++ = LO_UINT16( shortAddr );
  *pBuf++ = HI_UINT16( shortAddr );

  *pBuf++ = ReqType;
  *pBuf++ = StartIndex;

  return fillAndSend( &ZDP_TransID, &dstAddr, IEEE_addr_req, len );
}

/*********************************************************************
 * @fn          ZDP_MatchDescReq
 *
 * @brief       This builds and send a Match_Desc_req message.  This
 *              function sends a broadcast or unicast message
 *              requesting the list of endpoint/interfaces that
 *              match profile ID and cluster IDs.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - network address of interest
 * @param       ProfileID - Profile ID
 * @param       NumInClusters - number of input clusters
 * @param       InClusterList - input cluster ID list
 * @param       NumOutClusters - number of output clusters
 * @param       OutClusterList - output cluster ID list
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MatchDescReq( zAddrType_t *dstAddr, uint16 nwkAddr,
                                uint16 ProfileID,
                                byte NumInClusters, cId_t *InClusterList,
                                byte NumOutClusters, cId_t *OutClusterList,
                                byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  // nwkAddr+ProfileID+NumInClusters+NumOutClusters.
  byte i, len = 2 + 2 + 1 + 1;  // nwkAddr+ProfileID+NumInClusters+NumOutClusters.

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  len += (NumInClusters + NumOutClusters) * sizeof(uint16);

  if ( len >= ZDP_BUF_SZ-1 )
  {
    return afStatus_MEM_FAIL;
  }

  // The spec changed in Zigbee 2007 (2.4.3.1.7.1) to not allow sending
  // this command to 0xFFFF.  So, here we will filter this and replace
  // with 0xFFFD to only send to devices with RX ON.  This includes the
  // network address of interest.
  if ( ((dstAddr->addrMode == AddrBroadcast) || (dstAddr->addrMode == Addr16Bit))
      && (dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVALL) )
  {
    dstAddr->addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
  }
  if ( nwkAddr == NWK_BROADCAST_SHORTADDR_DEVALL )
  {
    nwkAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
  }

  *pBuf++ = LO_UINT16( nwkAddr );   // NWKAddrOfInterest
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = LO_UINT16( ProfileID );   // Profile ID
  *pBuf++ = HI_UINT16( ProfileID );

  *pBuf++ = NumInClusters; // Input cluster list
  if ( NumInClusters )
  {
    for (i=0; i<NumInClusters; ++i)  {
      *pBuf++ = LO_UINT16( InClusterList[i] );
      *pBuf++ = HI_UINT16( InClusterList[i] );
    }
  }

  *pBuf++ = NumOutClusters; // Output cluster list
  if ( NumOutClusters )
  {
    for (i=0; i<NumOutClusters; ++i)  {
      *pBuf++ = LO_UINT16( OutClusterList[i] );
      *pBuf++ = HI_UINT16( OutClusterList[i] );
    }
  }

  return fillAndSend( &ZDP_TransID, dstAddr, Match_Desc_req, len );
}

/*********************************************************************
 * @fn          ZDP_SimpleDescReq
 *
 * @brief       This builds and send a NWK_Simple_Desc_req
 *              message.  This function sends unicast message to the
 *              destination device.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - 16 bit address
 * @param       epIntf - endpoint/interface
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_SimpleDescReq( zAddrType_t *dstAddr, uint16 nwkAddr,
                                    byte endPoint, byte SecurityEnable )

{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  ZDP_TmpBuf[0] = LO_UINT16( nwkAddr );
  ZDP_TmpBuf[1] = HI_UINT16( nwkAddr );
  ZDP_TmpBuf[2] = endPoint;

  return fillAndSend( &ZDP_TransID, dstAddr, Simple_Desc_req, 3 );
}

/*********************************************************************
 * @fn          ZDP_UserDescSet
 *
 * @brief       This builds and send a User_Desc_set message to set
 *              the user descriptor.  This function sends unicast
 *              message to the destination device.
 *
 * @param       dstAddr - destination address
 * @param       nwkAddr - 16 bit address
 * @param       UserDescriptor - user descriptor
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_UserDescSet( zAddrType_t *dstAddr, uint16 nwkAddr,
                          UserDescriptorFormat_t *UserDescriptor,
                          byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = (UserDescriptor->len < AF_MAX_USER_DESCRIPTOR_LEN) ?
              UserDescriptor->len : AF_MAX_USER_DESCRIPTOR_LEN;
  byte addrLen = 2;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = len;
  addrLen = 3;

  pBuf = osal_memcpy( pBuf, UserDescriptor->desc, len );
  osal_memset( pBuf, AF_USER_DESCRIPTOR_FILL, AF_MAX_USER_DESCRIPTOR_LEN-len );

  return fillAndSend( &ZDP_TransID, dstAddr, User_Desc_set, (AF_MAX_USER_DESCRIPTOR_LEN + addrLen) );
}

/*********************************************************************
 * @fn          ZDP_ServerDiscReq
 *
 * @brief       Build and send a Server_Discovery_req request message.
 *
 * @param       serverMask - 16-bit bit-mask of server services being sought.
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_ServerDiscReq( uint16 serverMask, byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  zAddrType_t dstAddr;

  dstAddr.addrMode = AddrBroadcast;
  dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;

  *pBuf++ = LO_UINT16( serverMask );
  *pBuf = HI_UINT16( serverMask );

  FillAndSendTxOptions( &ZDP_TransID, &dstAddr, Server_Discovery_req, 2,
             ((SecurityEnable) ? AF_EN_SECURITY : AF_TX_OPTIONS_NONE) );
}

/*********************************************************************
 * @fn          ZDP_DeviceAnnce
 *
 * @brief       This builds and send a Device_Annce message.  This
 *              function sends a broadcast message.
 *
 * @param       nwkAddr - 16 bit address of the device
 * @param       IEEEAddr - 64 bit address of the device
 * @param       capabilities - device capabilities.  This field is only
 *                 sent for v1.1 networks.
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_DeviceAnnce( uint16 nwkAddr, uint8 *IEEEAddr,
                              byte capabilities, byte SecurityEnable )
{
  zAddrType_t dstAddr;
  uint8 len;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  dstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;

  ZDP_TmpBuf[0] = LO_UINT16( nwkAddr );
  ZDP_TmpBuf[1] = HI_UINT16( nwkAddr );
  osal_cpyExtAddr( &ZDP_TmpBuf[2], IEEEAddr );
  len = 2 + Z_EXTADDR_LEN;

  ZDP_TmpBuf[10] = capabilities;
  len++;

  return fillAndSend( &ZDP_TransID, &dstAddr, Device_annce, len );
}

/*********************************************************************
 * @fn          ZDP_ParentAnnce
 *
 * @brief       This builds and send a Parent_Annce and Parent_Annce_Rsp
 *              messages, it will depend on the clusterID parameter.
 *
 * @param       TransSeq - ZDP Transaction Sequence Number
 * @param       dstAddr - destination address
 * @param       numberOfChildren - 8 bit number of children
 * @param       childInfo - list of children information (ExtAddr and Age)
 * @param       clusterID - Parent_annce or Parent_annce_rsp
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_ParentAnnce( uint8 *TransSeq,
                            zAddrType_t *dstAddr,
                            uint8 numberOfChildren,
                            uint8 *childInfo,
                            cId_t clusterID,
                            uint8 SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  ZDO_ChildInfo_t *pChildInfo;
  uint8 i, len;
  uint8 *numOfChild;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  pChildInfo = (ZDO_ChildInfo_t *)childInfo;

  if ( dstAddr->addrMode == AddrBroadcast )
  {
    // Make sure is sent to 0xFFFC
    dstAddr->addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVZCZR;
  }
  len = 1;
  if ( clusterID == Parent_annce_rsp )
  {
    // + Status Byte
    len += 1;
    // Set the status bit to success
    *pBuf++ = 0;
  }
  
  numOfChild = pBuf;
  *pBuf++ = numberOfChildren;

  for ( i = 0; i < MAX_PARENT_ANNCE_CHILD; i++ )
  {
    pBuf = osal_cpyExtAddr( pBuf, pChildInfo[childIndex].extAddr );
    childIndex++;
    
    len += Z_EXTADDR_LEN;
    
    if ( childIndex == numberOfChildren )
    {
      pBuf = numOfChild;
      *pBuf = i + 1;
      // All childs are taken, restart index and go out
      childIndex = 0;
      return fillAndSend( TransSeq, dstAddr, clusterID, len );
    }
  }
  
  pBuf = numOfChild;
  *pBuf = MAX_PARENT_ANNCE_CHILD;
  if ( childIndex < numberOfChildren )
  {
    if ( clusterID == Parent_annce )
    {
      ZDApp_SetParentAnnceTimer();
    }
    if ( clusterID == Parent_annce_rsp )
    {
      osal_start_timerEx( ZDAppTaskID, ZDO_PARENT_ANNCE_EVT, 10 );
    }
  }

  return fillAndSend( TransSeq, dstAddr, clusterID, len );
}

/*********************************************************************
 * Address Responses
 */

/*********************************************************************
 * @fn      zdpProcessAddrReq
 *
 * @brief   Process an incoming NWK_addr_req or IEEE_addr_req message and then
 *          build and send a corresponding NWK_addr_rsp or IEEE_addr_rsp msg.
 *
 * @param   inMsg - incoming message
 *
 * @return  none
 */
void zdpProcessAddrReq( zdoIncomingMsg_t *inMsg )
{
  associated_devices_t *pAssoc;
  uint8 reqType;
  uint16 aoi = INVALID_NODE_ADDR;
  uint8 *ieee = NULL;

  reqType = inMsg->asdu[(inMsg->clusterID == NWK_addr_req) ? Z_EXTADDR_LEN : sizeof( uint16 ) ];

  if ( inMsg->clusterID == NWK_addr_req )

  {
    ieee = inMsg->asdu;

    if ( osal_ExtAddrEqual( saveExtAddr, ieee ) )
    {
      aoi = ZDAppNwkAddr.addr.shortAddr;
    }
    // Handle response for sleeping end devices
    else if ( (ZSTACK_ROUTER_BUILD)
      && (((pAssoc = AssocGetWithExt( ieee )) != NULL)
             && ((pAssoc->nodeRelation == CHILD_RFD) || (pAssoc->nodeRelation == CHILD_RFD_RX_IDLE)) ) )
    {
      aoi = pAssoc->shortAddr;
      if ( reqType != ZDP_ADDR_REQTYPE_SINGLE )
        reqType = 0xFF; // Force Invalid
    }
  }
  else  // if ( inMsg->clusterID == IEEE_addr_req )
  {
    aoi = BUILD_UINT16( inMsg->asdu[0], inMsg->asdu[1] );

    if ( aoi == ZDAppNwkAddr.addr.shortAddr )
    {
      ieee = saveExtAddr;
    }
    else if ( (ZSTACK_ROUTER_BUILD)
      && (((pAssoc = AssocGetWithShort( aoi )) != NULL)
             && (pAssoc->nodeRelation == CHILD_RFD)) )
    {
      AddrMgrEntry_t addrEntry;
      addrEntry.user = ADDRMGR_USER_DEFAULT;
      addrEntry.index = pAssoc->addrIdx;
      if ( AddrMgrEntryGet( &addrEntry ) )
      {
        ieee = addrEntry.extAddr;
      }

      if ( reqType != ZDP_ADDR_REQTYPE_SINGLE )
        reqType = 0xFF; // Force Invalid
    }
  }

  if ( ((aoi != INVALID_NODE_ADDR) && (ieee != NULL)) || (inMsg->wasBroadcast == FALSE) )
  {
    uint8 stat;
    uint8 *pBuf = ZDP_TmpBuf;
    // Status + IEEE-Addr + Nwk-Addr.
    uint8 len = 1 + Z_EXTADDR_LEN + 2;

    // If aoi and iee are both setup, we found results
    if ( (aoi != INVALID_NODE_ADDR) && (ieee != NULL) )
    {
      stat = ((reqType == ZDP_ADDR_REQTYPE_SINGLE) || (reqType == ZDP_ADDR_REQTYPE_EXTENDED))
                    ? ZDP_SUCCESS : ZDP_INVALID_REQTYPE;
              
      if(stat == ZDP_INVALID_REQTYPE)
      {
        //R21 Errata update CCB 2111 
        if(inMsg->wasBroadcast == TRUE)
        {
          return;
        }
        
        stat = ZDP_INVALID_REQTYPE;
      }
    }
    else
    {
      // not found and the req was unicast to this device
      stat = ZDP_DEVICE_NOT_FOUND;

      // Fill in the missing field with this device's address
      if ( inMsg->clusterID == NWK_addr_req )
      {
        //CCB 2112 Zigbee Core spec
        aoi = 0xFFFF;
      }
      else
      {
        //CCB 2113 Zigbee Core spec
        uint8 invalidIEEEAddr[Z_EXTADDR_LEN];
        osal_memset(invalidIEEEAddr,0xFF,Z_EXTADDR_LEN);
        ieee = invalidIEEEAddr;
      }
    }

    *pBuf++ = stat;

    pBuf = osal_cpyExtAddr( pBuf, ieee );

    *pBuf++ = LO_UINT16( aoi );
    *pBuf++ = HI_UINT16( aoi );

    if ( ZSTACK_ROUTER_BUILD )
    {
      if ( (reqType == ZDP_ADDR_REQTYPE_EXTENDED) && (aoi == ZDAppNwkAddr.addr.shortAddr)
           && (stat == ZDP_SUCCESS) )
      {
        uint8  cnt = 0;
        
        //Updated to only search for ZED devices as per R21 spec (2.4.3.1.1.2)
        uint16 *list = AssocMakeList( &cnt );

        if ( list != NULL )
        {
          byte idx = inMsg->asdu[(((inMsg->clusterID == NWK_addr_req) ? Z_EXTADDR_LEN : sizeof( uint16 )) + 1)];
          uint16 *pList = list + idx;

          // NumAssocDev field is only present on success.
          if ( cnt > idx )
          {
            cnt -= idx;
            len += (cnt * sizeof( uint16 ));
          }
          else
          {
            cnt = 0;
          }
          *pBuf++ = cnt;
          len++;

          // StartIndex field is only present if NumAssocDev field is non-zero.
          *pBuf++ = idx;
          len++;

          while ( cnt != 0 )
          {
            *pBuf++ = LO_UINT16( *pList );
            *pBuf++ = HI_UINT16( *pList );
            pList++;
            cnt--;
          }

          osal_mem_free( (uint8 *)list );
        }
        else
        {
          // NumAssocDev field is only present on success.
          *pBuf++ = 0;
          len++;
        }
      }
    }

    ZDP_TxOptions = AF_MSG_ACK_REQUEST;
    fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), (cId_t)(inMsg->clusterID | ZDO_RESPONSE_BIT), len );
    ZDP_TxOptions = AF_TX_OPTIONS_NONE;
  }
}

/*********************************************************************
 * @fn          ZDP_NodeDescMsg
 *
 * @brief       Builds and sends a Node Descriptor message, unicast to the
 *              specified device.
 *
 * @param       inMsg - incoming message
 * @param       nwkAddr - 16 bit network address for device
 * @param       pNodeDesc - pointer to the node descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_NodeDescMsg( zdoIncomingMsg_t *inMsg,
                           uint16 nwkAddr, NodeDescriptorFormat_t *pNodeDesc )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len;

  len = 1 + 2 + 13;  // Status + nwkAddr + Node descriptor

  *pBuf++ = ZDP_SUCCESS;

  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = (byte)((pNodeDesc->ComplexDescAvail << 3) |
                     (pNodeDesc->UserDescAvail << 4) |
                     (pNodeDesc->LogicalType & 0x07));

  *pBuf++ = (byte)((pNodeDesc->FrequencyBand << 3) | (pNodeDesc->APSFlags & 0x07));
  *pBuf++ = pNodeDesc->CapabilityFlags;
  *pBuf++ = pNodeDesc->ManufacturerCode[0];
  *pBuf++ = pNodeDesc->ManufacturerCode[1];
  *pBuf++ = pNodeDesc->MaxBufferSize;
  *pBuf++ = pNodeDesc->MaxInTransferSize[0];
  *pBuf++ = pNodeDesc->MaxInTransferSize[1];

  *pBuf++ = LO_UINT16( pNodeDesc->ServerMask );
  *pBuf++ = HI_UINT16( pNodeDesc->ServerMask );
  *pBuf++ = pNodeDesc->MaxOutTransferSize[0];
  *pBuf++ = pNodeDesc->MaxOutTransferSize[1];
  *pBuf = pNodeDesc->DescriptorCapability;

  return fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), Node_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_PowerDescMsg
 *
 * @brief       Builds and sends a Power Descriptor message, unicast to the
 *              specified device.
 *
 * @param       inMsg - incoming message (request)
 * @param       nwkAddr - 16 bit network address for device
 * @param       pPowerDesc - pointer to the node descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_PowerDescMsg( zdoIncomingMsg_t *inMsg,
                     uint16 nwkAddr, NodePowerDescriptorFormat_t *pPowerDesc )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = 1 + 2 + 2;  // Status + nwkAddr + Node Power descriptor.

  *pBuf++ = ZDP_SUCCESS;

  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = (byte)((pPowerDesc->AvailablePowerSources << 4)
                    | (pPowerDesc->PowerMode & 0x0F));
  *pBuf++ = (byte)((pPowerDesc->CurrentPowerSourceLevel << 4)
                    | (pPowerDesc->CurrentPowerSource & 0x0F));

  return fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), Power_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_SimpleDescMsg
 *
 * @brief       Builds and sends a Simple Descriptor message, unicast to the
 *              specified device.
 *
 * @param       inMsg - incoming message (request)
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       pSimpleDesc - pointer to the node descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_SimpleDescMsg( zdoIncomingMsg_t *inMsg, byte Status,
                              SimpleDescriptionFormat_t *pSimpleDesc )
{
  uint8 *pBuf = ZDP_TmpBuf;
  uint8 i, len;

  if ( Status == ZDP_SUCCESS && pSimpleDesc )
  {
    // Status + NWKAddrOfInterest + desc length + empty simple descriptor.
    len = 1 + 2 + 1 + 8;
    len += (pSimpleDesc->AppNumInClusters + pSimpleDesc->AppNumOutClusters) * sizeof ( uint16 );
  }
  else
  {
    len = 1 + 2 + 1; // Status + desc length
  }
  if ( len >= ZDP_BUF_SZ-1 )
  {
    return afStatus_MEM_FAIL;
  }

  *pBuf++ = Status;
  
  //From spec 2.4.3.1.5 The NWKAddrOfInterest field shall match 
  //that specified in the original Simple_Desc_req command
  *pBuf++ = inMsg->asdu[0];  
  *pBuf++ = inMsg->asdu[1];

  if ( len > 4 )
  {
    *pBuf++ = len - 4;   // Simple descriptor length

    *pBuf++ = pSimpleDesc->EndPoint;
    *pBuf++ = LO_UINT16( pSimpleDesc->AppProfId );
    *pBuf++ = HI_UINT16( pSimpleDesc->AppProfId );
    *pBuf++ = LO_UINT16( pSimpleDesc->AppDeviceId );
    *pBuf++ = HI_UINT16( pSimpleDesc->AppDeviceId );

    *pBuf++ = (byte)(pSimpleDesc->AppDevVer & 0x0F);

    *pBuf++ = pSimpleDesc->AppNumInClusters;
    if ( pSimpleDesc->AppNumInClusters )
    {
      for (i=0; i<pSimpleDesc->AppNumInClusters; ++i)
      {
        *pBuf++ = LO_UINT16( pSimpleDesc->pAppInClusterList[i] );
        *pBuf++ = HI_UINT16( pSimpleDesc->pAppInClusterList[i] );
      }
    }

    *pBuf++ = pSimpleDesc->AppNumOutClusters;
    if ( pSimpleDesc->AppNumOutClusters )
    {
      for (i=0; i<pSimpleDesc->AppNumOutClusters; ++i)
      {
        *pBuf++ = LO_UINT16( pSimpleDesc->pAppOutClusterList[i] );
        *pBuf++ = HI_UINT16( pSimpleDesc->pAppOutClusterList[i] );
      }
    }
  }

  else
  {
    *pBuf = 0; // Description Length = 0;
  }

  return fillAndSend( &(inMsg->TransSeq), &(inMsg->srcAddr), Simple_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_EPRsp
 *
 * @brief       This builds and send an endpoint list. Used in
 *              Active_EP_rsp and Match_Desc_Rsp
 *              message.  This function sends unicast message to the
 *              requesting device.
 *
 * @param       MsgType - either Active_EP_rsp or Match_Desc_Rsp
 * @param       dstAddr - destination address
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       nwkAddr - Device's short address that this response describes
 * @param       Count - number of endpoint/interfaces in list
 * @param       pEPIntfList - Array of Endpoint/Interfaces
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_EPRsp( uint16 MsgType, byte TransSeq, zAddrType_t *dstAddr,
                        byte Status, uint16 nwkAddr, byte Count,
                        uint8 *pEPList,
                        byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = 1 + 2 + 1;  // Status + nwkAddr + endpoint/interface count.
  byte txOptions;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  if ( MsgType == Match_Desc_rsp )
    txOptions = AF_MSG_ACK_REQUEST;
  else
    txOptions = 0;

    *pBuf++ = Status;
  *pBuf++ = LO_UINT16( nwkAddr );
  *pBuf++ = HI_UINT16( nwkAddr );

  *pBuf++ = Count;   // Endpoint/Interface count

  if ( Count )
  {
    len += Count;
    osal_memcpy( pBuf, pEPList, Count );
  }

  FillAndSendTxOptions( &TransSeq, dstAddr, MsgType, len, txOptions );
}

/*********************************************************************
 * @fn          ZDP_UserDescRsp
 *
 * @brief       Build and send the User Decriptor Response.
 *
 *
 * @param       dstAddr - destination address
 * @param       nwkAddrOfInterest -
 * @param       userDesc -
 * @param       SecurityEnable - Security Options
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_UserDescRsp( byte TransSeq, zAddrType_t *dstAddr,
                uint16 nwkAddrOfInterest, UserDescriptorFormat_t *userDesc,
                byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = 1 + 2 + 1;  // Status + nwkAddr + descriptor length.

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  len += userDesc->len;

  *pBuf++ = ZSUCCESS;

  *pBuf++ = LO_UINT16( nwkAddrOfInterest );
  *pBuf++ = HI_UINT16( nwkAddrOfInterest );

  *pBuf++ = userDesc->len;
  osal_memcpy( pBuf, userDesc->desc, userDesc->len );

  return (ZStatus_t)fillAndSend( &TransSeq, dstAddr, User_Desc_rsp, len );
}

/*********************************************************************
 * @fn          ZDP_ServerDiscRsp
 *
 * @brief       Build and send the Server_Discovery_rsp response.
 *
 * @param       transID - Transaction sequence number of request.
 * @param       dstAddr - Network Destination Address.
 * @param       status - Status of response to request.
 * @param       aoi - Network Address of Interest of request.
 * @param       serverMask - Bit map of service(s) being sought.
 * @param       SecurityEnable - Security Options
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_ServerDiscRsp( byte transID, zAddrType_t *dstAddr, byte status,
                           uint16 aoi, uint16 serverMask, byte SecurityEnable )
{
  const byte len = 1  + 2;  // status + aoi + mask.
  uint8 *pBuf = ZDP_TmpBuf;
  ZStatus_t stat;

  // Intentionally unreferenced parameters
  (void)aoi;
  (void)SecurityEnable;

  *pBuf++ = status;

  *pBuf++ = LO_UINT16( serverMask );
  *pBuf++ = HI_UINT16( serverMask );

  ZDP_TxOptions = AF_MSG_ACK_REQUEST;
  stat = fillAndSend( &transID, dstAddr, Server_Discovery_rsp, len );
  ZDP_TxOptions = AF_TX_OPTIONS_NONE;

  return ( stat );
}

/*********************************************************************
 * @fn          ZDP_GenericRsp
 *
 * @brief       Sends a response message with only the parameter status
 *              byte and the addr of interest for data.
 *              This function sends unicast message to the
 *              requesting device.
 *
 * @param       dstAddr - destination address
 * @param       status  - generic status for response
 * @param       aoi     - address of interest
 * @param       dstAddr - destination address
 * @param       rspId   - response cluster ID
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_GenericRsp( byte TransSeq, zAddrType_t *dstAddr,
                     byte status, uint16 aoi, uint16 rspID, byte SecurityEnable )
{
  uint8 len;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  ZDP_TmpBuf[0] = status;
  ZDP_TmpBuf[1] = LO_UINT16( aoi );
  ZDP_TmpBuf[2] = HI_UINT16( aoi );

  // Length byte
  ZDP_TmpBuf[3] = 0;
  len = 4;

  return fillAndSend( &TransSeq, dstAddr, rspID, len );
}

/*********************************************************************
 * Binding
 */
/*********************************************************************
 * @fn          ZDP_EndDeviceBindReq
 *
 * @brief       This builds and sends a End_Device_Bind_req message.
 *              This function sends a unicast message.
 *
 * @param       dstAddr - destination address
 * @param       LocalCoordinator - short address of local coordinator
 * @param       epIntf - Endpoint/Interface of Simple Desc
 * @param       ProfileID - Profile ID
 *
 *   The Input cluster list is the opposite of what you would think.
 *   This is the output cluster list of this device
 * @param       NumInClusters - number of input clusters
 * @param       InClusterList - input cluster ID list
 *
 *   The Output cluster list is the opposite of what you would think.
 *   This is the input cluster list of this device
 * @param       NumOutClusters - number of output clusters
 * @param       OutClusterList - output cluster ID list
 *
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_EndDeviceBindReq( zAddrType_t *dstAddr,
                                 uint16 LocalCoordinator,
                                 byte endPoint,
                                 uint16 ProfileID,
                                 byte NumInClusters, cId_t *InClusterList,
                                 byte NumOutClusters, cId_t *OutClusterList,
                                 byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  uint8 i, len;
  uint8 *ieeeAddr;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // LocalCoordinator + SrcExtAddr + ep + ProfileID +  NumInClusters + NumOutClusters.
  len = 2 + Z_EXTADDR_LEN + 1 + 2 + 1 + 1;
  len += (NumInClusters + NumOutClusters) * sizeof ( uint16 );

  if ( len >= ZDP_BUF_SZ-1 )
  {
    return afStatus_MEM_FAIL;
  }

  if ( LocalCoordinator != NLME_GetShortAddr() )
  {
    return afStatus_INVALID_PARAMETER;
  }

  *pBuf++ = LO_UINT16( LocalCoordinator );
  *pBuf++ = HI_UINT16( LocalCoordinator );

  ieeeAddr = NLME_GetExtAddr();
  pBuf = osal_cpyExtAddr( pBuf, ieeeAddr );

  *pBuf++ = endPoint;

  *pBuf++ = LO_UINT16( ProfileID );   // Profile ID
  *pBuf++ = HI_UINT16( ProfileID );

  *pBuf++ = NumInClusters; // Input cluster list
  for ( i = 0; i < NumInClusters; ++i )
  {
    *pBuf++ = LO_UINT16(InClusterList[i]);
    *pBuf++ = HI_UINT16(InClusterList[i]);
  }

  *pBuf++ = NumOutClusters; // Output cluster list
  for ( i = 0; i < NumOutClusters; ++i )
  {
    *pBuf++ = LO_UINT16(OutClusterList[i]);
    *pBuf++ = HI_UINT16(OutClusterList[i]);
  }

  return fillAndSend( &ZDP_TransID, dstAddr, End_Device_Bind_req, len );
}

/*********************************************************************
 * @fn          ZDP_BindUnbindReq
 *
 * @brief       This builds and send a Bind_req or Unbind_req message
 *              Depending on the ClusterID. This function
 *              sends a unicast message to the local coordinator.
 *
 * @param       BindOrUnbind - either Bind_req or Unbind_req
 * @param       dstAddr - destination address of the message
 * @param       SourceAddr - source 64 bit address of the binding
 * @param       SrcEPIntf - Source endpoint/interface
 * @param       ClusterID - Binding cluster ID
 * @param       DestinationAddr - destination 64 bit addr of binding
 * @param       DstEPIntf - destination endpoint/interface
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_BindUnbindReq( uint16 BindOrUnbind, zAddrType_t *dstAddr,
                              uint8 *SourceAddr, byte SrcEndPoint,
                              cId_t ClusterID,
                              zAddrType_t *destinationAddr, byte DstEndPoint,
                              byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // SourceAddr + SrcEPIntf + ClusterID +  addrMode.
  len = Z_EXTADDR_LEN + 1 + sizeof( cId_t ) + sizeof( uint8 );
  if ( destinationAddr->addrMode == Addr64Bit )
    len += Z_EXTADDR_LEN + 1;     // +1 for DstEPIntf
  else if ( destinationAddr->addrMode == AddrGroup )
    len += sizeof ( uint16 );

  pBuf = osal_cpyExtAddr( pBuf, SourceAddr );
  *pBuf++ = SrcEndPoint;

  *pBuf++ = LO_UINT16( ClusterID );

  *pBuf++ = HI_UINT16( ClusterID );
  *pBuf++ = destinationAddr->addrMode;
  if ( destinationAddr->addrMode == Addr64Bit )
  {
    pBuf = osal_cpyExtAddr( pBuf, destinationAddr->addr.extAddr );
    *pBuf = DstEndPoint;
  }
  else if ( destinationAddr->addrMode == AddrGroup )
  {
    *pBuf++ = LO_UINT16( destinationAddr->addr.shortAddr );
    *pBuf++ = HI_UINT16( destinationAddr->addr.shortAddr );
  }

  FillAndSendTxOptions( &ZDP_TransID, dstAddr, BindOrUnbind, len, AF_MSG_ACK_REQUEST );
}

/*********************************************************************
 * Network Management
 */

/*********************************************************************
 * @fn          ZDP_MgmtNwkDiscReq
 *
 * @brief       This builds and send a Mgmt_NWK_Disc_req message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       ScanChannels - 32 bit address bit map
 * @param       StartIndex - Starting index within the reporting network
 *                           list
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkDiscReq( zAddrType_t *dstAddr,
                               uint32 ScanChannels,
                               byte ScanDuration,
                               byte StartIndex,
                               byte SecurityEnable )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = sizeof( uint32 )+1+1;  // ScanChannels + ScanDuration + StartIndex.

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  pBuf = osal_buffer_uint32( pBuf, ScanChannels );

  *pBuf++ = ScanDuration;
  *pBuf = StartIndex;

  return fillAndSend( &ZDP_TransID, dstAddr, Mgmt_NWK_Disc_req, len );
}

/*********************************************************************
 * @fn          ZDP_MgmtDirectJoinReq
 *
 * @brief       This builds and send a Mgmt_Direct_Join_req message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       deviceAddr - 64 bit IEEE Address
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtDirectJoinReq( zAddrType_t *dstAddr,
                               uint8 *deviceAddr,
                               byte capInfo,
                               byte SecurityEnable )
{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  osal_cpyExtAddr( ZDP_TmpBuf, deviceAddr );
  ZDP_TmpBuf[Z_EXTADDR_LEN] = capInfo;

  return fillAndSend( &ZDP_TransID, dstAddr, Mgmt_Direct_Join_req, (Z_EXTADDR_LEN + 1) );
}

/*********************************************************************
 * @fn          ZDP_MgmtPermitJoinReq
 *
 * @brief       This builds and send a Mgmt_Permit_Join_req message.
 *
 * @param       dstAddr - destination address of the message
 * @param       duration - Permit duration
 * @param       TcSignificance - Trust Center Significance
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtPermitJoinReq( zAddrType_t *dstAddr, byte duration,
                                  byte TcSignificance, byte SecurityEnable )
{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // Build buffer
  ZDP_TmpBuf[ZDP_MGMT_PERMIT_JOIN_REQ_DURATION] = duration;
  ZDP_TmpBuf[ZDP_MGMT_PERMIT_JOIN_REQ_TC_SIG]   = TcSignificance;

  // Check of this is a broadcast message
  if ( (dstAddr) && ((dstAddr->addrMode == Addr16Bit) || (dstAddr->addrMode == AddrBroadcast))
      && ((dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVALL)
          || (dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVZCZR)
          || (dstAddr->addr.shortAddr == NWK_BROADCAST_SHORTADDR_DEVRXON)) )
  {
    // Send this to our self as well as broadcast to network
    zAddrType_t tmpAddr;

    tmpAddr.addrMode = Addr16Bit;
    tmpAddr.addr.shortAddr = NLME_GetShortAddr();

    fillAndSend( &ZDP_TransID, &tmpAddr, Mgmt_Permit_Join_req,
                      ZDP_MGMT_PERMIT_JOIN_REQ_SIZE );
  }

  // Send the message
  return fillAndSend( &ZDP_TransID, dstAddr, Mgmt_Permit_Join_req,
                      ZDP_MGMT_PERMIT_JOIN_REQ_SIZE );
}

/*********************************************************************
 * @fn          ZDP_MgmtLeaveReq
 *
 * @brief       This builds and send a Mgmt_Leave_req message.
 *
 * @param       dstAddr - destination address of the message
 *              IEEEAddr - IEEE adddress of device that is removed
 *              RemoveChildren - set to 1 to remove the children of the
 *                                device as well. 0 otherwise.
 *              Rejoin - set to 1 if the removed device should rejoin
                         afterwards. 0 otherwise.
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtLeaveReq( zAddrType_t *dstAddr, uint8 *IEEEAddr, uint8 RemoveChildren,
                 uint8 Rejoin, uint8 SecurityEnable )

{
  (void)SecurityEnable;  // Intentionally unreferenced parameter

  osal_cpyExtAddr( ZDP_TmpBuf, IEEEAddr );
  ZDP_TmpBuf[Z_EXTADDR_LEN] = 0;

  if ( RemoveChildren == TRUE )
  {
    ZDP_TmpBuf[Z_EXTADDR_LEN] |= ZDP_MGMT_LEAVE_REQ_RC;
  }
  if ( Rejoin == TRUE )
  {
    ZDP_TmpBuf[Z_EXTADDR_LEN] |= ZDP_MGMT_LEAVE_REQ_REJOIN;
  }

  return fillAndSend( &ZDP_TransID, dstAddr, Mgmt_Leave_req, (Z_EXTADDR_LEN + 1) );
}

/*********************************************************************
 * @fn          ZDP_MgmtNwkUpdateReq
 *
 * @brief       This builds and send a Mgmt_NWK_Update_req message. This
 *              function sends a unicast or broadcast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       ChannelMask - 32 bit address bit map
 * @param       ScanDuration - length of time to spend scanning each channel
 * @param       ScanCount - number of energy scans to be conducted
 * @param       NwkUpdateId - NWk Update Id value
 * @param       NwkManagerAddr - NWK address for device with Network Manager
 *                               bit set in its Node Descriptor
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkUpdateReq( zAddrType_t *dstAddr,
                                 uint32 ChannelMask,
                                 uint8 ScanDuration,
                                 uint8 ScanCount,
                                 uint8 NwkUpdateId,
                                 uint16 NwkManagerAddr )
{
  uint8 *pBuf = ZDP_TmpBuf;
  byte len = sizeof( uint32 ) + 1;  // ChannelMask + ScanDuration

  pBuf = osal_buffer_uint32( pBuf, ChannelMask );

  *pBuf++ = ScanDuration;

  if ( ScanDuration <= 0x05 )
  {
    // Request is to scan over channelMask
    len += sizeof( uint8 );

    *pBuf++ = ScanCount;
  }
  else if ( ( ScanDuration == 0xFE ) || ( ScanDuration == 0xFF ) )
  {
    // Request is to change Channel (0xFE) or apsChannelMask and NwkManagerAddr (0xFF)
    len += sizeof( uint8 );

    *pBuf++ = NwkUpdateId;

    if ( ScanDuration == 0xFF )
    {
      len += sizeof( uint16 );

      *pBuf++  = LO_UINT16( NwkManagerAddr );
      *pBuf++  = HI_UINT16( NwkManagerAddr );
    }
  }

  return fillAndSend( &ZDP_TransID, dstAddr, Mgmt_NWK_Update_req, len );
}


/*********************************************************************
 * Network Management Responses
 */

/*********************************************************************
 * @fn          ZDP_MgmtNwkDiscRsp
 *
 * @brief       This builds and send a Mgmt_NWK_Disc_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       NetworkCount - Total number of networks found
 * @param       StartIndex - Starting index within the reporting network
 *                           list
 * @param       NetworkListCount - number of network lists included
 *                                 in this message
 * @param       NetworkList - List of network descriptors
 * @param       SecurityEnable - Security Options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkDiscRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte NetworkCount,
                            byte StartIndex,
                            byte NetworkListCount,
                            networkDesc_t *NetworkList,
                            byte SecurityEnable )
{
  uint8 *buf;
  uint8 *pBuf;
  byte len = 1+1+1+1;  // Status + NetworkCount + StartIndex + NetworkCountList.
  byte idx;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  len += (NetworkListCount * ( ZDP_NETWORK_EXTENDED_DISCRIPTOR_SIZE - 2 ));

  buf = osal_mem_alloc( len+1 );
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = NetworkCount;
  *pBuf++ = StartIndex;
  *pBuf++ = NetworkListCount;

  for ( idx = 0; idx < NetworkListCount; idx++ )
  {
    osal_cpyExtAddr( pBuf, NetworkList->extendedPANID);
    pBuf += Z_EXTADDR_LEN;

    *pBuf++  = NetworkList->logicalChannel;                // LogicalChannel
    *pBuf    = NetworkList->stackProfile;                  // Stack profile
    *pBuf++ |= (byte)(NetworkList->version << 4);          // ZigBee Version
    *pBuf    = BEACON_ORDER_NO_BEACONS;                    // Beacon Order
    *pBuf++ |= (uint8)(BEACON_ORDER_NO_BEACONS << 4);      // Superframe Order

    if ( NetworkList->chosenRouter != INVALID_NODE_ADDR )
    {
      *pBuf++ = TRUE;                         // Permit Joining
    }
    else
    {
      *pBuf++ = FALSE;
    }

    NetworkList = NetworkList->nextDesc;    // Move to next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_NWK_Disc_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtLqiRsp
 *
 * @brief       This builds and send a Mgmt_Lqi_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       NeighborLqiEntries - Total number of entries found
 * @param       StartIndex - Starting index within the reporting list
 * @param       NeighborLqiCount - number of lists included
 *                                 in this message
 * @param       NeighborLqiList - List of NeighborLqiItems.  This list
 *                is the list to be sent, not the entire list
 * @param       SecurityEnable - true if secure
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_MgmtLqiRsp( byte TransSeq, zAddrType_t *dstAddr,
                          byte Status,
                          byte NeighborLqiEntries,
                          byte StartIndex,
                          byte NeighborLqiCount,
                          ZDP_MgmtLqiItem_t* NeighborList,
                          byte SecurityEnable )
{
  ZDP_MgmtLqiItem_t* list = NeighborList;
  uint8 *buf, *pBuf;
  byte len, x;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  if ( ZSuccess != Status )
  {
    ZDP_TmpBuf[0] = Status;
    return fillAndSend( &TransSeq, dstAddr, Mgmt_Lqi_rsp, 1 );
  }

  // (Status + NeighborLqiEntries + StartIndex + NeighborLqiCount) +
  //  neighbor LQI data.
  len = (1 + 1 + 1 + 1) + (NeighborLqiCount * ZDP_MGMTLQI_EXTENDED_SIZE);

  buf = osal_mem_alloc( len+1 );
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = NeighborLqiEntries;
  *pBuf++ = StartIndex;
  *pBuf++ = NeighborLqiCount;

  for ( x = 0; x < NeighborLqiCount; x++ )
  {
    osal_cpyExtAddr( pBuf, list->extPanID);         // Extended PanID
    pBuf += Z_EXTADDR_LEN;

    // EXTADDR
    pBuf = osal_cpyExtAddr( pBuf, list->extAddr );

    // NWKADDR
    *pBuf++ = LO_UINT16( list->nwkAddr );
    *pBuf++ = HI_UINT16( list->nwkAddr );

    // DEVICETYPE
    *pBuf = list->devType;

    // RXONIDLE
    *pBuf |= (uint8)(list->rxOnIdle << 2);

    // RELATIONSHIP
    *pBuf++ |= (uint8)(list->relation << 4);

    // PERMITJOINING
    *pBuf++ = (uint8)(list->permit);

    // DEPTH
    *pBuf++ = list->depth;

    // LQI
    *pBuf++ = list->lqi;

    list++; // next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_Lqi_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtRtgRsp
 *
 * @brief       This builds and send a Mgmt_Rtg_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       RoutingTableEntries - Total number of entries
 * @param       StartIndex - Starting index within the reporting list
 * @param       RoutingTableListCount - number of entries included
 *                                      in this message
 * @param       RoutingTableList - List of Routing Table entries
 * @param       SecurityEnable - true to enable security for this message
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_MgmtRtgRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte RoutingTableEntries,
                            byte StartIndex,
                            byte RoutingListCount,
                            rtgItem_t *RoutingTableList,
                            byte SecurityEnable )
{
  uint8 *buf;
  uint8 *pBuf;
  // Status + RoutingTableEntries + StartIndex + RoutingListCount.
  byte len = 1 + 1 + 1 + 1;
  byte x;

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // Add an array for Routing List data
  len += (RoutingListCount * ZDP_ROUTINGENTRY_SIZE);

  buf = osal_mem_alloc( (short)(len+1) );
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = RoutingTableEntries;
  *pBuf++ = StartIndex;
  *pBuf++ = RoutingListCount;

  for ( x = 0; x < RoutingListCount; x++ )
  {
    *pBuf++ = LO_UINT16( RoutingTableList->dstAddress );  // Destination Address
    *pBuf++ = HI_UINT16( RoutingTableList->dstAddress );

    *pBuf = (RoutingTableList->status & 0x07);
    if ( RoutingTableList->options & (ZP_MTO_ROUTE_RC | ZP_MTO_ROUTE_NRC) )
    {
      uint8 options = 0;
      options |= ZDO_MGMT_RTG_ENTRY_MANYTOONE;

      if ( RoutingTableList->options & ZP_RTG_RECORD )
      {
        options |= ZDO_MGMT_RTG_ENTRY_ROUTE_RECORD_REQUIRED;
      }

      if ( RoutingTableList->options & ZP_MTO_ROUTE_NRC )
      {
        options |= ZDO_MGMT_RTG_ENTRY_MEMORY_CONSTRAINED;
      }

      *pBuf |= (options << 3);
    }
    pBuf++;

    *pBuf++ = LO_UINT16( RoutingTableList->nextHopAddress );  // Next hop
    *pBuf++ = HI_UINT16( RoutingTableList->nextHopAddress );
    RoutingTableList++;    // Move to next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_Rtg_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtBindRsp
 *
 * @brief       This builds and send a Mgmt_Bind_rsp message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       Status - message status (ZDP_SUCCESS or other)
 * @param       BindingTableEntries - Total number of entries
 * @param       StartIndex - Starting index within the reporting list
 * @param       BindingTableListCount - number of entries included
 *                                 in this message
 * @param       BindingTableList - List of Binding Table entries
 * @param       SecurityEnable - Security Options
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDP_MgmtBindRsp( byte TransSeq, zAddrType_t *dstAddr,
                            byte Status,
                            byte BindingTableEntries,
                            byte StartIndex,
                            byte BindingTableListCount,
                            apsBindingItem_t *BindingTableList,
                            byte SecurityEnable )
{
  uint8 *buf;
  uint8 *pBuf;
  uint8 maxLen; // maxLen is the maximum packet length to allocate enough memory space
  uint8 len;    // Actual length varies due to different addrMode
  uint8 x;
  byte extZdpBindEntrySize = ZDP_BINDINGENTRY_SIZE + 1 + 1; // One more byte for cluserID and DstAddrMode
  byte shortZdpBindEntrySize = ZDP_BINDINGENTRY_SIZE + 1 + 1 + 2 - 8 - 1; // clusterID + DstAddrMode + shortAddr - ExtAddr - DstEndpoint

  (void)SecurityEnable;  // Intentionally unreferenced parameter

  // Status + BindingTableEntries + StartIndex + BindingTableListCount.
  maxLen = 1 + 1 + 1 + 1;
  maxLen += (BindingTableListCount * extZdpBindEntrySize );  //max length
  buf = osal_mem_alloc( maxLen + 1 );  // +1 for transaction ID

  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = Status;
  *pBuf++ = BindingTableEntries;
  *pBuf++ = StartIndex;
  *pBuf++ = BindingTableListCount;

  // Initial length = Status + BindingTableEntries + StartIndex + BindingTableListCount.
  // length += ZDP_BINDINGENTRY_SIZE   -- Version 1.0
  //           extZdpBindEntrySize     -- Version 1.1 extended address mode
  //           shortZdpBindEntrySize   -- Version 1.1 group address mode

  len = 1 + 1 + 1 + 1;
  for ( x = 0; x < BindingTableListCount; x++ )
  {
    pBuf = osal_cpyExtAddr( pBuf, BindingTableList->srcAddr );
    *pBuf++ = BindingTableList->srcEP;

    // Cluster ID
    *pBuf++ = LO_UINT16( BindingTableList->clusterID );
    *pBuf++ = HI_UINT16( BindingTableList->clusterID );

    *pBuf++ = BindingTableList->dstAddr.addrMode;
    if ( BindingTableList->dstAddr.addrMode == Addr64Bit )
    {
      len += extZdpBindEntrySize;
      pBuf = osal_cpyExtAddr( pBuf, BindingTableList->dstAddr.addr.extAddr );
      *pBuf++ = BindingTableList->dstEP;
    }
    else
    {
      len += shortZdpBindEntrySize;
      *pBuf++ = LO_UINT16( BindingTableList->dstAddr.addr.shortAddr );
      *pBuf++ = HI_UINT16( BindingTableList->dstAddr.addr.shortAddr );
    }
    BindingTableList++;    // Move to next list entry
  }

  FillAndSendBuffer( &TransSeq, dstAddr, Mgmt_Bind_rsp, len, buf );
}

/*********************************************************************
 * @fn          ZDP_MgmtNwkUpdateNotify
 *
 * @brief       This builds and send a Mgmt_NWK_Update_notify message. This
 *              function sends a unicast message.
 *
 * @param       dstAddr - destination address of the message
 * @param       status - message status (ZDP_SUCCESS or other)
 * @param       scannedChannels - List of channels scanned by the request
 * @param       totalTransmissions - Total transmissions
 * @param       transmissionFailures - Sum of transmission failures
 * @param       listCount - Number of records contained in the energyValues list
 * @param       energyValues - List of descriptors, one for each of ListCount,
 *                             of the enegry detect descriptors
 * @param       txOptions - Transmit options
 * @param       securityEnable - Security options
 *
 * @return      afStatus_t
 */
afStatus_t ZDP_MgmtNwkUpdateNotify( uint8 TransSeq, zAddrType_t *dstAddr,
                                    uint8 status, uint32 scannedChannels,
                                    uint16 totalTransmissions, uint16 transmissionFailures,
                                    uint8 listCount, uint8 *energyValues, uint8 txOptions,
                                    uint8 securityEnable )
{
  uint8 *buf;
  uint8 *pBuf;
  uint8 len;

  (void)securityEnable;  // Intentionally unreferenced parameter

  // Status + ScannedChannels + totalTransmissions + transmissionFailures + ListCount + energyValues
  len = 1 + 4 + 2 + 2 + 1 + listCount;

  buf = osal_mem_alloc( len+1 ); // +1 for transaction ID
  if ( buf == NULL )
  {
    return afStatus_MEM_FAIL;
  }

  pBuf = buf+1;

  *pBuf++ = status;

  pBuf = osal_buffer_uint32( pBuf, scannedChannels );

  *pBuf++ = LO_UINT16( totalTransmissions );
  *pBuf++ = HI_UINT16( totalTransmissions );

  *pBuf++ = LO_UINT16( transmissionFailures );
  *pBuf++ = HI_UINT16( transmissionFailures );

  *pBuf++ = listCount;

  if ( listCount > 0 )
    osal_memcpy( pBuf, energyValues, listCount );

  FillAndSendBufferTxOptions( &TransSeq, dstAddr, Mgmt_NWK_Update_notify, len, buf, txOptions );
}

/*********************************************************************
 * Functions to register for ZDO Over-the-air messages
 */

/*********************************************************************
 * @fn          ZDO_RegisterForZDOMsg
 *
 * @brief       Call this function to register of an incoming over
 *              the air ZDO message - probably a response message
 *              but requests can also be received.
 *              Messages are delivered to the task with ZDO_CB_MSG
 *              as the message ID.
 *
 * @param       taskID - Where you would like the message delivered
 * @param       clusterID - What message?
 *                          ZDO_ALL_MSGS_CLUSTERID - all responses
 *                          and device announce
 *
 * @return      ZSuccess - successful, ZMemError if not
 */
ZStatus_t ZDO_RegisterForZDOMsg( uint8 taskID, uint16 clusterID )
{
  ZDO_MsgCB_t *pList;
  ZDO_MsgCB_t *pLast;
  ZDO_MsgCB_t *pNew;

  // Look for duplicate
  pList = pLast = zdoMsgCBs;
  while ( pList )
  {
    if ( pList->taskID == taskID && pList->clusterID == clusterID )
      return ( ZSuccess );
    pLast = pList;
    pList = (ZDO_MsgCB_t *)pList->next;
  }

  // Add to the list
  pNew = (ZDO_MsgCB_t *)osal_mem_alloc( sizeof ( ZDO_MsgCB_t ) );
  if ( pNew )
  {
    pNew->taskID = taskID;
    pNew->clusterID = clusterID;
    pNew->next = NULL;
    if ( zdoMsgCBs )
    {
      pLast->next = pNew;
    }
    else
      zdoMsgCBs = pNew;
    return ( ZSuccess );
  }
  else
    return ( ZMemError );
}

/*********************************************************************
 * @fn          ZDO_RemoveRegisteredCB
 *
 * @brief       Call this function if you don't want to receive the
 *              incoming message.
 *
 * @param       taskID - Where the messages are being delivered.
 * @param       clusterID - What message?
 *
 * @return      ZSuccess - successful, ZFailure if not found
 */
ZStatus_t ZDO_RemoveRegisteredCB( uint8 taskID, uint16 clusterID )
{
  ZDO_MsgCB_t *pList;
  ZDO_MsgCB_t *pLast = NULL;

  pList = zdoMsgCBs;
  while ( pList )
  {
    if ( pList->taskID == taskID && pList->clusterID == clusterID )
    {
      if ( pLast )
      {
        // remove this one from the linked list
        pLast->next = pList->next;
      }
      else if ( pList->next )
      {
        // remove the first one from the linked list
        zdoMsgCBs = pList->next;
      }
      else
      {
        // remove the only item from the list
        zdoMsgCBs = (ZDO_MsgCB_t *)NULL;
      }
      osal_mem_free( pList );
      return ( ZSuccess );
    }
    pLast = pList;
    pList = pList->next;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn          ZDO_SendMsgCBs
 *
 * @brief       This function sends messages to registered tasks.
 *              Local to ZDO and shouldn't be called outside of ZDO.
 *
 * @param       inMsg - incoming message
 *
 * @return      TRUE if sent to at least 1 task, FALSE if not
 */
uint8 ZDO_SendMsgCBs( zdoIncomingMsg_t *inMsg )
{
  uint8 ret = FALSE;
  ZDO_MsgCB_t *pList = zdoMsgCBs;
  while ( pList )
  {
    if ( (pList->clusterID == inMsg->clusterID)
       || ((pList->clusterID == ZDO_ALL_MSGS_CLUSTERID)
           && ((inMsg->clusterID & ZDO_RESPONSE_BIT) || (inMsg->clusterID == Device_annce))) )
    {
      zdoIncomingMsg_t *msgPtr;

      // Send the address to the task
      msgPtr = (zdoIncomingMsg_t *)osal_msg_allocate( sizeof( zdoIncomingMsg_t ) + inMsg->asduLen );
      if ( msgPtr )
      {
        // copy struct
        osal_memcpy( msgPtr, inMsg, sizeof( zdoIncomingMsg_t ));

        if ( inMsg->asduLen )
        {
          msgPtr->asdu = (byte*)(((byte*)msgPtr) + sizeof( zdoIncomingMsg_t ));
          osal_memcpy( msgPtr->asdu, inMsg->asdu, inMsg->asduLen );
        }

        msgPtr->hdr.event = ZDO_CB_MSG;
        osal_msg_send( pList->taskID, (uint8 *)msgPtr );
        ret = TRUE;
      }
    }
    pList = (ZDO_MsgCB_t *)pList->next;
  }
  return ( ret );
}

/*********************************************************************
 * Incoming message processor
 */

/*********************************************************************
 * @fn          ZDP_IncomingData
 *
 * @brief       This function indicates the transfer of a data PDU (ASDU)
 *              from the APS sub-layer to the ZDO.
 *
 * @param       pData - Incoming Message
 *
 * @return      none
 */
void ZDP_IncomingData( afIncomingMSGPacket_t *pData )
{
  uint8 x = 0;
  uint8 handled;
  zdoIncomingMsg_t inMsg;

  inMsg.srcAddr.addrMode = Addr16Bit;
  inMsg.srcAddr.addr.shortAddr = pData->srcAddr.addr.shortAddr;
  inMsg.wasBroadcast = pData->wasBroadcast;
  inMsg.clusterID = pData->clusterId;
  inMsg.SecurityUse = pData->SecurityUse;

  inMsg.asduLen = pData->cmd.DataLength-1;
  inMsg.asdu = pData->cmd.Data+1;
  inMsg.TransSeq = pData->cmd.Data[0];
  inMsg.macDestAddr = pData->macDestAddr;
  inMsg.macSrcAddr = pData->macSrcAddr;

  handled = ZDO_SendMsgCBs( &inMsg );

#if (defined MT_ZDO_CB_FUNC)
#if !defined MT_TASK
  if (zgZdoDirectCB)
#endif
  {
    MT_ZdoDirectCB( pData, &inMsg );
  }
#endif

  while ( zdpMsgProcs[x].clusterID != 0xFFFF )
  {
    if ( zdpMsgProcs[x].clusterID == inMsg.clusterID )
    {
      zdpMsgProcs[x].pFn( &inMsg );
      return;
    }
    x++;
  }

  // Handle unhandled messages
  if ( !handled )
    ZDApp_InMsgCB( &inMsg );
}

/*********************************************************************
*********************************************************************/

