/**************************************************************************************************
  Filename:       DebugTrace.c
  Revised:        $Date: 2012-11-28 00:37:02 -0800 (Wed, 28 Nov 2012) $
  Revision:       $Revision: 32329 $


  Description:    This interface provides quick one-function-call functions to
                  Monitor and Test reporting mechanisms.


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

#if defined( MT_TASK ) || defined( APP_DEBUG )

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_TASK.h"
#include "MT_DEBUG.h"
#include "DebugTrace.h"

#if defined ( APP_DEBUG )
  #include "DebugApp.h"
#endif

 /*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

 /*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      debug_msg
 *
 * @brief
 *
 *   This feature allows modules to display debug information as
 *   applications execute in real-time.  This feature will work similar
 *   to "printf()" but will output to the serial port for display in
 *   the Z-Test tool.
 *
 *   This feature will most likely be compiled out in the production code
 *   to save code space.
 *
 * @param   byte compID - Component ID
 * @param   byte severity - CRITICAL(0x01), ERROR(0x02), INFORMATION(0x03)
 *                          or TRACE(0x04)
 * @param   byte numParams - number of parameter fields (param1-3)
 * @param   UINT16 param1 - user defined data
 * @param   UINT16 param2 - user defined data
 * @param   UINT16 param3 - user defined data
 *
 * @return  void
 */
void debug_msg( byte compID, byte severity, byte numParams, UINT16 param1, UINT16 param2, UINT16 param3 )
{

  mtDebugMsg_t *mtDebugMsg;
  UINT16 timestamp;

  if ( debugThreshold == 0 || debugCompId != compID )
    return;

  // Fill in the timestamp
  timestamp = 0;

  // Get a message buffer to build the debug message
  mtDebugMsg = (mtDebugMsg_t *)osal_msg_allocate( sizeof( mtDebugMsg_t ) );
  if ( mtDebugMsg )
  {
      mtDebugMsg->hdr.event = CMD_DEBUG_MSG;
      mtDebugMsg->compID = compID;
      mtDebugMsg->severity = severity;
      mtDebugMsg->numParams = numParams;

      mtDebugMsg->param1 = param1;
      mtDebugMsg->param2 = param2;
      mtDebugMsg->param3 = param3;
      mtDebugMsg->timestamp = timestamp;

      osal_msg_send( MT_TaskID, (uint8 *)mtDebugMsg );
  }

} /* debug_msg() */

/*********************************************************************
 * @fn      debug_str
 *
 * @brief
 *
 *   This feature allows modules to display a debug text string as
 *   applications execute in real-time. This feature will output to
 *   the serial port for display in the Z-Test tool.
 *
 *   This feature will most likely be compiled out in the production
 *   code in order to save code space.
 *
 * @param   byte *str_ptr - pointer to null-terminated string
 *
 * @return  void
 */
void debug_str( byte *str_ptr )
{
  mtDebugStr_t *msg;
  byte mln;
  byte strLen;

  // Text string length
  strLen = (byte)osal_strlen( (void*)str_ptr );

  // Debug string message length
  mln = sizeof ( mtDebugStr_t ) + strLen;

  // Get a message buffer to build the debug message
  msg = (mtDebugStr_t *)osal_msg_allocate( mln );
  if ( msg )
  {
    // Message type, length
    msg->hdr.event = CMD_DEBUG_STR;
    msg->strLen = strLen;

    // Append message, no terminator
    msg->pString = (uint8 *)(msg+1);
    osal_memcpy ( msg->pString, str_ptr, strLen );

    osal_msg_send( MT_TaskID, (uint8 *)msg );
  }
} // debug_str()

/*********************************************************************
*********************************************************************/
#endif  // MT_TASK
