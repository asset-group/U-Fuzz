/**************************************************************************************************
  Filename:       mac_rx_onoff.h
  Revised:        $Date: 2007-09-11 10:58:41 -0700 (Tue, 11 Sep 2007) $
  Revision:       $Revision: 15371 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2009 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MAC_RX_ONOFF_H
#define MAC_RX_ONOFF_H

/* ------------------------------------------------------------------------------------------------
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_defs.h"
#include "hal_types.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */
MAC_INTERNAL_API void macRxOnOffInit(void);
MAC_INTERNAL_API void macRxOnRequest(void);
MAC_INTERNAL_API void macRxOffRequest(void);
MAC_INTERNAL_API void macRxOn(void);
MAC_INTERNAL_API void macRxOff(void);


/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macRxOnFlag;
extern uint8 macRxEnableFlags;


/* ------------------------------------------------------------------------------------------------
 *                                            Macros
 * ------------------------------------------------------------------------------------------------
 */

/* debug macros */
#ifdef MAC_RX_ONOFF_DEBUG_LED
#include "hal_board.h"
#define MAC_DEBUG_TURN_ON_RX_LED()    HAL_TURN_ON_LED2()
#define MAC_DEBUG_TURN_OFF_RX_LED()   HAL_TURN_OFF_LED2()
#else
#define MAC_DEBUG_TURN_ON_RX_LED()
#define MAC_DEBUG_TURN_OFF_RX_LED()
#endif

/* interface macros */
#define MAC_RX_WAS_FORCED_OFF()     st( macRxOnFlag = 0; MAC_DEBUG_TURN_OFF_RX_LED(); )
#define MAC_RX_WAS_FORCED_ON()      st( macRxOnFlag = 1;  MAC_DEBUG_TURN_ON_RX_LED(); )


/**************************************************************************************************
 */
#endif
