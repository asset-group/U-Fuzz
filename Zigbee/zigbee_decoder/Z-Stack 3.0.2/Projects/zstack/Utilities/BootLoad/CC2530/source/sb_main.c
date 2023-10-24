/**************************************************************************************************
  Filename:       sb_main.c
  Revised:        $Date: 2013-11-19 09:36:42 -0800 (Tue, 19 Nov 2013) $
  Revision:       $Revision: 36147 $

  Description:    This module contains the main functionality of a Boot Loader for CC2530.
                  It is a minimal subset of functionality from ZMain.c, OnBoard.c and various
                  _hal_X.c modules for the CC2530EB target.


  Copyright 2009-2013 Texas Instruments Incorporated. All rights reserved.

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

#include "hal_board_cfg.h"
#include "hal_adc.h"
#include "hal_dma.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Delay jump to valid RC code, waiting for a force boot or force run indication via the
 * physical transport or button press indication. Set to zero to jump immediately, this
 * necessitates the RC to invalidate checksum/shadow to force boot mode.
 */
#if !defined SB_UART_DELAY
#define SB_UART_DELAY  0x260000  // About 1 minute.
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

#if HAL_KEY
#define SB1_PRESS  (P0_1 != 0)
#define SB2_PRESS  (P2_0 != 0)
#else
#define SB1_PRESS   0
#define SB2_PRESS   0
#endif

#if HAL_LED
#define SB_INIT_LEDS() st (  \
  HAL_TURN_OFF_LED1();  \
  LED1_DDR |= LED1_BV;  \
  HAL_TURN_OFF_LED2();  \
  LED2_DDR |= LED2_BV;  \
)
#define SB_TURN_OFF_LED1()  HAL_TURN_OFF_LED1()
#define SB_TURN_ON_LED1()   HAL_TURN_ON_LED1()
#define SB_TOGGLE_LED1()    HAL_TOGGLE_LED1()
#define SB_TURN_OFF_LED2()  HAL_TURN_OFF_LED2()
#define SB_TURN_ON_LED2()   HAL_TURN_ON_LED2()
#define SB_TOGGLE_LED2()    HAL_TOGGLE_LED2()
#else
#define SB_TURN_OFF_LED1()
#define SB_TURN_ON_LED1()
#define SB_TOGGLE_LED1()
#define SB_TURN_OFF_LED2()
#define SB_TURN_ON_LED2()
#define SB_TOGGLE_LED2()
#endif

#if !defined ResetWasWatchDog
#define ResetWasWatchDog ((SLEEPSTA & 0x18) == 0x10)
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */

halDMADesc_t dmaCh0;

/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void sblExec(void);
static void sblInit(void);
static void sblJump(void);
static void sblWait(void);
static void vddWait(uint8 vdd);

#include "_hal_uart_isr.c"

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
  vddWait(VDD_MIN_RUN);
  HAL_BOARD_INIT();

  // make sure the DMA channel is selected before we attempt to
  // to write anything to flash.
  sblInit();
  
  if (sbImgValid())
  {
    if ((SB_UART_DELAY == 0) || ResetWasWatchDog)
    {
      sblJump();
    }

    sblWait();
  }

  vddWait(VDD_MIN_NV);
  sblExec();
  HAL_SYSTEM_RESET();
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
  uint32 dlyCnt = 0;

  while (1)
  {
    HalUARTPollISR();

    if (sbExec() && sbImgValid())
    {
      SB_TURN_ON_LED1();
      SB_TURN_ON_LED2();
      // Delay to allow the SB_ENABLE_CMD response to be flushed.
      for (dlyCnt = 0; dlyCnt < 0x40000; dlyCnt++)
      {
        HalUARTPollISR();
      }

      sblJump();
    }
    else if (dlyCnt++ & 0x4000)
    {
      SB_TOGGLE_LED1();
    }
  }
}

/**************************************************************************************************
 * @fn          sblInit
 *
 * @brief       SBL initialization.
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
static void sblInit(void)
{
  halUARTCfg_t uartConfig;
  /* This is in place of calling HalDmaInit() which would require init of the other 4 DMA
   * descriptors in addition to just Channel 0.
   */
  HAL_DMA_SET_ADDR_DESC0(&dmaCh0);

  HalUARTInitISR();
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = HAL_UART_BR_115200;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.rx.maxBufSize        = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.tx.maxBufSize        = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.idleTimeout          = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.intEnable            = TRUE;
  uartConfig.callBackFunc         = NULL;
  HalUARTOpenISR(&uartConfig);

  SB_INIT_LEDS();
}

/**************************************************************************************************
 * @fn          sblJump
 *
 * @brief       Execute a simple long jump from non-banked SBL code to non-banked RC code space.
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
static void sblJump(void)
{
  SB_TURN_ON_LED1();
  SB_TURN_ON_LED2();
  while (SB1_PRESS || SB2_PRESS);
  SB_TURN_OFF_LED1();
  SB_TURN_OFF_LED2();
  asm("LJMP 0x2000\n");  // Immediate jump to run-code.
  HAL_SYSTEM_RESET();
}

/**************************************************************************************************
 * @fn          sblWait
 *
 * @brief       A timed-out wait loop that exits early upon receiving a force code/sbl byte.
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
static void sblWait(void)
{
  uint32 dlyCnt = SB_UART_DELAY;

  while (1)
  {
    uint8 ch;

    HalUARTPollISR();
    if (HalUARTReadISR(&ch, 1))
    {
      if (ch == SB_FORCE_BOOT)
      {
        break;
      }
      else if (ch == SB_FORCE_RUN)
      {
        dlyCnt = 0;
      }
    }

    if (SB1_PRESS)
    {
      break;
    }

    if (SB2_PRESS || (dlyCnt-- == 0))
    {
      sblJump();
    }

    // RR-xing LED display while waiting.
    if (dlyCnt & 0x2000)
    {
      SB_TURN_OFF_LED2();
      SB_TURN_ON_LED1();
    }
    else
    {
      SB_TURN_OFF_LED1();
      SB_TURN_ON_LED2();
    }
  }

  SB_TURN_OFF_LED1();
  SB_TURN_OFF_LED2();
}

/**************************************************************************************************
 * @fn          vddWait
 *
 * @brief       Loop waiting for 16 reads of the Vdd over the requested limit.
 *
 * input parameters
 *
 * @param       vdd - Vdd level to wait for.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void vddWait(uint8 vdd)
{
  uint8 cnt = 16;

  do {
    do {
      ADCCON3 = 0x0F;
      while (!(ADCCON1 & 0x80));
    } while (ADCH < vdd);
  } while (--cnt);
}

/**************************************************************************************************
*/
