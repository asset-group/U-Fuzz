/***************************************************************************************************
  Filename:       MTEL.h
  Revised:        $Date: 2013-05-17 11:05:33 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34353 $

  Description:

  Copyright 2007-2011 Texas Instruments Incorporated. All rights reserved.

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

 ***************************************************************************************************/
#ifndef MT_DEBUG_H
#define MT_DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "OSAL.h"

/***************************************************************************************************
 * TYPEDEFs
 ***************************************************************************************************/

typedef struct
{
  osal_event_hdr_t  hdr;
  uint8             compID;
  uint8             severity;
  uint8             numParams;
  uint16            param1;
  uint16            param2;
  uint16            param3;
  uint16            timestamp;
} mtDebugMsg_t;

typedef struct
{
  osal_event_hdr_t  hdr;
  uint8             strLen;
  uint8             *pString;
} mtDebugStr_t;

typedef struct {
#ifdef FEATURE_PACKET_FILTER_STATS
  uint32 nwkInvalidPackets;
  uint32 rxCrcFailure;
  uint32 rxCrcSuccess;
#endif
  uint8  fsmstat0;
  uint8  fsmstat1;
  uint8  macData_rxCount;
  uint8  macData_directCount;
  uint8  macMain_state;
  uint8  macRxActive;
  uint8  macTxActive;
} mtDebugMacDataDump_t;

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

#if defined (MT_DEBUG_FUNC)
/*
 * Process MT_DEBUG commands
 */
extern uint8 MT_DebugCommandProcessing(uint8 *pBuf);
#endif /* MT_DEBUG_FUNC */

/*
 * Process MT_DEBUG messages
 */
extern void MT_ProcessDebugMsg(mtDebugMsg_t *pData);

/*
 * Process MT_DEBUG strings
 */
extern void MT_ProcessDebugStr(mtDebugStr_t *pData);



#ifdef __cplusplus
}
#endif

#endif /* MTEL_H */

/***************************************************************************************************
 ***************************************************************************************************/
