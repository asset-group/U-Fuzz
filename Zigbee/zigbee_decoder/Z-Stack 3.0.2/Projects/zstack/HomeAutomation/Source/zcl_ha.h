/**************************************************************************************************
  Filename:       zcl_ha.h
  Revised:        $Date: 2014-06-23 15:23:54 -0700 (Mon, 23 Jun 2014) $
  Revision:       $Revision: 39166 $

  Description:    This file contains the Zigbee Cluster Library: Home
                  Automation Profile definitions.


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

#ifndef ZCL_HA_H
#define ZCL_HA_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
// Zigbee Home Automation Profile Identification
#define ZCL_HA_PROFILE_ID                                       0x0104

// Generic Device IDs
#define ZCL_HA_DEVICEID_ON_OFF_SWITCH                           0x0000
#define ZCL_HA_DEVICEID_LEVEL_CONTROL_SWITCH                    0x0001
#define ZCL_HA_DEVICEID_ON_OFF_OUTPUT                           0x0002
#define ZCL_HA_DEVICEID_LEVEL_CONTROLLABLE_OUTPUT               0x0003
#define ZCL_HA_DEVICEID_SCENE_SELECTOR                          0x0004
#define ZCL_HA_DEVICEID_CONFIGURATION_TOOL                      0x0005
#define ZCL_HA_DEVICEID_REMOTE_CONTROL                          0x0006
#define ZCL_HA_DEVICEID_COMBINED_INTERFACE                      0x0007
#define ZCL_HA_DEVICEID_RANGE_EXTENDER                          0x0008
#define ZCL_HA_DEVICEID_MAINS_POWER_OUTLET                      0x0009
#define ZCL_HA_DEVICEID_DOOR_LOCK                               0x000A
#define ZCL_HA_DEVICEID_DOOR_LOCK_CONTROLLER                    0x000B
#define ZCL_HA_DEVICEID_SIMPLE_SENSOR                           0x000C
#define ZCL_HA_DEVICEID_CONSUMPTION_AWARENESS_DEVICE            0x000D
#define ZCL_HA_DEVICEID_HOME_GATEWAY                            0x0050
#define ZCL_HA_DEVICEID_SMART_PLUG                              0x0051
#define ZCL_HA_DEVICEID_WHITE_GOODS                             0x0052
#define ZCL_HA_DEVICEID_METER_INTERFACE                         0x0053

// This is a reserved value which could be used for test purposes
#define ZCL_HA_DEVICEID_TEST_DEVICE                             0x00FF

// Lighting Device IDs
#define ZCL_HA_DEVICEID_ON_OFF_LIGHT                            0x0100
#define ZCL_HA_DEVICEID_DIMMABLE_LIGHT                          0x0101
#define ZCL_HA_DEVICEID_COLORED_DIMMABLE_LIGHT                  0x0102
#define ZCL_HA_DEVICEID_ON_OFF_LIGHT_SWITCH                     0x0103
#define ZCL_HA_DEVICEID_DIMMER_SWITCH                           0x0104
#define ZCL_HA_DEVICEID_COLOR_DIMMER_SWITCH                     0x0105
#define ZCL_HA_DEVICEID_LIGHT_SENSOR                            0x0106
#define ZCL_HA_DEVICEID_OCCUPANCY_SENSOR                        0x0107

// Closures Device IDs
#define ZCL_HA_DEVICEID_SHADE                                   0x0200
#define ZCL_HA_DEVICEID_SHADE_CONTROLLER                        0x0201
#define ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE                  0x0202
#define ZCL_HA_DEVICEID_WINDOW_COVERING_CONTROLLER              0x0203

// HVAC Device IDs
#define ZCL_HA_DEVICEID_HEATING_COOLING_UNIT                    0x0300
#define ZCL_HA_DEVICEID_THERMOSTAT                              0x0301
#define ZCL_HA_DEVICEID_TEMPERATURE_SENSOR                      0x0302
#define ZCL_HA_DEVICEID_PUMP                                    0x0303
#define ZCL_HA_DEVICEID_PUMP_CONTROLLER                         0x0304
#define ZCL_HA_DEVICEID_PRESSURE_SENSOR                         0x0305
#define ZCL_HA_DEVICEID_FLOW_SENSOR                             0x0306
#define ZCL_HA_DEVICEID_MINI_SPLIT_AC                           0x0307

// Intruder Alarm Systems (IAS) Device IDs
#define ZCL_HA_DEVICEID_IAS_CONTROL_INDICATING_EQUIPMENT        0x0400
#define ZCL_HA_DEVICEID_IAS_ANCILLARY_CONTROL_EQUIPMENT         0x0401
#define ZCL_HA_DEVICEID_IAS_ZONE                                0x0402
#define ZCL_HA_DEVICEID_IAS_WARNING_DEVICE                      0x0403

// Device type to display in LCD
#define ZCL_HA_DEVICE_COORDINATOR       0
#define ZCL_HA_DEVICE_ROUTER            1
#define ZCL_HA_DEVICE_END_DEVICE        2

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */

extern void zclHA_LcdStatusLine1( uint8 kind );
#define ZCL_HA_STATUSLINE_ZC    0
#define ZCL_HA_STATUSLINE_ZR    1
#define ZCL_HA_STATUSLINE_ZED   2

// convert 16 bits to an ascii hex number
extern void zclHA_uint16toa( uint16 u, char *string );

// convert 8 bits to an ascii decimal number
extern void zclHA_uint8toa(uint8 b, char *string);

// functions for dealing with a bit array
extern bool zclHA_isbit(uint8 *pArray, uint8 bitIndex);
extern void zclHA_setbit(uint8 *pArray, uint8 bitIndex);
extern void zclHA_clearbit(uint8 *pArray, uint8 bitIndex);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_HA_H */
