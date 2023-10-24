/**************************************************************************************************
  Filename:       OnBoard.c
  Revised:        $Date: 2012-11-15 11:48:16 -0800 (Thu, 15 Nov 2012) $
  Revision:       $Revision: 32196 $

  Description:    This file contains the UI and control for the
                  peripherals on the EVAL development board
  Notes:          This file targets the Chipcon CC2530/31


  Copyright 2005-2011 Texas Instruments Incorporated. All rights reserved.

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

#include "ZComDef.h"
#include "ZGlobals.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_SYS.h"
#include "DebugTrace.h"

/* Hal */
#include "hal_lcd.h"
#include "hal_mcu.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_led.h"
#include "hal_adc.h"

/* Allow access macRandomByte() */
#include "mac_radio_defs.h"

/*********************************************************************
 * CONSTANTS
 */

// Task ID not initialized
#define NO_TASK_ID 0xFF

// Minimum length RAM "pattern" for Stack check
#define MIN_RAM_INIT 12

/*********************************************************************
 * GLOBAL VARIABLES
 */

#if defined MAKE_CRC_SHDW
#pragma location="CRC_SHDW"
const CODE uint16 _crcShdw = 0xFFFF;
#pragma required=_crcShdw

#elif !defined MT_UBL_FUNC

#pragma location="LOCK_BITS_ADDRESS_SPACE"
__no_init uint8 _lockBits[16];
#pragma required=_lockBits

#if defined ZCL_KEY_ESTABLISH
#include "zcl_cert_data.c"
#else
#pragma location="IEEE_ADDRESS_SPACE"
__no_init uint8 _nvIEEE[Z_EXTADDR_LEN];
#pragma required=_nvIEEE
#endif

#pragma location="RESERVED_ADDRESS_SPACE"
__no_init uint8 _reserved[1932];
#pragma required=_reserved
#endif

// 64-bit Extended Address of this device
uint8 aExtendedAddress[8];

/*********************************************************************
 * LOCAL VARIABLES
 */

// Registered keys task ID, initialized to NOT USED.
static uint8 registeredKeysTaskID = NO_TASK_ID;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void ChkReset( void );
// function pointer for low voltage warning callback
static void (*gpLowVoltageWarning)( uint8 voltLevel ) = (void*) NULL;

/*********************************************************************
 * @fn      InitBoard()
 * @brief   Initialize the CC2420DB Board Peripherals
 * @param   level: COLD,WARM,READY
 * @return  None
 */
void InitBoard( uint8 level )
{
  if ( level == OB_COLD )
  {
    // IAR does not zero-out this byte below the XSTACK.
    *(uint8 *)0x0 = 0;
    // Interrupts off
    osal_int_disable( INTS_ALL );
    // Check for Brown-Out reset
    ChkReset();
  }
  else  // !OB_COLD
  {
    /* Initialize Key stuff */
    #if defined (ISR_KEYINTERRUPT)
    HalKeyConfig(HAL_KEY_INTERRUPT_ENABLE , OnBoard_KeyCallback);
    #else
    HalKeyConfig(HAL_KEY_INTERRUPT_DISABLE, OnBoard_KeyCallback);
    #endif
  }
}

/*********************************************************************
 * @fn      ChkReset()
 * @brief   Check reset bits - if reset cause is unknown, assume a
 *          brown-out (low power), assume batteries are not reliable,
 *          hang in a loop and sequence through the LEDs.
 * @param   None
 * @return  None
 *********************************************************************/
void ChkReset( void )
{
  uint8 rib;

  // Isolate reset indicator bits
  rib = SLEEPSTA & LRESET;

  if ( rib == RESETPO )
  {
    // Put code here to handle Power-On reset
  }
  else if ( rib == RESETEX )
  {
    // Put code here to handle External reset
  }
  else if ( rib == RESETWD )
  {
    // Put code here to handle WatchDog reset
  }
  else  // Unknown reason - not expected.
  {
    HAL_ASSERT(0);
  }
}

/*********************************************************************
 *                        "Keyboard" Support
 *********************************************************************/

/*********************************************************************
 * Keyboard Register function
 *
 * The keyboard handler is setup to send all keyboard changes to
 * one task (if a task is registered).
 *
 * If a task registers, it will get all the keys. You can change this
 * to register for individual keys.
 *********************************************************************/
uint8 RegisterForKeys( uint8 task_id )
{
  // Allow only the first task
  if ( registeredKeysTaskID == NO_TASK_ID )
  {
    registeredKeysTaskID = task_id;
    return ( true );
  }
  else
    return ( false );
}

/*********************************************************************
 * @fn      OnBoard_SendKeys
 *
 * @brief   Send "Key Pressed" message to application.
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  status
 *********************************************************************/
uint8 OnBoard_SendKeys( uint8 keys, uint8 state )
{
  keyChange_t *msgPtr;

  if ( registeredKeysTaskID != NO_TASK_ID )
  {
    // Send the address to the task
    msgPtr = (keyChange_t *)osal_msg_allocate( sizeof(keyChange_t) );
    if ( msgPtr )
    {
      msgPtr->hdr.event = KEY_CHANGE;
      msgPtr->state = state;
      msgPtr->keys = keys;

      osal_msg_send( registeredKeysTaskID, (uint8 *)msgPtr );
    }
    return ( ZSuccess );
  }
  else
    return ( ZFailure );
}

/*********************************************************************
 * @fn      OnBoard_KeyCallback
 *
 * @brief   Callback service for keys
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  void
 *********************************************************************/
void OnBoard_KeyCallback ( uint8 keys, uint8 state )
{
  uint8 shift;
  (void)state;

  shift = (keys & HAL_KEY_SW_6) ? true : false;

  if ( OnBoard_SendKeys( keys, shift ) != ZSuccess )
  {
    // Process SW1 here
    if ( keys & HAL_KEY_SW_1 )  // Switch 1
    {
    }
    // Process SW2 here
    if ( keys & HAL_KEY_SW_2 )  // Switch 2
    {
    }
    // Process SW3 here
    if ( keys & HAL_KEY_SW_3 )  // Switch 3
    {
    }
    // Process SW4 here
    if ( keys & HAL_KEY_SW_4 )  // Switch 4
    {
    }
    // Process SW5 here
    if ( keys & HAL_KEY_SW_5 )  // Switch 5
    {
    }
    // Process SW6 here
    if ( keys & HAL_KEY_SW_6 )  // Switch 6
    {
    }
  }
}

/*********************************************************************
 *                  Low Voltage Protectiion Support
 *********************************************************************/

/*********************************************************************
 * @fn      RegisterVoltageWarningCB
 *
 * @brief   Register Low Voltage Warning Callback
 *
 * @param   pVoltWarnCB - fundion pointer of the callback
 *
 * @return  none
 *********************************************************************/
void RegisterVoltageWarningCB( void (*pVoltWarnCB)(uint8) )
{
  gpLowVoltageWarning = pVoltWarnCB;
}

/*********************************************************************
 * @fn      OnBoard_CheckVoltage
 *
 * @brief   Check voltage and notify the callback of the status
 *
 * @param   none
 *
 * @return  TRUE  - The voltage is good for NV writing
 *          FALSE - The voltage is not high enough for NV writing
 *********************************************************************/
bool OnBoard_CheckVoltage( void )
{
  uint8 voltageMeasured;
  uint8 howGood;

  voltageMeasured = HalAdcCheckVddRaw();

  if ( voltageMeasured > VDD_MIN_GOOD )
  {
    howGood = VOLT_LEVEL_GOOD;
  }
  else if ( voltageMeasured > VDD_MIN_NV )
  {
    howGood = VOLT_LEVEL_CAUTIOUS;
  }
  else
  {
    howGood = VOLT_LEVEL_BAD;
  }
    
  if ( gpLowVoltageWarning )
  {
    if ( howGood < VOLT_LEVEL_GOOD )
    {
      gpLowVoltageWarning( howGood );
    }
  }

  return ( howGood > VOLT_LEVEL_BAD );
}

/*********************************************************************
 * @fn      OnBoard_stack_used
 *
 * @brief   Runs through the stack looking for touched memory.
 *
 * @param   none
 *
 * @return  Maximum number of bytes used by the stack.
 *********************************************************************/
uint16 OnBoard_stack_used(void)
{
  uint8 const *ptr;
  uint8 cnt = 0;

  for (ptr = CSTACK_END; ptr > CSTACK_BEG; ptr--)
  {
    if (STACK_INIT_VALUE == *ptr)
    {
      if (++cnt >= MIN_RAM_INIT)
      {
        ptr += MIN_RAM_INIT;
        break;
      }
    }
    else
    {
      cnt = 0;
    }
  }

  return (uint16)(CSTACK_END - ptr + 1);
}

/*********************************************************************
 * @fn      _itoa
 *
 * @brief   convert a 16bit number to ASCII
 *
 * @param   num -
 *          buf -
 *          radix -
 *
 * @return  void
 *
 *********************************************************************/
void _itoa(uint16 num, uint8 *buf, uint8 radix)
{
  char c,i;
  uint8 *p, rst[5];

  p = rst;
  for ( i=0; i<5; i++,p++ )
  {
    c = num % radix;  // Isolate a digit
    *p = c + (( c < 10 ) ? '0' : '7');  // Convert to Ascii
    num /= radix;
    if ( !num )
      break;
  }

  for ( c=0 ; c<=i; c++ )
    *buf++ = *p--;  // Reverse character order

  *buf = '\0';
}

/*********************************************************************
 * @fn        Onboard_rand
 *
 * @brief    Random number generator
 *
 * @param   none
 *
 * @return  uint16 - new random number
 *
 *********************************************************************/
uint16 Onboard_rand( void )
{
  return ( MAC_RADIO_RANDOM_WORD() );
}

/*********************************************************************
 * @fn        Onboard_wait
 *
 * @brief    Delay wait
 *
 * @param   uint16 - time to wait
 *
 * @return  none
 *
 *********************************************************************/
void Onboard_wait( uint16 timeout )
{
  while (timeout--)
  {
    asm("NOP");
    asm("NOP");
    asm("NOP");
  }
}

/*********************************************************************
 * @fn      Onboard_soft_reset
 *
 * @brief   Effect a soft reset.
 *
 * @param   none
 *
 * @return  none
 *
 *********************************************************************/
__near_func void Onboard_soft_reset( void )
{
  HAL_DISABLE_INTERRUPTS();
  // Abort all DMA channels to insure that ongoing operations do not
  // interfere with re-configuration.
  DMAARM = 0x80 | 0x1F;
  asm("LJMP 0x0");
}

#if defined MT_UBL_FUNC
#if !defined UBL_RC_IMG_PG_BEG
#define UBL_RC_IMG_PG_BEG  (HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE)
#endif
#if !defined UBL_MD_PG_OFFSET
#define UBL_MD_PG_OFFSET   (HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE + 2)
#endif
#if !defined UBL_ADDR
#define UBL_MD_ADDR        (HAL_SB_CRC_ADDR + 2)
#endif

/*********************************************************************
 * @fn      MT_UblCommandProcessing
 *
 * @brief   Effect a soft reset.
 *
 * @param   pBuf - Pointer to the received MT buffer.
 *
 * @return  MT_RPC_ERR_COMMAND_ID if the MT command is not MT_UBL_ERASE.
 *
 *********************************************************************/
uint8 MT_UblCommandProcessing(uint8 *pBuf);
uint8 MT_UblCommandProcessing(uint8 *pBuf)
{
  if (pBuf[MT_RPC_POS_CMD1] == MT_UBL_ERASE)
  {
    uint16 crc[2];
    HalFlashRead(UBL_RC_IMG_PG_BEG, UBL_MD_PG_OFFSET, (uint8 *)(crc+1), 2);

    if (crc[1] != 0)
    {
      crc[1] ^= 0xFFFF;  // Only write to zero bits that are not already zero.
      crc[0] = 0xFFFF;
      HalFlashWrite((UBL_MD_ADDR - 2) / HAL_FLASH_WORD_SIZE, (uint8 *)crc, 1);
    }

    Onboard_soft_reset();
  }

  return MT_RPC_ERR_COMMAND_ID;
}
#endif

/*********************************************************************
 *                    EXTERNAL I/O FUNCTIONS
 *
 * User defined functions to control external devices. Add your code
 * to the following functions to control devices wired to DB outputs.
 *
 *********************************************************************/

void BigLight_On( void )
{
  // Put code here to turn on an external light
}

void BigLight_Off( void )
{
  // Put code here to turn off an external light
}

void BuzzerControl( uint8 on )
{
  // Put code here to turn a buzzer on/off
  (void)on;
}

void Dimmer( uint8 lvl )
{
  // Put code here to control a dimmer
  (void)lvl;
}

// No dip switches on this board
uint8 GetUserDipSw( void )
{
  return 0;
}

/*********************************************************************
*********************************************************************/
