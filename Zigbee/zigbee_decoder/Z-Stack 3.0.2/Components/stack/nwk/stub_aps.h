/**************************************************************************************************
  Filename:       stub_aps.h
  Revised:        $Date: 2008-1-04 13:13:13 -0700 (Fri, 04 Jan 2008) $
  Revision:       $Revision: 1 $

  Description:    Primitives of the Stub Application Support Sub Layer Task functions.


  Copyright 2004-2011 Texas Instruments Incorporated. All rights reserved.

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

#ifndef SAPS_H
#define SAPS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "mac_api.h"
#include "ZMAC.h"
#include "APSMEDE.h"
#include "AF.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Inter-PAN frame doesn't include endpoints. Use this endpoint to distinguish the
// Inter-PAN frames passed between the Stub APS and Application.
// Note: Endpoints 0xF1 - 0xFE are currently reserved.
#define STUBAPS_INTER_PAN_EP        0xFE

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8 StubAPS_TaskID;

/*********************************************************************
 * STUB APS FUNCTIONS
 */

/*
 * Stub APS Task Initialization
 */
extern void StubAPS_Init( uint8 task_id );

/*
 * Stub APS Event Loop
 */
extern UINT16 StubAPS_ProcessEvent( uint8 task_id, uint16 events );

/*
 * This function changes the device's channel for inter-PAN communication.
 */
extern ZStatus_t StubAPS_SetInterPanChannel( uint8 channel );

/*
 * This function sets the device's channel back to the NIB channel.
 */
extern ZStatus_t StubAPS_SetIntraPanChannel( void );

/*
 * This function checks to see if a PAN is an Inter-PAN.
 */
extern uint8 StubAPS_InterPan( uint16 panId, uint8 endPoint );

/*
 * This function is used to register the application with Stub APS.
 */
extern void StubAPS_RegisterApp( endPointDesc_t *epDesc );

/*********************************************************************
 * INTER-PAN FUNCTIONS
 */

/*
 * This function requests the transfer of data from the next higher layer
 * to a single peer entity.
 */
extern ZStatus_t INTERP_DataReq( APSDE_DataReq_t *req );

/*
 * This function requests the MTU (Max Transport Unit) of the Inter-PAN
 * Data Service.
 */
extern uint8 INTERP_DataReqMTU( void );

/*
 * This function processes the data confirm from the MAC layer.
 */
extern void INTERP_DataConfirm( ZMacDataCnf_t *dataCnf );

/*
 * This function indicates the transfer of a data SPDU (MSDU)
 * from the MAC layer to the local application layer entity.
 */
extern void INTERP_DataIndication( macMcpsDataInd_t *dataInd );


/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* SAPS_H */
