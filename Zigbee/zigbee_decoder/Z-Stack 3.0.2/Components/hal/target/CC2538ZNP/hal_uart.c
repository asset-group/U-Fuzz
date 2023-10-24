/**************************************************************************************************
  Filename:       hal_uart.c
  Revised:        $Date: 2013-11-08 09:50:13 -0800 (Fri, 08 Nov 2013) $
  Revision:       $Revision: 35968 $

  Description:    This file contains the interface to the UART.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#if HAL_UART_USB
#include "hal_uart_usb.c"
#else
#include "hal_uart_isr.c"
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Global Functions
 * ------------------------------------------------------------------------------------------------
 */

/*************************************************************************************************
 * @fn      HalUARTInit()
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *************************************************************************************************/
void HalUARTInit(void)
{
#if HAL_UART_USB
  HalUARTInitUSB();
#else
  HalUARTInitIsr();
#endif
}

/*************************************************************************************************
 * @fn      HalUARTOpen()
 *
 * @brief   Open a port based on the configuration
 *
 * @param   port   - UART port
 *          config - contains configuration information
 *          cBack  - Call back function where events will be reported back
 *
 * @return  Status of the function call
 *************************************************************************************************/
uint8 HalUARTOpen(uint8 port, halUARTCfg_t *config)
{
#if HAL_UART_USB
  (void)port;
  HalUARTOpenUSB(config);
  return HAL_UART_SUCCESS;
#else
  return(HalUARTOpenIsr(port, config));
#endif
}

/*************************************************************************************************
 * @fn      Hal_UARTPoll
 *
 * @brief   This routine simulate polling and has to be called by the main loop
 *
 * @param   void
 *
 * @return  void
 *************************************************************************************************/
void HalUARTPoll(void)
{
#ifdef HAL_UART_USB
  HalUARTPollUSB();
#else
  HalUARTPollIsr();
#endif
}

/*************************************************************************************************
 * @fn      HalUARTClose()
 *
 * @brief   Close the UART
 *
 * @param   port - UART port (not used.)
 *
 * @return  none
 *************************************************************************************************/
void HalUARTClose(uint8 port)
{
#ifdef HAL_UART_USB
  
#else   
 HalUARTCloseIsr(port);
#endif
}

/*************************************************************************************************
 * @fn      HalUARTRead()
 *
 * @brief   Read a buffer from the UART
 *
 * @param   port - UART port (not used.)
 *          ppBuffer - pointer to a pointer that points to the data that will be read
 *          length - length of the requested buffer
 *
 * @return  length of buffer that was read
 *************************************************************************************************/
uint16 HalUARTRead ( uint8 port, uint8 *pBuffer, uint16 length )
{
#if HAL_UART_USB
  return HalUARTRx(pBuffer, length);
#else
  return (HalUARTReadIsr( port, pBuffer, length ));
#endif
}

/*************************************************************************************************
 * @fn      HalUARTWrite()
 *
 * @brief   Write a buffer to the UART
 *
 * @param   port    - UART port (not used.)
 *          pBuffer - pointer to the buffer that will be written
 *          length  - length of
 *
 * @return  length of the buffer that was sent
 *************************************************************************************************/
uint16 HalUARTWrite(uint8 port, uint8 *pBuffer, uint16 length)
{
#if HAL_UART_USB
  return HalUARTTx(pBuffer, length);
#else
  return (HalUARTWriteIsr( port, pBuffer, length ));
#endif
}

/*************************************************************************************************
 * @fn      Hal_UART_RxBufLen()
 *
 * @brief   Calculate Rx Buffer length of a port
 *
 * @param   port - UART port (not used.)
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
uint16 Hal_UART_RxBufLen (uint8 port)
{
#if HAL_UART_USB
  return HalUARTRxAvailUSB();
#else
  return (Hal_UART_RxBufLenIsr(port));
#endif
}

/**************************************************************************************************
*/





