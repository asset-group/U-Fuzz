/**************************************************************************************************
  Filename:       zcl_pi.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Protocol Interfaces Definitions


  Copyright 2010-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_PI_H
#define ZCL_PI_H

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

/*************************************************/
/***   Protocol Interface Cluster Attributes   ***/
/*************************************************/
  // Attributes of the Generic Tunnel cluster
#define ATTRID_PI_GENERIC_TUNNEL_MAX_IN_TRANSFER_SIZE      0x0001
#define ATTRID_PI_GENERIC_TUNNEL_MAX_OUT_TRANSFER_SIZE     0x0002
#define ATTRID_PI_GENERIC_TUNNEL_PROTOCOL_ADDR             0x0003

  // The BACnet Protocol Tunnel cluster does not contain any attributes

  // Attributes of the 11073 Protocol Tunnel cluster
#define ATTRID_PI_11073_TUNNEL_DEVICEID_LIST               0x0000
#define ATTRID_PI_11073_TUNNEL_MANAGER_TARGET              0x0001
#define ATTRID_PI_11073_TUNNEL_MANAGER_ENDPOINT            0x0002
#define ATTRID_PI_11073_TUNNEL_CONNECTED                   0x0003
#define ATTRID_PI_11073_TUNNEL_PREEMPTIBLE                 0x0004
#define ATTRID_PI_11073_TUNNEL_IDLE_TIMEOUT                0x0005

  // Attributes of the Input, Output and Value (BACnet Reqular) cluster
#define ATTRID_IOV_BACNET_REG_CHANGE_OF_STATE_CNT          0x000F
#define ATTRID_IOV_BACNET_REG_CHANGE_OF_STATE_TIME         0x0010
#define ATTRID_IOV_BACNET_REG_COV_INCREMENT                0x0016
#define ATTRID_IOV_BACNET_REG_DEVICE_TYPE                  0x001F
#define ATTRID_IOV_BACNET_REG_ELAPSED_ACT_TIME             0x0021
#define ATTRID_IOV_BACNET_REG_FEEDBACK_VALUE               0x0028
#define ATTRID_IOV_BACNET_REG_OBJECT_ID                    0x004B
#define ATTRID_IOV_BACNET_REG_OBJECT_NAME                  0x004D
#define ATTRID_IOV_BACNET_REG_OBJECT_TYPE                  0x004F
#define ATTRID_IOV_BACNET_REG_UPDATE_INT                   0x0076
#define ATTRID_IOV_BACNET_REG_TIME_OF_AT_RESET             0x0072
#define ATTRID_IOV_BACNET_REG_TIME_OF_SC_RESET             0x0073
#define ATTRID_IOV_BACNET_REG_PROFILE_NAME                 0x00A8

  // Attributes of the Input, Output and Value (BACnet Extended) cluster
#define ATTRID_IOV_BACNET_EXT_ACKED_TRANSIT                0x0000
#define ATTRID_IOV_BACNET_EXT_ALARM_VALUE                  0x0006
//#define ATTRID_IOV_BACNET_EXT_ALARM_VALUES                 0x0006
#define ATTRID_IOV_BACNET_EXT_NOTIFY_CLASS                 0x0011
#define ATTRID_IOV_BACNET_EXT_DEADBAND                     0x0019
#define ATTRID_IOV_BACNET_EXT_EVENT_ENABLE                 0x0023
#define ATTRID_IOV_BACNET_EXT_EVENT_STATE                  0x0024
#define ATTRID_IOV_BACNET_EXT_FAULT_VALUES                 0x0025
#define ATTRID_IOV_BACNET_EXT_HI_LIMIT                     0x002D
#define ATTRID_IOV_BACNET_EXT_LIMIT_ENABLE                 0x0034
#define ATTRID_IOV_BACNET_EXT_LO_LIMIT                     0x003B
#define ATTRID_IOV_BACNET_EXT_NOTIFY_TYPE                  0x0048
#define ATTRID_IOV_BACNET_EXT_TIME_DELAY                   0x0071
#define ATTRID_IOV_BACNET_EXT_EV_TIME_STAMPS               0x0082

/*** Connect Control field: 8-bit bitmap ***/
// Preemptible (bit 1) - indicates whether or not this connection can be
// removed by a different Data Management device.
#define CONNECT_CTRL_PREEMPTIBLE_BIT                       0x01

/*** Connect status values ***/
#define CONNECT_STATUS_DISCONNECTED                        0x00
#define CONNECT_STATUS_CONNECTED                           0x01
#define CONNECT_STATUS_NOT_AUTHORIZED                      0x02
#define CONNECT_STATUS_RECONNECT_REQ                       0x03
#define CONNECT_STATUS_ALREADY_CONNECTED                   0x04

/*** AckedTransitions attribute bits ***/
#define ACKED_TRANSIT_TO_OFFNORMAL                         0x01
#define ACKED_TRANSIT_TO_FAULT                             0x02
#define ACKED_TRANSIT_TO_NORMAL                            0x04

/*** EventState attribute values ***/
#define EVENT_STATE_NORMAL                                 0x00
#define EVENT_STATE_FAULT                                  0x01
#define EVENT_STATE_OFFNORMAL                              0x02
#define EVENT_STATE_HIGH_LIMIT                             0x03
#define EVENT_STATE_LOW_LIMIT                              0x04

/*** NotifyType attribute values ***/
#define NORIFY_TYPE_EVENTS                                 0x00
#define NORIFY_TYPE_ALARMS                                 0x01

/*************************************************/
/***    Protocol Interface Cluster Commands    ***/
/*************************************************/
  // Command IDs for the Generic Tunnel Cluster
#define COMMAND_PI_GENERIC_TUNNEL_MATCH_PROTOCOL_ADDR      0x00

#define COMMAND_PI_GENERIC_TUNNEL_MATCH_PROTOCOL_ADDR_RSP  0x00
#define COMMAND_PI_GENERIC_TUNNEL_ADVERTISE_PROTOCOL_ADDR  0x01

  // Command IDs for the BACnet Protocol Tunnel Cluster
#define COMMAND_PI_BACNET_TUNNEL_TRANSFER_NPDU             0x00

  // Command IDs for the 11073 Protocol Tunnel Cluster
#define COMMAND_PI_11073_TUNNEL_TRANSFER_APDU              0x00
#define COMMAND_PI_11073_TUNNEL_CONNECT_REQ                0x01
#define COMMAND_PI_11073_TUNNEL_DISCONNECT_REQ             0x02
#define COMMAND_PI_11073_TUNNEL_CONNECT_STATUS_NOTI        0x03

/************************************************************************************
 * MACROS
 */


/****************************************************************************
 * TYPEDEFS
 */
/*** Structures used for callback functions ***/

/*** ZCL Generic Tunnel Cluster: Match Protocol Address command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint8 seqNum;           // sequence number received with command
  uint8 len;              // length of address protocol
  uint8 *protocolAddr;    // protocol address
} zclPIMatchProtocolAddr_t;

/*** ZCL Generic Tunnel Cluster: Match Protocol Address Response ***/
typedef struct
{
  afAddrType_t *srcAddr;  // responder's address
  uint8 *ieeeAddr;        // device address
  uint8 len;              // length of address protocol
  uint8 *protocolAddr;    // protocol address
} zclPIMatchProtocolAddrRsp_t;

/*** ZCL Generic Tunnel Cluster: Advertise Protocol Address command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint8 len;              // length of address protocol
  uint8 *protocolAddr;    // protocol address
} zclPIAdvertiseProtocolAddr_t;

/*** ZCL BACnet Protocol Tunnel Cluster: Transfer NPDU command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint16 len;             // length of BACnet NPDU
  uint8 *npdu;            // BACnet NPDU
} zclBACnetTransferNPDU_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Transfer APDU command ***/
typedef struct
{
  afAddrType_t *srcAddr;  // requestor's address
  uint16 len;             // length of 11073 APDU
  uint8 *apdu;            // 11073 APDU
} zcl11073TransferAPDU_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Connect Request command ***/
typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8 seqNum;          // sequence number received with command
  uint8 connectCtrl;     // connect control
  uint16 idleTimeout;    // inactivity time (in minutes) which Data Management device
                         // will wait w/o receiving any data before it disconnects
  uint8 *managerAddr;    // IEEE address (64-bit) of Data Management device
                         // transmitting this frame
  uint8 managerEP;       // source endpoint from which Data Management device is
                         // transmitting this frame
} zcl11073ConnectReq_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Disconnect Request command ***/
typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8 seqNum;          // sequence number received with command
  uint8 *managerAddr;    // IEEE address (64-bit) of Data Management device
                         // transmitting this frame
} zcl11073DisconnectReq_t;

/*** ZCL 11073 Protocol Tunnel Cluster: Connect Status Notification command ***/
typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8 connectStatus;   // connect status
} zcl11073ConnectStatusNoti_t;

// This callback is called to process a Match Protocol Address command
typedef void (*zclPICB_MatchProtocolAddr_t)( zclPIMatchProtocolAddr_t *pCmd );

// This callback is called to process a Match Protocol Address response
typedef void (*zclPICB_MatchProtocolAddrRsp_t)( zclPIMatchProtocolAddrRsp_t *pRsp );

// This callback is called to process a Advertise Protocol Address command
typedef void (*zclPICB_AdvertiseProtocolAddr_t)( zclPIAdvertiseProtocolAddr_t *pCmd );

// This callback is called to process a BACnet Transfer NPDU command
typedef void (*zclPICB_BACnetTransferNPDU_t)( zclBACnetTransferNPDU_t *pCmd );

// This callback is called to process an 11037 Transfer APDU command
typedef void (*zclPICB_11073TransferAPDU_t)( zcl11073TransferAPDU_t *pCmd );

// This callback is called to process an 11037 Connect Request command
typedef void (*zclPICB_11073ConnectReq_t)( zcl11073ConnectReq_t *pCmd );

// This callback is called to process an 11037 Disconnect Request command
typedef void (*zclPICB_11073DisconnectReq_t)( zcl11073DisconnectReq_t *pCmd );

// This callback is called to process an 11037 Connect Status Notification command
typedef void (*zclPICB_11073ConnectStatusNoti_t)( zcl11073ConnectStatusNoti_t *pCmd );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclPICB_MatchProtocolAddr_t      pfnPI_MatchProtocolAddr;
  zclPICB_MatchProtocolAddrRsp_t   pfnPI_MatchProtocolAddrRsp;
  zclPICB_AdvertiseProtocolAddr_t  pfnPI_AdvertiseProtocolAddr;
  zclPICB_BACnetTransferNPDU_t     pfnPI_BACnetTransferNPDU;
  zclPICB_11073TransferAPDU_t      pfnPI_11073TransferAPDU;
  zclPICB_11073ConnectReq_t        pfnPI_11073ConnectReq;
  zclPICB_11073DisconnectReq_t     pfnPI_11073DisconnectReq;
  zclPICB_11073ConnectStatusNoti_t pfnPI_11073ConnectStatusNoti;
} zclPI_AppCallbacks_t;

/*********************************************************************
 * FUNCTION MACROS
 */

/*
 *  Send a BACnet Transfer NPDU Command. This command is used when a
 *  BACnet network layer wishes to transfer a BACnet NPDU across a
 *  ZigBee tunnel to another BACnet network layer.
 *
 *  Use like: ZStatus_t zclPI_Send_BACnetTransferNPDUCmd( uint16 srcEP, afAddrType_t *dstAddr,
 *                                                        uint16 len, uint8 *npdu,
 *                                                        uint8 disableDefaultRsp, uint8 seqNum );
 *  @param   srcEP - Sending application's endpoint
 *  @param   dstAddr - where you want the message to go
 *  @param   len - length of NPDU
 *  @param   npdu - pointer to NPDU to be sent
 *  @param   disableDefaultRsp - whether to disable the Default Response command
 *  @param   seqNum - sequence number
 *
 *  @return  ZStatus_t
 */
#define zclPI_Send_BACnetTransferNPDUCmd(a,b,c,d,e,f) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_PI_BACNET_PROTOCOL_TUNNEL,\
                                                                       COMMAND_PI_BACNET_TUNNEL_TRANSFER_NPDU, TRUE,\
                                                                       ZCL_FRAME_CLIENT_SERVER_DIR, (e), 0, (f), (c), (d) )
/*
 *  Call to send out an 11073 Disconnect Request Command. This command
 *  is generated when a Data Management device wishes to disconnect a
 *  tunnel connection existing on an agent device.
 *
 *  Use Like: ZStatus_t zclPI_Send_11073DisconnectReq( uint8 srcEP, afAddrType_t *dstAddr,
 *                                                     uint8 *managerAddr,
 *                                                     uint8 disableDefaultRsp, uint8 seqNum );
 *  @param   srcEP - Sending application's endpoint
 *  @param   dstAddr - where you want the message to go
 *  @param   managerAddr - IEEE address (64-bit) of Data Management device
                           transmitting this frame
 *  @param   disableDefaultRsp - whether to disable the Default Response command
 *  @param   seqNum - sequence number
 *
 *  @return  ZStatus_t
 */
#define zclPI_Send_11073DisconnectReq(a,b,c,d,e) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL,\
                                                                  COMMAND_PI_11073_TUNNEL_DISCONNECT_REQ, TRUE,\
                                                                  ZCL_FRAME_CLIENT_SERVER_DIR, (d), 0, (e),\
                                                                  Z_EXTADDR_LEN, (c) )
/*
 *  Call to send out an 11073 Connect Status Notification Command. This
 *  command is generated by an agent device in response to a connect
 *  request command, disconnect command, or in response to some other
 *  event that causes the tunnel to become connected or disconnected.
 *  It is also sent by the agent device to request the Data Management
 *  device to reconnect a tunnel.
 *
 *  Use Like: ZStatus_t zclPI_Send_11073ConnectStatusNoti( uint8 srcEP, afAddrType_t *dstAddr,
 *                                                         uint8 connectStatus, uint8 disableDefaultRsp,
 *                                                         uint8 seqNum );
 *  @param   srcEP - Sending application's endpoint
 *  @param   dstAddr - where you want the message to go
 *  @param   connectStatus - connect status
 *  @param   disableDefaultRsp - whether to disable the Default Response command
 *  @param   seqNum - sequence number
 *
 *  @return  ZStatus_t
 */
#define zclPI_Send_11073ConnectStatusNoti(a,b,c,d,e) zcl_SendCommand( (a), (b), ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL,\
                                                                      COMMAND_PI_11073_TUNNEL_CONNECT_STATUS_NOTI, TRUE,\
                                                                      ZCL_FRAME_SERVER_CLIENT_DIR, (d), 0, (e), 1, &(c) )

/****************************************************************************
 * VARIABLES
 */


/****************************************************************************
 * FUNCTIONS
 */

 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclPI_RegisterCmdCallbacks( uint8 endpoint, zclPI_AppCallbacks_t *callbacks );

/*
 * Send out a Match Protocol Address Command
 */
extern ZStatus_t zclPI_Send_MatchProtocolAddrCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint8 len, uint8 *protocolAddr,
                                                  uint8 disableDefaultRsp, uint8 seqNum );
/*
 *  Send a Match Protocol Address Response
*/
extern ZStatus_t zclPI_Send_MatchProtocolAddrRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint8 *ieeeAddr, uint8 len, uint8 *protocolAddr,
                                                  uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Send out an Advertise Protocol Address Command
 */
extern ZStatus_t zclPI_Send_AdvertiseProtocolAddrCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint8 len, uint8 *protocolAddr,
                                                      uint8 disableDefaultRsp, uint8 seqNum );
/*
 * Send out an 11073 Transfer APDU Command
 */
extern ZStatus_t zclPI_Send_11073TransferAPDUCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                                  uint16 len, uint8 *apdu, uint8 seqNum );
/*
 * Send out an 11073 Connect Request Command
 */
extern ZStatus_t zclPI_Send_11073ConnectReq( uint8 srcEP, afAddrType_t *dstAddr,
                                             uint8 connectCtrl, uint16 idleTimeout,
                                             uint8 *managerAddr, uint8 managerEP,
                                             uint8 disableDefaultRsp, uint8 seqNum );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_PI_H */
