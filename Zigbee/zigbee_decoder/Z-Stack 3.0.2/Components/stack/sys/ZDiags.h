/**************************************************************************************************
  Filename:       ZDiags.h
  Revised:        $Date: 2014-03-13 15:53:56 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37678 $

  Description:    This interface provides all the definitions for the
                  Diagnostics module.


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

#ifndef ZDIAGS_H
#define ZDIAGS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * CONSTANTS
 */
// System and Hardware Attributes, ID range 0 - 99
#define ZDIAGS_SYSTEM_CLOCK                             0x0000  // System Clock when stats were saved/cleared
#define ZDIAGS_NUMBER_OF_RESETS                         0x0001  // Increments every time the system resets
#define ZDIAGS_PERSISTENT_MEMORY_WRITES                 0x0002  // *** NOT IMPLEMENTED ***

// MAC Attributes, ID range 100 - 199
#define ZDIAGS_MAC_RX_CRC_PASS                          0x0064  // MAC diagnostic CRC success counter
#define ZDIAGS_MAC_RX_CRC_FAIL                          0x0065  // MAC diagnostic CRC failure counter
#define ZDIAGS_MAC_RX_BCAST                             0x0066  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_TX_BCAST                             0x0067  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_RX_UCAST                             0x0068  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_TX_UCAST                             0x0069  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_TX_UCAST_RETRY                       0x006A  // MAC layer retries a unicast
#define ZDIAGS_MAC_TX_UCAST_FAIL                        0x006B  // Mac layer fails to send a unicast

// NWK Atributes, ID range 200 - 299
#define ZDIAGS_ROUTE_DISC_INITIATED                     0x00C8  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NEIGHBOR_ADDED                           0x00C9  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NEIGHBOR_REMOVED                         0x00CA  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NEIGHBOR_STALE                           0x00CB  // *** NOT IMPLEMENTED ***
#define ZDIAGS_JOIN_INDICATION                          0x00CC  // *** NOT IMPLEMENTED ***
#define ZDIAGS_CHILD_MOVED                              0x00CD  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NWK_FC_FAILURE                           0x00CE  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NWK_DECRYPT_FAILURES                     0x00CF  // NWK packet decryption failed
#define ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES          0x00D0  // *** NOT IMPLEMENTED ***
#define ZDIAGS_RELAYED_UCAST                            0x00D1  // *** NOT IMPLEMENTED ***
#define ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED           0x00D2  // *** NOT IMPLEMENTED ***
#define ZDIAGS_PACKET_VALIDATE_DROP_COUNT               0x00D3  // NWK packet drop because of validation error

// APS Attributes, ID range 300 - 399
#define ZDIAGS_APS_RX_BCAST                             0x012C  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_TX_BCAST                             0x012D  // APS layer transmits broadcast
#define ZDIAGS_APS_RX_UCAST                             0x012E  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_TX_UCAST_SUCCESS                     0x012F  // APS layer successfully transmits a unicast
#define ZDIAGS_APS_TX_UCAST_RETRY                       0x0130  // APS layer retries the sending of a unicast
#define ZDIAGS_APS_TX_UCAST_FAIL                        0x0131  // APS layer fails to send a unicast
#define ZDIAGS_APS_FC_FAILURE                           0x0132  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_UNAUTHORIZED_KEY                     0x0133  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_DECRYPT_FAILURES                     0x0134  // APS packet decryption failed
#define ZDIAGS_APS_INVALID_PACKETS                      0x0135  // APS invalid packet dropped
#define ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS           0x0136  // Number of MAC retries per APS message successfully Tx

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint32 SysClock;                          // ZDIAGS_SYSTEM_CLOCK
  uint16 PersistentMemoryWrites;            // ZDIAGS_PERSISTENT_MEMORY_WRITES *** NOT IMPLEMENTED ***

  uint32 MacRxCrcPass;                      // ZDIAGS_MAC_RX_CRC_PASS
  uint32 MacRxCrcFail;                      // ZDIAGS_MAC_RX_CRC_FAIL
  uint32 MacRxBcast;                        // ZDIAGS_MAC_RX_BCAST *** NOT IMPLEMENTED ***
  uint32 MacTxBcast;                        // ZDIAGS_MAC_TX_BCAST *** NOT IMPLEMENTED ***
  uint32 MacRxUcast;                        // ZDIAGS_MAC_RX_UCAST *** NOT IMPLEMENTED ***
  uint32 MacTxUcast;                        // ZDIAGS_MAC_TX_UCAST *** NOT IMPLEMENTED ***
  uint32 MacTxUcastRetry;                   // ZDIAGS_MAC_TX_UCAST_RETRY
  uint32 MacTxUcastFail;                    // ZDIAGS_MAC_TX_UCAST_FAIL

  uint16 RouteDiscInitiated;                // ZDIAGS_ROUTE_DISC_INITIATED *** NOT IMPLEMENTED ***
  uint16 NeighborAdded;                     // ZDIAGS_NEIGHBOR_ADDED *** NOT IMPLEMENTED ***
  uint16 NeighborRemoved;                   // ZDIAGS_NEIGHBOR_REMOVED *** NOT IMPLEMENTED ***
  uint16 NeighborStale;                     // ZDIAGS_NEIGHBOR_STALE *** NOT IMPLEMENTED ***
  uint16 JoinIndication;                    // ZDIAGS_JOIN_INDICATION *** NOT IMPLEMENTED ***
  uint16 ChildMoved;                        // ZDIAGS_CHILD_MOVED *** NOT IMPLEMENTED ***
  uint16 NwkFcFailure;                      // ZDIAGS_NWK_FC_FAILURE *** NOT IMPLEMENTED ***
  uint16 NwkDecryptFailures;                // ZDIAGS_NWK_DECRYPT_FAILURES
  uint16 PacketBufferAllocateFailures;      // ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES *** NOT IMPLEMENTED ***
  uint16 RelayedUcast;                      // ZDIAGS_RELAYED_UCAST *** NOT IMPLEMENTED ***
  uint16 PhyToMacQueueLimitReached;         // ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED *** NOT IMPLEMENTED ***
  uint16 PacketValidateDropCount;           // ZDIAGS_PACKET_VALIDATE_DROP_COUNT

  uint16 ApsRxBcast;                        // ZDIAGS_APS_RX_BCAST *** NOT IMPLEMENTED ***
  uint16 ApsTxBcast;                        // ZDIAGS_APS_TX_BCAST
  uint16 ApsRxUcast;                        // ZDIAGS_APS_RX_UCAST *** NOT IMPLEMENTED ***
  uint16 ApsTxUcastSuccess;                 // ZDIAGS_APS_TX_UCAST_SUCCESS
  uint16 ApsTxUcastRetry;                   // ZDIAGS_APS_TX_UCAST_RETRY
  uint16 ApsTxUcastFail;                    // ZDIAGS_APS_TX_UCAST_FAIL
  uint16 ApsFcFailure;                      // ZDIAGS_APS_FC_FAILURE *** NOT IMPLEMENTED ***
  uint16 ApsUnauthorizedKey;                // ZDIAGS_APS_UNAUTHORIZED_KEY *** NOT IMPLEMENTED ***
  uint16 ApsDecryptFailures;                // ZDIAGS_APS_DECRYPT_FAILURES
  uint16 ApsInvalidPackets;                 // ZDIAGS_APS_INVALID_PACKETS
  uint16 MacRetriesPerApsTxSuccess;         // ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS
} DiagStatistics_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * FUNCTIONS
 */
extern uint8 ZDiagsInitStats( void );

extern uint32 ZDiagsClearStats( bool clearNV );

extern void ZDiagsUpdateStats( uint16 attributeId );

extern uint32 ZDiagsGetStatsAttr( uint16 attributeId );

extern DiagStatistics_t *ZDiagsGetStatsTable( void );

extern uint8 ZDiagsRestoreStatsFromNV( void );

extern uint32 ZDiagsSaveStatsToNV( void );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDIAGS_H */
