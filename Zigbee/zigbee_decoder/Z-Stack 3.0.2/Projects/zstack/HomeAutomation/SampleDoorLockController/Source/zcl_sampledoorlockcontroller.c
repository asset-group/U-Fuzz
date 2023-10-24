/**************************************************************************************************
  Filename:       zcl_sampledoorlockcontroller.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $

  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
  This application implements a ZigBee Door Lock Controller, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - LEDs:
    LED1 reflects the current door lock state (locked = on)

  Application-specific menu system:

    <CHANGE PIN> Change the PIN used in the Lock/Unlock command
      Up/Down changes the value of the current digit, OK/Select sets the current digit
      This screen shows the following information:
        Line1:
          After entering all 4 pin digits, briefly shows "PIN SAVED" message
        Line2:
          Shows the Enter PIN prompt with the current PIN digit visible

    <TOGGLE LOCK> Send a Lock/Unlock command to the remote door
      Up sends a lock command, Down sends an unlock command
      This screen shows the following information:
        Line1:
          Shows instructions normally, displays Lock/Unlock Failure message
          when applicable
        Line2:
          Shows the current lock state, as reported by the remote door

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "MT.h"
#include "MT_APP.h"
#include "OSAL_Nv.h"
#include "MT_SYS.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_closures.h"

#include "zcl_sampledoorlockcontroller.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include "bdb_interface.h"

#include "zcl_sampleapps_ui.h"

/*********************************************************************
 * MACROS
 */

// There is no attribute in the Mandatory Reportable Attribute list for now
#define zcl_MandatoryReportableAttribute( a ) ( a == NULL )

// BACnet Device ID of 0x3FFFFF indicates that the associated BACnet device
// is not commissioned.
#define PI_BACNET_COMMISSIONED( addr )        ( ( addr[0] == 3 )  && \
                                                ( addr[1] != 0xff || \
                                                  addr[2] != 0xff || \
                                                  addr[3] != 0x3f ) )

// 11073 System ID of zero indicates that the associated 11073 device
// is not commissioned.
#define PI_11073_COMMISSIONED( addr )         ( ( addr[0] == Z_EXTADDR_LEN ) && \
                                                ( addr[1] != 0x00 || \
                                                  addr[2] != 0x00 || \
                                                  addr[3] != 0x00 || \
                                                  addr[4] != 0x00 || \
                                                  addr[5] != 0x00 || \
                                                  addr[6] != 0x00 || \
                                                  addr[7] != 0x00 || \
                                                  addr[8] != 0x00 ) )

#define UI_CHANGE_PIN    1         
#define UI_TOGGLE_LOCK   2

#define APP_TITLE " DoorLock Cntrl "

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclSampleDoorLockController_TaskID;

extern int16 zdpExternalStateTaskID;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

afAddrType_t zclSampleDoorLockController_DstAddr;

uint8 giDoorLockPINCount = 0;

uint8 giDoorLockPINColumnCount = 0;

uint8 aiDoorLockPIN[] = {4, 0x31, 0x32, 0x33, 0x34};   // default PIN (size 4) of 1,2,3,4 (ASCII)

// Test Endpoint to allow SYS_APP_MSGs
static endPointDesc_t sampleDoorLockController_TestEp =
{
  SAMPLEDOORLOCKCONTROLLER_ENDPOINT,                                 // Test endpoint
  0,
  &zclSampleDoorLockController_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

static bool remoteDoorIsLocked = TRUE;
static bool cmdFailure = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleDoorLockController_HandleKeys( byte shift, byte keys );
static void zclSampleDoorLockController_BasicResetCB( void );

static void zclSampleDoorLockController_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);

#ifdef MT_APP_FUNC
static void zclSampleDoorLockController_ProcessAppMsg( uint8 srcEP, uint8 len, uint8 *msg );
static void zclSampleDoorLockController_ProcessFoundationMsg( afAddrType_t *dstAddr, uint16 clusterID,
                                                              zclFrameHdr_t *hdr, zclParseCmd_t *pParseCmd );
static void zclSampleDoorLockController_ProcessGeneralMsg( uint8 srcEP, afAddrType_t *dstAddr,
                                                           uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
static void zclSampleDoorLockController_ProcessGroupCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                         uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
static void zclSampleDoorLockController_ProcessSceneCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                         uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
static void zclSampleDoorLockController_ProcessClosuresMsg( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint16 clusterID, zclFrameHdr_t *hdr, uint8 len, uint8 *data );
#endif // MT_APP_FUNC

// Functions to process ZCL Foundation incoming Command/Response messages
static void zclSampleDoorLockController_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#if (! defined ZCL_REPORT_DESTINATION_DEVICE) || (defined MT_APP_FUNC)
#ifdef ZCL_READ
static uint8 zclSampleDoorLockController_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#endif
#ifdef MT_APP_FUNC
#ifdef ZCL_WRITE
static uint8 zclSampleDoorLockController_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#endif

static uint8 zclSampleDoorLockController_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_REPORT_DESTINATION_DEVICE
static void zclSampleDoorLockController_ProcessInReportCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_DISCOVER
static uint8 zclSampleDoorLockController_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
#endif

static ZStatus_t zclSampleDoorLockController_DoorLockRspCB ( zclIncoming_t *pInMsg, uint8 status );

static void zclSampleDoorLockController_UiActionChangePin(uint16 keys);
static void zclSampleDoorLockController_UiActionSendLockUnlock(uint16 keys);
void zclSampleDoorLockController_UiAppUpdateLcd(uint8 uiCurrentState, char * line[3]);
void zclSampleDoorLockController_UpdateLedState(void);

static void zclSampleApp_BatteryWarningCB( uint8 voltLevel);

/*********************************************************************
 * STATUS STRINGS
 */
#ifdef LCD_SUPPORTED
const char sStorePIN[]     = "   PIN SAVED";
const char sFail[]         = "Lock/Unlock Fail";
const char sDoorLocked[]   = "  Door Locked";
const char sDoorUnlocked[] = "  Door Unlocked";
#endif

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleDoorLockController_CmdCallbacks =
{
  zclSampleDoorLockController_BasicResetCB,           // Basic Cluster Reset command
  NULL,                                               // Identify Trigger Effect command
  NULL,                                               // On/Off cluster commands
  NULL,                                               // On/Off cluster enhanced command Off with Effect
  NULL,                                               // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                               // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                               // Level Control Move to Level command
  NULL,                                               // Level Control Move command
  NULL,                                               // Level Control Step command
  NULL,                                               // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                               // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                               // Scene Store Request command
  NULL,                                               // Scene Recall Request command
  NULL,                                               // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                               // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                               // Get Event Log command
  NULL,                                               // Publish Event Log command
#endif
  NULL,                                               // RSSI Location command
  NULL                                                // RSSI Location Response command
};

/*********************************************************************
 * ZCL Closure cluster Callback table
 */
static zclClosures_DoorLockAppCallbacks_t zclSampleDoorLockController_DoorLockCmdCallbacks =
{
  NULL,                                                  // DoorLock cluster command
  zclSampleDoorLockController_DoorLockRspCB,             // DoorLock Response
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

const uiState_t zclSampleDoorLockController_UiAppStatesMain[] =
{
  /*  UI_STATE_BACK_FROM_APP_MENU  */   {UI_STATE_DEFAULT_MOVE, UI_TOGGLE_LOCK, UI_KEY_SW_5_PRESSED, &UI_ActionBackFromAppMenu}, //do not change this line, except for the second item, which should point to the last entry in this menu
  /*  UI_CHANGE_PIN          */   {UI_STATE_DEFAULT_MOVE, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED, &zclSampleDoorLockController_UiActionChangePin},
  /*  UI_TOGGLE_LOCK         */   {UI_STATE_BACK_FROM_APP_MENU, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED, &zclSampleDoorLockController_UiActionSendLockUnlock},
};


/*********************************************************************
 * @fn          zclSampleDoorLockController_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclSampleDoorLockController_Init( byte task_id )
{
  zclSampleDoorLockController_TaskID = task_id;

  // Set destination address to indirect
  zclSampleDoorLockController_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleDoorLockController_DstAddr.endPoint = 0;
  zclSampleDoorLockController_DstAddr.addr.shortAddr = 0;

  // Register the Simple Descriptor for this application
  bdb_RegisterSimpleDescriptor( &zclSampleDoorLockController_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_CmdCallbacks );

  // Register the application's attribute list
  zclSampleDoorLockController_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, zclSampleDoorLockController_NumAttributes, zclSampleDoorLockController_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclSampleDoorLockController_TaskID );

  // Register low voltage NV memory protection application callback
  RegisterVoltageWarningCB( zclSampleApp_BatteryWarningCB );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclSampleDoorLockController_TaskID );
  
  bdb_RegisterCommissioningStatusCB( zclSampleDoorLockController_ProcessCommissioningStatus );

  //Register the ZCL DoorLock Cluster Library callback function
  zclClosures_RegisterDoorLockCmdCallbacks( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DoorLockCmdCallbacks );

  // Register for a test endpoint
  afRegister( &sampleDoorLockController_TestEp );

  // initialize NVM for storing PIN information
  if ( SUCCESS == osal_nv_item_init( ZCD_NV_APS_DOORLOCK_PIN, 5, aiDoorLockPIN ) )
    // use NVM PIN number in APP
    osal_nv_read( ZCD_NV_APS_DOORLOCK_PIN, 0, 5, aiDoorLockPIN );
  
  zdpExternalStateTaskID = zclSampleDoorLockController_TaskID;

  UI_Init(zclSampleDoorLockController_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, SAMPLEAPP_KEY_AUTO_REPEAT_EVT, &zclSampleDoorLockController_IdentifyTime, APP_TITLE, &zclSampleDoorLockController_UiAppUpdateLcd, zclSampleDoorLockController_UiAppStatesMain);

  zclSampleDoorLockController_UpdateLedState();
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
uint16 zclSampleDoorLockController_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclSampleDoorLockController_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
#ifdef MT_APP_FUNC
        case MT_SYS_APP_MSG:
          // Message received from MT
          zclSampleDoorLockController_ProcessAppMsg( ((mtSysAppMsg_t *)MSGpkt)->endpoint,
                                                     ((mtSysAppMsg_t *)MSGpkt)->appDataLen,
                                                     ((mtSysAppMsg_t *)MSGpkt)->appData );
          break;
#endif
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclSampleDoorLockController_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclSampleDoorLockController_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
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
  
  if ( events & SAMPLEDOORLOCKCONTROLLER_PIN_SAVE_TIMEOUT )
  {
    // resets the pin screen
    giDoorLockPINColumnCount = 0;
    
    // force ui update
    UI_UpdateLcd();
    
    return ( events ^ SAMPLEDOORLOCKCONTROLLER_PIN_SAVE_TIMEOUT );
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
 * @fn      zclSampleDoorLockController_HandleKeys
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
static void zclSampleDoorLockController_HandleKeys( byte shift, byte keys )
{
  UI_MainStateMachine(keys);
}


static void zclSampleDoorLockController_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
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
        osal_start_timerEx(zclSampleDoorLockController_TaskID, SAMPLEAPP_END_DEVICE_REJOIN_EVT, SAMPLEAPP_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
  }
  
  UI_UpdateComissioningStatus(bdbCommissioningModeMsg);
}

#ifdef MT_APP_FUNC
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessAppMsg
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
static void zclSampleDoorLockController_ProcessAppMsg( uint8 srcEP, uint8 len, uint8 *msg )
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

      zclSampleDoorLockController_ProcessFoundationMsg( &dstAddr, clusterID, &hdr, &cmd );
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
        zclSampleDoorLockController_ProcessGeneralMsg( srcEP, &dstAddr, clusterID, &hdr, dataLen, pData );
      }
      else if ( ZCL_CLUSTER_ID_CLOSURES( clusterID ) )
      {
        zclSampleDoorLockController_ProcessClosuresMsg( srcEP, &dstAddr, clusterID, &hdr, dataLen, pData );
      }
    }
  }
}

/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessFoundationMsg
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
static void zclSampleDoorLockController_ProcessFoundationMsg( afAddrType_t *dstAddr, uint16 clusterID,
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
        zcl_SendRead( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID, (zclReadCmd_t *)cmd,
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
        zcl_SendWrite( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                       ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_WRITE_UNDIVIDED:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWriteUndivided( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                                ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_WRITE_NO_RSP:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWriteNoRsp( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
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
        zcl_SendConfigReportCmd( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr,  clusterID, (zclCfgReportCmd_t *)cmd,
                                 ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      cmd = zclParseInReadReportCfgCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendReadReportCfgCmd( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID, (zclReadReportCfgCmd_t *)cmd,
                                  ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_REPORT:
      cmd = zclParseInReportCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendReportCmd( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID, (zclReportCmd_t *)cmd,
                           ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;
#endif // ZCL_REPORT

    default:
      // Unsupported command -- just forward it.
      zcl_SendCommand( pParseCmd->endpoint, dstAddr, clusterID, hdr->commandID, FALSE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, pParseCmd->dataLen, pParseCmd->pData );
      break;
  }
}

/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessGeneralMsg
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
static void zclSampleDoorLockController_ProcessGeneralMsg( uint8 srcEP, afAddrType_t *dstAddr,
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
      zclSampleDoorLockController_ProcessGroupCmd( srcEP, dstAddr, clusterID, hdr, len, data );
      break;
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
    case ZCL_CLUSTER_ID_GEN_SCENES:
      zclSampleDoorLockController_ProcessSceneCmd( srcEP, dstAddr, clusterID, hdr, len, data  );
      break;
#endif // ZCL_SCENES

    default:
      break;
  }
}

#ifdef ZCL_GROUPS
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessGroupCmd
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
static void zclSampleDoorLockController_ProcessGroupCmd( uint8 srcEP, afAddrType_t *dstAddr,
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
      zcl_SendCommand( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID,
                       hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
      break;
  }
}
#endif // ZCL_GROUPS

#ifdef ZCL_SCENES
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessSceneCmd
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
static void zclSampleDoorLockController_ProcessSceneCmd( uint8 srcEP, afAddrType_t *dstAddr,
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
      zcl_SendCommand( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID,
                       hdr->commandID, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                       hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
      break;
  }
}
#endif // ZCL_SCENES

/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessClosuresMsg
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
static void zclSampleDoorLockController_ProcessClosuresMsg( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint16 clusterID, zclFrameHdr_t *hdr,
                                                            uint8 len, uint8 *data )
{
#ifdef ZCL_DOORLOCK

  uint8 i;
  uint16 calculatedArraySize;

  if ( clusterID == ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK )
  {
    // Client-to-Server
    if ( zcl_ServerCmd( hdr->fc.direction ) )
    {
      switch( hdr->commandID )
      {
        case COMMAND_CLOSURES_LOCK_DOOR:
        {
          zclDoorLock_t cmd;

          // first octet of PIN/RFID Code variable string identifies its length
          calculatedArraySize = data[0] + 1;

          cmd.pPinRfidCode = osal_mem_alloc( calculatedArraySize );
          if ( !cmd.pPinRfidCode )
          {
            return;  // no memory
          }

          for ( i = 0; i < calculatedArraySize; i++ )
          {
            cmd.pPinRfidCode[i] = data[i];
          }

          zclClosures_SendDoorLockLockDoor( srcEP, dstAddr, &cmd, hdr->fc.disableDefaultRsp, hdr->transSeqNum );

          osal_mem_free( cmd.pPinRfidCode );
          break;
        }

        case COMMAND_CLOSURES_UNLOCK_DOOR:
        {
          zclDoorLock_t cmd;

          // first octet of PIN/RFID Code variable string identifies its length
          calculatedArraySize = data[0] + 1;

          cmd.pPinRfidCode = osal_mem_alloc( calculatedArraySize );
          if ( !cmd.pPinRfidCode )
          {
            return;  // no memory
          }

          for ( i = 0; i < calculatedArraySize; i++ )
          {
            cmd.pPinRfidCode[i] = data[i];
          }

          zclClosures_SendDoorLockUnlockDoor( srcEP, dstAddr, &cmd, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
          osal_mem_free( cmd.pPinRfidCode );
          break;
        }

        case COMMAND_CLOSURES_TOGGLE_DOOR:
        {
          zclDoorLock_t cmd;

          // first octet of PIN/RFID Code variable string identifies its length
          calculatedArraySize = data[0] + 1;

          cmd.pPinRfidCode = osal_mem_alloc( calculatedArraySize );
          if ( !cmd.pPinRfidCode )
          {
            return;  // no memory
          }

          for ( i = 0; i < calculatedArraySize; i++ )
          {
            cmd.pPinRfidCode[i] = data[i];
          }

          zclClosures_SendDoorLockToggleDoor( srcEP, dstAddr, &cmd, hdr->fc.disableDefaultRsp, hdr->transSeqNum );

          osal_mem_free( cmd.pPinRfidCode );
          break;
        }

        default:
          // Unsupported command -- just forward it.
          zcl_SendCommand( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID,
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
        case COMMAND_CLOSURES_LOCK_DOOR_RSP:
          zclClosures_SendDoorLockLockDoorRsp( srcEP, dstAddr, data[0], hdr->fc.disableDefaultRsp, hdr->transSeqNum );
          break;

        case COMMAND_CLOSURES_UNLOCK_DOOR_RSP:
          zclClosures_SendDoorLockUnlockDoorRsp( srcEP, dstAddr, data[0], hdr->fc.disableDefaultRsp, hdr->transSeqNum );
          break;

        case COMMAND_CLOSURES_TOGGLE_DOOR_RSP:
          zclClosures_SendDoorLockToggleDoorRsp( srcEP, dstAddr, data[0], hdr->fc.disableDefaultRsp, hdr->transSeqNum );
          break;

        default:
          // Unsupported command -- just forward it.
          zcl_SendCommand( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, dstAddr, clusterID,
                           hdr->commandID, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                           hdr->fc.disableDefaultRsp, 0, hdr->transSeqNum, len, data );
          break;
      }
    }
  }
  else
  {
    return; // unsupported cluster
  }
#endif // ZCL_DOORLOCK
}

#endif // MT_APP_FUNC

/*********************************************************************
 * @fn      zclSampleDoorLockController_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLockController_BasicResetCB( void )
{
  zclSampleDoorLockController_ResetAttributesToDefaultValues();
  
  UI_UpdateLcd();
  zclSampleDoorLockController_UpdateLedState();
}

/*********************************************************************
 * @fn      zclSampleDoorLockController_DoorLockRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Door Lock response for this application.
 *
 * @param   pInMsg - process incoming message
 * @param   status - response status of requesting door lock cmd
 *
 * @return  status
 */
static ZStatus_t zclSampleDoorLockController_DoorLockRspCB ( zclIncoming_t *pInMsg, uint8 status )
{
  // door lock: lock/unlock response
  if ( status == ZSuccess )
  { 
    // if we're using BDB reporting on the doorlock, we don't need to send read attribute
    // because the doorlock should send a report after its own state changes
#ifndef ZCL_REPORT_DESTINATION_DEVICE
    zclReadCmd_t readCmd;
    
    readCmd.numAttr = 1;
    readCmd.attrID[0] = ATTRID_CLOSURES_LOCK_STATE;

    pInMsg->hdr.transSeqNum += 1;
    zcl_SendRead( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                  ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK, &readCmd,
                  ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, pInMsg->hdr.transSeqNum );
#endif
  }
  // ZFailure
  else
  {
    cmdFailure = TRUE;
    UI_UpdateLcd();
  }
  
  return ( ZCL_STATUS_SUCCESS );
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
 * @fn      zclSampleDoorLockController_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclSampleDoorLockController_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg)
{
  switch ( pInMsg->zclHdr.commandID )
  {
#if (! defined ZCL_REPORT_DESTINATION_DEVICE) || (defined MT_APP_FUNC)
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleDoorLockController_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#endif
#ifdef MT_APP_FUNC //A write command will only be sent from the doorlock controller if triggered by the user over MT interface
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleDoorLockController_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleDoorLockController_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleDoorLockController_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleDoorLockController_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleDoorLockController_ProcessInReadReportCfgRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT_DESTINATION_DEVICE
    case ZCL_CMD_REPORT:
      zclSampleDoorLockController_ProcessInReportCmd( pInMsg );
      break;
#endif
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleDoorLockController_ProcessInDiscAttrsRspCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleDoorLockController_ProcessInDefaultRspCmd( pInMsg );
      break;

    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_REPORT_DESTINATION_DEVICE
static void zclSampleDoorLockController_ProcessInReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclReportCmd_t *pInDoorLockReport;
  
  pInDoorLockReport = (zclReportCmd_t *)pInMsg->attrCmd;
  
  if ( pInDoorLockReport->attrList[0].attrID != ATTRID_CLOSURES_LOCK_STATE )
  {
    return;
  }
  
  // read the lock state and display the information
  if ( pInDoorLockReport->attrList[0].attrData[0] == 1 )
  {
    // Locked
    remoteDoorIsLocked = TRUE;
  }
  else if ( pInDoorLockReport->attrList[0].attrData[0] == 2 )
  {
    // Unlocked
    remoteDoorIsLocked = FALSE;
  }
  
  zclSampleDoorLockController_UpdateLedState();
  UI_UpdateLcd();
}
#endif // ZCL_REPORT_DESTINATION_DEVICE

#if (! defined ZCL_REPORT_DESTINATION_DEVICE) || (defined MT_APP_FUNC)
#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleDoorLockController_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;

  // read the lock state and display the information
  // (door lock: read attribute response)
  if ( readRspCmd->attrList[0].data[0] == 1 )
  {
    // Locked
    remoteDoorIsLocked = TRUE;
  }
  else if ( readRspCmd->attrList[0].data[0] == 2 )
  {
    // Unlocked
    remoteDoorIsLocked = FALSE;
  }
  
  zclSampleDoorLockController_UpdateLedState();
  UI_UpdateLcd();

  return TRUE;
}
#endif // ZCL_READ
#endif // (! defined ZCL_REPORT_DESTINATION_DEVICE) || (defined MT_APP_FUNC)

#ifdef MT_APP_FUNC
#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleDoorLockController_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
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
#endif

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleDoorLockController_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
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

/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleDoorLockController_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return TRUE;
}

void zclSampleDoorLockController_UiActionChangePin(uint16 keys)
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

      aiDoorLockPIN[giDoorLockPINColumnCount + 1] = numBuff;   // copy current PIN number
    }
    else
    {
      // make sure '0' is copied into PIN variable
      aiDoorLockPIN[giDoorLockPINColumnCount + 1] = 0x30;   // ASCII '0'
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
      osal_nv_write( ZCD_NV_APS_DOORLOCK_PIN, 0, 5, aiDoorLockPIN );
      osal_start_timerEx( zclSampleDoorLockController_TaskID, SAMPLEDOORLOCKCONTROLLER_PIN_SAVE_TIMEOUT, 2000 );
    }
    else {
      // immediately reset pin screen
      osal_stop_timerEx( zclSampleDoorLockController_TaskID, SAMPLEDOORLOCKCONTROLLER_PIN_SAVE_TIMEOUT);
      osal_set_event( zclSampleDoorLockController_TaskID, SAMPLEDOORLOCKCONTROLLER_PIN_SAVE_TIMEOUT);
    }
  }
  
}

void zclSampleDoorLockController_UiActionSendLockUnlock(uint16 keys)
{
  zclDoorLock_t cmd;

  cmd.pPinRfidCode = aiDoorLockPIN;

  if ( keys & HAL_KEY_SW_1 )
  {
    zclClosures_SendDoorLockLockDoor( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DstAddr, &cmd, TRUE, bdb_getZCLFrameCounter() );
  }
  
  if ( keys & HAL_KEY_SW_3 )
  {
    zclClosures_SendDoorLockUnlockDoor( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DstAddr, &cmd, TRUE, bdb_getZCLFrameCounter() );
  }
}

void zclSampleDoorLockController_UpdateLedState(void)
{
  if ( remoteDoorIsLocked )
  {
    HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
  }
  else
  {
    HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF ); 
  }
}

void zclSampleDoorLockController_UiAppUpdateLcd(uint8 uiCurrentState, char * line[3])
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
      if (cmdFailure)
      {
        cmdFailure = FALSE;
        line[0] = (char *)sFail;
        osal_start_timerEx(zclSampleDoorLockController_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, 2000);
      }
      else
      {
        line[0] = "^:Lock  v:Unlock";
      }
        
      if ( remoteDoorIsLocked )
      {
        line[1] = (char *)sDoorLocked;
      }
      else
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
