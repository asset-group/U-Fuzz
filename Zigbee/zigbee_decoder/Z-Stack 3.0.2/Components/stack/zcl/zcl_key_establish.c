/**************************************************************************************************
  Filename:       zcl_key_establish.c
  Revised:        $Date: 2014-12-08 11:21:45 -0800 (Mon, 08 Dec 2014) $
  Revision:       $Revision: 41372 $

  Description:    Zigbee Cluster Library - SE (Smart Energy) Key Establishment


  Copyright 2007-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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


/**************************************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "ZComDef.h"
#include "eccapi_163.h"
#include "eccapi_283.h"
#include "ssp_hash.h"
#include "ZGlobals.h"
#include "AddrMgr.h"
#include "nwk_globals.h"
#include "NLMEDE.h"
#include "AF.h"
#include "ZDProfile.h"
#include "ZDObject.h"
#include "ZDSecMgr.h"
#include "stub_aps.h"
#include "zcl.h"
#include "zcl_se.h"
#include "zcl_key_establish.h"


/**************************************************************************************************
 * CONSTANTS
 */

// Key Establishment End Point Configuration
#define ZCL_KE_DEVICE_VERSION  0
#define ZCL_KE_FLAGS           0
#define ZCL_KE_CLUSTER_CNT     1

// Server-to-Client Cluster Commands
#define ZCL_KE_INITIATE_RSP      0x00
#define ZCL_KE_EPH_DATA_RSP      0x01
#define ZCL_KE_CFM_KEY_DATA_RSP  0x02
#define ZCL_KE_TERMINATE_CLIENT  0x03

// Client-to-Server Cluster Commands
#define ZCL_KE_INITIATE_REQ      0x00
#define ZCL_KE_EPH_DATA_REQ      0x01
#define ZCL_KE_CFM_KEY_DATA_REQ  0x02
#define ZCL_KE_TERMINATE_SERVER  0x03

// Key Length
#define ZCL_KE_KEY_LEN  16
#define ZCL_KE_MAC_LEN  16

// Timer Constants
#define ZCL_KE_TIMER_EVT  0x01

// ZCL_KE_STATE
#define ZCL_KE_INIT      0
#define ZCL_KE_READY     1
#define ZCL_KE_NO_CERTS  10

// ZCL_KE_MSG_TYPE
#define ZCL_KE_START_MSG         1
#define ZCL_KE_START_DIRECT_MSG  2
#define ZCL_KE_KEY_GEN_MSG       3

// Poll Rate Bits
#define ZCL_KE_CLIENT_POLL_RATE_BIT  0x01
#define ZCL_KE_SERVER_POLL_RATE_BIT  0x02

// Time between stage 1 and stage 2 -- see ZCL_KE_KEY_GEN_STAGES_*
#define ZCL_KE_KEY_GEN_TIMEOUT  500

// Invalid gen time
#define ZCL_KE_GEN_INVALID_TIME  0xFF

// Configured poll rate for end device during key establishment
#if !defined ( ZCL_KE_POLL_RATE )
#define ZCL_KE_POLL_RATE  1000
#endif

// Configure connection timeout in ms
#if !defined ( ZCL_KE_CONN_TIMEOUT )
#define ZCL_KE_CONN_TIMEOUT  60000
#endif

// Configure timeouts for suite discovery operations(Match_Desc_req, ZCL_CMD_READ, etc. )
#if !defined ( ZCL_KE_SUITE_DISCOVERY_TIMEOUT )
#define ZCL_KE_SUITE_DISCOVERY_TIMEOUT  60000
#endif 

// Configure server times for ephemeral and key data generation
#if !defined ( ZCL_KE_SERVER_EPH_DATA_GEN_TIME )
#define ZCL_KE_SERVER_EPH_DATA_GEN_TIME 30
#endif 

#if !defined ( ZCL_KE_SERVER_CFM_KEY_GEN_TIME)
#define ZCL_KE_SERVER_CFM_KEY_GEN_TIME 30
#endif 

// Configure client times for ephemeral and key data generation
#if !defined ( ZCL_KE_CLIENT_EPH_DATA_GEN_TIME )
#define ZCL_KE_CLIENT_EPH_DATA_GEN_TIME 30
#endif 

#if !defined ( ZCL_KE_CLIENT_CFM_KEY_GEN_TIME )
#define ZCL_KE_CLIENT_CFM_KEY_GEN_TIME 30
#endif 

// Configure the Trust Center's max server connections -- saved in NV "ZCD_NV_KE_MAX_DEVICES"
#if !defined ( ZCL_KE_MAX_SERVER_CONNECTIONS )
#define ZCL_KE_MAX_SERVER_CONNECTIONS  2
#endif

// ZCL_KE_SERVER_CONN_STATE
#define ZCL_KE_SERVER_CONN_INIT                   0
#define ZCL_KE_SERVER_CONN_EPH_DATA_REQ_WAIT      1
#define ZCL_KE_SERVER_CONN_KEY_GEN_WAIT           2
#define ZCL_KE_SERVER_CONN_KEY_GEN_QUEUED         3
#define ZCL_KE_SERVER_CONN_CFM_KEY_DATA_REQ_WAIT  4

// ZCL_KE_CLIENT_CONN_STATE
#define ZCL_KE_CLIENT_CONN_INIT                   0
#define ZCL_KE_CLIENT_CONN_MATCH_RSP_WAIT         1
#define ZCL_KE_CLIENT_CONN_READ_RSP_WAIT          2
#define ZCL_KE_CLIENT_CONN_INIT_RSP_WAIT          5
#define ZCL_KE_CLIENT_CONN_EPH_DATA_RSP_WAIT      6
#define ZCL_KE_CLIENT_CONN_KEY_GEN_WAIT           7
#define ZCL_KE_CLIENT_CONN_KEY_GEN_QUEUED         8
#define ZCL_KE_CLIENT_CONN_CFM_KEY_DATA_RSP_WAIT  9

// Certificate fields
#define ZCL_KE_CERT_ISSUER_LEN  8
#define ZCL_KE_CERT_163_EXT_ADDR_IDX   22
#define ZCL_KE_CERT_163_ISSUER_IDX     30
#define ZCL_KE_CERT_283_EXT_ADDR_IDX   28
#define ZCL_KE_CERT_283_ISSUER_IDX     11
#define ZCL_KE_CERT_283_TYPE_IDX       0
#define ZCL_KE_CERT_283_CURVE_IDX      9
#define ZCL_KE_CERT_283_HASH_IDX       10
#define ZCL_KE_CERT_283_KEY_USAGE_IDX  36

#define ZCL_KE_CERT_283_TYPE_VALUE     0x00
#define ZCL_KE_CERT_283_CURVE_VALUE    0x0D
#define ZCL_KE_CERT_283_HASH_VALUE     0x08
#define ZCL_KE_CERT_283_KEY_USAGE_BIT  0x08

// ZCL_KE_GET_FIELD_TYPES
#define ZCL_KE_PUBLIC_KEY_NV_ID   0    
#define ZCL_KE_PRIVATE_KEY_NV_ID  1     
#define ZCL_KE_CERT_NV_ID         2
#define ZCL_KE_PUBLIC_KEY_LEN     3
#define ZCL_KE_PRIVATE_KEY_LEN    4
#define ZCL_KE_CERT_LEN           5
#define ZCL_KE_CERT_EXT_ADDR_IDX  6
#define ZCL_KE_CERT_ISSUER_IDX    7
#define ZCL_KE_FIELDS_MAX         8

// Initiate payload fields for ZCL_KE_INITIATE_RSP and ZCL_KE_INITIATE_REQ
#define ZCL_KE_INITIATE_HDR_LEN     4
#define ZCL_KE_INITIATE_SUITE1_LEN  ZCL_KE_INITIATE_HDR_LEN + ECCAPI_CERT_163_LEN
#define ZCL_KE_INITIATE_SUITE2_LEN  ZCL_KE_INITIATE_HDR_LEN + ECCAPI_CERT_283_LEN

// Terminate payload fields for ZCL_KE_TERMINATE_CLIENT and ZCL_KE_TERMINATE_SERVER
#define ZCL_KE_TERMINATE_LEN  4


/**************************************************************************************************
 * TYPEDEFS
 */

// ZCL_KE_START_MSG payload
typedef struct
{
  osal_event_hdr_t hdr;
  uint8 taskID;
  uint16 partnerNwkAddr;
  uint8 transSeqNum;
} zclKE_StartMsg_t;

// ZCL_KE_START_DIRECT_MSG payload
typedef struct
{
  osal_event_hdr_t hdr;
  uint8 taskID;
  afAddrType_t partnerAddr;
  uint8 transSeqNum;
  uint16 suite;
} zclKE_StartDirectMsg_t;

// ZCL_KE_KEY_GEN_MSG payload
typedef struct
{
  osal_event_hdr_t hdr;
  uint16 partnerAddr;
  uint8 server;
} zclKE_KeyGenMsg_t;

// Local zclReadCmd_t structure
typedef struct
{
  uint8  numAttr;
  uint16 attrID[1];
} zclKE_ReadCmd_t;

typedef struct 
{
  uint16 suite;
  uint8 ephDataGenTime;
  uint8 cfmKeyGenTime;
  uint8 *pIdentity;
} zclKE_InitiateCmd_t;

typedef struct 
{
  uint8 *pEphData;
} zclKE_EphDataCmd_t;

typedef struct 
{
  uint8 *pMAC;
} zclKE_CfmKeyDataCmd_t;

typedef struct
{
  uint8 status; // ZCL_KE_TERMINATE_ERROR
  uint8 waitTime;
  uint16 suites;
} zclKE_TerminateCmd_t;

typedef struct zclKE_ConnType zclKE_Conn_t;

struct zclKE_ConnType
{
  uint8 taskID;
  uint8 state; // see ZCL_KE_SERVER_CONN_STATE or ZCL_KE_CLIENT_CONN_STATE
  uint8 transSeqNum;
  uint8 rmtEphDataGenTime;
  uint8 rmtCfmKeyGenTime;
  uint16 suite;
  uint32 stamp;
  uint32 timeout;
  afAddrType_t partner;
  uint8 *pEPublicKey;
  uint8 *pEPrivateKey;
  uint8 *pRmtEPublicKey;
  uint8 *pRmtCert;
  uint8 *pKey;
  uint8 *pMACKey;
  zclKE_Conn_t *pNext;
};

typedef struct
{
  uint8 error;
  zclIncoming_t *pInMsg;
  zclKE_Conn_t *pConn;
} zclKE_ConnCtxt_t;


/**************************************************************************************************
 * FUNCTION PROTOTYPES
 */


/**************************************************************************************************
 * LOCAL VARIABLES
 */

static uint8 zclKE_TaskID;
static uint16 zclKE_SupportedSuites = 0;
static uint8 zclKE_State = ZCL_KE_INIT; // see ZCL_KE_STATE
static zclKE_Conn_t *zclKE_ServerConnList = NULL;
static zclKE_Conn_t *zclKE_ClientConnList = NULL;

static CONST cId_t zclKE_ClusterList[ZCL_KE_CLUSTER_CNT] =
{
  ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
};

static CONST SimpleDescriptionFormat_t zclKE_SimpleDesc =
{
  ZCL_KE_ENDPOINT,
  ZCL_SE_PROFILE_ID,
  ZCL_SE_DEVICEID_PHYSICAL,
  ZCL_KE_DEVICE_VERSION,
  ZCL_KE_FLAGS,
  ZCL_KE_CLUSTER_CNT,
  (cId_t *)zclKE_ClusterList, 
  ZCL_KE_CLUSTER_CNT,
  (cId_t *)zclKE_ClusterList 
};

static CONST endPointDesc_t zclKE_EPDesc =
{
  ZCL_KE_ENDPOINT,
  0,
#ifndef ZCL_STANDALONE
  &zcl_TaskID,
#else
  &zclKE_TaskID,
#endif
  (SimpleDescriptionFormat_t *)&zclKE_SimpleDesc,
  (afNetworkLatencyReq_t)noLatencyReqs
};

static CONST zclAttrRec_t zclKE_ZCL_AttrList[] =
{
  {
    ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
    {  // Attribute record
      ATTRID_KE_SUITE,
      ZCL_DATATYPE_ENUM16,
      ACCESS_CONTROL_READ,
      (void *)&zclKE_SupportedSuites
    }
  },
};

// Note: should not be const since ZCL may temporarily change the value
static zclOptionRec_t zclKE_ZCL_Options[] =
{
  {
    ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
    ( AF_ACK_REQUEST ),
  },
};

#if defined( NWK_AUTO_POLL )
uint32 zclKE_PollRateSaved;
uint8  zclKE_PollRateSet = 0;
#endif

#if !defined( ECCAPI_163_DISABLED )
// see ZCL_KE_GET_FIELD_TYPES
static CONST uint16 zclKE_Suite1Fields[ZCL_KE_FIELDS_MAX] = 
{
  ZCD_NV_CA_PUBLIC_KEY,         // ZCL_KE_PUBLIC_KEY_NV_ID 
  ZCD_NV_DEVICE_PRIVATE_KEY,    // ZCL_KE_PRIVATE_KEY_NV_ID
  ZCD_NV_IMPLICIT_CERTIFICATE,  // ZCL_KE_CERT_NV_ID       
  ECCAPI_PUBLIC_KEY_163_LEN,    // ZCL_KE_PUBLIC_KEY_LEN
  ECCAPI_PRIVATE_KEY_163_LEN,   // ZCL_KE_PRIVATE_KEY_LEN
  ECCAPI_CERT_163_LEN,          // ZCL_KE_CERT_LEN         
  ZCL_KE_CERT_163_EXT_ADDR_IDX, // ZCL_KE_CERT_EXT_ADDR_IDX
  ZCL_KE_CERT_163_ISSUER_IDX,   // ZCL_KE_CERT_ISSUER_IDX  
  // ZCL_KE_FIELDS_MAX         
};
#endif // ECCAPI_163_DISABLED

#if !defined( ECCAPI_283_DISABLED )
// see ZCL_KE_GET_FIELD_TYPES
static CONST uint16 zclKE_Suite2Fields[ZCL_KE_FIELDS_MAX] =
{
  ZCD_NV_PUBLIC_KEY_283,        // ZCL_KE_PUBLIC_KEY_NV_ID 
  ZCD_NV_PRIVATE_KEY_283,       // ZCL_KE_PRIVATE_KEY_NV_ID
  ZCD_NV_CERT_283,              // ZCL_KE_CERT_NV_ID       
  ECCAPI_PUBLIC_KEY_283_LEN,    // ZCL_KE_PUBLIC_KEY_LEN
  ECCAPI_PRIVATE_KEY_283_LEN,   // ZCL_KE_PRIVATE_KEY_LEN
  ECCAPI_CERT_283_LEN,          // ZCL_KE_CERT_LEN         
  ZCL_KE_CERT_283_EXT_ADDR_IDX, // ZCL_KE_CERT_EXT_ADDR_IDX
  ZCL_KE_CERT_283_ISSUER_IDX,   // ZCL_KE_CERT_ISSUER_IDX  
  // ZCL_KE_FIELDS_MAX         
};
#endif // ECCAPI_283_DISABLED


/**************************************************************************************************
 * LOCAL FUNCTIONS
 */

#if defined( NWK_AUTO_POLL )
/**************************************************************************************************
 * @fn      zclKE_SetPollRate
 *
 * @brief   Set the network poll rate for key establishment.
 *
 * @param   user - ZCL_KE_SERVER_POLL_RATE_BIT or ZCL_KE_CLIENT_POLL_RATE_BIT
 *
 * @return  void
 */
static void zclKE_SetPollRate( uint8 user )
{
  if ( !zclKE_PollRateSet )
  {
    // Save and set the current poll rate
    zclKE_PollRateSaved = zgPollRate;
    NLME_SetPollRate( ZCL_KE_POLL_RATE );
  }

  zclKE_PollRateSet |= user;
}

/**************************************************************************************************
 * @fn      zclKE_SetPollRate
 *
 * @brief   Set the network poll rate for key establishment.
 *
 * @param   user - ZCL_KE_SERVER_POLL_RATE_BIT or ZCL_KE_CLIENT_POLL_RATE_BIT
 *
 * @return  void
 */
static void zclKE_RestorePollRate( uint8 user )
{
  zclKE_PollRateSet &= ~user;

  if ( !zclKE_PollRateSet )
  {
    // Restore poll rate
    NLME_SetPollRate( zclKE_PollRateSaved );
  }
}
#endif // NWK_AUTO_POLL

/**************************************************************************************************
 * @fn      zclKE_MemFree
 *
 * @brief   Release(and clear) allocated memory.
 *
 * @param   pv - pointer to be freed
 * @param   len - length of pointer memory allocated
 *
 * @return  void
 */
static void zclKE_MemFree( void *pv, uint16 len )
{
  if ( pv )
  {
    osal_memset( pv, 0, len );
    osal_mem_free( pv );
  }  
}

/**************************************************************************************************
 * @fn      zclKE_GetField
 *
 * @brief   Lookup the value of the field(ZCL_KE_GET_FIELD_TYPES) for the selected suite.
 *
 * @param   suite - selected suite
 * @param   field - see ZCL_KE_GET_FIELD_TYPES
 *
 * @return  uint16 - value of requested field
 */
static uint16 zclKE_GetField( uint16 suite, uint8 field )
{
  uint16 result;
 
  switch ( suite )
  {
#if !defined( ECCAPI_163_DISABLED )
    case ZCL_KE_SUITE_1:
      result = zclKE_Suite1Fields[field];
      break;
#endif

#if !defined( ECCAPI_283_DISABLED )
    case ZCL_KE_SUITE_2:
      result = zclKE_Suite2Fields[field];
      break;
#endif
      
    default:
      // Should never get here
      result = 0;      
      break;
  }

  return result;
}  

/**************************************************************************************************
 * @fn      zclKE_NotifyStatus
 *
 * @brief   Notify key establishment initiator(zclKE_Start or zclKE_StartDirect) of final status.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   partnerNwkAddr - partner network address  
 * @param   notifyStatus - see ZCL_KE_NOTIFY_STATUS
 * @param   pCmd - terminate command payload
 *
 * @return  void
 */
static void zclKE_NotifyStatus( uint8 taskID, uint16 partnerNwkAddr, 
                                uint8 notifyStatus, zclKE_TerminateCmd_t *pCmd )
{
  zclKE_StatusInd_t *pInd;

  // Allocate the indication
  pInd = (zclKE_StatusInd_t *)osal_msg_allocate( sizeof( zclKE_StatusInd_t ) );

  if ( pInd )
  {
    pInd->hdr.event = ZCL_KEY_ESTABLISH_IND;
    pInd->hdr.status = notifyStatus;
    pInd->partnerNwkAddr = partnerNwkAddr;

    if ( pCmd )
    {
      pInd->terminateError = pCmd->status ;
      pInd->suites = pCmd->suites;
      pInd->waitTime = pCmd->waitTime;
    }
    else
    {
      pInd->terminateError = ZCL_KE_TERMINATE_ERROR_NONE;
      pInd->suites = 0;
      pInd->waitTime = 0;
    }

    osal_msg_send( taskID, (uint8 *)pInd );
  }
}

/**************************************************************************************************
 * @fn      zclKE_StartTimer
 *
 * @brief   Start the key establishment timer.
 *
 * @param   timeout - requested timeout in milliseconds
 *
 * @return  void
 */
static void zclKE_StartTimer( uint32 timeout )
{
  if ( timeout )
  {
    // Check if timer needs to be started or adjusted to a smaller timeout 
    uint32 next = osal_get_timeoutEx( zclKE_TaskID, ZCL_KE_TIMER_EVT );

    if ( ( !next ) || ( timeout < next ) )
    {
      osal_start_timerEx( zclKE_TaskID, ZCL_KE_TIMER_EVT, timeout );
    }
  }
}

/**************************************************************************************************
 * @fn      zclKE_ConnRelease
 *
 * @brief   Release connection and associated memory allocations.
 *
 * @param   pConn - server or client connection
 *
 * @return  void
 */
static void zclKE_ConnRelease( zclKE_Conn_t *pConn )
{
  zclKE_MemFree( pConn->pEPublicKey, zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN ) );
  zclKE_MemFree( pConn->pEPrivateKey,zclKE_GetField( pConn->suite, ZCL_KE_PRIVATE_KEY_LEN ) );
  zclKE_MemFree( pConn->pRmtEPublicKey, zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN ) );
  zclKE_MemFree( pConn->pRmtCert, zclKE_GetField( pConn->suite, ZCL_KE_CERT_LEN ) );
  zclKE_MemFree( pConn->pKey, ZCL_KE_KEY_LEN );
  zclKE_MemFree( pConn->pMACKey, ZCL_KE_KEY_LEN );
  osal_mem_free( pConn );
}

/**************************************************************************************************
 * @fn      zclKE_ConnSetTimeout
 *
 * @brief   Set connection timeout.
 *
 * @param   pConn - server or client connection
 * @param   timeout - requested timeout in milliseconds
 *
 * @return  void
 */
static void zclKE_ConnSetTimeout( zclKE_Conn_t *pConn, uint32 timeout )
{
  pConn->stamp  = osal_GetSystemClock();
  pConn->timeout = timeout;
  zclKE_StartTimer( timeout );  
}

/**************************************************************************************************
 * @fn      zclKE_GetRandom
 *
 * @brief   Callback function that generates random seeds of the specified length.  This function
 *          should copy "size" bytes of random data into "pBuf".
 *
 * @param   pBuf - This is an unsigned char array of size at least "size" to hold the random data.
 * @param   size - The number of bytes of random data to compute and store.
 *
 * @return  int - MCE_SUCCESS.
 */
static int zclKE_GetRandom(uint8 *pBuf, uint32 size)
{
  // Input to SSP_GetTrueRandAES assumes size <= SEC_KEY_LEN
  // Therefore, call SSP_GetTrueRandAES multiple times to
  // fill out the buffer.
  while( size > SEC_KEY_LEN )
  {
    SSP_GetTrueRandAES( SEC_KEY_LEN, pBuf );
    size -= SEC_KEY_LEN;
    pBuf += SEC_KEY_LEN;
  }

  SSP_GetTrueRandAES( (uint8)size, pBuf );

  return MCE_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_HashFunc
 *
 * @brief   This function should compute the hash of the "pData" parameter of size  "dataLen", and
 *          store the result in the "pDigest" buffer parameter.
 *
 * @param   pDigest - output buffer(length must be 16)
 * @param   dataLen - length in bytes of the message to be hashed.
 * @param   pData - input buffer of data to be hashed
 *
 * @return  int - MCE_SUCCESS.
 */
static int zclKE_HashFunc( unsigned char *pDigest, 
                           unsigned long dataLen, 
                           unsigned char *pData )
{
  // Convert to bit length
  dataLen *= 8;
  sspMMOHash( NULL, 0, pData, ( uint16 )dataLen, pDigest );
  return MCE_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_KeyDeriveFunction
 *
 * @brief   Key Derive Function (ANSI X9.63).
 *          Note this is not a generalized KDF. It only applies to the KDF
 *          specified in ZigBee SE profile. Only the first two hashed keys
 *          are calculated and concatenated.
 *
 * @param   pZData - input shared secret
 * @param   zDataLen - input shared secret length(ZCL_KE_PRIVATE_KEY_LEN)
 * @param   pKeyData - output buffer ( 16*2 bytes)
 *
 * @return  void
 */
static void zclKE_KeyDeriveFunction( uint8 *pZData, uint16 zDataLen, uint8 *pKeyData )
{
  uint8 hashCounter[4] = {0x00, 0x00, 0x00, 0x01};
  uint8 hashedData[ECCAPI_PRIVATE_KEY_283_LEN + 4]; // Use max ZCL_KE_PRIVATE_KEY_LEN
  uint16 bitLen;

  bitLen = ( zDataLen + 4 ) * 8;

  // Calculate K1: Ki = Hash(Z || Counter1 )
  osal_memcpy( hashedData, pZData, zDataLen );
  osal_memcpy( &(hashedData[zDataLen]), hashCounter, 4);

  sspMMOHash(NULL, 0, hashedData, bitLen, pKeyData);

  // Indrement the counter
  hashedData[zDataLen + 3] = 0x02;

  sspMMOHash(NULL, 0, hashedData, bitLen, &(pKeyData[ZCL_KE_KEY_LEN]));
}

/**************************************************************************************************
 * @fn      zclKE_GenEphKeys
 *
 * @brief   Generate ephemeral keys.
 *
 * @param   pCtxt - connection context
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_GenEphKeys( zclKE_ConnCtxt_t *pCtxt )
{
  uint8 result;
  uint16 len;
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Allocate ephemeral public key 
  len = zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN );

  pConn->pEPublicKey = zcl_mem_alloc( len );

  if ( !pCtxt->pConn->pEPublicKey )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return FALSE;
  }

  // Allocate ephemeral private key 
  len = zclKE_GetField( pConn->suite, ZCL_KE_PRIVATE_KEY_LEN );

  pConn->pEPrivateKey = zcl_mem_alloc( len );

  if ( !pConn->pEPrivateKey )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return FALSE;
  }

  // Generate the ephemeral keys
  switch ( pConn->suite )
  {
#if !defined( ECCAPI_163_DISABLED )
    case ZCL_KE_SUITE_1:
      result = ZSE_ECCGenerateKey( pConn->pEPrivateKey,
                                   pConn->pEPublicKey,
                                   zclKE_GetRandom,
                                   NULL, 0);
      break;
#endif // !defined( ECCAPI_163_DISABLED )

#if !defined( ECCAPI_283_DISABLED )
    case ZCL_KE_SUITE_2:
      result = ZSE_ECCGenerateKey283( pConn->pEPrivateKey,
                                      pConn->pEPublicKey,
                                      zclKE_GetRandom,
                                      NULL, 0);
      break;
#endif // !defined( ECCAPI_283_DISABLED )

    default:
      // Should never get here
      result = MCE_ERR_BAD_INPUT;
      break;
  }

  if ( result != MCE_SUCCESS )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return FALSE;
  }

  return TRUE;
}

/**************************************************************************************************
 * @fn      zclKE_GenKeyBits
 *
 * @brief   Generate key bits.
 *
 * @param   pCtxt - connection context
 * @param   pKeyBits - key bits buffer(ZCL_KE_PRIVATE_KEY_LEN)
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_GenKeyBits( zclKE_ConnCtxt_t *pCtxt, uint8 *pKeyBits )
{
  uint8 success;
  uint8 result;
  uint8 rxOnIdleSaved;
  uint8 rxOnIdleSet;
  uint8 *pPublicKey = NULL;
  uint8 *pPrivateKey = NULL;
  uint16 publicKeyLen;
  uint16 privateKeyLen;
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Assume success
  success = TRUE;

  publicKeyLen = zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN );
  privateKeyLen = zclKE_GetField( pConn->suite, ZCL_KE_PRIVATE_KEY_LEN );
  
  do 
  {
    pPublicKey = osal_mem_alloc( publicKeyLen );
    if ( !pPublicKey )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    pPrivateKey = osal_mem_alloc( privateKeyLen );
    if ( !pPrivateKey )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    osal_nv_read( zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_NV_ID ), 0,
                  publicKeyLen, pPublicKey );

    osal_nv_read( zclKE_GetField( pConn->suite, ZCL_KE_PRIVATE_KEY_NV_ID ), 0,
                  privateKeyLen, pPrivateKey );

    // Turn off the radio before the key bit generation, in order to avoid
    // incoming message accumulation by interrupts during the long process time.
    rxOnIdleSet = FALSE;
    ZMacGetReq( ZMacRxOnIdle, &rxOnIdleSaved );
    ZMacSetReq( ZMacRxOnIdle, &rxOnIdleSet );

    // DO NOT BREAK -- until radio state is restored

    // Generate the ephemeral keys
    switch ( pConn->suite )
    {
#if !defined( ECCAPI_163_DISABLED )
      case ZCL_KE_SUITE_1:
        result = ZSE_ECCKeyBitGenerate( pPrivateKey, pConn->pEPrivateKey,
                                        pConn->pEPublicKey, pConn->pRmtCert,
                                        pConn->pRmtEPublicKey, pPublicKey,
                                        pKeyBits, zclKE_HashFunc, NULL, 0 );
        break;
#endif // !defined( ECCAPI_163_DISABLED )

#if !defined( ECCAPI_283_DISABLED )
      case ZCL_KE_SUITE_2:
        result = ZSE_ECCKeyBitGenerate283( pPrivateKey, pConn->pEPrivateKey,
                                           pConn->pEPublicKey, pConn->pRmtCert,
                                           pConn->pRmtEPublicKey, pPublicKey,
                                           pKeyBits, zclKE_HashFunc, NULL, 0 );
        break;
#endif // !defined( ECCAPI_283_DISABLED )

      default:
        // Should never get here
        result = MCE_ERR_BAD_INPUT;
        break;
    }

    if( result != MCE_SUCCESS )
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_KEY_CONFIRM;
      success = FALSE;
      // DO NOT BREAK -- until radio state is restored
    }

    // Restore radio state
    ZMacSetReq( ZMacRxOnIdle, &rxOnIdleSaved );

  } while ( 0 );

  // Cleanup local memory allocations
  zclKE_MemFree( pPublicKey, publicKeyLen );
  zclKE_MemFree( pPrivateKey, privateKeyLen );

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_GenKeys
 *
 * @brief   Generate keys.
 *
 * @param   pCtxt - connection context
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_GenKeys( zclKE_ConnCtxt_t *pCtxt )
{
  uint8 success;
  uint8 *pKeyBits = NULL;
  uint8 *pKeyData = NULL;
  uint16 privateKeyLen;
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Assume success
  success = TRUE;

  privateKeyLen = zclKE_GetField( pConn->suite, ZCL_KE_PRIVATE_KEY_LEN );

  do
  {
    // Allocate buffer for "zclKE_GenKeyBits"
    pKeyBits = osal_mem_alloc( privateKeyLen );
    if ( !pKeyBits )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    // Generate key bitstream
    if ( !zclKE_GenKeyBits( pCtxt, pKeyBits ) )
    {
      // pCtxt->error set in "zclKE_GenKeyBits"
      success = FALSE;
      break;
    }

    // Release and clear any memory blocks that are not needed
    zclKE_MemFree( pConn->pRmtCert, zclKE_GetField( pConn->suite, ZCL_KE_CERT_LEN ) ); 
    zclKE_MemFree( pConn->pEPrivateKey, privateKeyLen ); 
    pConn->pRmtCert = NULL;
    pConn->pEPrivateKey = NULL;

    // Allocate buffer for key data: KDF(KEY_BITS) = MAC_KEY_DATA || KEY_DATA
    pKeyData = osal_mem_alloc( 2 * ZCL_KE_KEY_LEN );
    if ( !pKeyData )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    pConn->pMACKey = osal_mem_alloc( ZCL_KE_KEY_LEN );
    if ( !pConn->pMACKey )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    pConn->pKey = osal_mem_alloc( ZCL_KE_KEY_LEN );
    if ( !pConn->pKey )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    // Derive the keying data using KDF function
    zclKE_KeyDeriveFunction( pKeyBits, privateKeyLen, pKeyData );

    // Save the derived keys
    osal_memcpy( pConn->pMACKey, pKeyData, ZCL_KE_KEY_LEN );
    osal_memcpy( pConn->pKey, &(pKeyData[ZCL_KE_KEY_LEN]), ZCL_KE_KEY_LEN );

  } while ( 0 );

  // Cleanup local memory allocations
  zclKE_MemFree( pKeyBits, privateKeyLen );
  zclKE_MemFree( pKeyData, 2 * ZCL_KE_KEY_LEN );

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_GenKeys
 *
 * @brief   Generate MAC value.
 *
 * @param   pConn - server or client connection
 * @param   initiator - key establishment initiator(TRUE) or responder(FALSE)
 * @param   MACu - MACu(TRUE) or MACv(FALSE)
 * @param   pMAC - MAC buffer
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_GenMAC( zclKE_Conn_t *pConn, uint8 initiator, uint8 MACu, uint8* pMAC )
{
  uint8 *hashBuf = NULL;
  uint16 bufLen;
  uint16 keyLen;
  uint8 rmtExtAddr[Z_EXTADDR_LEN];

  // Lookup remote extended address
  if ( !AddrMgrExtAddrLookup( pConn->partner.addr.shortAddr, rmtExtAddr ) )
  {
    return FALSE;
  }

  // Lookup key length
  keyLen = zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN );

  // MAC(U) = MAC(MacKey) { M(U) || ID(U) || ID(V) || E(U) || E(V) }
  bufLen = 1 + (Z_EXTADDR_LEN * 2) + (keyLen * 2);
  hashBuf = osal_mem_alloc( bufLen );
  if( !hashBuf )
  {
    return FALSE;
  }

  // Convert length to bit length
  bufLen = bufLen * 8;

  // Assumption for M(U) and M(V) is: M(U) = 0x02, M(V) = 0x03
  if( MACu == TRUE )
  {
    hashBuf[0] = 0x02; // M(U)
  }
  else
  {
    hashBuf[0] = 0x03; // M(V)
  }


  if ( ( initiator && MACu ) || ( !initiator && !MACu ) )
  {
    // MAC = MAC(MacKey) { M() || ID(L) || ID(R) || E(L) || E(R) }
    // L - Local, R - Remote
    // ID(L)
    SSP_MemCpyReverse( &hashBuf[1], NLME_GetExtAddr(), Z_EXTADDR_LEN);
    // ID(R)
    SSP_MemCpyReverse( &hashBuf[1 + Z_EXTADDR_LEN], rmtExtAddr, Z_EXTADDR_LEN );
    // E(L)
    osal_memcpy( &hashBuf[1 + (2 * Z_EXTADDR_LEN)], pConn->pEPublicKey, keyLen );
    // E(R)
    osal_memcpy( &hashBuf[1 + (2 * Z_EXTADDR_LEN) + keyLen], pConn->pRmtEPublicKey, keyLen );
  }
  else
  {
    // MAC = MAC(MacKey) { M() || ID(R) || ID(L) || E(R) || E(L) }
    // L - Local, R - Remote
    // ID(R)
    SSP_MemCpyReverse( &hashBuf[1], rmtExtAddr, Z_EXTADDR_LEN);
    // ID(L)
    SSP_MemCpyReverse( &hashBuf[1 + Z_EXTADDR_LEN], NLME_GetExtAddr(), Z_EXTADDR_LEN );
    // E(R)
    osal_memcpy( &hashBuf[1 + (2 * Z_EXTADDR_LEN)], pConn->pRmtEPublicKey, keyLen );
    // E(L)
    osal_memcpy( &hashBuf[1 + (2 * Z_EXTADDR_LEN) + keyLen], pConn->pEPublicKey, keyLen );
  }

  // Hash MAC
  SSP_KeyedHash( hashBuf, bufLen, pConn->pMACKey, pMAC );

  osal_mem_free( hashBuf );

  return TRUE;
}

/**************************************************************************************************
 * @fn      zclKE_CheckForAvailSuites
 *
 * @brief   Check to see which suites are available.
 *
 * @param   none
 *
 * @return  void
 */
static void zclKE_CheckForAvailSuites( void )
{
  uint8 certData[ECCAPI_PUBLIC_KEY_283_LEN];
  uint8 nullData[ECCAPI_PUBLIC_KEY_283_LEN] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };

#if !defined( ECCAPI_163_DISABLED )
  if ( ( SUCCESS == osal_nv_read( ZCD_NV_CA_PUBLIC_KEY, 0, 
                                  ECCAPI_PUBLIC_KEY_163_LEN, certData ) ) && 
       ( !osal_memcmp( certData, nullData, ECCAPI_PUBLIC_KEY_163_LEN )  )    )
  {
    zclKE_SupportedSuites |= ZCL_KE_SUITE_1;
  }
#endif // ECCAPI_163_DISABLED


#if !defined( ECCAPI_283_DISABLED )
  if ( ( SUCCESS == osal_nv_read( ZCD_NV_PUBLIC_KEY_283, 0, 
                                  ECCAPI_PUBLIC_KEY_283_LEN, certData ) ) && 
       ( !osal_memcmp( certData, nullData, ECCAPI_PUBLIC_KEY_283_LEN )  )    )
  {
    zclKE_SupportedSuites |= ZCL_KE_SUITE_2;
  }
#endif // ECCAPI_283_DISABLED
}

/**************************************************************************************************
 * @fn      zclKE_CheckMultSuiteBits
 *
 * @brief   Check suite field for multiple bits.
 *
 * @param   suite - suite field 
 *
 * @return  uint8 - TRUE multiple bits, FALSE if not
 */
static uint8 zclKE_CheckMultSuiteBits( uint16 suite )
{
  uint8 success = FALSE;
  uint8 bit;
  uint16 bitCount = 0;

  for ( bit = 0; bit < 16; bit++ )
  {
    if ( suite & ( 1 << bit ) )
    {
      bitCount++;
      if ( bitCount > 1 )
      {
        // Multiple bits detected
        success = TRUE;

        break;
      }
    }
    else if ( suite < ( 1 << bit ) )
    {
      // No more bits
      break;
    }
  }

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckSuite
 *
 * @brief   Check initiate command - suite.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckSuite( zclKE_ConnCtxt_t *pCtxt,
                                          zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;

  // Check for legal suites
  if ( ( pCmd->suite == ZCL_KE_SUITE_1 ) ||
       ( pCmd->suite == ZCL_KE_SUITE_2 )    )
  {
    // Is the selected suite supported
    if ( pCmd->suite & zclKE_SupportedSuites )
    {
      success = TRUE;
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_UNSUPPORTED_SUITE;

      success = FALSE;
    }
  }
  else
  {
    // Already failed but must check for multiple bits 
    if ( zclKE_CheckMultSuiteBits( pCmd->suite ) )
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_UNSUPPORTED_SUITE;
    }

    success = FALSE;
  }  

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckGenTimes
 *
 * @brief   Check initiate command - key generation times.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckGenTimes( zclKE_ConnCtxt_t *pCtxt,
                                             zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;

  if ( ( pCmd->ephDataGenTime < ZCL_KE_GEN_INVALID_TIME ) &&
       ( pCmd->cfmKeyGenTime < ZCL_KE_GEN_INVALID_TIME  )    )
  {
    success = TRUE;
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;

    success = FALSE;
  }

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckCertExtAddr
 *
 * @brief   Check initiate command - cert extended address.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckCertExtAddr( zclKE_ConnCtxt_t *pCtxt,
                                                zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;
  uint16 idx;
  uint8 extAddr[Z_EXTADDR_LEN];
  uint8 revExtAddr[Z_EXTADDR_LEN];

  // Get extended address index
  idx = zclKE_GetField( pCmd->suite, ZCL_KE_CERT_EXT_ADDR_IDX );

  // Lookup the extended address, reverse and compare with cert
  if ( ( AddrMgrExtAddrLookup( pCtxt->pInMsg->msg->srcAddr.addr.shortAddr,
                               extAddr )                                   ) &&
       ( osal_revmemcpy( revExtAddr, extAddr, Z_EXTADDR_LEN )              ) &&
       ( osal_memcmp( &pCmd->pIdentity[idx], revExtAddr, Z_EXTADDR_LEN )   )    )
  {
    success = TRUE;
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;

    success = FALSE;
  }
  
  return success;
}

/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckCertIssuer
 *
 * @brief   Check initiate command - cert issuer.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckCertIssuer( zclKE_ConnCtxt_t *pCtxt,
                                               zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;
  uint16 id = zclKE_GetField( pCmd->suite, ZCL_KE_CERT_NV_ID );
  uint16 len = zclKE_GetField( pCmd->suite, ZCL_KE_CERT_LEN );
  uint16 idx = zclKE_GetField( pCmd->suite, ZCL_KE_CERT_ISSUER_IDX );
  uint8 *pCert  = NULL;

  success = TRUE;

  do
  {
    uint8 *pCert  = (uint8 *)osal_mem_alloc( len );
    if ( !pCert )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      success = FALSE;
      break;
    }

    osal_nv_read( id, 0, len, pCert );

    if ( !osal_memcmp( &pCmd->pIdentity[idx], &pCert[idx], ZCL_KE_CERT_ISSUER_LEN ) )
    {
      pCtxt->error = ZCL_KE_TERMINATE_UNKNOWN_ISSUER;
      success = FALSE;
      break;
    }
  }
  while ( 0 );

  // Cleanup local memory allocations
  zclKE_MemFree( pCert, len );

  return success;
}

#if !defined( ECCAPI_283_DISABLED )
/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckCertSuite2
 *
 * @brief   Check initiate command - suite2 cert fields.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckCertSuite2( zclKE_ConnCtxt_t *pCtxt,
                                               zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;

  if ( ( pCmd->pIdentity[ZCL_KE_CERT_283_TYPE_IDX] ==
         ZCL_KE_CERT_283_TYPE_VALUE                       ) &&
       ( pCmd->pIdentity[ZCL_KE_CERT_283_CURVE_IDX] ==
         ZCL_KE_CERT_283_CURVE_VALUE                      ) &&
       ( pCmd->pIdentity[ZCL_KE_CERT_283_HASH_IDX] ==
         ZCL_KE_CERT_283_HASH_VALUE                       ) &&
       ( pCmd->pIdentity[ZCL_KE_CERT_283_KEY_USAGE_IDX] &
         ZCL_KE_CERT_283_KEY_USAGE_BIT                    )    )
  {
    success = TRUE;
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_INVALID_CERTIFICATE;

    success = FALSE;
  }

  return success;
}
#endif // !defined( ECCAPI_283_DISABLED )

/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckCertSpecific
 *
 * @brief   Check initiate command - suite specific cert fields.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckCertSpecific( zclKE_ConnCtxt_t *pCtxt,
                                                 zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;

  switch ( pCmd->suite )
  {
    case ZCL_KE_SUITE_1:
      success = TRUE;
      break;

#if !defined( ECCAPI_283_DISABLED )
    case ZCL_KE_SUITE_2:
      success = zclKE_InitiateCmdCheckCertSuite2( pCtxt, pCmd );
      break;
#endif

    default:
      // Should never get here
      success = TRUE;
      break;
  }
 
  return success;
}

/**************************************************************************************************
 * @fn      zclKE_InitiateCmdCheckFields
 *
 * @brief   Check fields of ZCL_KE_INITIATE_REQ and ZCL_KE_INITIATE_RSP payloads.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateCmdCheckFields( zclKE_ConnCtxt_t *pCtxt,
                                           zclKE_InitiateCmd_t *pCmd )
{
  uint8 success;

  if ( zclKE_InitiateCmdCheckSuite( pCtxt, pCmd ) &&
       zclKE_InitiateCmdCheckGenTimes( pCtxt, pCmd ) && 
       zclKE_InitiateCmdCheckCertExtAddr( pCtxt, pCmd ) &&
       zclKE_InitiateCmdCheckCertIssuer( pCtxt, pCmd ) &&
       zclKE_InitiateCmdCheckCertSpecific( pCtxt, pCmd )  )
  {
    success = TRUE;
  }
  else
  {
    success = FALSE;
  }

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_InitiateRspCheckFields
 *
 * @brief   Check fields of ZCL_KE_INITIATE_RSP payload.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_InitiateRspCheckFields( zclKE_ConnCtxt_t *pCtxt,
                                           zclKE_InitiateCmd_t *pCmd )
{
  uint8 success = FALSE;

  // First use the base initiate cmd check -- this should be called first so 
  // expected errors will be returned during specific tests
  if ( zclKE_InitiateCmdCheckFields( (zclKE_ConnCtxt_t *)pCtxt, pCmd ) )
  {
    // Is this the suite that was requested
    if ( pCtxt->pConn->suite == pCmd->suite )
    {
      success = TRUE;
    }    
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_UNSUPPORTED_SUITE;
    }
  }

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_ParseInitiateCmd
 *
 * @brief   Parse command(ZCL_KE_INITIATE_REQ or ZCL_KE_INITIATE_RSP).
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ParseInitiateCmd( zclIncoming_t *pInMsg, 
                                         zclKE_InitiateCmd_t *pCmd )
{
  uint8 *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_KE_INITIATE_HDR_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->suite = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;
  pCmd->ephDataGenTime = *pBuf++;
  pCmd->cfmKeyGenTime = *pBuf++;
  pCmd->pIdentity = pBuf;

  // Check for valid suite and identity length
  if ( pCmd->suite == ZCL_KE_SUITE_1 )
  {
    if ( pInMsg->pDataLen < ZCL_KE_INITIATE_SUITE1_LEN )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }
  }
  else if ( pCmd->suite == ZCL_KE_SUITE_2 )
  {
    if ( pInMsg->pDataLen < ZCL_KE_INITIATE_SUITE2_LEN )
    {
      return ZCL_STATUS_MALFORMED_COMMAND;
    }
  }
  else
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }
  
  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_ParseEphDataCmd
 *
 * @brief   Parse command(ZCL_KE_EPH_DATA_REQ or ZCL_KE_EPH_DATA_RSP).
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ParseEphDataCmd( uint16 suite,
                                        zclIncoming_t *pInMsg, 
                                        zclKE_EphDataCmd_t *pCmd )
{
  // Check for minimum packet length
  if ( pInMsg->pDataLen < zclKE_GetField( suite, ZCL_KE_PUBLIC_KEY_LEN ) )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->pEphData = pInMsg->pData;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_ParseCfmKeyDataCmd
 *
 * @brief   Parse command(ZCL_KE_CFM_KEY_DATA_REQ or ZCL_KE_CFM_KEY_DATA_RSP).
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ParseCfmKeyDataCmd( zclIncoming_t *pInMsg, 
                                           zclKE_CfmKeyDataCmd_t *pCmd )
{
  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_KE_MAC_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->pMAC = pInMsg->pData;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_ParseTerminateCmd
 *
 * @brief   Parse command(ZCL_KE_TERMINATE_SERVER or ZCL_KE_TERMINATE_CLIENT).
 *
 * @param   pInMsg - incoming message to process
 * @param   pCmd - command payload
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ParseTerminateCmd( zclIncoming_t *pInMsg, 
                                          zclKE_TerminateCmd_t *pCmd )
{
  uint8 *pBuf = pInMsg->pData;

  // Check for minimum packet length
  if ( pInMsg->pDataLen < ZCL_KE_TERMINATE_LEN )
  {
    return ZCL_STATUS_MALFORMED_COMMAND;
  }

  pCmd->status = *pBuf++;
  pCmd->waitTime = *pBuf++;
  pCmd->suites = BUILD_UINT16( pBuf[0], pBuf[1] );
  pBuf += 2;

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_SendInitiateCmd
 *
 * @brief   Send an initiate command.
 *
 * @param   pConn - client connection
 * @param   cmdID - ZCL_KE_INITIATE_REQ or ZCL_KE_INITIATE_RSP
 * @param   direction - ZCL_FRAME_SERVER_CLIENT_DIR or ZCL_FRAME_CLIENT_SERVER_DIR
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendInitiateCmd( zclKE_Conn_t *pConn, uint8 cmdID, uint8 direction )
{
  ZStatus_t status;
  uint8 *pCmdBuf;
  uint16 cmdBufLen;
  uint8 *pBuf;
  uint16 certLen;

  // Get the cert length based on suite
  certLen = zclKE_GetField( pConn->suite, ZCL_KE_CERT_LEN );

  // Allocate command buffer
  cmdBufLen = ZCL_KE_INITIATE_HDR_LEN + certLen;

  pCmdBuf = osal_mem_alloc( cmdBufLen );
  if ( pCmdBuf == NULL )
  {
    return ZMemError;
  }

  pBuf = pCmdBuf;
  *pBuf++ = LO_UINT16( pConn->suite );
  *pBuf++ = HI_UINT16( pConn->suite );
  *pBuf++ = ZCL_KE_SERVER_EPH_DATA_GEN_TIME;
  *pBuf++ = ZCL_KE_SERVER_CFM_KEY_GEN_TIME;

  // Get the certificate based on suite
  status = osal_nv_read( zclKE_GetField( pConn->suite, ZCL_KE_CERT_NV_ID ), 
                         0, certLen, pBuf );
  pBuf = pBuf + certLen;

  if ( status == SUCCESS )
  {
    status = zcl_SendCommand( ZCL_KE_ENDPOINT, &pConn->partner,
                              ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
                              cmdID, TRUE, direction,
                              TRUE, 0, pConn->transSeqNum, cmdBufLen, pCmdBuf );
  }

  osal_mem_free( pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_SendInitiateReq
 *
 * @brief   Send ZCL_KE_INITIATE_REQ.
 *
 * @param   pConn - client connection
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendInitiateReq( zclKE_Conn_t *pConn )
{
  return zclKE_SendInitiateCmd( pConn, ZCL_KE_INITIATE_REQ, ZCL_FRAME_CLIENT_SERVER_DIR );
}

/**************************************************************************************************
 * @fn      zclKE_SendInitiateRsp
 *
 * @brief   Send ZCL_KE_INITIATE_RSP.
 *
 * @param   pConn - client connection
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendInitiateRsp( zclKE_Conn_t *pConn )
{
  return zclKE_SendInitiateCmd( pConn, ZCL_KE_INITIATE_RSP, ZCL_FRAME_SERVER_CLIENT_DIR );
}

/**************************************************************************************************
 * @fn      zclKE_SendEphDataCmd
 *
 * @brief   Send an ephemeral data command.
 *
 * @param   pConn - client connection
 * @param   cmdID - ZCL_KE_EPH_DATA_REQ or ZCL_KE_EPH_DATA_RSP
 * @param   direction - ZCL_FRAME_SERVER_CLIENT_DIR or ZCL_FRAME_CLIENT_SERVER_DIR
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendEphDataCmd( zclKE_Conn_t *pConn, uint8 cmdID, uint8 direction )
{
  ZStatus_t status;
  uint8 *pCmdBuf;
  uint16 cmdBufLen;

  // Set command buffer fields
  cmdBufLen = zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN );
  pCmdBuf = pConn->pEPublicKey;

  status = zcl_SendCommand( ZCL_KE_ENDPOINT, &pConn->partner,
                            ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
                            cmdID, TRUE, direction,
                            TRUE, 0, pConn->transSeqNum, cmdBufLen, pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_SendEphDatReq
 *
 * @brief   Send ZCL_KE_EPH_DATA_REQ.
 *
 * @param   pConn - client connection
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendEphDatReq( zclKE_Conn_t *pConn )
{
  return zclKE_SendEphDataCmd( pConn, ZCL_KE_EPH_DATA_REQ, ZCL_FRAME_CLIENT_SERVER_DIR );
}

/**************************************************************************************************
 * @fn      zclKE_SendEphDataRsp
 *
 * @brief   Send ZCL_KE_EPH_DATA_RSP.
 *
 * @param   pConn - client connection
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendEphDataRsp( zclKE_Conn_t *pConn )
{
  return zclKE_SendEphDataCmd( pConn, ZCL_KE_EPH_DATA_RSP, ZCL_FRAME_SERVER_CLIENT_DIR );
}

/**************************************************************************************************
 * @fn      zclKE_SendCfmKeyDataCmd
 *
 * @brief   Send an confirm key data command.
 *
 * @param   pConn - client connection
 * @param   cmdID - ZCL_KE_CFM_KEY_DATA_REQ or ZCL_KE_CFM_KEY_DATA_RSP
 * @param   direction - ZCL_FRAME_SERVER_CLIENT_DIR or ZCL_FRAME_CLIENT_SERVER_DIR
 * @param   pMAC - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendCfmKeyDataCmd( zclKE_Conn_t *pConn, uint8 cmdID, 
                                          uint8 direction, uint8 *pMAC )
{
  ZStatus_t status;
  uint8 *pCmdBuf;
  uint16 cmdBufLen;

  // Set command buffer fields
  cmdBufLen = ZCL_KE_MAC_LEN;
  pCmdBuf = pMAC;

  status = zcl_SendCommand( ZCL_KE_ENDPOINT, &pConn->partner,
                            ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
                            cmdID, TRUE, direction,
                            TRUE, 0, pConn->transSeqNum, cmdBufLen, pCmdBuf );

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_SendCfmKeyDataReq
 *
 * @brief   Send ZCL_KE_CFM_KEY_DATA_REQ.
 *
 * @param   pConn - client connection
 * @param   pMAC - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendCfmKeyDataReq( zclKE_Conn_t *pConn, uint8 *pMAC )
{
  return zclKE_SendCfmKeyDataCmd( pConn, ZCL_KE_CFM_KEY_DATA_REQ,
                                  ZCL_FRAME_CLIENT_SERVER_DIR, pMAC );
}

/**************************************************************************************************
 * @fn      zclKE_SendCfmKeyDataRsp
 *
 * @brief   Send ZCL_KE_CFM_KEY_DATA_RSP.
 *
 * @param   pConn - client connection
 * @param   pMAC - command payload
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclKE_SendCfmKeyDataRsp( zclKE_Conn_t *pConn, uint8 *pMAC )
{
  return zclKE_SendCfmKeyDataCmd( pConn, ZCL_KE_CFM_KEY_DATA_RSP,
                                  ZCL_FRAME_SERVER_CLIENT_DIR, pMAC );
}

/**************************************************************************************************
 * @fn      zclKE_SendTerminate
 *
 * @brief   Send a terminate command.
 *
 * @param   dstAddr - destination address
 * @param   cmdID - ZCL_KE_TERMINATE_CLIENT or ZCL_KE_TERMINATE_SERVER
 * @param   direction - ZCL_FRAME_SERVER_CLIENT_DIR or ZCL_FRAME_CLIENT_SERVER_DIR
 * @param   seqNum - transaction sequence number
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_SendTerminate( afAddrType_t *dstAddr, uint8 cmdID, 
                                 uint8 direction, uint8 seqNum,
                                 zclKE_TerminateCmd_t *pCmd   )
{
  uint8 cmdBuf[ZCL_KE_TERMINATE_LEN];
  uint8 *pBuf = cmdBuf;

  *pBuf++ = pCmd->status;
  *pBuf++ = pCmd->waitTime;
  *pBuf++ = LO_UINT16( pCmd->suites );
  *pBuf++ = HI_UINT16( pCmd->suites );

  zcl_SendCommand( ZCL_KE_ENDPOINT, dstAddr, ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
                   cmdID, TRUE, direction, TRUE, 0, seqNum, ZCL_KE_TERMINATE_LEN,
                   cmdBuf );
}

/**************************************************************************************************
 * @fn      zclKE_ServerInit
 *
 * @brief   Initialize server side.
 *
 * @param   none
 *
 * @return  void
 */
static void zclKE_ServerInit( void )
{
  uint8 max = ZCL_KE_MAX_SERVER_CONNECTIONS;  

  osal_nv_item_init( ZCD_NV_KE_MAX_DEVICES, 
                     sizeof(uint8),
                     &max );
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnMax
 *
 * @brief   Check for maximum server connections.
 *
 * @param   none
 *
 * @return  uint8 - TRUE if maximum reached, FALSE if not
 */
#if defined ( ZDO_COORDINATOR )
static uint8 zclKE_ServerConnMax( void )
{
  // Assume maximum connections
  uint8 result = TRUE;
  uint8 max;

  // Set the maximum to zero in case NV read fails
  max = 0;

  // Get the maximum number of connections
  osal_nv_read( ZCD_NV_KE_MAX_DEVICES, 0, sizeof(uint8), &max );

  if ( max )
  {
    uint8 active = 0;
    zclKE_Conn_t *pConn = zclKE_ServerConnList;

    // Count the active connections
    while ( pConn )
    {
      active++;
      pConn = pConn->pNext;
    }

    // Are the active connections less than the maximum
    if ( active < max )
    {
      // Connections are not at maximum
      result = FALSE;
    }
  }

  return result;
}
#else
static uint8 zclKE_ServerConnMax( void )
{
  uint8 result;

  // Only allow one connection for non Trust Center devices
  if ( zclKE_ServerConnList )
  {
    result = TRUE;  
  }
  else
  {
    result = FALSE;  
  }

  return result;
}
#endif // ZDO_COORDINATOR

/**************************************************************************************************
 * @fn      zclKE_ServerConnAllowed
 *
 * @brief   Check if server connection is allowed.
 *
 * @param   partnerAddr - partner network address
 *
 * @return  uint8 - TRUE if allowed, FALSE if not
 */
static uint8 zclKE_ServerConnAllowed( uint16 partnerAddr )
{
  uint8 allowed;

  // 1) Server must be ready
  // 2) Only allow server connections when there are no active client connections.
  // 3) Check for maximum server connections.
  // 4) Only allow connections to the Trust Center (or from the Trust Center). 
  if ( ( zclKE_State == ZCL_KE_READY                             ) &&
       ( !zclKE_ClientConnList                                   ) &&
       ( !zclKE_ServerConnMax()                                  ) &&
       ( ( NLME_GetShortAddr() == APSME_TRUSTCENTER_NWKADDR ) ||
         ( partnerAddr == APSME_TRUSTCENTER_NWKADDR         )    )    )
  {
    allowed = TRUE;
  }
  else
  {
    allowed = FALSE;
  }

  return allowed;
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnCreate
 *
 * @brief   Create a new server connection.
 *
 * @param   pPartner - partner address
 *
 * @return  zclKE_Conn_t* - pointer to new connection, NULL if failure
 */
static zclKE_Conn_t *zclKE_ServerConnCreate( afAddrType_t *pPartner )
{
  zclKE_Conn_t *pNewConn = NULL;
  
  // Check for legal connections 
  if ( zclKE_ServerConnAllowed( pPartner->addr.shortAddr ) )
  {
    // Add a new connection
    pNewConn = osal_mem_alloc( sizeof( zclKE_Conn_t ) );

    if ( pNewConn )
    {
      // Initialize the connnection
      osal_memset( pNewConn, 0, sizeof( zclKE_Conn_t ) );
      pNewConn->partner = *pPartner;

      // Add to server connection list
      if ( zclKE_ServerConnList )
      {
        zclKE_Conn_t *pConn = zclKE_ServerConnList;

        while ( pConn->pNext )
        {
          pConn = pConn->pNext;
        }

        pConn->pNext = pNewConn;
      }
      else
      {
        zclKE_ServerConnList = pNewConn;
      }
    }
  }

#if defined( NWK_AUTO_POLL )
  // If connections, set the poll rate
  if ( zclKE_ServerConnList )
  {
    zclKE_SetPollRate( ZCL_KE_SERVER_POLL_RATE_BIT );
  }
#endif

  return pNewConn;
} 

/**************************************************************************************************
 * @fn      zclKE_ServerConnClose
 *
 * @brief   Close an existing server connection.
 *
 * @param   pConn - client connection
 *
 * @return  void
 */
static void zclKE_ServerConnClose( zclKE_Conn_t *pConn )
{
  zclKE_Conn_t *pCurrent = zclKE_ServerConnList;
  zclKE_Conn_t *pPrevious = NULL;

  // Search for connection
  while ( pCurrent )
  {
    // Is this the connection
    if ( pCurrent == pConn )
    {
      // Remove from list
      if ( pPrevious )
      {
        pPrevious->pNext = pCurrent->pNext;
      }
      else
      {
        zclKE_ServerConnList = pCurrent->pNext;
      }  

      // Release connection memory
      zclKE_ConnRelease( pConn );

      break;
    }
    
    // Save the current connection
    pPrevious = pCurrent;

    // Advance to the next connection
    pCurrent = pCurrent->pNext;
  }

#if defined( NWK_AUTO_POLL )
  // If no connections, restore poll rate
  if ( !zclKE_ServerConnList )
  {
    zclKE_RestorePollRate( ZCL_KE_SERVER_POLL_RATE_BIT );
  }
#endif
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnFind
 *
 * @brief   Find a server connection by partnerAddr.
 *
 * @param   partnerAddr - partner network address
 *
 * @return  zclKE_Conn_t* - pointer to existing connection, NULL if not found
 */
static zclKE_Conn_t *zclKE_ServerConnFind( uint16 partnerAddr )
{
  zclKE_Conn_t *pConn;

  pConn = zclKE_ServerConnList;

  while ( pConn )
  {
    if ( pConn->partner.addr.shortAddr == partnerAddr )
    {
      break;
    }

    pConn = pConn->pNext;
  }

  return pConn;
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnLookup
 *
 * @brief   Lookup an expected server connection including end point.
 *
 * @param   pPartner - partner address
 *
 * @return  zclKE_Conn_t* - pointer to existing connection, NULL if not found
 */
static zclKE_Conn_t *zclKE_ServerConnLookup( afAddrType_t *pPartner )
{
  zclKE_Conn_t *pConn = NULL;

  pConn = zclKE_ServerConnFind( pPartner->addr.shortAddr );

  // Check if connection was found and verify end point
  if ( pConn && ( pConn->partner.endPoint != pPartner->endPoint ) )
  {
    // End points do not match return NULL
    pConn = NULL;
  }

  return pConn;
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnGet
 *
 * @brief   Get an existing connection or create a new one.
 *
 * @param   pPartner - partner address
 *
 * @return  zclKE_Conn_t* - pointer to connection, NULL if not available
 */
static zclKE_Conn_t *zclKE_ServerConnGet( afAddrType_t *pPartner )
{
  zclKE_Conn_t *pConn = NULL;
 
  // Check for current connection
  pConn = zclKE_ServerConnFind( pPartner->addr.shortAddr );

  if ( !pConn )
  {
    // Create a new connection
    pConn = zclKE_ServerConnCreate( pPartner ); 
  }
  else
  {
    // Connection already open, verify end point
    if ( pConn->partner.endPoint != pPartner->endPoint )
    {
      // Connection has different end point, return NULL
      pConn = NULL;
    }
  }

  return pConn;
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnTerminate
 *
 * @brief   Terminate server connection.
 *
 * @param   pCtxt - connection context
 *
 * @return  void
 */
static void zclKE_ServerConnTerminate( zclKE_ConnCtxt_t *pCtxt )
{
  zclKE_TerminateCmd_t cmd;

  cmd.status = pCtxt->error;
  cmd.suites = zclKE_SupportedSuites;
  cmd.waitTime = 2 * ( ZCL_KE_SERVER_EPH_DATA_GEN_TIME + ZCL_KE_SERVER_CFM_KEY_GEN_TIME );

  if ( pCtxt->pConn )
  {
    zclKE_SendTerminate( &pCtxt->pConn->partner,
                         ZCL_KE_TERMINATE_CLIENT,
                         ZCL_FRAME_SERVER_CLIENT_DIR,
                         pCtxt->pConn->transSeqNum,
                         &cmd );

    zclKE_ServerConnClose( pCtxt->pConn );      
  }
  else if ( pCtxt->pInMsg )
  {
    zclKE_SendTerminate( &pCtxt->pInMsg->msg->srcAddr,
                         ZCL_KE_TERMINATE_CLIENT,
                         ZCL_FRAME_SERVER_CLIENT_DIR,
                         pCtxt->pInMsg->hdr.transSeqNum,
                         &cmd );
  }
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnKeyGenTimeout
 *
 * @brief   Timeout triggers send of ZCL_KE_KEY_GEN_MSG(see ZCL_KE_KEY_GEN_STAGES_SERVER).
 *
 * @param   pConn - connection
 *
 * @return  void
 */
static void zclKE_ServerConnKeyGenTimeout( zclKE_Conn_t *pConn )
{
  zclKE_KeyGenMsg_t *pMsg;

  // Queue the key generate message
  pMsg = (zclKE_KeyGenMsg_t *) osal_msg_allocate( sizeof( zclKE_KeyGenMsg_t ) );

  if (pMsg)
  {
    // Send key generate message
    pMsg->hdr.event = ZCL_KE_KEY_GEN_MSG;
    pMsg->hdr.status = 0;
    pMsg->server = TRUE;
    pMsg->partnerAddr = pConn->partner.addr.shortAddr;
    osal_msg_send( zclKE_TaskID, (uint8 *)pMsg ) ;

    // Change state
    pConn->state = ZCL_KE_SERVER_CONN_KEY_GEN_QUEUED;
  }
  else
  {
    // Terminate the connection
    zclKE_ConnCtxt_t ctxt;
    ctxt.pInMsg = NULL;
    ctxt.pConn = pConn;
    ctxt.error = ZCL_KE_TERMINATE_NO_RESOURCES;
    zclKE_ServerConnTerminate( &ctxt );
  }
}

/**************************************************************************************************
 * @fn      zclKE_ServerConnTimeout
 *
 * @brief   Server connection timeout.
 *
 * @param   pConn - connection
 *
 * @return  void
 */
static void zclKE_ServerConnTimeout( zclKE_Conn_t *pConn )
{
  // Clear timer info
  pConn->timeout = 0;

  switch ( pConn->state )
  {
    case ZCL_KE_SERVER_CONN_KEY_GEN_WAIT:
      zclKE_ServerConnKeyGenTimeout( pConn );
      break;

    default:
      zclKE_ServerConnClose( pConn ); 
      break;
  }
}

/**************************************************************************************************
 * @fn      zclKE_ServerProcessInitiateReq
 *
 * @brief   Process ZCL_KE_INITIATE_REQ.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_ServerProcessInitiateReq( zclKE_ConnCtxt_t *pCtxt,
                                            zclKE_InitiateCmd_t *pCmd )
{
  uint16 len;
  zclKE_Conn_t *pConn = pCtxt->pConn;

  if ( !zclKE_InitiateCmdCheckFields( pCtxt, pCmd ) )
  {
    // pCtxt->error set in "zclKE_InitiateCmdCheckFields"
    return;
  }

  // Finish setting up connection fields
  pConn->suite = pCmd->suite;
  pConn->rmtEphDataGenTime = pCmd->ephDataGenTime;
  pConn->rmtCfmKeyGenTime = pCmd->cfmKeyGenTime;

  len = zclKE_GetField( pConn->suite, ZCL_KE_CERT_LEN );

  // Copy the remote certificate
  pConn->pRmtCert = osal_mem_alloc( len );
  if ( !pConn->pRmtCert )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  osal_memcpy( pConn->pRmtCert, pCmd->pIdentity, len );

  // Send the ZCL_KE_INITIATE_RSP command
  if ( zclKE_SendInitiateRsp( pConn ) != ZSuccess )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Set state to wait for ZCL_KE_EPH_DATA_REQ
  pConn->state = ZCL_KE_SERVER_CONN_EPH_DATA_REQ_WAIT;

  // Set ephemeral data generation timeout
  zclKE_ConnSetTimeout( pConn, pConn->rmtEphDataGenTime * 1000 );
}

/**************************************************************************************************
 * @fn      zclKE_ServerProcessEphDataReq
 *
 * @brief   Process ZCL_KE_EPH_DATA_REQ.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_ServerProcessEphDataReq( zclKE_ConnCtxt_t *pCtxt,
                                           zclKE_EphDataCmd_t *pCmd )
{
  zclKE_Conn_t *pConn = pCtxt->pConn;
  uint16 len;
  
  len = zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN );

  // Copy the remote ephemeral public key
  pConn->pRmtEPublicKey = osal_mem_alloc( len );

  if ( !pConn->pRmtEPublicKey )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  osal_memcpy( pConn->pRmtEPublicKey, pCmd->pEphData, len );

  /*===============================================================================================
  * ZCL_KE_KEY_GEN_STAGES_SERVER: 
  *
  * Server key generation is broken into two stages in order to break up the calculation times, 
  * which can starve processing time for other tasks.
  *
  *   Stage 1(current):
  *     - generate ephemeral key data
  *     - start timer 500ms
  *
  *   Stage 2(next):
  *     - timer expires
  *     - generate keys bits
  *     - derive mac and key data
  *     - send ZCL_KE_EPH_DATA_REQ
  *
  ===============================================================================================*/

  // Generate ephemeral key data
  if ( !zclKE_GenEphKeys( pCtxt ) )
  {
    // pCtxt->error set in "zclKE_GenKeyBits"
    return;
  }

  // Set state to wait for key generation
  pConn->state = ZCL_KE_SERVER_CONN_KEY_GEN_WAIT;

  // Set key generation timeout
  zclKE_ConnSetTimeout( pConn, ZCL_KE_KEY_GEN_TIMEOUT );
}

/**************************************************************************************************
 * @fn      zclKE_ServerProcessKeyGen
 *
 * @brief   Process ZCL_KE_KEY_GEN_MSG.
 *
 * @param   pCtxt - connection context
 *
 * @return  void
 */
static void zclKE_ServerProcessKeyGen( zclKE_ConnCtxt_t *pCtxt )
{
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Handle server connection key generation stage 2 -- see ZCL_KE_KEY_GEN_STAGES_SERVER
  if ( !zclKE_GenKeys( pCtxt ) )
  {
    // pCtxt->error set in "zclKE_GenKeys"
    return;
  }

  // Send the ZCL_KE_EPH_DATA_RSP command
  if ( zclKE_SendEphDataRsp( pConn ) != ZSuccess )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Set state to wait for ZCL_KE_CFM_KEY_DATA_REQ
  pConn->state = ZCL_KE_SERVER_CONN_CFM_KEY_DATA_REQ_WAIT;

  // Set aging timeout
  zclKE_ConnSetTimeout( pConn, pConn->rmtCfmKeyGenTime * 1000 );
}

/**************************************************************************************************
 * @fn      zclKE_ServerProcessCfmKeyDataReq
 *
 * @brief   Process ZCL_KE_CFM_KEY_DATA_REQ.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_ServerProcessCfmKeyDataReq( zclKE_ConnCtxt_t *pCtxt,
                                              zclKE_CfmKeyDataCmd_t *pCmd )
{
  uint8 MAC[ZCL_KE_MAC_LEN];
  uint8 partnerExtAddr[Z_EXTADDR_LEN];
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Lookup partner's extended address -- required to add link key
  if ( !AddrMgrExtAddrLookup( pConn->partner.addr.shortAddr, partnerExtAddr ) )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Calculate MACu
  if ( !zclKE_GenMAC( pConn, FALSE, TRUE, MAC ) )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Compare MACu values
  if ( osal_memcmp( MAC, pCmd->pMAC, ZCL_KE_MAC_LEN ) != TRUE )
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_KEY_CONFIRM;
    return;
  }

  // Calculate MACv
  if ( !zclKE_GenMAC( pConn, FALSE, FALSE, MAC ) )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Send the ZCL_KE_CFM_KEY_DATA_RSP command
  if ( zclKE_SendCfmKeyDataRsp( pConn, MAC ) != ZSuccess )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Add the link key
  ZDSecMgrAddLinkKey( pConn->partner.addr.shortAddr,
                      partnerExtAddr,
                      pConn->pKey );

  // Done, close connection
  zclKE_ServerConnClose( pConn );
}

/**************************************************************************************************
 * @fn      zclKE_ServerHdlInitiateReq
 *
 * @brief   Handle ZCL_KE_INITIATE_REQ.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ServerHdlInitiateReq( zclKE_ConnCtxt_t *pCtxt )
{
  // Find an existing connection or create a new one
  pCtxt->pConn = zclKE_ServerConnGet( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    // Update transaction sequence number
    pCtxt->pConn->transSeqNum = pCtxt->pInMsg->hdr.transSeqNum;

    if ( pCtxt->pConn->state == ZCL_KE_SERVER_CONN_INIT )
    {
      zclKE_InitiateCmd_t cmd;

      if ( zclKE_ParseInitiateCmd( pCtxt->pInMsg, &cmd ) == ZCL_STATUS_SUCCESS )
      {
        zclKE_ServerProcessInitiateReq( pCtxt, &cmd );      
      }
      else
      {
        pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
      }
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
  }
  else
  {
    // No resources available
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
  }

  // Check for error and terminate connection
  if ( pCtxt->error )
  {
    zclKE_ServerConnTerminate( pCtxt ); 
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/**************************************************************************************************
 * @fn      zclKE_ServerHdlEphDataReq
 *
 * @brief   Handle ZCL_KE_EPH_DATA_REQ.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ServerHdlEphDataReq( zclKE_ConnCtxt_t *pCtxt )
{
  // Look for an existing connection
  pCtxt->pConn = zclKE_ServerConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    // Update transaction sequence number
    pCtxt->pConn->transSeqNum = pCtxt->pInMsg->hdr.transSeqNum;

    if ( pCtxt->pConn->state == ZCL_KE_SERVER_CONN_EPH_DATA_REQ_WAIT )
    {
      zclKE_EphDataCmd_t cmd;

      if ( zclKE_ParseEphDataCmd( pCtxt->pConn->suite, 
                                  pCtxt->pInMsg, 
                                  &cmd ) == ZCL_STATUS_SUCCESS )
      {
        zclKE_ServerProcessEphDataReq( pCtxt, &cmd );      
      }
      else
      {
        pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
      }
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
  }

  // Check for error and terminate connection
  if ( pCtxt->error )
  {
    zclKE_ServerConnTerminate( pCtxt ); 
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/**************************************************************************************************
 * @fn      zclKE_ServerHdlCfmKeyDataReq
 *
 * @brief   Handle ZCL_KE_CFM_KEY_DATA_REQ.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ServerHdlCfmKeyDataReq( zclKE_ConnCtxt_t *pCtxt )
{
  // Look for an existing connection
  pCtxt->pConn = zclKE_ServerConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    // Update transaction sequence number
    pCtxt->pConn->transSeqNum = pCtxt->pInMsg->hdr.transSeqNum;

    if ( pCtxt->pConn->state == ZCL_KE_SERVER_CONN_CFM_KEY_DATA_REQ_WAIT )
    {
      zclKE_CfmKeyDataCmd_t cmd;

      if ( zclKE_ParseCfmKeyDataCmd( pCtxt->pInMsg, &cmd ) == ZCL_STATUS_SUCCESS )
      {
        zclKE_ServerProcessCfmKeyDataReq( pCtxt, &cmd );      
      }
      else
      {
        pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
      }
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
  }

  // Check for error and terminate connection
  if ( pCtxt->error )
  {
    zclKE_ServerConnTerminate( pCtxt ); 
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/**************************************************************************************************
 * @fn      zclKE_ServerHdlTerminate
 *
 * @brief   Handle ZCL_KE_TERMINATE_SERVER.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ServerHdlTerminate( zclKE_ConnCtxt_t *pCtxt )
{
  // Look for an existing connection
  pCtxt->pConn = zclKE_ServerConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    // Update transaction sequence number
    pCtxt->pConn->transSeqNum = pCtxt->pInMsg->hdr.transSeqNum;

    zclKE_ServerConnClose( pCtxt->pConn );
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_ServerHdlSpecificCmd
 *
 * @brief   Server ZCL specific command handler.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ServerHdlSpecificCmd( zclIncoming_t *pInMsg )
{
  ZStatus_t status;
  zclKE_ConnCtxt_t ctxt;

  // Initialize connection context
  ctxt.pInMsg = pInMsg;
  ctxt.pConn = NULL;
  ctxt.error = 0;

  // Process the command
  switch ( pInMsg->hdr.commandID )
  {
    case ZCL_KE_INITIATE_REQ:
      status = zclKE_ServerHdlInitiateReq( &ctxt );
      break;

    case ZCL_KE_EPH_DATA_REQ:
      status = zclKE_ServerHdlEphDataReq( &ctxt );
      break;

    case ZCL_KE_CFM_KEY_DATA_REQ:
      status = zclKE_ServerHdlCfmKeyDataReq( &ctxt );
      break;

    case ZCL_KE_TERMINATE_SERVER:
      status = zclKE_ServerHdlTerminate( &ctxt );
      break;

    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_ServerKeyGenMsg
 *
 * @brief   Process server ZCL_KE_KEY_GEN_MSG.
 *
 * @param   pMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ServerKeyGenMsg( zclKE_KeyGenMsg_t *pMsg )
{
  zclKE_Conn_t *pConn;

  // Find the existing connection
  pConn = zclKE_ServerConnFind( pMsg->partnerAddr );

  // Check for connection and state
  if ( pConn && ( pConn->state == ZCL_KE_SERVER_CONN_KEY_GEN_QUEUED ) )
  {
    zclKE_ConnCtxt_t ctxt;

    ctxt.pInMsg = NULL;
    ctxt.pConn = pConn;
    ctxt.error = 0;

    zclKE_ServerProcessKeyGen( &ctxt );

    // Check for failure and terminate connection
    if ( ctxt.error )
    {
      zclKE_ServerConnTerminate( &ctxt ); 
    }
  }
}

/**************************************************************************************************
 * @fn      zclKE_ClientInit
 *
 * @brief   Initialize client side.
 *
 * @param   none
 *
 * @return  void
 */
static void zclKE_ClientInit( void )
{
  // Register for Match Descriptor Responses
  ZDO_RegisterForZDOMsg( zclKE_TaskID, Match_Desc_rsp );
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnAllowed
 *
 * @brief   Check if client connection is allowed.
 *
 * @param   partnerAddr - partner network address
 *
 * @return  uint8 - TRUE if allowed, FALSE if not
 */
static uint8 zclKE_ClientConnAllowed( uint16 partnerAddr )
{
  uint8 allowed;

  // 1) Client must be ready
  // 2) Only allow client connections when there are no active server connections.
  // 3) Check for maximum client connections(currently just one).
  // 4) Only allow connections to the Trust Center (or from the Trust Center). 
  if ( ( zclKE_State == ZCL_KE_READY                             ) &&
       ( !zclKE_ServerConnList                                   ) &&
       ( !zclKE_ClientConnList                                   ) &&
       ( ( NLME_GetShortAddr() == APSME_TRUSTCENTER_NWKADDR ) ||
         ( partnerAddr == APSME_TRUSTCENTER_NWKADDR         )    )    )
  {
    allowed = TRUE;
  }
  else
  {
    allowed = FALSE;
  }

  return allowed;
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnCreate
 *
 * @brief   Create a new client connection.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   pPartner - partner address
 * @param   transSeqNum - starting transaction sequence number
 * @param   suite - security suite
 *
 * @return  zclKE_Conn_t* - pointer to new connection, NULL if failure
 */
static zclKE_Conn_t *zclKE_ClientConnCreate( uint8 taskID,
                                             afAddrType_t *pPartner,
                                             uint8 transSeqNum,
                                             uint16 suite )
{
  zclKE_Conn_t *pConn = NULL;

  // Check for legal connections 
  if ( zclKE_ClientConnAllowed( pPartner->addr.shortAddr ) )
  {
    // Add connection
    pConn = osal_mem_alloc( sizeof( zclKE_Conn_t ) );
  
    if ( pConn )
    {
      // Initialize the connnection
      osal_memset( pConn, 0, sizeof( zclKE_Conn_t ) );
      pConn->taskID = taskID;
      pConn->partner = *pPartner;
      pConn->transSeqNum = transSeqNum;
      pConn->suite = suite;
  
      // Currently only one client connection active at a time
      zclKE_ClientConnList = pConn;
    }
  }

#if defined( NWK_AUTO_POLL )
  // If connections, set the poll rate
  if ( zclKE_ClientConnList )
  {
    zclKE_SetPollRate( ZCL_KE_CLIENT_POLL_RATE_BIT );
  }
#endif

  return pConn;
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnClose
 *
 * @brief   Close an existing client connection.
 *
 * @param   pConn - client connection
 * @param   notifyStatus - notify status of closure
 * @param   pCmd - terminate command
 *
 * @return  void
 */
static void zclKE_ClientConnClose( zclKE_Conn_t *pConn, 
                                   uint8 notifyStatus,
                                   zclKE_TerminateCmd_t *pCmd )
{
  zclKE_NotifyStatus( pConn->taskID,
                      pConn->partner.addr.shortAddr,
                      notifyStatus,
                      pCmd );

  // Release connection memory
  zclKE_ConnRelease( pConn );

  // Currently only one client connection
  zclKE_ClientConnList = NULL;    

#if defined( NWK_AUTO_POLL )
  // If no connections, restore poll rate
  if ( !zclKE_ClientConnList )
  {
    zclKE_RestorePollRate( ZCL_KE_CLIENT_POLL_RATE_BIT );
  }
#endif
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnFind
 *
 * @brief   Find a client connection by partnerAddr.
 *
 * @param   partnerAddr - partner network address
 *
 * @return  zclKE_Conn_t* - pointer to existing connection, NULL if not found
 */
static zclKE_Conn_t *zclKE_ClientConnFind( uint16 partnerAddr )
{
  zclKE_Conn_t *pConn;

  pConn = zclKE_ClientConnList;

  while ( pConn )
  {
    if ( pConn->partner.addr.shortAddr == partnerAddr )
    {
      break;
    }

    pConn = pConn->pNext;
  }

  return pConn;
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnLookup
 *
 * @brief   Lookup an expected client connection including end point.
 *
 * @param   pPartner - partner address
 *
 * @return  zclKE_Conn_t* - pointer to existing connection, NULL if not found
 */
static zclKE_Conn_t *zclKE_ClientConnLookup( afAddrType_t *pPartner )
{
  zclKE_Conn_t *pConn = NULL;

  pConn = zclKE_ClientConnFind( pPartner->addr.shortAddr );

  // Check if connection was found and verify end point
  if ( pConn && ( pConn->partner.endPoint != pPartner->endPoint ) )
  {
    // End points do not match return NULL
    pConn = NULL;
  }

  return pConn;
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnOpen
 *
 * @brief   Open a new client connection.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   pPartner - partner address
 * @param   transSeqNum - starting transaction sequence number
 * @param   suite - security suite
 * @param   ppConn - reference to output connection pointer
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_ClientConnOpen( uint8 taskID,
                                   afAddrType_t *pPartner,
                                   uint8 transSeqNum,
                                   uint16 suite,
                                   zclKE_Conn_t **ppConn )
{
  uint8 success = FALSE;
  uint8 extAddr[Z_EXTADDR_LEN];
  uint8 notifyErr ;

  *ppConn = NULL;

  do 
  {
    if ( zclKE_State != ZCL_KE_READY )
    {
      // Currently the only error state is no certs
      // if ( zclKE_State == ZCL_KE_NO_CERTS )
      notifyErr = ZCL_KE_NOTIFY_NO_CERTS;
      break;
    }

    // Make sure connection isn't already open
    if ( zclKE_ClientConnFind( pPartner->addr.shortAddr ) )
    {
      notifyErr = ZCL_KE_NOTIFY_BUSY;
      break;
    }

    // Verify partner's extended address
    if ( !AddrMgrExtAddrLookup( pPartner->addr.shortAddr, extAddr ) )
    {
      notifyErr = ZCL_KE_NOTIFY_NO_EXT_ADDR;
      break;
    }
    
    // Check for valid suite
    if ( !( suite & zclKE_SupportedSuites ) )
    {
      notifyErr = ZCL_KE_NOTIFY_BAD_SUITE;
      break;
    }

    // Create connection
    *ppConn = zclKE_ClientConnCreate( taskID, pPartner, transSeqNum, suite );

    if ( !( *ppConn ) )
    {
      notifyErr = ZCL_KE_NOTIFY_BUSY;
      break;
    }

    success = TRUE;
    
  } while ( 0 );

  if ( !success )
  {
    zclKE_NotifyStatus( taskID, pPartner->addr.shortAddr, notifyErr, NULL );
  }

  return success;
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnTerminate
 *
 * @brief   Terminate client connection.
 *
 * @param   pCtxt - connection context
 *
 * @return  void
 */
static void zclKE_ClientConnTerminate( zclKE_ConnCtxt_t *pCtxt )
{
  zclKE_TerminateCmd_t cmd;

  cmd.status = pCtxt->error;
  cmd.suites = zclKE_SupportedSuites;
  cmd.waitTime = 2 * ( ZCL_KE_CLIENT_EPH_DATA_GEN_TIME + ZCL_KE_CLIENT_CFM_KEY_GEN_TIME );

  if ( pCtxt->pConn )
  {
    zclKE_SendTerminate( &pCtxt->pConn->partner,
                         ZCL_KE_TERMINATE_SERVER,
                         ZCL_FRAME_CLIENT_SERVER_DIR,
                         pCtxt->pConn->transSeqNum,
                         &cmd );

    zclKE_ClientConnClose( pCtxt->pConn, ZCL_KE_NOTIFY_TERMINATE_SENT, &cmd ); 
  }
  else if ( pCtxt->pInMsg )
  {
    zclKE_SendTerminate( &pCtxt->pInMsg->msg->srcAddr,
                         ZCL_KE_TERMINATE_SERVER,
                         ZCL_FRAME_CLIENT_SERVER_DIR,
                         pCtxt->pInMsg->hdr.transSeqNum,
                         &cmd );
  }
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnKeyGenTimeout
 *
 * @brief   Timeout triggers send of ZCL_KE_KEY_GEN_MSG(see ZCL_KE_KEY_GEN_STAGES_CLIENT).
 *
 * @param   pConn - connection
 *
 * @return  void
 */
static void zclKE_ClientConnKeyGenTimeout( zclKE_Conn_t *pConn )
{
  zclKE_KeyGenMsg_t *pMsg;

  // Queue the key generate message
  pMsg = (zclKE_KeyGenMsg_t *) osal_msg_allocate( sizeof( zclKE_KeyGenMsg_t ) );

  if (pMsg)
  {
    // Send key generate message
    pMsg->hdr.event = ZCL_KE_KEY_GEN_MSG;
    pMsg->hdr.status = 0;
    pMsg->server = FALSE;
    pMsg->partnerAddr = pConn->partner.addr.shortAddr;
    osal_msg_send( zclKE_TaskID, (uint8 *)pMsg ) ;

    // Change state
    pConn->state = ZCL_KE_CLIENT_CONN_KEY_GEN_QUEUED;
  }
  else
  {
    // Terminate the connection
    zclKE_ConnCtxt_t ctxt;
    ctxt.pInMsg = NULL;
    ctxt.pConn = pConn;
    ctxt.error = ZCL_KE_TERMINATE_NO_RESOURCES;
    zclKE_ClientConnTerminate( &ctxt );
  }
}

/**************************************************************************************************
 * @fn      zclKE_ClientConnTimeout
 *
 * @brief   Client connection timeout.
 *
 * @param   pConn - connection
 *
 * @return  void
 */
static void zclKE_ClientConnTimeout( zclKE_Conn_t *pConn )
{
  // Clear timer info
  pConn->timeout = 0;

  switch ( pConn->state )
  {
    case ZCL_KE_CLIENT_CONN_KEY_GEN_WAIT:
      zclKE_ClientConnKeyGenTimeout( pConn );
      break;

    default:
      zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_TIMEOUT, NULL ); 
      break;
  }
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessMatchReq
 *
 * @brief   Process request to match an end point for key establishment.
 *
 * @param   pConn - connection
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_ClientProcessMatchReq( zclKE_Conn_t *pConn )
{
  ZStatus_t status;
  zAddrType_t dstAddr;
  cId_t cbke = ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT;

  // Send out a match for the key establishment
  dstAddr.addrMode = Addr16Bit;
  dstAddr.addr.shortAddr = pConn->partner.addr.shortAddr;

  status = ZDP_MatchDescReq( &dstAddr, pConn->partner.addr.shortAddr, 
                             ZCL_SE_PROFILE_ID, 1, &cbke, 0, NULL, FALSE );

  if ( status != ZSuccess )
  {
    return FALSE;
  }

  // Set state to wait for Match_Desc_rsp
  pConn->state = ZCL_KE_CLIENT_CONN_MATCH_RSP_WAIT;

  // Set match timeout
  zclKE_ConnSetTimeout( pConn, ZCL_KE_SUITE_DISCOVERY_TIMEOUT );

  return TRUE;
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessMatchRsp
 *
 * @brief   Process match of end point for key establishment.
 *
 * @param   pConn - connection
 * @param   ep - end point
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_ClientProcessMatchRsp( zclKE_Conn_t *pConn, uint8 ep )
{
#if defined( ZCL_READ )
  zclKE_ReadCmd_t cmd;

  // Save the end point
  pConn->partner.endPoint = ep;

  // Read the partner's suite attribute to find out which suites are supported
  cmd.numAttr = 1;
  cmd.attrID[0] = ATTRID_KE_SUITE;

  if ( zcl_SendRead( ZCL_KE_ENDPOINT, &pConn->partner,
                     ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT, (zclReadCmd_t*)&cmd,
                     ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, pConn->transSeqNum ) != ZSuccess )
  {
    return FALSE; 
  }

  // Increment the transaction number
  pConn->transSeqNum++;

  // Set state to wait for ZCL_CMD_READ_RSP
  pConn->state = ZCL_KE_CLIENT_CONN_READ_RSP_WAIT;

  // Set read response timeout
  zclKE_ConnSetTimeout( pConn, ZCL_KE_SUITE_DISCOVERY_TIMEOUT );

  return TRUE;
#else
  (void)pConn;
  (void)ep;
  return FALSE;
#endif // ZCL_READ
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessStart
 *
 * @brief   Process start request.
 *
 * @param   pCtxt - connection context
 *
 * @return  uint8 - TRUE if successful, FALSE if not
 */
static uint8 zclKE_ClientProcessStart( zclKE_ConnCtxt_t *pCtxt )
{
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Generate the ephemeral keys
  if ( !zclKE_GenEphKeys( pCtxt ) )
  {
    // pCtxt->error set in "zclKE_GenKeyBits"
    return FALSE;
  }  

  // Send the ZCL_KE_INITIATE_REQ
  if ( zclKE_SendInitiateReq( pConn ) != ZSuccess )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return FALSE;
  }

  // Increment the transaction number
  pConn->transSeqNum++;

  // Set state to wait for ZCL_KE_INITIATE_RSP
  pConn->state = ZCL_KE_CLIENT_CONN_INIT_RSP_WAIT;

  // Set aging timeout
  zclKE_ConnSetTimeout( pConn, pConn->rmtEphDataGenTime * 1000 );

  return TRUE;
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessInitiateRsp
 *
 * @brief   Process ZCL_KE_INITIATE_RSP.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_ClientProcessInitiateRsp( zclKE_ConnCtxt_t *pCtxt,
                                            zclKE_InitiateCmd_t *pCmd )
{
  if ( zclKE_InitiateRspCheckFields( pCtxt, pCmd ) )
  {
    // Finish setting up connection fields
    uint16 len;
    zclKE_Conn_t *pConn = pCtxt->pConn;
    pConn->rmtEphDataGenTime = pCmd->ephDataGenTime;
    pConn->rmtCfmKeyGenTime = pCmd->cfmKeyGenTime;

    len = zclKE_GetField( pConn->suite, ZCL_KE_CERT_LEN );

    // Copy the remote certificate
    pConn->pRmtCert = osal_mem_alloc( len );

    if ( !pConn->pRmtCert )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      return;
    }

    osal_memcpy( pConn->pRmtCert, pCmd->pIdentity, len );

    // Send the ZCL_KE_EPH_DATA_REQ command
    if ( zclKE_SendEphDatReq( pCtxt->pConn ) != ZSuccess )
    {
      pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
      return;
    }

    // Increment the transaction number
    pConn->transSeqNum++;

    // Update connection state    
    pConn->state = ZCL_KE_CLIENT_CONN_EPH_DATA_RSP_WAIT;

    // Set ephemeral data generation timeout
    zclKE_ConnSetTimeout( pConn, pConn->rmtEphDataGenTime * 1000 );
  }
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessEphDataRsp
 *
 * @brief   Process ZCL_KE_EPH_DATA_RSP.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_ClientProcessEphDataRsp( zclKE_ConnCtxt_t *pCtxt,
                                           zclKE_EphDataCmd_t *pCmd )
{
  zclKE_Conn_t *pConn = pCtxt->pConn;
  uint16 len;
  
  len = zclKE_GetField( pConn->suite, ZCL_KE_PUBLIC_KEY_LEN );

  // Copy the remote ephemeral public key
  pConn->pRmtEPublicKey = osal_mem_alloc( len );

  if ( !pConn->pRmtEPublicKey )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  osal_memcpy( pConn->pRmtEPublicKey, pCmd->pEphData, len );

  /*===============================================================================================
  * ZCL_KE_KEY_GEN_STAGES_CLIENT: 
  *
  * Client key generation is broken into two stages in order to break up the calculation times, 
  * which can starve processing time for other tasks.
  *
  *   Stage 1(current):
  *     - start timer 500ms
  *
  *   Stage 2(next):
  *     - timer expires
  *     - generate keys bits
  *     - derive mac and key data
  *     - generate MACu
  *     - send ZCL_KE_CFM_KEY_DATA_REQ
  *
  ===============================================================================================*/

  // Set state to wait for key generation
  pConn->state = ZCL_KE_CLIENT_CONN_KEY_GEN_WAIT;

  // Set key generation timeout
  zclKE_ConnSetTimeout( pConn, ZCL_KE_KEY_GEN_TIMEOUT );
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessKeyGen
 *
 * @brief   Process ZCL_KE_KEY_GEN_MSG.
 *
 * @param   pCtxt - connection context
 *
 * @return  void
 */
static void zclKE_ClientProcessKeyGen( zclKE_ConnCtxt_t *pCtxt )
{
  uint8 MAC[ZCL_KE_MAC_LEN];
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Handle server connection key generation stage 2 -- see ZCL_KE_KEY_GEN_STAGES_CLIENT
  if ( !zclKE_GenKeys( pCtxt ) )
  {
    // pCtxt->error set in "zclKE_GenKeys"
    return;
  }

  // Calculate MACu
  if ( !zclKE_GenMAC( pConn, TRUE, TRUE, MAC ) )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Send the ZCL_KE_CFM_KEY_DATA_REQ command
  if ( zclKE_SendCfmKeyDataReq( pConn, MAC ) != ZSuccess )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Increment the transaction number
  pConn->transSeqNum++;

  // Set state to wait for ZCL_KE_CFM_KEY_DATA_RSP
  pConn->state = ZCL_KE_CLIENT_CONN_CFM_KEY_DATA_RSP_WAIT;

  // Set aging timeout
  zclKE_ConnSetTimeout( pConn, pConn->rmtCfmKeyGenTime * 1000 );
}

/**************************************************************************************************
 * @fn      zclKE_ClientProcessCfmKeyDataRsp
 *
 * @brief   Process ZCL_KE_CFM_KEY_DATA_RSP.
 *
 * @param   pCtxt - connection context
 * @param   pCmd - command payload
 *
 * @return  void
 */
static void zclKE_ClientProcessCfmKeyDataRsp( zclKE_ConnCtxt_t *pCtxt,
                                              zclKE_CfmKeyDataCmd_t *pCmd )
{
  uint8 MAC[ZCL_KE_MAC_LEN];
  uint8 partnerExtAddr[Z_EXTADDR_LEN];
  zclKE_Conn_t *pConn = pCtxt->pConn;

  // Lookup partner's extended address -- required to add link key
  if ( !AddrMgrExtAddrLookup( pConn->partner.addr.shortAddr, partnerExtAddr ) )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Calculate MACv
  if ( !zclKE_GenMAC( pConn, TRUE, FALSE, MAC ) )
  {
    pCtxt->error = ZCL_KE_TERMINATE_NO_RESOURCES;
    return;
  }

  // Compare MACv values
  if ( osal_memcmp( MAC, pCmd->pMAC, ZCL_KE_MAC_LEN ) != TRUE )
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_KEY_CONFIRM;
    return;
  }

  // Add the link key
  ZDSecMgrAddLinkKey( pConn->partner.addr.shortAddr,
                      partnerExtAddr,
                      pConn->pKey );

  // Done, close connection
  zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_SUCCESS,  NULL );
}

/**************************************************************************************************
 * @fn      zclKE_ClientHdlInitiateRsp
 *
 * @brief   Handle ZCL_KE_INITIATE_RSP.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ClientHdlInitiateRsp( zclKE_ConnCtxt_t *pCtxt )
{
  // Look for an existing connection
  pCtxt->pConn = zclKE_ClientConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    if ( pCtxt->pConn->state == ZCL_KE_CLIENT_CONN_INIT_RSP_WAIT )
    {
      zclKE_InitiateCmd_t cmd;

      if ( zclKE_ParseInitiateCmd( pCtxt->pInMsg, &cmd ) == ZCL_STATUS_SUCCESS )
      {
        zclKE_ClientProcessInitiateRsp( pCtxt, &cmd );      
      }
      else
      {
        pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
      }
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
  }

  // Check for error and terminate connection
  if ( pCtxt->error )
  {
    zclKE_ClientConnTerminate( pCtxt ); 
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/**************************************************************************************************
 * @fn      zclKE_ClientHdlEphDataRsp
 *
 * @brief   Handle ZCL_KE_EPH_DATA_RSP.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ClientHdlEphDataRsp( zclKE_ConnCtxt_t *pCtxt )
{
  // Look for an existing connection
  pCtxt->pConn = zclKE_ClientConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    if ( pCtxt->pConn->state == ZCL_KE_CLIENT_CONN_EPH_DATA_RSP_WAIT )
    {
      zclKE_EphDataCmd_t cmd;

      if ( zclKE_ParseEphDataCmd( pCtxt->pConn->suite, 
                                  pCtxt->pInMsg, 
                                  &cmd ) == ZCL_STATUS_SUCCESS )
      {
        zclKE_ClientProcessEphDataRsp( pCtxt, &cmd );      
      }
      else
      {
        pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
      }
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
  }

  // Check for error and terminate connection
  if ( pCtxt->error )
  {
    zclKE_ClientConnTerminate( pCtxt ); 
  }

  return ZCL_STATUS_CMD_HAS_RSP;
}

/**************************************************************************************************
 * @fn      zclKE_ClientHdlCfmKeyDataRsp
 *
 * @brief   Handle ZCL_KE_CFM_KEY_DATA_RSP.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ClientHdlCfmKeyDataRsp( zclKE_ConnCtxt_t *pCtxt )
{
  ZStatus_t status;

  // Look for an existing connection
  pCtxt->pConn = zclKE_ClientConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    if ( pCtxt->pConn->state == ZCL_KE_CLIENT_CONN_CFM_KEY_DATA_RSP_WAIT )
    {
      zclKE_CfmKeyDataCmd_t cmd;

      if ( zclKE_ParseCfmKeyDataCmd( pCtxt->pInMsg, &cmd ) == ZCL_STATUS_SUCCESS )
      {
        zclKE_ClientProcessCfmKeyDataRsp( pCtxt, &cmd );
      }
      else
      {
        pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
      }
    }
    else
    {
      pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
    }
  }
  else
  {
    pCtxt->error = ZCL_KE_TERMINATE_BAD_MESSAGE;
  }

  // Check for error
  if ( pCtxt->error )
  {
    // Terminate connection
    zclKE_ClientConnTerminate( pCtxt );

    // Failure -- ZCL_KE_TERMINATE_SERVER sent
    status = ZCL_STATUS_CMD_HAS_RSP;
  }
  else
  {
    // Success -- no other response
    status = ZCL_STATUS_SUCCESS;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_ClientHdlTerminate
 *
 * @brief   Handle ZCL_KE_TERMINATE_CLIENT.
 *
 * @param   pCtxt - connection context
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ClientHdlTerminate( zclKE_ConnCtxt_t *pCtxt )
{
  // Look for an existing connection
  pCtxt->pConn = zclKE_ClientConnLookup( &pCtxt->pInMsg->msg->srcAddr );

  if ( pCtxt->pConn )
  {
    zclKE_TerminateCmd_t cmd = {0};

    zclKE_ParseTerminateCmd( pCtxt->pInMsg, &cmd );

    zclKE_ClientConnClose( pCtxt->pConn, ZCL_KE_NOTIFY_TERMINATE_RCVD, &cmd );
  }

  return ZCL_STATUS_SUCCESS;
}

/**************************************************************************************************
 * @fn      zclKE_ClientHdlSpecificCmd
 *
 * @brief   Client ZCL specific command handler.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_ClientHdlSpecificCmd( zclIncoming_t *pInMsg )
{
  ZStatus_t status;
  zclKE_ConnCtxt_t ctxt;

  // Initialize connection context
  ctxt.pInMsg = pInMsg;
  ctxt.pConn = NULL;
  ctxt.error = 0;

  // Process the command
  switch ( pInMsg->hdr.commandID )
  {
    case ZCL_KE_INITIATE_RSP:
      status = zclKE_ClientHdlInitiateRsp( &ctxt );
      break;

    case ZCL_KE_EPH_DATA_RSP:
      status = zclKE_ClientHdlEphDataRsp( &ctxt );
      break;

    case ZCL_KE_CFM_KEY_DATA_RSP:
      status = zclKE_ClientHdlCfmKeyDataRsp( &ctxt );
      break;

    case ZCL_KE_TERMINATE_CLIENT:
      status = zclKE_ClientHdlTerminate( &ctxt );
      break;

    // Unknown command
    default:
      status = ZCL_STATUS_FAILURE;
      break;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_ClientKeyGenMsg
 *
 * @brief   Process client ZCL_KE_KEY_GEN_MSG.
 *
 * @param   pMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ClientKeyGenMsg( zclKE_KeyGenMsg_t *pMsg )
{
  zclKE_Conn_t *pConn;

  // Find the existing connection
  pConn = zclKE_ClientConnFind( pMsg->partnerAddr );

  // Check for connection and state
  if ( pConn && ( pConn->state == ZCL_KE_CLIENT_CONN_KEY_GEN_QUEUED ) )
  {
    zclKE_ConnCtxt_t ctxt;

    ctxt.pInMsg = NULL;
    ctxt.pConn = pConn;
    ctxt.error = 0;

    zclKE_ClientProcessKeyGen( &ctxt );

    // Check for failure and terminate connection
    if ( ctxt.error )
    {
      zclKE_ClientConnTerminate( &ctxt ); 
    }
  }
}

#if defined( ZCL_READ )
/**************************************************************************************************
 * @fn      zclKE_ClientReadRspCmd
 *
 * @brief   ZCL_CMD_READ_RSP handler.
 *
 * @param   pPartner - partner address
 * @param   clusterID - cluster ID of read
 * @param   pCmd - zclReadRspCmd_t
 *
 * @return  void
 */
static void zclKE_ClientReadRspCmd( afAddrType_t *pPartner, uint16 clusterID, 
                                    zclReadRspCmd_t *pCmd  )
{
  zclKE_Conn_t *pConn;
  uint16 suites;
  uint16 selected;
  uint8 *pBuf;

  // Find the existing connection
  pConn = zclKE_ClientConnFind( pPartner->addr.shortAddr );

  // Check for valid connection and state
  if ( ( pConn                                            ) && 
       ( pConn->partner.endPoint == pPartner->endPoint    ) &&
       ( pConn->state == ZCL_KE_CLIENT_CONN_READ_RSP_WAIT )    )
  {
    // Check for valid ATTRID_KE_SUITE attribute
    if ( ( clusterID == ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT  ) &&
         ( pCmd->numAttr == 1                                ) &&
         ( pCmd->attrList[0].attrID == ATTRID_KE_SUITE       ) &&
         ( pCmd->attrList[0].status == ZCL_STATUS_SUCCESS    ) &&
         ( pCmd->attrList[0].dataType == ZCL_DATATYPE_ENUM16 )    )
    {
      pBuf = pCmd->attrList[0].data;

      // Get the supported suites
      suites = BUILD_UINT16( pBuf[0], pBuf[1] );
    }
    else
    {
      // Invalid attribute -- just try using device's lowest supported suite
      if ( zclKE_SupportedSuites & ZCL_KE_SUITE_1 )
      {
        suites = ZCL_KE_SUITE_1;
      }
      else // if ( suites & ZCL_KE_SUITE_2 )
      {
        suites = ZCL_KE_SUITE_2;
      }
    } 

    // Select highest supported suite
    if ( ( suites & ZCL_KE_SUITE_2                ) &&
         ( zclKE_SupportedSuites & ZCL_KE_SUITE_2 )    )
    {
      selected = ZCL_KE_SUITE_2;
    }
    else if ( ( suites & ZCL_KE_SUITE_1                ) &&
              ( zclKE_SupportedSuites & ZCL_KE_SUITE_1 )    )
    {
      selected = ZCL_KE_SUITE_1;
    }
    else
    {
      selected = 0;
    }

    // Check if suite selected
    if ( selected )
    {
      zclKE_ConnCtxt_t ctxt = {0, NULL, pConn};

      pConn->suite = selected;

      // Start key establishment process
      if ( !zclKE_ClientProcessStart( &ctxt ) )
      {  
        // Only possible error condition would be limited resources so return busy
        // (ctxt.error == ZCL_KE_TERMINATE_NO_RESOURCES)
        zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_BUSY, NULL );
      }
    }
    else
    {
      zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_NO_SUITE_MATCH, NULL ); 
    }
  }
}
#endif // ZCL_READ

/**************************************************************************************************
 * @fn      zclKE_HdlSpecificCmd
 *
 * @brief   ZCL specific command handler.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_HdlSpecificCmd( zclIncoming_t *pInMsg )
{
  ZStatus_t status;

  if ( zcl_ServerCmd( pInMsg->hdr.fc.direction ) )
  {
    // Process Client-to-Server Commands
    status = zclKE_ServerHdlSpecificCmd( pInMsg );
  }
  else
  {
    // Process Server-to-Client Commands
    status = zclKE_ClientHdlSpecificCmd( pInMsg );
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming cluster specific commands
 *          for this cluster library.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  ZStatus_t - status
 */
static ZStatus_t zclKE_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t status = ZCL_STATUS_SUCCESS;

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
  {
    return status; // Cluster not supported thru Inter-PAN
  }
#endif
  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Check for manufacturer specific command
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {
      status = zclKE_HdlSpecificCmd( pInMsg );
    }
    else
    {
      // Manufacturer specific command not handled
      status = ZCL_STATUS_FAILURE;
    }
  }
  else
  {
    // Should never get here
    status = ZCL_STATUS_FAILURE;
  }

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_ProcessKeyGenMsg
 *
 * @brief   Process ZCL_KE_KEY_GEN_MSG.
 *
 * @param   pMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ProcessKeyGenMsg( zclKE_KeyGenMsg_t *pMsg )
{
  if ( pMsg->server )
  {
    zclKE_ServerKeyGenMsg( pMsg );
  }
  else
  {
    zclKE_ClientKeyGenMsg( pMsg );
  }
}

/**************************************************************************************************
 * @fn      zclKE_ProcessAFMsgCmd
 *
 * @brief   Process AF_INCOMING_MSG_CMD.
 *
 * @param   pCmd - incoming command to process
 *
 * @return  void
 */
static void zclKE_ProcessAFMsgCmd( afIncomingMSGPacket_t *pCmd )
{
#ifdef ZCL_STANDALONE
  // Forward command to ZCL
  zcl_ProcessMessageMSG( pCmd );
#endif
  (void)pCmd;
}

/**************************************************************************************************
 * @fn      zclKE_ProcessZCLMsg
 *
 * @brief   Process ZCL_INCOMING_MSG.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ProcessZCLMsg( zclIncomingMsg_t *pInMsg )
{
  if ( pInMsg->zclHdr.commandID == ZCL_CMD_READ_RSP )
  {
#if defined( ZCL_READ )
    zclKE_ClientReadRspCmd( &pInMsg->srcAddr, pInMsg->clusterId, 
                            (zclReadRspCmd_t *)pInMsg->attrCmd  );
#endif // ZCL_READ
  }

  if ( pInMsg->attrCmd != NULL )
  {
    osal_mem_free( pInMsg->attrCmd );
    pInMsg->attrCmd = NULL;
  }
}

/**************************************************************************************************
 * @fn      zclKE_ProcessZDOMsg
 *
 * @brief   Process ZDO_CB_MSG.
 *
 * @param   pMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ProcessZDOMsg( zdoIncomingMsg_t *pMsg )
{
  if ( pMsg->clusterID == Match_Desc_rsp )
  {
    zclKE_Conn_t *pConn;

    // Find the existing connection
    pConn = zclKE_ClientConnFind( pMsg->srcAddr.addr.shortAddr );

    // Check for connection and state
    if ( pConn && ( pConn->state == ZCL_KE_CLIENT_CONN_MATCH_RSP_WAIT ) )
    {
      ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( pMsg );

      if ( pRsp )
      {
        if ( pRsp->cnt )
        {
          // Process the match response 
          if ( !zclKE_ClientProcessMatchRsp( pConn, pRsp->epList[0] ) )
          {
            // Resource failure 
            zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_BUSY, NULL );  
          } 
        }
        else
        {
          zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_NO_EP_MATCH, NULL );  
        }

        osal_mem_free( pRsp );
      }
    }
  }
}

/**************************************************************************************************
 * @fn      zclKE_ProcessTimerEvt
 *
 * @brief   Process timer.
 *
 * @param   none
 *
 * @return  void
 */
static void zclKE_ProcessTimerEvt( void )
{
  uint32 current = osal_GetSystemClock();
  uint32 elapsed;
  uint32 nextTimer = 0;
  uint32 timeout;
  zclKE_Conn_t *pNext;
  zclKE_Conn_t *pCurr;

  // Process server connections
  pCurr = zclKE_ServerConnList;

  while ( pCurr )
  {
    // Save next connection since current connection may be closed
    pNext = pCurr->pNext;

    // Is this connection waiting for a timeout
    if ( pCurr->timeout )
    {
      // Get elapsed time since last timeout
      elapsed = current - pCurr->stamp;

      if ( pCurr->timeout <= elapsed )
      {
        // Handle connection timeout
        zclKE_ServerConnTimeout( pCurr );
      }
      else
      {
        // Adjust next timeout
        timeout = pCurr->timeout - elapsed;

        if ( !nextTimer || ( timeout < nextTimer ) )
        {
          nextTimer = timeout;
        }
      }
    }

    // Set next connection to current
    pCurr = pNext;
  }

  // Process client connections
  pCurr = zclKE_ClientConnList;

  while ( pCurr )
  {
    // Save next connection since current connection may be closed
    pNext = pCurr->pNext;

    // Is this connection waiting for a timeout
    if ( pCurr->timeout )
    {
      // Get elapsed time since last timeout
      elapsed = current - pCurr->stamp;

      if ( pCurr->timeout <= elapsed )
      {
        // Handle connection timeout
        zclKE_ClientConnTimeout( pCurr );
      }
      else
      {
        // Adjust next timeout
        timeout = pCurr->timeout - elapsed;

        if ( !nextTimer || ( timeout < nextTimer ) )
        {
          nextTimer = timeout;
        }
      }
    }

    // Set next connection to current
    pCurr = pNext;
  }

  // Start the timer based on the next timeout
  zclKE_StartTimer( nextTimer );
}

/**************************************************************************************************
 * @fn      zclKE_ProcessStartMsg
 *
 * @brief   Start key establishment *WITH* end point discovery and suite selection.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ProcessStartMsg( zclKE_StartMsg_t *pMsg )
{
  zclKE_Conn_t *pConn;
  afAddrType_t partner = {0};

  partner.addr.shortAddr = pMsg->partnerNwkAddr;
  partner.addrMode = afAddr16Bit;

  // Create a new client connection
  if ( zclKE_ClientConnOpen( pMsg->taskID,
                             &partner,
                             pMsg->transSeqNum,
                             zclKE_SupportedSuites, 
                             &pConn ) )
  {
    if ( !zclKE_ClientProcessMatchReq( pConn ) )
    {
      // Only possible error condition would be limited resources so return busy
      // (ctxt.error == ZCL_KE_TERMINATE_NO_RESOURCES)
      zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_BUSY, NULL );
    }
  }
}

/**************************************************************************************************
 * @fn      zclKE_ProcessStartDirectMsg
 *
 * @brief   Start key establishment *WITHOUT* end point discovery and suite selection.
 *
 * @param   pMsg - incoming message to process
 *
 * @return  void
 */
static void zclKE_ProcessStartDirectMsg( zclKE_StartDirectMsg_t *pMsg )
{
  zclKE_Conn_t *pConn;

  // Open a new client connection
  if ( zclKE_ClientConnOpen( pMsg->taskID,
                             &pMsg->partnerAddr,
                             pMsg->transSeqNum,
                             pMsg->suite,
                             &pConn ) )
  {
    zclKE_ConnCtxt_t ctxt = {0, NULL, pConn};

    if ( !zclKE_ClientProcessStart( &ctxt ) )
    {  
      // Only possible error condition would be limited resources so return busy
      // (ctxt.error == ZCL_KE_TERMINATE_NO_RESOURCES)
      zclKE_ClientConnClose( pConn, ZCL_KE_NOTIFY_BUSY, NULL );
    }
  }
}


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
void zclKE_HdlGeneralCmd( zclIncoming_t *pInMsg )
{
  if ( pInMsg->hdr.commandID == ZCL_CMD_READ_RSP )
  {
#if defined( ZCL_READ )
    zclKE_ClientReadRspCmd( &pInMsg->msg->srcAddr, pInMsg->msg->clusterId,
                            (zclReadRspCmd_t *)pInMsg->attrCmd  );
#endif // ZCL_READ
  }
}

/**************************************************************************************************
 * @fn      zclKE_ECDSASignGetLen
 *
 * @brief   Returns length required for zclKE_ECDSASign "pOutBuf" field.
 *
 * @param   suite - selected security suite
 *
 * @return  uint8 - length for zclKE_ECDSASign "pOutBuf" field
 */
uint8 zclKE_ECDSASignGetLen( uint16 suite )
{
  uint8 len;

  len = (uint8)( zclKE_GetField( suite, ZCL_KE_PRIVATE_KEY_LEN ) * 2 );

  return len;
}

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
ZStatus_t zclKE_ECDSASign( uint16 suite, uint8 *pInBuf, uint8 inBufLen, uint8 *pOutBuf )
{
  uint8 status;
  uint8 result;
  uint8 msgDigest[ZCL_KE_MAC_LEN];
  uint16 bitLen = inBufLen * 8;
  uint8 *pPrivateKey = NULL;
  uint16 privateKeyLen;

  do
  {
    privateKeyLen = zclKE_GetField( suite, ZCL_KE_PRIVATE_KEY_LEN );

    pPrivateKey = osal_mem_alloc( privateKeyLen );
    if ( !pPrivateKey )
    {
      status = ZMemError;
      break;
    }

    osal_nv_read( zclKE_GetField( suite, ZCL_KE_PRIVATE_KEY_NV_ID ), 0,
                  privateKeyLen, pPrivateKey );

    // First hash the input buffer
    sspMMOHash(NULL, 0, pInBuf, bitLen, msgDigest);

    switch ( suite )
    {
#if !defined( ECCAPI_163_DISABLED )
      case ZCL_KE_SUITE_1:
        result = ZSE_ECDSASign( pPrivateKey, msgDigest, zclKE_GetRandom, 
                                pOutBuf, &pOutBuf[privateKeyLen], NULL, 0 );
        break;
#endif // !defined( ECCAPI_163_DISABLED )

#if !defined( ECCAPI_283_DISABLED )
      case ZCL_KE_SUITE_2:
        result = ZSE_ECDSASign283( pPrivateKey, msgDigest, zclKE_GetRandom, 
                                   pOutBuf, &pOutBuf[privateKeyLen], NULL, 0 );
        break;
#endif // !defined( ECCAPI_283_DISABLED )

      default:
        // Should never get here
        result = MCE_ERR_BAD_INPUT;
        break;
    }

    if( result != MCE_SUCCESS )
    {
      status = ZFailure;
      break;
    }

    status = ZSuccess;

  } while ( 0 );

  // Cleanup local memory allocations
  zclKE_MemFree( pPrivateKey, privateKeyLen );

  return status;
}

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
ZStatus_t zclKE_Start( uint8 taskID, uint16 partnerNwkAddr, uint8 transSeqNum )
{
  ZStatus_t status;
  zclKE_StartMsg_t *pMsg;

  // Allocate the message
  pMsg = (zclKE_StartMsg_t *)osal_msg_allocate( sizeof( zclKE_StartMsg_t ) );

  if ( pMsg )
  {
    pMsg->hdr.event = ZCL_KE_START_MSG;
    pMsg->hdr.status = 0;
    pMsg->taskID = taskID;
    pMsg->partnerNwkAddr = partnerNwkAddr;
    pMsg->transSeqNum = transSeqNum;
    status = osal_msg_send( zclKE_TaskID, (uint8 *)pMsg );
  }
  else
  {
    status = ZMemError;
  }  

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_StartDirect
 *
 * @brief   Start key establishment directly with partner at the pPartnerAddr.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   pPartnerAddr - valid partner network address and end point
 * @param   transSeqNum - starting transaction sequence number
 * @param   suite - selected security suite
 *
 * @return  ZStatus_t - status
 */
ZStatus_t zclKE_StartDirect( uint8 taskID, afAddrType_t *pPartnerAddr,
                             uint8 transSeqNum, uint16 suite )
{
  ZStatus_t status;
  zclKE_StartDirectMsg_t *pMsg;

  // Allocate the message
  pMsg = (zclKE_StartDirectMsg_t *)osal_msg_allocate( sizeof( zclKE_StartDirectMsg_t ) );

  if ( pMsg )
  {
    pMsg->hdr.event = ZCL_KE_START_DIRECT_MSG;
    pMsg->hdr.status = 0;
    pMsg->taskID = taskID;
    pMsg->partnerAddr = *pPartnerAddr;
    pMsg->transSeqNum = transSeqNum;
    pMsg->suite = suite;
    status = osal_msg_send( zclKE_TaskID, (uint8 *)pMsg );
  }
  else
  {
    status = ZMemError;
  }  

  return status;
}

/**************************************************************************************************
 * @fn      zclKE_Init
 *
 * @brief   Initialization function for the ZCL key establishment task.
 *
 * @param   taskID - OSAL task ID of this task
 *
 * @return  void
 */
void zclKE_Init( uint8 taskID )
{
  zclKE_TaskID = taskID;

  // Register end point description
  afRegister( (endPointDesc_t* )&zclKE_EPDesc );

  // Register ZCL attribute list
  zcl_registerAttrList( ZCL_KE_ENDPOINT, 1, zclKE_ZCL_AttrList );

  // Register ZCL cluster options
  zcl_registerClusterOptionList( ZCL_KE_ENDPOINT, 1, zclKE_ZCL_Options );

  // Register as a ZCL KE plugin
  zcl_registerPlugin( ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
                      ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT,
                      zclKE_HdlIncoming );

#ifndef ZCL_STANDALONE
  // Register for ZCL_INCOMING_MSG
  zcl_registerForMsg( zclKE_TaskID );
#endif

  // Initialize available suites
  zclKE_CheckForAvailSuites();

  // Initialize server functionality
  zclKE_ServerInit();

  // Initialize client functionality
  zclKE_ClientInit();

  // Set initial state
  if ( zclKE_SupportedSuites )
  {
    zclKE_State = ZCL_KE_READY;
  }
  else
  {
    zclKE_State = ZCL_KE_NO_CERTS;
  }
}

/**************************************************************************************************
 * @fn      zclKE_ProcessEvent
 *
 * @brief   Process an event for the ZCL key establishment task.
 *
 * @param   taskID - OSAL task ID of this task
 * @param   events - OSAL event mask
 *
 * @return  uint16 - OSAL events not processed
 */
uint16 zclKE_ProcessEvent( uint8 taskID, uint16 events )
{
  osal_event_hdr_t *pMsg;

  if ( events & SYS_EVENT_MSG )
  {
    pMsg = (osal_event_hdr_t *)osal_msg_receive( taskID );

    if ( pMsg )
    {
      switch ( pMsg->event )
      {
        case ZCL_KE_START_MSG:
          zclKE_ProcessStartMsg( (zclKE_StartMsg_t *)pMsg );
          break;

        case ZCL_KE_START_DIRECT_MSG:
          zclKE_ProcessStartDirectMsg( (zclKE_StartDirectMsg_t *)pMsg );
          break;

        case ZCL_KE_KEY_GEN_MSG:
          zclKE_ProcessKeyGenMsg( (zclKE_KeyGenMsg_t *)pMsg );
          break;

        case AF_INCOMING_MSG_CMD:
          zclKE_ProcessAFMsgCmd( (afIncomingMSGPacket_t *)pMsg );
          break;

        case ZCL_INCOMING_MSG:
          zclKE_ProcessZCLMsg( (zclIncomingMsg_t *)pMsg );
          break;

        case ZDO_CB_MSG:
          zclKE_ProcessZDOMsg( (zdoIncomingMsg_t *)pMsg );
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)pMsg );
    }

    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & ZCL_KE_TIMER_EVT )
  {
    zclKE_ProcessTimerEvt();

    return ( events ^ ZCL_KE_TIMER_EVT );
  }

  // Discard unknown events
  return 0;
}


/**************************************************************************************************
**************************************************************************************************/
