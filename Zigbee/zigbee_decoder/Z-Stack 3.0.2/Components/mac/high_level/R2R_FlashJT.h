/*******************************************************************************
  Filename:       R2R_FlashJT.h
  Revised:        $Date: 2014-12-15 17:16:20 -0800 (Mon, 15 Dec 2014) $
  Revision:       $Revision: 41505 $

  Description:    This file contains the defines for every High Level MAC
                  function which can be mapped to either itself (for Flash-Only
                  build), or to jump table offset in flash (ROM build). The
                  latter can be used to relocate any function to flash in the
                  event that software needs to be replaced.

  Copyright 2014 Texas Instruments Incorporated. All rights reserved.

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
*******************************************************************************/

#ifndef R2R_FLASH_JT_H
#define R2R_FLASH_JT_H

#if defined( ROM_BUILD )

/*******************************************************************************
 * EXTERNS
 */

// ROM's RAM table for pointers to ICall functions and flash jump tables.
// Note: This linker imported symbol is treated as a variable by the compiler.
// 0: iCall Dispatch Function Pointer
// 1: iCall Enter Critical Section Function Pointer
// 2: iCall Leave Crtiical Section Function Pointer
// 3: R2F Flash Jump Table Pointer
// 4: R2R Flash Jump Table Pointer
extern uint32 RAM_BASE_ADDR[];

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"

/*******************************************************************************
 * CONSTANTS
 */

// ROM's RAM table offset to R2R flash jump table pointer.
#define ROM_RAM_TABLE_R2R                          4

// Defines used for the flash jump table routines that are not part of build.
// Note: Any change to this table must accompany a change to R2R_Flash_JT[]!
#define R2R_JT_LOCATION                            (&RAM_BASE_ADDR[ROM_RAM_TABLE_R2R])

#define R2R_JT_BASE                                (*((uint32 **)R2R_JT_LOCATION))
#define R2R_JT_OFFSET(index)                       (*(R2R_JT_BASE+(index)))

// MAC ROM-to-ROM Functions
#define MAP_MAC_Init                               ((void               (*) (void))                                                                                      R2R_JT_OFFSET(0))
#define MAP_MAC_InitBeaconCoord                    ((void               (*) (void))                                                                                      R2R_JT_OFFSET(1))
#define MAP_MAC_InitBeaconDevice                   ((void               (*) (void))                                                                                      R2R_JT_OFFSET(2))   
#define MAP_MAC_InitCoord                          ((void               (*) (void))                                                                                      R2R_JT_OFFSET(3))                             
#define MAP_MAC_InitDevice                         ((void               (*) (void))                                                                                      R2R_JT_OFFSET(4))          
#define MAP_MAC_McpsDataAlloc                      ((macMcpsDataReq_t * (*) (uint8, uint8, uint8))                                                                       R2R_JT_OFFSET(5))  
#define MAP_MAC_McpsDataReq                        ((void               (*) (macMcpsDataReq_t *))                                                                        R2R_JT_OFFSET(6))
#define MAP_MAC_McpsPurgeReq                       ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(7))
#define MAP_MAC_MlmeAssociateReq                   ((void               (*) (macMlmeAssociateReq_t *))                                                                   R2R_JT_OFFSET(8))  
#define MAP_MAC_MlmeAssociateRsp                   ((void               (*) (macMlmeAssociateRsp_t *))                                                                   R2R_JT_OFFSET(9))   
#define MAP_MAC_MlmeDisassociateReq                ((void               (*) (macMlmeDisassociateReq_t *))                                                                R2R_JT_OFFSET(10))
#define MAP_MAC_MlmeGetPointerSecurityReq          ((uint8              (*) (uint8, void **))                                                                            R2R_JT_OFFSET(11))
#define MAP_MAC_MlmeGetReq                         ((uint8              (*) (uint8, void *))                                                                             R2R_JT_OFFSET(12))
#define MAP_MAC_MlmeGetReqSize                     ((uint8              (*) (uint8))                                                                                     R2R_JT_OFFSET(13))
#define MAP_MAC_MlmeGetSecurityReq                 ((uint8              (*) (uint8, void *))                                                                             R2R_JT_OFFSET(14))
#define MAP_MAC_MlmeGetSecurityReqSize             ((uint8              (*) (uint8))                                                                                     R2R_JT_OFFSET(15)) 
#define MAP_MAC_MlmeOrphanRsp                      ((void               (*) (macMlmeOrphanRsp_t *))                                                                      R2R_JT_OFFSET(16))       
#define MAP_MAC_MlmePollReq                        ((void               (*) (macMlmePollReq_t *))                                                                        R2R_JT_OFFSET(17))         
#define MAP_MAC_MlmeResetReq                       ((uint8              (*) (bool))                                                                                      R2R_JT_OFFSET(18))        
#define MAP_MAC_MlmeScanReq                        ((void               (*) (macMlmeScanReq_t *))                                                                        R2R_JT_OFFSET(19))     
#define MAP_MAC_MlmeSetReq                         ((uint8              (*) (uint8, void *))                                                                             R2R_JT_OFFSET(20))
#define MAP_MAC_MlmeSetSecurityReq                 ((uint8              (*) (uint8, void *))                                                                             R2R_JT_OFFSET(21)) 
#define MAP_MAC_MlmeStartReq                       ((void               (*) (macMlmeStartReq_t *))                                                                       R2R_JT_OFFSET(22))     
#define MAP_MAC_MlmeSyncReq                        ((void               (*) (macMlmeSyncReq_t *))                                                                        R2R_JT_OFFSET(23))       
#define MAP_MAC_PwrMode                            ((uint8            	(*) (void))                                                                                      R2R_JT_OFFSET(24))  
#define MAP_MAC_PwrNextTimeout                     ((uint32            	(*) (void))                                                                                      R2R_JT_OFFSET(25))  
#define MAP_MAC_PwrOffReq                          ((uint8            	(*) (uint8))                                                                                     R2R_JT_OFFSET(26))        
#define MAP_MAC_PwrOnReq                           ((void               (*) (void))                                                                                      R2R_JT_OFFSET(27))             
#define MAP_MAC_RandomByte                         ((uint8            	(*) (void))                                                                                      R2R_JT_OFFSET(28))         
#define MAP_MAC_ResumeReq                          ((void             	(*) (void))                                                                                      R2R_JT_OFFSET(29))            
#define MAP_MAC_YieldReq                           ((uint8             	(*) (void))                                                                                      R2R_JT_OFFSET(30))             
#define MAP_macAllocTxBuffer                       ((macTx_t *          (*) (uint8, macSec_t *))                                                                         R2R_JT_OFFSET(31)) 
#define MAP_macApiAssociateReq                     ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(32))
#define MAP_macApiAssociateRsp                     ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(33)) 
#define MAP_macApiBadState                         ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(34)) 
#define MAP_macApiBeaconStartReq                   ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(35))
#define MAP_macApiDataReq                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(36)) 
#define MAP_macApiDisassociateReq                  ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(37)) 
#define MAP_macApiOrphanRsp                        ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(38))
#define MAP_macApiPending                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(39))
#define MAP_macApiPollReq                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(40))
#define MAP_macApiPurgeReq                         ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(41))
#define MAP_macApiPwrOnReq                         ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(42))
#define MAP_macApiScanReq                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(43))
#define MAP_macApiStartReq                         ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(44)) 
#define MAP_macApiSyncReq                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(45)) 
#define MAP_macApiUnsupported                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(46)) 
#define MAP_macAssocDataReq                        ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(47))
#define MAP_macAssocDataReqComplete                ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(48))
#define MAP_macAssocDataRxInd                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(49)) 
#define MAP_macAssocFailed                         ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(50)) 
#define MAP_macAssocFrameResponseTimeout           ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(51))
#define MAP_macAssocRxDisassoc                     ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(52))
#define MAP_macAssociateCnf                        ((void               (*) (uint8, uint16))                                                                             R2R_JT_OFFSET(53))
#define MAP_macAutoPendAddSrcMatchTableEntry       ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(54))
#define MAP_macAutoPendMaintainSrcMatchTable       ((void               (*) (macTx_t*))                                                                                  R2R_JT_OFFSET(55))
#define MAP_macAutoPoll                            ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(56))
#define MAP_macBackoffTimerRolloverCallback        ((void               (*) (void))                                                                                      R2R_JT_OFFSET(57))
#define MAP_macBackoffTimerTriggerCallback         ((void               (*) (void))                                                                                      R2R_JT_OFFSET(58))
#define MAP_macBeaconBattLifeCallback              ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(59))
#define MAP_macBeaconCheckSched                    ((uint8              (*) (void))                                                                                      R2R_JT_OFFSET(60)) 
#define MAP_macBeaconCheckStartTime                ((uint8              (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(61)) 
#define MAP_macBeaconCheckTxTime                   ((uint8              (*) (void))                                                                                      R2R_JT_OFFSET(62)) 
#define MAP_macBeaconClearIndirect                 ((void               (*) (void))                                                                                      R2R_JT_OFFSET(63))
#define MAP_macBeaconInit                          ((void               (*) (void))                                                                                      R2R_JT_OFFSET(64))
#define MAP_macBeaconPeriodCallback                ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(65))
#define MAP_macBeaconPrepareCallback               ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(66)) 
#define MAP_macBeaconRequeue                       ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(67))
#define MAP_macBeaconReset                         ((void               (*) (void))                                                                                      R2R_JT_OFFSET(68))
#define MAP_macBeaconSchedRequested                ((void               (*) (void))                                                                                      R2R_JT_OFFSET(69))
#define MAP_macBeaconSetPrepareTime                ((void               (*) (void))                                                                                      R2R_JT_OFFSET(70))
#define MAP_macBeaconSetSched                      ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(71))
#define MAP_macBeaconSetupBroadcast                ((void               (*) (void))                                                                                      R2R_JT_OFFSET(72)) 
#define MAP_macBeaconSetupCap                      ((void               (*) (uint8, uint8, uint16))                                                                      R2R_JT_OFFSET(73))
#define MAP_macBeaconStartContinue                 ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(74))
#define MAP_macBeaconStartFrameResponseTimer       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(75)) 
#define MAP_macBeaconStopTrack                     ((void               (*) (void))                                                                                      R2R_JT_OFFSET(76))
#define MAP_macBeaconSyncLoss                      ((void               (*) (void))                                                                                      R2R_JT_OFFSET(77)) 
#define MAP_macBeaconTxCallback                    ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(78))
#define MAP_macBlacklistChecking                   ((uint8              (*) (keyDescriptor_t *, uint8 *, uint8 , deviceDescriptor_t **, keyDeviceDescriptor_t **))       R2R_JT_OFFSET(79))
#define MAP_macBroadcastPendCallback               ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(80))
#define MAP_macBuildAssociateReq                   ((uint8              (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(81))
#define MAP_macBuildAssociateRsp                   ((uint8              (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(82))
#define MAP_macBuildBeacon                         ((macTx_t *          (*) (uint8, uint8, bool))                                                                        R2R_JT_OFFSET(83)) 
#define MAP_macBuildBeaconNotifyInd                ((void               (*) (macMlmeBeaconNotifyInd_t *, macEvent_t *))                                                  R2R_JT_OFFSET(84))
#define MAP_macBuildCommonReq                      ((uint8              (*) (uint8, uint8, sAddr_t *, uint16 , uint16, macSec_t *))                                      R2R_JT_OFFSET(85))
#define MAP_macBuildDataFrame                      ((uint8              (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(86))
#define MAP_macBuildDisassociateReq                ((uint8              (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(87))
#define MAP_macBuildEnhanceBeaconReq               ((uint8              (*) (macTx_t *, sAddr_t *, macSec_t *, uint8, uint8, uint8))                                     R2R_JT_OFFSET(88)) 
#define MAP_macBuildHeader                         ((uint8              (*) (macTx_t *, uint8, sAddr_t *, uint16))                                                       R2R_JT_OFFSET(89))
#define MAP_macBuildPendAddr                       ((uint8 *            (*) (uint8 *, uint8, bool *))                                                                    R2R_JT_OFFSET(90))
#define MAP_macBuildRealign                        ((uint8              (*) (macTx_t *, sAddr_t *, uint16, uint16, uint8))                                               R2R_JT_OFFSET(91))
#define MAP_macCbackForEvent                       ((void               (*) (macEvent_t *, uint8))                                                                       R2R_JT_OFFSET(92))
#define MAP_macCcmStarInverseTransform             ((uint8              (*) (uint8 *, uint8, uint32 *, uint8 *,uint8 *, uint8, uint8 *, uint8))                          R2R_JT_OFFSET(93)) 
#define MAP_macCcmStarTransform                    ((uint8              (*) (uint8 *, uint32, uint8, uint8 *, uint8, uint8 *, uint8))                                    R2R_JT_OFFSET(94))
#define MAP_macCheckPendAddr                       ((uint8              (*) (uint8, uint8 *))                                                                            R2R_JT_OFFSET(95))
#define MAP_macCheckSched                          ((uint8              (*) (void))                                                                                      R2R_JT_OFFSET(96))
#define MAP_macCommStatusInd                       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(97)) 
#define MAP_macConflictSyncLossInd                 ((void               (*) (void))                                                                                      R2R_JT_OFFSET(98)) 
#define MAP_macCoordAddrCmp                        ((bool               (*) (sAddr_t *))                                                                                 R2R_JT_OFFSET(99))
#define MAP_macCoordDestAddrCmp                    ((bool               (*) (uint8 *p))                                                                                  R2R_JT_OFFSET(100))
#define MAP_macCoordReset                          ((void               (*) (void))                                                                                      R2R_JT_OFFSET(101))
#define MAP_macDataReset                           ((void               (*) (void))                                                                                      R2R_JT_OFFSET(102))
#define MAP_macDataRxInd                           ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(103))
#define MAP_macDataRxMemAlloc                      ((uint8 *            (*) (uint16))                                                                                    R2R_JT_OFFSET(104))
#define MAP_macDataRxMemFree                       ((uint8              (*) (uint8 **))                                                                                  R2R_JT_OFFSET(105))
#define MAP_macDataSend                            ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(106))
#define MAP_macDataTxComplete                      ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(107))
#define MAP_macDataTxDelayCallback                 ((void               (*) (uint8 ))                                                                                    R2R_JT_OFFSET(108))
#define MAP_macDataTxEnqueue                       ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(109))
#define MAP_macDataTxSend                          ((void               (*) (void))                                                                                      R2R_JT_OFFSET(110))
#define MAP_macDataTxTimeAvailable                 ((uint8              (*) (void))                                                                                      R2R_JT_OFFSET(111))
#define MAP_macDefaultAction                       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(112))
#define MAP_macDestAddrCmp                         ((bool               (*) (uint8 *, uint8 *))                                                                          R2R_JT_OFFSET(113))
#define MAP_macDestSAddrCmp                        ((bool               (*) (sAddr_t *, uint16 , uint8 *))                                                               R2R_JT_OFFSET(114))
#define MAP_macDeviceDescriptorLookup              ((uint8              (*) (deviceDescriptor_t *, uint8 *, uint8))                                                      R2R_JT_OFFSET(115))
#define MAP_macDeviceReset                         ((void               (*) (void))                                                                                      R2R_JT_OFFSET(116))
#define MAP_macDisassocComplete                    ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(117))
#define MAP_macEventLoop                           ((uint16             (*) (uint8 , uint16))                                                                            R2R_JT_OFFSET(118))
#define MAP_macExecute                             ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(119))
#define MAP_macFrameDuration                       ((uint8              (*) (uint8 , uint16))                                                                            R2R_JT_OFFSET(120))
#define MAP_macGetCoordAddress                     ((void               (*) (sAddr_t *))                                                                                 R2R_JT_OFFSET(121))
#define MAP_macGetMyAddrMode                       ((uint8              (*) (void))                                                                                      R2R_JT_OFFSET(122))
#define MAP_macIncomingFrameSecurity               ((uint8              (*) (macRx_t * ))                                                                                R2R_JT_OFFSET(123))
#define MAP_macIncomingFrameSecurityMaterialRetrieval   ((uint8         (*) (macRx_t *, keyDescriptor_t **, deviceDescriptor_t **, keyDeviceDescriptor_t **))            R2R_JT_OFFSET(124)) 
#define MAP_macIncomingKeyUsagePolicyChecking      ((uint8              (*) (keyDescriptor_t *, uint8, uint8))                                                           R2R_JT_OFFSET(125))                
#define MAP_macIncomingNonSlottedTx                ((void               (*) (void))                                                                                      R2R_JT_OFFSET(126))
#define MAP_macIncomingSecurityLevelChecking       ((uint8              (*) (uint8, uint8, uint8))                                                                       R2R_JT_OFFSET(127))
#define MAP_macIndirectExpire                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(128))
#define MAP_macIndirectMark                        ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(129))
#define MAP_macIndirectRequeueFrame                ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(130))
#define MAP_macIndirectSend                        ((bool               (*) (sAddr_t *, uint16 ))                                                                        R2R_JT_OFFSET(131))
#define MAP_macIndirectTxFrame                     ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(132))
#define MAP_macKeyDescriptorLookup                 ((uint8              (*) (uint8 *, uint8, keyDescriptor_t **))                                                        R2R_JT_OFFSET(133))
#define MAP_macMainReserve                         ((void               (*) (uint8 *))                                                                                   R2R_JT_OFFSET(134))
#define MAP_macMainReset                           ((void               (*) (void))                                                                                      R2R_JT_OFFSET(135))
#define MAP_macMemReadRam                          ((void               (*) (macRam_t *, uint8 *, uint8))                                                                R2R_JT_OFFSET(136))
#define MAP_macMemReadRamByte                      ((uint8              (*) (macRam_t *))                                                                                R2R_JT_OFFSET(137))
#define MAP_macMemWriteRam                         ((void               (*) (macRam_t *, uint8 *, uint8))                                                                R2R_JT_OFFSET(138))
#define MAP_macMgmtReset                           ((void               (*) (void))                                                                                      R2R_JT_OFFSET(139))
#define MAP_macNoAction                            ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(140))
#define MAP_macOutgoingFrameKeyRetrieval           ((uint8              (*) ( macTx_t *, sAddr_t *, uint16, uint8 **, uint32 **))                                        R2R_JT_OFFSET(141))
#define MAP_macOutgoingFrameSecurity               ((uint8              (*) ( macTx_t *, sAddr_t  *, uint16, uint8 **, uint32 **))                                       R2R_JT_OFFSET(142))
#define MAP_macOutgoingNonSlottedTx                ((void               (*) (void))                                                                                      R2R_JT_OFFSET(143))
#define MAP_macPanConflictComplete                 ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(144))
#define MAP_macPendAddrLen                         ((uint8              (*) (uint8 *))                                                                                   R2R_JT_OFFSET(145))
#define MAP_macPibIndex                            ((uint8              (*) (uint8))                                                                                     R2R_JT_OFFSET(146))
#define MAP_macPibReset                            ((void               (*) (void))                                                                                      R2R_JT_OFFSET(147))
#define MAP_macPollCnf                             ((void               (*) (uint8, uint8))                                                                              R2R_JT_OFFSET(148))
#define MAP_macPollDataReqComplete                 ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(149))
#define MAP_macPollDataRxInd                       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(150))
#define MAP_macPollFrameResponseTimeout            ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(151))
#define MAP_macPollRxAssocRsp                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(152))
#define MAP_macPollRxDisassoc                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(153))
#define MAP_macPwrReset                            ((void               (*) (void))                                                                                      R2R_JT_OFFSET(154))
#define MAP_macPwrVote                             ((void               (*) (void))                                                                                      R2R_JT_OFFSET(155))
#define MAP_macRxAssocRsp                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(156))
#define MAP_macRxAssociateReq                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(157))
#define MAP_macRxBeacon                            ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(158))
#define MAP_macRxBeaconCritical                    ((void               (*) (macRx_t *))                                                                                 R2R_JT_OFFSET(159))
#define MAP_macRxBeaconReq                         ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(160))
#define MAP_macRxCheckMACPendingCallback           ((bool               (*) (void))                                                                                      R2R_JT_OFFSET(161))
#define MAP_macRxCheckPendingCallback              ((bool               (*) (void))                                                                                      R2R_JT_OFFSET(162))
#define MAP_macRxCompleteCallback                  ((void               (*) (macRx_t *))                                                                                 R2R_JT_OFFSET(163))
#define MAP_macRxCoordRealign                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(164))
#define MAP_macRxDataReq                           ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(165))
#define MAP_macRxDisassoc                          ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(166))
#define MAP_macRxEnhancedBeaconReq                 ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(167))
#define MAP_macRxOrphan                            ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(168))
#define MAP_macRxPanConflict                       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(169))
#define MAP_macScanCnfInit                         ((void               (*) (macMlmeScanCnf_t *, macEvent_t *))                                                          R2R_JT_OFFSET(170))
#define MAP_macScanComplete                        ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(171))
#define MAP_macScanFailedInProgress                ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(172))
#define MAP_macScanNextChan                        ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(173))
#define MAP_macScanRxBeacon                        ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(174))
#define MAP_macScanRxCoordRealign                  ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(175))
#define MAP_macScanStartTimer                      ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(176))
#define MAP_macSecCpy                              ((void               (*) (macSec_t *, macSec_t *))                                                                    R2R_JT_OFFSET(177))
#define MAP_macSecurityPibIndex                    ((uint8              (*) (uint8))                                                                                     R2R_JT_OFFSET(178))
#define MAP_macSecurityPibReset                    ((void               (*) (void))                                                                                      R2R_JT_OFFSET(179))
#define MAP_macSendDataMsg                         ((uint8              (*) (uint8 , void *, macSec_t *))                                                                R2R_JT_OFFSET(180))
#define MAP_macSendMsg                             ((void               (*) (uint8 , void *))                                                                            R2R_JT_OFFSET(181))
#define MAP_macSetEvent                            ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(182))
#define MAP_macSetSched                            ((void               (*) (macTx_t *))                                                                                 R2R_JT_OFFSET(183))
#define MAP_macStartBeaconPrepareCallback          ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(184))
#define MAP_macStartBegin                          ((uint8              (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(185))
#define MAP_macStartBroadcastPendTimer             ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(186))
#define MAP_macStartComplete                       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(187))
#define MAP_macStartContinue                       ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(188))
#define MAP_macStartFrameResponseTimer             ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(189))
#define MAP_macStartResponseTimer                  ((void               (*) (macEvent_t *))                                                                              R2R_JT_OFFSET(190))
#define MAP_macStartSetParams                      ((void               (*) (macMlmeStartReq_t *))                                                                       R2R_JT_OFFSET(191))
#define MAP_macStateIdle                           ((bool               (*) (void))                                                                                      R2R_JT_OFFSET(192))
#define MAP_macStateIdleOrPolling                  ((bool               (*) (void))                                                                                      R2R_JT_OFFSET(193))
#define MAP_macStateScanning                       ((bool               (*) (void))                                                                                      R2R_JT_OFFSET(194))
#define MAP_macSyncTimeoutCallback                 ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(195))
#define MAP_macTaskInit                            ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(196))
#define MAP_macTimer                               ((void               (*) (macTimer_t *, uint32))                                                                      R2R_JT_OFFSET(197))
#define MAP_macTimerAddTimer                       ((void               (*) (macTimer_t *, macTimerHeader_t *))                                                          R2R_JT_OFFSET(198))
#define MAP_macTimerAligned                        ((void               (*) (macTimer_t *, uint32))                                                                      R2R_JT_OFFSET(199))
#define MAP_macTimerCancel                         ((void               (*) (macTimer_t *))                                                                              R2R_JT_OFFSET(200))
#define MAP_macTimerGetTime                        ((uint32             (*) (void))                                                                                      R2R_JT_OFFSET(201))
#define MAP_macTimerInit                           ((void               (*) (void))                                                                                      R2R_JT_OFFSET(202))
#define MAP_macTimerRealign                        ((void               (*) (macRx_t *, uint8))                                                                          R2R_JT_OFFSET(203))
#define MAP_macTimerRecalcUnaligned                ((void               (*) (int32, macTimer_t *))                                                                       R2R_JT_OFFSET(204))
#define MAP_macTimerRemoveTimer                    ((uint8              (*) (macTimer_t *, macTimerHeader_t *))                                                          R2R_JT_OFFSET(205))
#define MAP_macTimerSetRollover                    ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(206))
#define MAP_macTimerStart                          ((void               (*) (uint32 initTime, uint8))                                                                    R2R_JT_OFFSET(207))
#define MAP_macTimerSyncRollover                   ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(208))
#define MAP_macTimerUpdateBackoffTimer             ((void               (*) (void))                                                                                      R2R_JT_OFFSET(209))
#define MAP_macTrackPeriodCallback                 ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(210))
#define MAP_macTrackStartCallback                  ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(211))
#define MAP_macTrackTimeoutCallback                ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(212))
#define MAP_macTxBeaconCompleteCallback            ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(213))
#define MAP_macTxCompleteCallback                  ((void               (*) (uint8))                                                                                     R2R_JT_OFFSET(214))
#define MAP_macUpdatePanId                         ((void               (*) (uint16))                                                                                    R2R_JT_OFFSET(215))
#define MAP_mac_msg_deallocate                     ((void               (*) (uint8 **))                                                                                  R2R_JT_OFFSET(216))
#define MAP_sAddrCmp                               ((bool               (*) (const sAddr_t *, const sAddr_t *))                                                          R2R_JT_OFFSET(217))
#define MAP_sAddrCpy                               ((void               (*) (sAddr_t *, const sAddr_t *))                                                                R2R_JT_OFFSET(218))
#define MAP_sAddrExtCmp                            ((bool               (*) (const uint8 * , const uint8 * ))                                                            R2R_JT_OFFSET(219))
#define MAP_sAddrExtCpy                            ((void *             (*) (uint8 * , const uint8 * ))                                                                  R2R_JT_OFFSET(220))
#define MAP_sAddrIden                              ((bool               (*) (const sAddr_t *, const sAddr_t *))                                                          R2R_JT_OFFSET(221))

#else // Flash Only Build

// High Level MAC ROM-to-ROM Functions
#define MAP_MAC_Init                               MAC_Init
#define MAP_MAC_InitBeaconCoord                    MAC_InitBeaconCoord 
#define MAP_MAC_InitBeaconDevice                   MAC_InitBeaconDevice
#define MAP_MAC_InitCoord                          MAC_InitCoord                             
#define MAP_MAC_InitDevice                         MAC_InitDevice          
#define MAP_MAC_McpsDataAlloc                      MAC_McpsDataAlloc       
#define MAP_MAC_McpsDataReq                        MAC_McpsDataReq         
#define MAP_MAC_McpsPurgeReq                       MAC_McpsPurgeReq        
#define MAP_MAC_MlmeAssociateReq                   MAC_MlmeAssociateReq    
#define MAP_MAC_MlmeAssociateRsp                   MAC_MlmeAssociateRsp    
#define MAP_MAC_MlmeDisassociateReq                MAC_MlmeDisassociateReq
#define MAP_MAC_MlmeGetReq                         MAC_MlmeGetReq
#define MAP_MAC_MlmeGetPointerSecurityReq          MAC_MlmeGetPointerSecurityReq
#define MAP_MAC_MlmeGetReqSize                     MAC_MlmeGetReqSize
#define MAP_MAC_MlmeGetSecurityReq                 MAC_MlmeGetSecurityReq
#define MAP_MAC_MlmeGetSecurityReqSize             MAC_MlmeGetSecurityReqSize
#define MAP_MAC_MlmeOrphanRsp                      MAC_MlmeOrphanRsp       
#define MAP_MAC_MlmePollReq                        MAC_MlmePollReq         
#define MAP_MAC_MlmeResetReq                       MAC_MlmeResetReq        
#define MAP_MAC_MlmeScanReq                        MAC_MlmeScanReq    
#define MAP_MAC_MlmeSetReq                         MAC_MlmeSetReq
#define MAP_MAC_MlmeSetSecurityReq                 MAC_MlmeSetSecurityReq
#define MAP_MAC_MlmeStartReq                       MAC_MlmeStartReq        
#define MAP_MAC_MlmeSyncReq                        MAC_MlmeSyncReq         
#define MAP_MAC_PwrMode                            MAC_PwrMode             
#define MAP_MAC_PwrNextTimeout                     MAC_PwrNextTimeout      
#define MAP_MAC_PwrOffReq                          MAC_PwrOffReq           
#define MAP_MAC_PwrOnReq                           MAC_PwrOnReq            
#define MAP_MAC_RandomByte                         MAC_RandomByte          
#define MAP_MAC_ResumeReq                          MAC_ResumeReq           
#define MAP_MAC_YieldReq                           MAC_YieldReq            
#define MAP_macAllocTxBuffer                       macAllocTxBuffer 
#define MAP_macApiAssociateReq                     macApiAssociateReq 
#define MAP_macApiAssociateRsp                     macApiAssociateRsp 
#define MAP_macApiBadState                         macApiBadState 
#define MAP_macApiBeaconStartReq                   macApiBeaconStartReq 
#define MAP_macApiDataReq                          macApiDataReq 
#define MAP_macApiDisassociateReq                  macApiDisassociateReq 
#define MAP_macApiOrphanRsp                        macApiOrphanRsp 
#define MAP_macApiPending                          macApiPending 
#define MAP_macApiPollReq                          macApiPollReq 
#define MAP_macApiPurgeReq                         macApiPurgeReq 
#define MAP_macApiPwrOnReq                         macApiPwrOnReq 
#define MAP_macApiScanReq                          macApiScanReq 
#define MAP_macApiStartReq                         macApiStartReq 
#define MAP_macApiSyncReq                          macApiSyncReq 
#define MAP_macApiUnsupported                      macApiUnsupported 
#define MAP_macAssocDataReq                        macAssocDataReq 
#define MAP_macAssocDataReqComplete                macAssocDataReqComplete
#define MAP_macAssocDataRxInd                      macAssocDataRxInd 
#define MAP_macAssocFailed                         macAssocFailed 
#define MAP_macAssocFrameResponseTimeout           macAssocFrameResponseTimeout
#define MAP_macAssocRxDisassoc                     macAssocRxDisassoc 
#define MAP_macAssociateCnf                        macAssociateCnf
#define MAP_macAutoPendAddSrcMatchTableEntry       macAutoPendAddSrcMatchTableEntry
#define MAP_macAutoPendMaintainSrcMatchTable       macAutoPendMaintainSrcMatchTable
#define MAP_macAutoPoll                            macAutoPoll 
#define MAP_macBackoffTimerRolloverCallback        macBackoffTimerRolloverCallback
#define MAP_macBackoffTimerTriggerCallback         macBackoffTimerTriggerCallback
#define MAP_macBeaconBattLifeCallback              macBeaconBattLifeCallback
#define MAP_macBeaconCheckSched                    macBeaconCheckSched 
#define MAP_macBeaconCheckStartTime                macBeaconCheckStartTime
#define MAP_macBeaconCheckTxTime                   macBeaconCheckTxTime 
#define MAP_macBeaconClearIndirect                 macBeaconClearIndirect 
#define MAP_macBeaconInit                          macBeaconInit 
#define MAP_macBeaconPeriodCallback                macBeaconPeriodCallback
#define MAP_macBeaconPrepareCallback               macBeaconPrepareCallback
#define MAP_macBeaconRequeue                       macBeaconRequeue 
#define MAP_macBeaconReset                         macBeaconReset 
#define MAP_macBeaconSchedRequested                macBeaconSchedRequested
#define MAP_macBeaconSetPrepareTime                macBeaconSetPrepareTime
#define MAP_macBeaconSetSched                      macBeaconSetSched 
#define MAP_macBeaconSetupBroadcast                macBeaconSetupBroadcast
#define MAP_macBeaconSetupCap                      macBeaconSetupCap 
#define MAP_macBeaconStartContinue                 macBeaconStartContinue 
#define MAP_macBeaconStartFrameResponseTimer       macBeaconStartFrameResponseTimer
#define MAP_macBeaconStopTrack                     macBeaconStopTrack 
#define MAP_macBeaconSyncLoss                      macBeaconSyncLoss 
#define MAP_macBeaconTxCallback                    macBeaconTxCallback 
#define MAP_macBlacklistChecking                   macBlacklistChecking 
#define MAP_macBroadcastPendCallback               macBroadcastPendCallback
#define MAP_macBuildAssociateReq                   macBuildAssociateReq 
#define MAP_macBuildAssociateRsp                   macBuildAssociateRsp 
#define MAP_macBuildBeacon                         macBuildBeacon 
#define MAP_macBuildBeaconNotifyInd                macBuildBeaconNotifyInd
#define MAP_macBuildCommonReq                      macBuildCommonReq 
#define MAP_macBuildDataFrame                      macBuildDataFrame 
#define MAP_macBuildDisassociateReq                macBuildDisassociateReq
#define MAP_macBuildEnhanceBeaconReq               macBuildEnhanceBeaconReq
#define MAP_macBuildHeader                         macBuildHeader 
#define MAP_macBuildPendAddr                       macBuildPendAddr 
#define MAP_macBuildRealign                        macBuildRealign 
#define MAP_macCbackForEvent                       macCbackForEvent 
#define MAP_macCcmStarInverseTransform             macCcmStarInverseTransform
#define MAP_macCcmStarTransform                    macCcmStarTransform 
#define MAP_macCheckPendAddr                       macCheckPendAddr 
#define MAP_macCheckSched                          macCheckSched 
#define MAP_macCommStatusInd                       macCommStatusInd 
#define MAP_macConflictSyncLossInd                 macConflictSyncLossInd 
#define MAP_macCoordAddrCmp                        macCoordAddrCmp 
#define MAP_macCoordDestAddrCmp                    macCoordDestAddrCmp 
#define MAP_macCoordReset                          macCoordReset 
#define MAP_macDataReset                           macDataReset 
#define MAP_macDataRxInd                           macDataRxInd
#define MAP_macDataRxMemAlloc                      macDataRxMemAlloc 
#define MAP_macDataRxMemFree                       macDataRxMemFree 
#define MAP_macDataSend                            macDataSend 
#define MAP_macDataTxComplete                      macDataTxComplete 
#define MAP_macDataTxDelayCallback                 macDataTxDelayCallback 
#define MAP_macDataTxEnqueue                       macDataTxEnqueue 
#define MAP_macDataTxSend                          macDataTxSend 
#define MAP_macDataTxTimeAvailable                 macDataTxTimeAvailable 
#define MAP_macDefaultAction                       macDefaultAction 
#define MAP_macDestAddrCmp                         macDestAddrCmp 
#define MAP_macDestSAddrCmp                        macDestSAddrCmp 
#define MAP_macDeviceDescriptorLookup              macDeviceDescriptorLookup
#define MAP_macDeviceReset                         macDeviceReset 
#define MAP_macDisassocComplete                    macDisassocComplete 
#define MAP_macEventLoop                           macEventLoop 
#define MAP_macExecute                             macExecute 
#define MAP_macFrameDuration                       macFrameDuration 
#define MAP_macGetCoordAddress                     macGetCoordAddress 
#define MAP_macGetMyAddrMode                       macGetMyAddrMode 
#define MAP_macIncomingFrameSecurity               macIncomingFrameSecurity
#define MAP_macIncomingFrameSecurityMaterialRetrieval   macIncomingFrameSecurityMaterialRetrieval
#define MAP_macIncomingKeyUsagePolicyChecking      macIncomingKeyUsagePolicyChecking
#define MAP_macIncomingNonSlottedTx                macIncomingNonSlottedTx
#define MAP_macIncomingSecurityLevelChecking       macIncomingSecurityLevelChecking
#define MAP_macIndirectExpire                      macIndirectExpire 
#define MAP_macIndirectMark                        macIndirectMark 
#define MAP_macIndirectRequeueFrame                macIndirectRequeueFrame
#define MAP_macIndirectSend                        macIndirectSend 
#define MAP_macIndirectTxFrame                     macIndirectTxFrame 
#define MAP_macKeyDescriptorLookup                 macKeyDescriptorLookup 
#define MAP_macMainReserve                         macMainReserve 
#define MAP_macMainReset                           macMainReset 
#define MAP_macMemReadRam                          macMemReadRam                                 
#define MAP_macMemReadRamByte                      macMemReadRamByte                         
#define MAP_macMemWriteRam                         macMemWriteRam                                
#define MAP_macMgmtReset                           macMgmtReset 
#define MAP_macNoAction                            macNoAction 
#define MAP_macOutgoingFrameKeyRetrieval           macOutgoingFrameKeyRetrieval
#define MAP_macOutgoingFrameSecurity               macOutgoingFrameSecurity
#define MAP_macOutgoingNonSlottedTx                macOutgoingNonSlottedTx
#define MAP_macPanConflictComplete                 macPanConflictComplete 
#define MAP_macPendAddrLen                         macPendAddrLen 
#define MAP_macPibIndex                            macPibIndex
#define MAP_macPibReset                            macPibReset
#define MAP_macPollCnf                             macPollCnf 
#define MAP_macPollDataReqComplete                 macPollDataReqComplete 
#define MAP_macPollDataRxInd                       macPollDataRxInd 
#define MAP_macPollFrameResponseTimeout            macPollFrameResponseTimeout
#define MAP_macPollRxAssocRsp                      macPollRxAssocRsp 
#define MAP_macPollRxDisassoc                      macPollRxDisassoc 
#define MAP_macPwrReset                            macPwrReset 
#define MAP_macPwrVote                             macPwrVote 
#define MAP_macRxAssocRsp                          macRxAssocRsp 
#define MAP_macRxAssociateReq                      macRxAssociateReq 
#define MAP_macRxBeacon                            macRxBeacon 
#define MAP_macRxBeaconCritical                    macRxBeaconCritical 
#define MAP_macRxBeaconReq                         macRxBeaconReq 
#define MAP_macRxCheckMACPendingCallback           macRxCheckMACPendingCallback
#define MAP_macRxCheckPendingCallback              macRxCheckPendingCallback
#define MAP_macRxCompleteCallback                  macRxCompleteCallback 
#define MAP_macRxCoordRealign                      macRxCoordRealign 
#define MAP_macRxDataReq                           macRxDataReq 
#define MAP_macRxDisassoc                          macRxDisassoc 
#define MAP_macRxEnhancedBeaconReq                 macRxEnhancedBeaconReq 
#define MAP_macRxOrphan                            macRxOrphan 
#define MAP_macRxPanConflict                       macRxPanConflict 
#define MAP_macScanCnfInit                         macScanCnfInit 
#define MAP_macScanComplete                        macScanComplete 
#define MAP_macScanFailedInProgress                macScanFailedInProgress
#define MAP_macScanNextChan                        macScanNextChan 
#define MAP_macScanRxBeacon                        macScanRxBeacon 
#define MAP_macScanRxCoordRealign                  macScanRxCoordRealign 
#define MAP_macScanStartTimer                      macScanStartTimer 
#define MAP_macSecCpy                              macSecCpy 
#define MAP_macSecurityPibIndex                    macSecurityPibIndex
#define MAP_macSecurityPibReset                    macSecurityPibReset
#define MAP_macSendDataMsg                         macSendDataMsg 
#define MAP_macSendMsg                             macSendMsg 
#define MAP_macSetEvent                            macSetEvent 
#define MAP_macSetSched                            macSetSched 
#define MAP_macStartBeaconPrepareCallback          macStartBeaconPrepareCallback
#define MAP_macStartBegin                          macStartBegin 
#define MAP_macStartBroadcastPendTimer             macStartBroadcastPendTimer
#define MAP_macStartComplete                       macStartComplete 
#define MAP_macStartContinue                       macStartContinue 
#define MAP_macStartFrameResponseTimer             macStartFrameResponseTimer
#define MAP_macStartResponseTimer                  macStartResponseTimer 
#define MAP_macStartSetParams                      macStartSetParams 
#define MAP_macStateIdle                           macStateIdle 
#define MAP_macStateIdleOrPolling                  macStateIdleOrPolling 
#define MAP_macStateScanning                       macStateScanning 
#define MAP_macSyncTimeoutCallback                 macSyncTimeoutCallback 
#define MAP_macTaskInit                            macTaskInit 
#define MAP_macTimer                               macTimer 
#define MAP_macTimerAddTimer                       macTimerAddTimer 
#define MAP_macTimerAligned                        macTimerAligned 
#define MAP_macTimerCancel                         macTimerCancel 
#define MAP_macTimerGetTime                        macTimerGetTime 
#define MAP_macTimerInit                           macTimerInit 
#define MAP_macTimerRealign                        macTimerRealign 
#define MAP_macTimerRecalcUnaligned                macTimerRecalcUnaligned
#define MAP_macTimerRemoveTimer                    macTimerRemoveTimer 
#define MAP_macTimerSetRollover                    macTimerSetRollover 
#define MAP_macTimerStart                          macTimerStart 
#define MAP_macTimerSyncRollover                   macTimerSyncRollover 
#define MAP_macTimerUpdateBackoffTimer             macTimerUpdateBackoffTimer
#define MAP_macTrackPeriodCallback                 macTrackPeriodCallback 
#define MAP_macTrackStartCallback                  macTrackStartCallback 
#define MAP_macTrackTimeoutCallback                macTrackTimeoutCallback
#define MAP_macTxBeaconCompleteCallback            macTxBeaconCompleteCallback
#define MAP_macTxCompleteCallback                  macTxCompleteCallback 
#define MAP_macUpdatePanId                         macUpdatePanId 
#define MAP_mac_msg_deallocate                     mac_msg_deallocate 
#define MAP_sAddrCmp                               sAddrCmp 
#define MAP_sAddrCpy                               sAddrCpy 
#define MAP_sAddrExtCmp                            sAddrExtCmp 
#define MAP_sAddrExtCpy                            sAddrExtCpy 
#define MAP_sAddrIden                              sAddrIden

#endif // ROM_BUILD

#endif /* R2R_FLASH_JT_H */

