/**************************************************************************************************
  Filename:       bdb_interface.h
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Base Device Behavior interface such as 
                  MACRO configuration and API.


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

#ifndef BDB_INTERFACE_H
#define BDB_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "bdb.h"
 
 /*********************************************************************
 * CONSTANTS
 */
 
#define BDB_INSTALL_CODE_USE_IC_CRC  1    // IC + CRC are expected to be introduced (18 bytes), CRC is validated and TC link key is calcuated from it
#define BDB_INSTALL_CODE_USE_KEY     2    // TC link key is expected (16 bytes), the buffer provided will be used as the key
   

//bdbTCLinkKeyExchangeMethod
#define BDB_TC_LINK_KEY_EXCHANGE_APS_KEY     0x00
#define BDB_TC_LINK_KEY_EXCHANGE_CBKE        0x01    //Not supported yet by spec   
   
//Status notifications on APS TC Link exchange process for a joining device
#define BDB_TC_LK_EXCH_PROCESS_JOINING         0
#define BDB_TC_LK_EXCH_PROCESS_EXCH_SUCCESS    1
#define BDB_TC_LK_EXCH_PROCESS_EXCH_FAIL       2   


#ifdef BDB_REPORTING
#define BDBREPORTING_REPORTOFF       0xFFFF //Use for MaxInterval, No periodic nor value changed reporting
#define BDBREPORTING_NOPERIODIC      0x0000 //Use for MaxInterval, No periodic but value changed is enabled
#define BDBREPORTING_NOLIMIT         0x0000 //Use for MinInterval, there can be consecutive value change reporting without enforcing wait time
#endif
 /*********************************************************************
 * CONFIGURATION MACROS
 */
 
 /**********************
 * General configuration
 */

//Constants used by all nodes
#define BDBC_MIN_COMMISSIONING_TIME                    180    // 180 seconds
#define BDBC_REC_SAME_NETWORK_RETRY_ATTEMPS            3      //Maximum by BDB spec is 10

//Define if ZR devices will perform classical formation procedure or not (the network formed would be Distributed Network)
#define BDB_ROUTER_FORM_DISTRIBUTED_NWK_ENABLED     1
  
//Define how IC are introduced see
#define BDB_INSTALL_CODE_USE  BDB_INSTALL_CODE_USE_IC_CRC

//Time after which the device will reset itself after failining on 
//TCLK exchange procedure (more than BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX 
//attempts for the same request or Parent Lost during TCLK exchange process).
//This reset will perform a FN reset
#define BDB_TC_LINK_KEY_EXCHANGE_FAIL_LEAVE_TIMEOUT  5000

#define BDB_TL_IDENTIFY_TIME                         0xFFFF

//Default values for BDB attributes 
#define BDB_DEFAULT_COMMISSIONING_GROUP_ID                 0xFFFF   
#define BDB_DEFAULT_JOIN_USES_INSTALL_CODE_KEY             FALSE
#define BDB_DEFAULT_PRIMARY_CHANNEL_SET                    DEFAULT_CHANLIST //BDB specification default is: 0x02108800
#define BDB_DEFAULT_SCAN_DURATION                          0x04
#define BDB_DEFAULT_SECONDARY_CHANNEL_SET                  (DEFAULT_CHANLIST ^ 0x07FFF800)      //BDB specification default is: (0x07FFF800 ^ 0x02108800)
#define BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_METHOD            BDB_TC_LINK_KEY_EXCHANGE_APS_KEY
#define BDB_DEFAULT_TC_NODE_JOIN_TIMEOUT                   0x0F
  
#ifdef TP2_LEGACY_ZC
#define BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE                FALSE
#else
#define BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE                TRUE 
#endif
  
#define BDB_DEFAULT_DEVICE_UNAUTH_TIMEOUT                  3000  //Time joining device waits for nwk before attempt association again. In BDB is known as apsSecurityTimeOutPeriod

#define BDB_ALLOW_TL_STEALING                              TRUE 


 /******************
 * F&B configuration
 */
//Your JOB: Set this value according to your application
//Initiator devices in F&B procedure may enable this macro to perfom the searching as a periodic task
//for up to 3 minutes with a period of FINDING_AND_BINDING_PERIODIC_TIME between discovery attempts
#define FINDING_AND_BINDING_PERIODIC_ENABLE          TRUE    // Boolean
#define FINDING_AND_BINDING_PERIODIC_TIME            15       // in seconds

// Number of attemtps that will be done to retrieve the simple desc from a target 
// device or the IEEE Address if this is unknown. The number of attempts cannot 
// be greater than 36
#define FINDING_AND_BINDING_MAX_ATTEMPTS             4        


//Your JOB: Set this value according to your application
//This defines the time that initiator device will wait for Indentify query response 
//and simple descriptor from target devices. Consider Identify Query is broadcast while Simple Desc is unicast
//Consider that ZED will have to wait longer since their responses will need to 
//be pooled and will be dependent of the number of targets that is expected to be found
#if ZG_BUILD_RTR_TYPE
#define IDENTIFY_QUERY_RSP_TIMEOUT                         5000
#define SIMPLEDESC_RESPONSE_TIMEOUT                        (ROUTE_DISCOVERY_TIME * 1000)  // Timeout for ZR
#else   
#define IDENTIFY_QUERY_RSP_TIMEOUT                         10000
#define SIMPLEDESC_RESPONSE_TIMEOUT                        (3 * zgPollRate)      // Timeout for ZED  *Don't use time smaller than zgPollRate
#endif   


 /******************
 * Touchlink configuration
 */

/** Received signal strength threshold **/
// Manufacturer specific threshold (greater than -128),
// do not respond to Touch-link scan request if reached
#ifndef TOUCHLINK_WORST_RSSI
#define TOUCHLINK_WORST_RSSI                                    -40 // dBm
#endif
 
// Pre-programmed RSSI correction offset (0x00-0x20)
#ifndef TOUCHLINK_RSSI_CORRECTION
#define TOUCHLINK_RSSI_CORRECTION                                0x00
#endif

/** Pre-Installed Keys **/
#define TOUCHLINK_CERTIFICATION_ENC_KEY    { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,\
                                             0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf }

#define TOUCHLINK_CERTIFICATION_LINK_KEY   { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,\
                                             0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf }

#define TOUCHLINK_DEFAULT_AES_KEY          { 0x50, 0x68, 0x4c, 0x69, 0xea, 0x9c, 0xd1, 0x38,\
                                             0x43, 0x4c, 0x53, 0x4e, 0x8f, 0x8d, 0xba, 0xb4 }

#define TOUCHLINK_KEY_INDEX_DEV         0
#define TOUCHLINK_KEY_INDEX_MASTER      4
#define TOUCHLINK_KEY_INDEX_CERT        15

// For certification only:
#define TOUCHLINK_ENC_KEY  TOUCHLINK_CERTIFICATION_ENC_KEY
#define TOUCHLINK_LINK_KEY  TOUCHLINK_CERTIFICATION_LINK_KEY
#define TOUCHLINK_KEY_INDEX TOUCHLINK_KEY_INDEX_CERT

// For internal EP's simple descriptor
#define TOUCHLINK_INTERNAL_ENDPOINT                             13
#define TOUCHLINK_INTERNAL_DEVICE_ID                            0xE15E
#define TOUCHLINK_INTERNAL_FLAGS                                0
#define TOUCHLINK_DEFAULT_IDENTIFY_TIME                         3

 /******************
 * Reporting attributes configuration
 */
#ifdef BDB_REPORTING
//Your JOB: Set this value according to your application
//Maximum size in bytes used by reportable attributes registered in any 
//endpoint for the application (for analog attributes)
#ifndef BDBREPORTING_MAX_ANALOG_ATTR_SIZE
#define BDBREPORTING_MAX_ANALOG_ATTR_SIZE 4
#endif

//Your JOB: Set this value according to your application
//Max num of cluster with reportable attributes in any endpoint 
//(eg. 2 endpoints with same cluster with reportable attributes counts as 2, 
//regardless of the number of reportable attributes in the cluster)
#ifndef BDB_MAX_CLUSTERENDPOINTS_REPORTING
#define BDB_MAX_CLUSTERENDPOINTS_REPORTING 5 
#endif
  
//Default values contants used in the bdb reporting code
#define BDBREPORTING_DEFAULTMAXINTERVAL BDBREPORTING_REPORTOFF
#define BDBREPORTING_DEFAULTMININTERVAL 0x000A    

//Define the DISABLE_DEFAULT_RSP flag for reporting attributes
#define BDB_REPORTING_DISABLE_DEFAULT_RSP  FALSE
#endif 

/*********************************************************************
 * ENUM
 */
 

enum
{
  BDB_COMMISSIONING_INITIALIZATION,
  BDB_COMMISSIONING_NWK_STEERING,
  BDB_COMMISSIONING_FORMATION,
  BDB_COMMISSIONING_FINDING_BINDING,
  BDB_COMMISSIONING_TOUCHLINK,
  BDB_COMMISSIONING_PARENT_LOST,
};


enum
{
  BDB_COMMISSIONING_SUCCESS,       
  BDB_COMMISSIONING_IN_PROGRESS,   
  BDB_COMMISSIONING_NO_NETWORK,          
  BDB_COMMISSIONING_TL_TARGET_FAILURE,
  BDB_COMMISSIONING_TL_NOT_AA_CAPABLE,
  BDB_COMMISSIONING_TL_NO_SCAN_RESPONSE,
  BDB_COMMISSIONING_TL_NOT_PERMITTED,
  BDB_COMMISSIONING_TCLK_EX_FAILURE,              //Many attempts made and failed, or parent lost during the TCLK exchange
  BDB_COMMISSIONING_FORMATION_FAILURE,
  BDB_COMMISSIONING_FB_TARGET_IN_PROGRESS,        //No callback from bdb when the identify time runs out, the application can figure out from Identify time callback
  BDB_COMMISSIONING_FB_INITITATOR_IN_PROGRESS,
  BDB_COMMISSIONING_FB_NO_IDENTIFY_QUERY_RESPONSE,
  BDB_COMMISSIONING_FB_BINDING_TABLE_FULL,
  BDB_COMMISSIONING_NETWORK_RESTORED,               
  BDB_COMMISSIONING_FAILURE,              //Generic failure status for no commissioning mode supported, not allowed, invalid state to perform commissioning
};



typedef enum
{
  /** Instruct joining node to use Default Global Trust Center link key. 
  No key buffer requiered */
  zstack_UseDefaultGlobalTrustCenterLinkKey,
  /** Instruct the joining node to use the provided install code (16 bytes 
  + 2 CRC bytes) to derive APS Link key to be used during joining */
  zstack_UseInstallCode,
  /** Instruct the joining node to use the provided install code (16 bytes 
  + 2 CRC bytes) to derive APS Link key to be used during joining. 
  If it fails to decrypt Transport Key, it will automatically try Default 
  Global Trust Center Link Key */
  zstack_UseInstallCodeWithFallback,
  /** Instruct the joining node to use the provided APS Link key to be used 
  during joining (key size is 16 bytes) */
  zstack_UseAPSKey,
  /** Instruct the joining node to use the provided APS Link key to be used 
  during joining (key size is 16 bytes). If it fails to decrypt Transport 
  Key, it will automatically try Default Global Trust Center Link Key  */
  zstack_UseAPSKeyWithFallback,
}zstack_CentralizedLinkKeyModes_t;

 /*********************************************************************
 * TYPEDEFS
 */
 
typedef struct
{
  uint8  bdbCommissioningStatus;
  uint8  bdbCommissioningMode;
  uint8  bdbRemainingCommissioningModes;
}bdbCommissioningModeMsg_t;


typedef struct
{
  uint8 status;                  //status: BDB_TC_LK_EXCH_PROCESS_JOINING
  uint8 extAddr[Z_EXTADDR_LEN];
}bdb_TCLinkKeyExchProcess_t;



typedef void (*bdbGCB_IdentifyTimeChange_t)( uint8 endpoint );
typedef void (*bdbGCB_BindNotification_t)( bdbBindNotificationData_t *bindData );
typedef void (*bdbGCB_CommissioningStatus_t)(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);
typedef void (*bdbGCB_CBKETCLinkKeyExchange_t)( void );
typedef void (*bdbGCB_TCLinkKeyExchangeProcess_t) (bdb_TCLinkKeyExchProcess_t* bdb_TCLinkKeyExchProcess);
typedef void (*bdbGCB_FilterNwkDesc_t) (networkDesc_t *pBDBListNwk, uint8 count);
 


/*********************************************************************
 * GLOBAL VARIABLES
 */
#if ( defined ( BDB_TL_TARGET ) && (BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE) )
extern tlGCB_TargetEnable_t pfnTargetEnableChangeCB;
#endif
 
extern bool touchLinkTargetEnabled;

 /*********************************************************************
 * FUNCTION MACROS
 */

/*
 * Initialize the device with persistant data. Restore nwk (Silent rejoin for ZC and ZR, Rejoin for ZED), and resume reporting attributes.
 */
#define bdb_initialize() bdb_StartCommissioning(0)


 /*********************************************************************
 * FUNCTIONS
 */
   
 /*****************************
 * GENERAL API
 */

/*
 * @brief   Event Process for the task
 */
UINT16 bdb_event_loop( byte task_id, UINT16 events );

/*
 * @brief   Initialization for the task
 */
void bdb_Init( byte task_id );
 
/*
 * @brief   Start the commissioning process setting the commissioning mode given.
 */
void bdb_StartCommissioning(uint8 mode);

/*
 * @brief   Set the endpoint which will perform the finding and binding (either Target or Initiator)
 */
ZStatus_t bdb_SetIdentifyActiveEndpoint(uint8 activeEndpoint);

/*
 * @brief   Stops Finding&Binding for initiator devices. The result of this process 
 * is reported in bdb notifications callback.
 */  
void bdb_StopInitiatorFindingBinding(void);

/*  
 * @brief   Get the next ZCL Frame Counter for packet sequence number
 */
uint8 bdb_getZCLFrameCounter(void);

/*
 * @brief   Application interface to perform BDB Reset to FN.
 */
void bdb_resetLocalAction(void);

/*
 * @brief   Set the primary or seconday channel for discovery or formation procedure
 */
void bdb_setChannelAttribute(bool isPrimaryChannel, uint32 channel);

/*
 * @brief   Register an Application's Identify Time change callback function
 *          to let know the application when identify is active or not.
 */
void bdb_RegisterIdentifyTimeChangeCB( bdbGCB_IdentifyTimeChange_t pfnIdentifyTimeChange );

/*
 * @brief   Register an Application's notification callback function to let 
 *          know the application when a new bind is added to the binding table.
 */
void bdb_RegisterBindNotificationCB( bdbGCB_BindNotification_t pfnBindNotification );

/*
 * @brief   Get the F&B initiator status for periodic requests.
 */
void bdb_GetFBInitiatorStatus(uint8 *RemainingTime, uint8* AttemptsLeft);

/*
 * @brief   Register a callback in which the status of the procedures done in
 *          BDB commissioning process will be reported
 */
void bdb_RegisterCommissioningStatusCB( bdbGCB_CommissioningStatus_t bdbGCB_CommissioningStatus );

/*
 * @brief   Returns the state of bdbNodeIsOnANetwork attribute
 */
bool bdb_isDeviceNonFactoryNew(void);

/*
 * @brief   Sets the commissioning groupd ID
 */
void bdb_setCommissioningGroupID(uint16 groupID);

 /*
  * @brief   Creates a CRC for the install code passed.
  */
 uint16 bdb_GenerateInstallCodeCRC(uint8 *installCode);
 
/*
 * @brief   Returns the state of bdbTrustCenterRequireKeyExchange attribute
 */
bool bdb_doTrustCenterRequireKeyExchange(void);

 /*****************************
 * REPORTING ATTRIBUTES API
 */

#ifdef BDB_REPORTING

/*
 * @brief   Adds default configuration values for a Reportable Attribute Record
 */
ZStatus_t bdb_RepAddAttrCfgRecordDefaultToList(uint8 endpoint, uint16 cluster, uint16 attrID, uint16 minReportInt, uint16 maxReportInt, uint8* reportableChange);

/*
 * @brief   Notify BDB reporting attribute module about the change of an 
 *          attribute value to validate the triggering of a reporting attribute message.
 */
ZStatus_t bdb_RepChangedAttrValue(uint8 endpoint, uint16 cluster, uint16 attrID); //newvalue must a a buffer of size 8
#endif

 /*****************************
 * Trust Center API  (ZC)
 */

#if (ZG_BUILD_COORDINATOR_TYPE)

/*
 * @brief   Set BDB attribute bdbJoinUsesInstallCodeKey.
 */
void bdb_setJoinUsesInstallCodeKey(bool set);

/*
 * @brief   Set the bdb_setTCRequireKeyExchange attribute
 */
void bdb_setTCRequireKeyExchange(bool isKeyExchangeRequired);



/*
 * bdb_addInstallCode interface.
 */
ZStatus_t bdb_addInstallCode(uint8* pInstallCode, uint8* pExt);

/*
 * @brief   Register a callback to receive notifications on the joining devices 
 *          and its status on TC link key exchange. 
 */
void bdb_RegisterTCLinkKeyExchangeProcessCB( bdbGCB_TCLinkKeyExchangeProcess_t bdbGCB_TCLinkKeyExchangeProcess );

#endif



 /*****************************
 * Joining devices API  (ZR ZED)
 */

/*
 * @brief   Register an Application's Enable/Disable callback function. 
 *          Refer to touchLinkTarget_EnableCommissioning to enable/disable TL as target
 */
void bdb_RegisterTouchlinkTargetEnableCB( tlGCB_TargetEnable_t pfnTargetEnableChange );

 /*
 * @brief   Enable the reception of Commissioning commands.
 */
void touchLinkTarget_EnableCommissioning( uint32 timeoutTime );


 /*
 * @brief   Disable TouchLink commissioning on a target device.
 */
void touchLinkTarget_DisableCommissioning( void );

 /*
 * @brief   Get the remaining time for TouchLink on a target device.
 */
uint32 touchLinkTarget_GetTimer( void );

/*
 * @brief   Set the active centralized key to be used, Global or IC derived based on zstack_CentralizedLinkKeyModes_t
 */
ZStatus_t bdb_setActiveCentralizedLinkKey(uint8 zstack_CentralizedLinkKeyModes, uint8* pKey);


/*
 * @brief   Register a callback in which the TC link key exchange procedure will 
 *          be performed by application.  The result from this operation must be notified to using the 
 *          bdb_CBKETCLinkKeyExchangeAttempt interface.
 *          NOTE: NOT CERTIFIABLE AT THE MOMENT OF THIS RELEASE
 */
void bdb_RegisterCBKETCLinkKeyExchangeCB( bdbGCB_CBKETCLinkKeyExchange_t bdbGCB_CBKETCLinkKeyExchange );

/*
 * @brief   Tell BDB module the result of the TC link key exchange, to try
 *          the default process or to keep going with the joining process.
 */
void bdb_CBKETCLinkKeyExchangeAttempt(bool didSucces);


/*
 * @brief   Register a callback in which the application gets the list of network
 *          descriptors got from active scan.
 *          Use bdb_nwkDescFree to release the network descriptors that are not 
 *          of interest and leave those which are to be attempted.
 */
void bdb_RegisterForFilterNwkDescCB(bdbGCB_FilterNwkDesc_t bdbGCB_FilterNwkDesc);

/*
 * @brief   This function frees a network descriptor.
 */
ZStatus_t bdb_nwkDescFree(networkDesc_t* nodeDescToRemove);

/*
 * @brief   General function to allow stealing when performing TL as target
 */
void bdb_TouchlinkSetAllowStealing( bool allow );

/*
 * @brief   General function to get allow stealing value
 */
bool bdb_TouchlinkGetAllowStealing( void );

 /*****************************
 * ZED API
 */
#if (ZG_BUILD_ENDDEVICE_TYPE)

/*
 * @brief   Instruct the ZED to try to rejoin its previews network
 */
uint8 bdb_ZedAttemptRecoverNwk(void);
#endif

/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif //BDB_INTERFACE_H
