/**************************************************************************************************
  Filename:       gp_common.h
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

#ifndef GP_COMMON_H
#define GP_COMMON_H


#ifdef __cplusplus
extern "C"
{
#endif
  
  
  
/*********************************************************************
 * INCLUDES
 */
  
#include "ZComDef.h"
#include "ZMAC.h"
#include "AF.h"  
#include "zcl_green_power.h"
#include "cGP_stub.h"
#include "gp_interface.h"
  
/*********************************************************************
 * MACROS
 */

  //TODO: REMOVE THIS
#define GP_FIXED_TIME  
  
 /*********************************************************************
 * ENUM
 */
enum
{
  DGP_HANDLE_TYPE,
  GPEP_HANDLE_TYPE,
};

 /*********************************************************************
 * CONSTANTS
 */
 

// GP Shared key
#define GP_SHARED_KEY             { 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,\
                                    0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF }


#define GP_DATA_IND_QUEUE_MAX_ENTRY    5



          

#define GP_TRANSMIT_SCHEDULED_FRAME_EVENT     0x0001
#define GP_DUPLICATE_FILTERING_TIMEOUT_EVENT  0x0002
#define GP_COMMISSIONING_WINDOW_TIMEOUT       0x0004
#define GP_CHANNEL_CONFIGURATION_TIMEOUT      0x0008
#define GP_PROXY_ALIAS_CONFLICT_TIMEOUT       0x0010




#define GP_COMM_OPT_ACTION_MASK                     0x01
#define GP_COMM_OPT_EXIT_ON_WINDOW_EXPIRATION_MASK  0x02
#define GP_COMM_OPT_EXIT_ON_PAIRING_SUCCESS_MASK    0x04
#define GP_COMM_OPT_EXIT_ON_GP_COMM_MODE_EXIT_MASK  0x08
#define GP_COMM_OPT_CHANNEL_PRES_MASK               0x10
#define GP_COMM_OPT_UNICAST_COMM_MASK               0x20




#define GP_SECURITY_KEY_TYPE_NO_KEY                     0x00
#define GP_SECURITY_KEY_TYPE_ZIGBEE_NWK_KEY             0x01
#define GP_SECURITY_KEY_TYPE_GPD_GROUP_KEY              0x02
#define GP_SECURITY_KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP  0x03
#define GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY         0x04
#define GP_SECURITY_KEY_TYPE_DERIVED_IND_GPD_KEY        0x07


#define GP_RSP_CMD_OPT_APP_ID_MASK                       0x07
#define GP_RSP_CMD_OPT_TRANSMIT_ON_ENDPOINT_MATCH_MASK   0x08



#define GP_COMMISSIONING_COMMAND_ID                      0xE0  //Data frame
#define GP_DECOMMISSIONING_COMMAND_ID                    0xE1  //Data frame
#define GP_SUCCESS_COMMAND_ID                            0xE2  //Data frame
#define GP_CHANNEL_REQ_COMMAND_ID                        0xE3  //Maintenance frame



 /*********************************************************************
 * TYPEDEFS
 */
typedef uint8  (*GP_CheckAnnouncedDevice_t) ( uint8 *sinkIEEE, uint16 sinkNwkAddr );
   
/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte gp_TaskID;

extern uint8  zgGP_InCommissioningMode;     // Global flag that states if in commissioning mode or in operational mode.
extern uint16 gp_commissionerAddress;      //Address to which send the notifications during commissioning mode
extern ZDO_DeviceAnnce_t*                GP_aliasConflictAnnce; 
extern GP_CheckAnnouncedDevice_t         GP_CheckAnnouncedDeviceGCB;

/*********************************************************************
 * FUNCTION MACROS
 */
 
 
/*********************************************************************
 * FUNCTIONS
 */
   
/*
 * @brief       General function to init the NV items for proxy table  
 */
uint8 gp_ProxyTblInit( uint8 resetTable );
   
/*
 * @brief       Populate the given item data
 */
uint8 pt_ZclReadGetProxyEntry( uint16 nvId, uint8* pData, uint8* len );
 
/*
 * @brief   Handle Gp attributes.
 */
ZStatus_t zclGpp_ReadWriteAttrCB( uint16 clusterId, uint16 attrId, uint8 oper,
                                         uint8 *pValue, uint16 *pLen );
 
/*
 * @brief       Function to fill the options pramenter in a Proxy Table entry
 *              from a GP Pairing Command     
 */
static uint16 gp_pairingSetProxyTblOptions ( uint32 pairingOpt );

/*
 * @brief       General function fill the proxy table vector    
 */
void gp_PairingUpdateProxyTlb( gpPairingCmd_t* payload );
 
/*
 * @brief       To update the proxy table NV vectors
 */
uint8 gp_UpdateProxyTlb( uint8* pEntry, uint8 addSink );

/*
 * @brief       General function to check if it has the announced device 
 *              listed in the SinkAddressList and look for address conflict
 *              resolution.  
 */
uint8 gp_CheckAnnouncedDevice ( uint8 *sinkIEEE, uint16 sinkNwkAddr );

/*
 * @brief       General function fill the proxy table vector item  
 */
void gp_PopulateField( uint8* pField, uint8** pData, uint8 size );

/*
 * @brief   Initialize GP endpoint
 */
extern void gp_endpointInit(void);

/*
 * @brief   Stop Commissioning mode, either timeout or pairing success
 */
extern void gp_stopCommissioningMode(void);

/*
 * @brief   Return to operational channel after commissioning a GPD
 */
extern void gp_returnOperationalChannel(void);

/*
 * @brief This passes the MCPS data indications received in MAC to the application
 */
uint8 GP_DataInd(gp_DataInd_t *gp_DataInd);

/*
 * @brief Primitive to notify GP EndPoint the status of a previews DataReq
 */   
void GP_DataCnf(gp_DataCnf_t *gp_DataCnf);

/*
 * @brief   Parse the Gp Manteinance indication to Gp Notification command
 */
ZStatus_t zclGp_ManteinanceIndParse( gp_DataInd_t *pInd, gpCommissioningNotificationCmd_t *pGpNotification );
/*
 * @brief   Parse Gp Data Indication
 */
ZStatus_t zclGp_DataIndParse( gp_DataInd_t *pInd, gpNotificationCmd_t *pGpNotification );
/*
 * @brief   Find entry by handle in the list of dGP-DataInd pendings by GP-SecReq
 */
uint8* dGP_findHandle(uint8 handle);

/*
 * @brief       General function to get Proxy Table Entry Sec Frame Counter
 */
uint32 gp_getGppTblEntrySecFrame( gp_DataInd_t* pInd );

/*
 * @brief       Check framecounter freshness, security level and security key 
 *              type of an incomming data indication.
 */
uint8 gp_validateSecurity( gp_DataInd_t* pInd, uint8 *pKeyType, uint8 *pKey);

#ifdef __cplusplus
}
#endif


#endif /* GP_COMMON_H */
 