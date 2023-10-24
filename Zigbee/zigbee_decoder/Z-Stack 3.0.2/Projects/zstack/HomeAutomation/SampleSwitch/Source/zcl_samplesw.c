/**************************************************************************************************
  Filename:       zcl_samplesw.c
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $

  Description:    Zigbee Cluster Library - sample switch application.


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

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
  This application implements a ZigBee On/Off Switch, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - none (LED1 is currently unused by this application).

  Application-specific menu system:

    <TOGGLE LIGHT> Send an On, Off or Toggle command targeting appropriate devices from the binding table.
      Pressing / releasing [OK] will have the following functionality, depending on the value of the 
      zclSampleSw_OnOffSwitchActions attribute:
      - OnOffSwitchActions == 0: pressing [OK] will send ON command, releasing it will send OFF command;
      - OnOffSwitchActions == 1: pressing [OK] will send OFF command, releasing it will send ON command;
      - OnOffSwitchActions == 2: pressing [OK] will send TOGGLE command, releasing it will not send any command.

*********************************************************************/

#if ! defined ZCL_ON_OFF
#error ZCL_ON_OFF must be defined for this project.
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "MT_SYS.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_samplesw.h"
#include "zcl_diagnostic.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
#include "zcl_ota.h"
#include "hal_ota.h"
#endif

#include "bdb.h"
#include "bdb_interface.h"

#include "zcl_sampleapps_ui.h"

/*********************************************************************
 * MACROS
 */
#define UI_STATE_TOGGLE_LIGHT 1 //UI_STATE_BACK_FROM_APP_MENU is item #0, so app item numbers should start from 1

#define APP_TITLE "TI Sample Switch"

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclSampleSw_TaskID;

uint8 zclSampleSwSeqNum;

uint8 zclSampleSw_OnOffSwitchType = ON_OFF_SWITCH_TYPE_MOMENTARY;

uint8 zclSampleSw_OnOffSwitchActions;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
afAddrType_t zclSampleSw_DstAddr;

// Endpoint to allow SYS_APP_MSGs
static endPointDesc_t sampleSw_TestEp =
{
  SAMPLESW_ENDPOINT,                  // endpoint
  0,
  &zclSampleSw_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

//static uint8 aProcessCmd[] = { 1, 0, 0, 0 }; // used for reset command, { length + cmd0 + cmd1 + data }

devStates_t zclSampleSw_NwkState = DEV_INIT;

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
#define DEVICE_POLL_RATE                 8000   // Poll rate for end device
#endif

#define SAMPLESW_TOGGLE_TEST_EVT   0x1000
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleSw_HandleKeys( byte shift, byte keys );
static void zclSampleSw_BasicResetCB( void );

static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);


// Functions to process ZCL Foundation incoming Command/Response messages
static void zclSampleSw_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclSampleSw_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclSampleSw_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zclSampleSw_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 zclSampleSw_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclSampleSw_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclSampleSw_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg );
#endif

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
static void zclSampleSw_ProcessOTAMsgs( zclOTA_CallbackMsg_t* pMsg );
#endif

void zclSampleSw_UiActionToggleLight(uint16 keys);
void zclSampleSw_UiUpdateLcd(uint8 uiCurrentState, char * line[3]);

static void zclSampleApp_BatteryWarningCB( uint8 voltLevel);


/*********************************************************************
 * CONSTANTS
 */
  const uiState_t zclSampleSw_UiStatesMain[] = 
  {
    /*  UI_STATE_BACK_FROM_APP_MENU  */   {UI_STATE_DEFAULT_MOVE,       UI_STATE_TOGGLE_LIGHT,  UI_KEY_SW_5_PRESSED, &UI_ActionBackFromAppMenu}, //do not change this line, except for the second item, which should point to the last entry in this menu
    /*  UI_STATE_TOGGLE_LIGHT        */   {UI_STATE_BACK_FROM_APP_MENU, UI_STATE_DEFAULT_MOVE,  UI_KEY_SW_5_PRESSED | UI_KEY_SW_5_RELEASED, &zclSampleSw_UiActionToggleLight},
  };
  
/*********************************************************************
 * REFERENCED EXTERNALS
 */
extern int16 zdpExternalStateTaskID;

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleSw_CmdCallbacks =
{
  zclSampleSw_BasicResetCB,               // Basic Cluster Reset command
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
 * @fn          zclSampleSw_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclSampleSw_Init( byte task_id )
{
  zclSampleSw_TaskID = task_id;

  // Set destination address to indirect
  zclSampleSw_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleSw_DstAddr.endPoint = 0;
  zclSampleSw_DstAddr.addr.shortAddr = 0;

  // Register the Simple Descriptor for this application
  bdb_RegisterSimpleDescriptor( &zclSampleSw_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLESW_ENDPOINT, &zclSampleSw_CmdCallbacks );

  zclSampleSw_ResetAttributesToDefaultValues();
  
  // Register the application's attribute list
  zcl_registerAttrList( SAMPLESW_ENDPOINT, zclSampleSw_NumAttributes, zclSampleSw_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclSampleSw_TaskID );
  
  // Register low voltage NV memory protection application callback
  RegisterVoltageWarningCB( zclSampleApp_BatteryWarningCB );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclSampleSw_TaskID );
  
  bdb_RegisterCommissioningStatusCB( zclSampleSw_ProcessCommissioningStatus );

  // Register for a test endpoint
  afRegister( &sampleSw_TestEp );
  
#ifdef ZCL_DIAGNOSTIC
  // Register the application's callback function to read/write attribute data.
  // This is only required when the attribute data format is unknown to ZCL.
  zcl_registerReadWriteCB( SAMPLESW_ENDPOINT, zclDiagnostic_ReadWriteAttrCB, NULL );

  if ( zclDiagnostic_InitStats() == ZSuccess )
  {
    // Here the user could start the timer to save Diagnostics to NV
  }
#endif

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
  // Register for callback events from the ZCL OTA
  zclOTA_Register(zclSampleSw_TaskID);
#endif

  zdpExternalStateTaskID = zclSampleSw_TaskID;

  UI_Init(zclSampleSw_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, SAMPLEAPP_KEY_AUTO_REPEAT_EVT, &zclSampleSw_IdentifyTime, APP_TITLE, &zclSampleSw_UiUpdateLcd, zclSampleSw_UiStatesMain);

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
uint16 zclSampleSw_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  //Send toggle every 500ms
  if( events & SAMPLESW_TOGGLE_TEST_EVT )
  {
    osal_start_timerEx(zclSampleSw_TaskID,SAMPLESW_TOGGLE_TEST_EVT,500);
    zclGeneral_SendOnOff_CmdToggle( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, 0 );
    
    // return unprocessed events
    return (events ^ SAMPLESW_TOGGLE_TEST_EVT);
  }
  
  
  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclSampleSw_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclSampleSw_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclSampleSw_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          UI_DeviceStateUpdated((devStates_t)(MSGpkt->hdr.status));
          break;

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
        case ZCL_OTA_CALLBACK_IND:
          zclSampleSw_ProcessOTAMsgs( (zclOTA_CallbackMsg_t*)MSGpkt  );
          break;
#endif

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
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
 * @fn      zclSampleSw_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zclSampleSw_HandleKeys( byte shift, byte keys )
{
  UI_MainStateMachine(keys);
}


/*********************************************************************
 * @fn      zclSampleSw_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //After formation, perform nwk steering again plus the remaining commissioning modes that has not been processed yet
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
        osal_start_timerEx(zclSampleSw_TaskID, SAMPLEAPP_END_DEVICE_REJOIN_EVT, SAMPLEAPP_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
  }
  
  UI_UpdateComissioningStatus(bdbCommissioningModeMsg);
}

/*********************************************************************
 * @fn      zclSampleSw_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to  default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleSw_BasicResetCB( void )
{
  zclSampleSw_ResetAttributesToDefaultValues();

  // update the display
  UI_UpdateLcd( ); 
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
 * @fn      zclSampleSw_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclSampleSw_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleSw_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleSw_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleSw_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleSw_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleSw_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleSw_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      //zclSampleSw_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleSw_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleSw_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleSw_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleSw_ProcessInDiscAttrsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleSw_ProcessInDiscAttrsExtRspCmd( pInMsg );
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
 * @fn      zclSampleSw_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleSw_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return TRUE;
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleSw_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleSw_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
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
 * @fn      zclSampleSw_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleSw_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
  // Device is notified of the Default Response command.
  (void)pInMsg;
  return TRUE;
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleSw_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleSw_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclSampleSw_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleSw_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
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

/*********************************************************************
 * @fn      zclSampleSw_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleSw_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}
#endif // ZCL_DISCOVER

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
/*********************************************************************
 * @fn      zclSampleSw_ProcessOTAMsgs
 *
 * @brief   Called to process callbacks from the ZCL OTA.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleSw_ProcessOTAMsgs( zclOTA_CallbackMsg_t* pMsg )
{
  uint8 RxOnIdle;

  switch(pMsg->ota_event)
  {
  case ZCL_OTA_START_CALLBACK:
    if (pMsg->hdr.status == ZSuccess)
    {
      // Speed up the poll rate
      RxOnIdle = TRUE;
      ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
      NLME_SetPollRate( 2000 );
    }
    break;

  case ZCL_OTA_DL_COMPLETE_CALLBACK:
    if (pMsg->hdr.status == ZSuccess)
    {
      // Reset the CRC Shadow and reboot.  The bootloader will see the
      // CRC shadow has been cleared and switch to the new image
      HalOTAInvRC();
      SystemReset();
    }
    else
    {
#if (ZG_BUILD_ENDDEVICE_TYPE)    
      // slow the poll rate back down.
      RxOnIdle = FALSE;
      ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
      NLME_SetPollRate(DEVICE_POLL_RATE);
#endif
    }
    break;

  default:
    break;
  }
}
#endif // defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)

/****************************************************************************
****************************************************************************/

void zclSampleSw_UiActionToggleLight(uint16 keys)
{
  if (zclSampleSw_OnOffSwitchActions == ON_OFF_SWITCH_ACTIONS_TOGGLE)
  {
    if (keys & UI_KEY_SW_5_PRESSED)
    {
      zclGeneral_SendOnOff_CmdToggle( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, bdb_getZCLFrameCounter() );
    }
  }
  else if (((keys & UI_KEY_SW_5_PRESSED) && (zclSampleSw_OnOffSwitchActions == ON_OFF_SWITCH_ACTIONS_ON))
    || ((keys & UI_KEY_SW_5_RELEASED) && (zclSampleSw_OnOffSwitchActions == ON_OFF_SWITCH_ACTIONS_OFF)))
  {
    zclGeneral_SendOnOff_CmdOn( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, bdb_getZCLFrameCounter() );
  }
  else
  {
    zclGeneral_SendOnOff_CmdOff( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, bdb_getZCLFrameCounter() );
  }
}


void zclSampleSw_UiUpdateLcd(uint8 gui_state, char * line[3])
{
  line[2] = "< TOGGLE LIGHT >";
}


