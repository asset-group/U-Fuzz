/**************************************************************************************************
Filename:       znp_spi_udma.c
Revised:        $Date: 2013-11-13 11:52:24 -0800 (Wed, 13 Nov 2013) $
Revision:       $Revision: 36072 $

Description:

This file contains the interface to the H/W-specific ZNP SPI driver using uDMA. 
This UDma SPI is only a sample and has not been tested extensively.

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

#include "comdef.h"
#include "hal_assert.h"
#include "MT_RPC.h"
#include "OnBoard.h"
#include "osal.h"
#include "ZDApp.h"
#include "znp_app.h"
#include "znp_spi.h"
#include "hw_ioc.h"
#include "ssi.h"
#include "udma.h"

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

#if !defined ( NP_SPI_NODEBUG )
#define HAL_ENTER_CRITICAL_SECTION_DEBUG(x)    do { (void) (x); } while (0)
#define HAL_EXIT_CRITICAL_SECTION_DEBUG(x)     do { (void) (x); } while (0)
#else
#define HAL_ENTER_CRITICAL_SECTION_DEBUG(x)    HAL_ENTER_CRITICAL_SECTION(x)
#define HAL_EXIT_CRITICAL_SECTION_DEBUG(x)     HAL_EXIT_CRITICAL_SECTION(x) 
#endif

/* ------------------------------------------------------------------------------------------------
*                                           Constants
* ------------------------------------------------------------------------------------------------
*/

/* MRDY Port and Pin */
#define HAL_SPI_SS_BASE            GPIO_A_BASE
#define HAL_SPI_SS_PIN             GPIO_PIN_3

#define HAL_SPI_MRDY_BASE          GPIO_B_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_2

/* SRDY Port and Pin */
#define HAL_SPI_SRDY_BASE          GPIO_D_BASE 
#define HAL_SPI_SRDY_PIN           GPIO_PIN_5 

/* Reference data sheet section 8.2.4: When using variable-length transfers, then LEN
* should be set to the largest allowed transfer length plus one (or 254).
*/
#define NP_SPI_BUF_LEN  (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + 1)

/* SPI data register */
#define SPI_DATA                   (void *)(BSP_SPI_SSI_BASE + SSI_O_DR)

/* SPI DMA masks */
#define UDMA_CH10_SSI0RX_MASK      (1 << UDMA_CH10_SSI0RX)
#define UDMA_CH11_SSI0TX_MASK      (1 << UDMA_CH11_SSI0TX)


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

/* SPI variables */
static uint8 npSpiBuf[NP_SPI_BUF_LEN];
static volatile spiState_t npSpiState;

/* uDMA variables 
 * The application must allocate the channel control table.
 * This one is a full table for all modes and channels.
 * NOTE: This table must be 1024-byte aligned.
 */
#pragma data_alignment=1024
static unsigned char ucDMAControlTable[512];


/* ------------------------------------------------------------------------------------------------
*                                           Local Functions
* ------------------------------------------------------------------------------------------------
*/
static void spi_tx(uint8* buf);
static void npSpiUdmaCompleteIsr(void);
static void npSpiUdmaPrepareRx(void);
static void npSpiUdmaInit(void);


/**************************************************************************************************
* @fn          spi_tx
*
* @brief       This function is called to send an SPI packet
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
static void spi_tx(uint8* buf)
{
  uint8 tx_len = buf[0] + 3;
  
  osal_memcpy(npSpiBuf, buf, tx_len);
  
  /* Disable the RX channel in TX */
  SSIDMADisable(BSP_SPI_SSI_BASE, SSI_DMA_RX);  
  SSIDMAEnable(BSP_SPI_SSI_BASE, SSI_DMA_TX);
  
  /* The transfer buffers and transfer size are now configured using BASIC mode */
  uDMAChannelTransferSet(UDMA_CH11_SSI0TX | UDMA_PRI_SELECT,
                         UDMA_MODE_BASIC, npSpiBuf, SPI_DATA, tx_len);

  /* Pull the SRDY pin high */
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN); 
  
  /* Enable the TX channel. */
  uDMAChannelEnable(UDMA_CH11_SSI0TX);
}


/**************************************************************************************************
* @fn          npSpiUdmaCompleteIsr
*
* @brief       This ISR is vectored when uDMA for TX or RX is completed.
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
static void npSpiUdmaCompleteIsr(void)
{
  halIntState_t intState;
  uint32        intStatus;
  uint8         rx_len; 
  
  intStatus = uDMAIntStatus();
  
  HAL_ENTER_CRITICAL_SECTION_DEBUG(intState);
  if((npSpiState == NP_SPI_WAIT_RX) && (intStatus & UDMA_CH10_SSI0RX_MASK))
  {
    uDMAIntClear(UDMA_CH10_SSI0RX_MASK);
      
    /* The RX length is known at this point */
    rx_len = npSpiBuf[0] + 2;
    
    /* Start another RX DMA for the remaining of the bytes */
    uDMAChannelTransferSet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT,
                           UDMA_MODE_BASIC, SPI_DATA, &npSpiBuf[1], rx_len);
    uDMAChannelEnable(UDMA_CH10_SSI0RX);

    /* Poll for results. TODO: Add error/timeout */    
    while (uDMAChannelSizeGet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT) > 0);
    
    /* Process RX packet */
    npSpiRxIsr();
  } 
  else if((npSpiState == NP_SPI_WAIT_TX) && (intStatus & UDMA_CH11_SSI0TX_MASK))
  {
    uDMAIntClear(UDMA_CH11_SSI0TX_MASK);
  }
  else
  {
    /* TODO: Add debug. Should never get here! */
    uDMAIntClear(UDMA_CH10_SSI0RX_MASK | UDMA_CH11_SSI0TX_MASK);
  }
  HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
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
void npSpiInit(void) 
{
  uint32 ulDummy;
  
  if (ZNP_CFG1_UART == znpCfg1)
  {
    return;
  }
  
  /* Configure SRDY and deassert SRDY */
  GPIOPinTypeGPIOOutput(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN);
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  
  /* Configure MRDY and deassert MRDY */
  GPIOPinTypeGPIOInput(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN);
  GPIOPinWrite(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN, HAL_SPI_MRDY_PIN);
  
  /* Enable SSI peripheral module */
  SysCtrlPeripheralEnable(BSP_SPI_SSI_ENABLE_BM);
  
  /* Delay is essential for this customer */
  SysCtrlDelay(32);
  
  /* Configure pin type */
  GPIOPinTypeSSI(BSP_SPI_BUS_BASE, (BSP_SPI_MOSI | BSP_SPI_MISO | BSP_SPI_SCK | HAL_SPI_SS_PIN));
  
  /* Map SSI signals to the correct GPIO pins and configure them as HW ctrl'd */
  IOCPinConfigPeriphOutput(BSP_SPI_BUS_BASE, BSP_SPI_MISO, IOC_MUX_OUT_SEL_SSI0_TXD); 
  IOCPinConfigPeriphInput(BSP_SPI_BUS_BASE, BSP_SPI_SCK, IOC_CLK_SSIIN_SSI0);
  IOCPinConfigPeriphInput(BSP_SPI_BUS_BASE, BSP_SPI_MOSI, IOC_SSIRXD_SSI0);
  IOCPinConfigPeriphInput(BSP_SPI_BUS_BASE, HAL_SPI_SS_PIN, IOC_SSIFSSIN_SSI0);
  
  /* Disable SSI function */
  SSIDisable(BSP_SPI_SSI_BASE);
  
  /* Set system clock as SSI clock source */
  SSIClockSourceSet(BSP_SPI_SSI_BASE, SSI_CLOCK_SYSTEM);
  
  /* Configure SSI module to Motorola/Freescale SPI mode 3 Slave:
   * Polarity  = 1, observed in scope from MSP430 master 
   * Phase     = 1, observed in scope from MSP430 master
   * Word size = 8 bits
   * Clock     = 2MHz, observed MSP430 master clock is 2049180Hz 
   */
  SSIConfigSetExpClk(BSP_SPI_SSI_BASE, SysCtrlClockGet(), SSI_FRF_MOTO_MODE_3,
                     SSI_MODE_SLAVE, 2000000UL, 8); 
  
  /* Register SPI uDMA complete interrupt */
  SSIIntRegister(BSP_SPI_SSI_BASE, &npSpiUdmaCompleteIsr);
  
  /* Enable uDMA complete interrupt for SPI RX and TX */
  SSIDMAEnable(BSP_SPI_SSI_BASE, SSI_DMA_RX | SSI_DMA_TX);
  
  /* Configure SPI priority */
  IntPrioritySet(INT_SSI0, HAL_INT_PRIOR_SSI0);
  IntPrioritySet(INT_GPIOB, HAL_INT_PRIOR_SSI_MRDY);
  
  /* Enable the SSI function */
  SSIEnable(BSP_SPI_SSI_BASE);
  
  GPIOIntTypeSet(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN, GPIO_BOTH_EDGES);
  GPIOPortIntRegister(HAL_SPI_MRDY_BASE, *npSpiMrdyIsr);
  GPIOPinIntEnable(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN);
  
  /* Initialize uDMA for SPI */
  npSpiUdmaInit();
}


/**************************************************************************************************
* @fn          npSpiUdmaPrepareRx
*
* @brief       This function is called to set up uDMA for SPI RX. The uDMA and SPI must be 
*              initialized once already.
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
static void npSpiUdmaPrepareRx(void) 
{
  uint32 ulDummy;
  
  /* Flush the RX FIFO */
  while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &ulDummy));

  /* Prepare for the next one byte RX DMA */
  uDMAChannelTransferSet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT,
                         UDMA_MODE_BASIC, SPI_DATA, npSpiBuf, 1);
  uDMAChannelEnable(UDMA_CH10_SSI0RX);

  /* Disable the TX channel in RX */
  SSIDMADisable(BSP_SPI_SSI_BASE, SSI_DMA_TX);
  SSIDMAEnable(BSP_SPI_SSI_BASE, SSI_DMA_RX); 
}


/**************************************************************************************************
* @fn          npSpiUdmaInit
*
* @brief       This function is called to set up uDMA for SPI interface.
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
static void npSpiUdmaInit(void) 
{
  /* Enable the uDMA controller. */
  uDMAEnable();

  /* Set the base for the channel control table. */
  uDMAControlBaseSet(&ucDMAControlTable[0]);

  /* No attributes need to be set for a perpheral-based transfer.
   * The attributes are cleared by default, but are explicitly cleared
   * here, in case they were set elsewhere.
   */
  uDMAChannelAttributeDisable(UDMA_CH10_SSI0RX, UDMA_ATTR_ALL);
  uDMAChannelAttributeDisable(UDMA_CH11_SSI0TX, UDMA_ATTR_ALL);

  /* Now set up the characteristics of the transfer for
   * 8-bit data size, with source and destination increments
   * accordingly, and a byte-wise buffer copy. A bus arbitration
   * size of 1 is used.
   */
  uDMAChannelControlSet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT, 
                        UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_4);
  uDMAChannelControlSet(UDMA_CH11_SSI0TX | UDMA_PRI_SELECT, 
                        UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_4);
 
  /* Prepare SPI RX using uDMA */
  npSpiUdmaPrepareRx();
}


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

  if (npSpiState == NP_SPI_IDLE)
  {
     /* Poll for MRDY in case it was set before slave had setup the ISR.
      * Also, async responses may get queued, so flush them out here.
      */
    if ((GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN) == 0) || npSpiReadyCallback())
    {
      npSpiAReqReady();
    }
  }
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
  halIntState_t intState;
  
  HAL_ENTER_CRITICAL_SECTION_DEBUG(intState);   
  if (npSpiState == NP_SPI_WAIT_SREQ)
  {
    HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
    return npSpiBuf;
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION_DEBUG(intState); 
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
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION_DEBUG(intState); 
  if ((npSpiState == NP_SPI_WAIT_SREQ) && (GPIOPinRead(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN) == 0))
  {
    npSpiState = NP_SPI_WAIT_TX;
    
    /* Send TX packet using uDMA */
    spi_tx(pBuf);  
    HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
    osal_msg_deallocate((uint8 *)pBuf);
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
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

  HAL_ENTER_CRITICAL_SECTION_DEBUG(intState);
  if (npSpiState == NP_SPI_IDLE)
  {
    npSpiState = NP_SPI_WAIT_RX;

    /* Prepare SPI for uDMA RX */
    npSpiUdmaPrepareRx();

    /* If the salve has something to send, poll the SRDY pin low to inform
     * the host to poll.
     */
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0);   
  }
  HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
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
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION_DEBUG(intState);
  if (npSpiState == NP_SPI_WAIT_AREQ)
  {
    npSpiState = NP_SPI_IDLE;

    /* Pull the SRDY pin high */
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN); 
  }
  HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
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
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);
  if (npSpiState != NP_SPI_IDLE)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    return npSpiBuf;
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    return NULL;
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
  uint8 *pBuf;
  mtRpcCmdType_t type = (mtRpcCmdType_t)(npSpiBuf[1] & MT_RPC_CMD_TYPE_MASK);
  
  NP_SPI_ASSERT(npSpiState == NP_SPI_WAIT_RX);
  switch (type)
  {
  case MT_RPC_CMD_POLL:
    npSpiState = NP_SPI_WAIT_TX;
    if ((pBuf = npSpiPollCallback()) != NULL)
    {
      /* Send TX packet using uDMA */
      spi_tx(pBuf);
      osal_msg_deallocate((uint8 *)pBuf);
    }
    else
    {
      /* If there is no data in the queue, send 3 zeros. */
      pBuf = npSpiBuf;
      npSpiBuf[0] = npSpiBuf[1] = npSpiBuf[2] = 0;
          
      /* Send TX packet using uDMA */
      spi_tx(pBuf);
    }
    break;
    
  case MT_RPC_CMD_SREQ:
    npSpiState = NP_SPI_WAIT_SREQ;
    osal_set_event(znpTaskId, ZNP_SPI_RX_SREQ_EVENT);
    break;
    
  case MT_RPC_CMD_AREQ:
    npSpiState = NP_SPI_WAIT_AREQ;
    osal_set_event(znpTaskId, ZNP_SPI_RX_AREQ_EVENT);
    break;
    
  default:
    npSpiState = NP_SPI_IDLE;
    break;
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
  halIntState_t intState;
   
  HAL_ENTER_CRITICAL_SECTION_DEBUG(intState); 
  GPIOPinIntClear(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN);
   
  if (npSpiState == NP_SPI_IDLE)
  {
    /* Prepare SPI for uDMA RX */
    npSpiUdmaPrepareRx();

    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0);   
    npSpiState = NP_SPI_WAIT_RX;
  }
  else if(npSpiState == NP_SPI_WAIT_TX)
  {
    npSpiState = NP_SPI_IDLE;
  }
  HAL_EXIT_CRITICAL_SECTION_DEBUG(intState);
}

/**************************************************************************************************
*/
