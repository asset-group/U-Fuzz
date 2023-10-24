/**************************************************************************************************
  Filename:       ZDApp.c
  Revised:        $Date: 2015-10-06 12:04:24 -0700 (Tue, 06 Oct 2015) $
  Revision:       $Revision: 44520 $

  Description:    This file contains the interface to the Zigbee Device Application. This is the
                  Application part that the user can change. This also contains the Task functions.


  Copyright 2004-2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
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
#include "ZMAC.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_Nv.h"
#include "AF.h"
#include "APSMEDE.h"
#include "NLMEDE.h"
#include "AddrMgr.h"
#include "ZDProfile.h"
#include "ZDObject.h"
#include "ZDConfig.h"
#include "ZDSecMgr.h"
#include "ZDApp.h"
#include "DebugTrace.h"
#include "nwk_util.h"
#include "OnBoard.h"
#include "ZGlobals.h"
#include "ZDNwkMgr.h"
#include "rtg.h"
   
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
#endif
   
#include "bdb.h"
#include "bdb_interface.h"

#include "ssp.h"

/* HAL */
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_key.h"

#if defined( MT_MAC_FUNC ) || defined( MT_MAC_CB_FUNC )
  #error "ERROR! MT_MAC functionalities should be disabled on ZDO devices"
#endif

/*********************************************************************
 * CONSTANTS
 */

#if !defined( NWK_START_DELAY )
  #define NWK_START_DELAY             100   // in milliseconds
#endif

#if !defined( LEAVE_RESET_DELAY )
  #define LEAVE_RESET_DELAY           5000  // in milliseconds
#endif

#if !defined( EXTENDED_JOINING_RANDOM_MASK )
  #define EXTENDED_JOINING_RANDOM_MASK 0x007F
#endif

#if !defined( BEACON_REQUEST_DELAY )
  #define BEACON_REQUEST_DELAY        100   // in milliseconds
#endif

#if !defined( BEACON_REQ_DELAY_MASK )
  #define BEACON_REQ_DELAY_MASK       0x007F
#endif

#define MAX_RESUME_RETRY            3

#define MAX_DEVICE_UNAUTH_TIMEOUT   10000  // 10 seconds

// Beacon Order Settings (see NLMEDE.h)
#define DEFAULT_BEACON_ORDER        BEACON_ORDER_NO_BEACONS
#define DEFAULT_SUPERFRAME_ORDER    DEFAULT_BEACON_ORDER

// Leave control bits
#define ZDAPP_LEAVE_CTRL_INIT 0
#define ZDAPP_LEAVE_CTRL_SET  1
#define ZDAPP_LEAVE_CTRL_RA   2

// Address Manager Stub Implementation
#define ZDApp_NwkWriteNVRequest AddrMgrWriteNVRequest


#if !defined ZDO_NV_SAVE_RFDs
#define ZDO_NV_SAVE_RFDs  TRUE
#endif

// Delay time before updating NWK NV data to force fewer writes during high activity.
#if ZDO_NV_SAVE_RFDs
#define ZDAPP_UPDATE_NWK_NV_TIME 700
#else
#define ZDAPP_UPDATE_NWK_NV_TIME 65000
#endif

// Timeout value to process New Devices
#define ZDAPP_NEW_DEVICE_TIME     600   // in ms


//ZDP_BIND_SKIP_VALIDATION, redefined as ZDP_BIND_VALIDATION
#if defined ( ZDP_BIND_VALIDATION )
#if !defined MAX_PENDING_BIND_REQ
#define MAX_PENDING_BIND_REQ 3
#endif
#endif

#ifdef LEGACY_ZDO_LEDS
#define zdoHalLedSet HalLedSet
#else
#define zdoHalLedSet(...)
#endif

/******************************************************************************
 * TYPEDEFS
 */
typedef struct
{
  void   *next;
  uint16 shortAddr;
  uint16 timeDelta;
} ZDAppNewDevice_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

#if defined( LCD_SUPPORTED )
  uint8 MatchRsps = 0;
#endif

zAddrType_t ZDAppNwkAddr;

uint8 zdappMgmtNwkDiscRspTransSeq;
uint8 zdappMgmtNwkDiscReqInProgress = FALSE;
zAddrType_t zdappMgmtNwkDiscRspAddr;
uint8 zdappMgmtNwkDiscStartIndex;
uint8 zdappMgmtSavedNwkState;

uint8 continueJoining = TRUE;

uint8  _tmpRejoinState;

// The extended PanID used in ZDO layer for rejoin.
uint8 ZDO_UseExtendedPANID[Z_EXTADDR_LEN];

pfnZdoCb zdoCBFunc[MAX_ZDO_CB_FUNC];

#if defined ( ZDP_BIND_VALIDATION )
ZDO_PendingBindReq_t *ZDAppPendingBindReq = NULL;
#endif

uint32 runtimeChannel;
uint8 FrameCounterUpdated = FALSE;
/*********************************************************************
 * EXTERNAL VARIABLES
 */

extern bool    requestNewTrustCenterLinkKey;
extern uint32  requestLinkKeyTimeout;
extern CONST   uint8 gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES;
/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void ZDApp_NetworkStartEvt( void );
void ZDApp_DeviceAuthEvt( void );
void ZDApp_SaveNetworkStateEvt( void );

uint8 ZDApp_ReadNetworkRestoreState( void );
uint8 ZDApp_RestoreNetworkState( void );
void ZDAppDetermineDeviceType( void );
void ZDApp_InitUserDesc( void );
void ZDAppCheckForHoldKey( void );
void ZDApp_ProcessOSALMsg( osal_event_hdr_t *msgPtr );
void ZDApp_ProcessNetworkJoin( void );
void ZDApp_SetCoordAddress( uint8 endPoint, uint8 dstEP );
uint8 ZDApp_RestoreNwkKey( uint8 incrFrmCnt );
networkDesc_t* ZDApp_NwkDescListProcessing(void);

void ZDApp_SecInit( uint8 state );
UINT16 ZDApp_ProcessSecEvent( uint8 task_id, UINT16 events );
void ZDApp_ProcessSecMsg( osal_event_hdr_t *msgPtr );

void ZDApp_SendMsg( uint8 taskID, uint8 cmd, uint8 len, uint8 *buf );

void ZDApp_ResetTimerStart( uint16 delay );
void ZDApp_ResetTimerCancel( void );
void ZDApp_LeaveCtrlInit( void );
void ZDApp_LeaveCtrlSet( uint8 ra );
uint8 ZDApp_LeaveCtrlBypass( void );
void ZDApp_LeaveCtrlStartup( devStates_t* state, uint16* startDelay );
void ZDApp_LeaveUpdate( uint16 nwkAddr, uint8* extAddr, uint8 removeChildren, uint8 rejoin );
void ZDApp_NodeProfileSync( uint8 stackProfile );
void ZDApp_ProcessMsgCBs( zdoIncomingMsg_t *inMsg );
void ZDApp_RegisterCBs( void );
void ZDApp_InitZdoCBFunc(void);
#if defined ( ZDP_BIND_VALIDATION )
void ZDApp_SetPendingBindDefault( ZDO_PendingBindReq_t *pendBindReq );
void ZDApp_InitPendingBind( void );
void ZDApp_ProcessPendingBindReq( uint8 *extAddr );
void ZDApp_AgeOutPendingBindEntry( void );
#endif

void ZDApp_SetParentAnnceTimer( void );
void ZDApp_StoreNwkSecMaterial(void);

/*********************************************************************
 * LOCAL VARIABLES
 */

uint8 ZDAppTaskID;
uint8 nwkStatus;
endPointDesc_t *ZDApp_AutoFindMode_epDesc = (endPointDesc_t *)NULL;
uint8 ZDApp_LeaveCtrl;

devStates_t devState = DEV_HOLD;

// previous rejoin state
  devStates_t prevDevState = DEV_NWK_SEC_REJOIN_CURR_CHANNEL;

#if ( ZG_BUILD_RTRONLY_TYPE ) || ( ZG_BUILD_ENDDEVICE_TYPE )
  devStartModes_t devStartMode = MODE_JOIN;     // Assume joining
  //devStartModes_t devStartMode = MODE_RESUME; // if already "directly joined"
                        // to parent. Set to make the device do an Orphan scan.
#else
  // Set the default to coodinator
  devStartModes_t devStartMode = MODE_HARD;
#endif

uint8 retryCnt = 0;

endPointDesc_t ZDApp_epDesc =
{
  ZDO_EP,
  0,
  &ZDAppTaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for ZDO
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

uint32 ZDApp_SavedPollRate = POLL_RATE;

ZDAppNewDevice_t *ZDApp_NewDeviceList = NULL;

/* "Hold Key" status saved during ZDAppCheckForHoldKey() */
static uint8 zdappHoldKeys;

/*********************************************************************
 * @fn      ZDApp_Init
 *
 * @brief   ZDApp Initialization function.
 *
 * @param   task_id - ZDApp Task ID
 *
 * @return  None
 */
void ZDApp_Init( uint8 task_id )
{
  // Save the task ID
  ZDAppTaskID = task_id;

  // Initialize the ZDO global device short address storage
  ZDAppNwkAddr.addrMode = Addr16Bit;
  ZDAppNwkAddr.addr.shortAddr = INVALID_NODE_ADDR;
  (void)NLME_GetExtAddr();  // Load the saveExtAddr pointer.

  // Initialize ZDO items and setup the device - type of device to create.
  ZDO_Init();

  // Register the endpoint description with the AF
  // This task doesn't have a Simple description, but we still need
  // to register the endpoint.
  afRegister( (endPointDesc_t *)&ZDApp_epDesc );

#if defined( ZDO_USERDESC_RESPONSE )
  ZDApp_InitUserDesc();
#endif // ZDO_USERDESC_RESPONSE

  // Initialize the ZDO callback function pointers zdoCBFunc[]
  ZDApp_InitZdoCBFunc();

  ZDApp_RegisterCBs();

#if defined ( ZDP_BIND_VALIDATION )
#if defined ( REFLECTOR )
  ZDApp_InitPendingBind();
#endif
#endif
} /* ZDApp_Init() */

/*********************************************************************
 * @fn          ZDApp_SecInit
 *
 * @brief       ZDApp initialize security.
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
void ZDApp_SecInit( uint8 state )
{
  uint8 zgPreConfigKey[SEC_KEY_LEN];

  if ( ZG_SECURE_ENABLED && ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
  {
    // Set the Trust Center bit
    ZDO_Config_Node_Descriptor.ServerMask |= PRIM_TRUST_CENTER;
  }

  // Initialize ZigBee Device Security Manager
  ZDSecMgrInit(state);

  if ( ZG_SECURE_ENABLED )
  {
    if ( state != ZDO_INITDEV_RESTORED_NETWORK_STATE )
    {
      if( _NIB.nwkKeyLoaded == FALSE )
      {
        if ( ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE          ) ||
             ( ( zgSecurityMode == ZG_SECURITY_RESIDENTIAL ) && zgPreConfigKeys )    )
        {
            ZDSecMgrReadKeyFromNv(ZCD_NV_PRECFGKEY, zgPreConfigKey);
            SSP_UpdateNwkKey( zgPreConfigKey, 0);
            SSP_SwitchNwkKey( 0 );

            // clear local copy of key
            osal_memset(zgPreConfigKey, 0x00, SEC_KEY_LEN);
        }
      }
    }

    // clean the new devices list when Security module is initialized
    if ( ZDApp_NewDeviceList != NULL )
    {
      ZDAppNewDevice_t *pNewDeviceNext;

      while ( ZDApp_NewDeviceList )
      {
        pNewDeviceNext = (ZDAppNewDevice_t *) ZDApp_NewDeviceList->next;
        osal_mem_free( ZDApp_NewDeviceList );
        ZDApp_NewDeviceList = pNewDeviceNext;
      }
    }
  }
}

/*********************************************************************
 * @fn      ZDApp_event_loop()
 *
 * @brief   Main event loop for Zigbee device objects task. This function
 *          should be called at periodic intervals.
 *
 * @param   task_id - Task ID
 * @param   events  - Bitmap of events
 *
 * @return  none
 */
UINT16 ZDApp_event_loop( uint8 task_id, UINT16 events )
{
  uint8 *msg_ptr;

  if ( events & SYS_EVENT_MSG )
  {
    while ( (msg_ptr = osal_msg_receive( ZDAppTaskID )) )
    {
      ZDApp_ProcessOSALMsg( (osal_event_hdr_t *)msg_ptr );

      // Release the memory
      osal_msg_deallocate( msg_ptr );
    }

    // Return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & ZDO_NETWORK_INIT )
  {
    // Initialize apps and start the network
    ZDApp_ChangeState( DEV_INIT );

    ZDO_StartDevice( (uint8)ZDO_Config_Node_Descriptor.LogicalType, devStartMode,
                     DEFAULT_BEACON_ORDER, DEFAULT_SUPERFRAME_ORDER );

    // Return unprocessed events
    return (events ^ ZDO_NETWORK_INIT);
  }

  if ( ZSTACK_ROUTER_BUILD )
  {
    if ( events & ZDO_NETWORK_START )
    {
      ZDApp_NetworkStartEvt();

      // Return unprocessed events
      return (events ^ ZDO_NETWORK_START);
    }

    if ( events & ZDO_ROUTER_START )
    {
      if ( nwkStatus == ZSuccess )
      {
        if ( devState == DEV_END_DEVICE )
        {
          ZDApp_ChangeState( DEV_ROUTER );
        }

        osal_pwrmgr_device( PWRMGR_ALWAYS_ON );

        if ( zgChildAgingEnable == TRUE )
        {
          // Once the device has changed its state to a ROUTER set the timer to send
          // Parent annce
          ZDApp_SetParentAnnceTimer();
        }
      }
      else
      {
        // remain as end device
      }
      osal_set_event( ZDAppTaskID, ZDO_STATE_CHANGE_EVT );

      // Return unprocessed events
      return (events ^ ZDO_ROUTER_START);
    }

    if ( events & ZDO_PARENT_ANNCE_EVT )
    {
      ZDApp_SendParentAnnce();

      // Return unprocessed events
      return (events ^ ZDO_PARENT_ANNCE_EVT);
    }
  }

  if ( ZSTACK_END_DEVICE_BUILD )
  {
    if ( events & ZDO_VOLTAGE_CHECK )
    {
      nwkPollCount = 0;
      OnBoard_CheckVoltage();

      return (events ^ ZDO_VOLTAGE_CHECK);
    }
  }

  if( events & ZDO_REJOIN_BACKOFF )
  {
    if( devState == DEV_NWK_BACKOFF )
    {
      ZDApp_ChangeState(DEV_NWK_DISC);
      // Restart scan for rejoin
      ZDApp_StartJoiningCycle();
      osal_start_timerEx( ZDAppTaskID, ZDO_REJOIN_BACKOFF, zgDefaultRejoinScan );
    }
    else
    {
      // Rejoin backoff, silent period
      ZDApp_ChangeState(DEV_NWK_BACKOFF);
      ZDApp_StopJoiningCycle();
      osal_start_timerEx( ZDAppTaskID, ZDO_REJOIN_BACKOFF, zgDefaultRejoinBackoff );
    }

    return ( events ^ ZDO_REJOIN_BACKOFF);
  }

  if ( events & ZDO_STATE_CHANGE_EVT )
  {
    ZDO_UpdateNwkStatus( devState );

    // At start up, do one MTO route discovery if the device is a concentrator
    if ( zgConcentratorEnable == TRUE )
    {
      // Start next event
      osal_start_timerEx( NWK_TaskID, NWK_MTO_RTG_REQ_EVT, 100 );
    }

    // Return unprocessed events
    return (events ^ ZDO_STATE_CHANGE_EVT);
  }

  if ( events & ZDO_COMMAND_CNF )
  {
    // User defined logic

    // Return unprocessed events
    return (events ^ ZDO_COMMAND_CNF);
  }

  if ( events & ZDO_NWK_UPDATE_NV )
  {
    // Save only in valid state
    if ( _NIB.nwkState == NWK_ROUTER || _NIB.nwkState == NWK_ENDDEVICE )
    {
      ZDApp_SaveNetworkStateEvt();
    }

    // Return unprocessed events
    return (events ^ ZDO_NWK_UPDATE_NV);
  }

  if ( events & ZDO_DEVICE_RESET )
  {
#ifdef ZBA_FALLBACK_NWKKEY
    if ( devState == DEV_END_DEVICE_UNAUTH )
    {
      ZDSecMgrFallbackNwkKey();
    }
    else
#endif
    {
      // Set the NV startup option to force a "new" join.
      zgWriteStartupOptions( ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE | ZCD_STARTOPT_DEFAULT_CONFIG_STATE );

      // The device has been in the UNAUTH state, so reset
      // Note: there will be no return from this call
      SystemResetSoft();
    }
  }

#if defined ( ZDP_BIND_VALIDATION )
  if ( events & ZDO_PENDING_BIND_REQ_EVT )
  {
#if defined ( REFLECTOR )
    ZDApp_AgeOutPendingBindEntry();
#endif
    // Return unprocessed events
    return (events ^ ZDO_PENDING_BIND_REQ_EVT);
  }
#endif

  if ( ZG_SECURE_ENABLED )
  {
    return ( ZDApp_ProcessSecEvent( task_id, events ) );
  }
  else
  {
    // Discard or make more handlers
    return 0;
  }
}

/*********************************************************************
 * @fn      ZDApp_ProcessSecEvent()
 *
 * @brief   Process incoming security events.
 *
 * @param   task_id - Task ID
 * @param   events  - Bitmap of events
 *
 * @return  none
 */
UINT16 ZDApp_ProcessSecEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter

  if ( ZSTACK_ROUTER_BUILD )
  {
    if ( events & ZDO_NEW_DEVICE )
    {
      // process the new device event
      if ( ZDApp_NewDeviceList )
      {
        ZDAppNewDevice_t *pNewDevice;
        uint16 timeDelta;

        (void) ZDSecMgrNewDeviceEvent( ZDApp_NewDeviceList->shortAddr );

        pNewDevice = (ZDAppNewDevice_t *) ZDApp_NewDeviceList->next;
        osal_mem_free( ZDApp_NewDeviceList );
        ZDApp_NewDeviceList = pNewDevice;

        if ( pNewDevice )
        {
          timeDelta = pNewDevice->timeDelta;
          pNewDevice = pNewDevice->next;

          while ( pNewDevice )
          {
            pNewDevice->timeDelta -= timeDelta;
            pNewDevice = pNewDevice->next;
          }

          osal_start_timerEx( ZDAppTaskID, ZDO_NEW_DEVICE, timeDelta );
        }
      }

      // Return unprocessed events
      return (events ^ ZDO_NEW_DEVICE);
    }
  }

  if ( events & ZDO_DEVICE_AUTH )
  {
    ZDApp_StoreNwkSecMaterial();
    
    ZDApp_DeviceAuthEvt();

    bdb_setNodeIsOnANetwork(TRUE);
    
    bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_JOINING, TRUE);

    // Return unprocessed events
    return (events ^ ZDO_DEVICE_AUTH);
  }

  if ( events & ZDO_FRAMECOUNTER_CHANGE )
  {
    ZDApp_SaveNwkKey();

    // Return unprocessed events
    return (events ^ ZDO_FRAMECOUNTER_CHANGE);
  }

  if ( events & ZDO_APS_FRAMECOUNTER_CHANGE )
  {
#if defined (NV_RESTORE)
    ZDSecMgrSaveApsLinkKey();
#endif // (NV_RESTORE)

    // Return unprocessed events
    return (events ^ ZDO_APS_FRAMECOUNTER_CHANGE);
  }

  if ( events & ZDO_TCLK_FRAMECOUNTER_CHANGE )
  {
    ZDSecMgrSaveTCLinkKey();

    // Return unprocessed events
    return (events ^ ZDO_TCLK_FRAMECOUNTER_CHANGE);
  }

  // Discard or make more handlers
  return 0;
}

/*********************************************************************
 * Application Functions
 */

/*********************************************************************
 * @fn      ZDOInitDevice
 *
 * @brief   Start the device in the network.  This function will read
 *   ZCD_NV_STARTUP_OPTION (NV item) to determine whether or not to
 *   restore the network state of the device.
 *
 * @param   startDelay - timeDelay to start device (in milliseconds).
 *                       There is a jitter added to this delay:
 *                       ((NWK_START_DELAY + startDelay)
 *                       + (osal_rand() & EXTENDED_JOINING_RANDOM_MASK))
 *                       When startDelay is set to ZDO_INIT_HOLD_NWK_START
 *                       this function will hold the network init. Application
 *                       can start the device.
 * #@param  mode       - ZDO_INITDEV_CENTRALIZED or ZDO_INITDEV_DISTRIBUTED to specify
 *                       which mode should the device start with (only has effect on 
 *                       Router devices)
 *
 * NOTE:    If the application would like to force a "new" join, the
 *          application should set the ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *          bit in the ZCD_NV_STARTUP_OPTION NV item before calling
 *          this function. "new" join means to not restore the network
 *          state of the device. Use zgWriteStartupOptions() to set these
 *          options.
 *
 * @return
 *    ZDO_INITDEV_RESTORED_NETWORK_STATE  - The device's network state was
 *          restored.
 *    ZDO_INITDEV_NEW_NETWORK_STATE - The network state was initialized.
 *          This could mean that ZCD_NV_STARTUP_OPTION said to not restore, or
 *          it could mean that there was no network state to restore.
 */
uint8 ZDOInitDeviceEx( uint16 startDelay, uint8 mode)
{
  uint8 networkStateNV = ZDO_INITDEV_NEW_NETWORK_STATE;
  uint16 extendedDelay = 0;
  
  if ( devState == DEV_HOLD )
  {
    byte temp = FALSE;
    // Initialize the RAM items table, in case an NV item has been updated.
    zgInitItems( FALSE );
    
    //Turn off the radio
    ZMacSetReq(ZMacRxOnIdle, &temp);
  }
  else
  {
    byte temp = TRUE;
    //Turn on the radio
    ZMacSetReq(ZMacRxOnIdle, &temp);
  }

  ZDConfig_InitDescriptors();
  //devtag.071807.todo - fix this temporary solution
  _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;

#if defined ( NV_RESTORE )
  // Hold down the SW_BYPASS_NV key (defined in OnBoard.h)
  // while booting to skip past NV Restore.
  if ( zdappHoldKeys == SW_BYPASS_NV )
  {
    zdappHoldKeys = 0;   // Only once
    networkStateNV = ZDO_INITDEV_NEW_NETWORK_STATE;
  }
  else
  {
#if (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE)
    if ( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
    {
      networkStateNV = ZDO_INITDEV_RESTORED_NETWORK_STATE;
    }
    else
    {
      // Determine if NV should be restored
      networkStateNV = ZDApp_ReadNetworkRestoreState();
    }
#else
    // Determine if NV should be restored
    networkStateNV = ZDApp_ReadNetworkRestoreState();
#endif
  }

  if ( networkStateNV == ZDO_INITDEV_RESTORED_NETWORK_STATE )
  {
    networkStateNV = ZDApp_RestoreNetworkState();
#if (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE)
    if ( ( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL ) && ( networkStateNV == ZDO_INITDEV_NEW_NETWORK_STATE ) )
    {
      networkStateNV = ZDO_INITDEV_RESTORED_NETWORK_STATE;
    }
#endif
    runtimeChannel = (uint32) (1L << _NIB.nwkLogicalChannel);
  }
  else
  {
    // Wipe out the network state in NV
    NLME_InitNV();
    NLME_SetDefaultNV();
    // clear NWK key values
    ZDSecMgrClearNVKeyValues();
  }
#endif

  if ( networkStateNV == ZDO_INITDEV_NEW_NETWORK_STATE )
  {
    ZDAppDetermineDeviceType();

    // Only delay if joining network - not restoring network state
    extendedDelay = (uint16)((NWK_START_DELAY + startDelay)
              + (osal_rand() & EXTENDED_JOINING_RANDOM_MASK));

    runtimeChannel = zgDefaultChannelList;
    
    // Set the NV startup option to force a "new" join.
    zgWriteStartupOptions( ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );
    
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
    gp_ProxyTblInit( TRUE );
#elif defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
    uint16 i;
    uint8 status;
    uint8 emptyEntry[] = {0xFF, 0xFF, /* Options 16-bit bitmap */  \
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /*GPD ID Unsigned 32-bit integer/ IEEE address */  \
                         0x00, /* Endpoint Unsigned 8-bit integer */  \
                         0xFF, 0xFF, /* GPD Assigned Alias Unsigned 16-bit integer */  \
                         0xFF, /* Security Options 8-bit bitmap */  \
                         0xFF, 0xFF, 0xFF, 0xFF, /* GPD security frame counter Unsigned 32-bit Integer */  \
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* GPD key Security key */  \
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Lightweight sink address list sequence of octets */  \
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                         0x00, /* Free group entries flag */  \
                         0x00, 0x00, 0x00, 0x00, /* Sink group list sequence of octets */  \
                         0x00, 0x00, 0x00, 0x00,  \
                         0x00, /* Groupcast radius Unsigned 8-bit integer */  \
                         0x00 /* Search Counter Unsigned 8-bit integer */  \
                         };
    for( i = ZCD_NV_PROXY_TABLE_START; i <= ZCD_NV_PROXY_TABLE_END; i++ )
    {
      status = osal_nv_write( ( ZCD_NV_PROXY_TABLE_START + i ), 0,
                                          sizeof(emptyEntry), &emptyEntry );
      
      if( status != SUCCESS )
      {
        break;
      }
    }
#endif

    _NIB.nwkDevAddress = INVALID_NODE_ADDR;
    _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
    _NIB.nwkPanId = 0xFFFF;
    osal_memset(_NIB.extendedPANID, 0, Z_EXTADDR_LEN);
    NLME_SetUpdateID( 0 );
    
    if(ZG_DEVICE_RTRONLY_TYPE)
    {
      if(1 == mode)
      {
        //Update TC address as distributed network (TC none)
        ZDSecMgrUpdateTCAddress(0);
      }
      else
      {
        // Centralized mode
        uint8 tmp[Z_EXTADDR_LEN];
        osal_memset(tmp,0x00,Z_EXTADDR_LEN);
        ZDSecMgrUpdateTCAddress(tmp);
      }
    }

    // Update NIB in NV
    osal_nv_write( ZCD_NV_NIB, 0, sizeof( nwkIB_t ), &_NIB );

    // Reset the NV startup option to resume from NV by clearing
    // the "New" join option.
    zgWriteStartupOptions( ZG_STARTUP_CLEAR, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );
  
  }

  // Initialize the security for type of device
  ZDApp_SecInit( networkStateNV );

  if( ZDO_INIT_HOLD_NWK_START != startDelay )
  {
    devState = DEV_INIT;    // Remove the Hold state

    // Initialize leave control logic
    ZDApp_LeaveCtrlInit();

    // Trigger the network start
    ZDApp_NetworkInit( extendedDelay );
  }

  // set broadcast address mask to support broadcast filtering
  NLME_SetBroadcastFilter( ZDO_Config_Node_Descriptor.CapabilityFlags );

  return ( networkStateNV );
}

/*********************************************************************
 * @fn      ZDApp_ReadNetworkRestoreState
 *
 * @brief   Read the ZCD_NV_STARTUP_OPTION NV Item to state whether
 *          or not to restore the network state.
 *          If the read value has the ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *          bit set return the ZDO_INITDEV_NEW_NETWORK_STATE.
 *
 * @param   none
 *
 * @return  ZDO_INITDEV_NEW_NETWORK_STATE
 *          or ZDO_INITDEV_RESTORED_NETWORK_STATE based on whether or
 *          not ZCD_STARTOPT_DEFAULT_NETWORK_STATE bit is set in
 *          ZCD_NV_STARTUP_OPTION
 */
uint8 ZDApp_ReadNetworkRestoreState( void )
{
  uint8 networkStateNV = ZDO_INITDEV_RESTORED_NETWORK_STATE;

  // Look for the New Network State option.
  if ( zgReadStartupOptions() & ZCD_STARTOPT_DEFAULT_NETWORK_STATE )
  {
    networkStateNV = ZDO_INITDEV_NEW_NETWORK_STATE;
    bdb_setNodeIsOnANetwork(FALSE);
  }

  return ( networkStateNV );
}

/*********************************************************************
 * @fn      ZDAppDetermineDeviceType()
 *
 * @brief   Determines the type of device to start.
 *
 *          Looks at zgDeviceLogicalType and determines what type of
 *          device to start.  The types are:
 *            ZG_DEVICETYPE_COORDINATOR
 *            ZG_DEVICETYPE_ROUTER
 *            ZG_DEVICETYPE_ENDDEVICE
 *
 * @param   none
 *
 * @return  none
 */
void ZDAppDetermineDeviceType( void )
{
  if ( zgDeviceLogicalType == ZG_DEVICETYPE_COORDINATOR )
  {
    devStartMode = MODE_HARD;     // Start as a coordinator
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_COORDINATOR;
  }
  else
  {
    if ( zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER  )
      ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_ROUTER;
    else if ( zgDeviceLogicalType == ZG_DEVICETYPE_ENDDEVICE )
      ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_DEVICE;

    // If AIB_apsUseExtendedPANID is set to a non-zero value by commissioning
    // The device shall do rejoin the network. Otherwise, do normal join
    if ( nwk_ExtPANIDValid( AIB_apsUseExtendedPANID ) == false )
    {
      devStartMode = MODE_JOIN;     // Assume joining
    }
    else
    {
      devStartMode = MODE_REJOIN;
      prevDevState = DEV_NWK_SEC_REJOIN_CURR_CHANNEL;
    }
  }
}

/*********************************************************************
 * @fn      ZDApp_NetworkStartEvt()
 *
 * @brief   Process the Network Start Event
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_NetworkStartEvt( void )
{
  if ( nwkStatus == ZSuccess )
  {
    // Successfully started a ZigBee network
    if ( devState == DEV_COORD_STARTING )
    {
      //save NIB to NV before child joins if NV_RESTORE is defined
      ZDApp_NwkWriteNVRequest();
      ZDApp_ChangeState( DEV_ZB_COORD );
      
      if(bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_FORMATION)
      {
        bdb_nwkFormationAttempt(TRUE);
        ZDApp_StoreNwkSecMaterial();
      }
      else if(bdbCommissioningProcedureState.bdbCommissioningState == BDB_INITIALIZATION)
      {
        bdb_reportCommissioningState(BDB_INITIALIZATION,TRUE);
      }

      if ( zgChildAgingEnable == TRUE )
      {
        // Once the device has changed its state to a COORDINATOR set the timer to send
        // Parent annce
        ZDApp_SetParentAnnceTimer();
      }
    }
    else
    {
      osal_set_event( ZDAppTaskID, ZDO_STATE_CHANGE_EVT );
    }

    osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
  }
  else
  {
    // Try again with a higher energy threshold
    if ( ( NLME_GetEnergyThreshold() + ENERGY_SCAN_INCREMENT ) < 0xff )
    {
      NLME_SetEnergyThreshold( (uint8)(NLME_GetEnergyThreshold() + ENERGY_SCAN_INCREMENT) );
      osal_set_event( ZDAppTaskID, ZDO_NETWORK_INIT );
    }
    else
    {
      bdb_nwkFormationAttempt(FALSE);
    }
  }
}

/*********************************************************************
 * @fn      ZDApp_DeviceAuthEvt()
 *
 * @brief   Process the Device Authentic Event
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_DeviceAuthEvt( void )
{
  // received authentication from trust center
  if ( devState == DEV_END_DEVICE_UNAUTH )
  {
    // Stop the reset timer so it doesn't reset
    ZDApp_ResetTimerCancel();

    ZDApp_ChangeState( DEV_END_DEVICE );

    // Set the Power Manager Device
#if defined ( POWER_SAVING )
    osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

    if ( ZSTACK_ROUTER_BUILD )
    {
      if ( ZDO_Config_Node_Descriptor.LogicalType != NODETYPE_DEVICE )
      {
        // NOTE: first two parameters are not used, see NLMEDE.h for details
        NLME_StartRouterRequest( 0, 0, false );
      }
    }

    // Notify to save info into NV
    ZDApp_NVUpdate();

    // Save off the security
    ZDApp_SaveNwkKey();

    ZDApp_AnnounceNewAddress();

    if ( ( (ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE) == 0 )
        || ( (ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE)
          && (zgChildAgingEnable == TRUE) ) )
    {
      NLME_SetPollRate( ZDApp_SavedPollRate );
    }
  }
  else
  {
    ZDApp_NVUpdate();
  }
}

/*********************************************************************
 * @fn      ZDApp_SaveNetworkStateEvt()
 *
 * @brief   Process the Save the Network State Event
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_SaveNetworkStateEvt( void )
{
#if defined ( NV_RESTORE )
 #if defined ( NV_TURN_OFF_RADIO )
  // Turn off the radio's receiver during an NV update
  uint8 RxOnIdle;
  uint8 x = false;
  ZMacGetReq( ZMacRxOnIdle, &RxOnIdle );
  ZMacSetReq( ZMacRxOnIdle, &x );
 #endif

  // Update the Network State in NV
  NLME_UpdateNV( NWK_NV_NIB_ENABLE        |
                 NWK_NV_DEVICELIST_ENABLE |
                 NWK_NV_BINDING_ENABLE    |
                 NWK_NV_ADDRMGR_ENABLE );

  // Reset the NV startup option to resume from NV by
  // clearing the "New" join option.
  zgWriteStartupOptions( FALSE, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );

 #if defined ( NV_TURN_OFF_RADIO )
  ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
 #endif
#endif // NV_RESTORE
}

#if defined ( NV_RESTORE )
/*********************************************************************
 * @fn      ZDApp_RestoreNetworkState()
 *
 * @brief   This function will restore the network state of the
 *          device if the network state is stored in NV.
 *
 * @param   none
 *
 * @return
 *    ZDO_INITDEV_RESTORED_NETWORK_STATE  - The device's network state was
 *          restored.
 *    ZDO_INITDEV_NEW_NETWORK_STATE - The network state was not used.
 *          This could mean that zgStartupOption said to not restore, or
 *          it could mean that there was no network state to restore.
 *
 */
uint8 ZDApp_RestoreNetworkState( void )
{
  uint8 nvStat;

  // Initialize NWK NV items
  nvStat = NLME_InitNV();

  if ( nvStat == SUCCESS )
  {
    if ( NLME_RestoreFromNV() )
    {
      // Are we a coordinator
      ZDAppNwkAddr.addr.shortAddr = NLME_GetShortAddr();
      if ( ZDAppNwkAddr.addr.shortAddr == 0 )
      {
        ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_COORDINATOR;
      }
      if(ZG_DEVICE_ENDDEVICE_TYPE) 
      {
        devStartMode = MODE_REJOIN;
        _NIB.nwkState = NWK_INIT;
      }
      else
      {
        devStartMode = MODE_RESUME;
      }
      osal_cpyExtAddr( ZDO_UseExtendedPANID, _NIB.extendedPANID );
    }
    else
      nvStat = NV_ITEM_UNINIT;

    if ( ZG_SECURE_ENABLED )
    {
      nwkFrameCounterChanges = 0;

      if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
      {
        ZDApp_RestoreNwkKey( TRUE );
      }
    }

    // The default for RxOnWhenIdle is true for Routers and false for end devices
    // [setup in the NLME_RestoreFromNV()].  Change it here if you want something
    // other than default.
  }

  if ( nvStat == ZSUCCESS )
    return ( ZDO_INITDEV_RESTORED_NETWORK_STATE );
  else
    return ( ZDO_INITDEV_NEW_NETWORK_STATE );
}
#endif // NV_RESTORE

/*********************************************************************
 * @fn      ZDApp_InitUserDesc()
 *
 * @brief   Initialize the User Descriptor, the descriptor is read from NV
 *          when needed.  If you want to initialize the User descriptor to
 *          something other than all zero, do it here.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_InitUserDesc( void )
{
  UserDescriptorFormat_t ZDO_DefaultUserDescriptor;

  // Initialize the User Descriptor, the descriptor is read from NV
  // when needed.  If you want to initialize the User descriptor to something
  // other than all zero, do it here.
  osal_memset( &ZDO_DefaultUserDescriptor, 0, sizeof( UserDescriptorFormat_t ) );
  if ( ZSUCCESS == osal_nv_item_init( ZCD_NV_USERDESC,
         sizeof(UserDescriptorFormat_t), (void*)&ZDO_DefaultUserDescriptor ) )
  {
    if ( ZSUCCESS == osal_nv_read( ZCD_NV_USERDESC, 0,
         sizeof(UserDescriptorFormat_t), (void*)&ZDO_DefaultUserDescriptor ) )
    {
      if ( ZDO_DefaultUserDescriptor.len != 0 )
      {
        ZDO_Config_Node_Descriptor.UserDescAvail = TRUE;
      }
    }
  }
}

/*********************************************************************
 * @fn      ZDAppCheckForHoldKey()
 *
 * @brief   Check for key to set the device into Hold Auto Start
 *
 * @param   none
 *
 * @return  none
 */
void ZDAppCheckForHoldKey( void )
{
#if (defined HAL_KEY) && (HAL_KEY == TRUE)

  // Get Keypad directly to see if a HOLD is needed
  zdappHoldKeys = HalKeyRead();

  // Hold down the SW_BYPASS_START key (see OnBoard.h)
  // while booting to avoid starting up the device.
  if ( zdappHoldKeys == SW_BYPASS_START )
  {
    // Change the device state to HOLD on start up
    devState = DEV_HOLD;
  }
#endif // HAL_KEY
}

/*********************************************************************
 * @fn      ZDApp_ProcessOSALMsg()
 *
 * @brief   Process the incoming task message.
 *
 * @param   msgPtr - message to process
 *
 * @return  none
 */
void ZDApp_ProcessOSALMsg( osal_event_hdr_t *msgPtr )
{
  // Data Confirmation message fields
  uint8 sentEP;       // This should always be 0
  uint8 sentStatus;
  afDataConfirm_t *afDataConfirm;


  switch ( msgPtr->event )
  {
    // Incoming ZDO Message
    case AF_INCOMING_MSG_CMD:
      ZDP_IncomingData( (afIncomingMSGPacket_t *)msgPtr );
      break;

    case ZDO_CB_MSG:
      ZDApp_ProcessMsgCBs( (zdoIncomingMsg_t *)msgPtr );
      break;

    case AF_DATA_CONFIRM_CMD:
      // This message is received as a confirmation of a data packet sent.
      // The status is of ZStatus_t type [defined in NLMEDE.h]
      // The message fields are defined in AF.h
      afDataConfirm = (afDataConfirm_t *)msgPtr;
      sentEP = afDataConfirm->endpoint;
      sentStatus = afDataConfirm->hdr.status;

      // Action taken when confirmation is received.
#if defined ( ZIGBEE_FREQ_AGILITY )
      if ( pZDNwkMgr_ProcessDataConfirm )
        pZDNwkMgr_ProcessDataConfirm( afDataConfirm );
#endif
      (void)sentEP;
      (void)sentStatus;
      break;

    case ZDO_NWK_DISC_CNF:
      if (devState != DEV_NWK_DISC)
      {
        break;
      }
      if ( ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE )
      {
        //Rejoin or resume
        if(bdb_isDeviceNonFactoryNew()) 
        {
          if(bdb_rejoinNwk() == ZSuccess)
          {
            return;
          }
#if (ZG_BUILD_ENDDEVICE_TYPE)
          else
          {
            if(ZG_DEVICE_ENDDEVICE_TYPE)
            {
              bdb_parentLost();
              return;
            }
          }
#endif
        }
        
        if(nwk_getNwkDescList())
        {
          bdb_nwkDiscoveryAttempt(TRUE);
        }
        else
        {
          bdb_nwkDiscoveryAttempt(FALSE);
        }
      }
    break;

    case ZDO_NWK_JOIN_IND:
      if ( ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE )
      {
        if((bdbCommissioningProcedureState.bdbCommissioningState == BDB_INITIALIZATION) || (bdbCommissioningProcedureState.bdbCommissioningState == BDB_PARENT_LOST))
        {
          if(nwkStatus == ZSuccess)
          {
            bdb_reportCommissioningState(bdbCommissioningProcedureState.bdbCommissioningState,TRUE);
          }
        }
        ZDApp_ProcessNetworkJoin();
      }
      break;

    default:
      if ( ZG_SECURE_ENABLED )
        ZDApp_ProcessSecMsg( msgPtr );
      break;
  }

}

/*********************************************************************
 * @fn      ZDApp_ProcessMsgCBs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_ProcessMsgCBs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
#if defined ( ZDO_NWKADDR_REQUEST ) || defined ( ZDO_IEEEADDR_REQUEST ) || defined ( REFLECTOR )
    case NWK_addr_rsp:
    case IEEE_addr_rsp:
      {
        ZDO_NwkIEEEAddrResp_t *pAddrRsp;

        pAddrRsp = ZDO_ParseAddrRsp( inMsg );
        if ( pAddrRsp )
        {
          if ( pAddrRsp->status == ZSuccess )
          {
            ZDO_UpdateAddrManager( pAddrRsp->nwkAddr, pAddrRsp->extAddr );
          }

#if defined ( ZDP_BIND_VALIDATION )
          // look for pending bind entry for NWK_addr_rsp Only
          if ( inMsg->clusterID == NWK_addr_rsp )
          {
#if defined ( REFLECTOR )
            ZDApp_ProcessPendingBindReq( pAddrRsp->extAddr );
#endif
          }
#endif

          osal_mem_free( pAddrRsp );
        }
      }
      break;
#endif

#if defined ( REFLECTOR )
    case Bind_req:
    case Unbind_req:
      {
        ZDO_BindUnbindReq_t bindReq;
        ZDO_ParseBindUnbindReq( inMsg, &bindReq );
        ZDO_ProcessBindUnbindReq( inMsg, &bindReq );
      }
      break;
#endif

#if ( ZG_BUILD_COORDINATOR_TYPE )
    case Bind_rsp:
    case Unbind_rsp:
      if (ZG_DEVICE_COORDINATOR_TYPE && matchED)
      {
        ZDMatchSendState(
             (uint8)((inMsg->clusterID == Bind_rsp) ? ZDMATCH_REASON_BIND_RSP : ZDMATCH_REASON_UNBIND_RSP),
             ZDO_ParseBindRsp(inMsg), inMsg->TransSeq );
      }
      break;

    case End_Device_Bind_req:
#ifdef ZDO_ENDDEVICEBIND_RESPONSE
      if (ZG_DEVICE_COORDINATOR_TYPE)
      {
        ZDEndDeviceBind_t bindReq;
        ZDO_ParseEndDeviceBindReq( inMsg, &bindReq );
        ZDO_MatchEndDeviceBind( &bindReq );

        // Freeing the cluster lists - if allocated.
        if ( bindReq.numInClusters )
          osal_mem_free( bindReq.inClusters );
        if ( bindReq.numOutClusters )
          osal_mem_free( bindReq.outClusters );
      }
#endif
      break;
#endif
  }
}

/*********************************************************************
 * @fn      ZDApp_RegisterCBs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_RegisterCBs( void )
{
#if defined ( ZDO_IEEEADDR_REQUEST ) || defined ( REFLECTOR )
  ZDO_RegisterForZDOMsg( ZDAppTaskID, IEEE_addr_rsp );
#endif
#if defined ( ZDO_NWKADDR_REQUEST ) || defined ( REFLECTOR )
  ZDO_RegisterForZDOMsg( ZDAppTaskID, NWK_addr_rsp );
#endif
#if ZG_BUILD_COORDINATOR_TYPE
  ZDO_RegisterForZDOMsg( ZDAppTaskID, Bind_rsp );
  ZDO_RegisterForZDOMsg( ZDAppTaskID, Unbind_rsp );
#ifdef ZDO_ENDDEVICEBIND_RESPONSE
  ZDO_RegisterForZDOMsg( ZDAppTaskID, End_Device_Bind_req );
#endif
#endif
#if defined ( REFLECTOR )
  ZDO_RegisterForZDOMsg( ZDAppTaskID, Bind_req );
  ZDO_RegisterForZDOMsg( ZDAppTaskID, Unbind_req );
#endif
}

/*********************************************************************
 * @fn      ZDApp_ProcessSecMsg()
 *
 * @brief   Process the incoming security message.
 *
 * @param   msgPtr - message to process
 *
 * @return  none
 */
void ZDApp_ProcessSecMsg( osal_event_hdr_t *msgPtr )
{
  switch ( msgPtr->event )
  {
    case ZDO_TRANSPORT_KEY_IND:
      if ( ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE )
      {
        ZDSecMgrTransportKeyInd( (ZDO_TransportKeyInd_t*)msgPtr );
      }
      break;

    case ZDO_UPDATE_DEVICE_IND:
      if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
      {
        ZDSecMgrUpdateDeviceInd( (ZDO_UpdateDeviceInd_t*)msgPtr );
        
        // Look at GP proxy table for posible conflict with GPD alias NwkAddr
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
        ZDO_DeviceAnnce_t devAnnce;
        uint32 timeout;
        uint8 invalidAddr;
        uint8 sameAnnce;
        uint8 invalidIEEE[Z_EXTADDR_LEN] = {0xFF};
        
        osal_memcpy( devAnnce.extAddr, ((ZDO_UpdateDeviceInd_t*)msgPtr)->devExtAddr, Z_EXTADDR_LEN );
        devAnnce.nwkAddr = ((ZDO_UpdateDeviceInd_t*)msgPtr)->devAddr;
        
        timeout = osal_get_timeoutEx( gp_TaskID, GP_PROXY_ALIAS_CONFLICT_TIMEOUT );
        invalidAddr = osal_memcmp( devAnnce.extAddr, invalidIEEE, Z_EXTADDR_LEN );
        sameAnnce = osal_memcmp( &devAnnce, &GP_aliasConflictAnnce, sizeof( ZDO_DeviceAnnce_t ) );
        
        // Check GP proxy table to update the entry if necesary
        if( timeout && ( invalidAddr && sameAnnce ) )
        {
          osal_stop_timerEx( gp_TaskID, GP_PROXY_ALIAS_CONFLICT_TIMEOUT );
        }
        else if(GP_CheckAnnouncedDeviceGCB != NULL)
        {
          GP_CheckAnnouncedDeviceGCB( devAnnce.extAddr, devAnnce.nwkAddr );       
        }
#endif
      }
      break;

    case ZDO_REMOVE_DEVICE_IND:
      if ( ZG_BUILD_RTRONLY_TYPE && ( zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER ) )
      {
        ZDSecMgrRemoveDeviceInd( (ZDO_RemoveDeviceInd_t*)msgPtr );
      }
      break;

    case ZDO_REQUEST_KEY_IND:
      if ( ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD ) )
      {
        if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
        {
          ZDSecMgrRequestKeyInd( (ZDO_RequestKeyInd_t*)msgPtr );
        }
      }
      break;
    case ZDO_VERIFY_KEY_IND:
#if (ZG_BUILD_COORDINATOR_TYPE)
      if(ZG_DEVICE_COORDINATOR_TYPE)
      {
        if( ((ZDO_VerifyKeyInd_t*)msgPtr)->verifyKeyStatus == ZSuccess)
        {
          bdb_TCjoiningDeviceComplete( ((ZDO_VerifyKeyInd_t*)msgPtr)->extAddr );
        }
      }
  
      ZDSecMgrVerifyKeyInd( (ZDO_VerifyKeyInd_t*)msgPtr );
#endif
      break;

    case ZDO_SWITCH_KEY_IND:
      if ( ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE )
      {
        ZDSecMgrSwitchKeyInd( (ZDO_SwitchKeyInd_t*)msgPtr );
      }
      break;

    default:
      // Unsupported messages
      break;
  }
}

/*********************************************************************
 * @fn      ZDApp_ProcessNetworkJoin()
 *
 * @brief
 *
 *   Save off the Network key information.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_ProcessNetworkJoin( void )
{
  if ( (devState == DEV_NWK_JOINING) ||
      ((devState == DEV_NWK_ORPHAN)  &&
       (ZDO_Config_Node_Descriptor.LogicalType == NODETYPE_ROUTER)) )
  {
    // Result of a Join attempt by this device.
    if ( nwkStatus == ZSuccess )
    {
      osal_set_event( ZDAppTaskID, ZDO_STATE_CHANGE_EVT );

#if defined ( POWER_SAVING )
      osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

      if ( ZG_SECURE_ENABLED && ( ZDApp_RestoreNwkKey( TRUE ) == false ) )
      {
        if ( ZSTACK_END_DEVICE_BUILD )
        {
          NLME_SetPollRate( zgRejoinPollRate );
        }
        // wait for auth from trust center
        ZDApp_ChangeState( DEV_END_DEVICE_UNAUTH );
        
        bdb_nwkAssocAttemt(TRUE);
      }
      else
      {
        if ( ZSTACK_ROUTER_BUILD )
        {
          if ( devState == DEV_NWK_ORPHAN
            && ZDO_Config_Node_Descriptor.LogicalType != NODETYPE_DEVICE )
          {
            // Change NIB state to router for restore
            _NIB.nwkState = NWK_ROUTER;
          }
        }

        if ( devState == DEV_NWK_JOINING )
        {
          ZDApp_AnnounceNewAddress();
          if( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
          {
            bdb_setNodeIsOnANetwork(TRUE);
            bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, TRUE );
          }
        }

        ZDApp_ChangeState( DEV_END_DEVICE );

        if ( ZSTACK_ROUTER_BUILD )
        {
          // NOTE: first two parameters are not used, see NLMEDE.h for details
          if ( ZDO_Config_Node_Descriptor.LogicalType != NODETYPE_DEVICE )
          {
            NLME_StartRouterRequest( 0, 0, false );
          }
        }
      }
    }
    else
    {

      bdb_nwkAssocAttemt(FALSE);
    }
  }
  else if ( devState == DEV_NWK_ORPHAN ||
            devState == DEV_NWK_SEC_REJOIN_CURR_CHANNEL ||
            devState == DEV_NWK_TC_REJOIN_CURR_CHANNEL ||
            devState == DEV_NWK_TC_REJOIN_ALL_CHANNEL ||
            devState == DEV_NWK_SEC_REJOIN_ALL_CHANNEL )
  {
    // results of an orphaning attempt by this device
    if (nwkStatus == ZSuccess)
    {
      //When the device has successfully rejoined then reset retryCnt
      retryCnt = 0;

      // Verify NWK key is available before sending Device_annce
      if ( ZG_SECURE_ENABLED && ( ZDApp_RestoreNwkKey( TRUE ) == false ) )
      {
        if ( ZSTACK_END_DEVICE_BUILD )
        {
          NLME_SetPollRate( zgRejoinPollRate );
        }
        // wait for auth from trust center
        ZDApp_ChangeState( DEV_END_DEVICE_UNAUTH );

        // Start the reset timer for MAX UNAUTH time
        ZDApp_ResetTimerStart( MAX_DEVICE_UNAUTH_TIMEOUT );
      }
      else
      {
        ZDApp_ChangeState( DEV_END_DEVICE );

        osal_stop_timerEx( ZDAppTaskID, ZDO_REJOIN_BACKOFF );

        // setup Power Manager Device
#if defined ( POWER_SAVING )
        osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

        // The receiver is on, turn network layer polling off.
        if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
        {
          // if Child Table Management process is not enabled
          if ( zgChildAgingEnable == FALSE )
          {
            NLME_SetPollRate( 0 );
            NLME_SetQueuedPollRate( 0 );
            NLME_SetResponseRate( 0 );
          }
        }

        if ( ZSTACK_ROUTER_BUILD )
        {
          // NOTE: first two parameters are not used, see NLMEDE.h for details
          if ( ZDO_Config_Node_Descriptor.LogicalType != NODETYPE_DEVICE )
          {
            NLME_StartRouterRequest( 0, 0, false );
          }
        }

        ZDApp_AnnounceNewAddress();
      }
    }
    else
    {
      if ( devStartMode == MODE_RESUME )
      {
        if ( ++retryCnt <= MAX_RESUME_RETRY )
        {
          if ( _NIB.nwkPanId == 0xFFFF )
            devStartMode = MODE_JOIN;
          else
          {
            devStartMode = MODE_REJOIN;
            _tmpRejoinState = true;
            prevDevState = DEV_NWK_SEC_REJOIN_CURR_CHANNEL;
          }
        }
        // Do a normal join to the network after certain times of rejoin retries
        else if( AIB_apsUseInsecureJoin == true )
        {
          devStartMode = MODE_JOIN;
        }
      }
      else if(devStartMode == MODE_REJOIN)
      {
        if ( ZSTACK_END_DEVICE_BUILD )
        {
          devStartMode = MODE_REJOIN;
          _tmpRejoinState = true;
          _NIB.nwkState = NWK_INIT;
        }
      }

      // Clear the neighbor Table and network discovery tables.
      nwkNeighborInitTable();
      NLME_NwkDiscTerm();

      if ( ( ( (ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE) == 0 )
        || ( (ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE)
          && (zgChildAgingEnable == TRUE) ) ) && (devStartMode == MODE_REJOIN) )
      {
        NLME_SetPollRate( zgRejoinPollRate );
      }
    }
  }
#if defined ( ZIGBEEPRO )
  else if ( devState != DEV_HOLD )
  {
    #ifdef APP_TP2
      if(devState == DEV_END_DEVICE_UNAUTH )
      {
        return;
      }
    #endif
	  
    // Assume from address conflict

    // Notify the network
    ZDApp_AnnounceNewAddress();

    // Notify apps
    osal_set_event( ZDAppTaskID, ZDO_STATE_CHANGE_EVT );
  }
#endif
}

/******************************************************************************
 * @fn          ZDApp_StoreNwkSecMaterial
 *
 * @brief       Stores new entries in the NwkSecMaterial
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_StoreNwkSecMaterial(void)
{
  nwkSecMaterialDesc_t nwkSecMaterialDesc;
  uint8 i;
  uint8 emptyEntryIndexOffset = gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES;
  
  //Search if we do have security material for this network
  for( i = 0; i < gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES; i++)
  {
    osal_nv_read(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
    {
      if(osal_memcmp(_NIB.extendedPANID,nwkSecMaterialDesc.extendedPanID,Z_EXTADDR_LEN))
      {
        break;
      }
      if(osal_isbufset(nwkSecMaterialDesc.extendedPanID,0,Z_EXTADDR_LEN))
      {
        emptyEntryIndexOffset = i;
        break;
      }
    }
  }
  
  //ExtPanID not found and found an empty entry, save the extended PANID
  if(emptyEntryIndexOffset < gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES)
  {
    osal_memcpy(nwkSecMaterialDesc.extendedPanID, _NIB.extendedPANID, Z_EXTADDR_LEN);
    nwkSecMaterialDesc.FrameCounter = 0;
    osal_nv_write(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + emptyEntryIndexOffset,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
  }

}


/*********************************************************************
 * @fn      ZDApp_SaveNwkKey()
 *
 * @brief   Save off the Network key information.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_SaveNwkKey( void )
{
  nwkActiveKeyItems keyItems;
  nwkSecMaterialDesc_t nwkSecMaterialDesc;
  uint8 found = 0;
  uint8 i;
  
  SSP_ReadNwkActiveKey( &keyItems );

  osal_nv_write( ZCD_NV_NWKKEY, 0, sizeof( nwkActiveKeyItems ),
                (void *)&keyItems );
  
  //Search for the security material to update its framecounter
  for( i = 0; i < gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES; i++)
  {
    osal_nv_read(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
    {
      if(osal_memcmp(_NIB.extendedPANID,nwkSecMaterialDesc.extendedPanID,Z_EXTADDR_LEN))
      {
        nwkSecMaterialDesc.FrameCounter = keyItems.frameCounter;
        found = TRUE;
        //update the framecounter associated to this ExtPanID
        osal_nv_write(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
        break;
      }
    }
  }
  
  //If not found, then use the generic
  if(!found)
  {
    osal_memset(nwkSecMaterialDesc.extendedPanID,0xFF,Z_EXTADDR_LEN);
    nwkSecMaterialDesc.FrameCounter = keyItems.frameCounter;
    //update the framecounter associated to this ExtPanID
    osal_nv_write(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i - 1,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
  }
  
  
  nwkFrameCounterChanges = 0;
  
  // Clear copy in RAM before return.
  osal_memset( &keyItems, 0x00, sizeof(keyItems) );

}

/*********************************************************************
 * @fn      ZDApp_ForceConcentratorChange()
 *
 * @brief   Force a network concentrator change by resetting
 *          zgConcentratorEnable and zgConcentratorDiscoveryTime
 *          from NV and set nwk event.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_ForceConcentratorChange( void )
{
  osal_nv_read( ZCD_NV_CONCENTRATOR_ENABLE, 0, sizeof(zgConcentratorEnable), &zgConcentratorEnable );
  osal_nv_read( ZCD_NV_CONCENTRATOR_DISCOVERY, 0, sizeof(zgConcentratorDiscoveryTime), &zgConcentratorDiscoveryTime );

  if ( zgConcentratorEnable == TRUE )
  {
    // Start next event
    osal_start_timerEx( NWK_TaskID, NWK_MTO_RTG_REQ_EVT, 100 );
  }
  else
  {
    // Stop the next event
    osal_stop_timerEx( NWK_TaskID, NWK_MTO_RTG_REQ_EVT );
  }
}

/*********************************************************************
 * @fn      ZDApp_ResetNwkKey()
 *
 * @brief   Reset the Network key information in NV.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_ResetNwkKey( void )
{
  nwkActiveKeyItems keyItems;

  osal_memset( &keyItems, 0, sizeof( nwkActiveKeyItems ) );
  osal_nv_write( ZCD_NV_NWKKEY, 0, sizeof( nwkActiveKeyItems ),
                (void *)&keyItems );
}

/*********************************************************************
 * @fn      ZDApp_RestoreNwkSecMaterial()
 *
 * @brief   Restore the network frame counter associated to this ExtPanID and 
 *          increment it if found. This can only happens once per reset
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_RestoreNwkSecMaterial(void)
{
  uint8 Found = FALSE;
  uint8 i;
  nwkSecMaterialDesc_t nwkSecMaterialDesc;
  uint8 UpdateFrameCounter = FALSE;

  //Search if we do have security material for this network
  for( i = 0; i < gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES; i++)
  {
    osal_nv_read(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
    {
      if(osal_memcmp(_NIB.extendedPANID,nwkSecMaterialDesc.extendedPanID,Z_EXTADDR_LEN))
      {
        UpdateFrameCounter = TRUE;
        Found = TRUE;
        break;
      }
    }
  }    
  //Check if we do have frame counter stored in the generic
  if(!Found)
  {
    //The last entry readed has the Generic item, thefore, no need to read it again
    if(nwkSecMaterialDesc.FrameCounter)
    {
      UpdateFrameCounter = TRUE;
    }
  }  

  if(UpdateFrameCounter && (!FrameCounterUpdated))
  {
    FrameCounterUpdated = TRUE;
    
    // Increment the frame counter stored in NV
    nwkSecMaterialDesc.FrameCounter += ( MAX_NWK_FRAMECOUNTER_CHANGES +
                              NWK_FRAMECOUNTER_CHANGES_RESTORE_DELTA );
    
    nwkFrameCounter = nwkSecMaterialDesc.FrameCounter;
    
    osal_nv_write(ZCD_NV_NWK_SEC_MATERIAL_TABLE_START + i,0,sizeof(nwkSecMaterialDesc_t),&nwkSecMaterialDesc);
    
    nwkFrameCounterChanges = 0;
  }
  return;
}

/*********************************************************************
 * @fn      ZDApp_RestoreNwkKey(uint8 incrFrmCnt)
 *
 * @brief
 *
 *   Save off the Network key information.
 *
 * @param   incrFrmCnt - set to true if we want to increment the network
            frame counter, else set to false
 *
 * @return  true if restored from NV, false if not
 */
uint8 ZDApp_RestoreNwkKey( uint8 incrFrmCnt )
{
  nwkActiveKeyItems keyItems;
  uint8 ret = FALSE;

  ZDApp_RestoreNwkSecMaterial();

  // Restore the key information
  if ( osal_nv_read( ZCD_NV_NWKKEY, 0, sizeof(nwkActiveKeyItems), (void*)&keyItems )
      == ZSUCCESS )
  {
    uint8 nullKey[SEC_KEY_LEN];

    // initialize default value to compare to
    osal_memset( nullKey, 0x00, SEC_KEY_LEN );

    // if stored key is different than default value, then a key has been established
    if ( !osal_memcmp( keyItems.active.key, nullKey, SEC_KEY_LEN ) )
    {
      ret = TRUE;

      // Clear copy in RAM before return.
      osal_memset( &keyItems, 0x00, sizeof(keyItems) );
    }
  }

  return ( ret );
}

/*********************************************************************
 * @fn      ZDApp_ResetTimerStart
 *
 * @brief   Start the reset timer.
 *
 * @param   delay - delay time(ms) before reset
 *
 * @return  none
 */
void ZDApp_ResetTimerStart( uint16 delay )
{
  if ( !osal_get_timeoutEx( ZDAppTaskID, ZDO_DEVICE_RESET ) )
  {
    // Start the rest timer
    osal_start_timerEx( ZDAppTaskID, ZDO_DEVICE_RESET, delay );
  }
}

/*********************************************************************
 * @fn      ZDApp_ResetTimerCancel
 *
 * @brief   Cancel the reset timer.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_ResetTimerCancel( void )
{
  (void)osal_stop_timerEx(ZDAppTaskID, ZDO_DEVICE_RESET);
  (void)osal_clear_event(ZDAppTaskID, ZDO_DEVICE_RESET);
}

/*********************************************************************
 * @fn      ZDApp_LeaveCtrlInit
 *
 * @brief   Initialize the leave control logic.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_LeaveCtrlInit( void )
{
  uint8 status;


  // Initialize control state
  ZDApp_LeaveCtrl = ZDAPP_LEAVE_CTRL_INIT;

  status = osal_nv_item_init( ZCD_NV_LEAVE_CTRL,
                              sizeof(ZDApp_LeaveCtrl),
                              &ZDApp_LeaveCtrl );

  if ( status == ZSUCCESS )
  {
    // Read saved control
    osal_nv_read( ZCD_NV_LEAVE_CTRL,
                  0,
                  sizeof( uint8 ),
                  &ZDApp_LeaveCtrl);
  }
}

/*********************************************************************
 * @fn      ZDApp_LeaveCtrlSet
 *
 * @brief   Set the leave control logic.
 *
 * @param   ra - reassociate flag
 *
 * @return  none
 */
void ZDApp_LeaveCtrlSet( uint8 ra )
{
  ZDApp_LeaveCtrl = ZDAPP_LEAVE_CTRL_SET;

  if ( ra == TRUE )
  {
    ZDApp_LeaveCtrl |= ZDAPP_LEAVE_CTRL_RA;
  }

  // Write the leave control
  osal_nv_write( ZCD_NV_LEAVE_CTRL,
                 0,
                 sizeof( uint8 ),
                 &ZDApp_LeaveCtrl);
}

/*********************************************************************
 * @fn      ZDApp_LeaveCtrlReset
 *
 * @brief   Re-initialize the leave control logic.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_LeaveCtrlReset( void )
{
  // Set leave control to initialized state
  ZDApp_LeaveCtrl = ZDAPP_LEAVE_CTRL_INIT;

  // Write initialized control
  osal_nv_write( ZCD_NV_LEAVE_CTRL,
                0,
                sizeof( uint8 ),
                &ZDApp_LeaveCtrl);
}

/*********************************************************************
 * @fn      ZDApp_LeaveCtrlBypass
 *
 * @brief   Check if NV restore should be skipped during a leave reset.
 *
 * @param   none
 *
 * @return  uint8 - (TRUE bypass:FALSE do not bypass)
 */
uint8 ZDApp_LeaveCtrlBypass( void )
{
  uint8 bypass;

  if ( ZDApp_LeaveCtrl & ZDAPP_LEAVE_CTRL_SET )
  {
    bypass = TRUE;
  }
  else
  {
    bypass = FALSE;
  }

  return bypass;
}

/*********************************************************************
 * @fn      ZDApp_LeaveCtrlStartup
 *
 * @brief   Check for startup conditions during a leave reset.
 *
 * @param   state      - devState_t determined by leave control logic
 * @param   startDelay - startup delay
 *
 * @return  none
 */
void ZDApp_LeaveCtrlStartup( devStates_t* state, uint16* startDelay )
{
  *startDelay = 0;

  if ( ZDApp_LeaveCtrl & ZDAPP_LEAVE_CTRL_SET )
  {
    if ( ZDApp_LeaveCtrl & ZDAPP_LEAVE_CTRL_RA )
    {
      *startDelay = LEAVE_RESET_DELAY;
    }
    else
    {
      *state = DEV_HOLD;
    }

    // Reset leave control logic
    ZDApp_LeaveCtrlReset();
  }
}

/*********************************************************************
 * @fn      ZDApp_LeaveReset
 *
 * @brief   Setup a device reset due to a leave indication/confirm.
 *
 * @param   ra - reassociate flag
 *
 * @return  none
 */
void ZDApp_LeaveReset( uint8 ra )
{
  ZDApp_LeaveCtrlSet( ra );

  APSME_HoldDataRequests( LEAVE_RESET_DELAY);

  if ( ZSTACK_ROUTER_BUILD )
  {
    osal_stop_timerEx( NWK_TaskID, NWK_LINK_STATUS_EVT );
    osal_clear_event( NWK_TaskID, NWK_LINK_STATUS_EVT );
  }

  if (ZG_DEVICE_ENDDEVICE_TYPE)
  {
    // Save polling values to be restored after rejoin
    if ( ra == TRUE )
    {
       ZDApp_SavedPollRate = zgPollRate;
       savedResponseRate = zgResponsePollRate;
       savedQueuedPollRate = zgQueuedPollRate;
    }

    // Disable polling
    NLME_SetPollRate(0);
    NLME_SetResponseRate(0);
    NLME_SetQueuedPollRate(0);
  }

  if ( ra == TRUE )
  {
    devState = DEV_NWK_DISC;
    devStartMode = MODE_REJOIN;
    _tmpRejoinState = true;

    // For rejoin, specify the extended PANID to look for
    osal_cpyExtAddr( ZDO_UseExtendedPANID, _NIB.extendedPANID );

    _NIB.nwkState = NWK_DISC;
     NLME_NwkDiscTerm();

    ZDApp_NetworkInit((uint16)(NWK_START_DELAY + ((uint16) (osal_rand() & EXTENDED_JOINING_RANDOM_MASK ))));
  }
  else
  {
    ZDApp_ResetTimerStart( LEAVE_RESET_DELAY );
  }
}

/*********************************************************************
 * @fn      ZDApp_LeaveUpdate
 *
 * @brief   Update local device data related to leaving device.
 *
 * @param   nwkAddr        - NWK address of leaving device
 * @param   extAddr        - EXT address of leaving device
 * @param   removeChildren - remove children of leaving device
 * @param   rejoin         - if device will rejoin or not
 *
 * @return  none
 */
void ZDApp_LeaveUpdate( uint16 nwkAddr, uint8* extAddr,
                        uint8 removeChildren, uint8 rejoin )
{
  uint8 TC_ExtAddr[Z_EXTADDR_LEN];
  // Remove Apps Key for leaving device
  ZDSecMgrDeviceRemoveByExtAddr(extAddr);

  // Clear SECURITY bit from Address Manager
  ZDSecMgrAddrClear( extAddr );

  if ( pbindRemoveDev )
  {
    zAddrType_t devAddr;

    // Remove bind entry and all related data
    devAddr.addrMode = Addr64Bit;
    osal_memcpy(devAddr.addr.extAddr, extAddr, Z_EXTADDR_LEN);

    pbindRemoveDev(&devAddr);
  }

  // Remove if child
  if ( ZSTACK_ROUTER_BUILD )
  {
    // Router shall notify the Trust Center that a child End Device or
    // a neighbor Router (within radius=1) has left the network
    APSME_UpdateDeviceReq_t req;

    // forward authorization to the Trust Center
    req.dstAddr    = APSME_TRUSTCENTER_NWKADDR;
    req.devAddr    = nwkAddr;
    req.devExtAddr = extAddr;
    req.status = APSME_UD_DEVICE_LEFT;

    if ( rejoin == FALSE )
    {
      if(!APSME_IsDistributedSecurity())
      {
        if ( ZG_CHECK_SECURITY_MODE == ZG_SECURITY_SE_STANDARD )
        {
          uint8 found;
          APSME_GetRequest( apsTrustCenterAddress,0, TC_ExtAddr );
          
          APSME_SearchTCLinkKeyEntry(extAddr,&found,NULL);
          
          // For ZG_GLOBAL_LINK_KEY the message has to be sent twice one
          // un-encrypted and one APS encrypted, to make sure that it can interoperate
          // with legacy Coordinator devices which can only handle one or the other.
          if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY) && ( found == FALSE ) )
          {
            req.apsSecure = FALSE;

            // send and APSME_UPDATE_DEVICE request to the trust center
            APSME_UpdateDeviceReq( &req );
          }

          // send the message APS encrypted
          req.apsSecure = TRUE;

          // send and APSME_UPDATE_DEVICE request to the trust center
          APSME_UpdateDeviceReq( &req );
        }
        else
        {
          req.apsSecure = FALSE;

          // send and APSME_UPDATE_DEVICE request to the trust center
          APSME_UpdateDeviceReq( &req );
        }
      }
    }

    NLME_RemoveChild( extAddr, removeChildren );
  }

  // Remove Routing table related entry
  RTG_RemoveRtgEntry( nwkAddr, 0 );

  // Remove entry from neighborTable
  nwkNeighborRemove( nwkAddr, _NIB.nwkPanId );

  // Schedule to save data to NV
  ZDApp_NwkWriteNVRequest();
}

/*********************************************************************
 * @fn      ZDApp_NetworkDiscoveryReq
 *
 * @brief   Request a network discovery.
 *
 * @param  scanChannels -
 * @param  scanDuration -
 *
 * @return  ZStatus_t
 */
ZStatus_t ZDApp_NetworkDiscoveryReq( uint32 scanChannels, uint8 scanDuration)
{
  // Setup optional filters - tbd

  // Request NLME network discovery
  return NLME_NetworkDiscoveryRequest(scanChannels, scanDuration);
}

/*********************************************************************
 * @fn      ZDApp_JoinReq
 *
 * @brief   Request the device to join a parent in a network.
 *
 * @param   channel -
 * @param   panID -
 *
 * @return  ZStatus_t
 */
ZStatus_t ZDApp_JoinReq( uint8 channel, uint16 panID, uint8 *extendedPanID,
                         uint16 chosenParent, uint8 parentDepth, uint8 stackProfile )
{
  // Sync up the node with the stack profile (In the case where a pro device
  // joins a non-pro network, or verse versa)
  ZDApp_NodeProfileSync( stackProfile);

  // Request NLME Join Request
  return NLME_JoinRequest(extendedPanID, panID,channel,
                          ZDO_Config_Node_Descriptor.CapabilityFlags,
                          chosenParent, parentDepth);

}

/*********************************************************************
 * @fn      ZDApp_DeviceConfigured
 *
 * @brief   Check to see if the local device is configured (i.e., part
 *          of a network).
 *
 * @param   none
 *
 * @return  TRUE if configured. FALSE, otherwise.
 */
uint8 ZDApp_DeviceConfigured( void )
{
  uint16 nwkAddr = INVALID_NODE_ADDR;

  osal_nv_read( ZCD_NV_NIB, osal_offsetof( nwkIB_t, nwkDevAddress ),
                sizeof( uint16), &nwkAddr );

  // Does the NIB have anything more than default?
  return ( nwkAddr == INVALID_NODE_ADDR ? FALSE : TRUE );
}

/*********************************************************************
 * CALLBACK FUNCTIONS
 */


/*********************************************************************
 * @fn      ZDApp_SendEventMsg()
 *
 * @brief
 *
 *   Sends a Network Join message
 *
 * @param  cmd - command ID
 * @param  len - length (in bytes) of the buf field
 * @param  buf - buffer for the rest of the message.
 *
 * @return  none
 */
void ZDApp_SendEventMsg( uint8 cmd, uint8 len, uint8 *buf )
{
  ZDApp_SendMsg( ZDAppTaskID, cmd, len, buf );
}

/*********************************************************************
 * @fn      ZDApp_SendMsg()
 *
 * @brief   Sends a OSAL message
 *
 * @param  taskID - Where to send the message
 * @param  cmd - command ID
 * @param  len - length (in bytes) of the buf field
 * @param  buf - buffer for the rest of the message.
 *
 * @return  none
 */
void ZDApp_SendMsg( uint8 taskID, uint8 cmd, uint8 len, uint8 *buf )
{
  osal_event_hdr_t *msgPtr;

  // Send the address to the task
  msgPtr = (osal_event_hdr_t *)osal_msg_allocate( len );
  if ( msgPtr )
  {
    if ( (len > 0) && (buf != NULL) )
      osal_memcpy( msgPtr, buf, len );

    msgPtr->event = cmd;
    osal_msg_send( taskID, (uint8 *)msgPtr );
  }
}

/*********************************************************************
 * Call Back Functions from NWK  - API
 */

/*********************************************************************
 * @fn          ZDO_NetworkDiscoveryConfirmCB
 *
 * @brief       This function returns a choice of PAN to join.
 *
 * @param       status - return status of the nwk discovery confirm
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDO_NetworkDiscoveryConfirmCB(uint8 status)
{
  osal_event_hdr_t msg;

  // If Scan is initiated by ZDO_MGMT_NWK_DISC_REQ
  // Send ZDO_MGMT_NWK_DISC_RSP back
#if defined ( ZDO_MGMT_NWKDISC_RESPONSE )
  if ( zdappMgmtNwkDiscReqInProgress )
  {
    zdappMgmtNwkDiscReqInProgress = false;
    ZDO_FinishProcessingMgmtNwkDiscReq();
  }
  else
#endif
  {
    // Pass the confirm to another task if it registers the callback
    // Otherwise, pass the confirm to ZDApp.
    if (zdoCBFunc[ZDO_NWK_DISCOVERY_CNF_CBID] != NULL )
    {
      zdoCBFunc[ZDO_NWK_DISCOVERY_CNF_CBID]( (void*)&status );
    }
    else
    {
      // Otherwise, send scan confirm to ZDApp task to proceed
      msg.status = ZDO_SUCCESS;

      ZDApp_SendMsg( ZDAppTaskID, ZDO_NWK_DISC_CNF, sizeof(osal_event_hdr_t), (uint8 *)&msg );
    }
  }
  return (ZSuccess);
}  // ZDO_NetworkDiscoveryConfirmCB

/*********************************************************************
 * @fn          ZDApp_NwkDescListProcessing
 *
 * @brief       This function process the network discovery result and select
 *              a parent device to join itself.
 *
 * @param       none
 *
 * @return      ZStatus_t
 */

networkDesc_t* ZDApp_NwkDescListProcessing(void)
{
  networkDesc_t *pNwkDesc;
  uint8 i, ResultCount = 0;
  uint8 stackProfile;
  uint8 stackProfilePro;
  uint8 selected;

  // Count the number of nwk descriptors in the list
  pNwkDesc = nwk_getNwkDescList();
  while (pNwkDesc)
  {
    ResultCount++;
    pNwkDesc = pNwkDesc->nextDesc;
  }

  // process discovery results
  stackProfilePro = FALSE;
  selected = FALSE;


  for ( stackProfile = 0; stackProfile < STACK_PROFILE_MAX; stackProfile++ )
  {
    pNwkDesc = nwk_getNwkDescList();
    for ( i = 0; i < ResultCount; i++, pNwkDesc = pNwkDesc->nextDesc )
    {
       if ( nwk_ExtPANIDValid( ZDO_UseExtendedPANID ) == true )
      {
        // If the extended Pan ID is commissioned to a non zero value
        // Only join the Pan that has match EPID
        if ( osal_ExtAddrEqual( ZDO_UseExtendedPANID, pNwkDesc->extendedPANID) == false )
          continue;

      }
      else if ( zgConfigPANID != 0xFFFF )
      {
        // PAN Id is preconfigured. check if it matches
        if ( pNwkDesc->panId != zgConfigPANID )
          continue;
      }

      if ( pNwkDesc->chosenRouter != _NIB.nwkCoordAddress || _NIB.nwkCoordAddress == INVALID_NODE_ADDR )
      {
        // check that network is allowing joining
        if ( ZSTACK_ROUTER_BUILD )
        {
          if ( stackProfilePro == FALSE )
          {
            if ( !pNwkDesc->routerCapacity )
            {
              continue;
            }
          }
          else
          {
            if ( !pNwkDesc->deviceCapacity )
            {
              continue;
            }
          }
        }
        else if ( ZSTACK_END_DEVICE_BUILD )
        {
          if ( !pNwkDesc->deviceCapacity )
          {
            continue;
          }
        }
      }

      // check version of zigbee protocol
      if ( pNwkDesc->version != _NIB.nwkProtocolVersion )
        continue;

      // check version of stack profile
      if ( pNwkDesc->stackProfile != zgStackProfile  )
      {
        if ( ((zgStackProfile == HOME_CONTROLS) && (pNwkDesc->stackProfile == ZIGBEEPRO_PROFILE))
            || ((zgStackProfile == ZIGBEEPRO_PROFILE) && (pNwkDesc->stackProfile == HOME_CONTROLS))  )
        {
          stackProfilePro = TRUE;
        }

        if ( stackProfile == 0 )
        {
          continue;
        }
      }

      break;
    }

    if (i < ResultCount)
    {
     selected = TRUE;
      break;
    }

    // break if selected or stack profile pro wasn't found
    if ( (selected == TRUE) || (stackProfilePro == FALSE) )
    {
      break;
    }
  }

  if ( i == ResultCount )
  {
    nwk_desc_list_free();
    return (NULL);   // couldn't find appropriate PAN to join !
  }
  else
  {
    return (pNwkDesc);
  }
}

/*********************************************************************
 * @fn          ZDO_NetworkFormationConfirmCB
 *
 * @brief       This function reports the results of the request to
 *              initialize a coordinator in a network.
 *
 * @param       Status - Result of NLME_NetworkFormationRequest()
 *
 * @return      none
 */
void ZDO_NetworkFormationConfirmCB( ZStatus_t Status )
{
  nwkStatus = (byte)Status;

  if ( Status == ZSUCCESS )
  {
    bdb_setNodeIsOnANetwork(TRUE);
    
    if(ZG_DEVICE_COORDINATOR_TYPE)
    {
      // LED on shows Coordinator started
      zdoHalLedSet ( HAL_LED_3, HAL_LED_MODE_ON );

      zdoHalLedSet (HAL_LED_4, HAL_LED_MODE_OFF);

        #if defined ( ZBIT )
            SIM_SetColor(0xd0ffd0);
        #endif

      if ( devState == DEV_HOLD )
      {
        ZDApp_ChangeState( DEV_COORD_STARTING );
      }
    }

    if(ZG_DEVICE_RTR_TYPE)
    {
      uint8 x;
      uint8 tmpKey[SEC_KEY_LEN] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
      
      if(APSME_IsDistributedSecurity())
      {
        //Check if we have an extended PANID assigned
        if(nwk_ExtPANIDValid( _NIB.extendedPANID ) == false)
        {
          //Set the extended PANID as the extended address
          osal_cpyExtAddr( _NIB.extendedPANID, aExtendedAddress );
        } 
        
        ZMacSetReq(MAC_SUPERFRAME_PAN_COORD,0);
        
        //Set the MAC address
        ZMacSetReq( ZMacShortAddress, (uint8 *)&(_NIB.nwkDevAddress) );

        if ( _NIB.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
          x = true;
        else
          x = false;
        ZMacSetReq( ZMacRxOnIdle, &x );

        // Change NIB state to router for restore
        _NIB.nwkState = NWK_ROUTER;
        NLME_SetAssocFlags();

        //Restore the nwk security material using the generic index
        ZDApp_RestoreNwkSecMaterial();
        
        //Be sure to store the nwk FrameCounter if
        if(nwkFrameCounter == 0)
        {
          nwkFrameCounter = 1;
        }
        
        osal_set_event(ZDAppTaskID, ZDO_NWK_UPDATE_NV | ZDO_FRAMECOUNTER_CHANGE);
        
        ZDSecMgrGenerateRndKey(tmpKey);
         
        //Set the nwk key as the default and initialize the keySeqNum
        SSP_UpdateNwkKey( tmpKey, 0 );
        if ( !_NIB.nwkKeyLoaded )
        {
          SSP_SwitchNwkKey( 0 );
        }
        
        // Clear copy in RAM after use 
        osal_memset(tmpKey,0,SEC_KEY_LEN);
        
        //Success formation of distributed nwk
        bdb_nwkFormationAttempt(TRUE);
      }
    }
  }

  else
  {
    #if defined(BLINK_LEDS)
      zdoHalLedSet ( HAL_LED_3, HAL_LED_MODE_FLASH );  // Flash LED to show failure
    #endif
  }


  osal_set_event( ZDAppTaskID, ZDO_NETWORK_START );
}

/****************************************************************************
 * @fn          ZDApp_beaconIndProcessing
 *
 * @brief       This function processes the incoming beacon indication.
 *
 *              When another task (MT or App) is registered to process
 *              beacon indication themselves, this function will parse the
 *              beacon payload and pass the beacon descriptor to that task
 *              If no other tasks registered, this function will process
 *              the beacon payload and generate the network descriptor link
 *              list.
 *
 * @param
 *
 * @return      none
 *
 */
void ZDO_beaconNotifyIndCB( NLME_beaconInd_t *pBeacon )
{
  // Pass the beacon Indication to another task if it registers the callback
  // Otherwise, process the beacon notification here.
  if (zdoCBFunc[ZDO_BEACON_NOTIFY_IND_CBID] != NULL )
  {
    zdoCBFunc[ZDO_BEACON_NOTIFY_IND_CBID]( (void*)pBeacon );
  }
  else
  {
    networkDesc_t *pNwkDesc;
    networkDesc_t *pLastNwkDesc;
    uint8 found = false;

    // Add the network to the Network Descriptor List
    pNwkDesc = NwkDescList;
    pLastNwkDesc = NwkDescList;
    while (pNwkDesc)
    {
      if ((pNwkDesc->panId == pBeacon->panID) &&
          (pNwkDesc->logicalChannel == pBeacon->logicalChannel))
      {
        found = true;
        break;
      }
      pLastNwkDesc = pNwkDesc;
      pNwkDesc = pNwkDesc->nextDesc;
    }

    // If no existing descriptor found, make a new one and add to the list
    if (found == false)
    {
      pNwkDesc = osal_mem_alloc( sizeof(networkDesc_t)  );
      if ( !pNwkDesc )
      {
        // Memory alloc failed, discard this beacon
        return;
      }

      // Clear the network descriptor
      osal_memset( pNwkDesc, 0, sizeof(networkDesc_t)  );

      // Initialize the descriptor
      pNwkDesc->chosenRouter = INVALID_NODE_ADDR;
      pNwkDesc->chosenRouterDepth = 0xFF;

      // Save new entry into the descriptor list
      if ( !NwkDescList )
      {
        NwkDescList = pNwkDesc;
      }
      else
      {
        pLastNwkDesc->nextDesc = pNwkDesc;
      }
    }

    // Update the descriptor with the incoming beacon
    pNwkDesc->stackProfile   = pBeacon->stackProfile;
    pNwkDesc->version        = pBeacon->protocolVersion;
    pNwkDesc->logicalChannel = pBeacon->logicalChannel;
    pNwkDesc->panId          = pBeacon->panID;
    pNwkDesc->updateId       = pBeacon->updateID;

    // Save the extended PAN ID from the beacon payload only if 1.1 version network
    if ( pBeacon->protocolVersion != ZB_PROT_V1_0 )
    {
      osal_cpyExtAddr( pNwkDesc->extendedPANID, pBeacon->extendedPanID );
    }
    else
    {
      osal_memset( pNwkDesc->extendedPANID, 0xFF, Z_EXTADDR_LEN );
    }

    // check if this device is a better choice to join...
    // ...dont bother checking assocPermit flag is doing a rejoin
    if ( ( pBeacon->LQI > gMIN_TREE_LQI ) &&
        ( ( pBeacon->permitJoining == TRUE ) || ( bdb_isDeviceNonFactoryNew() ) ) )
    {
      uint8 selected = FALSE;
      uint8 capacity = FALSE;

#if defined ( ZIGBEEPRO )
      if ( ((pBeacon->LQI   > pNwkDesc->chosenRouterLinkQuality) &&
            (pBeacon->depth < MAX_NODE_DEPTH)) ||
          ((pBeacon->LQI   == pNwkDesc->chosenRouterLinkQuality) &&
           (pBeacon->depth < pNwkDesc->chosenRouterDepth)) )
      {
        selected = TRUE;
      }
#else
      if ( pBeacon->depth < pNwkDesc->chosenRouterDepth )
      {
        selected = TRUE;
      }
#endif

      if ( ZSTACK_ROUTER_BUILD )
      {
        capacity = pBeacon->routerCapacity;
      }
      else if ( ZSTACK_END_DEVICE_BUILD )
      {
        capacity = pBeacon->deviceCapacity;
      }

      if ( ( (capacity) || ( pBeacon->sourceAddr == _NIB.nwkCoordAddress ) ) && (selected) )
      {
        // this is the new chosen router for joining...
        pNwkDesc->chosenRouter            = pBeacon->sourceAddr;
        pNwkDesc->chosenRouterLinkQuality = pBeacon->LQI;
        pNwkDesc->chosenRouterDepth       = pBeacon->depth;
      }

      if ( pBeacon->deviceCapacity )
        pNwkDesc->deviceCapacity = 1;

      if ( pBeacon->routerCapacity )
        pNwkDesc->routerCapacity = 1;
    }
  }
}

/*********************************************************************
 * @fn          ZDO_StartRouterConfirmCB
 *
 * @brief       This function reports the results of the request to
 *              start functioning as a router in a network.
 *
 * @param       Status - Result of NLME_StartRouterRequest()
 *
 * @return      none
 */
void ZDO_StartRouterConfirmCB( ZStatus_t Status )
{
  nwkStatus = (byte)Status;

  if ( Status == ZSUCCESS )
  {
    // LED on shows Router started
    zdoHalLedSet ( HAL_LED_3, HAL_LED_MODE_ON );
    zdoHalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF);
    if ( devState == DEV_HOLD )
    {
      ZDApp_ChangeState( DEV_END_DEVICE );
    }
  }
#if defined(BLINK_LEDS)
  else
  {
    zdoHalLedSet( HAL_LED_3, HAL_LED_MODE_FLASH );  // Flash LED to show failure
  }
#endif

  osal_set_event( ZDAppTaskID, ZDO_ROUTER_START );
}

/*********************************************************************
 * @fn          ZDO_JoinConfirmCB
 *
 * @brief       This function allows the next higher layer to be notified
 *              of the results of its request to join itself or another
 *              device to a network.
 *
 * @param       Status - Result of NLME_JoinRequest()
 *
 * @return      none
 */
void ZDO_JoinConfirmCB( uint16 PanId, ZStatus_t Status )
{
  (void)PanId;  // remove if this parameter is used.

  nwkStatus = (byte)Status;

  if ( Status == ZSUCCESS )
  {
    if ( ZSTACK_END_DEVICE_BUILD
      || (ZSTACK_ROUTER_BUILD && BUILD_FLEXABLE && ((_NIB.CapabilityFlags & ZMAC_ASSOC_CAPINFO_FFD_TYPE) == 0)))
    {
      neighborEntry_t *pItem;

      // We don't need the neighbor table entries.
      // Clear the neighbor Table to remove beacon information
      nwkNeighborInitTable();

      // Force a neighbor table entry for the parent
      pItem = nwkNeighborFindEmptySlot();
      if ( pItem != NULL )
      {
        osal_memset( pItem, 0x00, sizeof ( neighborEntry_t  )  );
        pItem->neighborAddress = _NIB.nwkCoordAddress;
        osal_cpyExtAddr( pItem ->neighborExtAddr, _NIB. nwkCoordExtAddress );
        pItem->panId = _NIB. nwkPanId;
        pItem->linkInfo.rxLqi = DEF_LQI;
        pItem->linkInfo.txCounter = DEF_LINK_COUNTER;
        pItem->linkInfo.txCost = DEF_LINK_COST;
      }
    }

    if ( (devState == DEV_HOLD) )
    {
      ZDApp_ChangeState( DEV_NWK_JOINING );
    }

    if ( !ZG_SECURE_ENABLED )
    {
      // Notify to save info into NV
      ZDApp_NVUpdate();
    }

    NLME_SetPollRate( ZDApp_SavedPollRate );
  }

  // Pass the join confirm to higher layer if callback registered
  if (zdoCBFunc[ZDO_JOIN_CNF_CBID] != NULL )
  {
    zdoJoinCnf_t joinCnf;

    joinCnf.status = Status;
    joinCnf.deviceAddr = _NIB.nwkDevAddress;
    joinCnf.parentAddr = _NIB.nwkCoordAddress;

    zdoCBFunc[ZDO_JOIN_CNF_CBID]( (void*)&joinCnf );
  }
  
  // Notify ZDApp
  ZDApp_SendMsg( ZDAppTaskID, ZDO_NWK_JOIN_IND, sizeof(osal_event_hdr_t), (byte*)NULL );

}

/*********************************************************************
 * @fn          ZDO_PermitJoinCB
 *
 * @brief      This function is called when there is a change in the
 *             device's permit join status.
 *
 * @param       duration - the new duration
 *
 * @return      none
 */
void ZDO_PermitJoinCB( uint8 duration )
{
  // Pass the Permit Join status to higher layer if callback registered
  if (zdoCBFunc[ZDO_PERMIT_JOIN_CBID] != NULL )
  {
    zdoCBFunc[ZDO_PERMIT_JOIN_CBID]( (void*)&duration );
  }
}

/*********************************************************************
 * @fn          ZDO_AddrChangeIndicationCB
 *
 * @brief       This function notifies the application that this
 *              device's address has changed.  Could happen in
 *              a network with stochastic addressing (PRO).
 *
 * @param       newAddr - the new address
 *
 * @return      none
 */
void ZDO_AddrChangeIndicationCB( uint16 newAddr )
{
  ZDO_AddrChangeInd_t *pZDOAddrChangeMsg;
  epList_t *pItem = epList;

  // Notify to save info into NV
  ZDApp_NVUpdate();

  // Notify the applications
  osal_set_event( ZDAppTaskID, ZDO_STATE_CHANGE_EVT );

  while (pItem != NULL)
  {
    if (pItem->epDesc->endPoint != ZDO_EP)
    {
      pZDOAddrChangeMsg = (ZDO_AddrChangeInd_t *)osal_msg_allocate( sizeof( ZDO_AddrChangeInd_t ) );
      if (pZDOAddrChangeMsg != NULL)
      {
        pZDOAddrChangeMsg->hdr.event = ZDO_ADDR_CHANGE_IND;
        pZDOAddrChangeMsg->shortAddr = newAddr;
        osal_msg_send( *(pItem->epDesc->task_id), (uint8 *)pZDOAddrChangeMsg );
      }
    }
    pItem = pItem->nextDesc;
  }

  // Send out a device announce
  ZDApp_AnnounceNewAddress();
}

/*********************************************************************
 * @fn          ZDO_JoinIndicationCB
 *
 * @brief       This function allows the next higher layer of a
 *              coordinator to be notified of a remote join request.
 *
 * @param       ShortAddress - 16-bit address
 * @param       ExtendedAddress - IEEE (64-bit) address
 * @param       CapabilityFlags - Association Capability Flags
 * @param       type - of joining -
 *                          NWK_ASSOC_JOIN
 *                          NWK_ASSOC_REJOIN_UNSECURE
 *                          NWK_ASSOC_REJOIN_SECURE
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDO_JoinIndicationCB(uint16 ShortAddress, uint8 *ExtendedAddress,
                                uint8 CapabilityFlags, uint8 type)
{
  (void)ExtendedAddress;
  //check if the device is leaving before responding to rejoin request
  if( osal_get_timeoutEx( ZDAppTaskID , ZDO_DEVICE_RESET) )
  {
    return ZFailure; // device leaving , hence do not allow rejoin
  }

#if ZDO_NV_SAVE_RFDs
    (void)CapabilityFlags;

#else  // if !ZDO_NV_SAVE_RFDs
    if (CapabilityFlags & CAPINFO_DEVICETYPE_FFD)
#endif
    {
      ZDApp_NVUpdate();  // Notify to save info into NV.
    }

    if (ZG_SECURE_ENABLED)  // Send notification to TC of new device.
    {
      if ( type == NWK_ASSOC_JOIN ||
          type == NWK_ASSOC_REJOIN_UNSECURE ||
            type == NWK_ASSOC_REJOIN_SECURE )
      {
        uint16 timeToFire;
        ZDAppNewDevice_t *pNewDevice, *pDeviceList;

        pNewDevice = (ZDAppNewDevice_t *) osal_mem_alloc( sizeof(ZDAppNewDevice_t) );

        if ( pNewDevice == NULL )
        {
          // Memory alloc failed
          return ZMemError;
        }

        // Add the new device to the New Device List
        if ( ZDApp_NewDeviceList == NULL )
        {
          // The list is empty, add the first element
          ZDApp_NewDeviceList = pNewDevice;
        }
        else
        {
          pDeviceList = ZDApp_NewDeviceList;

          // Walk the list to last element
          while ( pDeviceList->next )
          {
            pDeviceList = (ZDAppNewDevice_t *) pDeviceList->next;
          }

          // Add new device at the end
          pDeviceList->next = pNewDevice;
        }

        // get the remaining time of the timer
        timeToFire = osal_get_timeoutEx( ZDAppTaskID, ZDO_NEW_DEVICE );

        pNewDevice->next = NULL;
        pNewDevice->shortAddr = ShortAddress;
        pNewDevice->timeDelta = ZDAPP_NEW_DEVICE_TIME - timeToFire;

        // Start the timer only if there is no pending timer
        if ( pNewDevice->timeDelta == ZDAPP_NEW_DEVICE_TIME )
        {
          osal_start_timerEx( ZDAppTaskID, ZDO_NEW_DEVICE, ZDAPP_NEW_DEVICE_TIME );
        }
      }
    }

    return ZSuccess;

}

/*********************************************************************
 * @fn          ZDO_ConcentratorIndicationCB
 *
 * @brief       This function allows the next higher layer of a
 *              device to be notified of existence of the concentrator.
 *
 * @param       nwkAddr - 16-bit NWK address of the concentrator
 * @param       extAddr - pointer to extended Address
 *                        NULL if not available
 * @param       pktCost - PktCost from RREQ
 *
 * @return      void
 */
void ZDO_ConcentratorIndicationCB( uint16 nwkAddr, uint8 *extAddr, uint8 pktCost )
{
  zdoConcentratorInd_t conInd;

  conInd.nwkAddr = nwkAddr;
  conInd.extAddr = extAddr;
  conInd.pktCost = pktCost;

  if( zdoCBFunc[ZDO_CONCENTRATOR_IND_CBID] != NULL )
  {
    zdoCBFunc[ZDO_CONCENTRATOR_IND_CBID]( (void*)&conInd );
  }
}

/*********************************************************************
 * @fn          ZDO_LeaveCnf
 *
 * @brief       This function allows the next higher layer to be
 *              notified of the results of its request for this or
 *              a child device to leave the network.
 *
 * @param       cnf - NLME_LeaveCnf_t
 *
 * @return      none
 */
void ZDO_LeaveCnf( NLME_LeaveCnf_t* cnf )
{
  // Check for this device
  if ( osal_ExtAddrEqual( cnf->extAddr,
                          NLME_GetExtAddr() ) == TRUE )
  {
    // Pass the leave confirm to higher layer if callback registered
    if ( ( zdoCBFunc[ZDO_LEAVE_CNF_CBID] == NULL ) ||
         ( (*zdoCBFunc[ZDO_LEAVE_CNF_CBID])( cnf ) == NULL ) )
    {
      // Prepare to leave with reset
      ZDApp_LeaveReset( cnf->rejoin );
    }
  }
  else if ( ZSTACK_ROUTER_BUILD )
  {
    // Remove device address(optionally descendents) from data
    ZDApp_LeaveUpdate( cnf->dstAddr,
                       cnf->extAddr,
                       cnf->removeChildren,
                       cnf->rejoin );
  }
}

/*********************************************************************
 * @fn          ZDO_LeaveInd
 *
 * @brief       This function allows the next higher layer of a
 *              device to be notified of a remote leave request or
 *              indication.
 *
 * @param       ind - NLME_LeaveInd_t
 *
 * @return      none
 */
void ZDO_LeaveInd( NLME_LeaveInd_t* ind )
{
  uint8 leave;

  // NWK layer filters out illegal requests
  if ( ind->request == TRUE )
  {
    byte temp = FALSE;
    
    // Only respond if we are not rejoining the network
    if ( ind->rejoin == FALSE )
    {
      // Notify network of leave
      NLME_LeaveRsp_t rsp;
      rsp.rejoin = ind->rejoin;

      if ( ZSTACK_ROUTER_BUILD )
      {
        rsp.removeChildren = ind->removeChildren;
      }
      else if ( ZSTACK_END_DEVICE_BUILD )
      {
        NLME_SetResponseRate(0);
        NLME_SetQueuedPollRate(0);
        rsp.removeChildren = 0;
      }
      
      bdb_setFN();
       
      NLME_LeaveRsp( &rsp );
    }

    if ( ZSTACK_END_DEVICE_BUILD )
    {
      // Stop polling and get ready to reset
      NLME_SetPollRate( 0 );
    }

    // Prepare to leave with reset
    ZDApp_LeaveReset( ind->rejoin );
    
    //Turn on the radio to avoid sending packets after sending the leave    
    ZMacSetReq(ZMacRxOnIdle, &temp);
  }
  else
  {
    leave = FALSE;

    // Check if this device needs to leave as a child or descendent
    if ( ind->srcAddr == NLME_GetCoordShortAddr() )
    {
      if ( ( ind->removeChildren == TRUE )   )
      {
        leave = TRUE;
      }
      else if ( ZDO_Config_Node_Descriptor.LogicalType == NODETYPE_DEVICE)
      {
        // old parents is leaving the network, child needs to search for a new parent
        ind->rejoin = TRUE;
        leave = TRUE;
      }

    }
    else if ( ind->removeChildren == TRUE )
    {
      // Check NWK address allocation algorithm
      //leave = RTG_ANCESTOR(nwkAddr,thisAddr);
    }

    if ( leave == TRUE )
    {
      // Prepare to leave with reset
      ZDApp_LeaveReset( ind->rejoin );
    }
    else
    {
      // Remove device address(optionally descendents) from data
      ZDApp_LeaveUpdate( ind->srcAddr,
                         ind->extAddr,
                         ind->removeChildren,
                         ind->rejoin );
    }
  }

  // Pass the leave indication to higher layer if callback registered.
  if (zdoCBFunc[ZDO_LEAVE_IND_CBID] != NULL)
  {
    (void)zdoCBFunc[ZDO_LEAVE_IND_CBID](ind);
  }
}

/*********************************************************************
 * @fn          ZDO_SyncIndicationCB
 *
 * @brief       This function allows the next higher layer of a
 *              coordinator to be notified of a loss of synchronization
 *                          with the parent/child device.
 *
 * @param       type: 0 - child; 1 - parent
 *
 *
 * @return      none
 */
void ZDO_SyncIndicationCB( uint8 type, uint16 shortAddr )
{
  (void)shortAddr;  // Remove this line if this parameter is used.

  if ( ZSTACK_END_DEVICE_BUILD
    || (ZSTACK_ROUTER_BUILD && BUILD_FLEXABLE && ((_NIB.CapabilityFlags & ZMAC_ASSOC_CAPINFO_FFD_TYPE) == 0)))
  {
    if ( type == 1 && retryCnt == 0 )
    {
      // We lost contact with our parent.  Clear the neighbor Table.
      nwkNeighborInitTable();
      
      //If we are Factory new, then report fail on association
      if(!bdb_isDeviceNonFactoryNew())
      {
        bdb_nwkAssocAttemt(FALSE);
      }
#if (ZG_BUILD_ENDDEVICE_TYPE)
      else
      {
        //We lost our parent
        bdb_parentLost();
      }
#endif
    }
  }
}

/*********************************************************************
 * @fn          ZDO_ManytoOneFailureIndicationCB
 *
 * @brief       This function allows the next higher layer of a
 *              concentrator to be notified of a many-to-one route
 *              failure.
 *
 * @param       none
 *
 *
 * @return      none
 */
void ZDO_ManytoOneFailureIndicationCB()
{
  // By default, the concentrator automatically redo many-to-one route
  // discovery to update all many-to-one routes in the network
  // If you want anything processing other than the default,
  // please replace the following code.

  RTG_MTORouteReq();
}

/*********************************************************************
 * @fn          ZDO_PollConfirmCB
 *
 * @brief       This function allows the next higher layer to be
 *              notified of a Poll Confirm.
 *
 * @param       none
 *
 * @return      none
 */
void ZDO_PollConfirmCB( uint8 status )
{
  (void)status;  // Remove this line if this parameter is used.
  return;
}

/*********************************************************************
 * @fn          ZDO_NetworkStatusCB
 *
 * @brief       Network Status Callback function
 *
 * @param       nwkDstAddr - message's destination address- used to determine
 *                           if the message was intended for this device or
 *                           a sleeping end device.
 * @param       statusCode - message's status code (ie. NWKSTAT_NONTREE_LINK_FAILURE)
 * @param       dstAddr - the destination address related to the status code
 *
 * @return      none
 */
void ZDO_NetworkStatusCB( uint16 nwkDstAddr, uint8 statusCode, uint16 dstAddr )
{
  (void)dstAddr;     // Remove this line if this parameter is used.

  if ( (nwkDstAddr == NLME_GetShortAddr())
      && (statusCode == NWKSTAT_NONTREE_LINK_FAILURE) )
  {
    // Routing error for dstAddr, this is informational and a Route
    // Request should happen automatically.
  }
}

/******************************************************************************
 * @fn          ZDApp_NwkWriteNVRequest (stubs AddrMgrWriteNVRequest)
 *
 * @brief       Stub routine implemented by NHLE. NHLE should call
 *              <AddrMgrWriteNV> when appropriate.
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_NwkWriteNVRequest( void )
{
#if defined ( NV_RESTORE )
  if ( !osal_get_timeoutEx( ZDAppTaskID, ZDO_NWK_UPDATE_NV ) )
  {
    // Trigger to save info into NV
    ZDApp_NVUpdate();
  }
#endif
}

/*********************************************************************
 * Call Back Functions from Security  - API
 */

 /*********************************************************************
 * @fn          ZDO_UpdateDeviceIndication
 *
 * @brief       This function notifies the "Trust Center" of a
 *              network when a device joins or leaves the network.
 *
 * @param       extAddr - pointer to 64 bit address of new device
 * @param       status  - 0 if a new device joined securely
 *                      - 1 if a new device joined un-securely
 *                      - 2 if a device left the network
 *
 * @return      true if newly joined device should be allowed to
 *                                              remain on network
 */
ZStatus_t ZDO_UpdateDeviceIndication( uint8 *extAddr, uint8 status )
{
  // can implement a network access policy based on the
  // IEEE address of newly joining devices...
  (void)extAddr;
  (void)status;

  return ZSuccess;
}

/*********************************************************************
 * @fn          ZDApp_InMsgCB
 *
 * @brief       This function is called to pass up any message that is
 *              not yet supported.  This allows for the developer to
 *              support features themselves..
 *
 * @return      none
 */
void ZDApp_InMsgCB( zdoIncomingMsg_t *inMsg )
{
  if ( inMsg->clusterID & ZDO_RESPONSE_BIT )
  {
    // Handle the response message
  }
  else
  {
    // Handle the request message by sending a generic "not supported".
    // Device Announce doesn't have a response.
    if ( !(inMsg->wasBroadcast) && inMsg->clusterID != Device_annce )
    {
      ZDP_GenericRsp( inMsg->TransSeq, &(inMsg->srcAddr), ZDP_NOT_SUPPORTED, 0,
                      (uint16)(inMsg->clusterID | ZDO_RESPONSE_BIT), inMsg->SecurityUse );
    }
  }
}


/*********************************************************************
 * @fn      ZDApp_ChangeMatchDescRespPermission()
 *
 * @brief   Changes the Match Descriptor Response permission.
 *
 * @param   endpoint - endpoint to allow responses
 * @param   action - true to allow responses, false to not
 *
 * @return  none
 */
void ZDApp_ChangeMatchDescRespPermission( uint8 endpoint, uint8 action )
{
  // Store the action
  afSetMatch( endpoint, action );
}

/*********************************************************************
 * @fn      ZDApp_NetworkInit()
 *
 * @brief   Used to start the network joining process
 *
 * @param   delay - mSec delay to wait before starting
 *
 * @return  none
 */
void ZDApp_NetworkInit( uint16 delay )
{
  if ( delay )
  {
    // Wait awhile before starting the device
    osal_start_timerEx( ZDAppTaskID, ZDO_NETWORK_INIT, delay );
  }
  else
  {
    osal_set_event( ZDAppTaskID, ZDO_NETWORK_INIT );
  }
}

/*********************************************************************
 * @fn      ZDApp_NwkStateUpdateCB()
 *
 * @brief   This function notifies that this device's network
 *          state info has been changed.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_NwkStateUpdateCB( void )
{
  // Notify to save info into NV
  if ( !osal_get_timeoutEx( ZDAppTaskID, ZDO_NWK_UPDATE_NV ) )
  {
    // Trigger to save info into NV
    ZDApp_NVUpdate();
  }
}

/*********************************************************************
 * @fn      ZDApp_NodeProfileSync()
 *
 * @brief   Sync node with stack profile.
 *
 * @param   stackProfile - stack profile of the network to join
 *
 * @return  none
 */
void ZDApp_NodeProfileSync( uint8 stackProfile )
{
  if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_DEVICETYPE_FFD  )
  {
    if ( stackProfile != zgStackProfile )
    {
      ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_DEVICE;
      ZDO_Config_Node_Descriptor.CapabilityFlags = CAPINFO_DEVICETYPE_RFD | CAPINFO_POWER_AC | CAPINFO_RCVR_ON_IDLE;
      NLME_SetBroadcastFilter( ZDO_Config_Node_Descriptor.CapabilityFlags );
    }
  }
}

/*********************************************************************
 * @fn      ZDApp_StartJoiningCycle()
 *
 * @brief   Starts the joining cycle of a device.  This will only
 *          continue an already started (or stopped) joining cycle.
 *
 * @param   none
 *
 * @return  TRUE if joining stopped, FALSE if joining or rejoining
 */
uint8 ZDApp_StartJoiningCycle( void )
{
  if ( devState == DEV_INIT || devState == DEV_NWK_DISC )
  {
    continueJoining = TRUE;
    ZDApp_NetworkInit( 0 );

    return ( TRUE );
  }
  else
    return ( FALSE );
}

/*********************************************************************
 * @fn      ZDApp_StopJoiningCycle()
 *
 * @brief   Stops the joining or rejoining process of a device.
 *
 * @param   none
 *
 * @return  TRUE if joining stopped, FALSE if joining or rejoining
 */
uint8 ZDApp_StopJoiningCycle( void )
{
  if ( devState == DEV_INIT || devState == DEV_NWK_DISC || devState == DEV_NWK_BACKOFF )
  {
    continueJoining = FALSE;
    return ( TRUE );
  }
  else
    return ( FALSE );
}

/*********************************************************************
 * @fn      ZDApp_AnnounceNewAddress()
 *
 * @brief   Send Device Announce and hold all transmissions for
 *          new address timeout.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_AnnounceNewAddress( void )
{
#if defined ( ZIGBEEPRO )
  // Turn off data request hold
  APSME_HoldDataRequests( 0 );
#endif

  ZDP_DeviceAnnce( NLME_GetShortAddr(), NLME_GetExtAddr(),
                     ZDO_Config_Node_Descriptor.CapabilityFlags, 0 );

#if defined ( ZIGBEEPRO )
  // Setup the timeout
  APSME_HoldDataRequests( ZDAPP_HOLD_DATA_REQUESTS_TIMEOUT );
#endif

  if ( ZSTACK_END_DEVICE_BUILD )
  {
    if ( zgChildAgingEnable == TRUE )
    {
      uint8 coordExtAddr[Z_EXTADDR_LEN];

      // Send the message to parent
      NLME_GetCoordExtAddr( coordExtAddr );
      NLME_SendEndDevTimeoutReq( NLME_GetCoordShortAddr(), coordExtAddr,
                                 zgEndDeviceTimeoutValue,
                                 zgEndDeviceConfiguration );
    }
  }
}

/*********************************************************************
 * @fn      ZDApp_SendParentAnnce()
 *
 * @brief   Send Parent Announce message.
 *
 * @param   none
 *
 * @return  none
 */
void ZDApp_SendParentAnnce( void )
{
  uint8 count;
  uint8 *childInfo;

  childInfo = AssocMakeListOfRfdChild( &count );

  if ( childInfo != NULL )
  {
    if ( count > 0 )
    {
      zAddrType_t dstAddr;

      dstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVZCZR;

      ZDP_ParentAnnceReq( dstAddr, count, childInfo, 0 );
    }

    // Free the list after the message has been sent
    osal_mem_free( childInfo );
  }
}

/*********************************************************************
 * @fn          ZDApp_NVUpdate
 *
 * @brief       Set the NV Update Timer.
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_NVUpdate( void )
{
#if defined ( NV_RESTORE )
  if ( (ZSTACK_END_DEVICE_BUILD)
       || (ZSTACK_ROUTER_BUILD
           && (_NIB.CapabilityFlags & CAPINFO_DEVICETYPE_FFD) == 0) )
  {
    // No need to wait, set the event to save the state
    osal_set_event(ZDAppTaskID, ZDO_NWK_UPDATE_NV);
  }
  else
  {
    // To allow for more changes to the network state before saving
    osal_start_timerEx( ZDAppTaskID, ZDO_NWK_UPDATE_NV, ZDAPP_UPDATE_NWK_NV_TIME );
  }
#endif
}

/*********************************************************************
 * @fn      ZDApp_CoordStartPANIDConflictCB()
 *
 * @brief   Returns a PAN ID for the network layer to use during
 *          a coordinator start and there is another network with
 *          the intended PANID.
 *
 * @param   panid - the intended PAN ID
 *
 * @return  PANID to try
 */
uint16 ZDApp_CoordStartPANIDConflictCB( uint16 panid )
{
  return ( panid + 1 );
}

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
void ZDO_SrcRtgIndCB (uint16 srcAddr, uint8 relayCnt, uint16* pRelayList )
{
  zdoSrcRtg_t srcRtg;

  srcRtg.srcAddr = srcAddr;
  srcRtg.relayCnt = relayCnt;
  srcRtg.pRelayList = pRelayList;

  if( zdoCBFunc[ZDO_SRC_RTG_IND_CBID] != NULL )
  {
    zdoCBFunc[ZDO_SRC_RTG_IND_CBID]( (void*)&srcRtg );
  }
}

/*********************************************************************
 * @fn          ZDApp_InitZdoCBFunc
 *
 * @brief       Call this function to initialize zdoCBFunc[]
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_InitZdoCBFunc( void )
{
  uint8 i;

  for ( i=0; i< MAX_ZDO_CB_FUNC; i++ )
  {
    zdoCBFunc[i] = NULL;
  }
}

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
ZStatus_t ZDO_RegisterForZdoCB( uint8 indID, pfnZdoCb pFn )
{
  // Check the range of the indication ID
  if ( indID < MAX_ZDO_CB_FUNC )
  {
    zdoCBFunc[indID] = pFn;
    return ZSuccess;
  }

  return ZInvalidParameter;
}

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
ZStatus_t ZDO_DeregisterForZdoCB( uint8 indID )
{
  // Check the range of the indication ID
  if ( indID < MAX_ZDO_CB_FUNC )
  {
    zdoCBFunc[indID] = NULL;
    return ZSuccess;
  }

  return ZInvalidParameter;
}

#if defined ( ZDP_BIND_VALIDATION )
#if defined ( REFLECTOR )
/*********************************************************************
 * @fn          ZDApp_SetPendingBindDefault
 *
 * @brief       This function initializes a specific entry of pending
 *              Bind Request.
 *
 * @param       pendBindReq - pointer to the entry in the table
 *
 * @return      none
 */
void ZDApp_SetPendingBindDefault( ZDO_PendingBindReq_t *pendBindReq )
{
  // Set it to an initial value
  osal_memset( pendBindReq, 0xFF, sizeof( ZDO_PendingBindReq_t ) );

  // We are checking for age 0 for aged-out records
  pendBindReq->age = 0;
}

/*********************************************************************
 * @fn          ZDApp_InitPendingBind
 *
 * @brief       This function initializes the buffer that holds
 *              pending Bind Request messages if no valid NWK address
 *              exists in Address Manager and a Network Address Req
 *              has been sent out.
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_InitPendingBind( void )
{
  if ( ZDAppPendingBindReq == NULL )
  {
    if ( ( ZDAppPendingBindReq = osal_mem_alloc( sizeof(ZDO_PendingBindReq_t) * MAX_PENDING_BIND_REQ ) ) != NULL )
    {
      uint8 i;

      for ( i = 0; i < MAX_PENDING_BIND_REQ; i++ )
      {
        // Set to default values
        ZDApp_SetPendingBindDefault( &ZDAppPendingBindReq[i] );
      }
    }
  }
}

/*********************************************************************
 * @fn          ZDApp_GetEmptyPendingBindReq
 *
 * @brief       This function looks for an empty entry.
 *
 * @param       none
 *
 * @return      Pointer to entry
 */
ZDO_PendingBindReq_t *ZDApp_GetEmptyPendingBindReq( void )
{
  uint8 i;

  if ( ZDAppPendingBindReq != NULL )
  {
    for ( i = 0; i < MAX_PENDING_BIND_REQ; i++ )
    {
      if ( ZDAppPendingBindReq[i].age == 0 )
      {
        return ( &ZDAppPendingBindReq[i] );
      }
    }
  }

  // No empty entry was found
  return NULL;
}

/*********************************************************************
 * @fn          ZDApp_ProcessPendingBindReq
 *
 * @brief       Process pending entry based on EXT address.
 *
 * @param       extAddr - of device to look up
 *
 * @return      none
 */
void ZDApp_ProcessPendingBindReq( uint8 *extAddr )
{
  uint8 i;

  // Loop through all the pending entries for that Ext Address
  // to create Bind Entries and send Bind Rsp
  if ( ZDAppPendingBindReq != NULL )
  {
    for ( i = 0; i < MAX_PENDING_BIND_REQ; i++ )
    {
      if ( osal_memcmp( ZDAppPendingBindReq[i].bindReq.dstAddress.addr.extAddr,
                        extAddr, Z_EXTADDR_LEN ) == TRUE )
      {
        uint8 bindStat = ZDP_TABLE_FULL; // Assume table is full

        // Add Bind entry
        if ( APSME_BindRequest( ZDAppPendingBindReq[i].bindReq.srcEndpoint,
                                ZDAppPendingBindReq[i].bindReq.clusterID,
                                &(ZDAppPendingBindReq[i].bindReq.dstAddress),
                                ZDAppPendingBindReq[i].bindReq.dstEndpoint ) == ZSuccess )
        {
          // valid entry
          bindStat = ZDP_SUCCESS;

          // Notify to save info into NV
          ZDApp_NVUpdate();
        }

        // Send back a response message
        ZDP_BindRsp( ZDAppPendingBindReq[i].transSeq, &(ZDAppPendingBindReq[i].srcAddr),
                     bindStat, ZDAppPendingBindReq[i].securityUse );

        // Set the pending request entry to default values
        ZDApp_SetPendingBindDefault( &ZDAppPendingBindReq[i] );
      }
    }
  }
}

/*********************************************************************
 * @fn          ZDApp_AgeOutPendingBindEntry
 *
 * @brief       Age out pending Bind Req entries.
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_AgeOutPendingBindEntry( void )
{
  uint8 i;
  bool entryFound = FALSE;

  if ( ZDAppPendingBindReq != NULL )
  {
    for ( i = 0; i < MAX_PENDING_BIND_REQ; i++ )
    {
      if ( ZDAppPendingBindReq[i].age > 1 )
      {
        ZDAppPendingBindReq[i].age--;

        entryFound = TRUE;
      }
      else if ( ZDAppPendingBindReq[i].age == 1 )
      {
        // The record has aged out and has valid data
        AddrMgrEntry_t entry;
        uint8 bindStat = ZDP_TABLE_FULL; // Assume table is full

        entry.user = ADDRMGR_USER_BINDING;

        // Remove the entry in address manager so we do not keep entries
        // with invalid addresses
        AddrMgrExtAddrSet( entry.extAddr, ZDAppPendingBindReq[i].bindReq.dstAddress.addr.extAddr );

        if ( AddrMgrEntryLookupExt( &entry ) == TRUE )
        {
          if ( entry.nwkAddr == INVALID_NODE_ADDR )
          {
            // Release the entry that contains an invalid NWK address
            AddrMgrEntryRelease( &entry );
          }
        }

        // Send the Bind Response with failure status
        ZDP_BindRsp( ZDAppPendingBindReq[i].transSeq,
                     &(ZDAppPendingBindReq[i].srcAddr),
                     bindStat, ZDAppPendingBindReq[i].securityUse );

        // Clear the record and set to default values
        ZDApp_SetPendingBindDefault( &ZDAppPendingBindReq[i] );
      }
    }
  }

  if ( entryFound == FALSE )
  {
    osal_stop_timerEx( ZDAppTaskID, ZDO_PENDING_BIND_REQ_EVT );
  }
  else
  {
    osal_start_timerEx( ZDAppTaskID, ZDO_PENDING_BIND_REQ_EVT,
                        AGE_OUT_PEND_BIND_REQ_DELAY );
  }
}
#endif
#endif

/*********************************************************************
 * @fn          ZDO_ChangeState
 *
 * @brief       Chance the device state
 *
 * @param       state - new state
 *
 * @return      none
 */
void ZDApp_ChangeState( devStates_t state )
{
  if ( devState != state )
  {
    devState = state;
    osal_set_event( ZDAppTaskID, ZDO_STATE_CHANGE_EVT );
  }
}

/*********************************************************************
 * @fn      ZDApp_SetRejoinScanDuration()
 *
 * @brief   Sets scan duration for rejoin for an end device
 *
 * @param   rejoinScanDuration - milliseconds
 *
 * @return  none
 */
void ZDApp_SetRejoinScanDuration( uint32 rejoinScanDuration )
{
  zgDefaultRejoinScan = rejoinScanDuration;
}

/*********************************************************************
 * @fn      ZDApp_SetRejoinBackoffDuration()
 *
 * @brief   Sets rejoin backoff duration for rejoin for an end device
 *
 * @param   rejoinBackoffDuration - milliseconds
 *
 * @return  none
 */
void ZDApp_SetRejoinBackoffDuration( uint32 rejoinBackoffDuration )
{
  zgDefaultRejoinBackoff = rejoinBackoffDuration;
}

/*********************************************************************
 * @fn          ZDApp_SetParentAnnceTimer
 *
 * @brief       This function sets up the link status timer.
 *
 * @param       none
 *
 * @return      none
 */
void ZDApp_SetParentAnnceTimer( void )
{
  // Parent Announce shall be sent no earlier than 10 seconds
  uint32 timeout = 10000;

  // Add with jitter of up to 10 seconds
  timeout += (osal_rand() & 0x2710);

  // Set timer to send the message
  osal_start_timerEx( ZDAppTaskID, ZDO_PARENT_ANNCE_EVT, timeout );
}

/*********************************************************************
*********************************************************************/
