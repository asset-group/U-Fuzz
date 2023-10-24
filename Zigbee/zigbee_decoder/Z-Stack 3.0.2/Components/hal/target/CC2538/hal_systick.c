/**************************************************************************************************
  Filename:       _hal_systick.c
  Revised:        $Date: 2013-05-17 11:25:11 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34355 $

  Description:    This module contains the HAL power management procedures for the CC2538.


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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "hal_systick.h"
#include "hal_mcu.h"

#include "hw_nvic.h"
#include "hw_ints.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
#define SYSTICK 1000 /* 1 millisecond = 1/SYSTICK */
#define TICK_IN_MS 1 /* 1 millisecond */ 

void SysTickIntHandler(void);
/* ------------------------------------------------------------------------------------------------
 *                                      Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/***************************************************************************************************
 * @fn      SysTickIntHandler
 *
 * @brief   The Systick Interrupt module
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
void SysTickIntHandler(void)
{
  /* Update OSAL timer and clock */
  osalAdjustTimer(TICK_IN_MS);
  
  /* Clear Sleep Mode */
  CLEAR_SLEEP_MODE();
}

/***************************************************************************************************
 * @fn      SysTickSetup
 *
 * @brief   Setup the Systick module
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
void SysTickSetup(void)
{
  unsigned long clockval;
  clockval = SysCtrlClockGet();
  clockval /= SYSTICK; 
  
  /* 1ms period for systick */
  SysTickPeriodSet(clockval); 
  
  /* Enable SysTick */
  SysTickEnable();
  
  /* Enable Systick interrupt */
  SysTickIntEnable();
}

