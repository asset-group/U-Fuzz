/**************************************************************************************************
  Filename:       boot_main.c
  Revised:        $Date: 2013-05-24 19:19:25 -0700 (Fri, 24 May 2013) $
  Revision:       $Revision: 34433 $

  Description:    This file defines the main functionality of the CC2538 Image Boot Manager.


  Copyright 2012-2013 Texas Instruments Incorporated. All rights reserved.

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

#include <string.h>
#include "hal_board_cfg.h"
#include "hal_types.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct {
  uint32 imageCRC[2];
  uint32 nvicJump[2];
} ibm_ledger_t;
static_assert((sizeof(ibm_ledger_t) == 16), "Need to PACK the ibm_ledger_t");

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define HAL_IBM_LEDGER_PAGE        254

static const ibm_ledger_t LedgerPageSignature = {
  0x01234567,
  0x89ABCDEF,
  0x02468ACE,
  0x13579BDF
};


/* ------------------------------------------------------------------------------------------------
 *                                        Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void EnterNvmApplication(uint32 spInit, uint32 resetVector);




/**************************************************************************************************
 * @fn          main
 *
 * @brief       This function is the C-main function invoked from the IAR reset ISR handler.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void main(void)
{
  uint32 ledgerPageAddr = FLASH_BASE + (HAL_IBM_LEDGER_PAGE * HAL_FLASH_PAGE_SIZE);

  for (int pgCnt = 0; pgCnt < HAL_IBM_LEDGER_PAGE; pgCnt++, ledgerPageAddr -= HAL_FLASH_PAGE_SIZE)
  {
    ibm_ledger_t *pLedger = (ibm_ledger_t *)ledgerPageAddr;
    int ledgerCnt = 0;

    if (memcmp(pLedger, &LedgerPageSignature, sizeof(ibm_ledger_t)))
    {
      continue;
    }

    for (pLedger++; ledgerCnt < (HAL_FLASH_PAGE_SIZE/sizeof(ibm_ledger_t)); ledgerCnt++, pLedger++)
    {
      if ( (pLedger->imageCRC[0] == 0xFFFFFFFF) || // Not expected except first 2-step programming.
           ((pLedger->imageCRC[0] != 0) && (pLedger->imageCRC[0] == pLedger->imageCRC[1])) )
      {
        // Sanity check NVIC entries.
        if ((pLedger->nvicJump[0] > 0x20004000) &&
            (pLedger->nvicJump[0] < 0x27007FFF) &&
            (pLedger->nvicJump[1] > FLASH_BASE) &&
            (pLedger->nvicJump[1] < 0x0027EFFF))
        {
          EnterNvmApplication(pLedger->nvicJump[0], pLedger->nvicJump[1]);
        }
      }
    }
  }


  SysCtrlDeepSleepSetting();
  HAL_DISABLE_INTERRUPTS();
  SysCtrlDeepSleep();
  HAL_SYSTEM_RESET();
}

/******************************************************************************
 * @fn      EnterNvmApplication
 *
 * @brief   The link register is loaded with a value causing the processor to fault in case of
 *          an accidental return from the application which should help with debugging.
 *          The stack pointer SP is loaded with the value of the initial stack pointer found
 *          at the top of the vector table.
 *          R1 is loaded with the address of the Reset ISR function found at the second entry
 *          in the vector table.
 *          Finally a branch is done to the address in R1 which is the Reset ISR function
 *          of the run-code image.
 *
 * @param   spInit - The initial value for the stack pointer from the run-code vector table.
 * @param   resetVector - The reset ISR from the run-code vector table.
 *
 * @return  None.
 */
static void EnterNvmApplication(uint32 spInit, uint32 resetVector)
{
  // Set the LR register to a value causing a processor fault in case of
  // an accidental return from the application.
  asm("mov r2, #0xffffffff");
  asm("mov lr, r2");

  asm("mov sp, r0");  // Setup the initial stack pointer value.

  asm("bx r1");  // Branch to application reset ISR.
}

/**************************************************************************************************
*/
