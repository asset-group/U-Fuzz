/**************************************************************************************************
  Filename:       zcl_gp.h
  Revised:        $Date: 2014-05-30 15:04:07 -0700 (Fri, 30 May 2014) $
  Revision:       $Revision: 38732 $

  Description:    This file contains the ZCL Green Power definitions.


  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZCL_GP_H
#define ZCL_GP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define ZCL_GATEWAY_PROFILE_ID    0xA1E0  // @todo CERT-UPDATE: remove. Re-use existing definition of Gateway Management Cluster Profile ID.

// Fixed Zigbee Green Power Endpoint for Proxy and Target
#define ZGP_ENDPOINT_ID                                               0x00F2

// ZGP Infrastructure Device IDs
#define ZGP_DEVICE_ID_PROXY                                           0x0060
#define ZGP_DEVICE_ID_PROXY_MINIMUN                                   0x0061
#define ZGP_DEVICE_ID_TARGET_PLUS                                     0x0062
#define ZGP_DEVICE_ID_TARGET                                          0x0063
#define ZGP_DEVICE_ID_COMMISSIONING_TOOL                              0x0064
#define ZGP_DEVICE_ID_COMBO                                           0x0065
#define ZGP_DEVICE_ID_COMBO_MINIMUM                                   0x0066

/**********************************************/
/*** Green Power Features                   ***/
/**********************************************
 *
 * Use one of the following compilation flags
 * to determine the device type to be created:
 *   ZGP_DEVICE_PROXY
 *   ZGP_DEVICE_TARGET
 *   ZGP_DEVICE_TARGETPLUS
 *   ZGP_DEVICE_COMBO
 *   ZGP_DEVICE_PROXY_MIN
 *   ZGP_DEVICE_COMBO_MIN
 *   ZGP_DEVICE_COMMISSIONING_TOOL
 *
 * Comment (or uncomment) the supported features
 * and supported commands bitmasks added to each
 * device in order to control its capabilities.
 *
 **********************************************/

#if !defined ( ZGP_DEVICE_ID )
  #if defined( ZGP_DEVICE_TARGET )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_TARGET
  #elif defined( ZGP_DEVICE_TARGETPLUS )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_TARGET_PLUS
  #elif defined( ZGP_DEVICE_PROXY )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_PROXY
  #elif defined( ZGP_DEVICE_COMBO )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_COMBO
  #elif defined( ZGP_DEVICE_PROXY_MIN )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_PROXY_MINIMUN
  #elif defined( ZGP_DEVICE_COMBO_MIN )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_COMBO_MINIMUM
  #elif defined( ZGP_DEVICE_COMMISSIONING_TOOL )
    #define ZGP_DEVICE_ID  ZGP_DEVICE_ID_COMMISSIONING_TOOL
  #else
    #error ZGP Device type is not defined
  #endif
#endif

#define SUPP_ZGP_FEATURE_COMMON                    0x000001
#define SUPP_ZGP_FEATURE_DIRECT                    0x000002
#define SUPP_ZGP_FEATURE_DERIVED_GROUPCAST         0x000004
#define SUPP_ZGP_FEATURE_PRECOMMISSIONED           0x000008
#define SUPP_ZGP_FEATURE_UNICAST                   0x000010
#define SUPP_ZGP_FEATURE_LIGHTWEIGHT_UNICAST       0x000020
#define SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_OPER     0x000040
#define SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER      0x000080
#define SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT         0x000100
#define SUPP_ZGP_FEATURE_SINGLE_HOP_COMMISS        0x000200
#define SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS         0x000400   // 0x0800
//#define SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_COMMISS   0x1000
#define SUPP_ZGP_FEATURE_CT_BASED_COMMISS          0x000800   // 0x2000
#define SUPP_ZGP_FEATURE_MAINT_ZGPD                0x001000   // 0x4000
#define SUPP_ZGP_FEATURE_SECURITY_LEVEL_NONE       0x002000
#define SUPP_ZGP_FEATURE_SECURITY_LEVEL_SHORT      0x004000
#define SUPP_ZGP_FEATURE_SECURITY_LEVEL_LONG       0x008000
#define SUPP_ZGP_FEATURE_SECURITY_LEVEL_FULL       0x010000
#define SUPP_ZGP_FEATURE_SINK_FORWARDING           0x020000
#define SUPP_ZGP_FEATURE_TRANSLATION_TABLE         0x040000

#define SUPP_ZGP_CMD_NOTIFICAITON                  0x0001
#define SUPP_ZGP_CMD_TUNNELING_STOP                0x0002
#define SUPP_ZGP_CMD_PAIRING_SEARCH                0x0004
#define SUPP_ZGP_CMD_NOTIFICATION_RESP             0x0008
#define SUPP_ZGP_CMD_PAIRING                       0x0010
#define SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE      0x0020
#define SUPP_ZGP_CMD_COMMISSIONING_NOTIF           0x0040
#define SUPP_ZGP_CMD_RESPONSE                      0x0080
#define SUPP_ZGP_CMD_TT_UPDATE                     0x0100
#define SUPP_ZGP_CMD_TT_REQ                        0x0200
#define SUPP_ZGP_CMD_TT_RESP                       0x0400
#define SUPP_ZGP_CMD_CONFIGURE_PAIRING             0x0800

#define SUPP_ZGP_SECURITY_NONE      0x0
#define SUPP_ZGP_SECURITY_SHORT     0x1
#define SUPP_ZGP_SECURITY_LONG      0x2
#define SUPP_ZGP_SECURITY_FULL      0x4

#if defined ( ZGP_DEVICE_PROXY ) || defined ( ZGP_DEVICE_COMBO )
  #define ZGP_DEVICE_SUPPORTED_CLIENT_FEATURE         ( \
              SUPP_ZGP_FEATURE_COMMON                   \
            + SUPP_ZGP_FEATURE_DIRECT                   \
            + SUPP_ZGP_FEATURE_DERIVED_GROUPCAST        \
            + SUPP_ZGP_FEATURE_PRECOMMISSIONED          \
            + SUPP_ZGP_FEATURE_UNICAST                  \
            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER     \
            + SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT        \
            + SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS        \
            + SUPP_ZGP_FEATURE_CT_BASED_COMMISS         \
            + SUPP_ZGP_FEATURE_MAINT_ZGPD             )
/*            + SUPP_ZGP_FEATURE_LIGHTWEIGHT_UNICAST      \*/
/*            + SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_COMMISS \*/
#endif

#if defined ( ZGP_DEVICE_PROXY_MIN )
  #define ZGP_DEVICE_SUPPORTED_CLIENT_FEATURE           \
            ( SUPP_ZGP_FEATURE_COMMON                   \
            + SUPP_ZGP_FEATURE_DIRECT                   \
            + SUPP_ZGP_FEATURE_DERIVED_GROUPCAST        \
            + SUPP_ZGP_FEATURE_PRECOMMISSIONED          \
/*            + SUPP_ZGP_FEATURE_UNICAST                  \
            + SUPP_ZGP_FEATURE_LIGHTWEIGHT_UNICAST      \
            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER     \
            + SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT        \
            + SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS        \
            + SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_COMMISS \
            + SUPP_ZGP_FEATURE_CT_BASED_COMMISS         \
            + SUPP_ZGP_FEATURE_MAINT_ZGPD            */ )
#endif

#if defined ( ZGP_DEVICE_COMBO_MIN )
////  #define ZGP_DEVICE_SUPPORTED_CLIENT_FEATURE           \
////            ( SUPP_ZGP_FEATURE_COMMON                   \
////            + SUPP_ZGP_FEATURE_DIRECT                   \
////            + SUPP_ZGP_FEATURE_DERIVED_GROUPCAST        \
////            + SUPP_ZGP_FEATURE_PRECOMMISSIONED          \
/////*            + SUPP_ZGP_FEATURE_UNICAST                  \
////            + SUPP_ZGP_FEATURE_LIGHTWEIGHT_UNICAST      \
////            + SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_COMMISS \
////            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER     \
////            + SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT        \
////            + SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS        \
////            + SUPP_ZGP_FEATURE_CT_BASED_COMMISS         \
////            + SUPP_ZGP_FEATURE_MAINT_ZGPD               \
////            */ )
#endif

#if defined ( ZGP_DEVICE_TARGET )
  #define ZGP_DEVICE_SUPPORTED_SERVER_FEATURE         ( \
              SUPP_ZGP_FEATURE_COMMON                   \
            + SUPP_ZGP_FEATURE_DERIVED_GROUPCAST        \
            + SUPP_ZGP_FEATURE_PRECOMMISSIONED          \
            + SUPP_ZGP_FEATURE_UNICAST                  \
            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER     \
            + SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT        \
            + SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS        \
            + SUPP_ZGP_FEATURE_CT_BASED_COMMISS         \
            + SUPP_ZGP_FEATURE_MAINT_ZGPD               \
            + SUPP_ZGP_FEATURE_TRANSLATION_TABLE )
/*          + SUPP_ZGP_FEATURE_LIGHTWEIGHT_UNICAST      \
            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_COMMISS  \*/
#endif

#if defined ( ZGP_DEVICE_TARGETPLUS ) || defined ( ZGP_DEVICE_COMBO )
  #define ZGP_DEVICE_SUPPORTED_SERVER_FEATURE         ( \
              SUPP_ZGP_FEATURE_COMMON                   \
            + SUPP_ZGP_FEATURE_DIRECT                   \
            + SUPP_ZGP_FEATURE_DERIVED_GROUPCAST        \
            + SUPP_ZGP_FEATURE_PRECOMMISSIONED          \
            + SUPP_ZGP_FEATURE_UNICAST                  \
            + SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_OPER    \
            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER     \
            + SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT        \
            + SUPP_ZGP_FEATURE_SINGLE_HOP_COMMISS       \
            + SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS        \
            + SUPP_ZGP_FEATURE_CT_BASED_COMMISS         \
            + SUPP_ZGP_FEATURE_MAINT_ZGPD               \
            + SUPP_ZGP_FEATURE_TRANSLATION_TABLE )
#endif

#if defined ( ZGP_DEVICE_COMBO_MIN )
  #define ZGP_DEVICE_SUPPORTED_SERVER_FEATURE         ( \
              SUPP_ZGP_FEATURE_COMMON                   \
            + SUPP_ZGP_FEATURE_DIRECT                   \
            + SUPP_ZGP_FEATURE_DERIVED_GROUPCAST        \
            + SUPP_ZGP_FEATURE_PRECOMMISSIONED          \
            + SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_OPER    \
            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER     \
            + SUPP_ZGP_FEATURE_PROXY_TABLE_MAINT        \
            + SUPP_ZGP_FEATURE_SINGLE_HOP_COMMISS       \
            + SUPP_ZGP_FEATURE_MULTI_HOP_COMMISS        \
            + SUPP_ZGP_FEATURE_CT_BASED_COMMISS         \
            + SUPP_ZGP_FEATURE_MAINT_ZGPD               \
            + SUPP_ZGP_FEATURE_TRANSLATION_TABLE        \
            + SUPP_ZGP_FEATURE_SINK_FORWARDING )

/*            + SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_COMMISS  \*/
/*            + SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_COMMISS \*/
#endif

#if defined ( ZGP_DEVICE_PROXY ) || defined ( ZGP_DEVICE_COMBO )
  #define ZGP_DEVICE_SUPPORTED_CLIENT_TX_CMD      ( \
              SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_TUNNELING_STOP           \
            + SUPP_ZGP_CMD_PAIRING_SEARCH           \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF    )

  #define ZGP_DEVICE_SUPPORTED_CLIENT_RX_CMD      ( \
              SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_TUNNELING_STOP           \
            + SUPP_ZGP_CMD_PAIRING_SEARCH           \
            + SUPP_ZGP_CMD_NOTIFICATION_RESP        \
            + SUPP_ZGP_CMD_PAIRING                  \
            + SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF      \
            + SUPP_ZGP_CMD_RESPONSE               )
#endif

#if defined ( ZGP_DEVICE_PROXY_MIN )
  #define ZGP_DEVICE_SUPPORTED_CLIENT_TX_CMD ( \
            SUPP_ZGP_CMD_NOTIFICAITON          \
/*        + SUPP_ZGP_CMD_TUNNELING_STOP        \
          + SUPP_ZGP_CMD_PAIRING_SEARCH        \
          + SUPP_ZGP_CMD_COMMISSIONING_NOTIF */ )

  #define ZGP_DEVICE_SUPPORTED_CLIENT_RX_CMD      ( \
              SUPP_ZGP_CMD_PAIRING                  \
/*          + SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_TUNNELING_STOP           \
            + SUPP_ZGP_CMD_PAIRING_SEARCH           \
            + SUPP_ZGP_CMD_NOTIFICATION_RESP        \
            + SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF      \
            + SUPP_ZGP_CMD_RESPONSE              */ )
#endif

#if defined ( ZGP_DEVICE_COMBO_MIN )
  #define ZGP_DEVICE_SUPPORTED_CLIENT_TX_CMD ( \
            SUPP_ZGP_CMD_NOTIFICAITON          \
/*        + SUPP_ZGP_CMD_TUNNELING_STOP        \
          + SUPP_ZGP_CMD_PAIRING_SEARCH        \
          + SUPP_ZGP_CMD_COMMISSIONING_NOTIF */ )

  #define ZGP_DEVICE_SUPPORTED_CLIENT_RX_CMD      ( \
             SUPP_ZGP_CMD_RESPONSE                  \
/*          + SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_TUNNELING_STOP           \
            + SUPP_ZGP_CMD_PAIRING_SEARCH           \
            + SUPP_ZGP_CMD_NOTIFICATION_RESP        \
            + SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF      \
            + SUPP_ZGP_CMD_RESPONSE              */ )
#endif

#if defined ( ZGP_DEVICE_TARGET ) || defined ( ZGP_DEVICE_TARGETPLUS ) || defined ( ZGP_DEVICE_COMBO )
  #define ZGP_DEVICE_SUPPORTED_SERVER_TX_CMD      ( \
              SUPP_ZGP_CMD_NOTIFICATION_RESP        \
            + SUPP_ZGP_CMD_PAIRING                  \
            + SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE \
            + SUPP_ZGP_CMD_RESPONSE                 \
            + SUPP_ZGP_CMD_TT_RESP                  \
            + SUPP_ZGP_CMD_CONFIGURE_PAIRING      )

  #define ZGP_DEVICE_SUPPORTED_SERVER_RX_CMD      ( \
            + SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_PAIRING_SEARCH           \
            + SUPP_ZGP_CMD_PAIRING                  \
            + SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF      \
            + SUPP_ZGP_CMD_TT_UPDATE                \
            + SUPP_ZGP_CMD_TT_REQ                   \
            + SUPP_ZGP_CMD_CONFIGURE_PAIRING       )

#endif

#if defined ( ZGP_DEVICE_COMBO_MIN )
  #define ZGP_DEVICE_SUPPORTED_SERVER_TX_CMD      ( \
            + SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_PAIRING                  \
            + SUPP_ZGP_CMD_PROXY_COMMISSIONING_MODE \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF      \
            + SUPP_ZGP_CMD_RESPONSE                 \
            + SUPP_ZGP_CMD_TT_UPDATE                \
            + SUPP_ZGP_CMD_TT_REQ                   \
            + SUPP_ZGP_CMD_TT_RESP                  \
            + SUPP_ZGP_CMD_CONFIGURE_PAIRING       )

  #define ZGP_DEVICE_SUPPORTED_SERVER_RX_CMD      ( \
            + SUPP_ZGP_CMD_NOTIFICAITON             \
            + SUPP_ZGP_CMD_PAIRING_SEARCH           \
            + SUPP_ZGP_CMD_COMMISSIONING_NOTIF      \
            + SUPP_ZGP_CMD_TT_UPDATE                \
            + SUPP_ZGP_CMD_TT_REQ                   \
            + SUPP_ZGP_CMD_TT_RESP                  \
            + SUPP_ZGP_CMD_CONFIGURE_PAIRING       )
#endif

#if defined ( ZGP_DEVICE_PROXY_MIN )
  #define ZGP_DEVICE_SUPPORTED_SECURITY         ( SUPP_ZGP_SECURITY_LONG )
#else
  #define ZGP_DEVICE_SUPPORTED_SECURITY         ( SUPP_ZGP_SECURITY_SHORT \
                                                + SUPP_ZGP_SECURITY_LONG  \
                                                + SUPP_ZGP_SECURITY_FULL )
#endif

#ifndef ZGP_DEVICE_SUPPORTED_CLIENT_TX_CMD
  #define ZGP_DEVICE_SUPPORTED_CLIENT_TX_CMD 0x0000
#endif
#ifndef ZGP_DEVICE_SUPPORTED_CLIENT_RX_CMD
  #define ZGP_DEVICE_SUPPORTED_CLIENT_RX_CMD 0x0000
#endif
#ifndef ZGP_DEVICE_SUPPORTED_SERVER_TX_CMD
  #define ZGP_DEVICE_SUPPORTED_SERVER_TX_CMD 0x0000
#endif
#ifndef ZGP_DEVICE_SUPPORTED_SERVER_RX_CMD
  #define ZGP_DEVICE_SUPPORTED_SERVER_RX_CMD 0x0000
#endif

#ifndef ZGP_DEVICE_SUPPORTED_CLIENT_FEATURE
  #define ZGP_DEVICE_SUPPORTED_CLIENT_FEATURE 0x0000
#endif
#ifndef ZGP_DEVICE_SUPPORTED_SERVER_FEATURE
  #define ZGP_DEVICE_SUPPORTED_SERVER_FEATURE 0x0000
#endif

#ifndef ZGP_STUB_SECURITY
  #define ZGP_STUB_SECURITY ( SUPP_ZGP_SECURITY_NONE )
#endif

#define SUPPORTED_C_FEATURE(x) ( ZGP_DEVICE_SUPPORTED_CLIENT_FEATURE & (x) )
#define SUPPORTED_S_FEATURE(x) ( ZGP_DEVICE_SUPPORTED_SERVER_FEATURE & (x) )
#define SUPPORTED_FEATURE(x) ( SUPPORTED_C_FEATURE(x) || SUPPORTED_S_FEATURE(x)  )
#define SUPPORTED_C_RX_CMD(x) ( ZGP_DEVICE_SUPPORTED_CLIENT_RX_CMD & (x) )
#define SUPPORTED_C_TX_CMD(x) ( ZGP_DEVICE_SUPPORTED_CLIENT_TX_CMD & (x) )
#define SUPPORTED_S_RX_CMD(x) ( ZGP_DEVICE_SUPPORTED_SERVER_RX_CMD & (x) )
#define SUPPORTED_S_TX_CMD(x) ( ZGP_DEVICE_SUPPORTED_SERVER_TX_CMD & (x) )
#define SUPPORTED_RX_CMD(x) ( SUPPORTED_C_RX_CMD(x) || SUPPORTED_S_RX_CMD(x) )
#define SUPPORTED_TX_CMD(x) ( SUPPORTED_C_TX_CMD(x) || SUPPORTED_S_TX_CMD(x) )

#define SUPPORTED_FEATURE_BIDIR ( SUPPORTED_FEATURE(SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_OPER)     \
                                || SUPPORTED_FEATURE(SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_OPER)     )
/*                                || SUPPORTED_FEATURE(SUPP_ZGP_FEATURE_SINGLE_HOP_BIDIR_COMMISS) \*/
/*                                || SUPPORTED_FEATURE(SUPP_ZGP_FEATURE_MULTI_HOP_BIDIR_COMMISS) ) */

#define SUPPORTED_ZGP_SECURITY_LEVEL(x) ( ZGP_DEVICE_SUPPORTED_SECURITY & (x) )
#define SUPPORTED_ZGP_SECURITY ( ZGP_DEVICE_SUPPORTED_SECURITY > SUPP_ZGP_SECURITY_NONE )

/**********************************************/
/*** Green Power Attributes                 ***/
/**********************************************/
// Server Attributes
#define ATTRID_GP_MAX_GPTT_ENTRIES                                    0x0000
#define ATTRID_GP_SINK_TABLE                                          0x0001
#define ATTRID_GP_COMM_MODE                                           0x0002
#define ATTRID_GP_COMM_EXIT_MODE                                      0x0003
#define ATTRID_GP_COMM_WINDOW                                         0x0004
#define ATTRID_GP_SECURITY_LEVEL                                      0x0005
#define ATTRID_GP_SINK_FEATURES                                       0x0006
#define ATTRID_GP_SINK_ACTIVE_FEATURES                                0x0007

#define ATTRID_GP_SHARED_SECURITY_KEYTYPE                             0x0020
#define ATTRID_GP_SHARED_SECURITY_KEY                                 0x0021
#define ATTRID_GP_LINK_KEY                                            0x0022

// Client Attributes
#define ATTRID_GP_MAX_PROXY_ENTRIES                                   0x0010
#define ATTRID_GP_PROXY_TABLE                                         0x0011
#define ATTRID_GP_NOTIFICATION_RETRY_NUMBER                           0x0012
#define ATTRID_GP_NOTIFICATION_RETRY_TIMER                            0x0013
#define ATTRID_GP_MAX_SEARCH_COUNTER                                  0x0014
#define ATTRID_GP_BLOCKED_SRC_ID                                      0x0015
#define ATTRID_GP_PROXY_FEATURES                                      0x0016
#define ATTRID_GP_PROXY_ACTIVE_FEATURES                               0x0017

// Server Commands
#define COMMAND_ZGP_NOTIFICATION                                      0x00
#define COMMAND_ZGP_PAIRING_SEARCH                                    0x01
#define COMMAND_ZGP_TUNNELING_STOP                                    0x03
#define COMMAND_ZGP_COMMISSIONING_NOTIFICATION                        0x04
#define COMMAND_ZGP_COMMISSIONING_SUCCESS                             0x05
#define COMMAND_ZGP_TRANSLATION_TABLE_UPDATE                          0x07
#define COMMAND_ZGP_TRANSLATION_TABLE_REQUEST                         0x08
#define COMMAND_ZGP_PAIRING_CONFIGURATION                             0x09

// Client Commands
#define COMMAND_ZGP_NOTIFICATION_RESPONSE                             0x00
#define COMMAND_ZGP_PAIRING                                           0x01
#define COMMAND_ZGP_PROXY_COMMISSIONING_MODE                          0x02
#define COMMAND_ZGP_RESPONSE                                          0x06
#define COMMAND_ZGP_TRANSLATION_TABLE_RESPONSE                        0x08

// Command Header Lengths
#define ZGP_CMDHDRLEN_NOTIFICATION                  11
#define ZGP_CMDHDRLEN_NOTIFICATION_EXTENDED         3
#define ZGP_CMDHDRLEN_COMMISSIONING_NOTIFICATION    11    // 10 // CERT-UPDATE - extend options on 2 bytes to support Application ID
#define ZGP_CMDHDRLEN_TRANSLATION_TABLE_UPDATE      7     // 6  // CERT-UPDATE - extend Options filed over 2 bytes
#define ZGP_CMD_TRANSLATION_TABLE_ENTRY_LEN         8     // 7  // CERT-UPDATE - add TTentryindex field
#define ZGP_CMD_TRANSLATION_TABLE_RSP_ENTRY_LEN     12
#define ZGP_CMDHDRLEN_CONFIGURE_PAIRING             10    // 9  // CERT-UPDATE - optBytes on 2 bytes to support Application ID
#define ZGP_CMDHDRLEN_PAIRING                       7     // 6  // CERT-UPDATE - extend options on 2 bytes to support Application ID
#define ZGP_CMDHDRLEN_RESPONSE                      9     // 8  // CERT-UPDATE - extend options on 2 bytes to support Application ID
#define ZGP_CMDHDRLEN_TRANSLATION_TABLE_RESPONSE    5     // 4  // CERT-UPDATE - extend options on 2 bytes to support Application ID

// Tunneling/Communication Modes
#define ZGP_TUNNEL_MODE_UNICAST                 0x00
#define ZGP_TUNNEL_MODE_GROUP_DGROUPID          0x01
#define ZGP_TUNNEL_MODE_GROUP_COMM_GROUPID      0x02
#define ZGP_TUNNEL_MODE_UNICAST_MINIMAL         0x03

// Security Key Types
#define ZGP_KEYTYPE_NONE                        0
#define ZGP_KEYTYPE_NWK                         1
#define ZGP_KEYTYPE_GROUP                       2
#define ZGP_KEYTYPE_NWK_DERIVED_GROUP           3
#define ZGP_KEYTYPE_OOBKEY                      4
#define ZGP_KEYTYPE_DERIVED_INDIVIDUAL          7

// Security Level Mode
#define ZGP_SECURITY_LEVEL_NONE                 0
#define ZGP_SECURITY_LEVEL_SHORT                1
#define ZGP_SECURITY_LEVEL_LONG                 2
#define ZGP_SECURITY_LEVEL_FULL                 3

#define ZGP_SECURITY_LEVEL_SHORT_MIC_SIZE       2
#define ZGP_SECURITY_LEVEL_STD_MIC_SIZE         4

// Exit Mode Attribute definitions (bitmap)
#define ZGP_EXIT_MODE_ON_COMMISSIONING_WINDOW_EXPIRATION    0x01
#define ZGP_EXIT_MODE_ON_FIRST_PAIRING_SUCCESS              0x02
#define ZGP_EXIT_MODE_ON_PROXY_COMMISSIONING_MODE           0x04

#define ZGP_TEMP_MASTER_CHANNEL_MASK                0x0F

// Radius field for groupcast communication
#define ZGP_GROUPCAST_RADIUS_DEFAULT                0xFF

// Translation Table NV definitions
#define ZGP_TRANSLATIONTABLE_ENTRY_MAX_ENDPOINTS       5
#define ZGP_TRANSLATIONTABLE_ENTRY_MAX_PAYLOAD_LEN     4

//Action fields for Pairing Configuration
#define ZGP_CONFPAIRING_ACTION_NONE             0x00
#define ZGP_CONFPAIRING_ACTION_EXTEND           0x01
#define ZGP_CONFPAIRING_ACTION_REPLACE          0x02
#define ZGP_CONFPAIRING_ACTION_REMOVE_PAIRING   0x03
#define ZGP_CONFPAIRING_ACTION_REMOVE_ZGPD      0x04

#if !defined (  ZGP_SINKTABLE_CGROUP_MAX_ENTRIES )
  #define ZGP_SINKTABLE_CGROUP_MAX_ENTRIES            2     // shall never get the 0xFF value, always lower !
#endif

//Action value for Translation Table update
#define ZGP_TTUPDATE_ACTION_ADD                 0x00
#define ZGP_TTUPDATE_ACTION_REPLACE             0x01
#define ZGP_TTUPDATE_ACTION_REMOVE              0x02

/*********************************************************************
 * TYPEDEFS
 */

// Sink Table Entry Options field
typedef struct
{
  unsigned int appliID:3;       // Application ID sub-field 0b000 for ZGP with SrcID, 0b001 for LPED, 0b010 for ZGP with IEEE Mac address
  unsigned int commMode:2;      // accepted tunneling mode for this SrcID. ie. ZGP_TUNNEL_MODE_UNICAST
  unsigned int seqNumCap:1;     //
  unsigned int rxOnCap:1;       // reception capability on this ZGPD.
  unsigned int fixedLoc:1;      // set if the location of this ZGPD is expected to change.
  unsigned int assignAliasUnicast:1;   // indicates that the assigned alias as stored in the ZGPD AssignedAlias field shall be used instead of the alias derived from SrcID.
  unsigned int secUse:1;        // Security related fields of the Sink Table entry are present
  unsigned int reserved:6;      // Reserved for future use
} gpSinkTableOptionsBits_t;

// ZGP Sink Table Options field bitmap
typedef union
{
  gpSinkTableOptionsBits_t optBits;
  uint16 optWord;
} gpSinkTableOptions_t;

// ZGP Sink Table Security Options field
typedef struct
{
  unsigned int level:2;         // security Level
  unsigned int keyType:3;       // Key Type
  unsigned int reserved:3;      // Reserved for future use
} gpSinkTableSecOptionsBits_t;

typedef union
{
  gpSinkTableSecOptionsBits_t optBits;
  uint8 optByte;
} gpSinkTableSecOptions_t;

typedef struct
{
  uint16 groupID;       // Group ID of list item
  uint16 alias;         // related alias of list item
} gpSinkGroupListItem_t;

// ZGP Sink Table Entry
typedef struct
{
  gpSinkTableOptions_t bitmap;      // Options field
  uint32 srcID;                     // srcID of the paired ZGPD
  uint8 deviceID;                   // The paired device's ID.
  uint8 numSinkGroups;              // The number of items in the Sink Group List
  gpSinkGroupListItem_t   groupList[ZGP_SINKTABLE_CGROUP_MAX_ENTRIES];    // Sink Table Sink Group List for CGroup  // @todo CERT-UPDATE: manage GroupList as for proxy Table and not fixed memory !
  uint16 assignedAliasUnicast;      // The commissioned 16 bit ID to be used as alias for this ZGPD for unicast communication mode
  uint8 groupcastRadius;            // The range limit of groupcast
  gpSinkTableSecOptions_t secOptions;  // The security options
  uint32 secFrameCounter;           // The incoming security frame counter for the srcID
  uint8 secKey[SEC_KEY_LEN];        // The security key for the srcID. It me be skipped, if common/derived key is used (as indicated by securityUse)

  uint16 tempMaster;                // Temp Master
  uint8 seqNum;                     // The last received sequence number from this ZGPD

  uint8 pendingForCommRes;          // Is the entry pending for a successful commissioning response

  // shorthand "Options" access
#define ST_securityUse        bitmap.optBits.secUse
#define assignedAliasUnicastFlag     bitmap.optBits.assignAliasUnicast
#define fixedLocation         bitmap.optBits.fixedLoc
#define rxOnCapability        bitmap.optBits.rxOnCap
#define seqNumCapability      bitmap.optBits.seqNumCap
#define tunnelCommMode        bitmap.optBits.commMode
} zclGP_SinkTableEntry_t;

// Proxy Table Entry Options field
typedef struct
{
  unsigned int appliID:3;           // application ID : 0b000 for ZGP with SrcID, 0b001 for LPED, 0b010 for ZGP with IEEE Mac address
  unsigned int entryActive:1;       // indicates that the current Proxy Table entry is active. A Proxy Table entry with the EntryActive flag equal to 0b0 can contain the SearchCounter field
  unsigned int entryValid:1;        // indicates that the current Proxy Table entry contains complete sink information
  unsigned int seqNumCap:1;         //
  unsigned int unicastZGPS:1;       // indicates that there is at least one ZGPS paired to this SrcID, that require unicast communication mode. Then, Sink address list field is present.
  unsigned int derivedGroupZGPS:1;  // indicates that there is at least one ZGPS paired to this SrcID, that requires groupcast communication mode with automatically-derived DGroupID
  unsigned int commGroupZGPS:1;     // indicates that there is at least one ZGPS paired to this SrcID, that require groupcast communication mode with the commissioned GroupID
  unsigned int firstToFwd:1;        // flag used for zgppTunnelingDelay calculation
  unsigned int inRange:1;           // indicates that this ZGPD is in range if this ZGPP. The default value is FALSE
  unsigned int ZGPDFixed:1;         // indicates portability capabilities of this ZGPD. The default value is FALSE
  unsigned int allUnicastRoutes:1;  // if set to 0b1, indicates that the ZGPP has active routes to all unicast sinks for this SrcID; if set to 0b0, it indicates that at least one unicast route is missing.
  unsigned int assignedAlias:1;     // indicates that the ZGPD has an assigned alias
  unsigned int securityUse:1;       // indicates that security-related fields of the Sink Table entry are present
  unsigned int reserved:1;          // Reserved for future use
} gpProxyTableOptionsBits_t;

// ZGP Proxy Table Options field bitmap
typedef union
{
  gpProxyTableOptionsBits_t optBits;
  uint16 optWord;
} gpProxyTableOptions_t;

// ZGP Sink Table Security Options field
typedef struct
{
  unsigned int level:2;         // security Level
  unsigned int keyType:3;       // Key Type
  unsigned int reserved:3;      // Reserved for future use
} gpProxyTableSecOptionsBits_t;

typedef union
{
  gpProxyTableSecOptionsBits_t optBits;
  uint8 optByte;
} gpProxyTableSecOptions_t;

typedef struct
{
  uint8 ieeeAddr[Z_EXTADDR_LEN];
  uint16 nwkAddr;
} zgpAddressList_t;

// ZGP Proxy Table Entry
typedef struct
{
  gpProxyTableOptions_t bitmap;         // Options field
  uint32 srcID;                         // srcID of the paired ZGPD
  gpProxyTableSecOptions_t secOptions;  // The security options
  uint8 seqNum;                         // The last received sequence number from this ZGPD
  uint16 assignedAlias;                 // The commissioned 16 bit ID to be used as alias for this ZGPD
  uint32 secFrameCounter;               // The incoming security frame counter for the srcID
  uint8 secKey[SEC_KEY_LEN];            // The security key for the srcID. It me be skipped, if common/derived key is used (as indicated by securityUse)
  uint8 numSinkAddrs;                   // The number of items in the Sink Address List
  uint8 numSinkGroups;                  // The number of items in the Sink Group List
  uint8 groupcastRadius;                // The range limit of groupcast
  uint8 searchCounter;

  // shorthand "Options" access
#define PT_securityUse      bitmap.optBits.securityUse
#define PT_assignedAlias    bitmap.optBits.assignedAlias
#define PT_allUnicastRoutes bitmap.optBits.allUnicastRoutes
#define PT_ZGPDFixed        bitmap.optBits.ZGPDFixed
#define PT_inRange          bitmap.optBits.inRange
#define PT_firstToFwd       bitmap.optBits.firstToFwd
#define PT_commGroupZGPS    bitmap.optBits.commGroupZGPS
#define PT_derivedGroupZGPS bitmap.optBits.derivedGroupZGPS
#define PT_unicastZGPS      bitmap.optBits.unicastZGPS
#define PT_seqNumCap        bitmap.optBits.seqNumCap
#define PT_entryValid       bitmap.optBits.entryValid
#define PT_entryActive      bitmap.optBits.entryActive
} zclGP_ProxyTableEntry_t;

typedef struct
{
  zclGP_ProxyTableEntry_t *pHdr;  // Proxy Table Entry fields
  zgpAddressList_t *pAddrList;    // Proxy Table Sink Address List
  gpSinkGroupListItem_t *pGroupList;             // Proxy Table Sink Group List
} zclGP_ProxyTable_t;

// ZGP Notification Options field bitmap
typedef struct
{
  unsigned int appliID:3;         // ZGP Application ID
  unsigned int alsoUcast:1;       // Indicates presence of ZGPSs paired to the same srcID with a different communications mode, as stored in this ZGPP's Green Power Device Table
  unsigned int alsoDerivedGrp:1;  // Indicates presence of ZGPSs paired to the same srcID with a different communications mode, as stored in this ZGPP's Green Power Device Table
  unsigned int alsoCommGrp:1;     // Indicates presence of ZGPSs paired to the same srcID with a different communications mode, as stored in this ZGPP's Green Power Device Table
  unsigned int secLevel:2;        // GPDF security level field
  unsigned int secKeyType:3;      // GPDF security key type field
  unsigned int appointTempMst:1;  // When set, indiates that the fields ZGPP short address and ZGPP Distance are present
  unsigned int zgppGPDFTxQFull:1; // indicates whether the ZGPP can still receive and store a GPDF Response for this SGPD srcID
  unsigned int reserved:3;        // Reserved for future use
} notificationOptionsbits_t;

// ZGP Notification Options field bitmap
typedef union
{
  notificationOptionsbits_t optBits;
  uint16 optWord;
} notificationOptions_t;

// ZGP Notification
typedef struct
{
  notificationOptions_t bitmap;
  uint32 ZGPDSrcID;
  uint32 ZGPDSecFrameCounter;
  uint8 ZGPDCmdID;
  uint16 ZGPPShortAddr;
  uint8 ZGPPDistance;
  uint8 payloadLen;
  uint8 *pZGPDCmdPayload;

  // shorthand "Options" access
#define zgppGPDFTxQueueFull     bitmap.optBits.zgppGPDFTxQFull
#define ZGPPpresent             bitmap.optBits.appointTempMst
#define notificationSecKeyType  bitmap.optBits.secKeyType
#define notificationSecLevel    bitmap.optBits.secLevel
#define alsoCommissionedGroup   bitmap.optBits.alsoCommGrp
#define alsoDerivedGroup        bitmap.optBits.alsoDerivedGrp
#define alsoUnicast             bitmap.optBits.alsoUcast
} zclGPNotification_t;

// ZGP Pairing Search "Options" field
typedef struct
{
  unsigned int appliID:3;           // ZGP application ID
  unsigned int reqUCastSinks:1;     // the proxy requests pairing information on uinicast sinks for the specified ZGPD
  unsigned int reqDerivedGCSinks:1; // the proxy requests pairing information on sinks accepting derived groupcast communication mode for the specified ZGPD
  unsigned int reqCommGCSinks:1;    // the proxy requests pairing information on sinks accepting commissioned GroupID communication mode for the specified ZGPD
  unsigned int reqZGPDFC:1;         // the proxy requests the security frame counter for the specified ZGPD
  unsigned int reqZGPDKey:1;        // the proxy requests the security key for the specified ZGPD
  unsigned int reserved:8;          // Reserved for future use
} pairingSearchOptionsbits_t;

// ZGP Pairing Search Options field bitmap
typedef union
{
  pairingSearchOptionsbits_t optBits;
  uint16 optWord;
} pairingSearchOptions_t;

// ZGP Pairing Search
typedef struct
{
  pairingSearchOptions_t bitmap;
  uint32 ZGPDSrcID;

  // shorthand "Options" access
#define requestZGPDKey                bitmap.optBits.reqZGPDKey
#define requestZGPDFrameCounter       bitmap.optBits.reqZGPDFC
#define requestCommGroupcastSinks     bitmap.optBits.reqCommGCSinks
#define requestDerivedGroupcastSinks  bitmap.optBits.reqDerivedGCSinks
#define requestUnicastSinks           bitmap.optBits.reqUCastSinks
} zclGPPairingSearch_t;

// ZGP Tunneling Stop "Options" field
typedef struct
{
  unsigned int appliID:3;           // ZGP Application ID filed
  unsigned int alsoDerivedGp:1;     // indicates the presence of derived Groupcast sink for the specified ZGPD
  unsigned int alsoCommGp:1;        // indicates the presence of commissioned Groupcast sink for the specified ZGPD
  unsigned int reserved:3;          // Reserved for future use
} tunnelingStopOptionsbits_t;

// ZGP Tunneling Stop Options field bitmap
typedef union
{
  tunnelingStopOptionsbits_t optBits;
  uint8 optByte;
} tunnelingStopOptions_t;

// ZGP Tunneling Stop
typedef struct
{
  tunnelingStopOptions_t bitmap;
  uint32 ZGPDSrcID;
  uint32 ZGPDFrameCounter;
  uint16 ZGPPShortAddr;
  uint8 ZGPPDistance;

  // These fields aren't sent over the air
  // They are used when queueing the message
  uint8 ZGPDCmdID;
  uint8 notifyTempMst;
  uint8 payloadLen;
  uint8 *pPayload;

  // shorthand "Options" access
#define tsAlsoCommGrp    bitmap.optBits.alsoCommGp
#define tsAlsoDerivedGrp bitmap.optBits.alsoDerivedGp
} zclGPTunnelingStop_t;

// ZGP Commissioning Notification Options field bitmap
typedef struct
{
  unsigned int appliID:3;           // ZGP Application ID
  unsigned int appTempMaster:1;    // allows the ZGPP to request ZGPS to select a ZGPP to foward commisioning reply to GPDF to this ZGPD
  unsigned int secLevel:2;         // GPDF security level field
  unsigned int secKeyType:3;       // GPDF security key type field
  unsigned int secProcFailed:1;    // the commissioning GPDF was protected but the security check failed
  unsigned int reserved:6;         // Reserved for future use
} notCommOptionsbits_t;

// ZGP Commissioning Notification Options field bitmap
typedef union
{
  notCommOptionsbits_t optBits;
  uint16 optWord;
} notCommOptions_t;

// ZGP Commissioning Notification
typedef struct
{
  notCommOptions_t bitmap;
  uint32 ZGPDSrcID;
  uint32 ZGPDFrameCounter;
  uint16 ZGPPShortAddr;
  uint8 ZGPPDistance;
  uint8 ZGPDCmdID;
  uint32 MIC;
  uint8 payloadLen;
  uint8 *pPayload;

  // shorthand "Options" access
#define secProcessingFailed   bitmap.optBits.secProcFailed
#define commSecKeyType        bitmap.optBits.secKeyType
#define commSecLevel          bitmap.optBits.secLevel
#define appointTempMaster     bitmap.optBits.appTempMaster
} zclGPCommNotification_t;

// ZGP Translation Table
typedef struct
{
  uint8 entryIdx;       // TT entry index
  uint8 ZGPDCmdID;      // ZGPD Command ID
  uint8 endPoint;       // destination endpoint
  uint16 profileID;     // destination profile ID
  uint16 clusterID;     // destination cluster ID
  uint8 cmdID;          // destination command ID
  uint8 cmdPayloadLen;
  uint8 cmdPayload[ZGP_TRANSLATIONTABLE_ENTRY_MAX_PAYLOAD_LEN];
} zgpTranslationTableUpdateRec_t;

// ZGP Translation Table Response
typedef struct
{
  uint32 ZGPDSrcID;     // ZGPD ID
  uint8 ZGPDCmdID;      // ZGPD Command ID
  uint8 endPoint;       // destination endpoint
  uint16 profileID;     // destination profile ID
  uint16 clusterID;     // destination cluster ID
  uint8 cmdID;          // destination command ID
  uint8 cmdPayloadLen;
  uint8 cmdPayload[ZGP_TRANSLATIONTABLE_ENTRY_MAX_PAYLOAD_LEN];
} zgpTranslationTableListEntry_t;

typedef struct
{
  unsigned int appliID:3;               // ZGP Application ID
  unsigned int action:2;                // action to perform on TT entry    // CERT-UPDATE - add support for action field
  unsigned int translationsNum:3;
  unsigned int reserved:8;
} zgpTranslationTableUpdateOptionsBits_t;

typedef union
{
  zgpTranslationTableUpdateOptionsBits_t optBits;
  uint16 optWord;
} zgpTranslationTableUpdateOptions_t;

// ZGP Translation Table Update Options field bitmap
typedef struct
{
  zgpTranslationTableUpdateOptions_t bitmap;
  uint32 ZGPDSrcID;
  zgpTranslationTableUpdateRec_t translationsSet[];

  // shorthand "Options" access
#define TT_ActionTrans      bitmap.optBits.action
#define TT_UpdateTransNum   bitmap.optBits.translationsNum
} zclGPTranslationTableUpdate_t;

// ZGP Translation Table Response Options field bitmap
typedef struct
{
  unsigned int appliID:3;   // ZGP Application ID
  unsigned int reserved:5;  // Reserved for future use
} TTResponseOptionsbits_t;

// ZGP Translation Table Response Options field bitmap
typedef union
{
  TTResponseOptionsbits_t optBits;
  uint8 optByte;
} TTResponseOptions_t;

// ZGP Translation Table Response Command
typedef struct
{
  uint8 status;             // SUCCESS or NOT_SUPPORTED
  TTResponseOptions_t bitmap;   // option field to support application ID
  uint8 totalNumEntries;    // number of entries in the ZGPD Command Translation Table of this ZGPS
  uint8 startIndex;         // this value equals the value in the request command
  uint8 count;              // number of translation table entries in this response command
  zgpTranslationTableListEntry_t transTable[]; // translation table entries
} zclGPTranslationTableResponse_t;

// ZGP Translation Table Request Command
typedef struct
{
  uint8 startIndex;     // The starting index into the ZGPD command translation table from which to get device information.
} zclGPTranslationRequest_t;

typedef struct
{
  uint8 ZGPDCmdID;      // ZGPD Command ID
  uint16 clusterID;     // destination cluster ID
  uint8 cmdID;          // destination command ID
  uint8 cmdPayloadLen;
// @todo CERT-UPDATE - add a field to map fields between cmd
} zclGPTranslationAutomaticItem_t;

// CERT-UPDATE: add type for Automatic Translation Table
typedef struct
{
  uint8 endPoint;       // destination endpoint
  uint16 profileID;     // destination profile ID
  uint8 ZGPDeviceID;    // ZGP Device ID
  uint8 count;              // number of automatic translation table entries
  zclGPTranslationAutomaticItem_t *transAutoTable; // translation table entries
} zclGPTranslationAutomaticTable_t;

// ZGP Notification Response Options field bitmap
typedef struct
{
  unsigned int appliID:3;          // ZGP Application ID
  unsigned int firstToFwd:1;       // The notification reached the ZGPS first
  unsigned int noPairingFlag:1;    // Indicates the sink has no pairing with the srcID
  unsigned int reserved:3;         // Reserved for future use
} notRspOptionsbits_t;

// ZGP Notification Response Options field bitmap
typedef union
{
  notRspOptionsbits_t optBits;
  uint8 optByte;
} notRspOptions_t;

// ZGP Notification Response
typedef struct
{
  notRspOptions_t bitmap;
  uint32 ZGPDSrcID;
  uint32 ZGPDFrameCounter;

  // shorthand "Options" access
#define noPairing        bitmap.optBits.noPairingFlag
#define firstToForward   bitmap.optBits.firstToFwd
} zclGPNotificationRsp_t;

// ZGP Pairing Options field bitmap
typedef struct
{
  unsigned int appliID:3;           // ZGP Application ID
  unsigned int addSink:1;           // whether the ZGP Sink wishes to add or remove Proxy Table pairing to the ZGPD
  unsigned int removeZGPD:1;        // indicates (1) that the ZGPD is being removed from the network.
  unsigned int commMode:2;          // communication mode requested by the ZGPS.  ie ZGP_TUNNEL_MODE_UNICAST
  unsigned int zgpdFixed:1;         // if set, copy the FixedLocation field of the Sink Table
  unsigned int macSeqNoCap:1;       // if set, copy the MAC sequence number capabilities from the Sink Table
  unsigned int secLevel:2;          // carry the values from the Sink Table
  unsigned int secKeyType:3;        // carry the values from the Sink Table
  unsigned int fcPresent:1;         // indicates whether the Security Frame Counter is present
  unsigned int keyPresent:1;        // indicates whether the Security key is present
  unsigned int aliasPresent:1;      // indicates whether the Assigned Alias is present in this message
  unsigned int fwdRadiusPresent:1;  // indicates whether the Forward Radius is present in this message
  unsigned int reserved:6;          // Reserved for future use
} pairingOptionsbits_t;

// ZGP Pairing Options field bitmap
typedef union
{
  pairingOptionsbits_t optBits;
  uint8 optBytesTab[3];
} pairingOptions_t;

// ZGP Pairing Command
typedef struct
{
  pairingOptions_t bitmap;
  uint32 ZGPDSrcID;
  uint8 sinkIEEEAddr[Z_EXTADDR_LEN];
  uint16 sinkNwkAddr;
  uint16 sinkGroupID;
  uint8 deviceID;
  uint32 ZGPDFrameCounter;
  uint8 ZGPDKey[SEC_KEY_LEN];
  uint16 assignedAlias;
  uint8 forwardingRadius;

  // shorthand "Options" access
#define pairingFwdRadiusPresent   bitmap.optBits.fwdRadiusPresent
#define pairingAliasPresent       bitmap.optBits.aliasPresent
#define pairingsecKeyPresent      bitmap.optBits.keyPresent
#define pairingsecFCPresent       bitmap.optBits.fcPresent
#define pairingtempMasterPresent  bitmap.optBits.tempMasterPresent
#define pairingSecKeyType         bitmap.optBits.secKeyType
#define pairingSecLevel           bitmap.optBits.secLevel
#define pairingMacSeqNoCap        bitmap.optBits.macSeqNoCap
#define pairingFixedLocation      bitmap.optBits.zgpdFixed
#define pairingCommMode           bitmap.optBits.commMode
#define pairingRemoveZGPD         bitmap.optBits.removeZGPD
#define pairingAddSink            bitmap.optBits.addSink
} zclGPPairing_t;

// ZGP Proxy Commissioning Mode Options field bitmap
typedef struct
{
  unsigned int action:1;            // indicates a request to enter commissioning mode, if 0 indicates a request to exist commissioning mode
  unsigned int exitMode:3;          // zgpsCommissioningExitMode attribute bitmap.  ie. ZGP_EXIT_MODE_ON_COMMISSIONING_WINDOW_EXPIRATION
  unsigned int channelPresent:1;    // indicates whether the channel field is present
  unsigned int reserved:2;          // Reserved for future use
} proxyCommOptionsbits_t;

// ZGP Proxy Commissioning Mode Options field bitmap
typedef union
{
  proxyCommOptionsbits_t optBits;
  uint8 optByte;
} proxyCommOptions_t;

// ZGP Proxy Commissioning Mode Command
typedef struct
{
  proxyCommOptions_t bitmap;
  uint16 commissioningWindow;
  uint8 channel;

  // shorthand "Options" access
#define proxyCommChannelPresent bitmap.optBits.channelPresent
#define proxyCommExitMode       bitmap.optBits.exitMode
#define proxyCommAction         bitmap.optBits.action
} zclGPProxyCommMode_t;

// ZGP Response Options field bitmap
typedef struct
{
  unsigned int appliID:3;   // ZGP Application ID
  unsigned int reserved:5;  // Reserved for future use
} responseOptionsbits_t;

// ZGP Response Options field bitmap
typedef union
{
  responseOptionsbits_t optBits;
  uint8 optByte;
} responseOptions_t;

// ZGP Response Command
typedef struct
{
  responseOptions_t bitmap;  // option bits field
  uint16 ZGPPShortAddr;   // address of the ZGPP which will transmit the GPDF Response frame to the ZGPD
  uint8 ZGPPTxChan;       // indicates the channel the Response GPDF will be sent on.
  uint32 ZGPDSrcID;       // intended ZGPD
  uint8 ZGPDCmdID;        // ZGPD command ID
  uint8 payloadLen;       // Length of the payload field
  uint8 payload[];        // GPDF command payload field
} zclGPDResponse_t;

typedef struct
{
  unsigned int action:3;
  unsigned int sendPairing:1;
  unsigned int reserved:4;
} confPairingActionsbits_t;

typedef union
{
  confPairingActionsbits_t actionsBits;
  uint8 actionsByte;
} confPairingActions_t;

// ZGP Pairing Configuration Command
typedef struct
{
  confPairingActions_t    actions;
  zclGP_SinkTableEntry_t  sinkTableParams;
  uint8 pairedEPsNum;
  uint8 *pPairedEPsSet;

  // shorthand "Actions" access
#define confPairingAction         actions.actionsBits.action
#define confPairingSendPairing    actions.actionsBits.sendPairing
} zclGPPairingConfigurationCmd_t;

// CERT-UPDATE - add enum type for incoming process indication
typedef enum
{
  zgpDIRECT_STUB,
  zgpDIRECT_EPPROXY,
  zgpINTERNAL_EPPROXY_BROADCAST,
  zgpINTERNAL_EPPROXY_GROUPCAST,
  zgpINTERNAL_EPPROXY_UNICAST
} zgpInProcessIndication_t;

// CERT-UPDATE - add field to indicate incoming process : either external from stub or EPP or internal from EPP
// This callback is called to process a Notification command
typedef ZStatus_t (*zclGP_NotificationCB_t)( uint8 transSeqNum, zclGPNotification_t *pCmd, zgpInProcessIndication_t inComeSrc );
// @todo CERT-UPDATE: filter should not use transSeqNum but GPDF SecureFrameCounter !

// This callback is called to process a Pairing Search command
typedef ZStatus_t (*zclGP_PairingSearchCB_t)( uint8 transSeqNum, zclGPPairingSearch_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Tunnel Stop command
typedef ZStatus_t (*zclGP_TunnelStopCB_t)( uint8 transSeqNum, zclGPTunnelingStop_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Commissioning Notification command
typedef ZStatus_t (*zclGP_CommissioningNotificationCB_t)( uint8 transSeqNum, zclGPCommNotification_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Translation Table Update command
typedef ZStatus_t (*zclGP_TranslationTableUpdateCB_t)( uint8 transSeqNum, zclGPTranslationTableUpdate_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Translation Table Request command
typedef ZStatus_t (*zclGP_TranslationTableRequestCB_t)( uint8 transSeqNum, zclGPTranslationRequest_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Configure Pairing command
typedef ZStatus_t (*zclGP_ConfigurePairingCB_t)( uint8 transSeqNum, zclGPPairingConfigurationCmd_t *pCmd, zgpInProcessIndication_t inComeSrc );

// This callback is called to process a Notification Response command
typedef ZStatus_t (*zclGP_NotificationResponseCB_t)( uint8 transSeqNum, zclGPNotificationRsp_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Pairing command
typedef ZStatus_t (*zclGP_PairingCB_t)( uint8 transSeqNum, zclGPPairing_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Proxy Commissioning Mode command
typedef ZStatus_t (*zclGP_ProxyCommissioningModeCB_t)( uint8 transSeqNum, zclGPProxyCommMode_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Response command
typedef ZStatus_t (*zclGP_ResponseCB_t)( uint8 transSeqNum, zclGPDResponse_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// This callback is called to process a Translation Table Response command
typedef ZStatus_t (*zclGP_TranslationTableResponseCB_t)( uint8 transSeqNum, zclGPTranslationTableResponse_t *pCmd, zgpInProcessIndication_t inComeSrc  );

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  // Received Server Commands
  zclGP_NotificationCB_t              pfnNotification;              // Notification Command
  zclGP_PairingSearchCB_t             pfnPairingSearch;             // Pairing Search Command
  zclGP_TunnelStopCB_t                pfnTunnelStop;                // Tunnel Stop Command
  zclGP_CommissioningNotificationCB_t pfnCommissioningNotification; // Commissioning Notification Command
  zclGP_TranslationTableUpdateCB_t    pfnTranslationTableUpdate;    // Translation Table Update Command
  zclGP_TranslationTableRequestCB_t   pfnTranslationTableRequest;   // Translation Table Request Command
  zclGP_ConfigurePairingCB_t          pfnPairingConfiguration;      // Configure Pairing Command

  // Received Client Commands
  zclGP_NotificationResponseCB_t      pfnNotificationResponse;      // Notification Response Command
  zclGP_PairingCB_t                   pfnPairing;                   // Pairing Command
  zclGP_ProxyCommissioningModeCB_t    pfnProxyCommissioningMode;    // Proxy Commissioning Mode Command
  zclGP_ResponseCB_t                  pfnResponse;                  // Response Command
  zclGP_TranslationTableResponseCB_t  pfnTranslationTableResponse;  // Translation Table Response Command
} zclGP_AppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Register for callbacks from this cluster library
  */
extern ZStatus_t zclGP_RegisterCmdCallbacks( uint8 endpoint, zclGP_AppCallbacks_t *callbacks );

/*
 * Call to send out a Notification Command
 *      pNotification - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_Notification( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPNotification_t *pNotification,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Pairing Search Command
 *      pPairingSearch - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_PairingSearch( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclGPPairingSearch_t *pPairingSearch,
                                    uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Tunneling Stop Command
 *      pTunnelingStop - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_TunnelingStop( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclGPTunnelingStop_t *pTunnelingStop,
                                    uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Commissioning Notification Command
 *      pCommNotif - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_CommissioningNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPCommNotification_t *pCommNotif,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Translation Table Update Command
 *      pTransTableUpdate - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_TranslationTableUpdate( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPTranslationTableUpdate_t *pTransTableUpdate,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Translation Table Request Command
 *      pTransTableReq - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_TranslationTableRequest( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclGPTranslationRequest_t *pTransTableReq,
                                    uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Configure Pairing Command
 *      pPairingConfig - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_PairingConfiguration( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPPairingConfigurationCmd_t *pPairingConfig,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Notification Response Command
 *      pNotRsp - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_NotificationResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclGPNotificationRsp_t *pNotRsp,
                                    uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Pairing Command
 *      pPairing - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_Pairing( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPPairing_t *pPairing,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Proxy Commissioning Mode Command
 *      pProxyCommMode - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_ProxyCommissioningMode( uint8 srcEP, afAddrType_t *dstAddr,
                                    zclGPProxyCommMode_t *pProxyCommMode,
                                    uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Response Command
 *      pRsp - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_Response( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPDResponse_t *pRsp,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a Translation Table Response Command
 *      pTransTblRsp - all of the command's parameters
 */
extern ZStatus_t zclGP_Send_TranslationTableResponse( uint8 srcEP, afAddrType_t *dstAddr,
                                   zclGPTranslationTableResponse_t *pTransTblRsp,
                                   uint8 disableDefaultRsp, uint8 seqNum );

/*
 * Call to send out a ZCL command according to a Translation Table's entry
 *      pTransTblEntry - the Translation Table's entry with the command's parameters
 */
extern ZStatus_t zclGP_Send_TranslatedCommand( afAddrType_t *dstAddr,
                                   zgpTranslationTableListEntry_t *pTransTblEntry,
                                   uint8 disableDefaultRsp, uint8 seqNum, uint8 gpdfDataLen, uint8 *pgpdfData);

#ifdef __cplusplus
}
#endif

#endif /* ZCL_GP_H */
