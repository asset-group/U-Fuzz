/**************************************************************************************************
  Filename:       zcl_hvac.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL HVAC definitions.


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

#ifndef ZCL_HVAC_H
#define ZCL_HVAC_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZCL_HVAC_CLUSTER

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */

/**************************************************************************/
/***          HVAC: Pump Config and Control Cluster Attributes          ***/
/**************************************************************************/
  // Pump information attribute set
#define ATTRID_HVAC_PUMP_MAX_PRESSURE                                    0x0000
#define ATTRID_HVAC_PUMP_MAX_SPEED                                       0x0001
#define ATTRID_HVAC_PUMP_MAX_FLOW                                        0x0002
#define ATTRID_HVAC_PUMP_MIN_CONST_PRESSURE                              0x0003
#define ATTRID_HVAC_PUMP_MAX_CONST_PRESSURE                              0x0004
#define ATTRID_HVAC_PUMP_MIN_COMP_PRESSURE                               0x0005
#define ATTRID_HVAC_PUMP_MAX_COMP_PRESSURE                               0x0006
#define ATTRID_HVAC_PUMP_MIN_CONST_SPEED                                 0x0007
#define ATTRID_HVAC_PUMP_MAX_CONST_SPEED                                 0x0008
#define ATTRID_HVAC_PUMP_MIN_CONST_FLOW                                  0x0009
#define ATTRID_HVAC_PUMP_MAX_CONST_FLOW                                  0x000A
#define ATTRID_HVAC_PUMP_MIN_CONST_TEMP                                  0x000B
#define ATTRID_HVAC_PUMP_MAX_CONST_TEMP                                  0x000C
  // Pump Dynamic Information attribute set
#define ATTRID_HVAC_PUMP_STATUS                                          0x0010
#define ATTRID_HVAC_PUMP_EFECTIVE_OPERATION_MODE                         0x0011
#define ATTRID_HVAC_PUMP_EFFECTIVE_CONTROL_MODE                          0x0012
#define ATTRID_HVAC_PUMP_CAPACITY                                        0x0013
#define ATTRID_HVAC_PUMP_SPEED                                           0x0014
#define ATTRID_HVAC_PUMP_LIFETIME_RUNNING_HOURS                          0x0015
#define ATTRID_HVAC_PUMP_POWER                                           0x0016
#define ATTRID_HVAC_LIFETIME_ENERGY_CONSUMED                             0x0017
  /*** Pump status attribute values ***/
#define HVAC_PUMP_DEVICE_FAULT                                           0x0000
#define HVAC_PUMP_SUPPLY_FAULT                                           0x0001
#define HVAC_PUMP_SPEED_LOW                                              0x0002
#define HVAC_PUMP_SPEED_HIGH                                             0x0003
#define HVAC_PUMP_LOCAL_OVERRIDE                                         0x0004
#define HVAC_PUMP_RUNNING                                                0x0005
#define HVAC_PUMP_REMOTE_PRESSURE                                        0x0006
#define HVAC_PUMP_REMOTE_FLOW                                            0x0007
#define HVAC_PUMP_REMOTE_TEMPERATURE                                     0x0008
  // Pump Settings attributes set
#define ATTRID_HVAC_PUMP_OPERATION_MODE                                  0x0020
#define ATTRID_HVAC_PUMP_CONTROL_MODE                                    0x0021
#define ATTRID_HVAC_PUMP_ALARM_MASK                                      0x0022
//#define ATTRID_HVAC_PUMP_CAPACITY_MIN_PERCENT_CHANGE                     0x0023
//#define ATTRID_HVAC_PUMP_CAPACITY_MIN_ABSOLUTE_CHANGE                    0x0024
/*** Operation mode attribute values ***/
#define HVAC_PUMP_OPERATION_MODE_NORMAL                                  0x00
#define HVAC_PUMP_OPERATION_MODE_MINIMUM                                 0x01
#define HVAC_PUMP_OPERATION_MODE_MAXIMUM                                 0x02
#define HVAC_PUMP_OPERATION_MODE_LOCAL                                   0x03
/*** Control mode attribute values ***/
#define HVAC_PUMP_CONTROL_MODE_CONSTANT_SPEED                            0x00
#define HVAC_PUMP_CONTROL_MODE_CONSTANT_PRESSURE                         0x01
#define HVAC_PUMP_CONTROL_MODE_PR0PORTIONAL_PRESSURE                     0x02
#define HVAC_PUMP_CONTROL_MODE_CONSTANT_FLOW	                         0x03
#define HVAC_PUMP_CONTROL_MODE_CONSTANT_TEMPERATURE	                     0x05
#define HVAC_PUMP_CONTROL_MODE_CONSTANT_AUTOMATIC                        0x07
/*** Alarm Mask attribute values ***/
#define HVAC_PUMP_SUPPLY_VOLTAGE_TOO_LOW                                 0x0001
#define HVAC_PUMP_SUPPLY_VOLTAGE_TOO_HIGH                                0x0002
#define HVAC_PUMP_POWER_MISSING_PHASE                                    0x0004
#define HVAC_PUMP_SYSTEM_PRESSURE_TOO_LOW                                0x0008
#define HVAC_PUMP_SYSTEM_PRESSURE_TOO_HIGH                               0x0010
#define HVAC_PUMP_DRY_RUNNING                                            0x0020
#define HVAC_PUMP_MOTOR_TEMPERATURE_TOO_HIGH                             0x0040
#define HVAC_PUMP_MOTOR_HAS_FATAL_FAILURE                                0x0080
#define HVAC_PUMP_ELECTRONIC_TEMPERATURE_TOO_HIGH                        0x0100
#define HVAC_PUMP_BLOCKED                                                0x0200
#define HVAC_PUMP_SENSOR_FAILURE                                         0x0400
#define HVAC_PUMP_ELECTRONIC_NON_FATAL_FAILURE                           0x0800
#define HVAC_PUMP_ELECTRONIC_FATAL_FAILURE                               0x1000
#define HVAC_PUMP_GENERAL_FAULT                                          0x2000
  // Pump Unit Table attributes set
//#define ATTRID_HVAC_PUMP_POWER_UNIT                                      0x0030
/*** Power Unit attribute values ***/
//#define HVAC_PUMP_POWER_UNIT_POWER_IN_WATTS                              0x00
//#define HVAC_PUMP_POWER_UNIT_POWER_IN_KILOWATTS                          0x01

/**************************************************************************/
/***          HVAC: Thermostat Cluster Attributes                       ***/
/**************************************************************************/
  // Thermostat information attribute set
#define ATTRID_HVAC_THERMOSTAT_LOCAL_TEMPERATURE                         0x0000 // M, R, INT16
#define ATTRID_HVAC_THERMOSTAT_OUTDOOR_TEMPERATURE                       0x0001 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_OCCUPANCY                                 0x0002 // O, R, BITMAP8
#define ATTRID_HVAC_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT               0x0003 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT               0x0004 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT               0x0005 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT               0x0006 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_PI_COOLING_DEMAND                         0x0007 // O, R, UINT8
#define ATTRID_HVAC_THERMOSTAT_PI_HEATING_DEMAND                         0x0008 // O, R, UINT8
#define ATTRID_HVAC_THERMOSTAT_SYSTEM_TYPE_CONFIGURATION                 0x0009 // O, R/W, BITMAP8

  // Thermostat settings attribute set
#define ATTRID_HVAC_THERMOSTAT_LOCAL_TEMP_CALIBRATION                    0x0010 // O, R/W, INT8
#define ATTRID_HVAC_THERMOSTAT_OCCUPIED_COOLING_SETPOINT                 0x0011 // M, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_OCCUPIED_HEATING_SETPOINT                 0x0012 // M, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT               0x0013 // O, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT               0x0014 // O, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT                   0x0015 // O, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT                   0x0016 // O, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT                   0x0017 // O, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT                   0x0018 // O, R/W, INT16
#define ATTRID_HVAC_THERMOSTAT_MIN_SETPOINT_DEAD_BAND                    0x0019 // O, R/W, INT8
#define ATTRID_HVAC_THERMOSTAT_REMOTE_SENSING                            0x001A // O, R/W, BITMAP8
#define ATTRID_HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER                          0x001B // M, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_SYSTEM_MODE                               0x001C // M, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_ALAM_MASK                                 0x001D // O, R, BITMAP8
#define ATTRID_HVAC_THERMOSTAT_RUNNING_MODE                              0x001E // O, R, ENUM8

  // Thermostat Schedule & HVAC Relay Attribute Set
#define ATTRID_HVAC_THERMOSTAT_START_OF_WEEK                             0x0020 // O, R, ENUM8
#define ATTRID_HVAC_THERMOSTAT_NUMBER_OF_WEEKLY_TRANSITIONS              0x0021 // O, R, UINT8
#define ATTRID_HVAC_THERMOSTAT_NUMBER_OF_DAILY_TRANSITIONS               0x0022 // O, R, UINT8
#define ATTRID_HVAC_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD                 0x0023 // O, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_DURATION        0x0024 // O, R/W, UINT16
#define ATTRID_HVAC_THERMOSTAT_PROGRAMMING_OPERATION_MODE                0x0025 // O, R/W, BITMAP8
  // HVAC Relay Attribute:
#define ATTRID_HVAC_THERMOSTAT_RUNNING_STATE                             0x0029 // O, R, BITMAP16

  // Thermostat Setpoint Change Tracking Attribute Set
#define ATTRID_HVAC_THERMOSTAT_SETPOINT_CHANGE_SOURCE                    0x0030 // O, R, ENUM8
#define ATTRID_HVAC_THERMOSTAT_SETPOINT_CHANGE_AMOUNT                    0x0031 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_SETPOINT_CHANGE_SOURCE_TIMESTAMP          0x0032 // O, R, UTC

  // Thermostat AC Information Attribute Set
#define ATTRID_HVAC_THERMOSTAT_AC_TYPE                                   0x0040 // O, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_AC_CAPACITY                               0x0041 // O, R/W, UINT16
#define ATTRID_HVAC_THERMOSTAT_AC_REFRIGERANT_TYPE                       0x0042 // O, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_AC_COMPRESSOR_TYPE                        0x0043 // O, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_AC_ERROR_CODE                             0x0044 // O, R/W, BITMAP32
#define ATTRID_HVAC_THERMOSTAT_AC_LOUVER_POSITION                        0x0045 // O, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_AC_COIL_TEMPERATURE                       0x0046 // O, R, INT16
#define ATTRID_HVAC_THERMOSTAT_AC_CAPACITY_FORMAT                        0x0047 // O, R/W, ENUM8

  // Thermostat Attribute Defaults
#define ATTR_DEFAULT_HVAC_THERMOSTAT_LOCAL_TEMPERATURE                    0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_OUTDOOR_TEMPERATURE                  0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_OCCUPANCY                            0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT          0x02BC
#define ATTR_DEFAULT_HVAC_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT          0x0BB8
#define ATTR_DEFAULT_HVAC_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT          0x0640
#define ATTR_DEFAULT_HVAC_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT          0x0C80
#define ATTR_DEFAULT_HVAC_THERMOSTAT_PI_COOLING_DEMAND                    0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_PI_HEATING_DEMAND                    0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_SYSTEM_TYPE_CONFIGURATION            0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_LOCAL_TEMP_CALIBRATION               0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_OCCUPIED_COOLING_SETPOINT            0x0A28
#define ATTR_DEFAULT_HVAC_THERMOSTAT_OCCUPIED_HEATING_SETPOINT            0x07D0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT          0x0A28
#define ATTR_DEFAULT_HVAC_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT          0x07D0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT              0x02BC
#define ATTR_DEFAULT_HVAC_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT              0x0BB8
#define ATTR_DEFAULT_HVAC_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT              0x02BC
#define ATTR_DEFAULT_HVAC_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT              0x0BB8
#define ATTR_DEFAULT_HVAC_THERMOSTAT_MIN_SETPOINT_DEAD_BAND               0x19
#define ATTR_DEFAULT_HVAC_THERMOSTAT_REMOTE_SENSING                       0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER                     0x04
#define ATTR_DEFAULT_HVAC_THERMOSTAT_SYSTEM_MODE                          0x01
#define ATTR_DEFAULT_HVAC_THERMOSTAT_ALAM_MASK                            0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_RUNNING_MODE                         0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_START_OF_WEEK                        0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_NUMBER_OF_WEEKLY_TRANSITIONS         0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_NUMBER_OF_DAILY_TRANSITIONS          0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD            0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_DURATION   0xFFFF
#define ATTR_DEFAULT_HVAC_THERMOSTAT_PROGRAMMING_OPERATION_MODE           0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_RUNNING_STATE                        0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_SETPOINT_CHANGE_SOURCE               0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_SETPOINT_CHANGE_AMOUNT               0x8000
#define ATTR_DEFAULT_HVAC_THERMOSTAT_SETPOINT_CHANGE_SOURCE_TIMESTAMP     0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_TYPE                              0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_CAPACITY                          0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_REFRIGERANT_TYPE                  0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_COMPRESSOR_TYPE                   0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_ERROR_CODE                        0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_LOUVER_POSITION                   0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_COIL_TEMPERATURE                  0
#define ATTR_DEFAULT_HVAC_THERMOSTAT_AC_CAPACITY_FORMAT                   0

  /*** Remote Sensing attribute values ***/
#define HVAC_THERMOSTAT_REMOTE_SENSING_LOCAL_TEMPERATURE                 0x0001
#define HVAC_THERMOSTAT_REMOTE_SENSING_OUTDOOR_TEMPERATURE               0x0002
#define HVAC_THERMOSTAT_REMOTE_SENSING_OCCUPANCY                         0x0004

  /*** Control Sequence of Operation attribute values ***/
#define HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER_COOLING_ONLY                    0x00
#define HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER_COOLING_WITH_REHEAT             0x01
#define HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER_HEATING_ONLY                    0x02
#define HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER_HEATING_WITH_REHEAT             0x03
#define HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER_COOLING_HEATING                 0x04
#define HVAC_THERMOSTAT_CTRL_SEQ_OF_OPER_COOLING_HEATING_WITH_REHEAT     0x05

  /*** System Mode attribute values ***/
#define HVAC_THERMOSTAT_SYSTEM_MODE_OFF                                  0x00
#define HVAC_THERMOSTAT_SYSTEM_MODE_AUTO                                 0x01
#define HVAC_THERMOSTAT_SYSTEM_MODE_COOL                                 0x03
#define HVAC_THERMOSTAT_SYSTEM_MODE_HEAT                                 0x04
#define HVAC_THERMOSTAT_SYSTEM_MODE_EMERGENCY_HEATING                    0x05
#define HVAC_THERMOSTAT_SYSTEM_MODE_PRECOOLING                           0x06
#define HVAC_THERMOSTAT_SYSTEM_MODE_FAN_ONLY                             0x07
#define HVAC_THERMOSTAT_SYSTEM_MODE_DRY                                  0x08
#define HVAC_THERMOSTAT_SYSTEM_MODE_SLEEP                                0x09

  /*** Thermostat Running State attribute values ***/
#define HVAC_THERMOSTAT_RUNNING_STATE_HEAT_1ST_STAGE_ON                  0x0001
#define HVAC_THERMOSTAT_RUNNING_STATE_COOL_1ST_STAGE_ON                  0x0002
#define HVAC_THERMOSTAT_RUNNING_STATE_FAN_1ST_STAGE_ON                   0x0004
#define HVAC_THERMOSTAT_RUNNING_STATE_HEAT_2ND_STAGE_ON                  0x0008
#define HVAC_THERMOSTAT_RUNNING_STATE_COOL_2ND_STAGE_ON                  0x0010
#define HVAC_THERMOSTAT_RUNNING_STATE_FAN_2ND_STAGE_ON                   0x0020
#define HVAC_THERMOSTAT_RUNNING_STATE_HEAT_3RD_STAGE_ON                  0x0040
#define HVAC_THERMOSTAT_RUNNING_STATE_COOL_3RD_STAGE_ON                  0x0080
#define HVAC_THERMOSTAT_RUNNING_STATE_FAN_3RD_STAGE_ON                   0x0100

  /*** Alarm Mask attribute values ***/
#define HVAC_THERMOSTAT_INITIALIZATION_FAILURE                           0x01
#define HVAC_THERMOSTAT_HARDWARE_FAILURE                                 0x02
#define HVAC_THERMOSTAT_SELF_CALIBRATION_FAILURE                         0x03

  // Thermostat Running Mode Attribute Values
#define HVAC_THERMOSTAT_RUNNING_MODE_OFF                                 0x00
#define HVAC_THERMOSTAT_RUNNING_MODE_COOL                                0x03
#define HVAC_THERMOSTAT_RUNNING_MODE_HEAT                                0x04

  // Thermostat Start Of Week Values Section
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_SUNDAY                       0x00
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_MONDAY                       0x01
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_TUESDAY                      0x02
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_WEDNESDAY                    0x03
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_THURSDAY                     0x04
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_FRIDAY                       0x05
#define HVAC_THERMOSTAT_START_OF_WEEK_VALUE_SATURDAY                     0x06

  // Thermostat Temperature Setpoint Hold Attribute
#define HVAC_THERMOSTAT_SETPOINT_HOLD_OFF                                0x00
#define HVAC_THERMOSTAT_SETPOINT_HOLD_ON                                 0x01

  // Thermostat Setpoint Change Source Values
#define HVAC_THERMOSTAT_SETPOINT_MANUAL                                  0x00 // User-initiated setpoint change via Thermostat
#define HVAC_THERMOSTAT_SETPOINT_SCHEDULE                                0x01 // Schedule/Internal programming-initiated setpoint change
#define HVAC_THERMOSTAT_SETPOINT_EXTERNAL                                0x02 // Externally-initiated setpoint change

  // Thermostat AC Type Enum
#define HVAC_THERMOSTAT_AC_TYPE_COOLING_AND_FIXED_SPEED                  0x01
#define HVAC_THERMOSTAT_AC_TYPE_HEAT_PUMP_AND_FIXED_SPEED                0x02
#define HVAC_THERMOSTAT_AC_TYPE_COOLING_AND_INVERTER                     0x03
#define HVAC_THERMOSTAT_AC_TYPE_HEAT_PUMP_AND_INVERTER                   0x04

  // Thermostat AC Refrigerant Type enum
#define HVAC_THERMOSTAT_REFRIGERANT_R22                                  0x01
#define HVAC_THERMOSTAT_REFRIGERANT_R410A                                0x02
#define HVAC_THERMOSTAT_REFRIGERANT_R407C                                0x03

  // Thermostat AC Compressor Type enum
#define HVAC_THERMOSTAT_COMPRESSOR_T1                                    0x01  // Max working ambient 43 degrees Celsius
#define HVAC_THERMOSTAT_COMPRESSOR_T2                                    0x02  // Max working ambient 35 degrees Celsius
#define HVAC_THERMOSTAT_COMPRESSOR_T3                                    0x03  // Max working ambient 52 degrees Celsius

  // Thermostat AC Louver Position
#define HVAC_THERMOSTAT_AC_LOUVER_FULLY_CLOSED                           0x01
#define HVAC_THERMOSTAT_AC_LOUVER_FULLY_OPEN                             0x02
#define HVAC_THERMOSTAT_AC_LOUVER_QUARTER_OPEN                           0x03
#define HVAC_THERMOSTAT_AC_LOUVER_HALF_OPEN                              0x04
#define HVAC_THERMOSTAT_AC_LOUVER_THREE_QUARTERS_OPEN                    0x05

  // Thermostat AC Capacity Format
#define HVAC_THERMOSTAT_AC_CAPACITY_BTUH                                 0x00

  // command IDs received by Server
#define COMMAND_THERMOSTAT_SETPOINT_RAISE_LOWER                          0x00 // zclCmdThermostatSetpointRaiseLowerPayload_t
#define COMMAND_THERMOSTAT_SET_WEEKLY_SCHEDULE                           0x01 // zclThermostatWeeklySchedule_t
#define COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE                           0x02 // zclThermostatGetWeeklySchedule_t
#define COMMAND_THERMOSTAT_CLEAR_WEEKLY_SCHEDULE                         0x03 // no payload
#define COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG                          0x04 // no payload

  // Mode field values for the setpoint raise/lower command
#define HVAC_THERMOSTAT_MODE_HEAT                                        0x00
#define HVAC_THERMOSTAT_MODE_COOL                                        0x01
#define HVAC_THERMOSTAT_MODE_BOTH                                        0x02

  // command IDs generated by Server
#define COMMAND_THERMOSTAT_GET_WEEKLY_SCHEDULE_RSP                       0x00 // zclThermostatWeeklySchedule_t
#define COMMAND_THERMOSTAT_GET_RELAY_STATUS_LOG_RSP                      0x01 // zclThermostatGetRelayStatusLogRsp_t

/**************************************************************************/
/***          HVAC: Fan Control Cluster Attributes                      ***/
/**************************************************************************/
  // Fan Control attribute set
#define ATTRID_HVAC_FAN_CTRL_FAN_MODE                                    0x0000 // M, R/W, ENUM8
#define ATTRID_HVAC_FAN_CTRL_FAN_SEQUENCE                                0x0001 // M, R/W, ENUM8

  /*** Fan Mode attribute values ***/
#define HVAC_FAN_CTRL_FAN_MODE_OFF                                       0x00
#define HVAC_FAN_CTRL_FAN_MODE_LOW                                       0x01
#define HVAC_FAN_CTRL_FAN_MODE_MEDIUM                                    0x02
#define HVAC_FAN_CTRL_FAN_MODE_HIGH                                      0x03
#define HVAC_FAN_CTRL_FAN_MODE_ON                                        0x04
#define HVAC_FAN_CTRL_FAN_MODE_AUTO                                      0x05
#define HVAC_FAN_CTRL_FAN_MODE_SMART                                     0x06

  /*** Fan Mode Sequence attribute values ***/
#define HVAC_FAN_CTRL_FAN_MODE_SEQ_LOW_MED_HI                            0x00
#define HVAC_FAN_CTRL_FAN_MODE_SEQ_LOW_HI                                0x01
#define HVAC_FAN_CTRL_FAN_MODE_SEQ_LOW_MED_HI_AUTO                       0x02
#define HVAC_FAN_CTRL_FAN_MODE_SEQ_LOW_HI_AUTO                           0x03
#define HVAC_FAN_CTRL_FAN_MODE_SEQ_ON_AUTO                               0x04

/**************************************************************************/
/***          HVAC: Dehumidifcation Control Cluster Attributes          ***/
/**************************************************************************/
  // Dehumidifcation Control Information attribute set
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY              0x0000
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_DEHUMID_COOLING                0x0001

  // Dehumidifcation Control Settings attribute set
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_RHDEHUMID_SETPOINT             0x0010
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY_MODE         0x0011
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_DEHUMID_LOCKOUT                0x0012
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_DEHUMID_HYSTERESIS             0x0013
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_DEHUMID_MAX_COOL               0x0014
#define ATTRID_HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY_DISPLAY      0x0015

   /*** Relative Humidity Mode attribute values ***/
#define HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY_MEASURED_LOCALLY    0x00
#define HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY_UPDATED_OVER_NET    0x00

   /*** Dehumidification Lockout attribute values ***/
#define HVAC_DEHUMIDIFICATION_CTRL_DEHIMID_NOT_ALLOWED                   0x00
#define HVAC_DEHUMIDIFICATION_CTRL_DEHIMID_ALLOWED                       0x01

     /*** Relative Humidity Display attribute values ***/
#define HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY_NOT_DISPLAYED       0x00
#define HVAC_DEHUMIDIFICATION_CTRL_RELATIVE_HUMIDITY_DISPLAYED           0x01

/**************************************************************************/
/***          HVAC: Thermostat User Interface Config Cluster Attributes ***/
/**************************************************************************/
  // Thermostat User Interface Config attribute set
#define ATTRID_HVAC_THERMOSTAT_UI_CONFIG_TEMP_DISPLAY_MODE               0x0000 // M, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_UI_CONFIG_KEYPAD_LOCKOUT                  0x0001 // M, R/W, ENUM8
#define ATTRID_HVAC_THERMOSTAT_UI_SCHEDULE_PROGRAMMING_VISIBILITY        0x0002 // O, R/W, ENUM8

  // Thermostat UI Attribute Defaults
#define ATTR_DEFAULT_HVAC_THERMOSTAT_UI_SCHEDULE_PROGRAMMING_VISIBILITY  0

  /*** Display Mode attribute values ***/
#define ATTRID_HVAC_THERMOSTAT_UI_CONFIG_DISPLAY_MODE_IN_C               0x00
#define ATTRID_HVAC_THERMOSTAT_UI_CONFIG_DISPLAY_MODE_IN_F               0x01

  /*** Keypad Lockout attribute values ***/
#define ATTRID_HVAC_THERMOSTAT_UI_KEYPAD_NO_LOCOUT                       0x00
#define ATTRID_HVAC_THERMOSTAT_UI_KEYPAD_LEVEL1_LOCOUT                   0x01
#define ATTRID_HVAC_THERMOSTAT_UI_KEYPAD_LEVEL2_LOCOUT                   0x02
#define ATTRID_HVAC_THERMOSTAT_UI_KEYPAD_LEVEL3_LOCOUT                   0x03
#define ATTRID_HVAC_THERMOSTAT_UI_KEYPAD_LEVEL4_LOCOUT                   0x04
#define ATTRID_HVAC_THERMOSTAT_UI_KEYPAD_LEVEL5_LOCOUT                   0x05

  // Thermostat Schedule Programming Visibility Values
#define HVAC_THERMOSTAT_UI_LOCAL_SCHEDULE_PROGRAMMING_ENABLE             0x00 // Local schedule programming functionality is enabled
#define HVAC_THERMOSTAT_UI_LOCAL_SCHEDULE_PROGRAMMING_DISABLED           0x01 // Local schedule programming functionality is disabled

/*********************************************************************
 * VARIABLES
 */


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

  /*** ZCL HVAC Thermostat Cluster: Setpoint Raise/Lower Cmd payload ***/
typedef struct
{
  uint8 mode;  // which setpoint is to be configured
  int8 amount; // amount setpoint(s) are to be increased (or decreased) by, in steps of 0.1°C
} zclCmdThermostatSetpointRaiseLowerPayload_t;

#define PAYLOAD_LEN_SETPOINT_RAISE_LOWER    2

  /*** ZCL HVAC Thermostat Cluster: Set Weekly Schedule & Get Weekly Schedule Response Cmd payloads ***/
// Heat Mode
typedef struct
{
  uint16 transitionTime;
  uint16 heatSetPoint;
} zclThermostatModeHeat_t;

// Cool Mode
typedef struct
{
  uint16 transitionTime;
  uint16 coolSetPoint;
} zclThermostatModeCool_t;

#define PAYLOAD_LEN_WEEKLY_SCHEDULE_COOL_HEAT_MODE   4

// Both Heat and Cool Modes
typedef struct
{
  uint16 transitionTime;
  uint16 heatSetPoint;
  uint16 coolSetPoint;
} zclThermostatModeBoth_t;

#define PAYLOAD_LEN_WEEKLY_SCHEDULE_BOTH_MODES    6

// mode selection and length based off numberOfTransitionsForSequence and modeForSequence
typedef union
{
  zclThermostatModeHeat_t   *psThermostatModeHeat;
  zclThermostatModeCool_t   *psThermostatModeCool;
  zclThermostatModeBoth_t   *psThermostatModeBoth;
} zclThermostateSequenceMode_t;

typedef struct
{
  uint8 numberOfTransitionsForSequence;   // maximum of 10 transitions per send
  uint8 dayOfWeekForSequence;
  uint8 modeForSequence;    // e.g. HVAC_THERMOSTAT_MODE_HEAT
  zclThermostateSequenceMode_t  sThermostateSequenceMode;
} zclThermostatWeeklySchedule_t;

#define PAYLOAD_LEN_WEEKLY_SCHEDULE   3   // not including attached structures

  /*** ZCL HVAC Thermostat Cluster: Get Weekly Schedule Cmd paylod ***/
typedef struct
{
  uint8 daysToReturn;
  uint8 modeToReturn;
} zclThermostatGetWeeklySchedule_t;

#define PAYLOAD_LEN_GET_WEEKLY_SCHEDULE   2

/*** ZCL HVAC Thermostat Cluster: Get Relay Status Log Response Cmd payload ***/
typedef struct
{
  uint16 timeOfDay;
  uint16 relayStatus;
  uint16 localTemperature;
  uint8 humidity;
  uint16 setPoint;
  uint16 unreadEntries;
} zclThermostatGetRelayStatusLogRsp_t;

#define PAYLOAD_LEN_GET_RELAY_STATUS_LOG_RSP    11

// This callback is called to process a Setpoint Raise/Lower command
typedef ZStatus_t (*zclHVAC_SetpointRaiseLower_t)( zclCmdThermostatSetpointRaiseLowerPayload_t *pCmd );

// This callback is called to process a Set Weekly Schedule command
typedef ZStatus_t (*zclHVAC_SetWeeklySchedule_t)( zclThermostatWeeklySchedule_t *pCmd );

// This callback is called to process a Get Weekly Schedule command
typedef ZStatus_t (*zclHVAC_GetWeeklySchedule_t)( zclThermostatGetWeeklySchedule_t *pCmd );

// This callback is called to process a Clear Weekly Schedule command
typedef ZStatus_t (*zclHVAC_ClearWeeklySchedule_t)( void );

// This callback is called to process a Get Relay Status Log command
typedef ZStatus_t (*zclHVAC_GetRelayStatusLog_t)( void );

// This callback is called to process a Get Weekly Schedule Response command
typedef ZStatus_t (*zclHVAC_GetWeeklyScheduleRsp_t)( zclThermostatWeeklySchedule_t *pCmd );

// This callback is called to process a Get Relay Status Log Response command
typedef ZStatus_t (*zclHVAC_GetRelayStatusLogRsp_t)( zclThermostatGetRelayStatusLogRsp_t *pCmd );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclHVAC_SetpointRaiseLower_t      pfnHVAC_SetpointRaiseLower;
  zclHVAC_SetWeeklySchedule_t       pfnHVAC_SetWeeklySchedule;
  zclHVAC_GetWeeklySchedule_t       pfnHVAC_GetWeeklySchedule;
  zclHVAC_ClearWeeklySchedule_t     pfnHVAC_ClearWeeklySchedule;
  zclHVAC_GetRelayStatusLog_t       pfnHVAC_GetRelayStatusLog;
  zclHVAC_GetWeeklyScheduleRsp_t    pfnHVAC_GetWeeklyScheduleRsp;
  zclHVAC_GetRelayStatusLogRsp_t    pfnHVAC_GetRelayStatusLogRsp;
} zclHVAC_AppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclHVAC_RegisterCmdCallbacks( uint8 endpoint, zclHVAC_AppCallbacks_t *callbacks );

/*
 * Send a Setpoint Raise/Lower Command
 */
ZStatus_t zclHVAC_SendSetpointRaiseLower( uint8 srcEP, afAddrType_t *dstAddr,
                                          uint8 mode, int8 amount,
                                          uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send a Set Weekly Schedule Command
 */
extern ZStatus_t zclHVAC_SendSetWeeklySchedule( uint8 srcEP, afAddrType_t *dstAddr,
                                                zclThermostatWeeklySchedule_t *pPayload,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send a Get Weekly Schedule Command
 */
extern ZStatus_t zclHVAC_SendGetWeeklySchedule( uint8 srcEP, afAddrType_t *dstAddr,
                                                uint8 daysToReturn, uint8 modeToReturn,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send a Clear Weekly Schedule Command
 */
extern ZStatus_t zclHVAC_SendClearWeeklySchedule( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send a Get Relay Status Log Command
 */
extern ZStatus_t zclHVAC_SendGetRelayStatusLog( uint8 srcEP, afAddrType_t *dstAddr,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send a Get Weekly Schedule Response Command
 */
extern ZStatus_t zclHVAC_SendGetWeeklyScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                   zclThermostatWeeklySchedule_t *pPayload,
                                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send a Get Relay Status Log Response Command
 */
extern ZStatus_t zclHVAC_SendGetRelayStatusLogRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                   zclThermostatGetRelayStatusLogRsp_t *pPayload,
                                                   uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif //ZCL_HVAC_CLUSTER

#endif /* ZCL_HVAC_H */
