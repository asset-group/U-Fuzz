/*******************************************************************************
  Filename:       armcm3flashutil.c
  Revised:        $Date: 2013-05-09 21:41:33 -0700 (Thu, 09 May 2013) $
  Revision:       $Revision: 34219 $

  Description:    Utility functions to erase/write flash memory pages.


  Copyright 2010-2013 Texas Instruments Incorporated. All rights reserved.

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
*******************************************************************************/

#include "hal_board_cfg.h"
#include "hal_types.h"
#include "hal_mcu.h"
#include "armcm3FlashUtil.h"
#include "rom.h"
#include "flash.h"

/*********************************************************************
 * MACROS
 */

/* Remainder when divided by 4 */
#define byte_offset(addr) ((uint32)addr & 3)

/* Greatest-multiple-of-4 <= addr */
#define aligned_address(addr) ((uint32)addr & ~3)

/*********************************************************************
 * @fn      flash_write_word
 *
 * @brief   Writes 4bytes of data to address ulAddress
 *
 * @param   ulAddress - Address to which data has to be written
 *          address has to be 4byte-aligned.
 *
 * @param   data - 4byte data
 *
 * @return  none
 */
static void flash_write_word( uint32 *ulAddress, uint32 data )
{
  ROM_ProgramFlash((uint32_t *)&data, (uint32_t)ulAddress, 4);
}

/*********************************************************************
 * @fn      initFlash
 *
 * @brief   Sets the clock parameter required by the flash-controller
 *
 * @param   none
 *
 * @return  none
 */
void initFlash( void )
{
  //FlashUsecSet( HAL_CPU_CLOCK_MHZ - 1 );
}

/*********************************************************************
 * @fn      flashErasePage
 *
 * @brief   Erases the page pointed by addr
 *
 * @param   addr - Address of the page to be erased.
 *          addr has to be page aligned.
 *
 * @return  none
 */
void flashErasePage( uint8 *addr )
{
  halIntState_t IntState;

  /* Set the clock frequency */
  initFlash();

  HAL_ENTER_CRITICAL_SECTION( IntState );

  /* Erase flash */
  FlashMainPageErase( (unsigned long)addr );

  HAL_EXIT_CRITICAL_SECTION( IntState );
}

/*********************************************************************
 * @fn      flashWrite
 *
 * @brief   Copies the data from buf(pointer) to
 * addr(pointer to flash memory). addr need not be aligned.
 * One can write the flash only in multiples of 4. The below logic is
 * required to implement data transfer of any number of bytes at any address
 *
 * @param  addr - To-address of the data
 * @param  len - Number of bytes to be transfered
 * @param  buf - From-address of the data
 *
 * @return len - None
 */
void flashWrite( uint8 *addr, uint16 len, uint8 *buf )
{
  if ( len > 0 )
  {
    /* 4-byte aligned pointer */
  uint32 *uint32ptr;
    /* 4-byte temporary variable */
  uint32 temp_u32;
  uint16 i = 0, j;

  /* start_bytes - unaligned byte count at the beggining
   * middle_bytes - aligned byte count at the middle
   * end_bytes - unaligned byte count at the end
   */
  uint16 start_bytes = 0, middle_bytes = 0, end_bytes = 0;
  halIntState_t IntState;

  /* Set the clock frequency */
  initFlash();

  /* Extract 4-byte aligned address */
  uint32ptr = (uint32 *)aligned_address(addr);

  /* Calculate the start_bytes */
    /* If the addr is not 4-byte aligned */
    if( byte_offset(addr) )
  {
      /* If the start-address and the end-address are in the
     * same 4-byte-aligned-chunk.
     */
    if((((uint32)addr) >> 2) == ((((uint32)addr) + len) >> 2))
    {
      start_bytes = len;
    }
    else
    {
      start_bytes = 4 - (byte_offset(addr));
    }
  }

  /* Calculate the middle_bytes and end_bytes */
    /* If there are any bytes left */
    if( (len - start_bytes) > 0 )
  {
    /* Highest-multiple-of-4 less than (len - start_bytes) */
    middle_bytes = ((len - start_bytes) & (~3));
    /* Remainder when divided by 4 */
    end_bytes = (len - start_bytes) & 3;
  }

    HAL_ENTER_CRITICAL_SECTION( IntState );

  /* Write the start bytes to the flash */
    if( start_bytes > 0 )
  {
    /* Take the first 4-byte chunk into a temp_u32 */
    temp_u32 = *uint32ptr;
    /* Write the required bytes into temp_u32 */
    for(; i < start_bytes; i++)
    {
      *(((uint8 *)(&temp_u32)) + i + byte_offset(addr)) = buf[i];
    }
    /* Write the 4-byte chunk into the flah */
    flash_write_word(uint32ptr, temp_u32);
    /* Increment the 4-byte-aligned-address by 4 */
    uint32ptr++;
  }

  /* Write the middle bytes to the flash */
    while( i < start_bytes + middle_bytes )
  {
    /* Extract 4 bytes into from the buf */
      *((uint8*)(&temp_u32)) = buf[i++];
      *((uint8*)(&temp_u32) + 1) = buf[i++];
      *((uint8*)(&temp_u32) + 2) = buf[i++];
      *((uint8*)(&temp_u32) + 3) = buf[i++];

    /* Write the 4-byte chunk into the flash */
      flash_write_word( uint32ptr, temp_u32 );
    /* Increment the 4-byte-aligned-address by 4 */
    uint32ptr++;
  }

  /* Write the end bytes to the flash */
    if( end_bytes > 0 )
  {
    j = 0;
    /* Take the first 4-byte chunk into a temp_u32 */
    temp_u32 = *uint32ptr;
    for(; i < len; i++)
    {
      *((uint8 *)&temp_u32 + j) = buf[i];
      j++;
    }
    /* Write the 4-byte chunk into the flash */
      flash_write_word( uint32ptr, temp_u32 );
  }

  HAL_EXIT_CRITICAL_SECTION( IntState );
  }
}

/*********************************************************************
*********************************************************************/
