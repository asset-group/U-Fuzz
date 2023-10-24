/**************************************************************************************************
  Filename:       sb_exec.c
  Revised:        $Date: 2013-09-12 17:23:02 -0700 (Thu, 12 Sep 2013) $
  Revision:       $Revision: 35297 $

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes 
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_board_cfg.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#if !defined MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
#define MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA  FALSE
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */

static uint8 sbBuf[SB_BUF_SIZE], sbCmd1, sbCmd2, sbFcs, sbIdx, sbLen, sbSte;
  
/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static uint8 sbCmnd(void);
static void sbResp(uint8 rsp, uint8 len);
static uint16 calcCRC(void);
static uint16 runPoly(uint16 crc, uint8 val);

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
uint8 sbExec(void)
{
  uint8 ch, rtrn = FALSE;

  while (SB_RX(&ch))
  {
    sbBuf[sbSte + sbIdx] = ch;
    switch (sbSte)
    {
    case SB_SOF_STATE:
      if (SB_SOF == ch)
      {
        sbSte = SB_LEN_STATE;
      }
      break;
    
    case SB_LEN_STATE:
      sbFcs = 0;
      sbSte = ((sbLen = ch) >= SB_BUF_SIZE) ? SB_SOF_STATE : SB_CMD1_STATE;
      break;

    case SB_CMD1_STATE:
      sbCmd1 = ch;
      sbSte = SB_CMD2_STATE;
      break;
    
    case SB_CMD2_STATE:
      sbCmd2 = ch;
      sbSte = (sbLen) ? SB_DATA_STATE : SB_FCS_STATE;
      break;

    case SB_DATA_STATE:
      if (++sbIdx == sbLen)
      {
        sbSte = SB_FCS_STATE;
      }
      break;
    
    case SB_FCS_STATE:
      if ((sbFcs == ch) && (sbCmd1 == SB_RPC_SYS_BOOT))
      {
        rtrn = sbCmnd();
      }
      else
      {
        // TODO - RemoTI did not have here or on bad length - adding could cause > 1 SB_INVALID_FCS
        //        for a single data packet which could put out of sync with PC for awhile or
        //        infinte, depending on PC-side?
        // sbResp(SB_INVALID_FCS, 1);
      }
    
      sbSte = sbIdx = 0;
      break;
    
    default:
      break;
    }
    sbFcs ^= ch;
  }

  return rtrn;
}

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
uint8 sbImgValid(void)
{
  uint16 crc[2];


  HalFlashRead(HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
               HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
               (uint8 *)crc, sizeof(crc));

  if ((crc[0] == 0xFFFF) || (crc[0] == 0x0000))
  {
    return FALSE;
  }

  if (crc[0] != crc[1])
  {
    crc[1] = calcCRC();
    HalFlashWrite((HAL_SB_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
    HalFlashRead(  HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
                   HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
                   (uint8 *)crc, sizeof(crc));
  }

  return ((crc[0] == crc[1]) && (crc[0] != 0xFFFF) && (crc[0] != 0x0000));
}

/**************************************************************************************************
 * @fn          sbCmnd
 *
 * @brief       Act on the SB command and received buffer.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE to indicate that the SB_ENABLE_CMD command was successful; FALSE otherwise.
 **************************************************************************************************
 */
static uint8 sbCmnd(void)
{
  uint16 tmp = BUILD_UINT16(sbBuf[SB_DATA_STATE], sbBuf[SB_DATA_STATE+1]) + SB_IMG_OSET;
  uint16 crc[2];
  uint8 len = 1;
  uint8 rsp = SB_SUCCESS;
  uint8 rtrn = FALSE;

  switch (sbCmd2)
  {
  case SB_HANDSHAKE_CMD:
    break;

  case SB_WRITE_CMD:
    if ((tmp % SB_WPG_SIZE) == 0)
    {
      HalFlashErase(tmp / SB_WPG_SIZE);
    }

    HalFlashWrite(tmp, sbBuf+SB_DATA_STATE+2, SB_RW_BUF_LEN / HAL_FLASH_WORD_SIZE);
    break;

  case SB_READ_CMD:
#if !MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
    if ((tmp / (HAL_FLASH_PAGE_SIZE / 4)) >= HAL_NV_PAGE_BEG)
    {
      rsp = SB_FAILURE;
      break;
    }
#endif
    HalFlashRead(tmp / (HAL_FLASH_PAGE_SIZE / 4),
                 (tmp % (HAL_FLASH_PAGE_SIZE / 4)) << 2,
                 sbBuf + SB_DATA_STATE + 3, SB_RW_BUF_LEN);
    sbBuf[SB_DATA_STATE+2] = sbBuf[SB_DATA_STATE+1];
    sbBuf[SB_DATA_STATE+1] = sbBuf[SB_DATA_STATE];
    len = SB_RW_BUF_LEN + 3;
    break;
  
  case SB_ENABLE_CMD:
    HalFlashRead(HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
                 HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
                 (uint8 *)crc, sizeof(crc));

    // Bootload master must have verified extra checks to be issuing the SB_ENABLE_CMD.
    //if ((crc[0] != crc[1]) && (crc[0] != 0xFFFF) && (crc[0] != 0x0000))
    if (crc[1] != crc[0])
    {
      crc[1] = crc[0];
      HalFlashWrite((HAL_SB_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
      HalFlashRead(  HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
                     HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
                     (uint8 *)crc, sizeof(crc));
    }

    // Bootload master must have verified extra checks to be issuing the SB_ENABLE_CMD.
    //if ((crc[0] == crc[1]) && (crc[0] != 0xFFFF) && (crc[0] != 0x0000))
    if (crc[0] == crc[1])
    {
      rtrn = TRUE;
    }
    else
    {
      rsp = SB_VALIDATE_FAILED;
    }
    break;
    
  default:
    break;
  }
  
  sbResp(rsp, len);
  return rtrn;
}

/**************************************************************************************************
 * @fn          sbResp
 *
 * @brief       Make the SB response.
 *
 * input parameters
 *
 * @param       rsp - The byte code response to send.
 * @param       len - The data length of the response.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void sbResp(uint8 rsp, uint8 len)
{
  int8 idx;

  sbBuf[SB_CMD2_STATE] |= 0x80;
  sbBuf[SB_DATA_STATE] = rsp;
  sbBuf[SB_LEN_STATE] = len;
  rsp = len ^ SB_RPC_SYS_BOOT;
  len += SB_FCS_STATE-1;

  for (idx = SB_CMD2_STATE; idx < len; idx++)
  {
    rsp ^= sbBuf[idx];
  }
  sbBuf[idx++] = rsp;
  
  SB_TX(sbBuf, idx);
}

/**************************************************************************************************
 * @fn          calcCRC
 *
 * @brief       Run the CRC16 Polynomial calculation over the RC image.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The CRC16 calculated.
 **************************************************************************************************
 */
static uint16 calcCRC(void)
{
  uint32 addr;
  uint16 crc = 0;

  // Run the CRC calculation over the active body of code.
  for (addr = HAL_SB_IMG_ADDR; addr < HAL_SB_IMG_ADDR + HAL_SB_IMG_SIZE; addr++)
  {
    if (addr == HAL_SB_CRC_ADDR)
    {
      addr += 3;
    }
    else
    {
      uint8 buf;
      HalFlashRead(addr / HAL_FLASH_PAGE_SIZE, addr % HAL_FLASH_PAGE_SIZE, &buf, 1);
      crc = runPoly(crc, buf);
    }
  }

  // IAR note explains that poly must be run with value zero for each byte of crc.
  crc = runPoly(crc, 0);
  crc = runPoly(crc, 0);

  return crc;
}

/**************************************************************************************************
 * @fn          runPoly
 *
 * @brief       Run the CRC16 Polynomial calculation over the byte parameter.
 *
 * input parameters
 *
 * @param       crc - Running CRC calculated so far.
 * @param       val - Value on which to run the CRC16.
 *
 * output parameters
 *
 * None.
 *
 * @return      crc - Updated for the run.
 **************************************************************************************************
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

/**************************************************************************************************
*/
