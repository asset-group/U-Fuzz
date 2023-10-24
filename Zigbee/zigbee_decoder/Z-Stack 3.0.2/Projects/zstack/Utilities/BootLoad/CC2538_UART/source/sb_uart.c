/**************************************************************************************************
  Filename:       sb_uart.c
  Revised:        $Date: 2013-06-27 15:44:38 -0700 (Thu, 27 Jun 2013) $
  Revision:       $Revision: 34663 $

  Description:    This file contains the implementation of a special SBL UART driver.


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

#include "hal_board.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"
#include "hw_ioc.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

#define HAL_UART_PORT                      UART0_BASE

#define HAL_UART_SYS_CTRL                  SYS_CTRL_PERIPH_UART0
#define HAL_UART_GPIO_BASE                 GPIO_A_BASE
#define HAL_UART_GPIO_PINS                (GPIO_PIN_0 | GPIO_PIN_1)
#define HAL_UART_INT_CTRL                  INT_UART0

#define HalUartISR                         interrupt_uart0

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

static uint8 txBuf[SB_BUF_SIZE];
static uint32 txHead, txTail;

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void procTx(void);

/*************************************************************************************************
 * @fn      sbUartInit()
 *
 * @brief   Initialize the UART.
 *
 * @param   none
 *
 * @return  none
 */
void sbUartInit(void)
{
  //
  // Set IO clock to the same as system clock
  //
  SysCtrlIOClockSet(SYS_CTRL_SYSDIV_32MHZ);
   
  //
  // Enable UART peripheral module
  //
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART0);

  //
  // Disable UART function
  //
  UARTDisable(UART0_BASE);

  //
  // Disable all UART module interrupts
  //
  UARTIntDisable(UART0_BASE, 0x1FFF);

  //
  // Set IO clock as UART clock source
  //
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

  //
  // Map UART signals to the correct GPIO pins and configure them as
  // hardware controlled.
  //
  IOCPinConfigPeriphOutput(GPIO_A_BASE, GPIO_PIN_1, IOC_MUX_OUT_SEL_UART0_TXD);
  GPIOPinTypeUARTOutput(GPIO_A_BASE, GPIO_PIN_1); 
  IOCPinConfigPeriphInput(GPIO_A_BASE, GPIO_PIN_0, IOC_UARTRXD_UART0);
  GPIOPinTypeUARTInput(GPIO_A_BASE, GPIO_PIN_0);
     
  //
  // Configure the UART for 115,200, 8-N-1 operation.
  // This function uses SysCtrlClockGet() to get the system clock
  // frequency.  This could be also be a variable or hard coded value
  // instead of a function call.
  //
  UARTConfigSetExpClk(UART0_BASE, SysCtrlClockGet(), 115200,
                     (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  UARTEnable(UART0_BASE);
}

/*************************************************************************************************
 * @fn      sbUartPoll
 *
 * @brief   This routine simulate polling and has to be called by the main loop.
 *
 * @param   void
 *
 * @return  void
 */
void sbUartPoll(void)
{
  procTx();
}

/*************************************************************************************************
 * @fn      sbUartWrite()
 *
 * @brief   Write a buffer to the UART.
 *
 * @param   pBuf - pointer to the buffer that will be written.
 *          len  - length of buffer to write.
 *
 * @return  length of the buffer that was sent.
 */
uint32 sbUartWrite(uint8 *pBuf, uint32 len)
{
  uint32 idx = txHead;
  uint32 cnt = txTail;

  if (cnt == idx)
  {
    cnt = SB_BUF_SIZE;
  }
  else if (cnt > idx)
  {
    cnt = SB_BUF_SIZE - cnt + idx;
  }
  else // (cnt < idx)
  {
    cnt = idx - cnt;
  }

  // Accept "all-or-none" on write request.
  if (cnt < len)
  {
    return 0;
  }

  idx = txTail;

  for (cnt = 0; cnt < len; cnt++)
  {
    txBuf[idx++] = pBuf[cnt];

    if (idx >= SB_BUF_SIZE)
    {
      // txBuf is a circular buffer. When reaching the end - go back to the start.
      idx = 0;
    }
  }

  txTail = idx;
  procTx();

  return len;  // Return the number of bytes actually put into the buffer.
}


bool sbUartCharAvail(void)
{
  return UARTCharsAvail(HAL_UART_PORT);
}


/*************************************************************************************************
 * @fn      sbUartGetChar
 *
 * @brief   Attempt to get an Rx byte.
 *
 * @param   pCh - Pointer to the character buffer into which to read an Rx byte.
 *
 * @return  Zero or One.
 */
uint32 sbUartGetChar(uint8 *pCh)
{
  if (UARTCharsAvail(HAL_UART_PORT))
  {
    *pCh = UARTCharGetNonBlocking(HAL_UART_PORT);
    return 1;
  }

  return 0;
}

/*************************************************************************************************
 * @fn      procTx
 *
 * @brief   Process Tx bytes.
 *
 * @param   void
 *
 * @return  void
 */
static void procTx(void)
{
  while ((txHead != txTail) && (UARTCharPutNonBlocking(HAL_UART_PORT, txBuf[txHead])))
  {
    if (++txHead >= SB_BUF_SIZE)
    {
      txHead = 0;
    }
  }
}

/**************************************************************************************************
*/
