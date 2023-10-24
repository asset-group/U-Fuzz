/**************************************************************************************************
  Filename:       OSAL_ZNP.c
  Revised:        $Date: 2014-11-06 23:59:26 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41038 $

  Description:    This file is the Application-specific mandatory OSAL file.


  Copyright 2009-2013 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "hal_drivers.h"
#include "hal_mcu.h"
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_Tasks.h"
#include "hal_board.h"
#include "MT.h"
#include "MT_TASK.h"
#if defined ( MT_SYS_JAMMER_FEATURE )
  #include "MT_SYS.h"
#endif

#include "nwk.h"
#include "APS.h"
#include "ZDApp.h"
#if defined ( ZIGBEE_FREQ_AGILITY ) || defined ( ZIGBEE_PANID_CONFLICT )
#include "ZDNwkMgr.h"
#endif
#if defined ( ZIGBEE_FRAGMENTATION )
#include "aps_frag.h"
#endif
#if defined ( INTER_PAN )
#include "stub_aps.h"
#endif
#include "sapi.h"
#include "znp_app.h"
#if defined ( ZCL_KEY_ESTABLISH )
  #include "zcl_key_establish.h"
#endif

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#if defined ( BDB_TL_INITIATOR )
  #include "bdb_touchlink_initiator.h"
#endif // BDB_TL_INITIATOR
#if defined ( BDB_TL_TARGET )
  #include "bdb_touchlink_target.h"
#endif // BDB_TL_TARGET
#endif // INTER_PAN


#include "bdb_interface.h"
#if (ZG_BUILD_RTR_TYPE)
  #include "gp_common.h"
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */

// The order in this table must be identical to the task initialization calls below in osalInitTask.
const pTaskEventHandlerFn tasksArr[] = {
  znpEventLoop,
  macEventLoop,
  nwk_event_loop,
#if (ZG_BUILD_RTR_TYPE)
  gp_event_loop,
#endif
  APS_event_loop,
#if defined ( ZIGBEE_FRAGMENTATION )
  APSF_ProcessEvent,
#endif
  ZDApp_event_loop,
#if defined ( ZIGBEE_FREQ_AGILITY ) || defined ( ZIGBEE_PANID_CONFLICT )
  ZDNwkMgr_event_loop,
#endif
#if defined( INTER_PAN )
  StubAPS_ProcessEvent,
#endif
  // Added to include TouchLink initiator functionality
  #if defined ( BDB_TL_INITIATOR )
    touchLinkInitiator_event_loop,
  #endif
  // Added to include TouchLink target functionality
  #if defined ( BDB_TL_TARGET )
    touchLinkTarget_event_loop,
  #endif
  bdb_event_loop,    
  SAPI_ProcessEvent,
#if defined ( ZCL_KEY_ESTABLISH )
  zclKE_ProcessEvent,
#endif
  Hal_ProcessEvent,
#if defined ( MT_SYS_JAMMER_FEATURE )
  jammerEventLoop,
#endif
};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16 *tasksEvents;

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

void osal_start_znp(void);
static void osal_run_task(uint8 idx);

/*********************************************************************
 * @fn      osalInitTasks
 *
 * @brief   This function invokes the initialization function for each task.
 *
 * @param   void
 *
 * @return  none
 */
void osalInitTasks( void )
{
  uint8 taskID = 0;

  tasksEvents = (uint16 *)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
  osal_memset( tasksEvents, 0, (sizeof( uint16 ) * tasksCnt));

  znpInit( taskID++ );
  macTaskInit( taskID++ );
  nwk_init( taskID++ );
#if (ZG_BUILD_RTR_TYPE)
  gp_Init( taskID++ );
#endif
  APS_Init( taskID++ );
#if defined ( ZIGBEE_FRAGMENTATION )
  APSF_Init( taskID++ );
#endif
  ZDApp_Init( taskID++ );
#if defined ( ZIGBEE_FREQ_AGILITY ) || defined ( ZIGBEE_PANID_CONFLICT )
  ZDNwkMgr_Init( taskID++ );
#endif
#if defined( INTER_PAN )
  StubAPS_Init( taskID++ );
#endif
  // Added to include TouchLink initiator functionality 
  #if defined( BDB_TL_INITIATOR )
    touchLinkInitiator_Init( taskID++ );
  #endif
  // Added to include TouchLink target functionality 
  #if defined ( BDB_TL_TARGET )
    touchLinkTarget_Init( taskID++ );
  #endif
    bdb_Init( taskID++ );
  SAPI_Init( taskID++ );
#if defined ( ZCL_KEY_ESTABLISH )
  zclKE_Init( taskID++ );
#endif
  Hal_Init( taskID++ );
#if defined ( MT_SYS_JAMMER_FEATURE )
  jammerInit( taskID++ );
#endif
}

/*********************************************************************
 * @fn      osal_start_znp
 *
 * @brief
 *
 *   This function is the main loop function of the task system.  It
 *   will look through all task events and call the task_event_processor()
 *   function for the task with the event.  If there are no events (for
 *   all tasks), this function puts the processor into Sleep.
 *   This Function doesn't return.
 *
 * @param   void
 *
 * @return  none
 */
void osal_start_znp( void )
{
#if !defined ( ZBIT ) && !defined ( UBIT )
  for(;;)  // Forever Loop
#endif
  {
#if defined( POWER_SAVING )
    uint8 busy = FALSE;
#endif
    uint8 idx;

#ifndef HAL_BOARD_CC2538
    osalTimeUpdate();
#endif
    Hal_ProcessPoll();

    for (idx = 1; idx < tasksCnt; idx++)
    {
      if (tasksEvents[idx])
      {
        osal_run_task(idx);
#if defined( POWER_SAVING )
        busy = TRUE;
#endif
        break;
      }
    }

    if (tasksEvents[0])  // Always run the ZNP task.
    {
      osal_run_task(0);
#if defined( POWER_SAVING )
      busy = TRUE;
#endif
    }

#if defined( POWER_SAVING )
    if (!busy)  // Complete pass through all task events with no activity?
    {
      osal_pwrmgr_powerconserve();  // Put the processor/system into sleep.
    }
#endif
  }
}

/*********************************************************************
 * @fn      osal_run_task
 *
 * @brief
 *
 *   This function is the main loop function of the task system.  It
 *   will look through all task events and call the task_event_processor()
 *   function for the task with the event.  If there are no events (for
 *   all tasks), this function puts the processor into Sleep.
 *   This Function doesn't return.
 *
 * @param   void
 *
 * @return  none
 */
static void osal_run_task(uint8 idx)
{
  uint16 events;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);
  events = tasksEvents[idx];
  tasksEvents[idx] = 0;  // Clear the Events for this task.
  HAL_EXIT_CRITICAL_SECTION(intState);

  events = (tasksArr[idx])( idx, events );

  HAL_ENTER_CRITICAL_SECTION(intState);
  tasksEvents[idx] |= events;  // Add back unprocessed events to the current task.
  HAL_EXIT_CRITICAL_SECTION(intState);
}

/*********************************************************************
*********************************************************************/
