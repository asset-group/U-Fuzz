/**************************************************************************************************
  Filename:       zcl_ss.c
  Revised:        $Date: 2014-05-07 10:36:12 -0700 (Wed, 07 May 2014) $
  Revision:       $Revision: 38449 $

  Description:    Zigbee Cluster Library - Security and Safety ( SS )


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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

/*******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ss.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

/*******************************************************************************
 * MACROS
 */
#define zclSS_ZoneTypeSupported( a ) ( (a) == SS_IAS_ZONE_TYPE_STANDARD_CIE              || \
                                       (a) == SS_IAS_ZONE_TYPE_MOTION_SENSOR             || \
                                       (a) == SS_IAS_ZONE_TYPE_CONTACT_SWITCH            || \
                                       (a) == SS_IAS_ZONE_TYPE_FIRE_SENSOR               || \
                                       (a) == SS_IAS_ZONE_TYPE_WATER_SENSOR              || \
                                       (a) == SS_IAS_ZONE_TYPE_GAS_SENSOR                || \
                                       (a) == SS_IAS_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE || \
                                       (a) == SS_IAS_ZONE_TYPE_VIBRATION_MOVEMENT_SENSOR || \
                                       (a) == SS_IAS_ZONE_TYPE_REMOTE_CONTROL            || \
                                       (a) == SS_IAS_ZONE_TYPE_KEY_FOB                   || \
                                       (a) == SS_IAS_ZONE_TYPE_KEYPAD                    || \
                                       (a) == SS_IAS_ZONE_TYPE_STANDARD_WARNING_DEVICE   || \
                                       (a) == SS_IAS_ZONE_TYPE_GLASS_BREAK_SENSOR        || \
                                       (a) == SS_IAS_ZONE_TYPE_SECURITY_REPEATER         )


/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct zclSSCBRec
{
  struct zclSSCBRec       *next;
  uint8                   endpoint; // Used to link it into the endpoint descriptor
  zclSS_AppCallbacks_t    *CBs;     // Pointer to Callback function
} zclSSCBRec_t;

typedef struct zclSS_ZoneItem
{
  struct zclSS_ZoneItem   *next;
  uint8                   endpoint; // Used to link it into the endpoint descriptor
  IAS_ACE_ZoneTable_t     zone;     // Zone info
} zclSS_ZoneItem_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 */
const uint8 zclSS_UknownIeeeAddress[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */
static zclSSCBRec_t *zclSSCBs = (zclSSCBRec_t *)NULL;
static uint8 zclSSPluginRegisted = FALSE;

#if defined(ZCL_ZONE) || defined(ZCL_ACE)
static zclSS_ZoneItem_t *zclSS_ZoneTable = (zclSS_ZoneItem_t *)NULL;
#endif // ZCL_ZONE || ZCL_ACE

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclSS_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclSS_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclSS_AppCallbacks_t *zclSS_FindCallbacks( uint8 endpoint );

#ifdef ZCL_ZONE
static ZStatus_t zclSS_ProcessInZoneStatusCmdsServer( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInZoneStatusCmdsClient( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );

static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_ChangeNotification( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_EnrollRequest( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_EnrollResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_InitNormalOperationMode( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_InitTestMode( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
#endif // ZCL_ZONE

#ifdef ZCL_ACE
static ZStatus_t zclSS_ProcessInACECmdsServer( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInACECmdsClient( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );

static ZStatus_t zclSS_ProcessInCmd_ACE_Arm( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_Bypass( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_Emergency( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_Fire( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_Panic( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneIDMap( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneInformation( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetPanelStatus( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetBypassedZoneList( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneStatus( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_ArmResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneIDMapResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneInformationResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_ZoneStatusChanged( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_PanelStatusChanged( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetPanelStatusResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_SetBypassedZoneList( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_BypassResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneStatusResponse( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
#endif // ZCL_ACE

#ifdef ZCL_WD
static ZStatus_t zclSS_ProcessInWDCmds( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );

static ZStatus_t zclSS_ProcessInCmd_WD_StartWarning( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
static ZStatus_t zclSS_ProcessInCmd_WD_Squawk( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs );
#endif // ZCL_WD

#ifdef ZCL_ZONE
static uint8 zclSS_GetNextFreeZoneID( void );
static ZStatus_t zclSS_AddZone( uint8 endpoint, IAS_ACE_ZoneTable_t *zone );
static uint8 zclSS_CountAllZones( void );
static uint8 zclSS_ZoneIDAvailable( uint8 zoneID );
#endif // ZCL_ZONE

#ifdef ZCL_ACE
static uint8 zclSS_Parse_UTF8String( uint8 *pBuf, UTF8String_t *pString, uint8 maxLen );
#endif  // ZCL_ACE

/******************************************************************************
 * @fn      zclSS_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclSS_RegisterCmdCallbacks( uint8 endpoint, zclSS_AppCallbacks_t *callbacks )
{
  zclSSCBRec_t *pNewItem;
  zclSSCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclSSPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_SS_IAS_ZONE,
                        ZCL_CLUSTER_ID_SS_IAS_WD,
                        zclSS_HdlIncoming );
    zclSSPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclSSCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclSSCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclSSCBs == NULL )
  {
    zclSSCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclSSCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}

#ifdef ZCL_ZONE
/*******************************************************************************
 * @fn      zclSS_Send_IAS_ZoneStatusChangeNotificationCmd
 *
 * @brief   Call to send out a Change Notification Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneStatus - current zone status - bit map
 * @param   extendedStatus - bit map, currently set to All zeros ( reserved)
 * @param   zoneID - allocated zone ID
 * @param   delay - delay from change in ZoneStatus attr to transmission of change notification cmd
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_IAS_Send_ZoneStatusChangeNotificationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint16 zoneStatus, uint8 extendedStatus,
                                                          uint8 zoneID, uint16 delay,
                                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_ZONE_STATUS_CHANGE_NOTIFICATION];

  buf[0] = LO_UINT16( zoneStatus );
  buf[1] = HI_UINT16( zoneStatus );
  buf[2] = extendedStatus;
  buf[3] = zoneID;
  buf[4] = LO_UINT16( delay );
  buf[5] = HI_UINT16( delay );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ZONE,
                          COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_ZONE_STATUS_CHANGE_NOTIFICATION, buf );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ZoneStatusEnrollRequestCmd
 *
 * @brief   Call to send out a Enroll Request Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneType - 	  current value of Zone Type attribute
 * @param   manufacturerCode - manuf. code from node descriptor for the device
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollRequestCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                     uint16 zoneType, uint16 manufacturerCode,
                                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_ZONE_ENROLL_REQUEST];

  buf[0] = LO_UINT16( zoneType );
  buf[1] = HI_UINT16( zoneType );
  buf[2] = LO_UINT16( manufacturerCode );
  buf[3] = HI_UINT16( manufacturerCode );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ZONE,
                          COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_ZONE_ENROLL_REQUEST, buf );
}

/*******************************************************************************
 * @fn      zclSS_IAS_Send_ZoneStatusEnrollResponseCmd
 *
 * @brief   Call to send out a Enroll Response Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   responseCode -  value of  response Code
 * @param   zoneID  - index to the zone table of the CIE
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint8 responseCode, uint8 zoneID,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP];

  buf[0] = responseCode;
  buf[1] = zoneID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ZONE,
                          COMMAND_SS_IAS_ZONE_STATUS_ENROLL_RESPONSE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP, buf );
}

/*******************************************************************************
 * @fn      zclSS_IAS_Send_ZoneStatusInitTestModeCmd
 *
 * @brief   Call to send out a Initiate Test Mode Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd -  pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_IAS_Send_ZoneStatusInitTestModeCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    zclZoneInitTestMode_t *pCmd,
                                                    uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_ZONE_STATUS_INIT_TEST_MODE];

  buf[0] = pCmd->testModeDuration;
  buf[1] = pCmd->currZoneSensitivityLevel;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ZONE,
                          COMMAND_SS_IAS_ZONE_STATUS_INIT_TEST_MODE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_ZONE_STATUS_INIT_TEST_MODE, buf );
}

#endif // ZCL_ZONE

#ifdef ZCL_ACE
/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_ArmCmd
 *
 * @brief   Call to send out a Arm  Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_ArmCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                     zclACEArm_t *pCmd,
                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pBuf;
  uint8 *pOutBuf;
  uint8 len = sizeof( zclACEArm_t ) -  sizeof( UTF8String_t ) +
              ( pCmd->armDisarmCode.strLen + 1 );
  ZStatus_t stat;

  pBuf = zcl_mem_alloc( len );
  if ( pBuf )
  {
    pOutBuf = pBuf;

    *pOutBuf++ = pCmd->armMode;

    *pOutBuf++ = pCmd->armDisarmCode.strLen;

    if ( pCmd->armDisarmCode.strLen )
    {
      pOutBuf = zcl_memcpy( pOutBuf, pCmd->armDisarmCode.pStr, pCmd->armDisarmCode.strLen );
    }

    *pOutBuf++ = pCmd->zoneID;

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_ARM, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, len, pBuf );

    zcl_mem_free( pBuf );
  }
  else
  {
    stat = ZMemError;
  }

  return( stat );
}

/*********************************************************************
 * @fn      zclSS_Send_IAS_ACE_BypassCmd
 *
 * @brief   Call to send out a Bypass Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_BypassCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                        zclACEBypass_t *pCmd,
                                        uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 *pBuf;
             // NumberOfZones + ListOfZones + ArMDisarm code UTF8 string
  uint8 len = 1 + pCmd->numberOfZones + ( pCmd->armDisarmCode.strLen + 1 );
  ZStatus_t stat;

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;

    *pBuf++ = pCmd->numberOfZones;

    pBuf = zcl_memcpy( pBuf, pCmd->bypassBuf, pCmd->numberOfZones );

    *pBuf++ = pCmd->armDisarmCode.strLen;

    if ( pCmd->armDisarmCode.strLen )
    {
      pBuf = zcl_memcpy( pBuf, pCmd->armDisarmCode.pStr, pCmd->armDisarmCode.strLen );
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_BYPASS, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_Send_IAS_ACE_GetZoneInformationCmd
 *
 * @brief   Call to send out a Get Zone Information Command ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneID - 8 bit value from 0 to 255
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_GetZoneInformationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    uint8 zoneID, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];

  buf[0] = zoneID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                          COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 1, buf );
}

/*********************************************************************
 * @fn      zclSS_Send_IAS_ACE_GetZoneStatusCmd
 *
 * @brief   Call to send out a Get Zone Status Command ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_GetZoneStatusCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                               zclACEGetZoneStatus_t *pCmd,
                                               uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_GET_ZONE_STATUS];

  buf[0] = pCmd->startingZoneID;
  buf[1] = pCmd->maxNumZoneIDs;
  buf[2] = pCmd->zoneStatusMaskFlag;
  buf[3] = LO_UINT16( pCmd->zoneStatusMask );
  buf[4] = HI_UINT16( pCmd->zoneStatusMask );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                          COMMAND_SS_IAS_ACE_GET_ZONE_STATUS, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_GET_ZONE_STATUS, buf );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_ArmResponse
 *
 * @brief   Call to send out a Arm Response Command ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   armNotification - notification parameter
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_ArmResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint8 armNotification, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];

  buf[0] = armNotification;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                          COMMAND_SS_IAS_ACE_ARM_RESPONSE, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, 1, buf );
}

/*********************************************************************
 * @fn      zclSS_Send_IAS_ACE_GetZoneIDMapResponseCmd
 *
 * @brief   Call to send out a Get Zone ID Map Response Cmd  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zoneIDMap - pointer to an array of 16 uint16
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_GetZoneIDMapResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint16 *zoneIDMap, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 *pIndex;
  uint8 j,len = ( ZONE_ID_MAP_ARRAY_SIZE * sizeof( uint16 ) );
  ZStatus_t stat;

  buf = zcl_mem_alloc( len );

  if ( buf )
  {
    pIndex = buf;

    for( j = 0; j < ZONE_ID_MAP_ARRAY_SIZE; j++ )
    {
      *pIndex++  = LO_UINT16( *zoneIDMap   );
      *pIndex++  = HI_UINT16( *zoneIDMap++ );
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP_RESPONSE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );

}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_GetZoneInformationResponseCmd
 *
 * @brief   Call to send out Get Zone Information Response Cmd (IAS ACE Cluster)
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_GetZoneInformationResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                            zclACEGetZoneInfoRsp_t *pCmd,
                                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 *pBuf;
              // zoneID (1) + zoneType (2) + zoneAddress (8) + zoneLabel UTF8
  uint8 len = 11 + ( pCmd->zoneLabel.strLen + 1 );
  ZStatus_t stat;

  buf = zcl_mem_alloc( len );

  if ( buf )
  {
    pBuf = buf;
    *pBuf++ = pCmd->zoneID;
    *pBuf++ = LO_UINT16( pCmd->zoneType);
    *pBuf++ = HI_UINT16( pCmd->zoneType);
    pBuf = zcl_cpyExtAddr( &buf[3], pCmd->ieeeAddr );

    *pBuf++ = pCmd->zoneLabel.strLen;

    if ( pCmd->zoneLabel.strLen )
    {
      pBuf = zcl_memcpy( pBuf, pCmd->zoneLabel.pStr, pCmd->zoneLabel.strLen );
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION_RESPONSE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_ZoneStatusChangedCmd
 *
 * @brief   Call to send out a Zone Status Changed Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_ZoneStatusChangedCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                   zclACEZoneStatusChanged_t *pCmd,
                                                   uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *pBuf;
  uint8 *pOutBuf;
  uint8 len = sizeof( zclACEZoneStatusChanged_t ) - sizeof( UTF8String_t ) +
              ( pCmd->zoneLabel.strLen + 1 );
  ZStatus_t stat;

  pBuf = zcl_mem_alloc( len );
  if ( pBuf )
  {
    pOutBuf = pBuf;

    *pOutBuf++ = pCmd->zoneID;

    *pOutBuf++ = LO_UINT16(pCmd->zoneStatus);
    *pOutBuf++ = HI_UINT16(pCmd->zoneStatus);

    *pOutBuf++ = pCmd->audibleNotification;

    *pOutBuf++ = pCmd->zoneLabel.strLen;

    if ( pCmd->zoneLabel.strLen )
    {
      pOutBuf = zcl_memcpy( pOutBuf, pCmd->zoneLabel.pStr, pCmd->zoneLabel.strLen );
    }

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_ZONE_STATUS_CHANGED, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, pBuf );

    zcl_mem_free( pBuf );
  }
  else
  {
    stat = ZMemError;
  }

  return( stat );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_PanelStatusChangedCmd
 *
 * @brief   Call to send out a Arm  Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_PanelStatusChangedCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    zclACEPanelStatusChanged_t *pCmd,
                                                    uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_PANEL_STATUS_CHANGED];

  buf[0] = pCmd->panelStatus;
  buf[1] = pCmd->secondsRemaining;
  buf[2] = pCmd->audibleNotification;
  buf[3] = pCmd->alarmStatus;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                          COMMAND_SS_IAS_ACE_PANEL_STATUS_CHANGED, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_PANEL_STATUS_CHANGED, buf );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_GetPanelStatusResponseCmd
 *
 * @brief   Call to send out a GetPanelStatusResponse Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_GetPanelStatusResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                        zclACEGetPanelStatusRsp_t *pCmd,
                                                        uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[PAYLOAD_LEN_GET_PANEL_STATUS_RESPONSE];

  buf[0] = pCmd->panelStatus;
  buf[1] = pCmd->secondsRemaining;
  buf[2] = pCmd->audibleNotification;
  buf[3] = pCmd->alarmStatus;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                          COMMAND_SS_IAS_ACE_GET_PANEL_STATUS_RESPONSE, TRUE,
                          ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0,
                          seqNum, PAYLOAD_LEN_GET_PANEL_STATUS_RESPONSE, buf );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_SetBypassedZoneListCmd
 *
 * @brief   Call to send out a SetBypassedZoneList Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_SetBypassedZoneListCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                     zclACESetBypassedZoneList_t *pCmd,
                                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 *pBuf;
  uint8 len = 1 + pCmd->numberOfZones;
  ZStatus_t stat;

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;

    *pBuf++ = pCmd->numberOfZones;
    zcl_memcpy( pBuf, pCmd->zoneID, pCmd->numberOfZones );

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_SET_BYPASSED_ZONE_LIST, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}

 /*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_BypassResponseCmd
 *
 * @brief   Call to send out a BypassResponse Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_BypassResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                zclACEBypassRsp_t *pCmd,
                                                uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 *pBuf;
  uint8 len = 1 + pCmd->numberOfZones;
  ZStatus_t stat;

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;

    *pBuf++ = pCmd->numberOfZones;
    zcl_memcpy( pBuf, pCmd->bypassResult, pCmd->numberOfZones );

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_BYPASS_RESPONSE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}

/*******************************************************************************
 * @fn      zclSS_Send_IAS_ACE_GetZoneStatusResponseCmd
 *
 * @brief   Call to send out a GetZoneStatusResponse Command  ( IAS ACE Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_ACE_GetZoneStatusResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                       zclACEGetZoneStatusRsp_t *pCmd,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 *pBuf;
  // zoneStatusComplete  + numberOfZones  + ( numberOfZones * ( ZoneID + Status ) )
  //          1          +      1         + (        X      * (   1    +   2    ) )
  uint8 len = 2 + ( pCmd->numberOfZones * sizeof( zclACEZoneStatus_t ) );
  ZStatus_t stat;

  buf = zcl_mem_alloc( len );
  if ( buf )
  {
    pBuf = buf;

    *pBuf++ = pCmd->zoneStatusComplete;
    *pBuf++ = pCmd->numberOfZones;
    zcl_memcpy( pBuf, pCmd->zoneInfo, pCmd->numberOfZones * sizeof( zclACEZoneStatus_t ) );

    stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_ACE,
                            COMMAND_SS_IAS_ACE_GET_ZONE_STATUS_RESPONSE, TRUE,
                            ZCL_FRAME_SERVER_CLIENT_DIR, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}
#endif // ZCL_ACE

#ifdef ZCL_WD
/*******************************************************************************
 * @fn      zclSS_Send_IAS_WD_StartWarningCmd
 *
 * @brief   Call to send out a Start Warning  Command (IAS WD Cluster)
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pWarning - pointer to command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_WD_StartWarningCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                             zclWDStartWarning_t *pWarning,
                                             uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[5]; // [warningMode + strobe + sirenLevel] + warningDuration + strobeDutyCycle + strobeLevel
  ZStatus_t stat;

  buf[0] = pWarning->warningmessage.warningbyte;
  buf[1] = LO_UINT16( pWarning->warningDuration );
  buf[2] = HI_UINT16( pWarning->warningDuration );
  buf[3] = pWarning->strobeDutyCycle;
  buf[4] = pWarning->strobeLevel;

  stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_WD,
                          COMMAND_SS_IAS_WD_START_WARNING, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 5, buf );

  return( stat );
}

/******************************************************************************
 * @fn      zclSS_Send_IAS_WD_StartWarningCmd
 *
 * @brief   Call to send out a Squawk Command  ( IAS WD Cluster )
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   squawk - pointer to the command structure
 * @param   disableDefaultRsp - toggle for enabling/disabling default response
 * @param   seqNum - command sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_Send_IAS_WD_SquawkCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                       zclWDSquawk_t *squawk,
                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[1];
  buf[0] = squawk->squawkbyte;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_SS_IAS_WD,
                          COMMAND_SS_IAS_WD_SQUAWK, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 1, buf);
}
#endif // ZCL_WD

/*********************************************************************
 * @fn      zclSS_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclSS_AppCallbacks_t *zclSS_FindCallbacks( uint8 endpoint )
{
  zclSSCBRec_t *pCBs;

  pCBs = zclSSCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclSS_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclSS_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
  {
    return ( stat ); // Cluster not supported thru Inter-PAN
  }
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclSS_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command -- ignore it.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands
    stat = ZFailure;
  }
  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat;
  zclSS_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = (void*)zclSS_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
  {
    return ( ZFailure );
  }

  switch ( pInMsg->msg->clusterId )
  {
#ifdef ZCL_ZONE
    case ZCL_CLUSTER_ID_SS_IAS_ZONE:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
      {
        stat = zclSS_ProcessInZoneStatusCmdsServer( pInMsg, pCBs );
      }
      else
      {
        stat = zclSS_ProcessInZoneStatusCmdsClient( pInMsg, pCBs );
      }
      break;
#endif // ZCL_ZONE

#ifdef ZCL_ACE
    case ZCL_CLUSTER_ID_SS_IAS_ACE:
      if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
      {
        stat = zclSS_ProcessInACECmdsServer( pInMsg, pCBs );
      }
      else
      {
        stat = zclSS_ProcessInACECmdsClient( pInMsg, pCBs );
      }
      break;
#endif // ZCL_ACE

#ifdef ZCL_WD
    case ZCL_CLUSTER_ID_SS_IAS_WD:
      stat = zclSS_ProcessInWDCmds( pInMsg, pCBs );
      break;
#endif // ZCL_WD

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

#ifdef ZCL_ZONE
/*********************************************************************
 * @fn      zclSS_ProcessInZoneStatusCmdsServer
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInZoneStatusCmdsServer( zclIncoming_t *pInMsg,
                                                      zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SS_IAS_ZONE_STATUS_ENROLL_RESPONSE:
      stat = zclSS_ProcessInCmd_ZoneStatus_EnrollResponse( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ZONE_STATUS_INIT_NORMAL_OP_MODE:
      stat = zclSS_ProcessInCmd_ZoneStatus_InitNormalOperationMode( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ZONE_STATUS_INIT_TEST_MODE:
      stat = zclSS_ProcessInCmd_ZoneStatus_InitTestMode( pInMsg, pCBs );
      break;

  default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_ProcessInZoneStatusCmdsClient
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInZoneStatusCmdsClient( zclIncoming_t *pInMsg,
                                                      zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION:
      stat = zclSS_ProcessInCmd_ZoneStatus_ChangeNotification( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST:
      stat = zclSS_ProcessInCmd_ZoneStatus_EnrollRequest( pInMsg, pCBs );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif // ZCL_ZONE

#ifdef ZCL_ACE
/*********************************************************************
 * @fn      zclSS_ProcessInACECmdsServer
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInACECmdsServer( zclIncoming_t *pInMsg,
                                               zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SS_IAS_ACE_ARM:
      stat = zclSS_ProcessInCmd_ACE_Arm( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_BYPASS:
      stat = zclSS_ProcessInCmd_ACE_Bypass( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_EMERGENCY:
      stat = zclSS_ProcessInCmd_ACE_Emergency( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_FIRE:
      stat = zclSS_ProcessInCmd_ACE_Fire( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_PANIC:
      stat = zclSS_ProcessInCmd_ACE_Panic( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP:
      stat = zclSS_ProcessInCmd_ACE_GetZoneIDMap( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION:
      stat = zclSS_ProcessInCmd_ACE_GetZoneInformation( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_PANEL_STATUS:
      stat = zclSS_ProcessInCmd_ACE_GetPanelStatus( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_BYPASSED_ZONE_LIST:
      stat = zclSS_ProcessInCmd_ACE_GetBypassedZoneList( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_ZONE_STATUS:
      stat = zclSS_ProcessInCmd_ACE_GetZoneStatus( pInMsg, pCBs );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_ProcessInACECmdsClient
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInACECmdsClient( zclIncoming_t *pInMsg,
                                               zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SS_IAS_ACE_ARM_RESPONSE:
      stat = zclSS_ProcessInCmd_ACE_ArmResponse( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP_RESPONSE:
      stat = zclSS_ProcessInCmd_ACE_GetZoneIDMapResponse( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION_RESPONSE:
      stat = zclSS_ProcessInCmd_ACE_GetZoneInformationResponse( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_ZONE_STATUS_CHANGED:
      stat = zclSS_ProcessInCmd_ACE_ZoneStatusChanged( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_PANEL_STATUS_CHANGED:
      stat = zclSS_ProcessInCmd_ACE_PanelStatusChanged( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_PANEL_STATUS_RESPONSE:
      stat = zclSS_ProcessInCmd_ACE_GetPanelStatusResponse( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_SET_BYPASSED_ZONE_LIST:
      stat = zclSS_ProcessInCmd_ACE_SetBypassedZoneList( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_BYPASS_RESPONSE:
      stat = zclSS_ProcessInCmd_ACE_BypassResponse( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_ACE_GET_ZONE_STATUS_RESPONSE:
      stat = zclSS_ProcessInCmd_ACE_GetZoneStatusResponse( pInMsg, pCBs );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif // ZCL_ACE

#ifdef ZCL_ZONE
/*********************************************************************
 * @fn      zclSS_AddZone
 *
 * @brief   Add a zone for an endpoint
 *
 * @param   endpoint - endpoint of new zone
 * @param   zone - new zone item
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_AddZone( uint8 endpoint, IAS_ACE_ZoneTable_t *zone )
{
  zclSS_ZoneItem_t *pNewItem;
  zclSS_ZoneItem_t *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclSS_ZoneItem_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  // Fill in the plugin record.
  pNewItem->next = (zclSS_ZoneItem_t *)NULL;
  pNewItem->endpoint = endpoint;
  zcl_memcpy( (uint8*)&(pNewItem->zone), (uint8*)zone, sizeof ( IAS_ACE_ZoneTable_t ));

  // Find spot in list
  if (  zclSS_ZoneTable == NULL )
  {
    zclSS_ZoneTable = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclSS_ZoneTable;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclSS_CountAllZones
 *
 * @brief   Count the total number of zones
 *
 * @param   none
 *
 * @return  number of zones
 */
uint8 zclSS_CountAllZones( void )
{
  zclSS_ZoneItem_t *pLoop;
  uint8 cnt = 0;

  // Look for end of list
  pLoop = zclSS_ZoneTable;
  while ( pLoop )
  {
    cnt++;
    pLoop = pLoop->next;
  }
  return ( cnt );
}

/*********************************************************************
 * @fn      zclSS_GetNextFreeZoneID
 *
 * @brief   Get the next free zone ID
 *
 * @param   none
 *
 * @return  free zone ID (0-ZCL_SS_MAX_ZONE_ID) ,
 *          (ZCL_SS_MAX_ZONE_ID + 1) if none is found (0xFF)
 */
static uint8 zclSS_GetNextFreeZoneID( void )
{
  static uint8 nextAvailZoneID = 0;

  if ( zclSS_ZoneIDAvailable( nextAvailZoneID ) == FALSE )
  {
    uint8 zoneID = nextAvailZoneID;

    // Look for next available zone ID
    do
    {
      if ( ++zoneID > ZCL_SS_MAX_ZONE_ID )
      {
        zoneID = 0; // roll over
      }
    } while ( (zoneID != nextAvailZoneID) && (zclSS_ZoneIDAvailable( zoneID ) == FALSE) );

    // Did we found a free zone ID?
    if ( zoneID != nextAvailZoneID )
    {
      nextAvailZoneID = zoneID;
    }
    else
    {
      return ( ZCL_SS_MAX_ZONE_ID + 1 );
    }
  }

  return ( nextAvailZoneID );
}


/*********************************************************************
 * @fn      zclSS_ZoneIDAvailable
 *
 * @brief   Check to see whether zoneID is available for use
 *
 * @param   zoneID - ID to look for zone
 *
 * @return  TRUE if zoneID is available, FALSE otherwise
 */
static uint8 zclSS_ZoneIDAvailable( uint8 zoneID )
{
  zclSS_ZoneItem_t *pLoop;

  if ( zoneID < ZCL_SS_MAX_ZONE_ID )
  {
    pLoop = zclSS_ZoneTable;
    while ( pLoop )
    {
      if ( pLoop->zone.zoneID == zoneID  )
      {
        return ( FALSE );
      }
      pLoop = pLoop->next;
    }

    // Zone ID not in use
    return ( TRUE );
  }

  return ( FALSE );
}
#endif // ZCL_ZONE

#if defined(ZCL_ZONE) || defined(ZCL_ACE)
/*********************************************************************
 * @fn      zclSS_FindZone
 *
 * @brief   Find a zone with endpoint and ZoneID
 *
 * @param   endpoint -
 * @param   zoneID - ID to look for zone
 *
 * @return  a pointer to the zone information, NULL if not found
 */
IAS_ACE_ZoneTable_t *zclSS_FindZone( uint8 endpoint, uint8 zoneID )
{
  zclSS_ZoneItem_t *pLoop;

  // Look for end of list
  pLoop = zclSS_ZoneTable;
  while ( pLoop )
  {
    if ( ( pLoop->endpoint == endpoint ) && ( pLoop->zone.zoneID == zoneID )  )
    {
      return ( &(pLoop->zone) );
    }
    pLoop = pLoop->next;
  }

  return ( (IAS_ACE_ZoneTable_t *)NULL );
}

/*********************************************************************
 * @fn      zclSS_RemoveZone
 *
 * @brief   Remove a zone with endpoint and zoneID
 *
 * @param   endpoint - endpoint of zone to be removed
 * @param   zoneID - ID to look for zone
 *
 * @return  TRUE if removed, FALSE if not found
 */
uint8 zclSS_RemoveZone( uint8 endpoint, uint8 zoneID )
{
  zclSS_ZoneItem_t *pLoop;
  zclSS_ZoneItem_t *pPrev;

  // Look for end of list
  pLoop = zclSS_ZoneTable;
  pPrev = NULL;
  while ( pLoop )
  {
    if ( pLoop->endpoint == endpoint && pLoop->zone.zoneID == zoneID )
    {
      if ( pPrev == NULL )
      {
        zclSS_ZoneTable = pLoop->next;
      }
      else
      {
        pPrev->next = pLoop->next;
      }

      // Free the memory
      zcl_mem_free( pLoop );

      return ( TRUE );
    }
    pPrev = pLoop;
    pLoop = pLoop->next;
  }

  return ( FALSE );
}

/*********************************************************************
 * @fn      zclSS_UpdateZoneAddress
 *
 * @brief   Update Zone Address for zoneID
 *
 * @param   endpoint - endpoint of zone
 * @param   zoneID - ID to look for zone
 * @param   ieeeAddr - Device IEEE Address
 *
 * @return  none
 */
void zclSS_UpdateZoneAddress( uint8 endpoint, uint8 zoneID, uint8 *ieeeAddr )
{
  IAS_ACE_ZoneTable_t *pZone;

  pZone = zclSS_FindZone( endpoint, zoneID );

  if ( pZone != NULL )
  {
    // Update the zone address
    zcl_cpyExtAddr( pZone->zoneAddress, ieeeAddr );
  }
}
#endif // ZCL_ZONE || ZCL_ACE

#ifdef ZCL_ZONE
/*******************************************************************************
 * @fn      zclSS_ProcessInCmd_ZoneStatus_ChangeNotification
 *
 * @brief   Process in the received StatusChangeNotification Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_ChangeNotification( zclIncoming_t *pInMsg,
                                                                   zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnChangeNotification )
  {
    zclZoneChangeNotif_t cmd;

    cmd.zoneStatus = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.extendedStatus = pInMsg->pData[2];
    cmd.zoneID = pInMsg->pData[3];
    cmd.delay = BUILD_UINT16( pInMsg->pData[4], pInMsg->pData[5] );

    return ( pCBs->pfnChangeNotification( &cmd, &(pInMsg->msg->srcAddr) ) );
  }

  return ( ZFailure );
}

/*******************************************************************************
 * @fn      zclSS_ProcessInCmd_ZoneStatus_EnrollRequest
 *
 * @brief   Process in the received StatusEnrollRequest Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_EnrollRequest( zclIncoming_t *pInMsg,
                                                              zclSS_AppCallbacks_t *pCBs )
{
  IAS_ACE_ZoneTable_t zone;
  ZStatus_t stat = ZFailure;
  uint16 zoneType;
  uint16 manuCode;
  uint8 responseCode;
  uint8 zoneID;

  zoneType = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
  manuCode = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );

  if ( zclSS_ZoneTypeSupported( zoneType ) )
  {
    // Add zone to the table if space is available
    if ( ( zclSS_CountAllZones() < ZCL_SS_MAX_ZONES-1 ) &&
       ( ( zoneID = zclSS_GetNextFreeZoneID() ) <= ZCL_SS_MAX_ZONE_ID ) )
    {
      zone.zoneID = zoneID;
      zone.zoneType = zoneType;

      // The application will fill in the right IEEE Address later
      zcl_cpyExtAddr( zone.zoneAddress, (void *)zclSS_UknownIeeeAddress );

      if ( zclSS_AddZone( pInMsg->msg->endPoint, &zone ) == ZSuccess )
      {
        responseCode = ZSuccess;
      }
      else
      {
        // CIE does not permit new zones to enroll at this time
        responseCode = SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT;
      }
    }
    else
    {
      // CIE reached its limit of number of enrolled zones
      responseCode = SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES;
    }
  }
  else
  {
    // Zone type is not known to CIE and is not supported
    responseCode = SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NOT_SUPPORTED;
  }

  // Callback the application so it can fill in the Device IEEE Address
  if ( pCBs->pfnEnrollRequest )
  {
    zclZoneEnrollReq_t req;

    req.srcAddr = &(pInMsg->msg->srcAddr);
    req.zoneID = zoneID;
    req.zoneType = zoneType;
    req.manufacturerCode = manuCode;

    stat = pCBs->pfnEnrollRequest( &req, pInMsg->msg->endPoint );
  }

  if ( stat == ZSuccess )
  {
    // Send a response back
    stat = zclSS_IAS_Send_ZoneStatusEnrollResponseCmd( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                                       responseCode, zoneID, true, pInMsg->hdr.transSeqNum );

    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
  else
  {
    return ( stat );
  }
}

/*******************************************************************************
 * @fn      zclSS_ProcessInCmd_ZoneStatus_EnrollResponse
 *
 * @brief   Process in the received Zone Enroll Response Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_EnrollResponse( zclIncoming_t *pInMsg,
                                                               zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnEnrollResponse )
  {
    zclZoneEnrollRsp_t rsp;

    rsp.responseCode = pInMsg->pData[0];
    rsp.zoneID = pInMsg->pData[1];

    return ( pCBs->pfnEnrollResponse( &rsp ) );
  }

  return ( ZFailure );
}

/*******************************************************************************
 * @fn      zclSS_ProcessInCmd_ZoneStatus_InitNormalOperationMode
 *
 * @brief   Process in the received Initiate Normal Operation Mode Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_InitNormalOperationMode( zclIncoming_t *pInMsg,
                                                                        zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnInitNormalOpMode )
  {
    pCBs->pfnInitNormalOpMode( pInMsg );

    return ( ZCL_STATUS_CMD_HAS_RSP  );
  }

  return ( ZFailure );
}

/*******************************************************************************
 * @fn      zclSS_ProcessInCmd_ZoneStatus_InitTestMode
 *
 * @brief   Process in the received Initiate Test Mode Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_InitTestMode( zclIncoming_t *pInMsg,
                                                             zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnInitTestMode )
  {
    zclZoneInitTestMode_t cmd;

    cmd.testModeDuration =  pInMsg->pData[0];
    cmd.currZoneSensitivityLevel = pInMsg->pData[1];

    pCBs->pfnInitTestMode( &cmd, pInMsg );

    return ( ZCL_STATUS_CMD_HAS_RSP  );
  }

  return ( ZFailure );
}
#endif // ZCL_ZONE

#ifdef ZCL_ACE
/*********************************************************************
 * @fn      zclSS_Parse_UTF8String
 *
 * @brief   Called to parse a UTF8String from a message
 *
 * @param   pBuf - pointer to the incoming message
 * @param   pString - pointer to the UTF8String_t
 * @param   maxLen - max length of the string field in pBuf
 *
 * @return  uint8 - number of bytes parsed from pBuf
 */
static uint8 zclSS_Parse_UTF8String( uint8 *pBuf, UTF8String_t *pString, uint8 maxLen )
{
  uint8 originalLen = 0;

  pString->strLen = *pBuf++;
  if ( pString->strLen == 0xFF )
  {
    // If character count is 0xFF, set string length to 0
    pString->strLen = 0;
  }

  if ( pString->strLen != 0 )
  {
    originalLen = pString->strLen; //save original length

    // truncate to maximum size
    if ( pString->strLen > maxLen )
    {
      pString->strLen = maxLen;
    }

    pString->pStr = pBuf;
  }
  else
  {
    pString->pStr = NULL;
  }

  return originalLen + 1; // this is including the strLen field
}

/*********************************************************************
 * @fn      zclSS_ParseInCmd_ACE_Arm
 *
 * @brief   Parse received ACE Arm Command.
 *
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_ParseInCmd_ACE_Arm( zclACEArm_t *pCmd, uint8 *pInBuf )
{
  uint8 fieldLen;

  pCmd->armMode = *pInBuf++;

  fieldLen = zclSS_Parse_UTF8String( pInBuf, &pCmd->armDisarmCode, ARM_DISARM_CODE_LEN );
  pInBuf += fieldLen;

  pCmd->zoneID = *pInBuf++;

  return ZSuccess;
}

/*********************************************************************
 * @fn      zclSS_ParseInCmd_ACE_Bypass
 *
 * @brief   Parse received ACE Bypass Command.
 *
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_ParseInCmd_ACE_Bypass( zclACEBypass_t *pCmd, uint8 *pInBuf )
{
  pCmd->numberOfZones = *pInBuf++;
  pCmd->bypassBuf = pInBuf;

  // point to the place in the receiving buffer where the ArmDisarm code is located
  pInBuf += pCmd->numberOfZones;

  (void)zclSS_Parse_UTF8String( pInBuf, &pCmd->armDisarmCode, ARM_DISARM_CODE_LEN );

  return ZSuccess;
}

/*********************************************************************
 * @fn      zclSS_ParseInCmd_ACE_GetZoneInformationResponse
 *
 * @brief   Parse received ACE GetZoneInformationResponse Command.
 *
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_ParseInCmd_ACE_GetZoneInformationResponse( zclACEGetZoneInfoRsp_t *pCmd,
                                                           uint8 *pInBuf )
{
  pCmd->zoneID = *pInBuf++;;

  pCmd->zoneType = BUILD_UINT16( pInBuf[0], pInBuf[1] );
  pInBuf += 2;

  pCmd->ieeeAddr = pInBuf;
  pInBuf += Z_EXTADDR_LEN;

  (void)zclSS_Parse_UTF8String( pInBuf, &pCmd->zoneLabel, ZONE_LABEL_LEN );

  return ZSuccess;
}

/*********************************************************************
 * @fn      zclSS_ParseInCmd_ACE_ZoneStatusChanged
 *
 * @brief   Parse received ACE ZoneStatusChanged Command.
 *
 * @param   pCmd - pointer to the output data struct
 * @param   pInBuf - pointer to the input data buffer
 *
 * @return  ZStatus_t
 */
ZStatus_t zclSS_ParseInCmd_ACE_ZoneStatusChanged( zclACEZoneStatusChanged_t *pCmd,
                                                  uint8 *pInBuf )
{
  pCmd->zoneID = *pInBuf++;
  pCmd->zoneStatus = BUILD_UINT16( pInBuf[0], pInBuf[1] );
  pInBuf += 2;
  pCmd->audibleNotification = *pInBuf++;

  (void)zclSS_Parse_UTF8String( pInBuf, &pCmd->zoneLabel, ZONE_LABEL_LEN );

  return ZSuccess;
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_Arm
 *
 * @brief   Process in the received Arm Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_Arm( zclIncoming_t *pInMsg,
                                             zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZFailure;

  if ( pCBs->pfnACE_Arm )
  {
    zclACEArm_t cmd;
    uint8 armNotification = 0xFF;

    if ( zclSS_ParseInCmd_ACE_Arm( &cmd, pInMsg->pData ) == ZSuccess )
    {
      armNotification = pCBs->pfnACE_Arm( &cmd );

      if ( armNotification != 0xFF )
      {
        // Send a response back
        zclSS_Send_IAS_ACE_ArmResponse( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                        armNotification, true, pInMsg->hdr.transSeqNum );

        return ( ZCL_STATUS_CMD_HAS_RSP );
      }
      else
      {
        stat = ZCL_STATUS_INVALID_VALUE;
      }
    }
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_Bypass
 *
 * @brief   Process in the received Bypass Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_Bypass( zclIncoming_t *pInMsg,
                                                zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_Bypass )
  {
    zclACEBypass_t cmd;

    if ( zclSS_ParseInCmd_ACE_Bypass( &cmd, pInMsg->pData ) == ZSuccess )
    {
      return ( pCBs->pfnACE_Bypass( &cmd ) );
    }
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_Emergency
 *
 * @brief   Process in the received Emergency Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_Emergency( zclIncoming_t *pInMsg,
                                                   zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_Emergency )
  {
    return ( pCBs->pfnACE_Emergency() );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_Fire
 *
 * @brief   Process in the received Fire Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_Fire( zclIncoming_t *pInMsg,
                                              zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_Fire )
  {
    return ( pCBs->pfnACE_Fire() );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_Panic
 *
 * @brief   Process in the received Panic Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_Panic( zclIncoming_t *pInMsg,
                                               zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_Panic )
  {
    return ( pCBs->pfnACE_Panic() );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetZoneIDMap
 *
 * @brief   Process in the received GetZoneIDMap Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneIDMap( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZFailure;
  uint16 zoneIDMap[16];
  uint16 mapSection;
  uint8 zoneID;
  uint8 i, j;

  for ( i = 0; i < 16; i++ )
  {
    mapSection = 0;

    // Find out Zone IDs that are allocated for this map section
    for ( j = 0; j < 16; j++ )
    {
      zoneID = 16 * i + j;
      if ( zclSS_FindZone( pInMsg->msg->endPoint, zoneID ) != NULL )
      {
        // Set the corresponding bit
        mapSection |= ( 0x01 << j );
      }
    }
    zoneIDMap[i] = mapSection;
  }

  if ( pCBs->pfnACE_GetZoneIDMap )
  {
    stat = pCBs->pfnACE_GetZoneIDMap( );
  }

  if ( stat == ZSuccess )
  {
    // Send a response back
    zclSS_Send_IAS_ACE_GetZoneIDMapResponseCmd( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                                zoneIDMap, true, pInMsg->hdr.transSeqNum );

    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
  else
  {
    return ( stat );
  }
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetZoneInformation
 *
 * @brief   Process in the received GetZoneInformation Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneInformation( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZFailure;

  if ( pCBs->pfnACE_GetZoneInformation )
  {
    stat = pCBs->pfnACE_GetZoneInformation( pInMsg->pData[0] );  // the callback function shall take care of sending
                                                                 // Get Zone Information Response command
  }

  if ( stat == ZSuccess )
  {
    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
  else
  {
    return ( stat );
  }
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetPanelStatus
 *
 * @brief   Process in the received GetPanelStatus Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetPanelStatus( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZFailure;

  if ( pCBs->pfnACE_GetPanelStatus )
  {
    stat = pCBs->pfnACE_GetPanelStatus();  // the callback function shall take care of sending Get Panel Status Response

    if ( stat == ZSuccess )
    {
      return ( ZCL_STATUS_CMD_HAS_RSP );
    }
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetBypassedZoneList
 *
 * @brief   Process in the received GetBypassedZoneList Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetBypassedZoneList( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZFailure;

  if ( pCBs->pfnACE_GetBypassedZoneList )
  {
    stat = pCBs->pfnACE_GetBypassedZoneList();  // the callback function shall take care of
                                                // sending Set Bypassed Zone List command
    if ( stat == ZSuccess )
    {
      return ( ZCL_STATUS_CMD_HAS_RSP );
    }
  }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetZoneStatus
 *
 * @brief   Process in the received GetZoneStatus Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneStatus( zclIncoming_t *pInMsg, zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat = ZFailure;

  if ( pCBs->pfnACE_GetZoneStatus )
  {
    zclACEGetZoneStatus_t cmd;

    cmd.startingZoneID = pInMsg->pData[0];
    cmd.maxNumZoneIDs = pInMsg->pData[1];
    cmd.zoneStatusMaskFlag = pInMsg->pData[2];
    cmd.zoneStatusMask = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );

    stat = pCBs->pfnACE_GetZoneStatus( &cmd );  // the callback function shall take care of
                                                // sending Get Zone Status Response
    if ( stat == ZSuccess )
    {
      return ( ZCL_STATUS_CMD_HAS_RSP );
    }
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_ArmResponse
 *
 * @brief   Process in the received ArmResponse Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_ArmResponse( zclIncoming_t *pInMsg,
                                                     zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_ArmResponse )
  {
    return ( pCBs->pfnACE_ArmResponse(pInMsg->pData[0]) );
  }

  return ( ZFailure );
}


/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetZoneIDMapResponse
 *
 * @brief   Process in the received GetZoneIDMapResponse Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneIDMapResponse( zclIncoming_t *pInMsg,
                                                              zclSS_AppCallbacks_t *pCBs )
{
  uint16 *buf;
  uint16 *pIndex;
  uint8 *pData;
  uint8 i, len = 32; // 16 fields of 2 octets

  buf = zcl_mem_alloc( len );

  if ( buf )
  {
    pIndex = buf;
    pData = pInMsg->pData;

    for ( i = 0; i < ZONE_ID_MAP_ARRAY_SIZE; i++ )
    {
      *pIndex++ = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
    }

    zcl_mem_free( buf );

    if ( pCBs->pfnACE_GetZoneIDMapResponse )
    {
      return ( pCBs->pfnACE_GetZoneIDMapResponse( buf ) );
    }

   return ( ZFailure );
  }
  else
  {
    return ( ZMemError );
  }
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetZoneInformationResponse
 *
 * @brief   Process in the received GetZoneInformationResponse Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneInformationResponse( zclIncoming_t *pInMsg,
                                                                    zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_GetZoneInformationResponse )
  {
    zclACEGetZoneInfoRsp_t cmd;

    if ( zclSS_ParseInCmd_ACE_GetZoneInformationResponse( &cmd, pInMsg->pData ) == ZSuccess )
    {
      return( pCBs->pfnACE_GetZoneInformationResponse( &cmd ) );
    }
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_ZoneStatusChanged
 *
 * @brief   Process in the received ZoneStatusChanged Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_ZoneStatusChanged( zclIncoming_t *pInMsg,
                                                           zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_ZoneStatusChanged )
  {
    zclACEZoneStatusChanged_t cmd;

    if ( zclSS_ParseInCmd_ACE_ZoneStatusChanged( &cmd, pInMsg->pData ) == ZSuccess )
    {
      return ( pCBs->pfnACE_ZoneStatusChanged( &cmd ) );
    }
    else
    {
      return ( ZMemError );
    }
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_PanelStatusChanged
 *
 * @brief   Process in the received PanelStatusChanged Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_PanelStatusChanged( zclIncoming_t *pInMsg,
                                                            zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_PanelStatusChanged )
  {
    zclACEPanelStatusChanged_t cmd;

    cmd.panelStatus =  pInMsg->pData[0];
    cmd.secondsRemaining =  pInMsg->pData[1];
    cmd.audibleNotification = pInMsg->pData[2];
    cmd.alarmStatus = pInMsg->pData[3];

    return ( pCBs->pfnACE_PanelStatusChanged( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetPanelStatusResponse
 *
 * @brief   Process in the received GetPanelStatusResponse Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetPanelStatusResponse( zclIncoming_t *pInMsg,
                                                                zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_GetPanelStatusResponse )
  {
    zclACEGetPanelStatusRsp_t cmd;

    cmd.panelStatus =  pInMsg->pData[0];
    cmd.secondsRemaining =  pInMsg->pData[1];
    cmd.audibleNotification = pInMsg->pData[2];
    cmd.alarmStatus = pInMsg->pData[3];

    return ( pCBs->pfnACE_GetPanelStatusResponse( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_SetBypassedZoneList
 *
 * @brief   Process in the received SetBypassedZoneList Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_SetBypassedZoneList( zclIncoming_t *pInMsg,
                                                             zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_SetBypassedZoneList )
  {
    zclACESetBypassedZoneList_t cmd;

    cmd.numberOfZones = pInMsg->pData[0];
    cmd.zoneID = &(pInMsg->pData[1]);   // point to the list of ZoneIDs

    return ( pCBs->pfnACE_SetBypassedZoneList( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_BypassResponse
 *
 * @brief   Process in the received BypassResponse Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_BypassResponse( zclIncoming_t *pInMsg,
                                                        zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_BypassResponse )
  {
    zclACEBypassRsp_t cmd;

    cmd.numberOfZones = pInMsg->pData[0];
    cmd.bypassResult = &(pInMsg->pData[1]);   // point to the list of Bypass Result for ZoneID x

    return ( pCBs->pfnACE_BypassResponse( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_ACE_GetZoneStatusResponse
 *
 * @brief   Process in the received GetZoneStatusResponse Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ACE_GetZoneStatusResponse( zclIncoming_t *pInMsg,
                                                               zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnACE_GetZoneStatusResponse )
  {
    zclACEGetZoneStatusRsp_t cmd;

    cmd.zoneStatusComplete = pInMsg->pData[0];
    cmd.numberOfZones = pInMsg->pData[1];
    cmd.zoneInfo = (zclACEZoneStatus_t *)&(pInMsg->pData[2]);   // point to the list of Zone Status

    return ( pCBs->pfnACE_GetZoneStatusResponse( &cmd ) );
  }

  return ( ZFailure );
}
#endif // ZCL_ACE

#ifdef ZCL_WD
/*********************************************************************
 * @fn      zclSS_ProcessInWDCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInWDCmds( zclIncoming_t *pInMsg,
                                        zclSS_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_SS_IAS_WD_START_WARNING:
      stat = zclSS_ProcessInCmd_WD_StartWarning( pInMsg, pCBs );
      break;

    case COMMAND_SS_IAS_WD_SQUAWK:
      stat = zclSS_ProcessInCmd_WD_Squawk( pInMsg, pCBs );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_WD_StartWarning
 *
 * @brief   Process in the received StartWarning Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_WD_StartWarning( zclIncoming_t *pInMsg,
                                                     zclSS_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnWD_StartWarning )
  {
    zclWDStartWarning_t cmd;

    cmd.warningmessage.warningbyte = pInMsg->pData[0];
    cmd.warningDuration = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

    return ( pCBs->pfnWD_StartWarning( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclSS_ProcessInCmd_WD_Squawk
 *
 * @brief   Process in the received Squawk Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return   ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_WD_Squawk( zclIncoming_t *pInMsg,
                                              zclSS_AppCallbacks_t *pCBs )
{
  zclWDSquawk_t cmd;

  if ( pCBs->pfnWD_Squawk )
  {
    cmd.squawkbyte = pInMsg->pData[0];

    return ( pCBs->pfnWD_Squawk( &cmd ) );
  }

  return ( ZFailure );
}
#endif // ZCL_WD

/*******************************************************************************
*******************************************************************************/

