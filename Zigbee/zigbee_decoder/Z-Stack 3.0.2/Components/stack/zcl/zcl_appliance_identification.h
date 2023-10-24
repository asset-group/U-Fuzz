/**************************************************************************************************
  Filename:       zcl_appliance_identification.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Appliance Identification definitions.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_APPLIANCE_IDENTIFICATION_H
#define ZCL_APPLIANCE_IDENTIFICATION_H

#ifdef ZCL_APPLIANCE_IDENTIFICATION

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"


/******************************************************************************
 * CONSTANTS
 */

/*************************************************************/
/***  EN50523 Appliance Identification Cluster Attributes ***/
/************************************************************/

// Server Attributes
#define ATTRID_APPLIANCE_IDENTIFICATION_BASIC_IDENTIFICATION            0x0000 // M, R, UINT56
#define ATTRID_APPLIANCE_IDENTIFICATION_COMPANY_NAME                    0x0010 // O, R, character string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_COMPANY_ID                      0x0011 // O, R, UINT16
#define ATTRID_APPLIANCE_IDENTIFICATION_BRAND_NAME                      0x0012 // O, R, character string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_BRAND_ID                        0x0013 // O, R, UINT16
#define ATTRID_APPLIANCE_IDENTIFICATION_MODEL                           0x0014 // O, R, octet string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PART_NUMBER                     0x0015 // O, R, octet string up to 16 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PRODUCT_REVISION                0x0016 // O, R, octet string up to 6 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_SOFTWARE_REVISION               0x0017 // O, R, octet string up to 6 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_NAME               0x0018 // O, R, octet string up to 2 octets
#define ATTRID_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_ID                 0x0019 // O, R, UINT16
#define ATTRID_APPLIANCE_IDENTIFICATION_CECED_SPECIFICATION_VERSION     0x001A // O, R, UINT8

// Server Attribute Defaults
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_BASIC_IDENTIFICATION            {0,0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_COMPANY_NAME                    " "
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_COMPANY_ID                      0
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_BRAND_NAME                      " "
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_BRAND_ID                        0
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_MODEL                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PART_NUMBER                     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PRODUCT_REVISION                {0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_SOFTWARE_REVISION               {0,0,0,0,0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_NAME               {0,0}
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_PRODUCT_TYPE_ID                 0
#define ATTR_DEFAULT_APPLIANCE_IDENTIFICATION_CECED_SPECIFICATION_VERSION     0

// Product Type IDs
#define PRODUCT_TYPE_ID_WHITE_GOODS           0x0000
#define PRODUCT_TYPE_ID_DISHWASHER            0x5601
#define PRODUCT_TYPE_ID_TUMBLE_DRYER          0x5602
#define PRODUCT_TYPE_ID_WASHER_DRYER          0x5603
#define PRODUCT_TYPE_ID_WASHING_MACHINE       0x5604
#define PRODUCT_TYPE_ID_HOBS                  0x5E03
#define PRODUCT_TYPE_ID_INDUCTION_HOBS        0x5E09
#define PRODUCT_TYPE_ID_OVEN                  0x5E01
#define PRODUCT_TYPE_ID_ELECTRICAL_OVEN       0x5E06
#define PRODUCT_TYPE_ID_REFRIGERATOR_FREEZER  0x6601

// Section 9.8.2.15 of HA 1.2 Spec 11-5474-47, Table 9.49 CECED Specification Version
#define CECED_VERSION_1_0_NC     0x10  // Compliant with v1.0, not certified
#define CECED_VERSION_1_0_C      0x1A  // Compliant with v1.0, certified

/*******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ZCL_APPLIANCE_IDENTIFICATION
#endif /* ZCL_APPLIANCE_IDENTIFICATION_H */
