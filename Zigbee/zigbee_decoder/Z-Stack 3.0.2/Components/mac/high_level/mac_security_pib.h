/**************************************************************************************************
  Filename:       mac_security_pib.h
  Revised:        $Date: 2014-11-06 11:03:55 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41021 $

  Description:    Internal interface file for the Security-related MAC PIB module.


  Copyright 2010-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef MAC_SECURITY_PIB_H
#define MAC_SECURITY_PIB_H

#ifndef MAX_DEVICE_TABLE_ENTRIES
  #define MAX_DEVICE_TABLE_ENTRIES          8
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "mac_api.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Implementation specific defines. The values depend on memory resources. If the user requires
 * a number that exceeds the maximum, "mac_security_pib.c" must also be changed.
 */
#ifndef MAX_KEY_TABLE_ENTRIES
  #define MAX_KEY_TABLE_ENTRIES             2
#endif

#ifndef MAX_SECURITY_LEVEL_TABLE_ENTRIES
  #define MAX_SECURITY_LEVEL_TABLE_ENTRIES  1
#endif

/* MAC key table related constants */
#ifndef MAX_KEY_ID_LOOKUP_ENTRIES
  #define MAX_KEY_ID_LOOKUP_ENTRIES         1
#endif

#define MAX_KEY_DEVICE_TABLE_ENTRIES        MAX_DEVICE_TABLE_ENTRIES

#ifndef MAX_KEY_USAGE_TABLE_ENTRIES
  #define MAX_KEY_USAGE_TABLE_ENTRIES       1
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Security-related MAC PIB type */
typedef struct
{
  uint8              keyTableEntries;
  uint8              deviceTableEntries;
  uint8              securityLevelTableEntries;

  uint8              autoRequestSecurityLevel;
  uint8              autoRequestKeyIdMode;
  uint8              autoRequestKeySource[MAC_KEY_SOURCE_MAX_LEN];
  uint8              autoRequestKeyIndex;

  uint8              defaultKeySource[MAC_KEY_SOURCE_MAX_LEN];
  sAddr_t            panCoordExtendedAddress;
  uint16             panCoordShortAddress;

  /* Propriority Security PIBs */
  keyDescriptor_t            macKeyTable[MAX_KEY_TABLE_ENTRIES];
  keyIdLookupDescriptor_t    macKeyIdLookupList[MAX_KEY_TABLE_ENTRIES][MAX_KEY_ID_LOOKUP_ENTRIES];
  keyDeviceDescriptor_t      macKeyDeviceList[MAX_KEY_TABLE_ENTRIES][MAX_KEY_DEVICE_TABLE_ENTRIES];
  keyUsageDescriptor_t       macKeyUsageList[MAX_KEY_TABLE_ENTRIES][MAX_KEY_USAGE_TABLE_ENTRIES];
  deviceDescriptor_t         macDeviceTable[MAX_DEVICE_TABLE_ENTRIES];
  securityLevelDescriptor_t  macSecurityLevelTable[MAX_SECURITY_LEVEL_TABLE_ENTRIES];

} macSecurityPib_t;


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* Pointer to the mac security PIB */
#if defined( FEATURE_MAC_PIB_PTR )
extern macSecurityPib_t* pMacSecurityPib;
#else
#define pMacSecurityPib (&macSecurityPib)
#endif /* FEATURE_DUAL_MAC_PIB */


/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macSecurityPibReset(void);
MAC_INTERNAL_API uint8 MAC_MlmeGetPointerSecurityReq(uint8 pibAttribute, void **pValue);
MAC_INTERNAL_API uint8 macSecurityPibIndex(uint8 pibAttribute);

/**************************************************************************************************
*/

#if defined( FEATURE_MAC_PIB_PTR ) && defined( CC26XX )
#error "ERROR! CC26XX does not support FEATURE_MAC_PIB_PTR."
#endif

#endif /* MAC_SECURITY_PIB_H */

