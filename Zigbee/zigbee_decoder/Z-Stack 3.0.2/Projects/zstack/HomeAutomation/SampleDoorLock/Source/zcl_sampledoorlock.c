/**************************************************************************************************
  Filename:       zcl_sampledoorlock.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013-2014 Texas Instruments Incorporated. All rights reserved.

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
  This application implements a ZigBee Door Lock, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - LEDs:
    LED1 reflect the current door lock state (locked = on)

  Application-specific menu system:

    <CHANGE PIN> Change the PIN used in the Lock/Unlock command
      Up/Down changes the value of the current digit, OK/Select sets the current digit
      This screen shows the following information:
        Line1:
          After entering all 4 pin digits, briefly shows "PIN SAVED" message
        Line2:
          Shows the Enter PIN prompt with the current PIN digit visible

    <TOGGLE LOCK> Toggles the local door lock
      Up locks the door, Down unlocks the door
      This screen shows the following information:
        Line1:
          Shows instructions normally, briefly displays INVALID PIN after receiving
          a Lock/Unlock command with the incorrect pin
        Line2:
          Shows the current local lock state

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "MT_APP.h"
#include "OSAL_Nv.h"
#include "MT_SYS.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "bdb_interface.h"
#ifdef BDB_REPORTING
#include "bdb_Reporting.h"
#endif
#include "zcl_closures.h"

#include "zcl_sampledoorlock.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include "zcl_sampleapps_ui.h"

/*********************************************************************
 * MACROS
 */

#define UI_CHANGE_PIN    1         
#define UI_TOGGLE_LOCK   2

#define APP_TITLE "    DoorLock    "

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclSampleDoorLock_TaskID; 

extern int16 zdpExternalStateTaskID;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#ifdef BDB_REPORTING
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 8
  static uint8 reportableChange[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 4
  static uint8 reportableChange[] = {0x01, 0x00, 0x00, 0x00}; 
#endif 
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 2
  static uint8 reportableChange[] = {0x01, 0x00};
#endif 
#endif

static uint8 giDoorLockPINCount = 0;

static uint8 giDoorLockPINColumnCount = 0;

// Master PIN code for DoorLock
static uint8 aiDoorLockMasterPINCode[] = {4,0x31,0x32,0x33,0x34};

// Test Endpoint to allow SYS_APP_MSGs
static endPointDesc_t sampleDoorLock_TestEp =
{
  8,                                 // Test endpoint
  0,
  &zclSampleDoorLock_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

static bool invalidPin = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleDoorLock_HandleKeys( byte shift, byte keys );
static void zclSampleDoorLock_BasicResetCB( void );

static void zclSampleDoorLock_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);

#ifdef MT_APP_FUNC
static void zclSampleDoorLock_ProcessAppMsg( uint8 srcEP, uint8 len, uint8 *msg );
static void zclSampleDoorLock_ProcessFoundationMsg( afAddrType_t *dstAddr, uint16 clusterID,
                                                    zclFrameHdr_t *hdr, zclParseCmd_t *pParseCmd );
static void zclSampleDoorLock_ProcessGeneralMsg( uint8 srcEP, afAddrType_t *dstAddr,
                                    uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
static void zclSampleDoorLock_ProcessGroupCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                  uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
static void zclSampleDoorLock_ProcessSceneCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
static void zclSampleDoorLock_ProcessClosuresMsg( uint8 srcEP, afAddrType_t *dstAddr,
                                    uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
#endif

// Functions to process ZCL Foundation incoming Command/Response messages
static ZStatus_t zclSampleDoorLock_DoorLockCB ( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd );
static ZStatus_t zclSampleDoorLock_DoorLockRspCB ( zclIncoming_t *pInMsg, uint8 status );
//static ZStatus_t zclSampleDoorLock_DoorLockToggleDoorCB( zclDoorLock_t *pCmd );
static ZStatus_t zclSampleDoorLock_DoorLockActuator ( uint8 newDoorLockState );

static void zclSampleDoorLock_UiActionChangePin(uint16 keys);
static void zclSampleDoorLock_UiActionLockUnlock(uint16 keys);
void zclSampleDoorLock_UiAppUpdateLcd(uint8 uiCurrentState, char * line[3]);
static void zclSampleDoorLock_UpdateLedState(void);

static void zclSampleApp_BatteryWarningCB( uint8 voltLevel);

/*********************************************************************
 * STATUS STRINGS
 */
#ifdef LCD_SUPPORTED
const char sStorePIN[]     = "    PIN SAVED   ";
const char sDoorLocked[]   = "  Door Locked";
const char sDoorUnlocked[] = "  Door Unlocked";
const char sInvalidPIN[]   = "  Invalid PIN";
#endif

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleDoorLock_CmdCallbacks =
{
  zclSampleDoorLock_BasicResetCB,         // Basic Cluster Reset command
  NULL,                                   // Identify Trigger Effect command
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                               // Level Control Move to Level command
  NULL,                                               // Level Control Move command
  NULL,                                               // Level Control Step command
  NULL,                                               // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                   // Scene Store Request command
  NULL,                                   // Scene Recall Request command
  NULL,                                   // Scene Response command
#endif
#if ZCL_ALARMS
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
 * ZCL Closure cluster Callback table
 */
static zclClosures_DoorLockAppCallbacks_t zclSampleDoorLock_DoorLockCmdCallbacks =
{
  zclSampleDoorLock_DoorLockCB,                           // DoorLock cluster command
  zclSampleDoorLock_DoorLockRspCB,                        // DoorLock Response
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

/*********************************************************************
 * CONSTANTS
 */

const uiState_t zclSampleDoorLock_UiAppStatesMain[] =
{
  /*  UI_STATE_BACK_FROM_APP_MENU  */   {UI_STATE_DEFAULT_MOVE, UI_TOGGLE_LOCK, UI_KEY_SW_5_PRESSED, &UI_ActionBackFromAppMenu}, //do not change this line, except for the second item, which should point to the last entry in this menu
  /*  UI_CHANGE_PIN          */   {UI_STATE_DEFAULT_MOVE, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED, &zclSampleDoorLock_UiActionChangePin},
  /*  UI_TOGGLE_LOCK         */   {UI_STATE_BACK_FROM_APP_MENU, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED, &zclSampleDoorLock_UiActionLockUnlock},
};

/*********************************************************************
 * @fn          zclSampleDoorLock_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclSampleDoorLock_Init( byte task_id )
{
  zclSampleDoorLock_TaskID = task_id;

  // Register the Simple Descriptor for this application
  bdb_RegisterSimpleDescriptor( &zclSampleDoorLock_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLEDOORLOCK_ENDPOINT, &zclSampleDoorLock_CmdCallbacks );

  // Register the application's attribute list
  zclSampleDoorLock_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLEDOORLOCK_ENDPOINT, zclSampleDoorLock_NumAttributes, zclSampleDoorLock_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclSampleDoorLock_TaskID );

  // Register low voltage NV memory protection application callback
  RegisterVoltageWarningCB( zclSampleApp_BatteryWarningCB );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclSampleDoorLock_TaskID );
  
  bdb_RegisterCommissioningStatusCB( zclSampleDoorLock_ProcessCommissioningStatus );

  //Register the ZCL DoorLock Cluster Library callback function
  zclClosures_RegisterDoorLockCmdCallbacks( SAMPLEDOORLOCK_ENDPOINT, &zclSampleDoorLock_DoorLockCmdCallbacks );

  // Register for a test endpoint
  afRegister( &sampleDoorLock_TestEp );

  // initialize NVM for storing PIN information
  if ( SUCCESS == osal_nv_item_init( ZCD_NV_APS_DOORLOCK_PIN, 5, aiDoorLockMasterPINCode ) )
    // use NVM PIN number in APP
    osal_nv_read( ZCD_NV_APS_DOORLOCK_PIN, 0, 5, aiDoorLockMasterPINCode );

#ifdef BDB_REPORTING
  //Adds the default configuration values for the temperature attribute of the ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK cluster, for endpoint SAMPLETEMPERATURESENSOR_ENDPOINT
  //Default maxReportingInterval value is 10 seconds
  //Default minReportingInterval value is 3 seconds
  //Default reportChange value is 0x01 ()
  bdb_RepAddAttrCfgRecordDefaultToList(SAMPLEDOORLOCK_ENDPOINT, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK, ATTRID_CLOSURES_LOCK_STATE, 0, 10, reportableChange);
#endif
  
  zdpExternalStateTaskID = zclSampleDoorLock_TaskID;

  UI_Init(zclSampleDoorLock_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, SAMPLEAPP_KEY_AUTO_REPEAT_EVT, &zclSampleDoorLock_IdentifyTime, APP_TITLE, &zclSampleDoorLock_UiAppUpdateLcd, zclSampleDoorLock_UiAppStatesMain);

  zclSampleDoorLock_UpdateLedState();
  UI_UpdateLcd();
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclSampleDoorLock_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclSampleDoorLock_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
#ifdef MT_APP_FUNC
        case MT_SYS_APP_MSG:
          // Message received from MT
          zclSampleDoorLock_ProcessAppMsg( ((mtSysAppMsg_t *)MSGpkt)->endpoint,
                                          ((mtSysAppMsg_t *)MSGpkt)->appDataLen,
                                          ((mtSysAppMsg_t *)MSGpkt)->appData );
          break;
#endif
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          if ( ((zclIncomingMsg_t *)MSGpkt)->attrCmd )
          {
            osal_mem_free( ((zclIncomingMsg_t *)MSGpkt)->attrCmd );
          }
          break;

        case KEY_CHANGE:
          zclSampleDoorLock_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          UI_DeviceStateUpdated((devStates_t)(MSGpkt->hdr.status));
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
  
  if ( events & SAMPLEDOORLOCK_PIN_SAVE_TIMEOUT )
  {
    // resets the pin screen
    giDoorLockPINColumnCount = 0;
    
    // force ui update
    UI_UpdateLcd();
    
    return ( events ^ SAMPLEDOORLOCK_PIN_SAVE_TIMEOUT );
  }
  
#if ZG_BUILD_ENDDEVICE_TYPE    
  if ( events & SAMPLEAPP_END_DEVICE_REJOIN_EVT )
  {
    bdb_ZedAttemptRecoverNwk();
    return ( events ^ SAMPLEAPP_END_DEVICE_REJOIN_EVT );
  }
#endif
  
  if ( events & SAMPLEAPP_LCD_AUTO_UPDATE_EVT )
  {
    UI_UpdateLcd();
    return ( events ^ SAMPLEAPP_LCD_AUTO_UPDATE_EVT );
  }

  if ( events & SAMPLEAPP_KEY_AUTO_REPEAT_EVT )
  {
    UI_MainStateMachine(UI_KEY_AUTO_PRESSED);
    return ( events ^ SAMPLEAPP_KEY_AUTO_REPEAT_EVT );
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      zclSampleDoorLock_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zclSampleDoorLock_HandleKeys( byte shift, byte keys )
{
  UI_MainStateMachine(keys);
}

#ifdef MT_APP_FUNC
/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessAppMsg
 *
 * @brief   Process DoorLock messages
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   len - number of bytes
 * @param   msg - pointer to message
 *          0 - lo byte destination address
 *          1 - hi byte destination address
 *          2 - destination endpoint
 *          3 - lo byte cluster ID
 *          4 - hi byte cluster ID
 *          5 - message length
 *          6 - destination address mode (first byte of data)
 *          7 - zcl command frame
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessAppMsg( uint8 srcEP, uint8 len, uint8 *msg )
{
  afAddrType_t dstAddr;
  uint16 clusterID;
  zclFrameHdr_t hdr;
  uint8 *pData;
  uint8 dataLen;

  dstAddr.addr.shortAddr = BUILD_UINT16( msg[0], msg[1] );
  msg += 2;
  dstAddr.endPoint = *msg++;
  clusterID = BUILD_UINT16( msg[0], msg[1] );
  msg += 2;
  dataLen = *msg++; // Length of message (Z-Tool can support up to 255 octets)
  dstAddr.addrMode = (afAddrMode_t)(*msg++);
  dataLen--; // Length of ZCL frame

  // Begining of ZCL frame
  pData = zclParseHdr( &hdr, msg );
  dataLen -= (uint8)( pData - msg );

  // Is this a foundation type message?
  if ( zcl_ProfileCmd( hdr.fc.type ) )
  {
    if ( hdr.fc.manuSpecific )
    {
      // We don't support any manufacturer specific command -- just forward it.
      zcl_SendCommand( srcEP, &dstAddr, clusterID, hdr.commandID, FALSE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr.fc.disableDefaultRsp, hdr.manuCode, hdr.transSeqNum, dataLen, pData );
    }
    else
    {
      zclParseCmd_t cmd;

      cmd.endpoint = srcEP;
      cmd.dataLen = dataLen;
      cmd.pData = pData;

      zclSampleDoorLock_ProcessFoundationMsg( &dstAddr, clusterID, &hdr, &cmd );
    }
  }
  else
  {
    // Nope, must be specific to the cluster ID
    if ( hdr.fc.manuSpecific )
    {
      // We don't support any manufacturer specific command -- just forward it.
      zcl_SendCommand( srcEP, &dstAddr, clusterID, hdr.commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr.fc.disableDefaultRsp, hdr.manuCode, hdr.transSeqNum, dataLen, pData );
    }
    else
    {
      if ( ZCL_CLUSTER_ID_GEN( clusterID ) )
      {
        zclSampleDoorLock_ProcessGeneralMsg( srcEP, &dstAddr, clusterID, &hdr, dataLen, pData );
      }
      else if ( ZCL_CLUSTER_ID_CLOSURES( clusterID ) )
      {
        zclSampleDoorLock_ProcessClosuresMsg( srcEP, &dstAddr, clusterID, &hdr, dataLen, pData );
      }
    }
  }
}

/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessFoundationMsg
 *
 * @brief   Process Foundation message
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   clusterID - real cluster ID
 * @param   hdr - pointer to the message header
 * @param   len - length of the received message
 * @param   data - received message
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessFoundationMsg( afAddrType_t *dstAddr, uint16 clusterID,
                                                zclFrameHdr_t *hdr, zclParseCmd_t *pParseCmd )
{
#if defined(ZCL_READ) || defined(ZCL_WRITE) || defined(ZCL_REPORT) || defined(ZCL_DISCOVER)
  void *cmd;
#endif

  switch ( hdr->commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ:
      cmd = zclParseInReadCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendRead( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclReadCmd_t *)cmd,
                      ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;
#endif // ZCL_READ

#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWrite( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                       ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_WRITE_UNDIVIDED:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWriteUndivided( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                                ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_WRITE_NO_RSP:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWriteNoRsp( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                            ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;
#endif // ZCL_WRITE

#ifdef ZCL_REPORT
    case ZCL_CMD_CONFIG_REPORT:
      cmd = zclParseInConfigReportCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendConfigReportCmd( SAMPLEDOORLOCK_ENDPOINT, dstAddr,  clusterID, (zclCfgReportCmd_t *)cmd,
                                 ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      cmd = zclParseInReadReportCfgCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendReadReportCfgCmd( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclReadReportCfgCmd_t *)cmd,
                                  ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_REPORT:
      cmd = zclParseInReportCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendReportCmd( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclReportCmd_t *)cmd,
                           ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;
#endif // ZCL_REPORT
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_ATTRS:
      cmd = zclParseInDiscAttrsCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendDiscoverAttrsCmd( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID, (zclDiscoverAttrsCmd_t *)cmd,
                                  ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;
#endif // ZCL_DISCOVER

    default:
      // Unsupported command -- just forward it.
      zcl_SendCommand( pParseCmd->endpoint, dstAddr, clusterID, hdr->commandID, FALSE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, pParseCmd->dataLen, pParseCmd->pData );
      break;
  }
}

/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessGeneralMsg
 *
 * @brief   Process General Cluster message
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   clusterID - real cluster ID
 * @param   hdr - pointer to the message header
 * @param   len - length of the received message
 * @param   data - received message
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessGeneralMsg( uint8 srcEP, afAddrType_t *dstAddr,
                  uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data )
{
  switch ( clusterID )
  {
#ifdef ZCL_BASIC
    case ZCL_CLUSTER_ID_GEN_BASIC:
      if ( hdr->commandID == COMMAND_BASIC_RESET_FACT_DEFAULT )
      {
        zclGeneral_SendBasicResetFactoryDefaults( srcEP, dstAddr, hdr->fc.disableDefaultRsp,
                                                  hdr->transSeqNum );
      }
      break;
#endif // ZCL_BASIC

#ifdef ZCL_GROUPS
    case ZCL_CLUSTER_ID_GEN_GROUPS:
      zclSampleDoorLock_ProcessGroupCmd( srcEP, dstAddr, clusterID, hdr, len, data );
      break;
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
    case ZCL_CLUSTER_ID_GEN_SCENES:
      zclSampleDoorLock_ProcessSceneCmd( srcEP, dstAddr, clusterID, hdr, len, data  );
      break;
#endif // ZCL_SCENES

    default:
      break;
  }
}

#ifdef ZCL_GROUPS
/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessGroupCmd
 *
 * @brief   Process Group Command
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   clusterID - real cluster ID
 * @param   hdr - pointer to the message header
 * @param   len - length of the received message
 * @param   data - received message
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessGroupCmd( uint8 srcEP, afAddrType_t *dstAddr,
                  uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data )
{
  uint16 groupID;
  uint8 grpCnt;
  uint16 *grpList;

  switch ( hdr->commandID )
  {
    case COMMAND_GROUP_ADD:
      groupID =  BUILD_UINT16( data[0], data[1] );
      data += 2;
      zclGeneral_SendGroupAdd( srcEP, dstAddr, groupID, data,
                               hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_GROUP_VIEW:
      groupID =  BUILD_UINT16( data[0], data[1] );
      zclGeneral_SendGroupView( srcEP, dstAddr, groupID,
                                hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_GROUP_GET_MEMBERSHIP:
      grpCnt = data[0];
      grpList = (uint16 *)(&data[1]);
      zclGeneral_SendGroupGetMembership( srcEP, dstAddr, grpCnt, grpList,
                                         hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_GROUP_REMOVE:
      groupID =  BUILD_UINT16( data[0], data[1] );
      zclGeneral_SendGroupRemove( srcEP,  dstAddr, groupID,
                                  hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_GROUP_REMOVE_ALL:
       zclGeneral_SendGroupRemoveAll( srcEP, dstAddr, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_GROUP_ADD_IF_IDENTIFYING:
      groupID =  BUILD_UINT16( data[0], data[1] );
      data += 2;
      zclGeneral_SendGroupAddIfIdentifying( srcEP, dstAddr, groupID, data,
                                            hdr->fc.disableDefaultRsp,  hdr->transSeqNum );
      break;

    default:
      // Unsupported command -- just forward it.
      zcl_SendCommand( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID,
                       hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
      break;
  }
}
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessSceneCmd
 *
 * @brief   Process Scene Command
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   clusterID - real cluster ID
 * @param   hdr - pointer to the message header
 * @param   len - length of the received message
 * @param   data - received message
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessSceneCmd( uint8 srcEP, afAddrType_t *dstAddr,
                  uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data )
{
  zclGeneral_Scene_t scene;
  uint8 *pData = data;
  uint8 nameLen;

  osal_memset( (uint8*)&scene, 0, sizeof( zclGeneral_Scene_t ) );

  scene.groupID = BUILD_UINT16( pData[0], pData[1] );
  pData += 2;   // Move past group ID
  scene.ID = *pData++;

  switch ( hdr->commandID )
  {
    case COMMAND_SCENE_ADD:
      // Parse the rest of the incoming message
      scene.transTime = BUILD_UINT16( pData[0], pData[1] );
      pData += 2;
      nameLen = *pData++; // Name length
      if ( nameLen > (ZCL_SCENE_NAME_LEN-1) )
      {
        scene.name[0] = ZCL_SCENE_NAME_LEN-1;
      }
      else
      {
        scene.name[0] = nameLen;
      }
      osal_memcpy( &(scene.name[1]), pData, scene.name[0] );
      pData += nameLen; // move past name, use original length

      // Add the extension field(s)
      scene.extLen = len - ( (uint8)( pData - data ) );
      if ( scene.extLen > 0 )
      {
        // Copy the extention field(s)
        if ( scene.extLen > ZCL_GEN_SCENE_EXT_LEN )
        {
          scene.extLen = ZCL_GEN_SCENE_EXT_LEN;
        }
        osal_memcpy( scene.extField, pData, scene.extLen );
      }

      zclGeneral_SendAddScene( srcEP, dstAddr, &scene,
                               hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_SCENE_VIEW:
      zclGeneral_SendSceneView( srcEP, dstAddr, scene.groupID, scene.ID,
                                hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_SCENE_REMOVE:
      zclGeneral_SendSceneRemove( srcEP, dstAddr, scene.groupID, scene.ID,
                                  hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_SCENE_REMOVE_ALL:
      zclGeneral_SendSceneRemoveAll( srcEP, dstAddr, scene.groupID,
                                     hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_SCENE_STORE:
      zclGeneral_SendSceneStore( srcEP, dstAddr, scene.groupID, scene.ID,
                                 hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_SCENE_RECALL:
      zclGeneral_SendSceneRecall( srcEP, dstAddr, scene.groupID, scene.ID,
                                  hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    case COMMAND_SCENE_GET_MEMBERSHIP:
      zclGeneral_SendSceneGetMembership( srcEP, dstAddr, scene.groupID,
                                         hdr->fc.disableDefaultRsp, hdr->transSeqNum );
      break;

    default:
      // Unsupported command -- just forward it.
      zcl_SendCommand( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID,
                       hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
      break;
  }
}
#endif // ZCL_SCENES

/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessClosuresMsg
 *
 * @brief   Process Closures Cluster Command
 *
 * @param   srcEP - Sending Apps endpoint
 * @param   dstAddr - where to send the request
 * @param   clusterID - real cluster ID
 * @param   hdr - pointer to the message header
 * @param   len - length of the received message
 * @param   data - received message
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessClosuresMsg( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint16 clusterID, zclFrameHdr_t *hdr,
                                                  uint8 len, uint8 *data )
{
#ifdef ZCL_DOORLOCK

  uint8 i;
  uint16 calculatedArraySize;

  // Client-to-Server
  if ( zcl_ServerCmd( hdr->fc.direction ) )
  {
    switch( hdr->commandID )
    {
      case COMMAND_CLOSURES_TOGGLE_DOOR:
      {
        zclDoorLock_t cmd;

        // first octet of PIN/RFID Code variable string identifies its length
        calculatedArraySize = data[0] + 1;

        cmd.pPinRfidCode = osal_mem_alloc( calculatedArraySize );
        if( !cmd.pPinRfidCode )
        {
          return;  // no memory
        }

        for( i = 0; i < calculatedArraySize; i++ )
        {
          cmd.pPinRfidCode[i] = data[i];
        }

        zclClosures_SendDoorLockToggleDoor( srcEP, dstAddr, &cmd, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd.pPinRfidCode );
        break;
      }

      default:
        // Unsupported command -- just forward it.
        zcl_SendCommand( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID,
                         hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                         hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
        break;
    }
  }
  // Server-to-Client
  else
  {
    switch( hdr->commandID )
    {
      case COMMAND_CLOSURES_TOGGLE_DOOR_RSP:
        zclClosures_SendDoorLockToggleDoorRsp( srcEP, dstAddr, data[0], hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        break;

      default:
        // Unsupported command -- just forward it.
        zcl_SendCommand( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID,
                         hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                         hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
        break;
    }
  }

#else
  // Unsupported command -- just forward it.
  zcl_SendCommand( SAMPLEDOORLOCK_ENDPOINT, dstAddr, clusterID,
                   hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                   hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
#endif // ZCL_DOORLOCK
}
#endif // MT_APP_FUNC

/*********************************************************************
 * @fn      zclSampleDoorLock_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLock_BasicResetCB( void )
{
  zclSampleDoorLock_ResetAttributesToDefaultValues();
  
  UI_UpdateLcd();
  zclSampleDoorLock_UpdateLedState();
}

/*********************************************************************
 * @fn      zclSampleApp_BatteryWarningCB
 *
 * @brief   Called to handle battery-low situation.
 *
 * @param   voltLevel - level of severity
 *
 * @return  none
 */
void zclSampleApp_BatteryWarningCB( uint8 voltLevel )
{
  if ( voltLevel == VOLT_LEVEL_CAUTIOUS )
  {
    // Send warning message to the gateway and blink LED
  }
  else if ( voltLevel == VOLT_LEVEL_BAD )
  {
    // Shut down the system
  }
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclSampleDoorLock_DoorLockCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Door Lock cluster Command for this application.
 *
 * @param   pInMsg - process incoming message
 * @param   pInCmd - PIN/RFID code of command
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSampleDoorLock_DoorLockCB ( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd )
{
  ZStatus_t rc = ZInvalidParameter;
  uint8 newDoorLockState;

  if (  osal_memcmp( aiDoorLockMasterPINCode, pInCmd->pPinRfidCode, 5 ) == TRUE )
  {
    // Lock the door
    if ( pInMsg->hdr.commandID == COMMAND_CLOSURES_LOCK_DOOR )
    {
      newDoorLockState = CLOSURES_LOCK_STATE_LOCKED;
    }
    // Unlock the door
    else if ( pInMsg->hdr.commandID == COMMAND_CLOSURES_UNLOCK_DOOR )
    {
      newDoorLockState = CLOSURES_LOCK_STATE_UNLOCKED;
    }
    // Toggle the door
    else if ( pInMsg->hdr.commandID == COMMAND_CLOSURES_TOGGLE_DOOR )
    {
      if (zclSampleDoorLock_LockState == CLOSURES_LOCK_STATE_LOCKED)
      {
        newDoorLockState = CLOSURES_LOCK_STATE_UNLOCKED;
      }
      else
      {
        newDoorLockState = CLOSURES_LOCK_STATE_LOCKED;
      }
    }
    else
    {
      return ( ZCL_STATUS_FAILURE );  // invalid command
    }
    
    rc = zclSampleDoorLock_DoorLockActuator( newDoorLockState );

    zclClosures_SendDoorLockStatusResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                              pInMsg->hdr.commandID,
                                              rc, TRUE, pInMsg->hdr.transSeqNum ); // ZCL_STATUS_SUCCESS and ZCL_STATUS_FAILURE share the values of ZSuccess and ZFailure, respectively.


    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
  else
  {
    // incorrect PIN received
  
    invalidPin = TRUE;

    zclClosures_SendDoorLockStatusResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                            pInMsg->hdr.commandID,
                                            ZCL_STATUS_INVALID_VALUE, FALSE, pInMsg->hdr.transSeqNum );
    
    zclSampleDoorLock_UpdateLedState();
    UI_UpdateLcd();
    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
}

/*********************************************************************
 * @fn      zclSampleDoorLock_DoorLockRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Door Lock response for this application.
 *
 * @param   cmd - Command ID
 * @param   srcAddr - Requestor's address
 * @param   transSeqNum - Transaction sequence number
 * @param   status - status response from server's door lock cmd
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSampleDoorLock_DoorLockRspCB ( zclIncoming_t *pInMsg, uint8 status )
{
  return ( ZCL_STATUS_SUCCESS );
}

static ZStatus_t zclSampleDoorLock_DoorLockActuator ( uint8 newDoorLockState )
{
  if ( newDoorLockState == zclSampleDoorLock_LockState )
  {
    return ZFailure;
  }
    
  zclSampleDoorLock_LockState = newDoorLockState;
  
#ifdef BDB_REPORTING      
  bdb_RepChangedAttrValue(SAMPLEDOORLOCK_ENDPOINT, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK, ATTRID_CLOSURES_LOCK_STATE);
#endif
  
  zclSampleDoorLock_UpdateLedState();
  UI_UpdateLcd();

  return ZSuccess;
}



static void zclSampleDoorLock_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
{
    switch(bdbCommissioningModeMsg->bdbCommissioningMode)
    {
      case BDB_COMMISSIONING_FORMATION:
        if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
        {
          //After formation, perform nwk steering again plus the remaining commissioning modes that has not been process yet
          bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
        }
        else
        {
          //Want to try other channels?
          //try with bdb_setChannelAttribute
        }
      break;
      case BDB_COMMISSIONING_NWK_STEERING:
        if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
        {
          //YOUR JOB:
          //We are on the nwk, what now?
        }
        else
        {
          //See the possible errors for nwk steering procedure
          //No suitable networks found
          //Want to try other channels?
          //try with bdb_setChannelAttribute
        }
      break;
      case BDB_COMMISSIONING_FINDING_BINDING:
        if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
        {
          //YOUR JOB:
        }
        else
        {
          //YOUR JOB:
          //retry?, wait for user interaction?
        }
      break;
      case BDB_COMMISSIONING_INITIALIZATION:
        //Initialization notification can only be successful. Failure on initialization 
        //only happens for ZED and is notified as BDB_COMMISSIONING_PARENT_LOST notification
        
        //YOUR JOB:
        //We are on a network, what now?
        
      break;
#if ZG_BUILD_ENDDEVICE_TYPE    
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        //We did recover from losing parent
      }
      else
      {
        //Parent not found, attempt to rejoin again after a fixed delay
        osal_start_timerEx(zclSampleDoorLock_TaskID, SAMPLEAPP_END_DEVICE_REJOIN_EVT, SAMPLEAPP_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
    }

  UI_UpdateComissioningStatus(bdbCommissioningModeMsg);
}



void zclSampleDoorLock_UiActionChangePin(uint16 keys)
{
  uint8 numBuff;    // used to convert decimal to ASCII
  
  if ( keys & HAL_KEY_SW_1 )
  {
    if ( giDoorLockPINColumnCount < 4 )
    {
      if(giDoorLockPINCount > 8)
      {
        giDoorLockPINCount = 0;
      }
      else
      {
        giDoorLockPINCount++;
      }
    }
  }
  
  if ( keys & HAL_KEY_SW_3 )
  {
    if ( giDoorLockPINColumnCount < 4 )
    {
      if ( giDoorLockPINCount < 1 )
      {
        giDoorLockPINCount = 9;
      }
      else
      {
        giDoorLockPINCount--;
      }
    }
  }
  
  if ( keys & HAL_KEY_SW_5 )
  {
    if ( giDoorLockPINCount > 0 )
    {
      _itoa(giDoorLockPINCount, &numBuff, 10);  // convert number to ASCII

      aiDoorLockMasterPINCode[giDoorLockPINColumnCount + 1] = numBuff;   // copy current PIN number
    }
    else
    {
      // make sure '0' is copied into PIN variable
      aiDoorLockMasterPINCode[giDoorLockPINColumnCount + 1] = 0x30;   // ASCII '0'
    }

    if(giDoorLockPINColumnCount < 3)
    {
      giDoorLockPINColumnCount++;   // adjust PIN column
    }
    else if ( giDoorLockPINColumnCount == 3 )
    {
      giDoorLockPINColumnCount = 4;   // hold here until PIN screen reset
      giDoorLockPINCount = 0;   // reset PIN count

      // store PIN to NVM
      osal_nv_write( ZCD_NV_APS_DOORLOCK_PIN, 0, 5, aiDoorLockMasterPINCode );
      osal_start_timerEx( zclSampleDoorLock_TaskID, SAMPLEDOORLOCK_PIN_SAVE_TIMEOUT, 2000 );
    }
    else {
      // immediately reset pin screen
      osal_stop_timerEx( zclSampleDoorLock_TaskID, SAMPLEDOORLOCK_PIN_SAVE_TIMEOUT);
      osal_set_event( zclSampleDoorLock_TaskID, SAMPLEDOORLOCK_PIN_SAVE_TIMEOUT);
    }
  }
  
}

void zclSampleDoorLock_UiActionLockUnlock(uint16 keys)
{
  if ( keys & HAL_KEY_SW_1 )
  {
    zclSampleDoorLock_DoorLockActuator(CLOSURES_LOCK_STATE_LOCKED);
  }
  
  if ( keys & HAL_KEY_SW_3 )
  {
    zclSampleDoorLock_DoorLockActuator(CLOSURES_LOCK_STATE_UNLOCKED);
  }
}

void zclSampleDoorLock_UpdateLedState(void)
{
  if ( zclSampleDoorLock_LockState == CLOSURES_LOCK_STATE_LOCKED )
  {
    HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
  }
  else if ( zclSampleDoorLock_LockState == CLOSURES_LOCK_STATE_UNLOCKED )
  {
    HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF ); 
  }
}

void zclSampleDoorLock_UiAppUpdateLcd(uint8 uiCurrentState, char * line[3])
{
  switch(uiCurrentState)
  {
    case UI_CHANGE_PIN:
    {
      uint8 i;
      
      uiConstStrCpy(line[1], "Enter PIN:      ");
      
      for (i = 0; i < giDoorLockPINColumnCount; i++)
      {
        line[1][11 + i] = '*';
      }
      
      if (giDoorLockPINColumnCount < 4)
      {
        _ltoa(giDoorLockPINCount, (void *)(&line[1][11 + giDoorLockPINColumnCount]), 10);
      }
      else
      {
        line[0] = (char *)sStorePIN;
      }

      line[1] = (char *)line[1];
      line[2] = "<  CHANGE PIN  >";
      break;
    }
      
    case UI_TOGGLE_LOCK:
      if( invalidPin )
      {
        line[0] = (char *)sInvalidPIN;
        invalidPin = FALSE;
        osal_start_timerEx(zclSampleDoorLock_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, 2000);
      }
      else
      {
        line[0] = "^:Lock  v:Unlock";
      }
      
      if ( zclSampleDoorLock_LockState == CLOSURES_LOCK_STATE_LOCKED )
      {
        line[1] = (char *)sDoorLocked;
      }
      else if ( zclSampleDoorLock_LockState == CLOSURES_LOCK_STATE_UNLOCKED )
      {
        line[1] = (char *)sDoorUnlocked;
      }
      
      line[2] = "< TOGGLE LOCK  >";
      break;
      
    default:
      break;
  }
}

/****************************************************************************
****************************************************************************/
