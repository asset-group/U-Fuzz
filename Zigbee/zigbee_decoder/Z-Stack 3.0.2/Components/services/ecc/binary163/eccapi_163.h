/**************************************************************************************************
  Filename:       eccapi_163.h
  Revised:        $Date: 2014-12-30 18:02:51 -0800 (Tue, 30 Dec 2014) $
  Revision:       $Revision: 41589 $

  Description:    This file abstracts the Certicom API.


  Copyright 2014 Texas Instruments Incorporated. All rights reserved.

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
#ifndef ECCAPI_163_H
#define ECCAPI_163_H

#ifdef __cplusplus
extern "C" {
#endif


// Suite 1 Security Information
#define ECCAPI_PUBLIC_KEY_163_LEN    22
#define ECCAPI_PRIVATE_KEY_163_LEN   21
#define ECCAPI_CERT_163_LEN          48
#define ECCAPI_CERT_163_EXT_ADDR_IDX 22

#define MCE_SUCCESS                0x00
#define MCE_ERR_FAIL_VERIFY        0x01
#define MCE_ERR_NULL_PRIVATE_KEY   0x02
#define MCE_ERR_NULL_PUBLIC_KEY    0x03
#define MCE_ERR_NULL_INPUT_BUF     0x04
#define MCE_ERR_NULL_OUTPUT_BUF    0x05
#define MCE_ERR_NULL_FUNC_PTR      0x06
#define MCE_ERR_NULL_EPHEM_PRI_KEY 0x07
#define MCE_ERR_NULL_EPHEM_PUB_KEY 0x08
#define MCE_ERR_BAD_INPUT          0x09

typedef int GetRandomDataFunc(unsigned char *buffer, unsigned long sz);
typedef int HashFunc(unsigned char *digest, unsigned long sz, unsigned char *data);
typedef int YieldFunc(void);
int aesMmoHash(unsigned char *digest, unsigned long sz, unsigned char *data);

int ZSE_ECDSASign(unsigned char *privateKey, 
                  unsigned char *msgDigest,
                  GetRandomDataFunc *GetRandomData, 
                  unsigned char *r, 
                  unsigned char *s,
                  YieldFunc *yield, 
                  unsigned long yieldLevel);

int ZSE_ECDSAVerify(unsigned char *publicKey, 
                    unsigned char *msgDigest, 
                    unsigned char *r, 
                    unsigned char *s, 
                    YieldFunc *yield, 
                    unsigned long yieldLevel);

int ZSE_ECCGenerateKey(unsigned char *privateKey, 
                       unsigned char *publicKey,
                       GetRandomDataFunc *GetRandomData, 
                       YieldFunc *yield, 
                       unsigned long yieldLevel);

int ZSE_ECCKeyBitGenerate(unsigned char *privateKey, 
                          unsigned char *ephemeralPrivateKey, 
                          unsigned char *ephemeralPublicKey, 
                          unsigned char *remoteCertificate, 
                          unsigned char *remoteEphemeralPublicKey, 
                          unsigned char *caPublicKey, 
                          unsigned char *keyBits, 
                          HashFunc *Hash, 
                          YieldFunc *yield, 
                          unsigned long yieldLevel);

int ZSE_ECQVReconstructPublicKey(unsigned char* certificate,
                                 unsigned char* caPublicKey,
                                 unsigned char* publicKey,
                                 HashFunc *Hash,
                                 YieldFunc *yield,
                                 unsigned long yieldLevel);


/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ECCAPI_163_H
