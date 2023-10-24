/**************************************************************************************************
  Filename:       ZDSecMgr.h
  Revised:        $Date: 2013-09-26 15:41:00 -0700 (Thu, 26 Sep 2013) $
  Revision:       $Revision: 35469 $

  Description:    This file contains the interface to the ZigBee Device Security Manager.


  Copyright 2005-2013 Texas Instruments Incorporated. All rights reserved.

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

#ifndef ZDSECMGR_H
#define ZDSECMGR_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "ZDApp.h"

/******************************************************************************
 * TYPEDEFS
 */

// Authentication options
typedef enum
{
  ZDSecMgr_Not_Authenticated = 0,   // The device has not been authenticated
  ZDSecMgr_Authenticated_CBCK,      // The devcie has been authenticated using CBKE
  ZDSecMgr_Authenticated_EA         // The device has been authenticated using EA
}ZDSecMgr_Authentication_Option;

// Total Number of APS keys that a TC can manage. This is equal to the number of devices a ZC can allow to join the network
#if !defined ( ZDSECMGR_TC_DEVICE_MAX )
  #if (ZG_BUILD_COORDINATOR_TYPE) 
    #define ZDSECMGR_TC_DEVICE_MAX 40
  #else
    #define ZDSECMGR_TC_DEVICE_MAX 3
  #endif
#endif
  
#if !defined (ZDSECMGR_TC_DEVICE_IC_MAX)
  #if (ZG_BUILD_COORDINATOR_TYPE) 
    #define ZDSECMGR_TC_DEVICE_IC_MAX  (ZCD_NV_TCLK_IC_TABLE_END - ZCD_NV_TCLK_IC_TABLE_START)
  #else
    #define ZDSECMGR_TC_DEVICE_IC_MAX 1
  #endif
#endif

#if (ZDSECMGR_TC_DEVICE_MAX > (ZCD_NV_TCLK_TABLE_END - ZCD_NV_TCLK_TABLE_START))
  #error: The number of devices is greater than the range of NvIds assigned for this purpose. Extend the range of the Ids or reduce the number of devices.
#endif

#if (ZDSECMGR_TC_DEVICE_IC_MAX > (ZCD_NV_TCLK_IC_TABLE_END - ZCD_NV_TCLK_IC_TABLE_START))
  #error: The number of install codes is greater than the range of NvIds assigned for this purpose. Extend the range of the Ids or reduce the number of install codes.
#endif

//Define whether or not the default keys must be attempted during joining a network and install codes has been set to be used.
//This is set to TRUE for devices which does not have a user interface to toggle between install code usage or default keys 
//for joining the network, so the default keys are attempted if install code fails during the joining process.
//Ej. devices available via retail channels.
#define ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY FALSE

extern CONST uint16 gZDSECMGR_TC_DEVICE_MAX;
extern CONST uint16 gZDSECMGR_TC_DEVICE_IC_MAX;
extern uint8  gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;

typedef struct
{
uint32              FrameCounter;
uint8               extendedPanID[Z_EXTADDR_LEN];   //Extended pan Id associated to the security material
}nwkSecMaterialDesc_t;
 



/******************************************************************************
 * PUBLIC FUNCTIONS
 */
/******************************************************************************
 * @fn          ZDSecMgrInit
 *
 * @brief       Initialize ZigBee Device Security Manager.
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
extern void ZDSecMgrInit(uint8 state);

/******************************************************************************
 * @fn          ZDSecMgrConfig
 *
 * @brief       Configure ZigBee Device Security Manager.
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrConfig( void );

/******************************************************************************
 * @fn          ZDSecMgrPermitJoining
 *
 * @brief       Process request to change joining permissions.
 *
 * @param       duration - [in] timed duration for join in seconds
 *                         - 0x00 not allowed
 *                         - 0xFF allowed without timeout
 *
 * @return      uint8 - success(TRUE:FALSE)
 */
extern uint8 ZDSecMgrPermitJoining( uint8 duration );

/******************************************************************************
 * @fn          ZDSecMgrPermitJoiningTimeout
 *
 * @brief       Process permit joining timeout
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrPermitJoiningTimeout( void );

/******************************************************************************
 * @fn          ZDSecMgrNewDeviceEvent
 *
 * @brief       Process a the new device event, if found reset new device
 *              event/timer.
 *
 * @param       ShortAddr - of New Device to process
 *
 * @return      uint8 - found(TRUE:FALSE)
 */
extern uint8 ZDSecMgrNewDeviceEvent( uint16 ShortAddr );

/******************************************************************************
 * @fn          ZDSecMgrTransportKeyInd
 *
 * @brief       Process the ZDO_TransportKeyInd_t message.
 *
 * @param       ind - [in] ZDO_TransportKeyInd_t indication
 *
 * @return      none
 */
extern void ZDSecMgrTransportKeyInd( ZDO_TransportKeyInd_t* ind );

/******************************************************************************
 * @fn          ZDSecMgrUpdateDeviceInd
 *
 * @brief       Process the ZDO_UpdateDeviceInd_t message.
 *
 * @param       ind - [in] ZDO_UpdateDeviceInd_t indication
 *
 * @return      none
 */
extern void ZDSecMgrUpdateDeviceInd( ZDO_UpdateDeviceInd_t* ind );

/******************************************************************************
 * @fn          ZDSecMgrRemoveDeviceInd
 *
 * @brief       Process the ZDO_RemoveDeviceInd_t message.
 *
 * @param       ind - [in] ZDO_RemoveDeviceInd_t indication
 *
 * @return      none
 */
extern void ZDSecMgrRemoveDeviceInd( ZDO_RemoveDeviceInd_t* ind );

/******************************************************************************
 * @fn          ZDSecMgrRequestKeyInd
 *
 * @brief       Process the ZDO_RequestKeyInd_t message.
 *
 * @param       ind - [in] ZDO_RequestKeyInd_t indication
 *
 * @return      none
 */
extern void ZDSecMgrRequestKeyInd( ZDO_RequestKeyInd_t* ind );

/******************************************************************************
 * @fn          ZDSecMgrSwitchKeyInd
 *
 * @brief       Process the ZDO_SwitchKeyInd_t message.
 *
 * @param       ind - [in] ZDO_SwitchKeyInd_t indication
 *
 * @return      none
 */
extern void ZDSecMgrSwitchKeyInd( ZDO_SwitchKeyInd_t* ind );


/******************************************************************************
 * @fn          ZDSecMgrVerifyKeyInd
 *
 * @brief       Process the ZDO_VerifyKeyInd_t message.
 *
 * @param       ind - [in] ZDO_VerifyKeyInd_t indication
 *
 * @return      none
 */
extern void ZDSecMgrVerifyKeyInd( ZDO_VerifyKeyInd_t* ind );


/******************************************************************************
 * @fn          ZDSecMgrGenerateRndKey
 *
 * @brief       Generate a random key. NOTE: Random key is generated by osal_rand, refer to osal_rand to see the random properties of the key generated by this mean.
 *
 * @param       pKey - [out] Buffer pointer in which the key will be passed.
 *
 * @return      none
 */
extern void ZDSecMgrGenerateRndKey(uint8* pKey);


/******************************************************************************
 * @fn          ZDSecMgrGenerateKeyFromSeed
 *
 * @brief       Generate the TC link key for an specific device usign seed and ExtAddr
 *
 * @param       [in]  extAddr  
 * @param       [in]  shift    number of byte shifts that the seed will do to 
 *                             generate a new key for the same device. 
 *                             This value must be less than SEC_KEY_LEN
 * @param       [out] key      buffer in which the key will be copied
 *
 * @return      none
 */
extern void ZDSecMgrGenerateKeyFromSeed(uint8 *extAddr, uint8 shift, uint8 *key);

/******************************************************************************
 * @fn          ZDSecMgrUpdateNwkKey
 *
 * @brief       Load a new NWK key and trigger a network wide update.
 *
 * @param       key       - [in] new NWK key
 * @param       keySeqNum - [in] new NWK key sequence number
 *
 * @return      ZStatus_t
 */
extern ZStatus_t ZDSecMgrUpdateNwkKey( uint8* key, uint8 keySeqNum, uint16 dstAddr );

/******************************************************************************
 * @fn          ZDSecMgrSwitchNwkKey
 *
 * @brief       Causes the NWK key to switch via a network wide command.
 *
 * @param       keySeqNum - [in] new NWK key sequence number
 *
 * @return      ZStatus_t
 */
extern ZStatus_t ZDSecMgrSwitchNwkKey( uint8 keySeqNum, uint16 dstAddr );

/******************************************************************************
 * @fn          ZDSecMgrRequestAppKey
 *
 * @brief       Request an application key with partner.
 *
 * @param       partExtAddr - [in] partner extended address
 *
 * @return      ZStatus_t
 */
extern ZStatus_t ZDSecMgrRequestAppKey( uint8 *partExtAddr );

/******************************************************************************
 * @fn          ZDSecMgrSetupPartner
 *
 * @brief       Setup for application key partner.
 *
 * @param       partNwkAddr - [in] partner network address
 * @param       partExtAddr - [in] partner extended address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSetupPartner( uint16 partNwkAddr, uint8* partExtAddr );

/******************************************************************************
 * @fn          ZDSecMgrAppKeyTypeSet
 *
 * @brief       Set application key type.
 *
 * @param       keyType - [in] application key type (KEY_TYPE_APP_MASTER@2 or
 *                                                   KEY_TYPE_APP_LINK@3
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAppKeyTypeSet( uint8 keyType );

/******************************************************************************
 * @fn          ZDSecMgrAddLinkKey
 *
 * @brief       Add the application link key to ZDSecMgr.
 *
 * @param       shortAddr - [in] short address of the partner device
 * @param       extAddr - [in] extended address of the partner device
 * @param       key - [in] link key
 *
 * @return      none
 */
extern ZStatus_t ZDSecMgrAddLinkKey( uint16 shortAddr, uint8 *extAddr, uint8 *key);

/******************************************************************************
 * @fn          ZDSecMgrDeviceRemoveByExtAddr
 *
 * @brief       Remove device entry by its ext address.
 *
 * @param       pAddr - pointer to the extended address
 *
 * @return      ZStatus_t
 */
extern ZStatus_t ZDSecMgrDeviceRemoveByExtAddr( uint8 *pAddr );

/******************************************************************************
 * @fn          ZDSecMgrAddrClear
 *
 * @brief       Clear security bit from Address Manager for specific device.
 *
 * @param       extAddr - [in] EXT address
 *
 * @return      ZStatus_t
 */
extern ZStatus_t ZDSecMgrAddrClear( uint8* extAddr );

/******************************************************************************
 * @fn          ZDSecMgrInitNV
 *
 * @brief       Initialize the SecMgr entry data in NV with all values set to 0
 *
 * @param       none
 *
 * @return      uint8 - <osal_nv_item_init> return codes
 */
extern uint8 ZDSecMgrInitNV( void );

/*********************************************************************
 * @fn          ZDSecMgrSetDefaultNV
 *
 * @brief       Write the defaults to NV for Entry table and for APS key data table
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrSetDefaultNV( void );

/******************************************************************************
 * @fn          ZDSecMgrAPSRemove
 *
 * @brief       Remove device from network.
 *
 * @param       nwkAddr - device's NWK address
 * @param       extAddr - device's Extended address
 * @param       parentAddr - parent's NWK address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAPSRemove( uint16 nwkAddr, uint8 *extAddr, uint16 parentAddr );

/******************************************************************************
 * @fn          ZDSecMgrAuthenticationCheck
 *
 * @brief       Check if the specific device has been authenticated or not
 *
 * @param       shortAddr - [in] short address
 *
 * @return      uint8 - TRUE @ authenticated
 *                      FALSE @ not authenticated
 */
uint8 ZDSecMgrAuthenticationCheck( uint16 shortAddr );

/******************************************************************************
 * @fn          APSME_TCLinkKeySync
 *
 * @brief       Sync Trust Center LINK key data.
 *
 * @param       srcAddr - [in] srcAddr
 * @param       si      - [in, out] SSP_Info_t
 *
 * @return      ZStatus_t
 */
extern ZStatus_t APSME_TCLinkKeySync( uint16 srcAddr, SSP_Info_t* si );

/******************************************************************************
 * @fn          APSME_TCLinkKeyLoad
 *
 * @brief       Load Trust Center LINK key data.
 *
 * @param       dstAddr - [in] dstAddr
 * @param       si      - [in, out] SSP_Info_t
 *
 * @return      ZStatus_t
 */
extern ZStatus_t APSME_TCLinkKeyLoad( uint16 dstAddr, SSP_Info_t* si );

/*********************************************************************
 * @fn          ZDSecMgrReadKeyFromNv
 *
 * @brief       Looks for a specific key in NV based on Index value
 *
 * @param   keyNvId - Index of key to look in NV
 *                    valid values are:
 *                    ZCD_NV_NWK_ACTIVE_KEY_INFO
 *                    ZCD_NV_NWK_ALTERN_KEY_INFO
 *                    ZCD_NV_TCLK_TABLE_START + <offset_in_table>
 *                    ZCD_NV_APS_LINK_KEY_DATA_START + <offset_in_table>
 *                    ZCD_NV_PRECFGKEY
 *
 * @param  *keyinfo - Data is read into this buffer.
 *
 * @return  SUCCESS if NV data was copied to the keyinfo parameter .
 *          Otherwise, NV_OPER_FAILED for failure.
 */
extern ZStatus_t ZDSecMgrReadKeyFromNv(uint16 keyNvId, void *keyinfo);



/******************************************************************************
 * @fn          ZDSecMgrInitNVKeyTables
 *
 * @brief       Initialize the NV table for All keys: NWK, Master, TCLK and APS
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
extern void ZDSecMgrInitNVKeyTables(uint8 setDefault);

/******************************************************************************
 * @fn          ZDSecMgrSaveApsLinkKey
 *
 * @brief       Save APS Link Key to NV. It will loop through all the keys
 *              to see which one to save.
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrSaveApsLinkKey(void);

/******************************************************************************
 * @fn          ZDSecMgrSaveTCLinkKey
 *
 * @brief       Save TC Link Key to NV. It will loop through all the keys
 *              to see which one to save.
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrSaveTCLinkKey(void);

/******************************************************************************
 * @fn          ZDSecMgrClearNVKeyValues
 *
 * @brief       If NV_RESTORE is enabled and the status of the network needs
 *              default values this fuction clears ZCD_NV_NWKKEY,
 *              ZCD_NV_NWK_ACTIVE_KEY_INFO and ZCD_NV_NWK_ALTERN_KEY_INFO link
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrClearNVKeyValues(void);

/******************************************************************************
 * @fn          ZDSecMgrFallbackNwkKey
 *
 * @brief       Use the ZBA fallback network key.
 *
 * @param       none
 *
 * @return      none
 */
extern void ZDSecMgrFallbackNwkKey( void );

/******************************************************************************
 * @fn          ZDSecMgrUpdateTCAddress
 *
 * @brief       Update TC extended address and save to NV.
 *
 * @param       extAddr - [in] extended address
 *
 * @return      none
 */
extern void ZDSecMgrUpdateTCAddress( uint8 *extAddr );


/******************************************************************************
******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDSECMGR_H */
