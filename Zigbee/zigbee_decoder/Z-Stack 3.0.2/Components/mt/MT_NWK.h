/**************************************************************************************************
  Filename:       MT_NWK.h
  Revised:        $Date: 2010-02-04 14:28:44 -0800 (Thu, 04 Feb 2010) $
  Revision:       $Revision: 21656 $

  Description:    MonitorTest functions for the NWK layer.


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


/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/

#include "hal_types.h"
#include "NLMEDE.h"

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/
#define NWKCB_CHECK(cbi) (_nwkCallbackSub & (cbi))

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
/* NWK Callback subscription IDs */
#define CB_ID_NLDE_DATA_CNF          0x0001
#define CB_ID_NLDE_DATA_IND          0x0002
#define CB_ID_NLME_INIT_COORD_CNF    0x0004
#define CB_ID_NLME_JOIN_CNF          0x0008
#define CB_ID_NLME_JOIN_IND          0x0010
#define CB_ID_NLME_LEAVE_CNF         0x0020
#define CB_ID_NLME_LEAVE_IND         0x0040
#define CB_ID_NLME_POLL_CNF          0x0080
#define CB_ID_NLME_SYNC_IND          0x0200
#define CB_ID_NLME_NWK_DISC_CNF      0x2000
#define CB_ID_NLME_START_ROUTER_CNF	 0x8000

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
extern uint16 _nwkCallbackSub;

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

#ifdef MT_NWK_FUNC
/*
 *   Process all the NWK commands that are issued by test tool
 */
extern uint8 MT_NwkCommandProcessing (byte *pBuf);

#endif   /* NWK Command Processing in MT */

#ifdef MT_NWK_CB_FUNC
/*
 * Process the callback subscription for NLDE-DATA.confirm
 */
extern void nwk_MTCallbackSubDataConfirm(byte nsduHandle, ZStatus_t status);

/*
 * Process the callback subscription for NLDE-DATA.indication
 */
extern void nwk_MTCallbackSubDataIndication(uint16 SrcAddress, int16 nsduLength,
                                            byte *nsdu, byte LinkQuality);

/*
 * Process the callback subscription for NLME-INIT-COORD.confirm
 */
extern void nwk_MTCallbackSubInitCoordConfirm(ZStatus_t Status);

/*
 * Process the callback subscription for NLME-START-ROUTER.confirm
 */
extern void nwk_MTCallbackSubStartRouterConfirm(ZStatus_t Status);

/*
 * Process the callback subscription for NLME_NWK-DISC.confirm
 */
extern void nwk_MTCallbackSubNetworkDiscoveryConfirm(byte ResultCount,	networkDesc_t *NetworkList);

/*
 * Process the callback subscription for NLME-JOIN.confirm
 */
extern void nwk_MTCallbackSubJoinConfirm(uint16 PanId, ZStatus_t Status);

/*
 * Process the callback subscription for NLME-INIT-COORD.indication
 */
extern void nwk_MTCallbackSubJoinIndication(uint16 ShortAddress, byte *ExtendedAddress,
                                            byte CapabilityFlags);

/*
 * Process the callback subscription for NLME-LEAVE.confirm
 */
extern void nwk_MTCallbackSubLeaveConfirm(byte *DeviceAddress, ZStatus_t Status);

/*
 * Process the callback subscription for NLME-LEAVE.indication
 */
extern void nwk_MTCallbackSubLeaveIndication(byte *DeviceAddress);

/*
 *  Process the callback subscription for NLME-SYNC.indication
 */
extern void nwk_MTCallbackSubSyncIndication(void);

/*
 *  Process the callback subscription for NLME-POLL.confirm
 */
extern void nwk_MTCallbackSubPollConfirm(byte status);

#endif   /*NWK Callback Processing in MT*/
/*
 * Process the callback for Ping
 *
 */
extern void nwk_MTCallbackPingConfirm(uint16 DstAddress, byte pingSeqNo,
                                      uint16 delay, byte routeCnt, byte *routeAddr);

/*********************************************************************
*********************************************************************/
