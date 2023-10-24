//*****************************************************************************
//! @file       bsp.h
//! @brief      Board support package header file for CC2538 on SmartRF06EB.
//!
//! Revised     $Date: 2013-04-11 10:41:57 -0700 (Thu, 11 Apr 2013) $
//! Revision    $Revision: 9707 $
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/
#ifndef __BSP_H__
#define __BSP_H__


/******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
* INCLUDES
*/
#include "hw_types.h"
#include "hw_memmap.h"


/******************************************************************************
* DEFINES
*/
// Clock speed defines
//! Default system clock speed
#define BSP_SYS_CLK_SPD         32000000UL
//! Default SPI clock speed. 8 MHz is supported by all SmartRF06EB peripherals.
#define BSP_SPI_CLK_SPD         8000000UL
//! Default UART clock speed (baud rate).
#define BSP_UART_CLK_SPD        115200

// SPI defines (Common for LCD, SD reader and accelerometer)
#define BSP_SPI_SSI_BASE        SSI0_BASE
//! Bitmask to enable SSI module.
#define BSP_SPI_SSI_ENABLE_BM   SYS_CTRL_PERIPH_SSI0
#define BSP_SPI_BUS_BASE        GPIO_A_BASE
#define BSP_SPI_SCK             GPIO_PIN_2      //!< PA2
#define BSP_SPI_MOSI            GPIO_PIN_4      //!< PA4
#define BSP_SPI_MISO            GPIO_PIN_5      //!< PA5

// 3.3-V domain defines
#define BSP_3V3_EN_BASE         GPIO_B_BASE
#define BSP_3V3_EN              GPIO_PIN_4      //!< PB4

// Board LED defines
#define BSP_LED_BASE            GPIO_C_BASE
#define BSP_LED_1               GPIO_PIN_0      //!< PC0
#define BSP_LED_2               GPIO_PIN_1      //!< PC1
#define BSP_LED_3               GPIO_PIN_2      //!< PC2
#define BSP_LED_4               GPIO_PIN_3      //!< PC3
#define BSP_LED_ALL             (BSP_LED_1 | \
                                 BSP_LED_2 | \
                                 BSP_LED_3 | \
                                 BSP_LED_4)     //!< Bitmask of all LEDs

// Board key defines
#define BSP_KEY_DIR_BASE        GPIO_C_BASE     //!< Base for left/right/up/down
#define BSP_KEY_SEL_BASE        GPIO_A_BASE     //!< Base for Select
#define BSP_KEY_1               GPIO_PIN_4      //!< PC4
#define BSP_KEY_2               GPIO_PIN_5      //!< PC5
#define BSP_KEY_3               GPIO_PIN_6      //!< PC6
#define BSP_KEY_4               GPIO_PIN_7      //!< PC7
#define BSP_KEY_5               GPIO_PIN_3      //!< PA3
#define BSP_KEY_ALL             (BSP_KEY_1| \
                                 BSP_KEY_2| \
                                 BSP_KEY_3| \
                                 BSP_KEY_4| \
                                 BSP_KEY_5)     //!< Bitmask of all keys
#define BSP_KEY_LEFT            BSP_KEY_1
#define BSP_KEY_RIGHT           BSP_KEY_2
#define BSP_KEY_UP              BSP_KEY_3
#define BSP_KEY_DOWN            BSP_KEY_4
#define BSP_KEY_SELECT          BSP_KEY_5
#define BSP_KEY_DIR_ALL         (BSP_KEY_LEFT|  \
                                 BSP_KEY_RIGHT| \
                                 BSP_KEY_UP|    \
                                 BSP_KEY_DOWN)  //!< Bitmask of all dir. keys

// Board LCD defines
#define BSP_LCD_MODE_BASE       GPIO_B_BASE
#define BSP_LCD_MODE            GPIO_PIN_2      //!< PB2 (LCD MODE, aka. A0)
#define BSP_LCD_RST_BASE        GPIO_B_BASE
#define BSP_LCD_RST             GPIO_PIN_3      //!< PB3
#define BSP_LCD_CS_BASE         GPIO_B_BASE
#define BSP_LCD_CS              GPIO_PIN_5      //!< PB5
#define BSP_LCD_SCK_BASE        BSP_SPI_BUS_BASE
#define BSP_LCD_SCK             BSP_SPI_SCK     //!< PA2
#define BSP_LCD_MOSI_BASE       BSP_SPI_BUS_BASE
#define BSP_LCD_MOSI            BSP_SPI_MOSI    //!< PA4
#define BSP_LCD_MISO_BASE       BSP_SPI_BUS_BASE
#define BSP_LCD_MISO            BSP_SPI_MISO    //!< PA5

// Board accelerometer defines
#define BSP_ACC_PWR_BASE        GPIO_D_BASE
#define BSP_ACC_PWR             GPIO_PIN_4      //!< PD4
#define BSP_ACC_INT_BASE        GPIO_D_BASE
#define BSP_ACC_INT             GPIO_PIN_2      //!< PD2
#define BSP_ACC_INT1_BASE       BSP_ACC_INT_BASE
#define BSP_ACC_INT1            BSP_ACC_INT     //!< ACC_INT1 == ACC_INT
#define BSP_ACC_INT2_BASE       GPIO_D_BASE
#define BSP_ACC_INT2            GPIO_PIN_2      //!< PD1
#define BSP_ACC_CS_BASE         GPIO_D_BASE
#define BSP_ACC_CS              GPIO_PIN_5      //!< PD5
#define BSP_ACC_SCK_BASE        BSP_SPI_BUS_BASE
#define BSP_ACC_SCK             BSP_SPI_SCK     //!< PA2
#define BSP_ACC_MOSI_BASE       BSP_SPI_BUS_BASE
#define BSP_ACC_MOSI            BSP_SPI_MOSI    //!< PA4
#define BSP_ACC_MISO_BASE       BSP_SPI_BUS_BASE
#define BSP_ACC_MISO            BSP_SPI_MISO    //!< PA5

// SD reader defines
#define BSP_SDCARD_CS_BASE      GPIO_D_BASE
#define BSP_SDCARD_CS           GPIO_PIN_0      //!< PD0
#define BSP_SDCARD_SCK_BASE     BSP_SPI_BUS_BASE
#define BSP_SDCARD_SCK          BSP_SPI_SCK     //!< PA2
#define BSP_SDCARD_MOSI_BASE    BSP_SPI_BUS_BASE
#define BSP_SDCARD_MOSI         BSP_SPI_MOSI    //!< PA4
#define BSP_SDCARD_MISO_BASE    BSP_SPI_BUS_BASE
#define BSP_SDCARD_MISO         BSP_SPI_MISO    //!< PA5

// Board ambient lightsensor defines
#define BSP_ALS_PWR_BASE        GPIO_A_BASE
#define BSP_ALS_PWR             GPIO_PIN_7      //!< PA7
#define BSP_ALS_OUT_BASE        GPIO_A_BASE
#define BSP_ALS_OUT             GPIO_PIN_6      //!< PA6

// UART backchannel defines
#define BSP_UART_BASE           UART0_BASE
#define BSP_UART_ENABLE_BM      SYS_CTRL_PERIPH_UART0
#define BSP_UART_BUS_BASE       GPIO_A_BASE
#define BSP_UART_RXD_BASE       BSP_UART_BUS_BASE
#define BSP_UART_RXD            GPIO_PIN_0      //!< PA0
#define BSP_UART_TXD_BASE       BSP_UART_BUS_BASE
#define BSP_UART_TXD            GPIO_PIN_1      //!< PA1
#define BSP_UART_CTS_BASE       GPIO_B_BASE
#define BSP_UART_CTS            GPIO_PIN_0      //!< PB0
#define BSP_UART_RTS_BASE       GPIO_D_BASE
#define BSP_UART_RTS            GPIO_PIN_3      //!< PD3
#define BSP_UART_INT_BM         0xF0            //!< Interrupts handled by bsp uart

/******************************************************************************
* FUNCTION PROTOTYPES
*/
extern void bspInit(uint32_t ui32SysClockSpeed);
extern void bspSpiInit(uint32_t ui32ClockSpeed);
extern uint32_t bspSpiClockSpeedGet(void);
extern void bspSpiClockSpeedSet(uint32_t ui32ClockSpeed);
extern void bsp3V3DomainEnable(void);
extern void bsp3V3DomainDisable(void);
extern void bsp3V3DomainDisableForced(void);
extern uint8_t bsp3V3DomainEnabled(void);
extern void bspAssert(void);


/******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __BSP_H__ */
