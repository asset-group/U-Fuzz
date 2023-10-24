/**************************************************************************************************
  Filename:       zcl_sampletemperaturesensor.c
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
  This application implements a ZigBee Temperature Sensor, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - LEDs:
    LED1 is not used in this application

  Application-specific menu system:

    <SET LOCAL TEMP> Set the temperature of the local temperature sensor
      Up/Down changes the temperature 
      This screen shows the following information:
        Line2:
          Shows the temperature of the local temperature sensor

*********************************************************************/

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
#include "zcl_ms.h"

#include "zcl_sampletemperaturesensor.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include "bdb_interface.h"
#include "bdb_Reporting.h"

#include "zcl_sampleapps_ui.h"
   
/*********************************************************************
 * MACROS
 */

// how often to report temperature
#define SAMPLETEMPERATURESENSOR_REPORT_INTERVAL   10000

#define GUI_LOCAL_TEMP    1

#define APP_TITLE "   Temp Sensor  "

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclSampleTemperatureSensor_TaskID;

extern int16 zdpExternalStateTaskID;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

devStates_t zclSampleTemperatureSensor_NwkState = DEV_INIT;

//static uint8 aProcessCmd[] = { 1, 0, 0, 0 }; // used for reset command, { length + cmd0 + cmd1 + data }

// Test Endpoint to allow SYS_APP_MSGs
/*
static endPointDesc_t sampleTemperatureSensor_TestEp =
{
  20,                                 // Test endpoint
  0,
  &zclSampleTemperatureSensor_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};
*/
#ifdef BDB_REPORTING
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 8
  uint8 reportableChange[] = {0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 0x2C01 is 300 in int16
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 4
  uint8 reportableChange[] = {0x2C, 0x01, 0x00, 0x00}; // 0x2C01 is 300 in int16
#endif 
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 2
  uint8 reportableChange[] = {0x2C, 0x01}; // 0x2C01 is 300 in int16
#endif 
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleTemperatureSensor_HandleKeys( byte shift, byte keys );
static void zclSampleTemperatureSensor_BasicResetCB( void );

static void zclSampleTemperatureSensor_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static void zclSampleTemperatureSensor_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclSampleTemperatureSensor_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclSampleTemperatureSensor_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zclSampleTemperatureSensor_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg );
#endif // ZCL_DISCOVER

static void zclSampleTemperatureSensor_UiActionChangeTemp(uint16 keys);

void zclSampleTemperatureSensor_uiAppUpdateLcd(uint8 uiCurrentState, char * line[3]);

static void zclSampleApp_BatteryWarningCB( uint8 voltLevel);

/*********************************************************************
 * STATUS STRINGS
 */

/*********************************************************************
 * CONSTANTS
 */
const uiState_t zclSampleTemperatureSensor_uiAppStatesMain[] =
{
  /*  UI_STATE_BACK_FROM_APP_MENU  */   {UI_STATE_DEFAULT_MOVE, GUI_LOCAL_TEMP, UI_KEY_SW_5_PRESSED, &UI_ActionBackFromAppMenu}, //do not change this line, except for the second item, which should point to the last entry in this menu
  /*  GUI_LOCAL_TEMP          */        {UI_STATE_BACK_FROM_APP_MENU, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED, &zclSampleTemperatureSensor_UiActionChangeTemp},
};

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleTemperatureSensor_CmdCallbacks =
{
  zclSampleTemperatureSensor_BasicResetCB,        // Basic Cluster Reset command
  NULL,                                           // Identify Trigger Effect command
  NULL,             				                      // On/Off cluster command
  NULL,                                           // On/Off cluster enhanced command Off with Effect
  NULL,                                           // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                           // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                           // Level Control Move to Level command
  NULL,                                           // Level Control Move command
  NULL,                                           // Level Control Step command
  NULL,                                           // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                           // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                           // Scene Store Request command
  NULL,                                           // Scene Recall Request command
  NULL,                                           // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                           // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                           // Get Event Log command
  NULL,                                           // Publish Event Log command
#endif
  NULL,                                           // RSSI Location command
  NULL                                            // RSSI Location Response command
};

/*********************************************************************
 * @fn          zclSampleTemperatureSensor_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclSampleTemperatureSensor_Init( byte task_id )
{
  zclSampleTemperatureSensor_TaskID = task_id;

  // Register the Simple Descriptor for this application
  bdb_RegisterSimpleDescriptor( &zclSampleTemperatureSensor_SimpleDesc ); 
  
  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLETEMPERATURESENSOR_ENDPOINT, &zclSampleTemperatureSensor_CmdCallbacks );

  // Register the application's attribute list
  zclSampleTemperatureSensor_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLETEMPERATURESENSOR_ENDPOINT, zclSampleTemperatureSensor_NumAttributes, zclSampleTemperatureSensor_Attrs );   

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclSampleTemperatureSensor_TaskID );

  // Register low voltage NV memory protection application callback
  RegisterVoltageWarningCB( zclSampleApp_BatteryWarningCB );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclSampleTemperatureSensor_TaskID );

  bdb_RegisterCommissioningStatusCB( zclSampleTemperatureSensor_ProcessCommissioningStatus );

#ifdef BDB_REPORTING
  //Adds the default configuration values for the temperature attribute of the ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT cluster, for endpoint SAMPLETEMPERATURESENSOR_ENDPOINT
  //Default maxReportingInterval value is 10 seconds
  //Default minReportingInterval value is 3 seconds
  //Default reportChange value is 300 (3 degrees)
  bdb_RepAddAttrCfgRecordDefaultToList(SAMPLETEMPERATURESENSOR_ENDPOINT, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, ATTRID_MS_TEMPERATURE_MEASURED_VALUE, 0, 10, reportableChange);
#endif
  
  zdpExternalStateTaskID = zclSampleTemperatureSensor_TaskID;

  UI_Init(zclSampleTemperatureSensor_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, SAMPLEAPP_KEY_AUTO_REPEAT_EVT, &zclSampleTemperatureSensor_IdentifyTime, APP_TITLE, zclSampleTemperatureSensor_uiAppUpdateLcd, zclSampleTemperatureSensor_uiAppStatesMain);

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
uint16 zclSampleTemperatureSensor_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclSampleTemperatureSensor_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclSampleTemperatureSensor_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclSampleTemperatureSensor_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
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
 * @fn      zclSampleTemperatureSensor_HandleKeys
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
static void zclSampleTemperatureSensor_HandleKeys( byte shift, byte keys )
{
  UI_MainStateMachine(keys);
}

/*********************************************************************
 * @fn      zclSampleTemperatureSensor_LcdDisplayMainMode
 *
 * @brief   Called to display the main screen on the LCD.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
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
        osal_start_timerEx(zclSampleTemperatureSensor_TaskID, SAMPLEAPP_END_DEVICE_REJOIN_EVT, SAMPLEAPP_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
    }

  UI_UpdateComissioningStatus(bdbCommissioningModeMsg);
}

/*********************************************************************
 * @fn      zclSampleTemperatureSensor_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_BasicResetCB( void )
{
  zclSampleTemperatureSensor_ResetAttributesToDefaultValues();
  
  UI_UpdateLcd();
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
 * @fn      zclSampleTemperatureSensor_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg)
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleTemperatureSensor_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleTemperatureSensor_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleTemperatureSensor_ProcessInConfigReportCmd( pInMsg );
      break;
      case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleTemperatureSensor_ProcessInReadReportCfgCmd( pInMsg );
      break;
    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleTemperatureSensor_ProcessInConfigReportRspCmd( pInMsg );
      break;
    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleTemperatureSensor_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      //zclSampleTemperatureSensor_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleTemperatureSensor_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd( pInMsg );
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
  {
    osal_mem_free( pInMsg->attrCmd );
  }
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleTemperatureSensor_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < readRspCmd->numAttr; i++ )
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return ( TRUE );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleTemperatureSensor_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < writeRspCmd->numAttr; i++ )
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleTemperatureSensor_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}
#endif // ZCL_DISCOVER

// GUI_LOCAL_TEMP
static void zclSampleTemperatureSensor_UiActionChangeTemp(uint16 keys)
{
  if ( keys & HAL_KEY_SW_1 )
  {
    // increase the temperature
    if ( zclSampleTemperatureSensor_MeasuredValue < zclSampleTemperatureSensor_MaxMeasuredValue )
    {
      zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MeasuredValue + 100;  // considering using whole number value
#ifdef BDB_REPORTING      
      uint8 status = bdb_RepChangedAttrValue(SAMPLETEMPERATURESENSOR_ENDPOINT, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, ATTRID_MS_TEMPERATURE_MEASURED_VALUE);
#endif
    }
    else if ( zclSampleTemperatureSensor_MeasuredValue >= zclSampleTemperatureSensor_MaxMeasuredValue )
    {
      zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MaxMeasuredValue;
    }
  }
  
  if ( keys & HAL_KEY_SW_3 )
  {
    // decrease the temperature
    if ( zclSampleTemperatureSensor_MeasuredValue > zclSampleTemperatureSensor_MinMeasuredValue )
    {
      zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MeasuredValue - 100;  // considering using whole number value
#ifdef BDB_REPORTING
      uint8 status = bdb_RepChangedAttrValue(SAMPLETEMPERATURESENSOR_ENDPOINT, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, ATTRID_MS_TEMPERATURE_MEASURED_VALUE);
#endif
    }
    else if ( zclSampleTemperatureSensor_MeasuredValue <= zclSampleTemperatureSensor_MinMeasuredValue )
    {
      zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MinMeasuredValue;
    }
  }
  
  UI_UpdateLcd();
}


void zclSampleTemperatureSensor_uiAppUpdateLcd(uint8 gui_state, char * line[3])
{
  static char sDisplayTemp[16];
  
  switch(gui_state)
  {
    case GUI_LOCAL_TEMP:
    // display current temperature
    osal_memcpy(sDisplayTemp, "TEMP: ", 6);
    _ltoa( ( zclSampleTemperatureSensor_MeasuredValue / 100 ), (void *)(&sDisplayTemp[6]), 10 );   // convert temperature to whole number
    osal_memcpy( &sDisplayTemp[8], "C", 2 );

    line[1] = (char *)sDisplayTemp;
    line[2] = "<SET LOCAL TEMP>";
    break;
    
    default:
    break;
  }
}

/****************************************************************************
****************************************************************************/


