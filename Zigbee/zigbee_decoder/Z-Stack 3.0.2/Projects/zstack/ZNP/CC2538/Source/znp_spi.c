/**************************************************************************************************
Filename:       znp_spi.c
Revised:        $Date: 2013-11-14 13:06:42 -0800 (Thu, 14 Nov 2013) $
Revision:       $Revision: 36101 $

Description:

This file contains the interface to the H/W-specific ZNP SPI driver. Please
NOTE that this driver is not yet complete and is in progress

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
#if defined POWER_SAVING
#include "OSAL_PwrMgr.h"
#endif        
#include "ZDApp.h"
#include "znp_app.h"
#include "znp_spi.h"
#include "hw_ioc.h"
#include "ssi.h"

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

#define NP_SPI_ASSERT( expr)        HAL_ASSERT( expr )

/* ------------------------------------------------------------------------------------------------
*                                           Constants
* ------------------------------------------------------------------------------------------------
*/

/* Chip Select Port and Pin */
#define HAL_SPI_SS_BASE            GPIO_A_BASE
#define HAL_SPI_SS_PIN             GPIO_PIN_3

#ifdef ZNP_ALT
/* MRDY Port and Pin and Interrupt */
#define HAL_SPI_MRDY_BASE          GPIO_B_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_2
#define HAL_INT_MRDY               INT_GPIOB

/* SRDY Port and Pin */
#define HAL_SPI_SRDY_BASE          GPIO_D_BASE
#define HAL_SPI_SRDY_PIN           GPIO_PIN_5

#elif BB_ZNP /* BeagleBone-ZNP connected */
#define HAL_SPI_MRDY_BASE          GPIO_D_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_3
#define HAL_INT_MRDY               INT_GPIOD

#define HAL_SPI_SRDY_BASE          GPIO_B_BASE
#define HAL_SPI_SRDY_PIN           GPIO_PIN_0

#else
/* MRDY Port and Pin and Interrupt */
#define HAL_SPI_MRDY_BASE          GPIO_C_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_5
#define HAL_INT_MRDY               INT_GPIOC

/* SRDY Port and Pin */
#define HAL_SPI_SRDY_BASE          GPIO_B_BASE
#define HAL_SPI_SRDY_PIN           GPIO_PIN_0
#endif

#define NP_SPI_BUF_LEN  (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + 1)

/* Incase of TX, Timeout after 1000 tries of writing to TX FIFO  */
#define SPI_TIMEOUT                1000

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
  NP_SPI_WAIT_SREQ,      /* Waiting for a synchronous request to finish processing. */
} spiState_t;

/* ------------------------------------------------------------------------------------------------
*                                           Local Variables
* ------------------------------------------------------------------------------------------------
*/

/* RX buffer */
static uint8 npSpiBuf[NP_SPI_BUF_LEN];
/* RX Index in RX Buffer */
uint32 rxId = 0;
static volatile spiState_t npSpiState;
static uint8 npSpiBufTx[4];

/* ------------------------------------------------------------------------------------------------
*                                           Local Functions
* ------------------------------------------------------------------------------------------------
*/

void MrdyIsr(void);
void npSpiIsr(void);
void npSpiErrorRecoveryIsr(void);

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
  uint32        ulDummy;
  
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
  
  SysCtrlDelay(32);
  
  /* Setup the MPSI, MISO, Clk and Chip Select pins */
  GPIOPinTypeSSI(BSP_SPI_BUS_BASE, (BSP_SPI_MOSI|BSP_SPI_MISO|BSP_SPI_SCK|HAL_SPI_SS_PIN));
  
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
  
  /* Register SPI interrupt */
  SSIIntRegister(BSP_SPI_SSI_BASE, &npSpiIsr);
  
  /* Push the SPI priority HIGH. With the interrupt model for SSI, It is 
   * important to NOTE that SSI and MRDY HAS to be the highest allowed 
   * priority interrupts. 
   */
  IntPrioritySet(INT_SSI0, HAL_INT_PRIOR_SSI0);
  IntPrioritySet(HAL_INT_MRDY, HAL_INT_PRIOR_SSI_MRDY);
  
  /* Disable SPI interrupt */
  SSIIntDisable(BSP_SPI_SSI_BASE, (SSI_TXFF  | SSI_RXFF  | SSI_RXTO  | SSI_RXOR));
  SSIIntClear(BSP_SPI_SSI_BASE, (SSI_TXFF  | SSI_RXFF  | SSI_RXTO  | SSI_RXOR)); 
  
  /*  Enable the SSI function */
  SSIEnable(BSP_SPI_SSI_BASE);
  
  /* Flush the RX FIFO */
  while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &ulDummy));
  
  /* MRDY Isr trigerred on both falling and rising edge */
  GPIOIntTypeSet(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN, GPIO_BOTH_EDGES);
  GPIOPortIntRegister(HAL_SPI_MRDY_BASE, *MrdyIsr);
  GPIOPinIntEnable(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN);
  
  /* Clear SSI Interrupt */
  SSIIntClear(BSP_SPI_SSI_BASE, (SSI_RXTO  | SSI_RXOR)); 
  SSIIntEnable(BSP_SPI_SSI_BASE, (SSI_RXFF  | SSI_RXTO | SSI_RXOR ));
  
  /* SPI debug code. Uncomment to use GPIOs to debug 
   * GPIOPinTypeGPIOOutput(GPIO_B_BASE, GPIO_PIN_1);
   * GPIOPinWrite(GPIO_B_BASE, GPIO_PIN_1, 0);
   * GPIOPinTypeGPIOOutput(GPIO_B_BASE, GPIO_PIN_3);
   * GPIOPinWrite(GPIO_B_BASE, GPIO_PIN_3, 0); 
   */
}

/**************************************************************************************************
* @fn          npSpiIsr
*
* @brief       SPI interrupt handler.
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
void npSpiIsr(void)
{
  uint8 volatile dummy;
  uint8 volatile status; 
  halIntState_t  intState;
  
  HAL_ENTER_CRITICAL_SECTION(intState);
  /* Read raw interrupt status */
  status = SSIIntStatus(BSP_SPI_SSI_BASE, 0);
  
  /* if interrupt source cannot be determined return immeduiately*/ 
  if( status == 0)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    return;
  }
  
  /* if state is NP_SPI_WAIT_RX and RX FIFO is half or more full */
  if((npSpiState == NP_SPI_WAIT_RX) && (status & SSI_RXFF))
  {
    /* Non Blocking read of bytes from RXFIFO and Increment
     * counter only if byte was successfully read 
     */
    while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &npSpiBuf[rxId ]) == 1)
    {
      rxId ++;
    }
    
    /* Process the bytes recieved after recieving the entire packet */
    if((rxId  >= (npSpiBuf[MT_RPC_POS_LEN] + MT_RPC_FRAME_HDR_SZ)))
    {
      rxId  = 0;
      npSpiRxIsr();
    }
  }
  else if((npSpiState == NP_SPI_WAIT_RX) && (status & SSI_RXTO))
  {
    while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &npSpiBuf[rxId ]) == 1)
    {
      rxId ++;
    }
    /* Clear the RXTO interrupt */
    SSIIntClear(BSP_SPI_SSI_BASE, SSI_RXTO);
    
    if((rxId  >= (npSpiBuf[MT_RPC_POS_LEN] + MT_RPC_FRAME_HDR_SZ)))
    {
      rxId  = 0;
      npSpiRxIsr();
    } 
  }
  else 
  {
    if(status == SSI_TXFF)
    {
      HAL_EXIT_CRITICAL_SECTION(intState);
      return;
    } 
    
    /* Workaround for ZNP to recover if Master sends bytes when 
     * ZNP is processing another command. 
     */
    if(npSpiState == NP_SPI_WAIT_AREQ || npSpiState == NP_SPI_WAIT_SREQ )
    {
      rxId  = 0;
      npSpiState = NP_SPI_IDLE;
      while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &dummy));
    }
    
    /* If RX overrun, clear the RXOR flag and error recovery*/
    if(status & SSI_RXOR)
    {
      SSIIntClear(BSP_SPI_SSI_BASE, SSI_RXOR); 
      if(npSpiState == NP_SPI_WAIT_RX)
      {
        rxId  = 0;
        while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &dummy));
        npSpiErrorRecoveryIsr();
      }
    }
    
    /* If RX Timeout in invalid npSpiState (not in NP_SPI_WAIT_RX is invalid)
     * clear the RXTO flag 
     */
    if(status & SSI_RXTO)
    {
      SSIIntClear(BSP_SPI_SSI_BASE, SSI_RXTO);
    }
  }
  HAL_EXIT_CRITICAL_SECTION(intState);
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
  uint8         dummy;
  if (ZNP_CFG1_UART == znpCfg1)
  {
    return;
  }
  
  if (npSpiState == NP_SPI_IDLE)
  {
    /* Poll for MRDY in case it was set before slave had setup the ISR.
     * Also, async responses may get queued, so flush them out here.
     */
    if ((GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN) == 0) || (npSpiReadyCallback()))
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
  HAL_ENTER_CRITICAL_SECTION(intState);
  if (npSpiState == NP_SPI_WAIT_SREQ)
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
  uint8         dummy;
  uint32        counter = 0;
  uint32        txCounter, txLen;
  
  HAL_ENTER_CRITICAL_SECTION(intState);
  
  /* If state is wait SREQ and SRDY is low */
  if ((npSpiState == NP_SPI_WAIT_SREQ) && (GPIOPinRead(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN) == 0))
  {
    /* Waiting to transmit state */
    npSpiState = NP_SPI_WAIT_TX;
    txLen = pBuf[MT_RPC_POS_LEN] + MT_RPC_FRAME_HDR_SZ;
    /* Reset txCounter*/
    txCounter = 0;
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
    
    /* Put Bytes in TX FIFO if less than the 'command length' and if lesser 
     * than the timeout. Timeout is safety exit mechanism, incase TX FIFO 
     * is full and Slave cannot write any more bytes to FIFO.
     */
    while(txCounter < txLen && (counter < SPI_TIMEOUT))
    {
      /* Increment counter only after successful write to TX FIFO */
      if(SSIDataPutNonBlocking(SSI0_BASE, pBuf[txCounter]) == 1)
      {
        txCounter++;
        /* Half Duplex protocol: empty any bytes recieved 
         * in RXFIFO, while Transmit to master
         */
        SSIDataGetNonBlocking(SSI0_BASE, &dummy);
      }
      /* increment timeout counter */
      counter++;
    }
    HAL_EXIT_CRITICAL_SECTION(intState);
  }
  else /* if ((npSpiState == NP_SPI_WAIT_SREQ.... */
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
  }
  
  /* Free the buffer */
  osal_msg_deallocate((uint8 *)pBuf);
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
    /* waiting to recieve bytes: Change State and set SRDY low */
    npSpiState = NP_SPI_WAIT_RX;
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0);
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
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION(intState);
  if (npSpiState == NP_SPI_WAIT_AREQ)
  {
    /* AREQ packet processing complete: Move to Idle and set SRDY high */
    npSpiState = NP_SPI_IDLE;
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  }
  HAL_EXIT_CRITICAL_SECTION(intState);
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
  uint8          *pBuf;
  uint8          dummy;
  uint32         counter = 0;
  uint32         txLen, txCount;
  mtRpcCmdType_t type = (mtRpcCmdType_t)(npSpiBuf[1] & MT_RPC_CMD_TYPE_MASK);
  
  NP_SPI_ASSERT(npSpiState == NP_SPI_WAIT_RX);
  switch (type)
  {
    
  case MT_RPC_CMD_POLL:
    if ( (pBuf = npSpiPollCallback()) == NULL )
    {
      pBuf = npSpiBufTx;
      npSpiBufTx[0] = 0;
      npSpiBufTx[1] = 0;
      npSpiBufTx[2] = 0;
    }
    npSpiState = NP_SPI_WAIT_TX;
    txLen = pBuf[MT_RPC_POS_LEN] + MT_RPC_FRAME_HDR_SZ;
    txCount = 0;
    
    /* Pull SRDY high to indicate Slave is ready to transmit bytes*/
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
    
    /* Put Bytes in TX FIFO if less than the 'command length' and if lesser 
     * than the timeout. Timeout is safety exit mechanism, incase TX FIFO 
     * is full and Slave cannot write any more bytes to FIFO.
     */
    while(txCount < txLen && (counter < SPI_TIMEOUT))
    {
      if(SSIDataPutNonBlocking(SSI0_BASE, pBuf[txCount]) == 1)
      {
        txCount++;
        SSIDataGetNonBlocking(SSI0_BASE, &dummy);
      }
      counter++;
    }
    
    /* free the previously allocated TX buffer, if it was not static allocation */
    if(pBuf != npSpiBuf)
    {
      osal_msg_deallocate((uint8 *)pBuf);
    }
    break;
    
  case MT_RPC_CMD_SREQ:
    npSpiState = NP_SPI_WAIT_SREQ;
    /* send SREQ event to ZNP Task*/
    osal_set_event(znpTaskId, ZNP_SPI_RX_SREQ_EVENT);
    break;
    
  case MT_RPC_CMD_AREQ:
    npSpiState = NP_SPI_WAIT_AREQ;
    /* send AREQ event to ZNP Task */
    osal_set_event(znpTaskId, ZNP_SPI_RX_AREQ_EVENT);
    break;
    
  default:
    npSpiState = NP_SPI_IDLE;
    break;
  }
}

/**************************************************************************************************
* @fn          npSpiErrorRecoveryIsr
*
* @brief       This function handles RX overrun recovery. The bytes are still
*              lost when RX overrun occurs, but tis ensures that the next packet
*              following the overrun is recieved properly.
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
void npSpiErrorRecoveryIsr(void)
{
  uint8          dummy;
  uint32         counter = 0;
  uint32         txLen, txCount;
  uint8          *pBuf;
  npSpiBufTx[0] = 0;
  npSpiBufTx[1] = 0;
  npSpiBufTx[2] = 0;
  pBuf = npSpiBufTx;
  
  npSpiState = NP_SPI_WAIT_TX;
  txLen = pBuf[MT_RPC_POS_LEN] + MT_RPC_FRAME_HDR_SZ;
  txCount = 0;
    
  /* Pull SRDY high to indicate Slave is ready to transmit bytes*/
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  while(txCount < txLen && (counter < SPI_TIMEOUT))
  {
    if(SSIDataPutNonBlocking(SSI0_BASE, pBuf[txCount]) == 1)
    {
      txCount++;
      SSIDataGetNonBlocking(SSI0_BASE, &dummy);
    }
    counter++;
  }
}


/**************************************************************************************************
* @fn          MrdyIsr
*
* @brief       This function handles the MRDY interrupt.
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
void MrdyIsr(void)
{
  halIntState_t intState;
  uint8 dummy;
  
  HAL_ENTER_CRITICAL_SECTION(intState); 
  /* Clear the MRDY interrupt */
  GPIOPinIntClear(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN);
  
  /* If state is idle get ready to recieve bytes by changing state and 
   * setting SRDY low
   */
  if (npSpiState == NP_SPI_IDLE)
  {
    npSpiState = NP_SPI_WAIT_RX;
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0); 
    
  }
  else if(npSpiState == NP_SPI_WAIT_TX)
  {
    /* If state is NP_SPI_WAIT_TX i.e. transmitting bytes and MRDY Isr 
     * has occured: implies transmission is complete because Master Has 
     * pulled MRDY up. Since this is half duplex protocol on full duplex  
     * SSI hardware, empty the RXFIFO of any remaining bytes recieved 
     * during transmission.
     */
    while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, &dummy));
    npSpiState = NP_SPI_IDLE;
  }
  HAL_EXIT_CRITICAL_SECTION(intState);
}

/**************************************************************************************************
*/

