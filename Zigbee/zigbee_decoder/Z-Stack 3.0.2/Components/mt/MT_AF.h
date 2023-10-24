/**************************************************************************************************
  Filename:       MT_AF.h
  Revised:        $Date: 2012-11-01 15:23:04 -0700 (Thu, 01 Nov 2012) $
  Revision:       $Revision: 32011 $

  Description:    MonitorTest functions for AF.

  Copyright 2007-2012 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MT_AF_H
#define MT_AF_H

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "ZComDef.h"
#include "MT.h"
#include "AF.h"
#include "OnBoard.h"

//#define AFCB_CHECK(cbi,task) ((_afCallbackSub & cbi) && (task == MT_TaskID))
#define AFCB_CHECK(cbi,task) ((1) && (task == MT_TaskID))

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
#if defined ( MT_AF_CB_FUNC )
#define CB_ID_AF_DATA_IND               0x0001
#define CB_ID_AF_DATA_CNF               0x0002
#define CB_ID_AF_REFLECT_ERROR          0x0004

#define SPI_AF_CB_TYPE                  0x0900
#endif

#if defined (INTER_PAN)
typedef enum {
  InterPanClr,
  InterPanSet,
  InterPanReg,
  InterPanChk
} InterPanCtl_t;
#endif
/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
extern uint16 _afCallbackSub;

/*
 * AF housekeeping executive.
 */
extern void MT_AfExec(void);

/*
 * Process AF commands
 */
extern uint8 MT_AfCommandProcessing(uint8 *pBuf);

/*
 * Process the callback subscription for AF Incoming data.
 */
extern void MT_AfIncomingMsg(afIncomingMSGPacket_t *pMsg);

/*
 * Process the callback subscription for Data confirm
 */
extern void MT_AfDataConfirm(afDataConfirm_t *pMsg);

/*
 * Process the callback subscription for Reflect Error
 */
extern void MT_AfReflectError(afReflectError_t *pMsg);

/*********************************************************************
*********************************************************************/
#endif
