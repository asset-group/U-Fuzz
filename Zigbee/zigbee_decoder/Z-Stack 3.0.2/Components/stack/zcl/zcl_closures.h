/**************************************************************************************************
  Filename:       zcl_closures.h
  Revised:        $Date: 2014-02-04 16:43:21 -0800 (Tue, 04 Feb 2014) $
  Revision:       $Revision: 37119 $

  Description:    This file contains the ZCL Closures definitions.


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

#ifndef ZCL_CLOSURES_H
#define ZCL_CLOSURES_H

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

/**********************************************/
/*** Shade Configuration Cluster Attributes ***/
/**********************************************/
  // Shade information attributes set
#define ATTRID_CLOSURES_PHYSICAL_CLOSED_LIMIT                          0x0000 // O, R, UINT16
#define ATTRID_CLOSURES_MOTOR_STEP_SIZE                                0x0001 // O, R, UINT8
#define ATTRID_CLOSURES_STATUS                                         0x0002 // M, R/W, BITMAP8

/*** Status attribute bit values ***/
#define CLOSURES_STATUS_SHADE_IS_OPERATIONAL                           0x01
#define CLOSURES_STATUS_SHADE_IS_ADJUSTING                             0x02
#define CLOSURES_STATUS_SHADE_DIRECTION                                0x04
#define CLOSURES_STATUS_SHADE_MOTOR_FORWARD_DIRECTION                  0x08

  // Shade settings attributes set
#define ATTRID_CLOSURES_CLOSED_LIMIT                                   0x0010
#define ATTRID_CLOSURES_MODE                                           0x0012

/*** Mode attribute values ***/
#define CLOSURES_MODE_NORMAL_MODE                                      0x00
#define CLOSURES_MODE_CONFIGURE_MODE                                   0x01

// cluster has no specific commands

/**********************************************/
/*** Logical Cluster ID - for mapping only ***/
/***  These are not to be used over-the-air ***/
/**********************************************/
#define ZCL_CLOSURES_LOGICAL_CLUSTER_ID_SHADE_CONFIG                   0x0010


/************************************/
/*** Door Lock Cluster Attributes ***/
/************************************/
#ifdef ZCL_DOORLOCK

#define ATTRID_CLOSURES_LOCK_STATE                                          0x0000
#define ATTRID_CLOSURES_LOCK_TYPE                                           0x0001
#define ATTRID_CLOSURES_ACTUATOR_ENABLED                                    0x0002
#define ATTRID_CLOSURES_DOOR_STATE                                          0x0003
#define ATTRID_CLOSURES_NUM_OF_DOOR_OPEN_EVENTS                             0x0004
#define ATTRID_CLOSURES_NUM_OF_DOOR_CLOSED_EVENTS                           0x0005
#define ATTRID_CLOSURES_OPEN_PERIOD                                         0x0006

// User, PIN, Schedule, & Log Information Attributes
#define ATTRID_DOORLOCK_NUM_OF_LOCK_RECORDS_SUPPORTED                       0x0010  // O, R, UINT16
#define ATTRID_DOORLOCK_NUM_OF_TOTAL_USERS_SUPPORTED                        0x0011  // O, R, UINT16
#define ATTRID_DOORLOCK_NUM_OF_PIN_USERS_SUPPORTED                          0x0012  // O, R, UINT16
#define ATTRID_DOORLOCK_NUM_OF_RFID_USERS_SUPPORTED                         0x0013  // O, R, UINT16
#define ATTRID_DOORLOCK_NUM_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER        0x0014  // O, R, UINT8
#define ATTRID_DOORLOCK_NUM_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER        0x0015  // O, R, UINT8
#define ATTRID_DOORLOCK_NUM_OF_HOLIDAY_SCHEDULEDS_SUPPORTED                 0x0016  // O, R, UINT8
#define ATTRID_DOORLOCK_MAX_PIN_LEN                                         0x0017  // O, R, UINT8
#define ATTRID_DOORLOCK_MIN_PIN_LEN                                         0x0018  // O, R, UINT8
#define ATTRID_DOORLOCK_MAX_RFID_LEN                                        0x0019  // O, R, UINT8
#define ATTRID_DOORLOCK_MIN_RFID_LEN                                        0x001A  // O, R, UINT8

// Operational Settings Attributes
#define ATTRID_DOORLOCK_ENABLE_LOGGING                                      0x0020  // O, R/W, BOOLEAN
#define ATTRID_DOORLOCK_LANGUAGE                                            0x0021  // O, R/W, CHAR STRING
#define ATTRID_DOORLOCK_LED_SETTINGS                                        0x0022  // O, R/W, UINT8
#define ATTRID_DOORLOCK_AUTO_RELOCK_TIME                                    0x0023  // O, R/W, UINT32
#define ATTRID_DOORLOCK_SOUND_VOLUME                                        0x0024  // O, R/W, UINT8
#define ATTRID_DOORLOCK_OPERATING_MODE                                      0x0025  // O, R/W, ENUM8
#define ATTRID_DOORLOCK_SUPPORTED_OPERATING_MODES                           0x0026  // O, R, BITMAP16
#define ATTRID_DOORLOCK_DEFAULT_CONFIGURATION_REGISTER                      0x0027  // O, R, BITMAP16
#define ATTRID_DOORLOCK_ENABLE_LOCAL_PROGRAMMING                            0x0028  // O, R/W, BOOLEAN
#define ATTRID_DOORLOCK_ENABLE_ONE_TOUCH_LOCKING                            0x0029  // O, R/W, BOOLEAN
#define ATTRID_DOORLOCK_ENABLE_INSIDE_STATUS_LED                            0x002A  // O, R/W, BOOLEAN
#define ATTRID_DOORLOCK_ENABLE_PRIVACY_MODE_BUTTON                          0x002B  // O, R/W, BOOLEAN

// Security Settings Attributes
#define ATTRID_DOORLOCK_WRONG_CODE_ENTRY_LIMIT                              0x0030  // O, R/W, UINT8
#define ATTRID_DOORLOCK_USER_CODE_TEMPORARY_DISABLE_TIME                    0x0031  // O, R/W, UINT8
#define ATTRID_DOORLOCK_SEND_PIN_OTA                                        0x0032  // O, R/W, BOOLEAN
#define ATTRID_DOORLOCK_REQUIRE_PIN_FOR_RF_OPERATION                        0x0033  // O, R/W, BOOLEAN
#define ATTRID_DOORLOCK_ZIGBEE_SECURITY_LEVEL                               0x0034  // O, R, UINT8

// Alarm and Event Masks Attributes
#define ATTRID_DOORLOCK_ALARM_MASK                                          0x0040  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_KEYPAD_OPERATION_EVENT_MASK                         0x0041  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_RF_OPERATION_EVENT_MASK                             0x0042  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_MANUAL_OPERATION_EVENT_MASK                         0x0043  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_RFID_OPERATION_EVENT_MASK                           0x0044  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_KEYPAD_PROGRAMMING_EVENT_MASK                       0x0045  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_RF_PROGRAMMING_EVENT_MASK                           0x0046  // O, R/W, BITMAP16
#define ATTRID_DOORLOCK_RFID_PROGRAMMING_EVENT_MASK                         0x0047  // O, R/W, BITMAP16

// User, PIN, Schedule, & Log Information Attribute Defaults
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_LOCK_RECORDS_SUPPORTED                       0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_TOTAL_USERS_SUPPORTED                        0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_PIN_USERS_SUPPORTED                          0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_RFID_USERS_SUPPORTED                         0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER        0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER        0
#define ATTR_DEFAULT_DOORLOCK_NUM_OF_HOLIDAY_SCHEDULEDS_SUPPORTED                 0
#define ATTR_DEFAULT_DOORLOCK_MAX_PIN_LENGTH                                      0x08
#define ATTR_DEFAULT_DOORLOCK_MIN_PIN_LENGTH                                      0x04
#define ATTR_DEFAULT_DOORLOCK_MAX_RFID_LENGTH                                     0x14
#define ATTR_DEFAULT_DOORLOCK_MIN_RFID_LENGTH                                     0x08

// Operational Settings Attribute Defaults
#define ATTR_DEFAULT_DOORLOCK_ENABLE_LOGGING                                      0
#define ATTR_DEFAULT_DOORLOCK_LANGUAGE                                            {0,0,0}
#define ATTR_DEFAULT_DOORLOCK_LED_SETTINGS                                        0
#define ATTR_DEFAULT_DOORLOCK_AUTO_RELOCK_TIME                                    0
#define ATTR_DEFAULT_DOORLOCK_SOUND_VOLUME                                        0
#define ATTR_DEFAULT_DOORLOCK_OPERATING_MODE                                      0
#define ATTR_DEFAULT_DOORLOCK_SUPPORTED_OPERATING_MODES                           0x0001
#define ATTR_DEFAULT_DOORLOCK_DEFAULT_CONFIGURATION_REGISTER                      0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_LOCAL_PROGRAMMING                            0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_ONE_TOUCH_LOCKING                            0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_INSIDE_STATUS_LED                            0
#define ATTR_DEFAULT_DOORLOCK_ENABLE_PRIVACY_MODE_BUTTON                          0

// Security Settings Attribute Defaults
#define ATTR_DEFAULT_DOORLOCK_WRONG_CODE_ENTRY_LIMIT                              0
#define ATTR_DEFAULT_DOORLOCK_USER_CODE_TEMPORARY_DISABLE_TIME                    0
#define ATTR_DEFAULT_DOORLOCK_SEND_PIN_OTA                                        0
#define ATTR_DEFAULT_DOORLOCK_REQUIRE_PIN_FOR_RF_OPERATION                        0
#define ATTR_DEFAULT_DOORLOCK_ZIGBEE_SECURITY_LEVEL                               0

// Alarm and Event Masks Attribute Defaults
#define ATTR_DEFAULT_DOORLOCK_ALARM_MASK                                          0x0000
#define ATTR_DEFAULT_DOORLOCK_KEYPAD_OPERATION_EVENT_MASK                         0x0000
#define ATTR_DEFAULT_DOORLOCK_RF_OPERATION_EVENT_MASK                             0x0000
#define ATTR_DEFAULT_DOORLOCK_MANUAL_OPERATION_EVENT_MASK                         0x0000
#define ATTR_DEFAULT_DOORLOCK_RFID_OPERATION_EVENT_MASK                           0x0000
#define ATTR_DEFAULT_DOORLOCK_KEYPAD_PROGRAMMING_EVENT_MASK                       0x0000
#define ATTR_DEFAULT_DOORLOCK_RF_PROGRAMMING_EVENT_MASK                           0x0000
#define ATTR_DEFAULT_DOORLOCK_RFID_PROGRAMMING_EVENT_MASK                         0x0000

 /******************************************************************************************
  * Operating Mode enumerations
  * Interface: (E = Enable; D = Disable)
  * Devices:  (K = Keypad; RF; RFID)
  */
#define DOORLOCK_OP_MODE_NORMAL                   0x00  // K = E;   RF = E;   RFID = E
#define DOORLOCK_OP_MODE_VACATION                 0x01  // K = D;   RF = E;   RFID = E
#define DOORLOCK_OP_MODE_PRIVACY                  0x02  // K = D;   RF = D;   RFID = D
#define DOORLOCK_OP_MODE_NO_RF_LOCK_UNLOCK        0x03  // K = E;   RF = D;   RFID = E
#define DOORLOCK_OP_MODE_PASSAGE                  0x04  // K = N/A; RF = N/A; RFID = N/A

/*** Lock State Attribute types ***/
#define CLOSURES_LOCK_STATE_NOT_FULLY_LOCKED               0x00
#define CLOSURES_LOCK_STATE_LOCKED                         0x01
#define CLOSURES_LOCK_STATE_UNLOCKED                       0x02

/*** Lock Type Attribute types ***/
#define CLOSURES_LOCK_TYPE_DEADBOLT                        0x00
#define CLOSURES_LOCK_TYPE_MAGNETIC                        0x01
#define CLOSURES_LOCK_TYPE_OTHER                           0x02
#define CLOSURES_LOCK_TYPE_MORTISE                         0x03
#define CLOSURES_LOCK_TYPE_RIM                             0x04
#define CLOSURES_LOCK_TYPE_LATCH_BOLT                      0x05
#define CLOSURES_LOCK_TYPE_CYLINDRICAL_LOCK                0x06
#define CLOSURES_LOCK_TYPE_TUBULAR_LOCK                    0x07
#define CLOSURES_LOCK_TYPE_INTERCONNECTED_LOCK             0x08
#define CLOSURES_LOCK_TYPE_DEAD_LATCH                      0x09
#define CLOSURES_LOCK_TYPE_DOOR_FURNITURE                  0x0A

/*** Door State Attribute types ***/
#define CLOSURES_DOOR_STATE_OPEN                           0x00
#define CLOSURES_DOOR_STATE_CLOSED                         0x01
#define CLOSURES_DOOR_STATE_ERROR_JAMMED                   0x02
#define CLOSURES_DOOR_STATE_ERROR_FORCED_OPEN              0x03
#define CLOSURES_DOOR_STATE_ERROR_UNSPECIFIED              0x04

/**********************************/
/*** Door Lock Cluster Commands ***/
/**********************************/
  // Server Commands Received
#define COMMAND_CLOSURES_LOCK_DOOR                         0x00 // M  zclDoorLock_t
#define COMMAND_CLOSURES_UNLOCK_DOOR                       0x01 // M  zclDoorLock_t
#define COMMAND_CLOSURES_TOGGLE_DOOR                       0x02 // O  zclDoorLock_t
#define COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT               0x03 // O  zclDoorLockUnlockTimeout_t
#define COMMAND_CLOSURES_GET_LOG_RECORD                    0x04 // O  zclDoorLockGetLogRecord_t
#define COMMAND_CLOSURES_SET_PIN_CODE                      0x05 // O  zclDoorLockSetPINCode_t
#define COMMAND_CLOSURES_GET_PIN_CODE                      0x06 // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_CLEAR_PIN_CODE                    0x07 // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES               0x08 // O  no payload
#define COMMAND_CLOSURES_SET_USER_STATUS                   0x09 // O  zclDoorLockSetUserStatus_t
#define COMMAND_CLOSURES_GET_USER_STATUS                   0x0A // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE             0x0B // O  zclDoorLockSetWeekDaySchedule_t
#define COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE             0x0C // O  zclDoorLockSchedule_t
#define COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE           0x0D // O  zclDoorLockSchedule_t
#define COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE             0x0E // O  zclDoorLockSetYearDaySchedule_t
#define COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE             0x0F // O  zclDoorLockSchedule_t
#define COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE           0x10 // O  zclDoorLockSchedule_t
#define COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE              0x11 // O  zclDoorLockSetHolidaySchedule_t
#define COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE              0x12 // O  zclDoorLockHolidayScheduleID_t
#define COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE            0x13 // O  zclDoorLockHolidayScheduleID_t
#define COMMAND_CLOSURES_SET_USER_TYPE                     0x14 // O  zclDoorLockSetUserType_t
#define COMMAND_CLOSURES_GET_USER_TYPE                     0x15 // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_SET_RFID_CODE                     0x16 // O  zclDoorLockSetRFIDCode_t
#define COMMAND_CLOSURES_GET_RFID_CODE                     0x17 // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_CLEAR_RFID_CODE                   0x18 // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES              0x19 // O  no payload

  // Server Commands Generated
#define COMMAND_CLOSURES_LOCK_DOOR_RSP                     0x00 // M  status field
#define COMMAND_CLOSURES_UNLOCK_DOOR_RSP                   0x01 // M  status field
#define COMMAND_CLOSURES_TOGGLE_DOOR_RSP                   0x02 // O  status field
#define COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT_RSP           0x03 // O  status field
#define COMMAND_CLOSURES_GET_LOG_RECORD_RSP                0x04 // O  zclDoorLockGetLogRecordRsp_t
#define COMMAND_CLOSURES_SET_PIN_CODE_RSP                  0x05 // O  status field
#define COMMAND_CLOSURES_GET_PIN_CODE_RSP                  0x06 // O  zclDoorLockGetPINCodeRsp_t
#define COMMAND_CLOSURES_CLEAR_PIN_CODE_RSP                0x07 // O  status field
#define COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES_RSP           0x08 // O  status field
#define COMMAND_CLOSURES_SET_USER_STATUS_RSP               0x09 // O  status field
#define COMMAND_CLOSURES_GET_USER_STATUS_RSP               0x0A // O  zclDoorLockGetUserStateRsp_t
#define COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE_RSP         0x0B // O  status field
#define COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE_RSP         0x0C // O  zclDoorLockGetWeekDayScheduleRsp_t
#define COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE_RSP       0x0D // O  status field
#define COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE_RSP         0x0E // O  status field
#define COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE_RSP         0x0F // O  zclDoorLockGetYearDayScheduleRsp_t
#define COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE_RSP       0x10 // O  status field
#define COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE_RSP          0x11 // O  status field
#define COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE_RSP          0x12 // O  zclDoorLockGetHolidayScheduleRsp_t
#define COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE_RSP        0x13 // O  status field
#define COMMAND_CLOSURES_SET_USER_TYPE_RSP                 0x14 // O  status field
#define COMMAND_CLOSURES_GET_USER_TYPE_RSP                 0x15 // O  zclDoorLockGetUserTypeRsp_t
#define COMMAND_CLOSURES_SET_RFID_CODE_RSP                 0x16 // O  status field
#define COMMAND_CLOSURES_GET_RFID_CODE_RSP                 0x17 // O  zclDoorLockGetRFIDCodeRsp_t
#define COMMAND_CLOSURES_CLEAR_RFID_CODE_RSP               0x18 // O  status field
#define COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES_RSP          0x19 // O  status field
#define COMMAND_CLOSURES_OPERATION_EVENT_NOTIFICATION      0x20 // O  zclDoorLockOperationalEventNotification_t
#define COMMAND_CLOSURES_PROGRAMMING_EVENT_NOTIFICATION    0x21 // O  zclDoorLockProgrammingEventNotification_t


/*** User Status Value enums ***/
#define USER_STATUS_AVAILABLE                                   0x00
#define USER_STATUS_OCCUPIED_ENABLED                            0x01
#define USER_STATUS_RESERVED                                    0x02
#define USER_STATUS_OCCUPIED_DISABLED                           0x03

/*** User Type Value enums ***/
#define USER_TYPE_UNRESTRICTED_USER                             0x00 // default
#define USER_TYPE_YEAR_DAY_SCHEDULE_USER                        0x01
#define USER_TYPE_WEEK_DAY_SCHEDULE_USER                        0x02
#define USER_TYPE_MASTER_USER                                   0x03

/*** Operation (Programming) Event Source Value enums ***/
#define OPERATION_EVENT_SOURCE_KEYPAD                           0x00
#define OPERATION_EVENT_SOURCE_RF                               0x01
#define OPERATION_EVENT_SOURCE_MANUAL                           0x02   // "Reserved" for Programming Event
#define OPERATION_EVENT_SOURCE_RFID                             0x03
#define OPERATION_EVENT_SOURCE_INDETERMINATE                    0xFF

/*** Operation Event Code Value enums ***/
#define OPERATION_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC            0x00 // Applicable: Keypad, RF, Manual, RFID
#define OPERATION_EVENT_CODE_LOCK                               0x01 // Applicable: Keypad, RF, Manual, RFID
#define OPERATION_EVENT_CODE_UNLOCK                             0x02 // Applicable: Keypad, RF, Manual, RFID
#define OPERATION_EVENT_CODE_LOCK_FAILURE_INVALID_PIN_OR_ID     0x03 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_LOCK_FAILURE_INVALID_SCHEDULE      0x04 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_UNLOCK_FAILURE_INVALID_PIN_OR_ID   0x05 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_UNLOCK_FAILURE_INVALID_SCHEDULE    0x06 // Applicable: Keypad, RF, RFID
#define OPERATION_EVENT_CODE_ONE_TOUCH_LOCK                     0x07 // Applicable: Manual
#define OPERATION_EVENT_CODE_KEY_LOCK                           0x08 // Applicable: Manual
#define OPERATION_EVENT_CODE_KEY_UNLOCK                         0x09 // Applicable: Manual
#define OPERATION_EVENT_CODE_AUTO_LOCK                          0x0A // Applicable: Manual
#define OPERATION_EVENT_CODE_SCHEDULE_LOCK                      0x0B // Applicable: Manual
#define OPERATION_EVENT_CODE_SCHEDULE_UNLOCK                    0x0C // Applicable: Manual
#define OPERATION_EVENT_CODE_MANUAL_LOCK                        0x0D // Applicable: Manual
#define OPERATION_EVENT_CODE_MANUAL_UNLOCK                      0x0E // Applicable: Manual

/*** Programming Event Code enums ***/
#define PROGRAMMING_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC          0x00 // Applicable: Keypad, RF, RFID
#define PROGRAMMING_EVENT_CODE_MASTER_CODE_CHANGED              0x01 // Applicable: Keypad
#define PROGRAMMING_EVENT_CODE_PIN_CODE_ADDED                   0x02 // Applicable: Keypad, RF
#define PROGRAMMING_EVENT_CODE_PIN_CODE_DELETED                 0x03 // Applicable: Keypad, RF
#define PROGRAMMING_EVENT_CODE_PIN_CODE_CHANGED                 0x04 // Applicable: Keypad, RF
#define PROGRAMMING_EVENT_CODE_RFID_CODE_ADDED                  0x05 // Applicable: RFID
#define PROGRAMMING_EVENT_CODE_RFID_CODE_DELETED                0x06 // Applicable: RFID

#define DOORLOCK_RES_PAYLOAD_LEN                                0x01

#endif //ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
/**********************************************/
/*** Window Covering Cluster Attribute Sets ***/
/**********************************************/
#define ATTRSET_WINDOW_COVERING_INFO                        0x0000
#define ATTRSET_WINDOW_COVERING_SETTINGS                    0x0010

/******************************************/
/*** Window Covering Cluster Attributes ***/
/******************************************/
//Window Covering Information
#define ATTRID_CLOSURES_WINDOW_COVERING_TYPE                ( ATTRSET_WINDOW_COVERING_INFO + 0x0000 )
#define ATTRID_CLOSURES_PHYSICAL_CLOSE_LIMIT_LIFT_CM        ( ATTRSET_WINDOW_COVERING_INFO + 0x0001 )
#define ATTRID_CLOSURES_PHYSICAL_CLOSE_LIMIT_TILT_DDEGREE   ( ATTRSET_WINDOW_COVERING_INFO + 0x0002 )
#define ATTRID_CLOSURES_CURRENT_POSITION_LIFT_CM            ( ATTRSET_WINDOW_COVERING_INFO + 0x0003 )
#define ATTRID_CLOSURES_CURRENT_POSITION_TILT_DDEGREE       ( ATTRSET_WINDOW_COVERING_INFO + 0x0004 )
#define ATTRID_CLOSURES_NUM_OF_ACTUATION_LIFT               ( ATTRSET_WINDOW_COVERING_INFO + 0x0005 )
#define ATTRID_CLOSURES_NUM_OF_ACTUATION_TILT               ( ATTRSET_WINDOW_COVERING_INFO + 0x0006 )
#define ATTRID_CLOSURES_CONFIG_STATUS                       ( ATTRSET_WINDOW_COVERING_INFO + 0x0007 )
#define ATTRID_CLOSURES_CURRENT_POSITION_LIFT_PERCENTAGE    ( ATTRSET_WINDOW_COVERING_INFO + 0x0008 )
#define ATTRID_CLOSURES_CURRENT_POSITION_TILT_PERCENTAGE    ( ATTRSET_WINDOW_COVERING_INFO + 0x0009 )

//Window Covering Setting
#define ATTRID_CLOSURES_INSTALLED_OPEN_LIMIT_LIFT_CM        ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0000 )
#define ATTRID_CLOSURES_INSTALLED_CLOSED_LIMIT_LIFT_CM      ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0001 )
#define ATTRID_CLOSURES_INSTALLED_OPEN_LIMIT_TILT_DDEGREE   ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0002 )
#define ATTRID_CLOSURES_INSTALLED_CLOSED_LIMIT_TILT_DDEGREE ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0003 )
#define ATTRID_CLOSURES_VELOCITY_LIFT                       ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0004 )
#define ATTRID_CLOSURES_ACCELERATION_TIME_LIFT              ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0005 )
#define ATTRID_CLOSURES_DECELERATION_TIME_LIFT              ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0006 )
#define ATTRID_CLOSURES_WINDOW_COVERING_MODE                ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0007 )
#define ATTRID_CLOSURES_INTERMEDIATE_SETPOINTS_LIFT         ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0008 )
#define ATTRID_CLOSURES_INTERMEDIATE_SETPOINTS_TILT         ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0009 )

/*** Window Covering Type Attribute types ***/
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE                       0x00
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_2_MOTOR               0x01
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_EXTERIOR              0x02
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_EXTERIOR_2_MOTOR      0x03
#define CLOSURES_WINDOW_COVERING_TYPE_DRAPERY                           0x04
#define CLOSURES_WINDOW_COVERING_TYPE_AWNING                            0x05
#define CLOSURES_WINDOW_COVERING_TYPE_SHUTTER                           0x06
#define CLOSURES_WINDOW_COVERING_TYPE_TILT_BLIND_TILT_ONLY              0x07
#define CLOSURES_WINDOW_COVERING_TYPE_TILT_BLIND_LIFT_AND_TILT          0x08
#define CLOSURES_WINDOW_COVERING_TYPE_PROJECTOR_SCREEN                  0x09


/****************************************/
/*** Window Covering Cluster Commands ***/
/****************************************/
#define COMMAND_CLOSURES_UP_OPEN                            ( 0x00 )
#define COMMAND_CLOSURES_DOWN_CLOSE                         ( 0x01 )
#define COMMAND_CLOSURES_STOP                               ( 0x02 )
#define COMMAND_CLOSURES_GO_TO_LIFT_VALUE                   ( 0x04 )
#define COMMAND_CLOSURES_GO_TO_LIFT_PERCENTAGE              ( 0x05 )
#define COMMAND_CLOSURES_GO_TO_TILT_VALUE                   ( 0x07 )
#define COMMAND_CLOSURES_GO_TO_TILT_PERCENTAGE              ( 0x08 )

#define ZCL_WC_GOTOVALUEREQ_PAYLOADLEN                      ( 2 )
#define ZCL_WC_GOTOPERCENTAGEREQ_PAYLOADLEN                 ( 1 )

#endif // ZCL_WINDOWCOVERING

/*********************************************************************
 * TYPEDEFS
 */


/*** Window Covering Cluster - Bits in Config/Status Attribute ***/
typedef struct
{
  uint8 Operational : 1;              // Window Covering is operational or not
  uint8 Online : 1;                   // Window Covering is enabled for transmitting over the Zigbee network or not
  uint8 CommandsReversed : 1;         // Identifies the direction of rotation for the Window Covering
  uint8 LiftControl : 1;              // Identifies, lift control supports open loop or closed loop
  uint8 TiltControl : 1;              // Identifies, tilt control supports open loop or closed loop
  uint8 LiftEncoderControlled : 1;    // Identifies, lift control uses Timer or Encoder
  uint8 TiltEncoderControlled : 1;    // Identifies, tilt control uses Timer or Encoder
  uint8 Reserved : 1;
}zclClosuresWcInfoConfigStatus_t;

/*** Window Covering Cluster - Bits in Mode Attribute ***/
typedef struct
{
  uint8 MotorReverseDirection : 1;    // Defines the direction of the motor rotation
  uint8 RunInCalibrationMode : 1;     // Defines Window Covering is in calibration mode or in normal mode
  uint8 RunInMaintenanceMode : 1;     // Defines motor is running in maintenance mode or in normal mode
  uint8 LEDFeedback : 1;              // Enables or Disables feedback LED
  uint8 Reserved : 4;
}zclClosuresWcSetMode_t;

/*** Window Covering Cluster - Setpoint type ***/
typedef enum
{
  lift = 0,
  tilt = 1,
}setpointType_t;

/*** Window Covering Cluster - Setpoint version ***/
typedef enum
{
  programSetpointVersion1 = 1,
  programSetpointVersion2,
}setpointVersion_t;

/*** Window Covering - Program Setpoint Command payload struct ***/
typedef struct
{
  setpointVersion_t version;        // Version of the Program Setpoint command
  uint8 setpointIndex;              // Index of the Setpoint
  uint16 setpointValue;             // Value of the Setpoint
  setpointType_t setpointType;      // Type of the Setpoint; it should be either lift or tilt
}programSetpointPayload_t;

/*** Window Covering Command - General struct ***/
typedef struct
{
  afAddrType_t            *srcAddr;   // requestor's address
  uint8                   cmdID;      // Command id
  uint8                   seqNum;     // Sequence number received with the message

  union                               // Payload
  {
    uint8 indexOfLiftSetpoint;
    uint8 percentageLiftValue;
    uint16 liftValue;
    uint8 indexOfTiltSetpoint;
    uint8 percentageTiltValue;
    uint16 TiltValue;
    programSetpointPayload_t programSetpoint;
  }un;
}zclWindowCovering_t;

#ifdef ZCL_DOORLOCK
  /*** Doorlock Cluster Server Commands Received Structs ***/

// Server Commands Received: Lock Door, Unlock Door, Toggle Door
typedef struct
{
  uint8 *pPinRfidCode;   // variable length string
} zclDoorLock_t;

// Server Commands Received: Unlock with Timeout
typedef struct
{
  uint16 timeout;   // in seconds
  uint8 *pPinRfidCode;    // variable length string
} zclDoorLockUnlockTimeout_t;

#define PAYLOAD_LEN_UNLOCK_TIMEOUT   2

// Server Commands Received: Get Log Record
typedef struct
{
  uint16 logIndex;
} zclDoorLockGetLogRecord_t;

#define PAYLOAD_LEN_GET_LOG_RECORD    2

// Server Commands Received: Set PIN Code
typedef struct
{
  uint16 userID;
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
  uint8 *pPIN;    // variable length string
} zclDoorLockSetPINCode_t;

#define PAYLOAD_LEN_SET_PIN_CODE    4 // not including pPIN

// Server Commands Received: Get PIN Code, Clear PIN Code, Get User Status,
//                           Get User Type, Get RFID Code, Clear RFID Code
typedef struct
{
  uint16 userID;
} zclDoorLockUserID_t;

#define PAYLOAD_LEN_USER_ID   2

// Server Commands Received: Set User Status
typedef struct
{
  uint16 userID;
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
} zclDoorLockSetUserStatus_t;

#define PAYLOAD_LEN_SET_USER_STATUS   3

// Server Commands Received: Set Week Day Schedule
typedef struct
{
  uint8 scheduleID;
  uint16 userID;
  uint8 daysMask;
  uint8 startHour;
  uint8 startMinute;
  uint8 endHour;
  uint8 endMinute;
} zclDoorLockSetWeekDaySchedule_t;

#define PAYLOAD_LEN_SET_WEEK_DAY_SCHEDULE   8

// Server Commands Received: Get Week Day Schedule, Clear Week Day Schedule,
//                           Get Year Day Schedule, Clear Year Day Schedule
typedef struct
{
  uint8 scheduleID;
  uint16 userID;
} zclDoorLockSchedule_t;

#define PAYLOAD_LEN_SCHEDULE    3

// Server Commands Received: Set Year Day Schedule
typedef struct
{
  uint8 scheduleID;
  uint16 userID;
  uint32 zigBeeLocalStartTime;
  uint32 zigBeeLocalEndTime;
} zclDoorLockSetYearDaySchedule_t;

#define PAYLOAD_LEN_SET_YEAR_DAY_SCHEDULE   11

// Server Commands Received: Set Holiday Schedule
typedef struct
{
 uint8 holidayScheduleID;
 uint32 zigBeeLocalStartTime;
 uint32 zigBeeLocalEndTime;
 uint8 operatingModeDuringHoliday;
} zclDoorLockSetHolidaySchedule_t;

#define PAYLOAD_LEN_SET_HOLIDAY_SCHEDULE    10

// Server Commands Received: Get Holiday Schedule, Clear Holiday Schedule
typedef struct
{
  uint8 holidayScheduleID;
} zclDoorLockHolidayScheduleID_t;

#define PAYLOAD_LEN_HOLIDAY_SCHEDULE    1

// Server Commands Received: Set User Type
typedef struct
{
  uint16 userID;
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
} zclDoorLockSetUserType_t;

#define PAYLOAD_LEN_SET_USER_TYPE   3

// Server Commands Received: Set RFID Code
typedef struct
{
  uint16 userID;
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
  uint8 *pRfidCode;   // variable length string
} zclDoorLockSetRFIDCode_t;

#define PAYLOAD_LEN_SET_RFID_CODE   4 // not including pRfidCode

  /*** Doorlock Cluster Server Commands Generated Structs ***/

// Client Commands Received: Get Log Record Response
typedef struct
{
  uint16 logEntryID;
  uint32 timestamp;
  uint8 eventType;    // e.g. USER_TYPE_UNRESTRICTED_USER
  uint8 source;
  uint8 eventIDAlarmCode;
  uint16 userID;
  uint8 *pPIN;    // variable length string
} zclDoorLockGetLogRecordRsp_t;

#define PAYLOAD_LEN_GET_LOG_RECORD_RSP    11  // not including pPIN

// Client Commands Received: Get PIN Code Response
typedef struct
{
  uint16 userID;
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
  uint8 *pCode;   // variable length string
} zclDoorLockGetPINCodeRsp_t;

#define PAYLOAD_LEN_GET_PIN_CODE_RSP    4 // not including pCode

// Client Commands Received: Get User Status Response
typedef struct
{
  uint16 userID;
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
} zclDoorLockGetUserStatusRsp_t;

#define PAYLOAD_LEN_GET_USER_STATUS_RSP   3

// Client Commands Received: Get User Type Response
typedef struct
{
  uint16 userID;
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
} zclDoorLockGetUserTypeRsp_t;

#define PAYLOAD_LEN_GET_USER_TYPE_RSP   3

// Client Commands Received: Get Week Day Schedule Response
typedef struct
{
  uint8 scheduleID;
  uint16 userID;
  uint8 status;
  uint8 daysMask;
  uint8 startHour;
  uint8 startMinute;
  uint8 endHour;
  uint8 endMinute;
} zclDoorLockGetWeekDayScheduleRsp_t;

#define PAYLOAD_LEN_GET_WEEK_DAY_SCHEDULE_RSP   9

// Client Commands Received: Get Year Day Schedule Response
typedef struct
{
  uint8 scheduleID;
  uint16 userID;
  uint8 status;
  uint32 zigBeeLocalStartTime;
  uint32 zigBeeLocalEndTime;
} zclDoorLockGetYearDayScheduleRsp_t;

#define PAYLOAD_LEN_GET_YEAR_DAY_SCHEDULE_RSP   12

// Client Commands Received: Get Holiday Schedule Response
typedef struct
{
  uint8 holidayScheduleID;
  uint8 status;
  uint32 zigBeeLocalStartTime;
  uint32 zigBeeLocalEndTime;
  uint8 operatingModeDuringHoliday;   // e.g. DOORLOCK_OP_MODE_NORMAL
} zclDoorLockGetHolidayScheduleRsp_t;

#define PAYLOAD_LEN_GET_HOLIDAY_SCHEDULE_RSP    11

// Client Commands Received: Get RFID Code Response
typedef struct
{
  uint16 userID;
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
  uint8 *pRfidCode;   // variable length string
} zclDoorLockGetRFIDCodeRsp_t;

#define PAYLOAD_LEN_GET_RFID_CODE_RSP   4 // not including pRfidCode

// Client Commands Received: Operation Event Notification
typedef struct
{
  uint8 operationEventSource;   // e.g. OPERATION_EVENT_SOURCE_KEYPAD
  uint8 operationEventCode;   // e.g. OPERATION_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC
  uint16 userID;
  uint8 pin;
  uint32 zigBeeLocalTime;
  uint8 *pData;   // variable length string
} zclDoorLockOperationEventNotification_t;

#define PAYLOAD_LEN_OPERATION_EVENT_NOTIFICATION    9 // not including pData

// Client Commands Received: Programming Event Notification
typedef struct
{
  uint8 programEventSource;   // e.g. OPERATION_EVENT_SOURCE_KEYPAD
  uint8 programEventCode;   // e.g. PROGRAMMING_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC
  uint16 userID;
  uint8 pin;
  uint8 userType;   // e.g. USER_TYPE_UNRESTRICTED_USER
  uint8 userStatus;   // e.g. USER_STATUS_AVAILABLE
  uint32 zigBeeLocalTime;
  uint8 *pData;   // variable length string
} zclDoorLockProgrammingEventNotification_t;

#define PAYLOAD_LEN_PROGRAMMING_EVENT_NOTIFICATION    11 // not including pData

//This callback is called to process an incoming Door Lock command
typedef ZStatus_t (*zclClosures_DoorLock_t) ( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd );

//This callback is called to process an incoming Door Lock Response command
typedef ZStatus_t (*zclClosures_DoorLockRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Unlock With Timeout command
typedef ZStatus_t (*zclClosures_DoorLockUnlockWithTimeout_t) ( zclIncoming_t *pInMsg, zclDoorLockUnlockTimeout_t *pCmd );

//This callback is called to process an incoming Get Log Record command
typedef ZStatus_t (*zclClosures_DoorLockGetLogRecord_t) ( zclIncoming_t *pInMsg, zclDoorLockGetLogRecord_t *pCmd );

//This callback is called to process an incoming Set PIN Code command
typedef ZStatus_t (*zclClosures_DoorLockSetPINCode_t) ( zclIncoming_t *pInMsg, zclDoorLockSetPINCode_t *pCmd );

//This callback is called to process an incoming Get PIN Code command
typedef ZStatus_t (*zclClosures_DoorLockGetPINCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

//This callback is called to process an incoming Clear PIN Code command
typedef ZStatus_t (*zclClosures_DoorLockClearPINCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

//This callback is called to process an incoming Clear All PIN Codes command
typedef ZStatus_t (*zclClosures_DoorLockClearAllPINCodes_t) ( zclIncoming_t *pInMsg );

//This callback is called to process an incoming Set User Status command
typedef ZStatus_t (*zclClosures_DoorLockSetUserStatus_t) ( zclIncoming_t *pInMsg, zclDoorLockSetUserStatus_t *pCmd );

//This callback is called to process an incoming Get User Status command
typedef ZStatus_t (*zclClosures_DoorLockGetUserStatus_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

//This callback is called to process an incoming Set Week Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockSetWeekDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSetWeekDaySchedule_t *pCmd );

//This callback is called to process an incoming Get Week Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockGetWeekDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

//This callback is called to process an incoming Clear Week Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockClearWeekDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

//This callback is called to process an incoming Set Year Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockSetYearDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSetYearDaySchedule_t *pCmd );

//This callback is called to process an incoming Get Year Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockGetYearDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

//This callback is called to process an incoming Clear Year Day Schedule command
typedef ZStatus_t (*zclClosures_DoorLockClearYearDaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSchedule_t *pCmd );

//This callback is called to process an incoming Set Holiday Schedule command
typedef ZStatus_t (*zclClosures_DoorLockSetHolidaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockSetHolidaySchedule_t *pCmd );

//This callback is called to process an incoming Get Holiday Schedule command
typedef ZStatus_t (*zclClosures_DoorLockGetHolidaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockHolidayScheduleID_t *pCmd );

//This callback is called to process an incoming Clear Holiday Schedule command
typedef ZStatus_t (*zclClosures_DoorLockClearHolidaySchedule_t) ( zclIncoming_t *pInMsg, zclDoorLockHolidayScheduleID_t *pCmd );

//This callback is called to process an incoming Set User Type command
typedef ZStatus_t (*zclClosures_DoorLockSetUserType_t) ( zclIncoming_t *pInMsg, zclDoorLockSetUserType_t *pCmd );

//This callback is called to process an incoming Get User Type command
typedef ZStatus_t (*zclClosures_DoorLockGetUserType_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

//This callback is called to process an incoming Set RFID Code command
typedef ZStatus_t (*zclClosures_DoorLockSetRFIDCode_t) ( zclIncoming_t *pInMsg, zclDoorLockSetRFIDCode_t *pCmd );

//This callback is called to process an incoming Get RFID Code command
typedef ZStatus_t (*zclClosures_DoorLockGetRFIDCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

//This callback is called to process an incoming Clear RFID Code command
typedef ZStatus_t (*zclClosures_DoorLockClearRFIDCode_t) ( zclIncoming_t *pInMsg, zclDoorLockUserID_t *pCmd );

//This callback is called to process an incoming Clear All RFID Codes command
typedef ZStatus_t (*zclClosures_DoorLockClearAllRFIDCodes_t) ( zclIncoming_t *pInMsg );

//This callback is called to process an incoming Lock Door Response command
typedef ZStatus_t (*zclClosures_DoorLockLockDoorRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Unlock Door Response command
typedef ZStatus_t (*zclClosures_DoorLockUnlockDoorRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Toggle Door Response command
typedef ZStatus_t (*zclClosures_DoorLockToggleDoorRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Unlock With Timeout Response command
typedef ZStatus_t (*zclClosures_DoorLockUnlockWithTimeoutRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get Log Record Response command
typedef ZStatus_t (*zclClosures_DoorLockGetLogRecordRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetLogRecordRsp_t *pCmd );

//This callback is called to process an incoming Set PIN Code Response command
typedef ZStatus_t (*zclClosures_DoorLockSetPINCodeRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get PIN Code Response command
typedef ZStatus_t (*zclClosures_DoorLockGetPINCodeRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetPINCodeRsp_t *pCmd );

//This callback is called to process an incoming Clear PIN Code Response command
typedef ZStatus_t (*zclClosures_DoorLockClearPINCodeRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Clear All PIN Codes Response command
typedef ZStatus_t (*zclClosures_DoorLockClearAllPINCodesRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Set User Status Response command
typedef ZStatus_t (*zclClosures_DoorLockSetUserStatusRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get User Status Response command
typedef ZStatus_t (*zclClosures_DoorLockGetUserStatusRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetUserStatusRsp_t *pCmd );

//This callback is called to process an incoming Set Week Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockSetWeekDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get Week Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockGetWeekDayScheduleRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetWeekDayScheduleRsp_t *pCmd );

//This callback is called to process an incoming Clear Week Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockClearWeekDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Set Year Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockSetYearDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get Year Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockGetYearDayScheduleRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetYearDayScheduleRsp_t *pCmd );

//This callback is called to process an incoming Clear Year Day Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockClearYearDayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Set Holiday Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockSetHolidayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get Holiday Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockGetHolidayScheduleRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetHolidayScheduleRsp_t *pCmd );

//This callback is called to process an incoming Clear Holiday Schedule Response command
typedef ZStatus_t (*zclClosures_DoorLockClearHolidayScheduleRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Set User Type Response command
typedef ZStatus_t (*zclClosures_DoorLockSetUserTypeRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get User Type Response command
typedef ZStatus_t (*zclClosures_DoorLockGetUserTypeRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetUserTypeRsp_t *pCmd );

//This callback is called to process an incoming Set RFID Code Response command
typedef ZStatus_t (*zclClosures_DoorLockSetRFIDCodeRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Get RFID Code Response command
typedef ZStatus_t (*zclClosures_DoorLockGetRFIDCodeRsp_t) ( zclIncoming_t *pInMsg, zclDoorLockGetRFIDCodeRsp_t *pCmd );

//This callback is called to process an incoming Clear RFID Code Response command
typedef ZStatus_t (*zclClosures_DoorLockClearRFIDCodeRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Clear All RFID Codes Response command
typedef ZStatus_t (*zclClosures_DoorLockClearAllRFIDCodesRsp_t) ( zclIncoming_t *pInMsg, uint8 status );

//This callback is called to process an incoming Operation Event Notification command
typedef ZStatus_t (*zclClosures_DoorLockOperationEventNotification_t) ( zclIncoming_t *pInMsg, zclDoorLockOperationEventNotification_t *pCmd );

//This callback is called to process an incoming Programming Event Notification command
typedef ZStatus_t (*zclClosures_DoorLockProgrammingEventNotification_t) ( zclIncoming_t *pInMsg, zclDoorLockProgrammingEventNotification_t *pCmd );
#endif // ZCL_DOORLOCK

//This callback is called to process an incoming Window Covering cluster basic commands
typedef void (*zclClosures_WindowCoveringSimple_t) ( void );

//This callback is called to process an incoming Window Covering cluster goto percentage commands
typedef bool (*zclClosures_WindowCoveringGotoPercentage_t) ( uint8 percentage );

//This callback is called to process an incoming Window Covering cluster goto value commands
typedef bool (*zclClosures_WindowCoveringGotoValue_t) ( uint16 value );

//This callback is called to process an incoming Window Covering cluster goto setpoint commands
typedef uint8 (*zclClosures_WindowCoveringGotoSetpoint_t) ( uint8 index );

//This callback is called to process an incoming Window Covering cluster program setpoint commands
typedef bool (*zclClosures_WindowCoveringProgramSetpoint_t) ( programSetpointPayload_t *setpoint );

#ifdef ZCL_DOORLOCK
// Register Callbacks DoorLock Cluster table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclClosures_DoorLock_t                                        pfnDoorLock;
  zclClosures_DoorLockRsp_t                                     pfnDoorLockRsp;
  zclClosures_DoorLockUnlockWithTimeout_t                       pfnDoorLockUnlockWithTimeout;
  zclClosures_DoorLockGetLogRecord_t                            pfnDoorLockGetLogRecord;
  zclClosures_DoorLockSetPINCode_t                              pfnDoorLockSetPINCode;
  zclClosures_DoorLockGetPINCode_t                              pfnDoorLockGetPINCode;
  zclClosures_DoorLockClearPINCode_t                            pfnDoorLockClearPINCode;
  zclClosures_DoorLockClearAllPINCodes_t                        pfnDoorLockClearAllPINCodes;
  zclClosures_DoorLockSetUserStatus_t                           pfnDoorLockSetUserStatus;
  zclClosures_DoorLockGetUserStatus_t                           pfnDoorLockGetUserStatus;
  zclClosures_DoorLockSetWeekDaySchedule_t                      pfnDoorLockSetWeekDaySchedule;
  zclClosures_DoorLockGetWeekDaySchedule_t                      pfnDoorLockGetWeekDaySchedule;
  zclClosures_DoorLockClearWeekDaySchedule_t                    pfnDoorLockClearWeekDaySchedule;
  zclClosures_DoorLockSetYearDaySchedule_t                      pfnDoorLockSetYearDaySchedule;
  zclClosures_DoorLockGetYearDaySchedule_t                      pfnDoorLockGetYearDaySchedule;
  zclClosures_DoorLockClearYearDaySchedule_t                    pfnDoorLockClearYearDaySchedule;
  zclClosures_DoorLockSetHolidaySchedule_t                      pfnDoorLockSetHolidaySchedule;
  zclClosures_DoorLockGetHolidaySchedule_t                      pfnDoorLockGetHolidaySchedule;
  zclClosures_DoorLockClearHolidaySchedule_t                    pfnDoorLockClearHolidaySchedule;
  zclClosures_DoorLockSetUserType_t                             pfnDoorLockSetUserType;
  zclClosures_DoorLockGetUserType_t                             pfnDoorLockGetUserType;
  zclClosures_DoorLockSetRFIDCode_t                             pfnDoorLockSetRFIDCode;
  zclClosures_DoorLockGetRFIDCode_t                             pfnDoorLockGetRFIDCode;
  zclClosures_DoorLockClearRFIDCode_t                           pfnDoorLockClearRFIDCode;
  zclClosures_DoorLockClearAllRFIDCodes_t                       pfnDoorLockClearAllRFIDCodes;
  zclClosures_DoorLockUnlockWithTimeoutRsp_t                    pfnDoorLockUnlockWithTimeoutRsp;
  zclClosures_DoorLockGetLogRecordRsp_t                         pfnDoorLockGetLogRecordRsp;
  zclClosures_DoorLockSetPINCodeRsp_t                           pfnDoorLockSetPINCodeRsp;
  zclClosures_DoorLockGetPINCodeRsp_t                           pfnDoorLockGetPINCodeRsp;
  zclClosures_DoorLockClearPINCodeRsp_t                         pfnDoorLockClearPINCodeRsp;
  zclClosures_DoorLockClearAllPINCodesRsp_t                     pfnDoorLockClearAllPINCodesRsp;
  zclClosures_DoorLockSetUserStatusRsp_t                        pfnDoorLockSetUserStatusRsp;
  zclClosures_DoorLockGetUserStatusRsp_t                        pfnDoorLockGetUserStatusRsp;
  zclClosures_DoorLockSetWeekDayScheduleRsp_t                   pfnDoorLockSetWeekDayScheduleRsp;
  zclClosures_DoorLockGetWeekDayScheduleRsp_t                   pfnDoorLockGetWeekDayScheduleRsp;
  zclClosures_DoorLockClearWeekDayScheduleRsp_t                 pfnDoorLockClearWeekDayScheduleRsp;
  zclClosures_DoorLockSetYearDayScheduleRsp_t                   pfnDoorLockSetYearDayScheduleRsp;
  zclClosures_DoorLockGetYearDayScheduleRsp_t                   pfnDoorLockGetYearDayScheduleRsp;
  zclClosures_DoorLockClearYearDayScheduleRsp_t                 pfnDoorLockClearYearDayScheduleRsp;
  zclClosures_DoorLockSetHolidayScheduleRsp_t                   pfnDoorLockSetHolidayScheduleRsp;
  zclClosures_DoorLockGetHolidayScheduleRsp_t                   pfnDoorLockGetHolidayScheduleRsp;
  zclClosures_DoorLockClearHolidayScheduleRsp_t                 pfnDoorLockClearHolidayScheduleRsp;
  zclClosures_DoorLockSetUserTypeRsp_t                          pfnDoorLockSetUserTypeRsp;
  zclClosures_DoorLockGetUserTypeRsp_t                          pfnDoorLockGetUserTypeRsp;
  zclClosures_DoorLockSetRFIDCodeRsp_t                          pfnDoorLockSetRFIDCodeRsp;
  zclClosures_DoorLockGetRFIDCodeRsp_t                          pfnDoorLockGetRFIDCodeRsp;
  zclClosures_DoorLockClearRFIDCodeRsp_t                        pfnDoorLockClearRFIDCodeRsp;
  zclClosures_DoorLockClearAllRFIDCodesRsp_t                    pfnDoorLockClearAllRFIDCodesRsp;
  zclClosures_DoorLockOperationEventNotification_t              pfnDoorLockOperationEventNotification;
  zclClosures_DoorLockProgrammingEventNotification_t            pfnDoorLockProgrammingEventNotification;
} zclClosures_DoorLockAppCallbacks_t;
#endif  // ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
// Register Callbacks Window Covering Cluster table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclClosures_WindowCoveringSimple_t                            pfnWindowCoveringUpOpen;
  zclClosures_WindowCoveringSimple_t                            pfnWindowCoveringDownClose;
  zclClosures_WindowCoveringSimple_t                            pfnWindowCoveringStop;
  zclClosures_WindowCoveringGotoValue_t                         pfnWindowCoveringGotoLiftValue;
  zclClosures_WindowCoveringGotoPercentage_t                    pfnWindowCoveringGotoLiftPercentage;
  zclClosures_WindowCoveringGotoValue_t                         pfnWindowCoveringGotoTiltValue;
  zclClosures_WindowCoveringGotoPercentage_t                    pfnWindowCoveringGotoTiltPercentage;
} zclClosures_WindowCoveringAppCallbacks_t;
#endif // ZCL_WINDOWCOVERING

/*********************************************************************
 * VARIABLES
 */


/*********************************************************************
 * FUNCTIONS
 */
#ifdef ZCL_DOORLOCK
 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclClosures_RegisterDoorLockCmdCallbacks( uint8 endpoint, zclClosures_DoorLockAppCallbacks_t *callbacks );

/*
 * The following functions are used in low-level routines.
 * See Function Macros for app-level send functions
 */
extern ZStatus_t zclClosures_SendDoorLockRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                  zclDoorLock_t *pPayload,
                                                  uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockUnlockTimeoutRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                               zclDoorLockUnlockTimeout_t *pPayload,
                                                               uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetLogRecordRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                              uint16 logIndex, uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetPINCodeRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                            zclDoorLockSetPINCode_t *pPayload,
                                                            uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockUserIDRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                        uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockClearAllCodesRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                               uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetUserStatusRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint16 userID, uint8 userStatus,
                                                               uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetWeekDayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                                    uint8 scheduleID, uint16 userID,
                                                                    uint8 daysMask, uint8 startHour,
                                                                    uint8 startMinute, uint8 endHour, uint8 endMinute,
                                                                    uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                          uint8 scheduleID, uint16 userID,
                                                          uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetYearDayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                                    uint8 scheduleID, uint16 userID,
                                                                    uint32 zigBeeLocalStartTime, uint32 zigBeeLocalEndTime,
                                                                    uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetHolidayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                                    uint8 holidayScheduleID, uint32 zigBeeLocalStartTime,
                                                                    uint32 zigBeeLocalEndTime, uint8 operatingModeDuringHoliday,
                                                                    uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockHolidayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                                 uint8 holidayScheduleID,
                                                                 uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetUserTypeRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint16 userID, uint8 userType,
                                                             uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockSetRFIDCodeRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                             zclDoorLockSetRFIDCode_t *pPayload,
                                                             uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockStatusResponse( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                         uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetLogRecordResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                               zclDoorLockGetLogRecordRsp_t *pPayload,
                                                               uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetPINCodeResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                             zclDoorLockGetPINCodeRsp_t *pPayload,
                                                             uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetUserStatusResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                                uint16 userID, uint8 userStatus,
                                                                uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetWeekDayScheduleResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockGetWeekDayScheduleRsp_t *pCmd,
                                                                     uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetYearDayScheduleResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockGetYearDayScheduleRsp_t *pCmd,
                                                                     uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetHolidayScheduleResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockGetHolidayScheduleRsp_t *pCmd,
                                                                     uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetUserTypeResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                              uint16 userID, uint8 userType,
                                                              uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockGetRFIDCodeResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                              zclDoorLockGetRFIDCodeRsp_t *pPayload,
                                                              uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockOperationEventNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                     zclDoorLockOperationEventNotification_t *pPayload,
                                                                     uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_SendDoorLockProgrammingEventNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                       zclDoorLockProgrammingEventNotification_t *pPayload,
                                                                       uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclClosures_RegisterWindowCoveringCmdCallbacks( uint8 endpoint, zclClosures_WindowCoveringAppCallbacks_t *callbacks );

/*
 * The following functions are used in low-level routines.
 * See Function Macros for app-level send functions
 */
extern ZStatus_t zclClosures_WindowCoveringSimpleReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint8 cmd, uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_WindowCoveringSendGoToValueReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint8 cmd, uint16 value,
                                                             uint8 disableDefaultRsp, uint8 seqNum );
extern ZStatus_t zclClosures_WindowCoveringSendGoToPercentageReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint8 cmd, uint8 percentageValue,
                                                                  uint8 disableDefaultRsp, uint8 seqNum );
#endif // ZCL_WINDOWCOVERING
/*********************************************************************
 * FUNCTION MACROS
 */

/*
 *  Send a Door Lock Lock Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockLockDoor( uint8 srcEP, afAddrType_t *dstAddr, zclDoorLock_t *pPayload, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockLockDoor(a, b, c, d, e) zclClosures_SendDoorLockRequest( (a), (b), COMMAND_CLOSURES_LOCK_DOOR, (c), (d), (e) )

/*
 *  Send a Door Lock Unlock Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockUnlockDoor( uint8 srcEP, afAddrType_t *dstAddr, zclDoorLock_t *pPayload, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockUnlockDoor(a, b, c, d, e) zclClosures_SendDoorLockRequest( (a), (b), COMMAND_CLOSURES_UNLOCK_DOOR, (c), (d), (e) )

/*
 *  Send a Door Lock Toggle Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockToggleDoor( uint8 srcEP, afAddrType_t *dstAddr, zclDoorLock_t *pPayload, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockToggleDoor(a, b, c, d, e) zclClosures_SendDoorLockRequest( (a), (b), COMMAND_CLOSURES_TOGGLE_DOOR, (c), (d), (e) )

/*
 *  Send a Get PIN Code Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetPINCode( uint8 srcEP, afAddrType_t *dstAddr, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockGetPINCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_CLOSURES_GET_PIN_CODE, (c), (d), (e) )

/*
 *  Send a Clear PIN Code Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearPINCode( uint8 srcEP, afAddrType_t *dstAddr, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearPINCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_CLOSURES_CLEAR_PIN_CODE, (c), (d), (e) )

/*
 *  Send a Clear All PIN Codes Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllPINCodes( uint8 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearAllPINCodes(a, b, c, d) zclClosures_SendDoorLockClearAllCodesRequest( (a), (b), COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES, (c), (d) )

/*
 *  Send a Get User Status Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetUserStatus( uint8 srcEP, afAddrType_t *dstAddr, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendDoorLockGetUserStatus(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_CLOSURES_GET_USER_STATUS, (c), (d), (e) )

/*
 *  Send a Get Week Day Schedule Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetWeekDaySchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 scheduleID, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum);
 */
#define zclClosures_SendDoorLockGetWeekDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE, (c), (d), (e), (f) )

/*
 *  Send a Clear Week Day Schedule Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearWeekDaySchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 scheduleID, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearWeekDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE, (c), (d), (e), (f) )

/*
 *  Send a Get Year Day Schedule Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetYearDaySchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 scheduleID, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockGetYearDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE, (c), (d), (e), (f) )

/*
 *  Send a Clear Year Day Schedule Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearYearDaySchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 scheduleID, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearYearDaySchedule(a, b, c, d, e, f) zclClosures_SendDoorLockScheduleRequest( (a), (b), COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE, (c), (d), (e), (f) )

/*
 *  Send a Get Holiday Schedule Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetHolidaySchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 holidayScheduleID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockGetHolidaySchedule(a, b, c, d, e) zclClosures_SendDoorLockHolidayScheduleRequest( (a), (b), COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE, (c), (d), (e) )

/*
 *  Send a Clear Holiday Schedule Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearHolidaySchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 holidayScheduleID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearHolidaySchedule(a, b, c, d, e) zclClosures_SendDoorLockHolidayScheduleRequest( (a), (b), COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE, (c), (d), (e) )

/*
 *  Send a Get User Type Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetUserType( uint8 srcEP, afAddrType_t *dstAddr, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockGetUserType(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_CLOSURES_GET_USER_TYPE, (c), (d), (e) )

/*
 *  Send a Get RFID Code Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockGetRFIDCode( uint8 srcEP, afAddrType_t *dstAddr, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockGetRFIDCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_CLOSURES_GET_RFID_CODE, (c), (d), (e) )

/*
 *  Send a Clear RFID Code Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearRFIDCode( uint8 srcEP, afAddrType_t *dstAddr, uint16 userID, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearRFIDCode(a, b, c, d, e) zclClosures_SendDoorLockUserIDRequest( (a), (b), COMMAND_CLOSURES_CLEAR_RFID_CODE, (c), (d), (e) )

/*
 *  Send a Clear All RFID Codes Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllRFIDCodes( uint8 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendDoorLockClearAllRFIDCodes(a, b, c, d) zclClosures_SendDoorLockClearAllCodesRequest( (a), (b), COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES, (c), (d) )

/*
 *  Send a Door Lock Lock Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockLockDoorRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendDoorLockLockDoorRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_LOCK_DOOR_RSP, (c), (d), (e) )

/*
 *  Send a Door Lock Unlock Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockUnlockDoorRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendDoorLockUnlockDoorRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_UNLOCK_DOOR_RSP, (c), (d), (e) )

/*
 *  Send a Toggle Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockToggleDoorRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockToggleDoorRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_TOGGLE_DOOR_RSP, (c), (d), (e) )

/*
 *  Send a Unlock With Timeout Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockUnlockWithTimeoutRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockUnlockWithTimeoutRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT_RSP, (c), (d), (e) )

/*
 *  Send a Set PIN Code Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetPINCodeRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetPINCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_PIN_CODE_RSP, (c), (d), (e) )

/*
 *  Send a Clear PIN Code Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearPINCodeRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearPINCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_PIN_CODE_RSP, (c), (d), (e) )

/*
 *  Send a Clear All PIN Codes Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllPINCodesRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearAllPINCodesRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES_RSP, (c), (d), (e) )

/*
 *  Send a Set User Status Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetUserStatusRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetUserStatusRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_USER_STATUS_RSP, (c), (d), (e) )

/*
 *  Send a Set Week Day Schedule Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetWeekDayScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetWeekDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE_RSP, (c), (d), (e) )

/*
 *  Send a Clear Week Day Schedule Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearWeekDayScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearWeekDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE_RSP, (c), (d), (e) )

/*
 *  Send a Set Year Day Schedule Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetYearDayScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetYearDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE_RSP, (c), (d), (e) )

/*
 *  Send a Clear Year Day Schedule Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearYearDayScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearYearDayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE_RSP, (c), (d), (e) )

/*
 *  Send a Set Holiday Schedule Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetHolidayScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetHolidayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE_RSP, (c), (d), (e) )

/*
 *  Send a Clear Holiday Schedule Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearHolidayScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearHolidayScheduleRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE_RSP, (c), (d), (e) )

/*
 *  Send a Set User Type Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetUserTypeRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetUserTypeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_USER_TYPE_RSP, (c), (d), (e) )

/*
 *  Send a Set RFID Code Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockSetRFIDCodeRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockSetRFIDCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_SET_RFID_CODE_RSP, (c), (d), (e) )

/*
 *  Send a Clear RFID Code Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearRFIDCodeRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearRFIDCodeRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_RFID_CODE_RSP, (c), (d), (e) )

/*
 *  Send a Clear All RFID Codes Response
 *  Use like:
 *      ZStatus_t zclClosures_SendDoorLockClearAllRFIDCodesRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 status, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclClosures_SendDoorLockClearAllRFIDCodesRsp(a, b, c, d, e) zclClosures_SendDoorLockStatusResponse( (a), (b), COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES_RSP, (c), (d), (e) )

/*
 *  Send a Up/Open Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendUpOpen( uint8 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendUpOpen(a, b, c, d) zclClosures_WindowCoveringSimpleReq( (a), (b), COMMAND_CLOSURES_UP_OPEN, (c), (d) )
/*
 *  Send a Down/Close Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendDownClose( uint8 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendDownClose(a, b, c, d) zclClosures_WindowCoveringSimpleReq( (a), (b), COMMAND_CLOSURES_DOWN_CLOSE, (c), (d) )

/*
 *  Send a Stop Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendStop( uint8 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendStop(a, b, c, d) zclClosures_WindowCoveringSimpleReq( (a), (b), COMMAND_CLOSURES_STOP, (c), (d) )

/*
 *  Send a GoToLiftValue Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToLiftValue( uint8 srcEP, afAddrType_t *dstAddr, uint16 liftValue, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendGoToLiftValue(a, b, c, d, e) zclClosures_WindowCoveringSendGoToValueReq( (a), (b), COMMAND_CLOSURES_GO_TO_LIFT_VALUE, (c), (d), (e))

/*
 *  Send a GoToLiftPercentage Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToLiftPercentage( uint8 srcEP, afAddrType_t *dstAddr, uint8 percentageLiftValue, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendGoToLiftPercentage(a, b, c, d, e) zclClosures_WindowCoveringSendGoToPercentageReq( (a), (b), COMMAND_CLOSURES_GO_TO_LIFT_PERCENTAGE, (c), (d), (e))

/*
 *  Send a GoToTiltValue Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToTiltValue( uint8 srcEP, afAddrType_t *dstAddr, uint16 tiltValue, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendGoToTiltValue(a, b, c, d, e) zclClosures_WindowCoveringSendGoToValueReq( (a), (b), COMMAND_CLOSURES_GO_TO_TILT_VALUE, (c), (d), (e))

/*
 *  Send a GoToTiltPercentage Request Command
 *  Use like:
 *      ZStatus_t zclClosures_SendGoToTiltPercentage( uint8 srcEP, afAddrType_t *dstAddr, uint8 percentageTiltValue, uint8 disableDefaultRsp, uint8 seqNum )
 */
#define zclClosures_SendGoToTiltPercentage(a, b, c, d, e) zclClosures_WindowCoveringSendGoToPercentageReq( (a), (b), COMMAND_CLOSURES_GO_TO_TILT_PERCENTAGE, (c), (d), (e))

#ifdef __cplusplus
}
#endif

#endif /* ZCL_CLOSURES_H */
