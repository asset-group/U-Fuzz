/**************************************************************************************************
  Filename:       mac_spec.h
  Revised:        $Date: 2011-09-16 11:36:37 -0700 (Fri, 16 Sep 2011) $
  Revision:       $Revision: 27599 $

  Description:    This file contains constants and other data defined by the 802.15.4 spec.


  Copyright 2005-2010 Texas Instruments Incorporated. All rights reserved.

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
**************************************************************************************************/

#ifndef MAC_SPEC_H
#define MAC_SPEC_H

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* PHY packet fields lengths in bytes */
#define MAC_PHY_SHR_LEN                 5       /* preamble bytes plus SFD byte */
#define MAC_PHY_PHR_LEN                 1       /* length byte */

/* MAC frame field lengths in bytes */
#define MAC_FCF_FIELD_LEN               2       /* frame control field */
#define MAC_SEQ_NUM_FIELD_LEN           1       /* sequence number  */
#define MAC_PAN_ID_FIELD_LEN            2       /* PAN ID  */
#define MAC_EXT_ADDR_FIELD_LEN          8       /* Extended address */
#define MAC_SHORT_ADDR_FIELD_LEN        2       /* Short address */
#define MAC_FCS_FIELD_LEN               2       /* FCS field */
#define MAC_SEC_CONTROL_FIELD_LEN       1       /* Security control field */

/* Frame offsets in bytes */
#define MAC_FCF_OFFSET                  0       /* offset to frame control field */
#define MAC_SEQ_NUM_OFFSET              2       /* offset to sequence number */
#define MAC_DEST_PAN_ID_OFFSET          3       /* offset to destination PAN ID */
#define MAC_DEST_ADDR_OFFSET            5       /* offset to destination address */

/* Frame control field bit masks */
#define MAC_FCF_FRAME_TYPE_MASK         0x0007
#define MAC_FCF_SEC_ENABLED_MASK        0x0008
#define MAC_FCF_FRAME_PENDING_MASK      0x0010
#define MAC_FCF_ACK_REQUEST_MASK        0x0020
#define MAC_FCF_INTRA_PAN_MASK          0x0040
#define MAC_FCF_DST_ADDR_MODE_MASK      0x0C00
#define MAC_FCF_FRAME_VERSION_MASK      0x3000
#define MAC_FCF_SRC_ADDR_MODE_MASK      0xC000

/* Frame control field bit positions */
#define MAC_FCF_FRAME_TYPE_POS          0
#define MAC_FCF_SEC_ENABLED_POS         3
#define MAC_FCF_FRAME_PENDING_POS       4
#define MAC_FCF_ACK_REQUEST_POS         5
#define MAC_FCF_INTRA_PAN_POS           6
/* Bit positions 8,9 for FCF as per new spec */
#define MAC_FCF_SEQ_NO_SUPPRESSION_POS  8
#define MAC_FCF_IE_LIST_PRESENT_POS     9
#define MAC_FCF_DST_ADDR_MODE_POS       10
#define MAC_FCF_FRAME_VERSION_POS       12
#define MAC_FCF_SRC_ADDR_MODE_POS       14

/* Security control field bit masks */
#define MAC_SCF_SECURITY_LEVEL_MASK     0x07
#define MAC_SCF_KEY_IDENTIFIER_MASK     0x18

/* Security control field bit positions */
#define MAC_SCF_SECURITY_LEVEL_POS      0
#define MAC_SCF_KEY_IDENTIFIER_POS      3

/* MAC Payload offsets in bytes */
#define MAC_SFS_OFFSET                  0
#define MAC_PENDING_ADDR_OFFSET         3       /* if GTS is not in use */

/* Beacon superframe spec bit positions, low byte */
#define MAC_SFS_BEACON_ORDER_POS        0
#define MAC_SFS_SUPERFRAME_ORDER_POS    4

/* Beacon superframe spec bit positions, high byte */
#define MAC_SFS_FINAL_CAP_SLOT_POS      0
#define MAC_SFS_BATT_LIFE_EXT_POS       4
#define MAC_SFS_PAN_COORD_POS           6
#define MAC_SFS_ASSOC_PERMIT_POS        7


/* Frame type */
#define MAC_FRAME_TYPE_BEACON           0
#define MAC_FRAME_TYPE_DATA             1
#define MAC_FRAME_TYPE_ACK              2
#define MAC_FRAME_TYPE_COMMAND          3
#define MAC_FRAME_TYPE_MAX_VALID        MAC_FRAME_TYPE_COMMAND

/* Internal Frame Type for frame version bits (13,12) set to (1,0) */
#define MAC_FRAME_TYPE_INTERNAL_MAC_VERSION_E   8

/* Command frame identifiers */
#define MAC_ASSOC_REQ_FRAME             1
#define MAC_ASSOC_RSP_FRAME             2
#define MAC_DISASSOC_NOTIF_FRAME        3
#define MAC_DATA_REQ_FRAME              4
#define MAC_PAN_CONFLICT_FRAME          5
#define MAC_ORPHAN_NOTIF_FRAME          6
#define MAC_BEACON_REQ_FRAME            7
#define MAC_COORD_REALIGN_FRAME         8
#define MAC_GTS_REQ_FRAME               9
#define MAC_ENHANCED_BEACON_REQ_FRAME   10

/* Length of command frame payload (includes command ID byte) */
#define MAC_ZERO_DATA_PAYLOAD           0
#define MAC_ASSOC_REQ_PAYLOAD           2
#define MAC_ASSOC_RSP_PAYLOAD           4
#define MAC_DISASSOC_NOTIF_PAYLOAD      2
#define MAC_DATA_REQ_PAYLOAD            1
#define MAC_PAN_CONFLICT_PAYLOAD        1
#define MAC_ORPHAN_NOTIF_PAYLOAD        1
#define MAC_BEACON_REQ_PAYLOAD          1
#define MAC_COORD_REALIGN_PAYLOAD       9
#define MAC_GTS_REQ_PAYLOAD             2
#define MAC_ENHANCED_BEACON_REQ_PAYLOAD  (4 + MAC_A_MAX_BEACON_PAYLOAD_LENGTH)

/* Length of command frames (max header plus payload) */
#define MAC_ZERO_DATA_FRAME_LEN         (21 + MAC_ZERO_DATA_PAYLOAD)
#define MAC_ASSOC_REQ_FRAME_LEN         (23 + MAC_ASSOC_REQ_PAYLOAD)
#define MAC_ASSOC_RSP_FRAME_LEN         (23 + MAC_ASSOC_RSP_PAYLOAD)
#define MAC_DISASSOC_NOTIF_FRAME_LEN    (17 + MAC_DISASSOC_NOTIF_PAYLOAD)
#define MAC_DATA_REQ_FRAME_LEN          (23 + MAC_DATA_REQ_PAYLOAD)
#define MAC_PAN_CONFLICT_FRAME_LEN      (23 + MAC_PAN_CONFLICT_PAYLOAD)
#define MAC_ORPHAN_NOTIF_FRAME_LEN      (17 + MAC_ORPHAN_NOTIF_PAYLOAD)
#define MAC_BEACON_REQ_FRAME_LEN        (7 + MAC_BEACON_REQ_PAYLOAD)
#define MAC_COORD_REALIGN_FRAME_LEN     (23 + MAC_COORD_REALIGN_PAYLOAD)
#define MAC_GTS_REQ_FRAME_LEN           (7 + MAC_GTS_REQ_PAYLOAD)
#define MAC_ENHANCED_BEACON_REQ_FRAME_LEN  (7 + MAC_ENHANCED_BEACON_REQ_PAYLOAD)

/* Beacon frame base length (max header plus minimum payload) */
#define MAC_BEACON_FRAME_BASE_LEN       (13 + 4)

/* Maximum number of pending addresses in a beacon */
#define MAC_PEND_ADDR_MAX               7

/* Associate response command frame status values */
#define MAC_ASSOC_SUCCESS               0             /* association successful */
#define MAC_ASSOC_CAPACITY              1             /* PAN at capacity */
#define MAC_ASSOC_DENIED                2             /* PAN access denied */

/* Beacon order and superframe order maximum values */
#define MAC_BO_NON_BEACON               15
#define MAC_SO_NONE                     15

/* Broadcast PAN ID */
#define MAC_PAN_ID_BROADCAST            0xFFFF

/* Number of symbols per octet */
#define MAC_SYMBOLS_PER_OCTET           2

/* Maximum phy frame size in bytes */
#define MAC_A_MAX_PHY_PACKET_SIZE       127

/* Phy RX <--> TX turnaround time in symbols */
#define MAC_A_TURNAROUND_TIME           12

/* Number of backoffs forming a superframe slot when the superframe order is equal to 0 */
#define MAC_A_BASE_SLOT_DURATION        3

/* Number of backoffs forming a superframe when the superframe order is equal to 0 */
#define MAC_A_BASE_SUPERFRAME_DURATION  (MAC_A_BASE_SLOT_DURATION * MAC_A_NUM_SUPERFRAME_SLOTS)

/* Maximum number of bytes added by the MAC sublayer to the payload of its beacon frame */
#define MAC_A_MAX_BEACON_OVERHEAD       75

/* Maximum size in bytes of a beacon payload */
#define MAC_A_MAX_BEACON_PAYLOAD_LENGTH (MAC_A_MAX_PHY_PACKET_SIZE - MAC_A_MAX_BEACON_OVERHEAD)

/* The number of superframes a GTS descriptor exists for in the beacon frame */
#define MAC_A_GTS_DESC_PERSISTENCE_TIME 4

/* Maximum size in bytes of a MAC frame header */
#define MAC_A_MAX_FRAME_OVERHEAD        25

/* The number of consecutive lost beacons that will case the MAC to declare a sync loss */
#define MAC_A_MAX_LOST_BEACONS          4

/* The maximum number of bytes that can be transmitted in the MAC frame payload */
#define MAC_A_MAX_FRAME_SIZE            (MAC_A_MAX_PHY_PACKET_SIZE - MAC_A_MAX_FRAME_OVERHEAD)

/* The maximum frame size in bytes that can be followed by a short interframe spacing period */
#define MAC_A_MAX_SIFS_FRAME_SIZE       18

/* The minimum number of symbols forming the CAP */
#define MAC_A_MIN_CAP_LENGTH            440

/* The minimum number of symbols forming a long interframe spacing period */
#define MAC_A_MIN_LIFS_PERIOD           40

/* The minimum number of symbols forming a short interframe spacing period */
#define MAC_A_MIN_SIFS_PERIOD           12

/* The number of slots contained in any superframe */
#define MAC_A_NUM_SUPERFRAME_SLOTS      16

/* The number of symbols forming a basic CSMA-CA time period */
#define MAC_A_UNIT_BACKOFF_PERIOD       20

/* Maximum value for energy detect */
#define MAC_SPEC_ED_MAX                 0xFF

/* Threshold above receiver sensitivity for minimum energy detect in dBm (see 6.9.7) */
#define MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY    10


/* ----- values specific to 2450 MHz PHY ----- */

/* minimum receiver sensitivity in dBm (see 6.5.3.3) */
#define MAC_SPEC_MIN_RECEIVER_SENSITIVITY   -85

/* Length of preamble field in symbols */
#define MAC_SPEC_PREAMBLE_FIELD_LENGTH      8

/* Length of SFD field in symbols */
#define MAC_SPEC_SFD_FIELD_LENGTH           2

/* Microseconds in one symbol */
#define MAC_SPEC_USECS_PER_SYMBOL           16

/* Microseconds in one backoff period */
#define MAC_SPEC_USECS_PER_BACKOFF          (MAC_SPEC_USECS_PER_SYMBOL * MAC_A_UNIT_BACKOFF_PERIOD)

/* octets (or bytes) per symbol */
#define MAC_SPEC_OCTETS_PER_SYMBOL          2


/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* Length of GTS fields in a beacon, not including one byte for GTS specification */
#define MAC_GTS_FIELDS_LEN(gtsSpec)     ((uint8)((((gtsSpec) & 0x07) * 3) + (((gtsSpec) & 0x07) ? 1 : 0)))

/*
 *  Macros to decode FCF and sequence number.  For efficiency, the FCF is accessed as separate
 *  bytes instead of as a 16-bit word.  The first byte (+0) is the lowest significant byte.
 *  The second byte (+1) is the most signficant byte.  NOTE!  Any change to the specification
 *  involving FCF would require updating these macros.
 *
 *  Macro parameter is pointer to start of frame.
 */

#define MAC_FRAME_TYPE(p)       ((p)[MAC_FCF_OFFSET+0] & 0x07)
#define MAC_SEC_ENABLED(p)      ((p)[MAC_FCF_OFFSET+0] & 0x08)  /* note: value is *non-zero* if true */
#define MAC_FRAME_PENDING(p)    ((p)[MAC_FCF_OFFSET+0] & 0x10)  /* note: value is *non-zero* if true */
#define MAC_ACK_REQUEST(p)      ((p)[MAC_FCF_OFFSET+0] & 0x20)  /* note: value is *non-zero* if true */
#define MAC_INTRA_PAN(p)        ((p)[MAC_FCF_OFFSET+0] & 0x40)  /* note: value is *non-zero* if true */
#define MAC_DEST_ADDR_MODE(p)   (((p)[MAC_FCF_OFFSET+1] >> 2) & 0x3)
#define MAC_FRAME_VERSION(p)    (((p)[MAC_FCF_OFFSET+1] >> 4) & 0x3)
#define MAC_SRC_ADDR_MODE(p)    (((p)[MAC_FCF_OFFSET+1] >> 6) & 0x3)
#define MAC_SEQ_NUMBER(p)       ((p)[MAC_SEQ_NUM_OFFSET])


/**************************************************************************************************
*/

#endif /* MAC_SPEC_H */
