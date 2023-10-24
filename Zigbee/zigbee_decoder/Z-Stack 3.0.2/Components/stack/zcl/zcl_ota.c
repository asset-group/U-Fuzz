/******************************************************************************
  Filename:       zcl_ota.c
  Revised:        $Date: 2014-12-08 16:48:47 -0800 (Mon, 08 Dec 2014) $
  Revision:       $Revision: 41382 $

  Description:    Zigbee Cluster Library - Over-the-Air Upgrade Cluster ( OTA )


  Copyright 2010-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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
******************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ota.h"
#include "ota_common.h"

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_ota.h"

#include "MT_OTA.h"
#include "ZDProfile.h"
#include "ZDObject.h"

#if defined ( INTER_PAN )
#include "stub_aps.h"
#endif

#if defined OTA_MMO_SIGN
#include "ota_signature.h"
#endif

/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS
 */
#define OTA_MAX_TRANSACTIONS        4
#define OTA_TRANSACTION_EXPIRATION  1500

#define ZCL_OTA_HDR_LEN_OFFSET      6  // Header length location in OTA upgrade image
#define ZCL_OTA_STK_VER_OFFSET      18 // Stack version location in OTA upgrade image

#define OTA_NEW_IMAGE_QUERY_RATE    30000 // ms - 5 minutes
/******************************************************************************
 * GLOBAL VARIABLES
 */
#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
#if defined __IOCC2538_H__
const otaCrc_t OTA_CRC @ ".crc" =
{
  0x20130521,  // CRC
  0x20130521   // CRC Shadow
};
#else
#pragma location="CRC"
const CODE otaCrc_t OTA_CRC =
{
  0x2013,  // CRC
  0x2013   // CRC Shadow
};
#endif
#pragma required=OTA_CRC

#if defined __IOCC2538_H__
const preamble_t OTA_Preamble @ ".preamble" =
{
  HAL_OTA_RC_MAX,       // Default program length of max if not using post-processing tool.
  OTA_MANUFACTURER_ID,  // Manufacturer ID
  OTA_TYPE_ID,          // Image Type
  0x00000001            // Image Version
};
#else
#pragma location="PREAMBLE"
const CODE preamble_t OTA_Preamble =
{
  0xFFFFFFFF,           // Default program length of max if not using post-processing tool.
  OTA_MANUFACTURER_ID,  // Manufacturer ID
  OTA_TYPE_ID,          // Image Type
  0x00000001            // Image Version
};
#endif
#pragma required=OTA_Preamble
#endif


// OTA attribute variables
uint8 zclOTA_UpgradeServerID[Z_EXTADDR_LEN];
uint32 zclOTA_FileOffset = 0xFFFFFFFF;
uint32 zclOTA_CurrentFileVersion = 0xFFFFFFFF;
uint16 zclOTA_CurrentZigBeeStackVersion;
uint32 zclOTA_DownloadedFileVersion = 0xFFFFFFFF;
uint16 zclOTA_DownloadedZigBeeStackVersion = 0xFFFF;
uint8 zclOTA_ImageUpgradeStatus;
uint16 zclOTA_ManufacturerID;
uint16 zclOTA_ImageTypeID;
uint16 zclOTA_MinBlockReqDelay = 0;
uint32 zclOTA_ImageStamp;

// Other OTA variables
uint16 zclOTA_ManufacturerId;                           // Manufacturer ID
uint16 zclOTA_ImageType;                                // Image type
afAddrType_t zclOTA_serverAddr;                         // Server address
uint8 zclOTA_AppTask = 0xFF;                            // Callback Task ID
zclOTA_QueryImageRspParams_t queryResponse;             // Global variable for sent query response

// Image block command field control value
uint8 zclOTA_ImageBlockFC = OTA_BLOCK_FC_REQ_DELAY_PRESENT; // set bitmask field control value(s) for device

/******************************************************************************
 * LOCAL VARIABLES
 */
// Other OTA variables

// Task ID
static uint8 zclOTA_TaskID;

// Sequence number
static uint8 zclOTA_SeqNo = 0;

static uint8 zclOTA_Permit = TRUE;

#if defined OTA_MMO_SIGN
static OTA_MmoCtrl_t zclOTA_MmoHash;
static uint8 zclOTA_DataToHash[OTA_MMO_HASH_SIZE];
static uint8 zclOTA_HashPos;
static uint8 zclOTA_SignerIEEE[Z_EXTADDR_LEN];
static uint8 zclOTA_SignatureData[OTA_SIGNATURE_LEN];
static uint8 zclOTA_Certificate[OTA_CERTIFICATE_LEN];
#endif // OTA_MMO_SIGN

#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
static uint32 zclOTA_DownloadedImageSize;  // Downloaded image size
static uint16 zclOTA_HeaderLen;            // Image header length

static uint16 zclOTA_UpdateDelay;
static zclOTA_FileID_t zclOTA_CurrentDlFileId;

static uint16 zclOTA_ElementTag;
static uint32 zclOTA_ElementLen;
static uint32 zclOTA_ElementPos;

// Retry counters
static uint8 zclOTA_BlockRetry;
static uint8 zclOTA_UpgradeEndRetry;

static uint8 zclOTA_ClientPdState;

// OTA Header Magic Number Bytes
static const uint8 zclOTA_HdrMagic[] = {0x1E, 0xF1, 0xEE, 0x0B};

static cId_t otaClusters = ZCL_CLUSTER_ID_OTA;

// OTA Service Discover Information:
static uint8 zclOta_OtaZDPTransSeq;

#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

// Used by the client to correlate the Upgrade End Request and received
// Default Response.
static uint8 zclOta_OtaUpgradeEndReqTransSeq;
/******************************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclOTA_HdlIncoming ( zclIncoming_t *pInMsg );
static void zclOTA_ProcessUnhandledFoundationZCLMsgs ( zclIncomingMsg_t *pMsg );
static void zclOTA_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );

#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
static void zclOTA_StartTimer ( uint16 eventId, uint32 minutes );
static ZStatus_t sendImageBlockReq ( afAddrType_t *dstAddr );
static void zclOTA_ProcessZDOMsgs ( zdoIncomingMsg_t *pMsg );
static void zclOTA_ImageBlockWaitExpired ( void );
static void zclOTA_UpgradeComplete ( uint8 status );
static uint8 zclOTA_CmpFileId ( zclOTA_FileID_t *f1, zclOTA_FileID_t *f2 );
static uint8 zclOTA_ProcessImageData ( uint8 *pData, uint8 len );

static ZStatus_t zclOTA_SendQueryNextImageReq ( afAddrType_t *dstAddr, zclOTA_QueryNextImageReqParams_t *pParams );
static ZStatus_t zclOTA_SendImageBlockReq ( afAddrType_t *dstAddr, zclOTA_ImageBlockReqParams_t *pParams );
static ZStatus_t zclOTA_SendUpgradeEndReq ( afAddrType_t *dstAddr, zclOTA_UpgradeEndReqParams_t *pParams );

static ZStatus_t zclOTA_ClientHdlIncoming ( zclIncoming_t *pInMsg );
#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

#if (defined OTA_SERVER) && (OTA_SERVER == TRUE)
static ZStatus_t zclOTA_SendQueryNextImageRsp ( afAddrType_t *dstAddr, zclOTA_QueryImageRspParams_t *pParams );
static ZStatus_t zclOTA_SendImageBlockRsp ( afAddrType_t *dstAddr, zclOTA_ImageBlockRspParams_t *pParams );
static ZStatus_t zclOTA_SendUpgradeEndRsp ( afAddrType_t *dstAddr, zclOTA_UpgradeEndRspParams_t *pParams );
static ZStatus_t zclOTA_SendQuerySpecificFileRsp ( afAddrType_t *dstAddr, zclOTA_QueryImageRspParams_t *pParams );

static ZStatus_t zclOTA_Srv_QueryNextImageReq ( afAddrType_t *pSrcAddr, zclOTA_QueryNextImageReqParams_t *pParam );
static ZStatus_t zclOTA_Srv_ImageBlockReq ( afAddrType_t *pSrcAddr, zclOTA_ImageBlockReqParams_t *pParam );
static ZStatus_t zclOTA_Srv_ImagePageReq ( afAddrType_t *pSrcAddr, zclOTA_ImagePageReqParams_t *pParam );
static ZStatus_t zclOTA_Srv_UpgradeEndReq ( afAddrType_t *pSrcAddr, zclOTA_UpgradeEndReqParams_t *pParam );
static ZStatus_t zclOTA_Srv_QuerySpecificFileReq ( afAddrType_t *pSrcAddr, zclOTA_QuerySpecificFileReqParams_t *pParam );

static void zclOTA_ProcessNextImgRsp ( uint8* pMSGpkt, zclOTA_FileID_t *pFileId, afAddrType_t *pAddr );
static void zclOTA_ProcessFileReadRsp ( uint8* pMSGpkt, zclOTA_FileID_t *pFileId, afAddrType_t *pAddr );
static void zclOTA_ServerHandleFileSysCb ( OTA_MtMsg_t* pMSGpkt );

static ZStatus_t zclOTA_ServerHdlIncoming ( zclIncoming_t *pInMsg );

static void zclOTA_InitBlockReqDelay ( void );
#endif // (defined OTA_SERVER) && (OTA_SERVER == TRUE)

/******************************************************************************
 * OTA ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
#define ZCL_OTA_MAX_ATTRIBUTES          11
CONST zclAttrRec_t zclOTA_Attrs[ZCL_OTA_MAX_ATTRIBUTES] =
{
  {
    ZCL_CLUSTER_ID_OTA,                     // Cluster IDs - defined in the foundation (ie. zcl.h)
    { // Attribute record
      ATTRID_UPGRADE_SERVER_ID,             // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_IEEE_ADDR,               // Data Type - found in zcl.h
      ACCESS_CONTROL_READ | ACCESS_CLIENT,  // Variable access control - found in zcl.h
      ( void * ) &zclOTA_UpgradeServerID    // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_FILE_OFFSET,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_FileOffset
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_CURRENT_FILE_VERSION,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_CurrentFileVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_CURRENT_ZIGBEE_STACK_VERSION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_CurrentZigBeeStackVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_DOWNLOADED_FILE_VERSION,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_DownloadedFileVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_DOWNLOADED_ZIGBEE_STACK_VERSION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_DownloadedZigBeeStackVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_IMAGE_UPGRADE_STATUS,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_ImageUpgradeStatus
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_MANUFACTURER_ID,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_ManufacturerID
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_IMAGE_TYPE_ID,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_ImageTypeID
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_MINIMUM_BLOCK_REQ_DELAY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_MinBlockReqDelay
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    { // Attribute record
      ATTRID_IMAGE_STAMP,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclOTA_ImageStamp
    }
  }
};

/******************************************************************************
 * OTA SIMPLE DESCRIPTOR
 */
#ifndef OTA_HA
#define ZCL_OTA_MAX_OPTIONS 1
zclOptionRec_t zclOta_Options[ZCL_OTA_MAX_OPTIONS] =
{
  {
    ZCL_CLUSTER_ID_OTA,
    ( AF_EN_SECURITY | AF_ACK_REQUEST ),
  },
};
#endif // OTA_HA

#if defined OTA_SERVER && (OTA_SERVER == TRUE)

// Cluster ID list for match descriptor of the OTA Server.
#define ZCL_OTA_MAX_INCLUSTERS       1
const cId_t zclOTA_InClusterList[ZCL_OTA_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_OTA
};
#else

#define ZCL_OTA_MAX_INCLUSTERS        0
#define zclOTA_InClusterList          NULL

#endif // (defined OTA_SERVER) && (OTA_SERVER == TRUE)

#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

// Cluster ID list for match descriptor of the OTA Client.
#define ZCL_OTA_MAX_OUTCLUSTERS      1
const cId_t zclOTA_OutClusterList[ZCL_OTA_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_OTA
};

#else

#define ZCL_OTA_MAX_OUTCLUSTERS       0
#define zclOTA_OutClusterList         NULL

#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

SimpleDescriptionFormat_t zclOTA_SimpleDesc =
{
  ZCL_OTA_ENDPOINT,                 //  int Endpoint;
  ZCL_OTA_SAMPLE_PROFILE_ID,        //  uint16 AppProfId[2];
  ZCL_OTA_SAMPLE_DEVICEID,          //  uint16 AppDeviceId[2];
  ZCL_OTA_DEVICE_VERSION,           //  int   AppDevVer:4;
  ZCL_OTA_FLAGS,                    //  int   AppFlags:4;
  ZCL_OTA_MAX_INCLUSTERS,           //  byte  AppNumInClusters;
  ( cId_t * ) zclOTA_InClusterList, //  byte *pAppInClusterList;
  ZCL_OTA_MAX_OUTCLUSTERS,          //  byte  AppNumInClusters;
  ( cId_t * ) zclOTA_OutClusterList //  byte *pAppInClusterList;
};

// Endpoint for OTA Cluster
static endPointDesc_t zclOTA_Ep =
{
  ZCL_OTA_ENDPOINT,
  0,
#ifndef ZCL_STANDALONE
  &zcl_TaskID,
#else
  &zclOTA_TaskID,
#endif
  ( SimpleDescriptionFormat_t * ) &zclOTA_SimpleDesc,
  ( afNetworkLatencyReq_t ) 0
};

/******************************************************************************
 * @fn      zclOTA_PermitOta
 *
 * @brief   Called to enable/disable OTA operation.
 *
 * @param   permit - TRUE to enable OTA, FALSE to diable OTA
 *
 * @return  none
 */
void zclOTA_PermitOta ( uint8 permit )
{
  zclOTA_Permit = permit;
}

/******************************************************************************
 * @fn      zclOTA_Register
 *
 * @brief   Called by an application to register for callback messages
 *          from the OTA.
 *
 * @param   applicationTaskId - Application Task ID
 *
 * @return  none
 */
void zclOTA_Register ( uint8 applicationTaskId )
{
  zclOTA_AppTask = applicationTaskId;
}

/******************************************************************************
 * @fn      zclOTA_RequestNextUpdate
 *
 * @brief   Called by an application after discovery of the OTA server
 *          to initiate the query next image of the OTA server.
 *
 * @param   srvAddr - Short address of the server
 * @param   srvEndPoint - Endpoint on the server
 *
 * @return  ZStatus_t
 */
void zclOTA_RequestNextUpdate ( uint16 srvAddr, uint8 srvEndPoint )
{
  // Record the server address
  zclOTA_serverAddr.addrMode = afAddr16Bit;
  zclOTA_serverAddr.endPoint = srvEndPoint;
  zclOTA_serverAddr.addr.shortAddr = srvAddr;

  // Set an event to query the server
  osal_set_event ( zclOTA_TaskID, ZCL_OTA_QUERY_SERVER_EVT );
}

/******************************************************************************
 * @fn      zclOTA_Init
 *
 * @brief   Call to initialize the OTA Client Task
 *
 * @param   task_id
 *
 * @return  none
 */
void zclOTA_Init ( uint8 task_id )
{
  zclOTA_TaskID = task_id;

  // Register for the cluster endpoint
  afRegister ( &zclOTA_Ep );

  // Register as a ZCL Plugin
  zcl_registerPlugin ( ZCL_CLUSTER_ID_OTA,
                       ZCL_CLUSTER_ID_OTA,
                       zclOTA_HdlIncoming );

  // Initialize attribute variables
  zclOTA_CurrentZigBeeStackVersion = OTA_STACK_VER_PRO;
  zclOTA_ImageUpgradeStatus = OTA_STATUS_NORMAL;

  // Register attribute list
  zcl_registerAttrList ( ZCL_OTA_ENDPOINT,
                         ZCL_OTA_MAX_ATTRIBUTES,
                         zclOTA_Attrs );
#ifndef OTA_HA
  // Register the application's cluster option list
  zcl_registerClusterOptionList ( ZCL_OTA_ENDPOINT, ZCL_OTA_MAX_OPTIONS, zclOta_Options );
#endif // OTA_HA

  // Register with the ZDO to receive Network Address Responses
  ZDO_RegisterForZDOMsg ( task_id, IEEE_addr_rsp );

  // The default upgradeServerID is FF:FF:FF:FF:FF:FF:FF:FF
  osal_memset ( zclOTA_UpgradeServerID, 0xFF, sizeof ( zclOTA_UpgradeServerID ) );

#if defined (OTA_SERVER) && (OTA_SERVER == TRUE)

  // Register with the files system
  MT_OtaRegister ( task_id );

  // Initialize rate to transfer file
  zclOTA_InitBlockReqDelay();

#endif // defined (OTA_SERVER) && (OTA_SERVER == TRUE)

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)

  OTA_ImageHeader_t header;

  preamble_t preamble;



  // Read the OTA File Header

  HalOTARead ( 0, ( uint8 * ) &header, sizeof ( OTA_ImageHeader_t ), HAL_OTA_DL );



  if ( header.magicNumber == OTA_HDR_MAGIC_NUMBER )

  {

    zclOTA_DownloadedFileVersion = header.fileId.version;

    zclOTA_DownloadedZigBeeStackVersion = header.stackVersion;

  }

  // Load the OTA Attributes from the constant values in NV

  HalOTARead ( PREAMBLE_OFFSET, ( uint8 * ) &preamble, sizeof ( preamble_t ), HAL_OTA_RC );

  zclOTA_ManufacturerId = preamble.manufacturerId;

  zclOTA_ImageType = preamble.imageType;

  zclOTA_CurrentFileVersion = preamble.imageVersion;

  // Register with the ZDO to receive Match Descriptor Responses
  ZDO_RegisterForZDOMsg ( task_id, Match_Desc_rsp );
  
#ifndef ZCL_STANDALONE
  // Register for all OTA End Point, unhandled, ZCL foundation commands
  zcl_registerForMsgExt( task_id, ZCL_OTA_ENDPOINT );
#endif

  // Per section 6.1 of ZigBee Over-the-Air Upgrading Cluster spec, we should
  // periodically query the server. It does not specify the rate.  For example's
  // sake, here we query the server periodically between 5-10 minutes.
  uint32 queryImgJitter = ( ( uint32 ) osal_rand() % OTA_NEW_IMAGE_QUERY_RATE ) + ( uint32 ) OTA_NEW_IMAGE_QUERY_RATE;
  osal_start_reload_timer ( task_id, ZCL_OTA_QUERY_SERVER_EVT, queryImgJitter );

  // Wake up in 5 seconds and do some service discovery for an OTA Server
  queryImgJitter = ( ( uint32 ) 5000 );
  osal_start_reload_timer ( task_id, ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT, queryImgJitter );
  
  // Initiliaze OTA Update End Request Transaction Seq Number
  zclOta_OtaUpgradeEndReqTransSeq = 0;

#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE) 
}

/******************************************************************************
 * @fn          zclOTA_event_loop
 *
 * @brief       Event Loop Processor for OTA Client task.
 *
 * @param       task_id - TaskId
 *              events - events
 *
 * @return      Unprocessed event bits
 */
uint16 zclOTA_event_loop ( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  if ( events & SYS_EVENT_MSG )
  {
    while ( ( MSGpkt = ( afIncomingMSGPacket_t * ) osal_msg_receive ( task_id ) ) )
    {
      switch ( MSGpkt->hdr.event )
      {
#ifdef ZCL_STANDALONE
        case AF_INCOMING_MSG_CMD:
          zcl_ProcessMessageMSG ( MSGpkt );
          break;
#endif

#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
        case ZDO_CB_MSG:
          zclOTA_ProcessZDOMsgs ( ( zdoIncomingMsg_t * ) MSGpkt );
          break;
#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

#if (defined OTA_SERVER) && (OTA_SERVER == TRUE)
        case MT_SYS_OTA_MSG:
          zclOTA_ServerHandleFileSysCb ( ( OTA_MtMsg_t* ) MSGpkt );
          break;
#endif
        case ZCL_INCOMING_MSG:
          zclOTA_ProcessUnhandledFoundationZCLMsgs ( ( zclIncomingMsg_t* ) MSGpkt );
          break;
          
        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate ( ( uint8 * ) MSGpkt );
    }

    // return unprocessed events
    return ( events ^ SYS_EVENT_MSG );
  }

#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
  if ( events & ZCL_OTA_IMAGE_BLOCK_WAIT_EVT )
  {
    // If the time has expired, perform the required action
    if ( zclOTA_UpdateDelay == 0 )
    {
      zclOTA_ImageBlockWaitExpired();
    }
    else
    {
      // Decrement the number of minutes to wait and reset the timer
      zclOTA_UpdateDelay--;
      osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_IMAGE_BLOCK_WAIT_EVT, 60000 );
    }

    return ( events ^ ZCL_OTA_IMAGE_BLOCK_WAIT_EVT );
  }

  if ( events & ZCL_OTA_UPGRADE_WAIT_EVT )
  {
    // If the time has expired, perform the required action
    if ( zclOTA_UpdateDelay == 0 )
    {
      if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_COUNTDOWN )
      {
        zclOTA_UpgradeComplete ( ZSuccess );
      }
      else if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_UPGRADE_WAIT )
      {
        if ( ++zclOTA_UpgradeEndRetry > OTA_MAX_END_REQ_RETRIES )
        {
          // If we have not heard from the server for N retries, perform the upgrade
          zclOTA_UpgradeComplete ( ZSuccess );
        }
        else
        {
          // Send another update end request
          zclOTA_UpgradeEndReqParams_t  req;

          req.status = ZSuccess;
          osal_memcpy ( &req.fileId, &zclOTA_CurrentDlFileId, sizeof ( zclOTA_FileID_t ) );

          zclOTA_SendUpgradeEndReq ( &zclOTA_serverAddr, &req );

          // Restart the timer for another hour
          zclOTA_StartTimer ( ZCL_OTA_UPGRADE_WAIT_EVT, 3600 );
        }
      }
    }
    else
    {
      // Decrement the number of minutes to wait and reset the timer
      zclOTA_UpdateDelay--;
      osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_UPGRADE_WAIT_EVT, 60000 );
    }

    return ( events ^ ZCL_OTA_UPGRADE_WAIT_EVT );
  }

  if ( events & ZCL_OTA_IMAGE_QUERY_TO_EVT )
  {
    if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_NORMAL )
    {
      // Notify the application task of the timeout waiting for the download to start
      zclOTA_CallbackMsg_t *pMsg;

      pMsg = ( zclOTA_CallbackMsg_t* ) osal_msg_allocate ( sizeof ( zclOTA_CallbackMsg_t ) );

      if ( pMsg )
      {
        pMsg->hdr.event = ZCL_OTA_CALLBACK_IND;
        pMsg->hdr.status = ZFailure;
        pMsg->ota_event = ZCL_OTA_START_CALLBACK;

        osal_msg_send ( zclOTA_AppTask, ( uint8* ) pMsg );
      }
    }

    return ( events ^ ZCL_OTA_IMAGE_QUERY_TO_EVT );
  }

  if ( events & ZCL_OTA_BLOCK_RSP_TO_EVT )
  {
    // We timed out waiting for a Block Response
    if ( ++zclOTA_BlockRetry > OTA_MAX_BLOCK_RETRIES )
    {
      // Send a failed update end request
      zclOTA_UpgradeEndReqParams_t  req;

      req.status = ZOtaAbort;
      osal_memcpy ( &req.fileId, &zclOTA_CurrentDlFileId, sizeof ( zclOTA_FileID_t ) );

      zclOTA_SendUpgradeEndReq ( &zclOTA_serverAddr, &req );

      zclOTA_UpgradeComplete ( ZOtaAbort );
    }
    else
    {
      // Send another block request
      sendImageBlockReq(&zclOTA_serverAddr);
    }
    
    return ( events ^ ZCL_OTA_BLOCK_RSP_TO_EVT );
  }

  if ( events & ZCL_OTA_QUERY_SERVER_EVT )
  {
    if (zclOTA_ImageUpgradeStatus == OTA_STATUS_NORMAL)
    {
      zclOTA_QueryNextImageReqParams_t queryParams;

      queryParams.fieldControl = 0;
      queryParams.fileId.manufacturer = OTA_MANUFACTURER_ID;
      queryParams.fileId.type = OTA_TYPE_ID;
      queryParams.fileId.version = zclOTA_CurrentFileVersion;

      zclOTA_SendQueryNextImageReq ( &zclOTA_serverAddr, &queryParams );

      // Set a timer to wait for the response
      osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_IMAGE_QUERY_TO_EVT, 10000 );
    }
    return ( events ^ ZCL_OTA_QUERY_SERVER_EVT );
  }

  if ( events & ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT )
  {

    sendImageBlockReq ( &zclOTA_serverAddr );

    return ( events ^ ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT );
  }

  if ( events & ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT )
  {
    zAddrType_t dstAddr;

    dstAddr.addrMode = AddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;

    // Look for OTA Server
    zclOta_OtaZDPTransSeq = ZDP_TransID;

    ZDP_MatchDescReq ( &dstAddr, NWK_BROADCAST_SHORTADDR,
#if (defined OTA_HA)
                       ZCL_HA_PROFILE_ID,
#else
                       ZCL_SE_PROFILE_ID,
#endif
                       1, &otaClusters,
                       0, NULL,   // No incoming clusters to discover
                       FALSE );


    osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_SEND_IEEE_ADD_REQ_EVT, 250 );
    
    return ( events ^ ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT );
  }

  if ( events & ZCL_OTA_SEND_IEEE_ADD_REQ_EVT )
  {
    
    // Request the IEEE address of the server to put into the
    // ATTRID_UPGRADE_SERVER_ID attribute
    ZDP_IEEEAddrReq ( zclOTA_serverAddr.addr.shortAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );

    return ( events ^ ZCL_OTA_SEND_IEEE_ADD_REQ_EVT );
  }
#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

  // Discard unknown events
  return 0;
}


/******************************************************************************
 * @fn          zclOTA_getStatus
 *
 * @brief       Retrieves current ZCL OTA Status
 *
 * @param       none
 *
 * @return      ZCL OTA Status
 */
uint8 zclOTA_getStatus ( void )
{
  return zclOTA_ImageUpgradeStatus;
}


/******************************************************************************
 * @fn      zclOTA_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_HdlIncoming ( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

  if ( zcl_ClusterCmd ( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      // Is command for server?
      if ( zcl_ServerCmd ( pInMsg->hdr.fc.direction ) )
      {
#if (defined OTA_SERVER) && (OTA_SERVER == TRUE)
        stat = zclOTA_ServerHdlIncoming ( pInMsg );
#else
        stat = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
#endif // defined (OTA_SERVER) && (OTA_SERVER == TRUE)
      }
      else // Else command is for client
      {
#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
        stat = zclOTA_ClientHdlIncoming ( pInMsg );
#else
        stat = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
      }
    }
    else
    {
      // We don't support any manufacturer specific command.
      stat = ZCL_STATUS_UNSUP_MANU_CLUSTER_COMMAND;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    stat = ZFailure;
  }

  return ( stat );
}

/******************************************************************************
 * @fn      zclOTA_ProcessZCLMsgs
 *
 * @brief   Process unhandled foundation ZCL messages for the OTA End Point.
 *
 * @param   pMsg - a Pointer to the ZCL message
 *
 * @return  none
 */
static void zclOTA_ProcessUnhandledFoundationZCLMsgs ( zclIncomingMsg_t *pMsg )
{
  switch ( pMsg->zclHdr.commandID )
  {
    case ZCL_CMD_DEFAULT_RSP:
      zclOTA_ProcessInDefaultRspCmd( pMsg );
      break;
    default :
      break;
  }

  if ( pMsg->attrCmd )
  {
    osal_mem_free( pMsg->attrCmd );
    pMsg->attrCmd = NULL;
  }
}

/******************************************************************************
 * @fn      zclOTA_ProcessInDefaultRspCmd 
 *
 * @brief   Passed along from application.
 *
 * @param   pInMsg - Pointer to Default Response Command
 *
 * @return  void
 */
static void zclOTA_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // If the OTA server issued a Default Response, most likely something bad
  // happened.
       
  zclDefaultRspCmd_t *defRspCmd = (zclDefaultRspCmd_t*)pInMsg->attrCmd;
    
  switch ( defRspCmd->statusCode )
  {
      
    case ( ZCL_STATUS_ABORT ) :
        
      switch ( zclOTA_ImageUpgradeStatus )
      {
        case ( OTA_STATUS_COMPLETE ) :
          if ( pInMsg->zclHdr.transSeqNum == zclOta_OtaUpgradeEndReqTransSeq )
          {
            // The server has issued an ABORT while we were waiting for the 
            // Upgrade End Response.
            zclOTA_ImageUpgradeStatus = OTA_STATUS_NORMAL;
            zclOta_OtaUpgradeEndReqTransSeq = 0;
          }
          break;
          
        // Handling for reception of the Default Response with status code == 
        // ABORT, in other OTA states, can be added here.
          
        default :
          break;
      }
      break;
      
    // Handling for other Defautl Response status codes and OTA states can 
    // be added here.
    default :
      break;
  }           
}

#if (defined OTA_SERVER) && (OTA_SERVER == TRUE)
/******************************************************************************
 * @fn      zclOTA_SendImageNotify
 *
 * @brief   Send an OTA Image Notify message.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendImageNotify ( afAddrType_t *dstAddr,
                                   zclOTA_ImageNotifyParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_IMAGE_NOTIFY];
  uint8 *pBuf = buf;

  *pBuf++ = pParams->payloadType;
  *pBuf++ = pParams->queryJitter;
  if ( pParams->payloadType >= NOTIFY_PAYLOAD_JITTER_MFG )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  }
  if ( pParams->payloadType >= NOTIFY_PAYLOAD_JITTER_MFG_TYPE )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  }
  if ( pParams->payloadType == NOTIFY_PAYLOAD_JITTER_MFG_TYPE_VERS )
  {
    pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );
  }

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_IMAGE_NOTIFY, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             zclOTA_SeqNo++, ( uint16 ) ( pBuf - buf ), buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_SendQueryNextImageRsp
 *
 * @brief   Send an OTA Query Next Image Response message.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendQueryNextImageRsp ( afAddrType_t *dstAddr,
    zclOTA_QueryImageRspParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP];
  uint8 *pBuf = buf;

  *pBuf++ = pParams->status;
  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = LO_UINT16 ( pParams->fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->fileId.type );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->imageSize );
  }

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_QUERY_NEXT_IMAGE_RSP, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             zclOTA_SeqNo++, ( uint16 ) ( pBuf - buf ), buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_SendImageBlockRsp
 *
 * @brief   Send an OTA Image Block Response mesage.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendImageBlockRsp ( afAddrType_t *dstAddr,
                                     zclOTA_ImageBlockRspParams_t *pParams )
{
  uint8 *buf;
  uint8 *pBuf;
  ZStatus_t status;
  uint8 len;

  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    len = PAYLOAD_MAX_LEN_IMAGE_BLOCK_RSP + pParams->rsp.success.dataSize;
  }
  else if ( pParams->status == ZCL_STATUS_WAIT_FOR_DATA )
  {
    len = PAYLOAD_MIN_LEN_IMAGE_BLOCK_WAIT;
  }
  else
  {
    len = 1;
  }

  buf = osal_mem_alloc ( len );

  if ( buf == NULL )
  {
    return ( ZMemError );
  }

  pBuf = buf;
  *pBuf++ = pParams->status;

  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    *pBuf++ = LO_UINT16 ( pParams->rsp.success.fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->rsp.success.fileId.manufacturer );
    *pBuf++ = LO_UINT16 ( pParams->rsp.success.fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->rsp.success.fileId.type );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->rsp.success.fileId.version );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->rsp.success.fileOffset );
    *pBuf++ = pParams->rsp.success.dataSize;
    osal_memcpy ( pBuf, pParams->rsp.success.pData, pParams->rsp.success.dataSize );
  }
  else if ( pParams->status == ZCL_STATUS_WAIT_FOR_DATA )
  {
    pBuf = osal_buffer_uint32 ( pBuf, pParams->rsp.wait.currentTime );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->rsp.wait.requestTime );
    *pBuf++ = LO_UINT16 ( pParams->rsp.wait.blockReqDelay );
    *pBuf++ = HI_UINT16 ( pParams->rsp.wait.blockReqDelay );
  }

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_IMAGE_BLOCK_RSP, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             zclOTA_SeqNo++, len, buf );

  osal_mem_free ( buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_SendUpgradeEndRsp
 *
 * @brief   Send an OTA Upgrade End Response mesage.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendUpgradeEndRsp ( afAddrType_t *dstAddr,
                                     zclOTA_UpgradeEndRspParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_UPGRADE_END_RSP];
  uint8 *pBuf = buf;

  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->currentTime );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->upgradeTime );

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_UPGRADE_END_RSP, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             zclOTA_SeqNo++, PAYLOAD_MAX_LEN_UPGRADE_END_RSP, buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_SendQuerySpecificFileRsp
 *
 * @brief   Send an OTA Query Specific File Response mesage.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendQuerySpecificFileRsp ( afAddrType_t *dstAddr,
    zclOTA_QueryImageRspParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_RSP];
  uint8 *pBuf = buf;

  *pBuf++ = pParams->status;
  if ( pParams->status == ZCL_STATUS_SUCCESS )
  {
    *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
    *pBuf++ = LO_UINT16 ( pParams->fileId.type );
    *pBuf++ = HI_UINT16 ( pParams->fileId.type );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );
    pBuf = osal_buffer_uint32 ( pBuf, pParams->imageSize );
  }

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_QUERY_SPECIFIC_FILE_RSP, TRUE,
                             ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0,
                             zclOTA_SeqNo++, ( uint16 ) ( pBuf - buf ), buf );

  return status;
}
#endif // defined (OTA_SERVER) && (OTA_SERVER == TRUE)

#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
/******************************************************************************
 * @fn      zclOTA_SendQueryNextImageReq
 *
 * @brief   Send an OTA Query Next Image Request mesage.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendQueryNextImageReq ( afAddrType_t *dstAddr,
    zclOTA_QueryNextImageReqParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_REQ];
  uint8 *pBuf = buf;

  *pBuf++ = pParams->fieldControl;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );
  if ( pParams->fieldControl == 1 )
  {
    *pBuf++ = LO_UINT16 ( pParams->hardwareVersion );
    *pBuf++ = HI_UINT16 ( pParams->hardwareVersion );
  }

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_QUERY_NEXT_IMAGE_REQ, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_SeqNo++, ( uint16 ) ( pBuf - buf ), buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_SendImageBlockReq
 *
 * @brief   Send an OTA Image Block Request mesage.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendImageBlockReq ( afAddrType_t *dstAddr,
                                     zclOTA_ImageBlockReqParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_IMAGE_BLOCK_REQ];
  uint8 *pBuf = buf;

  *pBuf++ = pParams->fieldControl;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->fileOffset );
  *pBuf++ = pParams->maxDataSize;

  if ( ( pParams->fieldControl & OTA_BLOCK_FC_NODES_IEEE_PRESENT ) != 0 )
  {
    osal_cpyExtAddr ( pBuf, pParams->nodeAddr );
    pBuf += Z_EXTADDR_LEN;
  }

  if ( ( pParams->fieldControl & OTA_BLOCK_FC_REQ_DELAY_PRESENT ) != 0 )
  {
    *pBuf++ = LO_UINT16 ( pParams->blockReqDelay );
    *pBuf++ = HI_UINT16 ( pParams->blockReqDelay );
  }

  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_IMAGE_BLOCK_REQ, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOTA_SeqNo++, ( uint16 ) ( pBuf - buf ), buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_SendUpgradeEndReq
 *
 * @brief   Send an OTA Upgrade End Request mesage.
 *
 * @param   dstAddr - where you want the message to go
 * @param   pParams - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_SendUpgradeEndReq ( afAddrType_t *dstAddr,
                                     zclOTA_UpgradeEndReqParams_t *pParams )
{
  ZStatus_t status;
  uint8 buf[PAYLOAD_MAX_LEN_UPGRADE_END_REQ];
  uint8 *pBuf = buf;

  *pBuf++ = pParams->status;
  *pBuf++ = LO_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = HI_UINT16 ( pParams->fileId.manufacturer );
  *pBuf++ = LO_UINT16 ( pParams->fileId.type );
  *pBuf++ = HI_UINT16 ( pParams->fileId.type );
  pBuf = osal_buffer_uint32 ( pBuf, pParams->fileId.version );

  zclOta_OtaUpgradeEndReqTransSeq = zclOTA_SeqNo++;
  
  status = zcl_SendCommand ( ZCL_OTA_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_OTA,
                             COMMAND_UPGRADE_END_REQ, TRUE,
                             ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0,
                             zclOta_OtaUpgradeEndReqTransSeq, ( uint16 ) ( pBuf - buf ), buf );

  return status;
}

/******************************************************************************
 * @fn      zclOTA_StartTimer
 *
 * @brief   Start a ZCL OTA timer.
 *
 * @param   eventId - OSAL event set on timer expiration
 * @param   seconds - timeout in seconds
 *
 * @return  None
 */
static void zclOTA_StartTimer ( uint16 eventId, uint32 seconds )
{
  // Record the number of whole minutes to wait
  zclOTA_UpdateDelay = ( seconds / 60 );

  // Set a timer for the remaining seconds to wait.
  osal_start_timerEx ( zclOTA_TaskID, eventId, ( seconds % 60 ) * 1000 );
}

/******************************************************************************
 * @fn      sendImageBlockReq
 *
 * @brief   Send an Image Block Request.
 *
 * @param   dstAddr - where you want the message to go
 *
 * @return  ZStatus_t
 */
static ZStatus_t sendImageBlockReq ( afAddrType_t *dstAddr )
{
  zclOTA_ImageBlockReqParams_t req;

  req.fieldControl = zclOTA_ImageBlockFC; // Image block command field control value
  req.fileId.manufacturer = zclOTA_ManufacturerId;
  req.fileId.type = zclOTA_ImageType;
  req.fileId.version = zclOTA_DownloadedFileVersion;
  req.fileOffset = zclOTA_FileOffset;

  if ( zclOTA_DownloadedImageSize - zclOTA_FileOffset < OTA_MAX_MTU )
  {
    req.maxDataSize = zclOTA_DownloadedImageSize - zclOTA_FileOffset;
  }
  else
  {
    req.maxDataSize = OTA_MAX_MTU;
  }

  req.blockReqDelay = zclOTA_MinBlockReqDelay;

  // Start a timer waiting for a response
  osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_BLOCK_RSP_TO_EVT, OTA_MAX_BLOCK_RSP_WAIT_TIME );

  return zclOTA_SendImageBlockReq ( dstAddr, &req );
}

/******************************************************************************
 * @fn      zclOTA_ProcessImageData
 *
 * @brief   Process image data as it is received from the host.
 *
 * @param   pData - pointer to the data
 * @param   len - length of the data
 *
 * @return  status of the operation
 */
uint8 zclOTA_ProcessImageData ( uint8 *pData, uint8 len )
{
  int8 i;
#if defined OTA_MMO_SIGN
  uint8 skipHash = FALSE;
#endif

  if ( zclOTA_ImageUpgradeStatus != OTA_STATUS_IN_PROGRESS )
  {
    return ZCL_STATUS_ABORT;
  }

#if (defined HAL_LED) && (HAL_LED == TRUE)
  // Toggle an LED to indicate we received a new block
  HalLedSet ( HAL_LED_2, HAL_LED_MODE_TOGGLE );
#endif

  // write data to secondary storage
  HalOTAWrite ( zclOTA_FileOffset, pData, len, HAL_OTA_DL );

  for ( i=0; i<len; i++ )
  {
    switch ( zclOTA_ClientPdState )
    {
        // verify header magic number
      case ZCL_OTA_PD_MAGIC_0_STATE:
        // Initialize control variables
#if defined OTA_MMO_SIGN
        osal_memset ( &zclOTA_MmoHash, 0, sizeof ( zclOTA_MmoHash ) );
        zclOTA_HashPos = 0;
#endif

        // Missing break intended
      case ZCL_OTA_PD_MAGIC_1_STATE:
      case ZCL_OTA_PD_MAGIC_2_STATE:
      case ZCL_OTA_PD_MAGIC_3_STATE:
        if ( pData[i] != zclOTA_HdrMagic[zclOTA_ClientPdState] )
        {
          return ZCL_STATUS_INVALID_IMAGE;
        }
        zclOTA_ClientPdState++;
        break;

      case ZCL_OTA_PD_HDR_LEN1_STATE:
        // get header length
        if ( zclOTA_FileOffset == ZCL_OTA_HDR_LEN_OFFSET )
        {
          zclOTA_HeaderLen = pData[i];
          zclOTA_ClientPdState = ZCL_OTA_PD_HDR_LEN2_STATE;
        }
        break;

      case ZCL_OTA_PD_HDR_LEN2_STATE:
        zclOTA_HeaderLen |= ( ( ( uint16 ) pData[i] ) << 8 ) & 0xFF00;
        zclOTA_ClientPdState = ZCL_OTA_PD_STK_VER1_STATE;
        break;

      case ZCL_OTA_PD_STK_VER1_STATE:
        // get stack version
        if ( zclOTA_FileOffset == ZCL_OTA_STK_VER_OFFSET )
        {
          zclOTA_DownloadedZigBeeStackVersion = pData[i];
          zclOTA_ClientPdState = ZCL_OTA_PD_STK_VER2_STATE;
        }
        break;

      case ZCL_OTA_PD_STK_VER2_STATE:
        zclOTA_DownloadedZigBeeStackVersion |= ( ( ( uint16 ) pData[i] ) << 8 ) & 0xFF00;
        zclOTA_ClientPdState = ZCL_OTA_PD_CONT_HDR_STATE;

        if ( zclOTA_DownloadedZigBeeStackVersion != OTA_HDR_STACK_VERSION )
        {
          return ZCL_STATUS_INVALID_IMAGE;
        }
        break;

      case ZCL_OTA_PD_CONT_HDR_STATE:
        // Complete the header
        if ( zclOTA_FileOffset == zclOTA_HeaderLen-1 )
        {
          zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_TAG1_STATE;
        }
        break;

      case ZCL_OTA_PD_ELEM_TAG1_STATE:
        zclOTA_ElementTag = pData[i];
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_TAG2_STATE;
        break;

      case ZCL_OTA_PD_ELEM_TAG2_STATE:
        zclOTA_ElementTag |= ( ( ( uint16 ) pData[i] ) << 8 ) & 0xFF00;
        zclOTA_ElementPos = 0;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN1_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN1_STATE:
        zclOTA_ElementLen = pData[i];
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN2_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN2_STATE:
        zclOTA_ElementLen |= ( ( uint32 ) pData[i] << 8 ) & 0x0000FF00;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN3_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN3_STATE:
        zclOTA_ElementLen |= ( ( uint32 ) pData[i] << 16 ) & 0x00FF0000;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_LEN4_STATE;
        break;

      case ZCL_OTA_PD_ELEM_LEN4_STATE:
        zclOTA_ElementLen |= ( ( uint32 ) pData[i] << 24 ) & 0xFF000000;
        zclOTA_ClientPdState = ZCL_OTA_PD_ELEMENT_STATE;

        // Make sure the length of the element isn't bigger than the image
        if ( zclOTA_ElementLen > ( zclOTA_DownloadedImageSize - zclOTA_FileOffset ) )
        {
          return ZCL_STATUS_INVALID_IMAGE;
        }

#if defined OTA_MMO_SIGN
        if ( zclOTA_ElementTag == OTA_ECDSA_SIGNATURE_TAG_ID )
        {
          if ( zclOTA_ElementLen != OTA_SIGNATURE_LEN + Z_EXTADDR_LEN )
          {
            return ZCL_STATUS_INVALID_IMAGE;
          }
        }
        else if ( zclOTA_ElementTag == OTA_ECDSA_CERT_TAG_ID )
        {
          if ( zclOTA_ElementLen != OTA_CERTIFICATE_LEN )
          {
            return ZCL_STATUS_INVALID_IMAGE;
          }
        }
#endif
        break;

      case ZCL_OTA_PD_ELEMENT_STATE:
#if defined OTA_MMO_SIGN
        if ( zclOTA_ElementTag == OTA_ECDSA_SIGNATURE_TAG_ID )
        {
          if ( zclOTA_ElementPos < Z_EXTADDR_LEN )
            zclOTA_SignerIEEE[zclOTA_ElementPos] = pData[i];
          else
          {
            zclOTA_SignatureData[zclOTA_ElementPos - Z_EXTADDR_LEN] = pData[i];

            skipHash = TRUE;
          }
        }
        else if ( zclOTA_ElementTag == OTA_ECDSA_CERT_TAG_ID )
        {
          zclOTA_Certificate[zclOTA_ElementPos] = pData[i];
        }
#endif

        if ( ++zclOTA_ElementPos == zclOTA_ElementLen )
        {
          // Element is complete
          if ( zclOTA_ElementTag == OTA_UPGRADE_IMAGE_TAG_ID )
          {
            // The serial flash can take up to 25 ms before it is ready for a read
            uint32 k;
            for ( k=0; k<0xffff; k++ )
            {
              asm ( "NOP" );
            }

            // When the image is complete, verify CRC
            if ( HalOTAChkDL ( HAL_OTA_CRC_OSET ) != SUCCESS )
            {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
              HalLcdWriteString ( "OTA CRC Fail", HAL_LCD_LINE_3 );
#endif
              return ZCL_STATUS_INVALID_IMAGE;
            }
          }

          zclOTA_ClientPdState = ZCL_OTA_PD_ELEM_TAG1_STATE;
        }
        break;

      default:
        break;
    }

#if defined OTA_MMO_SIGN
    if ( !skipHash )
    {
      // Maintain a buffer of data to hash
      zclOTA_DataToHash[zclOTA_HashPos++] = pData[i];

      // When the buffer reaches OTA_MMO_HASH_SIZE, update the Hash
      if ( zclOTA_HashPos == OTA_MMO_HASH_SIZE )
      {
        OTA_CalculateMmoR3 ( &zclOTA_MmoHash, zclOTA_DataToHash, OTA_MMO_HASH_SIZE, FALSE );
        zclOTA_HashPos = 0;
      }
    }
#endif

    // Check if the download is complete
    if ( ++zclOTA_FileOffset >= zclOTA_DownloadedImageSize )
    {
      zclOTA_ImageUpgradeStatus = OTA_STATUS_COMPLETE;

#if defined OTA_MMO_SIGN
      // Complete the hash calcualtion
      OTA_CalculateMmoR3 ( &zclOTA_MmoHash, zclOTA_DataToHash, zclOTA_HashPos, TRUE );

      // Validate the hash
      if ( OTA_ValidateSignature ( zclOTA_MmoHash.hash, zclOTA_Certificate,
                                   zclOTA_SignatureData, zclOTA_SignerIEEE ) != ZSuccess )
        return ZCL_STATUS_INVALID_IMAGE;
#endif

      return ZSuccess;
    }
  }

  return ZSuccess;
}

/******************************************************************************
 * @fn      zclOTA_ProcessImageNotify
 *
 * @brief   Process received Image Notify command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessImageNotify ( zclIncoming_t *pInMsg )
{
  zclOTA_ImageNotifyParams_t  param;
  zclOTA_QueryNextImageReqParams_t req;
  uint8 *pData;

  // verify message length
  if ( ( pInMsg->pDataLen > PAYLOAD_MAX_LEN_IMAGE_NOTIFY ) ||
       ( pInMsg->pDataLen < PAYLOAD_MIN_LEN_IMAGE_NOTIFY ) )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // verify  in 'normal' state
  if ( ( zclOTA_Permit == FALSE ) ||
       ( zclOTA_ImageUpgradeStatus != OTA_STATUS_NORMAL ) )
  {
    return ZFailure;
  }

  // parse message
  pData = pInMsg->pData;
  param.payloadType = *pData++;
  param.queryJitter = *pData++;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = osal_build_uint32 ( pData, 4 );

  // if message is broadcast
  if ( pInMsg->msg->wasBroadcast )
  {
    // verify manufacturer
    if ( ( param.payloadType >= NOTIFY_PAYLOAD_JITTER_MFG ) &&
         ( param.fileId.manufacturer != zclOTA_ManufacturerId ) )
    {
      return ZSuccess;
    }

    // verify image type
    if ( ( param.payloadType >= NOTIFY_PAYLOAD_JITTER_MFG_TYPE ) &&
         ( param.fileId.type != zclOTA_ImageType ) )
    {
      return ZSuccess;
    }

    // verify version; if version matches ignore
    if ( ( param.payloadType >= NOTIFY_PAYLOAD_JITTER_MFG_TYPE_VERS ) &&
         ( param.fileId.version == zclOTA_CurrentFileVersion ) )
    {
      return ZSuccess;
    }

    // get random value and compare to query jitter
    if ( ( ( uint8 ) osal_rand() % 100 ) > param.queryJitter )
    {
      // if greater than query jitter ignore;
      return ZSuccess;
    }
  }

  // if unicast message, or broadcast and still made it here, send query next image
  req.fieldControl = 0;
  req.fileId.manufacturer = zclOTA_ManufacturerId;
  req.fileId.type = zclOTA_ImageType;
  req.fileId.version = zclOTA_CurrentFileVersion;
  zclOTA_SendQueryNextImageReq ( & ( pInMsg->msg->srcAddr ), &req );

  return ZSuccess;
}

/******************************************************************************
 * @fn      zclOTA_ProcessQueryNextImageRsp
 *
 * @brief   Process received Query Next Image Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessQueryNextImageRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_QueryImageRspParams_t  param;
  uint8 *pData;
  uint8 status = ZFailure;

  // verify message length
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_RSP ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_QUERY_NEXT_IMAGE_RSP ) )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // ignore message if in 'download in progress' state
  if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS )
  {
    return ZSuccess;
  }

  // get status
  pData = pInMsg->pData;
  param.status = *pData++;

  // if status is success
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    // parse message
    param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.version = osal_build_uint32 ( pData, 4 );
    pData += 4;
    param.imageSize = osal_build_uint32 ( pData, 4 );

    // verify manufacturer id and image type
    if ( ( param.fileId.type == zclOTA_ImageType ) &&
         ( param.fileId.manufacturer == zclOTA_ManufacturerId ) )
    {
      // store file version and image size
      zclOTA_DownloadedFileVersion = param.fileId.version;
      zclOTA_DownloadedImageSize = param.imageSize;

      // initialize other variables
      zclOTA_FileOffset = 0;
      zclOTA_ClientPdState = ZCL_OTA_PD_MAGIC_0_STATE;

      // set state to 'in progress'
      zclOTA_ImageUpgradeStatus = OTA_STATUS_IN_PROGRESS;

      // store server address
      zclOTA_serverAddr = pInMsg->msg->srcAddr;

      // Store the file ID
      osal_memcpy ( &zclOTA_CurrentDlFileId, &param.fileId, sizeof ( zclOTA_FileID_t ) );

      // send image block request
      osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT, zclOTA_MinBlockReqDelay );
      status = ZCL_STATUS_CMD_HAS_RSP;

      osal_stop_timerEx ( zclOTA_TaskID, ZCL_OTA_IMAGE_QUERY_TO_EVT );
    }
  }

  if ( zclOTA_AppTask != 0xFF )
  {
    // Notify the application task of the failure
    zclOTA_CallbackMsg_t *pMsg;

    pMsg = ( zclOTA_CallbackMsg_t* ) osal_msg_allocate ( sizeof ( zclOTA_CallbackMsg_t ) );

    if ( pMsg )
    {
      pMsg->hdr.event = ZCL_OTA_CALLBACK_IND;
      pMsg->hdr.status = param.status;
      pMsg->ota_event = ZCL_OTA_START_CALLBACK;

      osal_msg_send ( zclOTA_AppTask, ( uint8* ) pMsg );
    }
  }

  return status;
}

/******************************************************************************
 * @fn      zclOTA_ProcessImageBlockRsp
 *
 * @brief   Process received Image Block Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessImageBlockRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_ImageBlockRspParams_t  param;
  zclOTA_UpgradeEndReqParams_t  req;
  uint8 *pData;
  uint8 status = ZSuccess;

  // verify in 'in progress' state
  if ( zclOTA_ImageUpgradeStatus != OTA_STATUS_IN_PROGRESS )
  {
    return ZSuccess;
  }

  // get status
  pData = pInMsg->pData;
  param.status = *pData++;

  // if status is success
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    // verify message length
    if ( pInMsg->pDataLen < PAYLOAD_MAX_LEN_IMAGE_BLOCK_RSP )
    {
      // no further processing if invalid
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // parse message
    param.rsp.success.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.rsp.success.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.rsp.success.fileId.version = osal_build_uint32 ( pData, 4 );
    pData += 4;
    param.rsp.success.fileOffset = osal_build_uint32 ( pData, 4 );
    pData += 4;
    param.rsp.success.dataSize = *pData++;
    param.rsp.success.pData = pData;

    // verify manufacturer, image type, file version, file offset
    if ( ( param.rsp.success.fileId.type != zclOTA_ImageType ) ||
         ( param.rsp.success.fileId.manufacturer != zclOTA_ManufacturerId ) ||
         ( param.rsp.success.fileId.version != zclOTA_DownloadedFileVersion ) )
    {
      status = ZCL_STATUS_INVALID_IMAGE;
    }
    else
    {
      // Drop duplicate packets (retries)
      if ( param.rsp.success.fileOffset != zclOTA_FileOffset )
      {
        return ZSuccess;
      }

      status = zclOTA_ProcessImageData ( param.rsp.success.pData, param.rsp.success.dataSize );

      // Stop the timer and clear the retry count
      zclOTA_BlockRetry = 0;
      osal_stop_timerEx ( zclOTA_TaskID, ZCL_OTA_BLOCK_RSP_TO_EVT );

      if ( status == ZSuccess )
      {
        if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_COMPLETE )
        {
          // send upgrade end req with success status
          osal_memcpy ( &req.fileId, &param.rsp.success.fileId, sizeof ( zclOTA_FileID_t ) );
          req.status = ZSuccess;
          zclOTA_SendUpgradeEndReq ( & ( pInMsg->msg->srcAddr ), &req );
        }
        else
        {
          // send image block request using rate limiting
          osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT, zclOTA_MinBlockReqDelay );
        }
      }
    }
  }
  // else if status is 'wait for data'
  else if ( param.status == ZCL_STATUS_WAIT_FOR_DATA )
  {
    // verify message length
    if ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_IMAGE_BLOCK_WAIT )
    {
      // no further processing if invalid
      return ZCL_STATUS_MALFORMED_COMMAND;
    }

    // parse message
    param.rsp.wait.currentTime = osal_build_uint32 ( pData, 4 );
    pData += 4;
    param.rsp.wait.requestTime = osal_build_uint32 ( pData, 4 );
    pData += 4;
    param.rsp.wait.blockReqDelay = BUILD_UINT16 ( pData[0], pData[1] );

    // check to see if device supports blockReqDelay rate limiting
    if ( ( zclOTA_ImageBlockFC & OTA_BLOCK_FC_REQ_DELAY_PRESENT ) != 0 )
    {
      if ( ( param.rsp.wait.requestTime - param.rsp.wait.currentTime ) > 0 )
      {
        // Stop the timer and clear the retry count
        zclOTA_BlockRetry = 0;
        osal_stop_timerEx ( zclOTA_TaskID, ZCL_OTA_BLOCK_RSP_TO_EVT );

        // set timer for next image block req
        zclOTA_StartTimer ( ZCL_OTA_IMAGE_BLOCK_WAIT_EVT,
                            ( param.rsp.wait.requestTime - param.rsp.wait.currentTime ) );
      }
      else
      {
        // if wait timer delta is 0, then update device with blockReqDelay value and use rate limiting
        zclOTA_MinBlockReqDelay = param.rsp.wait.blockReqDelay;

        osal_start_timerEx ( zclOTA_TaskID, ZCL_OTA_IMAGE_BLOCK_REQ_DELAY_EVT, zclOTA_MinBlockReqDelay );
      }
    }
    else
    {
      // Stop the timer and clear the retry count
      zclOTA_BlockRetry = 0;
      osal_stop_timerEx ( zclOTA_TaskID, ZCL_OTA_BLOCK_RSP_TO_EVT );

      // set timer for next image block req
      zclOTA_StartTimer ( ZCL_OTA_IMAGE_BLOCK_WAIT_EVT,
                          ( param.rsp.wait.requestTime - param.rsp.wait.currentTime ) );
    }
  }
  else if ( param.status == ZCL_STATUS_ABORT )
  {
    // download aborted; set state to 'normal' state
    zclOTA_ImageUpgradeStatus = OTA_STATUS_NORMAL;

    // Stop the timer and clear the retry count
    zclOTA_BlockRetry = 0;
    osal_stop_timerEx ( zclOTA_TaskID, ZCL_OTA_BLOCK_RSP_TO_EVT );
    zclOTA_UpgradeComplete ( ZOtaAbort );

    return ZSuccess;
  }
  else
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  if ( status != ZSuccess )
  {
    // download failed; set state to 'normal'
    zclOTA_ImageUpgradeStatus = OTA_STATUS_NORMAL;

    // send upgrade end req with failure status
    osal_memcpy ( &req.fileId, &param.rsp.success.fileId, sizeof ( zclOTA_FileID_t ) );
    req.status = status;
    zclOTA_SendUpgradeEndReq ( & ( pInMsg->msg->srcAddr ), &req );
  }

  return ZSuccess;
}

/******************************************************************************
 * @fn      zclOTA_ProcessUpgradeEndRsp
 *
 * @brief   Process received Upgrade End Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessUpgradeEndRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_UpgradeEndRspParams_t  param;
  zclOTA_FileID_t currentFileId = {zclOTA_ManufacturerId, zclOTA_ImageType, zclOTA_DownloadedFileVersion};
  uint8 *pData;

  
  // Clear the Upgrade End Request transaction sequence number.  At this stage
  // it's no longer needed.
  zclOta_OtaUpgradeEndReqTransSeq = 0;
  
  // verify message length
  if ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_UPGRADE_END_RSP )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // parse message
  pData = pInMsg->pData;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.currentTime = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.upgradeTime = osal_build_uint32 ( pData, 4 );

  // verify in 'download complete'  or 'waiting for upgrade' state
  if ( ( zclOTA_ImageUpgradeStatus == OTA_STATUS_COMPLETE ) ||
       ( ( zclOTA_ImageUpgradeStatus == OTA_STATUS_UPGRADE_WAIT ) && ( param.upgradeTime!=OTA_UPGRADE_TIME_WAIT ) ) )
  {
    // verify manufacturer, image type
    if ( zclOTA_CmpFileId ( &param.fileId, &currentFileId ) == FALSE )
    {
      return ZSuccess;
    }

    // check upgrade time
    if ( param.upgradeTime != OTA_UPGRADE_TIME_WAIT )
    {
      uint32 notifyDelay = 0;

      if ( param.upgradeTime > param.currentTime )
      {
        // time to wait before notification
        notifyDelay = param.upgradeTime - param.currentTime;
      }

      // set state to 'countdown'
      zclOTA_ImageUpgradeStatus = OTA_STATUS_COUNTDOWN;
      // set timer for upgrade complete notification
      zclOTA_StartTimer ( ZCL_OTA_UPGRADE_WAIT_EVT, notifyDelay );
    }
    else
    {
      // Wait for another upgrade end response
      zclOTA_ImageUpgradeStatus = OTA_STATUS_UPGRADE_WAIT;
      // Set a timer for 60 minutes to send another Upgrade End Rsp
      zclOTA_StartTimer ( ZCL_OTA_UPGRADE_WAIT_EVT, 3600 );
      zclOTA_UpgradeEndRetry = 0;
    }
  }

  return ZSuccess;
}

/******************************************************************************
 * @fn      zclOTA_ProcessQuerySpecificFileRsp
 *
 * @brief   Process received Query Specific File Response.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessQuerySpecificFileRsp ( zclIncoming_t *pInMsg )
{
  zclOTA_QueryImageRspParams_t  param;
  uint8 *pData;

  // verify message length
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_RSP ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_QUERY_SPECIFIC_FILE_RSP ) )
  {
    // no further processing if invalid
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  // ignore message if in 'download in progress' state
  if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS )
  {
    return ZSuccess;
  }

  // get status
  pData = pInMsg->pData;
  param.status = *pData++;

  // if status is success
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    // parse message
    param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.version = osal_build_uint32 ( pData, 4 );
    pData += 4;
    param.imageSize = osal_build_uint32 ( pData, 4 );

    // verify manufacturer id and image type
    if ( ( param.fileId.type == zclOTA_ImageType ) &&
         ( param.fileId.manufacturer == zclOTA_ManufacturerId ) )
    {
      // store file version and image size
      zclOTA_DownloadedFileVersion = param.fileId.version;
      zclOTA_DownloadedImageSize = param.imageSize;

      // initialize other variables
      zclOTA_FileOffset = 0;

      // set state to 'in progress'
      zclOTA_ImageUpgradeStatus = OTA_STATUS_IN_PROGRESS;

      // send image block request
      sendImageBlockReq ( & ( pInMsg->msg->srcAddr ) );
    }
  }

  return ZSuccess;
}

/******************************************************************************
 * @fn      zclOTA_ClientHdlIncoming
 *
 * @brief   Handle incoming client commands.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ClientHdlIncoming ( zclIncoming_t *pInMsg )
{
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_IMAGE_NOTIFY:
      return zclOTA_ProcessImageNotify ( pInMsg );

    case COMMAND_QUERY_NEXT_IMAGE_RSP:
      return zclOTA_ProcessQueryNextImageRsp ( pInMsg );

    case COMMAND_IMAGE_BLOCK_RSP:
      return zclOTA_ProcessImageBlockRsp ( pInMsg );

    case COMMAND_UPGRADE_END_RSP:
      return zclOTA_ProcessUpgradeEndRsp ( pInMsg );

    case COMMAND_QUERY_SPECIFIC_FILE_RSP:
      return zclOTA_ProcessQuerySpecificFileRsp ( pInMsg );

    default:
      return ZFailure;
  }
}

/******************************************************************************
 * @fn      zclOTA_CmpFileId
 *
 * @brief   Called to compare two file IDs
 *
 * @param   f1, f2 - Pointers to the two file IDs to compare
 *
 * @return  TRUE if the file IDs are the same, else FALSE
 */
static uint8 zclOTA_CmpFileId ( zclOTA_FileID_t *f1, zclOTA_FileID_t *f2 )
{
  if ( ( f1->manufacturer == 0xFFFF ) ||
       ( f2->manufacturer == 0xFFFF ) ||
       ( f1->manufacturer == f2->manufacturer ) )
  {
    if ( ( f1->type == 0xFFFF ) ||
         ( f2->type == 0xFFFF ) ||
         ( f1->type == f2->type ) )
    {
      if ( ( f1->version == 0xFFFFFFFF ) ||
           ( f2->version == 0xFFFFFFFF ) ||
           ( f1->version == f2->version ) )
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/******************************************************************************
 * @fn      zclOTA_ImageBlockWaitExpired
 *
 * @brief   Perform action on image block wait timer expiration.
 *
 * @param   none
 *
 * @return  none
 */
static void zclOTA_ImageBlockWaitExpired ( void )
{
  // verify in 'in progress' state
  if ( zclOTA_ImageUpgradeStatus == OTA_STATUS_IN_PROGRESS )
  {
    // request next block
    sendImageBlockReq ( &zclOTA_serverAddr );
  }
}

/******************************************************************************
 * @fn      zclOTA_UpgradeComplete
 *
 * @brief   Notify the application task that an upgrade has completed.
 *
 * @param   status - The status of the upgrade
 *
 * @return  none
 */
static void zclOTA_UpgradeComplete ( uint8 status )
{
  // Go back to the normal state
  zclOTA_ImageUpgradeStatus = OTA_STATUS_NORMAL;

  if ( ( zclOTA_DownloadedImageSize == OTA_HEADER_LEN_MIN_ECDSA ) ||
       ( zclOTA_DownloadedImageSize == OTA_HEADER_LEN_MIN ) )
  {
    status = ZFailure;
  }

  if ( zclOTA_AppTask != 0xFF )
  {
    // Notify the application task the upgrade stopped
    zclOTA_CallbackMsg_t *pMsg;

    pMsg = ( zclOTA_CallbackMsg_t* ) osal_msg_allocate ( sizeof ( zclOTA_CallbackMsg_t ) );

    if ( pMsg )
    {
      pMsg->hdr.event = ZCL_OTA_CALLBACK_IND;
      pMsg->hdr.status = status;
      pMsg->ota_event = ZCL_OTA_DL_COMPLETE_CALLBACK;

      osal_msg_send ( zclOTA_AppTask, ( uint8* ) pMsg );
    }
  }
  else
  {
    // We may have got here due to an aborted download.  Obviously, we should
    // only proceed with the next steps if the downlowd was successful.
    if ( status == ZSuccess )
    {
      // Reset the CRC Shadow and reboot.  The bootloader will see the
      // CRC shadow has been cleared and switch to the new image
      HalOTAInvRC();
      SystemReset();
    }
  }
}

/******************************************************************************
 * @fn      zclOTA_ProcessZDOMsgs
 *
 * @brief   Process callbacks from the ZDO.
 *
 * @param   pMsg - a Pointer to the message from the ZDO
 *
 * @return  none
 */
static void zclOTA_ProcessZDOMsgs ( zdoIncomingMsg_t *pMsg )
{
  switch ( pMsg->clusterID )
  {
    case IEEE_addr_rsp:
    {
      ZDO_NwkIEEEAddrResp_t *pNwkAddrRsp = ZDO_ParseAddrRsp ( pMsg );

      // If this is from the OTA server, record the server's IEEE address
      if ( pNwkAddrRsp != NULL )
      {
        if ( pNwkAddrRsp->nwkAddr == zclOTA_serverAddr.addr.shortAddr )
        {
          osal_memcpy ( &zclOTA_UpgradeServerID, pNwkAddrRsp->extAddr, Z_EXTADDR_LEN );
        }
        osal_mem_free ( pNwkAddrRsp );
      }
      break;
    }
  
    case Match_Desc_rsp:
    {
      if ( pMsg->TransSeq == zclOta_OtaZDPTransSeq )
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp ( pMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            zclOTA_serverAddr.addrMode = ( afAddrMode_t ) Addr16Bit;
            zclOTA_serverAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            zclOTA_serverAddr.endPoint = pRsp->epList[0];
            osal_stop_timerEx ( zclOTA_TaskID, ZCL_OTA_SEND_MATCH_DESCRIPTOR_EVT );
          }
          osal_mem_free ( pRsp );
        }
      }
      break;
    }
  }
}
#endif // (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)

#if defined (OTA_SERVER) && (OTA_SERVER == TRUE)
/******************************************************************************
 * @fn      zclOTA_ProcessNextImgRsp
 *
 * @brief   Handles a response to a MT_OTA_NEXT_IMG_RSP.
 *
 * @param   pMsg - The data from the server.
 *          pFileId - The ID of the OTA File.
 *          pAddr - The source of the message.
 *
 * @return  none
 */
void zclOTA_ProcessNextImgRsp ( uint8* pMsg, zclOTA_FileID_t *pFileId,
                                afAddrType_t *pAddr )
{
  zclOTA_QueryImageRspParams_t queryRsp;
  uint8 options;
  uint8 status;

  // Get the status of the operation
  status = *pMsg++;

  // Get the options
  options = *pMsg++;

  // Copy the file ID
  osal_memcpy ( &queryRsp.fileId, pFileId, sizeof ( zclOTA_FileID_t ) );

  // Set the image size
  if ( status == ZSuccess )
  {
    queryRsp.status = ZSuccess;
    queryRsp.imageSize = BUILD_UINT32 ( pMsg[0], pMsg[1], pMsg[2], pMsg[3] );
  }
  else
  {
    queryRsp.status = ZOtaNoImageAvailable;
    queryRsp.imageSize = 0;
  }

  queryResponse = queryRsp; // save global variable for query image response. Used later in image block request check

  // Send a response to the client
  if ( options & MT_OTA_QUERY_SPECIFIC_OPTION )
  {
    zclOTA_SendQuerySpecificFileRsp ( pAddr, &queryRsp );
  }
  else
  {
    zclOTA_SendQueryNextImageRsp ( pAddr, &queryRsp );
  }
}

/******************************************************************************
 * @fn      zclOTA_ProcessFileReadRsp
 *
 * @brief   Handles a response to a MT_OTA_FILE_READ_RSP.
 *
 * @param   pMsg - The data from the server.
 *          pFileId - The ID of the OTA File.
 *          pAddr - The source of the message.
 *
 * @return  none
 */
void zclOTA_ProcessFileReadRsp ( uint8* pMsg, zclOTA_FileID_t *pFileId,
                                 afAddrType_t *pAddr )
{
  zclOTA_ImageBlockRspParams_t blockRsp;

  // Set the status
  blockRsp.status = *pMsg++;

  // Check the status of the file read
  if ( blockRsp.status == ZSuccess )
  {
    // Fill in the response parameters
    osal_memcpy ( &blockRsp.rsp.success.fileId, pFileId, sizeof ( zclOTA_FileID_t ) );
    blockRsp.rsp.success.fileOffset = BUILD_UINT32 ( pMsg[0], pMsg[1], pMsg[2], pMsg[3] );
    pMsg += 4;
    blockRsp.rsp.success.dataSize = *pMsg++;
    blockRsp.rsp.success.pData = pMsg;
  }
  else
  {
    blockRsp.status = ZOtaAbort;
  }

  // Send the block response to the peer
  zclOTA_SendImageBlockRsp ( pAddr, &blockRsp );
}

/******************************************************************************
 * @fn      OTA_HandleFileSysCb
 *
 * @brief   Handles File Server Callbacks.
 *
 * @param   pMSGpkt - The data from the server.
 *
 * @return  none
 */
void zclOTA_ServerHandleFileSysCb ( OTA_MtMsg_t* pMSGpkt )
{
  zclOTA_FileID_t pFileId;
  afAddrType_t pAddr;
  uint8 *pMsg;

  if ( pMSGpkt != NULL )
  {
    // Get the File ID and AF Address
    pMsg = pMSGpkt->data;
    pMsg = OTA_StreamToFileId ( &pFileId, pMsg );
    pMsg = OTA_StreamToAfAddr ( &pAddr, pMsg );

    switch ( pMSGpkt->cmd )
    {
      case MT_OTA_NEXT_IMG_RSP:
        zclOTA_ProcessNextImgRsp ( pMsg, &pFileId, &pAddr );
        break;

      case MT_OTA_FILE_READ_RSP:
        zclOTA_ProcessFileReadRsp ( pMsg, &pFileId, &pAddr );
        break;

      default:
        break;
    }
  }
}

/******************************************************************************
 * @fn      zclOTA_Srv_QueryNextImageReq
 *
 * @brief   Handle a Query Next Image Request.
 *
 * @param   pSrcAddr - The source of the message
 *          pParam - message parameters
 *
 * @return  ZStatus_t
 *
 * @note    On a query next image, we must request a file listing
 *          from the File Server.  Then open a file if
 */
ZStatus_t zclOTA_Srv_QueryNextImageReq ( afAddrType_t *pSrcAddr, zclOTA_QueryNextImageReqParams_t *pParam )
{
  uint8 options = 0;
  uint8 status;

  if ( zclOTA_Permit )
  {
    if ( pParam->fieldControl )
    {
      options |= MT_OTA_HW_VER_PRESENT_OPTION;
    }

    // Request the next image for this device from the console via the MT File System
    status = MT_OtaGetImage ( pSrcAddr, &pParam->fileId, pParam->hardwareVersion, NULL, options );
  }
  else
  {
    status = ZOtaNoImageAvailable;
  }

  if ( status != ZSuccess )
  {
    zclOTA_QueryImageRspParams_t queryRsp;

    // Fill in the response parameters
    osal_memcpy ( &queryRsp.fileId, &pParam->fileId, sizeof ( zclOTA_FileID_t ) );
    queryRsp.status = ZOtaNoImageAvailable;
    queryRsp.imageSize = 0;

    // Send a failure response to the client
    zclOTA_SendQueryNextImageRsp ( pSrcAddr, &queryRsp );
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/******************************************************************************
 * @fn      zclOTA_Srv_ImageBlockReq
 *
 * @brief   Handle an Image Block Request.
 *
 * @param   pSrcAddr - The source of the message
 *          pParam - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_Srv_ImageBlockReq ( afAddrType_t *pSrcAddr, zclOTA_ImageBlockReqParams_t *pParam )
{
  uint8 status = ZFailure;

  if ( pParam->fileId.version != queryResponse.fileId.version )
  {
    status = ZCL_STATUS_NO_IMAGE_AVAILABLE;
  }
  else
  {

    if ( zclOTA_Permit && ( pParam != NULL ) )
    {
      uint8 len = pParam->maxDataSize;

      if ( len > OTA_MAX_MTU )
      {
        len = OTA_MAX_MTU;
      }

      // The item already exists in NV memory, read it from NV memory
      osal_nv_read ( ZCD_NV_OTA_BLOCK_REQ_DELAY, 0,
                     sizeof ( zclOTA_MinBlockReqDelay ), &zclOTA_MinBlockReqDelay );

      // check if client supports rate limiting feature, and if client rate needs to be set
      if ( ( ( pParam->fieldControl & OTA_BLOCK_FC_REQ_DELAY_PRESENT ) != 0 ) &&
           ( pParam->blockReqDelay != zclOTA_MinBlockReqDelay ) )
      {
        zclOTA_ImageBlockRspParams_t blockRsp;

        // Fill in the response parameters
        blockRsp.status = ZOtaWaitForData;
        osal_memcpy ( &blockRsp.rsp.success.fileId, &pParam->fileId, sizeof ( zclOTA_FileID_t ) );
        blockRsp.rsp.wait.currentTime = 0;
        blockRsp.rsp.wait.requestTime = 0;
        blockRsp.rsp.wait.blockReqDelay = zclOTA_MinBlockReqDelay;

        // Send a wait response with updated rate limit timing
        zclOTA_SendImageBlockRsp ( pSrcAddr, &blockRsp );
      }
      else
      {
        // Read the data from the OTA Console
        status = MT_OtaFileReadReq ( pSrcAddr, &pParam->fileId, len, pParam->fileOffset );

        // Send a wait response to the client
        if ( status != ZSuccess )
        {
          zclOTA_ImageBlockRspParams_t blockRsp;

          // Fill in the response parameters
          blockRsp.status = ZOtaWaitForData;
          osal_memcpy ( &blockRsp.rsp.success.fileId, &pParam->fileId, sizeof ( zclOTA_FileID_t ) );
          blockRsp.rsp.wait.currentTime = 0;
          blockRsp.rsp.wait.requestTime = OTA_SEND_BLOCK_WAIT;
          blockRsp.rsp.wait.blockReqDelay = zclOTA_MinBlockReqDelay;

          // Send the block to the peer
          zclOTA_SendImageBlockRsp ( pSrcAddr, &blockRsp );
        }
      }

      status = ZCL_STATUS_CMD_HAS_RSP;

    }
  }

  return status;
}

/******************************************************************************
 * @fn      zclOTA_Srv_ImagePageReq
 *
 * @brief   Handle an Image Page Request.  Note: Not currently supported.
 *
 * @param   pSrcAddr - The source of the message
 *          pParam - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_Srv_ImagePageReq ( afAddrType_t *pSrcAddr, zclOTA_ImagePageReqParams_t *pParam )
{
  // Send not supported resposne
  return ZUnsupClusterCmd;
}

/******************************************************************************
 * @fn      zclOTA_Srv_UpgradeEndReq
 *
 * @brief   Handle an Upgrade End Request.
 *
 * @param   pSrcAddr - The source of the message
 *          pParam - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_Srv_UpgradeEndReq ( afAddrType_t *pSrcAddr, zclOTA_UpgradeEndReqParams_t *pParam )
{
  uint8 status = ZFailure;
  
  if ( zclOTA_Permit && ( pParam != NULL ) )
  {
    zclOTA_UpgradeEndRspParams_t rspParms;

    if ( pParam->status == ZSuccess )
    {
      osal_memcpy ( &rspParms.fileId, &pParam->fileId, sizeof ( zclOTA_FileID_t ) );
      rspParms.currentTime = osal_GetSystemClock();
      rspParms.upgradeTime = rspParms.currentTime + OTA_UPGRADE_DELAY;

      // Send the response to the peer
      zclOTA_SendUpgradeEndRsp ( pSrcAddr, &rspParms );
    }

    // Notify the Console Tool
    MT_OtaSendStatus ( pSrcAddr->addr.shortAddr, MT_OTA_DL_COMPLETE, pParam->status, 0 );

    status = ZCL_STATUS_CMD_HAS_RSP;
  }

  return status;
}

/******************************************************************************
 * @fn      zclOTA_Srv_QuerySpecificFileReq
 *
 * @brief   Handles a Query Specific File Request.
 *
 * @param   pSrcAddr - The source of the message
 *          pParam - message parameters
 *
 * @return  ZStatus_t
 */
ZStatus_t zclOTA_Srv_QuerySpecificFileReq ( afAddrType_t *pSrcAddr, zclOTA_QuerySpecificFileReqParams_t *pParam )
{
  uint8 status;

  // Request the image from the console
  if ( zclOTA_Permit )
  {
    status = MT_OtaGetImage ( pSrcAddr, &pParam->fileId, 0,  pParam->nodeAddr, MT_OTA_QUERY_SPECIFIC_OPTION );
  }
  else
  {
    status = ZOtaNoImageAvailable;
  }

  if ( status != ZSuccess )
  {
    zclOTA_QueryImageRspParams_t queryRsp;

    // Fill in the response parameters
    osal_memcpy ( &queryRsp.fileId, &pParam->fileId, sizeof ( zclOTA_FileID_t ) );
    queryRsp.status = ZOtaNoImageAvailable;
    queryRsp.imageSize = 0;

    // Send a failure response to the client
    zclOTA_SendQuerySpecificFileRsp ( pSrcAddr, &queryRsp );
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/******************************************************************************
 * @fn      zclOTA_ProcessQueryNextImageReq
 *
 * @brief   Process received Query Next Image Request.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessQueryNextImageReq ( zclIncoming_t *pInMsg )
{
  zclOTA_QueryNextImageReqParams_t  param;
  uint8 *pData;

  /* verify message length */
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_QUERY_NEXT_IMAGE_REQ ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_QUERY_NEXT_IMAGE_REQ ) )
  {
    /* no further processing if invalid */
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  /* parse message parameters */
  pData = pInMsg->pData;
  param.fieldControl = *pData++;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = osal_build_uint32 ( pData, 4 );
  pData += 4;
  if ( ( param.fieldControl & 0x01 ) != 0 )
  {
    param.hardwareVersion = BUILD_UINT16 ( pData[0], pData[1] );
  }

  /* call callback */
  return zclOTA_Srv_QueryNextImageReq ( &pInMsg->msg->srcAddr, &param );
}

/******************************************************************************
 * @fn      zclOTA_ProcessImageBlockReq
 *
 * @brief   Process received Image Block Request.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessImageBlockReq ( zclIncoming_t *pInMsg )
{
  zclOTA_ImageBlockReqParams_t  param;
  uint8 *pData;

  /* verify message length */
  if ( ( pInMsg->pDataLen > PAYLOAD_MAX_LEN_IMAGE_BLOCK_REQ ) &&
       ( pInMsg->pDataLen < PAYLOAD_MIN_LEN_IMAGE_BLOCK_REQ ) )
  {
    /* no further processing if invalid */
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  /* parse message parameters */
  pData = pInMsg->pData;
  param.fieldControl = *pData++;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.fileOffset = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.maxDataSize = *pData++;
  if ( ( param.fieldControl & OTA_BLOCK_FC_NODES_IEEE_PRESENT ) != 0 )
  {
    osal_cpyExtAddr ( param.nodeAddr, pData );
    pData += 8;
  }
  if ( ( param.fieldControl & OTA_BLOCK_FC_REQ_DELAY_PRESENT ) != 0 )
  {
    param.blockReqDelay = BUILD_UINT16 ( pData[0], pData[1] );
  }

  /* call callback */
  return zclOTA_Srv_ImageBlockReq ( &pInMsg->msg->srcAddr, &param );
}

/******************************************************************************
 * @fn      zclOTA_ProcessImagePageReq
 *
 * @brief   Process received Image Page Request.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessImagePageReq ( zclIncoming_t *pInMsg )
{
  zclOTA_ImagePageReqParams_t  param;
  uint8 *pData;

  /* verify message length */
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_IMAGE_PAGE_REQ ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_IMAGE_PAGE_REQ ) )
  {
    /* no further processing if invalid */
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  /* parse message parameters */
  pData = pInMsg->pData;
  param.fieldControl = *pData++;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.fileOffset = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.maxDataSize = *pData++;
  param.pageSize = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.responseSpacing = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  if ( ( param.fieldControl & 0x01 ) != 0 )
  {
    osal_cpyExtAddr ( param.nodeAddr, pData );
  }

  /* call callback */
  return zclOTA_Srv_ImagePageReq ( &pInMsg->msg->srcAddr, &param );
}

/******************************************************************************
 * @fn      zclOTA_ProcessUpgradeEndReq
 *
 * @brief   Process received Upgrade End Request.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessUpgradeEndReq ( zclIncoming_t *pInMsg )
{
  zclOTA_UpgradeEndReqParams_t  param;
  uint8 *pData;

  /* verify message length */
  if ( ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_UPGRADE_END_REQ ) &&
       ( pInMsg->pDataLen != PAYLOAD_MIN_LEN_UPGRADE_END_REQ ) )
  {
    /* no further processing if invalid */
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  /* parse message parameters */
  pData = pInMsg->pData;
  param.status = *pData++;
  if ( param.status == ZCL_STATUS_SUCCESS )
  {
    param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
    pData += 2;
    param.fileId.version = osal_build_uint32 ( pData, 4 );
  }

  /* call callback */
  return zclOTA_Srv_UpgradeEndReq ( &pInMsg->msg->srcAddr, &param );
}

/******************************************************************************
 * @fn      zclOTA_ProcessQuerySpecificFileReq
 *
 * @brief   Process received Image Page Request.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ProcessQuerySpecificFileReq ( zclIncoming_t *pInMsg )
{
  zclOTA_QuerySpecificFileReqParams_t  param;
  uint8 *pData;

  /* verify message length */
  if ( pInMsg->pDataLen != PAYLOAD_MAX_LEN_QUERY_SPECIFIC_FILE_REQ )
  {
    /* no further processing if invalid */
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  /* parse message parameters */
  pData = pInMsg->pData;
  osal_cpyExtAddr ( param.nodeAddr, pData );
  pData += Z_EXTADDR_LEN;
  param.fileId.manufacturer = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.type = BUILD_UINT16 ( pData[0], pData[1] );
  pData += 2;
  param.fileId.version = osal_build_uint32 ( pData, 4 );
  pData += 4;
  param.stackVersion = BUILD_UINT16 ( pData[0], pData[1] );

  /* call callback */
  return zclOTA_Srv_QuerySpecificFileReq ( &pInMsg->msg->srcAddr, &param );
}

/******************************************************************************
 * @fn      zclOTA_ServerHdlIncoming
 *
 * @brief   Handle incoming server commands.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclOTA_ServerHdlIncoming ( zclIncoming_t *pInMsg )
{
  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_QUERY_NEXT_IMAGE_REQ:
      return zclOTA_ProcessQueryNextImageReq ( pInMsg );

    case COMMAND_IMAGE_BLOCK_REQ:
      return zclOTA_ProcessImageBlockReq ( pInMsg );

    case COMMAND_IMAGE_PAGE_REQ:
      return zclOTA_ProcessImagePageReq ( pInMsg );

    case COMMAND_UPGRADE_END_REQ:
      return zclOTA_ProcessUpgradeEndReq ( pInMsg );

    case COMMAND_QUERY_SPECIFIC_FILE_REQ:
      return zclOTA_ProcessQuerySpecificFileReq ( pInMsg );

    default:
      return ZFailure;
  }
}

/*********************************************************************
 * @fn          zclOTA_InitBlockReqDelay
 *
 * @brief       Initialization attribute Minimum Block Request Delay.
 *
 * @param       none
 *
 * @return      none
 */
static void zclOTA_InitBlockReqDelay ( void )
{
  // If the item doesn't exist in NV memory, create and initialize
  // it with the value passed in.
  if ( osal_nv_item_init ( ZCD_NV_OTA_BLOCK_REQ_DELAY,
                           sizeof ( zclOTA_MinBlockReqDelay ),
                           &zclOTA_MinBlockReqDelay ) == ZSuccess )
  {
    // The item already exists in NV memory, read it from NV memory
    osal_nv_read ( ZCD_NV_OTA_BLOCK_REQ_DELAY, 0,
                   sizeof ( zclOTA_MinBlockReqDelay ), &zclOTA_MinBlockReqDelay );
  }
}
#endif // defined (OTA_SERVER) && (OTA_SERVER == TRUE)


