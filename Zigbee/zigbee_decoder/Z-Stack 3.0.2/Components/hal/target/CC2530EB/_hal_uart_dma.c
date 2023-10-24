/**************************************************************************************************
  Filename:       _hal_uart_dma.c
  Revised:        $Date: 2010-04-01 18:14:33 -0700 (Thu, 01 Apr 2010) $
  Revision:       $Revision: 22068 $

  Description: This file contains the interface to the H/W UART driver by DMA.


  Copyright 2006-2010 Texas Instruments Incorporated. All rights reserved.

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
#include "hal_assert.h"
#include "hal_board.h"
#include "hal_defs.h"
#include "hal_dma.h"
#include "hal_mcu.h"
#include "hal_uart.h"
#if defined MT_TASK
#include "mt_uart.h"
#endif
#include "osal.h"

/*********************************************************************
 * MACROS
 */

//#define HAL_UART_ASSERT(expr)        HAL_ASSERT((expr))
#define HAL_UART_ASSERT(expr)

#if defined HAL_BOARD_CC2430EB || defined HAL_BOARD_CC2430DB || defined HAL_BOARD_CC2430BB
#define HAL_UART_DMA_NEW_RX_BYTE(IDX)  (DMA_PAD == LO_UINT16(dmaCfg.rxBuf[(IDX)]))
#define HAL_UART_DMA_GET_RX_BYTE(IDX)  (HI_UINT16(dmaCfg.rxBuf[(IDX)]))
#define HAL_UART_DMA_CLR_RX_BYTE(IDX)  (dmaCfg.rxBuf[(IDX)] = BUILD_UINT16((DMA_PAD ^ 0xFF), 0))
#else
#define HAL_UART_DMA_NEW_RX_BYTE(IDX)  (DMA_PAD == HI_UINT16(dmaCfg.rxBuf[(IDX)]))
#define HAL_UART_DMA_GET_RX_BYTE(IDX)  (LO_UINT16(dmaCfg.rxBuf[(IDX)]))
#define HAL_UART_DMA_CLR_RX_BYTE(IDX)  (dmaCfg.rxBuf[(IDX)] = BUILD_UINT16(0, (DMA_PAD ^ 0xFF)))
#endif

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
#if (HAL_UART_DMA == 1)
#define PxOUT                      P0
#define PxIN                       P0
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
#define PxIN                       P1
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

#if (HAL_UART_DMA == 1)
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
#define HAL_UART_DMA_TX_MAX        MT_UART_DEFAULT_MAX_TX_BUFF
#define HAL_UART_DMA_RX_MAX        MT_UART_DEFAULT_MAX_RX_BUFF
#define HAL_UART_DMA_HIGH          MT_UART_DEFAULT_THRESHOLD
#define HAL_UART_DMA_IDLE         (MT_UART_DEFAULT_IDLE_TIMEOUT * HAL_UART_MSECS_TO_TICKS)
#else
#if !defined HAL_UART_DMA_RX_MAX
#define HAL_UART_DMA_RX_MAX        256
#endif
#if !defined HAL_UART_DMA_TX_MAX
#define HAL_UART_DMA_TX_MAX        HAL_UART_DMA_RX_MAX
#endif
#if !defined HAL_UART_DMA_HIGH
#define HAL_UART_DMA_HIGH         (HAL_UART_DMA_RX_MAX / 2 - 16)
#endif
#if !defined HAL_UART_DMA_IDLE
#define HAL_UART_DMA_IDLE         (1 * HAL_UART_MSECS_TO_TICKS)
#endif
#endif
#if !defined HAL_UART_DMA_FULL
#define HAL_UART_DMA_FULL         (HAL_UART_DMA_RX_MAX - 16)
#endif

#if defined HAL_BOARD_CC2430EB || defined HAL_BOARD_CC2430DB || defined HAL_BOARD_CC2430BB
#define HAL_DMA_U0DBUF             0xDFC1
#define HAL_DMA_U1DBUF             0xDFF9
#else  /* CC2530 */
#define HAL_DMA_U0DBUF             0x70C1
#define HAL_DMA_U1DBUF             0x70F9
#endif

#if (HAL_UART_DMA == 1)
#define DMATRIG_RX                 HAL_DMA_TRIG_URX0
#define DMATRIG_TX                 HAL_DMA_TRIG_UTX0
#define DMA_UDBUF                  HAL_DMA_U0DBUF
#define DMA_PAD                    U0BAUD
#else
#define DMATRIG_RX                 HAL_DMA_TRIG_URX1
#define DMATRIG_TX                 HAL_DMA_TRIG_UTX1
#define DMA_UDBUF                  HAL_DMA_U1DBUF
#define DMA_PAD                    U1BAUD
#endif

/*********************************************************************
 * TYPEDEFS
 */

#if HAL_UART_DMA_RX_MAX <= 256
typedef uint8 rxIdx_t;
#else
typedef uint16 rxIdx_t;
#endif

#if HAL_UART_DMA_TX_MAX <= 256
typedef uint8 txIdx_t;
#else
typedef uint16 txIdx_t;
#endif

typedef struct
{
  uint16 rxBuf[HAL_UART_DMA_RX_MAX];
  rxIdx_t rxHead;
  rxIdx_t rxTail;
  uint8 rxTick;
  uint8 rxShdw;

  uint8 txBuf[2][HAL_UART_DMA_TX_MAX];
  txIdx_t txIdx[2];
  volatile uint8 txSel;
  uint8 txMT;
  uint8 txTick;           // 1-character time in 32kHz ticks according to baud rate,
                          // to be used in calculating time lapse since DMA ISR
                          // to allow delay margin before start firing DMA, so that
                          // DMA does not overwrite UART DBUF of previous packet
  
  volatile uint8 txShdw;  // Sleep Timer LSB shadow.
  volatile uint8 txShdwValid; // TX shadow value is valid
  uint8 txDMAPending;     // UART TX DMA is pending

  halUARTCBack_t uartCB;
} uartDMACfg_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

void HalUARTIsrDMA(void);

/*********************************************************************
 * LOCAL VARIABLES
 */

static uartDMACfg_t dmaCfg;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static rxIdx_t findTail(void);

// Invoked by functions in hal_uart.c when this file is included.
static void HalUARTInitDMA(void);
static void HalUARTOpenDMA(halUARTCfg_t *config);
static uint16 HalUARTReadDMA(uint8 *buf, uint16 len);
static uint16 HalUARTWriteDMA(uint8 *buf, uint16 len);
static void HalUARTPollDMA(void);
static uint16 HalUARTRxAvailDMA(void);
static void HalUARTSuspendDMA(void);
static void HalUARTResumeDMA(void);

/*****************************************************************************
 * @fn      findTail
 *
 * @brief   Find the rxBuf index where the DMA RX engine is working.
 *
 * @param   None.
 *
 * @return  Index of tail of rxBuf.
 *****************************************************************************/
static rxIdx_t findTail(void)
{
  rxIdx_t idx = dmaCfg.rxHead;

  do
  {
    if (!HAL_UART_DMA_NEW_RX_BYTE(idx))
    {
      break;
    }

#if HAL_UART_DMA_RX_MAX == 256
    idx++;
#else
    if (++idx >= HAL_UART_DMA_RX_MAX)
    {
      idx = 0;
    }
#endif
  } while (idx != dmaCfg.rxHead);

  return idx;
}

/******************************************************************************
 * @fn      HalUARTInitDMA
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTInitDMA(void)
{
  halDMADesc_t *ch;

  P2DIR &= ~P2DIR_PRIPO;
  P2DIR |= HAL_UART_PRIPO;

#if (HAL_UART_DMA == 1)
  PERCFG &= ~HAL_UART_PERCFG_BIT;    // Set UART0 I/O to Alt. 1 location on P0.
#else
  PERCFG |= HAL_UART_PERCFG_BIT;     // Set UART1 I/O to Alt. 2 location on P1.
#endif
  PxSEL  |= HAL_UART_Px_RX_TX;       // Enable Tx and Rx on P1.
  ADCCFG &= ~HAL_UART_Px_RX_TX;      // Make sure ADC doesnt use this.
  UxCSR = CSR_MODE;                  // Mode is UART Mode.
  UxUCR = UCR_FLUSH;                 // Flush it.

  // Setup Tx by DMA.
  ch = HAL_DMA_GET_DESC1234( HAL_DMA_CH_TX );

  // The start address of the destination.
  HAL_DMA_SET_DEST( ch, DMA_UDBUF );

  // Using the length field to determine how many bytes to transfer.
  HAL_DMA_SET_VLEN( ch, HAL_DMA_VLEN_USE_LEN );

  // One byte is transferred each time.
  HAL_DMA_SET_WORD_SIZE( ch, HAL_DMA_WORDSIZE_BYTE );

  // The bytes are transferred 1-by-1 on Tx Complete trigger.
  HAL_DMA_SET_TRIG_MODE( ch, HAL_DMA_TMODE_SINGLE );
  HAL_DMA_SET_TRIG_SRC( ch, DMATRIG_TX );

  // The source address is incremented by 1 byte after each transfer.
  HAL_DMA_SET_SRC_INC( ch, HAL_DMA_SRCINC_1 );

  // The destination address is constant - the Tx Data Buffer.
  HAL_DMA_SET_DST_INC( ch, HAL_DMA_DSTINC_0 );

  // The DMA Tx done is serviced by ISR in order to maintain full thruput.
  HAL_DMA_SET_IRQ( ch, HAL_DMA_IRQMASK_ENABLE );

  // Xfer all 8 bits of a byte xfer.
  HAL_DMA_SET_M8( ch, HAL_DMA_M8_USE_8_BITS );

  // DMA has highest priority for memory access.
  HAL_DMA_SET_PRIORITY( ch, HAL_DMA_PRI_HIGH );

  // Setup Rx by DMA.
  ch = HAL_DMA_GET_DESC1234( HAL_DMA_CH_RX );

  // The start address of the source.
  HAL_DMA_SET_SOURCE( ch, DMA_UDBUF );

  // Using the length field to determine how many bytes to transfer.
  HAL_DMA_SET_VLEN( ch, HAL_DMA_VLEN_USE_LEN );

  /* The trick is to cfg DMA to xfer 2 bytes for every 1 byte of Rx.
   * The byte after the Rx Data Buffer is the Baud Cfg Register,
   * which always has a known value. So init Rx buffer to inverse of that
   * known value. DMA word xfer will flip the bytes, so every valid Rx byte
   * in the Rx buffer will be preceded by a DMA_PAD char equal to the
   * Baud Cfg Register value.
   */
  HAL_DMA_SET_WORD_SIZE( ch, HAL_DMA_WORDSIZE_WORD );

  // The bytes are transferred 1-by-1 on Rx Complete trigger.
  HAL_DMA_SET_TRIG_MODE( ch, HAL_DMA_TMODE_SINGLE_REPEATED );
  HAL_DMA_SET_TRIG_SRC( ch, DMATRIG_RX );

  // The source address is constant - the Rx Data Buffer.
  HAL_DMA_SET_SRC_INC( ch, HAL_DMA_SRCINC_0 );

  // The destination address is incremented by 1 word after each transfer.
  HAL_DMA_SET_DST_INC( ch, HAL_DMA_DSTINC_1 );
  HAL_DMA_SET_DEST( ch, dmaCfg.rxBuf );
  HAL_DMA_SET_LEN( ch, HAL_UART_DMA_RX_MAX );

  // The DMA is to be polled and shall not issue an IRQ upon completion.
  HAL_DMA_SET_IRQ( ch, HAL_DMA_IRQMASK_DISABLE );

  // Xfer all 8 bits of a byte xfer.
  HAL_DMA_SET_M8( ch, HAL_DMA_M8_USE_8_BITS );

  // DMA has highest priority for memory access.
  HAL_DMA_SET_PRIORITY( ch, HAL_DMA_PRI_HIGH );
}

/******************************************************************************
 * @fn      HalUARTOpenDMA
 *
 * @brief   Open a port according tp the configuration specified by parameter.
 *
 * @param   config - contains configuration information
 *
 * @return  none
 *****************************************************************************/
static void HalUARTOpenDMA(halUARTCfg_t *config)
{
  dmaCfg.uartCB = config->callBackFunc;
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
      dmaCfg.txTick = 35; // (32768Hz / (9600bps / 10 bits))
                          // 10 bits include start and stop bits.
      break;
    case HAL_UART_BR_19200:
      UxGCR = 9;
      dmaCfg.txTick = 18;
      break;
    case HAL_UART_BR_38400:
      UxGCR = 10;
      dmaCfg.txTick = 9;
      break;
    case HAL_UART_BR_57600:
      UxGCR = 10;
      dmaCfg.txTick = 6;
      break;
    default:
      // HAL_UART_BR_115200
      UxGCR = 11;
      dmaCfg.txTick = 3;
      break;
  }

  // 8 bits/char; no parity; 1 stop bit; stop bit hi.
  if (config->flowControl)
  {
    UxUCR = UCR_FLOW | UCR_STOP;
    PxSEL |= HAL_UART_Px_CTS;
    // DMA Rx is always on (self-resetting). So flow must be controlled by the S/W polling the Rx
    // buffer level. Start by allowing flow.
    PxOUT &= ~HAL_UART_Px_RTS;
    PxDIR |=  HAL_UART_Px_RTS;
  }
  else
  {
    UxUCR = UCR_STOP;
  }

  dmaCfg.rxBuf[0] = *(volatile uint8 *)DMA_UDBUF;  // Clear the DMA Rx trigger.
  HAL_DMA_CLEAR_IRQ(HAL_DMA_CH_RX);
  HAL_DMA_ARM_CH(HAL_DMA_CH_RX);
  osal_memset(dmaCfg.rxBuf, (DMA_PAD ^ 0xFF), HAL_UART_DMA_RX_MAX*2);

  UxCSR |= CSR_RE;
  
  // Initialize that TX DMA is not pending
  dmaCfg.txDMAPending = FALSE;
  dmaCfg.txShdwValid = FALSE;
}

/*****************************************************************************
 * @fn      HalUARTReadDMA
 *
 * @brief   Read a buffer from the UART
 *
 * @param   buf  - valid data buffer at least 'len' bytes in size
 *          len  - max length number of bytes to copy to 'buf'
 *
 * @return  length of buffer that was read
 *****************************************************************************/
static uint16 HalUARTReadDMA(uint8 *buf, uint16 len)
{
  uint16 cnt;

  for (cnt = 0; cnt < len; cnt++)
  {
    if (!HAL_UART_DMA_NEW_RX_BYTE(dmaCfg.rxHead))
    {
      break;
    }
    *buf++ = HAL_UART_DMA_GET_RX_BYTE(dmaCfg.rxHead);
    HAL_UART_DMA_CLR_RX_BYTE(dmaCfg.rxHead);
#if HAL_UART_DMA_RX_MAX == 256
    (dmaCfg.rxHead)++;
#else
    if (++(dmaCfg.rxHead) >= HAL_UART_DMA_RX_MAX)
    {
      dmaCfg.rxHead = 0;
    }
#endif
  }
  PxOUT &= ~HAL_UART_Px_RTS;  // Re-enable the flow on any read.

  return cnt;
}

/******************************************************************************
 * @fn      HalUARTWriteDMA
 *
 * @brief   Write a buffer to the UART.
 *
 * @param   buf - pointer to the buffer that will be written, not freed
 *          len - length of
 *
 * @return  length of the buffer that was sent
 *****************************************************************************/
static uint16 HalUARTWriteDMA(uint8 *buf, uint16 len)
{
  uint16 cnt;
  halIntState_t his;
  uint8 txSel;
  txIdx_t txIdx;

  // Enforce all or none.
  if ((len + dmaCfg.txIdx[dmaCfg.txSel]) > HAL_UART_DMA_TX_MAX)
  {
    return 0;
  }

  HAL_ENTER_CRITICAL_SECTION(his);
  txSel = dmaCfg.txSel;
  txIdx = dmaCfg.txIdx[txSel];
  HAL_EXIT_CRITICAL_SECTION(his);

  for (cnt = 0; cnt < len; cnt++)
  {
    dmaCfg.txBuf[txSel][txIdx++] = buf[cnt];
  }

  HAL_ENTER_CRITICAL_SECTION(his);
  if (txSel != dmaCfg.txSel)
  {
    HAL_EXIT_CRITICAL_SECTION(his);
    txSel = dmaCfg.txSel;
    txIdx = dmaCfg.txIdx[txSel];

    for (cnt = 0; cnt < len; cnt++)
    {
      dmaCfg.txBuf[txSel][txIdx++] = buf[cnt];
    }
    HAL_ENTER_CRITICAL_SECTION(his);
  }

  dmaCfg.txIdx[txSel] = txIdx;

  if (dmaCfg.txIdx[(txSel ^ 1)] == 0)
  {
    // TX DMA is expected to be fired
    dmaCfg.txDMAPending = TRUE;
  }
  HAL_EXIT_CRITICAL_SECTION(his);
  return cnt;
}

/******************************************************************************
 * @fn      HalUARTPollDMA
 *
 * @brief   Poll a USART module implemented by DMA.
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
static void HalUARTPollDMA(void)
{
  uint16 cnt = 0;
  uint8 evt = 0;

  if (HAL_UART_DMA_NEW_RX_BYTE(dmaCfg.rxHead))
  {
    rxIdx_t tail = findTail();

    // If the DMA has transferred in more Rx bytes, reset the Rx idle timer.
    if (dmaCfg.rxTail != tail)
    {
      dmaCfg.rxTail = tail;

      // Re-sync the shadow on any 1st byte(s) received.
      if (dmaCfg.rxTick == 0)
      {
        dmaCfg.rxShdw = ST0;
      }
      dmaCfg.rxTick = HAL_UART_DMA_IDLE;
    }
    else if (dmaCfg.rxTick)
    {
      // Use the LSB of the sleep timer (ST0 must be read first anyway).
      uint8 decr = ST0 - dmaCfg.rxShdw;

      if (dmaCfg.rxTick > decr)
      {
        dmaCfg.rxTick -= decr;
        dmaCfg.rxShdw = ST0;
      }
      else
      {
        dmaCfg.rxTick = 0;
      }
    }
    cnt = HalUARTRxAvailDMA();
  }
  else
  {
    dmaCfg.rxTick = 0;
  }

  if (cnt >= HAL_UART_DMA_FULL)
  {
    evt = HAL_UART_RX_FULL;
  }
  else if (cnt >= HAL_UART_DMA_HIGH)
  {
    evt = HAL_UART_RX_ABOUT_FULL;
    PxOUT |= HAL_UART_Px_RTS;  // Disable Rx flow.
  }
  else if (cnt && !dmaCfg.rxTick)
  {
    evt = HAL_UART_RX_TIMEOUT;
  }

  if (dmaCfg.txMT)
  {
    dmaCfg.txMT = FALSE;
    evt |= HAL_UART_TX_EMPTY;
  }

  if (dmaCfg.txShdwValid)
  {
    uint8 decr = ST0;
    decr -= dmaCfg.txShdw;
    if (decr > dmaCfg.txTick)
    {
      // No protection for txShdwValid is required
      // because while the shadow was valid, DMA ISR cannot be triggered
      // to cause concurrent access to this variable.
      dmaCfg.txShdwValid = FALSE;
    }
  }
  
  if (dmaCfg.txDMAPending && !dmaCfg.txShdwValid)
  {
    // UART TX DMA is expected to be fired and enough time has lapsed since last DMA ISR
    // to know that DBUF can be overwritten
    halDMADesc_t *ch = HAL_DMA_GET_DESC1234(HAL_DMA_CH_TX);
    halIntState_t intState;

    // Clear the DMA pending flag
    dmaCfg.txDMAPending = FALSE;
    
    HAL_DMA_SET_SOURCE(ch, dmaCfg.txBuf[dmaCfg.txSel]);
    HAL_DMA_SET_LEN(ch, dmaCfg.txIdx[dmaCfg.txSel]);
    dmaCfg.txSel ^= 1;
    HAL_ENTER_CRITICAL_SECTION(intState);
    HAL_DMA_ARM_CH(HAL_DMA_CH_TX);
    do
    {
      asm("NOP");
    } while (!HAL_DMA_CH_ARMED(HAL_DMA_CH_TX));
    HAL_DMA_CLEAR_IRQ(HAL_DMA_CH_TX);
    HAL_DMA_MAN_TRIGGER(HAL_DMA_CH_TX);
    HAL_EXIT_CRITICAL_SECTION(intState);
  }
  else
  {
    halIntState_t his;

    HAL_ENTER_CRITICAL_SECTION(his);
    if ((dmaCfg.txIdx[dmaCfg.txSel] != 0) && !HAL_DMA_CH_ARMED(HAL_DMA_CH_TX)
                                          && !HAL_DMA_CHECK_IRQ(HAL_DMA_CH_TX))
    {
      HAL_EXIT_CRITICAL_SECTION(his);
      HalUARTIsrDMA();
    }
    else
    {
      HAL_EXIT_CRITICAL_SECTION(his);
    }
  }

  if (evt && (dmaCfg.uartCB != NULL))
  {
    dmaCfg.uartCB(HAL_UART_DMA-1, evt);
  }
}

/**************************************************************************************************
 * @fn      HalUARTRxAvailDMA()
 *
 * @brief   Calculate Rx Buffer length - the number of bytes in the buffer.
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 **************************************************************************************************/
static uint16 HalUARTRxAvailDMA(void)
{
  uint16 cnt = 0;

  if (HAL_UART_DMA_NEW_RX_BYTE(dmaCfg.rxHead))
  {
    uint16 idx;

    for (idx = 0; idx < HAL_UART_DMA_RX_MAX; idx++)
    {
      if (HAL_UART_DMA_NEW_RX_BYTE(idx))
      {
        cnt++;
      }
    }
  }

  return cnt;
}

/******************************************************************************
 * @fn      HalUARTSuspendDMA
 *
 * @brief   Suspend UART hardware before entering PM mode 1, 2 or 3.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void HalUARTSuspendDMA( void )
{
  PxOUT |= HAL_UART_Px_RTS;  // Disable Rx flow.
  UxCSR &= ~CSR_RE;
  P0IEN |=  HAL_UART_Px_CTS;  // Enable the CTS ISR.
}

/******************************************************************************
 * @fn      HalUARTResumeDMA
 *
 * @brief   Resume UART hardware after exiting PM mode 1, 2 or 3.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void HalUARTResumeDMA( void )
{
  P0IEN &= ~HAL_UART_Px_CTS;  // Disable the CTS ISR.
  UxUCR |= UCR_FLUSH;
  UxCSR |= CSR_RE;
  PxOUT &= ~HAL_UART_Px_RTS;  // Re-enable Rx flow.
}

/******************************************************************************
 * @fn      HalUARTIsrDMA
 *
 * @brief   Handle the Tx done DMA ISR.
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
void HalUARTIsrDMA(void);
void HalUARTIsrDMA(void)
{
  HAL_DMA_CLEAR_IRQ(HAL_DMA_CH_TX);

  // Indicate that the other buffer is free now.
  dmaCfg.txIdx[(dmaCfg.txSel ^ 1)] = 0;
  dmaCfg.txMT = TRUE;
  
  // Set TX shadow
  dmaCfg.txShdw = ST0;
  dmaCfg.txShdwValid = TRUE;

  // If there is more Tx data ready to go, re-start the DMA immediately on it.
  if (dmaCfg.txIdx[dmaCfg.txSel])
  {
    // UART TX DMA is expected to be fired
    dmaCfg.txDMAPending = TRUE;
  }
}

/******************************************************************************
******************************************************************************/
