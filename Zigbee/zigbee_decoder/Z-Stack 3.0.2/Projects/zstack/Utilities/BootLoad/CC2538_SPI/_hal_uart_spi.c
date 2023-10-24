/**************************************************************************************************
  Filename:       _hal_uart_spi.c
  Revised:        $Date: 2013-11-13 12:00:11 -0800 (Wed, 13 Nov 2013) $
  Revision:       $Revision: 36075 $

  Description: This file contains the interface to the H/W UART driver by SPI, by ISR.
               Note that this is a trivial implementation only to support the boot loader and
               is not fit for a general SPI communication protocol which would have to be more
               sophisticated like the RPC for ZNP.


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

/*********************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "hal_board.h"
#include "hal_defs.h"
#include "hal_uart.h"
#include "hw_ioc.h"
#include "ssi.h"

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * CONSTANTS
 */

/* MRDY Port and Pin */
#define HAL_SPI_SS_BASE            GPIO_A_BASE
#define HAL_SPI_SS_PIN             GPIO_PIN_3

#ifdef SBL_ALT
#define HAL_SPI_MRDY_BASE          GPIO_B_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_2

/* SRDY Port and Pin */
#define HAL_SPI_SRDY_BASE          GPIO_D_BASE
#define HAL_SPI_SRDY_PIN           GPIO_PIN_5

#elif BB_ZNP /* BeagleBone-ZNP connected */
#define HAL_SPI_MRDY_BASE          GPIO_D_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_3

#define HAL_SPI_SRDY_BASE          GPIO_B_BASE
#define HAL_SPI_SRDY_PIN           GPIO_PIN_0

#else
#define HAL_SPI_MRDY_BASE          GPIO_C_BASE
#define HAL_SPI_MRDY_PIN           GPIO_PIN_5

/* SRDY Port and Pin */
#define HAL_SPI_SRDY_BASE          GPIO_B_BASE
#define HAL_SPI_SRDY_PIN           GPIO_PIN_0  
#endif

/* Reference data sheet section 8.2.4: When using variable-length transfers, then LEN
 * should be set to the largest allowed transfer length plus one.
 */
#define NP_SPI_BUF_LEN  (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + 1)

#define SPI_TIMEOUT                1000

#define HAL_UART_SPI_RX_MAX        128
 
/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8 rxBuf[HAL_UART_SPI_RX_MAX];
  uint8 rxHead;
  volatile uint8 rxTail;
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
  uint32 ulDummy;
 
  /* Configure SRDY and deassert SRDY */
  GPIOPinTypeGPIOOutput(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN);
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  
  GPIOPinTypeGPIOInput(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN);
  GPIOPinWrite(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN, HAL_SPI_MRDY_PIN);
  
  /* Enable SSI peripheral module */
  SysCtrlPeripheralEnable(BSP_SPI_SSI_ENABLE_BM);
  
  /* Delay is essential for this customer */
  SysCtrlDelay(32);
  
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
 
  
  /* Disable SPI interrupt */
  SSIIntDisable(BSP_SPI_SSI_BASE, (SSI_TXFF  | SSI_RXFF  | SSI_RXTO  | SSI_RXOR));
  SSIIntClear(BSP_SPI_SSI_BASE, (SSI_TXFF  | SSI_RXFF  | SSI_RXTO  | SSI_RXOR)); 
  
  /*  Enable the SSI function */
  SSIEnable(BSP_SPI_SSI_BASE);

  /* Flush the RX FIFO */
  while(SSIDataGetNonBlocking(BSP_SPI_SSI_BASE, (uint32_t *)&ulDummy));
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
  SSIDisable(BSP_SPI_SSI_BASE);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_SSI0);
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

  uint32_t data;
  if ((spiCfg.rxHead == 0) && (spiCfg.rxHead == spiCfg.rxTail))
  {
    
    /* Empty bytes in the RX FIFO first */
    while(SSIDataGetNonBlocking(SSI0_BASE, &data));
    
    /* Slave signals ready for read by setting its ready flag first. */
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0);
  
    /* Wait for MRDY to be active low */
    while( GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN)  & HAL_SPI_MRDY_PIN )
    {
      ASM_NOP;
    }
    
    /* Blocking Read of first packet byte from RX FIFO */
    SSIDataGet(SSI0_BASE, &data);
    spiCfg.rxBuf[spiCfg.rxTail] = (uint8)data;
    spiCfg.rxTail++;
    
    /* Non Blocking Read of remaining bytes from RX FIFO */
    do{
      while(SSIDataGetNonBlocking(SSI0_BASE, &data) == 1)
      {
        spiCfg.rxBuf[spiCfg.rxTail] = (uint8)data;
        spiCfg.rxTail++;
      }
    }while( GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN) == 0);
    
     /* Set SRDY high to indicate end of reception */
    GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  }
  
  /* Copy requested number of bytes to buf */
  while ((spiCfg.rxHead != spiCfg.rxTail) && (cnt < len))
  {
    *buf++ = spiCfg.rxBuf[spiCfg.rxHead++];
    if (spiCfg.rxHead >= HAL_UART_SPI_RX_MAX)
    {
      spiCfg.rxHead = 0;
    }
    cnt++;
  }

  /* Reset the head and tail */
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
  uint16 cnt = 0; 
  uint32 volatile dummy;
  
  /* Wait for MRDY to be active low */
  while( GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN)  & HAL_SPI_MRDY_PIN )
  {
    ASM_NOP;
  }
  
  /* Set SRDY low */
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, 0);

  /* Blocking Write of first byte to TX FIFO */
  SSIDataPut(SSI0_BASE, *buf);
  SSIDataGetNonBlocking(SSI0_BASE, (uint32_t *)&dummy);
  buf++;
  cnt++;
  
  /* While MRDY is low and count is lesser than packeyt length , write to  
   * TX FIFO
   */
  while((cnt < len) && (GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN)== 0))
  {
    if(SSIDataPutNonBlocking(SSI0_BASE, *buf) == 1)
    {
      cnt++;
      SSIDataGetNonBlocking(SSI0_BASE, (uint32_t *)&dummy);
      buf++;
    }
  }
  
  /* Wait for MRDY to go high*/
  while(GPIOPinRead(HAL_SPI_MRDY_BASE, HAL_SPI_MRDY_PIN)== 0)
  {
    ASM_NOP;
  }
  
  /* Set SRDY high to indicate end of transmission */
  GPIOPinWrite(HAL_SPI_SRDY_BASE, HAL_SPI_SRDY_PIN, HAL_SPI_SRDY_PIN);
  return cnt;
}


/******************************************************************************
******************************************************************************/
