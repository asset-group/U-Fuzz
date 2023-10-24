/**************************************************************************************************
  Filename:       bdb.h
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Base Device Behavior definitions.


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

#ifndef BDB_H
#define BDB_H


#ifdef __cplusplus
extern "C"
{
#endif
  
  
  
/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "ssp.h"
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "zcl.h"
#include "zcl_general.h"  
#include "ZDProfile.h"
  
#if (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ROUTER) || (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ENDDEVICE)
//Optional
  #if defined ( INTER_PAN ) && ( defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET ) )
    #define BDB_TOUCHLINK_CAPABILITY_ENABLED          1
  #else
    #define BDB_TOUCHLINK_CAPABILITY_ENABLED          0
  #endif
#else
  #if defined ( INTER_PAN ) && ( defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET ) )
    #error TouchLink cannot be enabled for coordinator. Please make sure not to define either BDB_TL_INITIATOR or BDB_TL_TARGET
  #endif
#endif

//Configured per device  
#ifndef BDB_FINDING_BINDING_CAPABILITY_ENABLED
  #define BDB_FINDING_BINDING_CAPABILITY_ENABLED    1
#endif
#ifndef BDB_NETWORK_STEERING_CAPABILITY_ENABLED
  #define BDB_NETWORK_STEERING_CAPABILITY_ENABLED   1
#endif
#ifndef BDB_NETWORK_FORMATION_CAPABILITY_ENABLED
  #define BDB_NETWORK_FORMATION_CAPABILITY_ENABLED  1
#endif
  
/*********************************************************************
 * MACROS
 */
  
// bdbNodeCommissioningCapability MACROS  
#if (ZG_BUILD_COORDINATOR_TYPE)  
  #define BDB_NETWORK_STEERING_CAPABILITY      (BDB_NETWORK_STEERING_CAPABILITY_ENABLED<<0)
  #define BDB_NETWORK_FORMATION_CAPABILITY     (BDB_NETWORK_FORMATION_CAPABILITY_ENABLED<<1)
  #define BDB_FINDING_BINDING_CAPABILITY       (BDB_FINDING_BINDING_CAPABILITY_ENABLED<<2)
  #define BDB_TOUCHLINK_CAPABILITY             (0<<3)                                       //ZC cannot perform TL proceedure
#endif
#if (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ROUTER )
  #define BDB_NETWORK_STEERING_CAPABILITY      (BDB_NETWORK_STEERING_CAPABILITY_ENABLED<<0)
  #define BDB_NETWORK_FORMATION_CAPABILITY     (BDB_ROUTER_FORM_DISTRIBUTED_NWK_ENABLED<<1)
  #define BDB_FINDING_BINDING_CAPABILITY       (BDB_FINDING_BINDING_CAPABILITY_ENABLED<<2)
  #define BDB_TOUCHLINK_CAPABILITY             (BDB_TOUCHLINK_CAPABILITY_ENABLED<<3)      
#endif
#if (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ENDDEVICE)
  #define BDB_NETWORK_STEERING_CAPABILITY      (BDB_NETWORK_STEERING_CAPABILITY_ENABLED<<0)
  #define BDB_NETWORK_FORMATION_CAPABILITY     (0<<1)                                       //ZED cannot form nwk
  #define BDB_FINDING_BINDING_CAPABILITY       (BDB_FINDING_BINDING_CAPABILITY_ENABLED<<2)
  #define BDB_TOUCHLINK_CAPABILITY             (BDB_TOUCHLINK_CAPABILITY_ENABLED<<3)      
#endif  
  


//Initialization structure for bdb attributes
#if ((ZSTACK_DEVICE_BUILD & DEVICE_BUILD_COORDINATOR)  &&  (ZSTACK_DEVICE_BUILD & (DEVICE_BUILD_ROUTER | DEVICE_BUILD_ENDDEVICE) ) )
#define BDB_ATTRIBUTES_DEFAULT_CONFIG {.bdbNodeJoinLinkKeyType           = BDB_DEFAULT_NODE_JOIN_LINK_KEY_TYPE,          \
                                       .bdbTCLinkKeyExchangeAttempts     = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS,     \
                                       .bdbTCLinkKeyExchangeAttemptsMax  = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX, \
	                               .bdbTCLinkKeyExchangeMethod       = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_METHOD,      \
                                       .bdbCommissioningGroupID          = BDB_DEFAULT_COMMISSIONING_GROUP_ID,           \
	                               .bdbCommissioningMode             = BDB_DEFAULT_COMMISSIONING_MODE,               \
	                               .bdbCommissioningStatus           = BDB_DEFAULT_COMMISSIONING_STATUS,             \
	                               .bdbNodeCommissioningCapability   = BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY,    \
	                               .bdbNodeIsOnANetwork              = BDB_DEFAULT_NODE_IS_ON_A_NETWORK,             \
	                               .bdbPrimaryChannelSet             = BDB_DEFAULT_PRIMARY_CHANNEL_SET,              \
	                               .bdbScanDuration                  = BDB_DEFAULT_SCAN_DURATION,                    \
	                               .bdbSecondaryChannelSet           = BDB_DEFAULT_SECONDARY_CHANNEL_SET,            \
                                       .bdbJoinUsesInstallCodeKey        = BDB_DEFAULT_JOIN_USES_INSTALL_CODE_KEY,       \
                                       .bdbTrustCenterNodeJoinTimeout    = BDB_DEFAULT_TC_NODE_JOIN_TIMEOUT,             \
                                       .bdbTrustCenterRequireKeyExchange = BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE}

  
#else
#if (ZG_BUILD_COORDINATOR_TYPE)
#define BDB_ATTRIBUTES_DEFAULT_CONFIG {.bdbJoinUsesInstallCodeKey        = BDB_DEFAULT_JOIN_USES_INSTALL_CODE_KEY,       \
                                       .bdbTrustCenterNodeJoinTimeout    = BDB_DEFAULT_TC_NODE_JOIN_TIMEOUT,             \
                                       .bdbTrustCenterRequireKeyExchange = BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE,          \
                                       .bdbCommissioningGroupID          = BDB_DEFAULT_COMMISSIONING_GROUP_ID,           \
	                               .bdbCommissioningMode             = BDB_DEFAULT_COMMISSIONING_MODE,               \
	                               .bdbCommissioningStatus           = BDB_DEFAULT_COMMISSIONING_STATUS,             \
	                               .bdbNodeCommissioningCapability   = BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY,    \
	                               .bdbNodeIsOnANetwork              = BDB_DEFAULT_NODE_IS_ON_A_NETWORK,             \
	                               .bdbPrimaryChannelSet             = BDB_DEFAULT_PRIMARY_CHANNEL_SET,              \
	                               .bdbScanDuration                  = BDB_DEFAULT_SCAN_DURATION,                    \
	                               .bdbSecondaryChannelSet           = BDB_DEFAULT_SECONDARY_CHANNEL_SET}
#else

#define BDB_ATTRIBUTES_DEFAULT_CONFIG {.bdbNodeJoinLinkKeyType           = BDB_DEFAULT_NODE_JOIN_LINK_KEY_TYPE,          \
                                       .bdbTCLinkKeyExchangeAttempts     = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS,     \
                                       .bdbTCLinkKeyExchangeAttemptsMax  = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX, \
	                               .bdbTCLinkKeyExchangeMethod       = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_METHOD,      \
                                       .bdbCommissioningGroupID          = BDB_DEFAULT_COMMISSIONING_GROUP_ID,           \
	                               .bdbCommissioningMode             = BDB_DEFAULT_COMMISSIONING_MODE,               \
	                               .bdbCommissioningStatus           = BDB_DEFAULT_COMMISSIONING_STATUS,             \
	                               .bdbNodeCommissioningCapability   = BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY,    \
	                               .bdbNodeIsOnANetwork              = BDB_DEFAULT_NODE_IS_ON_A_NETWORK,             \
	                               .bdbPrimaryChannelSet             = BDB_DEFAULT_PRIMARY_CHANNEL_SET,              \
	                               .bdbScanDuration                  = BDB_DEFAULT_SCAN_DURATION,                    \
	                               .bdbSecondaryChannelSet           = BDB_DEFAULT_SECONDARY_CHANNEL_SET}
#endif
#endif

//Commissioning Modes
#define BDB_COMMISSIONING_MODE_IDDLE                0      // No pending commissioning procedures
#define BDB_COMMISSIONING_MODE_INITIATOR_TL         (1<<0)  
#define BDB_COMMISSIONING_MODE_NWK_STEERING         (1<<1)
#define BDB_COMMISSIONING_MODE_NWK_FORMATION        (1<<2)
#define BDB_COMMISSIONING_MODE_FINDING_BINDING      (1<<3)
#define BDB_COMMISSIONING_MODE_INITIALIZATION       (1<<4)
#define BDB_COMMISSIONING_MODE_PARENT_LOST          (1<<5)
                                         
#define BDB_COMMISSIONING_MODES                     (BDB_COMMISSIONING_MODE_FINDING_BINDING | BDB_COMMISSIONING_MODE_NWK_FORMATION |\
                                                     BDB_COMMISSIONING_MODE_NWK_STEERING    | BDB_COMMISSIONING_MODE_INITIATOR_TL |\
                                                     BDB_COMMISSIONING_MODE_INITIALIZATION  | BDB_COMMISSIONING_MODE_PARENT_LOST)
                                              
#define BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP 0x00

// bdbNodeJoinLinkKeyType
#define BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY           0x00
#define BDB_DISTRIBUTED_SECURITY_GLOBAL_LINK_KEY           0x01
#define BDB_INSTALL_CODE_DERIVED_PRECONFIGURED_LINK_KEY    0x02
#define BDB_TOUCHLINK_PRECONFIGURED_LINK_KEY               0x03  

#define BDB_FINDING_AND_BINDING_INITIATOR                  0x01
#define BDB_FINDING_AND_BINDING_TARGET                     0x02

#define BDB_ZIGBEE_RESERVED_ENDPOINTS_START                241
#define BDB_ZIGBEE_RESERVED_ENDPOINTS_END                  254



// New respondents require to get simple descritor request from remote device.
// If the respondent has a matching cluster a bind request is created, for which 
// if IEEE addr is missing then the entry is process to get its IEEE Addr by 
// sending an IEEE Addr Req
// Attempt counter also is used to designed which type of request will be send 
// by the usage of the mask FINDING_AND_BINDING_MISSING_IEEE_ADDR and the 
// assupmtion that the retries will not excede 36 attempts
   
#define FINDING_AND_BINDING_NEW_RESPONDENT      0x00
#define FINDING_AND_BINDING_MISSING_IEEE_ADDR   0x80
#define FINDING_AND_BINDING_PARENT_LOST         0x40
#define FINDING_AND_BINDING_RESPONDENT_COMPLETE 0xFF

 /*********************************************************************
 * CONSTANTS
 */

//Poll rate for Trust Center Link Key exchange process
#define TCLK_POLL_RATE                                      1000

// Zigbee Home Automation Profile Identification
#define Z3_PROFILE_ID                                       0x0104


// Define if Touchlink Target device will use fixed or random 
// channel from bdbcTLPrimaryChannelSet during commissioning
// when is Factory New (development only).
#define TOUCHLINK_FIXED_CHANNEL_ENABLE                          FALSE
#define TOUCHLINK_FIXED_CHANNEL                                 TOUCHLINK_FIRST_CHANNEL

// set TOUCHLINK_CH_OFFSET to Ch_Plus_1, Ch_Plus_2 or Ch_Plus_3 to shift
// the primary channel set (development only), allowing testing of multiple 
// touchlink devices without interference ONLY for testing propouses. if set 
// to No_Ch_offset (default) then no shift is applied.
#define TOUCHLINK_CH_OFFSET                                No_Ch_offset

//BDB Attribute initialization constants
#define BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS           0
#define BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX       0x03
#define BDB_DEFAULT_NODE_JOIN_LINK_KEY_TYPE                BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY
#define BDB_DEFAULT_NODE_IS_ON_A_NETWORK                   FALSE
#define BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY          (BDB_NETWORK_STEERING_CAPABILITY | BDB_NETWORK_FORMATION_CAPABILITY | BDB_FINDING_BINDING_CAPABILITY | BDB_TOUCHLINK_CAPABILITY)
#define BDB_DEFAULT_COMMISSIONING_STATUS                   BDB_COMMISSIONING_SUCCESS
#define BDB_DEFAULT_COMMISSIONING_MODE                     0x00 

#define BDBC_TC_LINK_KEY_EXANGE_TIMEOUT                    5000      // 5 seconds

//Constants for CRC calculations
#define CRC_ORDER    16u
#define CRC_POLYNOM  0x1021u
#define CRC_INIT     0xffffu
#define CRC_XOR      0xffffu
#define CRC_HIGHBIT  0x8000u //for CRC_ORDER =16

// TOUCHLINK Profile Constants
#define BDBCTL_INTER_PAN_TRANS_ID_LIFETIME               8000 // 8s
#define BDBCTL_MIN_STARTUP_DELAY_TIME                    2000 // 2s
#define BDBCTL_PRIMARY_CHANNEL_LIST                      ( 0x02108800 << TOUCHLINK_CH_OFFSET )
#define BDBCTL_RX_WINDOW_DURATION                        5000 // 5s
#define BDBCTL_SCAN_TIME_BASE_DURATION                   250  // 0.25s
#define BDBCTL_SECONDARY_CHANNEL_LIST                    ( 0x07fff800 ^ BDBCTL_PRIMARY_CHANNEL_LIST ) // TOUCHLINK Secondary Channels

// TOUCHLINK Channels (standard)
#define TOUCHLINK_FIRST_CHANNEL                         ( 11 + TOUCHLINK_CH_OFFSET )
#define TOUCHLINK_SECOND_CHANNEL                        ( 15 + TOUCHLINK_CH_OFFSET )
#define TOUCHLINK_THIRD_CHANNEL                         ( 20 + TOUCHLINK_CH_OFFSET )
#define TOUCHLINK_FOURTH_CHANNEL                        ( 25 + TOUCHLINK_CH_OFFSET )

#if ( TOUCHLINK_CH_OFFSET == No_Ch_offset )
#define TOUCHLINK_SECONDARY_CHANNELS_SET                {12, 13, 14, 16, 17, 18, 19, 21, 22, 23, 24, 26}
#elif ( TOUCHLINK_CH_OFFSET == Ch_Plus_1 )
#define TOUCHLINK_SECONDARY_CHANNELS_SET                {11, 13, 14, 15, 17, 18, 19, 20, 22, 23, 24, 25}
#elif ( TOUCHLINK_CH_OFFSET == Ch_Plus_2 )
#define TOUCHLINK_SECONDARY_CHANNELS_SET                {11, 12, 14, 15, 16, 18, 20, 21, 23, 24, 25, 26}
#elif ( TOUCHLINK_CH_OFFSET == Ch_Plus_3 )
#define TOUCHLINK_SECONDARY_CHANNELS_SET                {11, 12, 13, 15, 16, 17, 19, 20, 21, 22, 25, 26}
#endif

 /*********************************************************************
 * TYPEDEFS
 */

enum
{
BDB_COMMISSIONING_STATE_START_RESUME,            //Start/Resume the commissioning process according to commissionig modes
BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE,    //Perform the TC Link key exchange
BDB_COMMISSIONING_STATE_TL,                      //Perform Touchlink procedure as initiator
BDB_COMMISSIONING_STATE_JOINING,                 //Performs nwk discovery, joining attempt and nwk key reception
BDB_COMMISSIONING_STATE_STEERING_ON_NWK,         //Send mgmt permit joining
BDB_COMMISSIONING_STATE_FORMATION,               //Perform formtation procedure
BDB_COMMISSIONING_STATE_FINDING_BINDING,         //Perform Finding and binding procedure
BDB_INITIALIZATION,                              //Initialization process, for ZC/ZR means silent rejoin, for ZED nwk rejoin
BDB_PARENT_LOST,                                 //Parent lost, ask app to nwk Rejoin or giveup and reset

//Non-State related messages
BDB_TC_LINK_KEY_EXCHANGE_PROCESS,                //TC Notifications for TC link key exchange process with joining devices
BDB_NOTIFY_USER,                                 //Message to notify user about processing in BDB
BDB_ZDO_CB_MSG = 0xD3                                //To process ZDO CB Msg
};
    
 
typedef struct
{
uint32 bdbSecondaryChannelSet;
uint32 bdbPrimaryChannelSet;
uint16 bdbCommissioningGroupID;
uint8  bdbCommissioningStatus; 
uint8  bdbCommissioningMode;
uint8  bdbNodeCommissioningCapability;
uint8  bdbScanDuration;
bool   bdbNodeIsOnANetwork;
#if (ZG_BUILD_COORDINATOR_TYPE)
bool   bdbJoinUsesInstallCodeKey;
uint8  bdbTrustCenterNodeJoinTimeout;
bool   bdbTrustCenterRequireKeyExchange;
#endif
#if (ZG_BUILD_JOINING_TYPE)
uint8  bdbNodeJoinLinkKeyType;
uint8  bdbTCLinkKeyExchangeAttempts;
uint8  bdbTCLinkKeyExchangeAttemptsMax;
uint8  bdbTCLinkKeyExchangeMethod;
#endif
}bdbAttributes_t;


typedef struct respondentData
{
  afAddrType_t               data;
  uint8                      attempts;
  SimpleDescriptionFormat_t* SimpleDescriptor;
  struct respondentData*     pNext;
}bdbFindingBindingRespondent_t;



typedef struct bdb_joiningDeviceList_node
{
uint16 parentAddr;
uint8  bdbJoiningNodeEui64[Z_EXTADDR_LEN];
uint8  NodeJoinTimeout;
struct bdb_joiningDeviceList_node*  nextDev;
}bdb_joiningDeviceList_t;

//BDB Events
#define BDB_NWK_STEERING_NOT_IN_NWK_SUCCESS       0x0001
#define BDB_TC_LINK_KEY_EXCHANGE_FAIL             0x0002
#define BDB_CHANGE_COMMISSIONING_STATE            0x0004
#define BDB_REPORT_TIMEOUT                        0x0080
#define BDB_FINDING_AND_BINDING_PERIOD_TIMEOUT    0x0040
#define BDB_TC_JOIN_TIMEOUT                       0x0800
#define BDB_PROCESS_TIMEOUT                       0x1000
#define BDB_IDENTIFY_TIMEOUT                      0x2000
#define BDB_RESPONDENT_PROCESS_TIMEOUT            0x4000

//Msg event status
#define BDB_MSG_EVENT_SUCCESS             0
#define BDB_MSG_EVENT_FAIL                1




enum
{
BDB_JOIN_EVENT_NWK_DISCOVERY,
BDB_JOIN_EVENT_ASSOCIATION,
BDB_JOIN_EVENT_NO_NWK_KEY,
BDB_JOIN_EVENT_OTHER,
};

enum
{
BDB_TC_LINK_KEY_EXCHANGE_NOT_ACTIVE,
BDB_REQ_TC_STACK_VERSION,
BDB_REQ_TC_LINK_KEY,
BDB_REQ_VERIFY_TC_LINK_KEY,
BDB_TC_EXCHANGE_NEXT_STATE=1,
};

typedef struct
{
  osal_event_hdr_t hdr;
  uint8  buf[1];
}bdbInMsg_t;


enum 
{
BDB_JOIN_STATE_NWK_DISC,
BDB_JOIN_STATE_ASSOC,
BDB_JOIN_STATE_WAITING_NWK_KEY,
};

typedef struct
{
uint8    bdbCommissioningState;
uint8    bdbTCExchangeState;
uint8    bdbJoinState;
uint8    bdb_ParentLostSavedState;      //Commissioning state to be restore after parent is found
}bdbCommissioningProcedureState_t;

typedef struct
{
  zAddrType_t dstAddr;
  uint8       ep;
  uint16      clusterId;
}bdbBindNotificationData_t;

typedef void (*tlGCB_TargetEnable_t)( uint8 enable );

/*********************************************************************
 * GLOBAL VARIABLES
 */
 
extern byte bdb_TaskID;

extern bdbAttributes_t bdbAttributes;

extern uint8 touchLinkInitiator_TaskID;

extern epList_t *bdb_HeadEpDescriptorList;

extern epList_t *bdb_CurrEpDescriptorList;

extern bdbCommissioningProcedureState_t bdbCommissioningProcedureState;

extern uint8 bdb_ZclTransactionSequenceNumber;

extern uint8 bdb_FB_InitiatorCurrentCyclesNumber;

extern bdbFindingBindingRespondent_t *pRespondentHead;

extern bdbFindingBindingRespondent_t *pRespondentCurr;

extern bdbFindingBindingRespondent_t *pRespondentNext;

#if ( TOUCHLINK_CH_OFFSET > Ch_Plus_3 )
#error "ERROR! TOUCHLINK_CH_OFFSET can't be bigger than Ch_Plus_3"
#endif

#if defined (BDB_TL_INITIATOR) && !defined (INTER_PAN)
#error "ERROR! Add INTER_PAN flag to compilation symbols"
#endif

#if defined (BDB_TL_TARGET) && !defined (INTER_PAN)
#error "ERROR! Add INTER_PAN flag to compilation symbols"
#endif

#if defined (BDB_TL_TARGET) && defined (BDB_TL_INITIATOR)
#error "ERROR! a device cannot be Touchlink Target and Initiator at the same time"
#endif

/*********************************************************************
 * FUNCTION MACROS
 */
 
#define bdb_NotifyApp(a)   bdb_SendMsg(bdb_TaskID, BDB_NOTIFY_USER, BDB_MSG_EVENT_SUCCESS,sizeof(bdbCommissioningModeMsg_t),(a))


/*********************************************************************
 * FUNCTIONS
 */
extern void bdb_reportCommissioningState(uint8 bdbCommissioningState, bool didSuccess);
extern void bdb_setFN(void);
extern void bdb_touchlinkSendFNReset( void );
extern void bdb_setNodeIsOnANetwork(bool isOnANetwork);
extern void bdb_nwkFormationAttempt(bool didSucess);
extern void bdb_nwkDiscoveryAttempt(bool didSuccess);
extern void bdb_nwkAssocAttemt(bool didSuccess);
extern ZStatus_t bdb_rejoinNwk(void);
extern void touchLinkInitiator_ResetToFNProcedure( void );
extern void bdb_tcLinkKeyExchangeAttempt(bool didSuccess, uint8 bdbTCExchangeState);
extern void bdb_SendMsg(uint8 taskID, uint8 toCommissioningState,uint8 status, uint8 len, uint8 *buf);
extern void bdb_setNodeJoinLinkKeyType(uint8 KeyType);  
extern bdbFindingBindingRespondent_t* bdb_AddRespondentNode( bdbFindingBindingRespondent_t **pHead, zclIdentifyQueryRsp_t *pCmd );
extern void bdb_CreateRespondentList( bdbFindingBindingRespondent_t **pHead );


#if (ZG_BUILD_COORDINATOR_TYPE)
extern ZStatus_t bdb_TCAddJoiningDevice(uint16 parentAddr, uint8* JoiningExtAddr);
extern void bdb_TCjoiningDeviceComplete(uint8* JoiningExtAddr);
#endif

ZStatus_t bdb_addInstallCode(uint8* pInstallCode, uint8* pExt);

/*
 * @brief   Register the Simple descriptor. This function also registers 
 *          the profile's cluster conversion table.
 */
void bdb_RegisterSimpleDescriptor( SimpleDescriptionFormat_t *simpleDesc );

/*
 * @brief   Sends Identify query from the specified endpoint
 */
extern ZStatus_t bdb_SendIdentifyQuery( uint8 endpoint );

/*
 * @brief   This function free reserved memory for respondent list
 */
extern void bdb_zclRespondentListClean( bdbFindingBindingRespondent_t **pHead );



/*
 * @brief   Process the respondent list by sending Simple Descriptor request to 
 *          devices respondent in the list. Also send IEEE Addr Req to those 
 *          device for which a bind is created buy IEEE addr is missing.
 */
extern void bdb_ProcessRespondentList( void );

/*
 * @brief   Gives the Ep Type accourding to application clusters in
 *          simple descriptor
 */
extern uint8 bdb_zclFindingBindingEpType( endPointDesc_t *epDesc );

/*
 * @brief   Send out a Network Join Router or End Device Request command.
 *          using the selected Target.
 */
extern ZStatus_t bdb_Initiator_SendNwkJoinReq( void );

/*
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 */
extern void bdb_ZclIdentifyCmdInd( uint16 identifyTime, uint8 endpoint );

/*
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this 
 *          application.
 */
extern void bdb_ZclIdentifyQueryCmdInd( zclIdentifyQueryRsp_t *pCmd );

/*
 * @brief   Restore nwk parameters to invalid if the device is not on a network
 */
void bdb_ClearNetworkParams(void);

/*
 * @brief       Notify bdb that connection with parent is lost
 */
void bdb_parentLost(void);

/*
 * @brief       Restore the state of child device after parent lost
 */
void bdb_NetworkRestoredResumeState(void);

/*
 * @brief   Set the endpoint list to the active endpoint selected by the application for F&B process
 */
endPointDesc_t* bdb_setEpDescListToActiveEndpoint(void);

/*
 * @brief   Clean the F&B initiator process and reports the status to bdb state machine
 */
void bdb_exitFindingBindingWStatus( uint8 status);

/*
 * @brief   This function free reserved memory for respondent list
 */
void bdb_zclRespondentListClean( bdbFindingBindingRespondent_t **pHead );

 /*
  * @brief       Set channel and save it in Nv for joining/formation operations
  */
extern void bdb_setChannel(uint32 channel);

#ifdef __cplusplus
}
#endif


#endif /* BDB_H */
 
 
 
 
 
 
 
 
 
 
