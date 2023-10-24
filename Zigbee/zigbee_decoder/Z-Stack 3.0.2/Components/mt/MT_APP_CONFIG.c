/***************************************************************************************************
  Filename:       MT_APP_CONFIG.c
  Revised:        $Date: 2016-03-31 01:06:52 -0700 (Thu, 31 Marc 2016) $
  Revision:       $Revision:  $

  Description:    MonitorTest functions for application configuration.

  Copyright 2007-2013 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED ?AS IS? WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

 ***************************************************************************************************/

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
 
 
#include "ZComDef.h"
#include "MT.h"
#include "MT_APP_CONFIG.h"
#include "MT_UART.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "ZDApp.h"
 
/***************************************************************************************************
* LOCAL FUNCTIONs
***************************************************************************************************/
   
#if defined (MT_APP_CNF_FUNC)

static void MT_AppCnfSetNwkFrameCounter(uint8 *pBuf);   
static void MT_AppCnfSetDefaultRemoteEndDeviceTimeout(uint8 *pBuf);
static void MT_AppCnfSetEndDeviceTimeout(uint8 *pBuf);
#if (ZG_BUILD_COORDINATOR_TYPE)
static void MT_AppCnfSetAllowRejoinTCPolicy(uint8 *pBuf);
#endif

static void MT_AppCnfBDBSetChannel(uint8* pBuf);
static void MT_AppCnfBDBStartCommissioning(uint8* pBuf);
#if (ZG_BUILD_COORDINATOR_TYPE)
    static void MT_AppCnfBDBSetTCRequireKeyExchange(uint8 *pBuf);
    static void MT_AppCnfBDBAddInstallCode(uint8 *pBuf);
    static void MT_AppCnfBDBSetJoinUsesInstallCodeKey(uint8 *pBuf);
#endif
#if (ZG_BUILD_JOINING_TYPE)
    static void MT_AppCnfBDBSetActiveCentralizedKey(uint8* pBuf);
#endif

#if (ZG_BUILD_ENDDEVICE_TYPE)    
    static void MT_AppCnfBDBZedAttemptRecoverNwk(uint8* pBuf);
#endif
      
#endif


/***************************************************************************************************
* External variables
***************************************************************************************************/
extern uint32 nwkFrameCounter;
extern uint16 nwkFrameCounterChanges;


#if defined (MT_APP_CNF_FUNC)
uint8 MT_AppCnfCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_APP_CNF_SET_NWK_FRAME_COUNTER:
      MT_AppCnfSetNwkFrameCounter(pBuf);
    break;
    case MT_APP_CNF_SET_DEFAULT_REMOTE_ENDDEVICE_TIMEOUT:
      MT_AppCnfSetDefaultRemoteEndDeviceTimeout(pBuf);
    break;
    case MT_APP_CNF_SET_ENDDEVICETIMEOUT:
      MT_AppCnfSetEndDeviceTimeout(pBuf);
    break;
#if (ZG_BUILD_COORDINATOR_TYPE)
    case MT_APP_CNF_SET_ALLOWREJOIN_TC_POLICY:
      MT_AppCnfSetAllowRejoinTCPolicy(pBuf);
    break;
#endif
    

    case MT_APP_CNF_BDB_START_COMMISSIONING:
      MT_AppCnfBDBStartCommissioning(pBuf);
    break;
    case MT_APP_CNF_BDB_SET_CHANNEL:
      MT_AppCnfBDBSetChannel(pBuf);
    break;

#if (ZG_BUILD_COORDINATOR_TYPE)
      case MT_APP_CNF_BDB_ADD_INSTALLCODE:
        MT_AppCnfBDBAddInstallCode(pBuf);
      break;
      case MT_APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE:
        MT_AppCnfBDBSetTCRequireKeyExchange(pBuf);
      break;
      case MT_APP_CNF_BDB_SET_JOINUSESINSTALLCODEKEY:
        MT_AppCnfBDBSetJoinUsesInstallCodeKey(pBuf);
      break;
#endif
#if (ZG_BUILD_JOINING_TYPE)
      case MT_APP_CNF_BDB_SET_ACTIVE_DEFAULT_CENTRALIZED_KEY:
        MT_AppCnfBDBSetActiveCentralizedKey(pBuf);
      break;
#endif
      
#if (ZG_BUILD_ENDDEVICE_TYPE)        
      case MT_APP_CNF_BDB_ZED_ATTEMPT_RECOVER_NWK:
        MT_AppCnfBDBZedAttemptRecoverNwk(pBuf);
      break;
#endif

  }
  return status;
}


/***************************************************************************************************
* @fn      MT_AppCnfCommissioningNotification
*
* @brief   Notify the host processor about an event in BDB
*
* @param   bdbCommissioningModeMsg - Commissioning notification message
*
* @return  void
***************************************************************************************************/
void MT_AppCnfCommissioningNotification(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
{
  uint8 retArray[3];
  
  retArray[0] = bdbCommissioningModeMsg->bdbCommissioningStatus;
  retArray[1] = bdbCommissioningModeMsg->bdbCommissioningMode;
  retArray[2] = bdbCommissioningModeMsg->bdbRemainingCommissioningModes;

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_APP_CNF), MT_APP_CNF_BDB_COMMISSIONING_NOTIFICATION, sizeof(bdbCommissioningModeMsg_t), retArray);
}


/***************************************************************************************************
* @fn      MT_AppCnfBDBStartCommissioning
*
* @brief   Start the commissioning process setting the commissioning mode given.
*
* @param   pBuf - pointer to received buffer
*
* @return  void
***************************************************************************************************/
static void MT_AppCnfBDBStartCommissioning(uint8* pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  bdb_StartCommissioning(*pBuf);
  
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}


/***************************************************************************************************
* @fn      MT_AppCnfBDBSetChannel
*
* @brief   Set the primary or seconday channel for discovery or formation procedure
*
* @param   pBuf - pointer to received buffer
*
* @return  void
***************************************************************************************************/
static void MT_AppCnfBDBSetChannel(uint8* pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  uint8 isPrimary;
  uint32 Channel;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  isPrimary = *pBuf;
  pBuf++;
  
  Channel = osal_build_uint32(pBuf, sizeof(uint32));
    
  bdb_setChannelAttribute(isPrimary,Channel);
  
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}



#if (ZG_BUILD_COORDINATOR_TYPE)
/*********************************************************************
 * @fn          MT_AppCnfBDBSetTCRequireKeyExchange
 *
 * @brief       Configure bdbTrustCenterRequireKeyExchange attribute.
 *
 * @param       Set attribute to FALSE if *pBuf == 0, FALSE otherwise
 *
 * @return      none
 */
static void MT_AppCnfBDBSetTCRequireKeyExchange(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  if(*pBuf)
  {
    bdb_setTCRequireKeyExchange(TRUE);
  }
  else
  {
    bdb_setTCRequireKeyExchange(FALSE);
  }
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /*********************************************************************
 * @fn          MT_AppCnfBDBSetJoinUsesInstallCodeKey
 *
 * @brief       Configure bdbJoinUsesInstallCodeKey attribute.
 *
 * @param       Set attribute to FALSE if *pBuf == 0, FALSE otherwise
 *
 * @return      none
 */
static void MT_AppCnfBDBSetJoinUsesInstallCodeKey(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  if(*pBuf)
  {
    bdb_setJoinUsesInstallCodeKey(TRUE);
  }
  else
  {
    bdb_setJoinUsesInstallCodeKey(FALSE);
  }
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /*********************************************************************
 * @fn          MT_AppCnfBDBAddInstallCode
 *
 * @brief       Add a preconfigured key used as IC derived key to TC device
 *              see formats allowed in BDB_INSTALL_CODE_USE.
 *
 * @param       pBuf - pointer to received buffer
 *
 * @return      none
 */
static void MT_AppCnfBDBAddInstallCode(uint8* pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  uint8 *pExtAddr;
  uint8 installCodeFormat;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  installCodeFormat = *pBuf;
  
  pBuf++;
  
  /* Extended Addr */
  pExtAddr = pBuf;
  pBuf += Z_EXTADDR_LEN;  //Point to the IC data
   
  switch(installCodeFormat)
  {
    case BDB_INSTALL_CODE_USE_IC_CRC:
      retValue = bdb_addInstallCode(pBuf,pExtAddr);
    break;
    case BDB_INSTALL_CODE_USE_KEY:
      retValue = APSME_AddTCLinkKey(pBuf,pExtAddr);
    break;
  }
  
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
#endif
#if (ZG_BUILD_JOINING_TYPE)

 /*********************************************************************
 * @fn      MT_AppCnfBDBSetActiveCentralizedKey
 *
 * @brief   Set the active centralized key to be used.
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 */
static void MT_AppCnfBDBSetActiveCentralizedKey(uint8* pBuf)
{
  uint8 retValue;
  uint8 cmdId;
  uint8 keyMode;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  //get the key mode
  keyMode = *pBuf;
  
  //point to the key input
  pBuf++;

  retValue = bdb_setActiveCentralizedLinkKey(keyMode,pBuf);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

#endif //#if(ZG_BUILD_JOINING_TYPE)

#if (ZG_BUILD_ENDDEVICE_TYPE) 
 /*********************************************************************
 * @fn      MT_AppCnfBDBZedAttemptRecoverNwk
 *
 * @brief   Instruct the ZED to try to rejoin its previews network
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 */
static void MT_AppCnfBDBZedAttemptRecoverNwk(uint8* pBuf)
{
  uint8 retValue;
  uint8 cmdId;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
  
  retValue = bdb_ZedAttemptRecoverNwk();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
#endif




#if (ZG_BUILD_COORDINATOR_TYPE)
/***************************************************************************************************
* @fn      MT_AppCnfSetAllowRejoinTCPolicy
*
* @brief   Set the AllowRejoin TC policy
*
* @param   pBuf - pointer to received buffer
*
* @return  void
***************************************************************************************************/
static void MT_AppCnfSetAllowRejoinTCPolicy(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
 
  if(*pBuf)
  {
    zgAllowRejoins = TRUE;
  }
  else
  {
    zgAllowRejoins = FALSE;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
#endif

/***************************************************************************************************
 * @fn      MT_AppCnfSetEndDeviceTimeout
 *
 * @brief   Set End Device Timeout
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_AppCnfSetEndDeviceTimeout(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  
  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if(*pBuf > 14)
  {
    retValue = ZInvalidParameter;
  }
  else
  {
    /* Populate info */
    zgEndDeviceTimeoutValue = *pBuf;
    osal_nv_write( ZCD_NV_END_DEV_TIMEOUT_VALUE, 0, sizeof(zgEndDeviceTimeoutValue), &zgEndDeviceTimeoutValue);
  }
  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}
  
/***************************************************************************************************
 * @fn      MT_AppCnfSetDefaultRemoteEndDeviceTimeout
 *
 * @brief   Set Remote End Device Timeout
 *
 * @param   pBuf - pointer to received buffer
 *
 * @return  void
 ***************************************************************************************************/
static void MT_AppCnfSetDefaultRemoteEndDeviceTimeout(uint8 *pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;
  uint8  tempTimeoutIndex;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;
 
  /* Populate info */
  tempTimeoutIndex = *pBuf++;
  
  if(tempTimeoutIndex > 14)
  {
    retValue = ZInvalidParameter;
  }
  else
  {
    /* Populate info */
    zgNwkEndDeviceTimeoutDefault = tempTimeoutIndex;
    osal_nv_write( ZCD_NV_NWK_ENDDEV_TIMEOUT_DEF, 0, sizeof(zgNwkEndDeviceTimeoutDefault), &zgNwkEndDeviceTimeoutDefault );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

 /***************************************************************************************************
 * @fn      MT_AppCnfSetNwkFrameCounter
 *
 * @brief   Set the nwk frame counter to the specified value for the current network.
 *          THIS IS ONLY A DEBUG INTERFACE AND SHOULD NOT BE USE IN REAL APPLICATIONS
 *
 * @param   pBuf - nwk frame counter
 *
 * @return  void
 ***************************************************************************************************/
 static void MT_AppCnfSetNwkFrameCounter(uint8* pBuf)
{
  uint8 retValue = ZSuccess;
  uint8 cmdId;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  nwkFrameCounter = osal_build_uint32(pBuf, sizeof(uint32));
  nwkFrameCounterChanges = 0;
  
  //Save the NwkFrameCounter
  ZDApp_SaveNwkKey();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_APP_CNF), cmdId, 1, &retValue);
}

#endif

