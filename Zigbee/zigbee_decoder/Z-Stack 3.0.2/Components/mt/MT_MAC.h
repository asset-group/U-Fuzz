/**************************************************************************************************
  Filename:       MT_MAC.h
  Revised:        $Date: 2013-06-11 11:14:41 -0700 (Tue, 11 Jun 2013) $
  Revision:       $Revision: 34520 $

  Description:    MonitorTest functions for the MAC layer.


  Copyright 2004-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MT_MAC_H
#define MT_MAC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "ZMAC.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#if defined (MT_MAC_CB_FUNC)
  //MAC Callback subscription IDs
  #define CB_ID_NWK_SYNC_LOSS_IND         0x0001
  #define CB_ID_NWK_ASSOCIATE_IND         0x0002
  #define CB_ID_NWK_ASSOCIATE_CNF         0x0004
  #define CB_ID_NWK_BEACON_NOTIFY_IND     0x0008
  #define CB_ID_NWK_DATA_CNF              0x0010
  #define CB_ID_NWK_DATA_IND              0x0020
  #define CB_ID_NWK_DISASSOCIATE_IND      0x0040
  #define CB_ID_NWK_DISASSOCIATE_CNF      0x0080
  #define CB_ID_NWK_PURGE_CNF             0x0100
  #define CB_ID_NWK_ORPHAN_IND            0x0400
  #define CB_ID_NWK_POLL_CNF              0x0800
  #define CB_ID_NWK_SCAN_CNF              0x1000
  #define CB_ID_NWK_COMM_STATUS_IND       0x2000
  #define CB_ID_NWK_START_CNF             0x4000
  #define CB_ID_NWK_RX_ENABLE_CNF         0x8000
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint16 _macCallbackSub;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
/*********************************************************************
 * LOCAL FUNCTIONS
 */

#ifdef MT_MAC_FUNC

/*
 * MonitorTest function handling MAC commands
 */
extern uint8 MT_MacCommandProcessing( uint8 *pBuf );

#endif   /*MAC Command Processing in MT*/


#if defined ( MT_MAC_CB_FUNC )

/*
 *  Process the callback subscription for nwk_associate_ind
 */
extern void nwk_MTCallbackSubNwkAssociateInd( ZMacAssociateInd_t *param );

/*
 *  Process the callback subscription for nwk_associate_cnf
 */
extern void nwk_MTCallbackSubNwkAssociateCnf( ZMacAssociateCnf_t *param );

/*
 *  Process the callback subscription for nwk_data_cnf
 */
extern void nwk_MTCallbackSubNwkDataCnf( ZMacDataCnf_t *param );

/*
 *  Process the callback subscription for nwk_data_ind
 */
extern void nwk_MTCallbackSubNwkDataInd( ZMacDataInd_t *param );


/*
 * Process the callback subscription for nwk_disassociate_ind
 */
extern void nwk_MTCallbackSubNwkDisassociateInd( ZMacDisassociateInd_t *param );

/*
 *  Process the callback subscription for nwk_disassociate_cnf
 */
extern void nwk_MTCallbackSubNwkDisassociateCnf( ZMacDisassociateCnf_t *param );

/*
 *  Process the callback subscription for nwk_poll_ind
 */
extern void nwk_MTCallbackSubNwkPollInd( ZMacPollInd_t *param );

/*
 *  Process the callback subscription for nwk_orphan_ind
 */
extern void nwk_MTCallbackSubNwkOrphanInd( ZMacOrphanInd_t *param );

/*
 *  Process the callback subscription for nwk_poll_cnf
 */
extern void nwk_MTCallbackSubNwkPollCnf( byte Status );

/*
 *  Process the callback subscription for nwk_scan_cnf
 */
extern void nwk_MTCallbackSubNwkScanCnf( ZMacScanCnf_t *param );

/*
 *  Process the callback subscription for nwk_start_cnf
 */
extern void nwk_MTCallbackSubNwkStartCnf( uint8 Status );

/*
 *  Process the callback subscription for nwk_syncloss_ind
 */
extern void nwk_MTCallbackSubNwkSyncLossInd( ZMacSyncLossInd_t *param );

/*
 *  Process the callback subscription for nwk_Rx_Enable_cnf
 */
extern void nwk_MTCallbackSubNwkRxEnableCnf ( byte Status );

/*
 *  Process the callback subscription for nwk_Comm_Status_ind
 */
extern void nwk_MTCallbackSubCommStatusInd ( ZMacCommStatusInd_t *param );

/*
 *  Process the callback subscription for nwk_Purge_cnf
 */
extern void nwk_MTCallbackSubNwkPurgeCnf( ZMacPurgeCnf_t *param );

/*
 *  Process the callback subscription for nwk_Beacon_Notify_ind
 */
extern void nwk_MTCallbackSubNwkBeaconNotifyInd ( ZMacBeaconNotifyInd_t *param );
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MT_MAC_H */
