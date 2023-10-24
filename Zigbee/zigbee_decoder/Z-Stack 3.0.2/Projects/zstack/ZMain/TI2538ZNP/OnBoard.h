/**************************************************************************************************
  Filename:       OnBoard.h
  Revised:        $Date: 2013-11-14 15:32:37 -0800 (Thu, 14 Nov 2013) $
  Revision:       $Revision: 36109 $

  Description:   Defines stuff for Evaluation boards
  Notes:         This file targets the Texas Instruments CC2538 family


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ONBOARD_H
#define ONBOARD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include <intrinsics.h>

#include "hal_mcu.h"
#include "hal_uart.h"
#include "hal_sleep.h"
#include "osal.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */

/* 64-bit Extended Address of this device */
extern uint8 aExtendedAddress[8];

/*********************************************************************
 * CONSTANTS
 */

/* Timer clock and power-saving definitions */
#define TICK_COUNT         1  /* TIMAC requires this number to be 1 */

/* OSAL timer defines */
#define TICK_TIME   1000   /* Timer per tick - in micro-sec */

/* These Key definitions are unique to this development system.
 * They are used to bypass functions when starting up the device.
 */
#define SW_BYPASS_NV    HAL_KEY_SW_2  /* Bypass Network layer NV restore*/
#define SW_BYPASS_START HAL_KEY_SW_1  /* Bypass Network initialization */

/* LCD Support Defintions */
#ifdef LCD_SUPPORTED
  #if !defined DEBUG
    #define DEBUG  0
  #endif
  #if LCD_SUPPORTED==DEBUG
    #define SERIAL_DEBUG_SUPPORTED  /* Serial-debug */
  #endif
#else /* No LCD support */
  #undef SERIAL_DEBUG_SUPPORTED  /* No serial-debug */
#endif

/* Serial Port Definitions */
#if defined (ZAPP_P1)
  #define ZAPP_PORT HAL_UART_PORT_0 /*SERIAL_PORT1 */
#elif defined (ZAPP_P2)
  #define ZAPP_PORT HAL_UART_PORT_1 /*SERIAL_PORT2 */
#else
  #undef ZAPP_PORT
#endif
#if defined (ZTOOL_P1)
  #define ZTOOL_PORT HAL_UART_PORT_0 /*SERIAL_PORT1 */
#elif defined (ZTOOL_P2)
  #define ZTOOL_PORT HAL_UART_PORT_1 /*SERIAL_PORT2 */
#else
  #undef ZTOOL_PORT
#endif

/* Tx and Rx buffer size defines used by SPIMgr.c */
#define MT_UART_TX_BUFF_MAX  170
#define MT_UART_RX_BUFF_MAX  120
#define MT_UART_THRESHOLD    5
#define MT_UART_IDLE_TIMEOUT 5

#if !defined HAL_UART_PORT
#define HAL_UART_PORT              0
#endif
/* SOC defines the ideal sizes in the 
 * individual _hal_uart_dma/isr.c modules.
 */
#define HAL_UART_FLOW_THRESHOLD    5
#define HAL_UART_RX_BUF_SIZE       170
#define HAL_UART_TX_BUF_SIZE       120
#define HAL_UART_IDLE_TIMEOUT      5

/* Restart system from absolute beginning
 * Disables interrupts, forces WatchDog reset
 */
#define SystemReset()  HAL_SYSTEM_RESET()

/* Reset reason for reset indication */
#define ResetReason() (0)

#define BootLoader()   /* Not yet implemented */

/* Power conservation */
#define OSAL_SET_CPU_INTO_SLEEP(timeout) halSleep(timeout);  /* Called from OSAL_PwrMgr */


/* Internal (MCU) RAM addresses */
#define MCU_RAM_BEG 0x1100
#define MCU_RAM_END 0x20FF
#define MCU_RAM_LEN (MCU_RAM_END - MCU_RAM_BEG + 1)


#ifdef __IAR_SYSTEMS_ICC__
/* Internal (MCU) Stack addresses */
#define CSTACK_BEG ((uint8 const *)(_Pragma("segment=\"CSTACK\"") __segment_begin("CSTACK")))
#define CSTACK_END ((uint8 const *)(_Pragma("segment=\"CSTACK\"") __segment_end("CSTACK"))-1)
/* Stack Initialization Value */
#define STACK_INIT_VALUE  0xCD
#else
#error Check compiler compatibility.
#endif


/* The following Heap sizes are setup for typical TI sample applications,
 * and should be adjusted to your systems requirements.
 */
/* Internal (MCU) heap size */
#if !defined( INT_HEAP_LEN )
  #define INT_HEAP_LEN  6144  /* 6.0K */
#endif

/* Memory Allocation Heap */
#define MAXMEMHEAP INT_HEAP_LEN  /* Typically, 1.0-6.0K */

/* Initialization levels */
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

#if defined (HAL_UART_USB)
extern uint32 softReset;
#define SystemResetSoft()                                             \
do{                                                                   \
  HAL_DISABLE_INTERRUPTS();                                           \
  softReset = SOFT_RESET;                                             \
  HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);  \
}while(0)
#else
#define SystemResetSoft() SystemReset()
#endif

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  osal_event_hdr_t hdr;
  uint8 state; /* shift */
  uint8 keys;  /* keys */
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
  extern uint8 OnBoard_SendKeys( uint8 keys, uint8 state );
  
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

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ONBOARD_H */
