/******************************************************************************
  Filename:       ota_common.c
  Revised:        $Date: 2011-08-16 16:45:58 -0700 (Tue, 16 Aug 2011) $
  Revision:       $Revision: 27200 $

  Description:    This file contains code common to the OTA server,
                  client, and dongle.


  Copyright 2010-2011 Texas Instruments Incorporated. All rights reserved.

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
******************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "ota_common.h"

#ifdef _WIN32
#include <string.h>
#define osal_memcpy  memcpy
#define osal_strlen  strlen
#else
#include "osal.h"
#endif

/******************************************************************************
 * MACROS
 */
#define UINT16_BREAK_HEX(a, b)   (((a) >> (12-(4*(b)))) & 0xF)
#define UINT32_BREAK_HEX(a, b)   (((a) >> (28-(4*(b)))) & 0xF)

/*********************************************************************
 * CONSTANTS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */
static char HEX_char[] = "0123456789ABCDEF";

/******************************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 char2uint(char c);

/******************************************************************************
 * @fn      char2uint
 *
 * @brief   Converts a hex character to a uint8
 *
 * @param   c - Character to convert
 *
 * @return  uint8 value of c
 */
static uint8 char2uint(char c)
{
  if (c >= '0' && c <= '9')
  {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f')
  {
    return 0xA + c - 'a';
  }
  if (c >= 'A' && c <= 'F')
  {
    return 0xA + c - 'A';
  }

  return 0;
}

/******************************************************************************
 * @fn      OTA_ParseHeader
 *
 * @brief   Reads the OTA header from the input buffer.
 *
 * @param   pHdr - pointer to the header information
 * @param   pBuf - pointer to the input buffer
 *
 * @return  new buffer pointer
 */
uint8 *OTA_ParseHeader(OTA_ImageHeader_t *pHdr, uint8 *pBuf)
{
  uint8 i;

  // Get the Magic Number
  pHdr->magicNumber = BUILD_UINT32(pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
  pBuf += 4;

  // Get the Header Version
  pHdr->headerVersion = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  // Get the Header Length
  pHdr->headerLength = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  // Get the Field Control
  pHdr->fieldControl = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  // Get the Manufacturer ID
  pHdr->fileId.manufacturer = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  // Get the Image Type
  pHdr->fileId.type = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  // Get the File Version
  pHdr->fileId.version = BUILD_UINT32(pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
  pBuf += 4;

  // Get the Stack Version
  pHdr->stackVersion = BUILD_UINT16(pBuf[0], pBuf[1]);
  pBuf += 2;

  // Get the Header string
  for (i=0; i<OTA_HEADER_STR_LEN; i++)
  {
    pHdr->headerString[i] = *pBuf++;
  }

  // Get the Image Size
  pHdr->imageSize = BUILD_UINT32(pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
  pBuf += 4;

  // Get the Security Credential Version
  if (pHdr->fieldControl & OTA_FC_SCV_PRESENT)
  {
    pHdr->secCredentialVer = *pBuf++;
  }

  // Get the Upgrade File Destination
  if (pHdr->fieldControl & OTA_FC_DSF_PRESENT)
  {
    for (i=0; i<Z_EXTADDR_LEN; i++)
    {
      pHdr->destIEEE[i] = *pBuf++;
    }
  }

  // Get the Min and Max Hardware Versions
  if (pHdr->fieldControl & OTA_FC_HWV_PRESENT)
  {
    pHdr->minHwVer = BUILD_UINT16(pBuf[0], pBuf[1]);
    pBuf += 2;
    pHdr->maxHwVer = BUILD_UINT16(pBuf[0], pBuf[1]);
    pBuf += 2;
  }

  return pBuf;
}

/******************************************************************************
 * @fn      OTA_WriteHeader
 *
 * @brief   Writes the OTA header to the output buffer.
 *
 * @param   pHdr - pointer to the header information
 * @param   pHdr - pointer to the output buffer
 *
 * @return  none
 */
uint8 *OTA_WriteHeader(OTA_ImageHeader_t *pHdr, uint8 *pBuf)
{
  uint8 i;

  // Output the Magic Number
  // osal_buffer_uint32 not available under windows
  //pBuf = osal_buffer_uint32(pBuf, pHdr->magicNumber);
  *pBuf++ = BREAK_UINT32(pHdr->magicNumber, 0);
  *pBuf++ = BREAK_UINT32(pHdr->magicNumber, 1);
  *pBuf++ = BREAK_UINT32(pHdr->magicNumber, 2);
  *pBuf++ = BREAK_UINT32(pHdr->magicNumber, 3);

  // Output the Header Version
  *pBuf++ = LO_UINT16(pHdr->headerVersion);
  *pBuf++ = HI_UINT16(pHdr->headerVersion);

  // Output the Header Length
  *pBuf++ = LO_UINT16(pHdr->headerLength);
  *pBuf++ = HI_UINT16(pHdr->headerLength);

  // Output the Field Control
  *pBuf++ = LO_UINT16(pHdr->fieldControl);
  *pBuf++ = HI_UINT16(pHdr->fieldControl);

  // Output the Manufacturer ID
  *pBuf++ = LO_UINT16(pHdr->fileId.manufacturer);
  *pBuf++ = HI_UINT16(pHdr->fileId.manufacturer);

  // Output the Image Type
  *pBuf++ = LO_UINT16(pHdr->fileId.type);
  *pBuf++ = HI_UINT16(pHdr->fileId.type);

  // Output the File Version
  // osal_buffer_uint32 not available under windows
  //pBuf = osal_buffer_uint32(pBuf, pHdr->fileId.version);
  *pBuf++ = BREAK_UINT32(pHdr->fileId.version, 0);
  *pBuf++ = BREAK_UINT32(pHdr->fileId.version, 1);
  *pBuf++ = BREAK_UINT32(pHdr->fileId.version, 2);
  *pBuf++ = BREAK_UINT32(pHdr->fileId.version, 3);

  // Output the Stack Version
  *pBuf++ = LO_UINT16(pHdr->stackVersion);
  *pBuf++ = HI_UINT16(pHdr->stackVersion);

  // Output the Header string
  for (i=0; i<OTA_HEADER_STR_LEN; i++)
  {
    *pBuf++ = pHdr->headerString[i];
  }

  // Output the Image Size
  // osal_buffer_uint32 not available under windows
  //pBuf = osal_buffer_uint32(pBuf, pHdr->imageSize);
  *pBuf++ = BREAK_UINT32(pHdr->imageSize, 0);
  *pBuf++ = BREAK_UINT32(pHdr->imageSize, 1);
  *pBuf++ = BREAK_UINT32(pHdr->imageSize, 2);
  *pBuf++ = BREAK_UINT32(pHdr->imageSize, 3);

  // Output the Security Credential Version
  if (pHdr->fieldControl & OTA_FC_SCV_PRESENT)
  {
    *pBuf++ = pHdr->secCredentialVer;
  }

  // Output the Upgrade File Destination
  if (pHdr->fieldControl & OTA_FC_DSF_PRESENT)
  {
    for (i=0; i<Z_EXTADDR_LEN; i++)
    {
      *pBuf++ = pHdr->destIEEE[i];
    }
  }

  // Output the Min and Max Hardware Versions
  if (pHdr->fieldControl & OTA_FC_HWV_PRESENT)
  {
    *pBuf++ = LO_UINT16(pHdr->minHwVer);
    *pBuf++ = HI_UINT16(pHdr->minHwVer);

    *pBuf++ = LO_UINT16(pHdr->maxHwVer);
    *pBuf++ = HI_UINT16(pHdr->maxHwVer);
  }

  return pBuf;
}

/******************************************************************************
 * @fn      OTA_GetFileName
 *
 * @brief   Get the text of a filename from the file ID (zclOTA_FileID_t)
 *
 * @param   pName - Buffer to hold the name
 *          pFileID - Pointer to File ID structure
 *          text - Text description of file
 *
 * @return  none
 */
void OTA_GetFileName(char *pName, zclOTA_FileID_t *pFileId, char *text)
{
  int8 i, len;

  // Insert the manufacturer ID
  if (pFileId->manufacturer == 0xFFFF)
  {
    *pName++ = '*';
  }
  else
  {
    for (i=0; i<4; i++)
    {
      *pName++ = HEX_char[UINT16_BREAK_HEX(pFileId->manufacturer, i)];
    }
  }

  *pName++ = '-';

  // Insert the type
  if (pFileId->type == 0xFFFF)
  {
    *pName++ = '*';
  }
  else
  {
    for (i=0; i<4; i++)
    {
      *pName++ = HEX_char[UINT16_BREAK_HEX(pFileId->type, i)];
    }
  }

  *pName++ = '-';

  // Insert the type
  if (pFileId->version == 0xFFFFFFFF)
  {
    *pName++ = '*';
  }
  else
  {
    for (i=0; i<8; i++)
    {
      *pName++ = HEX_char[UINT32_BREAK_HEX(pFileId->version, i)];
    }
  }

  if (text)
  {
    len = (uint8) osal_strlen(text);

    if (len)
    {
      *pName++ = '-';
      osal_memcpy (pName, text, len);
      pName += len;
    }
  }

  osal_memcpy (pName, ".zigbee", 8);
}


/******************************************************************************
 * @fn      OTA_SplitFileName
 *
 * @brief   Get the file ID of an image from the filename text
 *
 * @param   pName - Buffer to hold the name
 *          pFileID - Pointer to File ID structure
 *
 * @return  none
 */
void OTA_SplitFileName(char *pName, zclOTA_FileID_t *pFileId)
{
  // The OTA Upgrade image file name should contain the following information
  // at the beginning of the name with each field separated by a dash ("-"): 
  // manufacturer code, image type and file version. The value of each field
  // stated should be in hexadecimal number and in capital letter. Each 
  // manufacturer may append more information to the name as seen fit to
  // make the name more specific. The OTA Upgrade file extension should be
  // ".zigbee". An example of OTA Upgrade image file name and extension is
  // "1001-00AB-10053519-upgradeMe.zigbee".
  if (pName && pFileId)
  {
    uint8 len = (uint8) osal_strlen(pName);

    if (len >= 19)
    {
      uint8 i;

      pFileId->manufacturer = 0;
      for (i=0; i<4; i++)
      {
        pFileId->manufacturer |= ((uint16) char2uint(*pName++)) << (12 - (4*i));
      }

      pName++;
      pFileId->type = 0;
      for (i=0; i<4; i++)
      {
        pFileId->type |= ((uint16) char2uint(*pName++)) << (12 - (4*i));
      }

      pName++;
      pFileId->version = 0;
      for (i=0; i<8; i++)
      {
        pFileId->version |= ((uint32) char2uint(*pName++)) << (28 - (4*i));
      }
    }
  }
}

/******************************************************************************
 * @fn      OTA_FileIdToStream
 *
 * @brief   Writes a file ID to a stream
 *
 * @param   pFileId - File ID
 *          pStream - Stream
 *
 * @return  new stream pointer
 */
uint8 *OTA_FileIdToStream(zclOTA_FileID_t *pFileId, uint8 *pStream)
{
  if (pStream)
  {
    *pStream++ = LO_UINT16(pFileId->manufacturer);
    *pStream++ = HI_UINT16(pFileId->manufacturer);

    *pStream++ = LO_UINT16(pFileId->type);
    *pStream++ = HI_UINT16(pFileId->type);

    // osal_buffer_uint32 not available under windows
    //pStream = osal_buffer_uint32(pStream, pFileId->version);
    *pStream++ = BREAK_UINT32(pFileId->version, 0);
    *pStream++ = BREAK_UINT32(pFileId->version, 1);
    *pStream++ = BREAK_UINT32(pFileId->version, 2);
    *pStream++ = BREAK_UINT32(pFileId->version, 3);
  }

  return pStream;
}

/******************************************************************************
 * @fn      OTA_StreamToFileId
 *
 * @brief   Reads a file ID from a stream
 *
 * @param   pFileId - File ID
 *          pStream - Stream
 *
 * @return  new stream pointer
 */
uint8 *OTA_StreamToFileId(zclOTA_FileID_t *pFileId, uint8 *pStream)
{
  if (pStream)
  {
    pFileId->manufacturer = BUILD_UINT16(pStream[0], pStream[1]);
    pStream += 2;
    pFileId->type = BUILD_UINT16(pStream[0], pStream[1]);
    pStream += 2;
    pFileId->version = BUILD_UINT32(pStream[0], pStream[1], pStream[2], pStream[3]);
    pStream += 4;
  }

  return pStream;
}

/******************************************************************************
 * @fn      OTA_AfAddrToStream
 *
 * @brief   Writes an afAddrType_t to a stream.
 *
 * @param   pAddr - Address
 *          pStream - Stream
 *
 * @return  new stream pointer
 *****************************************************************************/
uint8 *OTA_AfAddrToStream(afAddrType_t *pAddr, uint8 *pStream)
{
  if (pAddr && pStream)
  {
    *pStream++ = pAddr->addrMode;

    if (pAddr->addrMode == afAddr16Bit)
    {
      *pStream++ = LO_UINT16(pAddr->addr.shortAddr);
      *pStream++ = HI_UINT16(pAddr->addr.shortAddr);
    }
    else if (pAddr->addrMode == afAddr64Bit)
    {
      osal_memcpy(pStream, pAddr->addr.extAddr, Z_EXTADDR_LEN);
      pStream += Z_EXTADDR_LEN;
    }

    *pStream++ = pAddr->endPoint;

    *pStream++ = LO_UINT16(pAddr->panId);
    *pStream++ = HI_UINT16(pAddr->panId);
  }

  return pStream;
}

/******************************************************************************
 * @fn      OTA_StreamToAfAddr
 *
 * @brief   Reads an afAddrType_t from a stream.
 *
 * @param   pAddr - Address
 *          pStream - Stream
 *
 * @return  new stream pointer
 *****************************************************************************/
uint8 *OTA_StreamToAfAddr(afAddrType_t *pAddr, uint8 *pStream)
{
  if (pAddr && pStream)
  {
    pAddr->addrMode = (afAddrMode_t) *pStream++;

    if (pAddr->addrMode == afAddr16Bit)
    {
      pAddr->addr.shortAddr = BUILD_UINT16(pStream[0], pStream[1]);
      pStream+= 2;
    }
    else if (pAddr->addrMode == afAddr64Bit)
    {
      osal_memcpy(pAddr->addr.extAddr, pStream, Z_EXTADDR_LEN);
      pStream += Z_EXTADDR_LEN;
    }

    pAddr->endPoint = *pStream++;
    pAddr->panId = BUILD_UINT16(pStream[0], pStream[1]);
    pStream+= 2;
  }

  return pStream;
}
