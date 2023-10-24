/**************************************************************************************************
  Filename:       OnBoard.h
  Revised:        $Date: 2012-04-18 14:06:48 -0700 (Wed, 18 Apr 2012) $
  Revision:       $Revision: 30252 $

  Description:    Defines stuff for EVALuation boards
  Notes:          This file targets the Chipcon CC2530/31


  Copyright 2005-2010 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ONBOARD_H
#define ONBOARD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "hal_mcu.h"
#include "hal_uart.h"
#include "hal_sleep.h"
#include "osal.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */

// 64-bit Extended Address of this device
extern uint8 aExtendedAddress[8];

/*********************************************************************
 * CONSTANTS
 */

// Timer clock and power-saving definitions
#define TIMER_DECR_TIME    1  // 1ms - has to be matched with TC_OCC

/* OSAL timer defines */
#define TICK_TIME   1000   // Timer per tick - in micro-sec
/*
  Timer4 interrupts @ 1.0 msecs using 1/128 pre-scaler
  TICK_COUNT = (CPUMHZ / 128) / 1000
*/
#define TICK_COUNT  1  // 32 Mhz Output Compare Count

/* CC2430 DEFINITIONS */

// MEMCTR bit definitions
#define ALWAYS1    0x01  // Reserved: always=1
#define CACHDIS    0x02  // Flash cache: disable=1
#define FMAP0      0x10  // Flash bank map, bit0
#define FMAP1      0x20  // Flash bank map, bit1
#define FMAP       0x30  // Flash bank map, mask
#define MUNIF      0x40  // Memory mapping: unified=1

// PCON bit definitions
#define PMODESET   0x01  // Power mode control: 1=set PMx

// Reset bit definitions
#define LRESET     0x18  // Last reset bit mask
#define RESETPO    0x00  // Power-On reset
#define RESETEX    0x08  // External reset
#define RESETWD    0x10  // WatchDog reset

/* GPIO PORT DEFINITIONS */
// GPIO bit definitions
#define GPIO_0     0x01  // Px_0: GPIO=0, PIO=1
#define GPIO_1     0x02  // Px_1: GPIO=0, PIO=1
#define GPIO_2     0x04  // Px_2: GPIO=0, PIO=1
#define GPIO_3     0x08  // Px_3: GPIO=0, PIO=1
#define GPIO_4     0x10  // Px_4: GPIO=0, PIO=1
#define GPIO_5     0x20  // Px_5: GPIO=0, PIO=1
#define GPIO_6     0x40  // Px_6: GPIO=0, PIO=1
#define GPIO_7     0x80  // Px_7: GPIO=0, PIO=1

/* WATCHDOG TIMER DEFINITIONS */
// WDCTL bit definitions

#define WDINT0     0x01  // Interval, bit0
#define WDINT1     0x02  // Interval, bit1
#define WDINT      0x03  // Interval, mask
#define WDMODE     0x04  // Mode: watchdog=0, timer=1
#define WDEN       0x08  // Timer: disabled=0, enabled=1
#define WDCLR0     0x10  // Clear timer, bit0
#define WDCLR1     0x20  // Clear timer, bit1
#define WDCLR2     0x40  // Clear timer, bit2
#define WDCLR3     0x80  // Clear timer, bit3
#define WDCLR      0xF0  // Clear timer, mask

// WD timer intervals
#define WDTISH     0x03  // Short: clk * 64
#define WDTIMD     0x02  // Medium: clk * 512
#define WDTILG     0x01  // Long: clk * 8192
#define WDTIMX     0x00  // Maximum: clk * 32768

// WD clear timer patterns
#define WDCLP1     0xA0  // Clear pattern 1
#define WDCLP2     0x50  // Clear pattern 2

/*********************************************************************
 * MACROS
 */

// These Key definitions are unique to this development system.
// They are used to bypass functions when starting up the device.
#define SW_BYPASS_NV    HAL_KEY_SW_5  // Bypass Network layer NV restore
#define SW_BYPASS_START HAL_KEY_SW_1  // Bypass Network initialization

// LCD Support Defintions
#ifdef LCD_SUPPORTED
  #if !defined DEBUG
    #define DEBUG  0
  #endif
  #if LCD_SUPPORTED==DEBUG
    #define SERIAL_DEBUG_SUPPORTED  // Serial-debug
  #endif
#else // No LCD support
  #undef SERIAL_DEBUG_SUPPORTED  // No serial-debug
#endif

/* Serial Port Definitions */
#if defined (ZAPP_P1)
  #define ZAPP_PORT HAL_UART_PORT_0
#elif defined (ZAPP_P2)
  #define ZAPP_PORT HAL_UART_PORT_1
#else
  #undef ZAPP_PORT
#endif
#if defined (ZTOOL_P1)
  #define ZTOOL_PORT HAL_UART_PORT_0
#elif defined (ZTOOL_P2)
  #define ZTOOL_PORT HAL_UART_PORT_1
#else
  #undef ZTOOL_PORT
#endif

#define MT_UART_TX_BUFF_MAX  128
#define MT_UART_RX_BUFF_MAX  128
#define MT_UART_THRESHOLD   (MT_UART_RX_BUFF_MAX / 2)
#define MT_UART_IDLE_TIMEOUT 6

// Restart system from absolute beginning
// Disables interrupts, forces WatchDog reset
#define SystemReset()       \
{                           \
  HAL_DISABLE_INTERRUPTS(); \
  HAL_SYSTEM_RESET();       \
}

#define SystemResetSoft()  Onboard_soft_reset()

/* Reset reason for reset indication */
#define ResetReason() ((SLEEPSTA >> 3) & 0x03)

/* WATCHDOG TIMER DEFINITIONS */
#define WatchDogEnable(wdti)              \
{                                         \
  WDCTL = WDCLP1 | WDEN | (wdti & WDINT); \
  WDCTL = WDCLP2 | WDEN | (wdti & WDINT); \
}

// Wait for specified microseconds
#define MicroWait(t) Onboard_wait(t)

#define OSAL_SET_CPU_INTO_SLEEP(timeout) halSleep(timeout); /* Called from OSAL_PwrMgr */

#ifdef __IAR_SYSTEMS_ICC__
// Internal (MCU) Stack addresses
#define CSTACK_BEG ((uint8 const *)(_Pragma("segment=\"XSTACK\"") __segment_begin("XSTACK")))
#define CSTACK_END ((uint8 const *)(_Pragma("segment=\"XSTACK\"") __segment_end("XSTACK"))-1)
// Stack Initialization Value
#define STACK_INIT_VALUE  0xCD
#else
#error Check compiler compatibility.
#endif

/* The following Heap sizes are setup for typical TI sample applications,
 * and should be adjusted to your systems requirements.
 */
#if !defined INT_HEAP_LEN
#if defined RTR_NWK
  #define INT_HEAP_LEN  3072
#else
  #define INT_HEAP_LEN  2048
#endif
#endif
#define MAXMEMHEAP INT_HEAP_LEN

#define KEY_CHANGE_SHIFT_IDX 1
#define KEY_CHANGE_KEYS_IDX  2

// Initialization levels
#define OB_COLD  0
#define OB_WARM  1
#define OB_READY 2

#ifdef LCD_SUPPORTED
  #define BUZZER_OFF  0
  #define BUZZER_ON   1
  #define BUZZER_BLIP 2
#endif

  #define VOLT_LEVEL_BAD      0
  #define VOLT_LEVEL_CAUTIOUS 1
  #define VOLT_LEVEL_GOOD     2

typedef struct
{
  osal_event_hdr_t hdr;
  uint8 state; // shift
  uint8 keys;  // keys
} keyChange_t;

/*********************************************************************
 * FUNCTIONS
 */

  /*
   * Initialize the Peripherals
   *    level: 0=cold, 1=warm, 2=ready
   */
  extern void InitBoard( uint8 level );

 /*
  * Get elapsed timer clock counts
  */
  extern uint32 TimerElapsed( void );

  /*
   * Register for all key events
   */
  extern uint8 RegisterForKeys( uint8 task_id );

/* Keypad Control Functions */

  /*
   * Send "Key Pressed" message to application
   */
  extern uint8 OnBoard_SendKeys( uint8 keys, uint8 shift );

/* Voltage Measurement Functions */

  /* Register a callback */
  extern void RegisterVoltageWarningCB( void (*pVoltWarnCB)(uint8) );

  /* Measure voltage and report */
  extern bool OnBoard_CheckVoltage( void );
  
/* LCD Emulation/Control Functions */
  /*
   * Convert an interger to an ascii string
   */
  extern void _itoa( uint16 num, uint8 *buf, uint8 radix );


  extern void Dimmer( uint8 lvl );

/* External I/O Processing Functions */
  /*
   * Turn on an external lamp
   */
  extern void BigLight_On( void );

  /*
   * Turn off an external lamp
   */
  extern void BigLight_Off( void );

  /*
   * Turn on/off an external buzzer
   *   on:   BUZZER_ON or BUZZER_OFF
   */
  extern void BuzzerControl( uint8 on );

  /*
   * Get setting of external dip switch
   */
  extern uint8 GetUserDipSw( void );

  /*
   * Calculate the size of used stack
   */
  extern uint16 OnBoard_stack_used( void );

  /*
   * Callback routine to handle keys
   */
  extern void OnBoard_KeyCallback ( uint8 keys, uint8 state );

  /*
   * Board specific random number generator
   */
  extern uint16 Onboard_rand( void );

  /*
   * Board specific micro-second wait
   */
  extern void Onboard_wait( uint16 timeout );

  /*
   * Board specific soft reset.
   */
  extern __near_func void Onboard_soft_reset( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ONBOARD_H
