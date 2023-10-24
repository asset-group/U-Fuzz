/**************************************************************************************************
  Filename:       sb_exec.h
  Revised:        $Date: 2013-10-28 12:17:28 -0700 (Mon, 28 Oct 2013) $
  Revision:       $Revision: 35823 $

  Description:    Serial Bootloader Executive.

  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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



#define SB_RW_BUF_LEN               64 //SB_DEVICE_PAGE_SIZE // Use the biggest buffers possible
#define SB_BUF_HEADER_SIZE          64 // Current size is about 16, but set larger here just to be safe if it ever changed
#define SB_BUF_SIZE                 (SB_RW_BUF_LEN + SB_BUF_HEADER_SIZE)
#define SB_SOF                      0xFE
#define SB_HANDSHAKE                0xFE

#define SB_FORCE_BOOT               0x10
#define SB_FORCE_RUN               (SB_FORCE_BOOT ^ 0xFF)


/* Bootloader Serial Interface Subsystem */
#define SB_RPC_SYS_BOOT             0x4D

/* Commands to Bootloader */
#define SB_WRITE_CMD                0x01
#define SB_READ_CMD                 0x02
#define SB_ENABLE_CMD               0x03
#define SB_HANDSHAKE_CMD            0x04

/* Status codes */
#define SB_SUCCESS                  0
#define SB_FAILURE                  1
#define SB_INVALID_FCS              2
#define SB_INVALID_FILE             3
#define SB_FILESYSTEM_ERROR         4
#define SB_ALREADY_STARTED          5
#define SB_NO_RESPOSNE              6
#define SB_VALIDATE_FAILED          7
#define SB_CANCELED                 8

/* Serial RX States */
#define SB_SOF_STATE                0
#define SB_LEN_STATE                1
#define SB_FRAME_ID_STATE           2
#define SB_CMD_STATE                3
#define SB_DATA_STATE               4
#define SB_FCS_STATE                5
#define SB_LEN1_STATE               6
#define SB_LEN2_STATE               7
#define SB_LEN3_STATE               8
#define SB_LEN4_STATE               9
   
#define SB_CFG_IMG_B                0x80000000  // Set if there is a DIM Image-B to check.

#if !defined HAL_IMG_A_BEG
#define HAL_IMG_A_BEG  (FLASH_BASE + HAL_FLASH_PAGE_SIZE)  // Default SBL only takes Page 0 & 255.
#endif

#define SB_IMG_PROGRAMMED 0xA5A5A5A5
#define SB_IMG_VERIFIED   0x05A0A005

/* Reset vector maximum size according to the CC2538 datasheet */
#define CC2538_RESET_VECTOR_TABLE_MAX_SIZE 0x11C

/* NOTE: any changes to the following two values need to be reflected in both the 
 * appropriate .icf file as well as in the startup_ewarm.c file.
 */
#define CC2538_CODE_FLASH_START_ADDRESS (0x00200000)
   
/* Last address before the serial bootloader, NV and lock bits pages*/
#define CC2538_CODE_FLASH_END_ADDRESS (0x0027B7FF) 

/* The sbl_header is located right after the vector table */
#define SBL_HEADER_PTR ((uint8 *)(CC2538_CODE_FLASH_START_ADDRESS + CC2538_RESET_VECTOR_TABLE_MAX_SIZE)) 

#define SB_DEVICE_TYPE_2538 1
#define SB_DEVICE_PAGE_SIZE HAL_FLASH_PAGE_SIZE
#define SB_DEVICE_FLASH_SIZE (256 * HAL_FLASH_PAGE_SIZE)

#define SB_BOOTLOADER_REVISION 1
   
   
typedef struct 
{
   uint32 checksum;             /* ielftool-generated CRC-16 to be verified by boot loader. */
   uint32 compatibilityFlags;   /* Currently unused. for compatibility with future bootloaders. 
	                         * enable extending the sbl_hdr_t structure in the future.
                                 */
   uint32 imgStatus;            /* Equals to SB_IMG_WRITTEN if there is an image programmed into flash 
                                 * memory, and it had been verified to have the correct checksum
                                 */
   uint32 checksumBegin;        /*  Configuration bits. */ 
   uint32 checksumEnd;          /* Configuration bits. */ 
   uint32 vectorTableAddress;
} sbl_hdr_t;

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
uint8 sbImgValid(void);
uint8 sblPoll(void);

#endif
