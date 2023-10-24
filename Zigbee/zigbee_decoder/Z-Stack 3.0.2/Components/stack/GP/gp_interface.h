/**************************************************************************************************
  Filename:       gp_Interface.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Green Power interface.


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

#ifndef GP_INTERFACE_H
#define GP_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif
  
  
  
/*********************************************************************
 * INCLUDES
 */
#include "ZDObject.h"
  
/*********************************************************************
 * MACROS
 */
  
  
  

 /*********************************************************************
 * CONSTANTS
 */

/* The time the Green Power EndPoint of the proxy keeps the information on the received
GPDF, in order to filter out duplicates. By default 2 seconds */

#define gpDuplicateTimeout    2000

  
/* Time that the Basic proxy device will be absent of the operational network due 
  to bidirectional commissioning Section A.3.9.1 step 8*/  
#define gpBirectionalCommissioningChangeChannelTimeout    5000  
  
/* Per GP spec section A.1.5.2.1.1 the minimum gpTxQueue entry is 1 */
#define GP_TX_QUEUE_MAX_ENTRY          1     
  
 /*********************************************************************
 * TYPEDEFS
 */
  
typedef uint8 (*gpChangeChannelReq_t) (void);
typedef void  (*gpCommissioningMode_t) (bool);

/*********************************************************************
 * GLOBAL VARIABLES
 */
 
 
/*********************************************************************
 * FUNCTION MACROS
 */
 
 

/*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief GP Register for chaching channel
 */
extern void gp_RegisterGPChangeChannelReqForBDBCB(gpChangeChannelReq_t gpChangeChannelReq);

/*
 * @brief GP Task Event Processing Function
 */
extern UINT16 gp_event_loop( uint8 task_id, UINT16 events );


/*
 * @brief       Initialization function for the Green Power Stubs.
 */
extern void gp_Init( byte task_id );


/*
 * @brief   Register a callback in which the application will be notified about a change
 *          of channel for at most gpBirectionalCommissioningChangeChannelTimeout ms
 *          to perform GP bidirectional commissioning in the channel parameter.
 */
extern void gp_RegisterGPChangeChannelReqCB(gpChangeChannelReq_t gpChangeChannelReq);   
  
/*
 * @brief   Register a callback in which the application will be notified about 
 *          entering/exiting GP commissioning mode (enter = TRUE, Exit = FALSE)
 */
extern void gp_RegisterCommissioningModeCB(gpCommissioningMode_t gpCommissioningMode);
  





#ifdef __cplusplus
}
#endif


#endif /* GP_INTERFACE_H */
 
 
 
 
 
 
 
 
 
 
