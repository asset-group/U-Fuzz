/**************************************************************************************************
  Filename:       mac_assert.h
  Revised:        $Date: 2013-05-17 11:25:11 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34355 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2012 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MAC_ASSERT_H
#define MAC_ASSERT_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  The MAC_ASSERT() macro is for use during debugging.  The given expression must
 *  evaluate as "true" or else an assert occurs.  At that point, the call stack
 *  feature of the debugger can pinpoint where the problem occurred.
 *
 *  The MAC_ASSERT_FORCED() macro will immediately call the assert handler routine
 *  but only if asserts are enabled.
 *
 *  The MAC_ASSERT_STATEMENT() macro is used to insert a code statement that is only
 *  needed when asserts are enabled.
 *
 *  The MAC_ASSERT_DECLARATION() macro is used to insert a declaration that is only
 *  needed when asserts are enabled.
 *
 *  To disable asserts and save code size, the project should define MACNODEBUG.
 *  Also note, if HAL assert are disabled, MAC asserts are disabled as well.
 *
 */
#ifdef MACNODEBUG
#define MAC_ASSERT(expr)
#define MAC_ASSERT_FORCED()
#define MAC_ASSERT_STATEMENT(statement)
#define MAC_ASSERT_DECLARATION(declaration)
#else
#define MAC_ASSERT(expr)                        HAL_ASSERT( expr )
#define MAC_ASSERT_FORCED()                     HAL_ASSERT_FORCED()
#define MAC_ASSERT_STATEMENT(statement)         HAL_ASSERT_STATEMENT( statement )
#define MAC_ASSERT_DECLARATION(declaration)     HAL_ASSERT_DECLARATION( declaration )
#endif


/**************************************************************************************************
 */
#endif
