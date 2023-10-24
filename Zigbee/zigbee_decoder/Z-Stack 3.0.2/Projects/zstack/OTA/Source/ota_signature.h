/******************************************************************************
  Filename:       ota_signature.h
  Revised:        $Date: 2010-11-18 08:17:09 -0800 (Thu, 18 Nov 2010) $
  Revision:       $Revision: 24437 $

  Description:    This file contains code to calculate and verify OTA
                  signatures based on teh MMO AES Hash function.


  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
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
******************************************************************************/

#ifndef OTA_SIGNATURE_H
#define OTA_SIGNATURE_H

#include "hal_types.h"

#define OTA_MMO_ROUNDS                1
#define OTA_MMO_BLOCKSIZE             32
#define OTA_MMO_HASH_SIZE             16

#define OTA_MMO_IN_PROCESS            0
#define OTA_MMO_ERROR                 1
#define OTA_MMO_COMPLETE              2

#define OTA_SIGNATURE_LEN             42
#define OTA_SIGNATURE_ELEM_LEN        (OTA_SIGNATURE_LEN + Z_EXTADDR_LEN)
#define OTA_CERTIFICATE_LEN           48

typedef struct
{
  uint8 hash[OTA_MMO_HASH_SIZE];
  uint32 length;
} OTA_MmoCtrl_t;


#ifdef __cplusplus
extern "C"
{
#endif

// Entry functions
extern void OTA_CalculateMmoR3(OTA_MmoCtrl_t *pCtrl, uint8 *pData, uint8 len, uint8 lastBlock);
extern uint8 OTA_ValidateSignature(uint8 *pHash, uint8* pCert, uint8 *pSig, uint8 *pIEEE);
void sspMMOHash2 (uint8 *Pb, uint8 prefix, uint8 *Mb, uint16 bitlen, uint8 *Cstate);

#ifdef __cplusplus
}
#endif

#endif // OTA_SIGNATURE_H
