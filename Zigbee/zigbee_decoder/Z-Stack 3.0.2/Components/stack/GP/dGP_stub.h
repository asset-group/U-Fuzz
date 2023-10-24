/**************************************************************************************************
  Filename:       cGP_stub.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the common Green Power stub definitions.


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

#ifndef DGP_STUB_H
#define DGP_STUB_H


#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
 * INCLUDES
 */
 
 
 /*********************************************************************
 * CONSTANTS
 */
 
 /*********************************************************************
 * MACROS
 */
  
  
#define GP_NONCE_SEC_CONTROL_OUTGOING_APP_ID_GP   0xC5
#define GP_NONCE_SEC_CONTROL                      0x05
  
  
  
 
 /*********************************************************************
 * TYPEDEFS
 */
  

typedef void   (*GP_DataCnfGCB_t)(gp_DataCnf_t* gp_DataCnf);  
typedef void   (*GP_endpointInitGCB_t)(void);  
typedef void   (*GP_expireDuplicateFilteringGCB_t)(void);  
typedef void   (*GP_stopCommissioningModeGCB_t)(void); 
typedef void   (*GP_returnOperationalChannelGCB_t)(void);  
typedef uint8  (*GP_DataIndGCB_t)(gp_DataInd_t* gp_DataInd);
typedef uint8  (*GP_SecReqGCB_t)(gp_SecReq_t* gp_SecReq);

   
typedef struct
{
gpEventHdr_t hdr;
uint32       timestamp;         //Timestamp in backoff units
sAddr_t      srcAddr;
sAddr_t      dstAddr;
uint16       srcPanID;
uint16       dstPanID;
int8         Rssi;
uint8        LinkQuality;
uint8        SeqNumber;
uint8        mpduLen;
uint8        mpdu[1];            //This is a place holder for the buffer, its length depends on mpdu_len
}dgp_DataInd_t;
 

#define GP_NONCE_LENGTH   13
typedef struct
{
uint8  SourceAddr[Z_EXTADDR_LEN];
uint32 FrameCounter;
uint8  securityControl;
}gp_AESNonce_t;



/*********************************************************************
 * GLOBAL VARIABLES
 */
 
/* Callbacks for GP endpoint */
extern GP_DataCnfGCB_t                   GP_DataCnfGCB;
extern GP_endpointInitGCB_t              GP_endpointInitGCB;  
extern GP_expireDuplicateFilteringGCB_t  GP_expireDuplicateFilteringGCB;
extern GP_stopCommissioningModeGCB_t     GP_stopCommissioningModeGCB;
extern GP_returnOperationalChannelGCB_t  GP_returnOperationalChannelGCB;
extern GP_DataIndGCB_t                   GP_DataIndGCB;
extern GP_SecReqGCB_t                    GP_SecReqGCB; 
   
 
 /*********************************************************************
 * FUNCTION MACROS
 */
 
 
 /*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief       Handles GPDF to pass to application as GP SecReq to be validated.
 */
extern void dGP_DataInd(dgp_DataInd_t *dgp_DataInd);

/*
 * @brief       Notify the application about a GPDF being delivered
 */
extern void dGP_DataCnf(uint8 Status, uint8 GPmpduHandle);

/*   
 * @brief       Primitive by GP EndPoint to pass to dGP stub a request to send GPDF to a GPD
 */
extern bool GP_DataReq(gp_DataReq_t *gp_DataReq);

/*   
 * @brief       Primitive from dGP stub to GP EndPoint asking how to process a GPDF.   
 */
extern uint8 GP_SecReq(gp_SecReq_t *gp_SecReq);

/*
 * @brief       Response to dGP stub from GP endpoint on how to process the GPDF
 */
extern void  GP_SecRsp(gp_SecRsp_t *gp_SecRsp);
   

/*
 * @brief       Process the expiration of the packets in the duplicate filtering
 *              list. Assumption is the first in the queue is the first into expire.   
 */
extern void gp_expireDuplicateFiltering(void);

/*
 * @brief       Search for a DataInd entry with matching handle
 */
extern gp_DataInd_t* gp_DataIndGet(uint8 handle);

/*
 * @brief       Release the list of packets in the gpTxQueue
 */
extern void gp_FreeGpTxQueue(void);

/*
 * @brief       Append a DataInd to a list of DataInd (waiting for GP Sec Rsp List, 
 *              or list to filter duplicate packets)
 */
extern void gp_DataIndAppendToList(gp_DataInd_t *gp_DataInd, gp_DataInd_t **DataIndList);


/*
 * @brief  Returns a new handle for the type of msg.
 */
uint8 gp_GetHandle(uint8 handleType);

/*
 * @brief       Releases an element in the list, with the option to free memory 
 *              or not   
 */
void gp_DataIndReleaseFromList(bool freeMem, gp_DataInd_t* dataInd, gp_DataInd_t **DataIndList);

#ifdef __cplusplus
}
#endif


#endif /* DGP_STUB_H */
   
