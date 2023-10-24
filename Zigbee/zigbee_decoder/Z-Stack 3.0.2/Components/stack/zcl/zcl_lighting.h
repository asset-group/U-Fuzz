/**************************************************************************************************
  Filename:       zcl_lighting.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Lighting library definitions.


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

#ifndef ZCL_LIGHTING_H
#define ZCL_LIGHTING_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"

/******************************************************************************
 * CONSTANTS
 */

/*****************************************/
/***  Color Control Cluster Attributes ***/
/*****************************************/
  // Color Information attributes set
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_HUE                        0x0000
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_SATURATION                 0x0001
#define ATTRID_LIGHTING_COLOR_CONTROL_REMAINING_TIME                     0x0002
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_X                          0x0003
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_Y                          0x0004
#define ATTRID_LIGHTING_COLOR_CONTROL_DRIFT_COMPENSATION                 0x0005
#define ATTRID_LIGHTING_COLOR_CONTROL_COMPENSATION_TEXT                  0x0006
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE                  0x0007
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_MODE                         0x0008

  // Defined Primaries Inofrmation attribute Set
#define ATTRID_LIGHTING_COLOR_CONTROL_NUM_PRIMARIES                      0x0010
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_1_X                        0x0011
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_1_Y                        0x0012
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_1_INTENSITY                0x0013
  // 0x0014 is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_2_X                        0x0015
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_2_Y                        0x0016
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_2_INTENSITY                0x0017
  // 0x0018 is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_3_X                        0x0019
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_3_Y                        0x001a
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_3_INTENSITY                0x001b

  // Additional Defined Primaries Information attribute set
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_4_X                        0x0020
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_4_Y                        0x0021
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_4_INTENSITY                0x0022
  // 0x0023 is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_5_X                        0x0024
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_5_Y                        0x0025
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_5_INTENSITY                0x0026
  // 0x0027 is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_6_X                        0x0028
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_6_Y                        0x0029
#define ATTRID_LIGHTING_COLOR_CONTROL_PRIMARY_6_INTENSITY                0x002a

  // Defined Color Points Settings attribute set
#define ATTRID_LIGHTING_COLOR_CONTROL_WHITE_POINT_X                      0x0030
#define ATTRID_LIGHTING_COLOR_CONTROL_WHITE_POINT_Y                      0x0031
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_R_X                    0x0032
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_R_Y                    0x0033
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_R_INTENSITY            0x0034
  // 0x0035 is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_G_X                    0x0036
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_G_Y                    0x0037
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_B_INTENSITY            0x0038
  // 0x0039 is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_B_X                    0x003a
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_B_Y                    0x003b
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_POINT_G_INTENSITY            0x003c
  // 0x003d is reserved
#define ATTRID_LIGHTING_COLOR_CONTROL_ENHANCED_CURRENT_HUE               0x4000
#define ATTRID_LIGHTING_COLOR_CONTROL_ENHANCED_COLOR_MODE                0x4001
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_LOOP_ACTIVE                  0x4002
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_LOOP_DIRECTION               0x4003
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_LOOP_TIME                    0x4004
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_LOOP_START_ENHANCED_HUE      0x4005
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_LOOP_STORED_ENHANCED_HUE     0x4006
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_CAPABILITIES                 0x400a

 /***  Color Information attributes range limits   ***/
#define LIGHTING_COLOR_HUE_MAX                                           0xfe
#define LIGHTING_COLOR_SAT_MAX                                           0xfe
#define LIGHTING_COLOR_REMAINING_TIME_MAX                                0xfffe
#define LIGHTING_COLOR_CURRENT_X_MAX                                     0xfeff
#define LIGHTING_COLOR_CURRENT_Y_MAX                                     0xfeff
#define LIGHTING_COLOR_TEMPERATURE_MAX                                   0xfeff

  /*** Drift Compensation Attribute values ***/
#define DRIFT_COMP_NONE                                                  0x00
#define DRIFT_COMP_OTHER_UNKNOWN                                         0x01
#define DRIFT_COMP_TEMPERATURE_MONITOR                                   0x02
#define DRIFT_COMP_OPTICAL_LUMINANCE_MONITOR_FEEDBACK                    0x03
#define DRIFT_COMP_OPTICAL_COLOR_MONITOR_FEEDBACK                        0x04

  /*** Color Mode Attribute values ***/
#define COLOR_MODE_CURRENT_HUE_SATURATION                                0x00
#define COLOR_MODE_CURRENT_X_Y                                           0x01
#define COLOR_MODE_COLOR_TEMPERATURE                                     0x02

  /*** Enhanced Color Mode Attribute values ***/
#define ENHANCED_COLOR_MODE_CURRENT_HUE_SATURATION                       0x00
#define ENHANCED_COLOR_MODE_CURRENT_X_Y                                  0x01
#define ENHANCED_COLOR_MODE_COLOR_TEMPERATURE                            0x02
#define ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_SATURATION              0x03

  /*** Color Capabilities Attribute bit masks ***/
#define COLOR_CAPABILITIES_ATTR_BIT_HUE_SATURATION                       0x01
#define COLOR_CAPABILITIES_ATTR_BIT_ENHANCED_HUE                         0x02
#define COLOR_CAPABILITIES_ATTR_BIT_COLOR_LOOP                           0x04
#define COLOR_CAPABILITIES_ATTR_BIT_X_Y_ATTRIBUTES                       0x08
#define COLOR_CAPABILITIES_ATTR_BIT_COLOR_TEMPERATURE                    0x10

/*****************************************/
/***  Color Control Cluster Commands   ***/
/*****************************************/
#define COMMAND_LIGHTING_MOVE_TO_HUE                                     0x00
#define COMMAND_LIGHTING_MOVE_HUE                                        0x01
#define COMMAND_LIGHTING_STEP_HUE                                        0x02
#define COMMAND_LIGHTING_MOVE_TO_SATURATION                              0x03
#define COMMAND_LIGHTING_MOVE_SATURATION                                 0x04
#define COMMAND_LIGHTING_STEP_SATURATION                                 0x05
#define COMMAND_LIGHTING_MOVE_TO_HUE_AND_SATURATION                      0x06
#define COMMAND_LIGHTING_MOVE_TO_COLOR                                   0x07
#define COMMAND_LIGHTING_MOVE_COLOR                                      0x08
#define COMMAND_LIGHTING_STEP_COLOR                                      0x09
#define COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE                       0x0a
#define COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE                            0x40
#define COMMAND_LIGHTING_ENHANCED_MOVE_HUE                               0x41
#define COMMAND_LIGHTING_ENHANCED_STEP_HUE                               0x42
#define COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE_AND_SATURATION             0x43
#define COMMAND_LIGHTING_COLOR_LOOP_SET                                  0x44
#define COMMAND_LIGHTING_STOP_MOVE_STEP                                  0x47

  /***  Move To Hue Cmd payload: direction field values  ***/
#define LIGHTING_MOVE_TO_HUE_DIRECTION_SHORTEST_DISTANCE                 0x00
#define LIGHTING_MOVE_TO_HUE_DIRECTION_LONGEST_DISTANCE                  0x01
#define LIGHTING_MOVE_TO_HUE_DIRECTION_UP                                0x02
#define LIGHTING_MOVE_TO_HUE_DIRECTION_DOWN                              0x03
  /***  Move Hue Cmd payload: moveMode field values   ***/
#define LIGHTING_MOVE_HUE_STOP                                           0x00
#define LIGHTING_MOVE_HUE_UP                                             0x01
#define LIGHTING_MOVE_HUE_DOWN                                           0x03
  /***  Step Hue Cmd payload: stepMode field values ***/
#define LIGHTING_STEP_HUE_UP                                             0x01
#define LIGHTING_STEP_HUE_DOWN                                           0x03
  /***  Move Saturation Cmd payload: moveMode field values ***/
#define LIGHTING_MOVE_SATURATION_STOP                                    0x00
#define LIGHTING_MOVE_SATURATION_UP                                      0x01
#define LIGHTING_MOVE_SATURATION_DOWN                                    0x03
  /***  Step Saturation Cmd payload: stepMode field values ***/
#define LIGHTING_STEP_SATURATION_UP                                      0x01
#define LIGHTING_STEP_SATURATION_DOWN                                    0x03
  /***  Color Loop Set Cmd payload: action field values  ***/
#define LIGHTING_COLOR_LOOP_ACTION_DEACTIVATE                            0x00
#define LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_START_HUE               0x01
#define LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_ENH_CURR_HUE            0x02
  /***  Color Loop Set Cmd payload: direction field values   ***/
#define LIGHTING_COLOR_LOOP_DIRECTION_DECREMENT                          0x00
#define LIGHTING_COLOR_LOOP_DIRECTION_INCREMENT                          0x01

/*****************************************************************************/
/***          Ballast Configuration Cluster Attributes                     ***/
/*****************************************************************************/
  // Ballast Information attribute set
#define ATTRID_LIGHTING_BALLAST_CONFIG_PHYSICAL_MIN_LEVEL                0x0000
#define ATTRID_LIGHTING_BALLAST_CONFIG_PHYSICAL_MAX_LEVEL                0x0001
#define ATTRID_LIGHTING_BALLAST_BALLAST_STATUS                           0x0002
/*** Ballast Status Attribute values (by bit number) ***/
#define LIGHTING_BALLAST_STATUS_NON_OPERATIONAL                          1 // bit 0 is set
#define LIGHTING_BALLAST_STATUS_LAMP_IS_NOT_IN_SOCKET                    2 // bit 1 is set
  // Ballast Settings attributes set
#define ATTRID_LIGHTING_BALLAST_MIN_LEVEL                                0x0010
#define ATTRID_LIGHTING_BALLAST_MAX_LEVEL                                0x0011
#define ATTRID_LIGHTING_BALLAST_POWER_ON_LEVEL                           0x0012
#define ATTRID_LIGHTING_BALLAST_POWER_ON_FADE_TIME                       0x0013
#define ATTRID_LIGHTING_BALLAST_INTRISTIC_BALLAST_FACTOR                 0x0014
#define ATTRID_LIGHTING_BALLAST_BALLAST_FACTOR_ADJUSTMENT                0x0015
  // Lamp Information attributes set
#define ATTRID_LIGHTING_BALLAST_LAMP_QUANTITY                            0x0020
  // Lamp Settings attributes set
#define ATTRID_LIGHTING_BALLAST_LAMP_TYPE                                0x0030
#define ATTRID_LIGHTING_BALLAST_LAMP_MANUFACTURER                        0x0031
#define ATTRID_LIGHTING_BALLAST_LAMP_RATED_HOURS                         0x0032
#define ATTRID_LIGHTING_BALLAST_LAMP_BURN_HOURS                          0x0033
#define ATTRID_LIGHTING_BALLAST_LAMP_ALARM_MODE                          0x0034
#define ATTRID_LIGHTING_BALLAST_LAMP_BURN_HOURS_TRIP_POINT               0x0035
/*** Lamp Alarm Mode attribute values  ***/
#define LIGHTING_BALLAST_LAMP_ALARM_MODE_BIT_0_NO_ALARM                  0
#define LIGHTING_BALLAST_LAMP_ALARM_MODE_BIT_0_ALARM                     1

/*******************************************************************************
 * TYPEDEFS
 */

/*** ZCL Color Control Cluster: Move To Hue Cmd payload ***/
typedef struct
{
  uint8  hue;
  uint8  direction;
  uint16 transitionTime;
} zclCmdLightingMoveToHuePayload_t;

/*** ZCL Color Control Cluster: Move Hue Cmd payload ***/
typedef struct
{
  uint8 moveMode;
  uint8 rate;
} zclCmdLightingMoveHuePayload_t;

/*** ZCL Color Control Cluster: Step Hue Cmd payload ***/
typedef struct
{
  uint8 stepMode;
  uint8 transitionTime;
} zclCmdLightingStepHuePayload_t;

/*** ZCL Color Control Cluster: Move to Saturation Cmd payload ***/
typedef struct
{
  uint8  saturation;
  uint16 transitionTime;
} zclCmdLightingMoveToSaturationPayload_t;

/*** ZCL Color Control Cluster: Move Saturation Cmd payload ***/
typedef struct
{
  uint8 moveMode;
  uint8 rate;
} zclCmdLightingMoveSaturationPayload_t;

/*** ZCL Color Control Cluster: Step Saturation Cmd payload ***/
typedef struct
{
  uint8 stepMode;
  uint8 transitionTime;
} zclCmdLightingStepSaturationPayload_t;

/*** ZCL Color Control Cluster: Move To Hue and Saturation Cmd payload ***/
typedef struct
{
  uint8  hue;
  uint8  saturation;
  uint16 transitionTime;
} zclCmdLightingMoveToHueAndSaturationPayload_t;

/*** Structures used for callback functions ***/

typedef struct
{
  uint8  hue;            // target hue value
  uint8  direction;      // direction of change
  uint16 transitionTime; // tame taken to move to the target hue in 1/10 sec increments
} zclCCMoveToHue_t;

typedef struct
{
  uint8 moveMode; // LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP, LIGHTING_MOVE_HUE_DOWN
  uint8 rate;     // the movement in steps per second, where step is a change in the device's hue of one unit
} zclCCMoveHue_t;

typedef struct
{
  uint8 stepMode;       // LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
  uint8 stepSize;       // change to the current value of the device's hue
  uint8 transitionTime; // the movement in steps per 1/10 second
} zclCCStepHue_t;

typedef struct
{
  uint8 saturation;      // target saturation value
  uint16 transitionTime; // time taken move to the target saturation, in 1/10 second units
} zclCCMoveToSaturation_t;

typedef struct
{
  uint8 moveMode; // LIGHTING_MOVE_SATURATION_STOP, LIGHTING_MOVE_SATURATION_UP,
                  // LIGHTING_MOVE_SATURATION_DOWN
  uint8 rate;     // rate of movement in step/sec; step is the device's saturation of one unit
} zclCCMoveSaturation_t;

typedef struct
{
  uint8 stepMode;       // LIGHTING_STEP_SATURATION_UP, LIGHTING_STEP_SATURATION_DOWN
  uint8 stepSize;       // change to the current value of the device's hue
  uint8 transitionTime; // time to perform a single step in 1/10 of second
} zclCCStepSaturation_t;

typedef struct
{
  uint8 hue;             // a target hue
  uint8 saturation;      // a target saturation
  uint16 transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCMoveToHueAndSaturation_t;

typedef struct
{
  uint16 colorX;         // a target color X
  uint16 colorY;         // a target color Y
  uint16 transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCMoveToColor_t;

typedef struct
{
  int16 rateX;          // rate of movement in steps per second. A step is a change
                        // in the device's CurrentX attribute of one unit.
  int16 rateY;          // rate of movement in steps per second. A step is a change
                        // in the device's CurrentYattribute of one unit.
} zclCCMoveColor_t;

typedef struct
{
  int16 stepX;           // change to be added to the device's CurrentX attribute
  int16 stepY;           // change to be added to the device's CurrentY attribute
  uint16 transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCStepColor_t;

typedef struct
{
  uint16 colorTemperature; // a target color temperature
  uint16 transitionTime;   // time to move, equal of the value of the field in 1/10 seconds
} zclCCMoveToColorTemperature_t;

typedef struct
{
  uint16 enhancedHue;    // target enhanced hue value
  uint8 direction;       // direction of change
  uint16 transitionTime; // tame taken to move to the target hue in 1/10 sec increments
} zclCCEnhancedMoveToHue_t;

typedef struct
{
  uint8 moveMode; // LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP, LIGHTING_MOVE_HUE_DOWN
  uint16 rate;    // the movement in steps per second, where step is a change in the device's hue of one unit
} zclCCEnhancedMoveHue_t;

typedef struct
{
  uint8 stepMode;        // LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
  uint16 stepSize;       // change to the current value of the device's hue
  uint16 transitionTime; // the movement in steps per 1/10 second
} zclCCEnhancedStepHue_t;

typedef struct
{
  uint16 enhancedHue;    // a target hue
  uint8 saturation;      // a target saturation
  uint16 transitionTime; // time to move, equal of the value of the field in 1/10 seconds
} zclCCEnhancedMoveToHueAndSaturation_t;

typedef struct
{
  unsigned int action:1;
  unsigned int direction:1;
  unsigned int time:1;
  unsigned int startHue:1;
  unsigned int reserved:4;
} zclCCColorLoopSet_updateFlagsBits_t;

typedef union
{
  zclCCColorLoopSet_updateFlagsBits_t bits;
  uint8 byte;
} zclCCColorLoopSet_updateFlags_t;

typedef struct
{
  zclCCColorLoopSet_updateFlags_t updateFlags;
  uint8 action;     // LIGHTING_COLOR_LOOP_ACTION_DEACTIVATE, LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_START_HUE, LIGHTING_COLOR_LOOP_ACTION_ACTIVATE_FROM_ENH_CURR_HUE
  uint8 direction;  // LIGHTING_COLOR_LOOP_DIRECTION_DECREMENT, LIGHTING_COLOR_LOOP_DIRECTION_INCREMENT
  uint16 time;      // number of seconds on which to perform a full color loop
  uint16 startHue;  // the starting hue to use
} zclCCColorLoopSet_t;

// This callback is called to process a Move To Hue command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToHue_t)( zclCCMoveToHue_t *pCmd );

// This callback is called to process a Move Hue command
typedef ZStatus_t (*zclLighting_ColorControl_MoveHue_t)( zclCCMoveHue_t *pCmd );

// This callback is called to process a Step Hue command
typedef ZStatus_t (*zclLighting_ColorControl_StepHue_t)( zclCCStepHue_t *pCmd );

// This callback is called to process a Move To Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToSaturation_t)( zclCCMoveToSaturation_t *pCmd );

// This callback is called to process a Move Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_MoveSaturation_t)( zclCCMoveSaturation_t *pCmd );

// This callback is called to process a Step Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_StepSaturation_t)( zclCCStepSaturation_t *pCmd );

// This callback is called to process a Move to Hue and Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToHueAndSaturation_t)( zclCCMoveToHueAndSaturation_t *pCmd );

// This callback is called to process a Move to Color command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToColor_t)( zclCCMoveToColor_t *pCmd );

// This callback is called to process a Move Color command
typedef void (*zclLighting_ColorControl_MoveColor_t)( zclCCMoveColor_t *pCmd );

// This callback is called to process a Step Color command
typedef ZStatus_t (*zclLighting_ColorControl_StepColor_t)( zclCCStepColor_t *pCmd );

// This callback is called to process a Move to Color Temperature command
typedef ZStatus_t (*zclLighting_ColorControl_MoveToColorTemperature_t)( zclCCMoveToColorTemperature_t *pCmd );

// This callback is called to process an Enhanced Move To Hue command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedMoveToHue_t)( zclCCEnhancedMoveToHue_t *pCmd );

// This callback is called to process an Enhanced Move Hue command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedMoveHue_t)( zclCCEnhancedMoveHue_t *pCmd );

// This callback is called to process an Enhanced Step Hue command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedStepHue_t)( zclCCEnhancedStepHue_t *pCmd );

// This callback is called to process an Enhanced Move to Hue and Saturation command
typedef ZStatus_t (*zclLighting_ColorControl_EnhancedMoveToHueAndSaturation_t)( zclCCEnhancedMoveToHueAndSaturation_t *pCmd );

// This callback is called to process an Color Loop Set command
typedef ZStatus_t (*zclLighting_ColorControl_ColorLoopSet_t)( zclCCColorLoopSet_t *pCmd );

// This callback is called to process a Stop Step Move command
typedef ZStatus_t (*zclLighting_ColorControl_StopMoveStep_t)( void );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclLighting_ColorControl_MoveToHue_t                pfnColorControl_MoveToHue;
  zclLighting_ColorControl_MoveHue_t                  pfnColorControl_MoveHue;
  zclLighting_ColorControl_StepHue_t                  pfnColorControl_StepHue;
  zclLighting_ColorControl_MoveToSaturation_t         pfnColorControl_MoveToSaturation;
  zclLighting_ColorControl_MoveSaturation_t           pfnColorControl_MoveSaturation;
  zclLighting_ColorControl_StepSaturation_t           pfnColorControl_StepSaturation;
  zclLighting_ColorControl_MoveToHueAndSaturation_t   pfnColorControl_MoveToHueAndSaturation;
  zclLighting_ColorControl_MoveToColor_t              pfnColorControl_MoveToColor;
  zclLighting_ColorControl_MoveColor_t                pfnColorControl_MoveColor;
  zclLighting_ColorControl_StepColor_t                pfnColorControl_StepColor;
  zclLighting_ColorControl_MoveToColorTemperature_t   pfnColorControl_MoveToColorTemperature;
  zclLighting_ColorControl_EnhancedMoveToHue_t        pfnColorControl_EnhancedMoveToHue;
  zclLighting_ColorControl_EnhancedMoveHue_t          pfnColorControl_EnhancedMoveHue;
  zclLighting_ColorControl_EnhancedStepHue_t          pfnColorControl_EnhancedStepHue;
  zclLighting_ColorControl_EnhancedMoveToHueAndSaturation_t pfnColorControl_EnhancedMoveToHueAndSaturation;
  zclLighting_ColorControl_ColorLoopSet_t             pfnColorControl_ColorLoopSet;
  zclLighting_ColorControl_StopMoveStep_t             pfnColorControl_StopMoveStep;
} zclLighting_AppCallbacks_t;


/******************************************************************************
 * FUNCTION MACROS
 */
#ifdef ZCL_LIGHT_LINK_ENHANCE
/*
 *  Send out anStop Move Step Command - COMMAND_LIGHTING_STOP_MOVE_STEP
 *  Use like:
 *      ZStatus_t zclLighting_ColorControl_Send_StopMoveStepCmd( uint16 srcEP, afAddrType_t *dstAddr, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclLighting_ColorControl_Send_StopMoveStepCmd(a,b,c,d) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL, COMMAND_LIGHTING_STOP_MOVE_STEP, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, (c), 0, (d), 0, NULL )
#endif //ZCL_LIGHT_LINK_ENHANCE

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclLighting_RegisterCmdCallbacks( uint8 endpoint, zclLighting_AppCallbacks_t *callbacks );


/*
 * Call to send out a Move To Hue Command
 *      hue - target hue value
 *      direction - direction of hue change
 *      transitionTime - tame taken to move to the target hue in 1/10 sec increments
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                              uint8 hue, uint8 direction, uint16 transitionTime,
                                              uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move Hue Command
 *      moveMode - LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP, LIGHTING_MOVE_HUE_DOWN
 *      rate - the movement in steps per second (step is a change in the device's hue
 *             of one unit)
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                           uint8 moveMode, uint8 rate,
                                                           uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Step Hue Command
 *      stepMode - LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
 *      amount - number of hue units to step
 *      transitionTime - the movement in steps per 1/10 second
 */
extern ZStatus_t zclLighting_ColorControl_Send_StepHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                          uint8 stepMode, uint8 stepSize, uint8 transitionTime,
                                          uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move To Saturation Command
 *      saturation - target saturation value
 *      transitionTime - time taken move to the target saturation, in 1/10 second units
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                uint8 saturation, uint16 transitionTime,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move Saturation Command
 *      moveMode - LIGHTING_MOVE_SATURATION_STOP, LIGHTING_MOVE_SATURATION_UP,
 *                 LIGHTING_MOVE_SATURATION_DOWN
 *      rate -  rate of movement in step per second; step is the device's
 *              saturation of one unit
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint8 moveMode, uint8 rate,
                                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Step Saturation Command
 *      stepMode -  LIGHTING_STEP_SATURATION_UP, LIGHTING_STEP_SATURATION_DOWN
 *      amount -  number of units to change the saturation level by
 *      transitionTime - time to perform a single step in 1/10 of second
 */
extern ZStatus_t zclLighting_ColorControl_Send_StepSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                uint8 stepMode, uint8 stepSize, uint8 transitionTime,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move To Hue And Saturation  Command
 *      hue - 	target hue
 *      saturation -  target saturation
 *      transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToHueAndSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint8 hue, uint8 saturation, uint16 transitionTime,
                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move To Color Command
 *      colorX -  target color X
 *      colorY -  target color Y
 *      transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToColorCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                              uint16 colorX, uint16 colorY, uint16 transitionTime,
                                              uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move Color Command
 *      rateX -  rate of movement in steps per second. A step is a change
 *               in the device's CurrentX attribute of one unit.
 *      rateY -  rate of movement in steps per second. A step is a change
 *               in the device's CurrentY attribute of one unit.
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveColorCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                             int16 rateX, int16 rateY,
                                                             uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Step Color Command
 *      stepX -  change to be added to the device's CurrentX attribute
 *      stepY -  change to be added to the device's CurrentY attribute
 *      transitionTime -  time to perform the color change, equal of
 *                        the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_StepColorCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                int16 stepX, int16 stepY, uint16 transitionTime,
                                                uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Move To Color Temperature Command
 *      colorTemperature - a target color temperature
 *      transitionTime -  time to perform the color change, equal of
 *                        the value of the field in 1/10 seconds
 */
extern ZStatus_t zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                              uint16 colorTemperature, uint16 transitionTime,
                                                              uint8 disableDefaultRsp, uint8 seqNum );
#ifdef ZCL_LIGHT_LINK_ENHANCE
/*
 * Call to send out an Enhanced Move To Hue Command
 *      enhancedHue - a target extended hue for lamp
 *      direction - direction of hue change
 *      transitionTime -  time to perform the color change, equal of
 *                        the value of the field in 1/10 seconds
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveToHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                              uint16 enhancedHue, uint8 direction,
                                                              uint16 transitionTime, uint8 disableDefaultRsp,
                                                              uint8 seqNum );
/*
 * Call to send out an Enhanced Move Hue Command
 *      moveMode - LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP,
 *                 LIGHTING_MOVE_HUE_DOWN
 *      rate - the movement in steps per second, where step is
 *             a change in the device's hue of one unit
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 moveMode, uint16 rate,
                                                            uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send out an Enhanced Step Hue Command
 *      stepMode -	LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
 *      stepSize -  change to the current value of the device's hue
 *      transitionTime - the movement in steps per 1/10 second
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedStepHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                  uint8 stepMode, uint16 stepSize, uint16 transitionTime,
                                  uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Call to send out an Enhanced Move To Hue And Saturation Command
 *      Enhanced hue - a target Enhanced hue for lamp
 *      saturation - a target saturation
 *      transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveToHueAndSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint16 enhancedHue, uint8 saturation, uint16 transitionTime,
                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Color Loop Set Command
 *      updateFlag - which color loop attributes to update before the color loop is started
 *      action - action to take for the color loop
 *      direction - direction for the color loop (decrement or increment)
 *      time - number of seconds over which to perform a full color loop
 *      startHue - starting hue to use for the color loop
 */
ZStatus_t zclLighting_ColorControl_Send_ColorLoopSetCmd(uint8 srcEP, afAddrType_t *dstAddr,
                                                  zclCCColorLoopSet_updateFlags_t updateFlags, uint8 action,
                                                  uint8 direction, uint16 time, uint16 startHue,
                                                  uint8 disableDefaultRsp, uint8 seqNum);
#endif //ZCL_LIGHT_LINK_ENHANCE


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_LIGHTING_H */
