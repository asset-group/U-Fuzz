/**************************************************************************************************
  Filename:       sb_exec.h
  Revised:        $Date: 2014-11-19 13:29:24 -0800 (Wed, 19 Nov 2014) $
  Revision:       $Revision: 41175 $

  Description:    Serial Bootloader Executive.

  Copyright 2014 Texas Instruments Incorporated. All rights reserved.

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
#ifndef SB_EXEC_H
#define SB_EXEC_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes 
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_types.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define SB_BOOTLOADER_REVISION 0

#define SB_DEVICE_TYPE_2538 1
#define SB_DEVICE_TYPE_2530 2

#define SB_DEVICE_PAGE_SIZE 2048

#define SB_IMG_OSET                 (HAL_SB_IMG_ADDR / HAL_FLASH_WORD_SIZE)
// The write-page boundary since the write-address is passed as "actual address / flash word size".
#define SB_WPG_SIZE                 (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE)

#define SB_BUF_SIZE                 128
#define SB_SOF                      0xFE

#define SB_FORCE_BOOT               0x10
#define SB_FORCE_RUN               (SB_FORCE_BOOT ^ 0xFF)

#define SB_RW_BUF_LEN               64

// Bootloader Serial Interface Subsystem
#define SB_RPC_SYS_BOOT             0x4D

// Commands to Bootloader
#define SB_WRITE_CMD                0x01
#define SB_READ_CMD                 0x02
#define SB_ENABLE_CMD               0x03
#define SB_HANDSHAKE_CMD            0x04
#define SB_STATE_IND                0x05
#define SB_SWITCH_BAUDRATE_CMD      0x06
#define SB_ENABLE_REPORTING_CMD     0x07

// Status codes
#define SB_SUCCESS                  0
#define SB_FAILURE                  1
#define SB_INVALID_FCS              2
#define SB_INVALID_FILE             3
#define SB_FILESYSTEM_ERROR         4
#define SB_ALREADY_STARTED          5
#define SB_NO_RESPOSNE              6
#define SB_VALIDATE_FAILED          7
#define SB_CANCELED                 8

//verification status
#define SB_STATE_VERIFYING                  0
#define SB_STATE_VERIFICATION_ABORTED       1
#define SB_STATE_VERIFICATION_IMAGE_VALID   2
#define SB_STATE_VERIFICATION_IMAGE_INVALID 3
#define SB_STATE_VERIFICATION_FAILED        4
#define SB_STATE_BOOTLOADER_ACTIVE          5        
#define SB_STATE_WAITING                    6
#define SB_STATE_EXECUTING_IMAGE            7

// Serial RX States
#define SB_SOF_STATE                0
#define SB_LEN_STATE                1
#define SB_CMD1_STATE               2
#define SB_CMD2_STATE               3
#define SB_DATA_STATE               4
#define SB_FCS_STATE                5

//sbCmnd Result Codes
#define SB_CMND_IDLE                   0
#define SB_CMND_ENABLED_CMD_OK         1
#define SB_CMND_ENABLED_CMD_ERROR      2
#define SB_CMND_HANDSHAKE_CMD          3
#define SB_CMND_WRITE_CMD              4
#define SB_CMND_READ_CMD               5
#define SB_CMND_UNSUPPORTED            6
#define SB_CMND_FORCE_RUN              7
#define SB_CMND_SWITCH_BAUDRAT         8
#define SB_CMND_ENABLE_STATE_REPORTING 9

/* ------------------------------------------------------------------------------------------------
 *                                       Exported Variables
 * ------------------------------------------------------------------------------------------------
 */

extern __no_init volatile uint32 mainAppCommand;

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          sbExec
 *
 * @brief       Boot Loader main executive processing.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if sbCmnd() returns TRUE, indicating that an SB_ENABLE_CMD succeeded;
 *              FALSE otherwise.
 **************************************************************************************************
 */
uint8 sbExec(void);

/**************************************************************************************************
 * @fn          sbImgValid
 *
 * @brief       Check validity of the run-code image.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE or FALSE for image valid.
 **************************************************************************************************
 */
uint8 sbImgValid(uint8 * time_spent_validating);

/**************************************************************************************************
 * @fn          sbReportState
 *
 * @brief       Report the given state to the applicationBoot Loader main executive processing.
 *
 * input parameters
 *
 *   state -the state to report
 *
 * output parameters
 *
 * None.
 *
 * @return      (none)
 **************************************************************************************************
 */
void sbReportState(uint8 state);

#endif
