/**************************************************************************************************
  Filename:       sb_main.h
  Revised:        $Date: 2014-11-19 13:29:24 -0800 (Wed, 19 Nov 2014) $
  Revision:       $Revision: 41175 $

  Description:    Serial Bootloader Main functionality.

  Copyright 2009-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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
#ifndef SB_MAIN_H
#define SB_MAIN_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_types.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

#define SB_RX(pCH)  HalUARTRx((pCH), 1)
#define SB_TX(pBUF, CNT)  HalUARTTx((pBUF), (CNT))

#define SLEEP(_count) do {volatile uint32 count = (_count); while (count--);} while (0)

#if HAL_KEY
#define SB1_PRESS  (P1_2 == 0)
#define SB2_PRESS  (P1_3 == 0)
#else
#define SB1_PRESS   0
#define SB2_PRESS   0
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Global Functions
 * ------------------------------------------------------------------------------------------------
 */

extern uint8 HalUARTRx(uint8 *buf, uint8 max);
extern uint8 HalUARTTx(uint8 *buf, uint8 max);
bool sblIsUartTxPending(void);
void sbUartPoll(void);
void vddWait(uint8 vdd);


/* ------------------------------------------------------------------------------------------------
 *                                       External Variables
 * ------------------------------------------------------------------------------------------------
 */
 
extern uint8 sbStateReportingEnabled;


#endif
