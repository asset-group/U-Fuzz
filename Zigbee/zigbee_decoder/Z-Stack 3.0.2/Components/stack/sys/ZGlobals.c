/**************************************************************************************************
  Filename:       ZGlobals.c
  Revised:        $Date: 2015-10-05 14:56:09 -0700 (Mon, 05 Oct 2015) $
  Revision:       $Revision: 44517 $

  Description:    User definable Z-Stack parameters.


  Copyright 2007-2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
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
#include "OSAL_Nv.h"
#include "ZDObject.h"
#include "ZGlobals.h"
#include "ZDNwkMgr.h"
#include "OnBoard.h"
#include "ZDSecMgr.h"
#include "bdb.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

typedef struct zgItem
{
  uint16 id;
  uint16 len;
  void *buf;
} zgItem_t;

/*********************************************************************
 * NWK GLOBAL VARIABLES
 */

// Polling values
uint32 zgPollRate = POLL_RATE;
uint32 zgSavedPollRate = POLL_RATE;
uint16 zgQueuedPollRate = QUEUED_POLL_RATE;
uint16 zgResponsePollRate = RESPONSE_POLL_RATE;
uint16 zgRejoinPollRate = REJOIN_POLL_RATE;

// Rejoin backoff (silent period ) duration
uint32 zgDefaultRejoinBackoff = REJOIN_BACKOFF;

// Rejoin scan duration
uint32 zgDefaultRejoinScan = REJOIN_SCAN ;

// Transmission retries numbers
uint8 zgMaxDataRetries = NWK_MAX_DATA_RETRIES;
uint8 zgMaxPollFailureRetries = MAX_POLL_FAILURE_RETRIES;

// Default channel list
uint32 zgDefaultChannelList = 0;

// Stack profile Id
uint8 zgStackProfile = STACK_PROFILE_ID;

// Default indirect message holding timeout
uint8 zgIndirectMsgTimeout = NWK_INDIRECT_MSG_TIMEOUT;

// Security mode
uint8 zgSecurityMode = ZG_SECURITY_MODE;

// Secure permit join
uint8 zgSecurePermitJoin = TRUE;

// trustcenter allows rejoins using well known or default keys 
uint8 zgAllowRejoins = FALSE;   // FALSE by default

//if zgAllowRejoins is set to FALSE, the rejoining device will receive a rejoin rsp success and also a leave command.
//The leave command will have set the rejoin parameter = 'zgAllowRejoinsOptions' value.
uint8 zgAllowRejoinsOptions = FALSE;  //FALSE by default

//allowInstallCodes
uint8 zgAllowInstallCodes = ZG_IC_SUPPORTED_NOT_REQUIRED;
//Allow other devices in the network to change the TC permit joining policy
uint8 zgAllowRemoteTCPolicyChange = TRUE;

//Change these policies to FALSE is not supported
//uint8 zgAllowTrustCenterLinkKeyRequest = TRUE;
//uint8 zgAllowApplicationKeyRequests = TRUE;


// Trust center address
uint8 zgApsTrustCenterAddr[Z_EXTADDR_LEN] = { 0 };


// Route Discovery Time - amount of time that a route request lasts
uint8 zgRouteDiscoveryTime = ROUTE_DISCOVERY_TIME;

// Route expiry
uint8 zgRouteExpiryTime = ROUTE_EXPIRY_TIME;

// Extended PAN Id
uint8 zgExtendedPANID[Z_EXTADDR_LEN];

// Broadcast parameters
uint8 zgMaxBcastRetires   = MAX_BCAST_RETRIES;
uint8 zgPassiveAckTimeout = PASSIVE_ACK_TIMEOUT;
uint8 zgBcastDeliveryTime = BCAST_DELIVERY_TIME;

// Network mode
uint8 zgNwkMode = NWK_MODE;

// Many-to-one values
uint8 zgConcentratorEnable = CONCENTRATOR_ENABLE;
uint8 zgConcentratorDiscoveryTime = CONCENTRATOR_DISCOVERY_TIME;
uint8 zgConcentratorRadius = CONCENTRATOR_RADIUS;
uint8 zgConcentratorRC = CONCENTRATOR_ROUTE_CACHE;   // concentrator with route cache (no memory constraints)
uint8 zgNwkSrcRtgExpiryTime = SRC_RTG_EXPIRY_TIME;

// Cleanup Child Table according to routing traffic
uint8 zgRouterOffAssocCleanup = FALSE;

// Determines whether or not a remote NWK leave request command frame received
// by the local device is accepted .
uint8 zgNwkLeaveRequestAllowed = TRUE;



// Enable or disable processing a leave request confirm generated by local
// device when it sends a leave request to a remote device. 
// NOTE: This is only used for testing purposes, do not disable 
uint8 zgNwkProcessLocalLeaveCnf = TRUE;


//=======    Child Aging PARENT ROUTER (ZR/ZC) configuration   ========
// You can setup a router to support Child Table Aging in 1 of 2 modes of
// operation.  The first mode is NWK_PARENT_INFO_ORPHAN_NOTIFICATION and it
// expects end devices to use orphan scan periodically as a means of a keep-alive
// notification to the parent.  The other mode is NWK_PARENT_INFO_MAC_DATA_POLL
// which uses the end device's MAC POLL request as the keep-alive notification.
// The first method is preferred for new devices, where the end devices provide
// support for it (which will be manditory in future Zigbee Home Automation
// Specifications).
// The second method is compatible with older end devices without the need for
// specific child aging support.
//
// The method supported by the router (or coordinator) is determined at build time
// by setting zgNwkParentInformation to either NWK_PARENT_INFO_ORPHAN_NOTIFICATION
// or NWK_PARENT_INFO_MAC_DATA_POLL.
//
// End device built with Child Table Aging support both methods, the method is
// determined by the parent and communicated at run-time.
#if ( ZG_BUILD_RTR_TYPE )
uint8 zgNwkParentInformation = NWK_PARENT_INFO_MAC_DATA_POLL;
#else
uint8 zgNwkParentInformation = NWK_PARENT_INFO_UNDEFINED;
#endif

// This is an index into table Requested Timeout Enumerated Values.
// It is used by the parent router, it indicates the default timeout value
// for any end device that does not negotiate a different timeout value
uint8 zgNwkEndDeviceTimeoutDefault = NWK_END_DEV_TIMEOUT_DEFAULT;

// Index into table Requested Timeout Enumerated Values.
// Used to keep the leave message into MAC queue for child devices that has expired
uint8 zgNwkEndDeviceLeaveTimeoutDefault = NWK_END_DEVICE_LEAVE_TIMEOUT;
//=====================================================================

//==========    Child Aging END DEVICE configuration    ===============
// Values used by End Device when sending End Device Timeout Request
uint8 zgEndDeviceTimeoutValue  = END_DEV_TIMEOUT_VALUE;
uint8 zgEndDeviceConfiguration = END_DEV_CONFIGURATION;


//=====================================================================

// Determines if the Child Aging Table Management process is active or not.
// This feature is optional and it is disabled by default.
//
// NOTICE:  Before enabling Child Aging make sure to review all the related
// definitions in this file, especially zgNwkParentInformation.
uint8 zgChildAgingEnable = TRUE;

//==========    TouchLink NWK configuration    ===============
// Values used by Router when starts a network as initiator
uint8 zTouchLinkNwkStartRtr = FALSE;

/*********************************************************************
 * APS GLOBAL VARIABLES
 */

// The maximum number of retries allowed after a transmission failure
uint8 zgApscMaxFrameRetries = APSC_MAX_FRAME_RETRIES;

// The maximum number of seconds (milliseconds) to wait for an
// acknowledgement to a transmitted frame.

// This number is used by polled devices.
uint16 zgApscAckWaitDurationPolled = APSC_ACK_WAIT_DURATION_POLLED;

// This number is used by non-polled devices in the following formula:
//   (100 mSec) * (_NIB.MaxDepth * zgApsAckWaitMultiplier)
uint8 zgApsAckWaitMultiplier = 2;

// The maximum number of milliseconds for the end device binding
uint16 zgApsDefaultMaxBindingTime = APS_DEFAULT_MAXBINDING_TIME;

// The 64-big identifier of the network to join or form.
// Default set to all zeros
uint8 zgApsUseExtendedPANID[Z_EXTADDR_LEN] = {00,00,00,00,00,00,00,00};

// A boolean flag that indicates whether it is OK to use insecure join
// on startup. Default set to TRUE
uint8 zgApsUseInsecureJoin = TRUE;

// The radius of broadcast multicast transmissions
uint8 zgApsNonMemberRadius = APS_DEFAULT_NONMEMBER_RADIUS;

// Commissioned Network Address
uint16 zgNwkCommissionedNwkAddr = INVALID_NODE_ADDR;

// APS Duplication Rejection table variables
uint16 zgApscDupRejTimeoutInc = DEFAULT_APS_DUP_REJ_TIMEOUT_INCREMENT;
uint8  zgApscDupRejTimeoutCount = DEFAULT_APS_DUP_REJ_TIMEOUT;
uint16 zgApsMinDupRejTableSize = APS_DUP_REJ_ENTRIES;



/*********************************************************************
 * SECURITY GLOBAL VARIABLES
 */

// If TRUE, preConfigKey should be configured on all devices on the network
// If false, it is configured only on the coordinator and sent to other
// devices upon joining.
uint8 zgPreConfigKeys = FALSE;

// The type of link key in use.  This will determine the security
// policies associated with sending and receiving APS messages.
// If ZG_GLOBAL_LINK_KEY APS TCLK security may be used for specific APS commands
// If ZG_UNIQUE_LINK_KEY APS TCLK security is required for specific APS commands
uint8 zgApsLinkKeyType = ZG_GLOBAL_LINK_KEY;

// With changes introduced in R20 of the ZigBee specification,
// boolean value of zgUseDefaultTCLK is set depending on zgApsLinkKeyType value.
//
// For zgApsLinkKeyType = ZG_GLOBAL_LINK_KEY, zgUseDefaultTCLK = TRUE
// For zgApsLinkKeyType = ZG_UNIQUE_LINK_KEY, different devices have
// different value:
//      ZC should have             zgUseDefaultTCLK = FALSE
//      Other devices should have  zgUseDefaultTCLK = TRUE
// This is initialized in zgInitItems()
// If ZG_UNIQUE_LINK_KEY, individual trust center link key between each device
// and the trust center should be manually configured via MT_SYS_OSAL_NV_WRITE
uint8 zgUseDefaultTCLK;

#if defined ( APP_TP2_TEST_MODE )
uint8 guTxApsSecON = TP_GU_BOTH;
uint8 guEnforceRxApsSec = TP_GU_ALL;
#endif

uint8 zgApsAllowR19Sec = FALSE;
uint8 zgSwitchCoordKey = FALSE;
uint8 zgSwitchCoordKeyIndex = 0;

/*********************************************************************
 * ZDO GLOBAL VARIABLES
 */

// Configured PAN ID
uint16 zgConfigPANID = ZDAPP_CONFIG_PAN_ID;

// Device Logical Type
uint8 zgDeviceLogicalType = DEVICE_LOGICAL_TYPE;

// Startup Delay
uint8 zgStartDelay = START_DELAY;

#if !defined MT_TASK
// Flag to use verbose (i.e. "cc2480-style") direct MT callbacks in ZDProfile.c, ZDP_IncomingData().
uint8 zgZdoDirectCB = TRUE;
#endif

// Min number of attempted transmissions for Channel Interference detection
uint8 zgNwkMgrMinTransmissions = ZDNWKMGR_MIN_TRANSMISSIONS;

/*********************************************************************
 * APPLICATION GLOBAL VARIABLES
 */

// Network Manager Mode
uint8 zgNwkMgrMode = ZDNWKMGR_ENABLE;

/*********************************************************************
 * NON-STANDARD GLOBAL VARIABLES
 */

// Simple API Endpoint
uint8 zgSapiEndpoint = SAPI_ENDPOINT;

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * ZGlobal Item Table
 */
static CONST zgItem_t zgItemTable[] =
{
#if defined ( NV_INIT )
#if !defined MT_TASK
  {
    ZCD_NV_ZDO_DIRECT_CB, sizeof(zgZdoDirectCB), &zgZdoDirectCB
  },
#endif
  {
    ZCD_NV_LOGICAL_TYPE, sizeof(zgDeviceLogicalType), &zgDeviceLogicalType
  },
  {
    ZCD_NV_POLL_RATE, sizeof(zgPollRate), &zgPollRate
  },
  {
    ZCD_NV_QUEUED_POLL_RATE, sizeof(zgQueuedPollRate), &zgQueuedPollRate
  },
  {
    ZCD_NV_RESPONSE_POLL_RATE, sizeof(zgResponsePollRate), &zgResponsePollRate
  },
  {
    ZCD_NV_REJOIN_POLL_RATE, sizeof(zgRejoinPollRate), &zgRejoinPollRate
  },
  {
    ZCD_NV_DATA_RETRIES, sizeof(zgMaxDataRetries), &zgMaxDataRetries
  },
  {
    ZCD_NV_POLL_FAILURE_RETRIES, sizeof(zgMaxPollFailureRetries), &zgMaxPollFailureRetries
  },
  {
    ZCD_NV_CHANLIST, sizeof(zgDefaultChannelList), &zgDefaultChannelList
  },
  {
    ZCD_NV_SCAN_DURATION, sizeof(zgDefaultStartingScanDuration), &zgDefaultStartingScanDuration
  },
  {
    ZCD_NV_STACK_PROFILE, sizeof(zgStackProfile), &zgStackProfile
  },
  {
    ZCD_NV_INDIRECT_MSG_TIMEOUT, sizeof(zgIndirectMsgTimeout), &zgIndirectMsgTimeout
  },
  {
    ZCD_NV_ROUTE_EXPIRY_TIME, sizeof(zgRouteExpiryTime), &zgRouteExpiryTime
  },
  {
    ZCD_NV_EXTENDED_PAN_ID, Z_EXTADDR_LEN, zgExtendedPANID
  },
  {
    ZCD_NV_BCAST_RETRIES, sizeof(zgMaxBcastRetires), &zgMaxBcastRetires
  },
  {
    ZCD_NV_PASSIVE_ACK_TIMEOUT, sizeof(zgPassiveAckTimeout), &zgPassiveAckTimeout
  },
  {
    ZCD_NV_BCAST_DELIVERY_TIME, sizeof(zgBcastDeliveryTime), &zgBcastDeliveryTime
  },
  {
    ZCD_NV_NWK_MODE, sizeof(zgNwkMode), &zgNwkMode
  },
  {
    ZCD_NV_CONCENTRATOR_ENABLE, sizeof(zgConcentratorEnable), &zgConcentratorEnable
  },
  {
    ZCD_NV_CONCENTRATOR_DISCOVERY, sizeof(zgConcentratorDiscoveryTime), &zgConcentratorDiscoveryTime
  },
  {
    ZCD_NV_CONCENTRATOR_RADIUS, sizeof(zgConcentratorRadius), &zgConcentratorRadius
  },
  {
    ZCD_NV_CONCENTRATOR_RC, sizeof(zgConcentratorRC), &zgConcentratorRC
  },
  {
    ZCD_NV_SRC_RTG_EXPIRY_TIME, sizeof(zgNwkSrcRtgExpiryTime), &zgNwkSrcRtgExpiryTime
  },
  {
    ZCD_NV_ROUTE_DISCOVERY_TIME, sizeof(zgRouteDiscoveryTime), &zgRouteDiscoveryTime
  },
#ifndef NONWK
  {
    ZCD_NV_PANID, sizeof(zgConfigPANID), &zgConfigPANID
  },
  {
    ZCD_NV_PRECFGKEYS_ENABLE, sizeof(zgPreConfigKeys), &zgPreConfigKeys
  },
  {
    ZCD_NV_SECURITY_MODE, sizeof(zgSecurityMode), &zgSecurityMode
  },
  {
    ZCD_NV_SECURE_PERMIT_JOIN, sizeof(zgSecurePermitJoin), &zgSecurePermitJoin
  },
  {
    ZCD_NV_USE_DEFAULT_TCLK, sizeof(zgUseDefaultTCLK), &zgUseDefaultTCLK
  },
  {
    ZCD_NV_TRUSTCENTER_ADDR, Z_EXTADDR_LEN, zgApsTrustCenterAddr
  },
  {
    ZCD_NV_APS_LINK_KEY_TYPE, sizeof(zgApsLinkKeyType), &zgApsLinkKeyType
  },
#endif // NONWK
  {
    ZCD_NV_APS_FRAME_RETRIES, sizeof(zgApscMaxFrameRetries), &zgApscMaxFrameRetries
  },
  {
    ZCD_NV_APS_ACK_WAIT_DURATION, sizeof(zgApscAckWaitDurationPolled), &zgApscAckWaitDurationPolled
  },
  {
    ZCD_NV_APS_ACK_WAIT_MULTIPLIER, sizeof(zgApsAckWaitMultiplier), &zgApsAckWaitMultiplier
  },
  {
    ZCD_NV_BINDING_TIME, sizeof(zgApsDefaultMaxBindingTime), &zgApsDefaultMaxBindingTime
  },
  {
    ZCD_NV_APS_USE_EXT_PANID, Z_EXTADDR_LEN, zgApsUseExtendedPANID
  },
  {
    ZCD_NV_APS_USE_INSECURE_JOIN, sizeof(zgApsUseInsecureJoin), &zgApsUseInsecureJoin
  },
  {
    ZCD_NV_APS_NONMEMBER_RADIUS, sizeof(zgApsNonMemberRadius), &zgApsNonMemberRadius
  },
  {
    ZCD_NV_START_DELAY, sizeof(zgStartDelay), &zgStartDelay
  },
  {
    ZCD_NV_SAPI_ENDPOINT, sizeof(zgSapiEndpoint), &zgSapiEndpoint
  },
  {
    ZCD_NV_NWK_MGR_MODE, sizeof(zgNwkMgrMode), &zgNwkMgrMode
  },
  {
    ZCD_NV_NWKMGR_MIN_TX, sizeof(zgNwkMgrMinTransmissions), &zgNwkMgrMinTransmissions
  },
  {
    ZCD_NV_ROUTER_OFF_ASSOC_CLEANUP, sizeof(zgRouterOffAssocCleanup), &zgRouterOffAssocCleanup
  },
  {
    ZCD_NV_NWK_LEAVE_REQ_ALLOWED, sizeof(zgNwkLeaveRequestAllowed), &zgNwkLeaveRequestAllowed
  },
  {
    ZCD_NV_COMMISSIONED_NWK_ADDR, sizeof(zgNwkCommissionedNwkAddr), &zgNwkCommissionedNwkAddr
  },
  {
    ZCD_NV_APS_ALLOW_R19_SECURITY, sizeof(zgApsAllowR19Sec), &zgApsAllowR19Sec
  },
  {
    ZCD_NV_APS_DUPREJ_TIMEOUT_INC, sizeof(zgApscDupRejTimeoutInc), &zgApscDupRejTimeoutInc
  },
  {
    ZCD_NV_APS_DUPREJ_TIMEOUT_COUNT, sizeof(zgApscDupRejTimeoutCount), &zgApscDupRejTimeoutCount
  },
  {
    ZCD_NV_APS_DUPREJ_TABLE_SIZE, sizeof(zgApsMinDupRejTableSize), &zgApsMinDupRejTableSize
  },
  {
    ZCD_NV_NWK_CHILD_AGE_ENABLE, sizeof(zgChildAgingEnable), &zgChildAgingEnable
  },
  {
    ZCD_NV_NWK_PARENT_INFO, sizeof(zgNwkParentInformation), &zgNwkParentInformation
  },
  {
    ZCD_NV_NWK_ENDDEV_TIMEOUT_DEF, sizeof(zgNwkEndDeviceTimeoutDefault), &zgNwkEndDeviceTimeoutDefault
  },
  {
    ZCD_NV_END_DEV_TIMEOUT_VALUE, sizeof(zgEndDeviceTimeoutValue), &zgEndDeviceTimeoutValue
  },
  {
    ZCD_NV_END_DEV_CONFIGURATION, sizeof(zgEndDeviceConfiguration), &zgEndDeviceConfiguration
  },
#endif // NV_INIT
  // Last item -- DO NOT MOVE IT!
  {
    0x00, 0, NULL
  }
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static uint8 zgItemInit( uint16 id, uint16 len, void *buf, uint8 setDefault );

static void zgUpgradeNVItems( void );

#ifndef NONWK
static uint8 zgPreconfigKeyInit( uint8 setDefault );
#endif

#ifdef UPGRADE_SECURITY_NV_ITEMS
static void zgUpgradeSecurityNVItems( void );
#endif // UPGRADE_SECURITY_NV_ITEMS
/*********************************************************************
 * @fn       zgItemInit()
 *
 * @brief
 *
 *   Initialize a global item. If the item doesn't exist in NV memory,
 *   write the system default (value passed in) into NV memory. But if
 *   it exists, set the item to the value stored in NV memory.
 *
 *   Also, if setDefault is TRUE and the item exists, we will write
 *   the default value to NV space.
 *
 * @param   id - item id
 * @param   len - item len
 * @param   buf - pointer to the item
 * @param   setDefault - TRUE to set default, not read
 *
 * @return  ZSUCCESS if successful, NV_ITEM_UNINIT if item did not
 *          exist in NV, NV_OPER_FAILED if failure.
 */
static uint8 zgItemInit( uint16 id, uint16 len, void *buf, uint8 setDefault )
{
  uint8 status;

  // If the item doesn't exist in NV memory, create and initialize
  // it with the value passed in.
  status = osal_nv_item_init( id, len, buf );
  if ( status == ZSUCCESS )
  {
    if ( setDefault )
    {
      // Write the default value back to NV
      status = osal_nv_write( id, 0, len, buf );
    }
    else
    {
      // The item exists in NV memory, read it from NV memory
      status = osal_nv_read( id, 0, len, buf );
    }
  }

  return (status);
}

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn          zgInit
 *
 * @brief
 *
 *   Initialize the Z-Stack Globals. If an item doesn't exist in
 *   NV memory, write the system default into NV memory. But if
 *   it exists, set the item to the value stored in NV memory.
 *
 * NOTE: The Startup Options (ZCD_NV_STARTUP_OPTION) indicate
 *       that the Config state items (zgItemTable) need to be
 *       set to defaults (ZCD_STARTOPT_DEFAULT_CONFIG_STATE). The
 *
 * @param       none
 *
 * @return      ZSUCCESS if successful, NV_ITEM_UNINIT if item did not
 *              exist in NV, NV_OPER_FAILED if failure.
 */
uint8 zgInit( void )
{
  uint8  setDefault = FALSE;
  uint8  status;
  
#ifdef NV_RESTORE
  // Do we want to default the Config state values
  if ( zgReadStartupOptions() & ZCD_STARTOPT_DEFAULT_CONFIG_STATE )
  {
    setDefault = TRUE;
  }
#else
    setDefault = TRUE;
#endif
  
  status = osal_nv_item_init(ZCD_NV_BDBNODEISONANETWORK,sizeof(bdbAttributes.bdbNodeIsOnANetwork),&bdbAttributes.bdbNodeIsOnANetwork);

  //Force to reset state if device is forced to FN
  if((status == SUCCESS) && setDefault)
  {
    bdb_setNodeIsOnANetwork(false);
  }
  
#if defined (FEATURE_SYSTEM_STATS) 
  {
    // This sections tracks the number of resets
    uint16 bootCnt = 0;

    // Update the Boot Counter
    if ( osal_nv_item_init( ZCD_NV_BOOTCOUNTER, sizeof(bootCnt), &bootCnt ) == ZSUCCESS )
    {
      // Get the old value from NV memory
      osal_nv_read( ZCD_NV_BOOTCOUNTER, 0, sizeof(bootCnt), &bootCnt );
    }

    // Increment the Boot Counter and store it into NV memory
    if ( setDefault )
    {
      bootCnt = 0;
    }
    else
    {
      bootCnt++;
    }

    osal_nv_write( ZCD_NV_BOOTCOUNTER, 0, sizeof(bootCnt), &bootCnt );
  }
#endif  // FEATURE_SYSTEM_STATS

  zgUpgradeNVItems();

  // Initialize the Extended PAN ID as my own extended address
  ZMacGetReq( ZMacExtAddr, zgExtendedPANID );

  // Initialize the items table
  zgInitItems( setDefault );

#ifndef NONWK
  if ( ZG_SECURE_ENABLED )
  {
    // Initialize the Pre-Configured Key to the default key
    zgPreconfigKeyInit( setDefault );

    // Initialize NV items for all Keys: NWK, APS, TCLK and Master
    ZDSecMgrInitNVKeyTables( setDefault );
  }
#endif // NONWK

#ifdef UPGRADE_SECURITY_NV_ITEMS
  zgUpgradeSecurityNVItems();
#endif // UPGRADE_SECURITY_NV_ITEMS
  
  // Clear the Config State default
  if ( setDefault )
  {
    zgWriteStartupOptions( ZG_STARTUP_CLEAR, ZCD_STARTOPT_DEFAULT_CONFIG_STATE );
  }
  
  return ( ZSUCCESS );
}

/*********************************************************************
 * @fn          zgInitItems
 *
 * @brief       Initializes RAM variables from NV.  If NV items don't
 *              exist, then the NV is initialize with what is in RAM
 *              variables.
 *
 * @param       none
 *
 * @return      none
 */
void zgInitItems( uint8 setDefault )
{
  uint8  i = 0;

  if ( ZG_BUILD_COORDINATOR_TYPE && ( zgApsLinkKeyType == ZG_UNIQUE_LINK_KEY ) )
  {
    zgUseDefaultTCLK = FALSE;
  }
  else
  {
    // Most of the time default TCLK will be used
    zgUseDefaultTCLK = TRUE;
  }

  while ( zgItemTable[i].id != 0x00 )
  {
    // Initialize the item
    zgItemInit( zgItemTable[i].id, zgItemTable[i].len, zgItemTable[i].buf, setDefault  );

    // Move on to the next item
    i++;
  }
}

/*********************************************************************
 * @fn          zgReadStartupOptions
 *
 * @brief       Reads the ZCD_NV_STARTUP_OPTION NV Item.
 *
 * @param       none
 *
 * @return      the ZCD_NV_STARTUP_OPTION NV item
 */
uint8 zgReadStartupOptions( void )
{
  // Default to Use Config State and Use Network State
  uint8 startupOption = 0;

  // This should have been done in ZMain.c, but just in case.
  if ( osal_nv_item_init( ZCD_NV_STARTUP_OPTION,
                              sizeof(startupOption),
                              &startupOption ) == ZSUCCESS )
  {
    // Read saved startup control
    osal_nv_read( ZCD_NV_STARTUP_OPTION,
                  0,
                  sizeof( startupOption ),
                  &startupOption);
  }
  return ( startupOption );
}

/*********************************************************************
 * @fn          zgWriteStartupOptions
 *
 * @brief       Writes bits into the ZCD_NV_STARTUP_OPTION NV Item.
 *
 * @param       action - ZG_STARTUP_SET set bit, ZG_STARTUP_CLEAR to
 *               clear bit. The set bit is an OR operation, and the
 *               clear bit is an AND ~(bitOptions) operation.
 *
 * @param       bitOptions - which bits to perform action on:
 *                      ZCD_STARTOPT_DEFAULT_CONFIG_STATE
 *                      ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *
 * @return      ZSUCCESS if successful
 */
uint8 zgWriteStartupOptions( uint8 action, uint8 bitOptions )
{
  uint8 status;
  uint8 startupOptions = 0;

  status = osal_nv_read( ZCD_NV_STARTUP_OPTION,
                0,
                sizeof( startupOptions ),
                &startupOptions );

  if ( status == ZSUCCESS )
  {
    if ( action == ZG_STARTUP_SET )
    {
      // Set bits
      startupOptions |= bitOptions;
    }
    else
    {
      // Clear bits
      startupOptions &= (bitOptions ^ 0xFF);
    }

    // Changed?
    status = osal_nv_write( ZCD_NV_STARTUP_OPTION,
                 0,
                 sizeof( startupOptions ),
                 &startupOptions );
  }

  return ( status );
}

/*********************************************************************
 * @fn          zgSetItem
 *
 * @brief       Set RAM variables from set-NV, if it exist in the zgItemTable
 *
 * @param       id - NV ID
 *              len - NV item length
 *              buf - pointer to the input buffer
 *
 * @return      none
 */
void zgSetItem( uint16 id, uint16 len, void *buf )
{

  uint8  i = 0;

  // Look up the NV item table
  while ( zgItemTable[i].id != 0x00 )
  {
    if( zgItemTable[i].id == id )
    {
      if ( zgItemTable[i].len == len )
      {
        osal_memcpy( zgItemTable[i].buf, buf, len );
      }
      break;
    }
    // Move on to the next item
    i++;
  }
}

#ifndef NONWK
/*********************************************************************
 * @fn       zgPreconfigKeyInit()
 *
 * @brief
 *
 *   Initialize ZCD_NV_PRECFGKEY NV item. If the item doesn't exist in NV memory,
 *   write the system default (value passed in) into NV memory. But if
 *   it exists do not overwrite it.
 *
 *   Also, if setDefault is TRUE and the item exists, we will write
 *   the default value to NV space.
 *
 * @param   setDefault - TRUE to set default
 *
 * @return  ZSUCCESS if successful, NV_ITEM_UNINIT if item did not
 *          exist in NV, NV_OPER_FAILED if failure.
 */
static uint8 zgPreconfigKeyInit( uint8 setDefault )
{
  uint8              zgPreConfigKey[SEC_KEY_LEN];
  uint8              status;
  
  //NWK KEY
  //if nwk key is set to zeros, then generate a random key and use it
  osal_memset(zgPreConfigKey,0,SEC_KEY_LEN);
  
#if (ZG_BUILD_RTR_TYPE)
  if(ZG_DEVICE_RTR_TYPE)
  {  
    if(osal_memcmp(defaultKey, zgPreConfigKey,SEC_KEY_LEN))
    {
      ZDSecMgrGenerateRndKey(zgPreConfigKey);
    }
    else
    {
      // Initialize the Pre-Configured Key to the default key
      osal_memcpy( zgPreConfigKey, defaultKey, SEC_KEY_LEN );
    }
  }
#endif
  
  
  // If the item doesn't exist in NV memory, create and initialize it
  status = osal_nv_item_init( ZCD_NV_PRECFGKEY, SEC_KEY_LEN, zgPreConfigKey );
  if ( status == ZSUCCESS )
  {
    if ( setDefault )
    {
      // Write the default value back to NV
      status =  osal_nv_write( ZCD_NV_PRECFGKEY, 0, SEC_KEY_LEN, zgPreConfigKey );
    }
  }
  
  // clear local copy of default key
  osal_memset(zgPreConfigKey, 0x00, SEC_KEY_LEN);

  return (status);
}
#endif

/*********************************************************************
 * @fn       zgUpgradeNVItems()
 *
 * @brief
 *
 *   Function that upgrades NV Items that have changed.
 *
 * @param   none
 *
 * @return  none
 */
static void zgUpgradeNVItems( void )
{
#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  {
    // This conversion will only happen if the old poll rate exists and
    // the new poll rate doesn't exist.  It will read the old poll rate,
    // convert it to the new poll rate, create the new poll rate NV item,
    // then delete the old poll rate NV item.
    uint16 oldNvLen;
    uint16 newNvLen;

    // Use the length of the NV items to determine if they exist
    oldNvLen = osal_nv_item_len( ZCD_NV_POLL_RATE_OLD16 );
    newNvLen = osal_nv_item_len( ZCD_NV_POLL_RATE );
    if ( (newNvLen == 0) && (oldNvLen == sizeof ( uint16 )) )
    {
      // The old poll rate exists, so read it and convert to the new 32 bit poll rate
      uint16 oldPollRate;
      osal_nv_read( ZCD_NV_POLL_RATE_OLD16, 0, sizeof(uint16), &oldPollRate );
      zgPollRate = (uint32)oldPollRate;
      osal_nv_item_init( ZCD_NV_POLL_RATE, sizeof(zgPollRate), &zgPollRate );
      osal_nv_delete( ZCD_NV_POLL_RATE_OLD16, oldNvLen );
    }
  }
#endif // ZCD_NV_POLL_RATE_OLD16
}

/*********************************************************************
 * @fn       zgUpgradeSecurityNVItems()
 *
 * @brief
 *
 *   Function that upgrades NV Items that have changed.
 *
 * @param   none
 *
 * @return  none
 */
#ifdef UPGRADE_SECURITY_NV_ITEMS
static void zgUpgradeSecurityNVItems( void )
{
  //Nv configuration to allow upgrade from HA1.2 to Z3.0

  uint8 isOnANetwork = 1;
  //Read bdb attribute
  if(osal_nv_read(ZCD_NV_BDBNODEISONANETWORK, 0, sizeof(uint8), &isOnANetwork) == SUCCESS)
  {
    //After upgrade, this parameter will be 0, then update properly as it is in a network.
    //If the device was not in a network and is upgraded, the BDB will handle the network 
    //configuration status and will performing FN reset.
    
    nwkSecMaterialDesc_t  nwkSecMaterialDesc;
    nwkActiveKeyItems     keyItems;
    uint8                 keyAttributes;
    uint8                 tempKey[SEC_KEY_LEN];
    
    osal_memset(tempKey, 0x00, SEC_KEY_LEN);
    
    //Get the network frame counter from old NV item, also check if old key exists.
    //if it doesn't, we can assume that this is factory new Z3.0 device instead of 
    //upgraded ZHA 1.2 device
    osal_nv_read( ZCD_NV_NWKKEY, 0, sizeof( nwkActiveKeyItems ),(void *)&keyItems );
    
    if( osal_memcmp(tempKey, keyItems.active.key, SEC_KEY_LEN) )
    {
      // device is factory new, don't do anything 
    }
    // else if node is not on a network currently and is not factory new Z3.0 device, 
    // it must be ZHA 1.2 -> Z3.0.1 upgraded device
    else if(!isOnANetwork)
    {
      //If none of these parameters can be configured by Nv operations BDB will be able to initialize or handle.
      
      //set bdbNodeIsOnANetwork attribute to 1 indicating that we need to resume our network state
      isOnANetwork = 0x01;
      osal_nv_write( ZCD_NV_BDBNODEISONANETWORK, 0, sizeof(uint8), &isOnANetwork );
      
      //Set the network frame counter in new NV item, frame counter will be incremented by 1000+250 in ZDApp_RestoreNwkSecMaterial() 
      //to ensure that its outgoing packets can be used by devices in its previous network
      nwkSecMaterialDesc.FrameCounter = keyItems.frameCounter; 
      
      //Retrieve Extended PANID from NIB in NV
      osal_nv_read( ZCD_NV_NIB, osal_offsetof(nwkIB_t,extendedPANID), Z_EXTADDR_LEN, &nwkSecMaterialDesc.extendedPanID );
      
      //Set the BDB network security material, BDB state machine will restore network state if this is a valid entry
      osal_nv_write( ZCD_NV_NWK_SEC_MATERIAL_TABLE_START, 0, sizeof(nwkSecMaterialDesc_t), &nwkSecMaterialDesc );  
      
      //indicate to BDB that our previous nwk was non-R21, so don't expect TC Link Key exchange
      keyAttributes = ZG_NON_R21_NWK_JOINED;
      osal_nv_write(ZCD_NV_TCLK_TABLE_START, osal_offsetof(APSME_TCLKDevEntry_t,keyAttributes), sizeof(uint8), &keyAttributes); 
    }
  }
}
#endif // UPGRADE_SECURITY_NV_ITEMS
/*********************************************************************
*********************************************************************/
