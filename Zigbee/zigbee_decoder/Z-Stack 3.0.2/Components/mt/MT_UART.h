/***************************************************************************************************
  Filename:       MT_UART.h
  Revised:        $Date: 2009-07-02 15:24:39 -0700 (Thu, 02 Jul 2009) $
  Revision:       $Revision: 20269 $

  Description:    This header describes the functions that handle the serial port.

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

***************************************************************************************************/
#ifndef MT_UART_H
#define MT_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 *                                               INCLUDES
 ***************************************************************************************************/
#include "Onboard.h"
#include "OSAL.h"

/***************************************************************************************************
 *                                             CONSTANTS
 ***************************************************************************************************/

/* Start-of-frame delimiter for UART transport */
#define MT_UART_SOF                     0xFE

/* UART frame overhead for SOF and FCS bytes */
#define MT_UART_FRAME_OVHD              2

/* Default values */
#if defined (ZTOOL_P1) || defined (ZTOOL_P2)
  #define MT_UART_DEFAULT_PORT           ZTOOL_PORT
#elif defined (ZAPP_P1) || defined (ZAPP_P2)
  #define MT_UART_DEFAULT_PORT           ZAPP_PORT
#endif

#if !defined( MT_UART_DEFAULT_OVERFLOW )
  #define MT_UART_DEFAULT_OVERFLOW       TRUE
#endif

#if !defined MT_UART_DEFAULT_BAUDRATE
#define MT_UART_DEFAULT_BAUDRATE         HAL_UART_BR_38400
#endif
#define MT_UART_DEFAULT_THRESHOLD        MT_UART_THRESHOLD
#define MT_UART_DEFAULT_MAX_RX_BUFF      MT_UART_RX_BUFF_MAX
#if !defined( MT_UART_DEFAULT_MAX_TX_BUFF )
  #define MT_UART_DEFAULT_MAX_TX_BUFF    MT_UART_TX_BUFF_MAX
#endif
#define MT_UART_DEFAULT_IDLE_TIMEOUT     MT_UART_IDLE_TIMEOUT

/* Application Flow Control */
#define MT_UART_ZAPP_RX_NOT_READY         0x00
#define MT_UART_ZAPP_RX_READY             0x01

typedef struct
{
  osal_event_hdr_t  hdr;
  uint8             *msg;
} mtOSALSerialData_t;

/*
 * Initialization
 */
extern void MT_UartInit (void);

/*
 * Process ZTool Rx Data
 */
void MT_UartProcessZToolData ( uint8 port, uint8 taskId );

/*
 * Process ZApp Rx Data
 */
void MT_UartProcessZAppData ( uint8 port, uint8 event );

/*
 * Calculate the check sum
 */
extern uint8 MT_UartCalcFCS( uint8 *msg_ptr, uint8 length );

/*
 * Register TaskID for the application
 */
extern void MT_UartRegisterTaskID( uint8 taskID );

/*
 * Register max length that application can take
 */
extern void MT_UartZAppBufferLengthRegister ( uint16 maxLen );

/*
 * Turn Application flow control ON/OFF
 */
extern void MT_UartAppFlowControl ( uint8 status );

/***************************************************************************************************
***************************************************************************************************/

#endif  /* MT_UART_H */
