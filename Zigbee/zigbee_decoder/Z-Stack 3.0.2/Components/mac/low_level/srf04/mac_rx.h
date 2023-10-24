/**************************************************************************************************
  Filename:       mac_rx.h
  Revised:        $Date: 2014-03-13 10:50:10 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37663 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MAC_RX_H
#define MAC_RX_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "mac_high_level.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Defines
 * ------------------------------------------------------------------------------------------------
 */
#define RX_FILTER_OFF                   0
#define RX_FILTER_ALL                   1
#define RX_FILTER_NON_BEACON_FRAMES     2
#define RX_FILTER_NON_COMMAND_FRAMES    3

/* bit value used to form values of macRxActive */
#define MAC_RX_ACTIVE_PHYSICAL_BV       0x80

#define MAC_RX_ACTIVE_NO_ACTIVITY       0x00  /* zero reserved for boolean use, e.g. !macRxActive */
#define MAC_RX_ACTIVE_STARTED           (0x01 | MAC_RX_ACTIVE_PHYSICAL_BV)
#define MAC_RX_ACTIVE_DONE              0x02


/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RX_IS_PHYSICALLY_ACTIVE()   ((macRxActive & MAC_RX_ACTIVE_PHYSICAL_BV) || macRxOutgoingAckFlag)


/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macRxActive;
extern uint8 macRxFilter;
extern uint8 macRxOutgoingAckFlag;

#ifdef CC2591_COMPRESSION_WORKAROUND
  extern void macRxResetRssi(void);
#endif

/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
MAC_INTERNAL_API void macRxInit(void);
MAC_INTERNAL_API void macRxRadioPowerUpInit(void);
MAC_INTERNAL_API void macRxTxReset(void);
MAC_INTERNAL_API void macRxHaltCleanup(void);
MAC_INTERNAL_API void macRxThresholdIsr(void);
MAC_INTERNAL_API void macRxFifoOverflowIsr(void);
MAC_INTERNAL_API void macRxAckTxDoneCallback(void);


/**************************************************************************************************
 */
#endif
