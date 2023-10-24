/**************************************************************************************************
  Filename:       _hal_uart_usb.c
  Revised:        $Date: 2014-12-05 13:07:19 -0800 (Fri, 05 Dec 2014) $
  Revision:       $Revision: 41365 $

  Description: This file contains the interface to the H/W UART driver by USB.


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

/*********************************************************************
 * INCLUDES
 */

#include "hal_uart.h"
#include "usb.h"
#include "usb_cdc.h"
#include "hw_usb.h"
#include "usb_firmware_library_headers.h"
#include "mac_radio_reg_defs.h"
#include "usb_in_buffer.h"
#include "usb_out_buffer.h"

/*********************************************************************
 * MACROS
 */

#if !defined HAL_UART_BAUD_RATE
#define HAL_UART_BAUD_RATE  115200
#endif

/* The timeout tick is at 32-kHz, so multiply msecs by 33. */
#define HAL_UART_MSECS_TO_TICKS    33

#if !defined HAL_UART_USB_HIGH
#define HAL_UART_USB_HIGH         (256 / 2 - 16)
#endif
#if !defined HAL_UART_USB_IDLE
#define HAL_UART_USB_IDLE         (0 * HAL_UART_MSECS_TO_TICKS)
#endif

/* Max USB packet size, per specification; see also usb_cdc_descriptor.s51 */
#define HAL_UART_USB_TX_MAX        64

/***********************************************************************************
 * EXTERNAL VARIABLES
 */

/***********************************************************************************
 * GLOBAL VARIABLES
 */
void HalUARTInitUSB(void);
void HalUARTOpenUSB(halUARTCfg_t *config);
void halUartPollEvt(void);
void halUartPollRx(void);
void halUartPollTx(void);
void HalUARTPollUSB(void);
uint8 HalUARTRx(uint8 *buf, uint8 max);
uint16 HalUARTRxAvailUSB(void);
uint8 HalUARTTx(uint8 *buf, uint8 cnt);

/***********************************************************************************
 * LOCAL DATA
 */

/* NOTE: code in this module requires buffer sizes of exactly 256 bytes */
__no_init static uint8 halUartRxQ[256];
__no_init static uint8 halUartTxQ[256];

static uint8 halUartRxH, halUartRxT;
static uint8 halUartTxH, halUartTxT;

static uint8 rxTick;
static uint8 rxShdw;
static uint8 usbTxMT;
static halUARTCBack_t usbCB;

const uint32 USBBRRTable[] = {
  9600,
  19200,
  38400,
  57600,
  115200
};

/******************************************************************************
 * FUNCTIONS
 */

/***********************************************************************************
* @fn           HalUartInitUSB()
*
* @brief        USB UART init function.
*
* @param        none
*
* @return       none
*/
void HalUARTInitUSB(void)
{
  /* Stub */
}

/******************************************************************************
 * @fn      HalUARTOpenUSB()
 *
 * @brief   Open a port according tp the configuration specified by parameter.
 *
 * @param   config - contains configuration information
 *
 * @return  none
 *****************************************************************************/
void HalUARTOpenUSB(halUARTCfg_t *config)
{
  /* Set default line coding. */
  usbCdcData.lineCoding.dteRate =  USBBRRTable[config->baudRate];
  usbCdcData.lineCoding.charFormat = USBCDC_CHAR_FORMAT_1_STOP_BIT;
  usbCdcData.lineCoding.parityType = USBCDC_PARITY_TYPE_NONE;
  usbCdcData.lineCoding.dataBits = 8;

  /* Init USB library */
  usbfwInit();

  /* Initialize the USB interrupt handler with bit 
   * mask containing all processed USBIRQ events
   */
  usbirqInit(0xFFFF);
  usbCB = config->callBackFunc;
  
  /* Enable pullup on D+ */
  UsbDplusPullUpEnable();  
}

/**************************************************************************************************
 * @fn      HalUARTRxAvailUSB()
 *
 * @brief   Calculate Rx Buffer length - the number of bytes in the buffer.
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 **************************************************************************************************/
uint16 HalUARTRxAvailUSB(void)
{
  return ((halUartRxT >= halUartRxH)?
          halUartRxT - halUartRxH : sizeof(halUartRxQ) - halUartRxH + halUartRxT);
}

/***********************************************************************************
* @fn           halUartPollEvt
*
* @brief        Poll for USB events which are not directly related to the UART.
*
* @param        none
*
* @return       none
*/
void halUartPollEvt(void)
{
  /* Handle reset signaling on the bus */
  if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_RESET)
  {
    USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_RESET);
    usbfwResetHandler();
  }

  /* Handle packets on EP0 */
  if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_SETUP)
  {
    USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_SETUP);
    usbfwSetupHandler();
  }

  /* Handle USB suspend */
  if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_SUSPEND)
  {
    /* Clear USB suspend interrupt */
    USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_SUSPEND);

#if HAL_UART_USB_SUSPEND
    /* Take the chip into PM1 until a USB resume is deteceted. */
    usbsuspEnter();
#endif

    /* Running again; first clear RESUME interrupt */
    USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_RESUME);
  }
}

/***********************************************************************************
* @fn           halUartPollRx
*
* @brief        Poll for data from USB.
*
* @param        none
*
* @return       none
*/
void halUartPollRx(void)
{
  uint8 cnt;
  uint8 ep = USBFW_GET_SELECTED_ENDPOINT();
  USBFW_SELECT_ENDPOINT(4);

  /* If the OUT endpoint has received a complete packet. */
  if (USBFW_OUT_ENDPOINT_DISARMED())
  {
    halIntState_t intState;

    HAL_ENTER_CRITICAL_SECTION(intState);
    /* Get length of USB packet, this operation must not be interrupted. */
    cnt = USBFW_GET_OUT_ENDPOINT_COUNT_LOW();
    cnt += USBFW_GET_OUT_ENDPOINT_COUNT_HIGH() >> 8;
    HAL_EXIT_CRITICAL_SECTION(intState);

    while (cnt--)
    {
      halUartRxQ[halUartRxT++] = HWREG(USB_F4);
    }
    USBFW_ARM_OUT_ENDPOINT();

    /* If the USB has transferred in more Rx bytes, reset the Rx idle timer. */

    /* Re-sync the shadow on any 1st byte(s) received. */
    if (rxTick == 0)
    {
      rxShdw = ST0;
    }
    rxTick = HAL_UART_USB_IDLE;
  }
  else if (rxTick)
  {
    /* Use the LSB of the sleep timer (ST0 must be read first anyway). */
    uint8 decr = ST0 - rxShdw;

    if (rxTick > decr)
    {
      rxTick -= decr;
      rxShdw = ST0;
    }
    else
    {
      rxTick = 0;
    }
  }

  {
    uint8 evt = 0;
    cnt = halUartRxT - halUartRxH;

    if (cnt >= HAL_UART_USB_HIGH)
    {
      evt = HAL_UART_RX_ABOUT_FULL;
    }
    else if (cnt && !rxTick)
    {
      evt = HAL_UART_RX_TIMEOUT;
    }

    if (evt && (NULL != usbCB))
    {
      usbCB(0, evt);
    }
  }

  USBFW_SELECT_ENDPOINT(ep);
}

/***********************************************************************************
* @fn           halUartPollTx
*
* @brief        Poll for data to USB.
*
* @param        none
*
* @return       none
*/
void halUartPollTx(void)
{
  uint8 ep = USBFW_GET_SELECTED_ENDPOINT();
  USBFW_SELECT_ENDPOINT(4);

  /* If the IN endpoint is ready to accept data. */
  if (USBFW_IN_ENDPOINT_DISARMED())
  {
    if (halUartTxT == halUartTxH)
    {
      if (!usbTxMT && usbCB)
      {
        usbTxMT = TRUE;
        usbCB(0, HAL_UART_TX_EMPTY);
      }
    }
    else
    {
      uint8 max = HAL_UART_USB_TX_MAX;

      do
      {
        HWREG(USB_F4) = halUartTxQ[halUartTxH++];
      } while ((halUartTxH != halUartTxT) && (0 != --max));

      USBFW_ARM_IN_ENDPOINT();
    }
  }

  USBFW_SELECT_ENDPOINT(ep);
}

/***********************************************************************************
* @fn           HalUARTPollUSB
*
* @brief        The USB UART main task function. Should be called from the OSAL 
*               main loop.
*
* @param        none
*
* @return       none
*/
void HalUARTPollUSB(void)
{
#if defined HAL_BOOT_CODE
  while (USBIF)  usbirqFunctionEventHandler();
#endif
  halUartPollEvt();
  halUartPollRx();
  halUartPollTx();
}

/*************************************************************************************************
 * @fn      HalUARTRx()
 *
 * @brief   Read a buffer from the UART
 *
 * @param   buf - pointer to the buffer that will be written
 *          max - length of the buffer
 *
 * @return  length of the buffer that was read
 *************************************************************************************************/
uint8 HalUARTRx(uint8 *buf, uint8 max)
{
  uint8 cnt = 0;

  while ((halUartRxH != halUartRxT) && (cnt < max))
  {
    *buf++ = halUartRxQ[halUartRxH++];
    cnt++;
  }

  return cnt;
}

/*************************************************************************************************
 * @fn      HalUARTTx()
 *
 * @brief   Write a buffer to the UART
 *
 * @param   buf - pointer to the buffer that will be written
 *          cnt - length of the buffer
 *
 * @return  length of the buffer that was sent
 *************************************************************************************************/
uint8 HalUARTTx(uint8 *buf, uint8 cnt)
{
  uint8 len;

  for ( len = 0; len <  cnt; len++ )
  {
    halUartTxQ[halUartTxT++] = *buf++;
    
    /* Check for overrun */
    if ( ((halUartTxT + 1) & 0xFF) == halUartTxH )
    {
      /* Full, stop adding to the queue */
      break;
    }
  }

  usbTxMT = FALSE;

  return len;
}

/******************************************************************************
******************************************************************************/
