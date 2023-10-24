/**************************************************************************************************
  Filename:       sb_main.c
  Revised:        $Date: 2014-11-19 13:29:24 -0800 (Wed, 19 Nov 2014) $
  Revision:       $Revision: 41175 $

  Description:    This module contains the main functionality of a Boot Loader for CC2531.
                  It is a minimal subset of functionality from ZMain.c, OnBoard.c and various
                  _hal_X.c modules for the CC2530USB target.


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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_board_cfg.h"
#include "hal_adc.h"
#include "hal_dma.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec_v2.h"
#include "sb_main.h"
#include "usb_board_cfg.h"
#include "usb_interrupt.h"
#include "sb_shared.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

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

// Reset bit definitions
#define LRESET     0x18  // Last reset bit mask
#define RESETPO    0x00  // Power-On reset
#define RESETEX    0x08  // External reset
#define RESETWD    0x10  // WatchDog reset

#ifdef LOCK_THE_BOOT_IMAGE_PAGES_IN_HEX_FILE
#warning LOCK_THE_BOOT_IMAGE_PAGES_IN_HEX_FILE is enabled - please see comments in code
/* comments:
    Do not enable this when programming via IAR. Since not using the banked model for the boot project,
    when programming from IAR it will wrongly write the following value to 0x27FF0 rather than to 0x3FFF0.
    The lockbits are already cleared properly when programming with IAR, via Project->Options->Debugger
    /Texas Instruments->Flash lock protection (this does not apply for the generated hex image).
    Also note that the definition of _LOCKBITS_START in the linker file (xcl) is currently set for
    a 256k device (0x3FFF0). It has to be changed for devices with different flash size.
*/
#pragma location="LOCKBITS"
__root const uint8 CODE boot_image_lockbits = 0xF0; //mark the boot image pages as "read only"
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */

halDMADesc_t dmaCh0;
uint8 sbStateReportingEnabled = FALSE;
uint8 znpCfg1 = ZNP_CFG1_UART_USB;


/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void sblExec(void);
static void sblInit(uint8 bootloaderForcedByMainApp);
static void sblUnInit(void);

static uint8 sblWait(uint16 sbl_wait_time);

#include "_hal_uart_usb.c"

bool sblIsUartTxPending(void)
{
  return (halUartTxT != halUartTxH);
}

void sbUartPoll(void)
{
  HalUARTPollUSB();
}

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
  uint8 time_spent_validating;
  uint8 bootloaderForcedByMainApp = FALSE;
  uint32 mainAppCommandLocal = mainAppCommand;

  vddWait(VDD_MIN_RUN);

  mainAppCommand = MAIN_APP_CMD_NONE;
    
  if (mainAppCommandLocal == MAIN_APP_CMD_FORCE_BOOTLOADER)
  {
    bootloaderForcedByMainApp = TRUE;
  }
  else if ((mainAppCommandLocal == MAIN_APP_CMD_PASS_THROUGH) || ((SLEEPSTA & LRESET) == RESETWD))
  {
    // If reset due to WatchDog Timer - Transfer control to the main application immediately.
    // WatchDog Timer reset causes the hardware to disconnect the USB. Withought this jump here,
    // the SBL code will try to initiaize the CDC too early, which causes undesired behavior on the host
    // (e.g. on beaglebone black - the host gets stuck)
    asm("LJMP 0x2000\n");
  }

  sblInit(bootloaderForcedByMainApp);
  
  HAL_TURN_ON_LED1();
  HAL_TURN_ON_LED2();
  
  if ((!bootloaderForcedByMainApp) && (sbImgValid(&time_spent_validating)))
  {
    HAL_TURN_OFF_LED2();
    
    if (sblWait(SBL_WAIT_TIME > time_spent_validating ? SBL_WAIT_TIME - time_spent_validating : 0))
    {
      HAL_TURN_OFF_LED1();
      
      sbReportState(SB_STATE_EXECUTING_IMAGE);
      
      while(sblIsUartTxPending())
      {
        sbUartPoll();
      }
      
      SLEEP(0x2600); //Give the last bytes in the HW TX fifo (if any) enough time to be transmitted

      while (SB1_PRESS || SB2_PRESS);
      
      sblUnInit();
      
      // Simulate a reset for the Application code by an absolute jump to location 0x2000.
      asm("LJMP 0x2000\n");
    }
  }
  
  HAL_TURN_OFF_LED1();
  HAL_TURN_ON_LED2();
  
  vddWait(VDD_MIN_NV);
  sblExec();

  sblUnInit();

  asm("LJMP 0x2000\n");
}


/**************************************************************************************************
 * @fn          sblExec
 *
 * @brief       Infinite SBL execute loop that returns upon receiving a code enable.
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
  uint8 sbExec_rc;

  sbReportState(SB_STATE_BOOTLOADER_ACTIVE);

  while (1)
  {
    sbUartPoll();

    sbExec_rc = sbExec();
    if (sbExec_rc == SB_CMND_ENABLE_STATE_REPORTING)
    {
      sbReportState(SB_STATE_BOOTLOADER_ACTIVE);
    }
    else if (sbExec_rc == SB_CMND_ENABLED_CMD_OK)
    {
      // Delay to allow the SB_ENABLE_CMD response to be flushed.
      for (dlyCnt = 0; dlyCnt < 0x40000; dlyCnt++)
      {
        sbUartPoll();
      }
      
      break;
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
static void sblInit(uint8 bootloaderForcedByMainApp)
{
  HAL_BOARD_INIT();
  
  if (! bootloaderForcedByMainApp)
  {
    // If power on reset (i.e. reset not due to main software request) - pull down D+.
    // If reset is due to software request - do not touch the pull, otherwise the USB bus will reset and re-enumerate on the host.
    HAL_USB_PULLUP_DISABLE();
  }

  /* This is in place of calling HalDmaInit() which would require init of the other 4 DMA
   * descriptors in addition to just Channel 0.
   */
  HAL_DMA_SET_ADDR_DESC0(&dmaCh0);
  HalUARTInitUSB();
  SB_INIT_LEDS();
}

static void sblUnInit(void)
{
  SB_TURN_OFF_LED1();
  SB_TURN_OFF_LED2();
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
 * @return      TRUE to run the code image, FALSE to run the SBL.
 **************************************************************************************************
 */
static uint8 sblWait(uint16 sbl_wait_time)
{
  uint32 dlyCnt;
  uint8 rtrn = FALSE;
  uint8 sbExec_rc;
  
  dlyCnt = (uint32)0x7332 * sbl_wait_time; //0x7332 gives about 1 second
  
  sbReportState(SB_STATE_WAITING);
  
  while (1)
  {
    sbUartPoll();
    
    sbExec_rc = sbExec();

    if (sbExec_rc == SB_CMND_ENABLE_STATE_REPORTING)
    {
      sbReportState(SB_STATE_WAITING);
    }
    else if ((sbExec_rc == SB_CMND_FORCE_RUN) || (SB2_PRESS))
    {
      dlyCnt = 0;
    }
    else if (((sbExec_rc != SB_CMND_IDLE) && (sbExec_rc != SB_CMND_UNSUPPORTED)) || (SB1_PRESS))
    {
      break;
    }
    
    if (dlyCnt-- == 0)
    {
      rtrn = TRUE;
      break;
    }
  }
  
  return rtrn;
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
void vddWait(uint8 vdd)
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
