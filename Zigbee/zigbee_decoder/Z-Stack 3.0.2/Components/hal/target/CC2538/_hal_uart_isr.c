/**************************************************************************************************
  Filename:       _hal_uart_isr.c
  Revised:        $Date: 2014-12-05 13:07:19 -0800 (Fri, 05 Dec 2014) $
  Revision:       $Revision: 41365 $

  Description:    This file contains the interface to the UART.


  Copyright 2013-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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

#include "hal_board.h"
#include "hal_types.h"
#include "hal_uart.h"
#include "OSAL.h"
#include "OSAL_Timers.h"
#include "hw_ioc.h"
#include "hw_uart.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */
#define HAL_UART_PORT                      UART1_BASE
#define HAL_UART_SYS_CTRL                  SYS_CTRL_PERIPH_UART1
#define HAL_UART_INT_CTRL                  INT_UART1
#define HalUartISR                         interrupt_uart1

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

const uint32 UBRRTable[] = {
  9600,
  19200,
  38400,
  57600,
  115200
};

static halUARTCfg_t uartRecord;
static bool txMT;

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void recRst(void);
static void procRx(void);
static void procTx(void);

/* ------------------------------------------------------------------------------------------------
 *                                           Global Functions
 * ------------------------------------------------------------------------------------------------
 */

void interrupt_uart(void);
void HalUARTInitIsr(void);
uint8 HalUARTOpenIsr(uint8 port, halUARTCfg_t *config);
void HalUARTPollIsr(void);
void HalUARTCloseIsr(uint8 port);
uint16 HalUARTReadIsr ( uint8 port, uint8 *pBuffer, uint16 length );
uint16 HalUARTWriteIsr(uint8 port, uint8 *pBuffer, uint16 length);
uint16 Hal_UART_RxBufLenIsr(uint8 port);
void HalUartISR(void);

/*************************************************************************************************
 * @fn      HalUARTInitIsr()
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *************************************************************************************************/
void HalUARTInitIsr(void)
{
   SysCtrlPeripheralEnable(HAL_UART_SYS_CTRL);

  /* Setup PB0 as UART_CTS, PD3 as UART_RTS  
   * PA1 as UART_TX and PA0 as UART_RX
   */ 
  IOCPinConfigPeriphOutput(GPIO_A_BASE, GPIO_PIN_1, IOC_MUX_OUT_SEL_UART1_TXD);
  IOCPinConfigPeriphInput(GPIO_A_BASE, GPIO_PIN_0, IOC_UARTRXD_UART1);
  GPIOPinTypeUARTInput(GPIO_A_BASE, GPIO_PIN_0);
  GPIOPinTypeUARTOutput(GPIO_A_BASE, GPIO_PIN_1);  
  recRst();

}

/*************************************************************************************************
 * @fn      HalUARTOpenIsr()
 *
 * @brief   Open a port based on the configuration
 *
 * @param   port   - UART port
 *          config - contains configuration information
 *          cBack  - Call back function where events will be reported back
 *
 * @return  Status of the function call
 *************************************************************************************************/
uint8 HalUARTOpenIsr(uint8 port, halUARTCfg_t *config)
{
  if (uartRecord.configured)
  {
    HalUARTClose(port);
  }

  if (config->baudRate > HAL_UART_BR_115200)
  {
    return HAL_UART_BAUDRATE_ERROR;
  }

  if (((uartRecord.rx.pBuffer = osal_mem_alloc(config->rx.maxBufSize)) == NULL) ||
      ((uartRecord.tx.pBuffer = osal_mem_alloc(config->tx.maxBufSize)) == NULL))
  {
    if (uartRecord.rx.pBuffer != NULL)
    {
      osal_mem_free(uartRecord.rx.pBuffer);
      uartRecord.rx.pBuffer = NULL;
    }

    return HAL_UART_MEM_FAIL;
  }
  
  if(config->flowControl)
  {
    IOCPinConfigPeriphOutput(GPIO_D_BASE, GPIO_PIN_3, IOC_MUX_OUT_SEL_UART1_RTS);
    GPIOPinTypeUARTOutput(GPIO_D_BASE, GPIO_PIN_3);
    IOCPinConfigPeriphInput(GPIO_B_BASE, GPIO_PIN_0, IOC_UARTCTS_UART1);
    GPIOPinTypeUARTInput(GPIO_B_BASE, GPIO_PIN_0);
  }
  
  IntEnable(HAL_UART_INT_CTRL);

  uartRecord.configured = TRUE;
  uartRecord.baudRate = config->baudRate;
  uartRecord.flowControl = config->flowControl;
  uartRecord.flowControlThreshold = (config->flowControlThreshold > config->rx.maxBufSize) ? 0 :
                                     config->flowControlThreshold;
  uartRecord.idleTimeout = config->idleTimeout;
  uartRecord.rx.maxBufSize = config->rx.maxBufSize;
  uartRecord.tx.maxBufSize = config->tx.maxBufSize;
  uartRecord.intEnable = config->intEnable;
  uartRecord.callBackFunc = config->callBackFunc;

  UARTConfigSetExpClk(HAL_UART_PORT, SysCtrlClockGet(), UBRRTable[uartRecord.baudRate],
                         (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));

  /* FIFO level set to 1/8th for both RX and TX which is 2 bytes */
  UARTFIFOLevelSet(HAL_UART_PORT, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
  UARTFIFOEnable(HAL_UART_PORT);

  /* Clear and enable UART TX, RX, CTS and Recieve Timeout interrupt */
  UARTIntClear(HAL_UART_PORT, (UART_INT_RX | UART_INT_TX | UART_INT_CTS | UART_INT_RT ));
  UARTIntEnable(HAL_UART_PORT, (UART_INT_RX | UART_INT_TX | UART_INT_CTS | UART_INT_RT ));
  
  if(config->flowControl)
  {
    /* Enable hardware flow control by enabling CTS and RTS */
    HWREG(HAL_UART_PORT + UART_O_CTL) |= (UART_CTL_CTSEN | UART_CTL_RTSEN );
  }
  UARTEnable(HAL_UART_PORT);

  return HAL_UART_SUCCESS;
}

/*************************************************************************************************
 * @fn      Hal_UARTPollIsr
 *
 * @brief   This routine simulate polling and has to be called by the main loop
 *
 * @param   void
 *
 * @return  void
 *************************************************************************************************/
void HalUARTPollIsr(void)
{
  uint16 head = uartRecord.tx.bufferHead;
  uint16 tail = uartRecord.tx.bufferTail;
  /* If port is not configured, no point to poll it. */
  if (!uartRecord.configured)  
  {
    return;
  }

  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION(intState);
  procRx();
  procTx();
  HAL_EXIT_CRITICAL_SECTION(intState);

  uint8 evts = 0;
  /* Report if Rx Buffer is full. */
  if ((Hal_UART_RxBufLen(0) + 1) >= uartRecord.rx.maxBufSize)  
  {
    evts = HAL_UART_RX_FULL;
  }

  /* Report if Rx Buffer is idled. */
  if ((uartRecord.rxChRvdTime != 0) &&  
     ((osal_GetSystemClock() - uartRecord.rxChRvdTime) > uartRecord.idleTimeout))
  {
    uartRecord.rxChRvdTime = 0;
    evts |= HAL_UART_RX_TIMEOUT;
  }

  if (Hal_UART_RxBufLen(0) >= uartRecord.rx.maxBufSize - uartRecord.flowControlThreshold)
  {
    evts |= HAL_UART_RX_ABOUT_FULL;
  }

  if (!txMT && (head == tail))
  {
    txMT = true;
    evts |= HAL_UART_TX_EMPTY;
  }

  if (evts && uartRecord.callBackFunc)
  {
    (uartRecord.callBackFunc)(0, evts);
  }

}

/*************************************************************************************************
 * @fn      HalUARTCloseIsr()
 *
 * @brief   Close the UART
 *
 * @param   port - UART port (not used.)
 *
 * @return  none
 *************************************************************************************************/
void HalUARTCloseIsr(uint8 port)
{
  (void)port;

  UARTDisable(HAL_UART_PORT);

  if (uartRecord.configured)
  {
    (void)osal_mem_free(uartRecord.rx.pBuffer);
    (void)osal_mem_free(uartRecord.tx.pBuffer);
    recRst();
  }
}

/*************************************************************************************************
 * @fn      HalUARTReadIsr()
 *
 * @brief   Read a buffer from the UART
 *
 * @param   port - UART port (not used.)
 *          ppBuffer - pointer to a pointer that points to the data that will be read
 *          length - length of the requested buffer
 *
 * @return  length of buffer that was read
 *************************************************************************************************/
uint16 HalUARTReadIsr ( uint8 port, uint8 *pBuffer, uint16 length )
{
  uint16 cnt, idx;
  (void)port;

  /* If port is not configured, no point to read it. */
  if (!uartRecord.configured)
  {
    return 0;
  }

  /* If requested length is bigger than what in 
   * buffer, re-adjust it to the buffer length.
   */
  cnt = Hal_UART_RxBufLen(0);
  if (cnt < length)
  {
    length = cnt;
  }

  idx = uartRecord.rx.bufferHead;
  for (cnt = 0; cnt < length; cnt++)
  {
    pBuffer[cnt] = uartRecord.rx.pBuffer[idx++];

    if (idx >= uartRecord.rx.maxBufSize)
    {
      idx = 0;
    }
  }
  uartRecord.rx.bufferHead = idx;

  /* Return number of bytes read. */
  return length;  
}

/*************************************************************************************************
 * @fn      HalUARTWriteIsr()
 *
 * @brief   Write a buffer to the UART
 *
 * @param   port    - UART port (not used.)
 *          pBuffer - pointer to the buffer that will be written
 *          length  - length of
 *
 * @return  length of the buffer that was sent
 *************************************************************************************************/
uint16 HalUARTWriteIsr(uint8 port, uint8 *pBuffer, uint16 length)
{
  (void)port;

  if (!uartRecord.configured)
  {
    return 0;
  }

  uint16 idx = uartRecord.tx.bufferHead;
  uint16 cnt = uartRecord.tx.bufferTail;

  if (cnt == idx)
  {
    cnt = uartRecord.tx.maxBufSize;
  }
  else if (cnt > idx)
  {
    cnt = uartRecord.tx.maxBufSize - cnt + idx;
  }
  else /* (cnt < idx) */
  {
    cnt = idx - cnt;
  }

  /* Accept "all-or-none" on write request. */
  if (cnt < length)
  {
    return 0;
  }

  txMT = false;
  idx = uartRecord.tx.bufferTail;

  for (cnt = 0; cnt < length; cnt++)
  {
    uartRecord.tx.pBuffer[idx++] = pBuffer[cnt];

    if (idx >= uartRecord.tx.maxBufSize)
    {
      idx = 0;
    }
  }

  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION(intState);
  uartRecord.tx.bufferTail = idx;
  procTx();
  HAL_EXIT_CRITICAL_SECTION(intState);

  /* Return the number of bytes actually put into the buffer. */
  return length;  
}

/*************************************************************************************************
 * @fn      Hal_UART_RxBufLenIsr()
 *
 * @brief   Calculate Rx Buffer length of a port
 *
 * @param   port - UART port (not used.)
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
uint16 Hal_UART_RxBufLenIsr(uint8 port)
{
  int16 length = uartRecord.rx.bufferTail;
  (void)port;

  length -= uartRecord.rx.bufferHead;
  if  (length < 0)
    length += uartRecord.rx.maxBufSize;

  return (uint16)length;
}

/*************************************************************************************************
 * @fn      Hal_UART_TxBufLen()
 *
 * @brief   Calculate Tx Buffer length of a port
 *
 * @param   port - UART port (not used.)
 *
 * @return  length of current Tx buffer
 *************************************************************************************************/
uint16 Hal_UART_TxBufLen( uint8 port )
{
  int16 length = uartRecord.tx.bufferTail;
  (void)port;

  length -= uartRecord.tx.bufferHead;
  if  (length < 0)
    length += uartRecord.tx.maxBufSize;

  return (uint16)length;
}

/*************************************************************************************************
 * @fn      recRst()
 *
 * @brief   Reset the UART record.
 *
 * @param   none
 *
 * @return  none
 *************************************************************************************************/
static void recRst(void)
{
  uartRecord.configured        = FALSE;
  uartRecord.rx.bufferHead     = 0;
  uartRecord.rx.bufferTail     = 0;
  uartRecord.rx.pBuffer        = (uint8 *)NULL;
  uartRecord.tx.bufferHead     = 0;
  uartRecord.tx.bufferTail     = 0;
  uartRecord.tx.pBuffer        = (uint8 *)NULL;
  uartRecord.rxChRvdTime       = 0;
  uartRecord.intEnable         = FALSE;
}

/*************************************************************************************************
 * @fn      procRx
 *
 * @brief   Process Tx bytes.
 *
 * @param   void
 *
 * @return  void
 *************************************************************************************************/
static void procRx(void)
{
  uint16 tail = uartRecord.rx.bufferTail;

  while (UARTCharsAvail(HAL_UART_PORT))
  {
    uartRecord.rx.pBuffer[tail++] = UARTCharGetNonBlocking(HAL_UART_PORT);

    if (tail >= uartRecord.rx.maxBufSize)
    {
      tail = 0;
    }
  }

  if (uartRecord.rx.bufferTail != tail)
  {
    uartRecord.rx.bufferTail = tail;
    uartRecord.rxChRvdTime = osal_GetSystemClock();
  }
}

/*************************************************************************************************
 * @fn      procTx
 *
 * @brief   Process Tx bytes.
 *
 * @param   void
 *
 * @return  void
 *************************************************************************************************/
static void procTx(void)
{
  uint16 head = uartRecord.tx.bufferHead;
  uint16 tail = uartRecord.tx.bufferTail;

  while ((head != tail) && (UARTCharPutNonBlocking(HAL_UART_PORT, uartRecord.tx.pBuffer[head])))
  {
    if (++head >= uartRecord.tx.maxBufSize)
    {
      head = 0;
    }
  }

  uartRecord.tx.bufferHead = head;
}

/*************************************************************************************************
 * @fn      UART Rx/Tx ISR
 *
 * @brief   Called when a serial byte is ready to read and/or write.
 * NOTE:   Assumes that uartRecord.configured is TRUE if this interrupt is enabled.
 *
 * @param   void
 *
 * @return  void
**************************************************************************************************/
void HalUartISR(void)
{
  UARTIntClear(HAL_UART_PORT, (UART_INT_RX |  UART_INT_RT));
  procRx();

  UARTIntClear(HAL_UART_PORT, (UART_INT_TX | UART_INT_CTS));
  procTx();
}

/**************************************************************************************************
*/
