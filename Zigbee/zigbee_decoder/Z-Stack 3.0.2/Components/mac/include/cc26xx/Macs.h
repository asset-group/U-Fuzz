/**
  @file  Macs.h
  @brief Mac common definitions between Stack and App

  <!--
  Copyright 2015 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED ``AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
  -->
*/
#ifndef MACS_H
#define MACS_H

#include <stdint.h>


#define ICALL_MAC_CMD_EVENT_START   0xD0
  
enum
{
  MAC_INIT = ICALL_MAC_CMD_EVENT_START,
  MAC_INIT_DEVICE,
  MAC_INIT_COORD,
  MAC_INIT_BEACON_COORD,
  MAC_INIT_BEACON_DEVICE,
  MAC_SET_REQ,
  MAC_GET_REQ,
  MAC_SET_SECURITY_REQ,
  MAC_GET_SECURITY_REQ,
  MAC_RESET_REQ,
  MAC_SRC_MATCH_ENABLE,
  MAC_SRC_MATCH_ADD_ENTRY,
  MAC_SRC_MATCH_DELETE_ENTRY,
  MAC_SRC_MATCH_ACK_ALL_PENDING,
  MAC_SRC_MATCH_CHECK_ALL_PENDING,
  MAC_SET_RADIO_REG,
  MAC_UPDATE_PANID,
  MAC_MCPS_DATA_REQ,
  MAC_MCPS_DATA_ALLOC,
  MAC_MCPS_PURGE_REQ,
  MAC_MLME_ASSOCIATE_REQ,
  MAC_MLME_ASSOCIATE_RSP,
  MAC_MLME_DISASSOCIATE_REQ,
  MAC_MLME_ORPHAN_RSP,
  MAC_MLME_POLL_REQ,
  MAC_MLME_SCAN_REQ,
  MAC_START_REQ,
  MAC_SYNC_REQ,
  MAC_RANDOM_BYTE,
  MAC_RESUME_REQ,
  MAC_YIELD_REQ,
  MAC_MSG_DEALLOCATE,
  MAC_GET_SECURITY_PTR_REQ
};


/**************************************************************************************************
 *                             RF Front End Mode and Bias Configuration
 **************************************************************************************************/

/* RF Front End Settings */
#define RF_FE_DIFFERENTIAL               0
#define RF_FE_SINGLE_ENDED_RFP           1
#define RF_FE_SINGLE_ENDED_RFN           2
#define RF_FE_ANT_DIVERSITY_RFP_FIRST    3
#define RF_FE_ANT_DIVERSITY_RFN_FIRST    4
#define RF_FE_SINGLE_ENDED_RFP_EXT_PINS  5
#define RF_FE_SINGLE_ENDED_RFN_EXT_PINS  6

#define RF_FE_INT_BIAS                   (0<<3)
#define RF_FE_EXT_BIAS                   (1<<3)

/* TX power table calculation */
#define TX_POUT( IB, GC, TC )            (uint16)(((IB) & 0x3F) | (((GC) & 0x03) << 6) | (((TC)&0xFF) << 8))

#if defined( MODULE_CC26XX_7X7 )

  #define RF_FE_MODE_AND_BIAS       ( RF_FE_DIFFERENTIAL | RF_FE_INT_BIAS )
  #define RF_TX_POWER_TABLE         { \
    5,  /* tramsmit power level of the first entry */ \
    (uint16)(int16)-21, /* transmit power level of the last entry */ \
    /*   5 dBm */   TX_POUT( 49, 0, 0x31 ),  /* characterized as  5  dBm in datasheet */ \
    /*   4 dBm */   TX_POUT( 37, 0, 0x25 ),  /* characterized as  5  dBm in datasheet */ \
    /*   3 dBm */   TX_POUT( 49, 1, 0x71 ),  /* characterized as  3  dBm in datasheet */ \
    /*   2 dBm */   TX_POUT( 42, 1, 0x6A ),  /* characterized as  3  dBm in datasheet */ \
    /*   1 dBm */   TX_POUT( 37, 1, 0x65 ),  /* characterized as  1  dBm in datasheet */ \
    /*   0 dBm */   TX_POUT( 38, 3, 0xE6 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -1 dBm */   TX_POUT( 38, 3, 0xE6 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -2 dBm */   TX_POUT( 38, 3, 0xE6 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -3 dBm */   TX_POUT( 28, 3, 0xDC ),  /* characterized as -3  dBm in datasheet */ \
    /*  -4 dBm */   TX_POUT( 28, 3, 0xDC ),  /* characterized as -3  dBm in datasheet */ \
    /*  -5 dBm */   TX_POUT( 28, 3, 0xDC ),  /* characterized as -3  dBm in datasheet */ \
    /*  -6 dBm */   TX_POUT( 22, 3, 0xD6 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -7 dBm */   TX_POUT( 22, 3, 0xD6 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -8 dBm */   TX_POUT( 22, 3, 0xD6 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -9 dBm */   TX_POUT( 17, 3, 0xD1 ),  /* characterized as -9  dBm in datasheet */ \
    /* -10 dBm */   TX_POUT( 17, 3, 0xD1 ),  /* characterized as -9  dBm in datasheet */ \
    /* -11 dBm */   TX_POUT( 17, 3, 0xD1 ),  /* characterized as -9  dBm in datasheet */ \
    /* -12 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -13 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -14 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -15 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -16 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -17 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -18 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -19 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -20 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -21 dBm */   TX_POUT( 6,  3, 0xC6 )   /* characterized as -21 dBm in datasheet */ \
  }
  #define RF_TX_POWER_TABLE_SIZE 29
  /* Recommended overrides for IEEE 802.15.4, differential mode internal bias */
  #define RF_RADIO_OVERRIDE_TABLE   { \
    0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
    0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
    0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
    0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
    0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
    0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
    0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
    0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
    0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
    0x000288A3, /* ID: Adjust RSSI offset by 2 dB */                           \
    0x000F8883, /* XD and ID: Force LNA IB to maximum */                       \
    0x002B50DC, /* Adjust AGC DC filter */                                     \
    0x05000243, /* Increase synth programming timeout */                       \
    0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
    0x00018063, /* Disable pointer check */                                    \
    0xFFFFFFFF  /* End of override list */                                     \
  }
  #define RF_OVERRIDE_TABLE_SIZE 16
  #define RF_FE_IOD_NUM     1
  #define RF_FE_IOD         {NULL}
  #define RF_FE_IOD_VAL     {NULL}
  #define RF_FE_SATURATION  0
  #define RF_FE_SENSITIVITY 0

#elif defined( MODULE_CC26XX_5X5 )

  #ifdef PA_LNA_CC2592
    /* The package dependence is small enough to be neglected so we should use settings 
     * based on frontend configuration, e.g. 5XD should have the same settings as 4XD
     */
    #define RF_FE_MODE_AND_BIAS       ( RF_FE_DIFFERENTIAL | RF_FE_EXT_BIAS)
    #define RF_TX_POWER_TABLE         { \
      5,  /* tramsmit power level of the first entry */ \
      (uint16)(int16)-21, /* transmit power level of the last entry */ \
      /*   5 dBm */   TX_POUT( 45, 0, 0x2D ),  /* characterized as  5  dBm in datasheet */ \
      /*   4 dBm */   TX_POUT( 34, 0, 0x22 ),  /* characterized as  5  dBm in datasheet */ \
      /*   3 dBm */   TX_POUT( 27, 0, 0x1B ),  /* characterized as  3  dBm in datasheet */ \
      /*   2 dBm */   TX_POUT( 40, 1, 0x68 ),  /* characterized as  3  dBm in datasheet */ \
      /*   1 dBm */   TX_POUT( 36, 1, 0x64 ),  /* characterized as  1  dBm in datasheet */ \
      /*   0 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
      /*  -1 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
      /*  -2 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
      /*  -3 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
      /*  -4 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
      /*  -5 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
      /*  -6 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
      /*  -7 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
      /*  -8 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
      /*  -9 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
      /* -10 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
      /* -11 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
      /* -12 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
      /* -13 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
      /* -14 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
      /* -15 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
      /* -16 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
      /* -17 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
      /* -18 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
      /* -19 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
      /* -20 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
      /* -21 dBm */   TX_POUT( 5,  3, 0xC5 )   /* characterized as -21 dBm in datasheet */ \
    }
    #define RF_TX_POWER_TABLE_SIZE 29
    /* Recommended overrides for IEEE 802.15.4, differential mode external bias */
    #define RF_RADIO_OVERRIDE_TABLE   { \
      0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
      0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
      0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
      0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
      0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
      0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
      0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
      0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
      0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
      0x000388A3, /* XD: Adjust RSSI offset by 3 dB */                           \
      0x000F8883, /* XD and ID: Force LNA IB to maximum */                       \
      0x002B50DC, /* Adjust AGC DC filter */                                     \
      0x05000243, /* Increase synth programming timeout */                       \
      0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
      0x00018063, /* Disable pointer check */                                    \
      0x000088A3, /* RSSI adjustment for PA LNA*/                                \
      0xFFFFFFFF  /* End of override list */                                     \
    }
    #define RF_OVERRIDE_TABLE_SIZE 16
    #define RF_FE_IOD_NUM     3
    #define RF_FE_IOD         {IOC_O_IOCFG7, IOC_O_IOCFG13, IOC_O_IOCFG14}
    #define RF_FE_IOD_VAL     {0x30, 0x2F, 1}
    #define RF_FE_SATURATION  0
    #define RF_FE_SENSITIVITY 0
  #else
    /* The package dependence is small enough to be neglected so we should use settings 
     * based on frontend configuration, e.g. 5XD should have the same settings as 4XD
     */
    #define RF_FE_MODE_AND_BIAS       ( RF_FE_DIFFERENTIAL | RF_FE_EXT_BIAS)
    #define RF_TX_POWER_TABLE         { \
      5,  /* tramsmit power level of the first entry */ \
      (uint16)(int16)-21, /* transmit power level of the last entry */ \
      /*   5 dBm */   TX_POUT( 45, 0, 0x2D ),  /* characterized as  5  dBm in datasheet */ \
      /*   4 dBm */   TX_POUT( 34, 0, 0x22 ),  /* characterized as  5  dBm in datasheet */ \
      /*   3 dBm */   TX_POUT( 27, 0, 0x1B ),  /* characterized as  3  dBm in datasheet */ \
      /*   2 dBm */   TX_POUT( 40, 1, 0x68 ),  /* characterized as  3  dBm in datasheet */ \
      /*   1 dBm */   TX_POUT( 36, 1, 0x64 ),  /* characterized as  1  dBm in datasheet */ \
      /*   0 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
      /*  -1 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
      /*  -2 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
      /*  -3 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
      /*  -4 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
      /*  -5 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
      /*  -6 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
      /*  -7 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
      /*  -8 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
      /*  -9 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
      /* -10 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
      /* -11 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
      /* -12 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
      /* -13 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
      /* -14 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
      /* -15 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
      /* -16 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
      /* -17 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
      /* -18 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
      /* -19 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
      /* -20 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
      /* -21 dBm */   TX_POUT( 5,  3, 0xC5 )   /* characterized as -21 dBm in datasheet */ \
    }
    #define RF_TX_POWER_TABLE_SIZE 29
    /* Recommended overrides for IEEE 802.15.4, differential mode external bias */
    #define RF_RADIO_OVERRIDE_TABLE   { \
      0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
      0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
      0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
      0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
      0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
      0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
      0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
      0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
      0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
      0x000388A3, /* XD: Adjust RSSI offset by 3 dB */                           \
      0x000F8883, /* XD and ID: Force LNA IB to maximum */                       \
      0x002B50DC, /* Adjust AGC DC filter */                                     \
      0x05000243, /* Increase synth programming timeout */                       \
      0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
      0x00018063, /* Disable pointer check */                                    \
      0xFFFFFFFF  /* End of override list */                                     \
    }
    #define RF_OVERRIDE_TABLE_SIZE 16
    #define RF_FE_IOD_NUM     1
    #define RF_FE_IOD         {NULL}
    #define RF_FE_IOD_VAL     {NULL}
    #define RF_FE_SATURATION  0
    #define RF_FE_SENSITIVITY 0
  
  #endif
#elif defined( MODULE_CC26XX_4X4 )

  #define RF_FE_MODE_AND_BIAS       ( RF_FE_SINGLE_ENDED_RFP | RF_FE_EXT_BIAS )
  #define RF_TX_POWER_TABLE         { \
    3,  /* tramsmit power level of the first entry */ \
    (uint16)(int16)-21, /* transmit power level of the last entry */ \
    /*   3 dBm */   TX_POUT( 63, 0, 0x3F ),  /* characterized as  3  dBm in datasheet */ \
    /*   2 dBm */   TX_POUT( 46, 0, 0x2E ),  /* characterized as  3  dBm in datasheet */ \
    /*   1 dBm */   TX_POUT( 33, 0, 0x21 ),  /* characterized as  1  dBm in datasheet */ \
    /*   0 dBm */   TX_POUT( 45, 1, 0x6D ),  /* characterized as  0  dBm in datasheet */ \
    /*  -1 dBm */   TX_POUT( 45, 1, 0x6D ),  /* characterized as  0  dBm in datasheet */ \
    /*  -2 dBm */   TX_POUT( 45, 1, 0x6D ),  /* characterized as  0  dBm in datasheet */ \
    /*  -3 dBm */   TX_POUT( 33, 3, 0xE1 ),  /* characterized as -3  dBm in datasheet */ \
    /*  -4 dBm */   TX_POUT( 33, 3, 0xE1 ),  /* characterized as -3  dBm in datasheet */ \
    /*  -5 dBm */   TX_POUT( 33, 3, 0xE1 ),  /* characterized as -3  dBm in datasheet */ \
    /*  -6 dBm */   TX_POUT( 24, 3, 0xD8 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -7 dBm */   TX_POUT( 24, 3, 0xD8 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -8 dBm */   TX_POUT( 24, 3, 0xD8 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -9 dBm */   TX_POUT( 18, 3, 0xD2 ),  /* characterized as -9  dBm in datasheet */ \
    /* -10 dBm */   TX_POUT( 18, 3, 0xD2 ),  /* characterized as -9  dBm in datasheet */ \
    /* -11 dBm */   TX_POUT( 18, 3, 0xD2 ),  /* characterized as -9  dBm in datasheet */ \
    /* -12 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -13 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -14 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -15 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -16 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -17 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -18 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -19 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -20 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -21 dBm */   TX_POUT( 6,  3, 0xC6 )   /* characterized as -21 dBm in datasheet */ \
  }
  #define RF_TX_POWER_TABLE_SIZE 27
  /* Recommended overrides for IEEE 802.15.4, single-ended mode external bias */
  #define RF_RADIO_OVERRIDE_TABLE   { \
    0x00001107, /* Run patched MCE and RFE code from RAM for 500 kHz Rx IF and correct LNA gain */ \
    0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
    0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
    0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
    0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
    0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
    0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
    0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
    0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
    0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
    0x000288A3, /* XS and IS: Adjust RSSI offset by 2 dB */                    \
    0x08000323, /* XS and IS: Use 500 kHz Rx IF */                             \
    0x08000343, /* XS and IS: Use 500 kHz Tx IF */                             \
    0x001000A3, /* XS and IS: Use dynamic Tx IF */                             \
    0x05000243, /* Increase synth programming timeout */                       \
    0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
    0x00018063, /* Disable pointer check */                                    \
    0xFFFFFFFF  /* End of override list */                                     \
  }
  #define RF_OVERRIDE_TABLE_SIZE 18
  #define RF_FE_IOD_NUM     1
  #define RF_FE_IOD         {NULL}
  #define RF_FE_IOD_VAL     {NULL}
  #define RF_FE_SATURATION  0
  #define RF_FE_SENSITIVITY 0

#elif defined( MODULE_CC26XX_4X4D )

  #define RF_FE_MODE_AND_BIAS       ( RF_FE_DIFFERENTIAL | RF_FE_EXT_BIAS )
  #define RF_TX_POWER_TABLE         { \
    5,  /* tramsmit power level of the first entry */ \
    (uint16)(int16)-21, /* transmit power level of the last entry */ \
    /*   5 dBm */   TX_POUT( 45, 0, 0x2D ),  /* characterized as  5  dBm in datasheet */ \
    /*   4 dBm */   TX_POUT( 34, 0, 0x22 ),  /* characterized as  5  dBm in datasheet */ \
    /*   3 dBm */   TX_POUT( 27, 0, 0x1B ),  /* characterized as  3  dBm in datasheet */ \
    /*   2 dBm */   TX_POUT( 40, 1, 0x68 ),  /* characterized as  3  dBm in datasheet */ \
    /*   1 dBm */   TX_POUT( 36, 1, 0x64 ),  /* characterized as  1  dBm in datasheet */ \
    /*   0 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -1 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -2 dBm */   TX_POUT( 32, 1, 0x60 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -3 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
    /*  -4 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
    /*  -5 dBm */   TX_POUT( 27, 3, 0xDB ),  /* characterized as -3  dBm in datasheet */ \
    /*  -6 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -7 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -8 dBm */   TX_POUT( 20, 3, 0xD4 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -9 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
    /* -10 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
    /* -11 dBm */   TX_POUT( 15, 3, 0xCF ),  /* characterized as -9  dBm in datasheet */ \
    /* -12 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
    /* -13 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
    /* -14 dBm */   TX_POUT( 12, 3, 0xCC ),  /* characterized as -12 dBm in datasheet */ \
    /* -15 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
    /* -16 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
    /* -17 dBm */   TX_POUT( 9,  3, 0xC9 ),  /* characterized as -15 dBm in datasheet */ \
    /* -18 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
    /* -19 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
    /* -20 dBm */   TX_POUT( 7,  3, 0xC7 ),  /* characterized as -18 dBm in datasheet */ \
    /* -21 dBm */   TX_POUT( 5,  3, 0xC5 )   /* characterized as -21 dBm in datasheet */ \
  }
  #define RF_TX_POWER_TABLE_SIZE 29
  /* Recommended overrides for IEEE 802.15.4, differential mode external bias */
  #define RF_RADIO_OVERRIDE_TABLE   { \
    0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
    0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
    0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
    0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
    0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
    0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
    0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
    0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
    0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
    0x000388A3, /* XD: Adjust RSSI offset by 3 dB */                           \
    0x000F8883, /* XD and ID: Force LNA IB to maximum */                       \
    0x002B50DC, /* Adjust AGC DC filter */                                     \
    0x05000243, /* Increase synth programming timeout */                       \
    0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
    0x00018063, /* Disable pointer check */                                    \
    0xFFFFFFFF  /* End of override list */                                     \
  }
  #define RF_OVERRIDE_TABLE_SIZE 16
  #define RF_FE_IOD_NUM     1
  #define RF_FE_IOD         {NULL}
  #define RF_FE_IOD_VAL     {NULL}
  #define RF_FE_SATURATION  0
  #define RF_FE_SENSITIVITY 0

#else /* unknown package */

  /* This header file is shared with the Application, but the stack does not
   * use the package define. Leave this here for stack builds.
   * Note: Sadly, this means we can't generate an #error on the App side if a
   * known package isn't specified.
   */
  #define RF_FE_MODE_AND_BIAS       ( RF_FE_DIFFERENTIAL | RF_FE_INT_BIAS )
  #define RF_TX_POWER_TABLE         { \
    5,  /* tramsmit power level of the first entry */ \
    (uint16)(int16)-21, /* transmit power level of the last entry */ \
    /*   5 dBm */   TX_POUT( 49, 0, 0x31 ),  /* characterized as  5  dBm in datasheet */ \
    /*   4 dBm */   TX_POUT( 37, 0, 0x25 ),  /* characterized as  5  dBm in datasheet */ \
    /*   3 dBm */   TX_POUT( 49, 1, 0x71 ),  /* characterized as  3  dBm in datasheet */ \
    /*   2 dBm */   TX_POUT( 42, 1, 0x6A ),  /* characterized as  3  dBm in datasheet */ \
    /*   1 dBm */   TX_POUT( 37, 1, 0x65 ),  /* characterized as  1  dBm in datasheet */ \
    /*   0 dBm */   TX_POUT( 38, 3, 0xE6 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -1 dBm */   TX_POUT( 38, 3, 0xE6 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -2 dBm */   TX_POUT( 38, 3, 0xE6 ),  /* characterized as  0  dBm in datasheet */ \
    /*  -3 dBm */   TX_POUT( 28, 3, 0xDC ),  /* characterized as -3  dBm in datasheet */ \
    /*  -4 dBm */   TX_POUT( 28, 3, 0xDC ),  /* characterized as -3  dBm in datasheet */ \
    /*  -5 dBm */   TX_POUT( 28, 3, 0xDC ),  /* characterized as -3  dBm in datasheet */ \
    /*  -6 dBm */   TX_POUT( 22, 3, 0xD6 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -7 dBm */   TX_POUT( 22, 3, 0xD6 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -8 dBm */   TX_POUT( 22, 3, 0xD6 ),  /* characterized as -6  dBm in datasheet */ \
    /*  -9 dBm */   TX_POUT( 17, 3, 0xD1 ),  /* characterized as -9  dBm in datasheet */ \
    /* -10 dBm */   TX_POUT( 17, 3, 0xD1 ),  /* characterized as -9  dBm in datasheet */ \
    /* -11 dBm */   TX_POUT( 17, 3, 0xD1 ),  /* characterized as -9  dBm in datasheet */ \
    /* -12 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -13 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -14 dBm */   TX_POUT( 14, 3, 0xCE ),  /* characterized as -12 dBm in datasheet */ \
    /* -15 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -16 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -17 dBm */   TX_POUT( 11, 3, 0xCB ),  /* characterized as -15 dBm in datasheet */ \
    /* -18 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -19 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -20 dBm */   TX_POUT( 8,  3, 0xC8 ),  /* characterized as -18 dBm in datasheet */ \
    /* -21 dBm */   TX_POUT( 6,  3, 0xC6 )   /* characterized as -21 dBm in datasheet */ \
  }
  #define RF_TX_POWER_TABLE_SIZE 29
  /* Recommended overrides for IEEE 802.15.4, differential mode internal bias */
  #define RF_RADIO_OVERRIDE_TABLE   { \
    0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
    0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
    0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
    0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
    0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
    0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
    0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
    0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
    0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
    0x000288A3, /* ID: Adjust RSSI offset by 2 dB */                           \
    0x000F8883, /* XD and ID: Force LNA IB to maximum */                       \
    0x002B50DC, /* Adjust AGC DC filter */                                     \
    0x05000243, /* Increase synth programming timeout */                       \
    0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
    0x00018063, /* Disable pointer check */                                    \
    0xFFFFFFFF  /* End of override list */                                     \
  }
  #define RF_OVERRIDE_TABLE_SIZE 16
  #define RF_FE_IOD_NUM     1
  #define RF_FE_IOD         {NULL}
  #define RF_FE_IOD_VAL     {NULL}
  #define RF_FE_SATURATION  0
  #define RF_FE_SENSITIVITY 0

#endif /* MODULE_CC26XX_7X7 */

  /* Recommended overrides for IEEE 802.15.4, single-ended mode internal bias */
  #define RF_RADIO_OVERRIDE_TABLE_NOT_USED  { \
    0x00001107, /* Run patched MCE and RFE code from RAM for 500 kHz Rx IF and correct LNA gain */ \
    0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */                    \
    0x4001402D, /* Synth: Correct CKVD latency setting (address) */            \
    0x00608402, /* Synth: Correct CKVD latency setting (value) */              \
    0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */         \
    0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */           \
    0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */                  \
    0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */      \
    0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */ \
    0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */ \
    0x000288A3, /* XS and IS: Adjust RSSI offset by 2 dB */                    \
    0x08000323, /* XS and IS: Use 500 kHz Rx IF */                             \
    0x08000343, /* XS and IS: Use 500 kHz Tx IF */                             \
    0x001000A3, /* XS and IS: Use dynamic Tx IF */                             \
    0x05000243, /* Increase synth programming timeout */                       \
    0x002082C3, /* Set Rx FIFO threshold to avoid overflow */                  \
    0x00018063, /* Disable pointer check */                                    \
    0xFFFFFFFF  /* End of override list */                                     \
  }
  #define RF_OVERRIDE_TABLE_SIZE_NOT_USED 18

#define MAC_USER_CFG                { RF_FE_MODE_AND_BIAS, \
                                      RF_TX_POWER_TABLE, \
                                      RF_FE_IOD_NUM,  \
                                      RF_FE_IOD,      \
                                      RF_FE_IOD_VAL,   \
                                      RF_FE_SATURATION, \
                                      RF_FE_SENSITIVITY, \
                                      RF_RADIO_OVERRIDE_TABLE }

/**************************************************************************************************
 *                                      TYPEDEFS
 **************************************************************************************************/

typedef struct
{
  uint8_t   rfFeModeBias;  /* RF Front End Mode and Bias (based on package) */
                           /* RF TX power table (based on package)          */  
  uint16_t  rfTxPowerTable[RF_TX_POWER_TABLE_SIZE];
                           /* RF radio overrides table (based on package)   */
  uint8_t   rfFeIODNum;
  uint8_t   rfFeIOD[RF_FE_IOD_NUM];
  uint8_t   rfFeIODVal[RF_FE_IOD_NUM];
  int8_t    rfFeSaturation;
  int8_t    rfFeSensitivity;
  uint32_t  rfOverrideTable[RF_OVERRIDE_TABLE_SIZE];
} macUserCfg_t;

#endif /* MACS_H */