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

#ifndef CGP_STUB_H
#define CGP_STUB_H


#ifdef __cplusplus
extern "C"
{
#endif
  
  
  
/*********************************************************************
 * INCLUDES
 */
  
#include "ZComDef.h"
#include "ZMAC.h"
#include "ssp.h"
  
  
  
/*********************************************************************
 * MACROS
 */
  
#define GP_ZIGBEE_PROTOCOL_VER    0x03
  
//GP Nwk FrameControl  
//Masks  
#define GP_NWK_FRAME_CTRL_EXT_MASK      0x80
#define GP_AUTO_COMM_MASK               0x40
#define GP_ZIGBEE_PROTOCOL_VERSION_MASK 0x3C
#define GP_FRAME_TYPE_MASK              0x03
//Possitions
#define GP_NWK_FRAME_CTRL_EXT_POS       7
#define GP_AUTO_COMM_POS                6
#define GP_ZIGBEE_PROTOCOL_VERSION_POS  2
#define GP_FRAME_TYPE_POS               0
   
#define GP_NWK_FRAME_TYPE_MASK          0x03
#define GP_NWK_FRAME_TYPE_POS           0
#define GP_NWK_FRAME_TYPE_MAINTENANCE   1   
#define GP_NWK_FRAME_TYPE_DATA          0


//GP ExtendedNwkFrameControl   
#define GP_EXT_NWK_APP_ID_MASK           0x07
#define GP_EXT_NWK_APP_ID_POS            0
//defined by application   
#define GP_EXT_NWK_APP_SEC_LVL_MASK      0x18
#define GP_EXT_NWK_APP_SEC_LVL_POS       3
#define GP_EXT_NWK_APP_SEC_KEY_TYPE_MASK 0x20
#define GP_EXT_NWK_APP_SEC_KEY_TYPE_POS  5
#define GP_EXT_NWK_APP_RX_AFTER_TX_MASK  0x40
#define GP_EXT_NWK_APP_RX_AFTER_TX_POS   6
#define GP_EXT_NWK_APP_DIRECTION_MASK    0x80
#define GP_EXT_NWK_APP_DIRECTION_POS     7

   
#define GP_SRC_ID_UNSPECIFIED           0
#define GP_SRC_ID_RESERVED_RANGE_LOW    0xFFFFFFF9
#define GP_SRC_ID_RESERVED_RANGE_HIGH   0xFFFFFFFE
   
#define GP_APP_ID_DEFAULT               0   
#define GP_APP_ID_LPED                  1
#define GP_APP_ID_GP                    2

#define GP_ENDPOINT_RESERVED_RANGE_LOW  0xF1  
#define GP_ENDPOINT_RESERVED_RANGE_HIGH 0xFE
#define GP_ENDPOINT_ALL                 0xFF

#define GP_SECURITY_LVL_NO_SEC             0
#define GP_SECURITY_LVL_RESERVED           1   
#define GP_SECURITY_LVL_4FC_4MIC           2
#define GP_SECURITY_LVL_4FC_4MIC_ENCRYPT   3
  
#define GP_SECURITY_MIC_SIZE               4
   
#define GP_CMD_ID_F0                       0xF0
#define GP_CMD_ID_FF                       0xFF


//TxOptions for GP-Data.Request
#define GP_OPT_USE_TX_QUEUE_MASK                0x01
#define GP_OPT_USE_CSMA_CA_MASK                 0x02
#define GP_OPT_USE_MAC_ACK_MASK                 0x04
#define GP_OPT_TX_FRAME_TYPE_MAINTENANCE_MASK   0x10
#define GP_OPT_TX_FRAME_TYPE_DATA_MASK          0x00
#define GP_OPT_TX_ON_MATCHING_ENDPOINT_MASK     0x20


//TxOptions for cGP-Data.Request
#define CGP_OPT_USE_CSMA_CA_MASK                0x01
#define CGP_OPT_USE_MAC_ACK_MASK                0x02



#define GP_OPT_GPDF_TYPE_FOR_TX_MASK            0x18
#define GP_OPT_GPDF_TYPE_FOR_TX_POS             3
#define GP_NWK_FRAME_TYPE_MAINTENANCE           1   
#define GP_NWK_FRAME_TYPE_DATA                  0

 /*********************************************************************
 * CONSTANTS
 */
 
  

 /*********************************************************************
 * TYPEDEFS
 */

enum
{
GP_MAC_MCPS_DATA_CNF, //Related to Common GP stub  
GP_MAC_MCPS_DATA_IND, //Related to Common GP stub
DGP_DATA_IND,      //Related to dedicated GP stub
DLPED_DATA_IND,    //Related to DLPE  not supported
GP_DATA_IND,       //Related to GP endpoint  
GP_DATA_REQ,       //Related to GP endpoint  
GP_DATA_CNF,       //Related to GP endpoint  
GP_SEC_REQ,        //Related to GP endpoint  
GP_SEC_RSP,        //Related to GP endpoint  
};


enum
{
GP_DATA_CNF_TX_QUEUE_FULL,
GP_DATA_CNF_ENTRY_REPLACED,
GP_DATA_CNF_ENTRY_ADDED,
GP_DATA_CNF_ENTRY_EXPIRED,
GP_DATA_CNF_ENTRY_REMOVED,
GP_DATA_CNF_GPDF_SENDING_FINALIZED,
};

enum
{
GP_SEC_RSP_DROP_FRAME,
GP_SEC_RSP_MATCH,
GP_SEC_RSP_PASS_UNPROCESSED,
GP_SEC_RSP_TX_THEN_DROP,
GP_SEC_RSP_ERROR,
};

enum
{
GP_DATA_IND_STATUS_SECURITY_SUCCESS,
GP_DATA_IND_STATUS_NO_SECURITY,
GP_DATA_IND_STATUS_COUNTER_FAILURE,
GP_DATA_IND_STATUS_AUTH_FAILURE,
GP_DATA_IND_STATUS_UNPROCESSED
};


/* GP event header type */
typedef struct
{
  uint8   event;              /* MAC event */
  uint8   status;             /* MAC status */
} gpEventHdr_t;



typedef struct
{
uint8                  dGPStubHandle;
struct gp_DataInd_tag  *next;
uint32                 timeout;
}gp_DataIndSecReq_t;

typedef struct gp_DataInd_tag
{
gpEventHdr_t        hdr;
gp_DataIndSecReq_t  SecReqHandling;
uint32              timestamp;
uint8               status;
int8                Rssi;
uint8               LinkQuality;
uint8               SeqNumber;
sAddr_t             srcAddr;
uint16              srcPanID;
uint8               appID;
uint8               GPDFSecLvl;
uint8               GPDFKeyType;
bool                AutoCommissioning;
bool                RxAfterTx;
uint32              SrcId;
uint8               EndPoint;
uint32              GPDSecFrameCounter;
uint8               GPDCmmdID;
uint32              MIC;
uint8               GPDasduLength;
uint8               GPDasdu[1];         //This is a place holder for the buffer, the length depends on GPDasduLength
}gp_DataInd_t;




typedef struct
{
uint8        AppID;
union
  {
    uint32       SrcID;
    uint8        GPDExtAddr[Z_EXTADDR_LEN];
  }GPDId;
}gpd_ID_t;


typedef struct
{
gpEventHdr_t hdr;
bool         Action;
uint8        TxOptions;
gpd_ID_t     gpd_ID;
uint8        EndPoint;
uint8        GPDCmmdId;
uint8        GPEPhandle;
uint8        gpTxQueueEntryLifeTime[3];
uint8        GPDasduLength;
uint8        GPDasdu[1];         //This is a place holder for the buffer, the length depends on GPDasduLength
}gp_DataReq_t;

typedef struct
{
gpEventHdr_t   hdr;
uint32         timestamp;
ZMacDataReq_t  ZMacDataReq;
}gp_ZMacDataReq_t;

typedef struct
{
  gp_DataReq_t  *gp_DataReq;
}
gp_DataReqPending_t;


typedef struct
{
gpEventHdr_t hdr;
uint8        status;
uint8        GPEPhandle;
}gp_DataCnf_t;

typedef struct
{
gpEventHdr_t hdr;
uint8        status;
uint8        MPDUhandle;
}cgp_DataCnf_t;


typedef struct
{
uint8        GPDFSecLvl;
uint8        GPDFKeyType;
uint32       GPDSecFrameCounter;
}gp_SecData_t;




typedef struct
{
gpEventHdr_t hdr;
gpd_ID_t     gpd_ID;
uint8        EndPoint;
gp_SecData_t gp_SecData;
uint8        dGPStubHandle;
}gp_SecReq_t;



typedef struct
{
gpEventHdr_t  hdr;
uint8         Status;
uint8         dGPStubHandle;
gpd_ID_t      gpd_ID;
uint8         EndPoint;
gp_SecData_t  gp_SecData;
uint8         GPDKey[SEC_KEY_LEN];
}gp_SecRsp_t;




/*********************************************************************
 * GLOBAL VARIABLES
 */
 
extern byte gp_TaskID;
 
/*********************************************************************
 * FUNCTION MACROS
 */
 
 

/*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief This function reports the results CGP Data request being 
 *        handled by the cGP stub
 */

extern void CGP_DataCnf(uint8 Status, uint8 GPmpduHandle);

/*
 * @brief Allows dGP or dLPED stub send GPDF to GPD or LPED
 */

extern ZStatus_t CGP_DataReq(uint8 TxOptions, zAddrType_t *SrcAddr, uint16 SrcPanID,  zAddrType_t *dstAddr, uint16 DstPanID, uint8 mpdu_len, uint8*  mpdu, uint8 mpduHandle,uint32 timestamp);


/*
 * @brief       Funtion to parse GPDF to pass to dedicated Stub (dGP)
 */  
extern void cGP_processMCPSDataInd(macMcpsDataInd_t *pData);

/*
 * @brief Sends GPDF in the calculated window to the GPD. 
 */
extern void gp_SendScheduledMacDataReq(void);

#ifdef __cplusplus
}
#endif


#endif /* CGP_STUB_H */
 
 
 
 
 
 
 
 
 
 
