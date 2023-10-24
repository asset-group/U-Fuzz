/**************************************************************************************************
  Filename:       mac_radio_defs.c
  Revised:        $Date: 2015-02-17 14:17:44 -0800 (Tue, 17 Feb 2015) $
  Revision:       $Revision: 42683 $

  Description:    Describe the purpose and contents of the file.


  Copyright 2006-2015 Texas Instruments Incorporated. All rights reserved.

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
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_radio_defs.h"
#include "hal_types.h"
#include "hal_assert.h"
#include "hal_mcu.h"
#include "mac_pib.h"

/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */

uint8 reqTxBoost = FALSE;  /* CC2533/34-specific register settings to achieve highest Tx power. */
uint8 paLnaChip = PA_LNA_NONE;
   
#if defined HAL_PA_LNA_SE2431L || defined MAC_RUNTIME_SE2431L
  #define CPS_DIR         P0DIR
  #define CPS_PIN_OUTPUT  BV(7)  
    
  #define ANT_CSD_SEL_DIR P1DIR 
  #define CSD_PIN_OUTPUT   BV(3)  
  #define ANT_SEL_OUTPUT   BV(0)

#else
  #define CPS_DIR         P0DIR
  #define CPS_PIN_OUTPUT  0 
    
  #define ANT_CSD_SEL_DIR P1DIR 
  #define CSD_PIN_OUTPUT   0 
  #define ANT_SEL_OUTPUT   0 

#endif

/* ------------------------------------------------------------------------------------------------
 *                                        Global Constants
 * ------------------------------------------------------------------------------------------------
 */
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590 && \
    !defined HAL_PA_LNA_SE2431L && !defined HAL_PA_LNA_CC2592)

const uint8 CODE macRadioDefsTxPwrBare[] =
{
  3,  /* tramsmit power level of the first entry */
  (uint8)(int8)-22, /* transmit power level of the last entry */
  /*   3 dBm */   0xF5,   /* characterized as  4.5 dBm in datasheet */
  /*   2 dBm */   0xE5,   /* characterized as  2.5 dBm in datasheet */
  /*   1 dBm */   0xD5,   /* characterized as  1   dBm in datasheet */
  /*   0 dBm */   0xD5,   /* characterized as  1   dBm in datasheet */
  /*  -1 dBm */   0xC5,   /* characterized as -0.5 dBm in datasheet */
  /*  -2 dBm */   0xB5,   /* characterized as -1.5 dBm in datasheet */
  /*  -3 dBm */   0xA5,   /* characterized as -3   dBm in datasheet */
  /*  -4 dBm */   0x95,   /* characterized as -4   dBm in datasheet */
  /*  -5 dBm */   0x95,
  /*  -6 dBm */   0x85,   /* characterized as -6   dBm in datasheet */
  /*  -7 dBm */   0x85,
  /*  -8 dBm */   0x75,   /* characterized as -8   dBm in datasheet */
  /*  -9 dBm */   0x75,
  /* -10 dBm */   0x65,   /* characterized as -10  dBm in datasheet */
  /* -11 dBm */   0x65,
  /* -12 dBm */   0x55,   /* characterized as -12  dBm in datasheet */
  /* -13 dBm */   0x55,
  /* -14 dBm */   0x45,   /* characterized as -14  dBm in datasheet */
  /* -15 dBm */   0x45,
  /* -16 dBm */   0x35,   /* characterized as -16  dBm in datasheet */
  /* -17 dBm */   0x35,
  /* -18 dBm */   0x25,   /* characterized as -18  dBm in datasheet */
  /* -19 dBm */   0x25,
  /* -20 dBm */   0x15,   /* characterized as -20  dBm in datasheet */
  /* -21 dBm */   0x15,
  /* -22 dBm */   0x05    /* characterized as -22  dBm in datasheet */
};
#endif /* defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
          defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
          (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590 && \
          !defined HAL_PA_LNA_SE2431L && !defined HAL_PA_LNA_CC2592) */

#if defined HAL_PA_LNA || defined MAC_RUNTIME_CC2591
const uint8 CODE macRadioDefsTxPwrCC2591[] =
{
  20,  /* tramsmit power level of the first entry */
  (uint8)(int8)10, /* transmit power level of the last entry */
  /*  20 dBm */   0xE5,   /* characterized as 20 dBm in datasheet */
  /*  19 dBm */   0xD5,   /* characterized as 19 dBm in datasheet */
  /*  18 dBm */   0xC5,   /* characterized as 18 dBm in datasheet */
  /*  17 dBm */   0xB5,   /* characterized as 17 dBm in datasheet */
  /*  16 dBm */   0xA5,   /* characterized as 16 dBm in datasheet */
  /*  15 dBm */   0xA5,
  /*  14 dBm */   0x95,   /* characterized as 14.5 dBm in datasheet */
  /*  13 dBm */   0x85,   /* characterized as 13 dBm in datasheet */
  /*  12 dBm */   0x85,
  /*  11 dBm */   0x75,   /* characterized as 11.5 dBm in datasheet */
  /*  10 dBm */   0x65    /* characterized as 10 dBm in datasheet */
};
#endif

#if defined HAL_PA_LNA_CC2592 || defined MAC_RUNTIME_CC2592
const uint8 CODE macRadioDefsTxPwrCC2592[] =
{
  21,  /* tramsmit power level of the first entry */
  (uint8)(int8)10, /* transmit power level of the last entry */
  /*  21 dBm */   0xF4,   /* characterized as 21.47 dBm */
  /*  20 dBm */   0xD4,   /* characterized as 20.26 dBm */
  /*  19 dBm */   0xC0,   /* characterized as 19.31 dBm */
  /*  18 dBm */   0xA5,   /* characterized as 18.21 dBm */
  /*  17 dBm */   0x95,   /* characterized as 17.14 dBm */
  /*  16 dBm */   0x82,   /* characterized as 16.27 dBm */
  /*  15 dBm */   0x72,   /* characterized as 15.15 dBm */
  /*  14 dBm */   0x63,   /* characterized as 14.12 dBm */
  /*  13 dBm */   0x61,   /* characterized as 13.24 dBm */
  /*  12 dBm */   0x55,   /* characterized as 11.93 dBm */
  /*  11 dBm */   0x50,   /* characterized as 10.90 dBm */
  /*  10 dBm */   0x49    /* characterized as 10.29 dBm */
};
#endif

#if defined HAL_PA_LNA_CC2590 || defined MAC_RUNTIME_CC2590
const uint8 CODE macRadioDefsTxPwrCC2590[] =
{
  11,  /* tramsmit power level of the first entry */
  (uint8)(int8)-15, /* transmit power level of the last entry */
  /* Note that the characterization is preliminary */
  /* 11 dBm */   0xF5,   /* characterized as 11.1 dBm */
  /* 10 dBm */   0xF5,
  /*  9 dBm */   0xE5,   /* characterized as 9.3 dBm */
  /*  8 dBm */   0xD5,   /* characterized as 7.7 dBm */
  /*  7 dBm */   0xD5,
  /*  6 dBm */   0xC5,   /* characterized as 6.4 dBm */
  /*  5 dBm */   0xB5,   /* characterized as 5.2 dBm */
  /*  4 dBm */   0xA5,   /* characterized as 3.6 dBm */
  /*  3 dBm */   0xA5,
  /*  2 dBm */   0x95,   /* characterized as 1.6 dBm */
  /*  1 dBm */   0x95,
  /*  0 dBm */   0x85,   /* characterized as 0.4 dBm */
  /* -1 dBm */   0x75,   /* characterized as -1.1 dBm */
  /* -2 dBm */   0x75,
  /* -3 dBm */   0x65,   /* characterized as -2.9 dBm */
  /* -4 dBm */   0x65,
  /* -5 dBm */   0x55,   /* characterized as -5.0 dBm */
  /* -6 dBm */   0x55,
  /* -7 dBm */   0x45,   /* characterized as -7.3 dBm*/
  /* -8 dBm */   0x45,
  /* -9 dBm */   0x45,
  /* -10 dBm */  0x35,   /* characterized as -9.7 dBm */
  /* -11 dBm */  0x25,   /* characterized as -11.3 dBm */
  /* -12 dBm */  0x25,
  /* -13 dBm */  0x15,   /* characterized as -13.3 dBm */
  /* -14 dBm */  0x15,
  /* -15 dBm */  0x05    /* characterized as -15.4 dBm */
};
#endif

#if defined (HAL_PA_LNA_SE2431L) || defined (MAC_RUNTIME_SE2431L)
const uint8 CODE macRadioDefsTxPwrSE2431L[] =
{
  22,  /* tramsmit power level of the first entry */
  (uint8)(int8)10, /* transmit power level of the last entry */
  /* Note that the characterization is preliminary */
  /* 22 dBm */   0xF5,   /* characterized as 21.5 dBm */
  /* 21 dBm */   0xE5,   /* characterized as 21.1 dBm */
  /* 20 dBm */   0xD5,   /* characterized as 20.5 dBm */
  /* 19 dBm */   0xC5,   /* characterized as 19.7 dBm */
  /* 18 dBm */   0xB5,   /* characterized as 18.8 dBm */
  /* 17 dBm */   0xA5,   /* characterized as 17.4 dBm */
  /* 16 dBm */   0x95,   /* characterized as 15.5 dBm */
  /* 15 dBm */   0x95,   /* characterized as 15.5 dBm */
  /* 14 dBm */   0x85,   /* characterized as 14.2 dBm */
  /* 13 dBm */   0x85,   /* characterized as 14.2 dBm */
  /* 12 dBm */   0x75,   /* characterized as 12.3 dBm */
  /* 11 dBm */   0x75,   /* characterized as 12.3 dBm */
  /* 10 dBm */   0x65,   /* characterized as 10.6 dBm */
};
#endif

const uint8 CODE macRadioDefsTxPwrBare0x95[] =
{
  7,  /* tramsmit power level of the first entry */
  (uint8)(int8)-20, /* transmit power level of the last entry */
  /*   7 dBm */   0xFD,   /* characterized as  7   dBm in datasheet */
  /*   6 dBm */   0xFD,   /* clip to 7 dBm */
  /*   5 dBm */   0xFD,   /* clip to 7 dBm */
  /*   4 dBm */   0xEC,   /* characterized as  4.5 dBm in datasheet */
  /*   3 dBm */   0xDC,   /* characterized as  3   dBm in datasheet */
  /*   2 dBm */   0xDC,
  /*   1 dBm */   0xCC,   /* characterized as  1.7 dBm in datasheet */
  /*   0 dBm */   0xBC,   /* characterized as  0.3 dBm in datasheet */
  /*  -1 dBm */   0xAC,   /* characterized as -1   dBm in datasheet */
  /*  -2 dBm */   0xAC,
  /*  -3 dBm */   0x9C,   /* characterized as -2.8 dBm in datasheet */
  /*  -4 dBm */   0x9C,
  /*  -5 dBm */   0x8C,   /* characterized as -4.1 dBm in datasheet */
  /*  -6 dBm */   0x7C,   /* characterized as -5.9 dBm in datasheet */
  /*  -7 dBm */   0x7C,
  /*  -8 dBm */   0x6C,   /* characterized as -7.7 dBm in datasheet */
  /*  -9 dBm */   0x6C,
  /* -10 dBm */   0x5C,   /* characterized as -9.9 dBm in datasheet */
  /* -11 dBm */   0x5C,
  /* -12 dBm */   0x5C,   /* characterized as -9.9 dBm in datasheet */
  /* -13 dBm */   0x4C,   /* characterized as -12.8 dBm in datasheet */
  /* -14 dBm */   0x4C,
  /* -15 dBm */   0x3C,   /* characterized as -14.9 dBm in datasheet */
  /* -16 dBm */   0x3C,
  /* -17 dBm */   0x2C,   /* characterized as -16.6 dBm in datasheet */
  /* -18 dBm */   0x2C,
  /* -19 dBm */   0x1C,   /* characterized as -18.7 dBm in datasheet */
  /* -20 dBm */   0x1C    /* characterized as -18.7 dBm in datasheet */
};



#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
    defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592

/* TX power table array */
const uint8 CODE *const CODE macRadioDefsTxPwrTables[] =
{
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590 && \
    !defined HAL_PA_LNA_SE2431L && !defined HAL_PA_LNA_CC2592)
  macRadioDefsTxPwrBare,
#endif

#if defined HAL_PA_LNA || defined MAC_RUNTIME_CC2591
  macRadioDefsTxPwrCC2591,
#endif

#if defined HAL_PA_LNA_CC2590 || defined MAC_RUNTIME_CC2590
  macRadioDefsTxPwrCC2590,
#endif

#if defined HAL_PA_LNA_SE2431L || defined MAC_RUNTIME_SE2431L
  macRadioDefsTxPwrSE2431L,
#endif

#if defined HAL_PA_LNA_CC2592 || defined MAC_RUNTIME_CC2592
  macRadioDefsTxPwrCC2592,
#endif
};

/* TX power table array. */
/* TODO: Please note that the cc2533 uses the
 * same TXPOWER table with or without PA/LNA here.
 */
const uint8 CODE *const CODE macRadioDefsTxPwrTables0x95[] =
{
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
  (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590)
  macRadioDefsTxPwrBare0x95,
#endif

#if defined HAL_PA_LNA || defined MAC_RUNTIME_CC2591
  macRadioDefsTxPwrBare0x95,
#endif

#if defined HAL_PA_LNA_CC2590 || defined MAC_RUNTIME_CC2590
  macRadioDefsTxPwrBare0x95,
#endif
};

/* RSSI offset adjustment value array */
const int8 CODE macRadioDefsRssiAdj[] =
{
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590 && \
    !defined HAL_PA_LNA_CC2590 && !defined HAL_PA_LNA_CC2592)
  0,
#endif

#if defined HAL_PA_LNA || defined MAC_RUNTIME_CC2591
  -6, /* high gain mode */
  6,  /* low gain mode */
#endif

#if defined HAL_PA_LNA_CC2590 || defined MAC_RUNTIME_CC2590
 -10,
  0,
#endif

#if defined HAL_PA_LNA_SE2431L || defined MAC_RUNTIME_SE2431L
-12,   /*high gain mode of SE2431L*/
 0,    /*Low gain mode of SE2431L */
#endif

#if defined HAL_PA_LNA_CC2592 || defined MAC_RUNTIME_CC2592
 /* TBD Placeholder for CC2530+CC2592 */
 -10,
 -5,
#endif
};


/* Sensitivity adjusts*/
const int8 CODE macRadioDefsSensitivityAdj[] =
{
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
  defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
  (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590 && \
  !defined HAL_PA_LNA_SE2431L && !defined HAL_PA_LNA_CC2592)
  0,
#endif

#if defined HAL_PA_LNA || defined MAC_RUNTIME_CC2591
/* -98.8 approximated to -98 MAC_RADIO_RECEIVER_SENSITIVITY_DBM -1 = -98 */
  -1, 
#endif

#if defined HAL_PA_LNA_CC2590 || defined MAC_RUNTIME_CC2590
  -1,
#endif

#if defined HAL_PA_LNA_SE2431L || defined MAC_RUNTIME_SE2431L
/* -101.8 approximated to -101. MAC_RADIO_RECEIVER_SENSITIVITY_DBM -4 = -101 */
  -4, 
#endif

#if defined HAL_PA_LNA_CC2592 || defined MAC_RUNTIME_CC2592
 /* TBD Placeholder for CC2530+CC2592 */
  -1,
#endif
};


/* Saturation adjusts*/
const int8 CODE macRadioDefsSaturationAdj[] =
{
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
  defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
  (!defined HAL_PA_LNA && !defined HAL_PA_LNA_CC2590 && \
  !defined HAL_PA_LNA_SE2431L && !defined HAL_PA_LNA_CC2592)
  0,
#endif

#if defined HAL_PA_LNA || defined MAC_RUNTIME_CC2591
  0,  
#endif

#if defined HAL_PA_LNA_CC2590 || defined MAC_RUNTIME_CC2590
  0,
#endif

#if defined HAL_PA_LNA_SE2431L || defined MAC_RUNTIME_SE2431L
/* 5.5 approximated to 5. MAC_RADIO_RECEIVER_SATURATION_DBM -5 = 5 */
  -5,   
#endif

#if defined HAL_PA_LNA_CC2592 || defined MAC_RUNTIME_CC2592
 /* TBD Placeholder for CC2530+CC2592 */
  0,
#endif
};

#endif /* defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || ... */

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
    defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592
uint8 macRadioDefsRefTableId = 0;
#endif

/* RF observable control register value to output PA signal */
#define RFC_OBS_CTRL_PA_PD_INV        0x68

/* RF observable control register value to output LNA signal */
#define RFC_OBS_CTRL_LNAMIX_PD_INV    0x6A

/* OBSSELn register value to select RF observable 0 */
#define OBSSEL_OBS_CTRL0             0xFB

/* OBSSELn register value to select RF observable 1 */
#define OBSSEL_OBS_CTRL1             0xFC


/**************************************************************************************************
 * @fn          MAC_SelectRadioRegTable
 *
 * @brief       Select radio register table in case multiple register tables are included
 *              in the build
 *
 * @param       txPwrTblIdx - TX power register value table index
 * @param       rssiAdjIdx - RSSI adjustment value index
 *
 * @return      none
 **************************************************************************************************
 */
extern void MAC_SetRadioRegTable ( uint8 txPwrTblIdx, uint8 rssiAdjIdx )
{
  /* Depending on compile flags, the parameters may not be used */
  (void) txPwrTblIdx;
  (void) rssiAdjIdx;

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 ||  \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592
  if (txPwrTblIdx >= sizeof(macRadioDefsTxPwrTables)/sizeof(macRadioDefsTxPwrTables[0]))
  {
    txPwrTblIdx = 0;
  }
#endif /* defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 */

#if defined (MAC_RUNTIME_CC2591) || defined (MAC_RUNTIME_CC2590) || \
    defined (MAC_RUNTIME_SE2431L)  || defined (MAC_RUNTIME_CC2592) || \
    defined (HAL_PA_LNA) || defined (HAL_PA_LNA_CC2590) || \
    defined (HAL_PA_LNA_SE2431L)  || defined (HAL_PA_LNA_CC2592)
  if (rssiAdjIdx >= sizeof(macRadioDefsRssiAdj)/sizeof(macRadioDefsRssiAdj[0]))
  {
    rssiAdjIdx = 0;
  }

  macRadioDefsRefTableId = (txPwrTblIdx << 4) | rssiAdjIdx;

#endif /* defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || .. */
  
 
#if defined (MAC_RUNTIME_CC2591) && defined (MAC_RUNTIME_CC2590) && \
    defined (MAC_RUNTIME_SE2431L) && defined (MAC_RUNTIME_CC2592)
/* Determining PA LNA chip used from txPwrIdx*/  
  switch (txPwrTblIdx)
  {
  case MAC_CC2591_TX_PWR_TABLE_IDX:
    paLnaChip = PA_LNA_CC2591;
    break;
    
  case MAC_CC2590_TX_PWR_TABLE_IDX:
    paLnaChip = PA_LNA_CC2590;
    break;
    
  case MAC_SE2431L_TX_PWR_TABLE_IDX:
    paLnaChip = PA_LNA_SE2431L;
    break;
  
  case MAC_CC2592_TX_PWR_TABLE_IDX:
    paLnaChip = PA_LNA_CC2592;
    break;
    
    default
      paLnaChip = PA_LNA_NONE;
    break;
  }
#elif defined (MAC_RUNTIME_CC2591) || defined (HAL_PA_LNA)
  paLnaChip = PA_LNA_CC2591;
#elif defined (MAC_RUNTIME_CC2590) || defined (HAL_PA_LNA_CC2590)
  paLnaChip = PA_LNA_CC2590;
#elif defined (MAC_RUNTIME_SE2431L) || defined (HAL_PA_LNA_SE2431L)
  paLnaChip = PA_LNA_SE2431L;
#elif defined (MAC_RUNTIME_CC2592) || defined (HAL_PA_LNA_CC2592)
  paLnaChip = PA_LNA_CC2592;
#else
  paLnaChip = PA_LNA_NONE;
#endif
  
  if (paLnaChip == PA_LNA_SE2431L)
  {
    /* Setting CPS as Output by setting P0DIR = 0x80 */
    CPS_DIR  |= CPS_PIN_OUTPUT; 
    
    /* Setting CSD and Antenns Select as Output by setting P1DIR = 0x9 */
    ANT_CSD_SEL_DIR |= (CSD_PIN_OUTPUT | ANT_SEL_OUTPUT);
    
   /* Now EN or CSD line is controlled via software so setting it high here 
    * to start the SE2431L frontend
    */
    HAL_PA_LNA_RX_CSD_HIGH();
  }  
}


/**************************************************************************************************
 * @fn          macRadioTurnOnPower
 *
 * @brief       Logic and sequence for powering up the radio.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTurnOnPower(void)
{
  /* Enable RF error trap */
  MAC_MCU_RFERR_ENABLE_INTERRUPT();

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592
  /* table ID is referenced only when runtime configuration is enabled */
  if (macRadioDefsRefTableId & 0xf0)
#endif /* defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 */

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
    defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592
  { /* either if compound statement or non-conditional compound statement */
    
    /* (Re-)Configure PA and LNA control signals to RF frontend chips.
    * Note that The register values are not retained during sleep.
    */
    
    if (paLnaChip == PA_LNA_SE2431L)
    {
      /* CPS or P0_7 maps closely to the HGM line */
      HAL_PA_LNA_RX_HGM(); 
  
      /* EN or CSD line is controlled via software so setting it high here to start the SE2431L frontend */
      HAL_PA_LNA_RX_CSD_HIGH();
      
      /* CTX or P1_1 maps closely to PAEN */
      RFC_OBS_CTRL0 = RFC_OBS_CTRL_PA_PD_INV;
      OBSSEL1       = OBSSEL_OBS_CTRL0;
    }
    else if(paLnaChip == PA_LNA_CC2592)
    {
      /* P1_1 -> PAEN */
      RFC_OBS_CTRL0 = RFC_OBS_CTRL_PA_PD_INV;
      OBSSEL1       = OBSSEL_OBS_CTRL0;
      
      /* P1_0 -> EN (LNA control) */
      RFC_OBS_CTRL1 = RFC_OBS_CTRL_LNAMIX_PD_INV;
      OBSSEL0       = OBSSEL_OBS_CTRL1;
    }  
    else 
    {   
      /* P1_1 -> PAEN */
      RFC_OBS_CTRL0 = RFC_OBS_CTRL_PA_PD_INV;
      OBSSEL1       = OBSSEL_OBS_CTRL0;
      
      /* P1_4 -> EN (LNA control) */
      RFC_OBS_CTRL1 = RFC_OBS_CTRL_LNAMIX_PD_INV;
      OBSSEL4       = OBSSEL_OBS_CTRL1;
    }
    
    /* For any RX, change CCA settings for CC2591 compression workaround.
    * This will override LNA control if CC2591_COMPRESSION_WORKAROUND
    * flag is defined.
    */
  }
#endif /* defined MAC_RUNTIME_CC2591 || ... || defined HAL_PA_LNA_SE2431L... */

  if (macChipVersion <= REV_B)
  {
    /* radio initializations for disappearing RAM; PG1.0 and before only */
    MAC_RADIO_SET_PAN_ID(pMacPib->panId);
    MAC_RADIO_SET_SHORT_ADDR(pMacPib->shortAddress);
    MAC_RADIO_SET_IEEE_ADDR(pMacPib->extendedAddress.addr.extAddr);
  }

  /* Turn on frame filtering */
  MAC_RADIO_TURN_ON_RX_FRAME_FILTERING();
}


/**************************************************************************************************
 * @fn          macRadioTurnOffPower
 *
 * @brief       Logic and sequence for powering down the radio.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTurnOffPower(void)
{
#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592
  /* table ID is referenced only when runtime configuration is enabled */
  if (macRadioDefsRefTableId & 0xf0)
#endif /* defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 */

#if defined MAC_RUNTIME_CC2591 || defined MAC_RUNTIME_CC2590 || \
    defined MAC_RUNTIME_SE2431L || defined MAC_RUNTIME_CC2592 || \
    defined HAL_PA_LNA || defined HAL_PA_LNA_CC2590 || \
    defined HAL_PA_LNA_SE2431L || defined HAL_PA_LNA_CC2592
  { /* either if compound statement or non-conditional compound statement */
    
    if (paLnaChip == PA_LNA_SE2431L)
    {
      HAL_PA_LNA_RX_LGM(); 
      HAL_PA_LNA_RX_CSD_LOW();
    }
    else 
    {   
      if (paLnaChip == PA_LNA_CC2591  ||  paLnaChip == PA_LNA_CC2590)
      {
        /* Set direction of P1_4 to output and pulled down to prevent any leakage
         * when used to drive PA LNA		
		 */
        P1DIR |= BV(4);
        P1_4 = 0;
      }
    }
  }
  #endif /* defined MAC_RUNTIME_CC2591 || ... || defined HAL_PA_LNA_SE2431L */
  /* Disable RF error trap */
  MAC_MCU_RFERR_DISABLE_INTERRUPT();
}


/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if (HAL_CPU_CLOCK_MHZ != 32)
#error "ERROR: The only tested/supported clock speed is 32 MHz."
#endif

#if (MAC_RADIO_RECEIVER_SENSITIVITY_DBM > MAC_SPEC_MIN_RECEIVER_SENSITIVITY)
#error "ERROR: Radio sensitivity does not meet specification."
#endif

#if defined (HAL_PA_LNA) && defined (HAL_PA_LNA_CC2590)
#error "ERROR: HAL_PA_LNA and HAL_PA_LNA_CC2590 cannot be both defined."
#endif

#if defined (HAL_PA_LNA) && defined (MAC_RUNTIME_CC2591)
#error "ERROR: HAL_PA_LNA and MAC_RUNTIME_CC2591 cannot be both defined."
#endif

#if defined (HAL_PA_LNA_CC2590) && defined (MAC_RUNTIME_CC2590)
#error "ERROR: HAL_PA_LNA_CC2590 and MAC_RUNTIME_CC2590 cannot be both defined."
#endif

#if defined (HAL_PA_LNA_SE2431L) && defined (MAC_RUNTIME_SE2431L)
#error "ERROR: HAL_PA_LNA_SE2431L and MAC_RUNTIME_SE2431L cannot be both defined."
#endif

#if defined (HAL_PA_LNA_CC2592) && defined (MAC_RUNTIME_CC2592)
#error "ERROR: HAL_PA_LNA_CC2592 and MAC_RUNTIME_CC2592 cannot be both defined."
#endif

#if defined (CC2591_COMPRESSION_WORKAROUND)
#warning "WARNING: Contact TI customer support if your reference design is based on CC2530-CC2591EM rev 2.0 or prior."
#endif

/**************************************************************************************************
 */
