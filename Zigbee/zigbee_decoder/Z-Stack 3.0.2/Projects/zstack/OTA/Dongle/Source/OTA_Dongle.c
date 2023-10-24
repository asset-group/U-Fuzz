/**************************************************************************************************
  Filename:       OTA_Dongle.c
  Revised:        $Date: 2013-12-10 07:42:48 -0800 (Tue, 10 Dec 2013) $
  Revision:       $Revision: 36527 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2010-2013 Texas Instruments Incorporated. All rights reserved.

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
  This device will be like a Light device.  This application is not
  intended to be a Light device, but will use the device description
  to implement this sample code.

  Key Control - Refer to OTA_Dongle_HandleKeys():

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDProfile.h"
#include "ZDObject.h"
#include "AddrMgr.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ota.h"

#include "OTA_Dongle.h"
#include "ota_common.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include "MT.h"
#include "MT_APP.h"
#include "MT_NWK.h"

#include "bdb.h"
#include "bdb_interface.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 OTA_Dongle_TaskID;
uint8 OTA_Dongle_SeqNo;
devStates_t OTA_Dongle_devState;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

#define OTA_DONGLE_BINDINGLIST       2

// Endpoint to allow SYS_APP_MSGs
static endPointDesc_t ota_SysAppEp =
{
  OTA_SYSAPP_ENDPOINT,                // Sys App endpoint
  0,
  &OTA_Dongle_TaskID,
  (SimpleDescriptionFormat_t *) &OTA_Dongle_SimpleDesc,
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

static endPointDesc_t ota_DongleEp =
{
  OTA_DONGLE_ENDPOINT,                // Sys App endpoint
  0,
  &zcl_TaskID,
  (SimpleDescriptionFormat_t*) &OTA_Dongle_SimpleDesc,
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void OTA_Dongle_HandleKeys( byte shift, byte keys );
static void OTA_Dongle_BasicResetCB( void );
static void OTA_Dongle_ProcessIdentifyTimeChange( uint8 endpoint );

static void OTA_ProcessZDOMsgs(zdoIncomingMsg_t * pMsg);

static void OTA_ProcSysAppMsg(mtSysAppMsg_t *pMsg);
static void OTA_ProcessSysApp_ImageNotifyReq(uint8 *pData);
static void OTA_ProcessSysApp_ReadAttrReq(uint8 *pData);
static void OTA_ProcessSysApp_DiscoveryReq(uint8 *pData);
static void OTA_ProcessSysApp_JoinReq(uint8 *pData);

static void OTA_Send_DeviceInd(uint16 shortAddr);
static void OTA_Send_JoinInd(void);
static void OTA_Send_ReadAttrInd(uint16 cluster, uint16 shortAddr, zclReadRspStatus_t *pAttr);
static void OTA_Send_EndpointInd(uint16 addr, uint8 endpoint);
static void OTA_Send_DongleInd(void);

// Functions to process ZCL Foundation incoming Command/Response messages
static void OTA_Dongle_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 OTA_Dongle_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 OTA_Dongle_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 OTA_Dongle_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 OTA_Dongle_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t OTA_Dongle_CmdCallbacks =
{
  OTA_Dongle_BasicResetCB,                // Basic Cluster Reset command
  NULL,                                   // Identify Trigger Effect command
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                   // Level Control Move to Level command
  NULL,                                   // Level Control Move command
  NULL,                                   // Level Control Step command
  NULL,                                   // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                   // Scene Store Request command
  NULL,                                   // Scene Recall Request command
  NULL,                                   // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                   // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                   // Get Event Log command
  NULL,                                   // Publish Event Log command
#endif
  NULL,                                   // RSSI Location command
  NULL                                    // RSSI Location Response command
};

/*********************************************************************
 * @fn          OTA_Dongle_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void OTA_Dongle_Init( byte task_id )
{
  OTA_Dongle_TaskID = task_id;
  uint8 RxOnIdle = TRUE;

  OTA_Dongle_SeqNo = 0;

  ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( OTA_DONGLE_ENDPOINT, &OTA_Dongle_CmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( OTA_DONGLE_ENDPOINT, OTA_DONGLE_MAX_ATTRIBUTES, OTA_Dongle_Attrs );

  // Register the application's cluster option list
  zcl_registerClusterOptionList( OTA_DONGLE_ENDPOINT, OTA_DONGLE_MAX_OPTIONS, OTA_Dongle_Options );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( OTA_Dongle_TaskID );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( OTA_Dongle_TaskID );

  // Register endpoints
  afRegister( &ota_DongleEp );
  afRegister( &ota_SysAppEp );

  // Register with the ZDO to receive Match Descriptor Responses
  ZDO_RegisterForZDOMsg(task_id, Match_Desc_rsp);
  ZDO_RegisterForZDOMsg(task_id, Device_annce);

  bdb_RegisterIdentifyTimeChangeCB( OTA_Dongle_ProcessIdentifyTimeChange );

  // Start a timer to notify the console about the dongle
  osal_start_timerEx( OTA_Dongle_TaskID, OTA_DONGLE_DONGLE_NOTIFY_EVT, 4000 );
}

/*********************************************************************
 * @fn          OTA_Dongle_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 OTA_Dongle_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( OTA_Dongle_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          OTA_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;

        case MT_SYS_APP_MSG:
        case MT_SYS_APP_RSP_MSG:
          OTA_ProcSysAppMsg((mtSysAppMsg_t *)MSGpkt);
          break;

        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          OTA_Dongle_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          OTA_Dongle_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          OTA_Dongle_devState = (devStates_t)(MSGpkt->hdr.status);

          if ((OTA_Dongle_devState == DEV_END_DEVICE) || (OTA_Dongle_devState == DEV_ROUTER) || (OTA_Dongle_devState == DEV_ZB_COORD))
            OTA_Send_JoinInd();
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & OTA_DONGLE_DONGLE_NOTIFY_EVT )
  {
    OTA_Send_DongleInd();
    osal_start_timerEx( OTA_Dongle_TaskID, OTA_DONGLE_DONGLE_NOTIFY_EVT, 4000 );
    return ( events ^ OTA_DONGLE_DONGLE_NOTIFY_EVT );
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      OTA_ProcessSysApp_ImageNotifyReq
 *
 * @brief   Handles app messages from the console application.
 *
 * @param   pData - The data from the server.
 *
 * @return  none
 */
void OTA_ProcessSysApp_ImageNotifyReq(uint8 *pData)
{
  zclOTA_ImageNotifyParams_t imgNotifyParams;
  afAddrType_t dstAddr;

  // Setup the destination address
  dstAddr.addr.shortAddr = BUILD_UINT16(pData[0], pData[1]);
  dstAddr.endPoint = pData[2];
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.panId = _NIB.nwkPanId;

  // Fill the Send Image Notify Parameters
  imgNotifyParams.payloadType = pData[3];
  imgNotifyParams.queryJitter = pData[4];
  imgNotifyParams.fileId.manufacturer = BUILD_UINT16(pData[5], pData[6]);
  imgNotifyParams.fileId.type = BUILD_UINT16(pData[7], pData[8]);
  imgNotifyParams.fileId.version = BUILD_UINT32(pData[9], pData[10], pData[11], pData[12]);

  // Send the command
  zclOTA_SendImageNotify(&dstAddr, &imgNotifyParams);
}

/*********************************************************************
 * @fn      OTA_ProcessSysApp_ReadAttrReq
 *
 * @brief   Handles app messages from the console application.
 *
 * @param   pData - The data from the server.
 *
 * @return  none
 */
void OTA_ProcessSysApp_ReadAttrReq(uint8 *pData)
{
  uint8 readCmd[sizeof(zclReadCmd_t) + sizeof(uint16) * OTA_APP_MAX_ATTRIBUTES];
  zclReadCmd_t *pReadCmd = (zclReadCmd_t*) readCmd;
  afAddrType_t dstAddr;
  uint16 cluster;
  int8 i;

  // Setup the destination address
  dstAddr.addr.shortAddr = BUILD_UINT16(pData[0], pData[1]);
  dstAddr.endPoint = pData[2];
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.panId = _NIB.nwkPanId;

  // Fill the Send Image Notify Parameters
  cluster = BUILD_UINT16(pData[3], pData[4]);
  pReadCmd->numAttr = pData[5];

  if (pReadCmd->numAttr > OTA_APP_MAX_ATTRIBUTES)
    pReadCmd->numAttr = OTA_APP_MAX_ATTRIBUTES;

  pData += 6;
  for (i=0; i<pReadCmd->numAttr; i++)
  {
    pReadCmd->attrID[i] = BUILD_UINT16(pData[i*2], pData[i*2+1]);
  }

  // Send the command
  zcl_SendRead(OTA_DONGLE_ENDPOINT, &dstAddr, cluster, pReadCmd,
               ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, OTA_Dongle_SeqNo++);
}

/*********************************************************************
 * @fn      OTA_ProcessSysApp_DiscoveryReq
 *
 * @brief   Handles app messages from the console application.
 *
 * @param   pData - The data from the server.
 *
 * @return  none
 */
void OTA_ProcessSysApp_DiscoveryReq(uint8 *pData)
{

  cId_t otaCluster = ZCL_CLUSTER_ID_OTA;
  zAddrType_t dstAddr;

  // Send out a match for the key establishment
  dstAddr.addrMode = AddrBroadcast;
  dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
  ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR, ZCL_OTA_SAMPLE_PROFILE_ID,
                    0, NULL, 1, &otaCluster, FALSE );
}

/*********************************************************************
 * @fn      OTA_ProcessSysApp_JoinReq
 *
 * @brief   Handles app messages from the console application.
 *
 * @param   pData - The data from the server.
 *
 * @return  none
 */
void OTA_ProcessSysApp_JoinReq(uint8 *pData)
{
  // Setup Z-Stack global configuration
  zgConfigPANID = BUILD_UINT16(pData[0], pData[1]);
  zgDefaultChannelList = 0x00000800;
  zgDefaultChannelList <<= (pData[2] - 11);
  zgDefaultStartingScanDuration = 0;

  // Make sure all NWK layer callbacks go to the stack
  _nwkCallbackSub = 0;

  // Start the stack.  This will join the PAN specified above
  ZDOInitDevice(0);
}

/*********************************************************************
 * @fn      OTA_Send_DeviceInd
 *
 * @brief   Notifies the console about the existance of a device on the network.
 *
 * @param   none.
 *
 * @return  none
 */
void OTA_Send_DeviceInd(uint16 shortAddr)
{
  uint8 buffer[OTA_APP_DEVICE_IND_LEN];
  uint8 *pBuf = buffer;
  uint16 pan = _NIB.nwkPanId;

  *pBuf++ = OTA_SYSAPP_ENDPOINT;
  *pBuf++ = OTA_APP_DEVICE_IND;

  *pBuf++ = LO_UINT16(pan);
  *pBuf++ = HI_UINT16(pan);

  *pBuf++ = LO_UINT16(shortAddr);
  *pBuf++ = HI_UINT16(shortAddr);

  // Send the indication
  MT_BuildAndSendZToolResponse(MT_RPC_SYS_APP, MT_APP_MSG, OTA_APP_DEVICE_IND_LEN, buffer);
}

/*********************************************************************
 * @fn      OTA_Send_ReadAttrInd
 *
 * @brief   Notifies the console about attribute values for a device.
 *
 * @param   none.
 *
 * @return  none
 */
void OTA_Send_ReadAttrInd(uint16 cluster, uint16 shortAddr, zclReadRspStatus_t *pAttr)
{
  uint8 buffer[OTA_APP_READ_ATTRIBUTE_IND_LEN];
  uint8 *pBuf = buffer;
  uint8 len;

  *pBuf++ = OTA_SYSAPP_ENDPOINT;
  *pBuf++ = OTA_APP_READ_ATTRIBUTE_IND;

  *pBuf++ = LO_UINT16(_NIB.nwkPanId);
  *pBuf++ = HI_UINT16(_NIB.nwkPanId);

  *pBuf++ = LO_UINT16(cluster);
  *pBuf++ = HI_UINT16(cluster);

  *pBuf++ = LO_UINT16(shortAddr);
  *pBuf++ = HI_UINT16(shortAddr);

  *pBuf++ = LO_UINT16(pAttr->attrID);
  *pBuf++ = HI_UINT16(pAttr->attrID);
  *pBuf++ = pAttr->status;
  *pBuf++ = pAttr->dataType;

  len = zclGetDataTypeLength(pAttr->dataType);

  // We should not be reading attributes greater than 8 bytes in length
  if (len <= 8)
  {
  *pBuf++ = len;

  if (len)
  {
    uint8 *pStr;

    switch ( pAttr->dataType )
    {
      case ZCL_DATATYPE_UINT8:
        *pBuf = *((uint8 *)pAttr->data);
        break;

      case ZCL_DATATYPE_UINT16:
        *pBuf++ = LO_UINT16( *((uint16*)pAttr->data) );
        *pBuf++ = HI_UINT16( *((uint16*)pAttr->data) );
        break;

      case ZCL_DATATYPE_UINT32:
        pBuf = osal_buffer_uint32( pBuf, *((uint32*)pAttr->data) );
        break;

      case ZCL_DATATYPE_IEEE_ADDR:
        pStr = (uint8*)pAttr->data;
        osal_memcpy( pBuf, pStr, 8 );
        break;

      default:
        break;
    }
   }
  }
  else
    *pBuf = 0;

  // Send the indication
  MT_BuildAndSendZToolResponse(MT_RPC_SYS_APP, MT_APP_MSG, OTA_APP_READ_ATTRIBUTE_IND_LEN, buffer);
}

/*********************************************************************
 * @fn      OTA_ProcessSysApp_JoinReq
 *
 * @brief   Notifies the console that the dognle has joined a network.
 *
 * @param   none.
 *
 * @return  none
 */
void OTA_Send_JoinInd()
{
  uint8 buffer[OTA_APP_JOIN_IND_LEN];
  uint8 *pBuf = buffer;
  uint16 pan = _NIB.nwkPanId;

  *pBuf++ = OTA_SYSAPP_ENDPOINT;
  *pBuf++ = OTA_APP_JOIN_IND;

  *pBuf++ = LO_UINT16(pan);
  *pBuf = HI_UINT16(pan);

  // Send the indication
  MT_BuildAndSendZToolResponse(MT_RPC_SYS_APP, MT_APP_MSG, OTA_APP_JOIN_IND_LEN, buffer);
}

/*********************************************************************
 * @fn      OTA_ProcessSysApp_JoinReq
 *
 * @brief   Notifies the console about the OTA endpoint on a device.
 *
 * @param   none.
 *
 * @return  none
 */
void OTA_Send_EndpointInd(uint16 addr, uint8 endpoint)
{
  uint8 buffer[OTA_APP_ENDPOINT_IND_LEN];
  uint8 *pBuf = buffer;

  *pBuf++ = OTA_SYSAPP_ENDPOINT;
  *pBuf++ = OTA_APP_ENDPOINT_IND;

  *pBuf++ = LO_UINT16(_NIB.nwkPanId);
  *pBuf++ = HI_UINT16(_NIB.nwkPanId);

  *pBuf++ = LO_UINT16(addr);
  *pBuf++ = HI_UINT16(addr);

  *pBuf = endpoint;

  // Send the indication
  MT_BuildAndSendZToolResponse(MT_RPC_SYS_APP, MT_APP_MSG, OTA_APP_ENDPOINT_IND_LEN, buffer);
}

/*********************************************************************
 * @fn      OTA_Send_DongleInd
 *
 * @brief   Notifies the console about the Dongle.
 *
 * @param   none.
 *
 * @return  none
 */
void OTA_Send_DongleInd()
{
  uint8 buffer[OTA_APP_DONGLE_IND_LEN];
  uint8 *pBuf = buffer;

  *pBuf++ = OTA_SYSAPP_ENDPOINT;
  *pBuf++ = OTA_APP_DONGLE_IND;

  *pBuf++ = zgDeviceLogicalType;

  *pBuf++ = LO_UINT16(_NIB.nwkPanId);
  *pBuf++ = HI_UINT16(_NIB.nwkPanId);

  *pBuf++ = LO_UINT16(_NIB.nwkDevAddress);
  *pBuf++ = HI_UINT16(_NIB.nwkDevAddress);

  *pBuf++ = ZCL_OTA_ENDPOINT;

  *pBuf++ = _NIB.nwkLogicalChannel;

  *pBuf = OTA_Dongle_devState;

  // Send the indication
  MT_BuildAndSendZToolResponse(MT_RPC_SYS_APP, MT_APP_MSG, OTA_APP_DONGLE_IND_LEN, buffer);
}

/*********************************************************************
 * @fn      OTA_ProcSysAppMsg
 *
 * @brief   Handles sys app messages from the server application.
 *
 * @param   pMsg - The message from the server.
 *
 * @return  none
 */
void OTA_ProcSysAppMsg(mtSysAppMsg_t *pMsg)
{
  uint8 cmd;

  if (pMsg == NULL)
    return;

  cmd = *pMsg->appData++;

  switch(cmd)
  {
  case OTA_APP_READ_ATTRIBUTE_REQ:
    OTA_ProcessSysApp_ReadAttrReq(pMsg->appData);
    break;
  case OTA_APP_IMAGE_NOTIFY_REQ:
    OTA_ProcessSysApp_ImageNotifyReq(pMsg->appData);
    break;
  case OTA_APP_DISCOVERY_REQ:
    OTA_ProcessSysApp_DiscoveryReq(pMsg->appData);
    break;
  case OTA_APP_JOIN_REQ:
    OTA_ProcessSysApp_JoinReq(pMsg->appData);
    break;
  case OTA_APP_LEAVE_REQ:
    // Simulate a leave by rebooting the dongle
    SystemReset();
  default:
    break;
  }
}

/*********************************************************************
 * @fn      OTA_ProcessZDOMsgs
 *
 * @brief   Process messages from the ZDO layer.
 *
 * @param   pMsg - The message from the server.
 *
 * @return  none
 */
void OTA_ProcessZDOMsgs(zdoIncomingMsg_t * pMsg)
{
  if (pMsg)
  {
    if (pMsg->clusterID == Match_Desc_rsp)
    {
      ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( pMsg );

      if (pRsp)
      {
        // Notify the console application of the client device's OTA endpoint
        if (pRsp->cnt)
          OTA_Send_EndpointInd(pRsp->nwkAddr, pRsp->epList[0]);

        osal_mem_free(pRsp);
      }
    }
    else if (pMsg->clusterID == Device_annce)
    {
      cId_t otaCluster = ZCL_CLUSTER_ID_OTA;
      zAddrType_t dstAddr;

      ZDO_DeviceAnnce_t devAnnce;
      ZDO_ParseDeviceAnnce(pMsg, &devAnnce);
      OTA_Send_DeviceInd(devAnnce.nwkAddr);

      // Send out a match for the OTA cluster ID
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = devAnnce.nwkAddr;
      ZDP_MatchDescReq( &dstAddr, devAnnce.nwkAddr, ZCL_OTA_SAMPLE_PROFILE_ID,
                        0, NULL, 1, &otaCluster, FALSE );
    }
  }
}

/*********************************************************************
 * @fn      OTA_Dongle_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void OTA_Dongle_HandleKeys( byte shift, byte keys )
{
  (void)shift;  // Intentionally unreferenced parameter

  if ( keys & HAL_KEY_SW_1 )
  {
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING);
  }

  if ( keys & HAL_KEY_SW_2 )
  {
  }

  if ( keys & HAL_KEY_SW_3 )
  {
  }

  if ( keys & HAL_KEY_SW_4 )
  {
  }
  if ( keys & HAL_KEY_SW_5 )
  {
    bdb_resetLocalAction();
  }
  
}

/*********************************************************************
 * @fn      OTA_Dongle_ProcessIdentifyTimeChange
 *
 * @brief   Called to process any change to the IdentifyTime attribute.
 *
 * @param   none
 *
 * @return  none
 */
static void OTA_Dongle_ProcessIdentifyTimeChange( uint8 endpoint )
{
  if ( OTA_Dongle_IdentifyTime > 0 )
  {
    HalLedBlink ( HAL_LED_4, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
  }
}

/*********************************************************************
 * @fn      OTA_Dongle_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void OTA_Dongle_BasicResetCB( void )
{
  // Reset all attributes to default values
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      OTA_Dongle_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void OTA_Dongle_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg)
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      OTA_Dongle_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      OTA_Dongle_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      OTA_Dongle_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      OTA_Dongle_ProcessInDiscRspCmd( pInMsg );
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      OTA_Dongle_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 OTA_Dongle_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    OTA_Send_ReadAttrInd(pInMsg->clusterId, pInMsg->srcAddr.addr.shortAddr, &readRspCmd->attrList[i]);
  }

  return TRUE;
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      OTA_Dongle_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 OTA_Dongle_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return TRUE;
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      OTA_Dongle_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 OTA_Dongle_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return TRUE;
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      OTA_Dongle_ProcessInDiscRspCmd
 *
 * @brief   Process the "Profile" Discover Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 OTA_Dongle_ProcessInDiscRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}
#endif // ZCL_DISCOVER


/****************************************************************************
****************************************************************************/


