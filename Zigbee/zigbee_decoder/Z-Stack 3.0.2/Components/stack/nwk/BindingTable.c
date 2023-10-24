/**************************************************************************************************
  Filename:       BindingTable.c
  Revised:        $Date: 2014-10-08 08:37:03 -0700 (Wed, 08 Oct 2014) $
  Revision:       $Revision: 40512 $

  Description:    Device binding table functions.


  Copyright 2004-2014 Texas Instruments Incorporated. All rights reserved.

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
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_Nv.h"
#include "nwk_globals.h"
#include "AddrMgr.h"
#include "BindingTable.h"
#include "nwk_util.h"
#include "bdb.h"
#include "bdb_interface.h"
#if BDB_REPORTING  
#include "bdb_Reporting.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define NV_BIND_EMPTY   0xFF
#define NV_BIND_REC_SIZE (gBIND_REC_SIZE)
#define NV_BIND_ITEM_SIZE  (gBIND_REC_SIZE * gNWK_MAX_BINDING_ENTRIES)

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint8        srcEP;
  uint16       dstIndex;
  uint8        dstEP;
  uint8        dstAddrMode;
  uint8        clusterIDs;
  uint16*      clusterIDList;
} bindFields_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1) 
extern bdbGCB_BindNotification_t        pfnBindNotificationCB;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void BindAddrMgrCB( uint8 update, AddrMgrEntry_t *entryOld,
                    AddrMgrEntry_t *entryNew );
BindingEntry_t *bindFindEmpty( void );
uint16 bindingAddrMgsHelperFind( zAddrType_t *addr );
uint8 bindingAddrMgsHelperConvert( uint16 idx, zAddrType_t *addr );
void bindAddrMgrLocalLoad( void );

#if !defined ( BINDINGTABLE_NV_SINGLES )
  #if !defined ( DONT_UPGRADE_BIND )
    static uint8 BindCopyBackupToNewNV( uint16 dupLen, uint16 newLen );
    static uint8 BindUpgradeTableInNV( void );
  #endif
#endif // !BINDINGTABLE_NV_SINGLES


/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 bindAddrMgrLocalLoaded = FALSE;

/*********************************************************************
 * Function Pointers
 */

BindingEntry_t *(*pbindAddEntry)( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16 *clusterIds ) = (void*)NULL;
uint16 (*pbindNumOfEntries)( void ) = (void*)NULL;
void (*pbindRemoveDev)( zAddrType_t *Addr ) = (void*)NULL;
byte (*pBindInitNV)( void ) = (void*)NULL;
void (*pBindSetDefaultNV)( void ) = (void*)NULL;
uint16 (*pBindRestoreFromNV)( void ) = (void*)NULL;
void (*pBindWriteNV)( void ) = (void*)NULL;

#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
/*********************************************************************
 * @fn      BindAddrMgrCB()
 *
 * @brief   Address Manager Callback function
 *
 * @param   update -
 * @param   entry -
 *
 * @return  pointer to
 */
void BindAddrMgrCB( uint8 update, AddrMgrEntry_t *entryNew,
                    AddrMgrEntry_t *entryOld )
{
  // Check for either deleted items or changed Extended (Duplicate) addresses
}
#endif // ( ADDRMGR_CALLBACK_ENABLED == 1 )

/*********************************************************************
 * @fn      InitBindingTable()
 *
 * @brief
 *
 *   This function is used to initialise the binding table
 *
 * @param   none
 *
 * @return  none
 */
void InitBindingTable( void )
{
  osal_memset( BindingTable, 0xFF, gBIND_REC_SIZE * gNWK_MAX_BINDING_ENTRIES );

  pbindAddEntry = bindAddEntry;
  pbindNumOfEntries = bindNumOfEntries;
  pbindRemoveDev = bindRemoveDev;
  pBindInitNV = BindInitNV;
  pBindSetDefaultNV = BindSetDefaultNV;
  pBindRestoreFromNV = BindRestoreFromNV;
  pBindWriteNV = BindWriteNV;

  bindAddrMgrLocalLoaded = FALSE;

#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
  // Register with the address manager
  AddrMgrRegister( ADDRMGR_REG_BINDING, BindAddrMgrCB );
#endif
}

/*********************************************************************
 * @fn      bindFindEmpty()
 *
 * @brief   This function returns a pointer to an empty binding slot
 *
 * @param   none
 *
 * @return  pointer to binding table entry, NULL if not added
 */
BindingEntry_t *bindFindEmpty( void )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // It's empty if the index is "Not Found"
    if ( BindingTable[x].srcEP == NV_BIND_EMPTY )
    {
      return ( &BindingTable[x] );
    }
  }

  return ( (BindingEntry_t *)NULL );
}

/*********************************************************************
 * @fn      bindNumOfEntries()
 *
 * @brief   This function returns the number of binding table entries.
 *          The return for this is the number of clusters in the
 *          table NOT the number of entries.
 *
 * @param   none
 *
 * @return  number of entries
 */
uint16 bindNumOfEntries( void )
{
  bindTableIndex_t x;
  uint16 found;

  for ( found = 0, x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // It's empty if the index is "Not Found"
    if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
    {
      found += BindingTable[x].numClusterIds;
    }
  }

  return ( found );
}

/*********************************************************************
 * @fn      bindCapacity()
 *
 * @brief   This function returns the number of binding entries
 *          possible and used.
 *
 * @param   maxEntries - pointer to a place to put the max entries
 * @param   usedEntries - pointer to a place to put the number
 *               of used entries
 *
 * @return  none
 */
void bindCapacity( uint16 *maxEntries, uint16 *usedEntries  )
{
  bindTableIndex_t x;
  bindTableIndex_t used;

  for ( used = 0, x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // It's empty if the index is "Not Found"
    if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
    {
      used++;
    }
  }

  *maxEntries = gNWK_MAX_BINDING_ENTRIES;
  *usedEntries = used;
}

/*********************************************************************
 * @fn      bindAddEntry()
 *
 * @brief   This function is used to Add an entry to the binding table
 *
 * @param       srcEpInt - source endpoint
 * @param       dstAddr - destination Address
 * @param       dstEpInt - destination endpoint
 * @param       numClusterIds - number of cluster Ids in the list
 * @param       clusterIds - pointer to the Object ID list
 *
 * @return  pointer to binding table entry, NULL if not added
 */
BindingEntry_t *bindAddEntry( byte srcEpInt,
                              zAddrType_t *dstAddr, byte dstEpInt,
                              byte numClusterIds, uint16 *clusterIds )
{
  uint8            index;
  bindTableIndex_t bindIdx;
  BindingEntry_t*  entry;
  bindFields_t     fields;
  bdbBindNotificationData_t bindData;
#ifdef BDB_REPORTING  
  uint8 bindAdded = FALSE;
#endif

  //Zigbee Spec 2.2.4.3.1.1
  //Cannot create an endpoint for invalid endpoint index, neither for non-Group 
  //or Non-Extended IEEE Address modes
  if( (dstAddr->addrMode != AddrGroup) && (dstAddr->addrMode != Addr64Bit) ||
      (srcEpInt == 0) || (srcEpInt == 0xFF) )
  {
    return NULL;
  }
  //Do not accept neither binds to IEEE Addr with invalid endpoints
  if(( dstAddr->addrMode == Addr64Bit ) && (dstEpInt == 0))
  {
    return NULL;
  }
  
    
  osal_memcpy( &bindData.dstAddr, dstAddr, sizeof( zAddrType_t) );
  bindData.ep = dstEpInt;
    
  // initialize results
  entry = NULL;

  // make sure local addresses have been loaded
  bindAddrMgrLocalLoad();

  // setup fields
  fields.dstIndex = bindAddrIndexGet( dstAddr );
  fields.srcEP    = srcEpInt;

  if ( dstAddr->addrMode == AddrGroup )
  {
    fields.dstAddrMode = DSTGROUPMODE_GROUP;
    fields.dstEP       = 0;
  }
  else
  {
    fields.dstAddrMode = DSTGROUPMODE_ADDR;
    fields.dstEP       = dstEpInt;
  }

  if ( fields.dstIndex != INVALID_NODE_ADDR  )
  {
    for ( bindIdx = 0; bindIdx < gNWK_MAX_BINDING_ENTRIES; bindIdx++ )
    {
      if ( ( fields.srcEP       == BindingTable[bindIdx].srcEP        ) &&
           ( fields.dstAddrMode == BindingTable[bindIdx].dstGroupMode ) &&
           ( fields.dstIndex    == BindingTable[bindIdx].dstIdx       ) &&
           ( fields.dstEP       == BindingTable[bindIdx].dstEP        )    )
      {
        entry = &BindingTable[bindIdx];

        // break from loop
        break;
      }
    }

    if ( entry != NULL )
    {
      // Loop through the cluster IDs
      for ( index = 0; index < numClusterIds; index++ )
      {
        // Found - is the cluster already defined?
        if ( bindIsClusterIDinList( entry, clusterIds[index] ) == FALSE )
        {
          // Nope, add this cluster
          if ( bindAddClusterIdToList( entry, clusterIds[index] ) == FALSE )
          {
            // Indicate error if cluster list was full
            entry = NULL;
          }
          else
          {
            // new bind added - notify application
            bindData.clusterId = clusterIds[index];
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)             
            if ( pfnBindNotificationCB != NULL )
            {
              pfnBindNotificationCB( &bindData );
            }
#endif
#ifdef BDB_REPORTING  
            bdb_RepMarkHasBindingInEndpointClusterArray(srcEpInt, clusterIds[index], BDBREPORTING_FALSE, BDBREPORTING_TRUE); 
            bindAdded = TRUE;
#endif
          }
        }
      }
    }
    else
    {
      // Find an empty slot
      entry = bindFindEmpty();

      // Check against the maximum number allowed
      if ( entry != NULL )
      {
        // Add new entry
        entry->srcEP         = fields.srcEP;
        entry->dstGroupMode  = fields.dstAddrMode;
        entry->dstIdx        = fields.dstIndex;
        entry->dstEP         = fields.dstEP;

        if ( numClusterIds > gMAX_BINDING_CLUSTER_IDS )
        {
          numClusterIds = gMAX_BINDING_CLUSTER_IDS;
        }
        
        for(index = 0; index < numClusterIds; index++)
        {
          // new bind added - notify application
          bindData.clusterId = clusterIds[index];
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1) 
          if ( pfnBindNotificationCB != NULL )
          {
            pfnBindNotificationCB( &bindData );
          }
#endif
#ifdef BDB_REPORTING   
            bdb_RepMarkHasBindingInEndpointClusterArray(srcEpInt, clusterIds[index], BDBREPORTING_FALSE, BDBREPORTING_TRUE); 
            bindAdded = TRUE;
#endif
        }

        entry->numClusterIds = numClusterIds;

        osal_memcpy( entry->clusterIdList,
                     clusterIds,
                     numClusterIds * sizeof(uint16) );
      }
    }
  }
#ifdef BDB_REPORTING
  if(bindAdded == TRUE)
  {
    bdb_RepStartOrContinueReporting( );
  }
#endif
  return entry;
}

/*********************************************************************
 * @fn      bindRemoveEntry
 *
 * @brief   Removes a binding table entry.
 *
 * @param   pBind - pointer to binding table entry to delete
 *
 * @return  TRUE if Removed, FALSE if not
 */
byte bindRemoveEntry( BindingEntry_t *pBind )
{
  osal_memset( pBind, 0xFF, gBIND_REC_SIZE );
#ifdef BDB_REPORTING
  bdb_RepUpdateMarkBindings();
#endif
  return ( TRUE );
}

/*********************************************************************
 * @fn      bindIsClusterIDinList()
 *
 * @brief   Is the clusterID in the clusterID list?
 *
 * @param   enter - binding table entry
 * @param   clusterId  - Cluster ID to look for
 *
 * @return  TRUE if found, FALSE if not found
 */
byte bindIsClusterIDinList( BindingEntry_t *entry, uint16 clusterId )
{
  uint8 x;

  if ( entry != NULL )
  {
    for ( x = 0; x < entry->numClusterIds; x++ )
    {
      if ( entry->clusterIdList[x] == clusterId )
      {
        return ( TRUE );
      }
    }
  }

  return ( FALSE );
}

/*********************************************************************
 * @fn      bindRemoveClusterIdFromList()
 *
 * @brief   Removes a ClusterID from a list of ClusterIDs.
 *
 * @param   enter - binding table entry
 * @param   clusterId  - Cluster ID to look for
 *
 * @return  TRUE if there are at least 1 clusterID left, FALSE if none
 */
byte bindRemoveClusterIdFromList( BindingEntry_t *entry, uint16 clusterId )
{
  byte x;
  uint16 *listPtr;
  byte numIds;

#ifdef BDB_REPORTING
  uint8 numRemoved = 0;
#endif
  if ( entry )
  {
    if ( entry->numClusterIds > 0 )
    {
      listPtr = entry->clusterIdList;
      numIds = entry->numClusterIds;

      // Copy the new list over
      for ( x = 0; x < numIds; x++ )
      {
        if ( entry->clusterIdList[x] != clusterId )
        {
          *listPtr++ = entry->clusterIdList[x];
        }
        else
        {
          entry->numClusterIds--;
          
#ifdef BDB_REPORTING
           numRemoved++;
#endif          
          if ( entry->numClusterIds == 0 )
          {
            break;
          }
        }
      }
      
    }
  }

#ifdef BDB_REPORTING
  if(numRemoved>0)
    bdb_RepUpdateMarkBindings();
#endif 
  
  if ( entry && (entry->numClusterIds > 0) )
  {
    return ( TRUE );
  }
  else
  {
    return ( FALSE );
  }
}

/*********************************************************************
 * @fn      bindAddClusterIdToList()
 *
 * @brief   Adds a ClusterID to a list of ClusterIDs.
 *
 * @param   enter - binding table entry
 * @param   clusterId  - Cluster ID to Add
 *
 * @return  TRUE if Added, FALSE if not
 */
byte bindAddClusterIdToList( BindingEntry_t *entry, uint16 clusterId )
{
  if ( entry && entry->numClusterIds < gMAX_BINDING_CLUSTER_IDS )
  {
    // Add the new one
    entry->clusterIdList[entry->numClusterIds] = clusterId;
    entry->numClusterIds++;
    return ( TRUE );
  }
  return ( FALSE );
}

/*********************************************************************
 * @fn      bindFindExisting
 *
 * @brief   Finds an existing src/epint to dst/epint bind record
 *
 * @param   srcEpInt - Source Endpoint/Interface
 * @param   dstAddr - Destination address
 * @param   dstEpInt - Destination Endpoint/Interface
 *
 * @return  pointer to existing entry or NULL
 */
BindingEntry_t *bindFindExisting( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt )
{
  uint16 dstIdx;
  bindTableIndex_t x;

  // Find the records in the assoc list
  if ( dstAddr->addrMode == AddrGroup )
  {
    dstIdx = dstAddr->addr.shortAddr;
  }
  else
  {
    dstIdx = bindingAddrMgsHelperFind( dstAddr );
  }

  if ( dstIdx == INVALID_NODE_ADDR )
  {
    return ( (BindingEntry_t *)NULL );
  }

  // Start at the beginning
  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( (BindingTable[x].srcEP == srcEpInt) )
    {
      if ( ((dstAddr->addrMode == AddrGroup)
              && (BindingTable[x].dstGroupMode == DSTGROUPMODE_GROUP)
              && (dstIdx == BindingTable[x].dstIdx))
         || ((dstAddr->addrMode != AddrGroup)
             && (BindingTable[x].dstGroupMode == DSTGROUPMODE_ADDR)
             && (dstIdx == BindingTable[x].dstIdx) && (BindingTable[x].dstEP == dstEpInt)) )
      {
        return ( &BindingTable[x] );
      }
    }
  }

  return ( (BindingEntry_t *)NULL );
}

/*********************************************************************
 * @fn       bindRemoveDev()
 *
 * @brief
 *
 *   Remove all bind(s) entries associated to a device address (destination).
 *   Updates binding table.
 *
 * @param   Addr - address of device to be removed from Binding Table
 *
 * @return  none
 */
void bindRemoveDev( zAddrType_t *Addr )
{
  uint16 idx;
  bindTableIndex_t x;

  if ( Addr->addrMode == AddrGroup )
  {
    idx = Addr->addr.shortAddr;
  }
  else
  {
    idx = bindingAddrMgsHelperFind( Addr );
  }

  if ( idx == INVALID_NODE_ADDR )
  {
    return;
  }

  // Removes all the entries that match the destination Address/Index
  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( ( (Addr->addrMode == AddrGroup) && (BindingTable[x].dstGroupMode == DSTGROUPMODE_GROUP)
                                         && (BindingTable[x].dstIdx == idx) ) ||
         ( (Addr->addrMode != AddrGroup) && (BindingTable[x].dstGroupMode == DSTGROUPMODE_ADDR)
                                         && (BindingTable[x].dstIdx == idx) ) )
    {
      bindRemoveEntry( &BindingTable[x] );
    }
  }

  // If this is the last Bind Entry for that idx then clear BINDING
  // user from Address Manager
  bindAddressClear( idx );
}

/*********************************************************************
 * @fn       bindRemoveSrcDev()
 *
 * @brief
 *
 *   Remove binds(s) associated to device address (source).
 *   Updates binding table.
 *
 * @param   ep - endpoint to remove, 0xFF is all endpoints
 *
 * @return  none
 */
void bindRemoveSrcDev( uint8 ep )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( (ep == 0xFF) || (ep == BindingTable[x].srcEP) )
    {
      uint16 idx = BindingTable[x].dstIdx;

      bindRemoveEntry( &BindingTable[x] );

      // If this is the last Bind Entry for that idx then clear BINDING
      // user from Address Manager
      bindAddressClear( idx );
    }
  }
}

/*********************************************************************
 * @fn          bindNumBoundTo
 *
 * @brief       Calculate the number items this device is bound to.
 *              When srcMode is set to TRUE, discard what value devAddr
 *              has, it returns number count bound to the local dev.
 *
 * @param       devAddr - device Address
 * @param       devEP - endpoint
 * @param       srcMode - TRUE - assume devHandle is a source address
 *                        FALSE - destination address
 *
 * @return      status
 */
byte bindNumBoundTo( zAddrType_t *devAddr, byte devEpInt, byte srcMode )
{
  BindingEntry_t *pBind;
  uint16 idx;
  byte   num;
  bindTableIndex_t x;

  // Init
  num = 0;

  if ( devAddr->addrMode == AddrGroup )
  {
    idx = devAddr->addr.shortAddr;
  }
  else
  {
    idx = bindingAddrMgsHelperFind( devAddr );
  }

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];
    if ( srcMode )
    {
      if ( pBind->srcEP == devEpInt )
      {
        num++;
      }
    }
    else
    {
      if ( ((devAddr->addrMode == AddrGroup)
              && (pBind->dstGroupMode == DSTGROUPMODE_GROUP) && (pBind->dstIdx == idx))
          || ((devAddr->addrMode != AddrGroup) && (pBind->dstGroupMode == DSTGROUPMODE_ADDR)
                                && (pBind->dstIdx == idx) && (pBind->dstEP == devEpInt)) )
      {
        num++;
      }
    }
  }

  return num;
}

/*********************************************************************
 * @fn          bindNumReflections
 *
 * @brief       Counts the number of reflections needed for a
 *              endpoint and cluster ID combo.
 *
 * @param       ep - source endpoint
 * @param       clusterID - matching clusterID
 *
 * @return      number of reflections needed.
 */
uint16 bindNumReflections( uint8 ep, uint16 clusterID )
{
  bindTableIndex_t x;
  BindingEntry_t *pBind;
  uint16 cnt = 0;
  uint8 bindEP;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];
    bindEP = pBind->srcEP;

    if ( (bindEP == ep) && (bindIsClusterIDinList( pBind, clusterID )) )
    {
      cnt++;
    }
  }

  return ( cnt );
}

/*********************************************************************
 * @fn          bindFind
 *
 * @brief       Finds the binding entry for the source address, endpoint
 *              and cluster ID passed in as a parameter.
 *
 * @param       ep - source endpoint
 * @param       clusterID - matching clusterID
 * @param       skip - number of matches to skip before returning
 *
 * @return      pointer to the binding table entry, NULL if not found
 */
BindingEntry_t *bindFind( uint8 ep, uint16 clusterID, uint8 skipping )
{
  BindingEntry_t *pBind;
  byte skipped = 0;
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];

    if ( ( pBind->srcEP == ep) && bindIsClusterIDinList( pBind, clusterID ))
    {
      if ( skipped < skipping )
      {
        skipped++;
      }
      else
      {
        return ( pBind );
      }
    }
  }

  return ( (BindingEntry_t *)NULL );
}

/*********************************************************************
 * @fn      bindAddressClear
 *
 * @brief   Lookup a binding entry by specific Idx, if none is found
 *          clears the BINDING user from Address Manager.
 *
 * @param   dstIdx - Source Address Manager Index
 *
 * @return  none
 */
void bindAddressClear( uint16 dstIdx )
{
  bindTableIndex_t i;

  if ( dstIdx != INVALID_NODE_ADDR )
  {
    // Looks for a specific Idx
    for ( i = 0; i < gNWK_MAX_BINDING_ENTRIES; i++ )
    {
      if ( ( BindingTable[i].dstGroupMode != AddrGroup ) &&
           ( BindingTable[i].dstGroupMode == DSTGROUPMODE_ADDR ) &&
           ( BindingTable[i].dstIdx == dstIdx ) )
      {
        break;  // found at least one
      }
    }

    if ( i == gNWK_MAX_BINDING_ENTRIES )
    {
      // No binding entry is associated with dstIdx.
      // Remove user binding bit from the address manager entry corresponding to dstIdx.
      AddrMgrEntry_t addrEntry;

      addrEntry.user = ADDRMGR_USER_BINDING;
      addrEntry.index = dstIdx;

      AddrMgrEntryRelease( &addrEntry );
    }
  }
}

/*********************************************************************
 * @fn          bindUpdateAddr
 *
 * @brief       Update the network address in the binding table.
 *
 * @param       oldAddr - old network address
 * @param       newAddr - new network address
 *
 * @return      none
 */
void bindUpdateAddr( uint16 oldAddr, uint16 newAddr )
{
  uint16 oldIdx;
  uint16 newIdx;
  zAddrType_t addr;
  bindTableIndex_t x;
  BindingEntry_t *pBind;

  addr.addrMode = Addr16Bit;
  addr.addr.shortAddr = oldAddr;
  oldIdx = bindingAddrMgsHelperFind( &addr );
  addr.addr.shortAddr = newAddr;
  newIdx = bindingAddrMgsHelperFind( &addr );

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];

    if ( pBind->dstIdx == oldIdx )
    {
      pBind->dstIdx = newIdx;
    }
  }
}

/*********************************************************************
 * @fn      bindingAddrMgsHelperFind
 *
 * @brief   Turns an zAddrType_t to an Addr Manager index
 *
 * @param   addr - zAddrType_t
 *
 * @return  INVALID_NODE_ADDR if not found, otherwise an index
 */
uint16 bindingAddrMgsHelperFind( zAddrType_t *addr )
{
  AddrMgrEntry_t entry;

  // Resolve addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;
  if ( addr->addrMode == Addr16Bit )
  {
    entry.nwkAddr = addr->addr.shortAddr;
    AddrMgrEntryLookupNwk( &entry );
  }
  else
  {
    AddrMgrExtAddrSet( entry.extAddr, addr->addr.extAddr );
    AddrMgrEntryLookupExt( &entry );
  }

  return ( entry.index );
}

/*********************************************************************
 * @fn      bindingAddrMgsHelperConvert
 *
 * @brief   Convert an index into an zAddrType_t
 *
 * @param   idx -
 * @param   addr - zAddrType_t
 *
 * @return  TRUE if found, FALSE if not
 */
uint8 bindingAddrMgsHelperConvert( uint16 idx, zAddrType_t *addr )
{
  AddrMgrEntry_t entry;
  uint8 stat;

  // Resolve addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;
  entry.index = idx;
  stat = AddrMgrEntryGet( &entry );
  if ( stat )
  {
    addr->addrMode = Addr64Bit;
    osal_cpyExtAddr( addr->addr.extAddr, entry.extAddr );
  }

  return ( stat );
}

/*********************************************************************
 * @fn      bindingAddrMgsHelperConvertShort
 *
 * @brief   Convert an index into a short address
 *
 * @param   idx -
 *
 * @return  INVALID_NODE_ADDR if not available, otherwise the short address
 */
uint16 bindingAddrMgsHelperConvertShort( uint16 idx )
{
  AddrMgrEntry_t entry;

  // Resolve addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;
  entry.index = idx;
  AddrMgrEntryGet( &entry );

  return ( entry.nwkAddr );
}

/*********************************************************************
 * @fn      bindAddrMgrLocalLoad
 *
 * @brief   Load local(self and parent) address information into
 *          Address Manager
 *
 * @param   none
 *
 * @return  none
 */
void bindAddrMgrLocalLoad( void )
{
  AddrMgrEntry_t entry;
  uint16         parent;

  // add "local"(self and parent) address informtion into the Address
  // Manager
  if ( bindAddrMgrLocalLoaded == FALSE )
  {
    // add the device's address information
    entry.user    = ADDRMGR_USER_BINDING;
    entry.nwkAddr = _NIB.nwkDevAddress;
    AddrMgrExtAddrSet( entry.extAddr, NLME_GetExtAddr() );
    AddrMgrEntryUpdate( &entry );

    // make sure parent address is valid
    parent = NLME_GetCoordShortAddr();
    if ( ( parent != entry.nwkAddr     ) &&
         ( parent != INVALID_NODE_ADDR )    )
    {
      // add the parent's address information
      entry.nwkAddr = parent;
      NLME_GetCoordExtAddr( entry.extAddr );
      AddrMgrEntryUpdate( &entry );
    }

    bindAddrMgrLocalLoaded = TRUE;
  }
}

/*********************************************************************
 * @fn      bindAddrIndexGet
 *
 * @brief   Get bind address index.
 *
 * @param   addr - <zAddrType_t>
 *
 * @return  (uint16) address index
 */
uint16 bindAddrIndexGet( zAddrType_t* addr )
{
  AddrMgrEntry_t entry;
  uint8          update;

  update = FALSE;

  // sync binding addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;

  if ( addr->addrMode == Addr16Bit )
  {
    entry.nwkAddr = addr->addr.shortAddr;

    if ( AddrMgrEntryLookupNwk( &entry ) == FALSE )
    {
      update = TRUE;
    }
  }
  else if ( addr->addrMode == Addr64Bit )
  {
    AddrMgrExtAddrSet( entry.extAddr, addr->addr.extAddr );

    if ( AddrMgrEntryLookupExt( &entry ) == FALSE )
    {
      update = TRUE;
    }
  }
  else if ( addr->addrMode == AddrGroup )
  {
    entry.index = addr->addr.shortAddr;
  }
  else
  {
    entry.index = INVALID_NODE_ADDR;
  }

  if ( update )
  {
    AddrMgrEntryUpdate( &entry );
  }

  return entry.index;
}

/*********************************************************************
 * @fn      GetBindingTableEntry
 *
 * @brief   Get a pointer to the Nth valid binding table entry.
 *
 * @param   Nth valid entry being requested.
 *
 * @return  The Nth valid binding table entry.
 */
BindingEntry_t *GetBindingTableEntry( uint16 Nth )
{
  BindingEntry_t *rtrn = NULL;

#if defined ( REFLECTOR )
  bindTableIndex_t idx, cnt = 0;

  for ( idx = 0; idx < gNWK_MAX_BINDING_ENTRIES; idx++ )
  {
    if ( BindingTable[idx].srcEP != NV_BIND_EMPTY )
    {
      if ( cnt++ == Nth )
      {
        rtrn = BindingTable+idx;
        break;
      }
    }
  }
#else
  (void)Nth;
#endif

  return rtrn;
}

#if !defined ( BINDINGTABLE_NV_SINGLES )
/*********************************************************************
 * @fn          BindInitNV
 *
 * @brief       Initialize the Binding NV Item
 *
 * @param       none
 *
 * @return      ZSUCCESS if successful, NV_ITEM_UNINIT if item did not
 *              exist in NV, NV_OPER_FAILED if failure.
 */
byte BindInitNV( void )
{
  byte ret;

  // Initialize the device list
  ret = osal_nv_item_init( ZCD_NV_BINDING_TABLE,
                  (uint16)( sizeof( nvBindingHdr_t ) + NV_BIND_ITEM_SIZE ), NULL );

  if (ret == NV_ITEM_UNINIT) 
  {
    BindSetDefaultNV();
  }

  return ( ret );
}

/*********************************************************************
 * @fn          BindSetDefaultNV
 *
 * @brief       Write the defaults to NV
 *
 * @param       none
 *
 * @return      none
 */
void BindSetDefaultNV( void )
{
  nvBindingHdr_t hdr;

  // Initialize the header
  hdr.numRecs = 0;

  // Save off the header
  osal_nv_write( ZCD_NV_BINDING_TABLE, 0, sizeof( nvBindingHdr_t ), &hdr );
}

#if !defined ( DONT_UPGRADE_BIND )
/*********************************************************************
 * @fn          BindCopyBackupToNewNV
 *
 * @brief       Creates the New NV item, copies the backup data into
 *              the New NV ID, and Deletes the duplicate NV item.
 *
 * @param       dupLen - NV item length of the old Binding table.
 * @param       newLen - NV item length of the new Binding table to be created.
 *
 * @return      ZSuccess - All the actions were successful.
 *              ZFailure - Any of the actions failed.
 */
static uint8 BindCopyBackupToNewNV( uint16 dupLen, uint16 newLen )
{
  uint8 status = ZSuccess;
  uint16 bindLen;

  bindLen = osal_nv_item_len( ZCD_NV_BINDING_TABLE );


  if ( ( bindLen > 0 ) && ( bindLen != newLen ) )
  {
    // The existing item does not match the New length
    osal_nv_delete( ZCD_NV_BINDING_TABLE, bindLen );
  }

  // Create Binding Table NV item with the NEW legth
  if ( osal_nv_item_init( ZCD_NV_BINDING_TABLE, newLen, NULL ) != NV_OPER_FAILED )
  {
    nvBindingHdr_t hdrBackup;

    // Copy ONLY the valid records from the duplicate NV table into the new table
    // at the end of this process the table content will be compacted
    if ( osal_nv_read( ZCD_NV_DUPLICATE_BINDING_TABLE, 0, sizeof(nvBindingHdr_t), &hdrBackup ) == ZSuccess )
    {
      bindTableIndex_t i;
      uint16 validBackupRecs = 0;
      BindingEntry_t backupRec;

      // Read in the device list. This loop will stop when:
      // The total number of valid records has been reached either because:
      //          The new table is full of valid records OR
      //          The old table has less valid records than the size of the table
      for ( i = 0; ( validBackupRecs < gNWK_MAX_BINDING_ENTRIES ) && ( validBackupRecs < hdrBackup.numRecs ); i++ )
      {
        if ( osal_nv_read( ZCD_NV_DUPLICATE_BINDING_TABLE,
                          (uint16)(sizeof(nvBindingHdr_t) + (i * NV_BIND_REC_SIZE)),
                          NV_BIND_REC_SIZE, &backupRec ) == ZSuccess )
        {
          if ( backupRec.srcEP != NV_BIND_EMPTY )
          {
            // Save the valid record into the NEW NV table.
            if ( osal_nv_write( ZCD_NV_BINDING_TABLE,
                                (uint16)((sizeof(nvBindingHdr_t)) + (validBackupRecs * NV_BIND_REC_SIZE)),
                                NV_BIND_REC_SIZE, &backupRec ) != ZSuccess )
            {
               status = ZFailure;
               break; // Terminate the loop as soon as a problem with NV is detected
            }

            validBackupRecs++;
          }
        }
        else
        {
           status = ZFailure;
           break; // Terminate the loop as soon as a problem with NV is detected
        }
      }

      // Only save the header and delete the duplicate element if the previous
      // process was successful
      if ( status == ZSuccess )
      {
        // Save off the header
        if ( osal_nv_write( ZCD_NV_BINDING_TABLE, 0,
                            sizeof(nvBindingHdr_t), &validBackupRecs ) == ZSuccess )
        {
          // Delete the duplicate NV Item, once the data has been stored in the NEW table
          if ( osal_nv_delete( ZCD_NV_DUPLICATE_BINDING_TABLE, dupLen ) != ZSuccess )
          {
            status = ZFailure;
          }
        }
        else
        {
          status = ZFailure;
        }
      }
    }
    else
    {
      status = ZFailure;
    }
  }
  else
  {
    status = ZFailure;
  }

  return ( status );
}
#endif // !DONT_UPGRADE_BIND

#if !defined ( DONT_UPGRADE_BIND )
/*********************************************************************
 * @fn          BindUpgradeTableInNV
 *
 * @brief       Verifies if the existing table in NV has different size
 *              than the table defined by parameters in the current code.
 *              If different, creates a backup table, deletes the existing
 *              table and creates the new table with the new size. After
 *              this process is done ZCD_NV_BINDING_TABLE NV item contains
 *              only valid records retrieved from the original table, up to
 *              the maximum number of records defined by gNWK_MAX_BINDING_ENTRIES
 *
 * @param       none
 *
 * @return      ZSuccess - the Update process was sucessful.
 *              ZFailure - otherwise.
 */
static uint8 BindUpgradeTableInNV( void )
{
  uint8 status = ZSuccess;
  nvBindingHdr_t hdr;
  uint16 dupLen;
  uint16 bindLen;
  uint16 newLen;
  bool duplicateReady = FALSE;

  // Size of the Binding table based on current paramenters in the code
  newLen = sizeof(nvBindingHdr_t) + NV_BIND_ITEM_SIZE;

  // Size of the Binding table NV item, this is the whole size of the item,
  // it could inculde invalid records also
  bindLen = osal_nv_item_len( ZCD_NV_BINDING_TABLE );

  // Get the number of valid records from the Binding table
  osal_nv_read( ZCD_NV_BINDING_TABLE, 0, sizeof(nvBindingHdr_t), &hdr );

  // Identify if there is a duplicate NV item, if it is there, that means an
  // Upgrade process did not finish properly last time
  // The length function will return 0 if the Backup NV ID does not exist.
  dupLen = osal_nv_item_len( ZCD_NV_DUPLICATE_BINDING_TABLE );

  // A duplicate of the original Binding item will be done if:
  // 1) A duplicate NV item DOES NOT exist AND the size of the original Binding
  //    item in NV is different (larger/smaller) than the the length calculated
  //    from the parameters in the code. If they are the same there is no need
  //    to do the Upgrade process.
  // 2) A duplicate NV item exists (probably because the previous upgrade
  //    process was interrupted) and [the original Binding NV items exists AND
  //    has valid recods (it is important to make sure that valid records exist
  //    in the binding table because it is possible that the item was created
  //    but the data was not copied in the previous upgrade process).
  if ( ( ( dupLen == 0 ) && ( bindLen != newLen ) ) ||
       ( ( dupLen > 0 ) && ( bindLen > 0 ) && ( hdr.numRecs > 0 ) ) )
  {
    // Create a copy from original NV item into a duplicate NV item
    if ( ( status = nwkCreateDuplicateNV( ZCD_NV_BINDING_TABLE,
                                          ZCD_NV_DUPLICATE_BINDING_TABLE ) ) == ZSuccess )
    {
      // Delete the original NV item once the duplicate is ready
      if ( osal_nv_delete( ZCD_NV_BINDING_TABLE, bindLen ) != ZSuccess )
      {
        status = ZFailure;
      }
      else
      {
        duplicateReady = TRUE;
      }
    }
  }
  else if ( ( ( dupLen > 0 ) && ( bindLen == 0 ) ) ||
            ( ( dupLen > 0 ) && ( bindLen > 0 ) && ( hdr.numRecs == 0 ) ) )
  {
    // If for some reason a duplicate NV item was left in the system from a
    // previous upgrade process and:
    // 1) The original Binding NV item DOES NOT exist OR
    // 2) The original Binding NV item exist, but has no valid records.
    // it is necessary to rely in the data in the Duplicate item to create
    // the Binding table
    bindLen = dupLen;

    duplicateReady = TRUE;
  }

  if ( duplicateReady == TRUE )
  {
    // Creates the New Binding table, Copy data from backup and Delete backup NV ID
    status = BindCopyBackupToNewNV( bindLen, newLen );
  }
  return ( status );
}
#endif // !DONT_UPGRADE_BIND

/*********************************************************************
 * @fn          BindRestoreFromNV
 *
 * @brief       Restore the binding table from NV
 *
 * @param       none
 *
 * @return      Number of entries restored
 */
uint16 BindRestoreFromNV( void )
{
  nvBindingHdr_t hdr;

  hdr.numRecs = 0;

#if !defined ( DONT_UPGRADE_BIND )
  if ( BindUpgradeTableInNV() == ZSuccess )
#endif
  {
    if ( osal_nv_read( ZCD_NV_BINDING_TABLE, 0, sizeof(nvBindingHdr_t), &hdr ) == ZSuccess )
    {
      bindTableIndex_t x;
      uint16 validRecsCount = 0;

      // Read in the device list
      for ( x = 0; ( x < gNWK_MAX_BINDING_ENTRIES ) && ( validRecsCount < hdr.numRecs ); x++ )
      {
        if ( osal_nv_read( ZCD_NV_BINDING_TABLE,
                           (uint16)(sizeof(nvBindingHdr_t) + (x * NV_BIND_REC_SIZE)),
                           NV_BIND_REC_SIZE, &BindingTable[x] ) == ZSUCCESS )
        {
          if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
          {
            validRecsCount++;
          }
        }
      }
    }
  }
  return ( hdr.numRecs );
}

/*********************************************************************
 * @fn          BindWriteNV
 *
 * @brief       Save the Binding Table in NV
 *
 * @param       none
 *
 * @return      none
 */
void BindWriteNV( void )
{
  BindingEntry_t *pBind;
  BindingEntry_t bind;
  nvBindingHdr_t hdr;
  bindTableIndex_t x;

  hdr.numRecs = 0;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];

    osal_memcpy( &bind, pBind, gBIND_REC_SIZE );

    // Save the record to NV
    osal_nv_write( ZCD_NV_BINDING_TABLE,
                   (uint16)((sizeof(nvBindingHdr_t)) + (x * NV_BIND_REC_SIZE)),
                   NV_BIND_REC_SIZE, &bind );

    if ( pBind->srcEP != NV_BIND_EMPTY )
    {
      hdr.numRecs++;
    }
  }

  // Save off the header
  osal_nv_write( ZCD_NV_BINDING_TABLE, 0, sizeof(nvBindingHdr_t), &hdr );
}

#else // !BINDINGTABLE_NV_SINGLES
/*********************************************************************
 * @fn          BindInitNV
 *
 * @brief       Initialize the Binding NV Item
 *
 * @param       none
 *
 * @return      ZSUCCESS if successful, NV_ITEM_UNINIT if item did not
 *              exist in NV, NV_OPER_FAILED if failure.
 */
byte BindInitNV( void )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // Initialize each binding record
    osal_nv_item_init_ex( ZCD_NV_EX_BINDING_TABLE, x, NV_BIND_REC_SIZE, NULL );
  }
  return ( ZSUCCESS );
}

/*********************************************************************
 * @fn          BindSetDefaultNV
 *
 * @brief       Write the defaults to NV
 *
 * @param       none
 *
 * @return      none
 */
void BindSetDefaultNV( void )
{
  BindingEntry_t bind;
  bindTableIndex_t x;

  // Initialize a binding record
  osal_memset( &bind, 0xFF, sizeof ( BindingEntry_t ) );

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // Over write each binding record with an "empty" record
    osal_nv_write_ex( ZCD_NV_EX_BINDING_TABLE, x, 0, NV_BIND_REC_SIZE, &bind );
  }
}

/*********************************************************************
 * @fn          BindRestoreFromNV
 *
 * @brief       Restore the binding table from NV
 *
 * @param       none
 *
 * @return      Number of entries restored (non-emtpy)
 */
uint16 BindRestoreFromNV( void )
{
  bindTableIndex_t x;
  uint16 validRecsCount = 0;

  // Read in the device list
  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( osal_nv_read_ex( ZCD_NV_EX_BINDING_TABLE, x, 0,
                     (uint16)NV_BIND_REC_SIZE, &BindingTable[x] ) == ZSUCCESS )
    {
      // Check for non-empty record
      if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
      {
        // Count non-empty records
        validRecsCount++;
      }
    }
  }
  return ( validRecsCount );
}

/*********************************************************************
 * @fn          BindWriteNV
 *
 * @brief       Copy the Binding Table in NV
 *
 * @param       none
 *
 * @return      none
 */
void BindWriteNV( void )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // Save the record to NV
    osal_nv_write_ex( ZCD_NV_EX_BINDING_TABLE, x, 0,
                     (uint16)NV_BIND_REC_SIZE, &BindingTable[x] );
  }
}
#endif // BINDINGTABLE_NV_SINGLES

/*********************************************************************
*********************************************************************/
