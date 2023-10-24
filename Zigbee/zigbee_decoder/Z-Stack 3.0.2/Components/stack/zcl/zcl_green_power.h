/**************************************************************************************************
  Filename:       zcl_green_power.h
  Revised:        $Date: 2014-10-14 13:03:14 -0700 (Tue, 14 Oct 2014) $
  Revision:       $Revision: 40629 $

  Description:    This file contains the ZCL General definitions.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_GREEN_POWER_H
#define ZCL_GREEN_POWER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "cGP_stub.h"  

 /*********************************************************************
 * MACROS
 */ 

#define LSINK_ADDR_LEN                10   // in bytes   
#define PROXY_TBL_ENTRY_LEN           65   // in bytes   

#define PROXY_TBL_ENTRY_OPT                0
#define PROXY_TBL_ENTRY_GPD_ID             2
#define PROXY_TBL_ENTRY_GPD_EP            10
#define PROXY_TBL_ENTRY_ALIAS             11
#define PROXY_TBL_ENTRY_SEC_OPT           13
#define PROXY_TBL_ENTRY_SEC_FRAME         14
#define PROXY_TBL_ENTRY_GPD_KEY           18   
#define PROXY_TBL_ENTRY_1ST_LSINK_ADDR    34
#define PROXY_TBL_ENTRY_2ND_LSINK_ADDR    44
#define PROXY_TBL_ENTRY_GRP_TBL_ENTRIES   54
#define PROXY_TBL_ENTRY_1ST_GRP_ADDR      55
#define PROXY_TBL_ENTRY_2ND_GRP_ADDR      59
#define PROXY_TBL_ENTRY_GRP_RAD           63
#define PROXY_TBL_ENTRY_SEARCH_COUNTER    64
  
#define PROXY_TBL_ENTRY_1ST_GRP_BIT        0
#define PROXY_TBL_ENTRY_2ND_GRP_BIT        1
  

   
// Bit shift
#define GP_BIT(n)                  ( 1<<(n) )

// Create a bitmask of specified len.
#define GP_BIT_MASK(len)           ( GP_BIT(len)-1 )

// Create a bitfield mask of length starting at start bit.
#define GP_BF_MASK(start, len)     ( GP_BIT_MASK(len)<<(start) )

// Prepare a bitmask for insertion or combining.
#define GP_BF_PREP(x, start, len)  ( (x) & ( GP_BIT_MASK(len) << (start) ) )

// Extract a bitfield of length starting at start bit from y.
#define GP_BF_GET(y, start, len)   ( ((y)>>(start)) & GP_BIT_MASK(len) )

// Extract a bitfield of length starting at start bit from y.
#define GP_BF_GET_BIT(y, start)   ( GP_BF_GET(y, start, 1) )

// Insert a new bitfield value x into y.
#define GP_BF_SET(y, x, start, len)    \
    ( y= ((y) &~ GP_BF_MASK(start, len)) | GP_BF_PREP(x, start, len) )
      
// Insert a new bitfield value x into y.
#define PROXY_TBL_BF_SET(y, x, starty, leny, startx, lenx)    \
    ( *y= ((*y) &~ GP_BF_MASK(starty, leny)) | ( GP_BF_GET(x, startx, lenx) ) << ( starty ) )
      
      
#define GP_IS_APPLICATION_ID_GPD(x)                    ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                         GP_OPT_APP_ID_LEN) == \
                                                         GP_OPT_APP_ID_GPD )

#define GP_IS_APPLICATION_ID_IEEE(x)                   ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                         GP_OPT_APP_ID_LEN) == \
                                                         GP_OPT_APP_ID_IEEE )

#define GP_IS_REQ_TYP_GPD(x)                           ( GP_BF_GET(x, GP_OPT_REQ_TYP_BIT, \
                                                         GP_OPT_REQ_TYP_LEN) == \
                                                         GP_OPT_REQ_TYP_GPD_ID )

#define GP_IS_REQ_TPY_INDEX(x)                         ( GP_BF_GET(x, GP_OPT_REQ_TYP_BIT, \
                                                         GP_OPT_REQ_TYP_LEN) == \
                                                         GP_OPT_REQ_TYP_INDEX )

#define GP_ADD_SINK(x)                                 ( GP_BF_GET(x, GP_OPT_ADD_SINK_BIT, \
                                                         GP_OPT_ADD_SINK_BIT_FIELD_LEN) == \
                                                         GP_OPT_ADD_SINK_ADD_PAIRING )

#define GP_REMOVE_GPD(x)                               ( GP_BF_GET(x, GP_OPT_REMOVE_GPD_BIT, \
                                                         GP_OPT_REMOVE_GPD_FIELD_LEN) == \
                                                         GP_OPT_GPD_REMOVE )
      
#define GP_IS_COMMUNICATION_MODE_FULL_UNICAST(x)       ( GP_BF_GET(x, GP_OPT_COMMUNICATION_MODE_BIT, \
                                                         GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                         GP_OPT_COMMUNICATION_MODE_FULL_UNICAST )

#define GP_IS_COMMUNICATION_MODE_GRPCAST_DGROUP_ID(x)  ( GP_BF_GET(x, GP_OPT_COMMUNICATION_MODE_BIT, \
                                                         GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                         GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID )

#define GP_IS_COMMUNICATION_MODE_GRPCAST_GROUP_ID(x)   ( GP_BF_GET(x, GP_OPT_COMMUNICATION_MODE_BIT, \
                                                         GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                         GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID )

#define GP_IS_COMMUNICATION_MODE_LIGHT_UNICAST(x)      ( GP_BF_GET(x, GP_OPT_COMMUNICATION_MODE_BIT, \
                                                         GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                         GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST )

#define GP_GET_GPD_FIXED_BIT(x)                        ( GP_BF_GET_BIT(x, GP_OPT_GPD_FIXED) )

#define GP_GET_GPD_MAC_SEQ_CAP_BIT(x)                  ( GP_BF_GET_BIT(x, GP_OPT_GPD_MAC_SEC_CAP) )

#define GP_GET_SEC_LEVEL(x)                            ( GP_BF_GET(x, GP_OPT_SEC_LEVEL, \
                                                         GP_OPT_SEC_LEVEL_LEN) )

#define GP_GET_SEC_KEY_TYPE(x)                         ( GP_BF_GET(x, GP_OPT_SEC_KEY_TYPE, \
                                                         GP_OPT_SEC_KEY_TYPE_LEN) )

#define GP_SEC_COUNTER(x)                              ( GP_BF_GET(x, GP_OPT_GPD_SEC_COUNTER, \
                                                         GP_OPT_GPD_SEC_COUNTER_LEN) == \
                                                         GP_FIELD_PRESENT )

#define GP_SEC_KEY(x)                                  ( GP_BF_GET(x, GP_OPT_GPD_SEC_KEY, \
                                                         GP_OPT_GPD_SEC_KEY_LEN) == \
                                                         GP_FIELD_PRESENT )

#define GP_ALIAS(x)                                    ( GP_BF_GET(x, GP_OPT_ALIAS, \
                                                         GP_OPT_ALIAS_LEN) )

#define GP_FORWARDING_RADIUS(x)                        ( GP_BF_GET(x, GP_OPT_FORWARDING_RADIUS, \
                                                         GP_OPT_FORWARDING_RADIUS_LEN) )

/*******************************************************************************
* Proxy Table Entry Options Bitfield managment
*
* Bits: 0..2              3               4               5              6         
* ApplicationID      EntryActive      EntryValid      Sequence      Lightweight    
*                                                      number       Unicast GPS    
*                                                   capabilities
*
*             7               8                  9                10             11       
*          Derived      Commissioned       FirstToForward      InRange       GPD Fixed    
*         Group GPS       Group GPS
*                            
*                           12                     13                14                  15
*                   HasAllUnicastRoutes      AssignedAlias      SecurityUse      Options Extension
*
********************************************************************************/     

#define PROXY_TBL_GET_APPLICTION_ID(x)                 ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )
#define PROXY_TBL_COMP_APPLICTION_ID(y,x)              ( GP_BF_GET(y, GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) \
                                                         == GP_BF_GET(x, GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )

#define PAIRING_PROXY_TBL_SET_APPLICATION_ID(y, x)     ( PROXY_TBL_BF_SET(y, x, PROXY_TBL_OPT_APP_ID_BIT, PROXY_TBL_OPT_APP_ID_LEN, \
                                                                                GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )

#define PROXY_TBL_GET_ENTRY_ACTIVE(x)                  ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_ENTRY_ACTIVE_BIT) )
#define PROXY_TBL_SET_ENTRY_ACTIVE(y)                  ( SET_BIT(y, PROXY_TBL_OPT_ENTRY_ACTIVE_BIT) )
#define PROXY_TBL_CLR_ENTRY_ACTIVE(y)                  ( CLR_BIT(y, PROXY_TBL_OPT_ENTRY_ACTIVE_BIT) )

#define PROXY_TBL_GET_ENTRY_VALID(x)                   ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_ENTRY_VALID_BIT) )
#define PROXY_TBL_SET_ENTRY_VALID(y)                   ( SET_BIT(y, PROXY_TBL_OPT_ENTRY_VALID_BIT) )
#define PROXY_TBL_CLR_ENTRY_VALID(y)                   ( CLR_BIT(y, PROXY_TBL_OPT_ENTRY_VALID_BIT) )

#define PAIRING_PROXY_TBL_SET_MAC_SEQ_CAP(y, x)        ( PROXY_TBL_BF_SET(y, x, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_LEN, \
                                                                                GP_OPT_GPD_MAC_SEC_CAP, GP_OPT_GPD_MAC_SEC_CAP_LEN) )

#define PROXY_TBL_GET_LIGHTWIGHT_UNICAST(x)            ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT) )
#define PROXY_TBL_SET_LIGHTWIGHT_UNICAST(y)            ( SET_BIT(y, PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT) )
#define PROXY_TBL_CLR_LIGHTWIGHT_UNICAST(y)            ( CLR_BIT(y, PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT) )

#define PROXY_TBL_GET_DGROUP(x)                        ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_DGROUP_BIT) )
#define PROXY_TBL_SET_DGROUP(y)                        ( SET_BIT(y, PROXY_TBL_OPT_DGROUP_BIT) )
#define PROXY_TBL_CLR_DGROUP(y)                        ( CLR_BIT(y, PROXY_TBL_OPT_DGROUP_BIT) )

#define PROXY_TBL_GET_CGROUP(x)                        ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_CGROUP_BIT) )
#define PROXY_TBL_SET_CGROUP(y)                        ( SET_BIT(y, PROXY_TBL_OPT_CGROUP_BIT) )
#define PROXY_TBL_CLR_CGROUP(y)                        ( CLR_BIT(y, PROXY_TBL_OPT_CGROUP_BIT) )

#define PROXY_TBL_GET_FIRST_TO_FORWARD(x)              ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT) )
#define PROXY_TBL_SET_FIRST_TO_FORWARD(y)              ( SET_BIT(y, PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT) )
#define PROXY_TBL_CLR_FIRST_TO_FORWARD(y)              ( CLR_BIT(y, PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT) )

#define PROXY_TBL_SET_IN_RANGE(y)                      ( SET_BIT(y, PROXY_TBL_OPT_IN_RANGE_BIT) )
#define PROXY_TBL_CLR_IN_RANGE(y)                      ( CLR_BIT(y, PROXY_TBL_OPT_IN_RANGE_BIT) )

#define PROXY_TBL_GET_GPD_FIXED(x)                     ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_GPD_FIXED_BIT) )
#define PROXY_TBL_SET_GPD_FIXED(y)                     ( SET_BIT(y, PROXY_TBL_OPT_GPD_FIXED_BIT) )
#define PROXY_TBL_CLR_GPD_FIXED(y)                     ( CLR_BIT(y, PROXY_TBL_OPT_GPD_FIXED_BIT) )

#define PROXY_TBL_SET_HAS_ALL_ROUTES(y)                ( SET_BIT(y, PROXY_TBL_OPT_HAS_ALL_ROUTES_BIT) )
#define PROXY_TBL_CLR_HAS_ALL_ROUTES(y)                ( CLR_BIT(y, PROXY_TBL_OPT_HAS_ALL_ROUTES_BIT) )

#define PROXY_TBL_GET_ASSIGNED_ALIAS(x)                ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT) )
#define PROXY_TBL_SET_ASSIGNED_ALIAS(y)                ( SET_BIT(y, PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT) )
#define PROXY_TBL_CLR_ASSIGNED_ALIAS(y)                ( CLR_BIT(y, PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT) )

#define PROXY_TBL_GET_SEC_USE(x)                       ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_SEC_USE_BIT) )
#define PROXY_TBL_SET_SEC_USE(y)                       ( SET_BIT(y, PROXY_TBL_OPT_SEC_USE_BIT) )
#define PROXY_TBL_CLR_SEC_USE(y)                       ( CLR_BIT(y, PROXY_TBL_OPT_SEC_USE_BIT) )

#define PROXY_TBL_GET_SEC_CAP(x)                       ( GP_BF_GET_BIT(x, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT) )
#define PROXY_TBL_SET_SEC_CAP(y)                       ( SET_BIT(y, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT) )
#define PROXY_TBL_CLR_SEC_CAP(y)                       ( CLR_BIT(y, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT) )

#define PROXY_TBL_SET_OPT_EXT(y)                       ( SET_BIT(y, PROXY_TBL_OPT_EXT_BIT) )
#define PROXY_TBL_CLR_OPT_EXT(y)                       ( CLR_BIT(y, PROXY_TBL_OPT_EXT_BIT) )


/*******************************************************************************
* Proxy Table Entry Security Options Bitfield managment
*
* Bits: 0..1             3..4                5..7       
* SecurityLevel      SecurityKeyType       Reserved  
*
********************************************************************************/     

#define PROXY_TBL_GET_SEC_OPT_SECURITY_LVL(x)          ( GP_BF_GET(x, PROXY_TBL_SEC_OPT_SECURITY_LVL_BIT, PROXY_TBL_SEC_OPT_SECURITY_LVL_LEN) )

#define PROXY_TBL_GET_SEC_OPT_SECURITY_KEY_TYP(x)      ( GP_BF_GET(x, PROXY_TBL_SEC_OPT_SECURITY_KEY_TYP_BIT, PROXY_TBL_SEC_OPT_SECURITY_KEY_TYP_LEN) )


// GP notification options 

#define GP_NTF_SET_ALSO_UNICAST(y, x)                  ( PROXY_TBL_BF_SET(y, x, GP_NTF_OPT_ALSO_UNICAST, GP_NTF_OPT_ALSO_UNICAST_LEN, \
                                                             PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT, PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_LEN) )

#define GP_NTF_SET_ALSO_DGROUP(y, x)                   ( PROXY_TBL_BF_SET(y, x, GP_NTF_OPT_ALSO_DGROUP, GP_NTF_OPT_ALSO_DGROUP_LEN, \
                                                             PROXY_TBL_OPT_DGROUP_BIT, PROXY_TBL_OPT_DGROUP_LEN) )

#define GP_NTF_SET_ALSO_CGROUP(y, x)                   ( PROXY_TBL_BF_SET(y, x, GP_NTF_OPT_ALSO_CGROUP, GP_NTF_OPT_ALSO_CGROUP_LEN, \
                                                             PROXY_TBL_OPT_CGROUP_BIT, PROXY_TBL_OPT_CGROUP_LEN) )

#define GP_NTF_SET_SEC_LEVEL(y, x)                     ( PROXY_TBL_BF_SET(y, x, GP_NTF_OPT_SEC_LEVEL, GP_NTF_OPT_SEC_LEVEL_LEN, \
                                                             SEC_OPT_SEC_LEVEL, SEC_OPT_SEC_LEVEL_LEN) )

#define GP_NTF_SET_SEC_KEY_TYPE(y, x)                  ( PROXY_TBL_BF_SET(y, x, GP_NTF_OPT_SEC_KEY_TYPE, GP_NTF_OPT_SEC_KEY_TYPE_LEN, \
                                                             SEC_OPT_SEC_KEY_TYPE, SEC_OPT_SEC_KEY_TYPE_LEN) )

// GP commissioning notification options 
#define GP_CNTF_SET_SEC_LEVEL(y, x)                    ( PROXY_TBL_BF_SET(y, x, GP_CNTF_OPT_SEC_LEVEL, GP_CNTF_OPT_SEC_LEVEL_LEN, \
                                                             SEC_OPT_SEC_LEVEL, SEC_OPT_SEC_LEVEL_LEN) )

#define GP_CNTF_SET_SEC_KEY_TYPE(y, x)                 ( PROXY_TBL_BF_SET(y, x, GP_CNTF_OPT_SEC_KEY_TYPE, GP_CNTF_OPT_SEC_KEY_TYPE_LEN, \
                                                             SEC_OPT_SEC_KEY_TYPE, SEC_OPT_SEC_KEY_TYPE_LEN) )

#define GP_CNTF_SET_SEC_FAIL(y)                        ( SET_BIT(y, GP_CNTF_OPT_SEC_FAIL) )
#define GP_CNTF_CLR_SEC_FAIL(y)                        ( CLR_BIT(y, GP_CNTF_OPT_SEC_FAIL) )

#define GP_CNTF_SET_BIDIRECTIONAL_CAP(y)               ( SET_BIT(y, GP_CNTF_OPT_BIDIRECTIONAL_CAP) )

#define GP_CNTF_SET_PROXY_INFO(y)                      ( SET_BIT(y, GP_CNTF_OPT_PROXY_INFO) )


#define PROXY_TBL_SET_RX_AFTER_TX(y)                   ( SET_BIT(y, GP_NTF_OPT_RX_AFTER_TX) )
#define PROXY_TBL_CLR_RX_AFTER_TX(y)                   ( CLR_BIT(y, GP_NTF_OPT_RX_AFTER_TX) )

#define PROXY_TBL_SET_TX_QUEUE_FULL(y)                 ( SET_BIT(y, GP_NTF_OPT_TX_QUEUE_FULL) )
#define PROXY_TBL_CLR_TX_QUEUE_FULL(y)                 ( CLR_BIT(y, GP_NTF_OPT_TX_QUEUE_FULL) )

#define PROXY_TBL_SET_BIDIRECTIONAL_CAP(y)             ( SET_BIT(y, GP_NTF_OPT_BIDIRECTIONAL_CAP) )
#define PROXY_TBL_SET_PROXY_INFO(y)                    ( SET_BIT(y, GP_NTF_OPT_PROXY_INFO) )


/*********************************************************************
 * CONSTANTS
 */

// For internal EP's simple descriptor
#define GREEN_POWER_INTERNAL_ENDPOINT                       0xF2 
#define ZCL_GP_PROFILE_ID                                 0xA1E0
#define ZCL_GP_DEVICEID_PROXY_BASIC                       0x0061
  
/********************************/
/*** Green Power Cluster Attributes ***/
/********************************/
#define ATTRID_GP_GPS_MAX_SINK_TABLE_ENTRIES              0x0000
#define ATTRID_GP_SINK_TABLE                              0x0001
#define ATTRID_GP_GPS_COMMUNICATION_MODE                  0x0002
#define ATTRID_GP_GPS_COMMISSIONING_EXIT_MODE             0x0003
#define ATTRID_GP_GPS_COMMISSIONING_WINDOW                0x0004
#define ATTRID_GP_GPS_SECURITY_LEVEL                      0x0005
#define ATTRID_GP_GPS_FUNCTIONALITY                       0x0006
#define ATTRID_GP_GPS_ACTIVE_FUNCTIONALITY                0x0007

#define ATTRID_GP_GPP_MAX_PROXY_TABLE_ENTRIES             0x0010
#define ATTRID_GP_PROXY_TABLE                             0x0011
#define ATTRID_GP_GPP_NOTIFICATION_RETRY_NUMBER           0x0012
#define ATTRID_GP_GPP_NOTIFICATION_RETRY_TIMER            0x0013
#define ATTRID_GP_GPP_MAX_SEARCH_COUNTER                  0x0014
#define ATTRID_GP_GPP_BLOCKED_GPD_ID                      0x0015
#define ATTRID_GP_GPP_FUNCTIONALITY                       0x0016
#define ATTRID_GP_GPP_ACTIVE_FUNCTIONALITY                0x0017

// A.3.3.3 Attributes shared by client and server 
#define ATTRID_GP_SHARED_SEC_KEY_TYPE                     0x0020
#define ATTRID_GP_SHARED_SEC_KEY                          0x0021
#define ATTRID_GP_LINK_KEY                                0x0022
   
/*** Maximum number of Proxy Table entries supported by this device ***/
#define GPP_MAX_PROXY_TABLE_ENTRIES        0x05
   
/*** Number of full unicast GP Notification retries on lack of GP 
     Notification Response ***/
#define GPP_NOTIFICATION_RETRY_NUMBER      0x02

/*** Time in ms between full unicast GP Notification retries on 
     lack of GP Notification Response ***/
#define GPP_NOTIFICATION_RETRY_TIMER       0x64
   
/*** The frequency of sink rediscovery for inactive Proxy
     Table entries ***/  
#define GPP_MAX_SEARCH_COUNTER             0x0A
   
/*** A list holding information about blocked GPD IDs ***/ 
#define GPP_BLOCKED_GPD_ID                 0x00
   
/*** The optional GP functionality supported by this proxy ***/ 
/*** Format of the gppFunctionality attribute ***/
/***********************************************************************************************************
                            Indication Functionality Basic Proxy
-------------------------------------------------------------------------------------------------------------
b0    ||  GP feature                                                                              ||    0b1
b1    ||  Direct communication (reception of GPDF via GP stub)                                    ||    0b1
b2    ||  Derived groupcast communication                                                         ||    0b1
b3    ||  Pre-commissioned groupcast communication                                                ||    0b1
b4    ||  Full unicast communication                                                              ||    0b0
b5    ||  Lightweight unicast communication                                                       ||    0b1
b6    ||  Reserved                                                                                ||    0b0
b7    ||  Bidirectional operation                                                                 ||    0b0
b8    ||  Proxy Table maintenance (active and passive, for GPD mobility and GPP robustness)       ||    0b0
b9    ||  Reserved                                                                                ||    0b0
b10   ||  GP commissioning                                                                        ||    0b1
b11   ||  CT-based commissioning                                                                  ||    0b1
b12   ||  Maintenance of GPD (deliver channel/key during operation)                               ||    0b0
b13   ||  gpdSecurityLevel = 0b00                                                                 ||    0b1
b14   ||  Deprecated: gpdSecurityLevel = 0b01                                                     ||    0b0
b15   ||  gpdSecurityLevel = 0b10                                                                 ||    0b1
b16   ||  gpdSecurityLevel = 0b11                                                                 ||    0b1
b17   ||  Reserved                                                                                ||    0b0
b18   ||  Reserved                                                                                ||    0b0
b19   ||  GPD IEEE address                                                                        ||    0b1
      ||  b20 – b23 Reserved                                                                      ||    0b0
************************************************************************************************************/
#define GPP_FUNCTIONALITY                  0x00009AC2F  // Table 42 – Format of the gppFunctionality attribute

/*** The optional GP functionality supported by this proxy that
     is active ***/
#define GPP_ACTIVE_FUNCTIONALITY           0x000000001

  
#define GP_SHARED_SEC_KEY_TYPE             0x00
#define GP_LINK_LEY                        DEFAULT_TC_LINK_KEY
#define GP_CLUSTER_REVISION                0x0001

// A.3.4.2.2.2.1 Options parameter
#define GPP_TBL_OPT_APPLICTION_ID                  0  // 3bit field
#define GPP_TBL_OPT_ENTRY_ACTIVE                   3  // 1bit
#define GPP_TBL_OPT_ENTRY_VALID                    4  // 1bit
#define GPP_TBL_OPT_SEQUENCE_NUM_CAP               5  // 1bit
#define GPP_TBL_OPT_LIGHT_WEIGHT_UNICAST_GPS       6  // 1bit
#define GPP_TBL_OPT_DERIVED_GROUP_GPS              7  // 1bit
#define GPP_TBL_OPT_COMMISSIONED_GROUP_GPS         8  // 1bit
#define GPP_TBL_OPT_FIRST_TO_FORWARD               9  // 1bit 
#define GPP_TBL_OPT_IN_RANGE                      10  // 1bit
#define GPP_TBL_OPT_GPD_FIXED                     11  // 1bit
#define GPP_TBL_OPT_HAS_ALL_UNICAST_ROUTES        12  // 1bit
#define GPP_TBL_OPT_ASSIGNED_ALIAS                13  // 1bit
#define GPP_TBL_OPT_SECURITY_USE                  14  // 1bit
#define GPP_TBL_OPT_OPTIONS_EXTENSIONS            15  // 1bit 

// A.3.4.2.2.2.4 Security-related parameters
#define GPP_TBL_SEC_OPT_SECURITY_LEVEL             0  // 2bit field
#define GPP_TBL_SEC_OPT_SECURITY_KEY_TYPE          2  // 2bit field
  
// A.3.4.2.2.2.8 Extended Options parameter
#define GPP_TBL_EXT_OPT_FULL_UNICAST_GPS           0  // 2bit field


/******************************/
/*** Green Power Cluster Commands ***/
/******************************/
#define COMMAND_GP_NOTIFICATION                           0x00
#define COMMAND_GP_PAIRING_SEARCH                         0x01
#define COMMAND_GP_PAIRING_STOP                           0x03
#define COMMAND_GP_COMMISSIONING_NOTIFICATION             0x04
#define COMMAND_GP_COMMISSIONING_MODE                     0x05
#define COMMAND_GP_TRANSLATION_TABLE_UPDATE_COMMAND       0x07
#define COMMAND_GP_TRANSLATION_TABLE_REQUEST              0x08
#define COMMAND_GP_PAIRING_CONFIGURATION                  0x09
#define COMMAND_GP_SINK_TABLE_REQUEST                     0x0A
#define COMMAND_GP_PROXY_TABLE_RESPONSE                   0x0B
  
  
#define COMMAND_GP_NOTIFICATION_RESPONSE                  0x00
#define COMMAND_GP_PAIRING                                0x01
#define COMMAND_GP_PROXY_COMMISSIONING_MODE               0x02
#define COMMAND_GP_RESPONSE                               0x06
#define COMMAND_GP_TRANSLATION_TABLE_RESPONSE             0x08
#define COMMAND_GP_SINK_TABLE_RESPONSE                    0x0A
#define COMMAND_GP_PROXY_TABLE_REQUEST                    0x0B

#define GP_FIELD_PRESENT                                  0x01

/*************************************************************
* Proxy Table Entry Options Bitfield
**************************************************************/
// Application ID bitfied
#define PROXY_TBL_OPT_APP_ID_LEN                          0x03 // length of bitfield
#define PROXY_TBL_OPT_APP_ID_BIT                          0x00

// Entry Active Bit
#define PROXY_TBL_OPT_ENTRY_ACTIVE_LEN                    0x01
#define PROXY_TBL_OPT_ENTRY_ACTIVE_BIT                    0x03

// Entry Valid Bit
#define PROXY_TBL_OPT_ENTRY_VALID_LEN                     0x01
#define PROXY_TBL_OPT_ENTRY_VALID_BIT                     0x04
   
// GPD MAC sequence capablities
#define PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_LEN                 0x01
#define PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT                 0x05

// Lightweight Unicast GPS Bit
#define PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_LEN              0x01
#define PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT              0x06

// Derived Group GPS Bit
#define PROXY_TBL_OPT_DGROUP_LEN                          0x01
#define PROXY_TBL_OPT_DGROUP_BIT                          0x07

// Commissioned Group GPS Bit
#define PROXY_TBL_OPT_CGROUP_LEN                          0x01
#define PROXY_TBL_OPT_CGROUP_BIT                          0x00

// FirstToForward Bit
#define PROXY_TBL_OPT_FIRST_TO_FORWARD_LEN                0x01
#define PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT                0x01

// InRange Bit
#define PROXY_TBL_OPT_IN_RANGE_LEN                        0x01
#define PROXY_TBL_OPT_IN_RANGE_BIT                        0x02

// GPD Fixed Bit
#define PROXY_TBL_OPT_GPD_FIXED_LEN                       0x01
#define PROXY_TBL_OPT_GPD_FIXED_BIT                       0x03

// HasAllUnicastRoutes Bit
#define PROXY_TBL_OPT_HAS_ALL_ROUTES_LEN                  0x01
#define PROXY_TBL_OPT_HAS_ALL_ROUTES_BIT                  0x04

// Assigned Alias
#define PROXY_TBL_OPT_ASSIGNED_ALIAS_LEN                  0x01
#define PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT                  0x05
       
// SecurityUse Bit
#define PROXY_TBL_OPT_SEC_USE_LEN                         0x01
#define PROXY_TBL_OPT_SEC_USE_BIT                         0x06

// Options Extension Bit
#define PROXY_TBL_OPT_EXT_LEN                             0x01
#define PROXY_TBL_OPT_EXT_BIT                             0x07

/*************************************************************
* Proxy Table Entry Options Bitfield
**************************************************************/
#define PROXY_TBL_SEC_OPT_SECURITY_LVL_LEN                    0x02
#define PROXY_TBL_SEC_OPT_SECURITY_LVL_BIT                    0x00

#define PROXY_TBL_SEC_OPT_SECURITY_KEY_TYP_LEN                0x03
#define PROXY_TBL_SEC_OPT_SECURITY_KEY_TYP_BIT                0x02

/*************************************************************
* Pairing bit fields
**************************************************************/
// Application ID bitfied
#define GP_OPT_APP_ID_LEN                                 0x03 // length of bitfield
#define GP_OPT_APP_ID_BIT                                 0x00
// Values
#define GP_OPT_APP_ID_GPD                                 0x00
#define GP_OPT_APP_ID_IEEE                                0x02

// Request type bitfied
#define GP_OPT_REQ_TYP_LEN                                0x02 // length of bitfield
#define GP_OPT_REQ_TYP_BIT                                0x03
// Values
#define GP_OPT_REQ_TYP_GPD_ID                             0x00
#define GP_OPT_REQ_TYP_INDEX                              0x01

// Add Sink bitfield
#define GP_OPT_ADD_SINK_BIT_FIELD_LEN                     0x01
#define GP_OPT_ADD_SINK_BIT                               0x03
// Values
#define GP_OPT_ADD_SINK_REMOVE_PAIRING                    0x00
#define GP_OPT_ADD_SINK_ADD_PAIRING                       0x01
       
// Remove GPD bitfield
#define GP_OPT_REMOVE_GPD_FIELD_LEN                       0x01
#define GP_OPT_REMOVE_GPD_BIT                             0x04
// Values   
#define GP_OPT_GPD_NO_REMOVE                              0x00
#define GP_OPT_GPD_REMOVE                                 0x01

// Communication Mode bitfield
#define GP_OPT_COMMUNICATION_MODE_FIELD_LEN               0x02
#define GP_OPT_COMMUNICATION_MODE_BIT                     0x05
// Values
#define GP_OPT_COMMUNICATION_MODE_FULL_UNICAST            0x00
#define GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID       0x01
#define GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID        0x02
#define GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST           0x03

// GPD Fixed
#define GP_OPT_GPD_FIXED_LEN                              0x01
#define GP_OPT_GPD_FIXED                                  0x07

// GPD MAC sequence capablities
#define GP_OPT_GPD_MAC_SEC_CAP_LEN                        0x01
#define GP_OPT_GPD_MAC_SEC_CAP                            0x08

// Security Level
#define GP_OPT_SEC_LEVEL_LEN                              0x02
#define GP_OPT_SEC_LEVEL                                  0x09

// Security Key Type
#define GP_OPT_SEC_KEY_TYPE_LEN                           0x03
#define GP_OPT_SEC_KEY_TYPE                               0x0B

// GPD Security Frame Counter Present
#define GP_OPT_GPD_SEC_COUNTER_LEN                        0x01
#define GP_OPT_GPD_SEC_COUNTER                            0x0E

// GPD Security Key Present
#define GP_OPT_GPD_SEC_KEY_LEN                            0x01
#define GP_OPT_GPD_SEC_KEY                                0x0F

// GPD Assigned Alias Present
#define GP_OPT_ALIAS_LEN                                  0x01
#define GP_OPT_ALIAS                                      0x10

// GPD Forwarding Radius Present
#define GP_OPT_FORWARDING_RADIUS_LEN                      0x01
#define GP_OPT_FORWARDING_RADIUS                          0x11

/*************************************************************
* Security Related Parameters
**************************************************************/
// Security Level
#define SEC_OPT_SEC_LEVEL_LEN                             0x02
#define SEC_OPT_SEC_LEVEL                                 0x00

// Security Key Type
#define SEC_OPT_SEC_KEY_TYPE_LEN                          0x03
#define SEC_OPT_SEC_KEY_TYPE                              0x02
       
/*************************************************************
* Gp Notification options bit field
**************************************************************/
#define GP_NTF_OPT_ALSO_UNICAST_LEN                       0x01
#define GP_NTF_OPT_ALSO_UNICAST                           0x03

#define GP_NTF_OPT_ALSO_DGROUP_LEN                        0x01
#define GP_NTF_OPT_ALSO_DGROUP                            0x04

#define GP_NTF_OPT_ALSO_CGROUP_LEN                        0x01
#define GP_NTF_OPT_ALSO_CGROUP                            0x05

// Security Level
#define GP_NTF_OPT_SEC_LEVEL_LEN                          0x02
#define GP_NTF_OPT_SEC_LEVEL                              0x06

// Security Key Type
#define GP_NTF_OPT_SEC_KEY_TYPE_LEN                       0x03
#define GP_NTF_OPT_SEC_KEY_TYPE                           0x00

#define GP_NTF_OPT_RX_AFTER_TX_LEN                        0x01
#define GP_NTF_OPT_RX_AFTER_TX                            0x03

#define GP_NTF_OPT_TX_QUEUE_FULL_LEN                      0x01
#define GP_NTF_OPT_TX_QUEUE_FULL                          0x04

#define GP_NTF_OPT_BIDIRECTIONAL_CAP_LEN                  0x01
#define GP_NTF_OPT_BIDIRECTIONAL_CAP                      0x05

#define GP_NTF_OPT_PROXY_INFO_LEN                         0x01
#define GP_NTF_OPT_PROXY_INFO                             0x06

/*************************************************************
* Gp Commissioning Notification options bit field
**************************************************************/

// Security Level
#define GP_CNTF_OPT_SEC_LEVEL_LEN                         0x02
#define GP_CNTF_OPT_SEC_LEVEL                             0x04

// Security Key Type
#define GP_CNTF_OPT_SEC_KEY_TYPE_LEN                      0x03
#define GP_CNTF_OPT_SEC_KEY_TYPE                          0x06

#define GP_CNTF_OPT_SEC_FAIL_LEN                          0x01
#define GP_CNTF_OPT_SEC_FAIL                              0x01

#define GP_CNTF_OPT_BIDIRECTIONAL_CAP_LEN                 0x01
#define GP_CNTF_OPT_BIDIRECTIONAL_CAP                     0x02

#define GP_CNTF_OPT_PROXY_INFO_LEN                        0x01
#define GP_CNTF_OPT_PROXY_INFO                            0x03

/*********************************************************************
 * TYPEDEFS
 */

typedef struct gpNotificationMsg
{
  afAddrType_t                 addr;
  uint8                        secNum;
  uint8*                       pMsg;
  struct gpNotificationMsg*    pNext;
}gpNotificationMsg_t;

typedef struct gpCmdPayloadMsg
{
  uint8                        secNum;
  uint8                        lenght;
  uint8*                       pMsg;
  struct gpCmdPayloadMsg*      pNext;
}gpCmdPayloadMsg_t;

// Format of entries in the Proxy Table attribute
typedef struct
{
  uint16         options;
  uint32         gpdId;
  uint8          endPoint;
  uint16         gpdAssignedAlias;
  uint8          securityOptions;
  uint32         gpdSecurityFramecounter;
  uint8          gpdKey[SEC_KEY_LEN];
  uint8         *pLightweightSinkAddressList;
  uint8         *pSinkGroupList;
  uint8          groupcastRadius;
  uint8          SearchCounter;
  uint16         ExtendedOptions;
  uint8         *pFullUnicastSinkAddressList;
} ProxyTableEntryFormat_t;

// The format of entries in the gppBlockedGPDID attribute
typedef struct
{
  uint16         options;
  uint32         gpdId;
  uint8          endPoint;
  uint8          sequenceNumber;
  uint8          SearchCounter;
} GppBlockedGPDIDFormat_t;


/*** Structures used for callback functions ***/
typedef struct
{
  uint8          options;
  uint8         *pData;
} zclGpNotificationResponse_t;

typedef struct
{
  afAddrType_t  *srcAddr; // requestor's address
  uint8         options[3];
  uint8         *pData;
} zclGpPairing_t;

typedef struct
{
  uint16         srcAddr;
  uint8          options;
  uint8         *pData;
} zclGpProxyCommissioningMode_t;

typedef struct
{
  uint16         dstAddr;
  uint8          options;
  uint16         tempMasterShortAddr;
  uint8          tempMasterTxChannel;
  uint8         *pData;
} zclGpResponse_t;

typedef struct
{
  uint8          status;
  uint8          options;
  uint8          totalNumberOfEntries;
  uint8          startIndex;
  uint8          entriesCount;
  uint8         *pTranslationTableList;
} zclGpTranslationTableResponse_t;

typedef struct
{
  afAddrType_t *srcAddr; // requestor's address
  uint8        options;
  uint8        *pData;
} zclGpProxyTableRequest_t;

typedef struct
{
  uint8          status;
  uint8          tableEntriesTotal;
  uint8          startIndex;
  uint8          entriesCount;
  uint8         *proxyTableEntry;
} zclGpProxyTableResponse_t;

typedef struct
{
  uint32 options;
  uint32 gpdId;
  uint8  gpdIEEE[Z_EXTADDR_LEN];
  uint8  ep;
  uint8  sinkIEEE[Z_EXTADDR_LEN];
  uint16 sinkNwkAddr;
  uint16 sinkGroupID;
  uint8  deviceId;
  uint32 gpdSecCounter;
  uint8  gpdKey[SEC_KEY_LEN];
  uint16 assignedAlias;
  uint8  forwardingRadius;
} gpPairingCmd_t;

typedef struct
{
  uint8 options;
  uint32 gpdId;
  uint8  gpdIEEE[Z_EXTADDR_LEN];
  uint8  ep;
  uint8  index;
} gpProxyTableReqCmd_t;

typedef struct
{
  uint16 options;
  uint32 gpdId;
  uint8  gpdIEEE[Z_EXTADDR_LEN];
  uint8  ep;
  uint32 gpdSecCounter;
  uint8  cmdId;
  uint8  payloadLen;
  uint8  *cmdPayload;
  uint16 gppShortAddr;
  uint8  gppGpdLink;
} gpNotificationCmd_t;

typedef struct
{
  uint16 options;
  uint32 gpdId;
  uint8  gpdIEEE[Z_EXTADDR_LEN];
  uint8  ep;
  uint32 gpdSecCounter;
  uint8  cmdId;
  uint8  payloadLen;
  uint8  *cmdPayload;
  uint16 gppShortAddr;
  uint8 gppGpdLink;
  uint32 mic;
} gpCommissioningNotificationCmd_t;

// From sink to a proxy to acknowledge GP Notification received in full unicast mode
typedef void (*zclGCB_GP_Notification_Response_t)( zclGpNotificationResponse_t *pCmd );

// From sink to proxies to (de)register for tunneling service or to remove GPD from the network
typedef void (*zclGCB_GP_Pairing_t)( zclGpPairing_t *pCmd );

// From sink to proxies in the whole network to indicate commissioning mode
typedef void (*zclGCB_GP_Proxy_Commissioning_Mode_t)( zclGpProxyCommissioningMode_t *pCmd );

// From sink to selected proxies, to provide data to be transmitted to Rx-capable GPD
typedef void (*zclGCB_GP_Response_t)( zclGpResponse_t *pCmd );

// To receive information on requested selected Sink Table entries, by index or by GPD ID
typedef ZStatus_t (*zclGCB_GP_Translation_Table_Response_t)( zclGpTranslationTableResponse_t *pCmd );

// To request selected Proxy Table entries, by index or by GPD ID
typedef void (*zclGCB_GP_Proxy_Table_Request_t)( zclGpProxyTableRequest_t *pRsp );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclGCB_GP_Pairing_t                  pfnGpPairingCmd;
  zclGCB_GP_Proxy_Table_Request_t      pfnGpProxyTableReqCmd;
  zclGCB_GP_Proxy_Commissioning_Mode_t pfnGpProxyCommissioningModeCmd;
  zclGCB_GP_Response_t                 pfnGpResponseCommand;
} zclGp_AppCallbacks_t;


/*********************************************************************
 * FUNCTION MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclGp_RegisterCmdCallbacks( uint8 endpoint, zclGp_AppCallbacks_t *callbacks );

/*
 * Send GP Notification command from proxy to sink to tunnel GP frame. 
 */
extern ZStatus_t zclGp_Notification( afAddrType_t *dstAddr,
                                     uint16 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Pairing Search command from proxy to the sinks in entire network to 
 * get pairing indication related to GPD for Proxy Table update
 */
extern ZStatus_t zclGp_PairingSearch( afAddrType_t *dstAddr,
                                      uint16 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Tunneling Stop command from proxy to neighbor proxies to indicate 
 * GP Notification sent in full unicast mode
 */
extern ZStatus_t zclGp_TunnelingStop( afAddrType_t *dstAddr,
                                      uint8 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Commissioning Notification command from proxy to sink to tunnel 
 * GPD commissioning data.
 */
extern ZStatus_t zclGp_CommissioningNotification( afAddrType_t *dstAddr,
                                      uint16 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Sink Commissioning Mode command to enable commissioning mode of the 
 * sink, over the air
 */
extern ZStatus_t zclGp_SinkCommissioningMode( afAddrType_t *dstAddr, uint8 options, 
                                      uint16 gmpAddress, uint8 sinkEp, uint8 seqNum );

/*
 * Send GP Translation Table Update command to configure GPD Command Translation Table
 */
extern ZStatus_t zclGp_TranslationTableUpdate( afAddrType_t *dstAddr,
                                      uint16 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Translation Table Request command to provide GPD Command Translation 
 * Table content
 */
extern ZStatus_t zclGp_TranslationTableRequest( afAddrType_t *dstAddr,
                                      uint16 options, uint8 index, uint8 seqNum );

/*
 * Send GP Pairing Configuration command to configure Sink Table  
 */
extern ZStatus_t zclGp_PairingConfiguration( afAddrType_t *dstAddr, uint8 actions,
                                      uint16 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Sink Table Request command to read out selected Sink Table entries, 
 * by index or by GPD ID 
 */
extern ZStatus_t zclGp_SinkTableRequest( afAddrType_t *dstAddr,
                                      uint8 options, uint8 *pCmd, uint8 seqNum );

/*
 * Send GP Sink Table Request command to receive information on requested selected 
 * Proxy Table entries, by index or by GPD ID 
 */
extern ZStatus_t zclGp_SendGpProxyTableResponse( afAddrType_t *dstAddr, zclGpProxyTableResponse_t *rsp, 
                                          uint8 seqNum );

/*
 * @brief   Send the Green Power Notification Command to a device
 */
extern ZStatus_t zclGp_SendGpNotificationCommand( gpNotificationCmd_t *cmd, uint8 secNum );

/*
 * @brief   Send the Green Power Commissioning Notification Command to a device
 */
extern ZStatus_t zclGp_SendGpCommissioningNotificationCommand( gpCommissioningNotificationCmd_t *pCmd );

/*
 * @brief   Create Notification Msg List for paired sinks if empty
 */
void gp_CreateNotificationMsgList( gpNotificationMsg_t **pHead );

/*
 * @brief   Create Notification Msg List for paired sinks if empty
 */
void gp_CreateCmdPayloadMsgList( gpCmdPayloadMsg_t **pHead );

/*
 * @brief   Add node to Notification Msg list
 */
gpNotificationMsg_t* gp_AddNotificationMsgNode( gpNotificationMsg_t **pHead, gpCmdPayloadMsg_t *pMsg );

/*
 * @brief   Add node to Notification Msg list
 */
gpCmdPayloadMsg_t* gp_AddCmdPayloadMsgNode( gpCmdPayloadMsg_t **pHead, uint8* pBuf, uint8 len );

/*
 * @brief   Returns head pointer for  finding and binding respondent list
 */
gpNotificationMsg_t* gp_GetHeadNotificationMsg(void);

/*
 * @brief   Returns head pointer for  finding and binding respondent list
 */
gpNotificationMsg_t** gp_GetPHeadNotification(void);

/*
 * @brief   Returns head pointer for  finding and binding respondent list
 */
gpCmdPayloadMsg_t* gp_GetHeadCmdPayloadMsg(void);

/*
 * @brief   Returns head pointer for  finding and binding respondent list
 */
gpCmdPayloadMsg_t** gp_GetPHeadCmdPayload(void);

/*
 * @brief   This function free reserved memory for respondent list
 */
void gp_NotificationMsgClean( gpNotificationMsg_t **pHead );

/*
 * @brief   This function free reserved memory for respondent list
 */
void gp_CmdPayloadMsgClean( gpCmdPayloadMsg_t **pHead );

 /*
 * @brief   General function to get proxy table entry by NV index     
 */
uint8 gp_getProxyTableByIndex( uint16 nvIndex, uint8 *pEntry );

/*
 * @brief   General function to get proxy table entry by gpd_ID (GP Src ID or Extended Adddress)    
 */
uint8 gp_getProxyTableByGpId(gpd_ID_t *gpd_ID, uint8 *pEntry, uint16* NvProxyTableIndex);

/*
 * @brief   This function removes data of the given entry
 */
void gp_ResetProxyBasicTblEntry( uint8* entry );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_GREEN_POWER_H */
