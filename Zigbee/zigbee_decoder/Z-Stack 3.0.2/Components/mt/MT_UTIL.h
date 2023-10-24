/***************************************************************************************************
  Filename:       MT_UTIL.h
  Revised:        $Date: 2014-11-06 23:59:26 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41038 $

  Description:  MonitorTest Utility Functions


  Copyright 2007 - 2009 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MT_UTIL_H
#define MT_UTIL_H

#if defined ZCL_KEY_ESTABLISH
#include "zcl_key_establish.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

#if defined (MT_UTIL_FUNC)
  
  void MT_UtilSetDevNwkInfo( uint8 *pBuf );
/*
 * Process MT_SYS commands
 */
extern uint8 MT_UtilCommandProcessing(uint8 *pBuf);

#if defined ZCL_KEY_ESTABLISH
/***************************************************************************************************
 * @fn      MT_UtilKeyEstablishInd
 *
 * @brief   Proxy the ZCL_KEY_ESTABLISH_IND command.
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
void MT_UtilKeyEstablishInd(zclKE_StatusInd_t *pInd);
#endif
#endif /* MT_UTIL_FUNC */

#ifdef __cplusplus
}
#endif

#endif /* MTEL_H */

/***************************************************************************************************
 ***************************************************************************************************/
