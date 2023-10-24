/**************************************************************************************************
  Filename:       znp_soc.c
  Revised:        $Date: 2010-11-22 08:13:43 -0800 (Mon, 22 Nov 2010) $
  Revision:       $Revision: 24480 $

  Description:

  This file is the HAL-specific Application implementation for the ZNP by 8051 SOC.


  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

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

#include "comdef.h"
#include "hal_board_cfg.h"
#include "mac_radio_defs.h"
#include "MT.h"
#include "OSAL.h"
#include "OSAL_Nv.h"
#include "znp_app.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

#define MAC_RADIO_TX_ON()     st( RFST = ISTXON;   )

#define MOD_IF             4     // ~Modulation bit of MDMTEST1 register.
#define TX_PWR_MOD__SET(MOD_) st ( \
  if ((MOD_)) \
  { \
    MDMTEST1 |= BV(MOD_IF); \
  } \
  else \
  { \
    MDMTEST1 &= ~BV(MOD_IF); \
  } \
);

#define TX_PWR_TONE_SET(TONE) st ( \
  MDMTEST0 &= ~0xF0; \
  MDMTEST0 |= (TONE << 4); \
)

/**************************************************************************************************
 * @fn          znpTestRF
 *
 * @brief       This function initializes and checks the ZNP RF Test Mode NV items. It is designed
 *              to be invoked before/instead of MAC radio initialization.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void znpTestRF(void)
{
  uint8 rfTestParms[4] = { 0, 0, 0, 0 };

  if ((SUCCESS != osal_nv_item_init(ZNP_NV_RF_TEST_PARMS, 4, rfTestParms))  ||
      (SUCCESS != osal_nv_read(ZNP_NV_RF_TEST_PARMS, 0, 4, rfTestParms)) ||
      (rfTestParms[0] == 0))
  {
    return;
  }

  // Settings from SmartRF Studio
  MDMCTRL0 = 0x85;
  RXCTRL = 0x3F;
  FSCTRL = 0x5A;
  FSCAL1 = 0x2B;
  AGCCTRL1 = 0x11;
  ADCTEST0 = 0x10;
  ADCTEST1 = 0x0E;
  ADCTEST2 = 0x03;

  FRMCTRL0 = 0x43;
  FRMCTRL1 = 0x00;

  MAC_RADIO_RXTX_OFF();
  MAC_RADIO_SET_CHANNEL(rfTestParms[1]);
  MAC_RADIO_SET_TX_POWER(rfTestParms[2]);
  TX_PWR_TONE_SET(rfTestParms[3]);

  switch (rfTestParms[0])
  {
  case 1:  // Rx promiscuous mode.
    MAC_RADIO_RX_ON();
    break;

  case 2:  // Un-modulated Tx.
    TX_PWR_MOD__SET(1);
    // no break;

  case 3:  // Modulated Tx.
    // Modulated is default register setting, so no special action.

    // Now turn on Tx power for either mod or un-modulated Tx test.
    MAC_RADIO_TX_ON();
    break;

  default:  // Not expected.
    break;
  }

  // Clear the RF test mode.
  (void)osal_memset(rfTestParms, 0, 4);
  (void)osal_nv_write(ZNP_NV_RF_TEST_PARMS, 0, 4, rfTestParms);

  while (1);  // Spin in RF test mode until a hard reset.
}

/**************************************************************************************************
*/
