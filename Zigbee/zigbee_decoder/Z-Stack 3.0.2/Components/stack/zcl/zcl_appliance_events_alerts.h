/**************************************************************************************************
  Filename:       zcl_appliance_events_alerts.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Events & Alerts definitions.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_APPLIANCE_EVENTS_ALERTS_H
#define ZCL_APPLIANCE_EVENTS_ALERTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZCL_APPLIANCE_EVENTS_ALERTS

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */

  // Server Commands Received
#define COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS            0x00  // no payload

  // Server Commands Generated
#define COMMAND_APPLIANCE_EVENTS_ALERTS_GET_ALERTS_RSP        0x00  // zclApplianceEventsAlerts_t
#define COMMAND_APPLIANCE_EVENTS_ALERTS_ALERTS_NOTIFICATION   0x01  // zclApplianceEventsAlerts_t
#define COMMAND_APPLIANCE_EVENTS_ALERTS_EVENT_NOTIFICATION    0x02  // zclApplianceEventsAlertsEventNotification_t

  // Event ID values
#define APPLIANCE_EVENT_ID_END_OF_CYCLE                       0x01  // End of the working cycle reached
#define APPLIANCE_EVENT_ID_TEMPERATURE_REACHED                0x04  // Set temperature reached
#define APPLIANCE_EVENT_ID_END_OF_COOKING                     0x05  // End of cooking process
#define APPLIANCE_EVENT_ID_SWITCHING_OFF                      0x06
#define APPLIANCE_EVENT_ID_WRONG_DATA                         0xf7

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

  /*** ZCL Appliance Events & Alerts Cluster: Get Alerts Response & Alerts Notification payload ***/
typedef struct
{
  uint8 aAlert[3];     // each alert is 24 bits
} alertStructureRecord_t;

typedef struct
{
  uint8 alertsCount;
  alertStructureRecord_t *pAlertStructureRecord;  // variable length array based on alertsCount
} zclApplianceEventsAlerts_t;

/*** ZCL Appliance Events & Alerts Cluster: Event Notification Command payload ***/
typedef struct
{
  uint8 eventHeader;
  uint8 eventID;      // e.g. APPLIANCE_EVENT_ID_END_OF_CYCLE
} zclApplianceEventsAlertsEventNotification_t;


typedef ZStatus_t (*zclAppliance_Events_Alerts_GetAlerts_t)( void );
typedef ZStatus_t (*zclAppliance_Events_Alerts_GetAlertsRsp_t)( zclApplianceEventsAlerts_t *pCmd );
typedef ZStatus_t (*zclAppliance_Events_Alerts_AlertsNotification_t)( zclApplianceEventsAlerts_t *pCmd );
typedef ZStatus_t (*zclAppliance_Events_Alerts_EventNotification_t)( zclApplianceEventsAlertsEventNotification_t *pCmd );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclAppliance_Events_Alerts_GetAlerts_t                  pfnApplianceEventsAlerts_GetAlerts;
  zclAppliance_Events_Alerts_GetAlertsRsp_t               pfnApplianceEventsAlerts_GetAlertsRsp;
  zclAppliance_Events_Alerts_AlertsNotification_t         pfnApplianceEventsAlerts_AlertsNotification;
  zclAppliance_Events_Alerts_EventNotification_t          pfnApplianceEventsAlerts_EventNotification;
} zclApplianceEventsAlerts_AppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*** Register for callbacks from this cluster library ***/
extern ZStatus_t zclApplianceEventsAlerts_RegisterCmdCallbacks( uint8 endpoint, zclApplianceEventsAlerts_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_GetAlerts
 *
 * @brief   Request sent to server for Get Alerts.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_GetAlerts( uint8 srcEP, afAddrType_t *dstAddr,
                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_GetAlertsRsp
 *
 * @brief   Response sent to client for Get Alerts Response.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          alertsCount - Contains the length of the alert structure array
 *          aAlert - Contains the information of the Alert
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_GetAlertsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                             zclApplianceEventsAlerts_t *pPayload,
                                                             uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_AlertsNotification
 *
 * @brief   Response sent to client for Alerts Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          alertsCount - Contains the length of the alert structure array
 *          aAlert - Contains the information of the Alert
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_AlertsNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                   zclApplianceEventsAlerts_t *pPayload,
                                                                   uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclApplianceEventsAlerts_Send_EventNotification
 *
 * @brief   Response sent to client for Event Notification.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   eventHeader - a reserved field set to 0
 * @param   eventID - Identifies the event to be notified
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclApplianceEventsAlerts_Send_EventNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint8 eventHeader, uint8 eventID,
                                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif //ZCL_APPLIANCE_EVENTS_ALERTS
#endif /* ZCL_APPLIANCE_EVENTS_ALERTS_H */
