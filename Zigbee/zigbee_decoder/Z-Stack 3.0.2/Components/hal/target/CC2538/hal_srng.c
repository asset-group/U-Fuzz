/******************************************************************************
  Filename:       _hal_srng.c
  Revised:        $Date: 2013-05-17 11:25:11 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34355 $

  Description:   Hal Secure Random Number Generator.

  Copyright 2012 Texas Instruments Incorporated. All rights reserved.

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
#include "hal_types.h"
#include "srng.h"
#include "hal_mcu.h"
#include "mac_radio_reg_defs.h"
#include "mac_api.h"

/*******************************************************************************
 * @fn          ssp_srng_init
 *
 * @brief     Initialize SRNG key and V  wrapper
 *
 * input parameters
 *
 * @param entropy/random seed is passed in
 *
 * output parameters
 *
 * None
 *
 * @return None
 *******************************************************************************
 */
void ssp_srng_init(uint8 * entropy)
{
#ifndef HAL_NON_ISR_CS_NO_ARG_REQUIRED
  halIntState_t intstate;
#endif
  HAL_NON_ISR_ENTER_CRITICAL_SECTION(intstate);
  srng_init(entropy);
  HAL_NON_ISR_EXIT_CRITICAL_SECTION(intstate);
}

/*******************************************************************************
 * @fn     ssp_srng_generate
 *
 * @brief  Generate SRNG output wrapper 
 *
 * input parameters
 *
 * @param Pointer to SRNG Output
 * @param Length in bytes
 * addData pointer to Additional Data. The data is 16 bytes in size
 *
 * output parameters
 *
 * @param output SRNG output 
 *
 * @return SRNG_SUCCESS if successful
 *******************************************************************************
 */
uint8 ssp_srng_generate(uint8 *out, uint16 len, uint8 *addData)
{
#ifndef HAL_NON_ISR_CS_NO_ARG_REQUIRED
  halIntState_t intstate;
#endif
  uint8 state;
  HAL_NON_ISR_ENTER_CRITICAL_SECTION(intstate);
  state = srng_generate(out, len, addData);
  HAL_NON_ISR_EXIT_CRITICAL_SECTION(intstate);
  return state;
}

void ssp_srng_reseed(void)
{
  uint8 randomSeed[MAC_RANDOM_SEED_LEN];
  uint8 i,j;
  
  for(i = 0; i < MAC_RANDOM_SEED_LEN; i++)
  {
    uint8 rndByte = 0;
    for(j = 0; j < 8; j++)
    {
      /* use most random bit of analog to 
      * digital receive conversion to
      * populate the random seed 
      */
      rndByte = (rndByte << 1) | (RFRND & 0x01);
    }
    randomSeed[i] = rndByte;
  }
  ssp_srng_init( randomSeed );
}

