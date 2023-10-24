/**************************************************************************************************
  Filename:       MT_ZNP.h
  Revised:        $Date: 2011-08-22 16:18:12 -0700 (Mon, 22 Aug 2011) $
  Revision:       $Revision: 27245 $

  Description:    Declarations for the ZNP sub-module of the MT API.


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
#ifndef MT_ZNP_H
#define MT_ZNP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "comdef.h"
#include "nwk.h"
#include "ZDApp.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define MT_ZNP_EP_ID_LIST_MAX               4
#define MT_ZNP_ZDO_MSG_CB_LIST_MAX          16
//efine MAX_ZDO_CB_FUNC                     6 - currently in ZDApp.h

// mt_znp_basic_cfg_t.cmdDisc bit masks:
#define MT_ZNP_CMD_DISC_RESET_NWK           0x80
#define MT_ZNP_CMD_DISC_ZDO_START           0x40

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct {
  uint16 id;  // Application Profile Id.
  uint8 ep;   // Application EndPoint.
} ep_id_t;

typedef ep_id_t ep_id_list_t[MT_ZNP_EP_ID_LIST_MAX];

// An array of the ZDO function callbacks registered to MT_ZDO.
typedef uint8 zdo_func_cb_list_t[MAX_ZDO_CB_FUNC];

// An array of the uint16 Cluster Id's registered to receive ZDO message callbacks.
typedef uint16 zdo_msg_cb_list_t[MT_ZNP_ZDO_MSG_CB_LIST_MAX];

typedef struct {
  uint32        basicRspRate;                       // Rate at which to generate this AREQ response.

  uint32        zgChannelList;                      // ZCD_NV_CHANLIST.
  uint16        zgConfigPANID;                      // ZCD_NV_PANID.
  uint8         zgStartupOptions;                   // ZCD_NV_STARTUP_OPTION.
  uint8         zgDeviceLogicalType;                // ZCD_NV_LOGICAL_TYPE.

  // A high-use subset of the nwkIB_t _NIB.
  uint16        nwkDevAddress;                      // Device's short address.
  uint16        nwkCoordAddress;                    // Parent's short address.
  uint16        nwkPanId;                           // Device's PanId.
  uint8         nwkLogicalChannel;                  // Current logical channel in use.
  nwk_states_t  nwkState;                           // Device's network state.
  uint8         nwkCoordExtAddress[Z_EXTADDR_LEN];  // Parent's IEEE address.

  uint8         aExtendedAddress[Z_EXTADDR_LEN];    // 64-bit Extended Address of this device.
  
  devStates_t   devState;                           // ZDO device state.
#if defined INTER_PAN
  uint8 appEndPoint;
  uint8         spare1[2];
#else
  uint8         spare1[3];
#endif

  ep_id_list_t  epIdList;
  zdo_msg_cb_list_t zdoMsgCBList;
  zdo_func_cb_list_t zdoFuncCBList;
} mt_znp_basic_rsp_t;

typedef struct {
  uint32        basicRspRate;                       // Rate at which to generate the basic response.

  uint32        zgChannelList;                      // ZCD_NV_CHANLIST.
  uint16        zgConfigPANID;                      // ZCD_NV_PANID.
  uint8         zgDeviceLogicalType;                // ZCD_NV_LOGICAL_TYPE.

  uint8         cmdDisc;                            // Discrete command bits.
} mt_znp_basic_cfg_t;

/* ------------------------------------------------------------------------------------------------
 *                                          Global Variables
 * ------------------------------------------------------------------------------------------------
 */

extern uint32 MT_PeriodicMsgRate;

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

#if defined (MT_ZNP_FUNC)
/**************************************************************************************************
 * @fn          MT_ZnpCommandProcessing
 *
 * @brief       Process all MT ZNP commands.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing a ZNP command.
 *
 * output parameters
 *
 * None.
 *
 * @return      A 1-byte SRSP value or MT_RPC_SRSP_SENT or MT_RPC_SRSP_SENT;
 */
uint8 MT_ZnpCommandProcessing(uint8 *pBuf);

/**************************************************************************************************
 * @fn          MT_ZnpBasicResponse
 *
 * @brief       Build and send the ZNP Basic Response to the ZAP.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      true if message built and sent; false otherwise.
 */
bool MT_ZnpBasicRsp(void);
#endif

#ifdef __cplusplus
};
#endif

#endif
/**************************************************************************************************
*/
