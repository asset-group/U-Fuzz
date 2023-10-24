/**************************************************************************************************
  Filename:       mac_mem.c
  Revised:        $Date: 2009-08-12 14:34:18 -0700 (Wed, 12 Aug 2009) $
  Revision:       $Revision: 20549 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2009 Texas Instruments Incorporated. All rights reserved.

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
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"

/* low-level specific */
#include "mac_mem.h"

/* target specific */
#include "hal_mcu.h"

/* debug */
#include "mac_assert.h"


/**************************************************************************************************
 * @fn          macMemReadRamByte
 *
 * @brief       Read a byte from RAM.
 *
 * @param       pRam - pointer to byte RAM byte to read
 *
 * @return      byte read from RAM
 **************************************************************************************************
 */
uint8 macMemReadRamByte(macRam_t * pRam)
{
  return(*pRam);
}


/**************************************************************************************************
 * @fn          macMemWriteRam
 *
 * @brief       Write multiple bytes to RAM.
 *
 * @param       pRam  - pointer to RAM to be written to
 * @param       pData - pointer to data to write
 * @param       len   - number of bytes to write
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macMemWriteRam(macRam_t * pRam, uint8 * pData, uint8 len)
{
  while (len)
  {
    len--;
    *pRam = *pData;
    pRam++;
    pData++;
  }
}


/**************************************************************************************************
 * @fn          macMemReadRam
 *
 * @brief       Read multiple bytes from RAM.
 *
 * @param       pRam  - pointer to RAM to be read from
 * @param       pData - pointer to location to store read data
 * @param       len   - number of bytes to read
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macMemReadRam(macRam_t * pRam, uint8 * pData, uint8 len)
{
  while (len)
  {
    len--;
    *pData = *pRam;
    pRam++;
    pData++;
  }
}


/**************************************************************************************************
 * @fn          macMemWriteTxFifo
 *
 * @brief       Write multiple bytes to the transmit FIFO.
 *
 * @param       pData - pointer to bytes to be written to TX FIFO
 * @param       len   - number of bytes to write
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macMemWriteTxFifo(uint8 * pData, uint8 len)
{
  MAC_ASSERT(len != 0); /* pointless to write zero bytes */

  do
  {
    RFD = *pData;
    pData++;
    len--;
  }
  while (len);
}


/**************************************************************************************************
 * @fn          macMemReadRxFifo
 *
 * @brief       Read multiple bytes from receive FIFO.
 *
 * @param       pData - pointer to location to store read data
 * @param       len   - number of bytes to read from RX FIFO
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macMemReadRxFifo(uint8 * pData, uint8 len)
{
  MAC_ASSERT(len != 0); /* pointless to read zero bytes */

  do
  {
    *pData = RFD;
    pData++;
    len--;
  }
  while (len);
}


/**************************************************************************************************
*/
