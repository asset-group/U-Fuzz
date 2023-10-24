/**************************************************************************************************
  Filename:       MT_ZDO.h
  Revised:        $Date: 2012-02-16 15:49:27 -0800 (Thu, 16 Feb 2012) $
  Revision:       $Revision: 29347 $


  Description:    MonitorTest functions for the ZDO layer.


  Copyright 2004-2012 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED “AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "ZComDef.h"
#include "MT.h"
#include "APSMEDE.h"
#include "AF.h"
#include "ZDProfile.h"
#include "ZDObject.h"
#include "ZDApp.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
extern uint32 _zdoCallbackSub;

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/
#define ZDOCB_CHECK(cbi) (_zdoCallbackSub & (cbi))

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

/*
 *  MT ZDO initialization
 */
extern void MT_ZdoInit(void);

/*
 *   Process all the NWK commands that are issued by test tool
 */
extern uint8 MT_ZdoCommandProcessing(uint8* pBuf);

/*
 *  Callback to handle state change OSAL message from ZDO.
 */
extern void MT_ZdoStateChangeCB(osal_event_hdr_t *pMsg);

/*
 *   Process all the callbacks from ZDO
 */
extern void MT_ZdoDirectCB( afIncomingMSGPacket_t *pData,  zdoIncomingMsg_t *inMsg );

/*
 *   Proxy the ZDO_SendMsgCBs one message at a time.
 */
void MT_ZdoSendMsgCB(zdoIncomingMsg_t *pMsg);

/***************************************************************************************************
***************************************************************************************************/
