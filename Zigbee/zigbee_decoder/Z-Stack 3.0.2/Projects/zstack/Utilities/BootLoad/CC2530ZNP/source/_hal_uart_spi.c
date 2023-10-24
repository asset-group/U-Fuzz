/**************************************************************************************************
  Filename:       _hal_uart_spi.c
  Revised:        $Date: 2010-03-10 20:36:55 -0800 (Wed, 10 Mar 2010) $
  Revision:       $Revision: 21890 $

  Description: This file contains the interface to the H/W UART driver by SPI, by ISR.
               Note that this is a trivial implementation only to support the boot loader and
               is not fit for a general SPI communication protocol which would have to be more
               sophisticated like the RPC for ZNP.


  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "hal_board.h"
#include "hal_defs.h"
#include "hal_mcu.h"
#include "hal_uart.h"

/*********************************************************************
 * MACROS
 */

#define HAL_UART_SPI_RX_AVAIL() \
  (spiCfg.rxTail >= spiCfg.rxHead) ? \
  (spiCfg.rxTail - spiCfg.rxHead) : \
  (HAL_UART_SPI_RX_MAX - spiCfg.rxHead + spiCfg.rxTail)

#define HAL_UART_SPI_TX_AVAIL() \
  (spiCfg.txHead > spiCfg.txTail) ? \
  (spiCfg.txHead - spiCfg.txTail - 1) : \
  (HAL_UART_SPI_TX_MAX - spiCfg.txTail + spiCfg.txHead - 1)

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
#if (HAL_UART_SPI == 1)
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

#if (HAL_UART_SPI == 1)
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

#if !defined HAL_UART_SPI_RX_MAX
#define HAL_UART_SPI_RX_MAX        128
#endif
#if !defined HAL_UART_SPI_TX_MAX
#define HAL_UART_SPI_TX_MAX        HAL_UART_SPI_RX_MAX
#endif
 
/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8 rxBuf[HAL_UART_SPI_RX_MAX];
#if HAL_UART_SPI_RX_MAX < 256
  uint8 rxHead;
  volatile uint8 rxTail;
#else
  uint16 rxHead;
  volatile uint16 rxTail;
#endif

  uint8 txBuf[HAL_UART_SPI_TX_MAX];
#if HAL_UART_SPI_TX_MAX < 256
  volatile uint8 txHead;
  uint8 txTail;
#else
  volatile uint16 txHead;
  uint16 txTail;
#endif
} uartSPICfg_t;

/*********************************************************************
 * LOCAL VARIABLES
 */

static uartSPICfg_t spiCfg;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void HalUARTInitSPI(void);
uint16 HalUARTReadSPI(uint8 *buf, uint16 len);
uint16 HalUARTWriteSPI(uint8 *buf, uint16 len);

/******************************************************************************
 * @fn      HalUARTInitSPI
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTInitSPI(void)
{
  /* Set bit order to MSB */
  UxGCR |= BV(5);

  /* Set UART1 I/O to alternate 2 location on P1 pins. */
#if (HAL_UART_SPI == 1)
  PERCFG &= ~HAL_UART_PERCFG_BIT;    // Set UART0 I/O location to P0.
#else
  PERCFG |= HAL_UART_PERCFG_BIT;     // Set UART1 I/O location to P1.

  /* Give UART1 priority over Timer3. */
  P2SEL &= ~0x20;  /* PRI2P1 */
#endif

  /* Mode select UART1 SPI Mode as slave. */
  UxCSR = CSR_SLAVE;

  /* Select peripheral function on I/O pins. */
  PxSEL |= 0xF0;  /* SELP1_[7:4] */

  UxCSR |= CSR_RE;
  URXxIE = 1;
}

/******************************************************************************
 * @fn      HalUARTUnInitSPI
 *
 * @brief   UnInitialize the UART.
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTUnInitSPI(void)
{
  UxCSR = 0;
  URXxIE = 0;
  IEN2 &= ~UTXxIE;  
  UTXxIF = 1;
}

/*****************************************************************************
 * @fn      HalUARTReadSPI
 *
 * @brief   Execute a blocking read from master if Rx buffer is empty. Otherwise return up to 'len'
 *          bytes of read data.
 *
 * @param   buf  - valid data buffer at least 'len' bytes in size
 *          len  - max length number of bytes to copy to 'buf'
 *
 * @return  length of buffer that was read
 *****************************************************************************/
uint16 HalUARTReadSPI(uint8 *buf, uint16 len)
{
  uint16 cnt = 0;

  if ((spiCfg.rxHead == 0) && (spiCfg.rxHead == spiCfg.rxTail))
  {
    uint8 tmp;

    UxDBUF = 0;
    URXxIF = 0;
    // Slave signals ready for read by setting its ready flag first.
    SRDY_SET();
    while (!MRDY_SET);

    do {
      while (!URXxIF && MRDY_SET);
      tmp = UxDBUF;
      URXxIF = 0;

      // Master ends slave read by clearing its ready flag first.
      if (!MRDY_SET)
      {
        break;
      }

      spiCfg.rxBuf[spiCfg.rxTail] = tmp;
      if (++spiCfg.rxTail >= HAL_UART_SPI_RX_MAX)
      {
        spiCfg.rxTail = 0;
      }
    } while(1);

    // Master blocks waiting for slave to clear its ready flag before continuing.
    SRDY_CLR();
  }

  while ((spiCfg.rxHead != spiCfg.rxTail) && (cnt < len))
  {
    *buf++ = spiCfg.rxBuf[spiCfg.rxHead++];
    if (spiCfg.rxHead >= HAL_UART_SPI_RX_MAX)
    {
      spiCfg.rxHead = 0;
    }
    cnt++;
  }

  if (cnt == 0)
  {
    spiCfg.rxHead = spiCfg.rxTail = 0;
  }

  return cnt;
}

/******************************************************************************
 * @fn      HalUARTWriteSPI
 *
 * @brief   Write a buffer to the UART.
 * @brief   Execute a blocking write to the master.
 *
 * @param   buf - pointer to the buffer that will be written, not freed
 *          len - length of
 *
 * @return  length of the buffer that was sent
 *****************************************************************************/
uint16 HalUARTWriteSPI(uint8 *buf, uint16 len)
{
  uint16 cnt;

  // Enforce all or none and at least one.
  if ((HAL_UART_SPI_TX_AVAIL() < len) || (len == 0))
  {
    return 0;
  }

  UxDBUF = *buf++;
  for (cnt = 1; cnt < len; cnt++)
  {
    spiCfg.txBuf[spiCfg.txTail] = *buf++;

    if (spiCfg.txTail >= HAL_UART_SPI_TX_MAX-1)
    {
      spiCfg.txTail = 0;
    }
    else
    {
      spiCfg.txTail++;
    }
  }

  URXxIF = 0;
  // Master signals ready for slave write by setting its ready flag first.
  while (!MRDY_SET);
  // Master blocks waiting for slave to set its ready flag before continuing.
  SRDY_SET();

  do {
    while (!URXxIF && MRDY_SET);
    UxDBUF = spiCfg.txBuf[spiCfg.txHead];
    URXxIF = 0;

    if (spiCfg.txHead == spiCfg.txTail)
    {
      spiCfg.txBuf[spiCfg.txHead] = 0;
    }
    else if (++spiCfg.txHead >= HAL_UART_SPI_TX_MAX)
    {
      spiCfg.txHead = 0;
    }
  } while(MRDY_SET);  // Master ends slave write by clearing its ready flag first.

  // Master blocks waiting for slave to set its ready flag before continuing.
  SRDY_CLR();

  return cnt;
}

/******************************************************************************
******************************************************************************/
