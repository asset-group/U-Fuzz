/**************************************************************************************************
  Filename:       zcl.h
  Revised:        $Date: 2014-11-25 09:19:55 -0800 (Tue, 25 Nov 2014) $
  Revision:       $Revision: 41240 $

  Description:    This file contains the Zigbee Cluster Library Foundation definitions.


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

#ifndef ZCL_H
#define ZCL_H

#if defined ( BDB_REPORTING )
#ifndef ZCL_REPORTING_DEVICE
  #define ZCL_REPORTING_DEVICE
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#if !defined ( ZCL_STANDALONE ) || defined ( ZCL_STANDALONE_OSAL )
  #include "OSAL.h"
  #include "OSAL_Nv.h"
  #include "OSAL_Tasks.h"
#endif

#include "AF.h"
#include "aps_groups.h"

/*********************************************************************
 * CONSTANTS
 */

#define ZCL_DATABUF_SEND                                     0x0020  
  
// General Clusters
#define ZCL_CLUSTER_ID_GEN_BASIC                             0x0000
#define ZCL_CLUSTER_ID_GEN_POWER_CFG                         0x0001
#define ZCL_CLUSTER_ID_GEN_DEVICE_TEMP_CONFIG                0x0002
#define ZCL_CLUSTER_ID_GEN_IDENTIFY                          0x0003
#define ZCL_CLUSTER_ID_GEN_GROUPS                            0x0004
#define ZCL_CLUSTER_ID_GEN_SCENES                            0x0005
#define ZCL_CLUSTER_ID_GEN_ON_OFF                            0x0006
#define ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG              0x0007
#define ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL                     0x0008
#define ZCL_CLUSTER_ID_GEN_ALARMS                            0x0009
#define ZCL_CLUSTER_ID_GEN_TIME                              0x000A
#define ZCL_CLUSTER_ID_GEN_LOCATION                          0x000B
#define ZCL_CLUSTER_ID_GEN_ANALOG_INPUT_BASIC                0x000C
#define ZCL_CLUSTER_ID_GEN_ANALOG_OUTPUT_BASIC               0x000D
#define ZCL_CLUSTER_ID_GEN_ANALOG_VALUE_BASIC                0x000E
#define ZCL_CLUSTER_ID_GEN_BINARY_INPUT_BASIC                0x000F
#define ZCL_CLUSTER_ID_GEN_BINARY_OUTPUT_BASIC               0x0010
#define ZCL_CLUSTER_ID_GEN_BINARY_VALUE_BASIC                0x0011
#define ZCL_CLUSTER_ID_GEN_MULTISTATE_INPUT_BASIC            0x0012
#define ZCL_CLUSTER_ID_GEN_MULTISTATE_OUTPUT_BASIC           0x0013
#define ZCL_CLUSTER_ID_GEN_MULTISTATE_VALUE_BASIC            0x0014
#define ZCL_CLUSTER_ID_GEN_COMMISSIONING                     0x0015
#define ZCL_CLUSTER_ID_GEN_PARTITION                         0x0016

#define ZCL_CLUSTER_ID_OTA                                   0x0019

#define ZCL_CLUSTER_ID_GEN_POWER_PROFILE                     0x001A
#define ZCL_CLUSTER_ID_GEN_APPLIANCE_CONTROL                 0x001B

#define ZCL_CLUSTER_ID_GEN_POLL_CONTROL                      0x0020

#define ZCL_CLUSTER_ID_GREEN_POWER                           0x0021

// Retail Clusters
#define ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIGURATION           0x0022
#define ZCL_CLUSTER_ID_NEIGHBOR_CLEANING                     0x0023
#define ZCL_CLUSTER_ID_NEAREST_GATEWAY                       0x0024
   
// Closures Clusters
#define ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG                 0x0100
#define ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK                    0x0101
#define ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING              0x0102

// HVAC Clusters
#define ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL              0x0200
#define ZCL_CLUSTER_ID_HVAC_THERMOSTAT                       0x0201
#define ZCL_CLUSTER_ID_HVAC_FAN_CONTROL                      0x0202
#define ZCL_CLUSTER_ID_HVAC_DIHUMIDIFICATION_CONTROL         0x0203
#define ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG            0x0204

// Lighting Clusters
#define ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL                0x0300
#define ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG               0x0301

// Measurement and Sensing Clusters
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT            0x0400
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG   0x0401
#define ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT            0x0402
#define ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT               0x0403
#define ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT                   0x0404
#define ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY                  0x0405
#define ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING                  0x0406

// Security and Safety (SS) Clusters
#define ZCL_CLUSTER_ID_SS_IAS_ZONE                           0x0500
#define ZCL_CLUSTER_ID_SS_IAS_ACE                            0x0501
#define ZCL_CLUSTER_ID_SS_IAS_WD                             0x0502

// Protocol Interfaces
#define ZCL_CLUSTER_ID_PI_GENERIC_TUNNEL                     0x0600
#define ZCL_CLUSTER_ID_PI_BACNET_PROTOCOL_TUNNEL             0x0601
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_REG            0x0602
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_EXT            0x0603
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_REG           0x0604
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_EXT           0x0605
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_REG            0x0606
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_EXT            0x0607
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_REG            0x0608
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_EXT            0x0609
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_REG           0x060A
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_EXT           0x060B
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_REG            0x060C
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_EXT            0x060D
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_REG        0x060E
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_EXT        0x060F
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_REG       0x0610
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_EXT       0x0611
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_REG        0x0612
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_EXT        0x0613
#define ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL              0x0614
#define ZCL_CLUSTER_ID_PI_ISO7818_PROTOCOL_TUNNEL            0x0615
#define ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL                      0x0617

// Smart Energy (SE) Clusters
#define ZCL_CLUSTER_ID_SE_PRICE                              0x0700
#define ZCL_CLUSTER_ID_SE_DRLC                               0x0701
#define ZCL_CLUSTER_ID_SE_METERING                           0x0702
#define ZCL_CLUSTER_ID_SE_MESSAGING                          0x0703
#define ZCL_CLUSTER_ID_SE_TUNNELING                          0x0704
#define ZCL_CLUSTER_ID_SE_PREPAYMENT                         0x0705
#define ZCL_CLUSTER_ID_SE_ENERGY_MGMT                        0x0706
#define ZCL_CLUSTER_ID_SE_CALENDAR                           0x0707
#define ZCL_CLUSTER_ID_SE_DEVICE_MGMT                        0x0708
#define ZCL_CLUSTER_ID_SE_EVENTS                             0x0709
#define ZCL_CLUSTER_ID_SE_MDU_PAIRING                        0x070A
   
#define ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT                  0x0800
  
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION        0x0900
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE  0x0904
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING           0x0905
   
#define ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION           0x0B00
#define ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION               0x0B01
#define ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS            0x0B02
#define ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS               0x0B03
#define ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT             0x0B04
#define ZCL_CLUSTER_ID_HA_DIAGNOSTIC                         0x0B05

// Light Link cluster
#define ZCL_CLUSTER_ID_TOUCHLINK                             0x1000

 
/*** Frame Control bit mask ***/
#define ZCL_FRAME_CONTROL_TYPE                          0x03
#define ZCL_FRAME_CONTROL_MANU_SPECIFIC                 0x04
#define ZCL_FRAME_CONTROL_DIRECTION                     0x08
#define ZCL_FRAME_CONTROL_DISABLE_DEFAULT_RSP           0x10

/*** Frame Types ***/
#define ZCL_FRAME_TYPE_PROFILE_CMD                      0x00
#define ZCL_FRAME_TYPE_SPECIFIC_CMD                     0x01

/*** Frame Client/Server Directions ***/
#define ZCL_FRAME_CLIENT_SERVER_DIR                     0x00
#define ZCL_FRAME_SERVER_CLIENT_DIR                     0x01

/*** Chipcon Manufacturer Code ***/
#define CC_MANUFACTURER_CODE                            0x1001

/*** Foundation Command IDs ***/
#define ZCL_CMD_READ                                    0x00
#define ZCL_CMD_READ_RSP                                0x01
#define ZCL_CMD_WRITE                                   0x02
#define ZCL_CMD_WRITE_UNDIVIDED                         0x03
#define ZCL_CMD_WRITE_RSP                               0x04
#define ZCL_CMD_WRITE_NO_RSP                            0x05
#define ZCL_CMD_CONFIG_REPORT                           0x06
#define ZCL_CMD_CONFIG_REPORT_RSP                       0x07
#define ZCL_CMD_READ_REPORT_CFG                         0x08
#define ZCL_CMD_READ_REPORT_CFG_RSP                     0x09
#define ZCL_CMD_REPORT                                  0x0a
#define ZCL_CMD_DEFAULT_RSP                             0x0b
#define ZCL_CMD_DISCOVER_ATTRS                          0x0c
#define ZCL_CMD_DISCOVER_ATTRS_RSP                      0x0d
#define ZCL_CMD_DISCOVER_CMDS_RECEIVED                  0x11
#define ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP              0x12
#define ZCL_CMD_DISCOVER_CMDS_GEN                       0x13
#define ZCL_CMD_DISCOVER_CMDS_GEN_RSP                   0x14
#define ZCL_CMD_DISCOVER_ATTRS_EXT                      0x15
#define ZCL_CMD_DISCOVER_ATTRS_EXT_RSP                  0x16

#define ZCL_CMD_MAX           ZCL_CMD_DISCOVER_ATTRS_EXT_RSP

// define reporting constant
#define ZCL_REPORTING_OFF     0xFFFF  // turn off reporting (maxReportInt)

// define command direction flag masks
#define CMD_DIR_SERVER_GENERATED          0x01
#define CMD_DIR_CLIENT_GENERATED          0x02
#define CMD_DIR_SERVER_RECEIVED           0x04
#define CMD_DIR_CLIENT_RECEIVED           0x08

/*** Data Types ***/
#define ZCL_DATATYPE_NO_DATA                            0x00
#define ZCL_DATATYPE_DATA8                              0x08
#define ZCL_DATATYPE_DATA16                             0x09
#define ZCL_DATATYPE_DATA24                             0x0a
#define ZCL_DATATYPE_DATA32                             0x0b
#define ZCL_DATATYPE_DATA40                             0x0c
#define ZCL_DATATYPE_DATA48                             0x0d
#define ZCL_DATATYPE_DATA56                             0x0e
#define ZCL_DATATYPE_DATA64                             0x0f
#define ZCL_DATATYPE_BOOLEAN                            0x10
#define ZCL_DATATYPE_BITMAP8                            0x18
#define ZCL_DATATYPE_BITMAP16                           0x19
#define ZCL_DATATYPE_BITMAP24                           0x1a
#define ZCL_DATATYPE_BITMAP32                           0x1b
#define ZCL_DATATYPE_BITMAP40                           0x1c
#define ZCL_DATATYPE_BITMAP48                           0x1d
#define ZCL_DATATYPE_BITMAP56                           0x1e
#define ZCL_DATATYPE_BITMAP64                           0x1f
#define ZCL_DATATYPE_UINT8                              0x20
#define ZCL_DATATYPE_UINT16                             0x21
#define ZCL_DATATYPE_UINT24                             0x22
#define ZCL_DATATYPE_UINT32                             0x23
#define ZCL_DATATYPE_UINT40                             0x24
#define ZCL_DATATYPE_UINT48                             0x25
#define ZCL_DATATYPE_UINT56                             0x26
#define ZCL_DATATYPE_UINT64                             0x27
#define ZCL_DATATYPE_INT8                               0x28
#define ZCL_DATATYPE_INT16                              0x29
#define ZCL_DATATYPE_INT24                              0x2a
#define ZCL_DATATYPE_INT32                              0x2b
#define ZCL_DATATYPE_INT40                              0x2c
#define ZCL_DATATYPE_INT48                              0x2d
#define ZCL_DATATYPE_INT56                              0x2e
#define ZCL_DATATYPE_INT64                              0x2f
#define ZCL_DATATYPE_ENUM8                              0x30
#define ZCL_DATATYPE_ENUM16                             0x31
#define ZCL_DATATYPE_SEMI_PREC                          0x38
#define ZCL_DATATYPE_SINGLE_PREC                        0x39
#define ZCL_DATATYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATATYPE_OCTET_STR                          0x41
#define ZCL_DATATYPE_CHAR_STR                           0x42
#define ZCL_DATATYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATATYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATATYPE_ARRAY                              0x48
#define ZCL_DATATYPE_STRUCT                             0x4c
#define ZCL_DATATYPE_SET                                0x50
#define ZCL_DATATYPE_BAG                                0x51
#define ZCL_DATATYPE_TOD                                0xe0
#define ZCL_DATATYPE_DATE                               0xe1
#define ZCL_DATATYPE_UTC                                0xe2
#define ZCL_DATATYPE_CLUSTER_ID                         0xe8
#define ZCL_DATATYPE_ATTR_ID                            0xe9
#define ZCL_DATATYPE_BAC_OID                            0xea
#define ZCL_DATATYPE_IEEE_ADDR                          0xf0
#define ZCL_DATATYPE_128_BIT_SEC_KEY                    0xf1
#define ZCL_DATATYPE_UNKNOWN                            0xff

/*** Error Status Codes ***/
#define ZCL_STATUS_SUCCESS                              0x00
#define ZCL_STATUS_FAILURE                              0x01
// 0x02-0x7D are reserved.
#define ZCL_STATUS_NOT_AUTHORIZED                       0x7E
#define ZCL_STATUS_MALFORMED_COMMAND                    0x80
#define ZCL_STATUS_UNSUP_CLUSTER_COMMAND                0x81
#define ZCL_STATUS_UNSUP_GENERAL_COMMAND                0x82
#define ZCL_STATUS_UNSUP_MANU_CLUSTER_COMMAND           0x83
#define ZCL_STATUS_UNSUP_MANU_GENERAL_COMMAND           0x84
#define ZCL_STATUS_INVALID_FIELD                        0x85
#define ZCL_STATUS_UNSUPPORTED_ATTRIBUTE                0x86
#define ZCL_STATUS_INVALID_VALUE                        0x87
#define ZCL_STATUS_READ_ONLY                            0x88
#define ZCL_STATUS_INSUFFICIENT_SPACE                   0x89
#define ZCL_STATUS_DUPLICATE_EXISTS                     0x8a
#define ZCL_STATUS_NOT_FOUND                            0x8b
#define ZCL_STATUS_UNREPORTABLE_ATTRIBUTE               0x8c
#define ZCL_STATUS_INVALID_DATA_TYPE                    0x8d
#define ZCL_STATUS_INVALID_SELECTOR                     0x8e
#define ZCL_STATUS_WRITE_ONLY                           0x8f
#define ZCL_STATUS_INCONSISTENT_STARTUP_STATE           0x90
#define ZCL_STATUS_DEFINED_OUT_OF_BAND                  0x91
#define ZCL_STATUS_INCONSISTENT                         0x92
#define ZCL_STATUS_ACTION_DENIED                        0x93
#define ZCL_STATUS_TIMEOUT                              0x94
#define ZCL_STATUS_ABORT                                0x95
#define ZCL_STATUS_INVALID_IMAGE                        0x96
#define ZCL_STATUS_WAIT_FOR_DATA                        0x97
#define ZCL_STATUS_NO_IMAGE_AVAILABLE                   0x98
#define ZCL_STATUS_REQUIRE_MORE_IMAGE                   0x99

// 0xbd-bf are reserved.
#define ZCL_STATUS_HARDWARE_FAILURE                     0xc0
#define ZCL_STATUS_SOFTWARE_FAILURE                     0xc1
#define ZCL_STATUS_CALIBRATION_ERROR                    0xc2
// 0xc3-0xff are reserved.
#define ZCL_STATUS_CMD_HAS_RSP                          0xFF // Non-standard status (used for Default Rsp)

/*** Attribute Access Control - bit masks ***/
#define ACCESS_CONTROL_READ                             0x01  // attribute can be read
#define ACCESS_CONTROL_WRITE                            0x02  // attribute can be written
#define ACCESS_REPORTABLE                               0x04  // indicate attribute is reportable
#define ACCESS_CONTROL_COMMAND                          0x08
#define ACCESS_CONTROL_AUTH_READ                        0x10
#define ACCESS_CONTROL_AUTH_WRITE                       0x20
#define ACCESS_GLOBAL                                   0x40  // TI unique to indicate attributes that are in both, client and server side of the cluster in the endpoint  
#define ACCESS_CLIENT                                   0x80  // TI unique, indicate client side attribute
//NOTE: If no global or client access is define, then server side of the attribute is assumed

// Access Control for client
#define ACCESS_CONTROL_MASK                             0x07
   
// Access Control as reported OTA via DiscoveryAttributesExtended
#define ACCESS_CONTROLEXT_MASK                          0x07  // read/write/reportable bits same as above

#define ZCL_ATTR_ID_MAX                                 0xFFFF

// Used by Configure Reporting Command
#define ZCL_SEND_ATTR_REPORTS                           0x00
#define ZCL_EXPECT_ATTR_REPORTS                         0x01

// Predefined Maximum String Length
#define MAX_UTF8_STRING_LEN                             50

// Used by zclReadWriteCB_t callback function
#define ZCL_OPER_LEN                                    0x00 // Get length of attribute value to be read
#define ZCL_OPER_READ                                   0x01 // Read attribute value
#define ZCL_OPER_WRITE                                  0x02 // Write new attribute value

#define ATTRID_CLUSTER_REVISION                         0xFFFD // The ClusterRevision global attribute is mandatory for all cluster instances, client and server, conforming to ZCL revision 6 (ZCL6) and later ZCL revisions.

/*********************************************************************
 * MACROS
 */
#define zcl_ProfileCmd( a )         ( (a) == ZCL_FRAME_TYPE_PROFILE_CMD )
#define zcl_ClusterCmd( a )         ( (a) == ZCL_FRAME_TYPE_SPECIFIC_CMD )

#define zcl_ServerCmd( a )          ( (a) == ZCL_FRAME_CLIENT_SERVER_DIR )
#define zcl_ClientCmd( a )          ( (a) == ZCL_FRAME_SERVER_CLIENT_DIR )

#define UNICAST_MSG( msg )          ( (msg)->wasBroadcast == false && (msg)->groupId == 0 )

// Padding needed if buffer has odd number of octects in length
#define PADDING_NEEDED( bufLen )    ( (bufLen) % 2 )

// Check for Cluster IDs
#define ZCL_CLUSTER_ID_GEN( id )      ( /* (id) >= ZCL_CLUSTER_ID_GEN_BASIC &&*/ \
                                        (id) <= ZCL_CLUSTER_ID_GEN_COMMISSIONING )
#define ZCL_CLUSTER_ID_CLOSURES( id ) ( (id) >= ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG && \
                                        (id) <= ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING )
#define ZCL_CLUSTER_ID_HVAC( id )     ( (id) >= ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL && \
                                        (id) <= ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG )
#define ZCL_CLUSTER_ID_LIGHTING( id ) ( (id) >= ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL && \
                                        (id) <= ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG )
#define ZCL_CLUSTER_ID_MS( id )       ( (id) >= ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT && \
                                        (id) <= ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING )
#define ZCL_CLUSTER_ID_SS( id )       ( (id) >= ZCL_CLUSTER_ID_SS_IAS_ZONE && \
                                        (id) <= ZCL_CLUSTER_ID_SS_IAS_WD )
#define ZCL_CLUSTER_ID_KEY( id )      ( (id) == ZCL_CLUSTER_ID_GEN_KEY_ESTABLISHMENT )
#define ZCL_CLUSTER_ID_SE( id )       ( (id) >= ZCL_CLUSTER_ID_SE_PRICE && \
                                        (id) <= ZCL_CLUSTER_ID_SE_MDU_PAIRING )
#define ZCL_CLUSTER_ID_PI( id )       ( (id) >= ZCL_CLUSTER_ID_PI_GENERIC_TUNNEL && \
                                        (id) <= ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL )
#define ZCL_CLUSTER_ID_LL( id )       ( (id) == ZCL_CLUSTER_ID_LIGHT_LINK )
#define ZCL_CLUSTER_ID_PART( id )     ( (id) == ZCL_CLUSTER_ID_GEN_PARTITION )

#define ZCL_CLUSTER_ID_PC( id )       ( (id) == ZCL_CLUSTER_ID_GEN_POLL_CONTROL )

#define ZCL_CLUSTER_ID_EM( id )	      ( (id) == ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT )

#define ZCL_CLUSTER_ID_DIAG( id )     ( (id) == ZCL_CLUSTER_ID_HA_DIAGNOSTIC )

#define ZCL_CLUSTER_ID_MI( id )       ( (id) == ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION )

#define ZCL_CLUSTER_ID_APPL_C( id )   ( (id) == ZCL_CLUSTER_ID_GEN_APPLIANCE_CONTROL )

#define ZCL_CLUSTER_ID_APPL_I( id )   ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION )

#define ZCL_CLUSTER_ID_APPL_STAT( id ) ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS )

#define ZCL_CLUSTER_ID_APPL_E_A( id ) ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS )

#define ZCL_CLUSTER_ID_APPL_S( id )   ( (id) == ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS )

#define ZCL_CLUSTER_ID_PP( id )       ( (id) == ZCL_CLUSTER_ID_GEN_POWER_PROFILE )

#define ZCL_CLUSTER_ID_DL( id )       ( (id) == ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK )

/*********************************************************************
 * TYPEDEFS
 */
// zcl_ProcessMessageMSG() return codes
typedef enum
{
  ZCL_PROC_SUCCESS = 0,                 // Message was processed
  ZCL_PROC_INVALID ,                    // Format or parameter was wrong
  ZCL_PROC_EP_NOT_FOUND,                // Endpoint descriptor not found
  ZCL_PROC_NOT_OPERATIONAL,             // Can't respond to this command
  ZCL_PROC_INTERPAN_FOUNDATION_CMD,     // INTER-PAN and Foundation Command (not allowed)
  ZCL_PROC_NOT_SECURE,                  // Security was required but the message is not secure
  ZCL_PROC_MANUFACTURER_SPECIFIC,       // Manufacturer Specific command - not handled
  ZCL_PROC_MANUFACTURER_SPECIFIC_DR,    // Manufacturer Specific command - not handled, but default response sent
  ZCL_PROC_NOT_HANDLED,                 // No default response was sent and the message was not handled
  ZCL_PROC_NOT_HANDLED_DR,              // default response was sent and the message was not handled
} zclProcMsgStatus_t;

// ZCL header - frame control field
typedef struct
{
  unsigned int type:2;
  unsigned int manuSpecific:1;
  unsigned int direction:1;
  unsigned int disableDefaultRsp:1;
  unsigned int reserved:3;
} zclFrameControl_t;

// ZCL header
typedef struct
{
  zclFrameControl_t fc;
  uint16            manuCode;
  uint8             transSeqNum;
  uint8             commandID;
} zclFrameHdr_t;

#ifdef ZCL_READ
// Read Attribute Command format
typedef struct
{
  uint8  numAttr;            // number of attributes in the list
  uint16 attrID[];           // supported attributes list - this structure should
                             // be allocated with the appropriate number of attributes.
} zclReadCmd_t;

// Read Attribute Response Status record
typedef struct
{
  uint16 attrID;            // attribute ID
  uint8  status;            // should be ZCL_STATUS_SUCCESS or error
  uint8  dataType;          // attribute data type
  uint8  *data;             // this structure is allocated, so the data is HERE
                            // - the size depends on the attribute data type
} zclReadRspStatus_t;

// Read Attribute Response Command format
typedef struct
{
  uint8              numAttr;     // number of attributes in the list
  zclReadRspStatus_t attrList[];  // attribute status list
} zclReadRspCmd_t;
#endif // ZCL_READ

// Write Attribute record
typedef struct
{
  uint16 attrID;             // attribute ID
  uint8  dataType;           // attribute data type
  uint8  *attrData;          // this structure is allocated, so the data is HERE
                             //  - the size depends on the attribute data type
} zclWriteRec_t;

// Write Attribute Command format
typedef struct
{
  uint8         numAttr;     // number of attribute records in the list
  zclWriteRec_t attrList[];  // attribute records
} zclWriteCmd_t;

// Write Attribute Status record
typedef struct
{
  uint8  status;             // should be ZCL_STATUS_SUCCESS or error
  uint16 attrID;             // attribute ID
} zclWriteRspStatus_t;

// Write Attribute Response Command format
typedef struct
{
  uint8               numAttr;     // number of attribute status in the list
  zclWriteRspStatus_t attrList[];  // attribute status records
} zclWriteRspCmd_t;

// Configure Reporting Command format
typedef struct
{
  uint8  direction;          // to send or receive reports of the attribute
  uint16 attrID;             // attribute ID
  uint8  dataType;           // attribute data type
  uint16 minReportInt;       // minimum reporting interval
  uint16 maxReportInt;       // maximum reporting interval, 0xFFFF=off
  uint16 timeoutPeriod;      // timeout period
  uint8  *reportableChange;  // reportable change (only applicable to analog data type)
                             // - the size depends on the attribute data type
} zclCfgReportRec_t;

typedef struct
{
  uint8             numAttr;    // number of attribute IDs in the list
  zclCfgReportRec_t attrList[]; // attribute ID list
} zclCfgReportCmd_t;

// Attribute Status record
typedef struct
{
  uint8  status;             // should be ZCL_STATUS_SUCCESS or error
  uint8  direction;          // whether attributes are reported or reports of attributes are received
  uint16 attrID;             // attribute ID
} zclCfgReportStatus_t;

// Configure Reporting Response Command format
typedef struct
{
  uint8                numAttr;    // number of attribute status in the list
  zclCfgReportStatus_t attrList[]; // attribute status records
} zclCfgReportRspCmd_t;

// Read Reporting Configuration Command format
typedef struct
{
  uint8  direction; // to send or receive reports of the attribute
  uint16 attrID;    // attribute ID
} zclReadReportCfgRec_t;

typedef struct
{
  uint8                 numAttr;    // number of attributes in the list
  zclReadReportCfgRec_t attrList[]; // attribute ID list
} zclReadReportCfgCmd_t;

// Attribute Reporting Configuration record
typedef struct
{
  uint8  status;             // status field
  uint8  direction;          // to send or receive reports of the attribute
  uint16 attrID;             // attribute ID
  uint8  dataType;           // attribute data type
  uint16 minReportInt;       // minimum reporting interval
  uint16 maxReportInt;       // maximum reporting interval
  uint16 timeoutPeriod;      // timeout period
  uint8  *reportableChange;  // reportable change (only applicable to analog data type)
                             // - the size depends on the attribute data type
} zclReportCfgRspRec_t;

// Read Reporting Configuration Response Command format
typedef struct
{
  uint8                numAttr;    // number of records in the list
  zclReportCfgRspRec_t attrList[]; // attribute reporting configuration list
} zclReadReportCfgRspCmd_t;

// Attribute Report
typedef struct
{
  uint16 attrID;             // atrribute ID
  uint8  dataType;           // attribute data type
  uint8  *attrData;          // this structure is allocated, so the data is HERE
                             // - the size depends on the data type of attrID
} zclReport_t;

// Report Attributes Command format
typedef struct
{
  uint8       numAttr;       // number of reports in the list
  zclReport_t attrList[];    // attribute report list
} zclReportCmd_t;

// Default Response Command format
typedef struct
{
  uint8  commandID;
  uint8  statusCode;
} zclDefaultRspCmd_t;

// Discover Attributes and Attributes Extended Command format
typedef struct
{
  uint16 startAttr;          // specifies the minimum value of the identifier
                             // to begin attribute discovery.
  uint8  maxAttrIDs;         // maximum number of attribute IDs that are to be
                             // returned in the resulting response command.
} zclDiscoverAttrsCmd_t;

// Attribute Report info
typedef struct
{
  uint16 attrID;             // attribute ID
  uint8  dataType;           // attribute data type (see Table 17 in Spec)
} zclDiscoverAttrInfo_t;

// Discover Attributes Response Command format
typedef struct
{
  uint8             discComplete; // whether or not there're more attributes to be discovered
  uint8             numAttr;      // number of attributes in the list
  zclDiscoverAttrInfo_t attrList[];   // supported attributes list
} zclDiscoverAttrsRspCmd_t;

// String Data Type
typedef struct
{
  uint8 strLen;
  uint8 *pStr;
} UTF8String_t;

// Command format for the following:
// Discover Commands Received, Discover Commands Generated
typedef struct
{
  uint8 startCmdID;
  uint8 maxCmdID;
} zclDiscoverCmdsCmd_t;

// Command format for the following:
// Discover Commands Received Response Command, Discover Commands Generated Response
typedef struct
{
  uint8 discComplete;
  uint8 cmdType;    // either ZCL_CMD_DISCOVER_CMDS_GEN or ZCL_CMD_DISCOVER_CMDS_RECEIVED
  uint8 numCmd;     // number of provided commands
  uint8 *pCmdID;    // variable length array
} zclDiscoverCmdsCmdRsp_t;

// Discover Attributes Extended Response Command format
typedef struct
{
  uint16 attrID;
  uint8 attrDataType;
  uint8 attrAccessControl;
} zclExtAttrInfo_t;

typedef struct
{
  uint8 discComplete;
  uint8 numAttr;                  // number of attributes provided
  zclExtAttrInfo_t  aExtAttrInfo[];    // variable length array
} zclDiscoverAttrsExtRsp_t;

/*********************************************************************
 * Plugins
 */

// Incoming ZCL message, this buffer will be allocated, cmd will point to the
// the command record.
typedef struct
{
  afIncomingMSGPacket_t *msg;        // incoming message
  zclFrameHdr_t         hdr;         // ZCL header parsed
  uint8                 *pData;      // pointer to data after header
  uint16                pDataLen;    // length of remaining data
  void                  *attrCmd;    // pointer to the parsed attribute or command
} zclIncoming_t;

// Outgoing ZCL Cluster Specific Commands
typedef struct
{
  zclFrameHdr_t hdr;
  uint16        realClusterID;
  uint16        attrID;
  void          *cmdStruct;
  uint8         cmdLen;
  uint8         *cmdData;
} zclOutgoingCmd_t;

// Incoming ZCL message passed to the Application. This buffer will be
// allocated and attrCmd will point to the command record.
//
// NOTE - the Application must deallocate the message plus attrCmd buffer.
//
typedef struct
{
  osal_event_hdr_t hdr;         // OSAL header
  zclFrameHdr_t    zclHdr;      // ZCL header parsed
  uint16           clusterId;   // Cluster ID
  afAddrType_t     srcAddr;     // Sender's address
  uint8            endPoint;    // destination endpoint
  void             *attrCmd;    // pointer to the parsed attribute or command; must be freed by Application
} zclIncomingMsg_t;

// Function pointer type to handle incoming messages.
//   msg - incoming message
//   logicalClusterID - logical cluster ID
typedef ZStatus_t (*zclInHdlr_t)( zclIncoming_t *pInHdlrMsg );

// Function pointer type to handle incoming write commands.
//   msg - incoming message
//   logicalClusterID - logical cluster ID
//   writeRec - received data to be written
typedef ZStatus_t (*zclInWrtHdlr_t)( zclIncoming_t *msg, uint16 logicalClusterID, zclWriteRec_t *writeRec );

// Command record
typedef struct
{
  uint16   clusterID;
  uint8    cmdID;
  uint8    flag;  // one of CMD_DIR_CLIENT_GENERATED, CMD_DIR_CLIENT_RECEIVED, CMD_DIR_SERVER_GENERATED, CMD_DIR_SERVER_RECEIVED
} zclCommandRec_t;

// Attribute record
typedef struct
{
  uint16  attrId;         // Attribute ID
  uint8   dataType;       // Data Type - defined in AF.h
  uint8   accessControl;  // Read/write - bit field
  void    *dataPtr;       // Pointer to data field
} zclAttribute_t;

typedef struct
{
  uint16          clusterID;    // Real cluster ID
  zclAttribute_t  attr;
} zclAttrRec_t;

// Function pointer type to validate attribute data.
//
//   pAttr - where data to be written
//   pAttrInfo - pointer to attribute info
//
//   return  TRUE if data valid. FALSE, otherwise.
typedef uint8 (*zclValidateAttrData_t)( zclAttrRec_t *pAttr, zclWriteRec_t *pAttrInfo );

// Function pointer type to read/write attribute data.
//
//   clusterId - cluster that attribute belongs to
//   attrId - attribute to be read or written
//   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
//   pValue - pointer to attribute (length) value
//   pLen - length of attribute value read
//
//   return  ZCL_STATUS_SUCCESS: Operation successful
//           ZCL Error Status: Operation not successful
typedef ZStatus_t (*zclReadWriteCB_t)( uint16 clusterId, uint16 attrId, uint8 oper,
                                       uint8 *pValue, uint16 *pLen );

// Callback function prototype to authorize a Read or Write operation
//   on a given attribute.
//
//   srcAddr - source Address
//   pAttr - pointer to attribute
//   oper - ZCL_OPER_READ, or ZCL_OPER_WRITE
//
//   return  ZCL_STATUS_SUCCESS: Operation authorized
//           ZCL_STATUS_NOT_AUTHORIZED: Operation not authorized
typedef ZStatus_t (*zclAuthorizeCB_t)( afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper );

typedef struct
{
  uint16  clusterID;      // Real cluster ID
  uint8   option;
} zclOptionRec_t;

// Parse received command
typedef struct
{
  uint8  endpoint;
  uint16 dataLen;
  uint8  *pData;
} zclParseCmd_t;

// Attribute record list item
typedef struct zclAttrRecsList
{
  struct zclAttrRecsList *next;
  uint8                  endpoint;      // Used to link it into the endpoint descriptor
  zclReadWriteCB_t       pfnReadWriteCB;// Read or Write attribute value callback function
  zclAuthorizeCB_t       pfnAuthorizeCB;// Authorize Read or Write operation
  uint8                  numAttributes; // Number of the following records
  CONST zclAttrRec_t     *attrs;        // attribute records
} zclAttrRecsList;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8 zcl_TaskID;
//extern uint8 zcl_SeqNum;  Not longer used, refer to bdb_getZCLFrameCounter() in bdb_interface.h
extern uint8 zcl_InSeqNum;
extern uint8 zcl_radius;

/*********************************************************************
 * FUNCTION MACROS
 */
#ifdef ZCL_WRITE
/*
 *  Send a Write Command - ZCL_CMD_WRITE
 *  Use like:
 *      ZStatus_t zcl_SendWrite( uint8 srcEP, afAddrType_t *dstAddr, uint16 realClusterID, zclWriteCmd_t *writeCmd, uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zcl_SendWrite(a,b,c,d,e,f,g) (zcl_SendWriteRequest( (a), (b), (c), (d), ZCL_CMD_WRITE, (e), (f), (g) ))

/*
 *  Send a Write Undivided Command - ZCL_CMD_WRITE_UNDIVIDED
 *  Use like:
 *      ZStatus_t zcl_SendWriteUndivided( uint8 srcEP, afAddrType_t *dstAddr, uint16 realClusterID, zclWriteCmd_t *writeCmd, uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zcl_SendWriteUndivided(a,b,c,d,e,f,g) (zcl_SendWriteRequest( (a), (b), (c), (d), ZCL_CMD_WRITE_UNDIVIDED, (e), (f), (g) ))

/*
 *  Send a Write No Response Command - ZCL_CMD_WRITE_NO_RSP
 *  Use like:
 *      ZStatus_t zcl_SendWriteNoRsp( uint8 srcEP, afAddrType_t *dstAddr, uint16 realClusterID, zclWriteCmd_t *writeCmd, uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zcl_SendWriteNoRsp(a,b,c,d,e,f,g) (zcl_SendWriteRequest( (a), (b), (c), (d), ZCL_CMD_WRITE_NO_RSP, (e), (f), (g) ))
#endif // ZCL_WRITE

#if !defined ( ZCL_STANDALONE ) || defined ( ZCL_STANDALONE_OSAL )
  #define zcl_mem_alloc      osal_mem_alloc
  #define zcl_memset         osal_memset
  #define zcl_memcpy         osal_memcpy
  #define zcl_mem_free       osal_mem_free
  #define zcl_buffer_uint32  osal_buffer_uint32
  #define zcl_nv_item_init   osal_nv_item_init
  #define zcl_nv_write       osal_nv_write
  #define zcl_nv_read        osal_nv_read
  #define zcl_build_uint32   osal_build_uint32
  #define zcl_cpyExtAddr     osal_cpyExtAddr
#else
  extern void *zcl_mem_alloc( uint16 size );
  extern void *zcl_memset( void *dest, uint8 value, int len );
  extern void *zcl_memcpy( void *dst, void *src, unsigned int len );
  extern void zcl_mem_free(void *ptr);
  extern uint8* zcl_buffer_uint32( uint8 *buf, uint32 val );
  extern uint8 zcl_nv_item_init( uint16 id, uint16 len, void *buf );
  extern uint8 zcl_nv_write( uint16 id, uint16 ndx, uint16 len, void *buf );
  extern uint8 zcl_nv_read( uint16 id, uint16 ndx, uint16 len, void *buf );
  extern uint32 zcl_build_uint32( uint8 *swapped, uint8 len );
  extern void *zcl_cpyExtAddr(uint8 * pDest, const uint8 * pSrc);
#endif

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * callback function to handle messages externally
  */
extern uint8 zcl_HandleExternal( zclIncoming_t *pInMsg );


#if !defined ( ZCL_STANDALONE )
 /*
  * Initialization for the task
  */
extern void zcl_Init( byte task_id );
#endif

#if !defined ( ZCL_STANDALONE )
/*
 *  Event Process for the task
 */
extern UINT16 zcl_event_loop( byte task_id, UINT16 events );
#endif

#if !defined ( ZCL_STANDALONE )
/*
 *  Register the Application to receive the unprocessed Foundation command/response messages
 */
extern uint8 zcl_registerForMsg( uint8 taskId );

/*
 *  Register a Task to receive the unprocessed Foundation command/response messages
 *  addressed to a specific End Point
 */
extern uint8 zcl_registerForMsgExt( uint8 taskId, uint8 endPointId  );

#endif

/*
 *  Function for Plugins' to register for incoming messages
 */
extern ZStatus_t zcl_registerPlugin( uint16 startLogCluster, uint16 endLogCluster,
                                     zclInHdlr_t pfnIncomingHdlr );

/*
 *  Register Application's Command table
 */
extern ZStatus_t zcl_registerCmdList( uint8 endpoint, CONST uint8 cmdListSize, CONST zclCommandRec_t newCmdList[] );

/*
 *  Register Application's Attribute table
 */
extern ZStatus_t zcl_registerAttrList( uint8 endpoint, uint8 numAttr, CONST zclAttrRec_t attrList[] );

/*
 *  Register Application's Cluster Option table
 */
extern ZStatus_t zcl_registerClusterOptionList( uint8 endpoint, uint8 numOption, zclOptionRec_t optionList[] );

/*
 *  Register Application's attribute data validation callback routine
 */
extern ZStatus_t zcl_registerValidateAttrData( zclValidateAttrData_t pfnValidateAttrData );

/*
 *  Register the application's callback function to read/write attribute data.
 */
extern ZStatus_t zcl_registerReadWriteCB( uint8 endpoint, zclReadWriteCB_t pfnReadWriteCB,
                                          zclAuthorizeCB_t pfnAuthorizeCB );

/*
 *  Process incoming ZCL messages
 */
extern zclProcMsgStatus_t zcl_ProcessMessageMSG( afIncomingMSGPacket_t *pkt );

/*
 *  Function for Sending a Command
 */
extern ZStatus_t zcl_SendCommand( uint8 srcEP, afAddrType_t *dstAddr,
                                  uint16 clusterID, uint8 cmd, uint8 specific, uint8 direction,
                                  uint8 disableDefaultRsp, uint16 manuCode, uint8 seqNum,
                                  uint16 cmdFormatLen, uint8 *cmdFormat );

#ifdef ZCL_READ
/*
 *  Function for Reading an Attribute
 */
extern ZStatus_t zcl_SendRead( uint8 srcEP, afAddrType_t *dstAddr,
                               uint16 realClusterID, zclReadCmd_t *readCmd,
                               uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function for sending a Read response command
 */
extern ZStatus_t zcl_SendReadRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                  uint16 realClusterID, zclReadRspCmd_t *readRspCmd,
                                  uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function for reading a local attribute
 */
extern ZStatus_t zcl_ReadAttrData( uint8 endpoint, uint16 clusterId, uint16 attrId,
                                   uint8 *pAttrData, uint16 *pDataLen );

#endif // ZCL_READ

#ifdef ZCL_WRITE
/*
 *  Function for Writing an Attribute
 */
extern ZStatus_t zcl_SendWriteRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                       uint16 realClusterID, zclWriteCmd_t *writeCmd,
                                       uint8 cmd, uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function for sending a Write response command
 */
extern ZStatus_t zcl_SendWriteRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                   uint16 realClusterID, zclWriteRspCmd_t *writeRspCmd,
                                   uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_WRITE

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*
 *  Function for Configuring the Reporting mechanism for one or more attributes
 */
extern ZStatus_t zcl_SendConfigReportCmd( uint8 srcEP, afAddrType_t *dstAddr,
                          uint16 realClusterID, zclCfgReportCmd_t *cfgReportCmd,
                          uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

#endif

#ifdef ZCL_REPORTING_DEVICE
/*
 *  Function for sending a Configure Reporting Response Command
 */
extern ZStatus_t zcl_SendConfigReportRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                    uint16 realClusterID, zclCfgReportRspCmd_t *cfgReportRspCmd,
                    uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*
 *  Function for Reading the configuration details of the Reporting mechanism
 */
extern ZStatus_t zcl_SendReadReportCfgCmd( uint8 srcEP, afAddrType_t *dstAddr,
                              uint16 realClusterID, zclReadReportCfgCmd_t *readReportCfgCmd,
                              uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
#endif

#ifdef ZCL_REPORTING_DEVICE
/*
 *  Function for sending a Read Reporting Configuration Response command
 */
extern ZStatus_t zcl_SendReadReportCfgRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                        uint16 realClusterID, zclReadReportCfgRspCmd_t *readReportCfgRspCmd,
                        uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function for Reporting the value of one or more attributes
 */
extern ZStatus_t zcl_SendReportCmd( uint8 srcEP, afAddrType_t *dstAddr,
                              uint16 realClusterID, zclReportCmd_t *reportCmd,
                              uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
#endif

/*
 *  Function for sending the Default Response command
 */
extern ZStatus_t zcl_SendDefaultRspCmd( uint8 srcEP, afAddrType_t *dstAddr, uint16 realClusterID,
                                        zclDefaultRspCmd_t *defaultRspCmd, uint8 direction,
                                        uint8 disableDefaultRsp, uint16 manuCode, uint8 seqNum );

#ifdef ZCL_DISCOVER
/*
 *  Function to Discover the ID and Types of commands on a remote device
 */
extern ZStatus_t zcl_SendDiscoverCmdsCmd( uint8 srcEP, afAddrType_t *dstAddr,
                            uint16 clusterID, uint8 cmdType, zclDiscoverCmdsCmd_t *pDiscoverCmd,
                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function for sending the Discover Commands Response command
 */
extern ZStatus_t zcl_SendDiscoverCmdsRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                          uint16 clusterID, zclDiscoverCmdsCmdRsp_t *pDiscoverRspCmd,
                          uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function to Discover the ID and Types of the Attributes on a remote device
 */
extern ZStatus_t zcl_SendDiscoverAttrsCmd( uint8 srcEP, afAddrType_t *dstAddr,
                            uint16 realClusterID, zclDiscoverAttrsCmd_t *pDiscoverCmd,
                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 *  Function for sending the Discover Attributes Response command
 */
extern ZStatus_t zcl_SendDiscoverAttrsRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                      uint16 realClusterID, zclDiscoverAttrsRspCmd_t *pDiscoverRspCmd,
                      uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Function for sending the Discover Attributes Extended command
 */
extern ZStatus_t zcl_SendDiscoverAttrsExt( uint8 srcEP, afAddrType_t *dstAddr,
                                     uint16 realClusterID, zclDiscoverAttrsCmd_t *pDiscoverAttrsExt,
                                     uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Function for sending the Discover Attributes Extended Response command
 */
extern ZStatus_t zcl_SendDiscoverAttrsExtRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint16 realClusterID, zclDiscoverAttrsExtRsp_t *pDiscoverAttrsExtRsp,
                                         uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_DISCOVER

#ifdef ZCL_READ
/*
 * Function to parse the "Profile" Read Commands
 */
extern void *zclParseInReadCmd( zclParseCmd_t *pCmd );
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*
 * Function to parse the "Profile" Write, Write Undivided and Write No Response
 * Commands
 */
extern void *zclParseInWriteCmd( zclParseCmd_t *pCmd );
#endif // ZCL_WRITE

#ifdef ZCL_REPORTING_DEVICE
/*
 * Function to parse the "Profile" Configure Reporting Command
 */
extern void *zclParseInConfigReportCmd( zclParseCmd_t *pCmd );
/*
 * Function to parse the "Profile" Read Reporting Configuration Command
 */
extern void *zclParseInReadReportCfgCmd( zclParseCmd_t *pCmd );
#endif

#if defined ZCL_REPORTING_DEVICE || defined ZCL_REPORT_CONFIGURING_DEVICE
/*
 * Function to check to see if Data Type is Analog
 */
extern uint8 zclAnalogDataType( uint8 dataType );
#endif


#ifdef ZCL_REPORT_DESTINATION_DEVICE
/*
 * Function to parse the "Profile" Report attribute Command
 */
extern void *zclParseInReportCmd( zclParseCmd_t *pCmd );
#endif

#ifdef ZCL_DISCOVER
/*
 * Function to parse the "Profile" Discover Commands Command
 */
extern void *zclParseInDiscCmdsCmd( zclParseCmd_t *pCmd );

/*
 * Function to parse the "Profile" Discover Attributes and Attributes Extended Commands
 */
extern void *zclParseInDiscAttrsCmd( zclParseCmd_t *pCmd );

/*
 * Function to find the command record that matchs the parameters
 */
extern uint8 zclFindCmdRec( uint8 endpoint, uint16 clusterID, uint8 cmdID, zclCommandRec_t *pCmd );
#endif // ZCL_DISCOVER

/*
 * Function to parse header of the ZCL format
 */
extern uint8 *zclParseHdr( zclFrameHdr_t *hdr, uint8 *pData );

/*
 * Function to find the attribute record that matchs the parameters
 */
extern uint8 zclFindAttrRec( uint8 endpoint, uint16 realClusterID, uint16 attrId, zclAttrRec_t *pAttr );

#if defined ( ZCL_STANDALONE )
/*
 *  Set attribute record list for end point
 */
extern uint8 zclSetAttrRecList( uint8 endpoint, uint8 numAttr, CONST zclAttrRec_t attrList[] );
#endif

/*
 * Function to read the attribute's current value
 */
extern ZStatus_t zclReadAttrData( uint8 *pAttrData, zclAttrRec_t *pAttr, uint16 *pDataLen );

/*
 * Function to return the length of the datatype in length.
 */
extern uint8 zclGetDataTypeLength( uint8 dataType );

/*
 * Function to serialize attribute data.
 */
extern uint8 *zclSerializeData( uint8 dataType, void *attrData, uint8 *buf );

/*
 * Function to return the length of the attribute.
 */
extern uint16 zclGetAttrDataLength( uint8 dataType, uint8 *pData);

/*
 * Call to get original unprocessed AF message (not parsed by ZCL).
 *
 *   NOTE:  This function can only be called during a ZCL callback function
 *          and the calling function must NOT change any data in the message.
 *
 *  returns a pointer to original AF message, NULL if not processing
 *          AF message.
 */
extern afIncomingMSGPacket_t *zcl_getRawAFMsg( void );

/*
 * Call to the get the transaction sequence number from the incoming message.
 *
 *   NOTE:  This function can only be called during a ZCL callback function
 *          and the calling function must NOT change any data in the message.
 *
 * returns the transaction sequence number.
 */
extern uint8 zcl_getParsedTransSeqNum( void );



/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_H */
