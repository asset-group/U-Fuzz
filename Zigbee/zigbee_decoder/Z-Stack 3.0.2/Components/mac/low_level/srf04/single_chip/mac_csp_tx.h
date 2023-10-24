/**************************************************************************************************
  Filename:       mac_csp_tx.h
  Revised:        $Date: 2011-09-02 15:36:01 -0700 (Fri, 02 Sep 2011) $
  Revision:       $Revision: 27451 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2011 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MAC_CSP_TX_H
#define MAC_CSP_TX_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "mac_mcu.h"
#include "mac_high_level.h"


/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
MAC_INTERNAL_API void macCspTxReset(void);

MAC_INTERNAL_API void macCspTxPrepCsmaUnslotted(void);
MAC_INTERNAL_API void macCspTxPrepCsmaSlotted(void);
MAC_INTERNAL_API void macCspTxPrepSlotted(void);
MAC_INTERNAL_API void macCspTxPrepGreenPower(void);

MAC_INTERNAL_API void macCspTxGoCsma(void);
MAC_INTERNAL_API void macCspTxGoSlotted(void);
MAC_INTERNAL_API void macCspTxGoGreenPower(void);

MAC_INTERNAL_API void macCspForceTxDoneIfPending(void);

MAC_INTERNAL_API void macCspTxRequestAckTimeoutCallback(void);
MAC_INTERNAL_API void macCspTxCancelAckTimeoutCallback(void);

MAC_INTERNAL_API void macCspTxStopIsr(void);
MAC_INTERNAL_API void macCspTxIntIsr(void);


/**************************************************************************************************
 */
#endif
