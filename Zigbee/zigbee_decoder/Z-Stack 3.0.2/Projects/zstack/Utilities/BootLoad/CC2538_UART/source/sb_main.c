/**************************************************************************************************
  Filename:       sb_main.c
  Revised:        $Date: 2013-06-27 15:44:38 -0700 (Thu, 27 Jun 2013) $
  Revision:       $Revision: 34663 $
  
  Description:    Definitions for the CC2538 SBL.
  
  
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

#include "hal_board_cfg.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"


/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */

sbl_hdr_t *sbl_header_ptr = (sbl_hdr_t *)SBL_HEADER_PTR; // defined as a global, since will change 
                                                         // outside the scope of main, and when 
                                                         // declared as internal, the assembly may 
                                                         // result with this value not being updated 
                                                         // when used later. Another option would be 
                                                         // to define it internally, as volatile


/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void EnterNvmApplication(uint32 vector_table_address);
bool bootloaderCommunicationRequested(void);


/**************************************************************************************************
 * @fn          bootloaderCommunicationRequested
 *
 * @brief       provide a time window for the setial bootloader to initiate an image upload
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if bootloader tries to communicate over UART, FALSE otherwise.
 */
bool bootloaderCommunicationRequested(void)
{
  uint32 delay = 0x00FFFFFF; //about 27 seconds @ 32MHz clock
  
  while (delay--)
  {
    if (sbUartCharAvail())
    {
      return TRUE;
    }
  }
  
  return FALSE;
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
 */
void main(void)
{
  // On power-up, if there is a valid image already loaded, wait about 30 seconds to allow the 
  // host to force bootloader mode. Then, if bootloader mode not forced – execute the image. If 
  // there is no valid image already loaded – enter the bootloader mode immediately. An image is 
  // considered valid if it is already marked as valid. An image that is marked as new, is checked 
  // for validity by calculating its checksum and comparing it to the stored value. If the checksums 
  // match, the image is immediately marked as valid.

  HAL_DISABLE_INTERRUPTS();
  
  PREFETCH_ENABLE();
  SysCtrlClockStartSetting();
  
  sbUartInit();
  
  if (bootloaderCommunicationRequested() || (!sbImgValid()))
  {
    while (1)
    {
      sbUartPoll();
      
      if (sbExec())
      {
        // Delay to allow the SB_ENABLE_CMD response to be flushed (roughly 10's of milliseconds).
        for (uint32 dlyCnt = 0; dlyCnt < 0x44444; dlyCnt++)
        {
          sbUartPoll();
        }
        
        break;
      }
    }
  }
  
  EnterNvmApplication((uint32)sbl_header_ptr->vector_table_address);
  
  HAL_SYSTEM_RESET(); // not supposed to ever get here
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
 * @param   vector_table_address.
 *
 * @return  None.
 */
static void EnterNvmApplication(uint32 vector_table_address)
{
  asm("push {r0}"); // keep the argument value, in case it is overwritten by the next line, 
                    // which may potentially be a function call
  HWREG(NVIC_VTABLE) = vector_table_address;
  asm("pop {r0}");
  asm("ldr sp, [r0]");
  asm("mov lr, #0xffffffff"); // Set the LR register to a value causing a processor fault in 
                              // case of an accidental return from the application.
  asm("ldr r0, [r0, #4]");
  asm("bx r0");  // Branch to application reset ISR.
}

/**************************************************************************************************
 */
