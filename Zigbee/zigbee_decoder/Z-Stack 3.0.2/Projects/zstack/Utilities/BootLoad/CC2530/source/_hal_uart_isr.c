/**************************************************************************************************
  Filename:       hal_uart_isr.c
  Revised:        $Date: 2014-12-09 14:38:44 -0800 (Tue, 09 Dec 2014) $
  Revision:       $Revision: 41399 $

  Description: This file contains the interface to the H/W UART driver by polled-ISR for the SBL.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "hal_assert.h"
#include "hal_board.h"
#include "hal_defs.h"
#include "hal_mcu.h"
#include "hal_uart.h"

/*********************************************************************
 * MACROS
 */

//#define HAL_UART_ASSERT(expr)        HAL_ASSERT((expr))
#define HAL_UART_ASSERT(expr)

#define HAL_UART_ISR_RX_AVAIL() \
  (isrCfg.rxTail >= isrCfg.rxHead) ? \
  (isrCfg.rxTail - isrCfg.rxHead) : \
  (HAL_UART_ISR_RX_MAX - isrCfg.rxHead + isrCfg.rxTail)

#define HAL_UART_ISR_TX_AVAIL() \
  (isrCfg.txHead > isrCfg.txTail) ? \
  (isrCfg.txHead - isrCfg.txTail - 1) : \
  (HAL_UART_ISR_TX_MAX - isrCfg.txTail + isrCfg.txHead - 1)

/*********************************************************************
 * CONSTANTS
 */

// UxCSR - USART Control and Status Register.
#define CSR_MODE                   0x80
#define CSR_RE                     0x40
#define CSR_SLAVE                  0x20
#define CSR_FE                     0x10
#define CSR_ERR                    0x08
#define CSR_RX_BYTE                0x04
#define CSR_TX_BYTE                0x02
#define CSR_ACTIVE                 0x01

// UxUCR - USART UART Control Register.
#define UCR_FLUSH                  0x80
#define UCR_FLOW                   0x40
#define UCR_D9                     0x20
#define UCR_BIT9                   0x10
#define UCR_PARITY                 0x08
#define UCR_SPB                    0x04
#define UCR_STOP                   0x02
#define UCR_START                  0x01

#define UTX0IE                     0x04
#define UTX1IE                     0x08

#define P2DIR_PRIPO                0xC0

// Incompatible redefinitions result with more than one UART driver sub-module.
#undef PxOUT
#undef PxDIR
#undef PxSEL
#undef UxCSR
#undef UxUCR
#undef UxDBUF
#undef UxBAUD
#undef UxGCR
#undef URXxIE
#undef URXxIF
#undef UTXxIE
#undef UTXxIF
#undef HAL_UART_PERCFG_BIT
#undef HAL_UART_Px_RTS
#undef HAL_UART_Px_CTS
#undef HAL_UART_Px_RX_TX
#if (HAL_UART_ISR == 1)
#define PxOUT                      P0
#define PxDIR                      P0DIR
#define PxSEL                      P0SEL
#define UxCSR                      U0CSR
#define UxUCR                      U0UCR
#define UxDBUF                     U0DBUF
#define UxBAUD                     U0BAUD
#define UxGCR                      U0GCR
#define URXxIE                     URX0IE
#define URXxIF                     URX0IF
#define UTXxIE                     UTX0IE
#define UTXxIF                     UTX0IF
#else
#define PxOUT                      P1
#define PxDIR                      P1DIR
#define PxSEL                      P1SEL
#define UxCSR                      U1CSR
#define UxUCR                      U1UCR
#define UxDBUF                     U1DBUF
#define UxBAUD                     U1BAUD
#define UxGCR                      U1GCR
#define URXxIE                     URX1IE
#define URXxIF                     URX1IF
#define UTXxIE                     UTX1IE
#define UTXxIF                     UTX1IF
#endif

#if (HAL_UART_ISR == 1)
#define HAL_UART_PERCFG_BIT        0x01         // USART0 on P0, Alt-1; so clear this bit.
#define HAL_UART_Px_RX_TX          0x0C         // Peripheral I/O Select for Rx/Tx.
#define HAL_UART_Px_RTS            0x20         // Peripheral I/O Select for RTS.
#define HAL_UART_Px_CTS            0x10         // Peripheral I/O Select for CTS.
#else
#define HAL_UART_PERCFG_BIT        0x02         // USART1 on P1, Alt-2; so set this bit.
#define HAL_UART_Px_RTS            0x20         // Peripheral I/O Select for RTS.
#define HAL_UART_Px_CTS            0x10         // Peripheral I/O Select for CTS.
#define HAL_UART_Px_RX_TX          0xC0         // Peripheral I/O Select for Rx/Tx.
#endif

// The timeout tick is at 32-kHz, so multiply msecs by 33.
#define HAL_UART_MSECS_TO_TICKS    33

#if defined MT_TASK
#define HAL_UART_ISR_TX_MAX        MT_UART_DEFAULT_MAX_TX_BUFF
#define HAL_UART_ISR_RX_MAX        MT_UART_DEFAULT_MAX_RX_BUFF
#define HAL_UART_ISR_HIGH          MT_UART_DEFAULT_THRESHOLD
#define HAL_UART_ISR_IDLE         (MT_UART_DEFAULT_IDLE_TIMEOUT * HAL_UART_MSECS_TO_TICKS)
#else
#if !defined HAL_UART_ISR_RX_MAX
#define HAL_UART_ISR_RX_MAX        128
#endif
#if !defined HAL_UART_ISR_TX_MAX
#define HAL_UART_ISR_TX_MAX        HAL_UART_ISR_RX_MAX
#endif
#if !defined HAL_UART_ISR_HIGH
#define HAL_UART_ISR_HIGH         (HAL_UART_ISR_RX_MAX / 2 - 16)
#endif
#if !defined HAL_UART_ISR_IDLE
#define HAL_UART_ISR_IDLE         (6 * HAL_UART_MSECS_TO_TICKS)
#endif
#endif

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8 rxBuf[HAL_UART_ISR_RX_MAX];
#if HAL_UART_ISR_RX_MAX < 256
  uint8 rxHead;
  uint8 rxTail;
#else
  uint16 rxHead;
  uint16 rxTail;
#endif
  uint8 rxTick;
  uint8 rxShdw;

  uint8 txBuf[HAL_UART_ISR_TX_MAX];
#if HAL_UART_ISR_TX_MAX < 256
  uint8 txHead;
  uint8 txTail;
#else
  uint16 txHead;
  uint16 txTail;
#endif
  uint8 txMT;

  halUARTCBack_t uartCB;
} uartISRCfg_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uartISRCfg_t isrCfg;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void HalUARTInitISR(void);
static void HalUARTOpenISR(halUARTCfg_t *config);
uint16 HalUARTReadISR(uint8 *buf, uint16 len);
uint16 HalUARTWriteISR(uint8 *buf, uint16 len);
static void HalUARTPollISR(void);
static uint16 HalUARTRxAvailISR(void);
static void HalUARTSuspendISR(void);
static void HalUARTResumeISR(void);

static void halUartRxIsr(void);
static void halUartTxIsr(void);

/******************************************************************************
 * @fn      HalUARTInitISR
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTInitISR(void)
{
  // Set P2 priority - USART0 over USART1 if both are defined.
  P2DIR &= ~P2DIR_PRIPO;
  P2DIR |= HAL_UART_PRIPO;

#if (HAL_UART_ISR == 1)
  PERCFG &= ~HAL_UART_PERCFG_BIT;    // Set UART0 I/O location to P0.
#else
  PERCFG |= HAL_UART_PERCFG_BIT;     // Set UART1 I/O location to P1.
#endif
  PxSEL  |= HAL_UART_Px_RX_TX;       // Enable Tx and Rx on P1.
  ADCCFG &= ~HAL_UART_Px_RX_TX;      // Make sure ADC doesnt use this.
  UxCSR = CSR_MODE;                  // Mode is UART Mode.
  UxUCR = UCR_FLUSH;                 // Flush it.
}

/******************************************************************************
 * @fn      HalUARTUnInitISR
 *
 * @brief   UnInitialize the UART.
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTUnInitISR(void)
{
  UxCSR = 0;
  URXxIE = 0;
  URXxIF = 0;
  IEN2 &= ~UTXxIE;
  UTXxIF = 0;
}

/******************************************************************************
 * @fn      HalUARTOpenISR
 *
 * @brief   Open a port according tp the configuration specified by parameter.
 *
 * @param   config - contains configuration information
 *
 * @return  none
 *****************************************************************************/
static void HalUARTOpenISR(halUARTCfg_t *config)
{
  isrCfg.uartCB = config->callBackFunc;
  // Only supporting subset of baudrate for code size - other is possible.
  HAL_UART_ASSERT((config->baudRate == HAL_UART_BR_9600) ||
                  (config->baudRate == HAL_UART_BR_19200) ||
                  (config->baudRate == HAL_UART_BR_38400) ||
                  (config->baudRate == HAL_UART_BR_57600) ||
                  (config->baudRate == HAL_UART_BR_115200));

  if (config->baudRate == HAL_UART_BR_57600 ||
      config->baudRate == HAL_UART_BR_115200)
  {
    UxBAUD = 216;
  }
  else
  {
    UxBAUD = 59;
  }

  switch (config->baudRate)
  {
    case HAL_UART_BR_9600:
      UxGCR = 8;
      break;
    case HAL_UART_BR_19200:
      UxGCR = 9;
      break;
    case HAL_UART_BR_38400:
    case HAL_UART_BR_57600:
      UxGCR = 10;
      break;
    default:
      UxGCR = 11;
      break;
  }

  // 8 bits/char; no parity; 1 stop bit; stop bit hi.
  if (config->flowControl)
  {
    UxUCR = UCR_FLOW | UCR_STOP;
    PxSEL |= HAL_UART_Px_RTS | HAL_UART_Px_CTS;
  }
  else
  {
    UxUCR = UCR_STOP;
  }

  UxCSR |= CSR_RE;
}

/*****************************************************************************
 * @fn      HalUARTReadISR
 *
 * @brief   Read a buffer from the UART
 *
 * @param   buf  - valid data buffer at least 'len' bytes in size
 *          len  - max length number of bytes to copy to 'buf'
 *
 * @return  length of buffer that was read
 *****************************************************************************/
uint16 HalUARTReadISR(uint8 *buf, uint16 len)
{
  uint16 cnt = 0;

  while ((isrCfg.rxHead != isrCfg.rxTail) && (cnt < len))
  {
    *buf++ = isrCfg.rxBuf[isrCfg.rxHead++];
    if (isrCfg.rxHead >= HAL_UART_ISR_RX_MAX)
    {
      isrCfg.rxHead = 0;
    }
    cnt++;
  }

  return cnt;
}

/******************************************************************************
 * @fn      HalUARTWriteISR
 *
 * @brief   Write a buffer to the UART.
 *
 * @param   buf - pointer to the buffer that will be written, not freed
 *          len - length of
 *
 * @return  length of the buffer that was sent
 *****************************************************************************/
uint16 HalUARTWriteISR(uint8 *buf, uint16 len)
{
  uint16 cnt;

  // Enforce all or none.
  if (HAL_UART_ISR_TX_AVAIL() < len)
  {
    return 0;
  }

  for (cnt = 0; cnt < len; cnt++)
  {
    isrCfg.txBuf[isrCfg.txTail] = *buf++;
    isrCfg.txMT = 0;

    if (isrCfg.txTail >= HAL_UART_ISR_TX_MAX-1)
    {
      isrCfg.txTail = 0;
    }
    else
    {
      isrCfg.txTail++;
    }

    UTXxIF = 1;
  }

  return cnt;
}

/******************************************************************************
 * @fn      HalUARTPollISR
 *
 * @brief   Poll a USART module implemented by ISR.
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTPollISR(void)
{
  while(URXxIF)  halUartRxIsr();
  while(UTXxIF)  halUartTxIsr();
}

/**************************************************************************************************
 * @fn      HalUARTRxAvailISR()
 *
 * @brief   Calculate Rx Buffer length - the number of bytes in the buffer.
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 **************************************************************************************************/
static uint16 HalUARTRxAvailISR(void)
{
  return HAL_UART_ISR_RX_AVAIL();
}

/******************************************************************************
 * @fn      HalUARTSuspendISR
 *
 * @brief   Suspend UART hardware before entering PM mode 1, 2 or 3.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void HalUARTSuspendISR( void )
{
  UxCSR &= ~CSR_RE;
}

/******************************************************************************
 * @fn      HalUARTResumeISR
 *
 * @brief   Resume UART hardware after exiting PM mode 1, 2 or 3.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void HalUARTResumeISR( void )
{
  UxUCR |= UCR_FLUSH;
  UxCSR |= CSR_RE;
}

/***************************************************************************************************
 * @fn      halUartRxIsr
 *
 * @brief   UART Receive Interrupt
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
static void halUartRxIsr(void)
{
  uint8 tmp = UxDBUF;
  URXxIF = 0;

  isrCfg.rxBuf[isrCfg.rxTail] = tmp;

  if (++isrCfg.rxTail >= HAL_UART_ISR_RX_MAX)
  {
    isrCfg.rxTail = 0;
  }
}

/***************************************************************************************************
 * @fn      halUartTxIsr
 *
 * @brief   UART Transmit Interrupt
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
static void halUartTxIsr(void)
{
  UTXxIF = 0;

  if (isrCfg.txHead != isrCfg.txTail)
  {
    UxDBUF = isrCfg.txBuf[isrCfg.txHead++];

    if (isrCfg.txHead >= HAL_UART_ISR_TX_MAX)
    {
      isrCfg.txHead = 0;
    }
  }
}

/******************************************************************************
******************************************************************************/
