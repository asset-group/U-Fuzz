/**************************************************************************************************
  Filename:       zcl_meter_identification.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Meter Identification definitions.


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

#ifndef ZCL_METER_IDENTIFICATION_H
#define ZCL_METER_IDENTIFICATION_H

#ifdef ZCL_METER_IDENTIFICATION

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

/************************************************/
/***  Meter Identification Cluster Attributes ***/
/************************************************/

// Server Attributes
#define ATTRID_METER_IDENTIFICATION_COMPANY_NAME                0x0000  // M, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_METER_TYPE_ID               0x0001  // M, R, UINT16
#define ATTRID_METER_IDENTIFICATION_DATA_QUALITY_ID             0x0004  // M, R, UINT16
#define ATTRID_METER_IDENTIFICATION_CUSTOMER_NAME               0x0005  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_MODEL                       0x0006  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_PART_NUMBER                 0x0007  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_PRODUCT_REVISION            0x0008  // O, R, character string up to 6 octets
#define ATTRID_METER_IDENTIFICATION_SOFTWARE_REVISION           0x000A  // O, R, character string up to 6 octets
#define ATTRID_METER_IDENTIFICATION_UTILITY_NAME                0x000B  // O, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_POD                         0x000C  // M, R, character string up to 16 octets
#define ATTRID_METER_IDENTIFICATION_AVAILABLE_POWER             0x000D  // M, R, INT24
#define ATTRID_METER_IDENTIFICATION_POWER_THRESHOLD             0x000E  // M, R, INT24

// Server Attribute Defaults
#define ATTR_DEFAULT_METER_IDENTIFICATION_COMPANY_NAME                ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_METER_TYPE_ID               0
#define ATTR_DEFAULT_METER_IDENTIFICATION_DATA_QUALITY_ID             0
#define ATTR_DEFAULT_METER_IDENTIFICATION_CUSTOMER_NAME               ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_MODEL                       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_PART_NUMBER                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_PRODUCT_REVISION            {0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_SOFTWARE_REVISION           {0,0,0,0,0,0}
#define ATTR_DEFAULT_METER_IDENTIFICATION_UTILITY_NAME                ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_POD                         ""
#define ATTR_DEFAULT_METER_IDENTIFICATION_AVAILABLE_POWER             0
#define ATTR_DEFAULT_METER_IDENTIFICATION_POWER_THRESHOLD             0

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

#endif // ZCL_METER_IDENTIFICATION
#endif /* ZCL_METER_IDENTIFICATION_H */
