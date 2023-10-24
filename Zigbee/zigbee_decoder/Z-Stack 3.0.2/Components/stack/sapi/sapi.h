/**************************************************************************************************
  Filename:       sapi.h
  Revised:        $Date: 2008-03-14 11:20:21 -0700 (Fri, 14 Mar 2008) $
  Revision:       $Revision: 16590 $

  Description:    Z-Stack Simple Application Interface.


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

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

#ifndef SAPI_H
#define SAPI_H

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "af.h"

/******************************************************************************
 * CONSTANTS
 */

// Simple Task Events
#define ZB_ALLOW_BIND_TIMER               0x4000    //0x0001
#define ZB_BIND_TIMER                     0x2000    //0x0002
#define ZB_ENTRY_EVENT                    0x1000    //0x0004
#define ZB_USER_EVENTS                    0x00FF

// Find Device Search Types
#define ZB_IEEE_SEARCH                    1

// Device Info Constants
#define ZB_INFO_DEV_STATE                 0
#define ZB_INFO_IEEE_ADDR                 1
#define ZB_INFO_SHORT_ADDR                2
#define ZB_INFO_PARENT_SHORT_ADDR         3
#define ZB_INFO_PARENT_IEEE_ADDR          4
#define ZB_INFO_CHANNEL                   5
#define ZB_INFO_PAN_ID                    6
#define ZB_INFO_EXT_PAN_ID                7

// SAPI SendDataRequest destinations
#define ZB_BINDING_ADDR                   INVALID_NODE_ADDR
#define ZB_BROADCAST_ADDR                 0xffff

// SAPI Status Codes
#define ZB_SUCCESS                        ZSuccess
#define ZB_FAILURE                        ZFailure
#define ZB_INVALID_PARAMETER              ZInvalidParameter
#define ZB_ALREADY_IN_PROGRESS            ZSapiInProgress
#define ZB_TIMEOUT                        ZSapiTimeout
#define ZB_INIT                           ZSapiInit
#define ZB_AF_FAILURE                     afStatus_FAILED
#define ZB_AF_MEM_FAIL                    afStatus_MEM_FAIL
#define ZB_AF_INVALID_PARAMETER           afStatus_INVALID_PARAMETER

// SAPI Scan Duration Values
#define ZB_SCAN_DURATION_0                0   //  19.2 ms
#define ZB_SCAN_DURATION_1                1   //  38.4 ms
#define ZB_SCAN_DURATION_2                2   //  76.8 ms
#define ZB_SCAN_DURATION_3                3   //  153.6 ms
#define ZB_SCAN_DURATION_4                4   //  307.2 ms
#define ZB_SCAN_DURATION_5                5   //  614.4 ms
#define ZB_SCAN_DURATION_6                6   //  1.23 sec
#define ZB_SCAN_DURATION_7                7   //  2.46 sec
#define ZB_SCAN_DURATION_8                8   //  4.92 sec
#define ZB_SCAN_DURATION_9                9   //  9.83 sec
#define ZB_SCAN_DURATION_10               10  //  19.66 sec
#define ZB_SCAN_DURATION_11               11  //  39.32 sec
#define ZB_SCAN_DURATION_12               12  //  78.64 sec
#define ZB_SCAN_DURATION_13               13  //  157.28 sec
#define ZB_SCAN_DURATION_14               14  //  314.57 sec

// Device types definitions ( from ZGlobals.h file )
#define ZG_DEVICETYPE_COORDINATOR      0x00
#define ZG_DEVICETYPE_ROUTER           0x01
#define ZG_DEVICETYPE_ENDDEVICE        0x02
/******************************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint16 panID;
  uint8 channel;
} zb_NetworkList_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16 data;
} sapi_CbackEvent_t;

/******************************************************************************
 * PUBLIC VARIABLES
 */

extern uint8 sapi_TaskID;
extern endPointDesc_t sapi_epDesc;

/******************************************************************************
 * PUBLIC FUNCTIONS
 */

#ifdef __cplusplus
extern "C"
{
#endif

extern const SimpleDescriptionFormat_t zb_SimpleDesc;

/******************************************************************************
 * @fn          zb_SystemReset
 *
 * @brief       The zb_SystemReset function reboots the ZigBee Stack.  The
 *              zb_SystemReset function can be called after a call to
 *              zb_WriteConfiguration to restart Z-Stack with the updated
 *              configuration.
 *
 * @param       none
 *
 * @return      none
 */
extern void zb_SystemReset ( void );

/******************************************************************************
 * @fn          zb_StartRequest
 *
 * @brief       The zb_StartRequest function starts the ZigBee stack.  When the
 *              ZigBee stack starts, the device reads configuration parameters
 *              from Nonvolatile memory and the device joins its network.  The
 *              ZigBee stack calls the zb_StartConrifm callback function when
 *              the startup process completes.
 *
 * @param       none
 *
 * @return      none
 */
extern void zb_StartRequest ( void );

/******************************************************************************
 * @fn          zb_PermitJoiningRequest
 *
 * @brief       The zb_PermitJoiningRequest function is used to control the
 *              joining permissions and thus allow or disallow new devices from
 *              joining the network.
 *
 * @param       destination - The destination parameter indicates the address
 *                            of the device for which the joining permissions
 *                            should be set. This is usually the local device
 *                            address or the special broadcast address that denotes
 *                            all routers and coordinator ( 0xFFFC ). This way
 *                            the joining permissions of a single device or the
 *                            whole network can be controlled.
 *              timeout -  Indicates the amount of time in seconds for which
 *                         the joining permissions should be turned on.
 *                         If timeout is set to 0x00, the device will turn off the
 *                         joining permissions indefinitely. If it is set to 0xFF,
 *                         the joining permissions will be turned on indefinitely.
 *
 *
 * @return      ZB_SUCCESS or a failure code
 *
 */
extern uint8 zb_PermitJoiningRequest ( uint16 destination, uint8 timeout );

/******************************************************************************
 * @fn          zb_BindDevice
 *
 * @brief       The zb_BindDevice function establishes or removes a ‘binding’
 *              between two devices.  Once bound, an application can send
 *              messages to a device by referencing the commandId for the
 *              binding.
 *
 * @param       create - TRUE to create a binding, FALSE to remove a binding
 *              commandId - The identifier of the binding
 *              pDestination - The 64-bit IEEE address of the device to bind to
 *
 * @return      ZB_SUCCESS if the bind process started sucessfully, else
 *              en error code is returned.  If the return value is ZB_SUCCESS
 *              then the status of the bind operation is returned in the
 *              zb_BindConfirm callback.
 */
extern void zb_BindDevice ( uint8 create, uint16 commandId, uint8 *pDestination );

/******************************************************************************
 * @fn          zb_AllowBind
 *
 * @brief       The zb_AllowBind function puts the device into the
 *              Allow Binding Mode for a given period of time.  A peer device
 *              can establish a binding to a device in the Allow Binding Mode
 *              by calling zb_BindDevice with a destination address of NULL
 *
 * @param       timeout - The number of seconds to remain in the allow binding
 *                        mode.  Valid values range from 1 through 65.
 *
 * @return      ZB_SUCCESS if the device entered the allow bind mode, else
 *              an error code.
 */
extern void zb_AllowBind (  uint8 timeout );

/******************************************************************************
 * @fn          zb_SendDataRequest
 *
 * @brief       The zb_SendDataRequest function initiates transmission of data
 *              to a peer device
 *
 * @param       destination - The destination of the data.  The destination can
 *                            be one of the following:
 *                            - 16-Bit short address of device [0-0xfffD]
 *                            - ZB_BROADCAST_ADDR sends the data to all devices
 *                              in the network.
 *                            - ZB_BINDING_ADDR sends the data to a previously
 *                              bound device.
 *
 *              commandId - The command ID to send with the message.  If the
 *                          ZB_BINDING_ADDR destination is used, this parameter
 *                          also indicates the binding to use.
 *
 *              len - The size of the pData buffer in bytes
 *              handle - A handle used to identify the send data request.
 *              ack - TRUE if requesting acknowledgement from the destination.
 *              radius - The max number of routers the data can travel through
 *                       before the data is dropped.
 *
 * @return      none
 */
extern void zb_SendDataRequest ( uint16 destination, uint16 commandId, uint8 len,
                          uint8 *pData, uint8 handle, uint8 ack, uint8 radius );


/******************************************************************************
 * @fn          zb_ReadConfiguration
 *
 * @brief       The zb_ReadConfiguration function is used to get a
 *              Configuration Protperty from Nonvolatile memory.
 *
 * @param       configId - The identifier for the configuration property
 *              len - The size of the pValue buffer in bytes
 *              pValue - A buffer to hold the configuration property
 *
 * @return      none
 */
extern uint8 zb_ReadConfiguration( uint8 configId, uint8 len, void *pValue );

/******************************************************************************
 * @fn          zb_WriteConfiguration
 *
 * @brief       The zb_WriteConfiguration function is used to write a
 *              Configuration Property to nonvolatile memory.
 *
 * @param       configId - The identifier for the configuration property
 *              len - The size of the pValue buffer in bytes
 *              pValue - A buffer containing the new value of the
 *                       configuration property
 *
 * @return      none
 */
extern uint8 zb_WriteConfiguration( uint8 configId, uint8 len, void *pValue );

/******************************************************************************
 * @fn          zb_GetDeviceInfo
 *
 * @brief       The zb_GetDeviceInfo function retrieves a Device Information
 *              Property.
 *
 * @param       param - The identifier for the device information
 *              pValue - A buffer to hold the device information
 *
 * @return      none
 */
extern void zb_GetDeviceInfo ( uint8 param, void *pValue );

/******************************************************************************
 * @fn          zb_FindDeviceRequest
 *
 * @brief       The zb_FindDeviceRequest function is used to determine the
 *              short address for a device in the network.  The device initiating
 *              a call to zb_FindDeviceRequest and the device being discovered
 *              must both be a member of the same network.  When the search is
 *              complete, the zv_FindDeviceConfirm callback function is called.
 *
 * @param       searchType - The type of search to perform. Can be one of following:
 *                           ZB_IEEE_SEARCH - Search for 16-bit addr given IEEE addr.
 *              searchKey - Value to search on.
 *
 * @return      none
 */
extern void zb_FindDeviceRequest( uint8 searchType, void *searchKey );

/******************************************************************************
 * @fn          zb_HandleOsalEvent
 *
 * @brief       The zb_HandleOsalEvent function is called by the operating
 *              system when a task event is set
 *
 * @param       event - Bitmask containing the events that have been set
 *
 * @return      none
 */
extern void zb_HandleOsalEvent( uint16 event );

/******************************************************************************
 * @fn          zb_StartConfirm
 *
 * @brief       The zb_StartConfirm callback is called by the ZigBee stack
 *              after a start request operation completes
 *
 * @param       status - The status of the start operation.  Status of
 *                       ZB_SUCCESS indicates the start operation completed
 *                       successfully.  Else the status is an error code.
 *
 * @return      none
 */
extern void zb_StartConfirm( uint8 status );

/******************************************************************************
 * @fn          zb_SendDataConfirm
 *
 * @brief       The zb_SendDataConfirm callback function is called by the
 *              ZigBee after a send data operation completes
 *
 * @param       handle - The handle identifying the data transmission.
 *              status - The status of the operation.
 *
 * @return      none
 */
extern void zb_SendDataConfirm( uint8 handle, uint8 status );

/******************************************************************************
 * @fn          zb_BindConfirm
 *
 * @brief       The zb_BindConfirm callback is called by the ZigBee stack
 *              after a bind operation completes.
 *
 * @param       commandId - The command ID of the binding being confirmed.
 *              status - The status of the bind operation.
 *              allowBind - TRUE if the bind operation was initiated by a call
 *                          to zb_AllowBindRespones.  FALSE if the operation
 *                          was initiated by a call to ZB_BindDevice
 *
 * @return      none
 */
extern void zb_BindConfirm( uint16 commandId, uint8 status );

/******************************************************************************
 * @fn          zb_FindDeviceConfirm
 *
 * @brief       The zb_FindDeviceConfirm callback function is called by the
 *              ZigBee stack when a find device operation completes.
 *
 * @param       searchType - The type of search that was performed.
 *              searchKey - Value that the search was executed on.
 *              result - The result of the search.
 *
 * @return      none
 */
extern void zb_FindDeviceConfirm( uint8 searchType, uint8 *searchKey, uint8 *result );

/******************************************************************************
 * @fn          zb_ReceiveDataIndication
 *
 * @brief       The zb_ReceiveDataIndication callback function is called
 *              asynchronously by the ZigBee stack to notify the application
 *              when data is received from a peer device.
 *
 * @param       source - The short address of the peer device that sent the data
 *              command - The commandId associated with the data
 *              len - The number of bytes in the pData parameter
 *              pData - The data sent by the peer device
 *
 * @return      none
 */
extern void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData  );

extern void zb_AllowBindConfirm( uint16 source );

extern void zb_HandleKeys( uint8 shift, uint8 keys );


/* External declarations required by SAPI */
extern UINT16 SAPI_ProcessEvent( byte task_id, UINT16 events );
extern void SAPI_Init( byte task_id );
extern void osalAddTasks( void );

#ifdef __cplusplus
}
#endif

#endif // SAPI_H
