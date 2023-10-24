/**************************************************************************************************
  Filename:       mac_data.h
  Revised:        $Date: 2014-11-06 11:03:55 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41021 $

  Description:    Internal interface file for the MAC data module.


  Copyright 2005-2011 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MAC_DATA_H
#define MAC_DATA_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "OSAL.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Internal frame type values. */
#define MAC_INTERNAL_BEACON           0
#define MAC_INTERNAL_DATA             1
#define MAC_INTERNAL_ASSOC_REQ        2
#define MAC_INTERNAL_ASSOC_RSP        3
#define MAC_INTERNAL_DISASSOC_NOTIF   4
#define MAC_INTERNAL_DATA_REQ         5
#define MAC_INTERNAL_PAN_CONFLICT     6
#define MAC_INTERNAL_ORPHAN_NOTIF     7
#define MAC_INTERNAL_BEACON_REQ       8
#define MAC_INTERNAL_COORD_REALIGN    9
#define MAC_INTERNAL_GTS_REQ          10
#define MAC_INTERNAL_ENHANCED_BEACON_REQ  11

/* Internal zero length data frame */
#define MAC_INTERNAL_ZERO_DATA        MAC_INTERNAL_DATA

/* Enhanced Beacon Filter IE Type */
#define MAC_INTERNAL_ENHANCED_BEACON_FILTER_IE_TYPE  0x01

/* Enhanced Beacon Filter IE ID Mask */
#define MAC_INTERNAL_ENHANCED_BEACON_FILTER_IE_ID_MASK    0x1E

/* Enhanced Beacon Filter IE ID value */
#define MAC_INTERNAL_ENHANCED_BEACON_FILTER_IE_ID  0x0002 /* Bits 1,2,3,4 are 
                                                             considered */
/* Additional byte used by low level in tx buffer */
#define MAC_TX_OFFSET_LEN             1

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Data info type */
typedef struct
{
  osal_msg_q_t        txQueue;          /* transmit data queue */
  osal_msg_q_t        rxQueue;          /* receive data queue */
  uint16              duration;         /* duration of current frame */
  uint8               indirectCount;    /* number of indirect frames in tx queue */
  uint8               directCount;      /* number of direct frames in tx queue */
  uint8               rxCount;          /* number of frames in rx queue */
} macData_t;

/* general purpose data handling function type */
typedef void (*macDataTxFcn_t)(macTx_t *pMsg);

/* critical beacon handling function */
typedef void (*macRxBeaconCritical_t)(macRx_t *pBuf);

/* beacon tx complete function */
typedef void (*macTxBeaconComplete_t)(uint8 status);

/* tx frame retransmit function */
typedef void (*macTxFrameRetransmit_t)(void);

/* tx frame check schedule function */
typedef uint8 (*macDataCheckSched_t)(void);

/* tx frame check tx time function */
typedef uint8 (*macDataCheckTxTime_t)(void);

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* TX frame success to event */
extern const uint8 CODE macTxFrameSuccess[];

/* TX frame failure to event */
extern const uint8 CODE macTxFrameFailed[];

/* mac_data data */
extern macData_t macData;

/* indirect data handling functions */
extern macDataTxFcn_t macDataTxIndirect;
extern macDataTxFcn_t macDataRequeueIndirect;

/* critical beacon handling function */
extern macRxBeaconCritical_t macDataRxBeaconCritical;

/* beacon tx complete function */
extern macTxBeaconComplete_t macDataTxBeaconComplete;

/* tx frame set schedule function */
extern macDataTxFcn_t macDataSetSched;

/* tx frame check schedule function */
extern macDataCheckSched_t macDataCheckSched;

/* tx frame check tx time function */
extern macDataCheckTxTime_t macDataCheckTxTime;

/* tx frame beacon requeue function */
extern macDataTxFcn_t macDataBeaconRequeue;

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macDataReset(void);
MAC_INTERNAL_API macTx_t *macAllocTxBuffer(uint8 cmd, macSec_t *sec);
MAC_INTERNAL_API uint8 macFrameDuration(uint8 len, uint16 txOptions);
MAC_INTERNAL_API void macDataRetransmit(void);
MAC_INTERNAL_API uint8 macBuildDataFrame(macEvent_t *pEvent);
MAC_INTERNAL_API uint8 macCheckSched(void);
MAC_INTERNAL_API void macSetSched(macTx_t *pMsg);
MAC_INTERNAL_API void macDataTxComplete(macTx_t *pMsg);
MAC_INTERNAL_API void macDataTxSend(void);
MAC_INTERNAL_API void macDataTxEnqueue(macTx_t *pMsg);
MAC_INTERNAL_API void macDataSend(macEvent_t *pEvent);
MAC_INTERNAL_API void macApiDataReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macDataRxInd(macEvent_t *pEvent);

#if defined (CC26XX)
MAC_INTERNAL_API void macAutoPendMaintainSrcMatchTable(macTx_t *pCurr);
MAC_INTERNAL_API void macAutoPendAddSrcMatchTableEntry(macTx_t *pMsg);
#endif /* CC26XX */   

/**************************************************************************************************
*/

#endif /* MAC_DATA_H */
