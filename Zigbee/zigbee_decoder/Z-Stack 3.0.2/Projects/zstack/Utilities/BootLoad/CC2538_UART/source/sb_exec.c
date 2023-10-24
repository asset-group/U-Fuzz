/**************************************************************************************************
  Filename:       sb_exec.c
  Revised:        $Date: 2013-06-27 15:44:38 -0700 (Thu, 27 Jun 2013) $
  Revision:       $Revision: 34663 $

  Description:    Serial Bootloader Executive.

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

#include <string.h>

#include "flash.h"
#include "hal_board_cfg.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#if !defined MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
#define MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA TRUE
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */

// _sbBuf is defined as an array of uint32, in order for it to be 32bit-alligned. SB_BUF_SIZE is 
// specified in bytes. the calculation ((x+3)/4) gives the minimal number of 32bit words required 
// to store at least x bytes.
uint32 _sbBuf[(SB_BUF_SIZE + 3) / 4];
uint8 *sbBuf = (uint8 *)_sbBuf; // This is a 32bit alligned buffer of uint8's

// page_deleted stores a one-bit field per each page in flash, to keep track of the pages that the 
// bootloader have already erased during operation
static uint8 page_deleted[(SB_DEVICE_FLASH_SIZE / HAL_FLASH_PAGE_SIZE) /8];

/* ------------------------------------------------------------------------------------------------
 *                                       External Variables
 * ------------------------------------------------------------------------------------------------
 */

extern sbl_hdr_t *sbl_header_ptr;


/* ------------------------------------------------------------------------------------------------
 *                                       Local Macros
 * ------------------------------------------------------------------------------------------------
 */

// Check the bit in page_deleted that corresponds to the given page number, to see whether this 
// page was already erased by the bootloader
#define IS_PAGE_ERASED(_pageNumber) ((page_deleted[(_pageNumber) / 8] >> (_pageNumber % 8)) & 0x01)

// Set the bit in page_deleted that corresponds to the given page number, to mark that this page was erased by the bootloader
#define MARK_PAGE_ERASED(_pageNumber) do { \
	                                      page_deleted[(_pageNumber) / 8] |= (1 << ((_pageNumber) % 8));\
                                          } while (0)

#define GET_PAGE_NUMBER(_addr) (((uint32)(_addr) - FLASH_BASE) / HAL_FLASH_PAGE_SIZE)
#define GET_PAGE_ADDRESS(_pageNumber) (FLASH_BASE + ((_pageNumber) * HAL_FLASH_PAGE_SIZE))


/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static uint32 imageCrcCalc(uint8 *imageStart, uint8 *imageEnd);
static bool enableImg(void);
static bool sbCmnd(uint8 sbCmd, uint32 payloadLen);
static void sbResp(uint8 sbCmd, uint8 rsp, uint32 payloadLen);


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
 */
bool sbExec(void)
{
  // Parse the incoming packet’s header, and verify its FCS. Valid packets are transferred to 
  // sbCmnd() for further processing. For detailed packet format, please see the document "Serial 
  // Boot Loader for CC2538".
  static uint8 sbFrameId, sbCmd, sbFcs, sbSte = SB_SOF_STATE;
  static uint32 sbLen;
  static uint32 sbIdx;
  uint8 ch, rtrn = FALSE;
  
  while (SB_RX(&ch))
  {
    switch (sbSte)
    {
    case SB_SOF_STATE:
      if (SB_SOF == ch)
      {
        sbSte = SB_LEN_STATE;
        sbIdx = 0;
      }
      break;
      
    case SB_LEN_STATE: // this field is kept for backward compatibility of the protocol. 
                       // In case the length is larger than 254, this fields is set to 0xFF, 
		       // and there is an additional 32bit length, located in another location 
		       // in this header.
      sbLen = ch;
      sbFcs = 0;
      sbSte = SB_FRAME_ID_STATE;
      break;
      
    case SB_FRAME_ID_STATE:
      sbFrameId = ch;
      sbSte = SB_CMD_STATE;
      break;
      
    case SB_CMD_STATE:
      sbCmd = ch;
      
      switch (sbLen)
      {
      case 0:
        sbSte = SB_FCS_STATE;
        break;
      case 0xFF:
        sbSte = SB_LEN1_STATE;
        break;
      default:
        sbSte = SB_DATA_STATE;
        break;
      }
      break;
      
    case SB_LEN1_STATE:
      sbLen = ch;
      sbSte = SB_LEN2_STATE;
      break;
      
    case SB_LEN2_STATE:
      sbLen += ch << 8;
      sbSte = SB_LEN3_STATE;
      break;
      
    case SB_LEN3_STATE:
      sbLen += ch << 16;
      sbSte = SB_LEN4_STATE;
      break;
      
    case SB_LEN4_STATE:
      sbLen += ch << 24;
      sbSte = (sbLen) ? SB_DATA_STATE : SB_FCS_STATE;
      break;
      
    case SB_DATA_STATE:
      if (sbIdx >= sizeof(_sbBuf))
      {
        sbSte = SB_SOF_STATE; //discard this packet. the payload is too long.
      }
      else
      {
        sbBuf[sbIdx++] = ch;
        
        if (sbIdx == sbLen)
        {
          sbSte = SB_FCS_STATE;
        }
      }
      break;
      
    case SB_FCS_STATE:
      if ((sbFcs == ch) && (sbFrameId == SB_RPC_SYS_BOOT))
      {
        rtrn = sbCmnd(sbCmd, sbLen);
      }
      
      sbSte = SB_SOF_STATE;
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
 * @brief       Check whether the application image is valid. Calculate CRC to verify this, if not already done so.
 *
 * input parameters
 *
 * @param       none
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE or FALSE for image valid.
 */
bool sbImgValid(void)
{
  uint32_t imageVerified = SB_IMG_VERIFIED;
  
  if ((sbl_header_ptr->img_status == SB_IMG_PROGRAMMED) && 
      (sbl_header_ptr->checksum == imageCrcCalc((uint8 *)sbl_header_ptr->checksum_begin, 
      (uint8 *)sbl_header_ptr->checksum_end)))
  {
    (void)FlashMainPageProgram(&imageVerified, (uint32)&sbl_header_ptr->img_status, 4);
  }
  
  return (sbl_header_ptr->img_status == SB_IMG_VERIFIED); // Also act as a verification for the write of the 'verified' status
}


/**************************************************************************************************
 * @fn          imageCrcCalc
 *
 * @brief       Run the 0x18005 Polynomial CRC-16 calculation over the area specified.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The CRC-16 calculated.
 */
static uint32 imageCrcCalc(uint8 *imageStart, uint8 *imageEnd)
{
  uint8 *pBuf = imageStart;
  uint8 *excludeStart = (uint8 *)&(((sbl_hdr_t *)SBL_HEADER_PTR)->checksum);
  // The length of the checksum field which is later patched by the linker with the actual 
  // calculated checksum is 2 bytes. These bytes are skipped when calculating the checksum.
  uint32 excludeLength = 2;
  uint16 crc = 0;
  bool calc_done = (imageEnd < imageStart); // if imageEnd < imageStart, the range is empty, so no need to calculate.
  
  // CRC seed of 0x0000.
  RNDL = 0x00;
  RNDL = 0x00;
  
  while (!calc_done)
  {
    if ((excludeLength == 0) || (pBuf < excludeStart))
    {
      RNDH = *pBuf;
    }
    else
    {
      excludeLength--;
    }
    
    calc_done = (pBuf == imageEnd);
    
    pBuf++;
  }
  
  crc = RNDH; // Build the crc in 2 steps, to prevent compiler warning about the order of volatile access
  crc = (crc << 8) | RNDL;
  return crc;
}


/**************************************************************************************************
 * @fn          enableImg
 *
 * @brief       Flag the application image as already verified (no need to calcuate CRC)
 *
 * input parameters
 *
 * @param       none
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE or FALSE for image valid.
 */
static bool enableImg(void)
{
  uint32_t imageVerified = SB_IMG_VERIFIED;

  // Write the 4-bytes "magic-word" SB_IMG_VERIFIED to the appropriate location in flash, 
  // to specify that the image passed verification
  (void)FlashMainPageProgram(&imageVerified, (uint32)&sbl_header_ptr->img_status, 4);
  
  return (sbl_header_ptr->img_status == SB_IMG_VERIFIED); // Verify the write.
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
 */
static bool sbCmnd(uint8 sbCmd, uint32 payload_len)
{
  uint32 firstAddr;
  uint32 lastAddr;
  uint32 operationLength;
  uint32 writeLength;
  uint32 respPayloadLen = 0;
  uint32 pageNumber;
  uint32 i;
  uint32 actual_number_of_data_bytes_to_send;
  uint8 paddingLength;
  uint8 rsp = SB_SUCCESS;
  uint8 imageEnabledSuccessfully = FALSE;
  uint8 *pBuf;
  
  pBuf = sbBuf;
  
  switch (sbCmd)
  {
  case SB_HANDSHAKE_CMD:
    memset(page_deleted, 0, sizeof(page_deleted)); // Mark all pages as not-deleted-yet
    
    UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, SB_BOOTLOADER_REVISION);
    *pBuf++ = SB_DEVICE_TYPE_2538;
    UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, SB_RW_BUF_LEN);
    UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, SB_DEVICE_PAGE_SIZE);
    respPayloadLen = pBuf - sbBuf;
    break;
    
  case SB_WRITE_CMD:
    firstAddr = BUF_TO_UINT32_LITTLE_ENDIAN(pBuf);
    operationLength = BUF_TO_UINT32_LITTLE_ENDIAN(pBuf);
    writeLength = payload_len - (pBuf - sbBuf); // The payload_len includes the addr_offset 
                                                 // and the operationLength fields. The value 
						 // (pBuf - sbBuf) gives the number of bytes 
						 // used by those firelds. The remaining bytes 
						 // are the actual data bytes to be written. 
    lastAddr = firstAddr + operationLength - 1;
    if ((firstAddr < FLASH_BASE) || 
        (lastAddr > CC2538_CODE_FLASH_END_ADDRESS) || 
	(writeLength > operationLength))
    {
      rsp = SB_FAILURE;
      break;
    }
    
    // Before writing to a flash page for the first time during a bootloading session, the 
    // page must be erased. The following section makes sure that every page being written 
    // to have already been erased, otherwise, it erases it (before writing to it).
    // Note that the write command may span over more than a single page.
    for (pageNumber = GET_PAGE_NUMBER(firstAddr); pageNumber <= GET_PAGE_NUMBER(lastAddr); pageNumber++)
    {
      if (!IS_PAGE_ERASED(pageNumber))
      {
        if (FlashMainPageErase(GET_PAGE_ADDRESS(pageNumber)) != 0)
        {
          rsp = SB_FAILURE;
          break;
        }
        
        MARK_PAGE_ERASED(pageNumber);
      }
    }
    
    // Note that the start address (firstAddr) and the byte count (writeLength) must be 
    // word aligned. The start address is expected to be already aligned (by the SBL server), 
    // since aligning it here would require padding the buffer's start, which would require 
    // shifting the buffer content (as the buffer is passesd as (uint32_t *pui32Data) so it 
    // should be aligned by itself. The byte count is aligned below.
    paddingLength = ((4 - (writeLength & 0x00000003)) % 4);
    for (i = 0; i < paddingLength; i++)
    {
      pBuf[writeLength + i] = 0xFF;
    }
    writeLength += paddingLength;
    
    
    // If the page was successfully erased (or was previously erased), perform the write action.
    // Note that pBuf must point to a uint32-aligned address, as required by FlashMainPageProgram(). 
    // This is the case now (the prefixing field are total of 8 bytes), and _sbBuf is 32bit aligned.
    if ((rsp == SB_SUCCESS) && (writeLength > 0) && 
        (FlashMainPageProgram((uint32_t *)(pBuf), firstAddr, writeLength) != 0))
    {
      rsp = SB_FAILURE;
    }
    
    break;
    
  case SB_READ_CMD:
    firstAddr = BUF_TO_UINT32_LITTLE_ENDIAN(pBuf);
    operationLength = BUF_TO_UINT32_LITTLE_ENDIAN(pBuf);
    lastAddr = firstAddr + operationLength - 1;
    
    if ((firstAddr < FLASH_BASE) || 
        (lastAddr > CC2538_CODE_FLASH_END_ADDRESS) || 
	(operationLength > sizeof(_sbBuf)))
    {
      rsp = SB_FAILURE;
      break;
    }
    
#if !MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
#warning This limitation on the SBL read capability is permanent - will NV size ever change?
#warning This check assumes NV PAGES located at the end of the program flash memory
    if (GET_PAGE_NUMBER(lastAddr) >= HAL_NV_PAGE_BEG)
    {
      rsp = SB_FAILURE;
      break;
    }
#endif
    
    // If the end of the buffer is made only of 0xFF characters, no need to send them. Find 
    // out the number of bytes that needs to be sent:	
    for (actual_number_of_data_bytes_to_send = operationLength; 
	 (actual_number_of_data_bytes_to_send > 0) && ((*(uint8 *)(firstAddr + actual_number_of_data_bytes_to_send - 1)) == 0xFF); 
	 actual_number_of_data_bytes_to_send--);
    
    // As a future upgrade, memcopy can be avoided. Instead, may pass a pointer to the actual flash address
    (void)memcpy(pBuf, (const void *)firstAddr, actual_number_of_data_bytes_to_send);
    respPayloadLen = (pBuf - sbBuf) + actual_number_of_data_bytes_to_send;
    break;
    
  case SB_ENABLE_CMD:
    if (enableImg())
    {
      imageEnabledSuccessfully = TRUE;
    }
    else
    {
      rsp = SB_VALIDATE_FAILED;
    }
    break;
    
  default:
    break;
  }
  
  sbResp(sbCmd, rsp, respPayloadLen);
  return imageEnabledSuccessfully;
}


/**************************************************************************************************
 * @fn          sbResp
 *
 * @brief       Make the SB response.
 *
 * input parameters
 *
 * @param       sbCmd - the command to respond to
 * @param       rsp - The byte code response to send.
 * @param       payload_len - The data length of the response.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void sbResp(uint8 sbCmd, uint8 rsp, uint32 payload_len)
{
  uint8 fcs = 0;
  uint8 headerBuf[10];
  uint8 * pBuf = headerBuf;
  uint32 reported_len; // Include the payload len + one byte for rsp
  uint32 header_len;
  uint32 i;
  
  reported_len = payload_len + 1; 
  
  *pBuf++ = SB_SOF;
  if (reported_len < 0xFF)
  {
    *pBuf++ = (uint8)(reported_len & 0xFF);
  }
  else
  {
    *pBuf++ = 0xFF;
  }
  *pBuf++ = SB_RPC_SYS_BOOT;

  // The MSB of the command field is set, to mark that this is a reply
  *pBuf++ = sbCmd | 0x80;
  if (reported_len >= 0xFF)
  {
    UINT32_TO_BUF_LITTLE_ENDIAN(pBuf, reported_len);
  }
  *pBuf++ = rsp;
  
  header_len = pBuf - headerBuf;
  
  // Caluclate the fcs
  for (i = 1; i < header_len; i++) // The SOF char is not part of the checksum
  {
    fcs ^= headerBuf[i];
  }
  
  for (i = 0; i < payload_len; i++)
  {
    fcs ^= sbBuf[i];
  }
  
  while (SB_TX(headerBuf, header_len) == 0);
  if (payload_len > 0)
  {
    while (SB_TX(sbBuf, payload_len) == 0);
  }
  while (SB_TX(&fcs, 1) == 0);
}

/**************************************************************************************************
 */
