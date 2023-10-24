/**************************************************************************************************
  Filename:       BindingTable.h
  Revised:        $Date: 2014-07-16 11:03:22 -0700 (Wed, 16 Jul 2014) $
  Revision:       $Revision: 39430 $

  Description:    Device binding table.


  Copyright 2004-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef BINDINGTABLE_H
#define BINDINGTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "OSAL.h"
#include "nwk.h"
#include "AssocList.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define MAX_DEVICE_PAIRS 255  // temp value

#define DSTGROUPMODE_ADDR     0
#define DSTGROUPMODE_GROUP    1

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint16 numRecs;
} nvBindingHdr_t;

// Don't use sizeof( BindingEntry_t ) use gBIND_REC_SIZE when calculating
// the size of each binding table entry. gBIND_REC_SIZE is defined in nwk_global.c.
typedef struct
{
                        // No src address since the src is always the local device
  uint8 srcEP;
  uint8 dstGroupMode;   // Destination address type; 0 - Normal address index, 1 - Group address
  uint16 dstIdx;        // This field is used in both modes (group and non-group) to save NV and RAM space
                        // dstGroupMode = 0 - Address Manager index
                        // dstGroupMode = 1 - Group Address
  uint8 dstEP;
  uint8 numClusterIds;
  uint16 clusterIdList[MAX_BINDING_CLUSTER_IDS];
                      // Don't use MAX_BINDING_CLUSTERS_ID when
                      // using the clusterIdList field.  Use
                      // gMAX_BINDING_CLUSTER_IDS
} BindingEntry_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

// BindingTable is defined in nwk_globals.c and NWK_MAX_BINDING_ENTRIES
// is defined in f8wConfig.cfg. Don't use NWK_MAX_BINDING_ENTRIES as the
// number of records - use gNWK_MAX_BINDING_ENTRIES.
extern BindingEntry_t BindingTable[];

/*********************************************************************
 * FUNCTIONS
 */

/*
 * This function is used to initialise the binding table
 */
extern void InitBindingTable( void );

/*
 * Removes a binding table entry.
 */
extern byte bindRemoveEntry( BindingEntry_t *pBind );

/*
 * Is the clusterID in the clusterID list?
 */
extern byte bindIsClusterIDinList( BindingEntry_t *entry, uint16 clusterId );

/*
 * Removes a ClusterID from a list of ClusterIDs.
 */
extern byte bindRemoveClusterIdFromList( BindingEntry_t *entry, uint16 clusterId );

/*
 * Adds a ClusterID to a list of ClusterIDs.
 */
extern byte bindAddClusterIdToList( BindingEntry_t *entry, uint16 clusterId );

/*
 * Finds an existing src/epint to dst/epint bind record
 */
extern BindingEntry_t *bindFindExisting( byte srcEpInt,
                                     zAddrType_t *dstShortAddr, byte dstEpInt );

/*
 *  Remove bind(s) associated to a address (source or destination)
 */
extern void bindRemoveDev( zAddrType_t *shortAddr);

/*
 *  Remove bind(s) associated to a address (source)
 */
extern void bindRemoveSrcDev( uint8 ep );

/*
 * Calculate the number items this device is bound to.
 */
extern byte bindNumBoundTo( zAddrType_t *devAddr, byte devEpInt, byte srcMode );

/*
 * Count the number of reflections.
 */
extern uint16 bindNumReflections( uint8 ep, uint16 clusterID );

/*
 * Finds the binding entry for the source address,
 * endpoint and clusterID passed in as a parameter.
 */
extern BindingEntry_t *bindFind( uint8 ep, uint16 clusterID, uint8 skipping );

/*
 * Lookup a binding entry by specific Idx, if none is found
 * clears the BINDING user from Address Manager.
 */
extern void bindAddressClear( uint16 dstIdx );

/*
 * Processes the Hand Binding Timeout.
 */
extern void nwk_HandBindingTimeout( void );

/*
 * Initialize Binding Table NV Item
 */
extern byte BindInitNV( void );

/*
 * Initialize Binding Table NV Item
 */
extern void BindSetDefaultNV( void );

/*
 * Restore Binding Table from NV
 */
extern uint16 BindRestoreFromNV( void );

/*
 * Write Binding Table out to NV
 */
extern void BindWriteNV( void );

/*
 * Update network address in binding table
 */
extern void bindUpdateAddr( uint16 oldAddr, uint16 newAddr );

/*
 * This function is used to Add an entry to the binding table
 */
extern BindingEntry_t *bindAddEntry( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16 *clusterIds );

/*
 * This function returns the number of binding table entries
 */
extern uint16 bindNumOfEntries( void );

/*
 *  This function returns the number of binding entries
 *          possible and used.
 */
extern void bindCapacity( uint16 *maxEntries, uint16 *usedEntries );


/*
 *  This function returns the bind address index
 */
extern uint16 bindAddrIndexGet( zAddrType_t* addr );

/*********************************************************************
 * FUNCTION POINTERS
 */

/*
 * This function is used to Add an entry to the binding table
 */
extern BindingEntry_t *(*pbindAddEntry)( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16 *clusterIds );

/*
 * This function returns the number of binding table entries
 */
extern uint16 (*pbindNumOfEntries)( void );

/*
 *  Remove bind(s) associated to a address (source or destination)
 */
extern void (*pbindRemoveDev)( zAddrType_t *Addr );

/*
 * Initialize Binding Table NV Item
 */
extern byte (*pBindInitNV)( void );

/*
 * Initialize Binding Table NV Item
 */
extern void (*pBindSetDefaultNV)( void );

/*
 *  Restore binding table from NV
 */
extern uint16 (*pBindRestoreFromNV)( void );

/*
 *  Write binding table to NV
 */
extern void (*pBindWriteNV)( void );

/*
 * Convert address manager index to zAddrType_t for an extended address
 */
extern uint8 bindingAddrMgsHelperConvert( uint16 idx, zAddrType_t *addr );

/*
 * Convert address manager index to short address
 */
extern uint16 bindingAddrMgsHelperConvertShort( uint16 idx );

/*
 * Get a pointer to the Nth valid binding table entry.
 */
extern BindingEntry_t *GetBindingTableEntry( uint16 Nth );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* BINDINGTABLE_H */


