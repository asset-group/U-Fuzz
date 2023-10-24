/**************************************************************************************************
  Filename:       aps_frag.h
  Revised:        $Date: 2011-05-27 16:00:27 -0700 (Fri, 27 May 2011) $
  Revision:       $Revision: 26145 $

  Description:    Implements APS Application Data Unit Fragmentation


  Copyright 2006-2011 Texas Instruments Incorporated. All rights reserved.

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
#ifndef APSF_H
#define APSF_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "ZComDef.h"
#include "AF.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define APSF_SCHED_EVT             0x0001
#define APSF_MAX_WINDOW_SIZE       8  // Maximum allowable fragmentation window size.

/* ------------------------------------------------------------------------------------------------
 *                                            Macros
 * ------------------------------------------------------------------------------------------------
 */

#define APSF_Enabled              (APSF_taskID != 0xff)

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

extern uint8 APSF_taskID;

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

/*
 * Task Initialization
 */
extern void APSF_Init(uint8 task_id);

/*
 * Task Event Processor
 */
extern void APSF_Init(uint8 task_id);
extern UINT16 APSF_ProcessEvent( uint8 task_id, UINT16 events );
extern afStatus_t APSF_SendFragmented(APSDE_DataReq_t *pReq);
extern void APSF_ProcessAck(aps_FrameFormat_t *aff, uint16 srcAddr, uint8 status);
extern void APSF_SendOsalMsg(uint8 *msgPtr);

typedef afStatus_t APSF_SendFragmented_t(APSDE_DataReq_t *pReq);
typedef void APSF_ProcessAck_t(aps_FrameFormat_t *aff, uint16 srcAddr, uint8 status);
typedef void APSF_SendOsalMsg_t(uint8 *msgPtr);

extern APSF_SendFragmented_t *apsfSendFragmented;
extern APSF_ProcessAck_t *apsfProcessAck;
extern APSF_SendOsalMsg_t *apsfSendOsalMsg;

#ifdef __cplusplus
}
#endif
#endif
/**************************************************************************************************
*/
