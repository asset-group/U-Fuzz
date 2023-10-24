/******************************************************************************
  Filename:       _hal_aes.c
  Revised:        $Date: 2013-05-17 11:08:52 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34354 $

  Description:    Support for Hardware AES encryption.

  Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

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

/******************************************************************************
 * INCLUDES
 */
#include "osal.h"
#include "hal_aes.h"
#include "hal_mcu.h"
#include "hal_ccm.h"
#include "aes.h"

/******************************************************************************
 * MACROS
 */
/******************************************************************************
 * CONSTANTS
 */

/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */
void (*pSspAesEncrypt)( uint8 *, uint8 * ) = (void*)NULL;

/******************************************************************************
 * FUNCTION PROTOTYPES
 */

/******************************************************************************
 * @fn      HalAesInit
 *
 * @brief   Initilize AES engine
 *
 * input parameters
 *
 * @param   None
 *
 * @return  None
 */
void HalAesInit( void )
{
  HWREG(AES_CTRL_INT_CFG) |= 0x00000001;
  HWREG(AES_CTRL_INT_EN) |= 0x00000003;
}


/******************************************************************************
 * @fn      ssp_HW_KeyInit
 *
 * @brief   Writes the key into AES engine
 *
 * input parameters
 *
 * @param   AesKey  - Pointer to AES Key.
 *
 * @return  None
 */
void ssp_HW_KeyInit( uint8 *AesKey )
{
  /* Load the AES key 
   * KeyStore has rentention after PM2 
   */
  AESLoadKey( (uint8 *)AesKey, 0);
}


/******************************************************************************
 * @fn      sspAesEncryptHW
 *
 * @brief   Encrypts 16 byte block using AES encryption engine
 *
 * input parameters
 *
 * @param   AesKey  - Pointer to AES Key.
 * @param   Cstate  - Pointer to input data.
 *
 * output parameters
 *
 * @param   Cstate  - Pointer to encrypted data.
 *
 * @return  None
 *
 */
void sspAesEncryptHW( uint8 *AesKey, uint8 *Cstate )
{
  /* please note that ssp_HW_KeyInit(AesKey); should have already 
   * been called prior to using this function 
   */
  AESECBStart( Cstate, Cstate, 0, true, false);
  
  /* wait for completion of the operation */
  do
  {
    ASM_NOP;
  }while(!(AESECBCheckResult()));
  
  AESECBGetResult();
}


/******************************************************************************
 * @fn      sspAesDecryptHW
 *
 * @brief   Decrypts 16 byte block using AES encryption engine
 *
 * input parameters
 *
 * @param   AesKey  - Pointer to AES Key.
 * @param   Cstate  - Pointer to input data.
 *
 * output parameters
 *
 * @param   Cstate  - Pointer to decrypted data.
 *
 * @return  None
 *
 */
void sspAesDecryptHW( uint8 *AesKey, uint8 *Cstate )
{
  /* please note that ssp_HW_KeyInit(AesKey); should have already 
   * been called prior to using this function 
   */
  AESECBStart( Cstate, Cstate, 0, false, false);
  
  /* wait for completion of the operation */
  do
  {
    ASM_NOP;
  }while(!(AESECBCheckResult()));
  
  AESECBGetResult();
}


/******************************************************************************
 * @fn      sspAesEncryptHW_keylocation
 *
 * @brief   Encrypts 16 byte block using AES encryption engine
 *
 * input parameters
 *
 * @param   AesKey  - Pointer to AES Key.
 * @param   Cstate  - Pointer to input data.
 *
 * output parameters
 *
 * @param   Cstate  - Pointer to encrypted data.
 *
 * @return  None
 *
 */
void sspAesEncryptHW_keylocation(uint8 *msg_in, uint8 *msg_out, uint8 key_location )
{
  /* please note that ssp_HW_KeyInit(AesKey); should have already 
   * been called prior to using this function 
   */
  AESECBStart( msg_in, msg_out, key_location, true, false);
  
  /* wait for completion of the operation */
  do
  {
    ASM_NOP;
  }while(!(AESECBCheckResult()));
  
  AESECBGetResult();
}


/******************************************************************************
 * @fn      sspAesDecryptHW_keylocation
 *
 * @brief   Decrypts 16 byte block using AES decryption engine
 *
 * input parameters
 *
 * @param   AesKey  - Pointer to AES Key.
 * @param   Cstate  - Pointer to input data.
 *
 * output parameters
 *
 * @param   Cstate  - Pointer to encrypted data.
 *
 * @return  None
 *
 */
void sspAesDecryptHW_keylocation( uint8 *msg_in, uint8 *msg_out, uint8 key_location )
{
  /* please note that ssp_HW_KeyInit(AesKey); should have already 
   * been called prior to using this function 
   */
  AESECBStart( msg_in, msg_out, key_location, false, false);
  
  /* wait for completion of the operation */
  do
  {
    ASM_NOP;
  }while(!(AESECBCheckResult()));
  
  AESECBGetResult();
}


