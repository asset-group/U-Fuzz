/**************************************************************************************************
  Filename:       zcl_ss.h
  Revised:        $Date: 2014-04-29 23:23:15 -0700 (Tue, 29 Apr 2014) $
  Revision:       $Revision: 38309 $

  Description:    This file contains the ZCL Security and Safety definitions.


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

#ifndef ZCL_SS_H
#define ZCL_SS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
extern const uint8 zclSS_UknownIeeeAddress[];

/************************************/
/***  IAS Zone Cluster Attributes ***/
/************************************/
// Zone Information attributes set
#define ATTRID_SS_IAS_ZONE_STATE                                         0x0000 // M, R, ENUM8
#define ATTRID_SS_IAS_ZONE_TYPE                                          0x0001 // M, R, ENUM16
#define ATTRID_SS_IAS_ZONE_STATUS                                        0x0002 // M, R, BITMAP16

/*** Zone State Attribute values ***/
#define SS_IAS_ZONE_STATE_NOT_ENROLLED                                   0x00
#define SS_IAS_ZONE_STATE_ENROLLED                                       0x01

/*** Zone Type Attribute values ***/
// NOTE: if more Zone Type Attribute values are added,
//       zclSS_ZoneTypeSupported() macro SHALL be updated.
#define SS_IAS_ZONE_TYPE_STANDARD_CIE                                    0x0000
#define SS_IAS_ZONE_TYPE_MOTION_SENSOR                                   0x000D
#define SS_IAS_ZONE_TYPE_CONTACT_SWITCH                                  0x0015
#define SS_IAS_ZONE_TYPE_FIRE_SENSOR                                     0x0028
#define SS_IAS_ZONE_TYPE_WATER_SENSOR                                    0x002A
#define SS_IAS_ZONE_TYPE_GAS_SENSOR                                      0x002B
#define SS_IAS_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE                       0x002C
#define SS_IAS_ZONE_TYPE_VIBRATION_MOVEMENT_SENSOR                       0x002D
#define SS_IAS_ZONE_TYPE_REMOTE_CONTROL                                  0x010F
#define SS_IAS_ZONE_TYPE_KEY_FOB                                         0x0115
#define SS_IAS_ZONE_TYPE_KEYPAD                                          0x021D
#define SS_IAS_ZONE_TYPE_STANDARD_WARNING_DEVICE                         0x0225
#define SS_IAS_ZONE_TYPE_GLASS_BREAK_SENSOR                              0x0226
#define SS_IAS_ZONE_TYPE_SECURITY_REPEATER                               0x0229
#define SS_IAS_ZONE_TYPE_INVALID_ZONE_TYPE                               0xFFFF

/*** Zone Status Attribute values ***/
#define SS_IAS_ZONE_STATUS_ALARM1_ALARMED                                0x0001
#define SS_IAS_ZONE_STATUS_ALARM2_ALARMED                                0x0002
#define SS_IAS_ZONE_STATUS_TAMPERED_YES                                  0x0004
#define SS_IAS_ZONE_STATUS_BATTERY_LOW                                   0x0008
#define SS_IAS_ZONE_STATUS_SUPERVISION_REPORTS_YES                       0x0010
#define SS_IAS_ZONE_STATUS_RESTORE_REPORTS_YES                           0x0020
#define SS_IAS_ZONE_STATUS_TROUBLE_YES                                   0x0040
#define SS_IAS_ZONE_STATUS_AC_MAINS_FAULT                                0x0080

// Zone Settings attributes set
#define ATTRID_SS_IAS_CIE_ADDRESS                                        0x0010 // M, R/W, IEEE ADDRESS
#define ATTRID_SS_ZONE_ID                                                0x0011 // M, R, UINT8
#define ATTRID_SS_NUM_ZONE_SENSITIVITY_LEVELS_SUPPORTED                  0x0012 // O, R, UINT8
#define ATTRID_SS_CURRENT_ZONE_SENSITIVITY_LEVEL                         0x0013 // O, R, UINT8

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION                   0x00
#define COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST                        0x01

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_SS_IAS_ZONE_STATUS_ENROLL_RESPONSE                       0x00
#define COMMAND_SS_IAS_ZONE_STATUS_INIT_NORMAL_OP_MODE                   0x01
#define COMMAND_SS_IAS_ZONE_STATUS_INIT_TEST_MODE                        0x02

// permitted values for Enroll Response Code field
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_SUCCESS                  0x00
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NOT_SUPPORTED            0x01
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT         0x02
#define SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES           0x03

// Payload Lengths
#define PAYLOAD_LEN_ZONE_STATUS_CHANGE_NOTIFICATION   6
#define PAYLOAD_LEN_ZONE_ENROLL_REQUEST               4
#define PAYLOAD_LEN_ZONE_STATUS_ENROLL_RSP            2
#define PAYLOAD_LEN_ZONE_STATUS_INIT_TEST_MODE        2

/************************************/
/***  IAS ACE Cluster Attributes  ***/
/************************************/
// cluster has no attributes

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_SS_IAS_ACE_ARM                                           0x00
#define COMMAND_SS_IAS_ACE_BYPASS                                        0x01
#define COMMAND_SS_IAS_ACE_EMERGENCY                                     0x02
#define COMMAND_SS_IAS_ACE_FIRE                                          0x03
#define COMMAND_SS_IAS_ACE_PANIC                                         0x04
#define COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP                               0x05
#define COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION                          0x06
#define COMMAND_SS_IAS_ACE_GET_PANEL_STATUS                              0x07
#define COMMAND_SS_IAS_ACE_GET_BYPASSED_ZONE_LIST                        0x08
#define COMMAND_SS_IAS_ACE_GET_ZONE_STATUS                               0x09

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_SS_IAS_ACE_ARM_RESPONSE                                  0x00
#define COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP_RESPONSE                      0x01
#define COMMAND_SS_IAS_ACE_GET_ZONE_INFORMATION_RESPONSE                 0x02
#define COMMAND_SS_IAS_ACE_ZONE_STATUS_CHANGED                           0x03
#define COMMAND_SS_IAS_ACE_PANEL_STATUS_CHANGED                          0x04
#define COMMAND_SS_IAS_ACE_GET_PANEL_STATUS_RESPONSE                     0x05
#define COMMAND_SS_IAS_ACE_SET_BYPASSED_ZONE_LIST                        0x06
#define COMMAND_SS_IAS_ACE_BYPASS_RESPONSE                               0x07
#define COMMAND_SS_IAS_ACE_GET_ZONE_STATUS_RESPONSE                      0x08

/*** Arm Mode field permitted values ***/
#define SS_IAS_ACE_ARM_DISARM                                            0x00
#define SS_IAS_ACE_ARM_DAY_HOME_ZONES_ONLY                               0x01
#define SS_IAS_ACE_ARM_NIGHT_SLEEP_ZONES_ONLY                            0x02
#define SS_IAS_ACE_ARM_ALL_ZONES                                         0x03

/*** Arm Notification field permitted values ***/
#define SS_IAS_ACE_ARM_NOTIFICATION_ALL_ZONES_DISARMED                   0x00
#define SS_IAS_ACE_ARM_NOTIFICATION_DAY_HOME_ZONES_ONLY                  0x01
#define SS_IAS_ACE_ARM_NOTIFICATION_NIGHT_SLEEP_ZONES_ONLY               0x02
#define SS_IAS_ACE_ARM_NOTIFICATION_ALL_ZONES_ARMED                      0x03
#define SS_IAS_ACE_ARM_NOTIFICATION_INVALID_ARM_DISARM_CODE              0x04
#define SS_IAS_ACE_ARM_NOTIFICATION_NOT_READY_TO_ARM                     0x05
#define SS_IAS_ACE_ARM_NOTIFICATION_ALREADY_DISARMED                     0x06

/*** Panel Status field permitted values ***/
#define SS_IAS_ACE_PANEL_STATUS_ALL_ZONES_DISARMED                       0x00
#define SS_IAS_ACE_PANEL_STATUS_ARMED_STAY                               0x01
#define SS_IAS_ACE_PANEL_STATUS_ARMED_NIGHT                              0x02
#define SS_IAS_ACE_PANEL_STATUS_ARMED_AWAY                               0x03
#define SS_IAS_ACE_PANEL_STATUS_EXIT_DELAY                               0x04
#define SS_IAS_ACE_PANEL_STATUS_ENTRY_DELAY                              0x05
#define SS_IAS_ACE_PANEL_STATUS_NOT_READY_TO_ARM                         0x06
#define SS_IAS_ACE_PANEL_STATUS_IN_ALARM                                 0x07
#define SS_IAS_ACE_PANEL_STATUS_ARMING_STAY                              0x08
#define SS_IAS_ACE_PANEL_STATUS_ARMING_NIGHT                             0x09
#define SS_IAS_ACE_PANEL_STATUS_ARMING_AWAY                              0x0A

/*** Audible Notification field permitted values ***/
#define SS_IAS_ACE_AUDIBLE_NOTIFICATION_MUTE                             0x00
#define SS_IAS_ACE_AUDIBLE_NOTIFICATION_DEFAULT_SOUND                    0x01

/*** Alarm Status field permitted values ***/
#define SS_IAS_ACE_ALARM_STATUS_NO_ALARM                                 0x00
#define SS_IAS_ACE_ALARM_STATUS_BURGLAR                                  0x01
#define SS_IAS_ACE_ALARM_STATUS_FIRE                                     0x02
#define SS_IAS_ACE_ALARM_STATUS_EMERGENCY                                0x03
#define SS_IAS_ACE_ALARM_STATUS_POLICE_PANIC                             0x04
#define SS_IAS_ACE_ALARM_STATUS_FIRE_PANIC                               0x05
#define SS_IAS_ACE_ALARM_STATUS_EMERGENCY_PANIC                          0x06

/*** Bypass Result field permitted values ***/
#define SS_IAS_ACE_BYPASS_RESULT_ZONE_BYPASSED                           0x00
#define SS_IAS_ACE_BYPASS_RESULT_ZONE_NOT_BYPASSED                       0x01
#define SS_IAS_ACE_BYPASS_RESULT_NOT_ALLOWED                             0x02
#define SS_IAS_ACE_BYPASS_RESULT_INVALID_ZONE_ID                         0x03
#define SS_IAS_ACE_BYPASS_RESULT_UNKNOWN_ZONE_ID                         0x04
#define SS_IAS_ACE_BYPASS_RESULT_INVALID_ARM_DISARM_CODE                 0x05

// Field Lengths
#define ZONE_ID_MAP_ARRAY_SIZE  16
#define ARM_DISARM_CODE_LEN     8
#define ZONE_LABEL_LEN          24

// Payload Lengths
#define PAYLOAD_LEN_GET_ZONE_STATUS                 5
#define PAYLOAD_LEN_PANEL_STATUS_CHANGED            4
#define PAYLOAD_LEN_GET_PANEL_STATUS_RESPONSE       4

/************************************/
/***  IAS WD Cluster Attributes   ***/
/************************************/
// Maximum Duration attribute
#define ATTRID_SS_IAS_WD_MAXIMUM_DURATION                                0x0000

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_SS_IAS_WD_START_WARNING                                  0x00
#define COMMAND_SS_IAS_WD_SQUAWK                                         0x01

/***  warningMode field values ***/
#define SS_IAS_START_WARNING_WARNING_MODE_STOP                           0
#define SS_IAS_START_WARNING_WARNING_MODE_BURGLAR                        1
#define SS_IAS_START_WARNING_WARNING_MODE_FIRE                           2
#define SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY                      3
#define SS_IAS_START_WARNING_WARNING_MODE_POLICE_PANIC                   4
#define SS_IAS_START_WARNING_WARNING_MODE_FIRE_PANIC                     5
#define SS_IAS_START_WARNING_WARNING_MODE_EMERGENCY_PANIC                6

/*** start warning: strobe field values ***/
#define SS_IAS_START_WARNING_STROBE_NO_STROBE_WARNING                    0
#define SS_IAS_START_WARNING_STROBE_USE_STPOBE_IN_PARALLEL_TO_WARNING    1

/*** siren level field values ***/
#define SS_IAS_SIREN_LEVEL_LOW_LEVEL_SOUND                               0
#define SS_IAS_SIREN_LEVEL_MEDIUM_LEVEL_SOUND                            1
#define SS_IAS_SIREN_LEVEL_HIGH_LEVEL_SOUND                              2
#define SS_IAS_SIREN_LEVEL_VERY_HIGH_LEVEL_SOUND                         3

/*** strobe level field values ***/
#define SS_IAS_STROBE_LEVEL_LOW_LEVEL_STROBE                             0
#define SS_IAS_STROBE_LEVEL_MEDIUM_LEVEL_STROBE                          1
#define SS_IAS_STROBE_LEVEL_HIGH_LEVEL_STROBE                            2
#define SS_IAS_STROBE_LEVEL_VERY_HIGH_LEVEL_STROBE                       3

/*** squawkMode field values ***/
#define SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_ALARMED_NOTIFICATION_SOUND      0
#define SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_DISARMED_NOTIFICATION_SOUND     1

/*** squawk strobe field values ***/
#define SS_IAS_SQUAWK_STROBE_NO_STROBE_SQUAWK                            0
#define SS_IAS_SQUAWK_STROBE_USE_STROBE_BLINK_IN_PARALLEL_TO_SQUAWK      1

/*** squawk level field values ***/
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_LOW_LEVEL_SOUND                       0
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_MEDIUM_LEVEL_SOUND                    1
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_HIGH_LEVEL_SOUND                      2
#define SS_IAS_SQUAWK_SQUAWK_LEVEL_VERY_HIGH_LEVEL_SOUND                 3

// The maximum number of entries in the Zone table
#define ZCL_SS_MAX_ZONES                                                 256
#define ZCL_SS_MAX_ZONE_ID                                               254

/*********************************************************************
 * TYPEDEFS
 */

/***  typedef for IAS ACE Zone table ***/
typedef struct
{
  uint8   zoneID;
  uint16  zoneType;
  uint8   zoneAddress[Z_EXTADDR_LEN];
} IAS_ACE_ZoneTable_t;

/*** Structures used for callback functions ***/

typedef struct
{
  uint16 zoneStatus;     // current zone status - bit map
  uint8  extendedStatus; // bit map, currently set to All zeroes ( reserved )
  uint8  zoneID;         // allocated zone ID
  uint16 delay;          // delay from change in ZoneStatus attr to transmission of change notification cmd
} zclZoneChangeNotif_t;

typedef struct
{
  afAddrType_t *srcAddr;         // initiator's address
  uint8        zoneID;           // allocated zone ID
  uint16       zoneType;         // current value of Zone Type attribute
  uint16       manufacturerCode; // manufacturer Code from node descriptor for the device
} zclZoneEnrollReq_t;

typedef struct
{
  uint8 responseCode; // value of  response Code
  uint8 zoneID;       // index to the zone table of the CIE
} zclZoneEnrollRsp_t;

typedef struct
{
  uint8 testModeDuration;
  uint8 currZoneSensitivityLevel;
} zclZoneInitTestMode_t;

typedef struct
{
  uint8         armMode;
  UTF8String_t  armDisarmCode;
  uint8         zoneID;
} zclACEArm_t;

typedef struct
{
  uint8 numberOfZones; // number of zones ( one byte)
  uint8 *bypassBuf;    // zone IDs array of 256 entries one byte each
  UTF8String_t armDisarmCode;
} zclACEBypass_t;

typedef struct
{
  uint8  startingZoneID;     // at which the client like to obtain information
  uint8  maxNumZoneIDs;      // number of Zone statuses returned by Server
  uint8  zoneStatusMaskFlag; // boolean field
  uint16 zoneStatusMask;
} zclACEGetZoneStatus_t;

typedef struct
{
  uint8 zoneID;    // index to the zone table of the CIE
  uint16 zoneType; // value of Zone Type atribute
  uint8 *ieeeAddr; // pointer to 64 bit address
  UTF8String_t  zoneLabel;
} zclACEGetZoneInfoRsp_t;

typedef struct
{
  uint8   zoneID;
  uint16  zoneStatus;
  uint8   audibleNotification;
  UTF8String_t  zoneLabel;
} zclACEZoneStatusChanged_t;

typedef struct
{
  uint8 panelStatus;
  uint8 secondsRemaining;
  uint8 audibleNotification;
  uint8 alarmStatus;
} zclACEPanelStatusChanged_t;

typedef struct
{
  uint8 panelStatus;
  uint8 secondsRemaining;
  uint8 audibleNotification;
  uint8 alarmStatus;
} zclACEGetPanelStatusRsp_t;

typedef struct
{
  uint8 numberOfZones;
  uint8 *zoneID;  // List of Zone IDs included in the payload
} zclACESetBypassedZoneList_t;

typedef struct
{
  uint8 numberOfZones;
  uint8 *bypassResult; // List of Bypass results for Zone IDs requested
} zclACEBypassRsp_t;

typedef struct
{
  uint8 zoneID;
  uint16 zoneStatus;
} zclACEZoneStatus_t;

typedef struct
{
  uint8 zoneStatusComplete;
  uint8 numberOfZones;
  zclACEZoneStatus_t *zoneInfo; // this is a list of N - determined by numberOfZones
} zclACEGetZoneStatusRsp_t;

typedef struct
{
  unsigned int warnMode:4;        // Warning Mode
  unsigned int warnStrobe:2;      // Strobe
  unsigned int warnSirenLevel:2;  // Siren Level
} warningbits_t;

typedef union
{
  warningbits_t  warningbits;
  uint8          warningbyte;
} warning_t;

typedef struct
{
  warning_t   warningmessage;
  uint16      warningDuration;
  uint8       strobeDutyCycle;
  uint8       strobeLevel;
} zclWDStartWarning_t;

//typedef struct
//{
//  warning_t warnings; // bitfiels ( one byte )
//  uint16 duration;    // warning duration in seconds
//} zclWDStartWarning_t;

/***  ZCL WD Cluster: COMMAND_WD_SQUAWK Cmd payload ***/
typedef struct
{
  unsigned int squawkMode:4;
  unsigned int strobe:1;
  unsigned int reserved:1;
  unsigned int squawkLevel:2;
} squawkbits_t;

typedef union
{
  squawkbits_t  squawkbits;
  uint8         squawkbyte;
} zclWDSquawk_t;


// This callback is called to process a Change Notification command
typedef ZStatus_t (*zclSS_ChangeNotification_t)( zclZoneChangeNotif_t *pCmd, afAddrType_t *srcAddr );

// This callback is called to process a Enroll Request command
typedef ZStatus_t (*zclSS_EnrollRequest_t)( zclZoneEnrollReq_t *pReq, uint8 endpoint );

// This callback is called to process a Initiate Normal Operation Mode Response command
typedef ZStatus_t (*zclSS_InitNormalOpModeResponse_t)( zclIncoming_t *pInMsg );

// This callback is called to process a Initiate Test Mode Response command
typedef ZStatus_t (*zclSS_InitTestModeResponse_t)( zclIncoming_t *pInMsg );

// This callback is called to process a Enroll Response command
typedef ZStatus_t (*zclSS_EnrollResponse_t)( zclZoneEnrollRsp_t *pRsp );

// This callback is called to process a Initiate Normal Operation Mode command
typedef ZStatus_t (*zclSS_InitNormalOpMode_t)( zclIncoming_t *pInMsg );

// This callback is called to process a Initiate Test Mode Response command
typedef ZStatus_t (*zclSS_InitTestMode_t)( zclZoneInitTestMode_t *pCmd, zclIncoming_t *pInMsg );

// This callback is called to process an Arm command
typedef uint8 (*zclSS_ACE_Arm_t)( zclACEArm_t *pCmd );

// This callback is called to process a Bypass command
typedef ZStatus_t (*zclSS_ACE_Bypass_t)( zclACEBypass_t *pCmd );

// This callback is called to process an Emergency command
typedef ZStatus_t (*zclSS_ACE_Emergency_t)( void );

// This callback is called to process a Fire command
typedef ZStatus_t (*zclSS_ACE_Fire_t)( void );

// This callback is called to process a Panic command
typedef ZStatus_t (*zclSS_ACE_Panic_t)( void );

// This callback is called to process a Get Zone ID Map command
typedef ZStatus_t (*zclSS_ACE_GetZoneIDMap_t)( void );

// This callback is called to process a Get Zone Information command
typedef ZStatus_t (*zclSS_ACE_GetZoneInformation_t)( uint8 zoneID );

// This callback is called to process a Get Panel Status command
typedef ZStatus_t (*zclSS_ACE_GetPanelStatus_t)( void );

// This callback is called to process a Get Bypassed Zone List command
typedef ZStatus_t (*zclSS_ACE_GetBypassedZoneList_t)( void );

// This callback is called to process a Get Zone Status command
typedef ZStatus_t (*zclSS_ACE_GetZoneStatus_t)( zclACEGetZoneStatus_t *pCmd );

// This callback is called to process an Arm Response command
typedef ZStatus_t (*zclSS_ACE_ArmResponse_t)( uint8 armNotification );

// This callback is called to process a Get Zone ID Map Response command
typedef ZStatus_t (*zclSS_ACE_GetZoneIDMapResponse_t)( uint16 *zoneIDMap );

// This callback is called to process a Get Zone Information Response command
typedef ZStatus_t (*zclSS_ACE_GetZoneInformationResponse_t)( zclACEGetZoneInfoRsp_t *pRsp );

// This callback is called to process a Zone Status Changed command
typedef ZStatus_t (*zclSS_ACE_ZoneStatusChanged_t)( zclACEZoneStatusChanged_t *pCmd );

// This callback is called to process a Panel Status Changed command
typedef ZStatus_t (*zclSS_ACE_PanelStatusChanged_t)( zclACEPanelStatusChanged_t *pCmd );

// This callback is called to process a Get Panel Status Response command
typedef ZStatus_t (*zclSS_ACE_GetPanelStatusResponse_t)( zclACEGetPanelStatusRsp_t *pCmd );

// This callback is called to process a Set Bypassed Zone List command
typedef ZStatus_t (*zclSS_ACE_SetBypassedZoneList_t)( zclACESetBypassedZoneList_t *pCmd );

// This callback is called to process an Bypass Response command
typedef ZStatus_t (*zclSS_ACE_BypassResponse_t)( zclACEBypassRsp_t *pCmd );

// This callback is called to process an Get Zone Status Response command
typedef ZStatus_t (*zclSS_ACE_GetZoneStatusResponse_t)( zclACEGetZoneStatusRsp_t *pCmd );

// This callback is called to process a Start Warning command
typedef ZStatus_t (*zclSS_WD_StartWarning_t)( zclWDStartWarning_t *pCmd );

// This callback is called to process a Squawk command
typedef ZStatus_t (*zclSS_WD_Squawk_t)( zclWDSquawk_t *pCmd );


// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclSS_ChangeNotification_t               pfnChangeNotification;             // Change Notification command
  zclSS_EnrollRequest_t                    pfnEnrollRequest;                  // Enroll Request command
  zclSS_EnrollResponse_t                   pfnEnrollResponse;                 // Enroll Reponse command
  zclSS_InitNormalOpMode_t                 pfnInitNormalOpMode;               // Initiate Normal Operation Mode command
  zclSS_InitTestMode_t                     pfnInitTestMode;                   // Initiate Test Mode command
  zclSS_ACE_Arm_t                          pfnACE_Arm;                        // Arm command
  zclSS_ACE_Bypass_t                       pfnACE_Bypass;                     // Bypass command
  zclSS_ACE_Emergency_t                    pfnACE_Emergency;                  // Emergency command
  zclSS_ACE_Fire_t                         pfnACE_Fire;                       // Fire command
  zclSS_ACE_Panic_t                        pfnACE_Panic;                      // Panic command
  zclSS_ACE_GetZoneIDMap_t                 pfnACE_GetZoneIDMap;               // Get Zone ID Map command
  zclSS_ACE_GetZoneInformation_t           pfnACE_GetZoneInformation;         // Get Zone Information Command
  zclSS_ACE_GetPanelStatus_t               pfnACE_GetPanelStatus;             // Get Panel Status Command
  zclSS_ACE_GetBypassedZoneList_t          pfnACE_GetBypassedZoneList;        // Get Bypassed Zone List Command
  zclSS_ACE_GetZoneStatus_t                pfnACE_GetZoneStatus;              // Get Zone Status Command
  zclSS_ACE_ArmResponse_t                  pfnACE_ArmResponse;                // ArmResponse command
  zclSS_ACE_GetZoneIDMapResponse_t         pfnACE_GetZoneIDMapResponse;       // Get Zone ID Map Response command
  zclSS_ACE_GetZoneInformationResponse_t   pfnACE_GetZoneInformationResponse; // Get Zone Information Response command
  zclSS_ACE_ZoneStatusChanged_t            pfnACE_ZoneStatusChanged;          // Zone Status Changed command
  zclSS_ACE_PanelStatusChanged_t           pfnACE_PanelStatusChanged;         // Panel Status Changed command
  zclSS_ACE_GetPanelStatusResponse_t       pfnACE_GetPanelStatusResponse;     // Get Panel Status Response command
  zclSS_ACE_SetBypassedZoneList_t          pfnACE_SetBypassedZoneList;        // Set Bypassed Zone List command
  zclSS_ACE_BypassResponse_t               pfnACE_BypassResponse;             // Bypass Response command
  zclSS_ACE_GetZoneStatusResponse_t        pfnACE_GetZoneStatusResponse;      // Get Zone Status Response command
  zclSS_WD_StartWarning_t                  pfnWD_StartWarning;                // Start Warning command
  zclSS_WD_Squawk_t                        pfnWD_Squawk;                      // Squawk command

} zclSS_AppCallbacks_t;


/*********************************************************************
 * FUNCTION MACROS
 */

#ifdef ZCL_ZONE
/*
 *  Send a InitNormalOperationMode Cmd  ( IAS Zone Cluster )
 *  Use like:
 *      zclSS_IAS_Send_ZoneStatusInitNormalOperationModeCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_IAS_Send_ZoneStatusInitNormalOperationModeCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ZONE,\
                                                                          COMMAND_SS_IAS_ZONE_STATUS_INIT_NORMAL_OP_MODE, TRUE,\
                                                                          ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )
#endif // ZCL_ZONE

#ifdef ZCL_ACE
/*
 *  Send an Emergency Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_EmergencyCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_Send_IAS_ACE_EmergencyCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_SS_IAS_ACE_EMERGENCY, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )

/*
 *  Send a Fire Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_FireCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_Send_IAS_ACE_FireCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_SS_IAS_ACE_FIRE, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )

/*
 *  Send a Panic Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_PanicCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_Send_IAS_ACE_PanicCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_SS_IAS_ACE_PANIC, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )

/*
 *  Send a GetZoneIDMap Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_GetZoneIDMapCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_Send_IAS_ACE_GetZoneIDMapCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_SS_IAS_ACE_GET_ZONE_ID_MAP, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )
/*
 *  Send a GetPanelStatus Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_GetPanelStatusCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_Send_IAS_ACE_GetPanelStatusCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_SS_IAS_ACE_GET_PANEL_STATUS, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )
/*
 *  Send a GetBypassedZoneList Cmd  ( IAS ACE Cluster )
 *  Use like:
 *      ZStatus_t zclSS_Send_IAS_ACE_GetBypassedZoneListCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclSS_Send_IAS_ACE_GetBypassedZoneListCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_SS_IAS_ACE,\
                                                      COMMAND_SS_IAS_ACE_GET_BYPASSED_ZONE_LIST, TRUE,\
                                                      ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d),  0, NULL )
#endif // ZCL_ACE

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */


 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclSS_RegisterCmdCallbacks( uint8 endpoint, zclSS_AppCallbacks_t *callbacks );

#ifdef ZCL_ZONE
 /*
  * Call to send out a Change Notification Command
  */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusChangeNotificationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint16 zoneStatus, uint8 extendedStatus,
                                                             uint8 zoneID, uint16 delay,
                                                             uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Enroll Request Command
  */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollRequestCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint16 zoneType, uint16 manufacturerCode,
                                                      uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Enroll Response Command
  */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusEnrollResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint8 responseCode, uint8 zoneID,
                                                             uint8 disableDefaultRsp, uint8 seqNum );
 /*
  * Call to send out a Initiate Test Mode Command
  */
extern ZStatus_t zclSS_IAS_Send_ZoneStatusInitTestModeCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    zclZoneInitTestMode_t *pCmd,
                                                    uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_ZONE

#ifdef ZCL_ACE
/*
 * Call to parse Arm Command  ( IAS ACE Cluster )
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_Arm( zclACEArm_t *pCmd, uint8 *pInBuf );

/*
 * Call to parse Bypass Command  ( IAS ACE Cluster )
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_Bypass( zclACEBypass_t *pCmd, uint8 *pInBuf );

/*
 * Call to parse Get Zone Information Response Command  ( IAS ACE Cluster )
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_GetZoneInformationResponse( zclACEGetZoneInfoRsp_t *pCmd,
                                                                  uint8 *pInBuf );
/*
 * Call to parse Zone Status Changed Command  ( IAS ACE Cluster )
 */
extern ZStatus_t zclSS_ParseInCmd_ACE_ZoneStatusChanged( zclACEZoneStatusChanged_t *pCmd,
                                                         uint8 *pInBuf );
/*
  * Call to send out a Arm  Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_ArmCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACEArm_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Bypass Command ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_BypassCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                               zclACEBypass_t *pCmd,
                                               uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Get Zone Information Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneInformationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                          uint8 zoneID, uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Get Zone Status Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneStatusCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      zclACEGetZoneStatus_t *pCmd,
                                                      uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a ArmResponse  Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_ArmResponse( uint8 srcEP, afAddrType_t *dstAddr,
                       uint8 armNotification, uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Get Zone ID Map Response Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneIDMapResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                       uint16 *zoneIDMap, uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Get Zone Information Response Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneInformationResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    zclACEGetZoneInfoRsp_t *pCmd,
                                                    uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Zone Status Changed  Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_ZoneStatusChangedCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACEZoneStatusChanged_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Panel Status Changed  Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_PanelStatusChangedCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACEPanelStatusChanged_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Get Panel Status Response Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_GetPanelStatusResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACEGetPanelStatusRsp_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Set Bypassed Zone List Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_SetBypassedZoneListCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACESetBypassedZoneList_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Bypass Response Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_BypassResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACEBypassRsp_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Get Zone Status Response Command  ( IAS ACE Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_ACE_GetZoneStatusResponseCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                            zclACEGetZoneStatusRsp_t *pCmd,
                                            uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_ACE

#ifdef ZCL_WD
 /*
  * Call to send out a Start Warning Command  ( IAS WD Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_WD_StartWarningCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    zclWDStartWarning_t *warning,
                                                    uint8 disableDefaultRsp, uint8 seqNum );

 /*
  * Call to send out a Squawk Command  ( IAS WD Cluster )
  */
extern ZStatus_t zclSS_Send_IAS_WD_SquawkCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                              zclWDSquawk_t *squawk,
                                              uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_WD

#if defined(ZCL_ZONE) || defined(ZCL_ACE)
 /*
  * Call to update Zone Address for zoneID
  *   ieeeAddr - ptr to IEEE address
  */
extern void zclSS_UpdateZoneAddress( uint8 endpoint, uint8 zoneID, uint8 *ieeeAddr );


 /*
  * Call to remove a zone with endpoint and zoneID
  *   zoneID - zone to be removed
  */
extern uint8 zclSS_RemoveZone( uint8 endpoint, uint8 zoneID );


 /*
  * Call to find a zone with endpoint and zoneID
  *   zoneID - zone to be removed
  */extern IAS_ACE_ZoneTable_t *zclSS_FindZone( uint8 endpoint, uint8 zoneID );
#endif // ZCL_ZONE || ZCL_ACE

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SS_H */
