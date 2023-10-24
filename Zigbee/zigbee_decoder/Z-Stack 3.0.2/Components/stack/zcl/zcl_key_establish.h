/**************************************************************************************************
  Filename:       zcl_key_establish.h
  Revised:        $Date: 2014-11-06 23:59:26 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41038 $

  Description:    This file contains the ZCL (Smart Energy) Key Establishment definitions.


  Copyright 2007-2014 Texas Instruments Incorporated. All rights reserved.

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
#ifndef ZCL_KEY_ESTABLISH_H
#define ZCL_KEY_ESTABLISH_H

#ifdef __cplusplus
extern "C"
{
#endif


/**************************************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "OSAL.h"
#include "zcl.h"


/**************************************************************************************************
 * CONSTANTS
 */

// KE task endpoint
#define ZCL_KE_ENDPOINT  10

// ZCL Attribute constants
#define ATTRID_KE_SUITE  0x0000
#define ZCL_KE_SUITE_1   0x0001
#define ZCL_KE_SUITE_2   0x0002

// ZCL_KE_NOTIFY_STATUS
#define ZCL_KE_NOTIFY_SUCCESS         0x00 // Key establishment successful
#define ZCL_KE_NOTIFY_TIMEOUT         0x01 // Timeout
#define ZCL_KE_NOTIFY_TERMINATE_RCVD  0x02 // Terminate command recieved from partner
#define ZCL_KE_NOTIFY_TERMINATE_SENT  0x03 // Terminate command sent to partner
#define ZCL_KE_NOTIFY_BUSY            0x04 // Client/server connections busy OR no resources
#define ZCL_KE_NOTIFY_NO_EXT_ADDR     0x05 // Partner extended address not found
#define ZCL_KE_NOTIFY_BAD_SUITE       0x06 // Suite not supported on device
#define ZCL_KE_NOTIFY_NO_CERTS        0x07 // No certs installed
#define ZCL_KE_NOTIFY_NO_EP_MATCH     0x08 // Partner's Match_Desc_rsp has no CBKE endpoint
#define ZCL_KE_NOTIFY_NO_SUITE_MATCH  0x09 // Partner's supported suites do not match device's

// ZCL_KE_TERMINATE_ERROR
#define ZCL_KE_TERMINATE_ERROR_NONE          0x00
#define ZCL_KE_TERMINATE_UNKNOWN_ISSUER      0x01
#define ZCL_KE_TERMINATE_BAD_KEY_CONFIRM     0x02
#define ZCL_KE_TERMINATE_BAD_MESSAGE         0x03
#define ZCL_KE_TERMINATE_NO_RESOURCES        0x04
#define ZCL_KE_TERMINATE_UNSUPPORTED_SUITE   0x05
#define ZCL_KE_TERMINATE_INVALID_CERTIFICATE 0x06


/**************************************************************************************************
 * TYPEDEFS
 */

// ZCL_KEY_ESTABLISH_IND message payload
typedef struct
{
  osal_event_hdr_t hdr; //hdr::status -- see ZCL_KE_NOTIFY_STATUS
  uint16 partnerNwkAddr;
  uint8 terminateError; // see ZCL_KE_TERMINATE_ERROR
  uint16 suites; // only valid if terminateError set
  uint8 waitTime; // only valid if terminateError set
} zclKE_StatusInd_t;


/**************************************************************************************************
 * PUBLIC FUNCTIONS
 */

/**************************************************************************************************
 * @fn      zclKE_HdlGeneralCmd
 *
 * @brief   Handle general cluster commands in ZCL_STANDALONE mode.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  void
 */
extern void zclKE_HdlGeneralCmd( zclIncoming_t *pInMsg );

/**************************************************************************************************
 * @fn      zclKE_ECDSASignGetLen
 *
 * @brief   Returns length required for zclKE_ECDSASign "pOutBuf" field.
 *
 * @param   suite - selected security suite
 *
 * @return  uint8 - length for zclKE_ECDSASign "pOutBuf" field
 */
extern uint8 zclKE_ECDSASignGetLen( uint16 suite );

/**************************************************************************************************
 * @fn      zclKE_ECDSASign
 *
 * @brief   Creates an ECDSA signature of a message digest.
 *
 * @param   suite - selected security suite
 * @param   pInBuf - input buffer
 * @param   inBufLen - input buffer length
 * @param   pOutBuf - output buffer ( length == zclKE_ECDSASignGetLen )
 *
 * @return  ZStatus_t - status
 */
extern ZStatus_t zclKE_ECDSASign( uint16 suite, uint8 *pInBuf, uint8 inBufLen, uint8 *pOutBuf );

/**************************************************************************************************
 * @fn      zclKE_Start
 *
 * @brief   Start key establishment with selected partner at the nwkAddr.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   partnerNwkAddr - partner network address
 * @param   transSeqNum - starting transaction sequence number
 *
 * @return  ZStatus_t - status
 */
extern ZStatus_t zclKE_Start( uint8 taskID, uint16 partnerNwkAddr, uint8 transSeqNum );

/**************************************************************************************************
 * @fn      zclKE_StartDirect
 *
 * @brief   Start key establishment directly with partner at the pPartnerAddr.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   pPartnerAddr - valid partner short address and end point
 * @param   transSeqNum - starting transaction sequence number
 * @param   suite - selected security suite
 *
 * @return  ZStatus_t - status
 */
extern ZStatus_t zclKE_StartDirect( uint8 taskID, afAddrType_t *pPartnerAddr,
                                    uint8 transSeqNum, uint16 suite );

/**************************************************************************************************
 * @fn      zclKE_Init
 *
 * @brief   Initialization function for the application.
 *
 * @param   taskID - OSAL task ID
 *
 * @return  void
 */
void zclKE_Init( uint8 taskID );

/**************************************************************************************************
 * @fn      zclKE_ProcessEvent
 *
 * @brief   Process all events for the task.
 *
 * @param   taskID - OSAL task ID
 * @param   events - OSAL event mask
 *
 * @return  uint16 - OSAL events not process
 */
extern uint16 zclKE_ProcessEvent( uint8 taskID, uint16 events );


/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_KEY_ESTABLISH_H */
