/**************************************************************************************************
  Filename:       bdb_touchlink.c
  Revised:        $Date: 2013-12-06 15:53:38 -0800 (Fri, 06 Dec 2013) $
  Revision:       $Revision: 36460 $

  Description:    Zigbee Cluster Library - Light Link Profile.


  Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */
#include "OSAL_Nv.h"
#include "hal_aes.h"
#include "ssp_hash.h"
#include "nwk_util.h"
#include "ZDSecMgr.h"
#include "ZDObject.h"

#if defined( INTER_PAN )
  #include "stub_aps.h"
#if defined ( BDB_TL_INITIATOR )
  #include "bdb_touchlink_initiator.h"
#endif // BDB_TL_INITIATOR
#if defined ( BDB_TL_TARGET )
  #include "bdb_touchlink_target.h"
#endif // BDB_TL_TARGET
#endif

#include "bdb.h"
#include "bdb_interface.h"
#include "bdb_tlCommissioning.h"
#include "bdb_touchlink.h"

#if defined ( BDB_TL_TARGET ) || defined ( BDB_TL_INITIATOR )

/*********************************************************************
 * MACROS
 */
#define TOUCHLINK_NEW_MIN( min, max )                  ( ( (uint32)(max) + (uint32)(min) + 1 ) / 2 )

/*********************************************************************
 * CONSTANTS
 */

#define TOUCHLINK_NUM_DEVICE_INFO_ENTRIES              5


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint32 touchLinkLastAcceptedTransID;
   
// Used for Network Discovery
touchLinkDiscoveredNwkParam_t *pDiscoveredNwkParamList = NULL;

// Current Touch Link Transaction ID
uint32 touchLinkTransID;

// Scan Response ID
uint32 touchLinkResponseID;

// Our group ID range
uint16 touchLinkGrpIDsBegin;
uint16 touchLinkGrpIDsEnd;

// Flag for leave
uint8 touchLinkLeaveInitiated;

// Device Information Table
bdbTLDeviceInfo_t *touchLinkSubDevicesTbl[5];

// Touchlink distributed network flag
bool touchlinkDistNwk = FALSE;

bool touchlinkFNReset;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// TOUCHLINK Profile attributes - Our free network address and group ID ranges
static uint16 touchLinkFreeNwkAddrBegin;
static uint16 touchLinkFreeNwkAddrEnd;
static uint16 touchLinkFreeGrpIdBegin;
static uint16 touchLinkFreeGrpIdEnd;

static bool touchLinkIsInitiator;
static uint8 touchLinkTaskId;

// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define TOUCHLINK_EP_MAX_INCLUSTERS       1
static const cId_t touchLink_EP_InClusterList[TOUCHLINK_EP_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_TOUCHLINK
};

#define TOUCHLINK_EP_MAX_OUTCLUSTERS       1
static const cId_t touchLink_EP_OutClusterList[TOUCHLINK_EP_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_TOUCHLINK
};

static SimpleDescriptionFormat_t touchLink_EP_SimpleDesc =
{
  TOUCHLINK_INTERNAL_ENDPOINT,         //  int Endpoint;
  TOUCHLINK_PROFILE_ID,                //  uint16 AppProfId[2];
  TOUCHLINK_INTERNAL_DEVICE_ID,        //  uint16 AppDeviceId[2];
  TOUCHLINK_DEVICE_VERSION,            //  int   AppDevVer:4;
  TOUCHLINK_INTERNAL_FLAGS,            //  int   AppFlags:4;
  TOUCHLINK_EP_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)touchLink_EP_InClusterList, //  byte *pAppInClusterList;
  TOUCHLINK_EP_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)touchLink_EP_OutClusterList //  byte *pAppInClusterList;
};

#if defined( INTER_PAN )
// Define endpoint structure to register with STUB APS for INTER-PAN support
static endPointDesc_t touchLink_EP =
{
  TOUCHLINK_INTERNAL_ENDPOINT,
  0,
  &touchLinkTaskId,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};
#endif


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void touchLink_BuildAesKey( uint8 *pAesKey, uint32 transID, uint32 responseID, uint8 keyIndex );
void touchLink_ItemInit( uint16 id, uint16 len, void *pBuf );
static void *touchLink_BeaconIndCB ( void *param );
static void *touchLink_NwkDiscoveryCnfCB ( void *param );
static void touchLink_InitNV( void );

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLink_InitVariables
 *
 * @brief   Initialize the TOUCHLINK global and local variables.
 *
 * @param   initiator - if caller is Initiator
 *
 * @return  none
 */
void touchLink_InitVariables( bool initiator )
{
  touchLinkTransID = 0;
  touchLinkIsInitiator = initiator;

  if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    _NIB.nwkDevAddress = INVALID_NODE_ADDR;
  }

  // verify groups communication is initiated by broadcasts rather than multicasts
  _NIB.nwkUseMultiCast = FALSE;
  // detect and remove stored deprecated end device children after power up
  zgRouterOffAssocCleanup = TRUE;
  osal_nv_write(ZCD_NV_ROUTER_OFF_ASSOC_CLEANUP, 0, sizeof(zgRouterOffAssocCleanup), &zgRouterOffAssocCleanup);

  touchLink_InitFreeRanges( initiator );

  touchLink_InitNV();

  touchLinkLeaveInitiated = FALSE;

  // Initialize device info table
  osal_memset( touchLinkSubDevicesTbl, 0, sizeof( touchLinkSubDevicesTbl ) );
  
  touchLinkIsInitiator = FALSE;
  
  // set broadcast address mask to support broadcast filtering
  NLME_SetBroadcastFilter( ZDO_Config_Node_Descriptor.CapabilityFlags );
}

/*********************************************************************
 * @fn      touchLink_InitFreeRanges
 *
 * @brief   Initialize the TOUCHLINK free range global variables.
 *
 * @param   initiator - if caller is link initiator
 *
 * @return  none
 */
void touchLink_InitFreeRanges( bool initiator )
{
  // Initialize our free network address and group ID ranges
  if ( initiator )
  {
    touchLinkFreeNwkAddrBegin = APL_FREE_NWK_ADDR_RANGE_BEGIN;
    touchLinkFreeNwkAddrEnd = APL_FREE_NWK_ADDR_RANGE_END;

    touchLinkFreeGrpIdBegin = APL_FREE_GROUP_ID_RANGE_BEGIN;
    touchLinkFreeGrpIdEnd = APL_FREE_GROUP_ID_RANGE_END;
  }
  else
  {
    touchLinkFreeNwkAddrBegin = touchLinkFreeNwkAddrEnd = 0;
    touchLinkFreeGrpIdBegin = touchLinkFreeGrpIdEnd = 0;
  }

  // Initialize our local group ID range
  touchLinkGrpIDsBegin = touchLinkGrpIDsEnd = 0;
}

/*********************************************************************
 * @fn      touchLink_UpdateFreeRanges
 *
 * @brief   Update the TOUCHLINK free range global variables.
 *
 * @param   pParams - new parameters
 *
 * @return  none
 */
void touchLink_UpdateFreeRanges( bdbTLNwkParams_t *pParams )
{
  // Set our free network address and group ID ranges
  touchLinkFreeNwkAddrBegin = pParams->freeNwkAddrBegin;
  touchLinkFreeNwkAddrEnd = pParams->freeNwkAddrEnd;
  touchLinkFreeGrpIdBegin = pParams->freeGrpIDBegin;
  touchLinkFreeGrpIdEnd = pParams->freeGrpIDEnd;

  // Set our group ID range
  touchLinkGrpIDsBegin = pParams->grpIDsBegin;
  touchLinkGrpIDsEnd = pParams->grpIDsEnd;
}

/*********************************************************************
 * @fn      touchLink_GerFreeRanges
 *
 * @brief   Get the TOUCHLINK free range global variables.
 *
 * @param   pParams - parameter to get
 *
 * @return  none
 */
void touchLink_GerFreeRanges( bdbTLNwkParams_t *pParams )
{
  // Set our free network address and group ID ranges
  pParams->freeNwkAddrBegin = touchLinkFreeNwkAddrBegin;
  pParams->freeNwkAddrEnd = touchLinkFreeNwkAddrEnd;
  pParams->freeGrpIDBegin = touchLinkFreeGrpIdBegin;
  pParams->freeGrpIDEnd = touchLinkFreeGrpIdEnd;
  
    // Set our group ID range
  pParams->grpIDsBegin = touchLinkGrpIDsBegin;
  pParams->grpIDsEnd = touchLinkGrpIDsEnd;
}

/*********************************************************************
 * @fn      touchLink_IsValidSplitFreeRanges
 *
 * @brief   Checks to see if the resulting two ranges are not smaller
 *          than the threshold after division of a network address or
 *          group ID range. The Initiator splits its own free range
 *          in half and assigns the top half to the new device.
 *
 *          Note: A range (Nmin...Nmax) is split as follows:
 *
 *                N'min = (Nmax + Nmin + 1)/2
 *                N'max = Nmax
 *                Nmax = N'min - 1
 *
 * @param   totalGrpIDs - total number of group IDs needed
 *
 * @return  TRUE if split possible. FALSE, otherwise.
 */
bool touchLink_IsValidSplitFreeRanges( uint8 totalGrpIDs )
{
  if ( ( touchLinkFreeNwkAddrBegin != 0 ) && ( touchLinkFreeGrpIdBegin != 0 ) )
  {
      return ( ( ( ( touchLinkFreeNwkAddrEnd - touchLinkFreeNwkAddrBegin ) / 2 ) >= APL_FREE_ADDR_THRESHOLD ) &&
               ( ( ( touchLinkFreeGrpIdEnd - ( touchLinkFreeGrpIdBegin + totalGrpIDs - 1 ) ) / 2 ) >= APL_FREE_GROUP_ID_THRESHOLD ) );

  }

  return ( FALSE );
}

/*********************************************************************
 * @fn      touchLink_SplitFreeRanges
 *
 * @brief   Split our own free network address and group ID ranges
 *          in half and assign the top half to the new device.
 *
 *          Note: A range (Nmin...Nmax) is split as follows:
 *
 *                N'min = (Nmax + Nmin + 1)/2
 *                N'max = Nmax
 *                Nmax = N'min - 1
 *
 * output parameters
 *
 * @param   pAddrBegin - new address range begin
 * @param   pAddrEnd - new address range end
 * @param   pGrpIdBegin - new group id range begin
 * @param   pGrpIdEnd - new group id range end
 *
 * @return  none
 */
void touchLink_SplitFreeRanges( uint16 *pAddrBegin, uint16 *pAddrEnd,
                        uint16 *pGrpIdBegin, uint16 *pGrpIdEnd )
{
  if ( ( touchLinkFreeNwkAddrBegin != 0 ) && ( touchLinkFreeGrpIdBegin != 0 ) )
  {
    *pAddrBegin = TOUCHLINK_NEW_MIN( touchLinkFreeNwkAddrBegin, touchLinkFreeNwkAddrEnd );
    *pGrpIdBegin = TOUCHLINK_NEW_MIN( touchLinkFreeGrpIdBegin, touchLinkFreeGrpIdEnd );

    *pAddrEnd = touchLinkFreeNwkAddrEnd;
    *pGrpIdEnd = touchLinkFreeGrpIdEnd;

    // Update our max
    touchLinkFreeNwkAddrEnd = *pAddrBegin - 1;
    touchLinkFreeGrpIdEnd = *pGrpIdBegin - 1;
  }
  else
  {
    *pAddrBegin = *pAddrEnd = 0;
    *pGrpIdBegin = *pGrpIdEnd = 0;
  }
}

/*********************************************************************
 * @fn      touchLink_PopGrpIDRange
 *
 * @brief   Pop the requested number of group IDs out of the free group IDs range.
 *
 * input parameters
 *
 * @param   numGrpIDs - number of group IDs needed
 *
 * output parameters
 *
 * @param   pGrpIdBegin - new group id range begin, or 0 if unavaialable
 * @param   pGrpIdEnd - new group id range end, , or 0 if unavaialable
 *
 * @return  none
 */
void touchLink_PopGrpIDRange( uint8 numGrpIDs, uint16 *pGrpIDsBegin, uint16 *pGrpIDsEnd )
{
  if ( ( touchLinkFreeGrpIdBegin != 0 )
       && ( touchLinkFreeGrpIdBegin <= touchLinkFreeGrpIdEnd )
       && ( ( touchLinkFreeGrpIdEnd - touchLinkFreeGrpIdBegin ) >= numGrpIDs ) )
  {
    *pGrpIDsBegin = touchLinkFreeGrpIdBegin;

    // Update min free group id
    touchLinkFreeGrpIdBegin += numGrpIDs;

    *pGrpIDsEnd = touchLinkFreeGrpIdBegin - 1;
  }
  else
  {
    *pGrpIDsBegin = *pGrpIDsEnd = 0;
  }
}

/*********************************************************************
 * @fn      touchLink_PopNwkAddress
 *
 * @brief   Pop an avaialble short address out of the free network addresses range.
 *
 * @param   none
 *
 * @return  free address if available. 0, otherwise.
 */
uint16 touchLink_PopNwkAddress( void )
{
  if ( ( touchLinkFreeNwkAddrBegin == 0 ) || ( touchLinkFreeNwkAddrBegin > touchLinkFreeNwkAddrEnd ) )
  {
    return ( 0 );
  }

  return ( touchLinkFreeNwkAddrBegin++ );
}

/*********************************************************************
 * @fn      touchLink_GetNumSubDevices
 *
 * @brief   Get the total number of sub-devices (endpoints) registered.
 *
 * @param   startIndex - starting index
 *
 * @return  number of sub-devices
 */
uint8 touchLink_GetNumSubDevices( uint8 startIndex )
{
  uint8 numEPs = 0;

  for ( uint8 i = startIndex; i < TOUCHLINK_NUM_DEVICE_INFO_ENTRIES; i++ )
  {
    if ( touchLinkSubDevicesTbl[i] != NULL )
    numEPs++;
  }

  return ( numEPs );
}

/*********************************************************************
 * @fn      touchLink_GetNumGrpIDs
 *
 * @brief   Get the total number of group IDs required by this device.
 *
 * @param   none
 *
 * @return  number of group IDs
 */
uint8 touchLink_GetNumGrpIDs( void )
{
  uint8 numGrpIDs = 0;

  for ( uint8 i = 0; i < TOUCHLINK_NUM_DEVICE_INFO_ENTRIES; i++ )
  {
    if ( touchLinkSubDevicesTbl[i] != NULL )
    {
      numGrpIDs += touchLinkSubDevicesTbl[i]->grpIdCnt;
    }
  }

  return ( numGrpIDs );
}

/*********************************************************************
 * @fn      touchLink_GetSubDeviceInfo
 *
 * @brief   Get the sub-device information.
 *
 * input parameter
 *
 * @param   index - index of sub-device
 *
 * output parameter
 *
 * @param   pInfo - sub-device info (to be returned)
 *
 * @return  none
 */
void touchLink_GetSubDeviceInfo( uint8 index, bdbTLDeviceInfo_t *pInfo )
{
  if ( pInfo == NULL )
  {
    return;
  }
  if ( ( index < TOUCHLINK_NUM_DEVICE_INFO_ENTRIES ) &&
       ( touchLinkSubDevicesTbl[index] != NULL ) )
  {
    endPointDesc_t *epDesc = afFindEndPointDesc( touchLinkSubDevicesTbl[index]->endpoint );
    if ( epDesc != NULL )
    {
      // Copy sub-device info
      *pInfo = *(touchLinkSubDevicesTbl[index]);
    }
  }
  else
  {
    osal_memset( pInfo, 0, sizeof( bdbTLDeviceInfo_t ) );
  }
}


/*********************************************************************
 * @fn      touchLink_EncryptNwkKey
 *
 * @brief   Encrypt the current network key to be sent to a Target.
 *          In case of Factory New device generate new key.
 *
 * output parameter
 *
 * @param   pNwkKey - pointer to encrypted network key
 *
 * input parameters
 *
 * @param   keyIndex - key index
 * @param   transID - transaction id
 * @param   responseID - response id
 *
 * @return  none
 */
void touchLink_EncryptNwkKey( uint8 *pNwkKey, uint8 keyIndex, uint32 transID, uint32 responseID )
{
  uint8 aesKeyKey[SEC_KEY_LEN] = TOUCHLINK_DEFAULT_AES_KEY;
  uint8 masterKey[SEC_KEY_LEN] = TOUCHLINK_ENC_KEY;
  uint8 nwkKey[SEC_KEY_LEN];

  if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    touchLink_GenerateRandNum( nwkKey, SEC_KEY_LEN );
  }
  else
  {
    nwkActiveKeyItems keyItems;
    SSP_ReadNwkActiveKey( &keyItems );
    osal_memcpy( nwkKey, keyItems.active.key , SEC_KEY_LEN);
  }

  // Build the AES key
  touchLink_BuildAesKey( aesKeyKey, transID, responseID, keyIndex );

  if ( ( keyIndex == TOUCHLINK_KEY_INDEX_MASTER ) || ( keyIndex == TOUCHLINK_KEY_INDEX_CERT ) )
  {
    // Encypt with the master key
    sspAesEncrypt( masterKey, aesKeyKey );
  }
  // Encrypt the network key with the AES key
  sspAesEncrypt( aesKeyKey, nwkKey );

  // Copy in the encrypted network key
  osal_memcpy( pNwkKey, nwkKey, SEC_KEY_LEN );
}

/*********************************************************************
 * @fn      touchLink_DecryptNwkKey
 *
 * @brief   Decrypt the received network key and update.
 *
 * @param   pNwkKey - pointer to the encrypted network key
 * @param   keyIndex - key index
 * @param   transID - transaction id
 * @param   responseID - response id
 *
 * @return  none
 */
void touchLink_DecryptNwkKey( uint8 *pNwkKey, uint8 keyIndex, uint32 transID, uint32 responseID )
{
  uint8 aesKeyKey[SEC_KEY_LEN] = TOUCHLINK_DEFAULT_AES_KEY;

  uint8 nwkKey[SEC_KEY_LEN];

  uint8 masterKey[SEC_KEY_LEN] = TOUCHLINK_ENC_KEY;

  // Copy in the encrypted network key
  osal_memcpy( nwkKey, pNwkKey, SEC_KEY_LEN );

  touchLink_BuildAesKey( aesKeyKey, transID, responseID, keyIndex );

  if ( ( keyIndex == TOUCHLINK_KEY_INDEX_MASTER ) || ( keyIndex == TOUCHLINK_KEY_INDEX_CERT ) )
  {
    //encypt with the master key
    sspAesEncrypt( masterKey, aesKeyKey );
  }
  // Decrypt the network key with the AES key
  sspAesDecrypt( aesKeyKey, nwkKey );

  touchLink_UpdateNwkKey( nwkKey, keyIndex );
}

/*********************************************************************
 * @fn      touchLink_BuildAesKey
 *
 * @brief   Build an AES key using Transaction ID and Response ID.
 *
 * @param   pAesKey - pointer to AES to be built
 * @param   transID - transaction id
 * @param   responseID - response id
 *
 * @return  none
 */
static void touchLink_BuildAesKey( uint8 *pAesKey, uint32 transID, uint32 responseID, uint8 keyIndex )
{

  if ( ( keyIndex == TOUCHLINK_KEY_INDEX_MASTER ) || ( keyIndex == TOUCHLINK_KEY_INDEX_CERT ) )
  {
    // Copy transaction identifier to 1st byte
    pAesKey[0] = BREAK_UINT32( transID, 3 );
    pAesKey[1] = BREAK_UINT32( transID, 2 );
    pAesKey[2] = BREAK_UINT32( transID, 1 );
    pAesKey[3] = BREAK_UINT32( transID, 0 );

    // Copy response identifier 3rd bute
    pAesKey[8] = BREAK_UINT32( responseID, 3 );
    pAesKey[9] = BREAK_UINT32( responseID, 2 );
    pAesKey[10] = BREAK_UINT32( responseID, 1 );
    pAesKey[11] = BREAK_UINT32( responseID, 0 );
  }

  // Copy in the transaction identifier
  pAesKey[4] = BREAK_UINT32( transID, 3 );
  pAesKey[5] = BREAK_UINT32( transID, 2 );
  pAesKey[6] = BREAK_UINT32( transID, 1 );
  pAesKey[7] = BREAK_UINT32( transID, 0 );

  // Copy in the response identifier
  pAesKey[12] = BREAK_UINT32( responseID, 3 );
  pAesKey[13] = BREAK_UINT32( responseID, 2 );
  pAesKey[14] = BREAK_UINT32( responseID, 1 );
  pAesKey[15] = BREAK_UINT32( responseID, 0 );
}

/*********************************************************************
 * @fn      touchLink_UpdateNwkKey
 *
 * @brief   Update the network key.
 *
 * @param   pNwkParams - pointer to new network key
 * @param   keyIndex - key index
 *
 * @return  none
 */
void touchLink_UpdateNwkKey( uint8 *pNwkKey, uint8 keyIndex )
{
  uint32 nwkFrameCounterTmp;
  (void)keyIndex;

  // To prevent Framecounter out of sync issues, store the lastkey
  nwkFrameCounterTmp = nwkFrameCounter;  // (Global in SSP).

  // Update the network key
  SSP_UpdateNwkKey( pNwkKey, 0 );

  SSP_SwitchNwkKey( 0 );

  nwkFrameCounter  = nwkFrameCounterTmp; // restore

  // Save off the security
  ZDApp_SaveNwkKey();
}

/*********************************************************************
 * @fn      touchLink_GetNwkKeyBitmask
 *
 * @brief   Get the supported network key bitmask.
 *
 * @param   none
 *
 * @return  network key bitmask
 */
uint16 touchLink_GetNwkKeyBitmask( void )
{
  return ( (uint16)1 << TOUCHLINK_KEY_INDEX );
}

/*********************************************************************
 * @fn      touchLink_GenerateRandNum
 *
 * @brief   Fill buffer with random bytes.
 *
 * input parameter
 *
 * @param   numSize - size of buffer in bytes
 *
 * output parameter
 *
 * @param   pNum - pointer to buffer to be filled with random values
 *
 * @return  none
 */
void touchLink_GenerateRandNum( uint8 *pNum, uint8 numSize )
{
  if ( pNum && numSize )
  {
    uint8 lastByte = ( numSize - 1 );
    for ( uint8 i = 0; i < lastByte; i += 2 )
    {
      uint16 rand = osal_rand();
      pNum[i]   = LO_UINT16( rand );
      pNum[i+1] = HI_UINT16( rand );
    }

    // In case the number is odd
    if ( numSize % 2 )
    {
      pNum[lastByte] = LO_UINT16( osal_rand() );
    }
  }
}

/*********************************************************************
 * @fn      touchLink_GetRandPrimaryChannel
 *
 * @brief   Get randomly chosen TOUCHLINK primary channel.
 *
 * @return  channel
 */
uint8 touchLink_GetRandPrimaryChannel()
{
  uint8 channel = osal_rand() & 0x1F;
  if ( channel <= TOUCHLINK_FIRST_CHANNEL )
  {
    channel = TOUCHLINK_FIRST_CHANNEL;
  }
  else if ( channel <= TOUCHLINK_SECOND_CHANNEL )
  {
    channel = TOUCHLINK_SECOND_CHANNEL;
  }
  else if ( channel <= TOUCHLINK_THIRD_CHANNEL )
  {
    channel = TOUCHLINK_THIRD_CHANNEL;
  }
  else
  {
    channel = TOUCHLINK_FOURTH_CHANNEL;
  }
#ifdef TOUCHLINK_DEV_SELECT_FIRST_CHANNEL
#warning The device will always select the first primary channel
  channel = TOUCHLINK_FIRST_CHANNEL;
#endif
  return channel;
}

/*********************************************************************
 * @fn      touchLink_SetNIB
 *
 * @brief   Copy new Network Parameters to the NIB.
 *
 * @param   nwkState - network state
 * @param   nwkAddr - short address
 * @param   pExtendedPANID - pointer to extended PAN ID
 * @param   logicalChannel - channel
 * @param   panId - PAN identifier
 * @param   nwkUpdateId - nwtwork update identifier
 *
 * @return      void
 */
void touchLink_SetNIB( nwk_states_t nwkState, uint16 nwkAddr, uint8 *pExtendedPANID,
                 uint8 logicalChannel, uint16 panId, uint8 nwkUpdateId )
{
  // Copy the new network parameters to NIB
  _NIB.nwkState = nwkState;
  _NIB.nwkDevAddress = nwkAddr;
  _NIB.nwkLogicalChannel = logicalChannel;
  _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
  _NIB.channelList = (uint32)1 << logicalChannel;
  _NIB.nwkPanId = panId;
  _NIB.nodeDepth = 1;
  _NIB.MaxRouters = (uint8)gNWK_MAX_DEVICE_LIST;
  _NIB.MaxChildren = (uint8)gNWK_MAX_DEVICE_LIST;
  _NIB.allocatedRouterAddresses = 1;
  _NIB.allocatedEndDeviceAddresses = 1;

  if ( _NIB.nwkUpdateId != nwkUpdateId )
  {
    NLME_SetUpdateID( nwkUpdateId );
  }

  osal_cpyExtAddr( _NIB.extendedPANID, pExtendedPANID );

  // Save the NIB
  if ( ( nwkState == NWK_ROUTER ) || ( nwkState == NWK_ENDDEVICE ) )
  {
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );
  }
  // else will be updated when ED joins its parent
}

/*********************************************************************
 * @fn      touchLink_ProcessNwkUpdate
 *
 * @brief   Update our local network update id and logical channel.
 *
 * @param   nwkUpdateId - new network update id
 * @param   logicalChannel - new logical channel
 *
 * @return  void
 */
void touchLink_ProcessNwkUpdate( uint8 nwkUpdateId, uint8 logicalChannel )
{
  // Update the network update id
  NLME_SetUpdateID( nwkUpdateId );

  // Switch channel
  if ( _NIB.nwkLogicalChannel != logicalChannel )
  {
    _NIB.nwkLogicalChannel = logicalChannel;
    touchLink_SetChannel( logicalChannel );
  }

  // Update channel list
  _NIB.channelList = (uint32)1 << logicalChannel;

  // Our Channel has been changed -- notify to save info into NV
  ZDApp_NwkStateUpdateCB();
  touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );

  // Reset the total transmit count and the transmit failure counters
  _NIB.nwkTotalTransmissions = 0;
  nwkTransmissionFailures( TRUE );
}

/*********************************************************************
 * @fn      touchLink_UpdateNV
 *
 * @brief   Updates NV with NIB and free ranges items
 *
 * @param   enables - specifies what to update
 *
 * @return  none
 */
void touchLink_UpdateNV( uint8 enables )
{
#if defined ( NV_RESTORE )

 #if defined ( NV_TURN_OFF_RADIO )
  // Turn off the radio's receiver during an NV update
  uint8 RxOnIdle;
  uint8 x = FALSE;
  ZMacGetReq( ZMacRxOnIdle, &RxOnIdle );
  ZMacSetReq( ZMacRxOnIdle, &x );
 #endif

  if ( enables & TOUCHLINK_UPDATE_NV_NIB )
  {
    // Update NIB in NV
    osal_nv_write( ZCD_NV_NIB, 0, sizeof( nwkIB_t ), &_NIB );

    // Reset the NV startup option to resume from NV by clearing
    // the "New" join option.
    zgWriteStartupOptions( ZG_STARTUP_CLEAR, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );
  }

  if ( enables & TOUCHLINK_UPDATE_NV_RANGES )
  {
    // Store our free network address and group ID ranges
    osal_nv_write( ZCD_NV_MIN_FREE_NWK_ADDR, 0, sizeof( touchLinkFreeNwkAddrBegin ), &touchLinkFreeNwkAddrBegin );
    osal_nv_write( ZCD_NV_MAX_FREE_NWK_ADDR, 0, sizeof( touchLinkFreeNwkAddrEnd ), &touchLinkFreeNwkAddrEnd );
    osal_nv_write( ZCD_NV_MIN_FREE_GRP_ID, 0, sizeof( touchLinkFreeGrpIdBegin ), &touchLinkFreeGrpIdBegin );
    osal_nv_write( ZCD_NV_MAX_FREE_GRP_ID, 0, sizeof( touchLinkFreeGrpIdEnd ), &touchLinkFreeGrpIdEnd );

    // Store our group ID range
    osal_nv_write( ZCD_NV_MIN_GRP_IDS, 0, sizeof( touchLinkGrpIDsBegin ), &touchLinkGrpIDsBegin );
    osal_nv_write( ZCD_NV_MAX_GRP_IDS, 0, sizeof( touchLinkGrpIDsEnd ), &touchLinkGrpIDsEnd );
  }

 #if defined ( NV_TURN_OFF_RADIO )
  ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
 #endif

#endif // NV_RESTORE
}

/*********************************************************************
 * @fn          touchLink_InitNV
 *
 * @brief       Initialize free range RAM variables from NV. If NV items
 *              don't exist, then the NV is initialize with what is in
 *              RAM variables.
 *
 * @param       none
 *
 * @return      none
 */
static void touchLink_InitNV( void )
{
  // Initialize our free network address and group ID ranges
  touchLink_ItemInit( ZCD_NV_MIN_FREE_NWK_ADDR, sizeof( touchLinkFreeNwkAddrBegin ), &touchLinkFreeNwkAddrBegin );
  touchLink_ItemInit( ZCD_NV_MAX_FREE_NWK_ADDR, sizeof( touchLinkFreeNwkAddrEnd ), &touchLinkFreeNwkAddrEnd );
  touchLink_ItemInit( ZCD_NV_MIN_FREE_GRP_ID, sizeof( touchLinkFreeGrpIdBegin ), &touchLinkFreeGrpIdBegin );
  touchLink_ItemInit( ZCD_NV_MAX_FREE_GRP_ID, sizeof( touchLinkFreeGrpIdEnd ), &touchLinkFreeGrpIdEnd );

  // Initialize our group ID range
  touchLink_ItemInit( ZCD_NV_MIN_GRP_IDS, sizeof( touchLinkGrpIDsBegin ), &touchLinkGrpIDsBegin );
  touchLink_ItemInit( ZCD_NV_MAX_GRP_IDS, sizeof( touchLinkGrpIDsEnd ), &touchLinkGrpIDsEnd );
}

/*********************************************************************
 * @fn      touchLink_ItemInit
 *
 * @brief   Initialize an NV item. If the item doesn't exist in NV memory,
 *          write the default (value passed in) into NV memory. But if
 *          it exists, set the item to the value stored in NV memory.
 *
 * @param   id - item id
 * @param   len - item len
 * @param   buf - pointer to the item
 *
 * @return  none
 */
void touchLink_ItemInit( uint16 id, uint16 len, void *pBuf )
{
#if defined ( NV_RESTORE )
  // If the item doesn't exist in NV memory, create and initialize
  // it with the value passed in.
  if ( osal_nv_item_init( id, len, pBuf ) == ZSuccess )
  {
    // The item already exists in NV memory, read it from NV memory
    osal_nv_read( id, 0, len, pBuf );
  }
#endif // NV_RESTORE
}

/*********************************************************************
 * @fn      touchLink_SetMacNwkParams
 *
 * @brief   Configure MAC with our Network Parameters.
 *
 * @param   nwkAddr - network address
 * @param   panId - PAN identifier
 * @param   channel
 *
 * @return  void
 */
void touchLink_SetMacNwkParams( uint16 nwkAddr, uint16 panId, uint8 channel )
{
  // Set our short address
  ZMacSetReq( ZMacShortAddress, (byte*)&nwkAddr );

  // Set our PAN ID
  ZMacSetReq( ZMacPanId, (byte*)&panId );

  // Tune to the selected logical channel
  touchLink_SetChannel( channel );
}

/*********************************************************************
 * @fn      touchLink_SetChannel
 *
 * @brief   Set our channel.
 *
 * @param   channel - new channel to change to
 *
 * @return  void
 */
void touchLink_SetChannel( uint8 channel )
{
  bdb_setChannel( (uint32) ( 1L << channel ) );

  // Set the new channel
  ZMacSetReq( ZMacChannel, &channel );
}

/*********************************************************************
 * @fn      touchLink_SendDeviceInfoRsp
 *
 * @brief   Send out a Device Information Response command.
 *
 * @param   srcEP - sender's endpoint
 * @param   dstAddr - destination address
 * @param   startIndex - start index
 * @param   transID - received transaction id
 * @param   seqNum - received sequence number
 *
 * @return  ZStatus_t
 */
uint8 touchLink_SendDeviceInfoRsp( uint8 srcEP, afAddrType_t *dstAddr, uint8 startIndex,
                             uint32 transID, uint8 seqNum )
{
  bdbTLDeviceInfoRsp_t *pRsp;
  uint8 cnt;
  uint8 rspLen;
  uint8 status = ZSuccess;

  cnt = touchLink_GetNumSubDevices( startIndex );
  if ( cnt > TOUCHLINK_DEVICE_INFO_RSP_REC_COUNT_MAX )
  {
    cnt = TOUCHLINK_DEVICE_INFO_RSP_REC_COUNT_MAX; // should be between 0x00-0x05
  }

  rspLen = sizeof( bdbTLDeviceInfoRsp_t ) + ( cnt * sizeof( devInfoRec_t ) );

  pRsp = (bdbTLDeviceInfoRsp_t *)osal_mem_alloc( rspLen );
  if ( pRsp )
  {
    pRsp->transID = transID;

    pRsp->numSubDevices = touchLink_GetNumSubDevices( 0 );
    pRsp->startIndex = startIndex;
    pRsp->cnt = cnt;

    for ( uint8 i = 0; i < cnt; i++ )
    {
      devInfoRec_t *pRec = &(pRsp->devInfoRec[i]);

      osal_cpyExtAddr( pRec->ieeeAddr, NLME_GetExtAddr() );

      touchLink_GetSubDeviceInfo( startIndex + i, &(pRec->deviceInfo) );

      pRec->sort = 0;
    }

    // Send a response back
    status = bdbTL_Send_DeviceInfoRsp( srcEP, dstAddr, pRsp, seqNum );

    osal_mem_free( pRsp );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      touchLink_SendLeaveReq
 *
 * @brief   Send out a Leave Request command.
 *
 * @param   void
 *
 * @return  ZStatus_t
 */
ZStatus_t touchLink_SendLeaveReq( void )
{
  NLME_LeaveReq_t leaveReq;
  
  // Set every field to 0
  osal_memset( &leaveReq, 0, sizeof( NLME_LeaveReq_t ) );
  
  // Send out our leave
  return ( NLME_LeaveReq( &leaveReq ) );
}

/*********************************************************************
 * @fn      touchLink_GetMsgRssi
 *
 * @brief   Get the RSSI of the message just received through a ZCL callback.
 *
 * @param   none
 *
 * @return  RSSI if AF message was received, TOUCHLINK_WORST_RSSI otherwise.
 */
int8 touchLink_GetMsgRssi( void )
{
  afIncomingMSGPacket_t *pAF = zcl_getRawAFMsg();

  if ( pAF != NULL )
  {
    return ( pAF->rssi );
  }

  return ( TOUCHLINK_WORST_RSSI );
}

/*********************************************************************
 * @fn      touchLink_NewNwkUpdateId
 *
 * @brief   Determine the new network update id. The nwkUpdateId attribute
 *          can take the value of 0x00 - 0xff and may wrap around so care
 *          must be taken when comparing for newness.
 *
 * @param   ID1 - first nwk update id
 * @param   ID2 - second nwk update id
 *
 * @return  new nwk update ID
 */
uint8 touchLink_NewNwkUpdateId( uint8 ID1, uint8 ID2 )
{
  if ( ( (ID1 >= ID2) && ((ID1 - ID2) > 200) )
      || ( (ID1 < ID2) && ((ID2 - ID1) > 200) ) )
  {
    return ( MIN( ID1, ID2 ) );
  }

  return ( MAX( ID1, ID2 ) );
}

/*********************************************************************
 * @fn      touchLink_SetTouchLinkTaskId
 *
 * @brief   Register Target/Initiator taskID for commissioning events
 *
 * @param   taskID
 *
 * @return  none
 */
void touchLink_SetTouchLinkTaskId( uint8 taskID )
{
  touchLinkTaskId = taskID;

  // register internal EP for TOUCHLINK messages
  bdb_RegisterSimpleDescriptor( &touchLink_EP_SimpleDesc );

#if defined( INTER_PAN )
  // Register with Stub APS
  StubAPS_RegisterApp( &touchLink_EP );
#endif // INTER_PAN
}

/*********************************************************************
 * @fn      touchLink_PerformNetworkDisc
 *
 * @brief   Perform a Network Discovery scan.
 *          Scan results will be stored locally to analyze.
 *
 * @param   scanChannelList - channels to perform discovery scan
 *
 * @return  void
 */
void touchLink_PerformNetworkDisc( uint32 scanChannelList )
{
  NLME_ScanFields_t scan;

  scan.channels = scanChannelList;
  scan.duration = BEACON_ORDER_240_MSEC;
  scan.scanType = ZMAC_ACTIVE_SCAN;
  scan.scanApp  = NLME_DISC_SCAN;

  if ( NLME_NwkDiscReq2( &scan ) == ZSuccess )
  {
    // Register ZDO callback to handle the network discovery confirm and
    // beacon notification confirm
    ZDO_RegisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID, touchLink_NwkDiscoveryCnfCB );
    ZDO_RegisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID, touchLink_BeaconIndCB );
  }
  else
  {
    NLME_NwkDiscTerm();
  }
}

/*********************************************************************
 * @fn      touchLink_BeaconIndCB
 *
 * @brief   Process the incoming beacon indication.
 *
 * @param   param -  pointer to a parameter and a structure of parameters
 *
 * @return  void
 */
static void *touchLink_BeaconIndCB ( void *param )
{
  NLME_beaconInd_t *pBeacon = param;
  touchLinkDiscoveredNwkParam_t *pParam = pDiscoveredNwkParamList;
  touchLinkDiscoveredNwkParam_t *pLastParam;
  uint8 found = FALSE;


  // Add the network parameter to the Network Parameter List
  while ( pParam != NULL )
  {
    if ( ( pParam->PANID == pBeacon->panID ) &&
        ( pParam->logicalChannel == pBeacon->logicalChannel ) )
    {
      found = TRUE;
      break;
    }
    
    pLastParam = pParam;
    pParam = pParam->nextParam;
  }
  
  // If no existing parameter found, make a new one and add to the list
  if ( found == FALSE )
  {
    pParam = osal_mem_alloc( sizeof( touchLinkDiscoveredNwkParam_t ) );
    if ( pParam == NULL )
    {
      // Memory alloc failed, discard this beacon
      return ( NULL );
    }
    
    // Clear the network descriptor
    osal_memset( pParam, 0, sizeof( touchLinkDiscoveredNwkParam_t )  );
    
    // Initialize the descriptor
    pParam->chosenRouter = INVALID_NODE_ADDR;
    pParam->chosenRouterDepth = 0xFF;
    
    // Save new entry into the descriptor list
    if ( pDiscoveredNwkParamList == NULL )
    {
      // First element in the list
      pDiscoveredNwkParamList = pParam;
    }
    else
    {
      // Last element in the list
      pLastParam->nextParam = pParam;
    }
  }
  
  // Update the descriptor with the incoming beacon
  pParam->logicalChannel = pBeacon->logicalChannel;
  pParam->PANID          = pBeacon->panID;
  
  // Save the extended PAN ID from the beacon payload only if 1.1 version network
  if ( pBeacon->protocolVersion != ZB_PROT_V1_0 )
  {
    osal_cpyExtAddr( pParam->extendedPANID, pBeacon->extendedPanID );
  }
  else
  {
    osal_memset( pParam->extendedPANID, 0xFF, Z_EXTADDR_LEN );
  }
  
  // check if this device is a better choice to join...
  // ...dont bother checking assocPermit flag is doing a rejoin
  if ( pBeacon->LQI > gMIN_TREE_LQI )
  {
    uint8 selected = FALSE;
    uint8 capacity = FALSE;
    
    if ( _NIB.spare3 == ZIGBEEPRO )
    {
      if ( ((pBeacon->LQI   > pParam->chosenRouterLinkQuality) &&
            (pBeacon->depth < MAX_NODE_DEPTH)) ||
          ((pBeacon->LQI   == pParam->chosenRouterLinkQuality) &&
           (pBeacon->depth < pParam->chosenRouterDepth)) )
      {
        selected = TRUE;
      }
    }
    else
    {
      if ( pBeacon->depth < pParam->chosenRouterDepth )
      {
        selected = TRUE;
      }
    }
    
    capacity = pBeacon->routerCapacity;
    
    if ( (capacity) && (selected) )
    {
      // this is the new chosen router for joining...
      pParam->chosenRouter            = pBeacon->sourceAddr;
      pParam->chosenRouterLinkQuality = pBeacon->LQI;
      pParam->chosenRouterDepth       = pBeacon->depth;
    }
    
    if ( pBeacon->deviceCapacity )
      pParam->deviceCapacity = 1;
    
    if ( pBeacon->routerCapacity )
      pParam->routerCapacity = 1;
  }
    
  return ( NULL );
}

/*********************************************************************
 * @fn      touchLink_NwkDiscoveryCnfCB
 *
 * @brief   Send an event to inform the target the completion of
 *          network discovery scan
 *
 * @param   param - pointer to a parameter and a structure of parameters
 *
 * @return  void
 */
static void *touchLink_NwkDiscoveryCnfCB ( void *param )
{
#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
  // Scan completed. De-register the callbacks with ZDO
  ZDO_DeregisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID );
  ZDO_DeregisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID );

  NLME_NwkDiscTerm();

  if ( pDiscoveredNwkParamList != NULL )
  {
    // proceed to join the network, otherwise
    // Notify our task
    osal_set_event( touchLinkTaskId, TOUCHLINK_NWK_DISC_CNF_EVT );
  }
  else
  {
    // no suitable network in secondary channel list, then just wait for touchlink
#if ( ZSTACK_ROUTER_BUILD )
    // Try to create a new distributed network
    osal_set_event( touchLinkTaskId, TOUCHLINK_NWK_DISC_CNF_EVT );
#elif ( ZSTACK_END_DEVICE_BUILD )
    // Notify the BDB state machine 
    bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
    bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
    // No parent to join in 
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_PARENT_LOST;
    NLME_OrphanStateSet( );
    bdb_ZedAttemptRecoverNwk( );
#endif
  }

#else
  (void)touchLinkTaskId;
  (void)param;
#endif
  return ( NULL );
}

/****************************************************************************
 * @fn      touchLink_FreeNwkParamList
 *
 * @brief   This function frees any network discovery data.
 *
 * @param   none
 *
 * @return  none
 */
void touchLink_FreeNwkParamList( void )
{
  touchLinkDiscoveredNwkParam_t *pParam = pDiscoveredNwkParamList;
  touchLinkDiscoveredNwkParam_t *pNextParam;

  // deallocate the pDiscoveredNwkParamList memory
  while ( pParam != NULL )
  {
    pNextParam = pParam->nextParam;

    osal_mem_free( pParam );

    pParam = pNextParam;
  }

  pDiscoveredNwkParamList = NULL;
}

/****************************************************************************
 * @fn      touchLink_IsValidTransID
 *
 * @brief   Transaction ID Filter for Touch-Link received commands.
 *
 * @param   transID - received transaction ID
 *
 * @return  FALSE if not matching current or transaction expired
 */
bool touchLink_IsValidTransID( uint32 transID )
{
  if ( ( touchLinkTransID == 0 ) || ( ( touchLinkTransID != transID ) && ( touchLinkLastAcceptedTransID != transID ) ) )
  {
    return ( FALSE );
  }
  return ( TRUE );
}

/*********************************************************************
 * @fn      touchLink_RouterProcessZDOMsg
 *
 * @brief   Process incoming ZDO messages (for routers)
 *
 * @param   inMsg - message to process
 *
 * @return  none
 */
void touchLink_RouterProcessZDOMsg( zdoIncomingMsg_t *inMsg )
{
  ZDO_DeviceAnnce_t devAnnce;

  switch ( inMsg->clusterID )
  {
    case Device_annce:
      {
        // all devices should send link status, including the one sending it
        ZDO_ParseDeviceAnnce( inMsg, &devAnnce );

        linkInfo_t *linkInfo;

        // check if entry exists
        linkInfo = nwkNeighborGetLinkInfo( devAnnce.nwkAddr, _NIB.nwkPanId );

        // if not, look for a vacant entry to add this node...
        if ( linkInfo == NULL )
        {
          nwkNeighborAdd( devAnnce.nwkAddr, _NIB.nwkPanId, DEF_LQI );
          linkInfo = nwkNeighborGetLinkInfo( devAnnce.nwkAddr, _NIB.nwkPanId );
          linkInfo->txCost = DEF_LINK_COST;
          linkInfo->rxLqi = MIN_LQI_COST_1; 
  
          // if we have end device childs, send link status
          if ( AssocCount(CHILD_RFD, CHILD_RFD_RX_IDLE) > 0 )
          {
            NLME_UpdateLinkStatus();
          }
        }
      }
      break;

    case Mgmt_Permit_Join_req:
      {
        uint8 duration = inMsg->asdu[ZDP_MGMT_PERMIT_JOIN_REQ_DURATION];
        ZStatus_t stat = NLME_PermitJoiningRequest( duration );
        // Send a response if unicast
        if ( !inMsg->wasBroadcast )
        {
          ZDP_MgmtPermitJoinRsp( inMsg->TransSeq, &(inMsg->srcAddr), stat, false );
        }
      }
      break;

    default:
      break;
  }
}

/*********************************************************************
 * @fn      touchLink_PermitJoin
 *
 * @brief   Set the router permit join flag, to allow or deny classical
 *          commissioning by other ZigBee devices.
 *
 * @param   duration - enable up to aplcMaxPermitJoinDuration seconds,
 *                     0 to disable
 *
 * @return  status
 */
ZStatus_t touchLink_PermitJoin( uint8 duration )
{
  if ( duration > APLC_MAX_PERMIT_JOIN_DURATION )
  {
    duration = APLC_MAX_PERMIT_JOIN_DURATION;
  }
  return NLME_PermitJoiningRequest( duration );
}

/*********************************************************************
 * @fn      targetStartRtr
 *
 * @brief   Start operating on the new network.
 *
 * @param   pParams - pointer to received network parameters
 * @param   transID - transaction id
 *
 * @return  none
 */
void touchLinkStartRtr( bdbTLNwkParams_t *pParams, uint32 transID )
{
  // Copy the new network parameters to
  touchLink_SetNIB( NWK_ROUTER, pParams->nwkAddr, pParams->extendedPANID,
              pParams->logicalChannel, pParams->panId, _NIB.nwkUpdateId );

  // Apply the received network key
  touchLink_DecryptNwkKey( pParams->nwkKey, pParams->keyIndex, transID, touchLinkResponseID );

  // setting apsTrustCenterAddress to 0xffffffff
  ZDSecMgrUpdateTCAddress( NULL );

  NLME_PermitJoiningRequest(0);
  
  // Touchlink distributed network flag
  touchlinkDistNwk = TRUE;

  // Use the new free ranges
  //touchLink_UpdateFreeRanges( pParams );

  // Save free ranges
  touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

  // In case we're here after a leave
  touchLinkLeaveInitiated = FALSE;

  // Clear leave control logic
  ZDApp_LeaveCtrlReset();

  // Start operating on the new network
  ZDOInitDeviceEx( 0, 1 );
}

/*********************************************************************
 * @fn      touchLink_DevRejoin
 *
 * @brief   Perform a network rejoin
 *
 * @param   rejoinInf - pointer to received network parameters
 *
 * @return  none
 */
void touchLink_DevRejoin( bdbTLNwkRejoin_t *rejoinInf )
{
    networkDesc_t *pNwkDesc = NULL;
     
    // Initialize the security for type of device
    ZDApp_SecInit( ZDO_INITDEV_RESTORED_NETWORK_STATE );
    
    pNwkDesc = (networkDesc_t *)osal_mem_alloc( sizeof( networkDesc_t ) );
    if( pNwkDesc == NULL )
    {
      return;
    }
         
    NwkDescList = pNwkDesc;
    pNwkDesc->panId = rejoinInf->panId;
    pNwkDesc->logicalChannel = rejoinInf->logicalChannel;
    osal_memcpy( pNwkDesc->extendedPANID, rejoinInf->extendedPANID, Z_EXTADDR_LEN);
    pNwkDesc->chosenRouterDepth = 1;
    pNwkDesc->routerCapacity = 1;
    pNwkDesc->deviceCapacity = 1;
    pNwkDesc->version = 2;
    pNwkDesc->stackProfile = 2;
    pNwkDesc->chosenRouterLinkQuality = DEF_LQI;
    pNwkDesc->chosenRouter = rejoinInf->nwkAddr;
    pNwkDesc->updateId = rejoinInf->nwkUpdateId;
    pNwkDesc->nextDesc = NULL;
    
    // Save free ranges
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

    // In case we're here after a leave
    touchLinkLeaveInitiated = FALSE;

    // Clear leave control logic
    ZDApp_LeaveCtrlReset();
    
    // Let's join the network started by the target
    NLME_ReJoinRequest( rejoinInf->extendedPANID, _NIB.nwkLogicalChannel);
}

/*********************************************************************
 * @fn      touchLink_DeviceIsInitiator
 *
 * @brief   Set device initiator flag.
 *
 * @param   initiator - new flag value
 *
 * @return  none
 */
void touchLink_DeviceIsInitiator( bool initiator )
{
  touchLinkIsInitiator = initiator;
}

/*********************************************************************
 * @fn      touchLink_DeviceIsInitiator
 *
 * @brief   Get device initiator flag.
 *
 * @param   none
 *
 * @return  touchLinkIsInitiator - flag value
 */
bool touchLink_GetDeviceInitiator( void )
{
  return touchLinkIsInitiator;
}

#endif // BDB_TL_TARGET || BDB_TL_INITIATOR

/*********************************************************************
*********************************************************************/
