/**************************************************************************************************
  Filename:       zcl_power_profile.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Power Profile definitions.


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

#ifndef ZCL_POWER_PROFILE_H
#define ZCL_POWER_PROFILE_H

#ifdef ZCL_POWER_PROFILE

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

/*****************************************/
/***  Power Profile Cluster Attributes ***/
/*****************************************/

// Server Attributes
#define ATTRID_POWER_PROFILE_TOTAL_PROFILE_NUM                                  0x0000  // M, R, UINT8
#define ATTRID_POWER_PROFILE_MULTIPLE_SCHEDULING                                0x0001  // M, R, BOOLEAN
#define ATTRID_POWER_PROFILE_ENERGY_FORMATTING                                  0x0002  // M, R, 8-BIT BITMAP
#define ATTRID_POWER_PROFILE_ENERGY_REMOTE                                      0x0003  // M, R, BOOLEAN
#define ATTRID_POWER_PROFILE_SCHEDULE_MODE                                      0x0004  // M, R/W, 8-BIT BITMAP

// Server Attribute Defaults
#define ATTR_DEFAULT_POWER_PROFILE_TOTAL_PROFILE_NUM                            1
#define ATTR_DEFAULT_POWER_PROFILE_MULTIPLE_SCHEDULING                          0x00
#define ATTR_DEFAULT_POWER_PROFILE_ENERGY_FORMATTING                            0x01
#define ATTR_DEFAULT_POWER_PROFILE_ENERGY_REMOTE                                0x00
#define ATTR_DEFAULT_POWER_PROFILE_SCHEDULE_MODE                                0x00

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_POWER_PROFILE_POWER_PROFILE_REQ                                 0x00  // M, powerProfileID
#define COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_REQ                           0x01  // M, no payload
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_RSP                       0x02  // M, zclPowerProfileGetPowerProfilePriceRsp_t
#define COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE_RSP                    0x03  // M, zclPowerProfileGetOverallSchedulePriceRsp_t
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION               0x04  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP                        0x05  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_REQ            0x06  // M, powerProfileID
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_REQ                  0x07  // M, powerProfileID
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXT_RSP                   0x08  // M, zclPowerProfileGetPowerProfilePriceExtRsp_t

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_POWER_PROFILE_POWER_PROFILE_NOTIFICATION                        0x00  // M, zclPowerProfileNotification_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_RSP                                 0x01  // M, zclPowerProfileRsp_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_RSP                           0x02  // M, zclPowerProfileStateRsp_t
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE                           0x03  // O, powerProfileID
#define COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_NOTIFICATION                  0x04  // M, zclPowerProfileStateNotification_t
#define COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE                        0x05  // O, no payload
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_REQ                        0x06  // M, powerProfileID
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP                  0x07  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION         0x08  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_NOTIFICATION   0x09  // M, zclPowerProfileScheduleConstraintsNotification_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_RSP            0x0A  // M, zclPowerProfileScheduleConstraintsRsp_t
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXT                       0x0B  // O, zclPowerProfileGetPowerProfilePriceExt_t

// PowerProfileState enumeration field
#define POWER_PROFILE_IDLE                        0x00  // The PP is not defined in its parameters
#define POWER_PROFILE_PROGRAMMED                  0x01  // The PP is defined in its parameters but without a scheduled time reference
#define ENERGY_PHASE_RUNNING                      0x03  // An energy phase is running
#define ENERGY_PHASE_PAUSED                       0x04  // The current energy phase is paused
#define ENERGY_PHASE_WAITING_TO_START             0x05  // The Power Profile is in between two energy phases (see spec for details)
#define ENERGY_PHASE_WAITING_PAUSED               0x06  // The Power Profile is set to pause when being in the ENERGY_PHASE_WAITING_TO_START state
#define POWER_PROFILE_ENDED                       0x07  // The whole Power Profile is terminated

/*******************************************************************************
 * TYPEDEFS
 */

/*** ZCL Power Profile Cluster: Get Power Profile Price Response payload ***/
typedef struct
{
  uint8 powerProfileID;
  uint16 currency;
  uint32 price;
  uint8 priceTrailingDigit;
} zclPowerProfileGetPowerProfilePriceRsp_t;

// Duplicate structure for Get Power Profile Price Extended Response cmd
typedef zclPowerProfileGetPowerProfilePriceRsp_t zclPowerProfileGetPowerProfilePriceExtRsp_t;

/*** ZCL Power Profile Cluster: Power Profile Get Overall Schedule Price Response payload ***/
typedef struct
{
  uint16 currency;
  uint32 price;
  uint8 priceTrailingDigit;
} zclPowerProfileGetOverallSchedulePriceRsp_t;

/*** ZCL Power Profile Cluster: Energy Phases Schedule Notification and Response payloads ***/
typedef struct
{
  uint8 energyPhaseID;
  uint16 scheduledTime;
} scheduledPhasesRecord_t;

typedef struct
{
  uint8 powerProfileID;
  uint8 numOfScheduledPhases;
  scheduledPhasesRecord_t *pScheduledPhasesRecord;   // variable length array based off numOfScheduledPhases
} zclPowerProfileEnergyPhasesSchedule_t;

/*** Structures for Server Generated Commands ***/

/*** ZCL Power Profile Cluster: Power Profile Notification and Response payloads ***/
typedef struct
{
  uint8 energyPhaseID;
  uint8 macroPhaseID;
  uint16 expectedDuration;
  uint16 peakPower;
  uint16 energy;
  uint16 maxActivationDelay;
} transferredPhasesRecord_t;

typedef struct
{
  uint8 totalProfileNum;
  uint8 powerProfileID;
  uint8 numOfTransferredPhases;
  transferredPhasesRecord_t *pTransferredPhasesRecord;  // variable length array based off of numOfTransferredPhases
} zclPowerProfile_t;

// Duplicate structure for multiple commands
typedef zclPowerProfile_t zclPowerProfileNotification_t;
typedef zclPowerProfile_t zclPowerProfileRsp_t;

/*** ZCL Power Profile Cluster: Power Profile State Response and Notification payloads ***/
typedef struct
{
  uint8 powerProfileID;
  uint8 energyPhaseID;
  bool powerProfileRemoteControl;
  uint8 powerProfileState;          // e.g. POWER_PROFILE_IDLE
} powerProfileStateRecord_t;

typedef struct
{
  uint8 powerProfileCount;
  powerProfileStateRecord_t *pPowerProfileStateRecord;   // variable length array based off of powerProfileCount
} zclPowerProfileState_t;

// Duplicate structure for multiple commands
typedef zclPowerProfileState_t zclPowerProfileStateRsp_t;
typedef zclPowerProfileState_t zclPowerProfileStateNotification_t;

/*** ZCL Power Profile Cluster: Power Profile Schedule Constraints Notification and Response payloads ***/
typedef struct
{
  uint8 powerProfileID;
  uint16 startAfter;
  uint16 stopBefore;
} zclPowerProfileScheduleConstraints_t;

// Duplicate structure for multiple commands
typedef zclPowerProfileScheduleConstraints_t zclPowerProfileScheduleConstraintsNotification_t;
typedef zclPowerProfileScheduleConstraints_t zclPowerProfileScheduleConstraintsRsp_t;

/*** ZCL Power Profile Cluster: Get Power Profile Price Extended payload ***/
typedef struct
{
  uint8 options;
  uint8 powerProfileID;
  uint16 powerProfileStartTime;
} zclPowerProfileGetPowerProfilePriceExt_t;


typedef ZStatus_t (*zclPower_Profile_PowerProfileReq_t)( uint8 powerProfileID, afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_PowerProfileStateReq_t)( afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePriceRsp_t)( zclPowerProfileGetPowerProfilePriceRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetOverallSchedulePriceRsp_t)( zclPowerProfileGetOverallSchedulePriceRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleNotification_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleRsp_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileScheduleConstraintsReq_t)( uint8 powerProfileID, afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleStateReq_t)( uint8 powerProfileID, afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePriceExtRsp_t)( zclPowerProfileGetPowerProfilePriceExtRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileNotification_t)( zclPowerProfileNotification_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileRsp_t)( zclPowerProfileRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileStateRsp_t)( zclPowerProfileStateRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePrice_t)( uint8 powerProfileID, afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_PowerProfileStateNotification_t)( zclPowerProfileStateNotification_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetOverallSchedulePrice_t)( afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleReq_t)( uint8 powerProfileID, afAddrType_t *pSrcAddr, uint8 transSeqNum );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleStateRsp_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleStateNotification_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileScheduleConstraintsNotification_t)( zclPowerProfileScheduleConstraintsNotification_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileScheduleConstraintsRsp_t)( zclPowerProfileScheduleConstraintsRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePriceExt_t)( zclPowerProfileGetPowerProfilePriceExt_t *pCmd, afAddrType_t *pSrcAddr, uint8 transSeqNum );


// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclPower_Profile_PowerProfileReq_t                                pfnPowerProfile_PowerProfileReq;
  zclPower_Profile_PowerProfileStateReq_t                           pfnPowerProfile_PowerProfileStateReq;
  zclPower_Profile_GetPowerProfilePriceRsp_t                        pfnPowerProfile_GetPowerProfilePriceRsp;
  zclPower_Profile_GetOverallSchedulePriceRsp_t                     pfnPowerProfile_GetOverallSchedulePriceRsp;
  zclPower_Profile_EnergyPhasesScheduleNotification_t               pfnPowerProfile_EnergyPhasesScheduleNotification;
  zclPower_Profile_EnergyPhasesScheduleRsp_t                        pfnPowerProfile_EnergyPhasesScheduleRsp;
  zclPower_Profile_PowerProfileScheduleConstraintsReq_t             pfnPowerProfile_PowerProfileScheduleConstraintsReq;
  zclPower_Profile_EnergyPhasesScheduleStateReq_t                   pfnPowerProfile_EnergyPhasesScheduleStateReq;
  zclPower_Profile_GetPowerProfilePriceExtRsp_t                     pfnPowerProfile_GetPowerProfilePriceExtRsp;
  zclPower_Profile_PowerProfileNotification_t                       pfnPowerProfile_PowerProfileNotification;
  zclPower_Profile_PowerProfileRsp_t                                pfnPowerProfile_PowerProfileRsp;
  zclPower_Profile_PowerProfileStateRsp_t                           pfnPowerProfile_PowerProfileStateRsp;
  zclPower_Profile_GetPowerProfilePrice_t                           pfnPowerProfile_GetPowerProfilePrice;
  zclPower_Profile_PowerProfileStateNotification_t                  pfnPowerProfile_PowerProfileStateNotification;
  zclPower_Profile_GetOverallSchedulePrice_t                        pfnPowerProfile_GetOverallSchedulePrice;
  zclPower_Profile_EnergyPhasesScheduleReq_t                        pfnPowerProfile_EnergyPhasesScheduleReq;
  zclPower_Profile_EnergyPhasesScheduleStateRsp_t                   pfnPowerProfile_EnergyPhasesScheduleStateRsp;
  zclPower_Profile_EnergyPhasesScheduleStateNotification_t          pfnPowerProfile_EnergyPhasesScheduleStateNotification;
  zclPower_Profile_PowerProfileScheduleConstraintsNotification_t    pfnPowerProfile_PowerProfileScheduleConstraintsNotification;
  zclPower_Profile_PowerProfileScheduleConstraintsRsp_t             pfnPowerProfile_PowerProfileScheduleConstraintsRsp;
  zclPower_Profile_GetPowerProfilePriceExt_t                        pfnPowerProfile_GetPowerProfilePriceExt;
} zclPowerProfile_AppCallbacks_t;

/******************************************************************************
 * FUNCTION MACROS
 */

/*
 * Send Energy Phases Schedule Notification cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleNotification( uint8 srcEP, afAddrType_t *dstAddr,
 *                                                                   zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                                   uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleNotification(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION, \
                                                   (c), ZCL_FRAME_CLIENT_SERVER_DIR, (d), (e) )


/*
 * Send Energy Phases Schedule Response cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleRsp( uint8 srcEP, afAddrType_t *dstAddr,
 *                                                          zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                          uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleRsp(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP, \
                                                   (c), ZCL_FRAME_CLIENT_SERVER_DIR, (d), (e) )

/*
 * Send Energy Phases Schedule State Response cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateRsp( uint8 srcEP, afAddrType_t *dstAddr,
 *                                                               zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                               uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleStateRsp(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP, \
                                                   (c), ZCL_FRAME_SERVER_CLIENT_DIR, (d), (e) )

/*
 * Send Energy Phases Schedule State Notification cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateNotification( uint8 srcEP, afAddrType_t *dstAddr,
 *                                                                        zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                                        uint8 disableDefaultRsp, uint8 seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleStateNotification(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION, \
                                                   (c), ZCL_FRAME_SERVER_CLIENT_DIR, (d), (e) )

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */


/*** Register for callbacks from this cluster library ***/
extern ZStatus_t zclPowerProfile_RegisterCmdCallbacks( uint8 endpoint, zclPowerProfile_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileReq
 *
 * @brief   Request sent to server for Power Profile info.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                       uint8 powerProfileID,
                                                       uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateReq
 *
 * @brief   Generated in order to retrieve the identifiers of current Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceRsp
 *
 * @brief   Allows a client to communicate the cost associated with a defined
 *          Power Profile to a server requesting it.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 powerProfileID, uint16 currency,
                                                               uint32 price, uint8 priceTrailingDigit,
                                                               uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetOverallSchedulePriceRsp
 *
 * @brief   Allows a client to communicate the cost associated with all
 *          Power Profiles to a server requesting it.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetOverallSchedulePriceRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint16 currency, uint32 price, uint8 priceTrailingDigit,
                                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesSchedule
 *
 * @brief   Used for Power Profile Energy Phases Schedule commands.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmdID - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RSP,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RSP,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION
 * @param   pPayload:
 *          powerProfileID - specifies the Power Profile in question
 *          numOfScheduledPhases - total number of scheduled energy phases
 *          energyPhaseID - identifier of the specific energy phase
 *          scheduledTime - relative time scheduled in respect to previous energy phase (in minutes)
 * @param   direction - send command client-to-server or server-to-client
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesSchedule( uint8 srcEP, afAddrType_t *dstAddr, uint8 cmdID,
                                                            zclPowerProfileEnergyPhasesSchedule_t *pCmd,
                                                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsReq
 *
 * @brief   Request sent to server to request constraints of the Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                                          uint8 powerProfileID,
                                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesScheduleStateReq
 *
 * @brief   Request sent to server to check the states of the scheduling of a Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                                    uint8 powerProfileID,
                                                                    uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceExtRsp
 *
 * @brief   Allows a client to communicate the cost associated with all
 *          Power Profiles scheduled to a server.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceExtRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  uint8 powerProfileID, uint16 currency,
                                                                  uint32 price, uint8 priceTrailingDigit,
                                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileNotification
 *
 * @brief   Server sends information of specific parameters belonging to each phase.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          totalProfileNum - total number of profiles supported by the device
 *          powerProfileID - identifier of the specific Power Profile
 *          numOfTransferredPhases - number of phases transferred
 *          energyPhaseID - identifier of the specific Power Profile energy phase
 *          macroPhaseID - identifier of the specific Power Profile phase
 *          expectedDuration - estimated duration of the specific phase
 *          peakPower - estimated power of the specific phase
 *          energy - estimated energy consumption for the accounted phase
 *          maxActivationDelay - maximum interruption time between end of previous phase and start of next phase
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                zclPowerProfile_t *pPayload,
                                                                uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileRsp
 *
 * @brief   A response from the server to the PowerProfileReq command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          totalProfileNum - total number of profiles supported by the device
 *          powerProfileID - identifier of the specific Power Profile
 *          numOfTransferredPhases - number of phases transferred
 *          energyPhaseID - identifier of the specific Power Profile energy phase
 *          macroPhaseID - identifier of the specific Power Profile phase
 *          expectedDuration - estimated duration of the specific phase
 *          peakPower - estimated power of the specific phase
 *          energy - estimated energy consumption for the accounted phase
 *          maxActivationDelay - maximum interruption time between end of previous phase and start of next phase
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                       zclPowerProfile_t *pPayload,
                                                       uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateRsp
 *
 * @brief   Server communicates its current Power Profile(s) to requesting client.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          powerProfileCount - number of Power Profile Records that follow in the message
 *          powerProfileRecord - includes: powerProfileID, energyPhaseID, powerProfileRemoteControl, powerProfileState
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                            zclPowerProfileState_t *pPayload,
                                                            uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePrice
 *
 * @brief   Used by server to retrieve the cost associated to a specific Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePrice( uint8 srcEP, afAddrType_t *dstAddr,
                                                            uint8 powerProfileID,
                                                            uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateNotification
 *
 * @brief   Generated by server to update the state of the power profile and
 *          current energy phase.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          powerProfileCount - number of Power Profile Records that follow in the message
 *          powerProfileRecord - includes: powerProfileID, energyPhaseID, powerProfileRemoteControl, powerProfileState
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                     zclPowerProfileState_t *pPayload,
                                                                     uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetOverallSchedulePrice
 *
 * @brief   Generated by server to retrieve the overall cost associated to
 *          all Power Profiles scheduled by the scheduler for the next 24 hrs.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetOverallSchedulePrice( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesScheduleReq
 *
 * @brief   Generated by server to retrieve from scheduler the schedule of
 *          specific Power Profile carried in the payload.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleReq( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 powerProfileID,
                                                               uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsNotification
 *
 * @brief   Generated by server to notify client of imposed constraints and
 *          allow scheduler to set proper boundaries for scheduler.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Schedule Constraints Notification
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsNotification( uint8 srcEP, afAddrType_t *dstAddr,
                                                                                   zclPowerProfileScheduleConstraintsNotification_t *pCmd,
                                                                                   uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsRsp
 *
 * @brief   Generated by server in response to PowerProfileScheduleConstraintsReq.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Schedule Constraints Rsp cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                          zclPowerProfileScheduleConstraintsRsp_t *pCmd,
                                                                          uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceExt
 *
 * @brief   Generated by server to retrieve cost associated to a specific
 *          Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Get Power Profile Price Extended cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceExt( uint8 srcEP, afAddrType_t *dstAddr,
                                                               zclPowerProfileGetPowerProfilePriceExt_t *pCmd,
                                                               uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif
#endif // ZCL_POWER_PROFILE
#endif /* ZCL_POWER_PROFILE_H */
