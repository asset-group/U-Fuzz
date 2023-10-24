/**************************************************************************************************
  Filename:       zcl_cc.h
  Revised:        $Date: 2011-04-13 10:12:34 -0700 (Wed, 13 Apr 2011) $
  Revision:       $Revision: 25678 $

  Description:    Zigbee Cluster Library - Commissioning Cluster


  Copyright 2011 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_CC_H
#define ZCL_CC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * MACROS
 */
// Startup Mode
#define zcl_CCStartupMode( a )                   ( (a) & CC_STARTUP_MODE )
#define zcl_CCImmediate( a )                     ( (a) & CC_IMMEDIATE )

/*********************************************************************
 * CONSTANTS
 */

/**********************************************/
/*** Commissioning Clusters Attributes List ***/
/**********************************************/

// Commissioning Clusters Attribute Set
#define ATTRID_MASK_CC_STARTUP_PARAMS_STACK      0x0000
#define ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY   0x0010
#define ATTRID_MASK_CC_JOIN_PARAMS               0x0020
#define ATTRID_MASK_CC_END_DEVICE_PARAMS         0x0030
#define ATTRID_MASK_CC_CONCENTRATOR_PARAMS       0x0040

// Startup Parameters Attribute Set - Stack 0x000
#define ATTRID_CC_SHORT_ADDRESS                  ( 0x0000 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_EXTENDED_PANID                 ( 0x0001 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_PANID                          ( 0x0002 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_CHANNEL_MASK                   ( 0x0003 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_PROTOCOL_VERSION               ( 0x0004 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_STACK_PROFILE                  ( 0x0005 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_STARTUP_CONTROL                ( 0x0006 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
  
// Startup Parameters Attribute Set - Security 0x001  
#define ATTRID_CC_TRUST_CENTER_ADDRESS           ( 0x0000 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_TRUST_CENTER_MASTER_KEY        ( 0x0001 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_KEY                    ( 0x0002 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_USE_INSECURE_JOIN              ( 0x0003 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_PRECONFIGURED_LINK_KEY         ( 0x0004 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_KEY_SEQ_NUM            ( 0x0005 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_KEY_TYPE               ( 0x0006 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_MANAGER_ADDRESS        ( 0x0007 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )

// Join Parameters Attribute Set 0x002  
#define ATTRID_CC_SCAN_ATTEMPTS                  ( 0x0000 | ATTRID_MASK_CC_JOIN_PARAMS )
#define ATTRID_CC_TIME_BETWEEN_SCANS             ( 0x0001 | ATTRID_MASK_CC_JOIN_PARAMS )  
#define ATTRID_CC_REJOIN_INTERVAL                ( 0x0002 | ATTRID_MASK_CC_JOIN_PARAMS )
#define ATTRID_CC_MAX_REJOIN_INTERVAL            ( 0x0003 | ATTRID_MASK_CC_JOIN_PARAMS ) 
  
// End Device Parameters Attribute Set 0x003  
#define ATTRID_CC_INDIRECT_POLL_RATE             ( 0x0000 | ATTRID_MASK_CC_END_DEVICE_PARAMS )
#define ATTRID_CC_PARENT_RETRY_THRESHOLD         ( 0x0001 | ATTRID_MASK_CC_END_DEVICE_PARAMS )
  
// Concentrator Parameters Attribute Set 0x004
#define ATTRID_CC_CONCENTRATOR_FLAG              ( 0x0000 | ATTRID_MASK_CC_CONCENTRATOR_PARAMS )
#define ATTRID_CC_CONCENTRATOR_RADIUS            ( 0x0001 | ATTRID_MASK_CC_CONCENTRATOR_PARAMS )
#define ATTRID_CC_CONCENTRATOR_DISCOVERY_TIME    ( 0x0002 | ATTRID_MASK_CC_CONCENTRATOR_PARAMS )

/************************************************************/
/***    Commissioning Cluster Command ID                  ***/
/************************************************************/

// Commands Received by Commissioning Cluster Server
  
#define COMMAND_CC_RESTART_DEVICE                0x0000
#define COMMAND_CC_SAVE_STARTUP_PARAMS           0x0001
#define COMMAND_CC_RESTORE_STARTUP_PARAMS        0x0002
#define COMMAND_CC_RESET_STARTUP_PARAMS          0x0003
 
// Commands generated by Commissioning Cluster Server

#define COMMAND_CC_RESTART_DEVICE_RSP            0x0000
#define COMMAND_CC_SAVE_STARTUP_PARAMS_RSP       0x0001
#define COMMAND_CC_RESTORE_STARTUP_PARAMS_RSP    0x0002
#define COMMAND_CC_RESET_STARTUP_PARAMS_RSP      0x0003

/******************************************************************/
/***        Enumerations                                        ***/
/******************************************************************/
 
// StartupControl attribute values
#define CC_STARTUP_CONTROL_OPTION_0              0x00 // Silent join
#define CC_STARTUP_CONTROL_OPTION_1              0x01 // Form network
#define CC_STARTUP_CONTROL_OPTION_2              0x02 // Rejoin network
#define CC_STARTUP_CONTROL_OPTION_3              0x03 // MAC Associate

/******************************************************************/
/***        BitMap                                              ***/
/******************************************************************/

// Restart Device command Options bit masks:
//  - Startup Mode (bits: 0..2)
//  - Immediate (bit: 3)
#define CC_STARTUP_MODE                          0x07
#define CC_IMMEDIATE                             0x08

// Startup Mode Sub-Field Values
#define CC_STARTUP_MODE_REPLACE_RESTART          0x00
#define CC_STARTUP_MODE_ONLY_RESTART             0x01

// Reset Startup Parameters command Options
#define CC_RESET_CURRENT                         0x01
#define CC_RESET_ALL                             0x02
#define CC_ERASE_INDEX                           0x04

/******************************************************************/
/***        Other Constants                                     ***/
/******************************************************************/

// Default Attribute Values
#define CC_DEFAULT_SHORT_ADDR                    0xFFFF
#define CC_DEFAULT_PANID                         0xFFFF
#define CC_DEFAULT_PROTOCOL_VERSION              0x02
#define CC_DEFAULT_NETWORK_KEY_SEQ_NUM           0x00
#define CC_DEFAULT_NETWORK_MANAGER_ADDR          0x00
#define CC_DEFAULT_SCAN_ATTEMPTS                 0x05
#define CC_DEFAULT_TIME_BETWEEN_SCANS            0x64
#define CC_DEFAULT_REJOIN_INTERVAL               0x3C
#define CC_DEFAULT_MAX_REJOIN_INTERVAL           0x0E10
#define CC_DEFAULT_CONCENTRATOR_RADIUS           0x0F
#define CC_DEFAULT_CONCENTRATOR_DISCOVERY_TIME   0x00

// Max Attribute Values
#define CC_MAX_INDIRECT_POLL_RATE                0xFFFF
#define CC_MAX_PARENT_RETRY_THRESHOLD            0xFF
#define CC_MAX_CONCENTRATOR_RADIUS               0xFF
#define CC_MAX_CONCENTRATOR_DISCOVERY_TIME       0xFF

// Command Packet Length
#define CC_PACKET_LEN_RESTART_DEVICE             0x03
#define CC_PACKET_LEN_STARTUP_PARAMS_CMD         0x02
#define CC_PACKET_LEN_SERVER_RSP                 0x01
 

/********************************************************************
 * MACROS
 */
  
/*********************************************************************
 * TYPEDEFS
 */
  
/*** Structures used for callback functions ***/

// Restart Device command
typedef struct
{
  uint8 options;            
  uint8 delay;      
  uint8 jitter;
} zclCCRestartDevice_t;

// Startup Parameters command - Save, Restore, Reset
typedef struct
{
  uint8 options;            
  uint8 index;    
} zclCCStartupParams_t;

// Server Parameters Response command
typedef struct
{
  uint8 status;               
} zclCCServerParamsRsp_t;

/*********************************************************************
 * CALLBACKS
 */

/* Commands */

// This callback is called to process an incoming Restart Device command
typedef void (*zclCC_Restart_Device_t)( zclCCRestartDevice_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum );

// This callback is called to process an incoming Save Startup Parameters command
typedef void (*zclCC_Save_StartupParams_t)( zclCCStartupParams_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum );

// This callback is called to process an incoming Restore Startup Parameters command
typedef void (*zclCC_Restore_StartupParams_t)( zclCCStartupParams_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum );

// This callback is called to process an incoming Reset Startup Parameters command
typedef void (*zclCC_Reset_StartupParams_t)( zclCCStartupParams_t *pCmd, afAddrType_t *srcAddr, uint8 seqNum ) ;

/* Response */

// This callback is called to process an incoming Restart Device Response command
typedef void (*zclCC_Restart_DeviceRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8 seqNum );

// This callback is called to process an incoming Save Startup Parameters Response command
typedef void (*zclCC_Save_StartupParamsRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8 seqNum );

// This callback is called to process an incoming Restore Startup Parameters Response command
typedef void (*zclCC_Restore_StartupParamsRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8 seqNum );

// This callback is called to process an incoming Reset Startup Parameters Response command
typedef void (*zclCC_Reset_StartupParamsRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8 seqNum ) ;

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive

typedef struct			
{
  zclCC_Restart_Device_t            pfnRestart_Device;
  zclCC_Save_StartupParams_t        pfnSave_StartupParams;
  zclCC_Restore_StartupParams_t     pfnRestore_StartupParams;
  zclCC_Reset_StartupParams_t       pfnReset_StartupParams;
  zclCC_Restart_DeviceRsp_t         pfnRestart_DeviceRsp;
  zclCC_Save_StartupParamsRsp_t     pfnSave_StartupParamsRsp;
  zclCC_Restore_StartupParamsRsp_t  pfnRestore_StartupParamsRsp;
  zclCC_Reset_StartupParamsRsp_t    pfnReset_StartupParamsRsp;
} zclCC_AppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTION MACROS
 */

/*
 *  Send a Save Startup Parameters command
 *  Use like:
 *      ZStatus_t zclCC_Send_SaveStartupParams( uint8 srcEP, afAddrType_t *dstAddr, zclCCStartupParams_t *pCmd, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_SaveStartupParams(a,b,c,d,e) zclCC_Send_StartupParamsCmd( (a), (b), (c), COMMAND_CC_SAVE_STARTUP_PARAMS, (d), (e) )

/*
 *  Send a Restore Startup Parameters command
 *  Use like:
 *      ZStatus_t zclCC_Send_RestoreStartupParams( uint8 srcEP, afAddrType_t *dstAddr, zclCCStartupParams_t *pCmd, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_RestoreStartupParams(a,b,c,d,e) zclCC_Send_StartupParamsCmd( (a), (b), (c), COMMAND_CC_RESTORE_STARTUP_PARAMS, (d), (e) )

/*
 *  Send a Reset Startup Parameters command
 *  Use like:
 *      ZStatus_t zclCC_Send_ResetStartupParams( uint8 srcEP, afAddrType_t *dstAddr, zclCCStartupParams_t *pCmd, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_ResetStartupParams(a,b,c,d,e) zclCC_Send_StartupParamsCmd( (a), (b), (c), COMMAND_CC_RESET_STARTUP_PARAMS, (d), (e) )

/*
 *  Send a Restart Device Response
 *  Use like:
 *      ZStatus_t zclCC_Send_RestartDeviceRsp( uint8 srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_RestartDeviceRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_RESTART_DEVICE_RSP, (d), (e) )

/*
 *  Send a Save Startup Parameters Response
 *  Use like:
 *      ZStatus_t zclCC_Send_SaveStartupParamsRsp( uint8 srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_SaveStartupParamsRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_SAVE_STARTUP_PARAMS_RSP, (d), (e) )

/*
 *  Send a Restore Startup Parameters Response
 *  Use like:
 *      ZStatus_t zclCC_Send_RestoreStartupParamsRsp( uint8 srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_RestoreStartupParamsRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_RESTORE_STARTUP_PARAMS_RSP, (d), (e) )

/*
 *  Send a Reset Startup Parameters Response
 *  Use like:
 *      ZStatus_t zclCC_Send_ResetStartupParamsRsp( uint8 srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclCC_Send_ResetStartupParamsRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_RESET_STARTUP_PARAMS_RSP, (d), (e) )


/*********************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclCC_RegisterCmdCallbacks( uint8 endpoint, zclCC_AppCallbacks_t *callbacks );

/*
 * Send Restart Device Command
 */
ZStatus_t zclCC_Send_RestartDevice( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclCCRestartDevice_t *pCmd, 
                                    uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send Startup Parameters Command (Save, Restore or Reset)
 */
ZStatus_t zclCC_Send_StartupParamsCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                       zclCCStartupParams_t *pCmd, uint8 cmdId,
                                       uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Send Server Response (Restart Device, Save, Restore or Reset)
 */
ZStatus_t zclCC_Send_ServerParamsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                      zclCCServerParamsRsp_t *pCmd, uint8 cmdId,
                                      uint8 disableDefaultRsp, uint8 seqNum );

#ifdef __cplusplus
}
#endif

#endif /* ZCL_CC_H */
