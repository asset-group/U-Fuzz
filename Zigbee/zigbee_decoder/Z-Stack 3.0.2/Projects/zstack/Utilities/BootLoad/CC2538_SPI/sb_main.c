/**************************************************************************************************
Filename:       sb_main.c
Revised:        $Date: 2015-01-02 16:08:40 -0800 (Fri, 02 Jan 2015) $
Revision:       $Revision: 41598 $

Description:    This module contains the main functionality of a Boot Loader for
                CC2538. It is a minimal subset of functionality from ZMain.c,
                OnBoard.c and various _hal_X.c modules for the CC2538ZNP target.


Copyright 2013-2014 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
*                                          Includes
* ------------------------------------------------------------------------------------------------
*/

#include "hal_board_cfg.h"
#include "hal_adc.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"

/* ------------------------------------------------------------------------------------------------
*                                          Constants
* ------------------------------------------------------------------------------------------------
*/
#define ENABLE_RSP_DELAY 0xFFF
#define BOOT_DELAY       0x00FFFFFF
/* ------------------------------------------------------------------------------------------------
*                                           Macros
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
*                                       Global Variables
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
*                                       Global Variables
* ------------------------------------------------------------------------------------------------
*/

/* defined as a global, since will change
 * outside the scope of main, and when
 * declared as internal, the assembly may
 * result with this value not being updated
 * when used later. Another option would be
 * to define it internally, as volatile
 */
sbl_hdr_t *sbl_header_ptr = (sbl_hdr_t *)SBL_HEADER_PTR;

/* ------------------------------------------------------------------------------------------------
*                                       Local Variables
* ------------------------------------------------------------------------------------------------
*/

static uint8 sblReset;

/* ------------------------------------------------------------------------------------------------
*                                       Local Functions
* ------------------------------------------------------------------------------------------------
*/

static void sblExec(void);
static void EnterNvmApplication(uint32 vector_table_address);
static bool bootloaderCommunicationRequested(void);

#include "_hal_uart_spi.c"

/**************************************************************************************************
* @fn          main
*
* @brief       C-code main functionality.
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
**************************************************************************************************
*/
void main(void)
{
  /* Setup the clock startup sequence to 32 MHz external
   * osc and 32k sourced from external oscillator
   */
  IOCPadConfigSet(GPIO_D_BASE, 0xC0, IOC_OVERRIDE_ANA);
  SysCtrlClockSet(OSC_32KHZ, false, SYS_CTRL_SYSDIV_32MHZ);

  /* Check if clock is stable */
  HAL_CLOCK_STABLE();

  /* Turn on cache prefetch mode */
  PREFETCH_ENABLE();

  /* Boot Loader code execute */
  sblExec();

  /* Code should not come here */
  HAL_SYSTEM_RESET();
}

/**************************************************************************************************
* @fn          sblRxCB
*
* @brief       Callback function for the SPI Rx message ready.
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
**************************************************************************************************
*/
static void sblRxCB(uint8 port, uint8 event)
{
  (void)port;
  (void)event;

  sblReset |= sblPoll();
}

/**************************************************************************************************
* @fn          bootloaderCommunicationRequested
*
* @brief       Function to check if boot loader should run
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
**************************************************************************************************
*/
static bool bootloaderCommunicationRequested(void)
{
  /* about 15 seconds @ 32MHz clock */
  uint32 delay = BOOT_DELAY;
  uint8 ch;

  /* Set SRDY low to let master know that Slave is ready to recieve bytes */
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0);

  /* post decrement for delay, because for SB_FORCE_RUN condition delay is set
   * to zero to terminate the while loop. If pre decrement for delay with --delay
   * the SB_FORCE_RUN will NOT terminate the loop.
   */
  while (delay--)
  {
    /* Read byte */
    if(SSIDataGetNonBlocking(SSI0_BASE, (uint32_t *)&ch) == 1)
    {
      /* Bootloader should download code. */
      if (ch == SB_FORCE_BOOT)
      {
        /* Set SRDY high */
        GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
        return TRUE;
      }
      else if (ch == SB_FORCE_RUN)
      {
        delay = 0;
      }
    }
  }

  /* Set SRDY high */
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  /* Skip image download, jump to existing application image */
  return FALSE;
}

/**************************************************************************************************
* @fn          sblExec
*
* @brief       Infinite SBL execute loop that jumps upon receiving a code enable.
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
**************************************************************************************************
*/
static void sblExec(void)
{
  halUARTCfg_t uartConfig;
  uartConfig.callBackFunc = sblRxCB;
  HalUARTInitSPI();

  if (bootloaderCommunicationRequested() || (!sbImgValid()))
  {
    while (1)
    {
      /* Read , parse, and respond to incoming bytes*/
      sblRxCB(0, 0);

      /* exit loop if image is successfully verified and enabled */
      if (sblReset)
      {
        break;
      }
    }
  }

  /* Some delay to send final response to master */
  for(uint32 i = 0; i < ENABLE_RSP_DELAY; i++)
  {
    ASM_NOP;
  }

  /* Some delay to send final response to master */
  HalUARTUnInitSPI();

  /* Jump to application image */
  EnterNvmApplication((uint32)sbl_header_ptr->vectorTableAddress);
}

/**************************************************************************************************
* @fn          sbTx
*
* @brief       Serial Boot loader write API that makes the low-level write according to RPC mode.
*
* input parameters
*
* @param       buf - Pointer to a buffer of 'len' bytes to write to the serial transport.
* @param       len - Length in bytes of the 'buf'.
*
*
* output parameters
*
* None.
*
* @return      The count of the number of bytes written from the 'buf'.
**************************************************************************************************
*/
uint16 sbTx(uint8 *buf, uint16 len)
{
  return HalUARTWriteSPI(buf, len);
}

/**************************************************************************************************
* @fn          sbRx
*
* @brief       Serial Boot loader write API that makes the low-level read according to RPC mode.
*
* input parameters
*
* @param       buf - Pointer to a buffer of 'len' bytes to write to the serial transport.
* @param       len - Length in bytes of the 'buf'.
*
*
* output parameters
*
* None.
*
* @return      The count of the number of bytes written from the 'buf'.
**************************************************************************************************
*/
uint16 sbRx(uint8 *buf, uint16 len)
{
  return HalUARTReadSPI(buf, len);
}

/******************************************************************************
* @fn      EnterNvmApplication
*
* @brief   The link register is loaded with a value causing the processor to fault in case of
*          an accidental return from the application which should help with debugging.
*          The stack pointer SP is loaded with the value of the initial stack pointer found
*          at the top of the vector table.
*          The vector table address is updated.
*          Finally a branch is done to the address pointed by the reset vector.
*
* @param   vectorTableAddress.
*
* @return  None.
*/
static void EnterNvmApplication(uint32 vectorTableAddress)
{
  /* keep the argument value, in case it is overwritten by the next line,
   * which may potentially be a function call
   */
  asm("push {r0}");
  HWREG(NVIC_VTABLE) = vectorTableAddress;
  asm("pop {r0}");
  asm("ldr sp, [r0]");
  /* Set the LR register to a value causing a processor fault in
   * case of an accidental return from the application.
   */
  asm("mov lr, #0xffffffff");
  asm("ldr r0, [r0, #4]");
  /* Branch to application reset ISR. */
  asm("bx r0");
}
/**************************************************************************************************
*/
