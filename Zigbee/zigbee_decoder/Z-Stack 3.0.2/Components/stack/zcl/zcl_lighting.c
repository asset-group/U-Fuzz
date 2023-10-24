/**************************************************************************************************
  Filename:       zcl_lighting.c
  Revised:        $Date: 2013-07-03 15:56:37 -0700 (Wed, 03 Jul 2013) $
  Revision:       $Revision: 34689 $

  Description:    Zigbee Cluster Library -  Lighting


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

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_lighting.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclLightingCBRec
{
  struct zclLightingCBRec     *next;
  uint8                       endpoint; // Used to link it into the endpoint descriptor
  zclLighting_AppCallbacks_t  *CBs;     // Pointer to Callback function
} zclLightingCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclLightingCBRec_t *zclLightingCBs = (zclLightingCBRec_t *)NULL;
static uint8 zclLightingPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclLighting_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclLighting_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclLighting_AppCallbacks_t *zclLighting_FindCallbacks( uint8 endpoint );

static ZStatus_t zclLighting_ProcessInColorControlCmds( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );

static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToHue( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveHue( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_StepHue( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToSaturation( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveSaturation( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_StepSaturation( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToHueAndSaturation( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToColor( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveColor( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_StepColor( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToColorTemperature( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
#ifdef ZCL_LIGHT_LINK_ENHANCE
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHue( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedMoveHue( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedStepHue( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHueAndSaturation( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_ColorLoopSet( zclIncoming_t *pInMsg, zclLighting_AppCallbacks_t *pCBs );
#endif // ZCL_LIGHT_LINK_ENHANCE

/*********************************************************************
 * @fn      zclLighting_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclLighting_RegisterCmdCallbacks( uint8 endpoint, zclLighting_AppCallbacks_t *callbacks )
{
  zclLightingCBRec_t *pNewItem;
  zclLightingCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( zclLightingPluginRegisted == FALSE )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                        ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG,
                        zclLighting_HdlIncoming );
    zclLightingPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclLightingCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclLightingCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclLightingCBs == NULL )
  {
    zclLightingCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclLightingCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveToHueCmd
 *
 * @brief   Call to send out a Move To Hue Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   hue - target hue value
 * @param   direction - direction of hue change
 * @param   transitionTime - tame taken to move to the target hue in 1/10 sec increments
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveToHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint8 hue, uint8 direction, uint16 transitionTime,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4];

  buf[0] = hue;
  buf[1] = direction;
  buf[2] = LO_UINT16( transitionTime );
  buf[3] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_TO_HUE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 4, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveHueCmd
 *
 * @brief   Call to send out a Move To Hue Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   moveMode - LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP,
 *                     LIGHTING_MOVE_HUE_DOWN
 * @param   rate - the movement in steps per second, where step is
 *                 a change in the device's hue of one unit
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                    uint8 moveMode, uint8 rate,
                                                    uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[2];

  buf[0] = moveMode;
  buf[1] = rate;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_HUE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 2, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_StepHueCmd
 *
 * @brief   Call to send out a Step Hue Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   stepMode -	LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
 * @param   stepSize -  change to the current value of the device's hue
 * @param   transitionTime - the movement in steps per 1/10 second
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_StepHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                  uint8 stepMode, uint8 stepSize, uint8 transitionTime,
                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[3];

  buf[0] = stepMode;
  buf[1] = stepSize;
  buf[2] = transitionTime;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_STEP_HUE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveToSaturationCmd
 *
 * @brief   Call to send out a Move To Saturation Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   saturation - target saturation value
 * @param   transitionTime - time taken move to the target saturation,
 *                           in 1/10 second units
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveToSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint8 saturation, uint16 transitionTime,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[3];

  buf[0] = saturation;
  buf[1] = LO_UINT16( transitionTime );
  buf[2] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_TO_SATURATION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveSaturationCmd
 *
 * @brief   Call to send out a Move Saturation Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   moveMode - LIGHTING_MOVE_SATURATION_STOP, LIGHTING_MOVE_SATURATION_UP,
 *                     LIGHTING_MOVE_SATURATION_DOWN
 * @param   rate - rate of movement in step/sec; step is the device's saturation of one unit
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                           uint8 moveMode, uint8 rate,
                                                           uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[2];

  buf[0] = moveMode;
  buf[1] = rate;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_SATURATION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 2, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_StepSaturationCmd
 *
 * @brief   Call to send out a Step Saturation Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   stepMode -  LIGHTING_STEP_SATURATION_UP, LIGHTING_STEP_SATURATION_DOWN
 * @param   stepSize -  change to the current value of the device's hue
 * @param   transitionTime - time to perform a single step in 1/10 of second
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_StepSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                         uint8 stepMode, uint8 stepSize, uint8 transitionTime,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[3];

  buf[0] = stepMode;
  buf[1] = stepSize;
  buf[2] = transitionTime;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_STEP_SATURATION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveToHueAndSaturationCmd
 *
 * @brief   Call to send out a Move To Hue And Saturation Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   hue - a target hue
 * @param   saturation - a target saturation
 * @param   transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveToHueAndSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                   uint8 hue, uint8 saturation, uint16 transitionTime,
                                                   uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4];

  buf[0] = hue;
  buf[1] = saturation;
  buf[2] = LO_UINT16( transitionTime );
  buf[3] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_TO_HUE_AND_SATURATION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 4, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveToColorCmd
 *
 * @brief   Call to send out a Move To Color Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   colorX - a target color X
 * @param   colorY - a target color Y
 * @param   transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveToColorCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                       uint16 colorX, uint16 colorY, uint16 transitionTime,
                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[6];

  buf[0] = LO_UINT16( colorX );
  buf[1] = HI_UINT16( colorX );
  buf[2] = LO_UINT16( colorY );
  buf[3] = HI_UINT16( colorY );
  buf[4] = LO_UINT16( transitionTime );
  buf[5] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_TO_COLOR, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 6, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveColorCmd
 *
 * @brief   Call to send out a Move Color Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   rateX - rate of movement in steps per second. A step is a change
 *                  in the device's CurrentX attribute of one unit.
 * @param   rateY - rate of movement in steps per second. A step is a change
 *                  in the device's CurrentY attribute of one unit.
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveColorCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      int16 rateX, int16 rateY,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4];

  buf[0] = LO_UINT16( rateX );
  buf[1] = HI_UINT16( rateX );
  buf[2] = LO_UINT16( rateY );
  buf[3] = HI_UINT16( rateY );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_COLOR, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 4, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_StepColorCmd
 *
 * @brief   Call to send out a Step Color Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   stepX - change to be added to the device's CurrentX attribute
 * @param   stepY - change to be added to the device's CurrentY attribute
 * @param   transitionTime -  time to perform the color change, equal of
 *                            the value of the field in 1/10 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_StepColorCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                         int16 stepX, int16 stepY, uint16 transitionTime,
                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[6];

  buf[0] = LO_UINT16( stepX );
  buf[1] = HI_UINT16( stepX );
  buf[2] = LO_UINT16( stepY );
  buf[3] = HI_UINT16( stepY );
  buf[4] = LO_UINT16( transitionTime );
  buf[5] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_STEP_COLOR, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 6, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_MoveToColorTemperatureCmd
 *
 * @brief   Call to send out a Move To Color Temperature Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   colorTemperature - a target color temperature
 * @param   transitionTime -  time to perform the color change, equal of
 *                            the value of the field in 1/10 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                       uint16 colorTemperature, uint16 transitionTime,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[4];

  buf[0] = LO_UINT16( colorTemperature );
  buf[1] = HI_UINT16( colorTemperature );
  buf[2] = LO_UINT16( transitionTime );
  buf[3] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 4, buf );
}

#ifdef ZCL_LIGHT_LINK_ENHANCE
/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_EnhancedMoveToHueCmd
 *
 * @brief   Call to send out an Enhanced Move To Hue Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   enhancedHue - a target extended hue for lamp
 * @param   direction - direction of hue change
 * @param   transitionTime -  time to perform the color change, equal of
 *                            the value of the field in 1/10 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveToHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                              uint16 enhancedHue, uint8 direction,
                                                              uint16 transitionTime, uint8 disableDefaultRsp,
                                                              uint8 seqNum )
{
  uint8 buf[5];

  buf[0] = LO_UINT16( enhancedHue );
  buf[1] = HI_UINT16( enhancedHue );
  buf[2] = direction;
  buf[3] = LO_UINT16( transitionTime );
  buf[4] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 5, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_EnhancedMoveHueCmd
 *
 * @brief   Call to send out an Enhanced Move Hue Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   moveMode - LIGHTING_MOVE_HUE_STOP, LIGHTING_MOVE_HUE_UP,
 *                     LIGHTING_MOVE_HUE_DOWN
 * @param   rate - the movement in steps per second, where step is
 *                 a change in the device's hue of one unit
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 moveMode, uint16 rate,
                                                            uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[3];

  buf[0] = moveMode;
  buf[1] = LO_UINT16( rate );
  buf[2] = HI_UINT16( rate );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_ENHANCED_MOVE_HUE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 3, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_EnhancedStepHueCmd
 *
 * @brief   Call to send out an Enhanced Step Hue Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   stepMode -	LIGHTING_STEP_HUE_UP, LIGHTING_STEP_HUE_DOWN
 * @param   stepSize -  change to the current value of the device's hue
 * @param   transitionTime - the movement in steps per 1/10 second
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedStepHueCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                  uint8 stepMode, uint16 stepSize, uint16 transitionTime,
                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[5];

  buf[0] = stepMode;
  buf[1] = LO_UINT16( stepSize );
  buf[2] = HI_UINT16( stepSize );
  buf[3] = LO_UINT16( transitionTime );
  buf[4] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_ENHANCED_STEP_HUE, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 5, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_EnhancedMoveToHueAndSaturationCmd
 *
 * @brief   Call to send out an Enhanced Move To Hue And Saturation Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   Enhanced hue - a target Enhanced hue for lamp
 * @param   saturation - a target saturation
 * @param   transitionTime -  time to move, equal of the value of the field in 1/10 seconds
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_EnhancedMoveToHueAndSaturationCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint16 enhancedHue, uint8 saturation, uint16 transitionTime,
                                                  uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[5];

  buf[0] = LO_UINT16( enhancedHue );
  buf[1] = HI_UINT16( enhancedHue );
  buf[2] = saturation;
  buf[3] = LO_UINT16( transitionTime );
  buf[4] = HI_UINT16( transitionTime );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE_AND_SATURATION, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 5, buf );
}

/*********************************************************************
 * @fn      zclLighting_ColorControl_Send_ColorLoopSetCmd
 *
 * @brief   Call to send out a Color Loop Set Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   updateFlags - which color loop attributes to update before the color loop is started.
 * @param   action - action to take for the color loop
 * @param   direction - direction for the color loop (decrement or increment)
 * @param   time - number of seconds over which to perform a full color loop
 * @param   startHue - starting hue to use for the color loop
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t zclLighting_ColorControl_Send_ColorLoopSetCmd(uint8 srcEP, afAddrType_t *dstAddr,
                                                  zclCCColorLoopSet_updateFlags_t updateFlags, uint8 action,
                                                  uint8 direction, uint16 time, uint16 startHue,
                                                  uint8 disableDefaultRsp, uint8 seqNum)
{
  uint8 buf[7];

  buf[0] = updateFlags.byte;
  buf[1] = action;
  buf[2] = direction;
  buf[3] = LO_UINT16( time );
  buf[4] = HI_UINT16( time );
  buf[5] = LO_UINT16( startHue );
  buf[6] = HI_UINT16( startHue );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                          COMMAND_LIGHTING_COLOR_LOOP_SET, TRUE,
                          ZCL_FRAME_CLIENT_SERVER_DIR, disableDefaultRsp, 0, seqNum, 7, buf );
}
#endif //ZCL_LIGHT_LINK_ENHANCE

/*********************************************************************
 * @fn      zclLighting_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint - endpoint to find the application callbacks for
 *
 * @return  pointer to the callbacks
 */
static zclLighting_AppCallbacks_t *zclLighting_FindCallbacks( uint8 endpoint )
{
  zclLightingCBRec_t *pCBs;

  pCBs = zclLightingCBs;
  while ( pCBs != NULL )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }
  return ( (zclLighting_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclLighting_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
    return ( stat ); // Cluster not supported thru Inter-PAN
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclLighting_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    stat = ZFailure;
  }
  return ( stat );
}

/*********************************************************************
 * @fn      zclLighting_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclLighting_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = zclLighting_FindCallbacks( pInMsg->msg->endPoint );
  if (pCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL:
      stat = zclLighting_ProcessInColorControlCmds( pInMsg, pCBs );
      break;

    case ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG:
      // no commands
    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInColorControlCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis

 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInColorControlCmds( zclIncoming_t *pInMsg,
                                             zclLighting_AppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  switch ( pInMsg->hdr.commandID )
  {
    case COMMAND_LIGHTING_MOVE_TO_HUE:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveToHue( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_HUE:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveHue( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_STEP_HUE:
      stat = zclLighting_ProcessInCmd_ColorControl_StepHue( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_TO_SATURATION:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveToSaturation( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_SATURATION:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveSaturation( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_STEP_SATURATION:
      stat = zclLighting_ProcessInCmd_ColorControl_StepSaturation( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_TO_HUE_AND_SATURATION:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveToHueAndSaturation( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_TO_COLOR:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveToColor( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_COLOR:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveColor( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_STEP_COLOR:
      stat = zclLighting_ProcessInCmd_ColorControl_StepColor( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE:
      stat = zclLighting_ProcessInCmd_ColorControl_MoveToColorTemperature( pInMsg, pCBs );
      break;

#ifdef ZCL_LIGHT_LINK_ENHANCE
    case COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE:
      stat = zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHue( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_ENHANCED_MOVE_HUE:
      stat = zclLighting_ProcessInCmd_ColorControl_EnhancedMoveHue( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_ENHANCED_STEP_HUE:
      stat = zclLighting_ProcessInCmd_ColorControl_EnhancedStepHue( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE_AND_SATURATION:
      stat = zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHueAndSaturation( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_COLOR_LOOP_SET:
      stat = zclLighting_ProcessInCmd_ColorControl_ColorLoopSet( pInMsg, pCBs );
      break;

    case COMMAND_LIGHTING_STOP_MOVE_STEP:
      if ( pCBs->pfnColorControl_StopMoveStep ) // no payload to parse
      {
        stat = pCBs->pfnColorControl_StopMoveStep();
      }
      break;
#endif // ZCL_LIGHT_LINK_ENHANCE

    default:
      // Unknown command
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveToHue
 *
 * @brief   Process in the received Move To Hue Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToHue( zclIncoming_t *pInMsg,
                                                       zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_MoveToHue )
  {
    zclCCMoveToHue_t cmd;

    cmd.hue = pInMsg->pData[0];
    cmd.direction = pInMsg->pData[1];
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );

    if ( cmd.hue > LIGHTING_COLOR_HUE_MAX )
    {
      return ( ZCL_STATUS_INVALID_VALUE );
    }

    return ( pCBs->pfnColorControl_MoveToHue( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveHue
 *
 * @brief   Process in the received Move Hue Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveHue( zclIncoming_t *pInMsg,
                                                     zclLighting_AppCallbacks_t *pCBs )
{
  zclCCMoveHue_t cmd;

  cmd.moveMode = pInMsg->pData[0];
  cmd.rate = pInMsg->pData[1];

  // If the Rate field has a value of zero, the command has no effect and
  // a Default Response command is sent in response, with the status code
  // set to INVALID_FIELD.
  if ( ( cmd.rate == 0 ) && ( cmd.moveMode != LIGHTING_MOVE_HUE_STOP ) )
  {
    return ( ZCL_STATUS_INVALID_FIELD );
  }

  if ( pCBs->pfnColorControl_MoveHue )
    return ( pCBs->pfnColorControl_MoveHue( &cmd ) );

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_StepHue
 *
 * @brief   Process in the received Step Hue Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_StepHue( zclIncoming_t *pInMsg,
                                                     zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_StepHue )
  {
    zclCCStepHue_t cmd;

    cmd.stepMode = pInMsg->pData[0];
    cmd.stepSize = pInMsg->pData[1];
    cmd.transitionTime = pInMsg->pData[2];

    return ( pCBs->pfnColorControl_StepHue( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveToSaturation
 *
 * @brief   Process in the received Move to Saturation Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToSaturation( zclIncoming_t *pInMsg,
                                                              zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_MoveToSaturation )
  {
    zclCCMoveToSaturation_t cmd;

    cmd.saturation = pInMsg->pData[0];
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

    if ( cmd.saturation > LIGHTING_COLOR_SAT_MAX )
    {
      return ( ZCL_STATUS_INVALID_VALUE );
    }
    return ( pCBs->pfnColorControl_MoveToSaturation( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveSaturation
 *
 * @brief   Process in the received Move Saturation Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveSaturation( zclIncoming_t *pInMsg,
                                                            zclLighting_AppCallbacks_t *pCBs )
{
  zclCCMoveSaturation_t cmd;

  cmd.moveMode = pInMsg->pData[0];
  cmd.rate = pInMsg->pData[1];

  // If the Rate field has a value of zero, the command has no effect and
  // a Default Response command is sent in response, with the status code
  // set to INVALID_FIELD.
  if ( ( cmd.rate == 0 ) && ( cmd.moveMode != LIGHTING_MOVE_SATURATION_STOP ) )
  {
    return ( ZCL_STATUS_INVALID_FIELD );
  }

  if ( pCBs->pfnColorControl_MoveSaturation )
    return ( pCBs->pfnColorControl_MoveSaturation( &cmd ) );

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_StepSaturation
 *
 * @brief   Process in the received Step Saturation Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_StepSaturation( zclIncoming_t *pInMsg,
                                                            zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_StepSaturation )
  {
    zclCCStepSaturation_t cmd;

    cmd.stepMode = pInMsg->pData[0];
    cmd.stepSize = pInMsg->pData[1];
    cmd.transitionTime = pInMsg->pData[2];

    return ( pCBs->pfnColorControl_StepSaturation( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveToHueAndSaturation
 *
 * @brief   Process in the received Move To Hue And Saturation Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToHueAndSaturation( zclIncoming_t *pInMsg,
                                                                    zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_MoveToHueAndSaturation )
  {
    zclCCMoveToHueAndSaturation_t cmd;

    cmd.hue = pInMsg->pData[0];
    cmd.saturation = pInMsg->pData[1];
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );

    if ( ( cmd.hue > LIGHTING_COLOR_HUE_MAX ) || ( cmd.saturation > LIGHTING_COLOR_SAT_MAX ) )
    {
      return ( ZCL_STATUS_INVALID_VALUE );
    }

    return ( pCBs->pfnColorControl_MoveToHueAndSaturation( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveToColor
 *
 * @brief   Process in the received Move To Color Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToColor( zclIncoming_t *pInMsg,
                                                         zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_MoveToColor )
  {
    zclCCMoveToColor_t cmd;

    cmd.colorX = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.colorY = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[4], pInMsg->pData[5] );

    if ( ( cmd.colorX > LIGHTING_COLOR_CURRENT_X_MAX ) || ( cmd.colorY > LIGHTING_COLOR_CURRENT_Y_MAX ) )
    {
      return ( ZCL_STATUS_INVALID_VALUE );
    }

    return ( pCBs->pfnColorControl_MoveToColor( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveColor
 *
 * @brief   Process in the received Move Color Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveColor( zclIncoming_t *pInMsg,
                                                       zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_MoveColor )
  {
    zclCCMoveColor_t cmd;

    cmd.rateX = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.rateY = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );

    pCBs->pfnColorControl_MoveColor( &cmd );

    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_StepColor
 *
 * @brief   Process in the received Step Color Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_StepColor( zclIncoming_t *pInMsg,
                                                       zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_StepColor )
  {
    zclCCStepColor_t cmd;

    cmd.stepX = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.stepY = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[4], pInMsg->pData[5] );

    return ( pCBs->pfnColorControl_StepColor( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_MoveToColorTemperature
 *
 * @brief   Process in the received Move to Color Temperature Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_MoveToColorTemperature( zclIncoming_t *pInMsg,
                                                                    zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_MoveToColorTemperature )
  {
    zclCCMoveToColorTemperature_t cmd;

    cmd.colorTemperature = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );

    if ( cmd.colorTemperature > LIGHTING_COLOR_TEMPERATURE_MAX )
    {
      return ( ZCL_STATUS_INVALID_VALUE );
    }

    return ( pCBs->pfnColorControl_MoveToColorTemperature( &cmd ) );
  }

  return ( ZFailure );
}

#ifdef ZCL_LIGHT_LINK_ENHANCE
/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHue
 *
 * @brief   Process in the received Enhanced Move To Hue Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHue( zclIncoming_t *pInMsg,
                                                               zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_EnhancedMoveToHue )
  {
    zclCCEnhancedMoveToHue_t cmd;

    cmd.enhancedHue = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.direction = pInMsg->pData[2];
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );

    return ( pCBs->pfnColorControl_EnhancedMoveToHue( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_EnhancedMoveHue
 *
 * @brief   Process in the received Enhanced Move Hue Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedMoveHue( zclIncoming_t *pInMsg,
                                                             zclLighting_AppCallbacks_t *pCBs )
{
  zclCCEnhancedMoveHue_t cmd;

  cmd.moveMode = pInMsg->pData[0];
  cmd.rate = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

  // If the Rate field has a value of zero, the command has no effect and
  // a Default Response command is sent in response, with the status code
  // set to INVALID_FIELD.
  if ( ( cmd.rate == 0 ) && ( cmd.moveMode != LIGHTING_MOVE_HUE_STOP ) )
  {
    return ( ZCL_STATUS_INVALID_FIELD );
  }

  if ( pCBs->pfnColorControl_EnhancedMoveHue )
  {
    return ( pCBs->pfnColorControl_EnhancedMoveHue( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_EnhancedStepHue
 *
 * @brief   Process in the received Enhanced Step Hue Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedStepHue( zclIncoming_t *pInMsg,
                                                             zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_EnhancedStepHue )
  {
    zclCCEnhancedStepHue_t cmd;

    cmd.stepMode = pInMsg->pData[0];
    cmd.stepSize = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );

    return ( pCBs->pfnColorControl_EnhancedStepHue( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHueAndSaturation
 *
 * @brief   Process in the received Enhanced Move To Hue And Saturation Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_EnhancedMoveToHueAndSaturation( zclIncoming_t *pInMsg,
                                                                            zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_EnhancedMoveToHueAndSaturation )
  {
    zclCCEnhancedMoveToHueAndSaturation_t cmd;

    cmd.enhancedHue = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.saturation = pInMsg->pData[2];
    cmd.transitionTime = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );

    if ( cmd.saturation > LIGHTING_COLOR_SAT_MAX )
    {
      return ( ZCL_STATUS_INVALID_VALUE );
    }

    return ( pCBs->pfnColorControl_EnhancedMoveToHueAndSaturation( &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclLighting_ProcessInCmd_ColorControl_ColorLoopSet
 *
 * @brief   Process in the received Color Loop Set Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclLighting_ProcessInCmd_ColorControl_ColorLoopSet( zclIncoming_t *pInMsg,
                                                           zclLighting_AppCallbacks_t *pCBs )
{
  if ( pCBs->pfnColorControl_ColorLoopSet )
  {
    zclCCColorLoopSet_t cmd = {0};

    cmd.updateFlags.byte = pInMsg->pData[0];
    cmd.action = pInMsg->pData[1];
    cmd.direction = pInMsg->pData[2];
    cmd.time = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );
    cmd.startHue = BUILD_UINT16( pInMsg->pData[5], pInMsg->pData[6] );

    return ( pCBs->pfnColorControl_ColorLoopSet( &cmd ) );
  }

  return ( ZFailure );
}
#endif // ZCL_LIGHT_LINK_ENHANCE

/****************************************************************************
****************************************************************************/
