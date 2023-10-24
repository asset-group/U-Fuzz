/**************************************************************************************************
  Filename:       zcl_sampleapps_ui.c
  Revised:        $Date: 2016-8-1 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Z-Stack Sample Application User Interface.


  Copyright 2006-2016 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
  This file implements the user interface that is common to most of the Z-Stack sample applications.

  The sample applications are intended for the following platforms:
  - CC2530EM+SmartRF05
  - CC2538EM+SmartRF06

  The UI peripherals being used:

  - LCD based menu system:
    only using 3 lines of 16 characters each, for compatibility accross platforms.

  - Switches: 
    Using 4 directional switches and one selection switch. On SmartRF05, these are implemented by
    the joystick.

    - The switches have the following functionality, where applicable:
      - Left/Right: depending on the menu screen, these are used for either:
        - move to the previous/next menu screen
        - move to the previous/next digit/sub-item within a menu screen
      - Up/Down: change the value of the currently selected item
      - Ok (Select): execute the operation associated with the current menu screeen

  - LEDs:
    LED1 is used differently by the individual applications.
    the 3 other LEDs are common accross all the applications, and function as follows:

    - LED2: device state and type
      - Off: not connected to network
      - Constantly on: connected to the network as an end device
      - Blinking, 4 seconds period, 95% duty cycle: connected to the network as a router
      - Blinking, 4 seconds period, 75% duty cycle: connected to the network as a coordinator

    - LED3: identify status
      - Off: device is not identifying
      - Blinking, 1 second period, 50% duty cycle: device is identifying

    - LED4: open for joining status
      - Off: other devices cannot join through this device
      - Blinking, 1 second period, 25% duty cycle: other devices can join using standard comissioning
      - Blinking, 1 second period, 75% duty cycle: other devices can join using touchlink (i.e
        touchlink is enabled on the current device as a target)
      - On: device can join using either standard comissioning or touchlink

  The menu system:

    <HELP> Wellcome screen
      Displays the sample application name.
      Press and hold [OK] to show a simple help screen.
      
      Help screen
      Release [OK] to go back to the welcome screen.
      
    <CONFIGURE> Configuration sub-menu
      Press [OK] to display the configuration sub-menu
      
      <ADD INSTL CODE> install code sub-menu
        Press [OK] to display the install code sub-menu
        
        <SET INSTL CODE> set install code
          Press [OK] to to edit the install code
          
          Install code editing screen
            Press [LEFT] / [RIGHT] to select a digit
            Press [UP] / [DOWN] to change the selected digit
            press [OK] to to go back to the higher menu level
            
        <SET I.C. ADDR> set install code address (coordinator only)
          Press [OK] to to edit the install code address

          Install code address editing screen
            Press [LEFT] / [RIGHT] to select a digit
            Press [UP] / [DOWN] to change the selected digit
            press [OK] to to go back to the higher menu level
            
        <APLY INST CODE> apply install code
          Press [OK] to apply the address+Install-Code pair (coordinator) or the Install-Code (Routers and End-Devices)
          This menu screen also displays the status of the last apply-install-code operation

        <BACK> go back to higher menu level
          press [OK] to to go back to the higher menu level
        
      <T.L. TRGT TIME> Set the touchlink enable duration
        Press and hold [UP] / [DOWN] to increase / decrease the displayed time. The longer the press, the faster
        the change. 
        The maximum duration is 86400 seconds. Increasing it above this value will display '(forever)' - 
        in this setting, the touchlink target will stay active once started and untill manually stopped.
        The minimum duration is 1 second. Decreasing it below this value sets tohchlink target to always disabled.
        This menu item is only available if BDB_TL_TARGET is defined.

      <T.L. STEALING> enable/disable touchlink stealing
        Press [OK] to toggle (enable/disable) touchlink stealing.
        When stealing is enabled, a touchlink target may be 'stolen' by another device acting as a touchlink 
        initiator, which will take it out of its existing network and add it to another network.
        This menu item is only available if BDB_TL_TARGET is defined.

      <T.L. INITIATOR> enable/disable touchlink initiator
        Press [OK] to toggle (enable/disable) touchlink initiator.
        When enabled, touchlink initiator functionality will be executed as part of the commissioning procedure.
        This menu item is only available if BDB_TL_INITIATOR is defined.

      <NWK FORMATION> enable/disable network formation for when cannot connect to an existing network
        Press [OK] to toggle (enable/disable) network formation.
        When enabled, network formation will be executed as part of the commissioning procedure, unless 
        the device has already joined a network.

      <NWK STEERING> enable/disable attempting to connect to an existing network
        Press [OK] to toggle (enable/disable) network steering.
        When enabled, network steering will be executed as part of the commissioning procedure, so the
        device will try to join an existing network, and will open the network for joining of other devices
        once it is joined.

      <FINDNG+BINDNG> enable/disable finding and binding
        Press [OK] to toggle (enable/disable) finding and binding (F&B).
        When enabled, finding and binding will be executed as part of the commissioning procedure, so the
        device will either start identifying, look for matchng devices that are currently identifying, or 
        both (depending of whether it is a F&B target, initiator, or both).
      
      <PRI CHANL MASK> set the primary channel mask
        Press [OK] to edit the primary channel list

        Primary channel list editing screen
          Press [LEFT] / [RIGHT] to select a channel (from channel 11 to channel 26)
          Press [UP] / [DOWN] to enable / disable the selected channel, accordingly.
          press [OK] to to go back to the higher menu level

      <SEC CHANL MASK> set the secondary channel mask
        Press [OK] to edit the secondary channel list
        
        Secondary channel list editing screen
          Press [LEFT] / [RIGHT] to select a channel (from channel 11 to channel 26)
          Press [UP] / [DOWN] to enable / disable the selected channel, accordingly
          press [OK] to to go back to the higher menu level
        
      <PAN ID> set the PAN ID
        Press [OK] to edit the PAN ID to create / connect to
        
        PAN ID editing screen
          Press [LEFT] / [RIGHT] to select the digit to edit
          Press [UP] / [DOWN] to change the selected digit
          press [OK] to to go back to the higher menu level
        
      <BACK> go back to higher menu level
        press [OK] to to go back to the higher menu level
      
    <COMMISSION> start comissionin
      Press [OK] to start the commissioning procedure. (Note: this is disabled if a previous comissioning is
      already on-going).
      This procedure will execute the following methods, depending on whether they are enabled or disabled
      in the respective configuration item):
        - Touchlink (as Touchlink Initiator)
        - Network Steering
        - Network Formation
        - Finding and Binding
      This screen shows the following information
        Line1:
          Current commissioning method being executed:
            TL: Touchlink
            NS: Network Steering
            NF: Network Formation
            FB: Finding And Binding
            PL: Parent Lost (for end devices only)
            -- - idle (commissioning not currently active)
          Network status
            NotOnNwk - not currently connected to a network
            FORM - network was formed by the current device during the latest execution of the NF method
            JOIN - the current device joined an existing network during the latest execution of the NS method
            TCHL - the current device joined a network using touchlink 
            EXST - the device was already connected to a network when the commissioning was started
          Joining permission state (not showing on end devices):
            CLOSED - the current device is closed for joining of other devices
            OpenXXX - the current device is open for joining of other devices, and will close in XXX seconds
        Line2:
          IdXXX - The device is identifying (if XXX > 0), and will stop identifying in XXX seconds.
          SrchXXX/YY - The device is currently performing F&B as an Initiator (if XXX > 0), and will stop in
            XXX seconds. YY is the number of matching endpoints that were foind, for which bindings were 
            successfully created or already existed.
        
    <T.L. TARGET> start touchlink target
      Press [OK] to start / stop touchlink target functionality.
      When not active, pressing [OK] will start touchlink target for the duration defined by the respective
      configuration item.
      When active, pressing OK will immidiately stop touchlink target functionality.
      This menu item is only available if BDB_TL_TARGET is defined.

    <APP MENU> application-specific sub-menu
      Press [OK] to enter the application-specific sub-menu.
      See the main comment in the specific application c file for more details.
      
    <INFO> device and network information
      This screen has no operation related to it. It shows the following information:
        Line1:
          Device IEEE address
        Line2: (only meaningful when the device is on a network)
          nXXXX - the Network pan-id
          cXX - the Channel the network is active on
          aXXXX - the short Address of the device
        Line3:
          XXX (YYY) - the device is not connected to a network.
            XXX is either ZR, ZC or ZED, depending on the build configuration.
            YYY is the network-state value (see definition of devStates_t in ZDApp.h).
          ZC - the device is connected to a network as a coordinator.
          ZR CENT - the device is connected to a centralized network as a router.
          ZR DIST - the device is connected to a distributed network as a router.
          ZED pXXXX - the device is connected to a network as an end device. Parent address is XXXX.

    <RESET TO FN> reset the device to factory-new
      Press [OK] to reset the device to factory-new. The device will disconnect from the network, and 
      all configurations and network information will be deleted.

*********************************************************************/

#ifdef LEGACY_LCD_DEBUG
#error Please disable LEGACY_LCD_DEBUG.
#endif

#if (BDB_INSTALL_CODE_USE!=BDB_INSTALL_CODE_USE_IC_CRC)
#warning Install-Code functionality is disabled. To enable, make sure BDB_INSTALL_CODE_USE==BDB_INSTALL_CODE_USE_IC_CRC
#endif

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "hal_key.h"
#include "bdb_interface.h"
#include "hal_lcd.h"
#include "hal_led.h"
#include "zcl.h"
#include "zdapp.h"
#include "zcl_sampleapps_ui.h"

#ifdef BDB_TL_TARGET
  #include "bdb_touchlink_target.h"
#endif


/*********************************************************************
 * CONSTANT-MACROS AND ENUMERATIONS
 */
enum
{
  UI_STATE_DEFAULT = 0,
  UI_STATE_CONFIGURE,
  UI_STATE_COMMISSION,
#ifdef BDB_TL_TARGET
  UI_STATE_TOGGLE_TL_TARGET,
#endif
  UI_STATE_APP_SPECIFIC_MENU,
  UI_STATE_INFO,
  UI_STATE_RESET_TO_FACTORY_NEW,
  UI_STATE_STATE_HELP,
  UI_STATE_SET_INST_CODE,
#ifdef BDB_TL_TARGET
  UI_STATE_TOUCHLINK_TARGET,
  UI_STATE_TOUCHLINK_STEALING,
#endif
#ifdef BDB_TL_INITIATOR
  UI_STATE_TOUCHLINK_INITIATOR,
#endif
  UI_STATE_NETWORK_FORMATION,
  UI_STATE_NETWORK_STEERING,
  UI_STATE_FINDING_AND_BINDING,
  UI_STATE_SET_PRI_CHANEL_MASK_0,
  UI_STATE_SET_SEC_CHANEL_MASK_0,
  UI_STATE_SET_PAN_ID_0,
  UI_STATE_BACK_FROM_CONFIGURE,
  UI_STATE_SET_PRI_CHANEL_MASK_1,
  UI_STATE_SET_SEC_CHANEL_MASK_1,
  UI_STATE_SET_PAN_ID_1,
  UI_STATE_SET_INST_CODE_0,
#if (ZG_BUILD_COORDINATOR_TYPE)
  UI_STATE_SET_INST_CODE_ADDR_0,
#endif
  UI_STATE_SET_INST_CODE_DONE,
  UI_STATE_BACK_FROM_INSTALL_CODE,
  UI_STATE_SET_INST_CODE_1,
#if (ZG_BUILD_COORDINATOR_TYPE)
  UI_STATE_SET_INST_CODE_ADDR_1,
#endif
};

#define DEFAULT_COMISSIONING_MODE (BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_FINDING_BINDING)

#define TOUCHLINK_TIMEOUT_INCREMENTS_INITIAL 1000
#define TOUCHLINK_TIMEOUT_INCREMENTS_MAX 10000000

#define DEFAULT_TOUCHLINK_TARGET_TIMEOUT 30000

#define LCD_CURSOR_UPDATE_INTERVAL 500

#define LCD_AUTO_REFRESH_INTERVAL_ON_COMISSIONING_SCREEN 250

#define LCD_AUTO_UPDATE_INTERVAL_ON_TOUCHLINK_TARGET_SCREEN 1000
#define LCD_AUTO_UPDATE_INTERVAL_ON_TOUCHLINK_TARGET_SCREEN_END 50

#define LCD_LINE_LENGTH 16

#define STATUS_UNKNOWN 0xFF

#define LCD_CURSOR_CHAR ' '

#define UI_INSTALL_CODE_DEFAULT {0x83,0xFE,0xD3,0x40,0x7A,0x93,0x97,0x23,0xA5,0xC6,0x39,0xB2,0x69,0x16,0xD5,0x05,0xC3,0xB5} //This install code produces the key: 66B6900981E1EE3CA4206B6B861C02BB
#define UI_INSTALL_CODE_ADDR_DEFAULT {0xE7,0xFC,0x0E,0x04,0x00,0x4B,0x12,0x00}


#ifdef HAL_BOARD_CC2538
#define UI_LCD_LINE_1 HAL_LCD_LINE_3
#define UI_LCD_LINE_2 HAL_LCD_LINE_4
#define UI_LCD_LINE_3 HAL_LCD_LINE_5

#define LCD_CC2538_PREFIX_COUNT 3
#else
#define UI_LCD_LINE_1 HAL_LCD_LINE_1
#define UI_LCD_LINE_2 HAL_LCD_LINE_2
#define UI_LCD_LINE_3 HAL_LCD_LINE_3

#define LCD_CC2538_PREFIX_COUNT 0
#endif

#define UI_LED_IDENTIFY_DUTY_CYCLE    50

#define UI_LED_COORDINATOR_DUTY_CYCLE 75
#define UI_LED_ROUTER_DUTY_CYCLE      95
#define UI_LED_END_DEVICE_DUTY_CYCLE 100

#define UI_LED_DEVICE_STATE_FLASH_TIME 4000

#define UI_LED_DEVICE_STATE       HAL_LED_2
#define UI_LED_IDENTIFY           HAL_LED_3
#define UI_LED_NETWORK_OPEN_STATE HAL_LED_4

#define NWK_OPEN_FOR_JOINING 0x1
#define NWK_OPEN_TOUCHLINK_AS_TARGET 0x2

/*********************************************************************
 * TYPEDEFS
 */
   
/*********************************************************************
* LOCAL FUNCTIONS DECLARATIONS
*/
static void uiUintToString(uint32 value, char * str, uint8 base, uint8 num_of_digists, bool pad0, bool reverse);
static void uiArrayToString(uint8 * buf, char * str, uint8 num_of_digists, bool big_endian);
static void uiCreateChannelMaskString(uint32 channel_mask, char * str);


#ifdef BDB_TL_TARGET
static void uiActionToggleTlTarget(uint16 keys);
#endif
static void uiActionStartComissioning(uint16 keys);
static void uiActionSetInstallCode(uint16 keys);
static void uiActionResetToFactoryNew(uint16 keys);
static void uiActionConfigureEnables(uint16 keys);
static void uiActionMenuJump(uint16 keys);
static void uiActionProcessConfigureChannels(uint16 keys);
static void uiActionProcessConfigurePanId(uint16 keys);
static void uiActionProcessPrepareInstallCode(uint16 keys);
static void uiActionAppSecificMenu(uint16 keys);

static void *uiProcessPermitJoin( void *duration );

static void uiProcessIdentifyTimeChange( uint8 endpoint );

static void uiProcessBindNotification( bdbBindNotificationData_t *data );

#ifdef BDB_TL_TARGET
static void uiProcessTouchlinkTargetEnable( uint8 enable );
#endif

/*********************************************************************
 * CONSTANTS
 */
static const uiState_t gui_states_main[] = 
{
  /*  UI_STATE_DEFAULT                */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_RESET_TO_FACTORY_NEW,    UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
  /*  UI_STATE_CONFIGURE              */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
  /*  UI_STATE_COMMISSION             */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionStartComissioning},
#ifdef BDB_TL_TARGET
  /*  UI_STATE_TOGGLE_TL_TARGET       */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionToggleTlTarget},
#endif
  /*  UI_STATE_APP_SPECIFIC_MENU      */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionAppSecificMenu},
  /*  UI_STATE_INFO                   */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            0, NULL},
  /*  UI_STATE_RESET_TO_FACTORY_NEW   */ {UI_STATE_DEFAULT,               UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionResetToFactoryNew},

  /*  UI_STATE_STATE_HELP             */ {UI_STATE_UNCHANGED,             UI_STATE_UNCHANGED,               UI_KEY_SW_5_RELEASED,&uiActionMenuJump},

  /*  UI_STATE_SET_INST_CODE          */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_BACK_FROM_CONFIGURE,     UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
#ifdef BDB_TL_TARGET
  /*  UI_STATE_TOUCHLINK_TARGET       */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_1_RELEASED | UI_KEY_SW_3_RELEASED, &uiActionConfigureEnables},
  /*  UI_STATE_TOUCHLINK_STEALING     */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionConfigureEnables},
#endif
#ifdef BDB_TL_INITIATOR
  /*  UI_STATE_TOUCHLINK_INITIATOR    */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionConfigureEnables},
#endif
  /*  UI_STATE_NETWORK_FORMATION      */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionConfigureEnables},
  /*  UI_STATE_NETWORK_STEERING       */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionConfigureEnables},
  /*  UI_STATE_FINDING_AND_BINDING    */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionConfigureEnables},
  /*  UI_STATE_SET_PRI_CHANEL_MASK_0  */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
  /*  UI_STATE_SET_SEC_CHANEL_MASK_0  */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
  /*  UI_STATE_SET_PAN_ID_0           */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
  /*  UI_STATE_BACK_FROM_CONFIGURE    */ {UI_STATE_SET_INST_CODE,         UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},

  /*  UI_STATE_SET_PRI_CHANEL_MASK_1  */ {UI_STATE_UNCHANGED,             UI_STATE_UNCHANGED,               UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED | UI_KEY_SW_2_PRESSED  | UI_KEY_SW_4_PRESSED | UI_KEY_SW_5_RELEASED , &uiActionProcessConfigureChannels},
  /*  UI_STATE_SET_SEC_CHANEL_MASK_1  */ {UI_STATE_UNCHANGED,             UI_STATE_UNCHANGED,               UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED | UI_KEY_SW_2_PRESSED  | UI_KEY_SW_4_PRESSED | UI_KEY_SW_5_RELEASED , &uiActionProcessConfigureChannels},
  /*  UI_STATE_SET_PAN_ID_1           */ {UI_STATE_UNCHANGED,             UI_STATE_UNCHANGED,               UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED | UI_KEY_SW_2_PRESSED  | UI_KEY_SW_4_PRESSED | UI_KEY_SW_5_RELEASED , &uiActionProcessConfigurePanId},

  /*  UI_STATE_SET_INST_CODE_0        */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_BACK_FROM_INSTALL_CODE,  UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
#if (ZG_BUILD_COORDINATOR_TYPE)
  /*  UI_STATE_SET_INST_CODE_ADDR_0   */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},
#endif  
  /*  UI_STATE_SET_INST_CODE_DONE     */ {UI_STATE_DEFAULT_MOVE,          UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionSetInstallCode},
  /*  UI_STATE_BACK_FROM_INSTALL_CODE */ {UI_STATE_SET_INST_CODE_0,       UI_STATE_DEFAULT_MOVE,            UI_KEY_SW_5_PRESSED, &uiActionMenuJump},

  /*  UI_STATE_SET_INST_CODE_1        */ {UI_STATE_UNCHANGED,             UI_STATE_UNCHANGED,               UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED | UI_KEY_SW_2_PRESSED  | UI_KEY_SW_4_PRESSED | UI_KEY_SW_5_RELEASED , &uiActionProcessPrepareInstallCode},
#if (ZG_BUILD_COORDINATOR_TYPE)
  /*  UI_STATE_SET_INST_CODE_ADDR_1   */ {UI_STATE_UNCHANGED,             UI_STATE_UNCHANGED,               UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED | UI_KEY_SW_5_PRESSED | UI_KEY_SW_2_PRESSED  | UI_KEY_SW_4_PRESSED | UI_KEY_SW_5_RELEASED , &uiActionProcessPrepareInstallCode},
#endif
};

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * EXTERNAL REFERENCES
 */
extern uint8 aExtendedAddress[];

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 uiAppTaskId;
static uint16 uiLcdAutoUpdateEvent;
static uint16 * pUiIdentifyTimeAttribute;

static uiAppUpdateLcd_t uiAppUpdateLcd;
static const uiState_t * uiAppStatesMain = NULL;

static uint8 FBMatchesFound = 0;

static int uiCurrentState = UI_STATE_DEFAULT;
static bool uiCommissioningIsInitializing = FALSE;

static uint8 uiLcdCursorLine = 0xFF;
static uint8 uiLcdCursorCol;
static uint8 uiLcdCursorState;

static uint8 uiSelectedBdbComissioningModes = DEFAULT_COMISSIONING_MODE;

static ZStatus_t uiAddInstallCodeLastStatus = STATUS_UNKNOWN;

static char * uiCommissioningStateStr = "--";
static char * uiCommissioningNetworkConnrctionStr = NULL;

static bool uiComissioningIsActive = FALSE;

static bool uiResetInitiated = FALSE;

#ifdef BDB_TL_TARGET
static uint16 uiKeyAutoRepeatEvent;
static uint32 uiLcdAutoUpdateInterval;

static uint16 uiAutoKeyRepeatDelay = 500;
static uint32 uiAutoKeyRepeatCount = 0;

static uint32 uiTouchlinkTargetTimeout = DEFAULT_TOUCHLINK_TARGET_TIMEOUT;
#endif

static uint8 uiInstallCode[] = UI_INSTALL_CODE_DEFAULT;
#if (ZG_BUILD_COORDINATOR_TYPE)
static uint8 uiInstallCodeAddr[Z_EXTADDR_LEN] = UI_INSTALL_CODE_ADDR_DEFAULT;
#endif

static const uiState_t * uiStates = gui_states_main;

static char * uiAppTitleStr;

static uint8 uiNetworkOpenStateLedDutyCycle[] = {0, 25, 75, 100}; //25% = open for joining; 75% = touchlink target; 100% = both;
static uint8 uiNwkOpenState = 0x00;

static devStates_t uiNwkStateShadow = DEV_HOLD;

/*********************************************************************
 * LOCAL UTILITY FUNCTIONS
 */

/*********************************************************************
 * @fn          uiUintToString
 *
 * @brief       format an integer into a string buffer.
 *
 * @param       value - 32bit unsigned int value to be formatted
 *              str - pointer to a buffer to store the formatted bnumber
 *              base - base represenation of the value. currently only tested base 10 and 16
 *              num_of_digists - number of digits to include in the formatted string
 *              pad0 - should be set to TRUE to pad the number with leading 0's as required
 *              reverse - should be set to TRUE to reverse the output string
 *
 * @return      none
 */
static void uiUintToString (uint32 value, char * str, uint8 base, uint8 num_of_digists, bool pad0, bool reverse)
{
  int i;
  uint8 index;
  
  for (i = 0; i < num_of_digists; i++)
  {
    index = (reverse ? i : num_of_digists - 1 - i);
    str[index] = '0' + (value % base);
    if (str[index] > '9')
    {
      str[index] += 'A' - '0' - 10;
    }
    value /= base;
    if ((!pad0) && (value == 0))
    {
      break;
    }
  }
}

/*********************************************************************
 * @fn          uiArrayToString
 *
 * @brief       format a memory buffer into a string buffer in hex representation.
 *
 * @param       buf - pointer to a bufer to be formatted
 *              str - pointer to a buffer to store the formatted string
 *              num_of_digists - number of digits to include in the formatted string
 *              big_endian - whether the memory content should be represented as big or little endian
 *
 * @return      none
 */
static void uiArrayToString (uint8 * buf, char * str, uint8 num_of_digists, bool big_endian)
{
  int i;
  uint8 stringIndex;
  uint8 value;
  
  for (i = 0; i < num_of_digists; i++)
  {
    stringIndex = (big_endian ? i : num_of_digists - 1 - i);
    if(big_endian)
    {
      value = (buf[i / 2] >> (4 * (!(i % 2)))) & 0x0F;
    }
    else
    {
      value = (buf[i / 2] >> (4 * (i % 2))) & 0x0F;
    }
    str[stringIndex] = '0' + value;
    if (str[stringIndex] > '9')
    {
      str[stringIndex] += 'A' - '0' - 10;
    }
  }
}

/*********************************************************************
 * @fn          uiCreateChannelMaskString
 *
 * @brief       format a channel mask into a string bugger
 *
 * @param       channel_mask - bitmask of the enabled / disabled channels (bits 11-26 represent the
 *                respective channels)
 *              str - pointer to a buffer to store the formatted string
 *
 * @return      none
 */
static void uiCreateChannelMaskString(uint32 channel_mask, char * str)
{
  int i;
  uint32 mask = 0x00000800;
  
  for (i = 0; i < 16; i++)
  {
    str[i] = channel_mask & mask ? '*' : '-';
    mask <<= 1;
  }
  
  str[16] = 0;
}


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn          uiActionMenuJump
 *
 * @brief       State-machine action for jumping to another state
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionMenuJump(uint16 keys)
{
  switch (uiCurrentState)
  {
    case UI_STATE_DEFAULT:
      uiCurrentState = UI_STATE_STATE_HELP;
      break;
    case UI_STATE_STATE_HELP:
      uiCurrentState = UI_STATE_DEFAULT;
      break;
    case UI_STATE_CONFIGURE:
      uiCurrentState = UI_STATE_SET_INST_CODE;
      break;
    case UI_STATE_BACK_FROM_CONFIGURE:
      uiCurrentState = UI_STATE_CONFIGURE;
      break;
    case UI_STATE_SET_PRI_CHANEL_MASK_0:
      uiCurrentState = UI_STATE_SET_PRI_CHANEL_MASK_1;
      break;
    case UI_STATE_SET_SEC_CHANEL_MASK_0:
      uiCurrentState = UI_STATE_SET_SEC_CHANEL_MASK_1;
      break;
    case UI_STATE_SET_PAN_ID_0:
      uiCurrentState = UI_STATE_SET_PAN_ID_1;
      break;
    case UI_STATE_SET_INST_CODE:
      uiCurrentState = UI_STATE_SET_INST_CODE_0;
      break;
    case UI_STATE_SET_INST_CODE_0:
      uiCurrentState = UI_STATE_SET_INST_CODE_1;
      break;
#if (ZG_BUILD_COORDINATOR_TYPE)
    case UI_STATE_SET_INST_CODE_ADDR_0:
      uiCurrentState = UI_STATE_SET_INST_CODE_ADDR_1;
      break;
#endif
    case UI_STATE_BACK_FROM_INSTALL_CODE:
      uiCurrentState = UI_STATE_SET_INST_CODE;
      break;
  }
}

/*********************************************************************
 * @fn          uiActionStartComissioning
 *
 * @brief       State-machine action for starting comissioning
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionStartComissioning(uint16 keys)
{
  if ((!uiComissioningIsActive) && (uiSelectedBdbComissioningModes != 0))
  {
    uiComissioningIsActive = TRUE;

    FBMatchesFound = 0;
    
    //update LCD now, since bdb_StartCommissioning() is blocking for a few seconds,
    // and we want to give a prompt response to the user
    uiCommissioningIsInitializing = TRUE;
    UI_UpdateLcd();

    //if already on the network - mark it as 'existing connection'
    if ((uiNwkStateShadow == DEV_END_DEVICE) || (uiNwkStateShadow == DEV_ZB_COORD) || (uiNwkStateShadow == DEV_ROUTER))
    {
      uiCommissioningNetworkConnrctionStr = "EXST";
    }
    
    bdb_StartCommissioning(uiSelectedBdbComissioningModes);
  }
}

/*********************************************************************
 * @fn          uiActionSetInstallCode
 *
 * @brief       State-machine action for setting the install code
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionSetInstallCode(uint16 keys)
{
  uint16 crc = bdb_GenerateInstallCodeCRC(uiInstallCode);
  uiInstallCode[INSTALL_CODE_LEN] = crc & 0xFF;
  uiInstallCode[INSTALL_CODE_LEN + 1] = crc >> 8;

#if (ZG_BUILD_COORDINATOR_TYPE)
  if (ZG_DEVICE_COORDINATOR_TYPE)
  {  
    uiAddInstallCodeLastStatus = bdb_addInstallCode(uiInstallCode, uiInstallCodeAddr);
  }
  else
  {
    uiAddInstallCodeLastStatus = bdb_setActiveCentralizedLinkKey(zstack_UseInstallCode, uiInstallCode);
  }
#else
  uiAddInstallCodeLastStatus = bdb_setActiveCentralizedLinkKey(zstack_UseInstallCode, uiInstallCode);
#endif
}

/*********************************************************************
 * @fn          uiActionAppSecificMenu
 *
 * @brief       State-machine action for executing the application-specific sub-menus
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
void uiActionAppSecificMenu(uint16 keys)
{
  if (uiAppStatesMain != NULL)
  {
    uiStates = uiAppStatesMain;
    uiCurrentState = 1; //do not start from 0, which is the 'back' menu item
  }
}

/*********************************************************************
 * @fn          UI_ActionBackFromAppMenu
 *
 * @brief       Application state-machine action for returning to the common menu
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
void UI_ActionBackFromAppMenu(uint16 keys)
{
  uiStates = gui_states_main;
  uiCurrentState = UI_STATE_APP_SPECIFIC_MENU;
}

#ifdef BDB_TL_TARGET
/*********************************************************************
 * @fn          uiActionToggleTlTarget
 *
 * @brief       State-machine action for toggling touchlink-target functionality
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
void uiActionToggleTlTarget(uint16 keys)
{
  if (!touchLinkTargetEnabled)
  {
    if (uiTouchlinkTargetTimeout > 0)
    {
      touchLinkTarget_EnableCommissioning( uiTouchlinkTargetTimeout );
      uiLcdAutoUpdateInterval = LCD_AUTO_UPDATE_INTERVAL_ON_TOUCHLINK_TARGET_SCREEN;
    }
  }
  else
  {
    touchLinkTarget_DisableCommissioning();
    uiLcdAutoUpdateInterval = LCD_AUTO_UPDATE_INTERVAL_ON_TOUCHLINK_TARGET_SCREEN_END;
  }
}
#endif

/*********************************************************************
 * @fn          uiActionProcessConfigureChannels
 *
 * @brief       State-machine action for configuring channel masks
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionProcessConfigureChannels(uint16 keys)
{
  uint32 * channelMask;
  
  uiLcdCursorLine = 1;
  uiLcdCursorState = 0;

  if (uiCurrentState == UI_STATE_SET_PRI_CHANEL_MASK_1)
  {
    channelMask = &bdbAttributes.bdbPrimaryChannelSet;
  }
  else
  {
    channelMask = &bdbAttributes.bdbSecondaryChannelSet;
  }

  if (keys & UI_KEY_SW_5_RELEASED)
  {
    uiLcdCursorCol = 0;
  }
  
  if (keys & UI_KEY_SW_1_PRESSED)
  {
    *channelMask |= ((uint32)0x00000800 << uiLcdCursorCol);
    uiLcdCursorState = 1;
  }
  else if (keys & UI_KEY_SW_3_PRESSED)
  {
    *channelMask &= ~(((uint32)0x00000800 << uiLcdCursorCol));
    uiLcdCursorState = 1;
  }
  else if (keys & UI_KEY_SW_2_PRESSED)
  {
    if (uiLcdCursorCol < 15)
    {
      uiLcdCursorCol++;
    }
    else
    {
      uiLcdCursorCol = 0;
    }
  }
  else if (keys & UI_KEY_SW_4_PRESSED)
  {
    if (uiLcdCursorCol > 0)
    {
      uiLcdCursorCol--;
    }
    else
    {
      uiLcdCursorCol = 15;
    }
  }
  else if (keys & UI_KEY_SW_5_PRESSED)
  {
    if (uiCurrentState == UI_STATE_SET_PRI_CHANEL_MASK_1)
    {
      uiCurrentState = UI_STATE_SET_PRI_CHANEL_MASK_0;
    }
    else
    {
      uiCurrentState = UI_STATE_SET_SEC_CHANEL_MASK_0;
    }
    
    uiLcdCursorLine = 0xFF;
  }
}

/*********************************************************************
 * @fn          uiActionProcessConfigurePanId
 *
 * @brief       State-machine action for configuring the PAN ID
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionProcessConfigurePanId(uint16 keys)
{
  uint8 shift;
  uint8 digit;
  
  uiLcdCursorLine = 0;
  uiLcdCursorState = 0;
  
  if (keys & UI_KEY_SW_5_RELEASED)
  {
    uiLcdCursorCol = 6;
  }

  if ((keys & UI_KEY_SW_1_PRESSED) || (keys & UI_KEY_SW_3_PRESSED))
  {
    shift = 4 * (9 - uiLcdCursorCol);
    digit = (zgConfigPANID >> shift) & 0xF;
    if (keys & UI_KEY_SW_1_PRESSED)
    {
      if (digit < 15)
      {
        digit++;
      }
      else
      {
        digit = 0;
      }
    }
    else if (keys & UI_KEY_SW_3_PRESSED)
    {
      if (digit > 0)
      {
        digit--;
      }
      else
      {
        digit = 15;
      }
    }

    zgConfigPANID &= ~((uint32)0xF << shift);
    zgConfigPANID |= (uint32)digit << shift;
    uiLcdCursorState = 1;
  }
  else if (keys & UI_KEY_SW_2_PRESSED)
  {
    if (uiLcdCursorCol < 9)
    {
      uiLcdCursorCol++;
    }
  }
  else if (keys & UI_KEY_SW_4_PRESSED)
  {
    if (uiLcdCursorCol > 6)
    {
      uiLcdCursorCol--;
    }
  }
  else if (keys & UI_KEY_SW_5_PRESSED)
  {
    osal_nv_write(ZCD_NV_PANID, 0, osal_nv_item_len( ZCD_NV_PANID ), &zgConfigPANID); //todo: check and display result of osal_nv_write()

    uiCurrentState = UI_STATE_SET_PAN_ID_0;
    
    uiLcdCursorLine = 0xFF;
  }
}

/*********************************************************************
 * @fn          uiActionProcessPrepareInstallCode
 *
 * @brief       State-machine action for editing the install-code and the install-code address
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionProcessPrepareInstallCode(uint16 keys)
{
  static uint8 * pCurrentModifiedField;
  static uint8 CurrentModifiedFieldLen;
  static bool CurrentModifiedFieldIsBigEndian;
  
  static uint8 index;

  uint8 shift;
  uint8 digit;
  uint8 bufIndex;
  uint16 crc;

  uiLcdCursorState = 0;
  
  if (keys & UI_KEY_SW_5_RELEASED)
  {
    index = 0;
    
#if (ZG_BUILD_COORDINATOR_TYPE)
    if (uiCurrentState == UI_STATE_SET_INST_CODE_ADDR_1)
    {
      pCurrentModifiedField = uiInstallCodeAddr;
      CurrentModifiedFieldLen = sizeof(uiInstallCodeAddr);
      CurrentModifiedFieldIsBigEndian = FALSE;
    }
    else
#endif
    {
      pCurrentModifiedField = uiInstallCode;
      CurrentModifiedFieldLen = INSTALL_CODE_LEN;
      CurrentModifiedFieldIsBigEndian = TRUE;

      crc = bdb_GenerateInstallCodeCRC(uiInstallCode);
      uiInstallCode[INSTALL_CODE_LEN] = crc & 0xFF;
      uiInstallCode[INSTALL_CODE_LEN + 1] = crc >> 8;
    }
  }

  if ((keys & UI_KEY_SW_1_PRESSED) || (keys & UI_KEY_SW_3_PRESSED))
  {
    if(pCurrentModifiedField == uiInstallCode)
    {
      shift = 4 * ((CurrentModifiedFieldIsBigEndian ? (index + 1) : index ) % 2);
    }
    else
    {
      shift = 4 * ((CurrentModifiedFieldIsBigEndian ? index : (index + 1)) % 2);
    }
    bufIndex = (CurrentModifiedFieldIsBigEndian ? (index / 2) : (CurrentModifiedFieldLen - 1 - (index / 2)));
    digit = (pCurrentModifiedField[bufIndex] >> shift) & 0xF;

    if (keys & UI_KEY_SW_1_PRESSED)
    {
      if (digit < 15)
      {
        digit++;
      }
      else
      {
        digit = 0;
      }
    }
    else if (keys & UI_KEY_SW_3_PRESSED)
    {
      if (digit > 0)
      {
        digit--;
      }
      else
      {
        digit = 15;
      }
    }
    
    pCurrentModifiedField[bufIndex] &= ~((uint32)0xF << shift);
    pCurrentModifiedField[bufIndex] |= (uint32)digit << shift;
    uiLcdCursorState = 1;
  }
  else if (keys & UI_KEY_SW_2_PRESSED)
  {
    if (index < CurrentModifiedFieldLen * 2 - 1)
    {
      index++;
    }
    else
    {
      index = 0;
    }
  }
  else if (keys & UI_KEY_SW_4_PRESSED)
  {
    if (index > 0)
    {
      index --;
    }
    else
    {
      index = (CurrentModifiedFieldLen * 2 - 1);
    }
  }
  
  uiLcdCursorLine = index / LCD_LINE_LENGTH;
  uiLcdCursorCol = index % LCD_LINE_LENGTH;

  if ((uiCurrentState == UI_STATE_SET_INST_CODE_1) && ((keys & UI_KEY_SW_1_PRESSED) || (keys & UI_KEY_SW_3_PRESSED)))
  {
    crc = bdb_GenerateInstallCodeCRC(uiInstallCode);
    uiInstallCode[INSTALL_CODE_LEN] = crc & 0xFF;
    uiInstallCode[INSTALL_CODE_LEN + 1] = crc >> 8;
  }

  if (keys & UI_KEY_SW_5_PRESSED)
  {
#if (ZG_BUILD_COORDINATOR_TYPE)
    if (uiCurrentState == UI_STATE_SET_INST_CODE_ADDR_1)
    {
      uiCurrentState = UI_STATE_SET_INST_CODE_ADDR_0;
    }
    else
#endif
    {
      uiCurrentState = UI_STATE_SET_INST_CODE_0;
    }
    
    uiLcdCursorLine = 0xFF;
  }
}

/*********************************************************************
 * @fn          uiActionConfigureEnables
 *
 * @brief       State-machine action for toggling various configurations on and off
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionConfigureEnables(uint16 keys)
{
#if defined ( BDB_TL_TARGET )
  static uint32 TouchlinkTimeoutIncements = TOUCHLINK_TIMEOUT_INCREMENTS_INITIAL;
#endif

  switch (uiCurrentState)
  {
#ifdef BDB_TL_TARGET
    case UI_STATE_TOUCHLINK_TARGET:
      if (keys & UI_KEY_SW_1_PRESSED)
      {
        if (uiTouchlinkTargetTimeout < TOUCHLINK_TARGET_PERPETUAL - TouchlinkTimeoutIncements)
        {
          uiTouchlinkTargetTimeout += TouchlinkTimeoutIncements;
        }
        else
        {
          uiTouchlinkTargetTimeout = TOUCHLINK_TARGET_PERPETUAL;
        }
      }
      else if ((keys & UI_KEY_SW_3_PRESSED) && (uiTouchlinkTargetTimeout >= TouchlinkTimeoutIncements))
      {
        if ((uiTouchlinkTargetTimeout % TouchlinkTimeoutIncements) != 0)
        {
          uiTouchlinkTargetTimeout -= uiTouchlinkTargetTimeout % TouchlinkTimeoutIncements;
        }
        else
        {
          uiTouchlinkTargetTimeout -= TouchlinkTimeoutIncements;
        }
      }

      if ((keys & (UI_KEY_SW_1_PRESSED | UI_KEY_SW_3_PRESSED)) && ((uiTouchlinkTargetTimeout > 0) && (uiTouchlinkTargetTimeout < TOUCHLINK_TARGET_PERPETUAL)))
      {
        osal_start_timerEx(uiAppTaskId, uiKeyAutoRepeatEvent, uiAutoKeyRepeatDelay);
        uiAutoKeyRepeatDelay = 100;
        if (TouchlinkTimeoutIncements < TOUCHLINK_TIMEOUT_INCREMENTS_MAX)
        {
          uiAutoKeyRepeatCount++;
          if (uiAutoKeyRepeatCount == 3)
          {
            uiAutoKeyRepeatCount = 0;
            TouchlinkTimeoutIncements *= 2;
          }
        }
      }
      else
      {
        osal_stop_timerEx(uiAppTaskId, uiKeyAutoRepeatEvent);
        uiAutoKeyRepeatDelay = 500;
        uiAutoKeyRepeatCount = 0;
        TouchlinkTimeoutIncements = 1000;
      }
      break;
    case UI_STATE_TOUCHLINK_STEALING:
      bdb_TouchlinkSetAllowStealing( ! bdb_TouchlinkGetAllowStealing() );
      break;
#endif
#ifdef BDB_TL_INITIATOR
    case UI_STATE_TOUCHLINK_INITIATOR:
      uiSelectedBdbComissioningModes ^= BDB_COMMISSIONING_MODE_INITIATOR_TL;
      break;
#endif
    case UI_STATE_NETWORK_FORMATION:
      uiSelectedBdbComissioningModes ^= BDB_COMMISSIONING_MODE_NWK_FORMATION;
      break;
    case UI_STATE_NETWORK_STEERING:
      uiSelectedBdbComissioningModes ^= BDB_COMMISSIONING_MODE_NWK_STEERING;
      break;
    case UI_STATE_FINDING_AND_BINDING:
      uiSelectedBdbComissioningModes ^= BDB_COMMISSIONING_MODE_FINDING_BINDING;
      break;
  }
}

/*********************************************************************
 * @fn          uiActionResetToFactoryNew
 *
 * @brief       State-machine action for resetting the device to factory new
 *
 * @param       keys - the keypress code that triggered the call to this function
 *
 * @return      none
 */
static void uiActionResetToFactoryNew(uint16 keys)
{
  uiResetInitiated = TRUE;
  bdb_resetLocalAction();
}
  
/*********************************************************************
 * @fn          UI_MainStateMachine
 *
 * @brief       This is the main UI state machine engine.
 *              This function should be called from the application's key handler, and also from its event-loop
 *              for processing the key-repress event.
 *
 * @param       current_keys - a bitmask of the keys that are currently pressed, or 0xFFFF when this function
 *              is invoked to handle key auto-repeat.
 *
 * @return      none
 */
void UI_MainStateMachine( uint16 current_keys ) //argument is uint16 to allow 8 bits for the keys (for future use), and another unique value to mark a re-press of the previous keys.
{
  static byte PrevKeys = 0; //holds the keys that were pressed during the previous time this function was called. (Does not hold the keys that were released.)

  uint8 SavedLcdCursorLine = uiLcdCursorLine;
  bool LcdUpdateIsRequired = FALSE;
  uint16 keys; //will hold the bitmask of the currently pressed keys at the lower 8 bits, and the keys that have just been released at the higher 8 bits.

  if (uiResetInitiated)
  {
    return;
  }
  
  if (current_keys == UI_KEY_AUTO_PRESSED)
  {
    keys = PrevKeys;
  }
  else
  {
    keys = (current_keys | (((PrevKeys ^ current_keys) & PrevKeys) << 8));
  }
  
  uiLcdCursorLine = 0xFF;

  if ((keys & UI_KEY_SW_2_PRESSED) && (uiStates[uiCurrentState].next_state != UI_STATE_UNCHANGED))
  {
    if (uiStates[uiCurrentState].next_state == UI_STATE_DEFAULT_MOVE)
    {
      uiCurrentState++;
    }
    else
    {
      uiCurrentState = uiStates[uiCurrentState].next_state;
    }
    
    LcdUpdateIsRequired = TRUE;
  }
  else if ((keys & UI_KEY_SW_4_PRESSED) && (uiStates[uiCurrentState].prev_state != UI_STATE_UNCHANGED))
  {
    if (uiStates[uiCurrentState].prev_state == UI_STATE_DEFAULT_MOVE)
    {
      uiCurrentState--;
    }
    else
    {
      uiCurrentState = uiStates[uiCurrentState].prev_state;
    }
    
    LcdUpdateIsRequired = TRUE;
  }
  else if ((keys & uiStates[uiCurrentState].keys_mask) && (uiStates[uiCurrentState].state_func != NULL))
  {
    uiStates[uiCurrentState].state_func(keys);
    
    LcdUpdateIsRequired = TRUE;
  }

  if (LcdUpdateIsRequired)
  {
    UI_UpdateLcd();
  }
  else
  {
    uiLcdCursorLine = SavedLcdCursorLine;
  }

  PrevKeys = keys & 0xFF; //only remember the keys that are currently pressed. Released keys are irrelevant. the 0xFF is not needed, since the target is uint8, but it is there just so it is clear that this assignment of uint16 into uint8 is intentional.
}

/*********************************************************************
 * @fn          UI_UpdateComissioningStatus
 *
 * @brief       Update the comissioning status to be displayed when the respective menu-screen is selected.
 *              This function should be called from the application, from within the function registered to
 *              be notified about these events.
 *
 * @param       bdbCommissioningModeMsg - comissioning mode and state information
 *
 * @return      none
 */
void UI_UpdateComissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  uiCommissioningIsInitializing = FALSE;
  
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      uiCommissioningStateStr = "NF";
      if ((bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS) && (uiCommissioningNetworkConnrctionStr == NULL))
      {
        uiCommissioningNetworkConnrctionStr = "FORM";
      }
      break;
    case BDB_COMMISSIONING_NWK_STEERING:
      uiCommissioningStateStr = "NS";
      if ((bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS) && (uiCommissioningNetworkConnrctionStr == NULL))
      {
        uiCommissioningNetworkConnrctionStr = "JOIN";
      }
      break;
    case BDB_COMMISSIONING_FINDING_BINDING:
      uiCommissioningStateStr = "FB";
      break;
    case BDB_COMMISSIONING_INITIALIZATION:
      uiCommissioningStateStr = "IN";
      break;
#if ZG_BUILD_ENDDEVICE_TYPE    
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        uiCommissioningStateStr = "--";
      }
      else
      {
        uiCommissioningStateStr = "PL";
      }
      break;
#endif
#if BDB_TOUCHLINK_CAPABILITY_ENABLED
    case BDB_COMMISSIONING_TOUCHLINK:
      uiCommissioningStateStr = "TL";
      if ((bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS) && (uiCommissioningNetworkConnrctionStr == NULL))
      {
        uiCommissioningNetworkConnrctionStr = "TCHL";
      }
      break;
#endif
  }

  if ((bdbCommissioningModeMsg->bdbCommissioningStatus != BDB_COMMISSIONING_IN_PROGRESS) && (bdbCommissioningModeMsg->bdbRemainingCommissioningModes == 0))
  {
    uiCommissioningStateStr = "--";

    uiComissioningIsActive = FALSE;
  }
  
  UI_UpdateLcd();
}


void HalLcd_HW_WriteChar(uint8 line, uint8 col, char text);


/*********************************************************************
 * @fn          UI_UpdateLcd
 *
 * @brief       Update the LCD display.
 *              This function should be called by the application when handling the display-auto-refresh event, and
 *              also whenevet an action that may change the information being displayed is executed.
 *
 * @param       none
 *
 * @return      none
 */
void UI_UpdateLcd( void )
{
  char * line[3];
  char LineBuf[3][LCD_CC2538_PREFIX_COUNT + LCD_LINE_LENGTH + 1];
  uint8 i;
  uint8 *xad;
  bool LcdAutoRefreshRequired = FALSE;
  uint8 FBRemainingTimeLeft;
  uint8 PermitJoinDuration;
  
  LineBuf[0][LCD_CC2538_PREFIX_COUNT] = 0;
  LineBuf[1][LCD_CC2538_PREFIX_COUNT] = 0;
  LineBuf[2][LCD_CC2538_PREFIX_COUNT] = 0;
  line[0] = LineBuf[0] + LCD_CC2538_PREFIX_COUNT;
  line[1] = LineBuf[1] + LCD_CC2538_PREFIX_COUNT;
  line[2] = LineBuf[2] + LCD_CC2538_PREFIX_COUNT;

#ifdef HAL_BOARD_CC2538
  for (i = 0; i < LCD_CC2538_PREFIX_COUNT; i++)
  {
    LineBuf[0][i] = ' ';
    LineBuf[1][i] = ' ';
    LineBuf[2][i] = ' ';
  }
#endif

  osal_stop_timerEx(uiAppTaskId, uiLcdAutoUpdateEvent);

  if (uiStates == uiAppStatesMain)
  {
    if (uiCurrentState == UI_STATE_BACK_FROM_APP_MENU)
    {
      line[2] = "<     BACK     >";
    }
    else
    {
      uiAppUpdateLcd(uiCurrentState, line);
    }
  }
  else
  {
    switch (uiCurrentState)
    {
      case UI_STATE_FINDING_AND_BINDING:
        line[1] = uiSelectedBdbComissioningModes & BDB_COMMISSIONING_MODE_FINDING_BINDING ? "ENABLED" : "DISABLED";
        line[2] = "<FINDNG+BINDNG >";
        break;
      case UI_STATE_BACK_FROM_CONFIGURE:
        line[2] = "<     BACK     >";
        break;
      case UI_STATE_BACK_FROM_INSTALL_CODE:
        line[2] = "<     BACK     >";
        break;
      case UI_STATE_NETWORK_FORMATION:
        line[1] = uiSelectedBdbComissioningModes & BDB_COMMISSIONING_MODE_NWK_FORMATION ? "ENABLED" : "DISABLED";
        line[2] = "<NWK FORMATION >";
        break;
      case UI_STATE_NETWORK_STEERING:
        line[1] = uiSelectedBdbComissioningModes & BDB_COMMISSIONING_MODE_NWK_STEERING ? "ENABLED" : "DISABLED";
        line[2] = "< NWK STEERING >";
        break;
#ifdef BDB_TL_TARGET
      case UI_STATE_TOGGLE_TL_TARGET:
        {
          uint32 temp_u32;

          temp_u32 = touchLinkTarget_GetTimer();
          if (temp_u32 == 0)
          {
            line[1] = "DISABLED        ";
          }
          else if (temp_u32 == TOUCHLINK_TARGET_PERPETUAL)
          {
            line[1] = "ENABLED FOREVER ";
          }
          else
          {
            uiConstStrCpy(line[1], "ENABLED (     s)");
            uiUintToString( temp_u32 / 1000 + ((temp_u32 % 1000) > 0 ? 1 : 0), line[1] + 9, 10, 5, FALSE, FALSE); //note: timeouts longer than 65535 seconds will not display correctly
            osal_start_timerEx(uiAppTaskId, uiLcdAutoUpdateEvent, uiLcdAutoUpdateInterval);
          }
        }
        line[2] = "< T.L. TARGET  >";
        break;
      case UI_STATE_TOUCHLINK_TARGET:
        if (uiTouchlinkTargetTimeout == 0)
        {
          line[1] = "DISABLED        ";
        }
        else if (uiTouchlinkTargetTimeout == TOUCHLINK_TARGET_PERPETUAL)
        {
          line[1] = "Enable (forever)";
        }
        else
        {
          uiConstStrCpy(line[1], "Enable For     s");
          uiUintToString( uiTouchlinkTargetTimeout / 1000 + ((uiTouchlinkTargetTimeout % 1000) > 0 ? 1 : 0), line[1] + 10, 10, 5, FALSE, FALSE); //note: timeouts longer than 65535 seconds will not display correctly
          line[1][16] = 0;
        }
        line[2] = "<T.L. TRGT TIME>";
        break;
      case UI_STATE_TOUCHLINK_STEALING:
        line[1] = bdb_TouchlinkGetAllowStealing() ? "ENABLED" : "DISABLED";
        line[2] = "<T.L. STEALING >";
        break;
#endif
#ifdef BDB_TL_INITIATOR
      case UI_STATE_TOUCHLINK_INITIATOR:
        line[1] = uiSelectedBdbComissioningModes & BDB_COMMISSIONING_MODE_INITIATOR_TL ? "ENABLED" : "DISABLED";
        line[2] = "<T.L. INITIATOR>";
        break;
#endif
      case UI_STATE_CONFIGURE:
        line[2] = "<  CONFIGURE   >";
        break;
      case UI_STATE_RESET_TO_FACTORY_NEW:
        if (uiResetInitiated)
        {
          line[0] = "Resetting,";
          line[1] = "Please wait...";
        }
        else
        {
          line[2] = "< RESET TO FN  >";
        }
        break;
      case UI_STATE_DEFAULT:
        line[0] = uiAppTitleStr;
        line[1] = "hold OK for help";
        line[2] = "<     HELP     >";
        break;
      case UI_STATE_STATE_HELP:
#ifdef HAL_BOARD_CC2538
        line[0] = "< > to move     ";
        line[1] = "^ v change value";
#else
        line[0] = "\x1B \x1A to move     ";
        line[1] = "\x18 \x19 change value";
#endif        
        line[2] = "OK to execute   ";
        break;
      case UI_STATE_COMMISSION:
        if (uiCommissioningIsInitializing)
        {
          uiConstStrCpy(line[0], "Please wait...  ");
        }
        else
        {
          uiConstStrCpy(line[0], "xx              "); // TL/NF/NS/FB/-- TCHL/FORM/JOIN count/CLOSE

          osal_memcpy(line[0] + 0, uiCommissioningStateStr, 2);

          if ((uiNwkStateShadow != DEV_END_DEVICE) && (uiNwkStateShadow != DEV_ZB_COORD) && (uiNwkStateShadow != DEV_ROUTER))
          {
            uiConstStrOver(line[0] + 3, "NotOnNwk");
          }
          else
          {
            if (uiCommissioningNetworkConnrctionStr != NULL)
            {
              osal_memcpy(line[0] + 3, uiCommissioningNetworkConnrctionStr, 4);
              line[0][7] = ' ';
            }

            if ((uiNwkStateShadow == DEV_ZB_COORD) || (uiNwkStateShadow == DEV_ROUTER))
            {
              PermitJoinDuration = NLME_GetRemainingPermitJoiningDuration();
              
              if (PermitJoinDuration > 0)
              {
                uiConstStrOver(line[0] + 8, "Open");
                uiUintToString(PermitJoinDuration, line[0] + 12, 10, 3, TRUE, FALSE);
                LcdAutoRefreshRequired = TRUE;
              }
              else
              {
                uiConstStrOver(line[0] + 8, "CLOSED");
              }
            }
           }
        }
        
        uiConstStrCpy(line[1], "Id000 Srch000/00"); // IDENTIFYING, SEARCHING   - IDFYcnt SRCHcnt

        uiUintToString(*pUiIdentifyTimeAttribute, line[1] + 2, 10, 3, TRUE, FALSE);
        bdb_GetFBInitiatorStatus(&FBRemainingTimeLeft, NULL);
        uiUintToString(FBRemainingTimeLeft, line[1] + 10, 10, 3, TRUE, FALSE);
        uiUintToString(FBMatchesFound, line[1] + 14, 10, 2, TRUE, FALSE);

        if ((*pUiIdentifyTimeAttribute > 0) || (FBRemainingTimeLeft > 0) || (uiCommissioningStateStr != "--"))
        {
          LcdAutoRefreshRequired = TRUE;
        }

        line[2] = "<  COMMISSION  >";
        
        if (LcdAutoRefreshRequired)
        {
          osal_start_timerEx(uiAppTaskId, uiLcdAutoUpdateEvent, LCD_AUTO_REFRESH_INTERVAL_ON_COMISSIONING_SCREEN);
        }
        break;
      case UI_STATE_SET_INST_CODE:
        line[2] = "<ADD INSTL CODE>";
        break;
      case UI_STATE_SET_INST_CODE_0:
        uiArrayToString(uiInstallCode, line[0], 16, TRUE); 
        uiArrayToString(uiInstallCode + 8, line[1], 16, TRUE); 
        line[0][16] = 0;
        line[1][16] = 0;
        line[2] = "<SET INSTL CODE>";
        break;
#if (ZG_BUILD_COORDINATOR_TYPE)
      case UI_STATE_SET_INST_CODE_ADDR_0:
        uiArrayToString(uiInstallCodeAddr, line[0],16, FALSE); 
        line[0][16] = 0;
        line[2] = "<SET I.C. ADDR >";
        break;
#endif
      case UI_STATE_SET_INST_CODE_DONE:
        line[0] = "Last status:";
        
        switch (uiAddInstallCodeLastStatus)
        {
          case STATUS_UNKNOWN:
            line[1] = "---";
            break;
          case ZSuccess:
            line[1] = "SUCCESS";
            break;
          default:
            uiConstStrCpy(line[1], "ERROR (0x  )    ");
            uiUintToString( uiAddInstallCodeLastStatus, line[1] + 9, 16, 2, TRUE, FALSE);
            break;
        }
        line[2] = "<APLY INST CODE>";
        break;
#if (ZG_BUILD_COORDINATOR_TYPE)
      case UI_STATE_SET_INST_CODE_ADDR_1:
        uiArrayToString(uiInstallCodeAddr, line[0],16, FALSE); 
        line[0][16] = 0;
        line[2] = "       I.C. ADDR";
        break;
#endif
      case UI_STATE_SET_INST_CODE_1:
        uiArrayToString(uiInstallCode, line[0], 16, TRUE); 
        uiArrayToString(uiInstallCode + 8, line[1], 16, TRUE); 
        line[0][16] = 0;
        line[1][16] = 0;
        uiConstStrCpy(line[2], "       INST CODE");
        uiArrayToString( uiInstallCode + 16, line[2], 4, TRUE);
        break;
      case UI_STATE_APP_SPECIFIC_MENU:
        line[2] = "<   APP MENU   >";
        break;
      case UI_STATE_SET_PRI_CHANEL_MASK_0:
        line[0] = "1234567890123456";
        uiCreateChannelMaskString(bdbAttributes.bdbPrimaryChannelSet, line[1]);
        line[2] = "<PRI CHANL MASK>";
        break;
      case UI_STATE_SET_SEC_CHANEL_MASK_0:
        line[0] = "1234567890123456";
        uiCreateChannelMaskString(bdbAttributes.bdbSecondaryChannelSet, line[1]);
        line[2] = "<SEC CHANL MASK>";
        break;
      case UI_STATE_SET_PRI_CHANEL_MASK_1:
        line[0] = "1234567890123456";
        uiCreateChannelMaskString(bdbAttributes.bdbPrimaryChannelSet, line[1]);
        line[2] = " PRI CHANL MASK ";
        break;
      case UI_STATE_SET_SEC_CHANEL_MASK_1:
        line[0] = "1234567890123456";
        uiCreateChannelMaskString(bdbAttributes.bdbSecondaryChannelSet, line[1]);
        line[2] = " SEC CHANL MASK ";
        break;
      case UI_STATE_SET_PAN_ID_0:
        if (zgConfigPANID == 0xFFFF)
        {
          uiConstStrCpy(line[0], "    0xFFFF (any)");
        }
        else
        {
          uiConstStrCpy(line[0], "    0x          ");
          uiUintToString( zgConfigPANID, line[0] + 6, 16, 4, TRUE, FALSE);
        }
        line[2] = "<    PAN ID    >";
        break;
      case UI_STATE_SET_PAN_ID_1:
        if (zgConfigPANID == 0xFFFF)
        {
          uiConstStrCpy(line[0], "    0xFFFF (any)");
        }
        else
        {
          uiConstStrCpy(line[0], "    0x          ");
          uiUintToString( zgConfigPANID, line[0] + 6, 16, 4, TRUE, FALSE);
        }
        line[2] = "     PAN ID     ";
        break;
      case UI_STATE_INFO:
        // Display the extended address.
        xad = aExtendedAddress + Z_EXTADDR_LEN - 1;
        
        for (i = 0; i < Z_EXTADDR_LEN * 2; xad--)
        {
          uint8 ch;
          ch = (*xad >> 4) & 0x0F;
          line[0][i++] = ch + (( ch < 10 ) ? '0' : '7');
          ch = *xad & 0x0F;
          line[0][i++] = ch + (( ch < 10 ) ? '0' : '7');
        }
        
        line[0][Z_EXTADDR_LEN*2] = '\0';

        uiConstStrCpy( line[1], "nxxxx cxx axxxx ");

        uiUintToString( _NIB.nwkPanId, line[1] + 1 , 16, 4, TRUE, FALSE);
        line[1][5] = ' ';
        
        uiUintToString( _NIB.nwkLogicalChannel, (void *)(line[1] + 7), 10, 2, TRUE, FALSE );
        line[1][9] = ' ';
        
        uiUintToString( _NIB.nwkDevAddress, line[1] + 11, 16, 4, TRUE, FALSE );

        uiConstStrCpy(line[2], "          <INFO>");
        switch (uiNwkStateShadow)
        {
          case DEV_ZB_COORD:
            uiConstStrOver(line[2], "ZC");
            break;
          case DEV_ROUTER:
            if (APSME_IsDistributedSecurity())
            {
              uiConstStrOver(line[2], "ZR DIST");
            }
            else
            {
              uiConstStrOver(line[2], "ZR CENT");
            }
            break;
          case DEV_END_DEVICE:
            uiConstStrOver(line[2], "ZED p");
            uiUintToString( _NIB.nwkCoordAddress, line[2] + 5, 16, 4, TRUE, FALSE );
            break;
          default:
#if ZG_BUILD_COORDINATOR_TYPE
            uiConstStrOver(line[2], "ZC (   )");
            uiUintToString( uiNwkStateShadow, line[2] + 4, 10, 3, TRUE, FALSE );
#elif ZG_BUILD_RTRONLY_TYPE
            uiConstStrOver(line[2], "ZR (   )");
            uiUintToString( uiNwkStateShadow, line[2] + 4, 10, 3, TRUE, FALSE );
#elif ZG_BUILD_ENDDEVICE_TYPE
            uiConstStrOver(line[2], "ZED (   )");
            uiUintToString( uiNwkStateShadow, line[2] + 5, 10, 3, TRUE, FALSE );
#else
            uiConstStrOver(line[2], "(   )");
            uiUintToString( uiNwkStateShadow, line[2] + 1, 10, 3, TRUE, FALSE );
#endif
            break;
        }
        
        break;
      default:
        break;
    }
  }

  if (uiLcdCursorLine != 0xFF)
  {
    if (uiLcdCursorState == 0)
    {
      line[uiLcdCursorLine][uiLcdCursorCol] = LCD_CURSOR_CHAR; //note: the cursor must point to a writeable memory location, i.e. not const
    }
    uiLcdCursorState ^= 1;
    
    osal_start_timerEx(uiAppTaskId, uiLcdAutoUpdateEvent, LCD_CURSOR_UPDATE_INTERVAL);
  }

#ifdef HAL_BOARD_CC2538
  for (i = 0; i < 3; i++)
  {
    if (line[i] != LineBuf[i] + LCD_CC2538_PREFIX_COUNT)
    {
      osal_memcpy(LineBuf[i] + LCD_CC2538_PREFIX_COUNT, line[i], LCD_LINE_LENGTH + 1);
    }
    
    line[i] = LineBuf[i];
  }
#endif
  
#ifdef LCD_SUPPORTED
  HalLcd_HW_Clear();
  HalLcdWriteString( line[0], UI_LCD_LINE_1 );
  HalLcdWriteString( line[1], UI_LCD_LINE_2 );
  HalLcdWriteString( line[2], UI_LCD_LINE_3 );

#ifdef DEBUG_LCD_REFRESH
  {
    static bool toggle=TRUE;
    HalLcd_HW_WriteChar(3,15,(toggle = !toggle) ? '>' : ' ');
  }
#endif

#endif
}

/*********************************************************************
 * @fn          uiProcessPermitJoin
 *
 * @brief       Update the Network-Joining status LED
 *
 * @param       duration - The time left for joining, or 0 if the device is closed for joining
 *
 * @return      none
 */
static void *uiProcessPermitJoin( void *duration )
{
  if (*(uint8 *)duration > 0)
  {
    uiNwkOpenState |= NWK_OPEN_FOR_JOINING;
  }
  else
  {
    uiNwkOpenState &= ~NWK_OPEN_FOR_JOINING;
  }

  HalLedBlink ( UI_LED_NETWORK_OPEN_STATE, 0, uiNetworkOpenStateLedDutyCycle[uiNwkOpenState], HAL_LED_DEFAULT_FLASH_TIME );
  
  UI_UpdateLcd();
  
  return ( NULL );
}

/*********************************************************************
 * @fn          UI_Init
 *
 * @brief       Initialize the user-interface module.
 *              This function must be called by the application during its initializatin.
 *
 * @param       app_task_id_value - the task-id of the application task
 *              lcd_auto_update_event_value - the event mask (single bit) for using as the lcd-auto-update event.
 *              key_auto_repeat_event_value - the event mask (single bit) for using as the key-auto-repeat event. 
 *              ui_IdentifyTimeAttribute_value - a pointer to the identify attribute's value.
 *              app_title_value - a pointer to a string, holding the title of the sample app.
 *              _uiAppUpdateLcd - a pointer to a fuction to update the LCD when displaying the application-
 *                specific screens.
 *              _uiAppStatesMain - a pointer to the application-specific state-table.
 *
 * @return      none
 */
void UI_Init(uint8 app_task_id_value, uint16 lcd_auto_update_event_value, uint16 key_auto_repeat_event_value, uint16 * ui_IdentifyTimeAttribute_value, char * app_title_value, uiAppUpdateLcd_t _uiAppUpdateLcd, const uiState_t _uiAppStatesMain[])
{
  uiAppTaskId = app_task_id_value;
  uiLcdAutoUpdateEvent = lcd_auto_update_event_value;
  pUiIdentifyTimeAttribute = ui_IdentifyTimeAttribute_value;
  uiAppTitleStr = app_title_value;
  uiAppUpdateLcd = _uiAppUpdateLcd;
  uiAppStatesMain = _uiAppStatesMain;
#ifdef BDB_TL_TARGET
  uiKeyAutoRepeatEvent = key_auto_repeat_event_value;
#endif

  ZDO_RegisterForZdoCB(ZDO_PERMIT_JOIN_CBID, &uiProcessPermitJoin);
  
  bdb_RegisterIdentifyTimeChangeCB( uiProcessIdentifyTimeChange );
  bdb_RegisterBindNotificationCB( uiProcessBindNotification );
  
#ifdef BDB_TL_TARGET
  bdb_RegisterTouchlinkTargetEnableCB( uiProcessTouchlinkTargetEnable );
#endif

  bdb_StartCommissioning(BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP);
}

/*********************************************************************
 * @fn      uiProcessTouchlinkTargetEnable
 *
 * @brief   Uopdate the UI when the touchlink target functionality is enabled or disabled
 *
 * @param   enable - the current state of the touchlink target (0 - disabled; 1 - enabled)
 *
 * @return  none
 */
#ifdef BDB_TL_TARGET
static void uiProcessTouchlinkTargetEnable( uint8 enable )
{
  if ( enable )
  {
    uiNwkOpenState |= NWK_OPEN_TOUCHLINK_AS_TARGET;
  }
  else
  {
    uiNwkOpenState &= ~NWK_OPEN_TOUCHLINK_AS_TARGET;
  }

  HalLedBlink ( UI_LED_NETWORK_OPEN_STATE, 0, uiNetworkOpenStateLedDutyCycle[uiNwkOpenState], HAL_LED_DEFAULT_FLASH_TIME );

  UI_UpdateLcd();
}
#endif

/*********************************************************************
 * @fn      uiProcessIdentifyTimeChange
 *
 * @brief   Uopdate the UI when the identify time attribute's value has changed
 *
 * @param   endpoint - the endpoints which had its identify time attribute changed
 *
 * @return  none
 */
static void uiProcessIdentifyTimeChange( uint8 endpoint )
{
  if ( *pUiIdentifyTimeAttribute > 0 )
  {
    HalLedBlink ( UI_LED_IDENTIFY, 0xFF, UI_LED_IDENTIFY_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    HalLedSet ( UI_LED_IDENTIFY, HAL_LED_MODE_OFF );
  }
}

/*********************************************************************
 * @fn      uiProcessBindNotification
 *
 * @brief   Uopdate the UI when a bind is added
 *
 * @param   data - information about the new binding entry
 *
 * @return  none
 */
static void uiProcessBindNotification( bdbBindNotificationData_t *data )
{
  //note: 'data' argument contains information about the new binding, which can be useful for the user's application

  if (FBMatchesFound < 255)
  {
    FBMatchesFound++;
  }
}

/*********************************************************************
 * @fn      UI_DeviceStateUpdated
 *
 * @brief   Uopdate the UI when the device state has changed
 *
 * @param   NwkState - the new network-state of the device
 *
 * @return  none
 */
void UI_DeviceStateUpdated(devStates_t NwkState)
{
  uint8 LedDutyCycle;

  uiNwkStateShadow = NwkState;

  switch (NwkState)
  {
    case DEV_ZB_COORD:
      LedDutyCycle = UI_LED_COORDINATOR_DUTY_CYCLE;
      break;
    case DEV_ROUTER:
      LedDutyCycle = UI_LED_ROUTER_DUTY_CYCLE;
      break;
    case DEV_END_DEVICE:
      LedDutyCycle = UI_LED_END_DEVICE_DUTY_CYCLE;
      break;
    default:
      LedDutyCycle = 0;
      break;
  }
  
  HalLedBlink ( UI_LED_DEVICE_STATE, 0, LedDutyCycle, UI_LED_DEVICE_STATE_FLASH_TIME );

  UI_UpdateLcd();
}

