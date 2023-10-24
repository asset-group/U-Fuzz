/******************************************************************************
  Filename:       ZDSecMgr.c
  Revised:        $Date: 2014-06-05 11:25:00 -0700 (Thu, 05 Jun 2014) $
  Revision:       $Revision: 38833 $

  Description:    The ZigBee Device Security Manager.


  Copyright 2005-2014 Texas Instruments Incorporated. All rights reserved.

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
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_Nv.h"
#include "ZGlobals.h"
#include "ssp.h"
#include "nwk_globals.h"
#include "nwk.h"
#include "NLMEDE.h"
#include "AddrMgr.h"
#include "AssocList.h"
#include "APSMEDE.h"
#include "ZDConfig.h"
#include "ZDSecMgr.h"
  
#include "bdb.h"
#include "bdb_interface.h"

/******************************************************************************
 * CONSTANTS
 */
// maximum number of devices managed by this Security Manager
#if !defined ( ZDSECMGR_DEVICE_MAX )
  #define ZDSECMGR_DEVICE_MAX 3
#endif

// maximum number of LINK keys this device may store
#define ZDSECMGR_ENTRY_MAX ZDSECMGR_DEVICE_MAX

// total number of stored devices
#if !defined ( ZDSECMGR_STORED_DEVICES )
  #define ZDSECMGR_STORED_DEVICES 3
#endif

// Joining Device Policies: r21 spec 4.9.1
// This boolean indicates whether the device will request a new Trust Center Link key after joining.
// TC link key cannot be requested if join is performed on distributed nwk
bool  requestNewTrustCenterLinkKey = TRUE;  
//This integer indicates the maximum time in seconds that a device will wait for a response to a 
//request for a Trust Center link key.
uint32  requestLinkKeyTimeout = BDBC_TC_LINK_KEY_EXANGE_TIMEOUT;
 //bool acceptNewUnsolicitedApplicationLinkKey;   

// APSME Stub Implementations
#define ZDSecMgrLinkKeySet       APSME_LinkKeySet
#define ZDSecMgrLinkKeyNVIdGet   APSME_LinkKeyNVIdGet
#define ZDSecMgrKeyFwdToChild    APSME_KeyFwdToChild
#define ZDSecMgrIsLinkKeyValid   APSME_IsLinkKeyValid



/******************************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint16            ami;
  uint16            keyNvId;   // index to the Link Key table in NV
  ZDSecMgr_Authentication_Option authenticateOption;
} ZDSecMgrEntry_t;

typedef struct
{
  uint16          nwkAddr;
  uint8*          extAddr;
  uint16          parentAddr;
  uint8           secure;
  uint8           devStatus;
} ZDSecMgrDevice_t;

/******************************************************************************
 * EXTERNAL VARIABLES
 */
extern CONST uint8 gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES;
extern pfnZdoCb zdoCBFunc[MAX_ZDO_CB_FUNC];


/******************************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void   ZDApp_ResetTimerCancel( void );
/******************************************************************************
 * LOCAL VARIABLES
 */
#if 0 // Taken out because the following functionality is only used for test
      // purpose. A more efficient (above) way is used. It can be put
      // back in if customers request for a white/black list feature.
uint8 ZDSecMgrStoredDeviceList[ZDSECMGR_STORED_DEVICES][Z_EXTADDR_LEN] =
{
  { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
};
#endif

uint8 ZDSecMgrTCExtAddr[Z_EXTADDR_LEN]=
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8 ZDSecMgrTCAuthenticated = FALSE;

ZDSecMgrEntry_t* ZDSecMgrEntries  = NULL;

void ZDSecMgrAddrMgrCB( uint8 update, AddrMgrEntry_t* newEntry, AddrMgrEntry_t* oldEntry );

uint8 ZDSecMgrPermitJoiningEnabled;
uint8 ZDSecMgrPermitJoiningTimed;

APSME_TCLKDevEntry_t TrustCenterLinkKey;

APSME_ApsLinkKeyFrmCntr_t ApsLinkKeyFrmCntr[ZDSECMGR_ENTRY_MAX];
APSME_TCLinkKeyFrmCntr_t TCLinkKeyFrmCntr[ZDSECMGR_TC_DEVICE_MAX];

 CONST uint16 gZDSECMGR_TC_DEVICE_MAX = ZDSECMGR_TC_DEVICE_MAX;
 CONST uint16 gZDSECMGR_TC_DEVICE_IC_MAX = ZDSECMGR_TC_DEVICE_IC_MAX;
 uint8  gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY = ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;
/******************************************************************************
 * PRIVATE FUNCTIONS
 *
 *   ZDSecMgrAddrStore
 *   ZDSecMgrExtAddrStore
 *   ZDSecMgrExtAddrLookup
 *   ZDSecMgrEntryInit
 *   ZDSecMgrEntryLookup
 *   ZDSecMgrEntryLookupAMI
 *   ZDSecMgrEntryLookupExt
 *   ZDSecMgrEntryLookupExtGetIndex
 *   ZDSecMgrEntryFree
 *   ZDSecMgrEntryNew
 *   ZDSecMgrAppKeyGet
 *   ZDSecMgrAppKeyReq
 *   ZDSecMgrTclkReq
 *   ZDSecMgrAppConfKeyReq 
 *   ZDSecMgrSendNwkKey
 *   ZDSecMgrDeviceRemove
 *   ZDSecMgrDeviceValidateRM
 *   ZDSecMgrDeviceValidate
 *   ZDSecMgrDeviceJoin
 *   ZDSecMgrDeviceJoinDirect
 *   ZDSecMgrDeviceJoinFwd
 *   ZDSecMgrDeviceNew
 *   ZDSecMgrAssocDeviceAuth
 *   ZDSecMgrAuthNwkKey
 *   APSME_TCLinkKeyInit
 *   APSME_IsDefaultTCLK
 */

//-----------------------------------------------------------------------------
// address management
//-----------------------------------------------------------------------------
ZStatus_t ZDSecMgrAddrStore( uint16 nwkAddr, uint8* extAddr, uint16* ami );
ZStatus_t ZDSecMgrExtAddrStore( uint16 nwkAddr, uint8* extAddr, uint16* ami );
ZStatus_t ZDSecMgrExtAddrLookup( uint8* extAddr, uint16* ami );

//-----------------------------------------------------------------------------
// Trust Center management
//-----------------------------------------------------------------------------
uint8 ZDSecMgrTCExtAddrCheck( uint8* extAddr );
void ZDSecMgrTCDataLoad( uint8* extAddr );

//-----------------------------------------------------------------------------
// entry data
//-----------------------------------------------------------------------------
void ZDSecMgrEntryInit(uint8 state);
ZStatus_t ZDSecMgrEntryLookup( uint16 nwkAddr, ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrEntryLookupAMI( uint16 ami, ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrEntryLookupExt( uint8* extAddr, ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrEntryLookupExtGetIndex( uint8* extAddr, ZDSecMgrEntry_t** entry, uint16* entryIndex );
ZStatus_t ZDSecMgrEntryLookupAMIGetIndex( uint16 ami, uint16* entryIndex );
void ZDSecMgrEntryFree( ZDSecMgrEntry_t* entry );
ZStatus_t ZDSecMgrEntryNew( ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrAuthenticationSet( uint8* extAddr, ZDSecMgr_Authentication_Option option );
void ZDSecMgrApsLinkKeyInit(uint8 setDefault);
#if defined ( NV_RESTORE )
static void ZDSecMgrWriteNV(void);
static void ZDSecMgrRestoreFromNV(void);
static void ZDSecMgrUpdateNV( uint16 index );
#endif

//-----------------------------------------------------------------------------
// key support
//-----------------------------------------------------------------------------
ZStatus_t ZDSecMgrAppKeyGet( uint16  initNwkAddr,
                             uint8*  initExtAddr,
                             uint16  partNwkAddr,
                             uint8*  partExtAddr,
                             uint8** key,
                             uint8*  keyType );
void ZDSecMgrAppKeyReq( ZDO_RequestKeyInd_t* ind );
void ZDSecMgrTclkReq( ZDO_RequestKeyInd_t* ind );
void ZDSecMgrAppConfKeyReq( ZDO_VerifyKeyInd_t* ind );
ZStatus_t ZDSecMgrSendNwkKey( ZDSecMgrDevice_t* device );
void ZDSecMgrNwkKeyInit(uint8 setDefault);

//-----------------------------------------------------------------------------
// device management
//-----------------------------------------------------------------------------
void ZDSecMgrDeviceRemove( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceValidateRM( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceValidate( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceJoin( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceJoinDirect( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceJoinFwd( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceNew( ZDSecMgrDevice_t* device );

//-----------------------------------------------------------------------------
// association management
//-----------------------------------------------------------------------------
void ZDSecMgrAssocDeviceAuth( associated_devices_t* assoc );

//-----------------------------------------------------------------------------
// authentication management
//-----------------------------------------------------------------------------
void ZDSecMgrAuthNwkKey( void );

//-----------------------------------------------------------------------------
// APSME function
//-----------------------------------------------------------------------------
void APSME_TCLinkKeyInit( uint8 setDefault );
uint8 APSME_IsDefaultTCLK( uint8 *extAddr );
void ZDSecMgrGenerateSeed(uint8 setDefault );
void ZDSecMgrGenerateKeyFromSeed(uint8 *extAddr, uint8 shift, uint8 *key);
/******************************************************************************
 * @fn          ZDSecMgrAddrStore
 *
 * @brief       Store device addresses.
 *
 * @param       nwkAddr - [in] NWK address
 * @param       extAddr - [in] EXT address
 * @param       ami     - [out] Address Manager index
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAddrStore( uint16 nwkAddr, uint8* extAddr, uint16* ami )
{
  ZStatus_t      status;
  AddrMgrEntry_t entry;


  // add entry
  entry.user    = ADDRMGR_USER_SECURITY;
  entry.nwkAddr = nwkAddr;
  AddrMgrExtAddrSet( entry.extAddr, extAddr );

  if ( AddrMgrEntryUpdate( &entry ) == TRUE )
  {
    // return successful results
    *ami   = entry.index;
    status = ZSuccess;
  }
  else
  {
    // return failed results
    *ami   = entry.index;
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrExtAddrStore
 *
 * @brief       Store EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       ami     - [out] Address Manager index
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrExtAddrStore( uint16 nwkAddr, uint8* extAddr, uint16* ami )
{
  ZStatus_t      status;
  AddrMgrEntry_t entry;


  // add entry
  entry.user    = ADDRMGR_USER_SECURITY;
  entry.nwkAddr = nwkAddr;
  AddrMgrExtAddrSet( entry.extAddr, extAddr );

  if ( AddrMgrEntryUpdate( &entry ) == TRUE )
  {
    // return successful results
    *ami   = entry.index;
    status = ZSuccess;
  }
  else
  {
    // return failed results
    *ami   = entry.index;
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrExtAddrLookup
 *
 * @brief       Lookup index for specified EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       ami     - [out] Address Manager index
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrExtAddrLookup( uint8* extAddr, uint16* ami )
{
  ZStatus_t      status;
  AddrMgrEntry_t entry;


  // lookup entry
  entry.user = ADDRMGR_USER_SECURITY;
  AddrMgrExtAddrSet( entry.extAddr, extAddr );

  if ( AddrMgrEntryLookupExt( &entry ) == TRUE )
  {
    // return successful results
    *ami   = entry.index;
    status = ZSuccess;
  }
  else
  {
    // return failed results
    *ami   = entry.index;
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAddrClear
 *
 * @brief       Clear security bit from Address Manager for specific device.
 *
 * @param       extAddr - [in] EXT address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAddrClear( uint8* extAddr )
{
  ZStatus_t status;
  uint16 entryAmi;

  // get Address Manager Index
  status = ZDSecMgrExtAddrLookup( extAddr, &entryAmi );

  if ( status == ZSuccess )
  {
    AddrMgrEntry_t addrEntry;

    // Clear SECURITY User bit from the address manager
    addrEntry.user = ADDRMGR_USER_SECURITY;
    addrEntry.index = entryAmi;

    if ( AddrMgrEntryRelease( &addrEntry ) != TRUE )
    {
      // return failure results
      status = ZFailure;
    }
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryInit
 *
 * @brief       Initialize entry sub module
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
void ZDSecMgrEntryInit(uint8 state)
{
  if (ZDSecMgrEntries == NULL)
  {
    uint16 index;

    if ((ZDSecMgrEntries = osal_mem_alloc(sizeof(ZDSecMgrEntry_t) * ZDSECMGR_ENTRY_MAX)) == NULL)
    {
      return;
    }

    for (index = 0; index < ZDSECMGR_ENTRY_MAX; index++)
    {
      ZDSecMgrEntries[index].ami = INVALID_NODE_ADDR;

      ZDSecMgrEntries[index].keyNvId = SEC_NO_KEY_NV_ID;
    }
  }

#if defined NV_RESTORE
  if (state == ZDO_INITDEV_RESTORED_NETWORK_STATE)
  {
    ZDSecMgrRestoreFromNV();
  }
#else
  (void)state;
#endif
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookup
 *
 * @brief       Lookup entry index using specified NWK address.
 *
 * @param       nwkAddr - [in] NWK address
 * @param       entry   - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookup( uint16 nwkAddr, ZDSecMgrEntry_t** entry )
{
  uint16         index;
  AddrMgrEntry_t addrMgrEntry;

  // initialize results
  *entry = NULL;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    addrMgrEntry.user    = ADDRMGR_USER_SECURITY;
    addrMgrEntry.nwkAddr = nwkAddr;

    if ( AddrMgrEntryLookupNwk( &addrMgrEntry ) == TRUE )
    {
      for ( index = 0; index < ZDSECMGR_ENTRY_MAX ; index++ )
      {
        if ( addrMgrEntry.index == ZDSecMgrEntries[index].ami )
        {
          // return successful results
          *entry = &ZDSecMgrEntries[index];

          // break from loop
          return ZSuccess;
        }
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupAMI
 *
 * @brief       Lookup entry using specified address index
 *
 * @param       ami   - [in] Address Manager index
 * @param       entry - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupAMI( uint16 ami, ZDSecMgrEntry_t** entry )
{
  uint16 index;

  // initialize results
  *entry = NULL;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    for ( index = 0; index < ZDSECMGR_ENTRY_MAX ; index++ )
    {
      if ( ZDSecMgrEntries[index].ami == ami )
      {
        // return successful results
        *entry = &ZDSecMgrEntries[index];

        // break from loop
        return ZSuccess;
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupExt
 *
 * @brief       Lookup entry index using specified EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       entry   - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupExt( uint8* extAddr, ZDSecMgrEntry_t** entry )
{
  ZStatus_t status;
  uint16    ami;

  // initialize results
  *entry = NULL;
  status = ZNwkUnknownDevice;

  // lookup address index
  if ( ZDSecMgrExtAddrLookup( extAddr, &ami ) == ZSuccess )
  {
    status = ZDSecMgrEntryLookupAMI( ami, entry );
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupExtGetIndex
 *
 * @brief       Lookup entry index using specified EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       entryIndex - [out] valid index to the entry table
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupExtGetIndex( uint8* extAddr, ZDSecMgrEntry_t** entry, uint16* entryIndex )
{
  uint16 ami;
  uint16 index;

  // lookup address index
  if ( ZDSecMgrExtAddrLookup( extAddr, &ami ) == ZSuccess )
  {
    // verify data is available
    if ( ZDSecMgrEntries != NULL )
    {
      for ( index = 0; index < ZDSECMGR_ENTRY_MAX ; index++ )
      {
        if ( ZDSecMgrEntries[index].ami == ami )
        {
          // return successful results
          *entry = &ZDSecMgrEntries[index];
          *entryIndex = index;

          // break from loop
          return ZSuccess;
        }
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupAMIGetIndex
 *
 * @brief       Lookup entry using specified address index
 *
 * @param       ami   - [in] Address Manager index
 * @param       entryIndex - [out] valid index to the entry table
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupAMIGetIndex( uint16 ami, uint16* entryIndex )
{
  uint16 index;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    for ( index = 0; index < ZDSECMGR_ENTRY_MAX ; index++ )
    {
      if ( ZDSecMgrEntries[index].ami == ami )
      {
        // return successful results
        *entryIndex = index;

        // break from loop
        return ZSuccess;
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryFree
 *
 * @brief       Free entry.
 *
 * @param       entry - [in] valid entry
 *
 * @return      ZStatus_t
 */
void ZDSecMgrEntryFree( ZDSecMgrEntry_t* entry )
{
  APSME_LinkKeyData_t   *pApsLinkKey = NULL;

#if defined ( NV_RESTORE )
  ZStatus_t status;
  uint16 entryIndex;

  status = ZDSecMgrEntryLookupAMIGetIndex( entry->ami, &entryIndex );
#endif

  pApsLinkKey = (APSME_LinkKeyData_t *)osal_mem_alloc(sizeof(APSME_LinkKeyData_t));

  if (pApsLinkKey != NULL)
  {
    osal_memset( pApsLinkKey, 0x00, sizeof(APSME_LinkKeyData_t) );

    // Clear the APS Link key in NV
    osal_nv_write( entry->keyNvId, 0,
                        sizeof(APSME_LinkKeyData_t), pApsLinkKey);

    // set entry to invalid Key
    entry->keyNvId = SEC_NO_KEY_NV_ID;

    osal_mem_free(pApsLinkKey);
  }

  // marking the entry as INVALID_NODE_ADDR
  entry->ami = INVALID_NODE_ADDR;

  // set to default value
  entry->authenticateOption = ZDSecMgr_Not_Authenticated;

#if defined ( NV_RESTORE )
  if ( status == ZSuccess )
  {
    ZDSecMgrUpdateNV(entryIndex);
  }
#endif
}

/******************************************************************************
 * @fn          ZDSecMgrEntryNew
 *
 * @brief       Get a new entry.
 *
 * @param       entry - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryNew( ZDSecMgrEntry_t** entry )
{
  uint16 index;

  // initialize results
  *entry = NULL;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    // find available entry
    for ( index = 0; index < ZDSECMGR_ENTRY_MAX ; index++ )
    {
      if ( ZDSecMgrEntries[index].ami == INVALID_NODE_ADDR )
      {
        // return successful result
        *entry = &ZDSecMgrEntries[index];

        // Set the authentication option to default
        ZDSecMgrEntries[index].authenticateOption = ZDSecMgr_Not_Authenticated;

        // break from loop
        return ZSuccess;
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrAppKeyGet
 *
 * @brief       get an APP key - option APP(MASTER or LINK) key
 *
 * @param       initNwkAddr - [in] NWK address of initiator device
 * @param       initExtAddr - [in] EXT address of initiator device
 * @param       partNwkAddr - [in] NWK address of partner device
 * @param       partExtAddr - [in] EXT address of partner device
 * @param       key         - [out] APP(MASTER or LINK) key
 * @param       keyType     - [out] APP(MASTER or LINK) key type
 *
 * @return      ZStatus_t
 */
uint8 ZDSecMgrAppKeyType = KEY_TYPE_APP_LINK;    // Set the default key type
                                                 // to KEY_TYPE_APP_LINK since
                                                 // only specific requirement
                                                 // right now comes from SE profile
ZStatus_t ZDSecMgrAppKeyGet( uint16  initNwkAddr,
                             uint8*  initExtAddr,
                             uint16  partNwkAddr,
                             uint8*  partExtAddr,
                             uint8** key,
                             uint8*  keyType )
{
  // Intentionally unreferenced parameters
  (void)initNwkAddr;
  (void)initExtAddr;
  (void)partNwkAddr;
  (void)partExtAddr;

  //---------------------------------------------------------------------------
  // note:
  // should use a robust mechanism to generate keys, for example
  // combine EXT addresses and call a hash function
  //---------------------------------------------------------------------------
  SSP_GetTrueRand( SEC_KEY_LEN, *key );

  *keyType = ZDSecMgrAppKeyType;

  return ZSuccess;
}




/******************************************************************************
 * @fn          ZDSecMgrAppKeyReq
 *
 * @brief       Process request for APP key between two devices.
 *
 * @param       device - [in] ZDO_RequestKeyInd_t, request info
 *
 * @return      none
 */
void ZDSecMgrAppKeyReq( ZDO_RequestKeyInd_t* ind )
{
  APSME_TransportKeyReq_t req;
  uint8                   initExtAddr[Z_EXTADDR_LEN];
  uint16                  partNwkAddr;
  uint8                   key[SEC_KEY_LEN];


  // validate initiator and partner
  if ( ( APSME_LookupNwkAddr( ind->partExtAddr, &partNwkAddr ) == TRUE ) &&
       ( APSME_LookupExtAddr( ind->srcAddr, initExtAddr ) == TRUE      )   )
  {
    // point the key to some memory
    req.key = key;

    // get an APP key - option APP (MASTER or LINK) key
    if ( ZDSecMgrAppKeyGet( ind->srcAddr,
                            initExtAddr,
                            partNwkAddr,
                            ind->partExtAddr,
                            &req.key,
                            &req.keyType ) == ZSuccess )
    {
      // always secure
      req.nwkSecure = TRUE;
      req.apsSecure = TRUE;
      req.tunnel    = NULL;

      // send key to initiator device
      req.dstAddr   = ind->srcAddr;
      req.extAddr   = ind->partExtAddr;
      req.initiator = TRUE;
      APSME_TransportKeyReq( &req );

      // send key to partner device
      req.dstAddr   = partNwkAddr;
      req.extAddr   = initExtAddr;
      req.initiator = FALSE;

      APSME_TransportKeyReq( &req );

      // clear copy of key in RAM
      osal_memset( key, 0x00, SEC_KEY_LEN);

    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrTclkReq
 *
 * @brief       Process request for TCLK.
 *
 * @param       device - [in] ZDO_RequestKeyInd_t, request info
 *
 * @return      none
 */
void ZDSecMgrTclkReq( ZDO_RequestKeyInd_t* ind )
{
  APSME_TransportKeyReq_t req;
  uint8                   initExtAddr[Z_EXTADDR_LEN];
  uint16                  partNwkAddr;
  uint8                   key[SEC_KEY_LEN];
  APSME_TCLKDevEntry_t    TCLKDevEntry;
  uint8                   found;
    
  // validate initiator and partner
  if ( ( ( APSME_LookupNwkAddr( ind->partExtAddr, &partNwkAddr ) == TRUE ) || ( ind->keyType != KEY_TYPE_APP_MASTER ) ) &&
       (   APSME_LookupExtAddr( ind->srcAddr, initExtAddr )      == TRUE ) )
  {
    // point the key to some memory
    req.key = key;

    //Search for the entry
    APSME_SearchTCLinkKeyEntry(initExtAddr,&found, &TCLKDevEntry);

    //If found, generate the key accordingly to the key attribute
    if(found)
    {
      //Generate key from the seed, which would be the unique key
      ZDSecMgrGenerateKeyFromSeed(TCLKDevEntry.extAddr,TCLKDevEntry.SeedShift_IcIndex,req.key);
    
      // always secure
      req.nwkSecure = TRUE;
      req.apsSecure = TRUE;
      req.tunnel    = NULL;

      // send key to initiator device
      req.dstAddr   = ind->srcAddr;
      req.extAddr   = initExtAddr;
      req.initiator = TRUE;
      req.keyType   = KEY_TYPE_TC_LINK;
        
      APSME_TransportKeyReq( &req );
        
      // clear copy of key in RAM
      osal_memset( key, 0x00, SEC_KEY_LEN);
    
    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrAppConfKeyReq
 *
 * @brief       Process request for APP key between two devices.
 *
 * @param       device - [in] ZDO_VerifyKeyInd_t, request info
 *
 * @return      none
 */
void ZDSecMgrAppConfKeyReq( ZDO_VerifyKeyInd_t* ind )
{
  APSME_ConfirmKeyReq_t   req;

  // send key to initiator device
  req.dstAddr      = ind->srcAddr;
  req.status       = ind->verifyKeyStatus;
  req.dstExtAddr   = ind->extAddr;
  req.keyType      = ind->keyType;

  if ( ( ZSTACK_DEVICE_BUILD & DEVICE_BUILD_COORDINATOR ) != 0 )
  {
    APSME_ConfirmKeyReq( &req );
  }
  
}


/******************************************************************************
 * @fn          ZDSecMgrSendNwkKey
 *
 * @brief       Send NWK key to device joining network.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSendNwkKey( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;
  APSME_TransportKeyReq_t req;
  APSDE_FrameTunnel_t tunnel;
  nwkKeyDesc tmpKey;

  req.dstAddr   = device->nwkAddr;
  req.extAddr   = device->extAddr;

  req.keyType   = KEY_TYPE_NWK;

  // get the Active Key into a local variable
  if ( NLME_ReadNwkKeyInfo( 0, sizeof(tmpKey), &tmpKey,
                           ZCD_NV_NWK_ACTIVE_KEY_INFO ) != SUCCESS )
  {
    // set key data to all 0s if NV read fails
    osal_memset(&tmpKey, 0x00, sizeof(tmpKey));
  }

  if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
  {
    // set values
    req.keySeqNum = tmpKey.keySeqNum;
    req.key       = tmpKey.key;

    //devtag.pro.security.todo - make sure that if there is no link key the NWK
    //key isn't used to secure the frame at the APS layer -- since the receiving
    //device may not have a NWK key yet
    req.apsSecure = TRUE;

    // check if using secure hop to parent
    if ( device->parentAddr == NLME_GetShortAddr() )
    {
      req.nwkSecure = FALSE;
      req.tunnel    = NULL;
    }
    else
    {
      req.nwkSecure   = TRUE;
      req.tunnel      = &tunnel;
      req.tunnel->tna = device->parentAddr;
      req.tunnel->dea = device->extAddr;
    }
  }
  else
  {
    // default values
    //devtag.0604.verify
    req.nwkSecure = TRUE;
    req.apsSecure = FALSE;
    req.tunnel    = NULL;

    if ( device->parentAddr != NLME_GetShortAddr() )
    {
      req.dstAddr = device->parentAddr;
    }

    // send the real key
    if ( zgPreConfigKeys == FALSE )
    {
      req.keySeqNum = tmpKey.keySeqNum;
      req.key       = tmpKey.key;

      // check if using secure hop to to parent
      if ( device->parentAddr == NLME_GetShortAddr() )
      {
        req.nwkSecure = FALSE;
      }
    }
    else
    {
      // this is to send the all zero key when the NWK key has been preconfigured
      req.key       = NULL;
      req.keySeqNum = 0;
    }
  }

  status = APSME_TransportKeyReq( &req );

  // clear copy of key in RAM before return
  osal_memset( &tmpKey, 0x00, sizeof(nwkKeyDesc) );

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceRemoveByExtAddr
 *
 * @brief       Remove device entry by its ext address.
 *
 * @param       pAddr - pointer to the extended address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceRemoveByExtAddr( uint8 *pAddr )
{
  ZDSecMgrEntry_t *pEntry;
  uint8           retValue;

  retValue = (uint8)ZDSecMgrEntryLookupExt( pAddr, &pEntry );

  if( retValue == ZSuccess )
  {
    // remove device from entry data
    ZDSecMgrEntryFree( pEntry );
  }

  return retValue;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceRemove
 *
 * @brief       Remove device from network.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      none
 */
void ZDSecMgrDeviceRemove( ZDSecMgrDevice_t* device )
{
  APSME_RemoveDeviceReq_t remDevReq;
  NLME_LeaveReq_t         leaveReq;
  associated_devices_t*   assoc;
  uint8 TC_ExtAddr[Z_EXTADDR_LEN];

  // check if parent, remove the device
  if ( device->parentAddr == NLME_GetShortAddr() )
  {
    // this is the parent of the device
    leaveReq.extAddr        = device->extAddr;
    leaveReq.removeChildren = FALSE;
    leaveReq.rejoin         = FALSE;

    // find child association
    assoc = AssocGetWithExt( device->extAddr );

    if ( ( assoc != NULL                            ) &&
         ( assoc->nodeRelation >= CHILD_RFD         ) &&
         ( assoc->nodeRelation <= CHILD_FFD_RX_IDLE )    )
    {
      // check if associated device is authenticated
      if ( assoc->devStatus & DEV_SEC_AUTH_STATUS )
      {
        leaveReq.silent = FALSE;
      }
      else
      {
        leaveReq.silent = TRUE;
      }

      NLME_LeaveReq( &leaveReq );
    }
    else if ( device->nwkAddr == NLME_GetShortAddr() )
    {
      // this is when ZC wants that ZR removes itself from the network
      leaveReq.extAddr = NULL;
      leaveReq.silent = FALSE;

      NLME_LeaveReq( &leaveReq );
    }
  }
  else
  {
    // this is not the parent of the device
    remDevReq.parentAddr   = device->parentAddr;
    remDevReq.childExtAddr = device->extAddr;

    if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
    {
      uint8 found;
      APSME_GetRequest( apsTrustCenterAddress,0, TC_ExtAddr );
      
      APSME_SearchTCLinkKeyEntry(TC_ExtAddr,&found,NULL);
      
      // For ZG_GLOBAL_LINK_KEY the message has to be sent twice, one
      // APS un-encrypted and one APS encrypted, to make sure that it can interoperate
      // with legacy Coordinator devices which can only handle one or the other.
#if defined ( APP_TP2_TEST_MODE )
      if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( guTxApsSecON != TRUE ) )
#else
      if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( found == FALSE ) )
#endif
      {
        remDevReq.apsSecure = FALSE;

        APSME_RemoveDeviceReq( &remDevReq );
      }

#if defined ( APP_TP2_TEST_MODE )
      if ( guTxApsSecON != FALSE )
      {
        remDevReq.apsSecure = TRUE;

        APSME_RemoveDeviceReq( &remDevReq );
      }
#else
      remDevReq.apsSecure = TRUE;

      APSME_RemoveDeviceReq( &remDevReq );
#endif
    }
    else
    {
      remDevReq.apsSecure = FALSE;

      APSME_RemoveDeviceReq( &remDevReq );
    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceValidateRM (RESIDENTIAL MODE)
 *
 * @brief       Decide whether device is allowed.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceValidateRM( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;

  status = ZSuccess;

  (void)device;  // Intentionally unreferenced parameter

  // For test purpose, turning off the zgSecurePermitJoin flag will force
  // the trust center to reject any newly joining devices by sending
  // Remove-device to the parents.
  if ( zgSecurePermitJoin == FALSE )
  {
    status = ZNwkUnknownDevice;
  }



#if 0  // Taken out because the following functionality is only used for test
       // purpose. A more efficient (above) way is used. It can be put
       // back in if customers request for a white/black list feature.
       // ZDSecMgrStoredDeviceList[] is defined in ZDSecMgr.c

  // The following code processes the device black list (stored device list)
  // If the joining device is not part of the forbidden device list
  // Return ZSuccess. Otherwise, return ZNwkUnknownDevice. The trust center
  // will send Remove-device and ban the device from joining.

  uint8     index;
  uint8*    restricted;

  // Look through the stored device list - used for restricted devices
  for ( index = 0; index < ZDSECMGR_STORED_DEVICES; index++ )
  {
    restricted = ZDSecMgrStoredDeviceList[index];

    if ( AddrMgrExtAddrEqual( restricted, device->extAddr )  == TRUE )
    {
      // return as unknown device in regards to validation
      status = ZNwkUnknownDevice;

      // break from loop
      index = ZDSECMGR_STORED_DEVICES;
    }
  }

#endif

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceValidate
 *
 * @brief       Decide whether device is allowed.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceValidate( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;

  if ( ZDSecMgrPermitJoiningEnabled == TRUE )
  {
    status = ZDSecMgrDeviceValidateRM( device );
  }
  else
  {
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceJoin
 *
 * @brief       Try to join this device.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceJoin( ZDSecMgrDevice_t* device )
{
  ZStatus_t status = ZSuccess;
  uint16    ami;

  // attempt to validate device that joined/rejoined without security
  if ( device->secure == FALSE )
  {
    status = ZDSecMgrDeviceValidate( device );
  }

  if ( status == ZSuccess )
  {
    // Add the device to the address manager
    ZDSecMgrAddrStore( device->nwkAddr, device->extAddr, &ami );

    // Only send the key to devices that have not been authenticated
    if ( ( device->devStatus & DEV_SEC_INIT_STATUS ) &&
         ( device->secure == FALSE ) )
    {
      //send the nwk key data to the joining device
      status = ZDSecMgrSendNwkKey( device );
    }

    if ( status != ZSuccess )
    {
      ZDSecMgrAddrClear( device->extAddr );
    }
  }

  if ( status != ZSuccess )
  {
    // not allowed or transport key failed, remove the device
    ZDSecMgrDeviceRemove( device );
  }
  else
  {
    // Pass the Trust Center Device Indication to higher layer if callback registered
    if (zdoCBFunc[ZDO_TC_DEVICE_CBID] != NULL )
    {
      ZDO_TC_Device_t dev;

      dev.nwkAddr = device->nwkAddr;
      osal_memcpy( dev.extAddr, device->extAddr, Z_EXTADDR_LEN );
      dev.parentAddr = device->parentAddr;

      zdoCBFunc[ZDO_TC_DEVICE_CBID]( (void*)&dev );
    }
  }
      
      

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceJoinDirect
 *
 * @brief       Try to join this device as a direct child.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceJoinDirect( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;
  
  if(device->secure == FALSE)
  {  
    uint8  found;
    uint16 keyNvIndex;
    APSME_TCLKDevEntry_t TCLKDevEntry;
    
    keyNvIndex = APSME_SearchTCLinkKeyEntry(device->extAddr,&found, &TCLKDevEntry);
    
    //If found and it was verified, then allow it to join in a fresh state by erasing the key entry
    if((found == TRUE) && (TCLKDevEntry.keyAttributes == ZG_VERIFIED_KEY))
    {
      uint16 index;
      TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
      //Increase the shift by one. Validate the maximum shift of the seed which is 15
      TCLKDevEntry.SeedShift_IcIndex++;
      TCLKDevEntry.SeedShift_IcIndex &= 0x0F;
      
      TCLKDevEntry.rxFrmCntr = 0;
      TCLKDevEntry.txFrmCntr = 0;
      
      index = keyNvIndex - ZCD_NV_TCLK_TABLE_START;
      
      TCLinkKeyFrmCntr[index].rxFrmCntr = 0;
      TCLinkKeyFrmCntr[index].txFrmCntr = 0;
      
      //Update the entry
      osal_nv_write(keyNvIndex,0,sizeof(APSME_TCLKDevEntry_t), &TCLKDevEntry );
    }
    
  }

  status = ZDSecMgrDeviceJoin( device );

  if ( status == ZSuccess )
  {
    // set association status to authenticated
    ZDSecMgrAssocDeviceAuth( AssocGetWithShort( device->nwkAddr ) );
    
  #if (ZG_BUILD_COORDINATOR_TYPE)    
    //Add the device as joining device, if it did join unsecured
    if(device->secure == FALSE)
    {
      bdb_TCAddJoiningDevice(NLME_GetShortAddr(),device->extAddr);
    }
  #endif
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceJoinFwd
 *
 * @brief       Forward join to Trust Center.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceJoinFwd( ZDSecMgrDevice_t* device )
{
  ZStatus_t               status;
  APSME_UpdateDeviceReq_t req;
  uint8 TC_ExtAddr[Z_EXTADDR_LEN];

  // forward any joining device to the Trust Center -- the Trust Center will
  // decide if the device is allowed to join
  status = ZSuccess;

  // forward authorization to the Trust Center
  req.dstAddr    = APSME_TRUSTCENTER_NWKADDR;
  req.devAddr    = device->nwkAddr;
  req.devExtAddr = device->extAddr;

  // set security status, option for router to reject if policy set
  if ( (device->devStatus & DEV_HIGH_SEC_STATUS) )
  {
    if ( device->devStatus & DEV_REJOIN_STATUS )
    {
      if ( device->secure == TRUE )
      {
        req.status = APSME_UD_HIGH_SECURED_REJOIN;
      }
      else
      {
        req.status = APSME_UD_HIGH_UNSECURED_REJOIN;
      }
    }
    else
    {
      req.status = APSME_UD_HIGH_UNSECURED_JOIN;
    }
  }
  else
  {
    if ( device->devStatus & DEV_REJOIN_STATUS )
    {
      if ( device->secure == TRUE )
      {
        req.status = APSME_UD_STANDARD_SECURED_REJOIN;
      }
      else
      {
        req.status = APSME_UD_STANDARD_TRUST_CENTER_REJOIN;
      }
    }
    else
    {
      req.status = APSME_UD_STANDARD_UNSECURED_JOIN;
    }
  }

  // set association status to authenticated
  ZDSecMgrAssocDeviceAuth( AssocGetWithShort( device->nwkAddr ) );

  if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
  {
    uint8 found;
    APSME_GetRequest( apsTrustCenterAddress,0, TC_ExtAddr );
    
    APSME_SearchTCLinkKeyEntry(TC_ExtAddr,&found,NULL);
    
    // For ZG_GLOBAL_LINK_KEY the message has to be sent twice one
    // un-encrypted and one APS encrypted, to make sure that it can interoperate
    // with legacy Coordinator devices which can only handle one or the other.
#if defined ( APP_TP2_TEST_MODE )
    if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( guTxApsSecON != TRUE ) )
#else
    if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( found == FALSE ) )
#endif
    {
      req.apsSecure = FALSE;

      // send and APSME_UPDATE_DEVICE request to the trust center
      status = APSME_UpdateDeviceReq( &req );
    }

#if defined ( APP_TP2_TEST_MODE )
    if ( guTxApsSecON != FALSE )
    {
      // send the message APS encrypted
      req.apsSecure = TRUE;

      // send and APSME_UPDATE_DEVICE request to the trust center
      status = APSME_UpdateDeviceReq( &req );
    }
#else
    // send the message APS encrypted
    req.apsSecure = TRUE;

    // send and APSME_UPDATE_DEVICE request to the trust center
    status = APSME_UpdateDeviceReq( &req );
#endif
  }
  else
  {
    req.apsSecure = FALSE;

    // send and APSME_UPDATE_DEVICE request to the trust center
    status = APSME_UpdateDeviceReq( &req );
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceNew
 *
 * @brief       Process a new device.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceNew( ZDSecMgrDevice_t* joiner )
{
  ZStatus_t status;

  if ( ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
      || ( ( ZG_BUILD_RTR_TYPE ) && APSME_IsDistributedSecurity() ) )
  {
    // try to join this device
    status = ZDSecMgrDeviceJoinDirect( joiner );
  }
  else
  {
    status = ZDSecMgrDeviceJoinFwd( joiner );
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAssocDeviceAuth
 *
 * @brief       Set associated device status to authenticated
 *
 * @param       assoc - [in, out] associated_devices_t
 *
 * @return      none
 */
void ZDSecMgrAssocDeviceAuth( associated_devices_t* assoc )
{
  if ( assoc != NULL )
  {
    assoc->devStatus |= DEV_SEC_AUTH_STATUS;
  }
}

/******************************************************************************
 * @fn          ZDSecMgrAuthNwkKey
 *
 * @brief       Handle next step in authentication process
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrAuthNwkKey()
{
  if ( devState == DEV_END_DEVICE_UNAUTH )
  {
    // inform ZDO that device has been authenticated
    osal_set_event ( ZDAppTaskID, ZDO_DEVICE_AUTH );
  }
}

/******************************************************************************
 * PUBLIC FUNCTIONS
 */
/******************************************************************************
 * @fn          ZDSecMgrInit
 *
 * @brief       Initialize ZigBee Device Security Manager.
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
void ZDSecMgrAddrMgrCB( uint8 update, AddrMgrEntry_t* newEntry, AddrMgrEntry_t* oldEntry );
void ZDSecMgrAddrMgrCB( uint8           update,
                        AddrMgrEntry_t* newEntry,
                        AddrMgrEntry_t* oldEntry )
{
  (void)update;
  (void)newEntry;
  (void)oldEntry;
}
#endif // ( ADDRMGR_CALLBACK_ENABLED == 1 )

void ZDSecMgrInit(uint8 state)
{
  if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
  {
    // initialize sub modules
    ZDSecMgrEntryInit(state);

    if ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
    {
      APSME_SetRequest( apsTrustCenterAddress, 0, NLME_GetExtAddr() );
    }

    // register with Address Manager
#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
    AddrMgrRegister( ADDRMGR_REG_SECURITY, ZDSecMgrAddrMgrCB );
#endif
  }

  if ( ZG_SECURE_ENABLED )
  {
    if ( ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
         || ( ( ZG_BUILD_RTR_TYPE ) && APSME_IsDistributedSecurity() ) )
    {
      // setup joining permissions
      ZDSecMgrPermitJoiningEnabled = TRUE;  
      ZDSecMgrPermitJoiningTimed   = FALSE;
    }
  }

  // configure security based on security mode and type of device
  ZDSecMgrConfig();
}

/******************************************************************************
 * @fn          ZDSecMgrConfig
 *
 * @brief       Configure ZigBee Device Security Manager.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrConfig( void )
{
  if ( ZG_SECURE_ENABLED )
  {
    SSP_Init();

    if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
    {
      if ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
      {
        // COMMERCIAL MODE - COORDINATOR DEVICE
        APSME_SecurityCM_CD();
      }
      else if ( ZSTACK_ROUTER_BUILD )
      {
        // COMMERCIAL MODE - ROUTER DEVICE
        APSME_SecurityCM_RD();
      }
      else
      {
        // COMMERCIAL MODE - END DEVICE
        APSME_SecurityCM_ED();
      }
    }
    else // ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_RESIDENTIAL )
    {
      if ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
      {
        // RESIDENTIAL MODE - COORDINATOR DEVICE
        APSME_SecurityRM_CD();
      }
      else if ( ZSTACK_ROUTER_BUILD )
      {
        // RESIDENTIAL MODE - ROUTER DEVICE
        APSME_SecurityRM_RD();
      }
      else
      {
        // RESIDENTIAL MODE - END DEVICE
        APSME_SecurityRM_ED();
      }
    }
  }
  else
  {
    // NO SECURITY
    APSME_SecurityNM();
  }
}

/******************************************************************************
 * @fn          ZDSecMgrPermitJoining
 *
 * @brief       Process request to change joining permissions.
 *
 * @param       duration - [in] timed duration for join in seconds
 *                         - 0x00 not allowed
 *                         - 0xFF allowed without timeout
 *
 * @return      uint8 - success(TRUE:FALSE)
 */
uint8 ZDSecMgrPermitJoining( uint8 duration )
{
  uint8 accept;

  ZDSecMgrPermitJoiningTimed = FALSE;

  if ( duration > 0 )
  {
    ZDSecMgrPermitJoiningEnabled = TRUE;

    ZDSecMgrPermitJoiningTimed = TRUE;
  }
  else
  {
    ZDSecMgrPermitJoiningEnabled = FALSE;
  }

  accept = TRUE;

  return accept;
}

/******************************************************************************
 * @fn          ZDSecMgrPermitJoiningTimeout
 *
 * @brief       Process permit joining timeout
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrPermitJoiningTimeout( void )
{
  if ( ZDSecMgrPermitJoiningTimed == TRUE )
  {
    ZDSecMgrPermitJoiningEnabled = FALSE;
    ZDSecMgrPermitJoiningTimed   = FALSE;
  }
}

/******************************************************************************
 * @fn          ZDSecMgrNewDeviceEvent
 *
 * @brief       Process a the new device event, if found reset new device
 *              event/timer.
 *
 * @param       ShortAddr - of New Device to process
 *
 * @return      uint8 - found(TRUE:FALSE)
 */
uint8 ZDSecMgrNewDeviceEvent( uint16 ShortAddr )
{
  uint8                 found;
  ZDSecMgrDevice_t      device;
  AddrMgrEntry_t        addrEntry;
  associated_devices_t* assoc;
  ZStatus_t             status;

  // initialize return results
  found = FALSE;

  assoc = AssocGetWithShort( ShortAddr );

  if ( assoc != NULL )
  {
    // device found
    found = TRUE;

    // check for preconfigured security
    if ( zgPreConfigKeys == TRUE )
    {
      // set association status to authenticated
      ZDSecMgrAssocDeviceAuth( assoc );
    }

    // set up device info
    addrEntry.user  = ADDRMGR_USER_DEFAULT;
    addrEntry.index = assoc->addrIdx;
    AddrMgrEntryGet( &addrEntry );

    device.nwkAddr    = assoc->shortAddr;
    device.extAddr    = addrEntry.extAddr;
    device.parentAddr = NLME_GetShortAddr();

    // the new device performed Secured Rejoin
    if ( ( assoc->devStatus & DEV_SECURED_JOIN ) &&
         ( assoc->devStatus & DEV_REJOIN_STATUS ) )
    {
      device.secure     = TRUE;
    }
    else
    {
      device.secure     = FALSE;
    }
    device.devStatus  = assoc->devStatus;

    // process new device
    status = ZDSecMgrDeviceNew( &device );

    if ( status == ZSuccess )
    {
      assoc->devStatus &= ~DEV_SEC_INIT_STATUS;
    }
    else
    {
      // Clear SECURITY bit from Address Manager
      ZDSecMgrAddrClear( addrEntry.extAddr );

      // Remove the Association completely
      AssocRemove( addrEntry.extAddr );
    }
  }

  return found;
}

/******************************************************************************
 * @fn          ZDSecMgrTCExtAddrCheck
 *
 * @brief       Verifies if received ext. address matches TC ext. address.
 *
 * @param       extAddr - Extended address to be verified.
 *
 * @return      TRUE - extended address matches
 *              FALSE - otherwise
 */
uint8 ZDSecMgrTCExtAddrCheck( uint8* extAddr )
{
  uint8  lookup[Z_EXTADDR_LEN];
  APSME_GetRequest( apsTrustCenterAddress, 0, lookup );
  return osal_ExtAddrEqual( extAddr, lookup );
}

/******************************************************************************
 * @fn          ZDSecMgrTCDataLoad
 *
 * @brief       Stores the address of TC into address manager.
 *
 * @param       extAddr - Extended address to be verified.
 *
 * @return      none
 */
void ZDSecMgrTCDataLoad( uint8* extAddr )
{
  uint16 ami;
  AddrMgrEntry_t entry;

  // lookup using TC short address
  entry.user    = ADDRMGR_USER_DEFAULT;
  osal_cpyExtAddr( entry.extAddr, extAddr );

  // Verify if TC address has been added to Address Manager
  if ( !APSME_IsDistributedSecurity() && ( AddrMgrEntryLookupExt( &entry ) != TRUE ) )
  {
    ZDSecMgrAddrStore( APSME_TRUSTCENTER_NWKADDR, extAddr, &ami );
  }
}

/******************************************************************************
 * @fn          ZDSecMgrTransportKeyInd
 *
 * @brief       Process the ZDO_TransportKeyInd_t message.
 *
 * @param       ind - [in] ZDO_TransportKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrTransportKeyInd( ZDO_TransportKeyInd_t* ind )
{
  uint8 index;
  uint8 zgPreConfigKey[SEC_KEY_LEN];

  ZDSecMgrUpdateTCAddress( ind->srcExtAddr );
  
#if ZG_BUILD_JOINING_TYPE
  if(ZG_DEVICE_JOINING_TYPE)
  {
    //Update the TC address in the entry
    osal_nv_write(ZCD_NV_TCLK_TABLE_START, osal_offsetof(APSME_TCLKDevEntry_t,extAddr), Z_EXTADDR_LEN, ind->srcExtAddr);
  }
#endif
  
  // check for distributed security
  if ( ( ZG_BUILD_RTR_TYPE ) && osal_isbufset( ind->srcExtAddr, 0xFF, Z_EXTADDR_LEN ) )
  {
    ZDSecMgrPermitJoiningEnabled = TRUE;  
  }
  
  // load Trust Center data if needed
  ZDSecMgrTCDataLoad( ind->srcExtAddr );
  
  if ( ( ind->keyType == KEY_TYPE_NWK ) ||
       ( ind->keyType == 6            ) )
  {
    // check for dummy NWK key (all zeros)
    for ( index = 0;
          ( (index < SEC_KEY_LEN) && (ind->key[index] == 0) );
          index++ );

    if ( index == SEC_KEY_LEN )
    {
      // load preconfigured key - once!!
      if ( !_NIB.nwkKeyLoaded )
      {
        ZDSecMgrReadKeyFromNv(ZCD_NV_PRECFGKEY, zgPreConfigKey);
        SSP_UpdateNwkKey( zgPreConfigKey, 0 );
        SSP_SwitchNwkKey( 0 );

        // clear local copy of key
        osal_memset(zgPreConfigKey, 0x00, SEC_KEY_LEN);
      }
    }
    else
    {
      SSP_UpdateNwkKey( ind->key, ind->keySeqNum );
      if ( !_NIB.nwkKeyLoaded )
      {
        SSP_SwitchNwkKey( ind->keySeqNum );
      }
    }

    // handle next step in authentication process
    ZDSecMgrAuthNwkKey();
  }
  else if ( ind->keyType == KEY_TYPE_TC_LINK )
  {
    uint16 entryIndex;
    uint8 found;
    APSME_TCLKDevEntry_t TCLKDevEntry;
    
    //Search the entry, which should exist at this point
    entryIndex = APSME_SearchTCLinkKeyEntry(ind->srcExtAddr, &found, &TCLKDevEntry);
    
    if(found)
    {
      //If the key was an IC, then erase the entry since that will not longer be used.
      if(TCLKDevEntry.keyAttributes == ZG_PROVISIONAL_KEY)
      {
        APSME_EraseICEntry(&TCLKDevEntry.SeedShift_IcIndex);
      }
      
      TCLKDevEntry.keyAttributes = ZG_UNVERIFIED_KEY;
      TCLKDevEntry.keyType = ZG_UNIQUE_LINK_KEY;
      TCLKDevEntry.rxFrmCntr = 0;
      TCLKDevEntry.txFrmCntr = 0;
      TCLKDevEntry.SeedShift_IcIndex = 0;
      
      //Update the entry
      osal_nv_write(entryIndex,0,sizeof(APSME_TCLKDevEntry_t),&TCLKDevEntry);

      //Create the entry for the key
      if(ZSUCCESS == osal_nv_item_init(ZCD_NV_TCLK_JOIN_DEV,SEC_KEY_LEN,ind->key) )
      {
        //Or replace it if already existed
        osal_nv_write(ZCD_NV_TCLK_JOIN_DEV,0,SEC_KEY_LEN,ind->key);
      }
      
      bdb_tcLinkKeyExchangeAttempt(TRUE,BDB_REQ_VERIFY_TC_LINK_KEY);
    }
  }
  else if ( ind->keyType == KEY_TYPE_APP_LINK )
  {
    if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
    {
      uint16           ami;
      ZDSecMgrEntry_t* entry;

      // get the address index
      if ( ZDSecMgrExtAddrLookup( ind->srcExtAddr, &ami ) != ZSuccess )
      {
        // store new EXT address
        ZDSecMgrAddrStore( INVALID_NODE_ADDR, ind->srcExtAddr, &ami );
        ZDP_NwkAddrReq( ind->srcExtAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
      }

      ZDSecMgrEntryLookupAMI( ami, &entry );

      if ( entry == NULL )
      {
        // get new entry
        if ( ZDSecMgrEntryNew( &entry ) == ZSuccess )
        {
          // finish setting up entry
          entry->ami = ami;
        }
      }

      ZDSecMgrLinkKeySet( ind->srcExtAddr, ind->key );

#if defined NV_RESTORE
      ZDSecMgrWriteNV();  // Write the control record for the new established link key to NV.
#endif
    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrUpdateDeviceInd
 *
 * @brief       Process the ZDO_UpdateDeviceInd_t message.
 *
 * @param       ind - [in] ZDO_UpdateDeviceInd_t indication
 *
 * @return      none
 */
void ZDSecMgrUpdateDeviceInd( ZDO_UpdateDeviceInd_t* ind )
{
  ZDSecMgrDevice_t device;

  device.nwkAddr    = ind->devAddr;
  device.extAddr    = ind->devExtAddr;
  device.parentAddr = ind->srcAddr;
  device.devStatus  = DEV_SEC_INIT_STATUS;
  device.secure     = FALSE;

  // Trust Center should identify the type of JOIN/REJOIN and
  // Transport the NWK key accordingly, it will only be transported for:
  //              APSME_UD_STANDARD_UNSECURED_JOIN
  //   OR         APSME_UD_STANDARD_TRUST_CENTER_REJOIN
  if ( ind->status != APSME_UD_DEVICE_LEFT )
  {
    if ( ind->status == APSME_UD_STANDARD_SECURED_REJOIN )
    {
      device.devStatus &= ~DEV_SEC_INIT_STATUS;
      device.devStatus |=  DEV_SEC_AUTH_STATUS;
      device.secure = TRUE;
    }
    else
    {
#if (ZG_BUILD_COORDINATOR_TYPE)
      uint8  found;
      uint16 keyNvIndex;
      APSME_TCLKDevEntry_t TCLKDevEntry;
      
      keyNvIndex = APSME_SearchTCLinkKeyEntry(device.extAddr,&found, &TCLKDevEntry);
      
      //If found and it was verified, then allow it to join in a fresh state by erasing the key entry
      if((found == TRUE) && (TCLKDevEntry.keyAttributes == ZG_VERIFIED_KEY))
      {
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
        //Increase the shift by one. Validate the maximum shift of the seed which is 15
        TCLKDevEntry.SeedShift_IcIndex++;
        TCLKDevEntry.SeedShift_IcIndex &= 0x0F;
        
        TCLKDevEntry.rxFrmCntr = 0;
        TCLKDevEntry.txFrmCntr = 0;  
        
        //Update the entry
        osal_nv_write(keyNvIndex,0,sizeof(APSME_TCLKDevEntry_t), &TCLKDevEntry );
      }
      
      bdb_TCAddJoiningDevice(device.parentAddr,device.extAddr);
#endif
    
    }

    ZDSecMgrDeviceJoin( &device );
  }
}

/******************************************************************************
 * @fn          ZDSecMgrRemoveDeviceInd
 *
 * @brief       Process the ZDO_RemoveDeviceInd_t message.
 *
 * @param       ind - [in] ZDO_RemoveDeviceInd_t indication
 *
 * @return      none
 */
void ZDSecMgrRemoveDeviceInd( ZDO_RemoveDeviceInd_t* ind )
{
  ZDSecMgrDevice_t device;

  // only accept from Trust Center
  if ( ind->srcAddr == APSME_TRUSTCENTER_NWKADDR )
  {
    // look up NWK address
    if ( APSME_LookupNwkAddr( ind->childExtAddr, &device.nwkAddr ) == TRUE )
    {
      device.parentAddr = NLME_GetShortAddr();
      device.extAddr    = ind->childExtAddr;

      // remove device
      ZDSecMgrDeviceRemove( &device );
    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrRequestKeyInd
 *
 * @brief       Process the ZDO_RequestKeyInd_t message.
 *
 * @param       ind - [in] ZDO_RequestKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrRequestKeyInd( ZDO_RequestKeyInd_t* ind )
{
  if ( ind->keyType == KEY_TYPE_NWK )
  {
  }
  else if ( ind->keyType == KEY_TYPE_APP_MASTER )
  {
    ZDSecMgrAppKeyReq( ind );
  }
  else if ( ind->keyType == KEY_TYPE_TC_LINK )
  {
    ZDSecMgrTclkReq( ind );
  }
  //else ignore
}


/******************************************************************************
 * @fn          ZDSecMgrVerifyKeyInd
 *
 * @brief       Process the ZDO_VerifyKeyInd_t message.
 *
 * @param       ind - [in] ZDO_VerifyKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrVerifyKeyInd( ZDO_VerifyKeyInd_t* ind )
{
  ZDSecMgrAppConfKeyReq( ind );
}


/******************************************************************************
 * @fn          ZDSecMgrSwitchKeyInd
 *
 * @brief       Process the ZDO_SwitchKeyInd_t message.
 *
 * @param       ind - [in] ZDO_SwitchKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrSwitchKeyInd( ZDO_SwitchKeyInd_t* ind )
{
  SSP_SwitchNwkKey( ind->keySeqNum );

  // Save if nv
  ZDApp_NVUpdate();
}
  
/******************************************************************************
 * @fn          ZDSecMgrGenerateSeed
 *
 * @brief       Generate the seed for TC link keys and store it in Nv
 *
 * @param       SetDefault, force to use new seed
 *
 * @return      none
 */
void ZDSecMgrGenerateSeed(uint8 SetDefault)
{
  uint8 SeedKey[SEC_KEY_LEN];  
  
  ZDSecMgrGenerateRndKey(SeedKey);
  
  if((SUCCESS == osal_nv_item_init(ZCD_NV_TCLK_SEED,SEC_KEY_LEN,SeedKey)) && SetDefault)
  {
    //Force to use a new seed
    osal_nv_write(ZCD_NV_TCLK_SEED,0,SEC_KEY_LEN,SeedKey);
  }

  osal_memset(SeedKey,0,SEC_KEY_LEN);
}


/******************************************************************************
 * @fn          ZDSecMgrGenerateKeyFromSeed
 *
 * @brief       Generate the TC link key for an specific device usign seed and ExtAddr
 *
 * @param       [in]  extAddr  
 * @param       [in]  shift    number of byte shifts that the seed will do to 
 *                             generate a new key for the same device. 
 *                             This value must be less than SEC_KEY_LEN
 * @param       [out] key      buffer in which the key will be copied
 *
 * @return      none
 */
void ZDSecMgrGenerateKeyFromSeed(uint8 *extAddr, uint8 shift, uint8 *key)
{
  uint8 i;
  uint8 tempKey[SEC_KEY_LEN];
  
  if((key != NULL) && (extAddr != NULL))
  {
    //Read the key
    osal_nv_read(ZCD_NV_TCLK_SEED,0,SEC_KEY_LEN,tempKey);

    //shift the seed
    osal_memcpy(key, &tempKey[shift], SEC_KEY_LEN - shift);
    osal_memcpy(&key[SEC_KEY_LEN - shift], tempKey, shift);
   
    //Create the key from the seed
    for(i = 0; i < Z_EXTADDR_LEN; i++)
    {
      key[i] ^= extAddr[i];
      key[i+Z_EXTADDR_LEN] ^= extAddr[i];
    }
  }
}


/******************************************************************************
 * @fn          ZDSecMgrGenerateRndKey
 *
* @brief       Generate a random key. NOTE: Random key is generated by osal_rand, refer to osal_rand to see the random properties of the key generated by this mean.
 *
 * @param       pKey - [out] Buffer pointer in which the key will be passed.
 *
 * @return      none
 */
void ZDSecMgrGenerateRndKey(uint8* pKey)
{
  uint16  temp;
  uint8   index = 0;
  
  while(index < (SEC_KEY_LEN/2))
  {
    temp = osal_rand();
    pKey[index*2]   = (uint8) (temp & 0x00FF);
    pKey[index*2+1] = (uint8) ((temp >> 8) & 0x00FF);
    index++;
  }
}


#if ( ZG_BUILD_COORDINATOR_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrUpdateNwkKey
 *
 * @brief       Load a new NWK key and trigger a network update to the dstAddr.
 *
 * @param       key       - [in] new NWK key
 * @param       keySeqNum - [in] new NWK key sequence number
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrUpdateNwkKey( uint8* key, uint8 keySeqNum, uint16 dstAddr )
{
  ZStatus_t               status;
  APSME_TransportKeyReq_t req;

  // initialize common elements of local variables
  req.keyType   = KEY_TYPE_NWK;

  req.dstAddr   = dstAddr;
  req.keySeqNum = keySeqNum;
  req.key       = key;
  req.extAddr   = NULL;
  req.nwkSecure = TRUE;
  req.tunnel    = NULL;

  if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
  {
    // Broadcast transport NWK key
    if (( dstAddr == NWK_BROADCAST_SHORTADDR_DEVALL ) ||
        ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVZCZR) ||
        ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVRXON))
    {
      req.apsSecure = FALSE;
      status = APSME_TransportKeyReq( &req );
    }
    else
    {
      AddrMgrEntry_t          addrEntry;

      addrEntry.user = ADDRMGR_USER_SECURITY;
      addrEntry.nwkAddr = dstAddr;

      status = ZFailure;

      if ( AddrMgrEntryLookupNwk( &addrEntry ) == TRUE )
      {
        req.extAddr = addrEntry.extAddr;
        req.apsSecure = TRUE;
        status = APSME_TransportKeyReq( &req );
      }
    }
  }
  else // ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_RESIDENTIAL )
  {
    req.apsSecure = FALSE;
    status = APSME_TransportKeyReq( &req );
  }

  SSP_UpdateNwkKey( key, keySeqNum );

  // Save if nv
  ZDApp_NVUpdate();

  return status;
}
#endif // ( ZG_BUILD_COORDINATOR_TYPE )

#if ( ZG_BUILD_COORDINATOR_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrSwitchNwkKey
 *
 * @brief       Causes the NWK key to switch via a network command to the dstAddr.
 *
 * @param       keySeqNum - [in] new NWK key sequence number
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSwitchNwkKey( uint8 keySeqNum, uint16 dstAddr )
{
  ZStatus_t            status;
  APSME_SwitchKeyReq_t req;

  // initialize common elements of local variables
  req.dstAddr = dstAddr;
  req.keySeqNum = keySeqNum;

  if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
  {
    // Broadcast switch NWK key
    if (( dstAddr == NWK_BROADCAST_SHORTADDR_DEVALL ) ||
        ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVZCZR) ||
        ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVRXON))
    {
      req.apsSecure = FALSE;
      status = APSME_SwitchKeyReq( &req );
    }
    else
    {
      AddrMgrEntry_t          addrEntry;

      addrEntry.user = ADDRMGR_USER_SECURITY;
      addrEntry.nwkAddr = dstAddr;

      status = ZFailure;

      if ( AddrMgrEntryLookupNwk( &addrEntry ) == TRUE )
      {
        req.dstAddr = addrEntry.nwkAddr;
        req.apsSecure = TRUE;
        status = APSME_SwitchKeyReq( &req );
      }
    }
  }
  else // ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_RESIDENTIAL )
  {
    req.apsSecure = FALSE;
    status = APSME_SwitchKeyReq( &req );
  }

  if ( dstAddr >= NWK_BROADCAST_SHORTADDR_DEVZCZR)
  {
    zgSwitchCoordKey = TRUE;
    zgSwitchCoordKeyIndex = keySeqNum;
  }
  // Save if nv
  ZDApp_NVUpdate();

  return status;
}
#endif // ( ZG_BUILD_COORDINATOR_TYPE )

/******************************************************************************
 * @fn          ZDSecMgrRequestAppKey
 *
 * @brief       Request an application key with partner.
 *
 * @param       partExtAddr - [in] partner extended address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrRequestAppKey( uint8 *partExtAddr )
{
  ZStatus_t status;
  APSME_RequestKeyReq_t req;

  req.dstAddr = 0;
  req.keyType = KEY_TYPE_APP_MASTER;

  req.partExtAddr = partExtAddr;
  status = APSME_RequestKeyReq( &req );

  return status;
}

#if ( ZG_BUILD_JOINING_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrSetupPartner
 *
 * @brief       Setup for application key partner.
 *
 * @param       partNwkAddr - [in] partner network address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSetupPartner( uint16 partNwkAddr, uint8* partExtAddr )
{
  AddrMgrEntry_t entry;
  ZStatus_t      status;

  status = ZFailure;

  // update the address manager
  entry.user    = ADDRMGR_USER_SECURITY;
  entry.nwkAddr = partNwkAddr;
  AddrMgrExtAddrSet( entry.extAddr, partExtAddr );

  if ( AddrMgrEntryUpdate( &entry ) == TRUE )
  {
    status = ZSuccess;

    // check for address discovery
    if ( partNwkAddr == INVALID_NODE_ADDR )
    {
      status = ZDP_NwkAddrReq( partExtAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
    }
    else if ( !AddrMgrExtAddrValid( partExtAddr ) )
    {
      status = ZDP_IEEEAddrReq( partNwkAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
    }
  }

  return status;
}
#endif // ( ZG_BUILD_JOINING_TYPE )

#if ( ZG_BUILD_COORDINATOR_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrAppKeyTypeSet
 *
 * @brief       Set application key type.
 *
 * @param       keyType - [in] application key type (KEY_TYPE_APP_MASTER@2 or
 *                                                   KEY_TYPE_APP_LINK@3
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAppKeyTypeSet( uint8 keyType )
{
  if ( keyType == KEY_TYPE_APP_LINK )
  {
    ZDSecMgrAppKeyType = KEY_TYPE_APP_LINK;
  }
  else
  {
    ZDSecMgrAppKeyType = KEY_TYPE_APP_MASTER;
  }

  return ZSuccess;
}
#endif

/******************************************************************************
 * ZigBee Device Security Manager - Stub Implementations
 */


/******************************************************************************
 * @fn          ZDSecMgrLinkKeySet (stubs APSME_LinkKeySet)
 *
 * @brief       Set <APSME_LinkKeyData_t> for specified NWK address.
 *
 * @param       extAddr - [in] EXT address
 * @param       data    - [in] APSME_LinkKeyData_t
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrLinkKeySet( uint8* extAddr, uint8* key )
{
  ZStatus_t status;
  ZDSecMgrEntry_t* entry;
  APSME_LinkKeyData_t *pApsLinkKey = NULL;
  uint16 Index;

  // lookup entry index for specified EXT address
  status = ZDSecMgrEntryLookupExtGetIndex( extAddr, &entry, &Index );

  if ( status == ZSuccess )
  {
    // point to NV item
    entry->keyNvId = ZCD_NV_APS_LINK_KEY_DATA_START + Index;

    pApsLinkKey = (APSME_LinkKeyData_t *)osal_mem_alloc(sizeof(APSME_LinkKeyData_t));

    if (pApsLinkKey != NULL)
    {
      // read the key form NV, keyNvId must be ZCD_NV_APS_LINK_KEY_DATA_START based
      osal_nv_read( entry->keyNvId, 0,
                   sizeof(APSME_LinkKeyData_t), pApsLinkKey );

      // set new values of the key
      osal_memcpy( pApsLinkKey->key, key, SEC_KEY_LEN );
      pApsLinkKey->rxFrmCntr = 0;
      pApsLinkKey->txFrmCntr = 0;

      osal_nv_write( entry->keyNvId, 0,
                    sizeof(APSME_LinkKeyData_t), pApsLinkKey );

      // clear copy of key in RAM
      osal_memset(pApsLinkKey, 0x00, sizeof(APSME_LinkKeyData_t));

      osal_mem_free(pApsLinkKey);

      // set initial values for counters in RAM
      ApsLinkKeyFrmCntr[entry->keyNvId - ZCD_NV_APS_LINK_KEY_DATA_START].txFrmCntr = 0;
      ApsLinkKeyFrmCntr[entry->keyNvId - ZCD_NV_APS_LINK_KEY_DATA_START].rxFrmCntr = 0;
    }
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAuthenticationSet
 *
 * @brief       Mark the specific device as authenticated or not
 *
 * @param       extAddr - [in] EXT address
 * @param       option  - [in] authenticated or not
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAuthenticationSet( uint8* extAddr, ZDSecMgr_Authentication_Option option )
{
  ZStatus_t        status;
  ZDSecMgrEntry_t* entry;


  // lookup entry index for specified EXT address
  status = ZDSecMgrEntryLookupExt( extAddr, &entry );

  if ( status == ZSuccess )
  {
    entry->authenticateOption = option;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAuthenticationCheck
 *
 * @brief       Check if the specific device has been authenticated or not
 *              For non-trust center device, always return TRUE
 *
 * @param       shortAddr - [in] short address
 *
 * @return      TRUE @ authenticated with CBKE
 *              FALSE @ not authenticated
 */

uint8 ZDSecMgrAuthenticationCheck( uint16 shortAddr )
{
#if defined (TC_LINKKEY_JOIN)

  ZDSecMgrEntry_t* entry;
  uint8 extAddr[Z_EXTADDR_LEN];

  // If the local device is not the trust center, always return TRUE
  APSME_GetRequest( apsTrustCenterAddress, 0, extAddr );
  if ( ! osal_ExtAddrEqual( extAddr , NLME_GetExtAddr() ) )
  {
    return TRUE;
  }
  // Otherwise, check the authentication option
  else if ( AddrMgrExtAddrLookup( shortAddr, extAddr ) )
  {
    // lookup entry index for specified EXT address
    if ( ZDSecMgrEntryLookupExt( extAddr, &entry ) == ZSuccess )
    {
      if ( entry->authenticateOption != ZDSecMgr_Not_Authenticated )
      {
        return TRUE;
      }
      else
      {
        return FALSE;
      }
    }
    else
    {
      // it may have been secured with TCLK only
      uint16    ami;

      // lookup address index in address manager
      if ( ZDSecMgrExtAddrLookup( extAddr, &ami ) == ZSuccess )
      {
        return TRUE;
      }
    }
  }
  return FALSE;

#else
  (void)shortAddr;  // Intentionally unreferenced parameter

  // For non AMI/SE Profile, perform no check and always return TRUE.
  return TRUE;

#endif // TC_LINKKEY_JOIN
}


/******************************************************************************
 * @fn          ZDSecMgrLinkKeyNVIdGet (stubs APSME_LinkKeyNVIdGet)
 *
 * @brief       Get Key NV ID for specified NWK address.
 *
 * @param       extAddr - [in] EXT address
 * @param       keyNvId - [out] NV ID
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrLinkKeyNVIdGet(uint8* extAddr, uint16 *pKeyNvId)
{
  ZStatus_t status;
  ZDSecMgrEntry_t* entry;

  // lookup entry index for specified NWK address
  status = ZDSecMgrEntryLookupExt( extAddr, &entry );

  if ( status == ZSuccess )
  {
    // return the index to the NV table
    *pKeyNvId = entry->keyNvId;
  }
  else
  {
    *pKeyNvId = SEC_NO_KEY_NV_ID;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrIsLinkKeyValid (stubs APSME_IsLinkKeyValid)
 *
 * @brief       Verifies if Link Key in NV has been set.
 *
 * @param       extAddr - [in] EXT address
 *
 * @return      TRUE - Link Key has been established
 *              FALSE - Link Key in NV has default value.
 */
uint8 ZDSecMgrIsLinkKeyValid(uint8* extAddr)
{
  APSME_LinkKeyData_t *pKeyData;
  uint16 apsLinkKeyNvId;
  uint8 nullKey[SEC_KEY_LEN];
  uint8 status = FALSE;

  // initialize default vealue to compare to
  osal_memset(nullKey, 0x00, SEC_KEY_LEN);

  // check for APS link NV ID
  APSME_LinkKeyNVIdGet( extAddr, &apsLinkKeyNvId );

  if (apsLinkKeyNvId != SEC_NO_KEY_NV_ID )
  {
    pKeyData = (APSME_LinkKeyData_t *)osal_mem_alloc(sizeof(APSME_LinkKeyData_t));

    if (pKeyData != NULL)
    {
      // retrieve key from NV
      if ( osal_nv_read( apsLinkKeyNvId, 0,
                        sizeof(APSME_LinkKeyData_t), pKeyData) == ZSUCCESS)
      {
        // if stored key is different than default value, then a key has been established
        if (!osal_memcmp(pKeyData, nullKey, SEC_KEY_LEN))
        {
          status = TRUE;
        }
      }

      // clear copy of key in RAM
      osal_memset(pKeyData, 0x00, sizeof(APSME_LinkKeyData_t));

      osal_mem_free(pKeyData);
    }
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrKeyFwdToChild (stubs APSME_KeyFwdToChild)
 *
 * @brief       Verify and process key transportation to child.
 *
 * @param       ind - [in] APSME_TransportKeyInd_t
 *
 * @return      uint8 - success(TRUE:FALSE)
 */
uint8 ZDSecMgrKeyFwdToChild( APSME_TransportKeyInd_t* ind )
{
  // verify from Trust Center
  if ( ind->srcAddr == APSME_TRUSTCENTER_NWKADDR )
  {
    // check for initial NWK key
    if ( ( ind->keyType == KEY_TYPE_NWK ) ||
         ( ind->keyType == 6            ) )
    {
      // set association status to authenticated
      ZDSecMgrAssocDeviceAuth( AssocGetWithExt( ind->dstExtAddr ) );
    }

    return TRUE;
  }

  return FALSE;
}

/******************************************************************************
 * @fn          ZDSecMgrAddLinkKey
 *
 * @brief       Add the application link key to ZDSecMgr. Also mark the device
 *              as authenticated in the authenticateOption. Note that this function
 *              is hardwared to CBKE right now.
 *
 * @param       shortAddr - short address of the partner device
 * @param       extAddr - extended address of the partner device
 * @param       key - link key
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAddLinkKey( uint16 shortAddr, uint8 *extAddr, uint8 *key)
{
  uint16           ami;
  ZDSecMgrEntry_t* entry;

  /* Store the device address in the addr manager */
  if( ZDSecMgrAddrStore( shortAddr, extAddr, &ami ) != ZSuccess )
  {
    /* Adding to Addr Manager fails */
    return ZFailure;
  }

  /* Lookup entry using specified address index */
  ZDSecMgrEntryLookupAMI( ami, &entry );

  // If no existing entry, create one
  if ( entry == NULL )
  {
    if ( ZDSecMgrEntryNew( &entry ) == ZSuccess )
    {
      entry->ami = ami;
    }
    else
    {
      /* Security Manager full */
      return ZBufferFull;
    }
  }
  // Write the link key
  APSME_LinkKeySet( extAddr, key );

#if defined (TC_LINKKEY_JOIN)
  // Mark the device as authenticated.
  ZDSecMgrAuthenticationSet( extAddr, ZDSecMgr_Authenticated_CBCK );
#endif

#if defined NV_RESTORE
  ZDSecMgrWriteNV();  // Write the new established link key to NV.
#endif

  return ZSuccess;
}

/******************************************************************************
 * @fn          ZDSecMgrInitNV
 *
 * @brief       Initialize the SecMgr entry data in NV with all values set to 0
 *
 * @param       none
 *
 * @return      uint8 - <osal_nv_item_init> return codes
 */
uint8 ZDSecMgrInitNV(void)
{

  uint8 rtrn = osal_nv_item_init(ZCD_NV_APS_LINK_KEY_TABLE,
                (sizeof(nvDeviceListHdr_t) + (sizeof(ZDSecMgrEntry_t) * ZDSECMGR_ENTRY_MAX)), NULL);

  // If the item does not already exist, set all values to 0
  if (rtrn == NV_ITEM_UNINIT)
  {
    nvDeviceListHdr_t hdr;
    hdr.numRecs = 0;
    osal_nv_write(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr);
  }

  rtrn |= osal_nv_item_init( ZCD_NV_TRUSTCENTER_ADDR, Z_EXTADDR_LEN,
                             zgApsTrustCenterAddr );

  return rtrn;
}

#if defined ( NV_RESTORE )
/*********************************************************************
 * @fn      ZDSecMgrWriteNV()
 *
 * @brief   Save off the APS link key list to NV
 *
 * @param   none
 *
 * @return  none
 */
static void ZDSecMgrWriteNV( void )
{
  uint16 i;
  nvDeviceListHdr_t hdr;

  hdr.numRecs = 0;

  if (ZDSecMgrEntries != NULL)
  {
    for ( i = 0; i < ZDSECMGR_ENTRY_MAX; i++ )
    {
      // Save off the record
      osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE,
                    (uint16)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))),
                    sizeof(ZDSecMgrEntry_t), &ZDSecMgrEntries[i] );

      if ( ZDSecMgrEntries[i].ami != INVALID_NODE_ADDR )
      {
        hdr.numRecs++;
      }
    }
  }

  // Save off the header
  osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof( nvDeviceListHdr_t ), &hdr );
}
#endif // NV_RESTORE

#if defined ( NV_RESTORE )
/******************************************************************************
 * @fn          ZDSecMgrRestoreFromNV
 *
 * @brief       Restore the APS Link Key entry data from NV. It does not restore
 *              the key data itself as they remain in NV until they are used.
 *              Only list data is restored.
 *              Restore zgTrustCenterAdress from NV.
 *
 * @param       none
 *
 * @return      None.
 */
static void ZDSecMgrRestoreFromNV( void )
{
  nvDeviceListHdr_t hdr;
  APSME_LinkKeyData_t *pApsLinkKey = NULL;

  if ((osal_nv_read(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr) == ZSUCCESS) &&
      ((hdr.numRecs > 0) && (hdr.numRecs <= ZDSECMGR_ENTRY_MAX)))
  {
    uint8 x;

    pApsLinkKey = (APSME_LinkKeyData_t *)osal_mem_alloc(sizeof(APSME_LinkKeyData_t));

    for (x = 0; x < ZDSECMGR_ENTRY_MAX; x++)
    {
      if ( osal_nv_read( ZCD_NV_APS_LINK_KEY_TABLE,
                        (uint16)(sizeof(nvDeviceListHdr_t) + (x * sizeof(ZDSecMgrEntry_t))),
                        sizeof(ZDSecMgrEntry_t), &ZDSecMgrEntries[x] ) == SUCCESS )
      {
        // update data only for valid entries
        if ( ZDSecMgrEntries[x].ami != INVALID_NODE_ADDR )
        {
          if (pApsLinkKey != NULL)
          {
            // read the key form NV, keyNvId must be ZCD_NV_APS_LINK_KEY_DATA_START based
            osal_nv_read( ZDSecMgrEntries[x].keyNvId, 0,
                         sizeof(APSME_LinkKeyData_t), pApsLinkKey );

            // set new values for the counter
            pApsLinkKey->txFrmCntr += ( MAX_APS_FRAMECOUNTER_CHANGES + 1 );

            // restore values for counters in RAM
            ApsLinkKeyFrmCntr[ZDSecMgrEntries[x].keyNvId - ZCD_NV_APS_LINK_KEY_DATA_START].txFrmCntr =
                                            pApsLinkKey->txFrmCntr;

            ApsLinkKeyFrmCntr[ZDSecMgrEntries[x].keyNvId - ZCD_NV_APS_LINK_KEY_DATA_START].rxFrmCntr =
                                            pApsLinkKey->rxFrmCntr;

            osal_nv_write( ZDSecMgrEntries[x].keyNvId, 0,
                          sizeof(APSME_LinkKeyData_t), pApsLinkKey );

            // clear copy of key in RAM
            osal_memset(pApsLinkKey, 0x00, sizeof(APSME_LinkKeyData_t));
          }
        }
      }
    }

    if (pApsLinkKey != NULL)
    {
      osal_mem_free(pApsLinkKey);
    }
  }

  osal_nv_read( ZCD_NV_TRUSTCENTER_ADDR, 0, Z_EXTADDR_LEN, zgApsTrustCenterAddr );
}
#endif // NV_RESTORE

/*********************************************************************
 * @fn          ZDSecMgrSetDefaultNV
 *
 * @brief       Write the defaults to NV for Entry table and for APS key data table
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrSetDefaultNV( void )
{
  uint16 i;
  nvDeviceListHdr_t hdr;
  ZDSecMgrEntry_t secMgrEntry;
  APSME_LinkKeyData_t *pApsLinkKey = NULL;

  // Initialize the header
  hdr.numRecs = 0;

  // clear the header
  osal_nv_write(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr);

  osal_memset( &secMgrEntry, 0x00, sizeof(ZDSecMgrEntry_t) );

  for ( i = 0; i < ZDSECMGR_ENTRY_MAX; i++ )
  {
    // Clear the record
    osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE,
                (uint16)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))),
                        sizeof(ZDSecMgrEntry_t), &secMgrEntry );
  }

  pApsLinkKey = (APSME_LinkKeyData_t *)osal_mem_alloc(sizeof(APSME_LinkKeyData_t));

  if (pApsLinkKey != NULL)
  {
    osal_memset( pApsLinkKey, 0x00, sizeof(APSME_LinkKeyData_t) );

    for ( i = 0; i < ZDSECMGR_ENTRY_MAX; i++ )
    {
      // Clear the record
      osal_nv_write( (ZCD_NV_APS_LINK_KEY_DATA_START + i), 0,
                    sizeof(APSME_LinkKeyData_t), pApsLinkKey);
    }

    osal_mem_free(pApsLinkKey);
  }
}

#if defined ( NV_RESTORE )
/*********************************************************************
 * @fn      ZDSecMgrUpdateNV()
 *
 * @brief   Updates one entry of the APS link key table to NV
 *
 * @param   index - to the entry in security manager table
 *
 * @return  none
 */
static void ZDSecMgrUpdateNV( uint16 index )
{
  nvDeviceListHdr_t hdr;

  if (ZDSecMgrEntries != NULL)
  {
    // Save off the record
    osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE,
                   (uint16)((sizeof(nvDeviceListHdr_t)) + (index * sizeof(ZDSecMgrEntry_t))),
                   sizeof(ZDSecMgrEntry_t), &ZDSecMgrEntries[index] );
  }

  if (osal_nv_read(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr) == ZSUCCESS)
  {
    if ( ZDSecMgrEntries[index].ami == INVALID_NODE_ADDR )
    {
      if (hdr.numRecs > 0)
      {
        hdr.numRecs--;
      }
    }
    else
    {
      hdr.numRecs++;
    }

    // Save off the header
    osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof( nvDeviceListHdr_t ), &hdr );
  }
}
#endif // NV_RESTORE

/******************************************************************************
 * @fn          ZDSecMgrAPSRemove
 *
 * @brief       Remove device from network.
 *
 * @param       nwkAddr - device's NWK address
 * @param       extAddr - device's Extended address
 * @param       parentAddr - parent's NWK address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAPSRemove( uint16 nwkAddr, uint8 *extAddr, uint16 parentAddr )
{
  ZDSecMgrDevice_t device;

  if ( ( nwkAddr == INVALID_NODE_ADDR ) ||
       ( extAddr == NULL )              ||
       ( parentAddr == INVALID_NODE_ADDR ) )
  {
    return ( ZFailure );
  }

  device.nwkAddr = nwkAddr;
  device.extAddr = extAddr;
  device.parentAddr = parentAddr;

  // remove device
  ZDSecMgrDeviceRemove( &device );

  return ( ZSuccess );
}

/******************************************************************************
 * @fn          APSME_TCLinkKeyInit
 *
 * @brief       Initialize the NV table for preconfigured TC link key
 *
 *              When zgUseDefaultTCL is set to TRUE, the default preconfig
 *              Trust Center Link Key is written to NV. A single tclk is used
 *              by all devices joining the network.
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
void APSME_TCLinkKeyInit(uint8 setDefault)
{
  APSME_TCLKDevEntry_t TCLKDevEntry;
  uint8                rtrn;
  uint16               i;
  
  // Clear the data for the keys
  osal_memset( &TCLKDevEntry, 0x00, sizeof(APSME_TCLKDevEntry_t) );
  TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
  
  // Initialize all NV items
  for( i = 0; i < gZDSECMGR_TC_DEVICE_MAX; i++ )
  {
    // If the item doesn't exist in NV memory, create and initialize
    // it with the default value passed in, either defaultTCLK or 0
    rtrn = osal_nv_item_init( (ZCD_NV_TCLK_TABLE_START + i),
                               sizeof(APSME_TCLKDevEntry_t), &TCLKDevEntry);

    if (rtrn == SUCCESS)
    {
      if(setDefault)
      {
        //Force to initialize the entry
        osal_nv_write(ZCD_NV_TCLK_TABLE_START + i, 0, sizeof(APSME_TCLKDevEntry_t), &TCLKDevEntry);
        TCLinkKeyFrmCntr[i].txFrmCntr = 0;
        TCLinkKeyFrmCntr[i].rxFrmCntr = 0;
      }
      else
      {
        // set the Frame counters to 0 to existing keys in NV
        osal_nv_read( ( ZCD_NV_TCLK_TABLE_START + i), 0,
                       sizeof(APSME_TCLKDevEntry_t), &TCLKDevEntry );

        // increase the value stored in NV
        TCLKDevEntry.txFrmCntr += ( MAX_TCLK_FRAMECOUNTER_CHANGES + 1 );

        osal_nv_write( ( ZCD_NV_TCLK_TABLE_START + i), 0,
                        sizeof(APSME_TCLKDevEntry_t), &TCLKDevEntry );

        // set initial values for counters in RAM
        TCLinkKeyFrmCntr[i].txFrmCntr = TCLKDevEntry.txFrmCntr;
        TCLinkKeyFrmCntr[i].rxFrmCntr = TCLKDevEntry.rxFrmCntr;
        
        // Making sure data is cleared and set to default for every key all the time
        osal_memset( &TCLKDevEntry, 0x00, sizeof(APSME_TCLKDevEntry_t) );
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
      }
    }
  }

  if(setDefault)
  {
    //Force to erase all IC
    APSME_EraseICEntry(NULL);
  }
}


/******************************************************************************
 * @fn          APSME_TCLinkKeySync
 *
 * @brief       Sync Trust Center LINK key data.
 *
 * @param       srcAddr - [in] srcAddr
 * @param       si      - [in, out] SSP_Info_t
 *
 * @return      ZStatus_t
 */
ZStatus_t APSME_TCLinkKeySync( uint16 srcAddr, SSP_Info_t* si )
{
  APSME_TCLKDevEntry_t  TCLKDevEntry;
  ZStatus_t             status = ZSecNoKey;
  uint32                *tclkRxFrmCntr;
  uint16                entryIndex = 0xFFFF;
  uint16                selectedId = gZDSECMGR_TC_DEVICE_MAX;
  uint8                 entryFound = FALSE;
  
#if ZG_BUILD_JOINING_TYPE
  uint8   defaultEntry[Z_EXTADDR_LEN];
#endif
 
  // Look up the IEEE address of the trust center if it's available
  if ( AddrMgrExtAddrValid( si->extAddr ) == FALSE )
  {
    APSME_LookupExtAddr( srcAddr, si->extAddr );
  }

  entryIndex = APSME_SearchTCLinkKeyEntry(si->extAddr,&entryFound,&TCLKDevEntry);
  
#if ZG_BUILD_JOINING_TYPE
  if(ZG_DEVICE_JOINING_TYPE && !entryFound)
  {
    osal_memset(defaultEntry, 0, Z_EXTADDR_LEN);
    entryIndex = APSME_SearchTCLinkKeyEntry(defaultEntry,&entryFound,&TCLKDevEntry);
  }
#endif
  
  if(entryFound)
  {
    status = ZSuccess;
    
    selectedId = entryIndex - ZCD_NV_TCLK_TABLE_START;

    switch(TCLKDevEntry.keyAttributes)
    {
      case ZG_UNVERIFIED_KEY:
        #if ZG_BUILD_JOINING_TYPE
        if(ZG_DEVICE_JOINING_TYPE)
        {   
          si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
          break;
        }
        #endif
      case ZG_DEFAULT_KEY:
        if(ZG_DEVICE_JOINING_TYPE)
        { 
          //If default was found, then it is joining as FN, then try distributed key as well
          si->distributedKeyTry = TRUE;
        }
        si->keyNvId = ZCD_NV_TCLK_DEFAULT;
      break;
      case ZG_PROVISIONAL_KEY:
        si->keyNvId = ZCD_NV_TCLK_IC_TABLE_START + TCLKDevEntry.SeedShift_IcIndex;
        //Attempt to use default keys (centralized and distributed) if IC fails?
        si->distributedKeyTry = gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;
        si->defaultKeyTry = gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;
      break;
      case ZG_VERIFIED_KEY:
      case ZG_NON_R21_NWK_JOINED:
        //Only verify the frame counter for Verified keys, or keys used with non R21 TC
        tclkRxFrmCntr = &TCLinkKeyFrmCntr[selectedId].rxFrmCntr;

        if ( si->frmCntr >= *tclkRxFrmCntr )
        {
          // update the rx frame counter
          *tclkRxFrmCntr = si->frmCntr + 1;
          status = ZSuccess;
          
          #if ZG_BUILD_JOINING_TYPE
          if(ZG_DEVICE_JOINING_TYPE)
          {   
            si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
            break;
          }
          #endif
          #if ZG_BUILD_COORDINATOR_TYPE
          if(ZG_DEVICE_COORDINATOR_TYPE)
          {
            si->keyNvId = ZCD_NV_TCLK_SEED;
            si->seedShift = TCLKDevEntry.SeedShift_IcIndex;
          }
          #endif
          osal_memcpy(si->dstExtAddr, TCLKDevEntry.extAddr,Z_EXTADDR_LEN);
        }
        else
        {
          status = ZSecOldFrmCount;
        }
      break;
      default:
        //This should not happen
      break;
    }
  }
  else
  {
    if (ZG_DEVICE_COORDINATOR_TYPE) 
    {    
      if(bdb_doTrustCenterRequireKeyExchange() == FALSE)
      {
        //If the TCLK exchange is not mandated, and the key cannot be found for this device, 
        //is because we have erased the entry. Try with default key.
        si->keyNvId = ZCD_NV_TCLK_DEFAULT;
        status = ZSuccess;
      }
    }
  }
  
  
  return status;
}

/******************************************************************************
 * @fn          APSME_TCLinkKeyLoad
 *
 * @brief       Load Trust Center LINK key data.
 *
 * @param       dstAddr - [in] dstAddr
 * @param       si      - [in, out] SSP_Info_t
 *
 * @return      ZStatus_t
 */
ZStatus_t APSME_TCLinkKeyLoad( uint16 dstAddr, SSP_Info_t* si )
{
  APSME_TCLKDevEntry_t TCLKDevEntry;
  AddrMgrEntry_t       addrEntry;
  ZStatus_t            status = ZSecNoKey;
  uint16               entryIndex;
  uint8                extAddrFound;
  uint8                found;

  
  // Look up the ami of the srcAddr if available
  addrEntry.user    = ADDRMGR_USER_DEFAULT;
  addrEntry.nwkAddr = dstAddr;

  extAddrFound = AddrMgrExtAddrValid( si->extAddr );
  
  if(extAddrFound)
  {
    entryIndex = APSME_SearchTCLinkKeyEntry(si->extAddr,&found,&TCLKDevEntry);
    if(entryIndex != 0xFFFF)
    {
      uint16 i = entryIndex - ZCD_NV_TCLK_TABLE_START;
      
      if(found)
      {
        switch(TCLKDevEntry.keyAttributes)
        {
          case ZG_UNVERIFIED_KEY:
            #if ZG_BUILD_JOINING_TYPE
            if(ZG_DEVICE_JOINING_TYPE)
            {   
              si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
              break;
            }
            #endif
          case ZG_DEFAULT_KEY:
          case ZG_NON_R21_NWK_JOINED:
            si->keyNvId = ZCD_NV_TCLK_DEFAULT;
          break;
          case ZG_PROVISIONAL_KEY:
            si->keyNvId = ZCD_NV_TCLK_IC_TABLE_START + TCLKDevEntry.SeedShift_IcIndex;
          break;
          case ZG_VERIFIED_KEY:
            #if ZG_BUILD_JOINING_TYPE
            if(ZG_DEVICE_JOINING_TYPE)
            {   
              si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
              break;
            }
            #endif
            #if ZG_BUILD_COORDINATOR_TYPE
            if(ZG_DEVICE_COORDINATOR_TYPE)
            {
              si->keyNvId = ZCD_NV_TCLK_SEED;
              si->seedShift = TCLKDevEntry.SeedShift_IcIndex;
            }
            #endif
            osal_memcpy(si->dstExtAddr, TCLKDevEntry.extAddr,Z_EXTADDR_LEN);
          break;
          default:
            //This should not happen
          break;
        }
      }
      //Not found, then create an entry for it. Adding the device to the TCLKDev Entries for first time
      else
      {
        //Initialize the entry
        osal_memcpy(TCLKDevEntry.extAddr, si->extAddr, Z_EXTADDR_LEN);
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
        TCLKDevEntry.keyType = ZG_GLOBAL_LINK_KEY;
        //Create the entry with a random shift of the seed. Validate the maximum shift of the seed which is 15
        TCLKDevEntry.SeedShift_IcIndex = osal_rand() & 0x000F;
        TCLKDevEntry.txFrmCntr = 0;
        TCLKDevEntry.rxFrmCntr = 0;
        //save entry in nv
        osal_nv_write(entryIndex,0,sizeof(APSME_TCLKDevEntry_t),&TCLKDevEntry);
        //Initialize framecounter
        osal_memset(&TCLinkKeyFrmCntr[i],0,sizeof(APSME_TCLinkKeyFrmCntr_t));
        // set the keyNvId to use
        si->keyNvId = ZCD_NV_TCLK_DEFAULT;
      }

      // update link key related fields
      si->keyID   = SEC_KEYID_LINK;
      si->frmCntr = TCLinkKeyFrmCntr[i].txFrmCntr;

      // update outgoing frame counter
      TCLinkKeyFrmCntr[i].txFrmCntr++;

  #if defined ( NV_RESTORE )
      // write periodically to NV
      if ( !(TCLinkKeyFrmCntr[i].txFrmCntr % MAX_TCLK_FRAMECOUNTER_CHANGES) )
      {
        // set the flag to write key to NV
        TCLinkKeyFrmCntr[i].pendingFlag = TRUE;

        // Notify the ZDApp that the frame counter has changed.
        osal_set_event( ZDAppTaskID, ZDO_TCLK_FRAMECOUNTER_CHANGE );
      }
  #endif
    }  
    else
    {
      //If no more TCLK entries, try global so we can try to add legacy devices
      si->keyNvId = ZCD_NV_TCLK_DEFAULT;
    }
    status = ZSuccess;
  }
  
  // If no TC link key found, remove the device from the address manager
  if ( (status != ZSuccess) && (AddrMgrEntryLookupNwk(&addrEntry) == TRUE) )
  {
    AddrMgrEntryRelease( &addrEntry );
  }

  return status;
}

/******************************************************************************
 * @fn          APSME_IsDefaultTCLK
 *
 * @brief       Return TRUE or FALSE based on the extended address.  If the
 *              input ext address is all FFs, it means the trust center link
 *              assoiciated with the address is the default trust center link key
 *
 * @param       extAddr - [in] extended address
 *
 * @return      uint8 TRUE/FALSE
 */
uint8 APSME_IsDefaultTCLK( uint8 *extAddr )
{
  return osal_isbufset( extAddr, 0xFF, Z_EXTADDR_LEN );
}

/******************************************************************************
 * @fn          ZDSecMgrNwkKeyInit
 *
 * @brief       Initialize the NV items for
 *                  ZCD_NV_NWKKEY,
 *                  ZCD_NV_NWK_ACTIVE_KEY_INFO and
 *                  ZCD_NV_NWK_ALTERN_KEY_INFO
 *
 * @param       setDefault
 *
 * @return      none
 */
void ZDSecMgrNwkKeyInit(uint8 setDefault)
{
  uint8 status;
  nwkKeyDesc nwkKey;
  // Initialize NV items for NWK key, this structure contains the frame counter
  // and is only used when NV_RESTORE is enabled
  nwkActiveKeyItems keyItems;
  

  uint8 i,nwkFrameCounterReset = FALSE;
  nwkSecMaterialDesc_t nwkSecMaterialDesc;
  
  //NwkSecMaterial entry is empty if set to 0s
  osal_memset(&nwkSecMaterialDesc,0,sizeof(nwkSecMaterialDesc_t));
#ifdef NV_RESTORE  
  // Has been set to reset the nwk security material?
  if ( zgReadStartupOptions() & ZCD_STARTOPT_CLEAR_NWK_FRAME_COUNTER )
  {
    nwkFrameCounterReset = TRUE;
  }
#else
  nwkFrameCounterReset = TRUE;
#endif
  
  //Initialize the nwk security material
  for( i = 0; i < gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES; i++)
  {
    if((osal_nv_item_init(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc) == SUCCESS) && (nwkFrameCounterReset))
    {
      osal_nv_write(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
    }
  }
  //Set the last item to the generic nwk security material by setting extPanId to 0xFFs
  osal_memset(nwkSecMaterialDesc.extendedPanID,0xFF,Z_EXTADDR_LEN);
  
  if((osal_nv_item_init(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i - 1,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc) == SUCCESS) && (nwkFrameCounterReset))
  {
    osal_nv_write(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i - 1,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
  }


  osal_memset( &keyItems, 0, sizeof( nwkActiveKeyItems ) );

  status = osal_nv_item_init( ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), (void *)&keyItems );

#if defined ( NV_RESTORE )
  // reset the values of NV items if NV_RESTORE is not enabled
  if ((status == SUCCESS) && (setDefault == TRUE))
  {
    // clear NV data to default values
    osal_nv_write( ZCD_NV_NWKKEY, 0, sizeof(nwkActiveKeyItems), &keyItems );
  }
#else
  (void)setDefault;   // to eliminate compiler warning

  // reset the values of NV items if NV_RESTORE is not enabled
  if (status == SUCCESS)
  {
    osal_nv_write( ZCD_NV_NWKKEY, 0, sizeof(nwkActiveKeyItems), &keyItems );
  }
#endif // defined (NV_RESTORE)

  // Initialize NV items for NWK Active and Alternate keys. These items are used
  // all the time, independently of NV_RESTORE being set or not
  osal_memset( &nwkKey, 0x00, sizeof(nwkKey) );

  status = osal_nv_item_init( ZCD_NV_NWK_ACTIVE_KEY_INFO, sizeof(nwkKey), &nwkKey);

#if defined ( NV_RESTORE )
  // reset the values of NV items if NV_RESTORE is not enabled
  if ((status == SUCCESS) && (setDefault == TRUE))
  {
    // clear NV data to default values
    osal_nv_write( ZCD_NV_NWK_ACTIVE_KEY_INFO, 0, sizeof(nwkKey), &nwkKey );
  }
#else
  // reset the values of NV items if NV_RESTORE is not enabled
  if (status == SUCCESS)
  {
    osal_nv_write( ZCD_NV_NWK_ACTIVE_KEY_INFO, 0, sizeof(nwkKey), &nwkKey );
  }
#endif // defined (NV_RESTORE)

  status = osal_nv_item_init( ZCD_NV_NWK_ALTERN_KEY_INFO, sizeof(nwkKey), &nwkKey );

#if defined ( NV_RESTORE )
  // reset the values of NV items if NV_RESTORE is not enabled
  if ((status == SUCCESS) && (setDefault == TRUE))
  {
    // clear NV data to default values
    osal_nv_write( ZCD_NV_NWK_ALTERN_KEY_INFO, 0, sizeof(nwkKey), &nwkKey );
  }
#else
  // reset the values of NV items if NV_RESTORE is not enabled
  if (status == SUCCESS)
  {
    osal_nv_write( ZCD_NV_NWK_ALTERN_KEY_INFO, 0, sizeof(nwkKey), &nwkKey );
  }
#endif // defined (NV_RESTORE)

}



/*********************************************************************
 * @fn          ZDSecMgrReadKeyFromNv
 *
 * @brief       Looks for a specific key in NV based on Index value
 *
 * @param   keyNvId - Index of key to look in NV
 *                    valid values are:
 *                    ZCD_NV_NWK_ACTIVE_KEY_INFO
 *                    ZCD_NV_NWK_ALTERN_KEY_INFO
 *                    ZCD_NV_TCLK_TABLE_START + <offset_in_table>
 *                    ZCD_NV_APS_LINK_KEY_DATA_START + <offset_in_table>
 *                    ZCD_NV_PRECFGKEY
 *
 * @param  *keyinfo - Data is read into this buffer.
 *
 * @return  SUCCESS if NV data was copied to the keyinfo parameter .
 *          Otherwise, NV_OPER_FAILED for failure.
 */
ZStatus_t ZDSecMgrReadKeyFromNv(uint16 keyNvId, void *keyinfo)
{
  if ((keyNvId == ZCD_NV_NWK_ACTIVE_KEY_INFO) ||
      (keyNvId == ZCD_NV_NWK_ALTERN_KEY_INFO))
  {
    // get NWK active or alternate key from NV
    return (osal_nv_read(keyNvId,
                         osal_offsetof(nwkKeyDesc, key),
                         SEC_KEY_LEN,
                         keyinfo));
  }
  
  else if(keyNvId == ZCD_NV_TCLK_DEFAULT)
  {
    osal_memcpy(keyinfo,defaultTCLinkKey,SEC_KEY_LEN);
    return ZSuccess;
  }
  
#if (ZG_BUILD_JOINING_TYPE) 
  else if(keyNvId == ZCD_NV_DISTRIBUTED_KEY)
  {
    if(ZG_DEVICE_JOINING_TYPE)
    {
      osal_memcpy(keyinfo,distributedDefaultKey,SEC_KEY_LEN);
      return ZSuccess;
    }
  }
#endif
  
  else if((keyNvId == ZCD_NV_TCLK_JOIN_DEV) || (keyNvId == ZCD_NV_PRECFGKEY)) 
  {
    // Read entry keyNvId of the TC link key table from NV. keyNvId should be
    // ZCD_NV_TCLK_TABLE_START + <offset_in_table>
    return (osal_nv_read(keyNvId,
                         0,
                         SEC_KEY_LEN,
                         keyinfo));
  }
    else if (keyNvId == ZCD_NV_PRECFGKEY)
  {
    // Read entry keyNvId of the Preconfig key from NV.
    return (osal_nv_read(keyNvId,
                         0,
                         SEC_KEY_LEN,
                         keyinfo));
  }

  else if((keyNvId >= ZCD_NV_TCLK_IC_TABLE_START) &&
           (keyNvId < (ZCD_NV_TCLK_IC_TABLE_START + gZDSECMGR_TC_DEVICE_IC_MAX)))
  {
    //Read the key derived from the IC
    return (osal_nv_read(keyNvId,
                         0,
                         SEC_KEY_LEN,
                         keyinfo));
  
  }
  else if ((keyNvId >= ZCD_NV_APS_LINK_KEY_DATA_START) &&
           (keyNvId < (ZCD_NV_APS_LINK_KEY_DATA_START + ZDSECMGR_ENTRY_MAX)))
  {
    // Read entry keyNvId of the APS link key table from NV. keyNvId should be
    // ZCD_NV_APS_LINK_KEY_DATA_START + <offset_in_table>
    return (osal_nv_read(keyNvId,
                         osal_offsetof(APSME_LinkKeyData_t, key),
                         SEC_KEY_LEN,
                         keyinfo));
  }

  return NV_OPER_FAILED;
}

/******************************************************************************
 * @fn          ZDSecMgrApsLinkKeyInit
 *
 * @brief       Initialize the NV table for Application link keys
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
void ZDSecMgrApsLinkKeyInit(uint8 setDefault)
{
  APSME_LinkKeyData_t pApsLinkKey;
  uint8 i;
  uint8 status;

  // Initialize all NV items for APS link key, if not exist already.
  osal_memset( &pApsLinkKey, 0x00, sizeof(APSME_LinkKeyData_t) );

  for( i = 0; i < ZDSECMGR_ENTRY_MAX; i++ )
  {
    status = osal_nv_item_init( (ZCD_NV_APS_LINK_KEY_DATA_START + i),
                               sizeof(APSME_LinkKeyData_t), &pApsLinkKey );

#if defined ( NV_RESTORE )
  // If the set default is requested, the APS Link key needs to be erased, regardless of the NV_RESTORE enabled
      if ((status == SUCCESS) && (setDefault == TRUE ))
      {
        osal_nv_write( (ZCD_NV_APS_LINK_KEY_DATA_START + i), 0,
                      sizeof(APSME_LinkKeyData_t), &pApsLinkKey );

      }
#else
    // reset the values of NV items if NV_RESTORE is not enabled
    if (status == SUCCESS)
    {
      osal_nv_write( (ZCD_NV_APS_LINK_KEY_DATA_START + i), 0,
                    sizeof(APSME_LinkKeyData_t), &pApsLinkKey );

    }
#endif // defined (NV_RESTORE)
  }
}


/******************************************************************************
 * @fn          ZDSecMgrInitNVKeyTables
 *
 * @brief       Initialize the NV table for All keys: NWK, Master, TCLK and APS
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
void ZDSecMgrInitNVKeyTables(uint8 setDefault)
{
  ZDSecMgrNwkKeyInit(setDefault);
  ZDSecMgrApsLinkKeyInit(setDefault); 
  APSME_TCLinkKeyInit(setDefault);
  
#if ZG_BUILD_COORDINATOR_TYPE
  if(ZG_DEVICE_COORDINATOR_TYPE)
  {
    ZDSecMgrGenerateSeed(setDefault);
    
  }
#endif
}


/******************************************************************************
 * @fn          ZDSecMgrSaveApsLinkKey
 *
 * @brief       Save APS Link Key to NV. It will loop through all the keys
 *              to see which one to save.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrSaveApsLinkKey(void)
{
  APSME_LinkKeyData_t *pKeyData = NULL;
  int i;

  pKeyData = (APSME_LinkKeyData_t *)osal_mem_alloc(sizeof(APSME_LinkKeyData_t));

  if (pKeyData != NULL)
  {
    // checks all pending flags to know which one to save
    for (i = 0; i < ZDSECMGR_ENTRY_MAX; i++)
    {
      if (ApsLinkKeyFrmCntr[i].pendingFlag == TRUE)
      {
        // retrieve key from NV
        if (osal_nv_read(ZCD_NV_APS_LINK_KEY_DATA_START + i, 0,
                         sizeof(APSME_LinkKeyData_t), pKeyData) == SUCCESS)
        {
          pKeyData->txFrmCntr = ApsLinkKeyFrmCntr[i].txFrmCntr;
          pKeyData->rxFrmCntr = ApsLinkKeyFrmCntr[i].rxFrmCntr;

          // Write the APS link key back to the NV
          osal_nv_write(ZCD_NV_APS_LINK_KEY_DATA_START + i, 0,
                        sizeof(APSME_LinkKeyData_t), pKeyData);

          // clear the pending write flag
          ApsLinkKeyFrmCntr[i].pendingFlag = FALSE;
        }
      }
    }

    // clear copy of key in RAM
    osal_memset( pKeyData, 0x00, sizeof(APSME_LinkKeyData_t) );

    osal_mem_free(pKeyData);
  }
}

/******************************************************************************
 * @fn          ZDSecMgrSaveTCLinkKey
 *
 * @brief       Save TC Link Key to NV. It will loop through all the keys
 *              to see which one to save.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrSaveTCLinkKey(void)
{
  APSME_TCLKDevEntry_t *pKeyData = NULL;
  uint16 i;

  pKeyData = (APSME_TCLKDevEntry_t *)osal_mem_alloc(sizeof(APSME_TCLKDevEntry_t));

  if (pKeyData != NULL)
  {
    for( i = 0; i < gZDSECMGR_TC_DEVICE_MAX; i++ )
    {
      if (TCLinkKeyFrmCntr[i].pendingFlag == TRUE)
      {
        if (osal_nv_read(ZCD_NV_TCLK_TABLE_START + i, 0,
                         sizeof(APSME_TCLKDevEntry_t), pKeyData) == SUCCESS)
        {
          pKeyData->txFrmCntr = TCLinkKeyFrmCntr[i].txFrmCntr;
          pKeyData->rxFrmCntr = TCLinkKeyFrmCntr[i].rxFrmCntr;

          // Write the TC link key back to the NV
          osal_nv_write(ZCD_NV_TCLK_TABLE_START + i, 0,
                        sizeof(APSME_TCLKDevEntry_t), pKeyData);

          // clear the pending write flag
          TCLinkKeyFrmCntr[i].pendingFlag = FALSE;
        }
      }
    }

    osal_mem_free(pKeyData);
  }
}

/******************************************************************************
 * @fn          ZDSecMgrUpdateTCAddress
 *
 * @brief       Update Trust Center address and save to NV.
 *
 * @param       extAddr - [in] extended address or NULL if no TC protected
 *
 * @return      none
 */
void ZDSecMgrUpdateTCAddress( uint8 *extAddr )
{
  uint8 noTCAddress[Z_EXTADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  APSME_SetRequest( apsTrustCenterAddress, 0, ( extAddr != NULL) ? extAddr : noTCAddress );
  osal_cpyExtAddr( zgApsTrustCenterAddr, ( extAddr != NULL) ? extAddr : noTCAddress );
  
#if defined ( NV_RESTORE )
  osal_nv_write( ZCD_NV_TRUSTCENTER_ADDR, 0, Z_EXTADDR_LEN, zgApsTrustCenterAddr );
#endif
}

#if defined ( ZBA_FALLBACK_NWKKEY )
/******************************************************************************
 * @fn          ZDSecMgrFallbackNwkKey
 *
 * @brief       Use the ZBA fallback network key.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrFallbackNwkKey( void )
{
  if ( !_NIB.nwkKeyLoaded )
  {
    uint8 fallbackKey[SEC_KEY_LEN];

    ZDSecMgrReadKeyFromNv( ZCD_NV_PRECFGKEY, fallbackKey );
    SSP_UpdateNwkKey( fallbackKey, 0);
    SSP_SwitchNwkKey( 0 );

    // clear local copy of key
    osal_memset( fallbackKey, 0x00, SEC_KEY_LEN );

    // handle next step in authentication process
    ZDSecMgrAuthNwkKey();
  }
}
#endif // defined ( ZBA_FALLBACK_NWKKEY )

#if defined ( NV_RESTORE )
/******************************************************************************
 * @fn          ZDSecMgrClearNVKeyValues
 *
 * @brief       If NV_RESTORE is enabled and the status of the network needs
 *              default values this fuction clears ZCD_NV_NWKKEY,
 *              ZCD_NV_NWK_ACTIVE_KEY_INFO and ZCD_NV_NWK_ALTERN_KEY_INFO link
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrClearNVKeyValues(void)
{
  nwkActiveKeyItems keyItems;
  nwkKeyDesc nwkKey;

  osal_memset(&keyItems, 0x00, sizeof(nwkActiveKeyItems));

  osal_nv_write(ZCD_NV_NWKKEY, 0, sizeof(nwkActiveKeyItems), &keyItems);

  // Initialize NV items for NWK Active and Alternate keys.
  osal_memset( &nwkKey, 0x00, sizeof(nwkKeyDesc) );

  osal_nv_write(ZCD_NV_NWK_ACTIVE_KEY_INFO, 0, sizeof(nwkKeyDesc), &nwkKey);

  osal_nv_write(ZCD_NV_NWK_ALTERN_KEY_INFO, 0, sizeof(nwkKeyDesc), &nwkKey);
}
#endif // defined ( NV_RESTORE )

/******************************************************************************
******************************************************************************/
