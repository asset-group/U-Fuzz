/**************************************************************************************************
Filename:       mac_radio_reg_defs.h
Revised:        $Date: 2013-05-17 11:25:11 -0700 (Fri, 17 May 2013) $
Revision:       $Revision: 34355 $

Description:    RFcore Radio Registers


Copyright 2011-2012 Texas Instruments Incorporated. All rights reserved.

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
#ifndef MAC_RADIO_REG_DEFS_H
#define MAC_RADIO_REG_DEFS_H

#ifdef __cplusplus
extern "C"
{
#endif
  
#include "hw_types.h"
#include "hw_rfcore_sfr.h"
#include "hw_smwdthrosc.h"
#include "hw_rfcore_xreg.h"
#include "hw_rfcore_ffsm.h"
#include "hw_cctest.h"
  
  
  /* SFR registers */
#define T2CSPCFG                     HWREG(RFCORE_SFR_MTCSPCFG)
#define T2CTRL                       HWREG(RFCORE_SFR_MTCTRL)
#define T2IRQM                       HWREG(RFCORE_SFR_MTIRQM)
#define T2IRQF                       HWREG(RFCORE_SFR_MTIRQF)
#define T2MSEL                       HWREG(RFCORE_SFR_MTMSEL)
#define T2M0                         HWREG(RFCORE_SFR_MTM0)
#define T2M1                         HWREG(RFCORE_SFR_MTM1)
#define T2MOVF2                      HWREG(RFCORE_SFR_MTMOVF2)
#define T2MOVF1                      HWREG(RFCORE_SFR_MTMOVF1)
#define T2MOVF0                      HWREG(RFCORE_SFR_MTMOVF0)
#define RFD                          HWREG(RFCORE_SFR_RFDATA)       
#define RFERRF                       HWREG(RFCORE_SFR_RFERRF)      
#define RFIRQF1                      HWREG(RFCORE_SFR_RFIRQF1)      
#define RFIRQF0                      HWREG(RFCORE_SFR_RFIRQF0)       
#define RFST                         HWREG(RFCORE_SFR_RFST)   
  
  
  /* SMWDT registers */
#define WDCTL                        HWREG(SMWDTHROSC_WDCTL)
#define ST0                          HWREG(SMWDTHROSC_ST0)        
#define ST1                          HWREG(SMWDTHROSC_ST1)          
#define ST2                          HWREG(SMWDTHROSC_ST2)        
#define ST3                          HWREG(SMWDTHROSC_ST3)        
#define STLOAD                       HWREG(SMWDTHROSC_STLOAD)     
#define STCC                         HWREG(SMWDTHROSC_STCC)        
#define STCS                         HWREG(SMWDTHROSC_STCS)         
#define STV0                         HWREG(SMWDTHROSC_STCV0)       
#define STV1                         HWREG(SMWDTHROSC_STCV1)        
#define STV2                         HWREG(SMWDTHROSC_STCV2)        
#define STV3                         HWREG(SMWDTHROSC_STCV3) 
#define HSOSCCAL                     HWREG(SMWDTHROSC_HSOSCCAL)  
#define RCOSCCAL                     HWREG(SMWDTHROSC_RCOSCCAL) 
  
  
  /* XREG registers */
  /* Address information for RX Control */
#define FRMFILT0                     HWREG(RFCORE_XREG_FRMFILT0)
#define FRMFILT1                     HWREG(RFCORE_XREG_FRMFILT1)
#define SRCMATCH                     HWREG(RFCORE_XREG_SRCMATCH)
#define SRCSHORTEN0                  HWREG(RFCORE_XREG_SRCSHORTEN0)
#define SRCSHORTEN1                  HWREG(RFCORE_XREG_SRCSHORTEN1)
#define SRCSHORTEN2                  HWREG(RFCORE_XREG_SRCSHORTEN2)
#define SRCEXTEN0                    HWREG(RFCORE_XREG_SRCEXTEN0)
#define SRCEXTEN1                    HWREG(RFCORE_XREG_SRCEXTEN1)
#define SRCEXTEN2                    HWREG(RFCORE_XREG_SRCEXTEN2)
  
  /* Radio Control */
#define FRMCTRL0                     HWREG(RFCORE_XREG_FRMCTRL0)
#define FRMCTRL1                     HWREG(RFCORE_XREG_FRMCTRL1)
#define RXENABLE                     HWREG(RFCORE_XREG_RXENABLE)
#define RXMASKSET                    HWREG(RFCORE_XREG_RXMASKSET)
#define RXMASKCLR                    HWREG(RFCORE_XREG_RXMASKCLR)
#define FREQTUNE                     HWREG(RFCORE_XREG_FREQTUNE)
#define FREQCTRL                     HWREG(RFCORE_XREG_FREQCTRL)
#define TXPOWER                      HWREG(RFCORE_XREG_TXPOWER)
#define TXCTRL                       HWREG(RFCORE_XREG_TXCTRL)
#define FSMSTAT0                     HWREG(RFCORE_XREG_FSMSTAT0)
#define FSMSTAT1                     HWREG(RFCORE_XREG_FSMSTAT1)
#define FIFOPCTRL                    HWREG(RFCORE_XREG_FIFOPCTRL)
#define FSMCTRL                      HWREG(RFCORE_XREG_FSMCTRL)
#define CCACTRL0                     HWREG(RFCORE_XREG_CCACTRL0)
#define CCACTRL1                     HWREG(RFCORE_XREG_CCACTRL1)
#define RSSI                         HWREG(RFCORE_XREG_RSSI)
#define RSSISTAT                     HWREG(RFCORE_XREG_RSSISTAT)
#define RXFIRST                      HWREG(RFCORE_XREG_RXFIRST)
#define RXFIFOCNT                    HWREG(RFCORE_XREG_RXFIFOCNT)
#define TXFIFOCNT                    HWREG(RFCORE_XREG_TXFIFOCNT)
#define RXFIRST_PTR                  HWREG(RFCORE_XREG_RXFIRST_PTR)
#define RXLAST_PTR                   HWREG(RFCORE_XREG_RXLAST_PTR)
#define RXP1_PTR                     HWREG(RFCORE_XREG_RXP1_PTR)
#define RXP2_PTR                     HWREG(RFCORE_XREG_RXP2_PTR)
#define TXFIRST_PTR                  HWREG(RFCORE_XREG_TXFIRST_PTR)
#define TXLAST_PTR                   HWREG(RFCORE_XREG_TXLAST_PTR)
  
  /* Interrupt Controller Registers */
#define RFIRQM0                      HWREG(RFCORE_XREG_RFIRQM0)
#define RFIRQM1                      HWREG(RFCORE_XREG_RFIRQM1)
#define RFERRM                       HWREG(RFCORE_XREG_RFERRM)
#define D18_SPARE_OPAMPMC            HWREG(RFCORE_XREG_D18_SPARE_OPAMPMC)
  
  /* Random Number Generator */
#define RFRND                        HWREG(RFCORE_XREG_RFRND)
  
  /* Analog and Digital Radio Test And Tuning */
#define MDMCTRL0                     HWREG(RFCORE_XREG_MDMCTRL0)
#define MDMCTRL1                     HWREG(RFCORE_XREG_MDMCTRL1)
#define FREQEST                      HWREG(RFCORE_XREG_FREQEST)
#define RXCTRL                       HWREG(RFCORE_XREG_RXCTRL)
#define FSCTRL                       HWREG(RFCORE_XREG_FSCTRL)
#define FSCAL0                       HWREG(RFCORE_XREG_FSCAL0)
#define FSCAL1                       HWREG(RFCORE_XREG_FSCAL1)
#define FSCAL2                       HWREG(RFCORE_XREG_FSCAL2)
#define FSCAL3                       HWREG(RFCORE_XREG_FSCAL3)
#define AGCCTRL0                     HWREG(RFCORE_XREG_AGCCTRL0)
#define AGCCTRL1                     HWREG(RFCORE_XREG_AGCCTRL1)
#define AGCCTRL2                     HWREG(RFCORE_XREG_AGCCTRL2)
#define AGCCTRL3                     HWREG(RFCORE_XREG_AGCCTRL3)
#define ADCTEST0                     HWREG(RFCORE_XREG_ADCTEST0)
#define ADCTEST1                     HWREG(RFCORE_XREG_ADCTEST1)
#define ADCTEST2                     HWREG(RFCORE_XREG_ADCTEST2)
#define MDMTEST0                     HWREG(RFCORE_XREG_MDMTEST0)
#define MDMTEST1                     HWREG(RFCORE_XREG_MDMTEST1)
#define DACTEST0                     HWREG(RFCORE_XREG_DACTEST0)
#define DACTEST1                     HWREG(RFCORE_XREG_DACTEST1)
#define DACTEST2                     HWREG(RFCORE_XREG_DACTEST2)
#define ATEST                        HWREG(RFCORE_XREG_ATEST)
#define PTEST0                       HWREG(RFCORE_XREG_PTEST0)
#define PTEST1                       HWREG(RFCORE_XREG_PTEST1)
  
  /*CSP */
#define CSPPROG0                     HWREG(RFCORE_XREG_CSPPROG0)
#define CSPPROG1                     HWREG(RFCORE_XREG_CSPPROG1)  
#define CSPPROG2                     HWREG(RFCORE_XREG_CSPPROG2)
#define CSPPROG3                     HWREG(RFCORE_XREG_CSPPROG3)
#define CSPPROG4                     HWREG(RFCORE_XREG_CSPPROG4)
#define CSPPROG5                     HWREG(RFCORE_XREG_CSPPROG5)
#define CSPPROG6                     HWREG(RFCORE_XREG_CSPPROG6)  
#define CSPPROG7                     HWREG(RFCORE_XREG_CSPPROG7)
#define CSPPROG8                     HWREG(RFCORE_XREG_CSPPROG8)
#define CSPPROG9                     HWREG(RFCORE_XREG_CSPPROG9)
#define CSPPROG10                    HWREG(RFCORE_XREG_CSPPROG10)
#define CSPPROG11                    HWREG(RFCORE_XREG_CSPPROG11)  
#define CSPPROG12                    HWREG(RFCORE_XREG_CSPPROG12)
#define CSPPROG13                    HWREG(RFCORE_XREG_CSPPROG13)
#define CSPPROG14                    HWREG(RFCORE_XREG_CSPPROG14)
#define CSPPROG15                    HWREG(RFCORE_XREG_CSPPROG15)
#define CSPPROG16                    HWREG(RFCORE_XREG_CSPPROG16)  
#define CSPPROG17                    HWREG(RFCORE_XREG_CSPPROG17)
#define CSPPROG18                    HWREG(RFCORE_XREG_CSPPROG18)
#define CSPPROG19                    HWREG(RFCORE_XREG_CSPPROG19)
#define CSPPROG20                    HWREG(RFCORE_XREG_CSPPROG20) 
#define CSPPROG21                    HWREG(RFCORE_XREG_CSPPROG21)
#define CSPPROG22                    HWREG(RFCORE_XREG_CSPPROG22)
#define CSPPROG23                    HWREG(RFCORE_XREG_CSPPROG23)
#define CSPCTRL                      HWREG(RFCORE_XREG_CSPCTRL)
#define CSPSTAT                      HWREG(RFCORE_XREG_CSPSTAT)
#define CSPX                         HWREG(RFCORE_XREG_CSPX)        
#define CSPY                         HWREG(RFCORE_XREG_CSPY)        
#define CSPZ                         HWREG(RFCORE_XREG_CSPZ)        
#define CSPT                         HWREG(RFCORE_XREG_CSPT)
  
#define RFC_DUTY_CYCLE               HWREG(RFCORE_XREG_RFC_DUTY_CYCLE)
#define RFC_OBS_CTRL0                HWREG(RFCORE_XREG_RFC_OBS_CTRL0)
#define RFC_OBS_CTRL1                HWREG(RFCORE_XREG_RFC_OBS_CTRL1)
#define RFC_OBS_CTRL2                HWREG(RFCORE_XREG_RFC_OBS_CTRL2)
  
#define ACOMPGAINI                   HWREG(RFCORE_XREG_ACOMPGAINI)
#define ACOMPGAINQ                   HWREG(RFCORE_XREG_ACOMPGAINQ)
#define ACOMPDCI                     HWREG(RFCORE_XREG_ACOMPDCI)
#define ACOMPDCQ                     HWREG(RFCORE_XREG_ACOMPDCQ)
#define ACOMPQS                      HWREG(RFCORE_XREG_ACOMPQS)
#define ACOMPCFG                     HWREG(RFCORE_XREG_ACOMPCFG)
#define ACOMPCALIL                   HWREG(RFCORE_XREG_ACOMPCALIL)
#define ACOMPCALIH                   HWREG(RFCORE_XREG_ACOMPCALIH)
#define ACOMPCALQL                   HWREG(RFCORE_XREG_ACOMPCALQL)
#define ACOMPCALQH                   HWREG(RFCORE_XREG_ACOMPCALQH)
#define TXFILTCFG                    HWREG(RFCORE_XREG_TXFILTCFG)
#define TXMIXCFG                     HWREG(RFCORE_XREG_TXMIXCFG)
#define TXMIXSTAT                    HWREG(RFCORE_XREG_TXMIXSTAT) 
  
  
  /* FFSM registers */
  /* Source Address Matching Control */
#define SRCRESMASK0                  HWREG(RFCORE_FFSM_SRCRESMASK0) 
#define SRCRESMASK1                  HWREG(RFCORE_FFSM_SRCRESMASK1) 
#define SRCRESMASK2                  HWREG(RFCORE_FFSM_SRCRESMASK2) 
#define SRCRESINDEX                  HWREG(RFCORE_FFSM_SRCRESINDEX) 
#define SRCEXTPENDEN0                HWREG(RFCORE_FFSM_SRCEXTPENDEN0) 
#define SRCEXTPENDEN1                HWREG(RFCORE_FFSM_SRCEXTPENDEN1) 
#define SRCEXTPENDEN2                HWREG(RFCORE_FFSM_SRCEXTPENDEN2) 
#define SRCSHORTPENDEN0              HWREG(RFCORE_FFSM_SRCSHORTPENDEN0) 
#define SRCSHORTPENDEN1              HWREG(RFCORE_FFSM_SRCSHORTPENDEN1) 
#define SRCSHORTPENDEN2              HWREG(RFCORE_FFSM_SRCSHORTPENDEN2) 
  
  /* Local Address Information */
#define EXT_ADDR0                    HWREG(RFCORE_FFSM_EXT_ADDR0) 
#define EXT_ADDR1                    HWREG(RFCORE_FFSM_EXT_ADDR1) 
#define EXT_ADDR2                    HWREG(RFCORE_FFSM_EXT_ADDR2) 
#define EXT_ADDR3                    HWREG(RFCORE_FFSM_EXT_ADDR3) 
#define EXT_ADDR4                    HWREG(RFCORE_FFSM_EXT_ADDR4) 
#define EXT_ADDR5                    HWREG(RFCORE_FFSM_EXT_ADDR5)  
#define EXT_ADDR6                    HWREG(RFCORE_FFSM_EXT_ADDR6)  
#define EXT_ADDR7                    HWREG(RFCORE_FFSM_EXT_ADDR7) 
#define PAN_ID0                      HWREG(RFCORE_FFSM_PAN_ID0) 
#define PAN_ID1                      HWREG(RFCORE_FFSM_PAN_ID1) 
#define SHORT_ADDR0                  HWREG(RFCORE_FFSM_SHORT_ADDR0) 
#define SHORT_ADDR1                  HWREG(RFCORE_FFSM_SHORT_ADDR1)
 
  
  /* CC Test registers*/
#define OBSSEL0                      HWREG(CCTEST_OBSSEL0)
#define OBSSEL1                      HWREG(CCTEST_OBSSEL1)
#define OBSSEL2                      HWREG(CCTEST_OBSSEL2)
#define OBSSEL3                      HWREG(CCTEST_OBSSEL3)
#define OBSSEL4                      HWREG(CCTEST_OBSSEL4)
#define OBSSEL5                      HWREG(CCTEST_OBSSEL5)
#define OBSSEL6                      HWREG(CCTEST_OBSSEL6)
#define OBSSEL7                      HWREG(CCTEST_OBSSEL7)
  //*****************************************************************************
  //
  // Mark the end of the C bindings section for C++ compilers.
  //
  //*****************************************************************************
#ifdef __cplusplus
}
#endif  

#endif