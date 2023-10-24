/******************************************************************************
  Filename:       hal_ota.h
  Revised:        $Date: 2013-05-24 11:51:00 -0700 (Fri, 24 May 2013) $
  Revision:       $Revision: 34424 $

  Description:    HAL support for OTA for the CC2538.

  Copyright 2012-2013 Texas Instruments Incorporated. All rights reserved.

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
******************************************************************************/
#ifndef HAL_OTA_H
#define HAL_OTA_H

/******************************************************************************
 * INCLUDES
 */

#include "hal_board_cfg.h"
#include "hal_types.h"

/******************************************************************************
 * CONSTANTS
 */

#if (defined HAL_IMG_AREA)
#if (HAL_IMG_AREA == 0)
#define HAL_OTA_RC_ADDR            HAL_IMG_A_BEG
#define HAL_OTA_DL_ADDR            HAL_IMG_B_BEG
#define HAL_OTA_RC_MAX            (HAL_IMG_B_BEG - HAL_IMG_A_BEG)
#define HAL_OTA_DL_MAX           (((FLASH_BASE) + (HAL_NV_PAGE_BEG * HAL_FLASH_PAGE_SIZE)) -\
                                   HAL_IMG_B_BEG)
#elif (HAL_IMG_AREA == 1)
#define HAL_OTA_RC_ADDR            HAL_IMG_B_BEG
#define HAL_OTA_DL_ADDR            HAL_IMG_A_BEG
#define HAL_OTA_DL_MAX            (HAL_IMG_B_BEG - HAL_IMG_A_BEG)
#define HAL_OTA_RC_MAX           (((FLASH_BASE) + (HAL_NV_PAGE_BEG * HAL_FLASH_PAGE_SIZE)) -\
                                   HAL_IMG_B_BEG)
#else
#error Invalid HAL_IMG_AREA
#endif
#else
#if (defined OTA_CLIENT) && (OTA_CLIENT == TRUE)
#error Undefined HAL_IMG_AREA
#endif
#endif

// OTA Image Area consists of [Header/Pad to CRC][CRC][Preamble][NVIC] where NVIC must reside on a
// 512-byte boundary, so work backwards from that requirement leaving room for OTA header growth.
// Dependencies exist for preamble immediately after and adjcent to the CRC.
#define PREAMBLE_OFFSET           (HAL_NVIC_OSET - sizeof(preamble_t))
#define HAL_OTA_CRC_OSET          (PREAMBLE_OFFSET - sizeof(otaCrc_t))

/*********************************************************************
 * TYPEDEFS
 */

typedef enum {
  HAL_OTA_RC,  /* Run code / active image.          */
  HAL_OTA_DL   /* Downloaded code to be activated later. */
} image_t;

typedef struct {
  uint32 crc[2];
} otaCrc_t;
static_assert((sizeof(otaCrc_t) == 8), "Need to update the PREAMBLE_OFFSET macro.");

typedef struct {
  uint32 programLength;
  uint16 manufacturerId;
  uint16 imageType;
  uint32 imageVersion;
} preamble_t;
static_assert((sizeof(preamble_t) == 12), "Need to PACK the preamble_t");

/*********************************************************************
 * FUNCTIONS
 */

uint8 HalOTAChkDL(uint32 dlImagePreambleOffset);
void HalOTAInit(void);
void HalOTAInvRC(void);
uint32 HalOTAAvail(void);
void HalOTARead(uint32 oset, uint8 *pBuf, uint16 len, image_t type);
void HalOTAWrite(uint32 oset, uint8 *pBuf, uint16 len, image_t type);

#endif
/******************************************************************************
*/
