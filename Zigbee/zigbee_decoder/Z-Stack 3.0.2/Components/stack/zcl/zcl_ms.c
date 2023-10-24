/**************************************************************************************************
  Filename:       zcl_ms.c
  Revised:        $Date: 2013-06-11 13:53:09 -0700 (Tue, 11 Jun 2013) $
  Revision:       $Revision: 34523 $

  Description:    Zigbee Cluster Library - Measurements and Sensing ( MS )


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
#include "zcl_ms.h"

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
typedef struct zclMSCBRec
{
  struct zclMSCBRec     *next;
  uint8                 endpoint; // Used to link it into the endpoint descriptor
  zclMS_AppCallbacks_t  *CBs;     // Pointer to Callback function
} zclMSCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclMSCBRec_t *zclMSCBs = (zclMSCBRec_t *)NULL;
static uint8 zclMSPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclMS_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclMS_AppCallbacks_t *zclMS_FindCallbacks( uint8 endpoint );

static ZStatus_t zclMS_ProcessIn_IlluminanceMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_IlluminanceLevelSensingCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_TemperatureMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_PressureMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_FlowMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_RelativeHumidityCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_OccupancySensingCmds( zclIncoming_t *pInMsg );

/*********************************************************************
 * @fn      zclMS_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclMS_RegisterCmdCallbacks( uint8 endpoint, zclMS_AppCallbacks_t *callbacks )
{
  zclMSCBRec_t *pNewItem;
  zclMSCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclMSPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
                        ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING,
                        zclMS_HdlIncoming );
    zclMSPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclMSCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclMSCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclMSCBs == NULL )
  {
    zclMSCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclMSCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclMS_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclMS_AppCallbacks_t *zclMS_FindCallbacks( uint8 endpoint )
{
  zclMSCBRec_t *pCBs;
  
  pCBs = zclMSCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }
  return ( (zclMS_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclMS_HdlIncoming
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
static ZStatus_t zclMS_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclMS_HdlInSpecificCommands( pInMsg );
    }
    else
    {
      // We don't support any manufacturer specific command -- ignore it.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands
    stat = ZFailure;
  }
  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclMS_AppCallbacks_t *pCBs;
  
  // make sure endpoint exists
  pCBs = (void*)zclMS_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
    return ( ZFailure );
  
  switch ( pInMsg->msg->clusterId )			
  {
    case ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT:
      stat = zclMS_ProcessIn_IlluminanceMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG:
      stat = zclMS_ProcessIn_IlluminanceLevelSensingCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
      stat = zclMS_ProcessIn_TemperatureMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT:
      stat = zclMS_ProcessIn_PressureMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT:
      stat = zclMS_ProcessIn_FlowMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
      stat = zclMS_ProcessIn_RelativeHumidityCmds( pInMsg );
      break;
      
    case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
      stat = zclMS_ProcessIn_OccupancySensingCmds( pInMsg );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_IlluminanceMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_IlluminanceMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZFailure;

  // there are no specific command for this cluster yet.
  // instead of suppressing a compiler warnings( for a code porting reasons )
  // fake unused call here and keep the code skeleton intact
 (void)pInMsg;
  if ( stat != ZFailure )
    zclMS_FindCallbacks( 0 );

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_IlluminanceLevelSensingCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_IlluminanceLevelSensingCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8 cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )				
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_TemperatureMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_TemperatureMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8 cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )				
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_PressureMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_PressureMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8 cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )				
  {
    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_FlowMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_FlowMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8 cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )				
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_RelativeHumidityCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_RelativeHumidityCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8 cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )				
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_OccupancySensingCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_OccupancySensingCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8 cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )				
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/****************************************************************************
****************************************************************************/

