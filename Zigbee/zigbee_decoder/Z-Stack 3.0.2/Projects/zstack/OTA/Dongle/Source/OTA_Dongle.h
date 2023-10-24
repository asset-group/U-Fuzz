/**************************************************************************************************
  Filename:       OTA_Dongle.h
  Revised:        $Date: 2011-08-16 16:45:58 -0700 (Tue, 16 Aug 2011) $
  Revision:       $Revision: 27200 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2010-2011 Texas Instruments Incorporated. All rights reserved.

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

#ifndef OTA_DONGLE_H
#define OTA_DONGLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define OTA_DONGLE_ENDPOINT                     13
#define OTA_DONGLE_MAX_ATTRIBUTES               12
#define OTA_DONGLE_MAX_OPTIONS                  1
#define OTA_SAMPLE_PROFILE_ID                   ZCL_OTA_SAMPLE_PROFILE_ID
#define OTA_SAMPLE_DEVICEID                     0

// Application Events
#define OTA_DONGLE_DONGLE_NOTIFY_EVT            0x0002

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t OTA_Dongle_SimpleDesc;
extern CONST zclAttrRec_t OTA_Dongle_Attrs[];
extern zclOptionRec_t OTA_Dongle_Options[];
extern uint16 OTA_Dongle_IdentifyTime;

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void OTA_Dongle_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 OTA_Dongle_event_loop( byte task_id, UINT16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OTA_DONGLE_H */
