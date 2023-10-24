/**************************************************************************************************
  Filename:       startup_ewarm.c
  Revised:        $Date: 2013-05-24 19:19:25 -0700 (Fri, 24 May 2013) $
  Revision:       $Revision: 34433 $

  Description:    Startup file for cc2538


  Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

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

#include "hal_board_cfg.h"

#define BOOTLOADER_BACKDOOR_DISABLE 0xEFFFFFFF
#define FLASH_START_ADDR 0x0027F800

// Global variable .. critical section depth
unsigned long rtosCSDepth = 0;

//*****************************************************************************
//
// Enable the IAR extensions for this source file.
//
//*****************************************************************************
#pragma language=extended

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
static void IntDefaultHandler(void);

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern void __iar_program_start(void);

//*****************************************************************************
//
// Reserve space for the system stack.
//
//*****************************************************************************
#ifndef STACK_SIZE
#define STACK_SIZE                              200
#endif
static unsigned long pulStack[STACK_SIZE] @ ".noinit";

//*****************************************************************************
//
// A union that describes the entries of the vector table.  The union is needed
// since the first entry is the stack pointer and the remainder are function
// pointers.
//
//*****************************************************************************
typedef union
{
    void (*pfnHandler)(void);
    unsigned long ulPtr;
}
uVectorEntry;

//*****************************************************************************
//
// The minimal vector table for a Cortex M3.  Note that the proper constructs
// must be placed on this to ensure that it ends up at physical address
// 0x0020.0000 (flash start address)
//
//*****************************************************************************
__root const uVectorEntry __vector_table[] @ ".intvec" =
{
    { .ulPtr = (unsigned long)pulStack + sizeof(pulStack) },
                                            // 0 The initial stack pointer
    __iar_program_start,                    // 1 The reset handler
    IntDefaultHandler,                      // 2 The NMI handler
    IntDefaultHandler,                      // 3 The hard fault handler
    IntDefaultHandler,                      // 4 The MPU fault handler
    IntDefaultHandler,                      // 5 The bus fault handler
    IntDefaultHandler,                      // 6 The usage fault handler
    0,                                      // 7 Reserved
    0,                                      // 8 Reserved
    0,                                      // 9 Reserved
    0,                                      // 10 Reserved
    IntDefaultHandler,                      // 11 SVCall handler
    IntDefaultHandler,                      // 12 Debug monitor handler
    0,                                      // 13 Reserved
    IntDefaultHandler,                      // 14 The PendSV handler
    IntDefaultHandler,                      // 15 The SysTick handler
    IntDefaultHandler,                      // 16 GPIO Port A
    IntDefaultHandler,                      // 17 GPIO Port B
    IntDefaultHandler,                      // 18 GPIO Port C
    IntDefaultHandler,                      // 19 GPIO Port D
    0,                                      // 20 none
    IntDefaultHandler,                      // 21 UART0 Rx and Tx
    IntDefaultHandler,                      // 22 UART1 Rx and Tx
    IntDefaultHandler,                      // 23 SSI0 Rx and Tx
    IntDefaultHandler,                      // 24 I2C Master and Slave
    0,                                      // 25 Reserved
    0,                                      // 26 Reserved
    0,                                      // 27 Reserved
    0,                                      // 28 Reserved
    0,                                      // 29 Reserved
    IntDefaultHandler,                      // 30 ADC Sequence 0
    0,                                      // 31 Reserved
    0,                                      // 32 Reserved
    0,                                      // 33 Reserved
    IntDefaultHandler,                      // 34 Watchdog timer, timer 0
    IntDefaultHandler,                      // 35 Timer 0 subtimer A
    IntDefaultHandler,                      // 36 Timer 0 subtimer B
    IntDefaultHandler,                      // 37 Timer 1 subtimer A
    IntDefaultHandler,                      // 38 Timer 1 subtimer B
    IntDefaultHandler,                      // 39 Timer 2 subtimer A
    IntDefaultHandler,                      // 40 Timer 2 subtimer B
    IntDefaultHandler,                      // 41 Analog Comparator 0
    IntDefaultHandler,                      // 42 RFCore Rx/Tx
    IntDefaultHandler,                      // 43 RFCore Error
    IntDefaultHandler,                      // 44 IcePick
    IntDefaultHandler,                      // 45 FLASH Control
    IntDefaultHandler,                      // 46 AES
    IntDefaultHandler,                      // 47 PKA
    IntDefaultHandler,                      // 48 Sleep Timer
    IntDefaultHandler,                      // 49 MacTimer
    IntDefaultHandler,                      // 50 SSI1 Rx and Tx
    IntDefaultHandler,                      // 51 Timer 3 subtimer A
    IntDefaultHandler,                      // 52 Timer 3 subtimer B
    0,                                      // 53 Reserved
    0,                                      // 54 Reserved
    0,                                      // 55 Reserved
    0,                                      // 56 Reserved
    0,                                      // 57 Reserved
    0,                                      // 58 Reserved
    0,                                      // 59 Reserved
    IntDefaultHandler,                      // 60 USB 2538
    0,                                      // 61 Reserved
    IntDefaultHandler,                      // 62 uDMA
    IntDefaultHandler,                      // 63 uDMA Error
};

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected interrupt.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
  HAL_SYSTEM_RESET();
}

