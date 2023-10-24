/**************************************************************************************************
  Filename:       bdb_touchlink.h
  Revised:        $Date: 2013-07-15 15:29:01 -0700 (Mon, 15 Jul 2013) $
  Revision:       $Revision: 34720 $

  Description:    This file contains the BDB TouchLink Initiator definitions.


  Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef TOUCHLINK_INITIATOR_H
#define TOUCHLINK_INITIATOR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "bdb_tlCommissioning.h"

/*********************************************************************
 * CONSTANTS
 */
// time interval (in msec) between target selection and configuration,
// to allow target visual identification by the user.
#define TOUCHLINK_INITIATOR_IDENTIFY_INTERVAL          500

// If defined, the initiator will determine new TouchLink network parameters.
// Otherwise, it will be determind by the target starting the network.
//#define TOUCHLINK_INITIATOR_SET_NEW_NWK_PARAMS

#define TOUCHLINK_NWK_DISC_CNF_EVT                               0x0001
#define TOUCHLINK_NWK_JOIN_IND_EVT                               0x0002
#define TOUCHLINK_START_NWK_EVT                                  0x0004
#define TOUCHLINK_JOIN_NWK_ATTEMPT_EVT                           0x0008
#define TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT                     0x0010
#define TOUCHLINK_TL_SCAN_BASE_EVT                               0x0020
#define TOUCHLINK_CFG_TARGET_EVT                                 0x0040
#define TOUCHLINK_W4_NWK_START_RSP_EVT                           0x0080
#define TOUCHLINK_W4_NWK_JOIN_RSP_EVT                            0x0100
#define TOUCHLINK_DISABLE_RX_EVT                                 0x0200
#define TOUCHLINK_W4_REJOIN_EVT                                  0x0400
#define TOUCHLINK_NOTIFY_APP_EVT                                 0x0800
#define TOUCHLINK_NWK_RTR_START_EVT                              0x1000
#define TOUCHLINK_NWK_FORMATION_SUCCESS_EVT                      0x2000
  
/*********************************************************************
 * TYPEDEFS
 */
// This callback is called to notify the application when a target device is
// successfully touch-linked.
typedef ZStatus_t (*touchLink_NotifyAppTLCB_t)( epInfoRec_t *pData );

// This callback is called to decide whether to select a device, which responded to scan, as a target.
// Note newScanRsp value should be copied if used beyond the call scope.
typedef uint8 (*touchLink_SelectDiscDevCB_t)( const bdbTLScanRsp_t *newScanRsp, int8 newRssi );

/*********************************************************************
 * MACROS
 */

// TOUCHLINK Commissioning Utility commands initiation enablement
#define TOUCHLINK_UTILITY_SEND_EPINFO_ENABLED
#define TOUCHLINK_UTILITY_SEND_GETEPLIST_ENABLED
#define TOUCHLINK_UTILITY_SEND_GETGRPIDS_ENABLED

/*********************************************************************
 * VARIABLES
 */
extern bdbTLDeviceInfo_t touchLinkSampleRemote_DeviceInfo;

/*********************************************************************
 * FUNCTIONS
 */

/*-------------------------------------------------------------------
 * TASK API - These functions must only be called by OSAL.
 */

/*
 * Initialization for the TouchLink Initiator task
 */
void touchLinkInitiator_Init( uint8 task_id );

/*
 *  Event Process for the TouchLink Initiator task
 */
uint16 touchLinkInitiator_event_loop( uint8 task_id, uint16 events );

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/

/*
 * Start the TouchLink Initiator device in the network
 */
ZStatus_t touchLinkInitiator_InitDevice( void );

/*
 * Register application task to receive unprocessed messages
 */
ZStatus_t touchLinkInitiator_RegisterForMsg( uint8 taskId );

/*
 * Register an Application's Touch-Link Notify callback function
 */
void touchLinkInitiator_RegisterNotifyTLCB( touchLink_NotifyAppTLCB_t pfnNotifyApp );

/*
 * Register an Application's Selection callback function
 */
void touchLinkInitiator_RegisterSelectDiscDevCB( touchLink_SelectDiscDevCB_t pfnSelectDiscDev );

/*
 * Start Touch-Link device discovery
 */
ZStatus_t touchLinkInitiator_StartDevDisc( void );

/*
 * Abort Touch-Link
 */
ZStatus_t touchLinkInitiator_AbortTL( void );

/*
 * Change Channel for Frequency Agility
 */
ZStatus_t touchLinkInitiator_ChannelChange( uint8 targetChannel );

/*
 * Send EP Info
 */
ZStatus_t touchLinkInitiator_SendEPInfo( uint8 srcEP, afAddrType_t *dstAddr, uint8 seqNum);

/*
 * Send Reset to FN to the selected target of the current TL transaction
 */
ZStatus_t touchLinkInitiator_ResetToFNSelectedTarget( void );

#if (ZSTACK_ROUTER_BUILD)
/*
 * Set the router permit join flag
 */
ZStatus_t touchLinkInitiator_PermitJoin( uint8 duration );
#endif

/*
 * @brief   Select a unique PAN ID and Extended PAN ID when compared to
 *          the PAN IDs and Extended PAN IDs of the networks detected
 *          on the TOUCHLINK channels. The selected Extended PAN ID must be
 *          a random number (and not equal to our IEEE address).
 */
void initiatorSelectNwkParams( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* TOUCHLINK_INITIATOR_H */
