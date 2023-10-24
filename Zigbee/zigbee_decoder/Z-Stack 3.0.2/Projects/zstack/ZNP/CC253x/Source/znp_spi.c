/**************************************************************************************************
  Filename:       znp_spi.c
  Revised:        $Date: 2010-07-28 18:42:54 -0700 (Wed, 28 Jul 2010) $
  Revision:       $Revision: 23203 $

  Description:

  This file contains the interface to the H/W-specific ZNP SPI driver.


  Copyright 2009-2010 Texas Instruments Incorporated. All rights reserved.

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

#include "comdef.h"
#include "hal_assert.h"
#include "hal_dma.h"
#include "MT_RPC.h"
#include "OnBoard.h"
#include "osal.h"
#if defined POWER_SAVING
#include "OSAL_PwrMgr.h"
#endif        
#include "ZDApp.h"
#include "znp_app.h"
#include "znp_spi.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  The MAC_ASSERT macro is for use during debugging.
 *  The given expression must evaluate as "true" or else fatal error occurs.
 *  At that point, the call stack feature of the debugger can pinpoint where the problem occurred.
 *
 *  To disable this feature and save code size, the project should define NP_SPI_NODEBUG to TRUE.
 */

#if !defined ( NP_SPI_NODEBUG )
  #define NP_SPI_NODEBUG              TRUE
#endif

#if ( NP_SPI_NODEBUG )
  #define NP_SPI_ASSERT( expr )
#else
  #define NP_SPI_ASSERT( expr)        HAL_ASSERT( expr )
#endif

#if defined CC2530_MK
#define DMATRIG_RX  HAL_DMA_TRIG_URX0
#define DMATRIG_TX  HAL_DMA_TRIG_UTX0
#define DMA_UDBUF   NP_SPI_U0DBUF
#else
#define DMATRIG_RX  HAL_DMA_TRIG_URX1
#define DMATRIG_TX  HAL_DMA_TRIG_UTX1
#define DMA_UDBUF   NP_SPI_U1DBUF
#endif

#define DMA_RX() \
  st( \
    volatile uint8 clearRx = *((uint8 *)DMA_UDBUF); \
    \
    HAL_DMA_CLEAR_IRQ(HAL_DMA_CH_RX); \
    \
    HAL_DMA_ARM_CH(HAL_DMA_CH_RX); \
  )

#define DMA_TX( buf ) \
  st( \
    halDMADesc_t *ch = HAL_DMA_GET_DESC1234(HAL_DMA_CH_TX); \
    \
    HAL_DMA_SET_SOURCE(ch, (buf)); \
    \
    HAL_DMA_CLEAR_IRQ(HAL_DMA_CH_TX); \
    \
    HAL_DMA_ARM_CH(HAL_DMA_CH_TX); \
    \
    HAL_DMA_START_CH(HAL_DMA_CH_TX); \
  )

#define HAL_DMA_GET_SOURCE( pDesc, src ) \
  st( \
    src = (uint16)(pDesc->srcAddrH) << 8; \
    src += pDesc->srcAddrL; \
  )

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

#define NP_SPI_U0DBUF  0x70C1
#define NP_SPI_U1DBUF  0x70F9

/* UxCSR - USART Control and Status Register. */
#define CSR_MODE       0x80
#define CSR_RE         0x40
#define CSR_SLAVE      0x20
#define CSR_FE         0x10
#define CSR_ERR        0x08
#define CSR_RX_BYTE    0x04
#define CSR_TX_BYTE    0x02
#define CSR_ACTIVE     0x01

/* UxUCR - USART UART Control Register. */
#define UCR_FLUSH      0x80
#define UCR_FLOW       0x40
#define UCR_D9         0x20
#define UCR_BIT9       0x10
#define UCR_PARITY     0x08
#define UCR_SPB        0x04
#define UCR_STOP       0x02
#define UCR_START      0x01

#define UTX0IE         0x04
#define UTX1IE         0x08

#define NP_SPI_RX_SREQ_EVENT  ZNP_SPI_RX_SREQ_EVENT
#define NP_SPI_RX_AREQ_EVENT  ZNP_SPI_RX_AREQ_EVENT

/* Reference data sheet section 8.2.4: When using variable-length transfers, then LEN
 * should be set to the largest allowed transfer length plus one.
 */
#define NP_SPI_BUF_LEN  (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + 1)

#if defined CC2530_MK
#define NP_RDYIn_BIT     BV(0)
#define NP_RDYIn         P2_0
#define NP_RDYOut        P1_0
#define NP_RDYOut_BIT    BV(0)

#elif !defined CC2530ZNP_MK

#define NP_RDYIn_BIT     BV(3)
#define NP_RDYIn         P0_3
#define NP_RDYOut        P0_4
#define NP_RDYOut_BIT    BV(4)
#endif

#define NP_CSR_MODE      BV(5)  //  CSR_SLAVE

/* ------------------------------------------------------------------------------------------------
 *                                           TypeDefs
 * ------------------------------------------------------------------------------------------------
 */

typedef enum
{
  NP_SPI_IDLE,           /* Idle, no transaction in progress. */
  NP_SPI_MRDY,           /* Idle, but got MRDY ISR, so waiting to enable the RX DMA. */
  NP_SPI_WAIT_RX,        /* Waiting for RX to complete. */
  NP_SPI_WAIT_TX,        /* Waiting for TX to complete. */
  NP_SPI_WAIT_AREQ,      /* Waiting for asynchronous request to finish processing. */
  NP_SPI_WAIT_SREQ       /* Waiting for a synchronous request to finish processing. */
} spiState_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

static uint8 npSpiBuf[NP_SPI_BUF_LEN];

static volatile spiState_t npSpiState;

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void dmaInit(void);

/**************************************************************************************************
 * @fn          dmaInit
 *
 * @brief       This function initializes the DMA for the SPI driver.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void dmaInit(void)
{
  halDMADesc_t *ch;

  /* Setup Tx by DMA. */
  ch = HAL_DMA_GET_DESC1234(HAL_DMA_CH_TX);

  /* The start address of the source and destination. */
  HAL_DMA_SET_SOURCE(ch, npSpiBuf);
  HAL_DMA_SET_DEST(ch, DMA_UDBUF);

  /* Transfer the first byte + the number of bytes indicated by the first byte + 2 more bytes. */
  HAL_DMA_SET_VLEN(ch, HAL_DMA_VLEN_1_P_VALOFFIRST_P_2);
  HAL_DMA_SET_LEN(ch, NP_SPI_BUF_LEN);

  /* One byte is transferred each time. */
  HAL_DMA_SET_WORD_SIZE(ch, HAL_DMA_WORDSIZE_BYTE);

  /* The bytes are transferred 1-by-1 on Tx Complete trigger. */
  HAL_DMA_SET_TRIG_MODE(ch, HAL_DMA_TMODE_SINGLE);
  HAL_DMA_SET_TRIG_SRC(ch, DMATRIG_TX);

  /* The source address is decremented by 1 byte after each transfer. */
  HAL_DMA_SET_SRC_INC(ch, HAL_DMA_SRCINC_1);

  /* The destination address is constant - the Tx Data Buffer. */
  HAL_DMA_SET_DST_INC(ch, HAL_DMA_DSTINC_0);

  /* The DMA shall issue an IRQ upon completion. */
  HAL_DMA_SET_IRQ(ch, HAL_DMA_IRQMASK_ENABLE);

  /* Xfer all 8 bits of a byte xfer. */
  HAL_DMA_SET_M8(ch, HAL_DMA_M8_USE_8_BITS);

  /* DMA has highest priority for memory access. */
  HAL_DMA_SET_PRIORITY(ch, HAL_DMA_PRI_HIGH);

  /* Setup Rx by DMA. */
  ch = HAL_DMA_GET_DESC1234(HAL_DMA_CH_RX);

  /* The start address of the source and destination. */
  HAL_DMA_SET_SOURCE(ch, DMA_UDBUF);
  HAL_DMA_SET_DEST(ch, npSpiBuf);

  /* Transfer the first byte + the number of bytes indicated by the first byte + 2 more bytes. */
  HAL_DMA_SET_VLEN(ch, HAL_DMA_VLEN_1_P_VALOFFIRST_P_2);
  HAL_DMA_SET_LEN(ch, NP_SPI_BUF_LEN);

  HAL_DMA_SET_WORD_SIZE(ch, HAL_DMA_WORDSIZE_BYTE);

  /* The bytes are transferred 1-by-1 on Rx Complete trigger. */
  HAL_DMA_SET_TRIG_MODE(ch, HAL_DMA_TMODE_SINGLE);
  HAL_DMA_SET_TRIG_SRC(ch, DMATRIG_RX);

  /* The source address is constant - the Rx Data Buffer. */
  HAL_DMA_SET_SRC_INC(ch, HAL_DMA_SRCINC_0);

  /* The destination address is incremented by 1 byte after each transfer. */
  HAL_DMA_SET_DST_INC(ch, HAL_DMA_DSTINC_1);

  /* The DMA shall issue an IRQ upon completion. */
  HAL_DMA_SET_IRQ(ch, HAL_DMA_IRQMASK_ENABLE);

  /* Xfer all 8 bits of a byte xfer. */
  HAL_DMA_SET_M8(ch, HAL_DMA_M8_USE_8_BITS);

  /* DMA has highest priority for memory access. */
  HAL_DMA_SET_PRIORITY(ch, HAL_DMA_PRI_HIGH);
}

/**************************************************************************************************
 * @fn          npSpiInit
 *
 * @brief       This function is called to set up the SPI interface.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
#if defined CC2530_MK
void npSpiInit(void)  // TODO - hard-coded for USART0 alt1 SPI for now.
{
  if (ZNP_CFG1_UART == znpCfg1)
  {
    return;
  }

  /* Set bit order to MSB */
  U0GCR |= BV(5);

  /* Set UART0 I/O to alternate 1 location on P1 pins. */
  //PERCFG |= 0x02;  /* U1CFG */

  /* Mode select UART1 SPI Mode as slave. */
  U0CSR = NP_CSR_MODE;

  /* Select peripheral function on I/O pins. */
  P0SEL |= 0x3C;  /* SELP0_[5:2] */

  /* Give UART1 priority over Timer3. */
  //P2SEL &= ~0x20;  /* PRI2P1 */

  /* Set RDY to inactive high. */
  NP_RDYOut = 1;

  /* Select general purpose on I/O pins. */
  P1SEL &= ~(NP_RDYOut_BIT);  /* P1.0 SRDY - GPIO */
  P2SEL &= ~(NP_RDYIn_BIT);   /* P2.0 MRDY - GPIO */

  /* Select GPIO direction */
  P1DIR |= NP_RDYOut_BIT;  /* P1.0 SRDY - OUT */
  P2DIR &= ~NP_RDYIn_BIT;  /* P2.0 MRDY - IN */

  /* Falling edge on P2 pins triggers interrupt. */
  PICTL |= BV(3);  /* P2ICON */

  /* Trigger an interrupt on MRDY input. */
  P2IFG &= ~NP_RDYIn_BIT;
  P2IEN |=  NP_RDYIn_BIT;
  IEN2 |= 0x02;

  dmaInit();

  U0CSR |= CSR_RE;
}
#else
void npSpiInit(void)
{
  if (ZNP_CFG1_UART == znpCfg1)
  {
    return;
  }

  /* Set bit order to MSB */
  U1GCR |= BV(5);

  /* Set UART1 I/O to alternate 2 location on P1 pins. */
  PERCFG |= 0x02;  /* U1CFG */

  /* Mode select UART1 SPI Mode as slave. */
  U1CSR = NP_CSR_MODE;

  /* Select peripheral function on I/O pins. */
  P1SEL |= 0xF0;  /* SELP1_[7:4] */

  /* Give UART1 priority over Timer3. */
  P2SEL &= ~0x20;  /* PRI2P1 */

  /* Set RDY to inactive high. */
  NP_RDYOut = 1;

  /* Select general purpose on I/O pins. */
  P0SEL &= ~(NP_RDYIn_BIT);   /* P0.3 MRDY - GPIO */
  P0SEL &= ~(NP_RDYOut_BIT);  /* P0.4 SRDY - GPIO */

  /* Select GPIO direction */
  P0DIR &= ~NP_RDYIn_BIT;  /* P0.3 MRDY - IN */
  P0DIR |= NP_RDYOut_BIT;  /* P0.4 SRDY - OUT */

  P0INP &= ~NP_RDYIn_BIT;  /* Pullup/down enable of MRDY input. */
  P2INP &= ~BV(5);         /* Pullup all P0 inputs. */

  /* Falling edge on P0 pins triggers interrupt. */
  PICTL |= BV(0);  /* P0ICON */

  /* Trigger an interrupt on MRDY input. */
  P0IFG &= ~NP_RDYIn_BIT;
  P0IEN |=  NP_RDYIn_BIT;
  P0IE = 1;

  dmaInit();

  U1CSR |= CSR_RE;
}
#endif

/**************************************************************************************************
 * @fn          npSpiMonitor
 *
 * @brief       This function monitors the SPI signals for error conditions and for the end of a
 *              transaction. If an error is detected it attempts to recover.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiMonitor(void)
{
  if (ZNP_CFG1_UART == znpCfg1)
  {
    return;
  }

#if ZNP_RUN_WDOG
  if ((devState != DEV_HOLD) && (npSpiState == NP_SPI_IDLE))
  {
    // Enable the watchdog for 1 second period and pet it.
    WatchDogEnable( WDTIMX );
  }
#endif

  switch (npSpiState)
  {
  case NP_SPI_IDLE:
    NP_SPI_ASSERT((P1IFG & NP_RDYIn_BIT) == 0);
    break;

#if defined POWER_SAVING
  case NP_SPI_MRDY:
    npSpiState = NP_SPI_WAIT_RX;
    DMA_RX();
    NP_RDYOut = 0;
    break;
#endif        

  case NP_SPI_WAIT_RX:
    NP_SPI_ASSERT((HAL_DMA_CHECK_IRQ(HAL_DMA_CH_RX)) == 0);
    break;

  case NP_SPI_WAIT_TX:
    NP_SPI_ASSERT((HAL_DMA_CHECK_IRQ(HAL_DMA_CH_TX)) == 0);
    break;

  case NP_SPI_WAIT_AREQ:
    break;

  default:
    NP_SPI_ASSERT(0);
    break;
  }

  if (npSpiState == NP_SPI_IDLE)
  {
    *((uint8 *)DMA_UDBUF) = 0;  // Clear the SPI Tx buffer to zero.

    /* Poll for MRDY in case it was set before slave had setup the ISR.
     * Also, async responses may get queued, so flush them out here.
     */
    if ((NP_RDYIn == 0) || (npSpiReadyCallback()))
    {
      npSpiAReqReady();
    }
  }
  else
  {
    halIntState_t his;

    HAL_ENTER_CRITICAL_SECTION(his);
    if (((npSpiState == NP_SPI_WAIT_RX) &&
        (!HAL_DMA_CH_ARMED(HAL_DMA_CH_RX) && !HAL_DMA_CHECK_IRQ(HAL_DMA_CH_RX)))
    ||  ((npSpiState == NP_SPI_WAIT_TX) &&
        (!HAL_DMA_CH_ARMED(HAL_DMA_CH_TX) && !HAL_DMA_CHECK_IRQ(HAL_DMA_CH_TX))))
    {
      HAL_EXIT_CRITICAL_SECTION(his);

      if (npSpiState == NP_SPI_WAIT_RX)
      {
        npSpiRxIsr();
      }
      else  // if (npSpiState == NP_SPI_WAIT_TX)
      {
        npSpiTxIsr();
      }
    }
    else
    {
      HAL_EXIT_CRITICAL_SECTION(his);
    }
  }

#if defined POWER_SAVING
  /* A simple ZAP application sending a unicast at 2-Hz was seen to bog down to < 1-Hz OTA unicast
   * when the ZNP was configured to be a ZED (i.e. POWER_SAVING was enabled). So adding this delay
   * of only 10 msecs before re-enabling CONSERVE showed that the problem was fixed while still
   * allowing the ZNP to enter sleep.
   */
  static uint8 znpSpiActiveShdw;

  if (ZG_DEVICE_ENDDEVICE_TYPE && (npSpiState == NP_SPI_IDLE))
  {
    if (znpSpiActiveShdw)
    {
      uint8 rxOnIdle;
      (void)ZMacGetReq(ZMacRxOnIdle, &rxOnIdle);
      if (!rxOnIdle)
      {
        znpSpiActiveShdw = FALSE;
        if (ZSuccess != osal_start_timerEx(znpTaskId, ZNP_PWRMGR_CONSERVE_EVENT,
                                                      ZNP_PWRMGR_CONSERVE_DELAY))
        {
          (void)osal_set_event(znpTaskId, ZNP_PWRMGR_CONSERVE_EVENT);
        }
      }
    }
  }
  else if (!znpSpiActiveShdw)
  {
    znpSpiActiveShdw = TRUE;
    (void)osal_stop_timerEx(znpTaskId, ZNP_PWRMGR_CONSERVE_EVENT);
    (void)osal_clear_event(znpTaskId, ZNP_PWRMGR_CONSERVE_EVENT);
    (void)osal_pwrmgr_task_state(znpTaskId, PWRMGR_HOLD);
  }
#endif
}

/**************************************************************************************************
 * @fn          npSpiSRspAlloc
 *
 * @brief       This function is called by MT to allocate a buffer in which to build an SRSP frame.
 *              MT must only call this function after processing a received SREQ frame.
 *
 * input parameters
 *
 * @param       len - Length of the buffer required.
 *
 * output parameters
 *
 * None.
 *
 * @return      NULL for failure; a pointer to the npSpiBuf on success. Success is determined by
 *              the correct npSpiState and H/W signals as well as a valid length request.
 **************************************************************************************************
 */
uint8 *npSpiSRspAlloc(uint8 len)
{
  if (npSpiState == NP_SPI_WAIT_SREQ)
  {
    return npSpiBuf;
  }
  else
  {
    return NULL;
  }
}

/**************************************************************************************************
 * @fn          npSpiAReqAlloc
 *
 * @brief       This function is called by MT to allocate a buffer in which to buld an AREQ frame.
 *
 * input parameters
 *
 * @param       len - Length of the buffer required.
 *
 * output parameters
 *
 * None.
 *
 * @return      NULL for failure; otherwise a pointer to the data of an osal message.
 **************************************************************************************************
 */
uint8 *npSpiAReqAlloc(uint8 len)
{
  return osal_msg_allocate(len + MT_RPC_FRAME_HDR_SZ);
}

/**************************************************************************************************
 * @fn          npSpiSRspReady
 *
 * @brief       This function is called by MT to notify SPI driver that an SRSP is ready to Tx.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the buffer to transmit on the SPI.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiSRspReady(uint8 *pBuf)
{
  if ((npSpiState == NP_SPI_WAIT_SREQ) && (NP_RDYOut == 0))
  {
    npSpiState = NP_SPI_WAIT_TX;
    DMA_TX( pBuf );
    NP_RDYOut = 1;
  }
}

/**************************************************************************************************
 * @fn          npSpiAReqReady
 *
 * @brief       This function is called by MT to notify the SPI driver that an AREQ frame is ready
 *              to be transmitted.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiAReqReady(void)
{
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION(intState);

  if (npSpiState == NP_SPI_IDLE)
  {
    npSpiState = NP_SPI_WAIT_RX;
    DMA_RX();
    NP_RDYOut = 0;
  }

  HAL_EXIT_CRITICAL_SECTION(intState);
}

/**************************************************************************************************
 * @fn          npSpiAReqComplete
 *
 * @brief       This function is called by MT to notify the SPI driver that the processing of a
 *              received AREQ is complete.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiAReqComplete(void)
{
  if (npSpiState == NP_SPI_WAIT_AREQ)
  {
    npSpiState = NP_SPI_IDLE;
  }
}

/**************************************************************************************************
 * @fn          npSpiGetReqBuf
 *
 * @brief       This function is called by the application to get the buffer containing the
 *              currently received AREQ or SREQ.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to the buffer containing the currently received AREQ or SREQ.
 **************************************************************************************************
 */
uint8 *npSpiGetReqBuf(void)
{
  if (npSpiState != NP_SPI_IDLE)
  {
    return npSpiBuf;
  }
  else
  {
    return NULL;
  }
}

/**************************************************************************************************
 * @fn          npSpiMrdyIsr
 *
 * @brief       This function is called when a GPIO falling-edge interrupt occurs on the MRDY.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiMrdyIsr(void)
{
  if (npSpiState == NP_SPI_IDLE)
  {
#if defined POWER_SAVING
    if (ZG_DEVICE_ENDDEVICE_TYPE)
    {
      npSpiState = NP_SPI_MRDY;
    }
    else
#endif        
    {
      npSpiState = NP_SPI_WAIT_RX;
      DMA_RX();
      NP_RDYOut = 0;
    }
  }
}

/**************************************************************************************************
 * @fn          npSpiRxIsr
 *
 * @brief       This function handles the DMA Rx complete interrupt.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiRxIsr(void)
{
  mtRpcCmdType_t type = (mtRpcCmdType_t)(npSpiBuf[1] & MT_RPC_CMD_TYPE_MASK);
  uint8 *pBuf, rdy = 1;

  NP_SPI_ASSERT(npSpiState == NP_SPI_WAIT_RX);

  switch (type)
  {
  case MT_RPC_CMD_POLL:
    if ( (pBuf = npSpiPollCallback()) == NULL )
    {
      pBuf = npSpiBuf;
      npSpiBuf[0] = 0;
      npSpiBuf[1] = 0;
      npSpiBuf[2] = 0;
    }
    npSpiState = NP_SPI_WAIT_TX;
    DMA_TX(pBuf);
    break;

  case MT_RPC_CMD_SREQ:
    npSpiState = NP_SPI_WAIT_SREQ;
    osal_set_event(znpTaskId, ZNP_SPI_RX_SREQ_EVENT);
    rdy = 0;
    break;

  case MT_RPC_CMD_AREQ:
    npSpiState = NP_SPI_WAIT_AREQ;
    osal_set_event(znpTaskId, ZNP_SPI_RX_AREQ_EVENT);
    break;

  default:
    npSpiState = NP_SPI_IDLE;
    break;
  }
  NP_RDYOut = rdy;
}

/**************************************************************************************************
 * @fn          npSpiTxIsr
 *
 * @brief       This function handles the DMA Tx complete interrupt.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void npSpiTxIsr(void)
{
  halDMADesc_t *ch = HAL_DMA_GET_DESC1234(HAL_DMA_CH_TX);
  uint16 src;

  NP_SPI_ASSERT(npSpiState == NP_SPI_WAIT_TX);

  HAL_DMA_GET_SOURCE( ch, src );

  if ((uint8 *)src != npSpiBuf)
  {
    osal_msg_deallocate((uint8 *)src);
  }

  npSpiState = NP_SPI_IDLE;
}

/**************************************************************************************************
 * @fn          npSpiIdle
 *
 * @brief       This function returns true if SPI is idle and there is no queued data.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      True if SPI is idle with no quequed data.
 **************************************************************************************************
 */
bool npSpiIdle(void)
{
  return (npSpiState == NP_SPI_IDLE && !npSpiReadyCallback());
}

/**************************************************************************************************
 * @fn          portN-Isr
 *
 * @brief       This function handles the PORT-N interrupt.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
#if defined CC2530_MK
HAL_ISR_FUNCTION(port2Isr, P2INT_VECTOR)
{
  P2IFG = 0;
  P2IF = 0;
#else
HAL_ISR_FUNCTION(port0Isr, P0INT_VECTOR)
{
  P0IFG = 0;
  P0IF = 0;
#endif

  // Knowing which pin requires a #define from _hal_uart_dma.c
  //if (P0IFG & NP_RDYIn_BIT)
  {
    if (ZNP_CFG1_UART == znpCfg1)
    {
      osal_set_event(znpTaskId, ZNP_UART_TX_READY_EVENT);
    }
    else
    {
      npSpiMrdyIsr();
    }
  }
}

/**************************************************************************************************
*/
