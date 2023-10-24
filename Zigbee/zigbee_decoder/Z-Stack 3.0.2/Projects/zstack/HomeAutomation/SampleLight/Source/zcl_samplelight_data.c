/**************************************************************************************************
  Filename:       zcl_samplelight_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


  Description:    Zigbee Cluster Library - sample device application.


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

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_poll_control.h"
#include "zcl_electrical_measurement.h"
#include "zcl_diagnostic.h"
#include "zcl_meter_identification.h"
#include "zcl_appliance_identification.h"
#include "zcl_appliance_events_alerts.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_statistics.h"
#include "zcl_hvac.h"

#include "zcl_samplelight.h"

/*********************************************************************
 * CONSTANTS
 */

#define SAMPLELIGHT_DEVICE_VERSION     1
#define SAMPLELIGHT_FLAGS              0

#define SAMPLELIGHT_HWVERSION          1
#define SAMPLELIGHT_ZCLVERSION         1

#define DEFAULT_PHYSICAL_ENVIRONMENT 0
#define DEFAULT_DEVICE_ENABLE_STATE DEVICE_ENABLED
#define DEFAULT_IDENTIFY_TIME 0
#define DEFAULT_ON_OFF_TRANSITION_TIME 20
#define DEFAULT_ON_LEVEL ATTR_LEVEL_ON_LEVEL_NO_EFFECT
#define DEFAULT_ON_TRANSITION_TIME 20
#define DEFAULT_OFF_TRANSITION_TIME 20
#define DEFAULT_MOVE_RATE 0 // as fast as possible

#define DEFAULT_ON_OFF_STATE LIGHT_OFF
#define DEFAULT_LEVEL ATTR_LEVEL_MAX_LEVEL

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
 
//global attributes
const uint16 zclSampleLight_clusterRevision_all = 0x0001; //currently all cluster implementations are according to ZCL6, which has revision #1. In the future it is possible that different clusters will have different revisions, so they will have to use separate attribute variables.

// Basic Cluster
const uint8 zclSampleLight_HWRevision = SAMPLELIGHT_HWVERSION;
const uint8 zclSampleLight_ZCLVersion = SAMPLELIGHT_ZCLVERSION;
const uint8 zclSampleLight_ManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 zclSampleLight_ModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclSampleLight_DateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclSampleLight_PowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 zclSampleLight_LocationDescription[17];
uint8 zclSampleLight_PhysicalEnvironment;
uint8 zclSampleLight_DeviceEnable;

// Identify Cluster
uint16 zclSampleLight_IdentifyTime;

// Groups Cluster
uint8 zclSampleLight_GroupsNameSupport = 0;

// On/Off Cluster
uint8  zclSampleLight_OnOff;

// Level Control Cluster
#ifdef ZCL_LEVEL_CTRL
uint8  zclSampleLight_LevelCurrentLevel;
uint16 zclSampleLight_LevelRemainingTime;
uint16 zclSampleLight_LevelOnOffTransitionTime;
uint8  zclSampleLight_LevelOnLevel;
uint16 zclSampleLight_LevelOnTransitionTime;
uint16 zclSampleLight_LevelOffTransitionTime;
uint8  zclSampleLight_LevelDefaultMoveRate;
#endif

#if ZCL_DISCOVER
CONST zclCommandRec_t zclSampleLight_Cmds[] =
{
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    COMMAND_BASIC_RESET_FACT_DEFAULT,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_ON,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_TOGGLE,
    CMD_DIR_SERVER_RECEIVED
  },
#ifdef ZCL_LEVEL_CONTROL
  ,{
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  }
#endif // ZCL_LEVEL_CONTROL
};

CONST uint8 zclCmdsArraySize = ( sizeof(zclSampleLight_Cmds) / sizeof(zclSampleLight_Cmds[0]) );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

// NOTE: The attributes listed in the AttrRec must be in ascending order 
// per cluster to allow right function of the Foundation discovery commands
 
CONST zclAttrRec_t zclSampleLight_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclSampleLight_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclSampleLight_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclSampleLight_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclSampleLight_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)zclSampleLight_LocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleLight_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleLight_DeviceEnable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleLight_IdentifyTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },  
#endif

  // *** On/Off Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    { // Attribute record
      ATTRID_ON_OFF,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void *)&zclSampleLight_OnOff
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },
  
#ifdef ZCL_LEVEL_CTRL
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_CURRENT_LEVEL,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void *)&zclSampleLight_LevelCurrentLevel
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_REMAINING_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_LevelRemainingTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_OFF_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOnOffTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_LEVEL,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOnLevel
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOnTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_OFF_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOffTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_DEFAULT_MOVE_RATE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelDefaultMoveRate
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },
#endif
#ifdef ZCL_GROUPS
  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {
      ATTRID_GROUP_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void*)&zclSampleLight_GroupsNameSupport
    }
  },

  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },
#endif  
  {
    ZCL_CLUSTER_ID_GEN_SCENES,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },


#ifdef ZCL_DIAGNOSTIC
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NUMBER_OF_RESETS,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_RX_BCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_BCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_RX_UCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_UCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_RX_BCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_BCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_RX_UCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_UCAST_SUCCESS,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_UCAST_RETRY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_UCAST_FAIL,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_ROUTE_DISC_INITIATED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NEIGHBOR_ADDED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NEIGHBOR_REMOVED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NEIGHBOR_STALE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_JOIN_INDICATION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_CHILD_MOVED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NWK_FC_FAILURE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_FC_FAILURE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_UNAUTHORIZED_KEY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NWK_DECRYPT_FAILURES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_DECRYPT_FAILURES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_RELAYED_UCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI,
      ZCL_DATATYPE_INT8,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_clusterRevision_all
    }
  },
#endif // ZCL_DIAGNOSTIC



};

uint8 CONST zclSampleLight_NumAttributes = ( sizeof(zclSampleLight_Attrs) / sizeof(zclSampleLight_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
const cId_t zclSampleLight_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_ON_OFF
#ifdef ZCL_LEVEL_CTRL
  , ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL
#endif
};

#define ZCLSAMPLELIGHT_MAX_INCLUSTERS   (sizeof(zclSampleLight_InClusterList) / sizeof(zclSampleLight_InClusterList[0]))
 
SimpleDescriptionFormat_t zclSampleLight_SimpleDesc =
{
  SAMPLELIGHT_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId;
#ifdef ZCL_LEVEL_CTRL
  ZCL_HA_DEVICEID_DIMMABLE_LIGHT,        //  uint16 AppDeviceId;
#else
  ZCL_HA_DEVICEID_ON_OFF_LIGHT,          //  uint16 AppDeviceId;
#endif
  SAMPLELIGHT_DEVICE_VERSION,            //  int   AppDevVer:4;
  SAMPLELIGHT_FLAGS,                     //  int   AppFlags:4;
  ZCLSAMPLELIGHT_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclSampleLight_InClusterList, //  byte *pAppInClusterList;
  0,        //  byte  AppNumInClusters;
  NULL //  byte *pAppInClusterList;
};

// Added to include ZLL Target functionality 
#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
bdbTLDeviceInfo_t tlSampleLight_DeviceInfo =
{
  SAMPLELIGHT_ENDPOINT,                  //uint8 endpoint;
  ZCL_HA_PROFILE_ID,                        //uint16 profileID;
#ifdef ZCL_LEVEL_CTRL
      ZCL_HA_DEVICEID_DIMMABLE_LIGHT,        //  uint16 AppDeviceId;
#else
      ZCL_HA_DEVICEID_ON_OFF_LIGHT,          //  uint16 AppDeviceId;
#endif

  SAMPLELIGHT_DEVICE_VERSION,                    //uint8 version;
  SAMPLELIGHT_NUM_GRPS                   //uint8 grpIdCnt;
};
#endif
/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
 
/*********************************************************************
 * @fn      zclSampleLight_ResetAttributesToDefaultValues
 *
 * @brief   Reset all writable attributes to their default values.
 *
 * @param   none
 *
 * @return  none
 */
void zclSampleLight_ResetAttributesToDefaultValues(void)
{
  int i;
  
  zclSampleLight_LocationDescription[0] = 16;
  for (i = 1; i <= 16; i++)
  {
    zclSampleLight_LocationDescription[i] = ' ';
  }
  
  zclSampleLight_PhysicalEnvironment = DEFAULT_PHYSICAL_ENVIRONMENT;
  zclSampleLight_DeviceEnable = DEFAULT_DEVICE_ENABLE_STATE;
  
#ifdef ZCL_IDENTIFY
  zclSampleLight_IdentifyTime = DEFAULT_IDENTIFY_TIME;
#endif
#ifdef ZCL_LEVEL_CTRL
  zclSampleLight_LevelCurrentLevel = DEFAULT_LEVEL;
  zclSampleLight_LevelRemainingTime = 0;
  zclSampleLight_LevelOnOffTransitionTime = DEFAULT_ON_OFF_TRANSITION_TIME;
  zclSampleLight_LevelOnLevel = DEFAULT_ON_LEVEL;
  zclSampleLight_LevelOnTransitionTime = DEFAULT_ON_TRANSITION_TIME;
  zclSampleLight_LevelOffTransitionTime = DEFAULT_OFF_TRANSITION_TIME;
  zclSampleLight_LevelDefaultMoveRate = DEFAULT_MOVE_RATE;
#endif
  zclSampleLight_OnOff = DEFAULT_ON_OFF_STATE;
  
  zclSampleLight_IdentifyTime = 0;
}

/****************************************************************************
****************************************************************************/
