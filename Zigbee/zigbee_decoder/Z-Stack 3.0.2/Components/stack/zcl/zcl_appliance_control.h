/**************************************************************************************************
  Filename:       zcl_appliance_control.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Control definitions.


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

#ifndef ZCL_APPLIANCE_CONTROL_H
#define ZCL_APPLIANCE_CONTROL_H

#ifdef ZCL_APPLIANCE_CONTROL

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

/*********************************************/
/***  Appliance Control Cluster Attributes ***/
/********************************************/

// Server Attributes
#define ATTRID_APPLIANCE_CONTROL_START_TIME                             0x0000  // M, R, UINT16
#define ATTRID_APPLIANCE_CONTROL_FINISH_TIME                            0x0001  // M, R, UINT16
#define ATTRID_APPLIANCE_CONTROL_REMAINING_TIME                         0x0002  // O, R, UINT16

// Server Attribute Defaults
#define ATTR_DEFAULT_APPLIANCE_CONTROL_START_TIME                       0x0000
#define ATTR_DEFAULT_APPLIANCE_CONTROL_FINISH_TIME                      0x0000
#define ATTR_DEFAULT_APPLIANCE_CONTROL_REMAINING_TIME                   0x0000

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_APPLIANCE_CONTROL_COMMAND_EXECUTION                     0x00  // M, zclApplianceControlCommandExecution_t
#define COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE                          0x01  // M, no payload
#define COMMAND_APPLIANCE_CONTROL_WRITE_FUNCTIONS                       0x02  // O, zclApplianceControlWriteFunctions_t
#define COMMAND_APPLIANCE_CONTROL_OVERLOAD_PAUSE_RESUME                 0x03  // O, no payload
#define COMMAND_APPLIANCE_CONTROL_OVERLOAD_PAUSE                        0x04  // O, no payload
#define COMMAND_APPLIANCE_CONTROL_OVERLOAD_WARNING                      0x05  // O, zclApplianceControlOverloadWarning_t

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE_RSP                      0x00  // M, zclApplianceControlSignalState_t
#define COMMAND_APPLIANCE_CONTROL_SIGNAL_STATE_NOTIFICATION             0x01  // M, zclApplianceControlSignalState_t

// Command Identification Values enumeration field
#define APPLIANCE_CONTROL_RESERVED                             0x00
#define APPLIANCE_CONTROL_START                                0x01  // Start appliance cycle
#define APPLIANCE_CONTROL_STOP                                 0x02  // Stop appliance cycle
#define APPLIANCE_CONTROL_PAUSE                                0x03  // Pause appliance cycle
#define APPLIANCE_CONTROL_START_SUPERFREEZING                  0x04  // Start superfreezing cycle
#define APPLIANCE_CONTROL_STOP_SUPERFREEZING                   0x05  // Stop superfreezing cycle
#define APPLIANCE_CONTROL_START_SUPERCOOLING                   0x06  // Start supercooling cycle
#define APPLIANCE_CONTROL_STOP_SUPERCOOLING                    0x07  // Stop supercooling cycle
#define APPLIANCE_CONTROL_DISABLE_GAS                          0x08
#define APPLIANCE_CONTROL_ENABLE_GAS                           0x09

// Event ID Warning Event enumeration field
#define APPLIANCE_CONTROL_WARNING_1                            0x00  // Overall power above "available power" level
#define APPLIANCE_CONTROL_WARNING_2                            0x01  // Overall power above "power threshold" level
#define APPLIANCE_CONTROL_WARNING_3                            0x02  // Overall power back below the "available power" level
#define APPLIANCE_CONTROL_WARNING_4                            0x03  // Overall power back below the "power threshold" level
#define APPLIANCE_CONTROL_WARNING_5                            0x04  // Overall power will be potentially above "available power"
                                                                     // if the appliance starts

// Appliance Status enumeration field
#define APPLIANCE_CONTROL_STATUS_RESERVED                      0x00
#define APPLIANCE_CONTROL_STATUS_OFF                           0x01  // Appliance in off state
#define APPLIANCE_CONTROL_STATUS_STAND_BY                      0x02  // Appliance in stand-by
#define APPLIANCE_CONTROL_STATUS_PROGRAMMED                    0x03  // Appliance already programmed
#define APPLIANCE_CONTROL_STATUS_PROGRAMMED_WAITING_TO_START   0x04  // Appliance already programmed and ready to start
#define APPLIANCE_CONTROL_STATUS_RUNNING                       0x05  // Appliance is running
#define APPLIANCE_CONTROL_STATUS_PAUSE                         0x06  // Appliance is in pause
#define APPLIANCE_CONTROL_STATUS_END_PROGRAMMED                0x07  // Appliance end programmed tasks
#define APPLIANCE_CONTROL_STATUS_FAILURE                       0x08  // Appliance is in a failure state
#define APPLIANCE_CONTROL_STATUS_PROGRAM_INTERRUPTED           0x09  // The appliance programmed tasks have been interrupted
#define APPLIANCE_CONTROL_STATUS_IDLE                          0x0A  // Appliance in idle state
#define APPLIANCE_CONTROL_STATUS_RINSE_HOLD                    0x0B  // Appliance in rinse hold
#define APPLIANCE_CONTROL_STATUS_SERVICE                       0x0C  // Appliance in service state
#define APPLIANCE_CONTROL_STATUS_SUPERFREEZING                 0x0D  // Appliance in superfreezing state
#define APPLIANCE_CONTROL_STATUS_SUPERCOOLING                  0x0E  // Appliance in supercooling state
#define APPLIANCE_CONTROL_STATUS_SUPERHEATING                  0x0F  // Appliance in superheating state

/*******************************************************************************
 * TYPEDEFS
 */

/*** SERVER COMMANDS RECEIVED ***/

/*** ZCL Appliance Control Cluster: Command Execution payload ***/
typedef struct
{
  uint8 commandID;    // e.g. APPLIANCE_CONTROL_RESERVED
} zclApplianceControlCommandExecution_t;

/*** ZCL Appliance Control Cluster: Write Functions payload ***/
typedef struct
{
  uint16 functionID;
  uint8 functionDataType;   // enumerated function data type
  uint8 *pFunctionData;    // variable based on functionDataType value
} zclApplianceControlWriteFunctions_t;

/*** ZCL Appliance Control Cluster: Overload Warning payload ***/
typedef struct
{
  uint8 warningEvent;   // e.g. APPLIANCE_CONTROL_WARNING_1
} zclApplianceControlOverloadWarning_t;

/*** SERVER COMMANDS GENERATED ***/

/*** ZCL Appliance Control Cluster: Signal State Response & Notification payload ***/
typedef struct
{
  uint8 applianceStatus;   // e.g. APPLIANCE_CONTROL_STATUS_RESERVED
  uint8 remoteEnableFlagsDeviceStatus2;
  uint24 applianceStatus2;
} zclApplianceControlSignalState_t;


typedef ZStatus_t (*zclAppliance_Control_CommandExecution_t)( zclIncoming_t *pInMsg, zclApplianceControlCommandExecution_t *pCmd );
typedef ZStatus_t (*zclAppliance_Control_SignalState_t)( zclIncoming_t *pInMsg );
typedef ZStatus_t (*zclAppliance_Control_WriteFunctions_t)( zclIncoming_t *pInMsg, zclApplianceControlWriteFunctions_t *pCmd );
typedef ZStatus_t (*zclAppliance_Control_OverloadPauseResume_t)( void );
typedef ZStatus_t (*zclAppliance_Control_OverloadPause_t)( void );
typedef ZStatus_t (*zclAppliance_Control_OverloadWarning_t)( zclApplianceControlOverloadWarning_t *pCmd );
typedef ZStatus_t (*zclAppliance_Control_SignalStateRsp_t)( zclApplianceControlSignalState_t *pCmd );
typedef ZStatus_t (*zclAppliance_Control_SignalStateNotification_t)( zclApplianceControlSignalState_t *pCmd );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclAppliance_Control_CommandExecution_t                  pfnApplianceControl_CommandExecution;
  zclAppliance_Control_SignalState_t                       pfnApplianceControl_SignalState;
  zclAppliance_Control_WriteFunctions_t                    pfnApplianceControl_WriteFunctions;
  zclAppliance_Control_OverloadPauseResume_t               pfnApplianceControl_OverloadPauseResume;
  zclAppliance_Control_OverloadPause_t                     pfnApplianceControl_OverloadPause;
  zclAppliance_Control_OverloadWarning_t                   pfnApplianceControl_OverloadWarning;
  zclAppliance_Control_SignalStateRsp_t                    pfnApplianceControl_SignalStateRsp;
  zclAppliance_Control_SignalStateNotification_t           pfnApplianceControl_SignalStateNotification;
} zclApplianceControl_AppCallbacks_t;

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */

/*** Register for callbacks from this cluster library ***/
extern ZStatus_t zclApplianceControl_RegisterCmdCallbacks( uint8 endpoint, zclApplianceControl_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclApplianceControl_Send_CommandExecution
 *
 * @brief   Request sent to server for Appliance Control Execution of a Command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   commandID - specifies the Appliance Control command to be executed
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_CommandExecution( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 commandID,
                                                            uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_SignalState
 *
 * @brief   Request sent to server for Appliance Control Signal State.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_SignalState( uint8 srcEP, afAddrType_t *dstAddr,
                                                       uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_WriteFunctions
 *
 * @brief   Request sent to server for Appliance Control Write Functions.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          functionID - Contains the ID of the function to be written
 *          functionDataType - Contains the data type of the attribute to be written
 *          functionData[] - Contains the actual value of the function to be written
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_WriteFunctions( uint8 srcEP, afAddrType_t *dstAddr,
                                                          zclApplianceControlWriteFunctions_t *pPayload,
                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_OverloadPauseResume
 *
 * @brief   Request sent to server for Appliance Control Overload Pause Resume.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_OverloadPauseResume( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_OverloadPause
 *
 * @brief   Request sent to server for Appliance Control Overload Pause.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_OverloadPause( uint8 srcEP, afAddrType_t *dstAddr,
                                                         uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_OverloadWarning
 *
 * @brief   Request sent to server for Appliance Control Overload Warning.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   warningEvent - The ID of the events to be communicated to alert about possible overload
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_OverloadWarning( uint8 srcEP, afAddrType_t *dstAddr,
                                                           uint8 warningEvent,
                                                           uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_SignalStateRsp
 *
 * @brief   Response sent to client for Appliance Control Signal State Response.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   applianceStatus - current status of the appliance
 * @param   remoteEnableFlagsDeviceStatus2 - current status of appliance correlated with remote control
 * @param   applianceStatus2 - detailed definition of appliance state
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_SignalStateRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint8 applianceStatus, uint8 remoteEnableFlagsDeviceStatus2,
                                                          uint24 applianceStatus2,
                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceControl_Send_SignalStateNotification
 *
 * @brief   Response sent to client for Appliance Control Signal State Response.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   applianceStatus - current status of the appliance
 * @param   remoteEnableFlagsDeviceStatus2 - current status of appliance correlated with remote control
 * @param   applianceStatus2 - detailed definition of appliance state
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceControl_Send_SignalStateNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                   uint8 applianceStatus, uint8 remoteEnableFlagsDeviceStatus2,
                                                                   uint24 applianceStatus2,
                                                                   uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif
#endif // ZCL_APPLIANCE_CONTROL
#endif /* ZCL_APPLIANCE_CONTROL_H */
