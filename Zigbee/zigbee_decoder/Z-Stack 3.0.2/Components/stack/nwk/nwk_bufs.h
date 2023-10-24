/**************************************************************************************************
  Filename:       nwk_bufs.h
  Revised:        $Date: 2014-11-18 02:32:26 -0800 (Tue, 18 Nov 2014) $
  Revision:       $Revision: 41160 $

  Description:    Network buffer utility functions.


  Copyright 2004-2014 Texas Instruments Incorporated. All rights reserved.

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

#ifndef NWK_BUFS_H
#define NWK_BUFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "ZMAC.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
// Data buffer queue states
#define NWK_DATABUF_INIT        0     // Initialized but not queued.
#define NWK_DATABUF_WAITING     1     // Waiting to be sent out.
#define NWK_DATABUF_SENT        2     // Sent to the MAC - waiting for confirm.
#define NWK_DATABUF_CONFIRMED   3     // Waiting to be resent or deleted.
#define NWK_DATABUF_SCHEDULED   4     // Queued and waiting N msecs til send.
#define NWK_DATABUF_HOLD        5     // Hold msg for sleeping end device.
#define NWK_DATABUF_DONE        6     // Waiting to be deleted.

// Handle options
#define HANDLE_NONE               0x0000u
#define HANDLE_CNF                0x0001u
#define HANDLE_WAIT_FOR_ACK       0x0002u
#define HANDLE_BROADCAST          0x0004u
#define HANDLE_REFLECT            0x0008u
#define HANDLE_DELAY              0x0010u
#define HANDLE_HI_DELAY           0x0020u
//#define HANDLE_DIRECT             0x0040
#define HANDLE_SKIP_ROUTING       0x0040u
#define HANDLE_RTRY_MASK          0x0380u
#define HANDLE_RTRY_SHIFT         7
#define HANDLE_FORCE_INDIRECT     0x0400u
#define HANDLE_INDIRECT_HOLD      0x0800u      // A bit indicating the indirect msg has been in HOLD state
#define HANDLE_MCAST              0x1000u
#define HANDLE_MASK  \
  ~( HANDLE_CNF | HANDLE_WAIT_FOR_ACK | HANDLE_BROADCAST | HANDLE_DELAY | \
     HANDLE_HI_DELAY | HANDLE_FORCE_INDIRECT | HANDLE_MCAST)

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint8 type;
  uint16 txOptions;
  void* load;
} nwkDB_UserData_t;

typedef struct
{
  ZMacDataReq_t *pDataReq;
  void *next;
  uint16 dataX;
  uint16 handleOptions;     // Packet type options
  uint8 nsduHandle;         // unique ID
  uint8 state;              // state of buffer
  uint8 retries;            // number of APS retries
  uint8 lastCnfStatus;      // The last MAC_MCPS_DATA_CNF status
  nwkDB_UserData_t ud;      // user data
  uint16 macSrcAddr;        // original MAC src address
  uint8  apsRetries;        // Number of retries by APS layer
} nwkDB_t;

typedef uint8 (*nwkDB_FindMatchCB_t)( nwkDB_t* db, void* mf );

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Variable initialization
 */
extern void nwkbufs_init( void );

/*
 * Send the next buffer
 */
extern void nwk_SendNextDataBuf( void );

/*
 * Delete all buffers for a device
 */
extern uint8 nwk_DeleteDataBufs( uint16 nwkAddr );

/*
 * Determines whether or not the data buffers are full.
 */
extern byte nwk_MacDataBuffersFull( void );

/*
 * Add buffer to the send queue, if already in queue - set
 */
extern uint8 nwk_MacDataBuffersAdd( nwkDB_t* db, uint8 sent );

/*
 * Deallocate the sent MAC Data Buffer
 *
 */
extern uint8 nwk_MacDataBuffersDealloc( byte handle );

/*
 * Checks if an end device has a message pending in the MAC NP.
 */
extern uint8 nwkDB_MaxIndirectSent( uint16 addr );


/*********************************************************************
*  Queued Allocated functions
*/

/*
 * Create the header
 */
extern nwkDB_t *nwkDB_CreateHdr( ZMacDataReq_t *pkt, byte handle, uint16 handleOptions );

/*
 * Add a buffer to the queue.
 */
extern ZStatus_t nwkDB_Add( nwkDB_t *pkt, byte type, uint16 dataX );

/*
 * Find the number of buffers with type.
 */
extern byte nwkDB_CountTypes( byte type );

/*
 * Find the next type in list.
 */
extern nwkDB_t *nwkDB_FindNextType( nwkDB_t *pkt, byte type, byte direct );

/*
 * Find the rec with handle.
 */
extern nwkDB_t *nwkDB_FindHandle( byte handle );

/*
 * Find the rec with destination address.
 */
extern nwkDB_t *nwkDB_FindDstAddr( uint16 addr );

/*
 * Find the rec with MAC data packet.
 */
extern nwkDB_t *nwkDB_FindDataPkt( ZMacDataReq_t *pkt );

/*
 * Find a buffer match.
 */
extern nwkDB_t* nwkDB_FindMatch( nwkDB_FindMatchCB_t cb, void* mf );

/*
 * Find and remove from the list.  This function doesn't
 *           free the memory used by the record.
 */
extern void nwkDB_RemoveFromList( nwkDB_t *pkt );

/*
 * Frees the data, mac packet and hdr
 */
extern void nwkDB_DeleteRecAll( nwkDB_t *rec );

/*
 * Setup hold state and timer tick.
 */
extern void nwkbufs_hold( nwkDB_t *rec );

/*
 * Return cntIndirectHolding
 */
extern uint8 nwkDB_ReturnIndirectHoldingCnt( void );

/*
 * Count Indirect held message
 */
extern uint8 nwkDB_CountIndirectHold( void );

/*
 * Delete all records and reset queue
 */
extern void nwkbufs_reset( void );

/*
 * Stub to load the user frame data
 */
extern void* nwkDB_UserDataLoad( nwkDB_UserData_t* ud );

/*********************************************************************
*  Broadcast Message Queue functions
*/

/*
 * Add a broadcast data indication to the network broadcast queue
 */
extern uint8 nwk_broadcastSend( uint8 *msg_ptr );

/*
 * Remove a broadcast data indication to the network broadcast queue
 */
extern uint8 *nwk_broadcastReceive( void );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* NWK_BUFS_H */


