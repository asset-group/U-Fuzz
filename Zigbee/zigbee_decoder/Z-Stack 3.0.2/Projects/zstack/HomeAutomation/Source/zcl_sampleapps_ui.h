 /**************************************************************************************************
  Filename:       zcl_sampleapps_ui.h
  Revised:        $Date: 2016-08-01 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the generic Sample Application User Interface.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_SAMPLEAPPS_UI_H
#define ZCL_SAMPLEAPPS_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * MACROS
 */
#define UI_STATE_BACK_FROM_APP_MENU 0

#define UI_STATE_DEFAULT_MOVE 254
#define UI_STATE_UNCHANGED 255

#define UI_KEY_AUTO_PRESSED 0xFFFF

#define UI_KEY_SW_1_PRESSED HAL_KEY_SW_1
#define UI_KEY_SW_2_PRESSED HAL_KEY_SW_2
#define UI_KEY_SW_3_PRESSED HAL_KEY_SW_3
#define UI_KEY_SW_4_PRESSED HAL_KEY_SW_4
#define UI_KEY_SW_5_PRESSED HAL_KEY_SW_5

#define UI_KEY_SW_1_RELEASED (HAL_KEY_SW_1 << 8)
#define UI_KEY_SW_2_RELEASED (HAL_KEY_SW_2 << 8)
#define UI_KEY_SW_3_RELEASED (HAL_KEY_SW_3 << 8)
#define UI_KEY_SW_4_RELEASED (HAL_KEY_SW_4 << 8)
#define UI_KEY_SW_5_RELEASED (HAL_KEY_SW_5 << 8)

#define UI_LED_APP HAL_LED_1

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint8 next_state; // state to move to when [RIGHT] is pressed
  uint8 prev_state; // state to move to when [LEFT] is changed
  uint16 keys_mask; // pressing the keys enabled in this bitmask will execute the state function (state_func)
  void (*state_func)(uint16); // the function to execute when any of the keys in keys_mask is pressed/released (according to the mask)
} uiState_t;

typedef void (* uiAppUpdateLcd_t)(uint8 uiCurrentState, char * line[3]);

/*********************************************************************
 * FUNCTIONAL MACROS
 */

//uiConstStrOver - copy src string (must be a constant quoted string) onto dst,
//without copying the null character that terminates src
#define uiConstStrOver(dst,src) do {osal_memcpy(dst, src, sizeof(src) - 1);} while (0) 

//uiConstStrCpy - copy src string (must be a constant quoted string) onto dst, 
//including copying the null character that terminates src
#define uiConstStrCpy(dst,src) do {osal_memcpy(dst, src, sizeof(src));} while (0)

/*********************************************************************
* FUNCTIONS
*/

/*
 * User-Interface intialization
 */
void UI_Init(uint8 app_task_id_value, uint16 lcd_auto_update_event_value, uint16 key_auto_repeat_event_value, uint16 * ui_IdentifyTimeAttribute_value, char * app_title_value, uiAppUpdateLcd_t _uiAppUpdateLcd, const uiState_t _uiAppStatesMain[]);

/*
 * Process commissioning status change
 */
void UI_UpdateComissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

/*
 * Update the LCD display
 */
void UI_UpdateLcd( void );

/*
 * Main UI state-machine
 */
void UI_MainStateMachine( uint16 keys );

/*
 * Menu-item action for returning from the application-specific menu to the common menu
 */
void UI_ActionBackFromAppMenu(uint16 keys);

/*
 * Process a change in the device's network-state
 */
void UI_DeviceStateUpdated(devStates_t NwkState);

#endif
