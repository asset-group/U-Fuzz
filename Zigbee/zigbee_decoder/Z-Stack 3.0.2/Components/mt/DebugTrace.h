/**************************************************************************************************
  Filename:       DebugTrace.h
  Revised:        $Date: 2008-10-07 14:47:15 -0700 (Tue, 07 Oct 2008) $
  Revision:       $Revision: 18212 $

  Description:    This interface provides quick one-function-call functions to
                  Monitor and Test reporting mechanisms.


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

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

#ifndef DEBUGTRACE_H
#define DEBUGTRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */

/*
 * Windows Print String
 */
// command id's		
#define CMDID_RTG_ADD					1
#define CMDID_RTG_EXP					0x81
#define CMDID_RREQ_SEND				2
#define CMDID_RREQ_DROP				0x82
#define CMDID_RREP_SEND				3
#define CMDID_RREP_DROP				0x83
#define CMDID_RREQ_EXP				4

#define CMDID_DATA_SEND					6
#define CMDID_DATA_FORWARD			7
#define CMDID_DATA_RECEIVE			8

#define CMDID_BCAST_RCV				0x10
#define CMDID_BCAST_ACK				0x11
#define CMDID_BCAST_RETX			0x12

#define CMDID_BCAST_EXP				0x13
#define CMDID_BCAST_ERR				0x15

#define WPRINTSTR( s )

#if defined ( MT_TASK )
  /*
   * Trace Message
   *       - Same as debug_msg with SEVERITY_TRACE already filled in
   *       - Used to stand out in the source code.
   */
  #define TRACE_MSG( compID, nParams, p1, p2, p3 )  debug_msg( compID, SEVERITY_TRACE, nParams, p1, p2, p3 )


  /*
   * Debug Message (SEVERITY_INFORMATION)
   *      - Use this macro instead of calling debug_msg directly
   *      - So, it can be easily compiled out later
   */
  #define DEBUG_INFO( compID, subCompID, nParams, p1, p2, p3 )  debug_msg( compID, subCompID, nParams, p1, p2, p3 )


  /*** TEST MESSAGES ***/
  #define DBG_NWK_STARTUP           debug_msg( COMPID_TEST_NWK_STARTUP,         SEVERITY_INFORMATION, 0, 0, 0, 0 )
  #define DBG_SCAN_CONFIRM          debug_msg( COMPID_TEST_SCAN_CONFIRM,        SEVERITY_INFORMATION, 0, 0, 0, 0 )
  #define DBG_ASSOC_CONFIRM         debug_msg( COMPID_TEST_ASSOC_CONFIRM,       SEVERITY_INFORMATION, 0, 0, 0, 0 )
  #define DBG_REMOTE_DATA_CONFIRM   debug_msg( COMPID_TEST_REMOTE_DATA_CONFIRM, SEVERITY_INFORMATION, 0, 0, 0, 0 )

#else

  #define TRACE_MSG( compID, nParams, p1, p2, p3 )
  #define DEBUG_INFO( compID, subCompID, nParams, p1, p2, p3 )
  #define DBG_NWK_STARTUP
  #define DBG_SCAN_CONFIRM
  #define DBG_ASSOC_CONFIRM
  #define DBG_REMOTE_DATA_CONFIRM

#endif

/*********************************************************************
 * CONSTANTS
 */

#define SEVERITY_CRITICAL     0x01
#define SEVERITY_ERROR        0x02
#define SEVERITY_INFORMATION  0x03
#define SEVERITY_TRACE        0x04

#define NO_PARAM_DEBUG_LEN   5
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

  /*
   * Debug Message - Sent out serial port
   */


extern void debug_msg( uint8 compID, uint8 severity, uint8 numParams,
                       uint16 param1, uint16 param2, uint16 param3 );

extern void debug_str( uint8 *str_ptr );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* DEBUGTRACE_H */
