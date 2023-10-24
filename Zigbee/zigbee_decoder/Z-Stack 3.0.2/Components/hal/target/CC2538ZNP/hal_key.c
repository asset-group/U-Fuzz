/**************************************************************************************************
  Filename:       hal_key.c
  Revised:        $Date: 2014-12-05 13:07:19 -0800 (Fri, 05 Dec 2014) $
  Revision:       $Revision: 41365 $

  Description:    This file contains the interface to the HAL KEY Service.


  Copyright 2012-2014 Texas Instruments Incorporated. All rights reserved.

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

/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_types.h"
#include "hal_key.h"
#include "hal_sleep.h"
#include "osal.h"
#include "OnBoard.h"
#include "hal_drivers.h"
#include "hal_mcu.h"

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
#define HAL_KEY_WAKE_INIT()

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
#if (HAL_KEY == TRUE)
static uint8 halSavedKeys;
static uint8 halIntKeys;
static halKeyCBack_t pHal_KeyProcessFunction;
bool Hal_KeyIntEnable;
#endif /* HAL_KEY */

/**************************************************************************************************
 *                                        EXTERNAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                        FUNCTIONS - API
 **************************************************************************************************/
void interrupt_keybd(void);
uint8 hal_key_no_debounce(void);

/**************************************************************************************************
 * @fn      HalKeyInit
 *
 * @brief   Initilize Key Service
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HalKeyInit( void )
{
#if (HAL_KEY == TRUE)
  /* Initialize previous key to 0 */
  halSavedKeys = 0;

  /* Initialize callback function */
  pHal_KeyProcessFunction  = NULL;
#endif /* HAL_KEY */
}

/**************************************************************************************************
* @fn      hal_key_keys()
*
* @brief   Determine if key was pressed and which key was pressed
*
* @param   none
*
* @return  0 if no key pressed, HAL_KEY_SW bit(s) if pressed
**************************************************************************************************/
uint8 hal_key_keys(void)                                           
{                                                                 
  uint8 x = 0;
  uint8 ucKeysPressed = bspKeyPushed(BSP_KEY_ALL);
  
  if(ucKeysPressed & BSP_KEY_LEFT)
  {
    x |= HAL_KEY_SW_4;
  }
  if(ucKeysPressed & BSP_KEY_RIGHT)
  {
    x |= HAL_KEY_SW_2;
  }
  if(ucKeysPressed & BSP_KEY_UP)
  {
    x |= HAL_KEY_SW_1;
  }
  if(ucKeysPressed & BSP_KEY_DOWN)
  {
    x |= HAL_KEY_SW_3;
  }
  if(ucKeysPressed & BSP_KEY_SELECT)
  {
    x |= HAL_KEY_SW_5;
  }
  return x; 
}

/**************************************************************************************************
* @fn      hal_key_int_keys()
*
* @brief   Determine if key was and which key was pressed during interrupt
*
* @param   None
*
* @return  0 if no key pressed, HAL_KEY_SW bit(s) if pressed
**************************************************************************************************/
uint8 hal_key_int_keys(void)                                      
{ 
  uint8 x = 0;
  /* Get bitmask of buttons pushed (clear directional keys' bitmask) */
  uint8 ucKeysPressed;
  
  ucKeysPressed = bspKeyPushed(BSP_KEY_DIR_ALL);
  if(ucKeysPressed & BSP_KEY_LEFT)
  {
    x |= HAL_KEY_SW_4;
  }
  if(ucKeysPressed & BSP_KEY_RIGHT)
  {
    x |= HAL_KEY_SW_2;
  }
  if(ucKeysPressed & BSP_KEY_UP)
  {
    x |= HAL_KEY_SW_1;
  }
  if(ucKeysPressed & BSP_KEY_DOWN)
  {
    x |= HAL_KEY_SW_3;
  }
  
  ucKeysPressed = bspKeyPushed(BSP_KEY_SELECT);
  if(ucKeysPressed & BSP_KEY_SELECT)
  {
    x |= HAL_KEY_SW_5;
  }
  return x;
}

/**************************************************************************************************
* @fn      HalKeyConfig
*
* @brief   Configure the Key serivce
*
* @param   interruptEnable - TRUE/FALSE, enable/disable interrupt
*          cback - pointer to the CallBack function
*
* @return  None
**************************************************************************************************/
void HalKeyConfig( bool interruptEnable, halKeyCBack_t cback)
{
#if (HAL_KEY == TRUE)
  /* Enable/Disable Interrupt or */
  Hal_KeyIntEnable = interruptEnable;
  
  /* Register the callback fucntion */
  pHal_KeyProcessFunction = cback;  
  
  /* Determine if interrupt is enable or not */
  if ( Hal_KeyIntEnable )
  {
    
    /* Initialize key handler to use interrupts */
    bspKeyInit(BSP_KEY_MODE_ISR);
    
    /* Map function dirKeyIsr to UP, LEFT, RIGHT and DOWN keys */
    bspKeyIntRegister((BSP_KEY_UP|BSP_KEY_LEFT|BSP_KEY_RIGHT|BSP_KEY_DOWN),
                      &interrupt_keybd);
    
    /* Map function selectKeyIsr to SELECT key */
    bspKeyIntRegister(BSP_KEY_SELECT, &interrupt_keybd);
    
    /* Enable interrupts on all keys */
    bspKeyIntEnable(BSP_KEY_ALL);
    
    IntPrioritySet(INT_GPIOC, HAL_INT_PRIOR_KEY);              
    IntPrioritySet(INT_GPIOA, HAL_INT_PRIOR_KEY);     
    
    /* Cancel polling if there is one active */
    osal_stop_timerEx(Hal_TaskID, HAL_KEY_EVENT);
  }
  else
  {
    bspKeyInit(BSP_KEY_MODE_POLL);
    
    if( cback != NULL)
    {
      /* Start polling if callback function is setup*/
      osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
    }
  }
#endif /* HAL_KEY */
}

/**************************************************************************************************
 * @fn      HalKeyRead
 *
 * @brief   Read the current value of a key
 *
 * @param   None
 *
 * @return  0 if no key pressed, HAL_KEY_SW bit(s) if pressed
 **************************************************************************************************/
uint8 HalKeyRead( void )
{
  uint8 keys = 0;

#if (HAL_KEY == TRUE)
  if (!Hal_KeyIntEnable)
  {
    keys = hal_key_keys();
  }
  else
  {
    keys = hal_key_int_keys();
  }
#endif /* HAL_KEY */

  return keys;
}

/**************************************************************************************************
 * @fn      HalKeyPoll
 *
 * @brief   Send call back if key(s) is pressed
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalKeyPoll( void )
{
#if (HAL_KEY == TRUE)
  uint8 keys = 0;
  
  /* if polling is using */
  if (!Hal_KeyIntEnable)
  {
    /* Get keys */
    keys = hal_key_keys ();

    /* If same as before, no keys */
    if ( keys == halSavedKeys )
    {
      return;
    }

    /* Store the current keys for comparation next time */
    halSavedKeys = keys;

  }
  
  /* Callback */
  if (keys && (pHal_KeyProcessFunction))
  {
    (pHal_KeyProcessFunction) (keys, HAL_KEY_STATE_NORMAL);
  }
  
#endif /* HAL_KEY */
}

#ifdef POWER_SAVING
/**************************************************************************************************
 * @fn      HalKeyEnterSleep
 *
 * @brief   Get called to enter sleep mode
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalKeyEnterSleep ( void )
{
  /* nothing to do */
}

/**************************************************************************************************
* @fn      hal_key_no_debounce()
*
* @brief   Determine if key is pressed and which key is pressed
*
* @param   None
*
* @return  0 if no key pressed, HAL_KEY_SW bit(s) if pressed
**************************************************************************************************/

uint8 hal_key_no_debounce(void)
{
  uint8 x = 0;
  uint8 dirPins, selPin;
  
  dirPins = (~GPIOPinRead(BSP_KEY_DIR_BASE,
                          BSP_KEY_DIR_ALL)) \
                          & BSP_KEY_DIR_ALL;
  selPin  = (~GPIOPinRead(BSP_KEY_SEL_BASE,
                          BSP_KEY_SELECT)) \
                          & BSP_KEY_SELECT;
  if(dirPins & BSP_KEY_LEFT)
  {
    x |= HAL_KEY_SW_4;
  }
  if(dirPins & BSP_KEY_RIGHT)
  {
    x |= HAL_KEY_SW_2;
  }
  if(dirPins & BSP_KEY_UP)
  {
    x |= HAL_KEY_SW_1;
  }
  if(dirPins & BSP_KEY_DOWN)
  {
    x |= HAL_KEY_SW_3;
  }
  if(selPin & BSP_KEY_SELECT)
  {
    x |= HAL_KEY_SW_5;
  }
  return x;
}

/**************************************************************************************************
 * @fn      HalKeyExitSleep
 *
 * @brief   Get called when sleep is over
 *
 * @param   None
 *
 * @return  0 if no key pressed, HAL_KEY_SW bit(s) if pressed
 **************************************************************************************************/
uint8 HalKeyExitSleep ( void )
{
  uint8 keys = 0;

  /* Get keys */
  if (!Hal_KeyIntEnable)
  {
    keys = hal_key_no_debounce();
  }
  else
  {
    keys = hal_key_int_keys();
  }
  return ( keys );
}
#endif /* POWER_SAVING */

/**************************************************************************************************
 * @fn      INTERRUPT_KEYBD
 *
 * @brief   Interrupt Service Routine for keyboard
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void interrupt_keybd(void)
{
  /* Clear the Power interrupt registers */
  SysCtrlPowIntClear();
  
#if (HAL_KEY == TRUE)
  /* Read the key before it gone */
  halIntKeys = hal_key_int_keys();
  
  if (halIntKeys && (pHal_KeyProcessFunction))
  {
    (pHal_KeyProcessFunction) (halIntKeys, HAL_KEY_STATE_NORMAL);
  }
#endif /* HAL_KEY */
  
  CLEAR_SLEEP_MODE();
}

/**************************************************************************************************
**************************************************************************************************/
