/**************************************************************************************************
  Filename:       gp_common.c
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the implementation of the cGP stub.


  Copyright 2006-2015 Texas Instruments Incorporated. All rights reserved.

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

#include "ZGlobals.h"
#include "ZComDef.h"
#include "zcl_general.h"
#include "AF.h"
#include "gp_common.h"
#include "gp_interface.h"
#include "zcl_green_power.h"
#include "OSAL.h"
#include "dGP_stub.h"
#include "mac_api.h"
#include "ZDSecMgr.h"
#include "bdb.h"

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

 /*********************************************************************
 * MACROS
 */ 

   
 /*********************************************************************
 * CONSTANTS
 */

#define PGG_COMMISSIONING_WINDOW   180  //180 seconds by defaut


#ifdef GP_SHARED_KEY
  CONFIG_ITEM uint8 zgpSharedKey[SEC_KEY_LEN] = GP_SHARED_KEY;
#else
  CONFIG_ITEM uint8 zgpSharedKey[SEC_KEY_LEN] = {0xFF};
#endif

/*********************************************************************
 * TYPEDEFS
 */
   
 /*********************************************************************
 * GLOBAL VARIABLES
 */


uint8  zclGp_gppMaxProxyTableEntries = GPP_MAX_PROXY_TABLE_ENTRIES;
uint8 *pZclGp_ProxyTableEntries = NULL;
uint8  zclGp_gppNotificationRetryNumber = GPP_NOTIFICATION_RETRY_NUMBER;
uint8  zclGp_gppNotificationRetryTimer = GPP_NOTIFICATION_RETRY_TIMER;
uint8  zclGp_gppMaxSearchCounter = GPP_MAX_SEARCH_COUNTER;
uint8 *pZclGp_gppBlockedGPDID = NULL;

/**********************************************************************************
ZigBee PRO Green Power feature specification
Basic functionality set
Version 1.0

Table 42 – Format of the gppFunctionality attribute
----------------------------------------------------------------------------------
Indication   | Functionality                                        | Basic Proxy
-------------|------------------------------------------------------|-------------
b0           | GP feature                                           | 0b1
b1           | Direct communication (reception of GPDF via GP stub) | 0b1
b2           | Derived groupcast communication                      | 0b1
b3           | Pre-commissioned groupcast communication             | 0b1
b4           | Full unicast communication                           | 0b0
b5           | Lightweight unicast communication                    | 0b1
b6           | Reserved                                             | 0b0
b7           | Bidirectional operation                              | 0b0
b8           | Proxy Table maintenance (active and passive,         | 0b0
             | for GPD mobility and GPP robustness)                 |
b9           | Reserved                                             | 0b0
b10          | GP commissioning                                     | 0b1
b11          | CT-based commissioning                               | 0b1
b12          | Maintenance of GPD (deliver channel/key              | 0b0
             | during operation)                                    | 
b13          | gpdSecurityLevel  = 0b00                             | 0b1
b14          | Deprecated: gpdSe curityLevel = 0b01                 | 0b0
b15          | gpdSecurityLevel  = 0b10                             | 0b1
b16          | gpdSecurityLevel  = 0b11                             | 0b1
b17          | Reserved                                             | 0b0
b18          | Reserved                                             | 0b0
b19          | GPD IEEE address                                     | 0b1
b20 – b23    | Reserved                                             | 0b0
----------------------------------------------------------------------------------
***********************************************************************************/

//                                          b0-b7       b8-b15      b16-23
//                                        0b00101111  0b10101100  0b00001001
uint8  zclGp_gppFunctionality[3] =       {   0x2F,      0xAC,        0x09};
uint8  zclGp_gppActiveFunctionality[3] = {   0x2F,      0xAC,        0x09};

uint8  zclGp_gpSharedSecurityKeyType = GP_SHARED_SEC_KEY_TYPE;
uint8  zclGp_gpSharedSecurityKey[SEC_KEY_LEN] = GP_SHARED_KEY;
uint8  zclGp_gpLinkKey[SEC_KEY_LEN] = GP_LINK_LEY;
uint16 zclGp_clusterRevision = GP_CLUSTER_REVISION;

static gpCommissioningMode_t  pfnCommissioningMode = NULL;
static gpChangeChannelReq_t   pfnChangeChannelReq = NULL;
static gpChangeChannelReq_t   pfnChangeChannelReqForBDB = NULL;


uint8 gp_commissioningOptions = 0;           //Commissioning Options from the ommissioningMode command
uint16 gp_commissioningUnicastAddress = 0;   //address of the device to which send the notifications

uint8  zgGP_InCommissioningMode = FALSE;     // Global flag that states if in commissioning mode or in operational mode.
uint16 gp_commissionerAddress = 0xFFFF;      //Address to which send the notifications during commissioning mode
byte   gp_tempLogicalChannel = 0;            //Holder of the operational nwk channel



  

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t zclGp_Attrs[] =
{
  // *** Green Power Proxy Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_MAX_PROXY_TABLE_ENTRIES,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclGp_gppMaxProxyTableEntries
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_PROXY_TABLE,
      ZCL_DATATYPE_LONG_OCTET_STR,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_NOTIFICATION_RETRY_NUMBER,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
      (void *)&zclGp_gppNotificationRetryNumber
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_NOTIFICATION_RETRY_TIMER,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
      (void *)&zclGp_gppNotificationRetryTimer
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_MAX_SEARCH_COUNTER,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
      (void *)&zclGp_gppMaxSearchCounter
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_BLOCKED_GPD_ID,
      ZCL_DATATYPE_LONG_OCTET_STR,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&pZclGp_gppBlockedGPDID
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_FUNCTIONALITY,
      ZCL_DATATYPE_BITMAP24,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclGp_gppFunctionality
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_GPP_ACTIVE_FUNCTIONALITY,
      ZCL_DATATYPE_BITMAP24,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclGp_gppActiveFunctionality
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_SHARED_SEC_KEY_TYPE,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
      (void *)&zclGp_gpSharedSecurityKeyType
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_SHARED_SEC_KEY,
      ZCL_DATATYPE_128_BIT_SEC_KEY,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
      (void *)&zclGp_gpSharedSecurityKey
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_GP_LINK_KEY,
      ZCL_DATATYPE_128_BIT_SEC_KEY,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CLIENT,
      (void *)&zclGp_gpLinkKey
    }
  },
  {
    ZCL_CLUSTER_ID_GREEN_POWER,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclGp_clusterRevision
    }
  }
};


uint8 CONST zclGp_NumAttributes = ( sizeof(zclGp_Attrs) / sizeof(zclGp_Attrs[0]) );

 /*********************************************************************
 * EXTERNAL VARIABLES
 */

//List to filter duplicated packets
gp_DataInd_t        *gp_DataIndList;


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define GREEN_POWER_EP_MAX_INCLUSTERS       1

#define GREEN_POWER_EP_MAX_OUTCLUSTERS       1
static const cId_t greenPower_EP_OutClusterList[GREEN_POWER_EP_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GREEN_POWER
};

static SimpleDescriptionFormat_t greenPower_EP_SimpleDesc =
{
  GREEN_POWER_INTERNAL_ENDPOINT,         //  int Endpoint;
  ZCL_GP_PROFILE_ID,                     //  uint16 AppProfId;
  ZCL_GP_DEVICEID_PROXY_BASIC,           //  uint16 AppDeviceId;
  0,                                     //  int   AppDevVer:4;
  0,                                     //  int   AppFlags:4;
  0,                                     //  byte  AppNumInClusters;
  NULL,                                  //  byte *pAppInClusterList;
  GREEN_POWER_EP_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)greenPower_EP_OutClusterList  //  byte *pAppInClusterList;
};




 /*********************************************************************
 * LOCAL FUNCTIONS
 */
static void GP_ProccessCommissioningNotification(gp_DataInd_t *gp_DataInd);
static void GP_ProccessNotification(gp_DataInd_t *gp_DataInd);

static void zclGp_GpPairingCommandCB( zclGpPairing_t *pCmd );
static void zclGp_GpProxyTableReqCB( zclGpProxyTableRequest_t *pCmd );
static void zclGp_GpProxyCommissioningModeCB(zclGpProxyCommissioningMode_t* pCmd);
static void zclGp_GpResponseCommandCB(zclGpResponse_t* pCmd);
static void gp_u32CastPointer( uint8 *data, uint8 *p );
static void gp_u16CastPointer( uint8 *data, uint8 *p );
static void gp_u8CastPointer( uint8 *data, uint8 *p );
static void gp_ZclPairingParse( zclGpPairing_t* pCmd, gpPairingCmd_t* payload );
static void gp_ZclProxyTableReqParse( zclGpProxyTableRequest_t* pCmd, gpProxyTableReqCmd_t* payload );
static uint8 gp_SecurityOperationProxy( gp_DataInd_t* pInd, uint8* pKeyType, uint8* pKey);
static gp_DataInd_t* gp_DataIndFindDuplicate(uint8 handle, uint8 secLvl);
static uint8 GP_RecoveryKey(uint8 GPDFKeyType,uint8 KeyType, uint8 status, uint8 *Key);
 



/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGp_AppCallbacks_t zclGpProxy_CmdCallbacks =
{
  zclGp_GpPairingCommandCB,            //
  zclGp_GpProxyTableReqCB,
  zclGp_GpProxyCommissioningModeCB,    //
  zclGp_GpResponseCommandCB  
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/


 
void gp_endpointInit(void)
{

  bdb_RegisterSimpleDescriptor( &greenPower_EP_SimpleDesc );
  
  zclGp_RegisterCmdCallbacks( GREEN_POWER_INTERNAL_ENDPOINT, &zclGpProxy_CmdCallbacks );
  
  // Register the application's attribute list
  zcl_registerAttrList( GREEN_POWER_INTERNAL_ENDPOINT, zclGp_NumAttributes, zclGp_Attrs );
  
  zcl_registerReadWriteCB( GREEN_POWER_INTERNAL_ENDPOINT, zclGpp_ReadWriteAttrCB, NULL );
  
  gp_ProxyTblInit( FALSE );

}

/*********************************************************************
 * @fn      gp_stopCommissioningMode
 *
 * @brief   Stops the commissioning mode
 *
 * @param   none
 *
 * @return  none
 */
void gp_stopCommissioningMode(void)
{
  //Callback to notify about the end of the commissioning mode
  gp_commissioningOptions = 0;
  zgGP_InCommissioningMode = FALSE;
  gp_commissionerAddress = 0xFFFF;
  osal_stop_timerEx(gp_TaskID,GP_COMMISSIONING_WINDOW_TIMEOUT);
  
  //Release the gpTxQueue, proxy does not support bidirectional communication on operational mode
  //A.3.2.8
  gp_FreeGpTxQueue();
  
  //Notify the user that we have exited the commissioning mode
  if(pfnCommissioningMode)
  {
    pfnCommissioningMode(FALSE);
  }
}

/*********************************************************************
 * @fn      zclGp_ManteinanceIndParse
 *
 * @brief   Parse the Gp Manteinance indication to Gp Notification command
 *
 * @param   pInd - Pointer to the incoming data
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_ManteinanceIndParse( gp_DataInd_t *pInd, gpCommissioningNotificationCmd_t *pGpNotification )
{
  uint8 currEntry[PROXY_TBL_ENTRY_LEN];
  uint8  ntfOpt[2] = {0x00, 0x00};
  int8 RSSI;
  uint8 LQI;
  
  gp_ResetProxyBasicTblEntry(currEntry);
  
  
  if ( pInd->GPDCmmdID != GP_CHANNEL_REQ_COMMAND_ID )
  {
    if ( PROXY_TBL_COMP_APPLICTION_ID( pInd->appID, GP_OPT_APP_ID_GPD ) )
    {
      pGpNotification->gpdId = pInd->SrcId;
      ntfOpt[0] = GP_OPT_APP_ID_GPD;
    }
    else if ( PROXY_TBL_COMP_APPLICTION_ID( pInd->appID, GP_OPT_APP_ID_IEEE ) )
    {
      osal_memcpy( pGpNotification->gpdIEEE, &(pInd->srcAddr.addr.extAddr), Z_EXTADDR_LEN );
      pGpNotification->ep = pInd->EndPoint;
      ntfOpt[0] = GP_OPT_APP_ID_IEEE;
    }
  }
  
  // Set the options bit field
  ( pInd->RxAfterTx == TRUE ) ?PROXY_TBL_SET_RX_AFTER_TX( ( uint8* )&ntfOpt[0] ) : PROXY_TBL_CLR_RX_AFTER_TX( ( uint8* )&ntfOpt[0] );
  GP_CNTF_SET_SEC_LEVEL( ( uint8* )&ntfOpt[0], currEntry[PROXY_TBL_ENTRY_SEC_OPT] );
  GP_CNTF_SET_SEC_KEY_TYPE( ( uint16* )ntfOpt, currEntry[PROXY_TBL_ENTRY_SEC_OPT] );
  if( pInd->status == GP_DATA_IND_STATUS_AUTH_FAILURE )
  {
    GP_CNTF_SET_SEC_FAIL( ( uint8* )&ntfOpt[1] );
    pGpNotification->mic = pInd->MIC;
  }
  else
  {
    GP_CNTF_CLR_SEC_FAIL( ( uint8* )&ntfOpt[1] );
    pGpNotification->mic = 0xFFFFFFFF;
  }
  GP_CNTF_SET_BIDIRECTIONAL_CAP( ( uint8* )&ntfOpt[1] );

  GP_CNTF_SET_PROXY_INFO( ( uint8* )&ntfOpt[1] );
  
  pGpNotification->options |= ( ( ntfOpt[0] ) & 0x00FF );
  pGpNotification->options |= ( ( ( ntfOpt[1] ) <<  8 ) & 0xFF00 );
  
  pGpNotification->gpdSecCounter = pInd->GPDSecFrameCounter;
  pGpNotification->cmdId = pInd->GPDCmmdID;

  if ( pInd->GPDasduLength > 0 )
  {
    pGpNotification->payloadLen = pInd->GPDasduLength;
    pGpNotification->cmdPayload = pInd->GPDasdu;
  }
  
  pGpNotification->gppShortAddr = _NIB.nwkDevAddress;
  
  RSSI = pInd->Rssi;
  ( RSSI > 8 ) ?RSSI = 8 : ( RSSI < -109 ) ?RSSI = -109 : NULL;
  RSSI += 110;
  RSSI /= 2;
  
  ( pInd->LinkQuality == 0 ) ?LQI = 0 : ( pInd->LinkQuality > 0 ) ?LQI = 2 : NULL;
  
  pGpNotification->gppGpdLink = RSSI;
  pGpNotification->gppGpdLink |= ( LQI << 6 );
    
  return SUCCESS;
}

/*********************************************************************
 * @fn      zclGp_DataIndParse
 *

 * @brief   Parse the Gp Data indication to Gp Notification command
 *
 * @param   pInd - Pointer to the incoming data
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGp_DataIndParse( gp_DataInd_t *pInd, gpNotificationCmd_t *pGpNotification )
{
  uint8 currEntry[PROXY_TBL_ENTRY_LEN];
  uint8  ntfOpt[2] = {0x00, 0x00};
  uint8 i;
  int8 RSSI;
  uint8 LQI;
  uint8 status;
  
  gp_ResetProxyBasicTblEntry(currEntry);
  
  for ( i = 1; i <= GPP_MAX_PROXY_TABLE_ENTRIES ; i++ )
  {
    status = gp_getProxyTableByIndex( ( ZCD_NV_PROXY_TABLE_START + i ), currEntry );

    if ( status == NV_OPER_FAILED )
    {
      return status;
    }
    
    // if the entry is empty
    if ( status == NV_INVALID_DATA )
    {
      continue;
    }
  
    if ( PROXY_TBL_COMP_APPLICTION_ID( pInd->appID, currEntry[PROXY_TBL_ENTRY_OPT] ) )
    {
      if ( pInd->appID == GP_OPT_APP_ID_GPD )
      {
        if ( osal_memcmp( &pInd->SrcId, &currEntry[PROXY_TBL_ENTRY_GPD_ID + 4], sizeof ( uint32 ) ) )
        {
          // Entry found
          pGpNotification->gpdId = pInd->SrcId;
          ntfOpt[0] = GP_OPT_APP_ID_GPD;
          break;
        }
      }
      else if ( pInd->appID == GP_OPT_APP_ID_IEEE )
      {
        if ( osal_memcmp( &pInd->srcAddr, &currEntry[PROXY_TBL_ENTRY_GPD_ID], Z_EXTADDR_LEN ) )
        {
          // Entry found
          osal_memcpy( pGpNotification->gpdIEEE, &(pInd->srcAddr.addr.extAddr), Z_EXTADDR_LEN );
          pGpNotification->ep = pInd->EndPoint;
          ntfOpt[0] = GP_OPT_APP_ID_IEEE;
          break;
        }
      }
    }
    else
    {
      continue;
    }
  }
  
  // Set the options bit field
  GP_NTF_SET_ALSO_UNICAST( ( uint8* )&ntfOpt[0], currEntry[PROXY_TBL_ENTRY_OPT] );
  GP_NTF_SET_ALSO_DGROUP( ( uint8* )&ntfOpt[0], currEntry[PROXY_TBL_ENTRY_OPT] );
  GP_NTF_SET_ALSO_CGROUP( ( uint8* )&ntfOpt[0], currEntry[PROXY_TBL_ENTRY_OPT] );
  GP_NTF_SET_SEC_LEVEL( ( uint8* )&ntfOpt[0], currEntry[PROXY_TBL_ENTRY_SEC_OPT] );
  GP_NTF_SET_SEC_KEY_TYPE( ( uint8* )&ntfOpt[1], currEntry[PROXY_TBL_ENTRY_SEC_OPT] );
  pInd->RxAfterTx ?PROXY_TBL_SET_RX_AFTER_TX( ( uint8* )&ntfOpt[1] ) : PROXY_TBL_CLR_RX_AFTER_TX( ( uint8* )&ntfOpt[1] );
  
  // Mandatory to set this sub fields to 0b1 by the Green Power Proxy Basic Spec in A.3.3.4.1
  PROXY_TBL_CLR_TX_QUEUE_FULL( ( uint8* )&ntfOpt[1] );
  PROXY_TBL_SET_BIDIRECTIONAL_CAP( ( uint8* )&ntfOpt[1] );
  PROXY_TBL_SET_PROXY_INFO( ( uint8* )&ntfOpt[1] );
  
  pGpNotification->options |= ( ( ntfOpt[0] ) & 0x00FF );
  pGpNotification->options |= ( ( ( ntfOpt[1] ) <<  8 ) & 0xFF00 );
  
  pGpNotification->gpdSecCounter = pInd->GPDSecFrameCounter;
  pGpNotification->cmdId = pInd->GPDCmmdID;

  if ( pInd->GPDasduLength > 0 )
  {
    pGpNotification->payloadLen = pInd->GPDasduLength;
    pGpNotification->cmdPayload = pInd->GPDasdu;
  }
  
  pGpNotification->gppShortAddr = _NIB.nwkDevAddress;
  
  RSSI = pInd->Rssi;
  ( RSSI > 8 ) ?RSSI = 8 : ( RSSI < -109 ) ?RSSI = -109 : NULL;
  RSSI += 110;
  RSSI /= 2;
  
  ( pInd->LinkQuality == 0 ) ?LQI = 0 : ( pInd->LinkQuality > 0 ) ?LQI = 2 : NULL;
  
  pGpNotification->gppGpdLink = RSSI;
  pGpNotification->gppGpdLink |= ( LQI << 6 );
    
  return SUCCESS;
}

/*********************************************************************
 * @fn      zclGp_GpPairingCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received an Gp Notification Command for this application.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
static void zclGp_GpPairingCommandCB( zclGpPairing_t *pCmd )
{
  gpPairingCmd_t pairingPayload = {0x00};

  gp_ZclPairingParse( pCmd, &pairingPayload );
  gp_PairingUpdateProxyTlb( &pairingPayload );
  
  
  if(gp_commissioningOptions & GP_COMM_OPT_EXIT_ON_PAIRING_SUCCESS_MASK)
  {
    gp_stopCommissioningMode();
  }
}

/*********************************************************************
 * @fn      zclGp_GpProxyTableReqCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp proxy table req.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
static void zclGp_GpProxyTableReqCB( zclGpProxyTableRequest_t *pCmd )
{
  uint8 i;
  uint8 entryLen = 0;
  uint8 maxEntryLen = 0;
  uint8* buf = NULL;
  uint8 currEntry[PROXY_TBL_ENTRY_LEN];
  
  
  gp_ResetProxyBasicTblEntry(currEntry);
  zclGpProxyTableResponse_t proxyTblRsp = {0};
  gpProxyTableReqCmd_t proxyTblReqPayload = {0};
  
  proxyTblRsp.proxyTableEntry = NULL;
  

  gp_ZclProxyTableReqParse( pCmd, &proxyTblReqPayload );
  
  // To get total entries for "Total number of non-empty Proxy Table entries" field
  for ( i = 0; i <= GPP_MAX_PROXY_TABLE_ENTRIES; i++ )
  {
    if ( gp_getProxyTableByIndex( ( ZCD_NV_PROXY_TABLE_START + i ), currEntry ) == SUCCESS )
    {
      proxyTblRsp.tableEntriesTotal += 1;
    }
  }
  
  // for Request Table Entries by Index
  if ( GP_IS_REQ_TPY_INDEX ( proxyTblReqPayload.options ) )
  {
    if ( (proxyTblRsp.tableEntriesTotal == 0x00) || (proxyTblRsp.tableEntriesTotal < proxyTblReqPayload.index) )
    {
      proxyTblRsp.status = SUCCESS;
      proxyTblRsp.startIndex = proxyTblReqPayload.index;
      proxyTblRsp.entriesCount = 0x00;
      // Send response and exit
      zclGp_SendGpProxyTableResponse( pCmd->srcAddr, &proxyTblRsp, zcl_InSeqNum );
      return;
    }
    for ( i = proxyTblReqPayload.index; i <= GPP_MAX_PROXY_TABLE_ENTRIES; i++ )
    {    
      maxEntryLen = entryLen;    // Save the last iteration Lenght
      
      // Get the length of the payload
      if ( pt_ZclReadGetProxyEntry ( ( ZCD_NV_PROXY_TABLE_START + i ), NULL, &entryLen ) != SUCCESS )
      {
        break;
      }
      
      if ( entryLen >= 75 )   // max payload for entries
      {
        entryLen = maxEntryLen;   // The last valid lenght is the MAX for this packet
        break;
      }
    }

    if ( entryLen )
    {
      buf = zcl_mem_alloc( entryLen );
      if ( buf != NULL )
      {
        // Load the buffer - serially
        uint8 *pBuf = buf;
      
        entryLen = 0;
        proxyTblRsp.entriesCount = 0;
        for ( i = proxyTblReqPayload.index; i <= GPP_MAX_PROXY_TABLE_ENTRIES ; i++ )
        {  
          if( pt_ZclReadGetProxyEntry ( ( ZCD_NV_PROXY_TABLE_START + i ), pBuf, &entryLen ) != SUCCESS )
          {
            break;
          }
          
          proxyTblRsp.entriesCount += 1;
          if ( entryLen >= maxEntryLen )   // to see if we reched the MAX calculated payload
          {
            break;
          }
        }
      }
    }
  }
  
  // for Request Table Entries by GPD ID
  if ( GP_IS_REQ_TYP_GPD ( proxyTblReqPayload.options ) )
  {
    uint8 lookForEntry = 0;
    gpd_ID_t gpd_ID;
    uint16 NvProxyTableIndex;
    uint8  ProxyTableEntryTemp[PROXY_TBL_ENTRY_LEN];
    
    if ( GP_IS_APPLICATION_ID_GPD( proxyTblReqPayload.options ) )
    {
      gpd_ID.AppID = GP_OPT_APP_ID_GPD;
      gpd_ID.GPDId.SrcID = proxyTblReqPayload.gpdId;
      lookForEntry = 1;
      proxyTblRsp.startIndex = 0xFF;
    }
    else if ( GP_IS_APPLICATION_ID_IEEE( proxyTblReqPayload.options ) )
    {
      gpd_ID.AppID = GP_OPT_APP_ID_IEEE;
      osal_memcpy( gpd_ID.GPDId.GPDExtAddr, proxyTblReqPayload.gpdIEEE, Z_EXTADDR_LEN );
      lookForEntry = 1;
      proxyTblRsp.startIndex = 0xFF;
    }
    
    if ( lookForEntry )
    {
      if ( gp_getProxyTableByGpId(&gpd_ID, ProxyTableEntryTemp, &NvProxyTableIndex) == ZSuccess )
      {    
        maxEntryLen = entryLen;    // Save the last iteration Lenght
        
        // Get the length of the payload
        if ( pt_ZclReadGetProxyEntry ( NvProxyTableIndex, NULL, &entryLen ) != ZSuccess )
        {
          entryLen = 0;
        }
      
        if ( entryLen >= 75 )   // max payload for entries
        {
          entryLen = maxEntryLen;   // The last valid lenght is the MAX for this packet
        }
      }
      else
      {
        proxyTblRsp.status = ZCL_STATUS_NOT_FOUND;
      }
      
      if ( entryLen )
      {
        buf = zcl_mem_alloc( entryLen );
        if ( buf != NULL )
        {
          // Load the buffer - serially
          uint8 *pBuf = buf;
        
          entryLen = 0;
          proxyTblRsp.entriesCount = 0;
        
          if ( pt_ZclReadGetProxyEntry ( NvProxyTableIndex, pBuf, &entryLen ) == SUCCESS )
          {
            proxyTblRsp.entriesCount = 1;
          }
        }
      }
    }
  }
  
  // Send response and exit
  proxyTblRsp.startIndex = proxyTblReqPayload.index;
  proxyTblRsp.proxyTableEntry = buf;
  zclGp_SendGpProxyTableResponse( pCmd->srcAddr, &proxyTblRsp, zcl_InSeqNum );
  zcl_mem_free( buf );
}


/*********************************************************************
 * @fn      zclGp_GpResponseCommandCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp Response command. (Ref A.3.5.2.1)
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
static void zclGp_GpResponseCommandCB(zclGpResponse_t *pCmd)
{
  gp_DataReq_t  *gp_DataReq;
  gpd_ID_t      gpd_ID;
  uint8         endpoint = 0;
  uint8         command;
  uint8         payloadLen = 0;
  
  //No bidirectional communication in operational mode
  //A.3.2.8
  if(!zgGP_InCommissioningMode)
  {
    return;
  }
  
  gpd_ID.AppID = pCmd->options & GP_RSP_CMD_OPT_APP_ID_MASK;
  if(gpd_ID.AppID == GP_APP_ID_DEFAULT)
  {
    gpd_ID.GPDId.SrcID = osal_build_uint32(pCmd->pData,sizeof(uint32));
    pCmd->pData += sizeof(uint32);
  }
  else if(gpd_ID.AppID == GP_APP_ID_DEFAULT)
  {
    gpd_ID.GPDId.SrcID = osal_build_uint32(pCmd->pData,sizeof(uint32));
    pCmd->pData += sizeof(uint32);
    
    endpoint = *pCmd->pData;
    pCmd->pData++;
  }
  else
  {
    //Invalid app ID
    return;
  }
  
  command  = *pCmd->pData;
  pCmd->pData++;
  payloadLen = *pCmd->pData;
  pCmd->pData++;
   
  gp_DataReq = (gp_DataReq_t*)osal_msg_allocate(sizeof(gp_DataReq_t) + payloadLen);  
  
  if(gp_DataReq == NULL)
  {
    //FAIL no memory
    return;
  }
  if(payloadLen == 0xFF)
  {
    payloadLen = 0;
  }
  gp_DataReq->TxOptions = GP_OPT_USE_TX_QUEUE_MASK;
  gp_DataReq->EndPoint = endpoint;
  osal_memcpy(gp_DataReq->GPDasdu,pCmd->pData,payloadLen);
  gp_DataReq->GPDasduLength = payloadLen;
  gp_DataReq->GPDCmmdId = command;
  osal_memcpy(&gp_DataReq->gpd_ID,&gpd_ID,sizeof(gpd_ID_t));

  gp_DataReq->GPEPhandle = gp_GetHandle(GPEP_HANDLE_TYPE);
      
  gp_DataReq->hdr.event = GP_DATA_REQ;
  gp_DataReq->hdr.status = 0;  
  
  if(pCmd->options & GP_RSP_CMD_OPT_TRANSMIT_ON_ENDPOINT_MATCH_MASK)
  {
    gp_DataReq->TxOptions |= GP_OPT_TX_ON_MATCHING_ENDPOINT_MASK;
  }
  
  //Validate the command being send as unicast and we are the tempMaster
  if((pCmd->tempMasterShortAddr == _NIB.nwkDevAddress) && (pCmd->dstAddr == _NIB.nwkDevAddress))
  {
    uint16 NvProxyTableIndex;
    uint8  ProxyTableEntryTemp[PROXY_TBL_ENTRY_LEN];
    
    //Check if the entry exist, if so, set the first to forward flag to 1
    if ( gp_getProxyTableByGpId(&gpd_ID,ProxyTableEntryTemp,&NvProxyTableIndex) == ZSuccess )
    {
      //Update FirstToForward to 1
      if(PROXY_TBL_GET_FIRST_TO_FORWARD(ProxyTableEntryTemp[PROXY_TBL_ENTRY_OPT]) == 0)
      {
        PROXY_TBL_SET_FIRST_TO_FORWARD(&ProxyTableEntryTemp[PROXY_TBL_ENTRY_OPT]);
        osal_nv_write(NvProxyTableIndex,PROXY_TBL_ENTRY_OPT,2,&ProxyTableEntryTemp[PROXY_TBL_ENTRY_OPT]);
      }
    }
    //Depends on TempMasterAddress
    gp_DataReq->Action = TRUE;
    
    //Step 6 of Section A.3.9.1 The Procedure
    //Check if we are in a different channel
    if((pCmd->tempMasterTxChannel + 0x0B) != _NIB.nwkLogicalChannel)
    {
      //did we got permission to attend channel request?
      if(osal_get_timeoutEx(gp_TaskID,GP_CHANNEL_CONFIGURATION_TIMEOUT))
      {
        gp_tempLogicalChannel = _NIB.nwkLogicalChannel;
        _NIB.nwkLogicalChannel = pCmd->tempMasterTxChannel;
        ZMacSetReq( ZMacChannel, &(_NIB.nwkLogicalChannel) );
      }
    }
  }
  else
  {
    uint16 NvProxyTableIndex;
    uint8  ProxyTableEntryTemp[PROXY_TBL_ENTRY_LEN];
       
    //We are not the tempMaster or this was not a unicast to us, set the first to forward flag to 0
    if( gp_getProxyTableByGpId(&gpd_ID,ProxyTableEntryTemp,&NvProxyTableIndex) == ZSuccess )
    {
      if(PROXY_TBL_GET_FIRST_TO_FORWARD(ProxyTableEntryTemp[PROXY_TBL_ENTRY_OPT]) == 1)
      {
        PROXY_TBL_SET_FIRST_TO_FORWARD(&ProxyTableEntryTemp[PROXY_TBL_ENTRY_OPT]);
        osal_nv_write(NvProxyTableIndex,PROXY_TBL_ENTRY_OPT,2,&ProxyTableEntryTemp[PROXY_TBL_ENTRY_OPT]);
      } 
    }
    
    //Also remove any packet to the GPD
    gp_DataReq->Action = 0;
  }
  
  osal_msg_send(gp_TaskID,(uint8*)gp_DataReq);
}



/*********************************************************************
 * @fn      zclGp_GpProxyCommissioningModeCB
 *
 * @brief   Callback from the ZCL GreenPower Cluster Library when
 *          it received a Gp Commissioning Mode command.
 *
 * @param   pCmd - command payload
 *
 * @return  none
 */
static void zclGp_GpProxyCommissioningModeCB(zclGpProxyCommissioningMode_t* pCmd)
{
  gp_commissioningOptions = pCmd->options;
  uint32 CommissioningWindow = PGG_COMMISSIONING_WINDOW;
  
  if ( ( zgGP_InCommissioningMode == TRUE ) && ( gp_commissionerAddress != pCmd->srcAddr ) )
  {
    // If is in commissioning mode and the soruce address is different from 
    // the device that set the proxy in commissioning mode, then drop
    // the request.
    return;
  }

  //Enter in commissioning mode
  if(gp_commissioningOptions & GP_COMM_OPT_ACTION_MASK)
  {
  
#if 0
    if(gp_commissioningOptions & GP_COMM_OPT_CHANNEL_PRES_MASK)
    {
      //Section A.3.3.5.3, channel field is not supported in the current version of the spec
    }
#endif
 
    if(gp_commissioningOptions & GP_COMM_OPT_EXIT_ON_WINDOW_EXPIRATION_MASK)
    {
      CommissioningWindow = osal_build_uint16(pCmd->pData);
      pCmd->pData += sizeof(uint16);
      
      
    }
    if(gp_commissioningOptions & GP_COMM_OPT_UNICAST_COMM_MASK)
    {
      gp_commissionerAddress = pCmd->srcAddr;
    }
    else
    {
      gp_commissionerAddress = 0xFFFF;
    }
    
    //Convert to timer units
    CommissioningWindow = CommissioningWindow * 1000;
    
    //Exit upon expire
    zgGP_InCommissioningMode = TRUE;
    
    //Notify the user about entering in commissioning mode
    if(pfnCommissioningMode)
    {
      pfnCommissioningMode(TRUE);
    }
  }
  //Exit commissioning mode
  else
  { 
    gp_stopCommissioningMode();
  }
}

 /*********************************************************************
 * @fn          gp_ZclPairingParse
 *
 * @brief       Parse the pairing data message payload
 *
 * @param       
 *
 * @return      
 */
static void gp_ZclPairingParse( zclGpPairing_t* pCmd, gpPairingCmd_t* payload )
{

  payload->options |= ( ( ( uint32 ) pCmd->options[2] << 16 ) & 0x00FF0000 );
  payload->options |= ( ( ( uint32 ) pCmd->options[1] <<  8 ) & 0x0000FF00 );
  payload->options |= ( ( ( uint32 ) pCmd->options[0] )       & 0x000000FF );
    
  // Options bitfield
  // If Application Id bitfield is 0b000
  if( GP_IS_APPLICATION_ID_GPD( payload->options ) )
  {        
    // Populate GPD ID
    gp_PopulateField( ( uint8* )&payload->gpdId, &pCmd->pData, sizeof( payload->gpdId ) );
    
    // Populate GPD IEEE Invalid
    osal_memset ( &payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN );
    // Populate EP Invalid
    payload->ep = 0xFF;
  }
  // If Application Id bitfield is 0b010
  else if( GP_IS_APPLICATION_ID_IEEE( payload->options ) )
  {
    // Populate GPD ID Invalid
    payload->gpdId = 0xFFFFFFFF;
    
    // Populate GPD IEEE
    gp_PopulateField( ( uint8* )&payload->gpdIEEE, &pCmd->pData, Z_EXTADDR_LEN );
    // Populate EP
    gp_PopulateField( ( uint8* )&payload->ep, &pCmd->pData, sizeof( payload->ep ) );
  }
  
  // If Remove GPD bit is 0b0
  if( !GP_REMOVE_GPD( payload->options ) )
  {
    // Communication Mode 0b00 or 0b11
    if( ( GP_IS_COMMUNICATION_MODE_FULL_UNICAST( payload->options ) ) || ( GP_IS_COMMUNICATION_MODE_LIGHT_UNICAST( payload->options ) ) )
    {
      // Populate Sink Addesses
      gp_PopulateField( ( uint8* )&payload->sinkIEEE, &pCmd->pData, Z_EXTADDR_LEN );
      gp_PopulateField( ( uint8* )&payload->sinkNwkAddr, &pCmd->pData, sizeof( payload->sinkNwkAddr ) );
      
      // Populate Grp Address Ivalid
      payload->sinkGroupID = 0xFFFF;
    }
    // Communication Mode 0b01 or 0b10
    else if ( ( GP_IS_COMMUNICATION_MODE_GRPCAST_DGROUP_ID( payload->options ) ) || ( GP_IS_COMMUNICATION_MODE_GRPCAST_GROUP_ID( payload->options ) ) )
    {
      // Populate Sink Addesses Invalid
      osal_memset ( &payload->sinkIEEE, 0xFF, Z_EXTADDR_LEN );
      payload->sinkNwkAddr = 0xFFFF;
      
      // Populate Grp Address
      if( GP_IS_APPLICATION_ID_IEEE( payload->options ) )
      {
        payload->sinkGroupID |= ((payload->gpdIEEE[6]) << 8) & 0xFF00;
        payload->sinkGroupID |= (payload->gpdIEEE[7]) & 0x00FF;
        pCmd->pData +=2;
      }
      else
      {
        gp_PopulateField( ( uint8* )&payload->sinkGroupID, &pCmd->pData, sizeof( payload->sinkGroupID ) );
      }
    }
  }
  else
  {
    uint8 i;
    uint8 status;
    uint8 currEntry[PROXY_TBL_ENTRY_LEN];
      
    gp_ResetProxyBasicTblEntry(currEntry);
    // Remove
    for ( i = 0; i <= GPP_MAX_PROXY_TABLE_ENTRIES ; i++ )
    {
      status = gp_getProxyTableByIndex( ( ZCD_NV_PROXY_TABLE_START + i ), currEntry );

      if ( status == NV_OPER_FAILED )
      {
        // FAIL
        return;
      }
    
      // if the entry is empty
      if ( status == NV_INVALID_DATA )
      {
        // Look for the next entry
        continue;
      }
      
      if( GP_IS_APPLICATION_ID_GPD( payload->options ) )
      {
        if ( osal_memcmp( &currEntry[PROXY_TBL_ENTRY_GPD_ID + 4], &payload->gpdId, sizeof ( uint32 ) ) )
        {
          // Remove this GPD entry
          gp_ResetProxyBasicTblEntry( currEntry );
        }
      }
      else if( GP_IS_APPLICATION_ID_IEEE( payload->options ) )
      {   
        if ( osal_memcmp( &currEntry[PROXY_TBL_ENTRY_GPD_ID ], payload->sinkIEEE, Z_EXTADDR_LEN ) )
        {
          // Remove this GPD entry
          gp_ResetProxyBasicTblEntry( currEntry );
        }
      }
    }
    return;
  }
  
  // If Add Sink bit is 0b1
  if ( GP_ADD_SINK( payload->options ) )
  {
    // Populate Device ID
    gp_PopulateField( ( uint8* )&payload->deviceId, &pCmd->pData, sizeof( payload->deviceId ) );
    
    if ( GP_SEC_COUNTER( payload->options ) )
    {
      // Populate GPD security frame counter
      gp_PopulateField( ( uint8* )&payload->gpdSecCounter, &pCmd->pData, sizeof( payload->gpdSecCounter ) );
    }
    else
    {
      payload->gpdSecCounter = 0xFFFFFFFF;
    }
    
    if ( GP_SEC_KEY ( payload->options ) )
    {
      gp_PopulateField( ( uint8* )&payload->gpdKey, &pCmd->pData, SEC_KEY_LEN );
    }
    else
    {
      osal_memset ( &payload->gpdKey, 0xFF, SEC_KEY_LEN );
    }
    
    if ( GP_ALIAS ( payload->options ) )
    {
      gp_PopulateField( ( uint8* )&payload->assignedAlias, &pCmd->pData, sizeof( payload->assignedAlias ) );
    }
    else
    {
      payload->assignedAlias = 0xFFFF;
    }
    if ( GP_FORWARDING_RADIUS ( payload->options ) )
    {
      gp_PopulateField( ( uint8* )&payload->forwardingRadius, &pCmd->pData, sizeof( payload->forwardingRadius ) );
    }
    else
    {
      payload->forwardingRadius = 0xFF;
    }
  }
  else
  {
    // Ivalidate every field
    payload->deviceId = 0xFF;
    payload->gpdSecCounter = 0xFFFFFFFF;
    payload->assignedAlias = 0xFFFF;
    payload->forwardingRadius = 0xFF;
  }
}

 /*********************************************************************
 * @fn          gp_ZclProxyTableReqParse
 *
 * @brief       Parse the proxy table request data message payload
 *
 * @param       
 *
 * @return      
 */
static void gp_ZclProxyTableReqParse( zclGpProxyTableRequest_t* pCmd, gpProxyTableReqCmd_t* payload )
{

  payload->options =  pCmd->options;
    
  // If Request type bitfield is 0b00
  if( GP_IS_REQ_TYP_GPD( payload->options ) )
  {  
    // If Application Id bitfield is 0b000
    if( GP_IS_APPLICATION_ID_GPD( payload->options ) )
    {        
      // Populate GPD ID
      gp_PopulateField( ( uint8* )&payload->gpdId, &pCmd->pData, sizeof( payload->gpdId ) );
    
      // Populate GPD IEEE Invalid
      osal_memset ( &payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN );
      // Populate EP Invalid
      payload->ep = 0xFF;
    }
    // If Application Id bitfield is 0b010
    else if( GP_IS_APPLICATION_ID_IEEE( payload->options ) )
    {
      // Populate GPD ID Invalid
      payload->gpdId = 0xFFFFFFFF;
    
      // Populate GPD IEEE
      gp_PopulateField( ( uint8* )&payload->gpdIEEE, &pCmd->pData, Z_EXTADDR_LEN );
      // Populate EP
      gp_PopulateField( ( uint8* )&payload->ep, &pCmd->pData, sizeof( payload->ep ) );
    }
    // Populate Index Invalid
    payload->index = 0xFF;
  }
  // If Request type bitfield is 0b01
  else if( GP_IS_REQ_TPY_INDEX( payload->options ) )
  {
    // Populate GPD ID Invalid
    payload->gpdId = 0xFFFFFFFF;
    
    // Populate GPD IEEE Invalid
    osal_memset ( &payload->gpdIEEE, 0xFF, Z_EXTADDR_LEN );
    
    // Populate EP Invalid
    payload->ep = 0xFF;
    
    // Populate index
    gp_PopulateField( ( uint8* )&payload->index, &pCmd->pData, sizeof( payload->index ) );
  }

}

 /*********************************************************************
 * @fn          gp_PopulateField
 *
 * @brief       General function fill the proxy table vector item
 *
 * @param       
 *
 * @return      
 */
void gp_PopulateField( uint8* pField, uint8** pData, uint8 size )
{
  switch ( size )
  {
    case ( sizeof( uint8 ) ):
      gp_u8CastPointer( pField, *pData );
      break;
    case ( sizeof( uint16 ) ):
      gp_u16CastPointer( pField, *pData );
      break;
    case ( sizeof( uint32 ) ):
      gp_u32CastPointer( pField, *pData );
      break;
    case ( Z_EXTADDR_LEN ):
      osal_memcpy( pField, *pData, Z_EXTADDR_LEN );
      break;
    case ( SEC_KEY_LEN ):
      osal_memcpy( pField, *pData, SEC_KEY_LEN );
      break;
  }
  *pData += size;
}

 /*********************************************************************
 * @fn          gp_ProxyTblInit
 *
 * @brief       General function to init the NV items for proxy table
 *
 * @param       
 *
 * @return      
 */

uint8 gp_ProxyTblInit( uint8 resetTable )
{
  uint8 i;
  uint8 status;
  uint8 emptyEntry[PROXY_TBL_ENTRY_LEN];
  
  gp_ResetProxyBasicTblEntry( emptyEntry );
  
  for ( i = 0; i <= GPP_MAX_PROXY_TABLE_ENTRIES ; i++ )
  {
    status = osal_nv_item_init( ( ZCD_NV_PROXY_TABLE_START + i ),
                                       PROXY_TBL_ENTRY_LEN, &emptyEntry );
    
    if ( ( status != SUCCESS ) && ( status != NV_ITEM_UNINIT ) )
    {
      return status;
    }
    if ( ( status == SUCCESS ) && ( resetTable == TRUE ) )
    {
      status = osal_nv_write( ( ZCD_NV_PROXY_TABLE_START + i ), 0,
                                    PROXY_TBL_ENTRY_LEN, &emptyEntry );
    }
  }
  return status;
}


 /*********************************************************************
 * @fn          gp_getProxyTableByGpId
 *
 * @brief       General function to get proxy table entry by gpd_ID (GP Src ID or Extended Adddress)
 *
 * @param       gpd_ID  - address to look for in the table
 *              pEntry  - buffer in which the entry of the table will be copied
 *
 * @return      
 */

uint8 gp_getProxyTableByGpId(gpd_ID_t *gpd_ID, uint8 *pEntry,uint16 *NvProxyTableIndex)
{
  uint8 i;
  uint8 status;

  if((pEntry == NULL) ||  (gpd_ID == NULL))
  {
    return ZInvalidParameter;
  }     
  

  for ( i = 0; i <= GPP_MAX_PROXY_TABLE_ENTRIES ; i++ )
  {
    status = gp_getProxyTableByIndex( ( ZCD_NV_PROXY_TABLE_START + i ), pEntry );

    if ( status == NV_OPER_FAILED )
    {
      // FAIL
      return ZFailure;
    }
    
    // if the entry is empty
    if ( status == NV_INVALID_DATA )
    {
      continue;
    }

    //Check that App ID is the same

    if ( PROXY_TBL_COMP_APPLICTION_ID( gpd_ID->AppID, pEntry[PROXY_TBL_ENTRY_OPT] ) )
    {
      if ( gpd_ID->AppID == GP_OPT_APP_ID_GPD )
      {
        if ( osal_memcmp( &gpd_ID->GPDId.SrcID, &pEntry[PROXY_TBL_ENTRY_GPD_ID + 4], sizeof ( uint32 ) ) )
        {
          // Entry found
          break;
        }
      }

      else if ( gpd_ID->AppID == GP_OPT_APP_ID_IEEE )
      {
        if ( osal_memcmp( &gpd_ID->GPDId.GPDExtAddr, &pEntry[PROXY_TBL_ENTRY_GPD_ID], Z_EXTADDR_LEN ) )
        {
          // Entry found
          break;
        }
      }
    }
    else
    {
      continue;
    }
  }

  if(i <= GPP_MAX_PROXY_TABLE_ENTRIES)
  {
    if(NvProxyTableIndex != NULL)
    {
      *NvProxyTableIndex = ZCD_NV_PROXY_TABLE_START + i;
    }
    return ZSuccess;
  }
    
  return ZInvalidParameter;
}

 /*********************************************************************
 * @fn          gp_SecurityOperationProxy
 *
 * @brief       Performs Security Operations according to Proxy
 *
 * @param       ind - pointer to gp data indication
 * @param       pFrameCounter 
 * @param       pKeyType 
 * @param       pkey Key to be used to decript the packet (if applies)
 *
 * @return      GP_SEC_RSP status
 */
uint8 gp_SecurityOperationProxy( gp_DataInd_t* pInd, uint8* pKeyType, uint8* pKey)
{
  uint8    currEntry[PROXY_TBL_ENTRY_LEN];
  uint8    status;
  uint32   SecFrameCounter = 0;
  uint8    securityCheckFail = FALSE;
  uint8    endpointCheckFail = FALSE;
  uint16   NvProxyTableIndex = 0;
  gpd_ID_t gpd_ID;
  
  gp_ResetProxyBasicTblEntry(currEntry);
  
  if((pKeyType == NULL) || (pKey == NULL) || (pInd == NULL))
  {
    return GP_SEC_RSP_ERROR;
  }
  *pKeyType = 0;
  osal_memset(pKey,0,SEC_KEY_LEN);
  
  gpd_ID.AppID = pInd->appID;
  
  if(gpd_ID.AppID == GP_APP_ID_DEFAULT)
  {
    gpd_ID.GPDId.SrcID = pInd->SrcId;
  }
  else
  {
    osal_memcpy(gpd_ID.GPDId.GPDExtAddr,pInd->srcAddr.addr.extAddr,Z_EXTADDR_LEN);
  }
  
  status = gp_getProxyTableByGpId(&gpd_ID,currEntry,&NvProxyTableIndex);
  
  //Not found
  if(status == ZInvalidParameter)
  {
    //Section A.3.5.2.1 if in commissioning mode and GPDF from proxy that do not 
    //have entry, then drop frame
    if(zgGP_InCommissioningMode == FALSE)
    {
      return GP_SEC_RSP_DROP_FRAME;
    }
    if(pInd->GPDFKeyType == 0)
    {
      //If there is no shared key, then pass unprocess
      if(osal_memcmp(pKey,zgpSharedKey,SEC_KEY_LEN))
      {
        return GP_SEC_RSP_PASS_UNPROCESSED;
      }
      osal_memcpy(pKey,zgpSharedKey,SEC_KEY_LEN);
    }
    else
    {
      return GP_SEC_RSP_PASS_UNPROCESSED;
    }
  }
  
  //error
  else if(status == ZFailure)
  {
    //Not found, or error, drop the frame
    return GP_SEC_RSP_DROP_FRAME;
  }
  
  //Found
  else
  {
    
//Active/Inactive entries in the proxy table are not supported    
#if 1
      //If security level is zero then don't check and pass unproessed
      if( (pInd->GPDFSecLvl == PROXY_TBL_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) ) &&
          ( GP_SECURITY_LVL_NO_SEC == PROXY_TBL_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) ))
      {
        pInd->GPDSecFrameCounter = (uint32)pInd->SeqNumber;
        return GP_SEC_RSP_PASS_UNPROCESSED;
      }
      //Check security Section A.3.7.3.3
      //Check framecounter freshness
      SecFrameCounter = osal_build_uint32(&currEntry[PROXY_TBL_ENTRY_SEC_FRAME],sizeof(uint32));
      if(SecFrameCounter >= pInd->GPDSecFrameCounter)
      {
        securityCheckFail = TRUE;
      }
      //Compare the security level
      else if(pInd->GPDFSecLvl != PROXY_TBL_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_ENTRY_SEC_OPT]))
      {
        securityCheckFail = TRUE;
      }
      //Mapping of security key type (section A.1.4.1.3 Table 12)
      else if((PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) <= 0x03)
              && (pInd->GPDFKeyType == 1))
      {
        securityCheckFail = TRUE;
      }
      else if(  ((PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x07) ||
                 (PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x04)) 
               && (pInd->GPDFKeyType == 0) )
      {
        securityCheckFail = TRUE;
        
      }
      else if( (PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x05) ||
               (PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x06) )
      {
        //keytype reserved
        securityCheckFail = TRUE;
      }
      
      if(securityCheckFail == TRUE)
      {
        if(zgGP_InCommissioningMode == FALSE)
        {
          return GP_SEC_RSP_DROP_FRAME;
        }
        else
        {
          return GP_SEC_RSP_PASS_UNPROCESSED;
        }
      }
      else
      {
        //Securty check success
        osal_memcpy(pKey,&currEntry[PROXY_TBL_ENTRY_GPD_KEY],SEC_KEY_LEN);
        *pKeyType = PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]);
      
        //compare the endpoint
        if(endpointCheckFail)
        {
          return GP_SEC_RSP_TX_THEN_DROP;
        }
        else
        {
          return GP_SEC_RSP_MATCH;
        }
      }
      
//Active/Inactive entries in the proxy table are not supported
#else
    //Is active
    if(PROXY_TBL_GET_ENTRY_ACTIVE(currEntry[PROXY_TBL_ENTRY_OPT]))
    {
      //Check security Section A.3.7.3.3
      //Check framecounter freshness
      SecFrameCounter = osal_build_uint32(&currEntry[PROXY_TBL_ENTRY_SEC_FRAME],sizeof(uint32));
      if(SecFrameCounter >= pInd->GPDSecFrameCounter)
      {
        securityCheckFail = TRUE;
      }
      //Compare the security level
      else if(pInd->GPDFSecLvl != PROXY_TBL_GET_SEC_OPT_SECURITY_LVL(currEntry[PROXY_TBL_ENTRY_SEC_OPT]))
      {
        securityCheckFail = TRUE;
      }
      //Mapping of security key type (section A.1.4.1.3 Table 12)
      else if((PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) <= 0x03)
              && (pInd->GPDFKeyType == 1))
      {
        securityCheckFail = TRUE;
      }
      else if(  ((PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x07) ||
                 (PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x04)) 
               && (pInd->GPDFKeyType == 0) )
      {
        securityCheckFail = TRUE;
        
      }
      else if( (PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x05) ||
               (PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(currEntry[PROXY_TBL_ENTRY_SEC_OPT]) == 0x06) )
      {
        //keytype reserved
        securityCheckFail = TRUE;
      }
      
      if(securityCheckFail == TRUE)
      {
        if(zgGP_InCommissioningMode == FALSE)
        {
          return GP_SEC_RSP_DROP_FRAME;
        }
        else
        {
          return GP_SEC_RSP_PASS_UNPROCESSED;
        }
      }
      else
      {
        //Securty check success
        osal_memcpy(pKey,&currEntry[PROXY_TBL_ENTRY_GPD_KEY],SEC_KEY_LEN);
        *pKeyType = GP_GET_SEC_KEY_TYPE(currEntry[PROXY_TBL_ENTRY_SEC_OPT]);
      
        //compare the endpoint
        if(endpointCheckFail)
        {
          return GP_SEC_RSP_TX_THEN_DROP;
        }
        else
        {
          return GP_SEC_RSP_MATCH;
        }
      }
    }

    //Inactive
    else
    {
      //Found, inactive and in operational mode
      if(zgGP_InCommissioningMode == FALSE)
      {
        //Check framecounter freshness
        SecFrameCounter = osal_build_uint32(&currEntry[PROXY_TBL_ENTRY_SEC_FRAME],sizeof(uint32));
        if(SecFrameCounter < pInd->GPDSecFrameCounter)
        {
          //Update framecounter
           osal_memcpy(&currEntry[PROXY_TBL_ENTRY_SEC_FRAME],(uint8*)&pInd->GPDSecFrameCounter,sizeof(uint32));

           status = osal_nv_write( NvProxyTableIndex, 0,
                               PROXY_TBL_ENTRY_LEN, currEntry );
           
           if ( status != SUCCESS )
           {
             // FAIL
             return GP_SEC_RSP_ERROR;
           }
        }
        return GP_SEC_RSP_DROP_FRAME;
      }
      //Found,inactive, and in commissioning mode
      else
      {
        //If key type = 0, its the same as not found
        if(pInd->GPDFKeyType == 0)
        {
          Found = FALSE;
        }
        else
        {
          return GP_SEC_RSP_PASS_UNPROCESSED;
        }
      }
    }
#endif
    
  }


  //Should not reach this point
  return GP_SEC_RSP_DROP_FRAME;
}

 /*********************************************************************
 * @fn          gp_u32CastPointer
 *
 * @brief       General function fill uint32 from pionter.
 *
 * @param       pBitField - pointer to the bit field.
 *              bit       - position of the bit to set in the given bitfield.
 *
 * @return      
 */
static void gp_u32CastPointer( uint8 *data, uint8 *p )
{
    *( uint32* )data |= ( ( *( uint32* ) p++ )       & 0x000000FF );
    *( uint32* )data |= ( ( *( uint32* ) p++ <<  8 ) & 0x0000FF00 );
    *( uint32* )data |= ( ( *( uint32* ) p++ << 16 ) & 0x00FF0000 );
    *( uint32* )data |= ( ( *( uint32* ) p++ << 24 ) & 0xFF000000 );
}

 /*********************************************************************
 * @fn          gp_u16CastPointer
 *
 * @brief       General function fill uint16 from pionter.
 *
 * @param       pBitField - pointer to the bit field.
 *              bit       - position of the bit to set in the given bitfield.
 *
 * @return      
 */
static void gp_u16CastPointer( uint8 *data, uint8 *p )
{
    *( uint16* )data |= ( ( *( uint16* ) p++ )       & 0x00FF );
    *( uint16* )data |= ( ( *( uint16* ) p++ <<  8 ) & 0xFF00 );
}

 /*********************************************************************
 * @fn          gp_u8CastPointer
 *
 * @brief       General function fill uint16 from pionter.
 *
 * @param       pBitField - pointer to the bit field.
 *              bit       - position of the bit to set in the given bitfield.
 *
 * @return      
 */
static void gp_u8CastPointer( uint8 *data, uint8 *p )
{
    *( uint8* )data |= ( ( *( uint8* ) p ) );
}

 /*********************************************************************
 * @fn          gp_getProxyTableByIndex
 *
 * @brief       General function to get proxy table entry by NV index
 *
 * @param       nvIndex - NV Id of proxy table
 *              pEntry  - pointer to PROXY_TBL_ENTRY_LEN array
 *
 * @return      
 */
uint8 gp_getProxyTableByIndex( uint16 nvIndex, uint8 *pEntry )
{
  uint8 status;
  uint16 emptyEntry = 0xFFFF;
  
  
  if ( ( nvIndex < ZCD_NV_PROXY_TABLE_START ) || ( nvIndex > ZCD_NV_PROXY_TABLE_END ) )
  {
    return NV_OPER_FAILED;
  }
  
  status = osal_nv_read( nvIndex, 0,
                          PROXY_TBL_ENTRY_LEN, pEntry );
  
  if ( status != SUCCESS )
  {
    // Return the failure status of NV read procedure
    return status;
  }
    
      
  // if the entry is empty
  if ( osal_memcmp( pEntry, &emptyEntry, sizeof ( uint16 ) ) )
  {
    return NV_INVALID_DATA;
  }
  
  return status;
}

/*********************************************************************
 * @fn          GP_DataInd
 *
 * @brief       This passes the MCPS data indications received in MAC to the application
 *
 * @param       gp_DataInd
 *
 * @return      FreeMsg - TRUE if the message will be released, False if it will 
 *                        be keeped for duplicate filtering
 */
uint8 GP_DataInd(gp_DataInd_t *gp_DataInd)
{
  uint32   timeout;
  uint8    freeMsg = FALSE;
  gpd_ID_t gpd_ID;
  uint8    ProxyTableEntryTemp[PROXY_TBL_ENTRY_LEN];
  uint16   ProxyTableEntryIndex = 0;
  
  //If authentication/decryption fail, then drop the frame and stop processing
  //GP Basic proxy A.3.5.2.3
  if(gp_DataInd->status == GP_DATA_IND_STATUS_COUNTER_FAILURE ||
      gp_DataInd->status == GP_DATA_IND_STATUS_AUTH_FAILURE )
  {
    return freeMsg;
  }

  gp_DataIndAppendToList(gp_DataInd, &gp_DataIndList);
  
  gp_DataInd->SecReqHandling.timeout = gpDuplicateTimeout;
  //Consider the current time elapsed to the next timeout
  timeout = osal_get_timeoutEx(gp_TaskID,GP_DUPLICATE_FILTERING_TIMEOUT_EVENT);
  
  if(timeout)
  {
    gp_DataInd->SecReqHandling.timeout += timeout;
  }  
  else
  {
    osal_start_timerEx(gp_TaskID,GP_DUPLICATE_FILTERING_TIMEOUT_EVENT,gp_DataInd->SecReqHandling.timeout);
  }  

  gpd_ID.AppID = gp_DataInd->appID;
  if(gp_DataInd->appID == GP_OPT_APP_ID_IEEE)
  {
    osal_memcpy(gpd_ID.GPDId.GPDExtAddr, gp_DataInd->srcAddr.addr.extAddr, Z_EXTADDR_LEN);
  }
  else
  {
    gpd_ID.GPDId.SrcID = gp_DataInd->SrcId;
  }

  if(gp_getProxyTableByGpId(&gpd_ID,ProxyTableEntryTemp,&ProxyTableEntryIndex) == ZSuccess)
  {

    //Update Sec Frame counter to proxy table  A.3.6.1.3
    osal_nv_write( ProxyTableEntryIndex, PROXY_TBL_ENTRY_SEC_FRAME,
                       sizeof(uint32), (uint8*) &gp_DataInd->GPDSecFrameCounter );
  }  

  if ( zgGP_InCommissioningMode == TRUE )
  {
    if(gp_DataInd->GPDCmmdID == GP_CHANNEL_REQ_COMMAND_ID)
    {
      if(pfnChangeChannelReqForBDB)
      {
        //Check if we got permission from BDB
        if(!pfnChangeChannelReqForBDB())
        {
          //No permisssion
          return freeMsg;
        }
      }
      if(pfnChangeChannelReq)
      {
        //Check if we got permission from BDB
        if(!pfnChangeChannelReq())
        {
          //No permisssion
          return freeMsg;
        }
      }
      
      osal_start_timerEx(gp_TaskID,GP_CHANNEL_CONFIGURATION_TIMEOUT,gpBirectionalCommissioningChangeChannelTimeout);
    }
    GP_ProccessCommissioningNotification( gp_DataInd );
  }
  else if ( ( zgGP_InCommissioningMode == FALSE ) )
  {
    GP_ProccessNotification( gp_DataInd );
  }
  
  return freeMsg;
}

/*********************************************************************
 * @fn          GP_ProccessCommissioningNotification
 *
 * @brief       General function to process the GP Manteinance Indication in operational mode
 *
 * @param       gp_DataInd
 *
 * @return      none
 */
static void GP_ProccessCommissioningNotification(gp_DataInd_t *gp_DataInd)
{
  gpCommissioningNotificationCmd_t gpNotification;
  
  if ( ( gp_DataInd->GPDCmmdID == GP_COMMISSIONING_COMMAND_ID ) || ( gp_DataInd->GPDCmmdID == GP_SUCCESS_COMMAND_ID ) ||
       ( gp_DataInd->GPDCmmdID == GP_CHANNEL_REQ_COMMAND_ID ) )
  {
  
    osal_memset( &gpNotification, 0, sizeof ( gpCommissioningNotificationCmd_t ) );
    zclGp_ManteinanceIndParse( gp_DataInd, &gpNotification );
  
    zclGp_SendGpCommissioningNotificationCommand ( &gpNotification );
  }
}

/*********************************************************************
 * @fn          GP_ProccessNotification
 *
 * @brief       General function to process the GP Data Indication in operational mode
 *
 * @param       gp_DataInd
 *
 * @return      none
 */
static void GP_ProccessNotification(gp_DataInd_t *gp_DataInd)
{
  gpNotificationCmd_t gpNotification;
  
  osal_memset( &gpNotification, 0, sizeof ( gpNotificationCmd_t ) );
  zclGp_DataIndParse( gp_DataInd, &gpNotification );
  
  zclGp_SendGpNotificationCommand ( &gpNotification, gp_DataInd->SeqNumber );
}

/*********************************************************************
 * @fn          GP_SecReq
 *
 * @brief       Primitive from dGP stub to GP EndPoint asking how to process a GPDF.
 *
 * @param       gp_SecReq 
 *
 * @return      none
 */
uint8 GP_SecReq(gp_SecReq_t *gp_SecReq)
{
  gp_SecRsp_t *gp_SecRsp = NULL;
  uint8 status;
  uint8  ProxyTableEntryTemp[PROXY_TBL_ENTRY_LEN];
  gp_DataInd_t* temp;
  uint8   KeyType;
  uint8   Key[SEC_KEY_LEN];


  gp_SecRsp = (gp_SecRsp_t*)osal_msg_allocate(sizeof(gp_SecRsp_t));

  //No mem, then do not release the msg, process later
  if(gp_SecRsp == NULL)
  {
    return FALSE;
  }
  
  gp_SecRsp->hdr.event = GP_SEC_RSP;
  gp_SecRsp->hdr.status = ZSuccess;

  gp_SecRsp->dGPStubHandle = gp_SecReq->dGPStubHandle;
  gp_SecRsp->EndPoint = gp_SecReq->EndPoint;

  osal_memcpy(&gp_SecRsp->gp_SecData,&gp_SecReq->gp_SecData, sizeof(gp_SecData_t));
  osal_memcpy(&gp_SecRsp->gpd_ID,&gp_SecReq->gpd_ID, sizeof(gpd_ID_t));

  gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;
 
  //Find duplicates A.3.6.1.2 Duplicate filtering
  if( gp_DataIndFindDuplicate(gp_SecReq->dGPStubHandle, gp_SecReq->gp_SecData.GPDFSecLvl) )
  {  //Check if the entry exist
    if ( gp_getProxyTableByGpId(&gp_SecReq->gpd_ID, ProxyTableEntryTemp, NULL) == ZSuccess )
    {
      gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;
      osal_msg_send(gp_TaskID,(uint8*)gp_SecRsp);

      return TRUE;
    }
  }
  
  temp = gp_DataIndGet(gp_SecReq->dGPStubHandle);
    
  //Section A.3.7.3.3
  status = gp_SecurityOperationProxy(temp,&KeyType,Key);

  switch(status)
  {
    case GP_SEC_RSP_ERROR:  //This should not happen
    case GP_SEC_RSP_DROP_FRAME:
      gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;
    break;
    
    case GP_SEC_RSP_PASS_UNPROCESSED:  
      gp_SecRsp->Status = GP_SEC_RSP_PASS_UNPROCESSED;
    break;
    
    case GP_SEC_RSP_MATCH:
    case GP_SEC_RSP_TX_THEN_DROP:
      if(GP_RecoveryKey(gp_SecReq->gp_SecData.GPDFKeyType,KeyType,status,Key) == GP_SEC_RSP_DROP_FRAME)
      {
        gp_SecRsp->Status = GP_SEC_RSP_DROP_FRAME;
      }
      else
      {
        gp_SecRsp->Status = status;
        osal_memcpy(gp_SecRsp->GPDKey,Key,SEC_KEY_LEN);
      }
    break;
  }


  osal_msg_send(gp_TaskID,(uint8*)gp_SecRsp);
  return TRUE;
}

/*********************************************************************
 * @fn          GP_RecoveryKey
 *
 * @brief       Procedure to retrive the key to be used to decrypt the GPDF
 *
 * @param       GPDFKeyType   KeyType from the GPDF
 * @param       KeyType       Key type in the Proxy table entry for this GPD
 * @param       Status        Previous status to search key (MATCH, TX_THEN_DROP)
 * @param       Key[out]      Key to be used
 *                       
 * @return      Status
 */
uint8 GP_RecoveryKey(uint8 GPDFKeyType,uint8 KeyType, uint8 status, uint8 *Key)
{
  uint8 TempKey[SEC_KEY_LEN] = {0xFF};
  //TODO: A.3.7.3.4 Incoming frames: key recovery
  if(GPDFKeyType)
  {
    if(KeyType == GP_SECURITY_KEY_TYPE_OUT_OF_BOX_GPD_KEY)
    {
      //Is the key of the Proxy table entry empty?
      if(osal_memcmp(&Key,&TempKey,SEC_KEY_LEN))
      {
        return GP_SEC_RSP_DROP_FRAME;
      }          
      else
      {
        //There is a key, then use it
        return status;
      }
    }
    else if(KeyType == GP_SECURITY_KEY_TYPE_DERIVED_IND_GPD_KEY)
    {
      //Is the key of the Proxy table entry empty?
      if(osal_memcmp(&Key,&TempKey,SEC_KEY_LEN))
      {
        return GP_SEC_RSP_DROP_FRAME;
      }  
      //There is a key, then use it. Derived keys are provided by the Sink 
      //device at paring time according to A.3.7.1.2.2
      return status;
    }
    else
    {
      return GP_SEC_RSP_DROP_FRAME;
    }
  }
  else
  {
    uint8  gpSharedSecKeyType;
    uint16 AttLen;

    //Get the SharedKeyType Attribute
    zcl_ReadAttrData(GREEN_POWER_INTERNAL_ENDPOINT,ZCL_CLUSTER_ID_GREEN_POWER,ATTRID_GP_SHARED_SEC_KEY_TYPE,&gpSharedSecKeyType,&AttLen);

    if(KeyType == gpSharedSecKeyType)
    {
      if(!osal_memcmp(TempKey,zgpSharedKey,SEC_KEY_LEN))
      {
        //Use key shared key
        osal_memcpy(Key,zgpSharedKey,SEC_KEY_LEN);
        return status;
      }
    }
    if(KeyType == GP_SECURITY_KEY_TYPE_ZIGBEE_NWK_KEY)
    {
      ZDSecMgrReadKeyFromNv(ZCD_NV_PRECFGKEY,Key);
    }
    //There is a key, then use it. Derived keys are provided by the Sink 
    //device at paring time according to A.3.7.1.2.2
    else
    {
      return GP_SEC_RSP_DROP_FRAME;
    }
  }
  return status;
}


gp_DataInd_t* gp_DataIndFindDuplicate(uint8 handle, uint8 secLvl)
{
  gp_DataInd_t* temp;
  gp_DataInd_t* tempList;
  temp = gp_DataIndGet(handle);
  
  if(temp != NULL)
  {
    tempList = gp_DataIndList;
    
    while(tempList != NULL)
    {
      //search for MAC seq num
      if(secLvl == 0)
      {
        if((temp->SeqNumber == tempList->SeqNumber) && (temp->appID == tempList->appID)&&
           (temp->SecReqHandling.dGPStubHandle != tempList->SecReqHandling.dGPStubHandle))
        {
          break;
        }
      }
      //Other secLevels uses SecFrameCounter
      else
      {
        if((temp->GPDSecFrameCounter == tempList->GPDSecFrameCounter) && 
           (temp->SecReqHandling.dGPStubHandle != tempList->SecReqHandling.dGPStubHandle))
        {
          
          break;
        }
      }
      tempList = tempList->SecReqHandling.next;
    }
  }
  return tempList;
}
 
/*********************************************************************
 * @fn          GP_DataCnf
 *
 * @brief       Primitive to notify GP EndPoint the status of a previews DataReq
 *
 * @param       gp_DataCnf
 *
 * @return      none
 */
void GP_DataCnf(gp_DataCnf_t *gp_DataCnf)
{
  //Current spec does not mandates to do anything with this.
  switch(gp_DataCnf->status)
  {
    case GP_DATA_CNF_TX_QUEUE_FULL:
    case GP_DATA_CNF_ENTRY_REPLACED:
    case GP_DATA_CNF_ENTRY_ADDED:
    case GP_DATA_CNF_ENTRY_EXPIRED:
    case GP_DATA_CNF_ENTRY_REMOVED:
    case GP_DATA_CNF_GPDF_SENDING_FINALIZED:
    break;
  }  
}

 /*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

void gp_RegisterCommissioningModeCB(gpCommissioningMode_t gpCommissioningMode)
{
  if(gpCommissioningMode)
  {
     pfnCommissioningMode = gpCommissioningMode;
  }
}

/*********************************************************************
 * @fn          gp_RegisterGPChangeChannelReqCB
 *
 * @brief       Register a callback in which the application will be notified about a change
 *              of channel for at most gpBirectionalCommissioningChangeChannelTimeout ms
 *              to perform GP bidirectional commissioning in the channel parameter.
 *
 * @param       gpChangeChannelReq
 *
 * @return      none
 */
void gp_RegisterGPChangeChannelReqCB(gpChangeChannelReq_t gpChangeChannelReq)
{
  if(gpChangeChannelReq)
  {
    pfnChangeChannelReq = gpChangeChannelReq;
  }
}

/*********************************************************************
 * @fn          gp_RegisterGPChangeChannelReqForBDBCB
 *
 * @brief       Register a callback in which the bdb will be notified about a change
 *              of channel for at most gpBirectionalCommissioningChangeChannelTimeout ms
 *              to perform GP bidirectional commissioning in the channel parameter.
 *
 * @param       gpChangeChannelReq
 *
 * @return      none
 */
void gp_RegisterGPChangeChannelReqForBDBCB(gpChangeChannelReq_t gpChangeChannelReq)
{
  if(gpChangeChannelReq)
  {
    pfnChangeChannelReqForBDB = gpChangeChannelReq;
  }
}




/*********************************************************************
 * @fn          gp_returnOperationalChannel
 *
 * @brief       Return to the operational channel after bidirectional commissioning
 *
 * @param       none
 *
 * @return      none
 */
void gp_returnOperationalChannel(void)
{
  gp_DataReq_t gp_DataReq;
  _NIB.nwkLogicalChannel = gp_tempLogicalChannel;
  ZMacSetReq( ZMacChannel, &(_NIB.nwkLogicalChannel) );
  osal_stop_timerEx(gp_TaskID,GP_CHANNEL_CONFIGURATION_TIMEOUT);
  
  gp_DataReq.Action = 0;
  gp_DataReq.gpd_ID.AppID = GP_APP_ID_DEFAULT;
  gp_DataReq.gpd_ID.GPDId.SrcID = 0;
  
  GP_DataReq(&gp_DataReq);
}

     
 /*********************************************************************
 * @fn          gp_expireDuplicateFiltering
 *
 * @brief       Process the expiration of the packets in the duplicate filtering
 *              list. Assumption is the first in the queue is the first into expire.
 *
 * @param       none
 *
 * @return      none
 */    
void gp_expireDuplicateFiltering(void)
{
  gp_DataInd_t * temp;
  uint32 timeout;
  
  temp = gp_DataIndList;
  timeout = gp_DataIndList->SecReqHandling.timeout;
  
  while(temp != NULL)
  {
    if(timeout >= temp->SecReqHandling.timeout)
    {
      gp_DataInd_t *expired = temp;   
      
      temp = temp->SecReqHandling.next;
      gp_DataIndReleaseFromList(TRUE, expired,&gp_DataIndList);
    }
    else
    {
      temp->SecReqHandling.timeout -= timeout;
      temp = temp->SecReqHandling.next;
    }
  }
  osal_start_timerEx(gp_TaskID, GP_DUPLICATE_FILTERING_TIMEOUT_EVENT, timeout);
}

#endif
/*********************************************************************
*********************************************************************/

