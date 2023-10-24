/**************************************************************************************************
  Filename:       ZDNwkMgr.h
  Revised:        $Date: 2007-07-18 10:02:49 -0700 (Wed, 18 Jul 2007) $
  Revision:       $Revision: 14945 $

  Description:    This file contains the interface to the ZigBee Network Manager.


  Copyright 2007-2010 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZDNWKMGR_H
#define ZDNWKMGR_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "nwk_globals.h"
#include "nwk_util.h"
#include "ZDApp.h"

/******************************************************************************
 * CONSTANTS
 */

// Network Manager Role
#define ZDNWKMGR_DISABLE                  0x00
#define ZDNWKMGR_ENABLE                   0x01

// Energy level threshold
#define ZDNWKMGR_ACCEPTABLE_ENERGY_LEVEL  0x1E

// Minimum transmissions attempted for Channel Interference detection
#if !defined ( ZDNWKMGR_MIN_TRANSMISSIONS )
  #define ZDNWKMGR_MIN_TRANSMISSIONS      20
#endif

// Minimum transmit failure rate for Channel Interference detection
#define ZDNWKMGR_CI_TX_FAILURE            25

// Minimum transmit failure rate for Channel Change
#define ZDNWKMGR_CC_TX_FAILURE            50

// Min and Max Scan Counts for Update Request
#define ZDNWKMGR_MIN_SCAN_COUNT           0
#define ZDNWKMGR_MAX_SCAN_COUNT           5

// Update Request and Notify timers
#define ZDNWKMGR_UPDATE_NOTIFY_TIMER      60  // 1(h) * 60(m)
#define ZDNWKMGR_UPDATE_REQUEST_TIMER     60  // 1(h) * 60(m)

// Network Manager Events
#define ZDNWKMGR_CHANNEL_CHANGE_EVT       0x0001
#define ZDNWKMGR_UPDATE_NOTIFY_EVT        0x0002
#define ZDNWKMGR_UPDATE_REQUEST_EVT       0x0004
#define ZDNWKMGR_SCAN_REQUEST_EVT         0x0008

#define ZDNWKMGR_BCAST_DELIVERY_TIME      ( _NIB.BroadcastDeliveryTime * 100 )

/*********************************************************************
 * TYPEDEFS
 */

// Used for Management Network Update Request message
typedef struct
{
  osal_event_hdr_t hdr;
  uint8 transSeq;
  uint16 srcAddr;
  uint32 channelMask;
  uint8 scanDuration;
  uint8 scanCount;
  int16 nwkManagerAddr;
  uint8 wasBroadcast;
} ZDNwkMgr_MgmtNwkUpdateRequest_t;

// Used for Management Network Update Notify command
typedef struct
{
  osal_event_hdr_t hdr;
  uint16 srcAddr;
  uint8 status;
  uint32 scannedChannels;
  uint16 totalTransmissions;
  uint16 txFailures;
  uint8 listCount;
  uint8 *energyValues;
} ZDNwkMgr_MgmtNwkUpdateNotify_t;

// Used for Channel Interference message
typedef struct
{
  osal_event_hdr_t hdr;
  uint16 totalTransmissions;
  uint16 txFailures;
} ZDNwkMgr_ChanInterference_t;

// Used for ED Scan Confirm message
typedef struct
{
  osal_event_hdr_t hdr;
  uint8 status;
  uint32 scannedChannels;
  uint8 energyDetectList[ED_SCAN_MAXCHANNELS];
} ZDNwkMgr_EDScanConfirm_t;

// Used for Network Report command
typedef struct
{
  osal_event_hdr_t hdr;
  uint16 srcAddr;
  uint8  reportType;
  uint8  EPID[Z_EXTADDR_LEN];
  uint8  reportInfoCnt;
  uint16 panIDs[];
} ZDNwkMgr_NetworkReport_t;

// Used for Network Update command
typedef struct
{
  osal_event_hdr_t hdr;
  uint8  updateType;
  uint8  updateInfoCnt;
  uint16 newPanID;
} ZDNwkMgr_NetworkUpdate_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte ZDNwkMgr_TaskID;

/******************************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * Task Level Control
 */
/*
 * ZDNwkMgr Task Initialization Function
 */
extern void ZDNwkMgr_Init( byte task_id );

/*
 * ZDNwkMgr Task Event Processing Function
 */
extern UINT16 ZDNwkMgr_event_loop( byte task_id, UINT16 events );

/*********************************************************************
 * Application Level Functions
 */
// Frequency Agility functions
extern void (*pZDNwkMgr_EDScanConfirmCB)( NLME_EDScanConfirm_t *EDScanConfirm );
extern void (*pZDNwkMgr_ProcessDataConfirm)( afDataConfirm_t *afDataConfirm );
extern void (*pZDNwkMgr_ReportChannelInterference)( NLME_ChanInterference_t *chanInterference );

// PAN ID Conflict functions
extern void (*pZDNwkMgr_NetworkReportCB)( ZDNwkMgr_NetworkReport_t *pReport );
extern void (*pZDNwkMgr_NetworkUpdateCB)( ZDNwkMgr_NetworkUpdate_t *pUpdate );

#if defined ( NWK_MANAGER )
/*
 * ZDNwkMgr set the local device as the Network Manager
 */
extern void NwkMgr_SetNwkManager( void );
#endif

/******************************************************************************
******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDNWKMGR_H */
