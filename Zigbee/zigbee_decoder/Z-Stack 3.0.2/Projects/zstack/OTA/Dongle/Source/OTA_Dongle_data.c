/**************************************************************************************************
  Filename:       OTA_Dongle_data.c
  Revised:        $Date: 2011-08-16 16:45:58 -0700 (Tue, 16 Aug 2011) $
  Revision:       $Revision: 27200 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2010-2011 Texas Instruments Incorporated. All rights reserved.

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
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_ota.h"
#include "zcl_general.h"

#include "OTA_Dongle.h"

/*********************************************************************
 * CONSTANTS
 */

#define OTA_DONGLE_DEVICE_VERSION     0
#define OTA_DONGLE_FLAGS              0

#define OTA_DONGLE_HWVERSION          1
#define OTA_DONGLE_ZCLVERSION         1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Basic Cluster
const uint8 OTA_Dongle_HWRevision = OTA_DONGLE_HWVERSION;
const uint8 OTA_Dongle_ZCLVersion = OTA_DONGLE_ZCLVERSION;
const uint8 OTA_Dongle_ManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 OTA_Dongle_ModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 OTA_Dongle_DateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 OTA_Dongle_PowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 OTA_Dongle_LocationDescription[17] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 OTA_Dongle_PhysicalEnvironment = 0;
uint8 OTA_Dongle_DeviceEnable = DEVICE_ENABLED;

// Identify Cluster
uint16 OTA_Dongle_IdentifyTime = 0;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t OTA_Dongle_Attrs[OTA_DONGLE_MAX_ATTRIBUTES] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&OTA_Dongle_HWRevision      // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&OTA_Dongle_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)OTA_Dongle_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)OTA_Dongle_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)OTA_Dongle_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&OTA_Dongle_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)OTA_Dongle_LocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&OTA_Dongle_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&OTA_Dongle_DeviceEnable
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&OTA_Dongle_IdentifyTime
    }
  },

};

/*********************************************************************
 * CLUSTER OPTION DEFINITIONS
 */
zclOptionRec_t OTA_Dongle_Options[OTA_DONGLE_MAX_OPTIONS] =
{
  {
    ZCL_CLUSTER_ID_OTA,
#ifndef OTA_HA
    ( AF_EN_SECURITY ),
#endif
  },
};

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define OTA_DONGLE_MAX_INCLUSTERS       3
const cId_t OTA_Dongle_InClusterList[OTA_DONGLE_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
};

#define OTA_DONGLE_MAX_OUTCLUSTERS       1
const cId_t OTA_Dongle_OutClusterList[OTA_DONGLE_MAX_OUTCLUSTERS] =
{
  ZCL_CLUSTER_ID_GEN_BASIC
};

SimpleDescriptionFormat_t OTA_Dongle_SimpleDesc =
{
  OTA_DONGLE_ENDPOINT,                  //  int Endpoint;
  OTA_SAMPLE_PROFILE_ID,                //  uint16 AppProfId[2];
  OTA_SAMPLE_DEVICEID,                  //  uint16 AppDeviceId[2];
  OTA_DONGLE_DEVICE_VERSION,            //  int   AppDevVer:4;
  OTA_DONGLE_FLAGS,                     //  int   AppFlags:4;
  OTA_DONGLE_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)OTA_Dongle_InClusterList, //  byte *pAppInClusterList;
  OTA_DONGLE_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)OTA_Dongle_OutClusterList //  byte *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/****************************************************************************
****************************************************************************/


