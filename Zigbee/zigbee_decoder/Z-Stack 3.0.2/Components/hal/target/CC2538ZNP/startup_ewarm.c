/**************************************************************************************************
  Filename:       startup_ewarm.c
  Revised:        $Date: 2013-10-25 13:41:32 -0700 (Fri, 25 Oct 2013) $
  Revision:       $Revision: 35804 $

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

#include "hal_assert.h"
#include "gpio.h"
#include "hw_memmap.h"

#if defined OTA_CLIENT
#define FLASH_START_ADDR 0x0027F800  // page for Image Boot Manager
#define BOOTLOADER_BACKDOOR_DISABLE     0xEFFFFFFF
#elif defined SBL_CLIENT
#define FLASH_START_ADDR 0x0027B000  // page for Serial Bootloader.
#define BOOTLOADER_BACKDOOR_DISABLE     0xEFFFFFFF
#else 
#define FLASH_START_ADDR 0x00200000
#define BOOTLOADER_BACKDOOR_DISABLE     0xEFFFFFFF
#endif

//
// This register defines the emulator override controls 
// for power mode and peripheral clock gate. 
//
#define SYS_CTRL_EMUOVR         0x400D20B4  
#define EMUOVR_PM0              0xFF


// Access content of Hardware registers
/*#define HWREG(x)                                                              \
        (*((volatile unsigned long *)(x))) */

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
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);
static void ResetIsr(void);
static void uDMAIntHandler(void);
static void uDMAErrIntHandler(void);   
//*****************************************************************************
//
// External declaration for the interrupt handler used by the application.
//
//*****************************************************************************
extern void halSleepTimerIsr(void);
extern void macMcuTimer2Isr(void);
extern void macMcuRfIsr(void);
extern void macMcuRfErrIsr(void);
extern void SysTickIntHandler(void);
extern void interrupt_keybd(void);
extern void interrupt_uart0(void);

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern void __iar_program_start(void);
extern void interrupt_uart1(void);

//*****************************************************************************
//
// Reserve space for the system stack.
//
//*****************************************************************************
#ifndef STACK_SIZE
#define STACK_SIZE  512  // Number of 32-bit words in pulStack[]
#endif
static unsigned long pulStack[STACK_SIZE] @ ".noinit";

#if ( ((!defined OTA_CLIENT) && (!defined SBL_CLIENT)) || (defined OTA_INITIAL_IMAGE) )
//*****************************************************************************
//
// Customer Configuration Area in Lock Page
// Holds Image Vector table address (bytes 2012 - 2015) and
// Image Valid bytes (bytes 2008 -2011)
//
//*****************************************************************************

// Create section for Customer configuration area in upper flash page
#pragma section = ".cca"

typedef struct
{
    unsigned long ulImageBackdoor;
    unsigned long ulImageValid;
    unsigned long ulImageVectorAddr;
} lockPageCCA_t;

__root const lockPageCCA_t __cca @ ".cca" =
{
  BOOTLOADER_BACKDOOR_DISABLE,  // Bootloader backdoor disabled
  0,                            // Image valid bytes
  FLASH_START_ADDR // Vector table located at flash start address
};
#endif

#ifdef OTA_INITIAL_IMAGE
typedef struct 
{
  unsigned long  imageCRC[2];
  unsigned long  nvicJump[2];
} ibmLedger_t;

typedef struct
{
  unsigned long ledgerSignature[4];
  ibmLedger_t initialBoot;
} ledgerInit_t;

__root const ledgerInit_t __ledgerInit @ ".ledger" =
{
  {0x01234567,
   0x89ABCDEF,
   0x02468ACE,
   0x13579BDF},  // place the recognizable signature pattern
  {0xFFFFFFFF,
   0xFFFFFFFF,   // tell-tale CRC pattern that signifies initial image
   (unsigned long)pulStack + sizeof(pulStack),  // initial stack pointer
   (unsigned long)&ResetIsr}                    // reset ISR
};

#endif

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
    ResetIsr,                               // 1 The reset handler
    NmiSR,                                  // 2 The NMI handler
    FaultISR,                               // 3 The hard fault handler
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
    SysTickIntHandler,                      // 15 The SysTick handler
    IntDefaultHandler,                      // 16 GPIO Port A
    IntDefaultHandler,                      // 17 GPIO Port B
    IntDefaultHandler,                      // 18 GPIO Port C
    IntDefaultHandler,                      // 19 GPIO Port D
    0,                                      // 20 none
    IntDefaultHandler,                      // 21 UART0 Rx and Tx
#if ((defined HAL_UART_USB)  || (defined HAL_UART_SPI)) 
    IntDefaultHandler,                      // 22 UART1 Rx and Tx
#else
    interrupt_uart1,                        // 22 UART1 Rx and Tx
#endif
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
    macMcuRfIsr,                            // 42 RFCore Rx/Tx
    macMcuRfErrIsr,                         // 43 RFCore Error
    IntDefaultHandler,                      // 44 IcePick
    IntDefaultHandler,                      // 45 FLASH Control
    IntDefaultHandler,                      // 46 AES
    IntDefaultHandler,                      // 47 PKA
    halSleepTimerIsr,                       // 48 Sleep Timer
    macMcuTimer2Isr,                        // 49 MacTimer
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
#ifdef SPI_DMA_CC2538    
    uDMAIntHandler,                         // 62 uDMA
    uDMAErrIntHandler,                      // 63 uDMA Error
#else
    IntDefaultHandler,
    0
#endif      
};

#ifdef SBL_CLIENT
extern const unsigned long __checksum_begin;
extern const unsigned long __checksum_end;
#define SBL_COMPATIBILITY_FLAGS 0xFFFFFFFF //for future use
__root const unsigned long sbl_header_after_crc[] @ ".image_status" =	{SBL_COMPATIBILITY_FLAGS, 0xA5A5A5A5, (unsigned long)&__checksum_begin, (unsigned long)&__checksum_end, (unsigned long)&__vector_table};
#endif
//*****************************************************************************
//
// This ISR handler gets called when the processor receives a Reset interrupt. 
// This handler calls a workaround and then calls start of application
//
//*****************************************************************************
static void ResetIsr(void)
{
  //
  // When Emulator is connected to device in PM1/2/3, emulator loses connection 
  // on next sleep. This workaround puts device in PM0 when connected to 
  // Emulator and prevents the connection loss even when emulator is 
  // connected to device during sleep.
  //
   HWREG(SYS_CTRL_EMUOVR) = EMUOVR_PM0; 
   
  //
  // The ‘pStopAtResetIsr’ variable is used for synchronization with the macro 
  // function. The C-SPY macro function set this to 0xA5F01248 before the 
  // system reset call, and to 0 after to avoid code execution beyond resetISR
  // before the workaround reset has completed. The RAM address was just picked 
  // randomly now. The timeout variable is used just to ensure the unlikely case 
  // where the value at the RAM address has the value 0xA5F01248 at startup  
  // Should only be used for debug builds, and not in production code
  //
#ifdef FEATURE_RESET_MACRO
  unsigned long timeout = 2000000; 
  volatile unsigned long* pStopAtResetIsr = (unsigned long*)0x20003000;
  while((*pStopAtResetIsr == 0xA5F01248) && (timeout--));
#endif
  __iar_program_start();
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
static void
NmiSR(void)
{
  halAssertHandler();
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
FaultISR(void)
{
  halAssertHandler();
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
  halAssertHandler();
}



