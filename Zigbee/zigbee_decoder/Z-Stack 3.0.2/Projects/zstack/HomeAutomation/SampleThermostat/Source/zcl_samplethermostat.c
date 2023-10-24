/**************************************************************************************************
  Filename:       zcl_samplethermostat.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $

  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013 - 2014 Texas Instruments Incorporated. All rights reserved.

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
  This application implements a ZigBee Thermostat, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - LEDs:
    LED1 on indicates that the system is currently heating or cooling.
    LED1 off indicates that the system is currently off.

  Application-specific menu system:

    <REMOTE TEMP> View the temperature of the remote temperature sensor
      Buttons have no affect on this screen
      This screen shows the following information:
        Line2:
          Shows the temperature of the remote temperature sensor

    <SET HEAT TEMP> Changes the heating point temperature
      Up/Down changes the temperature at which heating will activate
      This screen shows the following information:
        Line2:
          Shows current heating point temperature

    <SET COOL TEMP> Changes the cooling point temperature
      Up/Down changes the temperature at which cooling will activate
      This screen shows the following information:
        Line2:
          Shows current cooling point temperature

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
#include "MT_SYS.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_hvac.h"
#include "zcl_ms.h"

#include "zcl_samplethermostat.h"

#include "bdb_interface.h"


#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include "zcl_sampleapps_ui.h"

/*********************************************************************
 * MACROS
 */

#define GUI_REMOTE_TEMP    1
#define GUI_SET_HEATING   2         
#define GUI_SET_COOLING   3

#define APP_TITLE "   Thermostat   "

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 zclSampleThermostat_TaskID;

extern int16 zdpExternalStateTaskID;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

devStates_t zclSampleThermostat_NwkState = DEV_INIT;

//static uint8 aProcessCmd[] = { 1, 0, 0, 0 }; // used for reset command, { length + cmd0 + cmd1 + data }

// Test Endpoint to allow SYS_APP_MSGs
/*
static endPointDesc_t sampleThermostat_TestEp =
{
  20,                                 // Test endpoint
  0,
  &zclSampleThermostat_TaskID,
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
static void zclSampleThermostat_HandleKeys( byte shift, byte keys );
static void zclSampleThermostat_BasicResetCB( void );
#ifdef MT_APP_FUNC
static void zclSampleThermostat_ProcessAppMsg( uint8 srcEP, uint8 len, uint8 *msg );
static void zclSampleThermostat_ProcessFoundationMsg( afAddrType_t *dstAddr, uint16 clusterID,
                                                      zclFrameHdr_t *hdr, zclParseCmd_t *pParseCmd );
#endif

static void zclSampleThermostat_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);


// app display functions
void zclSampleThermostat_LcdDisplayUpdate(void);
void zclSampleThermostat_LcdDisplayMainMode(void);
void zclSampleThermostat_LcdDisplayHeatMode(void);
void zclSampleThermostat_LcdDisplayCoolMode(void);
void zclSampleThermostat_LcdDisplayHelpMode(void);

// Functions to process ZCL Foundation incoming Command/Response messages
static void zclSampleThermostat_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclSampleThermostat_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclSampleThermostat_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_REPORT_DESTINATION_DEVICE
static void zclSampleThermostat_ProcessInReportCmd( zclIncomingMsg_t *pInMsg );
#endif  // ZCL_REPORT_DESTINATION_DEVICE
static uint8 zclSampleThermostat_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );

static void zclSampleThermostat_UiActionSetHeating(uint16 keys);
static void zclSampleThermostat_UiActionSetCooling(uint16 keys);
void zclSampleThermostat_UiAppUpdateLcd(uint8 uiCurrentState, char * line[3]);
static void zclSampleThermostat_UpdateLedState(void);

static void zclSampleApp_BatteryWarningCB( uint8 voltLevel);

/*********************************************************************
 * STATUS STRINGS
 */

/*********************************************************************
 * CONSTANTS
 */
const uiState_t zclSampleThermostat_UiAppStatesMain[] =
{
  /*  UI_STATE_BACK_FROM_APP_MENU  */   {UI_STATE_DEFAULT_MOVE, GUI_SET_COOLING, UI_KEY_SW_5_PRESSED, &UI_ActionBackFromAppMenu}, //do not change this line, except for the second item, which should point to the last entry in this menu
  /*  GUI_REMOTE_TEMP         */   {UI_STATE_DEFAULT_MOVE, UI_STATE_DEFAULT_MOVE, NULL},
  /*  GUI_SET_HEATING          */   {UI_STATE_DEFAULT_MOVE, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED, &zclSampleThermostat_UiActionSetHeating},
  /*  GUI_SET_COOLING         */   {UI_STATE_BACK_FROM_APP_MENU, UI_STATE_DEFAULT_MOVE, UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED, &zclSampleThermostat_UiActionSetCooling},
};

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleThermostat_CmdCallbacks =
{
  zclSampleThermostat_BasicResetCB,            // Basic Cluster Reset command
  NULL,                                        // Identify Trigger Effect command
  NULL,             				                   // On/Off cluster command
  NULL,                                        // On/Off cluster enhanced command Off with Effect
  NULL,                                        // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                        // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                        // Level Control Move to Level command
  NULL,                                        // Level Control Move command
  NULL,                                        // Level Control Step command
  NULL,                                        // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                        // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                        // Scene Store Request command
  NULL,                                        // Scene Recall Request command
  NULL,                                        // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                        // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                        // Get Event Log command
  NULL,                                        // Publish Event Log command
#endif
  NULL,                                        // RSSI Location command
  NULL                                         // RSSI Location Response command
};

/*********************************************************************
 * @fn          zclSampleThermostat_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclSampleThermostat_Init( byte task_id )
{
  zclSampleThermostat_TaskID = task_id;

  // Register the Simple Descriptor for this application
  bdb_RegisterSimpleDescriptor( &zclSampleThermostat_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLETHERMOSTAT_ENDPOINT, &zclSampleThermostat_CmdCallbacks );

  // Register the application's attribute list
  zclSampleThermostat_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLETHERMOSTAT_ENDPOINT, zclSampleThermostat_NumAttributes, zclSampleThermostat_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclSampleThermostat_TaskID );

  // Register low voltage NV memory protection application callback
  RegisterVoltageWarningCB( zclSampleApp_BatteryWarningCB );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclSampleThermostat_TaskID );

  bdb_RegisterCommissioningStatusCB( zclSampleThermostat_ProcessCommissioningStatus );

#ifdef BDB_REPORTING
  //Adds the default configuration values for the reportable attributes of the ZCL_CLUSTER_ID_HVAC_THERMOSTAT cluster, for endpoint SAMPLETHERMOSTAT_ENDPOINT
  //Default maxReportingInterval value is 10 seconds
  //Default minReportingInterval value is 3 seconds
  //Default reportChange value is 300 (3 degrees)
  bdb_RepAddAttrCfgRecordDefaultToList(SAMPLETHERMOSTAT_ENDPOINT, ZCL_CLUSTER_ID_HVAC_THERMOSTAT, ATTRID_HVAC_THERMOSTAT_LOCAL_TEMPERATURE, 0, 10, reportableChange);
  bdb_RepAddAttrCfgRecordDefaultToList(SAMPLETHERMOSTAT_ENDPOINT, ZCL_CLUSTER_ID_HVAC_THERMOSTAT, ATTRID_HVAC_THERMOSTAT_PI_COOLING_DEMAND, 0, 10, reportableChange);
  bdb_RepAddAttrCfgRecordDefaultToList(SAMPLETHERMOSTAT_ENDPOINT, ZCL_CLUSTER_ID_HVAC_THERMOSTAT, ATTRID_HVAC_THERMOSTAT_PI_HEATING_DEMAND, 0, 10, reportableChange);
#endif  
  
  zdpExternalStateTaskID = zclSampleThermostat_TaskID;

  UI_Init(zclSampleThermostat_TaskID, SAMPLEAPP_LCD_AUTO_UPDATE_EVT, SAMPLEAPP_KEY_AUTO_REPEAT_EVT, &zclSampleThermostat_IdentifyTime, APP_TITLE, &zclSampleThermostat_UiAppUpdateLcd, zclSampleThermostat_UiAppStatesMain);

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
uint16 zclSampleThermostat_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclSampleThermostat_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
#ifdef MT_APP_FUNC
        case MT_SYS_APP_MSG:
          // Message received from MT
          zclSampleThermostat_ProcessAppMsg( ((mtSysAppMsg_t *)MSGpkt)->endpoint,
                                          ((mtSysAppMsg_t *)MSGpkt)->appDataLen,
                                          ((mtSysAppMsg_t *)MSGpkt)->appData );
#endif
          break;

        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclSampleThermostat_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclSampleThermostat_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
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
 * @fn      zclSampleThermostat_HandleKeys
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
static void zclSampleThermostat_HandleKeys( byte shift, byte keys )
{
  UI_MainStateMachine(keys);
}

#ifdef MT_APP_FUNC

/*********************************************************************
 * @fn      zclSampleThermostat_ProcessAppMsg
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
static void zclSampleThermostat_ProcessAppMsg( uint8 srcEP, uint8 len, uint8 *msg )
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

      zclSampleThermostat_ProcessFoundationMsg( &dstAddr, clusterID, &hdr, &cmd );
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
  }
}

/*********************************************************************
 * @fn      zclSampleThermostat_ProcessFoundationMsg
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
static void zclSampleThermostat_ProcessFoundationMsg( afAddrType_t *dstAddr, uint16 clusterID,
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
        zcl_SendRead( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclReadCmd_t *)cmd,
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
        zcl_SendWrite( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                       ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_WRITE_UNDIVIDED:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWriteUndivided( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
                                ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_WRITE_NO_RSP:
      cmd = zclParseInWriteCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendWriteNoRsp( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclWriteCmd_t *)cmd,
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
        zcl_SendConfigReportCmd( SAMPLETHERMOSTAT_ENDPOINT, dstAddr,  clusterID, (zclCfgReportCmd_t *)cmd,
                                 ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      cmd = zclParseInReadReportCfgCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendReadReportCfgCmd( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclReadReportCfgCmd_t *)cmd,
                                  ZCL_FRAME_CLIENT_SERVER_DIR, hdr->fc.disableDefaultRsp, hdr->transSeqNum );
        osal_mem_free( cmd );
      }
      break;

    case ZCL_CMD_REPORT:
      cmd = zclParseInReportCmd( pParseCmd );
      if ( cmd )
      {
        zcl_SendReportCmd( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclReportCmd_t *)cmd,
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
        zcl_SendDiscoverAttrsCmd( SAMPLETHERMOSTAT_ENDPOINT, dstAddr, clusterID, (zclDiscoverAttrsCmd_t *)cmd,
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

#endif // MT_APP_FUNC

/*********************************************************************
 * @fn      zclSampleThermostat_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleThermostat_BasicResetCB( void )
{
  zclSampleThermostat_ResetAttributesToDefaultValues();
  
  UI_UpdateLcd();  
  zclSampleThermostat_UpdateLedState();
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
 * @fn      zclSampleThermostat_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclSampleThermostat_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg)
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleThermostat_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleThermostat_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleThermostat_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleThermostat_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleThermostat_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleThermostat_ProcessInReadReportCfgRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT_DESTINATION_DEVICE
    case ZCL_CMD_REPORT:
      zclSampleThermostat_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleThermostat_ProcessInDefaultRspCmd( pInMsg );
      break;

    default:
      break;
  }

  if ( pInMsg->attrCmd )
  {
    osal_mem_free( pInMsg->attrCmd );
  }
}


static void zclSampleThermostat_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
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
        osal_start_timerEx(zclSampleThermostat_TaskID, SAMPLEAPP_END_DEVICE_REJOIN_EVT, SAMPLEAPP_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
    }

  UI_UpdateComissioningStatus(bdbCommissioningModeMsg);
}


#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclSampleThermostat_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleThermostat_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
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

  return ( TRUE );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleThermostat_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleThermostat_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

#ifdef ZCL_REPORT_DESTINATION_DEVICE
/*********************************************************************
 * @fn      zclSampleThermostat_ProcessInReportCmd
 *
 * @brief   Process the "Profile" Report Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static void zclSampleThermostat_ProcessInReportCmd( zclIncomingMsg_t *pInMsg )
{
  zclReportCmd_t *pInTempSensorReport;
  
  pInTempSensorReport = (zclReportCmd_t *)pInMsg->attrCmd;

  if ( pInTempSensorReport->attrList[0].attrID != ATTRID_MS_TEMPERATURE_MEASURED_VALUE )
  {
    return;
  }
  
  // store the current temperature value sent over the air from temperature sensor
  zclSampleThermostat_LocalTemperature = BUILD_UINT16(pInTempSensorReport->attrList[0].attrData[0], pInTempSensorReport->attrList[0].attrData[1]);
  
  UI_UpdateLcd();
  zclSampleThermostat_UpdateLedState();
}
#endif  // ZCL_REPORT_DESTINATION_DEVICE

/*********************************************************************
 * @fn      zclSampleThermostat_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclSampleThermostat_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

static void zclSampleThermostat_UiActionSetHeating(uint16 keys)
{
  if ( keys & HAL_KEY_SW_1 )
  {
    // increase heating setpoint, considering whole numbers where necessary
    if ( zclSampleThermostat_OccupiedHeatingSetpoint < zclSampleThermostat_MaxHeatSetpointLimit )
    {
      zclSampleThermostat_OccupiedHeatingSetpoint = zclSampleThermostat_OccupiedHeatingSetpoint + 100;
    }
    else if ( zclSampleThermostat_OccupiedHeatingSetpoint >= zclSampleThermostat_MaxHeatSetpointLimit )
    {
      zclSampleThermostat_OccupiedHeatingSetpoint = zclSampleThermostat_MaxHeatSetpointLimit;
    }
  }
  
  if ( keys & HAL_KEY_SW_3 )
  {
    // decrease heating setpoint, considering whole numbers where necessary
    if ( zclSampleThermostat_OccupiedHeatingSetpoint > zclSampleThermostat_MinHeatSetpointLimit )
    {
      zclSampleThermostat_OccupiedHeatingSetpoint = zclSampleThermostat_OccupiedHeatingSetpoint - 100;
    }
    else if ( zclSampleThermostat_OccupiedHeatingSetpoint <= zclSampleThermostat_MinHeatSetpointLimit )
    {
      zclSampleThermostat_OccupiedHeatingSetpoint = zclSampleThermostat_MinHeatSetpointLimit;
    }
  }
  
  UI_UpdateLcd();
  zclSampleThermostat_UpdateLedState();
}

static void zclSampleThermostat_UiActionSetCooling(uint16 keys)
{
  if ( keys & HAL_KEY_SW_1 )
  {
    // increase cooling setpoint, considering whole numbers where necessary
    if ( zclSampleThermostat_OccupiedCoolingSetpoint < zclSampleThermostat_MaxCoolSetpointLimit )
    {
      zclSampleThermostat_OccupiedCoolingSetpoint = zclSampleThermostat_OccupiedCoolingSetpoint + 100;
    }
    else if ( zclSampleThermostat_OccupiedCoolingSetpoint >= zclSampleThermostat_MaxCoolSetpointLimit )
    {
      zclSampleThermostat_OccupiedCoolingSetpoint = zclSampleThermostat_MaxCoolSetpointLimit;
    }
  }
  
  if ( keys & HAL_KEY_SW_3 )
  {
    // decrease cooling setpoint, considering whole numbers where necessary
    if ( zclSampleThermostat_OccupiedCoolingSetpoint > zclSampleThermostat_MinCoolSetpointLimit )
    {
      zclSampleThermostat_OccupiedCoolingSetpoint = zclSampleThermostat_OccupiedCoolingSetpoint - 100;
    }
    else if ( zclSampleThermostat_OccupiedCoolingSetpoint <= zclSampleThermostat_MinCoolSetpointLimit )
    {
      zclSampleThermostat_OccupiedCoolingSetpoint = zclSampleThermostat_MinCoolSetpointLimit;
    }
  }
  
  UI_UpdateLcd();
  zclSampleThermostat_UpdateLedState();
}

void zclSampleThermostat_UpdateLedState(void)
{
  // use LEDs to show heating or cooling cycles based off local temperature
  if ( zclSampleThermostat_LocalTemperature != NULL )
  {
    if ( zclSampleThermostat_LocalTemperature <= zclSampleThermostat_OccupiedHeatingSetpoint )
    {
      // turn on heating
      zclSampleThermostat_SystemMode = HVAC_THERMOSTAT_SYSTEM_MODE_HEAT;
      HalLedSet ( HAL_LED_1, HAL_LED_MODE_ON );
    }
    else if ( zclSampleThermostat_LocalTemperature >= zclSampleThermostat_OccupiedCoolingSetpoint )
    {
      // turn on cooling
      zclSampleThermostat_SystemMode = HVAC_THERMOSTAT_SYSTEM_MODE_COOL;
      HalLedSet ( HAL_LED_1, HAL_LED_MODE_ON );
    }
    else
    {
      // turn off heating/cooling
      zclSampleThermostat_SystemMode = HVAC_THERMOSTAT_SYSTEM_MODE_OFF;
      HalLedSet ( HAL_LED_1, HAL_LED_MODE_OFF );
    }
  }
}

void zclSampleThermostat_UiAppUpdateLcd(uint8 gui_state, char * line[3])
{ 
  static char sDisplayTemp[16];
  
  switch(gui_state)
  {
    case GUI_REMOTE_TEMP:
      osal_memcpy( sDisplayTemp, "TEMP: ", 6 );
      
      // if local temperature has not been set, make note on display
      if ( zclSampleThermostat_LocalTemperature == NULL )
      {
        osal_memcpy( &sDisplayTemp[6], "N/A", 4 );
      }
      else
      {
        _ltoa( ( zclSampleThermostat_LocalTemperature / 100 ), (void *)(&sDisplayTemp[6]), 10 ); // only use whole number
        osal_memcpy( &sDisplayTemp[8], "C", 2 );
      }
      line[1] = (char *)sDisplayTemp;
      line[2] = "< REMOTE TEMP  >";
      break;
      
    case GUI_SET_HEATING:
      osal_memcpy( sDisplayTemp, "HEAT TEMP: ", 11 );
      _ltoa( ( zclSampleThermostat_OccupiedHeatingSetpoint / 100 ), (void *)(&sDisplayTemp[11]), 10 ); // only use whole number
      osal_memcpy( &sDisplayTemp[13], "C", 2 );

      line[1] = (char *)sDisplayTemp;
      line[2] = "<SET HEAT TEMP >";
      break;
      
    case GUI_SET_COOLING:
      osal_memcpy(sDisplayTemp, "COOL TEMP: ", 11);
      _ltoa( ( zclSampleThermostat_OccupiedCoolingSetpoint / 100 ), (void *)(&sDisplayTemp[11]), 10 ); // only use whole number
      osal_memcpy( &sDisplayTemp[13], "C", 2 );
      
      line[1] = (char *)sDisplayTemp;
      line[2] = "<SET COOL TEMP >";
      break;
      
    default:
      break;
  }
}

/****************************************************************************
****************************************************************************/


