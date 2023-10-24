/**************************************************************************************************
  Filename:       sb_exec.c
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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes 
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_board_cfg.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec_v2.h"
#include "sb_main.h"
#ifdef INCLUDE_REVISION_INFORMATION
#include "revision_info.h"
#else
#define CODE_REVISION_NUMBER 0
#endif
#include "sb_shared.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#if !defined MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
#define MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA  FALSE
#endif

#define TIME_IT_TAKES_TO_CALC_CRC 30 //seconds, approx

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
static uint16 calcCRC(uint8 * abort);
static uint16 runPoly(uint16 crc, uint8 val);

/* ------------------------------------------------------------------------------------------------
 *                                       Externals
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 znpCfg1;

/* ------------------------------------------------------------------------------------------------
 *                                       System-Globals
 * ------------------------------------------------------------------------------------------------
 */
 
__no_init volatile uint32 mainAppCommand;

#pragma location="SBL_CMD"
const CODE uint16 _sblCmdAddr = (uint16)&mainAppCommand;
#pragma required=_sblCmdAddr
	
#pragma location="SBL_SIG"
const CODE uint32 _sblSig = SBL_SIGNATURE;
#pragma required=_sblSig
	
#pragma location="SBL_REV"
const CODE uint32 _sblRev = CODE_REVISION_NUMBER;
#pragma required=_sblRev

/* ------------------------------------------------------------------------------------------------
 *                                       Functional Macros
 * ------------------------------------------------------------------------------------------------
 */
 
#define DOES_BUF_CONTAIN_PRECALC_CRC(wordAddr) ((wordAddr <= (HAL_SB_CRC_ADDR / HAL_FLASH_WORD_SIZE)) && ((wordAddr + (SB_RW_BUF_LEN / HAL_FLASH_WORD_SIZE)) >= ((HAL_SB_CRC_ADDR + (HAL_SB_CRC_LEN / 2)) / HAL_FLASH_WORD_SIZE)))

/* ------------------------------------------------------------------------------------------------
 *                                       Functions
 * ------------------------------------------------------------------------------------------------
 */
 
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
void sbReportState(uint8 state)
{
  if (sbStateReportingEnabled)
  {
    sbBuf[SB_SOF_STATE] = SB_SOF;
    sbBuf[SB_CMD1_STATE] = SB_RPC_SYS_BOOT;
    sbBuf[SB_CMD2_STATE] = SB_STATE_IND;
    sbResp(state,1);
  }
}

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
 * @return      SB_CMND_IDLE if no command was currently executed, otherwise, returns a constant
 *              that indicates the command that was just executed. See "sbCmnd Result Codes"
 **************************************************************************************************
 */
uint8 sbExec(void)
{
  uint8 ch, rtrn = SB_CMND_IDLE;

  while (SB_RX(&ch))
  {
    if (ch == SB_FORCE_RUN)
    {
      rtrn = SB_CMND_FORCE_RUN;
    }
    
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
      sbSte = sbIdx = 0;
	  
      if ((sbFcs == ch) && (sbCmd1 == SB_RPC_SYS_BOOT))
      {
        return sbCmnd();
      }
      else
      {
        // TODO - RemoTI did not have here or on bad length - adding could cause > 1 SB_INVALID_FCS
        //        for a single data packet which could put out of sync with PC for awhile or
        //        infinte, depending on PC-side?
        // sbResp(SB_INVALID_FCS, 1);
      }
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
uint8 sbImgValid(uint8 * time_spent_validating)
{
  uint16 crc[2];
  uint8 abort;
  
  HalFlashRead(HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
    HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
    (uint8 *)crc, sizeof(crc));
  
  if ((crc[0] != crc[1]) || (crc[0] == 0xFFFF))
  {
    crc[1] = calcCRC(&abort);
    
    if (abort)
    {
      sbReportState(SB_STATE_VERIFICATION_ABORTED);
      return FALSE;
    }
    
    if (crc[0] == crc[1])
    {
      if (crc[0] == 0xFFFF)
      {
        crc[0] = crc[1] = 0x2010;
      }
      
      HalFlashWrite((HAL_SB_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
      HalFlashRead(  HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
        HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
        (uint8 *)crc, sizeof(crc));
      if (crc[0] == crc[1])
      {
        sbReportState(SB_STATE_VERIFICATION_IMAGE_VALID);
      }
      else
      {
        sbReportState(SB_STATE_VERIFICATION_FAILED);
      }
    }
    else
    {
      sbReportState(SB_STATE_VERIFICATION_IMAGE_INVALID);
    }
	
    *time_spent_validating = TIME_IT_TAKES_TO_CALC_CRC;
  }
  else
  {
    *time_spent_validating = 0;
  }
  
  return (crc[0] == crc[1]);
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
* @return      SB_CMND_IDLE is no command was currently executed, otherwise, returns a constant
*              that indicates the command that was just executed. See "sbCmnd Result Codes"
**************************************************************************************************
*/
static uint8 sbCmnd(void)
{
  uint16 tmp = BUILD_UINT16(sbBuf[SB_DATA_STATE], sbBuf[SB_DATA_STATE+1]) + SB_IMG_OSET;
  uint16 crc[2];
  uint8 len = 1;
  uint8 rsp = SB_SUCCESS;
  uint8 rtrn = SB_CMND_UNSUPPORTED;
  uint8 *pBuf;
  static uint8 crcPatched = FALSE;
  
  
  switch (sbCmd2)
  {
    case SB_HANDSHAKE_CMD:
      rtrn = SB_CMND_HANDSHAKE_CMD;
      vddWait(VDD_MIN_NV);
      
      pBuf = &sbBuf[SB_DATA_STATE+1];
      UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, SB_BOOTLOADER_REVISION);
      *pBuf++ = SB_DEVICE_TYPE_2530;
      UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, SB_RW_BUF_LEN);
      UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, SB_DEVICE_PAGE_SIZE);
      UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, _sblRev);
      len += (pBuf - &sbBuf[SB_DATA_STATE+1]);
      break;
    
    case SB_WRITE_CMD:
      rtrn = SB_CMND_WRITE_CMD;
      vddWait(VDD_MIN_NV); // in case handshake was not performed. Anyhow, if already waited for this voltage, vddWait() will return immidiately
      
      if ((tmp % SB_WPG_SIZE) == 0)
      {
        HalFlashErase(tmp / SB_WPG_SIZE);
      }

      /* If the pre-calculated checksum is 0x0000, change it to 0xA5A5 when writing to flash.
         When calculating checksum at runtime, 0x0000 is also converted to 0xA5A5. This way
         0x0000 is never written as valid checksum in flash, which allows the main application
         to invalidate the image by writing 0x0000 to the crc shadow field in flash. */

      if (DOES_BUF_CONTAIN_PRECALC_CRC(tmp)) //the next flash write includes the pre-calculated CRC
      {
        pBuf = &sbBuf[SB_DATA_STATE + 2 + (HAL_SB_CRC_ADDR - ((uint32)tmp * HAL_FLASH_WORD_SIZE))];
		  
        if ((pBuf[0] == 0)
          && (pBuf[1] == 0))
        {
          crcPatched = TRUE;
          pBuf[0] = 0xA5;
          pBuf[1] = 0xA5;
        }
        else
        {
          crcPatched = FALSE;
        }
      }

      HalFlashWrite(tmp, sbBuf+SB_DATA_STATE+2, SB_RW_BUF_LEN / HAL_FLASH_WORD_SIZE);
      break;
    
    case SB_READ_CMD:
      rtrn = SB_CMND_READ_CMD;
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

      if (DOES_BUF_CONTAIN_PRECALC_CRC(tmp) //the current flash read block include the pre-calculated CRC
        && crcPatched)
      {
        pBuf = &sbBuf[SB_DATA_STATE + 3 + (HAL_SB_CRC_ADDR - ((uint32)tmp * HAL_FLASH_WORD_SIZE))];
		
        pBuf[0] = 0;
        pBuf[1] = 0;
      }
      break;
    
    case SB_ENABLE_CMD:
      HalFlashRead(HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
        HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
        (uint8 *)crc, sizeof(crc));
      
      // Bootload master must have verified extra checks to be issuing the SB_ENABLE_CMD.
      if ((crc[1] != crc[0]) || (crc[0] == 0xFFFF))
      {
        if (crc[0] == 0xFFFF)
        {
          crc[0] = 0x2010;
        }
        crc[1] = crc[0];
        HalFlashWrite((HAL_SB_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
        HalFlashRead(  HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
          HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
          (uint8 *)crc, sizeof(crc));
      }
      
      if (crc[0] == crc[1])
      {
        rtrn = SB_CMND_ENABLED_CMD_OK;
      }
      else
      {
        rtrn = SB_CMND_ENABLED_CMD_ERROR;
        rsp = SB_VALIDATE_FAILED;
      }
      break;

    case SB_SWITCH_BAUDRATE_CMD:
      if ((znpCfg1 == ZNP_CFG1_UART) || (znpCfg1 == ZNP_CFG1_UART_USB))
      {
        uint8 _U0BAUD = sbBuf[SB_DATA_STATE + 0];
        uint8 _U0GCR = sbBuf[SB_DATA_STATE + 1];
        
        sbResp(rsp, len); //first, send the response using the old baudrate
        
        if (znpCfg1 == ZNP_CFG1_UART)
        {
          while(sblIsUartTxPending())
          {
            sbUartPoll();
          }
          
          SLEEP(0x2600); //without this wait, the FCS is deformed, since the baudrate will change while the FCS is still in the TX fifo.
  
          U0BAUD = _U0BAUD;
          U0GCR = ((U0GCR&~0x1F) | _U0GCR);
         
          SLEEP(0x2600); //wait for the new baudrate to take effect
        }
        
        sbResp(rsp, len); //now, resend the response using the new baudrate

        return SB_CMND_SWITCH_BAUDRAT;
      }
      break;

    case SB_ENABLE_REPORTING_CMD:
      sbStateReportingEnabled = sbBuf[SB_DATA_STATE];
      if (sbStateReportingEnabled)
      {
        rtrn = SB_CMND_ENABLE_STATE_REPORTING;
      }
      break;

    default:
      return SB_CMND_UNSUPPORTED;
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
  rsp = len;
  len += SB_FCS_STATE - 1;

  for (idx = SB_CMD1_STATE; idx < len; idx++)
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
static uint16 calcCRC(uint8 * abort)
{
  uint32 addr;
  uint16 crc = 0;
  uint8 buf[512];
  int i;
  uint16 chunk_size = sizeof(buf);
  *abort = FALSE;
  uint8 sbExec_rc;
  
  if (znpCfg1 == ZNP_CFG1_UART)
  {
    URX0IE = 1;
    HAL_ENABLE_INTERRUPTS();
  }

  if ((znpCfg1 == ZNP_CFG1_UART) || (znpCfg1 == ZNP_CFG1_UART_USB))
  {
    sbReportState(SB_STATE_VERIFYING);
  }
  
  // Run the CRC calculation over the active body of code.
  for (addr = HAL_SB_IMG_ADDR; addr < HAL_SB_IMG_ADDR + HAL_SB_IMG_SIZE; addr += chunk_size)
  {
	if ((znpCfg1 == ZNP_CFG1_UART) || (znpCfg1 == ZNP_CFG1_UART_USB))
    {
      sbUartPoll();

      sbExec_rc = sbExec();
      if (sbExec_rc == SB_CMND_ENABLE_STATE_REPORTING)
      {
        sbReportState(SB_STATE_VERIFYING);
      }
      else if ((sbExec_rc != SB_CMND_IDLE) &&
               (sbExec_rc != SB_CMND_UNSUPPORTED) &&
               (sbExec_rc != SB_CMND_FORCE_RUN))
      {
          *abort = TRUE;
          break;
      }
    }
    
    if ((znpCfg1 == ZNP_CFG1_UART_USB) && (SB1_PRESS))
    {
      break;
    }
    
    HalFlashRead(addr / HAL_FLASH_PAGE_SIZE, addr % HAL_FLASH_PAGE_SIZE, buf, chunk_size);
    for (i = 0; i < chunk_size; i++)
    {
      if ((addr + i < HAL_SB_CRC_ADDR) || (addr + i >= HAL_SB_CRC_ADDR + HAL_SB_CRC_LEN))
      {
        crc = runPoly(crc, buf[i]);
      }
    }
  }
  
  // IAR note explains that poly must be run with value zero for each byte of crc.
  crc = runPoly(crc, 0);
  crc = runPoly(crc, 0);
  
  if (znpCfg1 == ZNP_CFG1_UART)
  {
    HAL_DISABLE_INTERRUPTS();
    URX0IE = 0;
  }

  if (crc == 0)
  {
  	return 0xA5A5;
  }
  
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
