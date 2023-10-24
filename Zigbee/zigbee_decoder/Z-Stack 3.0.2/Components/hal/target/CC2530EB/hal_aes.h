/**************************************************************************************************
  Filename:       hal_aes.h
  Revised:        $Date: 2011-11-08 11:26:03 -0800 (Tue, 08 Nov 2011) $
  Revision:       $Revision: 28264 $

  Description:    Support for HW/SW AES encryption.


  Copyright 2007-2011 Texas Instruments Incorporated. All rights reserved.

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

#ifndef     HAL_AES_H_
#define     HAL_AES_H_

#include "ZComDef.h"

#define     STATE_BLENGTH   16      // Number of bytes in State
#define     KEY_BLENGTH     16      // Number of bytes in Key
#define     KEY_EXP_LENGTH  176     // Nb * (Nr+1) * 4

/* AES Engine is default to hardware AES. To turn on software AES, #define one of the followings:
 * #define SOFTWARE_AES TRUE, uses software aes  ( slowest setting )
 * #define SW_AES_AND_KEY_EXP TRUE, enables software aes with key expansion ( improves speed at the cost of 176 bytes of data (RAM) )
 */
#if ((defined SOFTWARE_AES) && (SOFTWARE_AES == TRUE)) && ((defined SW_AES_AND_KEY_EXP) && (SW_AES_AND_KEY_EXP == TRUE))
#error "SOFTWARE_AES and SW_AES_AND_KEY_EXP cannot be both defined."
#endif

extern void HalAesInit( void );
extern void AesLoadBlock( uint8 * );
extern void AesStartBlock( uint8 *, uint8 * );
extern void AesStartShortBlock( uint8 *, uint8 * );
extern void AesLoadIV(uint8 *);
extern void AesDmaSetup( uint8 *, uint16, uint8 *, uint16 );
extern void AesLoadKey( uint8 * );

extern void (*pSspAesEncrypt)( uint8 *, uint8 * );
extern void ssp_HW_KeyInit (uint8 *);
extern void sspKeyExpansion (uint8 *, uint8 *);

extern void sspAesEncryptHW (uint8 *, uint8 *);
extern void sspAesEncryptKeyExp (uint8 *, uint8 *);
extern void sspAesEncryptBasic (uint8 *, uint8 *);
extern void sspAesEncrypt( uint8 *key, uint8 *buf );

extern void sspAesDecryptHW( uint8 *, uint8 * );
extern void sspAesDecrypt( uint8 *key, uint8 *buf );

#define AES_BUSY    0x08
#define ENCRYPT     0x00
#define DECRYPT     0x01

// Macro for setting the mode of the AES operation
#define AES_SETMODE(mode) do { ENCCS &= ~0x70; ENCCS |= mode; } while (0)

// _mode_ is one of
#define CBC         0x00
#define CFB         0x10
#define OFB         0x20
#define CTR         0x30
#define ECB         0x40
#define CBC_MAC     0x50

// Macro for starting or stopping encryption or decryption
#define AES_SET_ENCR_DECR_KEY_IV(mode) \
   do {                                \
    ENCCS = (ENCCS & ~0x07) | mode     \
   } while(0)

// Where _mode_ is one of
#define AES_ENCRYPT     0x00;
#define AES_DECRYPT     0x02;
#define AES_LOAD_KEY    0x04;
#define AES_LOAD_IV     0x06;

// Macro for starting the AES module for either encryption, decryption,
// key or initialisation vector loading.
#define AES_START()     ENCCS |= 0x01

/* Used by DMA macros to shift 1 to create a mask for DMA registers. */
#define  HAL_DMA_AES_IN    1
#define  HAL_DMA_AES_OUT   2

/* AES registers */
#define  HAL_AES_IN_ADDR   0x70B1
#define  HAL_AES_OUT_ADDR  0x70B2

#if !defined (HAL_AES_DMA) || (HAL_AES_DMA == FALSE)
#define HAL_AES_DELAY()   \
  do {                    \
       uint8 delay = 15;  \
       while(delay--);    \
     } while(0)
#endif

// End of CC2530 hardware AES engine definitions

#endif  // HAL_AES_H_
