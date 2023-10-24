/**************************************************************************************************
  Filename:       mac_cfg.c
  Revised:        $Date: 2014-05-09 18:50:29 -0700 (Fri, 09 May 2014) $
  Revision:       $Revision: 38492 $

  Description:    Compile-time parameters which are configurable by the user.


  Copyright 2005-2007 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_api.h"
#include "mac_security_pib.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* maximum number of data frames in transmit queue */
#ifndef MAC_CFG_TX_DATA_MAX
#define MAC_CFG_TX_DATA_MAX         2
#endif

/* maximum number of frames of all types in transmit queue */
#ifndef MAC_CFG_TX_MAX
#define MAC_CFG_TX_MAX              5
#endif

/* maximum number of frames in receive queue */
#ifndef MAC_CFG_RX_MAX
#define MAC_CFG_RX_MAX              2
#endif

/* allocate additional bytes in the data indication for application-defined headers */
#ifndef MAC_CFG_DATA_IND_OFFSET
#define MAC_CFG_DATA_IND_OFFSET     0
#endif

/* determine whether MAC_MLME_POLL_IND will be sent to the application */
#ifndef MAC_CFG_APP_PENDING_QUEUE
#define MAC_CFG_APP_PENDING_QUEUE   FALSE
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* configurable parameters */
const macCfg_t macCfg =
{
  MAC_CFG_TX_DATA_MAX,
  MAC_CFG_TX_MAX,
  MAC_CFG_RX_MAX,
  MAC_CFG_DATA_IND_OFFSET,
  MAX_DEVICE_TABLE_ENTRIES,
  MAC_CFG_APP_PENDING_QUEUE,
  MAC_MAX_FRAME_SIZE
};
