/**************************************************************************************************
  Filename:       zcl_diagnostic.h
  Revised:        $Date: 2014-03-13 11:21:25 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37667 $

  Description:    This file contains the ZCL Diagnostic definitions.


  Copyright 2013-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_DIAGNOSTIC_H
#define ZCL_DIAGNOSTIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZCL_DIAGNOSTIC

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"


/******************************************************************************
 * CONSTANTS
 */

/***************************************/
/***  Diagnostic Cluster Attributes ***/
/***************************************/

// Server Attributes
#define ATTRID_DIAGNOSTIC_NUMBER_OF_RESETS                            0x0000  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES                    0x0001  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_MAC_RX_BCAST                                0x0100  // O, R, UINT32
#define ATTRID_DIAGNOSTIC_MAC_TX_BCAST                                0x0101  // O, R, UINT32
#define ATTRID_DIAGNOSTIC_MAC_RX_UCAST                                0x0102  // O, R, UINT32
#define ATTRID_DIAGNOSTIC_MAC_TX_UCAST                                0x0103  // O, R, UINT32
#define ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY                          0x0104  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL                           0x0105  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_RX_BCAST                                0x0106  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_TX_BCAST                                0x0107  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_RX_UCAST                                0x0108  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_TX_UCAST_SUCCESS                        0x0109  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_TX_UCAST_RETRY                          0x010A  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_TX_UCAST_FAIL                           0x010B  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_ROUTE_DISC_INITIATED                        0x010C  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_NEIGHBOR_ADDED                              0x010D  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_NEIGHBOR_REMOVED                            0x010E  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_NEIGHBOR_STALE                              0x010F  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_JOIN_INDICATION                             0x0110  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_CHILD_MOVED                                 0x0111  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_NWK_FC_FAILURE                              0x0112  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_FC_FAILURE                              0x0113  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_UNAUTHORIZED_KEY                        0x0114  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_NWK_DECRYPT_FAILURES                        0x0115  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_APS_DECRYPT_FAILURES                        0x0116  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES             0x0117  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_RELAYED_UCAST                               0x0118  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED              0x0119  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT                  0x011A  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT      0x011B  // O, R, UINT16
#define ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI                            0x011C  // O, R, UINT8
#define ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI                           0x011D  // O, R, INT8

// Server Attribute Defaults
#define ATTR_DEFAULT_DIAGNOSTIC_NUMBER_OF_RESETS                            0
#define ATTR_DEFAULT_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES                    0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_RX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_RX_UCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_UCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_UCAST_RETRY                          0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_UCAST_FAIL                           0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_RX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_RX_UCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_UCAST_SUCCESS                        0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_UCAST_RETRY                          0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_UCAST_FAIL                           0
#define ATTR_DEFAULT_DIAGNOSTIC_ROUTE_DISC_INITIATED                        0
#define ATTR_DEFAULT_DIAGNOSTIC_NEIGHBOR_ADDED                              0
#define ATTR_DEFAULT_DIAGNOSTIC_NEIGHBOR_REMOVED                            0
#define ATTR_DEFAULT_DIAGNOSTIC_NEIGHBOR_STALE                              0
#define ATTR_DEFAULT_DIAGNOSTIC_JOIN_INDICATION                             0
#define ATTR_DEFAULT_DIAGNOSTIC_CHILD_MOVED                                 0
#define ATTR_DEFAULT_DIAGNOSTIC_NWK_FC_FAILURE                              0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_FC_FAILURE                              0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_UNAUTHORIZED_KEY                        0
#define ATTR_DEFAULT_DIAGNOSTIC_NWK_DECRYPT_FAILURES                        0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_DECRYPT_FAILURES                        0
#define ATTR_DEFAULT_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES             0
#define ATTR_DEFAULT_DIAGNOSTIC_RELAYED_UCAST                               0
#define ATTR_DEFAULT_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED              0
#define ATTR_DEFAULT_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT                  0
#define ATTR_DEFAULT_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT      0
#define ATTR_DEFAULT_DIAGNOSTIC_LAST_MESSAGE_LQI                            0
#define ATTR_DEFAULT_DIAGNOSTIC_LAST_MESSAGE_RSSI                           0

/*******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */
extern uint8 zclDiagnostic_InitStats( void );

extern uint32 zclDiagnostic_ClearStats( bool clearNV );

extern ZStatus_t zclDiagnostic_GetStatsAttr( uint16 attributeId, uint32 *attrValue, uint16 *dataLen );

extern ZStatus_t zclDiagnostic_ReadWriteAttrCB( uint16 clusterId, uint16 attrId, uint8 oper,
                                                uint8 *pValue, uint16 *pLen );

extern uint8 zclDiagnostic_RestoreStatsFromNV( void );

extern uint32 zclDiagnostic_SaveStatsToNV( void );

/*
 * Register for callbacks from this cluster library
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ZCL_DIAGNOSTIC

#endif /* ZCL_DIAGNOSTIC_H */
