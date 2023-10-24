/**************************************************************************************************
  Filename:       OSAL_pwrmgr.c
  Revised:        $Date: 2014-11-21 16:17:37 -0800 (Fri, 21 Nov 2014) $
  Revision:       $Revision: 41218 $

  Description:    This file contains the OSAL Power Management API.


  Copyright 2004-2014 Texas Instruments Incorporated. All rights reserved.

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

#include "comdef.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_Timers.h"
#include "OSAL_PwrMgr.h"
#include "ZGlobals.h"

#ifdef USE_ICALL
#include <ICall.h>
#endif /* USE_ICALL */

#ifdef OSAL_PORT2TIRTOS
/* Direct port to TI-RTOS API */
#if defined CC26XX
#include <ti/sysbios/family/arm/cc26xx/Power.h>
#include <ti/sysbios/family/arm/cc26xx/PowerCC2650.h>
#endif /* CC26XX */
#endif /* OSAL_PORT2TIRTOS */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/* This global variable stores the power management attributes.
 */
pwrmgr_attribute_t pwrmgr_attribute;
#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
uint8 pwrmgr_initialized = FALSE;
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      osal_pwrmgr_init
 *
 * @brief   Initialize the power management system.
 *
 * @param   none.
 *
 * @return  none.
 */
void osal_pwrmgr_init( void )
{
#if !defined USE_ICALL && !defined OSAL_PORT2TIRTOS
#if defined POWER_SAVING && ZSTACK_END_DEVICE_BUILD                  
  pwrmgr_attribute.pwrmgr_device = PWRMGR_BATTERY;   // Default to power conservation for ZED if power saving enabled.
#else
  pwrmgr_attribute.pwrmgr_device = PWRMGR_ALWAYS_ON; // No power conservation for routing devices.
#endif
#endif /* USE_ICALL */
  pwrmgr_attribute.pwrmgr_task_state = 0;            // Cleared.  All set to conserve
#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
  pwrmgr_initialized = TRUE;
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */
}

#if !defined USE_ICALL && !defined OSAL_PORT2TIRTOS
/*********************************************************************
 * @fn      osal_pwrmgr_device
 *
 * @brief   Sets the device power characteristic.
 *
 * @param   pwrmgr_device - type of power devices. With PWRMGR_ALWAYS_ON
 *          selection, there is no power savings and the device is most
 *          likely on mains power. The PWRMGR_BATTERY selection allows the
 *          HAL sleep manager to enter sleep.
 *
 * @return  none
 */
void osal_pwrmgr_device( uint8 pwrmgr_device )
{
  pwrmgr_attribute.pwrmgr_device = pwrmgr_device;
}
#endif /* !defined USE_ICALL && !defined OSAL_PORT2TIRTOS*/

/*********************************************************************
 * @fn      osal_pwrmgr_task_state
 *
 * @brief   This function is called by each task to state whether or
 *          not this task wants to conserve power.
 *
 * @param   task_id - calling task ID.
 *          state - whether the calling task wants to
 *          conserve power or not.
 *
 * @return  SUCCESS if task complete
 */
uint8 osal_pwrmgr_task_state( uint8 task_id, uint8 state )
{
  halIntState_t intState;

  if ( task_id >= tasksCnt )
    return ( INVALID_TASK );

#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
  if ( !pwrmgr_initialized )
  {
    /* If voting is made before this module is initialized,
     * pwrmgr_task_state will reset later when the module is
     * initialized, and cause incorrect activity count.
     */
    return ( SUCCESS );
  }
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */

  HAL_ENTER_CRITICAL_SECTION( intState );

  if ( state == PWRMGR_CONSERVE )
  {
#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
    uint16 cache = pwrmgr_attribute.pwrmgr_task_state;
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */
    // Clear the task state flag
    pwrmgr_attribute.pwrmgr_task_state &= ~(1 << task_id );
#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
    if (cache != 0 && pwrmgr_attribute.pwrmgr_task_state == 0)
    {
#ifdef USE_ICALL
      /* Decrement activity counter */
      ICall_pwrUpdActivityCounter(FALSE);
#else /* USE_ICALL */
      Power_releaseConstraint(Power_SD_DISALLOW);
      Power_releaseConstraint(Power_SB_DISALLOW);
#endif /* USE_ICALL */
    }
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */
  }
  else
  {
#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
    if (pwrmgr_attribute.pwrmgr_task_state == 0)
    {
#ifdef USE_ICALL
      /* Increment activity counter */
      ICall_pwrUpdActivityCounter(TRUE);
#else /* USE_ICALL */
      Power_setConstraint(Power_SD_DISALLOW);
      Power_setConstraint(Power_SB_DISALLOW);
#endif /* USE_ICALL */
    }
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */
    // Set the task state flag
    pwrmgr_attribute.pwrmgr_task_state |= (1 << task_id);
  }

  HAL_EXIT_CRITICAL_SECTION( intState );

  return ( SUCCESS );
}

#if defined( POWER_SAVING ) && !(defined USE_ICALL || defined OSAL_PORT2TIRTOS)
/*********************************************************************
 * @fn      osal_pwrmgr_powerconserve
 *
 * @brief   This function is called from the main OSAL loop when there are
 *          no events scheduled and shouldn't be called from anywhere else.
 *
 * @param   none.
 *
 * @return  none.
 */
void osal_pwrmgr_powerconserve( void )
{
  uint32        next;
  halIntState_t intState;

  // Should we even look into power conservation
  if ( pwrmgr_attribute.pwrmgr_device != PWRMGR_ALWAYS_ON )
  {
    // Are all tasks in agreement to conserve
    if ( pwrmgr_attribute.pwrmgr_task_state == 0 )
    {
      // Hold off interrupts.
      HAL_ENTER_CRITICAL_SECTION( intState );

      // Get next time-out
      next = osal_next_timeout();

      // Re-enable interrupts.
      HAL_EXIT_CRITICAL_SECTION( intState );

      // Put the processor into sleep mode
      OSAL_SET_CPU_INTO_SLEEP( next );
    }
  }
}
#endif /* POWER_SAVING */

/*********************************************************************
*********************************************************************/
