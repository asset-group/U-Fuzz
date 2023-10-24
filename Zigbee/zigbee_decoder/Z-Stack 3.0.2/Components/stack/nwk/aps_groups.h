/**************************************************************************************************
  Filename:       aps_groups.h
  Revised:        $Date: 2007-10-28 18:41:49 -0700 (Sun, 28 Oct 2007) $
  Revision:       $Revision: 15799 $

  Description:    Application Support Sub Layer group management functions.


  Copyright 2006-2007 Texas Instruments Incorporated. All rights reserved.

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

#ifndef APSGROUPS_H
#define APSGROUPS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * MACROS
 */
#define aps_GroupsRemaingCapacity() ( APS_MAX_GROUPS - aps_CountAllGroups() )
  
/*********************************************************************
 * CONSTANTS
 */
#define APS_GROUPS_FIND_FIRST           0xFE
#define APS_GROUPS_EP_NOT_FOUND         0xFE

#define APS_GROUP_NAME_LEN              16
  
/*********************************************************************
 * TYPEDEFS
 */

// Group Table Element
typedef struct
{
  uint16 ID;                       // Unique to this table
  uint8  name[APS_GROUP_NAME_LEN]; // Human readable name of group
} aps_Group_t;

typedef struct apsGroupItem
{
  struct apsGroupItem  *next;
  uint8                endpoint;
  aps_Group_t          group;
} apsGroupItem_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern apsGroupItem_t *apsGroupTable;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Add a group for an endpoint
 */
extern ZStatus_t aps_AddGroup( uint8 endpoint, aps_Group_t *group );

/*
 * Find a group with endpoint and groupID
 *  - returns a pointer to the group information, NULL if not found
 */
extern aps_Group_t *aps_FindGroup( uint8 endpoint, uint16 groupID );

/*
 * Find a group for an endpoint
 *  - returns endpoint found, or 0xFF for not found
 */
extern uint8 aps_FindGroupForEndpoint( uint16 groupID, uint8 lastEP );

/*
 * Find all groups for an endpoint
 *  - returns number of groups copied to groupList
 */
extern uint8 aps_FindAllGroupsForEndpoint( uint8 endpoint, uint16 *groupList );

/*
 * Remove a group with endpoint and groupID
 *  - returns TRUE if removed, FALSE if not found
 */
extern uint8 aps_RemoveGroup( uint8 endpoint, uint16 groupID );

/*
 * Remove all groups for endpoint
 */
extern void aps_RemoveAllGroup( uint8 endpoint );

/*
 * Count the number of groups for an endpoint
 */
extern uint8 aps_CountGroups( uint8 endpoint );

/*
 * Count the number of groups
 */
extern uint8 aps_CountAllGroups( void );

/*
 * Initialize the Group Table NV Space
 */
extern uint8 aps_GroupsInitNV( void );

/*
 * Initialize the Group Table NV Space to default (no entries)
 */
extern void aps_GroupsSetDefaultNV( void );

/*
 * Write the group table to NV
 */
extern void aps_GroupsWriteNV( void );

/*
 * Read the group table from NV
 */
extern uint16 aps_GroupsRestoreFromNV( void );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* APSGROUPS_H */


