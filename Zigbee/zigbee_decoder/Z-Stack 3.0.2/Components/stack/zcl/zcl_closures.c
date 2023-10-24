/**************************************************************************************************
  Filename:       zcl_closures.c
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    Zigbee Cluster Library - Closures.


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

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
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_closures.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
#ifdef ZCL_DOORLOCK
typedef struct zclClosuresDoorLockCBRec
{
  struct zclClosuresDoorLockCBRec     *next;
  uint8                                endpoint; // Used to link it into the endpoint descriptor
  zclClosures_DoorLockAppCallbacks_t  *CBs;     // Pointer to Callback function
} zclClosuresDoorLockCBRec_t;
#endif

#ifdef ZCL_WINDOWCOVERING
typedef struct zclClosuresWindowCoveringCBRec
{
  struct zclClosuresWindowCoveringCBRec     *next;
  uint8                                     endpoint; // Used to link it into the endpoint descriptor
  zclClosures_WindowCoveringAppCallbacks_t  *CBs;     // Pointer to Callback function
} zclClosuresWindowCoveringCBRec_t;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZCL_DOORLOCK
static zclClosuresDoorLockCBRec_t *zclClosuresDoorLockCBs = (zclClosuresDoorLockCBRec_t *)NULL;
#endif

#ifdef ZCL_WINDOWCOVERING
static zclClosuresWindowCoveringCBRec_t *zclClosuresWindowCoveringCBs = (zclClosuresWindowCoveringCBRec_t *)NULL;
#endif

#ifdef ZCL_DOORLOCK
static uint8 zclDoorLockPluginRegisted = FALSE;
#endif

#ifdef ZCL_WINDOWCOVERING
static uint8 zclWindowCoveringPluginRegisted = FALSE;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if defined(ZCL_DOORLOCK) || defined(ZCL_WINDOWCOVERING)
static ZStatus_t zclClosures_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclClosures_HdlInSpecificCommands( zclIncoming_t *pInMsg );
#endif

#ifdef ZCL_DOORLOCK
static zclClosures_DoorLockAppCallbacks_t *zclClosures_FindDoorLockCallbacks( uint8 endpoint );
static ZStatus_t zclClosures_ProcessInDoorLockCmds( zclIncoming_t *pInMsg,
                                                    zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLock( zclIncoming_t *pInMsg,
                                                zclClosures_DoorLockAppCallbacks_t *pCBs );
#ifdef ZCL_DOORLOCK_EXT
static ZStatus_t zclClosures_ProcessInDoorLockUnlockWithTimeout( zclIncoming_t *pInMsg,
                                                                 zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetLogRecord( zclIncoming_t *pInMsg,
                                                            zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetPINCode( zclIncoming_t *pInMsg,
                                                          zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetPINCode( zclIncoming_t *pInMsg,
                                                          zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearPINCode( zclIncoming_t *pInMsg,
                                                            zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearAllPINCodes( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetUserStatus( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetUserStatus( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetWeekDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetWeekDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearWeekDaySchedule( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetYearDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetYearDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearYearDaySchedule( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetHolidaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetHolidaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearHolidaySchedule( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetUserType( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetUserType( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetRFIDCode( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetRFIDCode( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearRFIDCode( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearAllRFIDCodes( zclIncoming_t *pInMsg,
                                                                 zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockUnlockWithTimeoutRsp( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetLogRecordRsp( zclIncoming_t *pInMsg,
                                                               zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetPINCodeRsp( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetPINCodeRsp( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearPINCodeRsp( zclIncoming_t *pInMsg,
                                                               zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearAllPINCodesRsp( zclIncoming_t *pInMsg,
                                                                   zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetUserStatusRsp( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetUserStatusRsp( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetWeekDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetWeekDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearWeekDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                       zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetYearDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetYearDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearYearDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                       zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetHolidayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetHolidayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearHolidayScheduleRsp( zclIncoming_t *pInMsg,
                                                                       zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetUserTypeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetUserTypeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockSetRFIDCodeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockGetRFIDCodeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearRFIDCodeRsp( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockClearAllRFIDCodesRsp( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockOperationEventNotification( zclIncoming_t *pInMsg,
                                                                          zclClosures_DoorLockAppCallbacks_t *pCBs );
static ZStatus_t zclClosures_ProcessInDoorLockProgrammingEventNotification( zclIncoming_t *pInMsg,
                                                                            zclClosures_DoorLockAppCallbacks_t *pCBs );
#endif //ZCL_DOORLOCK_EXT
#endif //ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
static zclClosures_WindowCoveringAppCallbacks_t *zclClosures_FindWCCallbacks( uint8 endpoint );
static ZStatus_t zclClosures_ProcessInWindowCovering( zclIncoming_t *pInMsg,
                                                      zclClosures_WindowCoveringAppCallbacks_t *pCBs );
#endif //ZCL_WINDOWCOVERING

#ifdef ZCL_DOORLOCK
/*********************************************************************
 * @fn      zclClosures_RegisterDoorLockCmdCallbacks
 *
 * @brief   Register an applications DoorLock command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclClosures_RegisterDoorLockCmdCallbacks( uint8 endpoint, zclClosures_DoorLockAppCallbacks_t *callbacks )
{
  zclClosuresDoorLockCBRec_t *pNewItem;
  zclClosuresDoorLockCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclDoorLockPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                        ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                        zclClosures_HdlIncoming );
    zclDoorLockPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclClosuresDoorLockCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclClosuresDoorLockCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclClosuresDoorLockCBs == NULL )
  {
    zclClosuresDoorLockCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclClosuresDoorLockCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclClosures_FindDoorLockCallbacks
 *
 * @brief   Find the DoorLock callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclClosures_DoorLockAppCallbacks_t *zclClosures_FindDoorLockCallbacks( uint8 endpoint )
{
  zclClosuresDoorLockCBRec_t *pCBs;

  pCBs = zclClosuresDoorLockCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclClosures_DoorLockAppCallbacks_t *)NULL );
}
#endif // ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
/*********************************************************************
 * @fn      zclClosures_RegisterWindowCoveringCmdCallbacks
 *
 * @brief   Register an applications Window Covering command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclClosures_RegisterWindowCoveringCmdCallbacks( uint8 endpoint, zclClosures_WindowCoveringAppCallbacks_t *callbacks )
{
  zclClosuresWindowCoveringCBRec_t *pNewItem;
  zclClosuresWindowCoveringCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclWindowCoveringPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING,
                        ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING,
                        zclClosures_HdlIncoming );
    zclWindowCoveringPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclClosuresWindowCoveringCBRec_t ) );
  if ( pNewItem == NULL )
  {
    return ( ZMemError );
  }

  pNewItem->next = (zclClosuresWindowCoveringCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclClosuresWindowCoveringCBs == NULL )
  {
    zclClosuresWindowCoveringCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclClosuresWindowCoveringCBs;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclClosures_FindWCCallbacks
 *
 * @brief   Find the Window Covering callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclClosures_WindowCoveringAppCallbacks_t *zclClosures_FindWCCallbacks( uint8 endpoint )
{
  zclClosuresWindowCoveringCBRec_t *pCBs;

  pCBs = zclClosuresWindowCoveringCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
    {
      return ( pCBs->CBs );
    }
    pCBs = pCBs->next;
  }
  return ( (zclClosures_WindowCoveringAppCallbacks_t *)NULL );
}
#endif // ZCL_WINDOWCOVERING

#if defined(ZCL_DOORLOCK) || defined(ZCL_WINDOWCOVERING)
/*********************************************************************
 * @fn      zclClosures_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   logicalClusterID
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
    return ( stat ); // Cluster not supported thru Inter-PAN
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      stat = zclClosures_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands -- should never get here
    stat = ZFailure;
  }
  return ( stat );
}

/*********************************************************************
 * @fn      zclClosures_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library

 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat;
#ifdef ZCL_DOORLOCK
  zclClosures_DoorLockAppCallbacks_t *pDLCBs;
#endif

#ifdef ZCL_WINDOWCOVERING
  zclClosures_WindowCoveringAppCallbacks_t *pWCCBs;
#endif

#ifdef ZCL_DOORLOCK
  // make sure endpoint exists
  pDLCBs = zclClosures_FindDoorLockCallbacks( pInMsg->msg->endPoint );
  if ( pDLCBs == NULL )
  {
    return ( ZFailure );
  }
#endif

#ifdef ZCL_WINDOWCOVERING
  // make sure endpoint exists
  pWCCBs = zclClosures_FindWCCallbacks( pInMsg->msg->endPoint );
  if ( pWCCBs == NULL )
  {
    return ( ZFailure );
  }
#endif

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK:
#ifdef ZCL_DOORLOCK
      stat = zclClosures_ProcessInDoorLockCmds( pInMsg, pDLCBs );
#endif //ZCL_DOORLOCK
      break;

    case ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING:
#ifdef ZCL_WINDOWCOVERING
      stat = zclClosures_ProcessInWindowCovering( pInMsg, pWCCBs );
#endif //ZCL_WINDOWCOVERING
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}
#endif // defined(ZCL_DOORLOCK) || defined(ZCL_WINDOWCOVERING)

#ifdef ZCL_DOORLOCK
/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockCmds
 *
 * @brief   Process in the received DoorLock Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the Application callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockCmds( zclIncoming_t *pInMsg,
                                                    zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t stat;

  // Client-to-Server
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_CLOSURES_LOCK_DOOR:
      case COMMAND_CLOSURES_UNLOCK_DOOR:
      case COMMAND_CLOSURES_TOGGLE_DOOR:
        stat = zclClosures_ProcessInDoorLock( pInMsg, pCBs );
        break;
        
#ifdef ZCL_DOORLOCK_EXT
      case COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT:
        stat = zclClosures_ProcessInDoorLockUnlockWithTimeout( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_LOG_RECORD:
        stat = zclClosures_ProcessInDoorLockGetLogRecord( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_PIN_CODE:
        stat = zclClosures_ProcessInDoorLockSetPINCode( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_PIN_CODE:
        stat = zclClosures_ProcessInDoorLockGetPINCode( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_PIN_CODE:
        stat = zclClosures_ProcessInDoorLockClearPINCode( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES:
        stat = zclClosures_ProcessInDoorLockClearAllPINCodes( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_USER_STATUS:
        stat = zclClosures_ProcessInDoorLockSetUserStatus( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_USER_STATUS:
        stat = zclClosures_ProcessInDoorLockGetUserStatus( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockSetWeekDaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockGetWeekDaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockClearWeekDaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockSetYearDaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockGetYearDaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockClearYearDaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockSetHolidaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockGetHolidaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE:
        stat = zclClosures_ProcessInDoorLockClearHolidaySchedule( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_USER_TYPE:
        stat = zclClosures_ProcessInDoorLockSetUserType( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_USER_TYPE:
        stat = zclClosures_ProcessInDoorLockGetUserType( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_RFID_CODE:
        stat = zclClosures_ProcessInDoorLockSetRFIDCode( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_RFID_CODE:
        stat = zclClosures_ProcessInDoorLockGetRFIDCode( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_RFID_CODE:
        stat = zclClosures_ProcessInDoorLockClearRFIDCode( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES:
        stat = zclClosures_ProcessInDoorLockClearAllRFIDCodes( pInMsg, pCBs );
        break;
#endif
      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }
  // Server-to-Client
  else
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_CLOSURES_LOCK_DOOR_RSP:
      case COMMAND_CLOSURES_UNLOCK_DOOR_RSP:
      case COMMAND_CLOSURES_TOGGLE_DOOR_RSP:
        stat = zclClosures_ProcessInDoorLock( pInMsg, pCBs );
        break;
        
#ifdef ZCL_DOORLOCK_EXT
        
      case COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT_RSP:
        stat = zclClosures_ProcessInDoorLockUnlockWithTimeoutRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_LOG_RECORD_RSP:
        stat = zclClosures_ProcessInDoorLockGetLogRecordRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_PIN_CODE_RSP:
        stat = zclClosures_ProcessInDoorLockSetPINCodeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_PIN_CODE_RSP:
        stat = zclClosures_ProcessInDoorLockGetPINCodeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_PIN_CODE_RSP:
        stat = zclClosures_ProcessInDoorLockClearPINCodeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES_RSP:
        stat = zclClosures_ProcessInDoorLockClearAllPINCodesRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_USER_STATUS_RSP:
        stat = zclClosures_ProcessInDoorLockSetUserStatusRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_USER_STATUS_RSP:
        stat = zclClosures_ProcessInDoorLockGetUserStatusRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockSetWeekDayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockGetWeekDayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockClearWeekDayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockSetYearDayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockGetYearDayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockClearYearDayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockSetHolidayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockGetHolidayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE_RSP:
        stat = zclClosures_ProcessInDoorLockClearHolidayScheduleRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_USER_TYPE_RSP:
        stat = zclClosures_ProcessInDoorLockSetUserTypeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_USER_TYPE_RSP:
        stat = zclClosures_ProcessInDoorLockGetUserTypeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_SET_RFID_CODE_RSP:
        stat = zclClosures_ProcessInDoorLockSetRFIDCodeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_GET_RFID_CODE_RSP:
        stat = zclClosures_ProcessInDoorLockGetRFIDCodeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_RFID_CODE_RSP:
        stat = zclClosures_ProcessInDoorLockClearRFIDCodeRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES_RSP:
        stat = zclClosures_ProcessInDoorLockClearAllRFIDCodesRsp( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_OPERATION_EVENT_NOTIFICATION:
        stat = zclClosures_ProcessInDoorLockOperationEventNotification( pInMsg, pCBs );
        break;

      case COMMAND_CLOSURES_PROGRAMMING_EVENT_NOTIFICATION:
        stat = zclClosures_ProcessInDoorLockProgrammingEventNotification( pInMsg, pCBs );
        break;
        
#endif //ZCL_DOORLOCK_EXT

      default:
        // Unknown command
        stat = ZFailure;
        break;
    }
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLock
 *
 * @brief   Process in the received Door Lock cmds
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLock( zclIncoming_t *pInMsg,
                                                zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t status;

  // Client-to-Server
  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_CLOSURES_LOCK_DOOR:
      case COMMAND_CLOSURES_UNLOCK_DOOR:
      case COMMAND_CLOSURES_TOGGLE_DOOR:
        if ( pCBs->pfnDoorLock )
        {
          uint8 i;
          uint8 calculatedArrayLen;
          zclDoorLock_t cmd;

          // first octet of PIN/RFID Code variable string identifies its length
          calculatedArrayLen = pInMsg->pData[0] + 1; // add first byte of string

          cmd.pPinRfidCode = zcl_mem_alloc( calculatedArrayLen );

          if ( !cmd.pPinRfidCode )
          {
            return ( ZMemError );  // no memory
          }

          for ( i = 0; i < calculatedArrayLen; i++ )
          {
            cmd.pPinRfidCode[i] = pInMsg->pData[i];
          }

          status = ( pCBs->pfnDoorLock( pInMsg, &cmd ) );
          zcl_mem_free( cmd.pPinRfidCode );
          return status;
        }

        return ( ZCL_STATUS_FAILURE );
        break;

      default:
        return ( ZFailure );   // Error ignore the command
    }
  }
  // Server-to-Client
  else
  {
    switch(pInMsg->hdr.commandID)
    {
      case COMMAND_CLOSURES_LOCK_DOOR_RSP:
      case COMMAND_CLOSURES_UNLOCK_DOOR_RSP:
      case COMMAND_CLOSURES_TOGGLE_DOOR_RSP:
        if ( pCBs->pfnDoorLockRsp )
        {
          return ( pCBs->pfnDoorLockRsp( pInMsg, pInMsg->pData[0] ) );
        }

        return ( ZCL_STATUS_FAILURE );
        break;

      default:
        return ( ZFailure );   // Error ignore the command
    }
  }
}

#ifdef ZCL_DOORLOCK_EXT
/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockUnlockWithTimeout
 *
 * @brief   Process in the received Unlock With Timeout cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockUnlockWithTimeout( zclIncoming_t *pInMsg,
                                                                 zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t status;

  if ( pCBs->pfnDoorLockUnlockWithTimeout )
  {
    uint8 i;
    uint8 offset;
    uint8 calculatedArrayLen;
    zclDoorLockUnlockTimeout_t cmd;

    // first octet of PIN/RFID Code variable string identifies its length
    calculatedArrayLen = pInMsg->pData[2] + 1;  // add first byte of string

    cmd.pPinRfidCode = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pPinRfidCode )
    {
      return ( ZMemError );  // no memory
    }

    cmd.timeout = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    offset = 2;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pPinRfidCode[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockUnlockWithTimeout( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pPinRfidCode );
    return status;
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetLogRecord
 *
 * @brief   Process in the received Get Log Record cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetLogRecord( zclIncoming_t *pInMsg,
                                                            zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  if ( pCBs->pfnDoorLockGetLogRecord )
  {
    zclDoorLockGetLogRecord_t cmd;

    cmd.logIndex = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

    return ( pCBs->pfnDoorLockGetLogRecord( pInMsg, &cmd ) );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetPINCode
 *
 * @brief   Process in the received Set PIN Code cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetPINCode( zclIncoming_t *pInMsg,
                                                          zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetPINCode )
 {
    uint8 i;
    uint8 offset;
    uint8 calculatedArrayLen;
    zclDoorLockSetPINCode_t cmd;
    ZStatus_t status;

    // first octet of PIN/RFID Code variable string identifies its length
    calculatedArrayLen = pInMsg->pData[4] + 1; // add first byte of string

    cmd.pPIN = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pPIN )
    {
      return ( ZMemError );  // no memory
    }

    cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.userStatus = pInMsg->pData[2];
    cmd.userType = pInMsg->pData[3];
    offset = 4;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pPIN[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockSetPINCode( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pPIN );
    return status;
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetPINCode
 *
 * @brief   Process in the received Get PIN Code cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetPINCode( zclIncoming_t *pInMsg,
                                                          zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetPINCode )
 {
   zclDoorLockUserID_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

   return ( pCBs->pfnDoorLockGetPINCode( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearPINCode
 *
 * @brief   Process in the received Clear PIN Code cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearPINCode( zclIncoming_t *pInMsg,
                                                            zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearPINCode )
 {
   zclDoorLockUserID_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

   return ( pCBs->pfnDoorLockClearPINCode( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearAllPINCodes
 *
 * @brief   Process in the received Clear All PIN Codes cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearAllPINCodes( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearAllPINCodes )
 {
   // no payload

   return ( pCBs->pfnDoorLockClearAllPINCodes( pInMsg ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetUserStatus
 *
 * @brief   Process in the received Set User Status cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetUserStatus( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetUserStatus )
 {
   zclDoorLockSetUserStatus_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
   cmd.userStatus = pInMsg->pData[2];

   return ( pCBs->pfnDoorLockSetUserStatus( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetUserStatus
 *
 * @brief   Process in the received Get User Status cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetUserStatus( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetUserStatus )
 {
   zclDoorLockUserID_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

   return ( pCBs->pfnDoorLockGetUserStatus( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetWeekDaySchedule
 *
 * @brief   Process in the received Set Week Day Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetWeekDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetWeekDaySchedule )
 {
   zclDoorLockSetWeekDaySchedule_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
   cmd.daysMask = pInMsg->pData[3];
   cmd.startHour = pInMsg->pData[4];
   cmd.startMinute = pInMsg->pData[5];
   cmd.endHour = pInMsg->pData[6];
   cmd.endMinute = pInMsg->pData[7];

   return ( pCBs->pfnDoorLockSetWeekDaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetWeekDaySchedule
 *
 * @brief   Process in the received Get Week Day Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetWeekDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetWeekDaySchedule )
 {
   zclDoorLockSchedule_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

   return ( pCBs->pfnDoorLockGetWeekDaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearWeekDaySchedule
 *
 * @brief   Process in the received Clear Week Day Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearWeekDaySchedule( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearWeekDaySchedule )
 {
   zclDoorLockSchedule_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

   return ( pCBs->pfnDoorLockClearWeekDaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}
/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetYearDaySchedule
 *
 * @brief   Process in the received Set Year Day Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetYearDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetYearDaySchedule )
 {
   zclDoorLockSetYearDaySchedule_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
   cmd.zigBeeLocalStartTime = BUILD_UINT32( pInMsg->pData[3], pInMsg->pData[4], pInMsg->pData[5], pInMsg->pData[6] );
   cmd.zigBeeLocalEndTime = BUILD_UINT32( pInMsg->pData[7], pInMsg->pData[8], pInMsg->pData[9], pInMsg->pData[10] );

   return ( pCBs->pfnDoorLockSetYearDaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetYearDaySchedule
 *
 * @brief   Process in the received Get Year Day Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetYearDaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetYearDaySchedule )
 {
   zclDoorLockSchedule_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

   return ( pCBs->pfnDoorLockGetYearDaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearYearDaySchedule
 *
 * @brief   Process in the received Clear Year Day Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearYearDaySchedule( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearYearDaySchedule )
 {
   zclDoorLockSchedule_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );

   return ( pCBs->pfnDoorLockClearYearDaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetHolidaySchedule
 *
 * @brief   Process in the received Set Holiday Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetHolidaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetHolidaySchedule )
 {
   zclDoorLockSetHolidaySchedule_t cmd;

   cmd.holidayScheduleID = pInMsg->pData[0];
   cmd.zigBeeLocalStartTime = BUILD_UINT32( pInMsg->pData[1], pInMsg->pData[2], pInMsg->pData[3], pInMsg->pData[4] );
   cmd.zigBeeLocalEndTime = BUILD_UINT32( pInMsg->pData[5], pInMsg->pData[6], pInMsg->pData[7], pInMsg->pData[8] );
   cmd.operatingModeDuringHoliday = pInMsg->pData[9];

   return ( pCBs->pfnDoorLockSetHolidaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetHolidaySchedule
 *
 * @brief   Process in the received Get Holiday Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetHolidaySchedule( zclIncoming_t *pInMsg,
                                                                  zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetHolidaySchedule )
 {
   zclDoorLockHolidayScheduleID_t cmd;

   cmd.holidayScheduleID = pInMsg->pData[0];

   return ( pCBs->pfnDoorLockGetHolidaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearHolidaySchedule
 *
 * @brief   Process in the received Clear Holiday Schedule cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearHolidaySchedule( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearHolidaySchedule )
 {
   zclDoorLockHolidayScheduleID_t cmd;

   cmd.holidayScheduleID = pInMsg->pData[0];

   return ( pCBs->pfnDoorLockClearHolidaySchedule( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetUserType
 *
 * @brief   Process in the received Set User Type cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetUserType( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetUserType )
 {
   zclDoorLockSetUserType_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
   cmd.userType = pInMsg->pData[2];

   return ( pCBs->pfnDoorLockSetUserType( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetUserType
 *
 * @brief   Process in the received Get User Type cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetUserType( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetUserType )
 {
   zclDoorLockUserID_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

   return ( pCBs->pfnDoorLockGetUserType( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetRFIDCode
 *
 * @brief   Process in the received Set RFID Code cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetRFIDCode( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t status;

  if ( pCBs->pfnDoorLockSetRFIDCode )
  {
    uint8 i;
    uint8 offset;
    uint8 calculatedArrayLen;
    zclDoorLockSetRFIDCode_t cmd;

    // first octet of PIN/RFID Code variable string identifies its length
    calculatedArrayLen = pInMsg->pData[4] + 1;  // add first byte of string

    cmd.pRfidCode = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pRfidCode )
    {
      return ( ZMemError );  // no memory
    }

    cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.userStatus = pInMsg->pData[2];
    cmd.userType = pInMsg->pData[3];
    offset = 4;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pRfidCode[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockSetRFIDCode( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pRfidCode );
    return status;
  }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetRFIDCode
 *
 * @brief   Process in the received Get RFID Code cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetRFIDCode( zclIncoming_t *pInMsg,
                                                           zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetRFIDCode )
 {
   zclDoorLockUserID_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

   return ( pCBs->pfnDoorLockGetRFIDCode( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearRFIDCode
 *
 * @brief   Process in the received Clear RFID Code cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearRFIDCode( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearRFIDCode )
 {
   zclDoorLockUserID_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );

   return ( pCBs->pfnDoorLockClearRFIDCode( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearAllRFIDCodes
 *
 * @brief   Process in the received Clear All RFID Codes cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearAllRFIDCodes( zclIncoming_t *pInMsg,
                                                                 zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearAllRFIDCodes )
 {
   // no payload

   return ( pCBs->pfnDoorLockClearAllRFIDCodes( pInMsg ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockUnlockWithTimeoutRsp
 *
 * @brief   Process in the received Unlock With Timeout Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockUnlockWithTimeoutRsp( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockUnlockWithTimeoutRsp )
 {
   return ( pCBs->pfnDoorLockUnlockWithTimeoutRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetLogRecordRsp
 *
 * @brief   Process in the received Get Log Record Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetLogRecordRsp( zclIncoming_t *pInMsg,
                                                               zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t status;

  if ( pCBs->pfnDoorLockGetLogRecordRsp )
  {
    uint8 i;
    uint8 offset;
    uint8 calculatedArrayLen;
    zclDoorLockGetLogRecordRsp_t cmd;

    // first octet of PIN/RFID Code variable string identifies its length
    calculatedArrayLen = pInMsg->pData[11] + 1;  // add first byte of string

    cmd.pPIN = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pPIN )
    {
      return ( ZMemError );  // no memory
    }

    cmd.logEntryID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.timestamp = BUILD_UINT32( pInMsg->pData[2], pInMsg->pData[3], pInMsg->pData[4], pInMsg->pData[5] );
    cmd.eventType = pInMsg->pData[6];
    cmd.source = pInMsg->pData[7];
    cmd.eventIDAlarmCode = pInMsg->pData[8];
    cmd.userID = BUILD_UINT16( pInMsg->pData[9], pInMsg->pData[10] );
    offset = 11;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pPIN[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockGetLogRecordRsp( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pPIN );
    return status;
  }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetPINCodeRsp
 *
 * @brief   Process in the received Set PIN Code Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetPINCodeRsp( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetPINCodeRsp )
 {
   return ( pCBs->pfnDoorLockSetPINCodeRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetPINCodeRsp
 *
 * @brief   Process in the received Get PIN Code Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetPINCodeRsp( zclIncoming_t *pInMsg,
                                                             zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t status;

  if ( pCBs->pfnDoorLockGetPINCodeRsp )
  {
    uint8 i;
    uint8 offset;
    uint8 calculatedArrayLen;
    zclDoorLockGetPINCodeRsp_t cmd;

    // first octet of PIN/RFID Code variable string identifies its length
    calculatedArrayLen = pInMsg->pData[4] + 1; // add first byte of string

    cmd.pCode = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pCode )
    {
      return ( ZMemError );  // no memory
    }

    cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.userStatus = pInMsg->pData[2];
    cmd.userType = pInMsg->pData[3];
    offset = 4;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pCode[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockGetPINCodeRsp( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pCode );
    return status;
  }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearPINCodeRsp
 *
 * @brief   Process in the received Clear PIN Code Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearPINCodeRsp( zclIncoming_t *pInMsg,
                                                               zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearPINCodeRsp )
 {
   return ( pCBs->pfnDoorLockClearPINCodeRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearAllPINCodesRsp
 *
 * @brief   Process in the received Clear All PIN Codes Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearAllPINCodesRsp( zclIncoming_t *pInMsg,
                                                                   zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearAllPINCodesRsp )
 {
   return ( pCBs->pfnDoorLockClearAllPINCodesRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetUserStatusRsp
 *
 * @brief   Process in the received Set User Status Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetUserStatusRsp( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetUserStatusRsp )
 {
   return ( pCBs->pfnDoorLockSetUserStatusRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetUserStatusRsp
 *
 * @brief   Process in the received Get User Status Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetUserStatusRsp( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetUserStatusRsp )
 {
   zclDoorLockGetUserStatusRsp_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
   cmd.userStatus = pInMsg->pData[2];

   return ( pCBs->pfnDoorLockGetUserStatusRsp( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetWeekDayScheduleRsp
 *
 * @brief   Process in the received Set Week Day Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetWeekDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetWeekDayScheduleRsp )
 {
   return ( pCBs->pfnDoorLockSetWeekDayScheduleRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetWeekDayScheduleRsp
 *
 * @brief   Process in the received Get Week Day Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetWeekDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetWeekDayScheduleRsp )
 {
   zclDoorLockGetWeekDayScheduleRsp_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
   cmd.status = pInMsg->pData[3];

   if ( cmd.status == ZCL_STATUS_SUCCESS )
   {
     cmd.daysMask = pInMsg->pData[4];
     cmd.startHour = pInMsg->pData[5];
     cmd.startMinute = pInMsg->pData[6];
     cmd.endHour = pInMsg->pData[7];
     cmd.endMinute = pInMsg->pData[8];
   }

   return ( pCBs->pfnDoorLockGetWeekDayScheduleRsp( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearWeekDayScheduleRsp
 *
 * @brief   Process in the received Clear Week Day Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearWeekDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                       zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearWeekDayScheduleRsp )
 {
   return ( pCBs->pfnDoorLockClearWeekDayScheduleRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetYearDayScheduleRsp
 *
 * @brief   Process in the received Set Year Day Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetYearDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetYearDayScheduleRsp )
 {
   return ( pCBs->pfnDoorLockSetYearDayScheduleRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetYearDayScheduleRsp
 *
 * @brief   Process in the received Get Year Day Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetYearDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetYearDayScheduleRsp )
 {
   zclDoorLockGetYearDayScheduleRsp_t cmd;

   cmd.scheduleID = pInMsg->pData[0];
   cmd.userID = BUILD_UINT16( pInMsg->pData[1], pInMsg->pData[2] );
   cmd.status = pInMsg->pData[3];

   if ( cmd.status == ZCL_STATUS_SUCCESS )
   {
     cmd.zigBeeLocalStartTime = BUILD_UINT32( pInMsg->pData[4], pInMsg->pData[5], pInMsg->pData[6], pInMsg->pData[7] );
     cmd.zigBeeLocalEndTime = BUILD_UINT32( pInMsg->pData[8], pInMsg->pData[9], pInMsg->pData[10], pInMsg->pData[11] );
   }

   return ( pCBs->pfnDoorLockGetYearDayScheduleRsp( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearYearDayScheduleRsp
 *
 * @brief   Process in the received Clear Year Day Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearYearDayScheduleRsp( zclIncoming_t *pInMsg,
                                                                       zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearYearDayScheduleRsp )
 {
   return ( pCBs->pfnDoorLockClearYearDayScheduleRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetHolidayScheduleRsp
 *
 * @brief   Process in the received Set Holiday Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetHolidayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetHolidayScheduleRsp )
 {
   return ( pCBs->pfnDoorLockSetHolidayScheduleRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetHolidayScheduleRsp
 *
 * @brief   Process in the received Get Holiday Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetHolidayScheduleRsp( zclIncoming_t *pInMsg,
                                                                     zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetHolidayScheduleRsp )
 {
   zclDoorLockGetHolidayScheduleRsp_t cmd;

   cmd.holidayScheduleID = pInMsg->pData[0];
   cmd.status = pInMsg->pData[1];

   if ( cmd.status == ZCL_STATUS_SUCCESS )
   {
     cmd.zigBeeLocalStartTime = BUILD_UINT32( pInMsg->pData[2], pInMsg->pData[3], pInMsg->pData[4], pInMsg->pData[5] );
     cmd.zigBeeLocalEndTime = BUILD_UINT32( pInMsg->pData[6], pInMsg->pData[7], pInMsg->pData[8], pInMsg->pData[9] );
     cmd.operatingModeDuringHoliday = pInMsg->pData[10];
   }

   return ( pCBs->pfnDoorLockGetHolidayScheduleRsp( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearHolidayScheduleRsp
 *
 * @brief   Process in the received Clear Holiday Schedule Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearHolidayScheduleRsp( zclIncoming_t *pInMsg,
                                                                       zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearHolidayScheduleRsp )
 {
   return ( pCBs->pfnDoorLockClearHolidayScheduleRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetUserTypeRsp
 *
 * @brief   Process in the received Set User Type Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetUserTypeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetUserTypeRsp )
 {
   return ( pCBs->pfnDoorLockSetUserTypeRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetUserTypeRsp
 *
 * @brief   Process in the received Get User Type Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetUserTypeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockGetUserTypeRsp )
 {
   zclDoorLockGetUserTypeRsp_t cmd;

   cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
   cmd.userType = pInMsg->pData[2];

   return ( pCBs->pfnDoorLockGetUserTypeRsp( pInMsg, &cmd ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockSetRFIDCodeRsp
 *
 * @brief   Process in the received Set RFID Code Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockSetRFIDCodeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockSetRFIDCodeRsp )
 {
   return ( pCBs->pfnDoorLockSetRFIDCodeRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockGetRFIDCodeRsp
 *
 * @brief   Process in the received Get RFID Code Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockGetRFIDCodeRsp( zclIncoming_t *pInMsg,
                                                              zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  ZStatus_t status;

  if ( pCBs->pfnDoorLockGetRFIDCodeRsp )
  {
    uint8 i;
    uint8 offset;
    uint8 calculatedArrayLen;
    zclDoorLockGetRFIDCodeRsp_t cmd;

    // first octet of PIN/RFID Code variable string identifies its length
    calculatedArrayLen = pInMsg->pData[4] + 1; // add first byte of string

    cmd.pRfidCode = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pRfidCode )
    {
      return ( ZMemError );  // no memory
    }

    cmd.userID = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
    cmd.userStatus = pInMsg->pData[2];
    cmd.userType = pInMsg->pData[3];
    offset = 4;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pRfidCode[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockGetRFIDCodeRsp( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pRfidCode );
    return status;
  }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearRFIDCodeRsp
 *
 * @brief   Process in the received Clear RFID Code Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearRFIDCodeRsp( zclIncoming_t *pInMsg,
                                                                zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearRFIDCodeRsp )
 {
   return ( pCBs->pfnDoorLockClearRFIDCodeRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockClearAllRFIDCodesRsp
 *
 * @brief   Process in the received Clear All RFID Codes Response cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockClearAllRFIDCodesRsp( zclIncoming_t *pInMsg,
                                                                    zclClosures_DoorLockAppCallbacks_t *pCBs )
{
 if ( pCBs->pfnDoorLockClearAllRFIDCodesRsp )
 {
   return ( pCBs->pfnDoorLockClearAllRFIDCodesRsp( pInMsg, pInMsg->pData[0] ) );
 }

 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockOperationEventNotification
 *
 * @brief   Process in the received Operation Event Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockOperationEventNotification( zclIncoming_t *pInMsg,
                                                                          zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;
  uint8 calculatedArrayLen;
  zclDoorLockOperationEventNotification_t cmd;
  ZStatus_t status;

 if ( pCBs->pfnDoorLockOperationEventNotification )
 {
    calculatedArrayLen = pInMsg->pData[9] + 1;  // add first byte of string

    cmd.pData = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pData )
    {
      return ( ZMemError );  // no memory
    }

    cmd.operationEventSource = pInMsg->pData[0];
    cmd.operationEventCode = pInMsg->pData[1];
    cmd.userID = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
    cmd.pin = pInMsg->pData[4];
    cmd.zigBeeLocalTime = BUILD_UINT32( pInMsg->pData[5], pInMsg->pData[6], pInMsg->pData[7], pInMsg->pData[8] );
    offset = 9;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pData[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockOperationEventNotification( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pData );
    return status;
  }
 return ( ZFailure );
}

/*********************************************************************
 * @fn      zclClosures_ProcessInDoorLockProgrammingEventNotification
 *
 * @brief   Process in the received Programming Event Notification cmd
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to the application callbacks
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInDoorLockProgrammingEventNotification( zclIncoming_t *pInMsg,
                                                                            zclClosures_DoorLockAppCallbacks_t *pCBs )
{
  uint8 i;
  uint8 offset;
  uint8 calculatedArrayLen;
  zclDoorLockProgrammingEventNotification_t cmd;
  ZStatus_t status;

  if ( pCBs->pfnDoorLockProgrammingEventNotification )
  {
    calculatedArrayLen = pInMsg->pData[11] + 1; // add first byte of string

    cmd.pData = zcl_mem_alloc( calculatedArrayLen );
    if ( !cmd.pData )
    {
      return ( ZMemError );  // no memory
    }

    cmd.programEventSource = pInMsg->pData[0];
    cmd.programEventCode = pInMsg->pData[1];
    cmd.userID = BUILD_UINT16( pInMsg->pData[2], pInMsg->pData[3] );
    cmd.pin = pInMsg->pData[4];
    cmd.userType = pInMsg->pData[5];
    cmd.userStatus = pInMsg->pData[6];
    cmd.zigBeeLocalTime = BUILD_UINT32( pInMsg->pData[7], pInMsg->pData[8], pInMsg->pData[9], pInMsg->pData[10] );
    offset = 11;
    for ( i = 0; i < calculatedArrayLen; i++ )
    {
      cmd.pData[i] = pInMsg->pData[offset++];
    }

    status = ( pCBs->pfnDoorLockProgrammingEventNotification( pInMsg, &cmd ) );
    zcl_mem_free( cmd.pData );
    return status;
  }

 return ( ZFailure );
}

#endif //ZCL_DOORLOCK_EXT

/*********************************************************************
 * @fn      zclClosures_SendDoorLockRequest
 *
 * @brief   Call to send out a Door Lock Lock/Unlock/Toggle Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_CLOSURES_LOCK_DOOR, COMMAND_CLOSURES_UNLOCK_DOOR, COMMAND_CLOSURES_TOGGLE_DOOR
 * @param   pPayload:
 *           aPinRfidCode - PIN/RFID code in ZCL Octet String Format
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                           zclDoorLock_t *pPayload,
                                           uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedBufSize = pPayload->pPinRfidCode[0] + 1;  // add first byte of string

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  for ( i = 0; i < calculatedBufSize; i++ )
  {
    pBuf[i] = pPayload->pPinRfidCode[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockUnlockTimeoutRequest
 *
 * @brief   Call to send out a Unlock with Timeout Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           timeout - The timeout in seconds
 *           aPinRfidCode - PIN/RFID code in ZCL Octet String Format
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockUnlockTimeoutRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                        zclDoorLockUnlockTimeout_t *pPayload,
                                                        uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedArrayLen = pPayload->pPinRfidCode[0] + 1;  // add first byte of string

  // determine total size of buffer
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_UNLOCK_TIMEOUT;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->timeout);
  pBuf[1] = HI_UINT16(pPayload->timeout);
  offset = 2;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pPinRfidCode[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetLogRecordRequest
 *
 * @brief   Call to send out a Get Log Record Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   logIndex - Log number between 1 - [max log attribute]
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetLogRecordRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                       uint16 logIndex, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_GET_LOG_RECORD];

  payload[0] = LO_UINT16( logIndex );
  payload[1] = HI_UINT16( logIndex );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_GET_LOG_RECORD, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_GET_LOG_RECORD, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetPINCodeRequest
 *
 * @brief   Call to send out a Set PIN Code Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           pPIN - A ZigBee string indicating the PIN code used to create the event on the door lock
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetPINCodeRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                     zclDoorLockSetPINCode_t *pPayload,
                                                     uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedArrayLen = pPayload->pPIN[0] + 1;  // add first byte of string

  // determine the total buffer size
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_SET_PIN_CODE;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->userID);
  pBuf[1] = HI_UINT16(pPayload->userID);
  pBuf[2] = pPayload->userStatus;
  pBuf[3] = pPayload->userType;
  offset = 4;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pPIN[i];
  }


  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_SET_PIN_CODE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockUserIDRequest
 *
 * @brief   Call to send out a User ID Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_CLOSURES_GET_PIN_CODE, COMMAND_CLOSURES_CLEAR_PIN_CODE,
 *                COMMAND_CLOSURES_GET_USER_STATUS, COMMAND_CLOSURES_GET_USER_TYPE,
 *                COMMAND_CLOSURES_GET_RFID_CODE, COMMAND_CLOSURES_CLEAR_RFID_CODE
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockUserIDRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                 uint16 userID, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_USER_ID];

  payload[0] = LO_UINT16(userID);
  payload[1] = HI_UINT16(userID);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_USER_ID, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockClearAllCodesRequest
 *
 * @brief   Call to send out a Clear All Codes Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES, COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockClearAllCodesRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                        uint8 disableDefaultRsp, uint8 seqNum )
{
  // no payload

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 0, NULL );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetUserStatusRequest
 *
 * @brief   Call to send out a Set User Status Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userStatus - Used to indicate what the status is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetUserStatusRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                        uint16 userID, uint8 userStatus,
                                                        uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_SET_USER_STATUS];

  payload[0] = LO_UINT16(userID);
  payload[1] = HI_UINT16(userID);
  payload[2] = userStatus;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_SET_USER_STATUS, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_SET_USER_STATUS, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetWeekDayScheduleRequest
 *
 * @brief   Call to send out a Set Week Day Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   daysMask - Bitmask of the effective days in the order XSFTWTMS
 * @param   startHour - The start hour of the Week Day Schedule: 0-23
 * @param   startMinute - The start minute of the Week Day Schedule: 0-59
 * @param   endHour - The end hour of the Week Day Schedule: 0-23
 * @param   endMinute - The end minute of the Week Day Schedule: 0-59
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetWeekDayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint8 scheduleID, uint16 userID,
                                                             uint8 daysMask, uint8 startHour,
                                                             uint8 startMinute, uint8 endHour,
                                                             uint8 endMinute, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_SET_WEEK_DAY_SCHEDULE];

  payload[0] = scheduleID;
  payload[1] = LO_UINT16(userID);
  payload[2] = HI_UINT16(userID);
  payload[3] = daysMask;
  payload[4] = startHour;
  payload[5] = startMinute;
  payload[6] = endHour;
  payload[7] = endMinute;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_SET_WEEK_DAY_SCHEDULE, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockScheduleRequest
 *
 * @brief   Call to send out a Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE, COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE,
 *                COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE, COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE
 * @param   scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                   uint8 scheduleID, uint16 userID,
                                                   uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_SCHEDULE];

  payload[0] = scheduleID;
  payload[1] = LO_UINT16(userID);
  payload[2] = HI_UINT16(userID);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_SCHEDULE, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetYearDayScheduleRequest
 *
 * @brief   Call to send out a Set Year Day Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetYearDayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint8 scheduleID, uint16 userID,
                                                             uint32 zigBeeLocalStartTime,
                                                             uint32 zigBeeLocalEndTime,
                                                             uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_SET_YEAR_DAY_SCHEDULE];

  payload[0] = scheduleID;
  payload[1] = LO_UINT16(userID);
  payload[2] = HI_UINT16(userID);
  payload[3] = BREAK_UINT32(zigBeeLocalStartTime, 0);
  payload[4] = BREAK_UINT32(zigBeeLocalStartTime, 1);
  payload[5] = BREAK_UINT32(zigBeeLocalStartTime, 2);
  payload[6] = BREAK_UINT32(zigBeeLocalStartTime, 3);
  payload[7] = BREAK_UINT32(zigBeeLocalEndTime, 0);
  payload[8] = BREAK_UINT32(zigBeeLocalEndTime, 1);
  payload[9] = BREAK_UINT32(zigBeeLocalEndTime, 2);
  payload[10] = BREAK_UINT32(zigBeeLocalEndTime, 3);

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_SET_YEAR_DAY_SCHEDULE, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetHolidayScheduleRequest
 *
 * @brief   Call to send out a Set Holiday Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   holidayScheduleID - A unique ID for given Holiday Schedule (0 to 254)
 * @param   zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   operatingModeDuringHoliday - A valid enumeration value as listed in operating mode attribute
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetHolidayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                             uint8 holidayScheduleID,
                                                             uint32 zigBeeLocalStartTime,
                                                             uint32 zigBeeLocalEndTime,
                                                             uint8 operatingModeDuringHoliday,
                                                             uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_SET_HOLIDAY_SCHEDULE];

  payload[0] = holidayScheduleID;
  payload[1] = BREAK_UINT32(zigBeeLocalStartTime, 0);
  payload[2] = BREAK_UINT32(zigBeeLocalStartTime, 1);
  payload[3] = BREAK_UINT32(zigBeeLocalStartTime, 2);
  payload[4] = BREAK_UINT32(zigBeeLocalStartTime, 3);
  payload[5] = BREAK_UINT32(zigBeeLocalEndTime, 0);
  payload[6] = BREAK_UINT32(zigBeeLocalEndTime, 1);
  payload[7] = BREAK_UINT32(zigBeeLocalEndTime, 2);
  payload[8] = BREAK_UINT32(zigBeeLocalEndTime, 3);
  payload[9] = operatingModeDuringHoliday;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_SET_HOLIDAY_SCHEDULE, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockHolidayScheduleRequest
 *
 * @brief   Call to send out a Holiday Schedule Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE, COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE
 * @param   holidayScheduleID - A unique ID for given Holiday Schedule (0 to 254)
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockHolidayScheduleRequest( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmd,
                                                          uint8 holidayScheduleID,
                                                          uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_HOLIDAY_SCHEDULE];

  payload[0] = holidayScheduleID;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_HOLIDAY_SCHEDULE, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetUserTypeRequest
 *
 * @brief   Call to send out a Set User Type Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userType - Used to indicate what the type is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetUserTypeRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint16 userID, uint8 userType,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_SET_USER_TYPE];

  payload[0] = LO_UINT16(userID);
  payload[1] = HI_UINT16(userID);
  payload[2] = userType;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_SET_USER_TYPE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_SET_USER_TYPE, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockSetRFIDCodeRequest
 *
 * @brief   Call to send out a Set RFID Code Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           aRfidCode - A ZigBee string indicating the RFID code used to create the event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockSetRFIDCodeRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                                      zclDoorLockSetRFIDCode_t *pPayload,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedArrayLen = pPayload->pRfidCode[0] + 1;   // add first byte of string

  // determine the total buffer size
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_SET_RFID_CODE;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->userID);
  pBuf[1] = HI_UINT16(pPayload->userID);
  pBuf[2] = pPayload->userStatus;
  pBuf[3] = pPayload->userType;
  offset = 4;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pRfidCode[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_SET_RFID_CODE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockStatusResponse
 *
 * @brief   Call to send out a Status Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - COMMAND_CLOSURES_LOCK_DOOR_RSP, COMMAND_CLOSURES_UNLOCK_DOOR_RSP
 *                COMMAND_CLOSURES_TOGGLE_RSP, COMMAND_CLOSURES_UNLOCK_WITH_TIMEOUT_RSP,
 *                COMMAND_CLOSURES_SET_PIN_CODE_RSP, COMMAND_CLOSURES_CLEAR_PIN_CODE_RSP,
 *                COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES_RSP, COMMAND_CLOSURES_SET_USER_STATUS_RSP,
 *                COMMAND_CLOSURES_SET_WEEK_DAY_SCHEDULE_RSP, COMMAND_CLOSURES_CLEAR_WEEK_DAY_SCHEDULE_RSP,
 *                COMMAND_CLOSURES_SET_YEAR_DAY_SCHEDULE_RSP, COMMAND_CLOSURES_CLEAR_YEAR_DAY_SCHEDULE_RSP,
 *                COMMAND_CLOSURES_SET_HOLIDAY_SCHEDULE_RSP, COMMAND_CLOSURES_CLEAR_HOLIDAY_SCHEDULE_RSP,
 *                COMMAND_CLOSURES_SET_USER_TYPE_RSP, COMMAND_CLOSURES_SET_RFID_CODE_RSP,
 *                COMMAND_CLOSURES_CLEAR_RFID_CODE_RSP, COMMAND_CLOSURES_CLEAR_ALL_RFID_CODES_RSP
 * @param   status - Returns the state due to the requesting command
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockStatusResponse( uint8 srcEP, afAddrType_t *dstAddr,uint8 cmd,
                                                  uint8 status, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[1];   // 1 byte payload

  payload[0] = status;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          cmd, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, sizeof( payload ), payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetLogRecordResponse
 *
 * @brief   Call to send out a Get Log Record Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           logEntryID - The index into the log table where this log entry is stored
 *           timestamp - A ZigBeeLocalTime used to timestamp all events and alarms on the door lock
 *           eventType - Indicates the type of event that took place on the door lock
 *           source - A source value of available sources
 *           eventIDAlarmCode - A one byte value indicating the type of event that took place on the door lock
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           aPIN - A ZigBee string indicating the PIN code used to create the event on the door lock
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetLogRecordResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                        zclDoorLockGetLogRecordRsp_t *pPayload,
                                                        uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedArrayLen = pPayload->pPIN[0] + 1;   // add first byte of string

  // determine the total buffer size
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_GET_LOG_RECORD_RSP;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->logEntryID);
  pBuf[1] = HI_UINT16(pPayload->logEntryID);
  pBuf[2] = BREAK_UINT32(pPayload->timestamp, 0);
  pBuf[3] = BREAK_UINT32(pPayload->timestamp, 1);
  pBuf[4] = BREAK_UINT32(pPayload->timestamp, 2);
  pBuf[5] = BREAK_UINT32(pPayload->timestamp, 3);
  pBuf[6] = pPayload->eventType;
  pBuf[7] = pPayload->source;
  pBuf[8] = pPayload->eventIDAlarmCode;
  pBuf[9] = LO_UINT16(pPayload->userID);
  pBuf[10] = HI_UINT16(pPayload->userID);
  offset = 11;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pPIN[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_GET_LOG_RECORD_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetPINCodeResponse
 *
 * @brief   Call to send out a Get PIN Code Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           aCode - Returned PIN number
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetPINCodeResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                      zclDoorLockGetPINCodeRsp_t *pPayload,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedArrayLen = pPayload->pCode[0] + 1;   // add first byte of string

  // determine the total buffer size
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_GET_PIN_CODE_RSP;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->userID);
  pBuf[1] = HI_UINT16(pPayload->userID);
  pBuf[2] = pPayload->userStatus;
  pBuf[3] = pPayload->userType;
  offset = 4;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pCode[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_GET_PIN_CODE_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetUserStatusResponse
 *
 * @brief   Call to send out a Get User Status Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userStatus - Used to indicate what the status is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetUserStatusResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                         uint16 userID, uint8 userStatus,
                                                         uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_GET_USER_STATUS_RSP];

  payload[0] = LO_UINT16(userID);
  payload[1] = HI_UINT16(userID);
  payload[2] = userStatus;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_GET_USER_STATUS_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_GET_USER_STATUS_RSP, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetWeekDayScheduleResponse
 *
 * @brief   Call to send out a Get Week Day Schedule Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd:
 *               scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 *               userID - User ID is between 0 - [# PINs User supported attribute]
 *               status - Returns the state due to the requesting command
 *               daysMask - Bitmask of the effective days in the order XSFTWTMS
 *               startHour - The start hour of the Week Day Schedule: 0-23
 *               startMinute - The start minute of the Week Day Schedule: 0-59
 *               endHour - The end hour of the Week Day Schedule: 0-23
 *               endMinute - The end minute of the Week Day Schedule: 0-59
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetWeekDayScheduleResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                              zclDoorLockGetWeekDayScheduleRsp_t *pCmd,
                                                              uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_GET_WEEK_DAY_SCHEDULE_RSP];
  uint8 payloadLen = sizeof( payload );

  payload[0] = pCmd->scheduleID;
  payload[1] = LO_UINT16( pCmd->userID );
  payload[2] = HI_UINT16( pCmd->userID );
  payload[3] = pCmd->status;

  if ( pCmd->status == ZCL_STATUS_SUCCESS )
  {
    payload[4] = pCmd->daysMask;
    payload[5] = pCmd->startHour;
    payload[6] = pCmd->startMinute;
    payload[7] = pCmd->endHour;
    payload[8] = pCmd->endMinute;
  }
  else
  {
    payloadLen = 4;
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_GET_WEEK_DAY_SCHEDULE_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, payloadLen, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetYearDayScheduleResponse
 *
 * @brief   Call to send out a Get Year Day Schedule Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd:
 *             scheduleID - The Schedule ID # is between 0 - [# Schedule IDs per user attribute]
 *             userID - User ID is between 0 - [# PINs User supported attribute]
 *             status - Returns the state due to the requesting command
 *             zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 *             zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetYearDayScheduleResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                              zclDoorLockGetYearDayScheduleRsp_t *pCmd,
                                                              uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_GET_YEAR_DAY_SCHEDULE_RSP];
  uint8 payloadLen = sizeof( payload );

  payload[0] = pCmd->scheduleID;
  payload[1] = LO_UINT16( pCmd->userID );
  payload[2] = HI_UINT16( pCmd->userID );
  payload[3] = pCmd->status;

  if ( pCmd->status == ZCL_STATUS_SUCCESS )
  {
    payload[4] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 0 );
    payload[5] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 1 );
    payload[6] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 2 );
    payload[7] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 3 );
    payload[8] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 0 );
    payload[9] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 1 );
    payload[10] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 2 );
    payload[11] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 3 );
  }
  else
  {
    payloadLen = 4;
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_GET_YEAR_DAY_SCHEDULE_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, payloadLen, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetHolidayScheduleResponse
 *
 * @brief   Call to send out a Get Holiday Schedule Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd:
 *             holidayScheduleID - A unique ID for given Holiday Schedule (0 to 254)
 *             status - Returns the state due to the requesting command
 *             zigBeeLocalStartTime - Start time of the Year Day Schedule representing by ZigBeeLocalTime
 *             zigBeeLocalEndTime - End time of the Year Day Schedule representing by ZigBeeLocalTime
 *             operatingModeDuringHoliday - A valid enumeration value as listed in operating mode attribute
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetHolidayScheduleResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                              zclDoorLockGetHolidayScheduleRsp_t *pCmd,
                                                              uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_GET_HOLIDAY_SCHEDULE_RSP];
  uint8 payloadLen = sizeof( payload );

  payload[0] = pCmd->holidayScheduleID;
  payload[1] = pCmd->status;

  if ( pCmd->status == ZCL_STATUS_SUCCESS )
  {
    payload[2] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 0 );
    payload[3] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 1 );
    payload[4] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 2 );
    payload[5] = BREAK_UINT32( pCmd->zigBeeLocalStartTime, 3 );
    payload[6] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 0 );
    payload[7] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 1 );
    payload[8] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 2 );
    payload[9] = BREAK_UINT32( pCmd->zigBeeLocalEndTime, 3 );
    payload[10] = pCmd->operatingModeDuringHoliday;
  }
  else
  {
    payloadLen = 2;
  }

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_GET_HOLIDAY_SCHEDULE_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, payloadLen, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetUserTypeResponse
 *
 * @brief   Call to send out a Get User Type Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   userID - User ID is between 0 - [# PINs User supported attribute]
 * @param   userType - Used to indicate what the type is for a specific User ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetUserTypeResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                       uint16 userID, uint8 userType,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[PAYLOAD_LEN_GET_USER_TYPE_RSP];

  payload[0] = LO_UINT16(userID);
  payload[1] = HI_UINT16(userID);
  payload[2] = userType;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                          COMMAND_CLOSURES_GET_USER_TYPE_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                          disableDefaultRsp, 0, seqNum, PAYLOAD_LEN_GET_USER_TYPE_RSP, payload );
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockGetRFIDCodeResponse
 *
 * @brief   Call to send out a Get RFID Code Response
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           userType - Used to indicate what the type is for a specific User ID
 *           aRfidCode - A ZigBee string indicating the RFID code used to create the event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockGetRFIDCodeResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                                       zclDoorLockGetRFIDCodeRsp_t *pPayload,
                                                       uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint8 calculatedBufSize;
  ZStatus_t status;

  // first octet of PIN/RFID Code variable string identifies its length
  calculatedArrayLen = pPayload->pRfidCode[0] + 1;   // add first byte of string

  // determine total size of buffer
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_GET_RFID_CODE_RSP;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = LO_UINT16(pPayload->userID);
  pBuf[1] = HI_UINT16(pPayload->userID);
  pBuf[2] = pPayload->userStatus;
  pBuf[3] = pPayload->userType;
  offset = 4;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pRfidCode[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_GET_RFID_CODE_RSP, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockOperationEventNotification
 *
 * @brief   Call to send out a Operation Event Notification
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           operationEventSource - Indicates where the event was triggered from
 *           operationEventCode - (Optional) a notification whenever there is a significant operation event on the lock
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           pin - The PIN that is associated with the User ID who performed the event
 *           zigBeeLocalTime - Indicates when the event is triggered
 *           aData - Used to pass data associated with a particular event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockOperationEventNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                              zclDoorLockOperationEventNotification_t *pPayload,
                                                              uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint16 calculatedBufSize;
  ZStatus_t status;

  // set variable length if data is available
  calculatedArrayLen = pPayload->pData[0] + 1; // add first byte of string

  // determine total size of buffer
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_OPERATION_EVENT_NOTIFICATION;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->operationEventSource;
  pBuf[1] = pPayload->operationEventCode;
  pBuf[2] = LO_UINT16( pPayload->userID );
  pBuf[3] = HI_UINT16( pPayload->userID );
  pBuf[4] = pPayload->pin;
  pBuf[5] = BREAK_UINT32(pPayload->zigBeeLocalTime, 0);
  pBuf[6] = BREAK_UINT32(pPayload->zigBeeLocalTime, 1);
  pBuf[7] = BREAK_UINT32(pPayload->zigBeeLocalTime, 2);
  pBuf[8] = BREAK_UINT32(pPayload->zigBeeLocalTime, 3);
  offset = 9;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pData[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_OPERATION_EVENT_NOTIFICATION, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

/*********************************************************************
 * @fn      zclClosures_SendDoorLockProgrammingEventNotification
 *
 * @brief   Call to send out a Programming Event Notification
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *           programEventSource - Indicates where the event was triggered from
 *           programEventCode - (Optional) a notification whenever there is a significant programming event on the lock
 *           userID - User ID is between 0 - [# PINs User supported attribute]
 *           pin - The PIN that is associated with the User ID who performed the event
 *           userType - Used to indicate what the type is for a specific User ID
 *           userStatus - Used to indicate what the status is for a specific User ID
 *           zigBeeLocalTime - Indicates when the event is triggered
 *           dataLen - Manufacture specific, describes length of aData
 *           aData - Used to pass data associated with a particular event
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_SendDoorLockProgrammingEventNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                zclDoorLockProgrammingEventNotification_t *pPayload,
                                                                uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 i;
  uint8 *pBuf;  // variable length payload
  uint8 offset;
  uint8 calculatedArrayLen;
  uint16 calculatedBufSize;
  ZStatus_t status;

  // set variable length if data is available
  calculatedArrayLen = pPayload->pData[0] + 1;  // add first byte of string

  // determine total size of buffer
  calculatedBufSize = calculatedArrayLen + PAYLOAD_LEN_PROGRAMMING_EVENT_NOTIFICATION;

  pBuf = zcl_mem_alloc( calculatedBufSize );
  if ( !pBuf )
  {
    return ( ZMemError );  // no memory
  }

  // over-the-air is always little endian. Break into a byte stream.
  pBuf[0] = pPayload->programEventSource;
  pBuf[1] = pPayload->programEventCode;
  pBuf[2] = LO_UINT16( pPayload->userID );
  pBuf[3] = HI_UINT16( pPayload->userID );
  pBuf[4] = pPayload->pin;
  pBuf[5] = pPayload->userType;
  pBuf[6] = pPayload->userStatus;
  pBuf[7] = BREAK_UINT32(pPayload->zigBeeLocalTime, 0);
  pBuf[8] = BREAK_UINT32(pPayload->zigBeeLocalTime, 1);
  pBuf[9] = BREAK_UINT32(pPayload->zigBeeLocalTime, 2);
  pBuf[10] = BREAK_UINT32(pPayload->zigBeeLocalTime, 3);
  offset = 11;
  for ( i = 0; i < calculatedArrayLen; i++ )
  {
    pBuf[offset++] = pPayload->pData[i];
  }

  status = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
                            COMMAND_CLOSURES_PROGRAMMING_EVENT_NOTIFICATION, TRUE, ZCL_FRAME_SERVER_CLIENT_DIR,
                            disableDefaultRsp, 0, seqNum, calculatedBufSize, pBuf );
  zcl_mem_free( pBuf );
  return status;
}

#endif //ZCL_DOORLOCK

#ifdef ZCL_WINDOWCOVERING
/*********************************************************************
 * @fn      zclClosures_ProcessInWindowCovering
 *
 * @brief   Process in the received Window Covering cluster Command.
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclClosures_ProcessInWindowCovering( zclIncoming_t *pInMsg,
                                                      zclClosures_WindowCoveringAppCallbacks_t *pCBs )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;
  uint8 *pData = pInMsg->pData;

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    switch ( pInMsg->hdr.commandID )
    {
    case COMMAND_CLOSURES_UP_OPEN:
      if ( pCBs->pfnWindowCoveringUpOpen )
      {
        pCBs->pfnWindowCoveringUpOpen();
      }
      break;

    case COMMAND_CLOSURES_DOWN_CLOSE:
      if ( pCBs->pfnWindowCoveringDownClose )
      {
        pCBs->pfnWindowCoveringDownClose();
      }
      break;

    case COMMAND_CLOSURES_STOP:
      if ( pCBs->pfnWindowCoveringStop )
      {
        pCBs->pfnWindowCoveringStop();
      }
      break;

    case COMMAND_CLOSURES_GO_TO_LIFT_VALUE:
      if ( pCBs->pfnWindowCoveringGotoLiftValue )
      {
        if ( pCBs->pfnWindowCoveringGotoLiftValue( BUILD_UINT16( pData[0], pData[1] ) ) == FALSE )
        {
          status = ZCL_STATUS_INVALID_VALUE;
        }
      }
      break;

    case COMMAND_CLOSURES_GO_TO_LIFT_PERCENTAGE:
      if ( pCBs->pfnWindowCoveringGotoLiftPercentage )
      {
        if ( pCBs->pfnWindowCoveringGotoLiftPercentage( pData[0] ) == FALSE )
        {
          status = ZCL_STATUS_INVALID_VALUE;
        }
      }
      break;

    case COMMAND_CLOSURES_GO_TO_TILT_VALUE:
      if ( pCBs->pfnWindowCoveringGotoTiltValue )
      {
        if ( pCBs->pfnWindowCoveringGotoTiltValue( BUILD_UINT16( pData[0], pData[1] ) ) == FALSE )
        {
          status = ZCL_STATUS_INVALID_VALUE;
        }
      }
      break;

    case COMMAND_CLOSURES_GO_TO_TILT_PERCENTAGE:
      if ( pCBs->pfnWindowCoveringGotoTiltPercentage )
      {
        if ( pCBs->pfnWindowCoveringGotoTiltPercentage( pData[0] ) == FALSE )
        {
          status = ZCL_STATUS_INVALID_VALUE;
        }
      }
      break;

    default:
      return( ZFailure );
    }
  }
  // no Client command

  return ( status );
}

/*********************************************************************
 * @fn      zclClosures_WindowCoveringSimpleReq
 *
 * @brief   Call to send out a Window Covering command with no payload
 *          as Up/Open, Down/Close or Stop
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - Command ID
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_WindowCoveringSimpleReq( uint8 srcEP, afAddrType_t *dstAddr,
                                               uint8 cmd, uint8 disableDefaultRsp, uint8 seqNum )
{
  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum, 0, NULL );
}

/*********************************************************************
 * @fn      zclClosures_WindowCoveringSendGoToValueReq
 *
 * @brief   Call to send out a Go to Value Request Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - Command ID for COMMAND_CLOSURES_GO_TO_LIFT_VALUE
 * @param   liftValue - payload
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_WindowCoveringSendGoToValueReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                      uint8 cmd, uint16 Value,
                                                      uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[ZCL_WC_GOTOVALUEREQ_PAYLOADLEN];

  buf[0] = LO_UINT16( Value );
  buf[1] = HI_UINT16( Value );

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum,
                          ZCL_WC_GOTOVALUEREQ_PAYLOADLEN, buf );
}

/*********************************************************************
 * @fn      zclClosures_WindowCoveringSendGoToPercentageReq
 *
 * @brief   Call to send out a Go to Percentage Request Command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmd - Command ID e.g. COMMAND_CLOSURES_GO_TO_LIFT_PERCENTAGE
 * @param   percentageLiftValue - payload
 * @param   disableDefaultRsp - decides default response is necessary or not
 * @param   seqNum - sequence number of the command packet
 *
 * @return  ZStatus_t
 */
ZStatus_t zclClosures_WindowCoveringSendGoToPercentageReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                           uint8 cmd, uint8 percentageValue,
                                                           uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[ZCL_WC_GOTOPERCENTAGEREQ_PAYLOADLEN];

  buf[0] = percentageValue;

  return zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING,
                          cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          disableDefaultRsp, 0, seqNum,
                          ZCL_WC_GOTOPERCENTAGEREQ_PAYLOADLEN, buf );
}
#endif //ZCL_WINDOWCOVERING

/********************************************************************************************
*********************************************************************************************/
