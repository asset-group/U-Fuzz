/**************************************************************************************************
  Filename:       _hal_oad.c
  Revised:        $Date: 2010-07-08 18:39:25 -0700 (Thu, 08 Jul 2010) $
  Revision:       $Revision: 22957 $

  Description:    This module contains optionally-compiled Boot Code to support OAD.
                  The rest of the functionality is the H/W specific drivers to read/write
                  the flash/NV containing the ACTIVE and the DOWNLOADED images.
  Notes:          This version targets the Texas Instruments CC253x family of processors.


  Copyright 2008-2010 Texas Instruments Incorporated. All rights reserved.

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

#include "comdef.h"
#include "hal_board_cfg.h"
#include "hal_dma.h"
#include "hal_flash.h"
#include "hal_oad.h"
#include "hal_types.h"

/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */

#if HAL_OAD_BOOT_CODE
halDMADesc_t dmaCh0;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static uint16 runPoly(uint16 crc, uint8 val);
#if HAL_OAD_XNV_IS_SPI
static void HalSPIRead(uint32 addr, uint8 *pBuf, uint16 len);
static void HalSPIWrite(uint32 addr, uint8 *pBuf, uint16 len);
#endif

#if HAL_OAD_BOOT_CODE
static void vddWait(uint8 vdd);
static void dl2rc(void);
static uint16 crcCalc(void);

/**************************************************************************************************
 * @fn          main
 *
 * @brief       ISR for the reset vector.
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
#pragma location="NEAR_CODE"
void main(void)
{
  HAL_BOARD_INIT();
  vddWait(VDD_MIN_RUN);
 
#if HAL_OAD_XNV_IS_SPI
  XNV_SPI_INIT();
#endif
  /* This is in place of calling HalDmaInit() which would require init of the other 4 DMA
   * descriptors in addition to just Channel 0.
   */
  HAL_DMA_SET_ADDR_DESC0( &dmaCh0 );

  while (1)
  {
    uint16 crc[2];

    HalFlashRead(HAL_OAD_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
                 HAL_OAD_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
                 (uint8 *)crc, sizeof(crc));

    if (crc[0] == crc[1])
    {
      break;
    }
    else if ((crc[0] != 0) && (crc[0] == crcCalc()))
    {
      crc[1] = crc[0];
      HalFlashWrite((HAL_OAD_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
    }
    else
    {
      dl2rc();
    }
  }

  // Simulate a reset for the Application code by an absolute jump to location 0x0800.
  asm("LJMP 0x800\n");
}

/*********************************************************************
 * @fn      vddWait
 *
 * @brief   Loop waiting for 256 reads of the Vdd over the requested limit.
 *
 * @param   vdd - Vdd level to wait for.
 *
 * @return  None.
 *********************************************************************/
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

/*********************************************************************
 * @fn      dl2rc
 *
 * @brief   Copy the DL image to the RC image location.
 *
 *  NOTE:   Assumes that DL image ends on a flash word boundary.
 *
 * @param   None.
 *
 * @return  None.
 *********************************************************************/
static void dl2rc(void)
{
  preamble_t preamble;
  uint32 oset;
  uint16 addr = HAL_OAD_RC_START / HAL_FLASH_WORD_SIZE;
  uint8 buf[4];

  vddWait(VDD_MIN_OAD);
  HalOADRead(PREAMBLE_OFFSET, (uint8 *)&preamble, sizeof(preamble_t), HAL_OAD_DL);

  for (oset = 0; oset < preamble.len; oset += HAL_FLASH_WORD_SIZE)
  {
    HalOADRead(oset, buf, HAL_FLASH_WORD_SIZE, HAL_OAD_DL);
    if ((addr % (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE)) == 0)
    {
      HalFlashErase(addr / (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE));
    }
    HalFlashWrite(addr++, buf, 1);
  }
}

/*********************************************************************
 * @fn      crcCalc
 *
 * @brief   Run the CRC16 Polynomial calculation over the RC image.
 *
 * @param   None.
 *
 * @return  The CRC16 calculated.
 */
static uint16 crcCalc(void)
{
  preamble_t preamble;
  uint32 oset;
  uint16 crc = 0;

  HalOADRead(PREAMBLE_OFFSET, (uint8 *)&preamble, sizeof(preamble_t), HAL_OAD_RC);
  if (preamble.len > HAL_OAD_DL_SIZE)
  {
    return 0;
  }

  // Run the CRC calculation over the active body of code.
  for (oset = 0; oset < preamble.len; oset++)
  {
    if (oset == HAL_OAD_CRC_OSET)
    {
      oset += 3;
    }
    else
    {
      uint8 buf;
      HalOADRead(oset, &buf, 1, HAL_OAD_RC);
      crc = runPoly(crc, buf);
    }
  }

  // IAR note explains that poly must be run with value zero for each byte of crc.
  crc = runPoly(crc, 0);
  crc = runPoly(crc, 0);

  return crc;
}
#endif

/*********************************************************************
 * @fn      runPoly
 *
 * @brief   Run the CRC16 Polynomial calculation over the byte parameter.
 *
 * @param   crc - Running CRC calculated so far.
 * @param   val - Value on which to run the CRC16.
 *
 * @return  crc - Updated for the run.
 */
static uint16 runPoly(uint16 crc, uint8 val)
{
  const uint16 poly = 0x1021;
  uint8 cnt;

  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
  {
    uint8 msb = (crc & 0x8000) ? 1 : 0;

    crc <<= 1;
    if (val & 0x80)  crc |= 0x0001;
    if (msb)         crc ^= poly;
  }

  return crc;
}

/*********************************************************************
 * @fn      HalOADChkDL
 *
 * @brief   Run the CRC16 Polynomial calculation over the DL image.
 *
 * @param   dlImagePreambleOffset - Offset into the monolithic DL image to read the preamble.
 *
 * @return  SUCCESS or FAILURE.
 *********************************************************************/
uint8 HalOADChkDL(uint8 dlImagePreambleOffset)
{
  preamble_t preamble;
  uint32 oset;
  uint16 crc = 0, crc2;

  HalOADRead(dlImagePreambleOffset, (uint8 *)&preamble, sizeof(preamble_t), HAL_OAD_DL);

  // Run the CRC calculation over the downloaded image.
  for (oset = 0; oset < preamble.len; oset++)
  {
    if ((oset < HAL_OAD_CRC_OSET) || (oset >= HAL_OAD_CRC_OSET+4))
    {
      uint8 buf;
      HalOADRead(oset, &buf, 1, HAL_OAD_DL);
      crc = runPoly(crc, buf);
    }
  }

  // IAR note explains that poly must be run with value zero for each byte of crc.
  crc = runPoly(crc, 0);
  crc = runPoly(crc, 0);

  HalOADRead(HAL_OAD_CRC_OSET, (uint8 *)&crc2, sizeof(crc2), HAL_OAD_DL);
  return (crc2 == crc) ? SUCCESS : FAILURE;
}

/*********************************************************************
 * @fn      HalOADInvRC
 *
 * @brief   Invalidate the active image so that the boot code will instantiate the DL image on the
 *          next reset.
 *
 * @param   None.
 *
 * @return  None.
 *********************************************************************/
void HalOADInvRC(void)
{
  uint16 crc[2] = {0,0xFFFF};
  HalFlashWrite((HAL_OAD_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
}

/*********************************************************************
 * @fn      HalOADRead
 *
 * @brief   Read from the storage medium according to image type.
 *
 * @param   oset - Offset into the monolithic image.
 * @param   pBuf - Pointer to the buffer in which to copy the bytes read.
 * @param   len - Number of bytes to read.
 * @param   type - Which image: HAL_OAD_RC or HAL_OAD_DL.
 *
 * @return  None.
 *********************************************************************/
void HalOADRead(uint32 oset, uint8 *pBuf, uint16 len, image_t type)
{
  if (HAL_OAD_RC != type)
  {
#if HAL_OAD_XNV_IS_INT
    preamble_t preamble;

    HalOADRead(PREAMBLE_OFFSET, (uint8 *)&preamble, sizeof(preamble_t), HAL_OAD_RC);
    //oset += HAL_OAD_RC_START + preamble.len;
    oset += HAL_OAD_RC_START + HAL_OAD_DL_OSET;
#elif HAL_OAD_XNV_IS_SPI
    oset += HAL_OAD_DL_OSET;
    HalSPIRead(oset, pBuf, len);
    return;
#endif
  }
  else
  {
    oset += HAL_OAD_RC_START;
  }

  HalFlashRead(oset / HAL_FLASH_PAGE_SIZE, oset % HAL_FLASH_PAGE_SIZE, pBuf, len);
}

/*********************************************************************
 * @fn      HalOADWrite
 *
 * @brief   Write to the storage medium according to the image type.
 *
 *  NOTE:   Destructive write on page boundary! When writing to the first flash word
 *          of a page boundary, the page is erased without saving/restoring the bytes not written.
 *          Writes anywhere else on a page assume that the location written to has been erased.
 *
 * @param   oset - Offset into the monolithic image, aligned to HAL_FLASH_WORD_SIZE.
 * @param   pBuf - Pointer to the buffer in from which to write.
 * @param   len - Number of bytes to write. If not an even multiple of HAL_FLASH_WORD_SIZE,
 *                remainder bytes are overwritten with garbage.
 * @param   type - Which image: HAL_OAD_RC or HAL_OAD_DL.
 *
 * @return  None.
 *********************************************************************/
void HalOADWrite(uint32 oset, uint8 *pBuf, uint16 len, image_t type)
{
  if (HAL_OAD_RC != type)
  {
#if HAL_OAD_XNV_IS_INT
    preamble_t preamble;

    HalOADRead(PREAMBLE_OFFSET, (uint8 *)&preamble, sizeof(preamble_t), HAL_OAD_RC);
    //oset += HAL_OAD_RC_START + preamble.len;
    oset += HAL_OAD_RC_START + HAL_OAD_DL_OSET;
#elif HAL_OAD_XNV_IS_SPI
    oset += HAL_OAD_DL_OSET;
    HalSPIWrite(oset, pBuf, len);
    return;
#endif
  }
  else
  {
    oset += HAL_OAD_RC_START;
  }

  if ((oset % HAL_FLASH_PAGE_SIZE) == 0)
  {
    HalFlashErase(oset / HAL_FLASH_PAGE_SIZE);
  }

  HalFlashWrite(oset / HAL_FLASH_WORD_SIZE, pBuf, len / HAL_FLASH_WORD_SIZE);
}

#if HAL_OAD_XNV_IS_INT
/*********************************************************************
 * @fn      HalOADAvail
 *
 * @brief   Determine the space available for downloading an image.
 *
 * @param   None.
 *
 * @return  Number of bytes available for storing an OAD image.
 *********************************************************************/
uint32 HalOADAvail(void)
{
  /*
  preamble_t preamble;

  HalOADRead(PREAMBLE_OFFSET, (uint8 *)&preamble, sizeof(preamble_t), HAL_OAD_RC);
  return HAL_OAD_DL_MAX - preamble.len;
   */
  return HAL_OAD_DL_MAX - HAL_OAD_DL_OSET;
}

#elif HAL_OAD_XNV_IS_SPI

/*********************************************************************
 * CONSTANTS
 */

#define XNV_STAT_CMD  0x05
#define XNV_WREN_CMD  0x06
#define XNV_WRPG_CMD  0x0A
#define XNV_READ_CMD  0x0B

#define XNV_STAT_WIP  0x01

/*********************************************************************
 * @fn      xnvSPIWrite
 *
 * @brief   SPI write sequence for code size savings.
 *
 * @param   ch - The byte to write to the SPI.
 *
 * @return  None.
 *********************************************************************/
static void xnvSPIWrite(uint8 ch);
static void xnvSPIWrite(uint8 ch)
{
  XNV_SPI_TX(ch);
  XNV_SPI_WAIT_RXRDY();
}

/*********************************************************************
 * @fn      HalOADAvail
 *
 * @brief   Determine the space available for downloading an image.
 *
 * @param   None.
 *
 * @return  Number of bytes available for storing an OAD image.
 *********************************************************************/
uint32 HalOADAvail(void)
{
  return HAL_OAD_DL_MAX - HAL_OAD_DL_OSET;
}

/*********************************************************************
 * @fn      HalSPIRead
 *
 * @brief   Read from the external NV storage via SPI.
 *
 * @param   addr - Offset into the external NV.
 * @param   pBuf - Pointer to the buffer in which to copy the bytes read from external NV.
 * @param   len - Number of bytes to read from external NV.
 *
 * @return  None.
 *********************************************************************/
static void HalSPIRead(uint32 addr, uint8 *pBuf, uint16 len)
{
#if !HAL_OAD_BOOT_CODE
  uint8 shdw = P1DIR;
  halIntState_t his;
  HAL_ENTER_CRITICAL_SECTION(his);
  P1DIR |= BV(3);
#endif

  XNV_SPI_BEGIN();
  do {
    xnvSPIWrite(XNV_STAT_CMD);
  } while (XNV_SPI_RX() & XNV_STAT_WIP);
  XNV_SPI_END();
  asm("NOP"); asm("NOP");

  XNV_SPI_BEGIN();
  xnvSPIWrite(XNV_READ_CMD);
  xnvSPIWrite(addr >> 16);
  xnvSPIWrite(addr >> 8);
  xnvSPIWrite(addr);
  xnvSPIWrite(0);

  while (len--)
  {
    xnvSPIWrite(0);
    *pBuf++ = XNV_SPI_RX();
  }
  XNV_SPI_END();

#if !HAL_OAD_BOOT_CODE
  P1DIR = shdw;
  HAL_EXIT_CRITICAL_SECTION(his);
#endif
}

/*********************************************************************
 * @fn      HalSPIWrite
 *
 * @brief   Write to the external NV storage via SPI.
 *
 * @param   addr - Offset into the external NV.
 * @param   pBuf - Pointer to the buffer in from which to write bytes to external NV.
 * @param   len - Number of bytes to write to external NV.
 *
 * @return  None.
 *********************************************************************/
static void HalSPIWrite(uint32 addr, uint8 *pBuf, uint16 len)
{
  uint8 cnt;
#if !HAL_OAD_BOOT_CODE
  uint8 shdw = P1DIR;
  halIntState_t his;
  HAL_ENTER_CRITICAL_SECTION(his);
  P1DIR |= BV(3);
#endif

  while (len)
  {
    XNV_SPI_BEGIN();
    do {
      xnvSPIWrite(XNV_STAT_CMD);
    } while (XNV_SPI_RX() & XNV_STAT_WIP);
    XNV_SPI_END();
    asm("NOP"); asm("NOP");

    XNV_SPI_BEGIN();
    xnvSPIWrite(XNV_WREN_CMD);
    XNV_SPI_END();
    asm("NOP"); asm("NOP");

    XNV_SPI_BEGIN();
    xnvSPIWrite(XNV_WRPG_CMD);
    xnvSPIWrite(addr >> 16);
    xnvSPIWrite(addr >> 8);
    xnvSPIWrite(addr);

    // Can only write within any one page boundary, so prepare for next page write if bytes remain.
    cnt = 0 - (uint8)addr;
    if (cnt)
    {
      addr += cnt;
    }
    else
    {
      addr += 256;
    }

    do
    {
      xnvSPIWrite(*pBuf++);
      cnt--;
      len--;
    } while (len && cnt);
    XNV_SPI_END();
  }

#if !HAL_OAD_BOOT_CODE
  P1DIR = shdw;
  HAL_EXIT_CRITICAL_SECTION(his);
#endif
}
#else
#error Invalid Xtra-NV for OAD.
#endif

/**************************************************************************************************
*/
