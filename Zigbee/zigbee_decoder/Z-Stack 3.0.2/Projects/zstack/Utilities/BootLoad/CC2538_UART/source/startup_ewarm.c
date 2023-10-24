/**************************************************************************************************
  Filename:       startup_ewarm.c
  Revised:        $Date: 2013-06-27 15:44:38 -0700 (Thu, 27 Jun 2013) $
  Revision:       $Revision: 34663 $

  Description:    Startup file for the CC2538 SBL.


  Copyright 2012-2013 Texas Instruments Incorporated. All rights reserved.

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

#include "hal_mcu.h"

//*****************************************************************************
//
// Enable the IAR extensions for this source file.
//
//*****************************************************************************
#pragma language=extended

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern void __iar_program_start(void);

//*****************************************************************************
//
// Definitions for boot loader backdoor configuration.
//
//****************************************************************************
#define BACKDOOR_ENABLE_BIT       0x10
#define BACKDOOR_PIN_LEVEL_HIGH   0x08
#define BACKDOOR_PIN_NUMBER_MASK  0x07

typedef struct
{
  unsigned char ulImageBackdoor[4];  // Note that the backdoor configuration is in the MSB.
  unsigned long ulImageValid;
  unsigned long ulImageVectorAddr;
} lockPageCCA_t;

__no_init unsigned long cstack @ 0x20000800;  // Mark the address of the C-code call stack.

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
} uVectorEntry;

//*****************************************************************************
//
// The minimal vector table for a Cortex M3.
//
//*****************************************************************************
__root const uVectorEntry __vector_table[] @ ".intvec" =
{
  { .ulPtr = (unsigned long)&cstack },  // 0 The initial stack pointer.
  __iar_program_start                   // 1 The reset handler.
};


// Create section for Customer configuration area in upper flash page.
__root const lockPageCCA_t __cca @ ".cca" =
{
  // If required use the following values to enable the backdoor for 
  // Port-A, Pin-7, active low within 10-usec after reset to force boot.
  // { 0xFF, 0xFF, 0xFF, BACKDOOR_ENABLE_BIT | 0x07},
	
  {0,0,0,0},                    // Initial values, backdoor disabled  
  0x00000000,                   // Image valid field.
  (unsigned long)__vector_table // FLASH_BASE    
                                // Vector table located at flash start address.
};


/*******************************************************************************************************
*/
