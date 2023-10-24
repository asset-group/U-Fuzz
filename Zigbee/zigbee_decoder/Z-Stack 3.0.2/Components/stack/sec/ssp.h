/**************************************************************************************************
  Filename:       ssp.h
  Revised:        $Date: 2014-11-18 02:32:26 -0800 (Tue, 18 Nov 2014) $
  Revision:       $Revision: 41160 $

  Description:    Security Service Provider (SSP) interface


  Copyright 2004-2014 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#ifndef SSP_H
#define SSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define SSP_APPLY         0
#define SSP_REMOVE        1
#define SSP_REMOVE_RETRY  2

// Auxiliary header field lengths
#define FRAME_COUNTER_LEN 4
//Threshold after which the frame counter will be reset if a valid APSME-SWITCH-KEY is processed.
#define NWK_FRAME_COUNTER_RESET_THRESHOLD  0x80000000
  
#define SEC_KEY_LEN  16  // 128/8 octets (128-bit key is standard for ZigBee)
#define INSTALL_CODE_LEN      16
#define INSTALL_CODE_CRC_LEN  2  
#define BITS_PER_BYTE         8
#define APS_MIC_LEN           4

// Security Key Indentifiers
#define SEC_KEYID_LINK      0x00
#define SEC_KEYID_NWK       0x01
#define SEC_KEYID_TRANSPORT 0x02
#define SEC_KEYID_LOAD      0x03

// Security Levels
#define SEC_MASK        0x07
#define SEC_NONE        0x00
#define SEC_MIC_32      0x01
#define SEC_MIC_64      0x02

#define SEC_MIC_128     0x03
#define SEC_ENC         0x04
#define SEC_ENC_MIC_32  0x05
#define SEC_ENC_MIC_64  0x06
#define SEC_ENC_MIC_128 0x07

// Key types
#define KEY_TYPE_NWK        1   // Standard Network Key
#define KEY_TYPE_APP_MASTER 2   // Application Master Key
#define KEY_TYPE_APP_LINK   3   // Application Link Key
#define KEY_TYPE_TC_LINK    4   // Trust Center Link Key

#define SSP_AUXHDR_CTRL      0
#define SSP_AUXHDR_FRAMECNTR 1

#define SSP_AUXHDR_KEYID_MASK     0x03
#define SSP_AUXHDR_KEYID_SHIFT    3
#define SSP_AUXHDR_EXTNONCE_SHIFT 5
#define SSP_AUXHDR_EXTNONCE_BIT   0x01
#define SSP_AUXHDR_LEVEL_MASK     0x07

#define SSP_AUXHDR_MIN_LEN    5
#define SSP_AUXHDR_SEQNUM_LEN 1
#define SSP_AUXHDR_EXT_LEN ( SSP_AUXHDR_MIN_LEN + Z_EXTADDR_LEN )
#define SSP_AUXHDR_NWK_LEN ( SSP_AUXHDR_EXT_LEN + SSP_AUXHDR_SEQNUM_LEN  )

#define SSP_MIC_LEN_MAX 16

#define SSP_NONCE_LEN 13

#define SSP_TEXT_LEN 4

// SSP_MacTagData_t::type
#define SSP_MAC_TAGS_SKKE 0
#define SSP_MAC_TAGS_EA   1

// Error value used when security key NV ID is not available
#define SEC_NO_KEY_NV_ID        0

 
/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8 keySeqNum;
  uint8 key[SEC_KEY_LEN];
} nwkKeyDesc;

typedef struct
{
  nwkKeyDesc  active;
  uint32      frameCounter;
} nwkActiveKeyItems;

typedef struct
{
  uint8 hdrLen;
  uint8 auxLen;
  uint8 msgLen;
  uint8 secLevel;
  uint8 keyId;
  uint32 frameCtr;
  uint8 *key;
} ssp_ctx;

typedef struct
{
  uint8* initExtAddr;
  uint8* rspExtAddr;
  uint8* key;
  uint8* qeu;
  uint8* qev;
  uint8* text1;
  uint8* text2;
  uint8* tag1;
  uint8* tag2;
  uint8* linkKey;
  uint8  type;
} SSP_MacTagData_t;

typedef struct
{
  uint8  dir;
  uint8  secLevel;
  uint8  hdrLen;
  uint8  sduLen;   //service data unit length
  uint8* pdu;      //protocol data unit
  uint8  extAddr[Z_EXTADDR_LEN];
  uint8  keyID;
  uint16 keyNvId; // NV ID of key: NWK, TCLK or APS
  uint8  keySeqNum;
  uint32 frmCntr;
  uint8  auxLen;
  uint8  micLen;
  uint8  dstExtAddr[Z_EXTADDR_LEN];
  bool   distributedKeyTry;  //Attempting to validate if TransportKey uses distributed key
  bool   defaultKeyTry;      //Attempting to validate if TransportKey uses default key when install code is in use
  uint8  seedShift;
} SSP_Info_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint32 nwkFrameCounter;
extern uint32 distributedFrameCounter;
extern uint16 nwkFrameCounterChanges;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * SSP Initialization
 */
extern void SSP_Init( void );

/*
 * Parse Auxillary Header
 */
extern void SSP_ParseAuxHdr( SSP_Info_t* si );

/*
 * Process Security Information
 */
extern ZStatus_t SSP_Process( SSP_Info_t* si );

/*
 * Process MAC TAG Data - Generate Tags
 */
extern ZStatus_t SSP_GetMacTags( SSP_MacTagData_t* data );

/*
 * Returns Random Bits
 */
extern void SSP_GetTrueRand( uint8 len, uint8 *rand );

/*
 * Returns 8*len random bits using AES based mechanism
 * ( currently less than 128 bits )
 */
extern ZStatus_t SSP_GetTrueRandAES( uint8 len, uint8 *rand );

/*
 * Store the 16 byte random seed in NV
 */
extern void SSP_StoreRandomSeedNV( uint8 *pSeed );

/*
 * Read the network active key information
 */
extern void SSP_ReadNwkActiveKey( nwkActiveKeyItems *items );

/*
 * Get the index for the selected network key in NV
 */
extern uint16 SSP_GetNwkKey( uint8 seqNum );

/*
 * Secure/Unsecure a network PDU
 */
extern ZStatus_t SSP_NwkSecurity(uint8 ed_flag, uint8 *msg, uint8 hdrLen, uint8 nsduLen);

/*
 * Set the alternate network key
 */
extern void SSP_UpdateNwkKey( uint8 *key, uint8 keySeqNum );

/*
 * Make the alternate network key as active
 */
extern void SSP_SwitchNwkKey( uint8 seqNum );

extern void SSP_BuildNonce( uint8 *addr, uint32 frameCntr, uint8 secCtrl, uint8 *nonce );

extern uint8 SSP_GetMicLen( uint8 securityLevel );

/*
 * Duplicate osal_memcpy functionality, but reverse copy
 */
extern uint8* SSP_MemCpyReverse( uint8* dst, uint8* src, unsigned int len );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* SSP_H */
