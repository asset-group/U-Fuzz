/**************************************************************************************************
  Filename:       ZDApp.h
  Revised:        $Date: 2015-02-12 15:03:08 -0800 (Thu, 12 Feb 2015) $
  Revision:       $Revision: 42535 $

  Description:    This file contains the interface to the Zigbee Device Application. This is the
                  Application part that the use can change. This also contains the Task functions.


  Copyright 2004-2015 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZDAPP_H
#define ZDAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "ZMAC.h"
#include "NLMEDE.h"
#include "APS.h"
#include "AF.h"
#include "ZDProfile.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Set this value for use in choosing a PAN to join
// modify ZDApp.c to enable this...
#define ZDO_CONFIG_MAX_BO             15

  // Task Events
#define ZDO_NETWORK_INIT          0x0001
#define ZDO_NETWORK_START         0x0002
#define ZDO_DEVICE_RESET          0x0004
#define ZDO_COMMAND_CNF           0x0008
#define ZDO_STATE_CHANGE_EVT      0x0010
#define ZDO_ROUTER_START          0x0020
#define ZDO_NEW_DEVICE            0x0040
#define ZDO_DEVICE_AUTH           0x0080
#define ZDO_NWK_UPDATE_NV         0x0100
#define ZDO_FRAMECOUNTER_CHANGE   0x0200
#define ZDO_TCLK_FRAMECOUNTER_CHANGE  0x0400
#define ZDO_APS_FRAMECOUNTER_CHANGE   0x0800
#if defined ( ZDP_BIND_VALIDATION )
#define ZDO_PENDING_BIND_REQ_EVT      0x1000
#endif
#define ZDO_REJOIN_BACKOFF        0x2000
#define ZDO_PARENT_ANNCE_EVT      0x4000
#define ZDO_VOLTAGE_CHECK         0x8000
// Incoming to ZDO
#define ZDO_NWK_DISC_CNF        0x01
#define ZDO_NWK_JOIN_IND        0x02
#define ZDO_NWK_JOIN_REQ        0x03
#define ZDO_TRANSPORT_KEY_IND   0x04
#define ZDO_UPDATE_DEVICE_IND   0x05
#define ZDO_REMOVE_DEVICE_IND   0x06
#define ZDO_REQUEST_KEY_IND     0x07
#define ZDO_SWITCH_KEY_IND      0x08
#define ZDO_VERIFY_KEY_IND      0x09

//  ZDO command message fields
#define ZDO_CMD_ID     0
#define ZDO_CMD_ID_LEN 1

//  ZDO security message fields
#define ZDO_TRANSPORT_KEY_IND_LEN   \
  ((uint8)                          \
   (sizeof(ZDO_TransportKeyInd_t) ) )

#define ZDO_UPDATE_DEVICE_IND_LEN   \
  ((uint8)                          \
   (sizeof(ZDO_UpdateDeviceInd_t) ) )

#define ZDO_REMOVE_DEVICE_IND_LEN   \
  ((uint8)                          \
   (sizeof(ZDO_RemoveDeviceInd_t) ) )

#define ZDO_REQUEST_KEY_IND_LEN   \
  ((uint8)                        \
   (sizeof(ZDO_RequestKeyInd_t) ) )

#define ZDO_VERIFY_KEY_IND_LEN   \
  ((uint8)                        \
   (sizeof(ZDO_VerifyKeyInd_t) ) )

#define ZDO_SWITCH_KEY_IND_LEN   \
  ((uint8)                       \
   (sizeof(ZDO_SwitchKeyInd_t) ) )

#define ZDO_CONFIRM_KEY_IND_LEN     \
  ((uint8)                          \
   (sizeof(APSME_ConfirmKeyReq_t) ) )

#define NWK_RETRY_DELAY                 1000   // in milliseconds

#if defined ( ZDP_BIND_VALIDATION )
#define AGE_OUT_PEND_BIND_REQ_DELAY     1000   // One second interval to age out the record

#if !defined MAX_TIME_ADDR_REQ
#define MAX_TIME_ADDR_REQ           5      // Maximum number of seconds
#endif
#endif

#define ZDO_MATCH_DESC_ACCEPT_ACTION    1   // Message field

#if !defined NUM_DISC_ATTEMPTS
#define NUM_DISC_ATTEMPTS           4
#endif

// ZDOInitDevice init modes
#define ZDO_INITDEV_CENTRALIZED  0x00
#define ZDO_INITDEV_DISTRIBUTED  0x01

// ZDOInitDevice return values
#define ZDO_INITDEV_RESTORED_NETWORK_STATE      0x00
#define ZDO_INITDEV_NEW_NETWORK_STATE           0x01
#define ZDO_INITDEV_LEAVE_NOT_STARTED           0x02

#if defined ( MANAGED_SCAN )
  // Only use in a battery powered device

  // This is the number of times a channel is scanned at the shortest possible
  // scan time (which is 30 MS (2 x 15).  The idea is to scan one channel at a
  // time (from the channel list), but scan it multiple times.
  #define MANAGEDSCAN_TIMES_PRE_CHANNEL         5
  #define MANAGEDSCAN_DELAY_BETWEEN_SCANS       150   // milliseconds

extern uint8 zdoDiscCounter;

#endif // MANAGED_SCAN

// Use the following to delay application data request after startup.
#define ZDAPP_HOLD_DATA_REQUESTS_TIMEOUT        0 // in milliseconds

// Init ZDO, but hold and wait for application to start the joining or
// forming network
#define ZDO_INIT_HOLD_NWK_START       0xFFFF

#if !defined( MAX_NWK_FRAMECOUNTER_CHANGES )
  // The number of times the frame counter can change before
  // saving to NV
  #define MAX_NWK_FRAMECOUNTER_CHANGES    1000
#endif
    
#if !defined( NWK_FRAMECOUNTER_CHANGES_RESTORE_DELTA )
// Additional counts to add to the frame counter when restoring from NV
// This amount is in addition to MAX_NWK_FRAMECOUNTER_CHANGES
#define NWK_FRAMECOUNTER_CHANGES_RESTORE_DELTA    250
#endif

#define STACK_PROFILE_MAX 2

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  DEV_HOLD,                                // Initialized - not started automatically
  DEV_INIT,                                // Initialized - not connected to anything
  DEV_NWK_DISC,                            // Discovering PAN's to join
  DEV_NWK_JOINING,                         // Joining a PAN
  DEV_NWK_SEC_REJOIN_CURR_CHANNEL,         // ReJoining a PAN in secure mode scanning in current channel, only for end devices
  DEV_END_DEVICE_UNAUTH,                   // Joined but not yet authenticated by trust center
  DEV_END_DEVICE,                          // Started as device after authentication
  DEV_ROUTER,                              // Device joined, authenticated and is a router
  DEV_COORD_STARTING,                      // Started as Zigbee Coordinator
  DEV_ZB_COORD,                            // Started as Zigbee Coordinator
  DEV_NWK_ORPHAN,                          // Device has lost information about its parent..
  DEV_NWK_KA,                              // Device is sending KeepAlive message to its parent
  DEV_NWK_BACKOFF,                         // Device is waiting before trying to rejoin
  DEV_NWK_SEC_REJOIN_ALL_CHANNEL,          // ReJoining a PAN in secure mode scanning in all channels, only for end devices
  DEV_NWK_TC_REJOIN_CURR_CHANNEL,          // ReJoining a PAN in Trust center mode scanning in current channel, only for end devices
  DEV_NWK_TC_REJOIN_ALL_CHANNEL            // ReJoining a PAN in Trust center mode scanning in all channels, only for end devices
} devStates_t;

typedef enum
{
  ZDO_SUCCESS,
  ZDO_FAIL
} zdoStatus_t;


typedef struct
{
  osal_event_hdr_t hdr;
  uint8       dstAddrDstEP;
  zAddrType_t dstAddr;
  uint8       dstAddrClusterIDLSB;
  uint8       dstAddrClusterIDMSB;
  uint8       dstAddrRemove;
  uint8       dstAddrEP;
} ZDO_NewDstAddr_t;

//  ZDO security message types
typedef struct
{
  osal_event_hdr_t hdr;
  uint16           srcAddr;
  uint8            keyType;
  uint8            keySeqNum;
  uint8            key[SEC_KEY_LEN];
  uint8            srcExtAddr[Z_EXTADDR_LEN];
  uint8            initiator;
  uint8            secure;
} ZDO_TransportKeyInd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16           srcAddr;
  uint8            devExtAddr[Z_EXTADDR_LEN];
  uint16           devAddr;
  uint8            status;
} ZDO_UpdateDeviceInd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16           srcAddr;
  uint8            childExtAddr[Z_EXTADDR_LEN];
} ZDO_RemoveDeviceInd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16           srcAddr;
  uint8            keyType;
  uint8            partExtAddr[Z_EXTADDR_LEN];
} ZDO_RequestKeyInd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16           srcAddr;
  uint8            keySeqNum;
} ZDO_SwitchKeyInd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16           srcAddr;
  uint8            keyType;
  uint8            extAddr[Z_EXTADDR_LEN];
  uint8            keyHash[SEC_KEY_LEN];
  uint8            verifyKeyStatus;
} ZDO_VerifyKeyInd_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16 nwkAddr;
  uint8 numInClusters;
  uint16 *pInClusters;
  uint8 numOutClusters;
  uint16 *pOutClusters;
} ZDO_MatchDescRspSent_t;

typedef struct
{
  osal_event_hdr_t hdr;
  uint16 shortAddr;
} ZDO_AddrChangeInd_t;

/* ZDO Indication Callback Registration */
typedef void* (*pfnZdoCb)( void *param );

/* ZDO Indication callback ID */
enum
{
  ZDO_SRC_RTG_IND_CBID,
  ZDO_CONCENTRATOR_IND_CBID,
  ZDO_NWK_DISCOVERY_CNF_CBID,
  ZDO_BEACON_NOTIFY_IND_CBID,
  ZDO_JOIN_CNF_CBID,
  ZDO_LEAVE_CNF_CBID,
  ZDO_LEAVE_IND_CBID,
  ZDO_PERMIT_JOIN_CBID,
  ZDO_TC_DEVICE_CBID,
  MAX_ZDO_CB_FUNC               // Must be at the bottom of the list
};

typedef struct
{
  uint16 srcAddr;
  uint8  relayCnt;
  uint16 *pRelayList;
} zdoSrcRtg_t;

typedef struct
{
  uint16 nwkAddr;
  uint8  *extAddr;
  uint8  pktCost;
} zdoConcentratorInd_t;

/* Keep the same format as NLME_beaconInd_t */
typedef struct
{
  uint16 sourceAddr;  /* Short address of the device sends the beacon */
  uint16 panID;
  uint8  logicalChannel;
  uint8	 permitJoining;
  uint8	 routerCapacity;
  uint8	 deviceCapacity;
  uint8  protocolVersion;
  uint8  stackProfile ;
  uint8	 LQI ;
  uint8  depth ;
  uint8  updateID;
  uint8  extendedPanID[8];
} zdoBeaconInd_t;

typedef struct
{
  uint8  status;
  uint16 deviceAddr;
  uint16 parentAddr;
} zdoJoinCnf_t;

typedef struct
{
  uint8       srcAddress[Z_EXTADDR_LEN];
  uint8       srcEndpoint;
  uint16      clusterID;
  zAddrType_t dstAddress;
  uint8       dstEndpoint;
} ZDO_BindUnbindReq_t;

typedef struct
{
  uint16 nwkAddr;
  uint8 extAddr[Z_EXTADDR_LEN];
  uint16 parentAddr;
} ZDO_TC_Device_t;

#if defined ( ZDP_BIND_VALIDATION )
typedef struct
{
  ZDO_BindUnbindReq_t bindReq;
  zAddrType_t         srcAddr;
  uint8               securityUse;
  uint8               transSeq;
  uint8               age;
} ZDO_PendingBindReq_t;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8 ZDAppTaskID;
extern uint8 nwkStatus;
extern devStates_t devState;

/* Always kept up to date by the network state changed logic, so use this addr
 * in function calls the require my network address as one of the parameters.
 */
extern zAddrType_t ZDAppNwkAddr;
extern uint8 saveExtAddr[];  // Loaded with value by ZDApp_Init().

extern uint8 zdappMgmtNwkDiscRspTransSeq;
extern uint8 zdappMgmtNwkDiscReqInProgress;
extern zAddrType_t zdappMgmtNwkDiscRspAddr;
extern uint8 zdappMgmtNwkDiscStartIndex;
extern uint8 zdappMgmtSavedNwkState;

extern uint8 ZDO_UseExtendedPANID[Z_EXTADDR_LEN];

extern uint32 runtimeChannel;

/*********************************************************************
 * FUNCTIONS - API
 */

extern void ZDO_AddrChangeIndicationCB( uint16 newAddr );

/*********************************************************************
 * Task Level Control
 */
  /*
   * ZdApp Task Initialization Function
   */
  extern void ZDApp_Init( uint8 task_id );

  /*
   * ZdApp Task Event Processing Function
   */
  extern UINT16 ZDApp_event_loop( uint8 task_id, UINT16 events );

/*********************************************************************
 * Application Level Functions
 */

/*
 *  Start the device in the network.  This function will read
 *   ZCD_NV_STARTUP_OPTION (NV item) to determine whether or not to
 *   restore the network state of the device.
 *
 *  startDelay - timeDelay to start device (in milliseconds).
 *      There is a jitter added to this delay:
 *              ((NWK_START_DELAY + startDelay)
 *              + (osal_rand() & EXTENDED_JOINING_RANDOM_MASK))
 *
 *  NOTE:   If the application would like to force a "new" join, the
 *          application should set the ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *          bit in the ZCD_NV_STARTUP_OPTION NV item before calling
 *          this function.
 *
 *  returns:
 *    ZDO_INITDEV_RESTORED_NETWORK_STATE  - The device's network state was
 *          restored.
 *    ZDO_INITDEV_NEW_NETWORK_STATE - The network state was initialized.
 *          This could mean that ZCD_NV_STARTUP_OPTION said to not restore, or
 *          it could mean that there was no network state to restore.
 */
#define ZDOInitDevice(a)  ZDOInitDeviceEx(a,ZDO_INITDEV_CENTRALIZED)  
  
/*
 *  Start the device in the network.  This function will read
 *   ZCD_NV_STARTUP_OPTION (NV item) to determine whether or not to
 *   restore the network state of the device.
 *
 *  startDelay - timeDelay to start device (in milliseconds).
 *      There is a jitter added to this delay:
 *              ((NWK_START_DELAY + startDelay)
 *              + (osal_rand() & EXTENDED_JOINING_RANDOM_MASK))
 *
 *  mode - ZDO_INITDEV_CENTRALIZED or ZDO_INITDEV_DISTRIBUTED to specify
 *         which mode should the device start with (only has effect on Router devices)
 *
 *
 *  NOTE:   If the application would like to force a "new" join, the
 *          application should set the ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *          bit in the ZCD_NV_STARTUP_OPTION NV item before calling
 *          this function.
 *
 *  returns:
 *    ZDO_INITDEV_RESTORED_NETWORK_STATE  - The device's network state was
 *          restored.
 *    ZDO_INITDEV_NEW_NETWORK_STATE - The network state was initialized.
 *          This could mean that ZCD_NV_STARTUP_OPTION said to not restore, or
 *          it could mean that there was no network state to restore.
 */ 
uint8 ZDOInitDeviceEx( uint16 startDelay, uint8 mode);


/*
 * Sends an osal message to ZDApp with parameter as the msg data byte.
 */
extern void ZDApp_SendEventMsg( uint8 cmd, uint8 len, uint8 *buf );

/*
 * Start the network formation process
 *    delay - millisecond wait before
 */
extern void ZDApp_NetworkInit( uint16 delay );

/*
 * Request a network discovery
 */
extern ZStatus_t ZDApp_NetworkDiscoveryReq( uint32 scanChannels, uint8 scanDuration);

/*
 * Request the device to join a parent on a network
 */
extern ZStatus_t ZDApp_JoinReq( uint8 channel, uint16 panID,
                                uint8 *extendedPanID, uint16 chosenParent,
                                uint8 parentDepth, uint8 stackProfile);

#if defined ( ZDP_BIND_VALIDATION )
/*
 * Find an empty slot to store pending Bind Request
 */
extern ZDO_PendingBindReq_t *ZDApp_GetEmptyPendingBindReq( void );
#endif

/*********************************************************************
 * Callback FUNCTIONS - API
 */
/*********************************************************************
 * Call Back Functions from NWK  - API
 */

/*
 * ZDO_NetworkDiscoveryConfirmCB - scan results
 */
extern ZStatus_t ZDO_NetworkDiscoveryConfirmCB( uint8 status );

/*
 * ZDO_NetworkFormationConfirm - results of the request to
 *              initialize a coordinator in a network
 */
extern void ZDO_NetworkFormationConfirmCB( ZStatus_t Status );

/*
 * ZDApp_beaconIndProcessing - processes the incoming beacon
 *              indication.
 */
extern void ZDO_beaconNotifyIndCB( NLME_beaconInd_t *beacon );

/*
 * ZDO_JoinConfirmCB - results of its request to join itself or another
 *              device to a network
 */
extern void ZDO_JoinConfirmCB( uint16 PanId, ZStatus_t Status );

/*
 * ZDO_JoinIndicationCB - notified of a remote join request
 */
ZStatus_t ZDO_JoinIndicationCB(uint16 ShortAddress, uint8 *ExtendedAddress,
                                 uint8 CapabilityFlags, uint8 type);

/*
 * ZDO_ConcentratorIndicationCB - notified of a concentrator existence
 */
extern void ZDO_ConcentratorIndicationCB( uint16 nwkAddr, uint8 *extAddr, uint8 pktCost );

/*
 * ZDO_StartRouterConfirm -  results of the request to
 *              start functioning as a router in a network
 */
extern void ZDO_StartRouterConfirmCB( ZStatus_t Status );

/*
 * ZDO_LeaveCnf - results of the request for this or a child device
 *                to leave
 */
extern void ZDO_LeaveCnf( NLME_LeaveCnf_t* cnf );

/*
 * ZDO_LeaveInd - notified of a remote leave request or indication
 */
extern void ZDO_LeaveInd( NLME_LeaveInd_t* ind );

/*
 * ZDO_SyncIndicationCB - notified of sync loss with parent
 */
extern void ZDO_SyncIndicationCB( uint8 type, uint16 shortAddr );

/*
 * ZDO_ManytoOneFailureIndicationCB - notified a many-to-one route failure
 */
extern void ZDO_ManytoOneFailureIndicationCB( void );

/*
 * ZDO_PollConfirmCB - notified of poll confirm
 */
extern void ZDO_PollConfirmCB( uint8 status );

/*
 * ZDO_NetworkStatusCB - notified of received network status messages.
 */
extern void ZDO_NetworkStatusCB( uint16 nwkDstAddr, uint8 statusCode, uint16 dstAddr );

/*
 * ZDO_PermitJoinCB - When there is a change in Permit Join status
 */
extern void ZDO_PermitJoinCB( uint8 duration );




/*********************************************************************
 * Call Back Functions from Security  - API
 */
extern ZStatus_t ZDO_UpdateDeviceIndication( uint8 *extAddr, uint8 status );



/*
 * ZDApp_InMsgCB - Allow the ZDApp to handle messages that are not supported
 */
extern void ZDApp_InMsgCB( zdoIncomingMsg_t *inMsg );

extern void ZDO_StartRouterConfirm( ZStatus_t Status );

/*********************************************************************
 * Call Back Functions from Apllication  - API
 */
/*
 * ZDO_NwkUpdateCB - Network state info has changed
 */
extern void ZDApp_NwkStateUpdateCB( void );

/*********************************************************************
 * ZDO Control Functions
 */

/*
 * ZDApp_ChangeMatchDescRespPermission
 *    - Change the Match Descriptor Response permission.
 */
extern void ZDApp_ChangeMatchDescRespPermission( uint8 endpoint, uint8 action );

/*
 * ZDApp_SaveNwkKey
 *     - Save off the Network key information.
 */
extern void ZDApp_SaveNwkKey( void );

/*
 * ZDApp_ResetNwkKey
 *    - Re initialize the NV Nwk Key
 */
extern void ZDApp_ResetNwkKey( void );

/*
 * ZDApp_StartJoiningCycle
 *    - Starts the joining cycle of a device.  This will only continue an
 *      already started (or stopped) joining cycle.
 *
 *    returns  TRUE if joining started, FALSE if not in joining or rejoining
 */
extern uint8 ZDApp_StartJoiningCycle( void );

/*
 * ZDApp_StopJoiningCycle
 *    - Stops the joining or rejoining process of a device.
 *
 *    returns  TRUE if joining stopped, FALSE if joining or rejoining
 */
extern uint8 ZDApp_StopJoiningCycle( void );

/*
 * ZDApp_AnnounceNewAddress
 *   - Announce a new address
 */
extern void ZDApp_AnnounceNewAddress( void );

/*
 * ZDApp_SendParentAnnce
 *   - Send Parent Announce message
 */
extern void ZDApp_SendParentAnnce( void );

/*
 * ZDApp_NVUpdate - Initiate an NV update
 */
extern void ZDApp_NVUpdate( void );

/*
 * Callback from network layer when coordinator start has a conflict with
 * an existing PAN ID.
 */
extern uint16 ZDApp_CoordStartPANIDConflictCB( uint16 panid );

/*
 * ZDApp_LeaveReset
 *    - Setup a device reset due to a leave indication/confirm
 */
extern void ZDApp_LeaveReset( uint8 ra );

/*
 * ZDApp_LeaveCtrlReset
 *    - Re-initialize the leave control logic
 */
extern void ZDApp_LeaveCtrlReset( void );

/*
 * ZDApp_DeviceConfigured
 *    - Check to see if the local device is configured
 */
extern uint8 ZDApp_DeviceConfigured( void );

/*
 * ZDApp_ForceConcentratorChange
 *    - reset zgConcentratorEnable and zgConcentratorDiscoveryTime from NV and set nwk event.
 */
extern void ZDApp_ForceConcentratorChange( void );

/*********************************************************************
 * @fn          ZDApp_SecInit
 *
 * @brief       ZDApp initialize security.
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
extern void ZDApp_SecInit( uint8 state );

/*********************************************************************
 * @fn          ZDO_SrcRtgIndCB
 *
 * @brief       This function notifies the ZDO available src route record received.
 *
 * @param       srcAddr - source address of the source route
 * @param       relayCnt - number of devices in the relay list
 * @param       relayList - relay list of the source route
 *
 * @return      none
 */
extern void ZDO_SrcRtgIndCB (uint16 srcAddr, uint8 relayCnt, uint16* pRelayList );

/*********************************************************************
 * @fn          ZDO_RegisterForZdoCB
 *
 * @brief       Call this function to register the higher layer (for
 *              example, the Application layer or MT layer) with ZDO
 *              callbacks to get notified of some ZDO indication like
 *              existence of a concentrator or receipt of a source
 *              route record.
 *
 * @param       indID - ZDO Indication ID
 * @param       pFn   - Callback function pointer
 *
 * @return      ZSuccess - successful, ZInvalidParameter if not
 */
extern ZStatus_t ZDO_RegisterForZdoCB( uint8 indID, pfnZdoCb pFn );

/*********************************************************************
 * @fn          ZDO_DeregisterForZdoCB
 *
 * @brief       Call this function to de-register the higher layer (for
 *              example, the Application layer or MT layer) with ZDO
 *              callbacks to get notified of some ZDO indication like
 *              existence of a concentrator or receipt of a source
 *              route record.
 *
 * @param       indID - ZDO Indication ID
 *
 * @return      ZSuccess - successful, ZInvalidParameter if not
 */
extern ZStatus_t ZDO_DeregisterForZdoCB( uint8 indID );

/*********************************************************************
 * @fn          ZDApp_ChangeState
 *
 * @brief       Call this function to change the device state.
 *
 * @param       state - new state
 *
 * @return      none
 */
extern void ZDApp_ChangeState( devStates_t state );

/*
 * ZDApp_SetRejoinScanDuration
 *    - Sets scan duration for rejoin for an end device
 *
 *    returns  none
 */
extern void ZDApp_SetRejoinScanDuration ( uint32 rejoinScanDuration);

/*
 * ZDApp_SetRejoinScanDuration
 *    - Sets rejoin backoff duration for rejoin for an end device
 *
 *    returns  none
 */
extern void ZDApp_SetRejoinBackoffDuration ( uint32 rejoinBackoffDuration);

/*
 * @brief   Restore the network frame counter associated to this ExtPanID and 
 *          increment it if found. This can only happens once per reset
 *
 *    returns  none
 */
extern void ZDApp_RestoreNwkSecMaterial(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDOBJECT_H */
