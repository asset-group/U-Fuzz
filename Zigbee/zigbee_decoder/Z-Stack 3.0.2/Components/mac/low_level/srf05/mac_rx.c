/**************************************************************************************************
  Filename:       mac_rx.c
  Revised:        $Date: 2014-03-13 10:50:10 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37663 $

  Description:    Describe the purpose and contents of the file.


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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_defs.h"
#include "hal_types.h"

/* OSAL */
#include "OSAL.h"

/* high-level */
#include "mac_high_level.h"
#include "mac_spec.h"
#include "mac_pib.h"

/* MAC security */
#include "mac_security.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_radio.h"

/* target specific */
#include "mac_radio_defs.h"
#include "mac_autopend.h"

/* debug */
#include "mac_assert.h"


typedef struct macTimer_s
{
  struct macTimer_s     *pNext;                     /* next timer in queue */
  int32                 backoff;                    /* timer expiration count */
  void                  (*pFunc)(uint8 parameter);  /* timer callback function */
  uint8                 parameter;                  /* callback function parameter */
} macTimer_t;

extern macTimer_t macTxAckIsrTimer;
extern void macTimerCancel(macTimer_t *pTimer);


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT   16   /* adjustable to tune performance */

/* receive FIFO bytes needed to start a valid receive (see function rxStartIsr for details) */
#define RX_THRESHOLD_START_LEN    (MAC_PHY_PHR_LEN        +  \
                                   MAC_FCF_FIELD_LEN      +  \
                                   MAC_SEQ_NUM_FIELD_LEN  +  \
                                   MAC_FCS_FIELD_LEN)

/* maximum size of addressing fields (note: command frame identifier processed as part of address) */
#define MAX_ADDR_FIELDS_LEN  ((MAC_EXT_ADDR_FIELD_LEN + MAC_PAN_ID_FIELD_LEN) * 2)

/* addressing mode reserved value */
#define ADDR_MODE_RESERVERED  1

/* length of command frame identifier */
#define CMD_FRAME_ID_LEN      1

/* packet size mask is equal to the maximum value */
#define PHY_PACKET_SIZE_MASK  0x7F

/* value for promiscuous off, must not conflict with other mode variants from separate include files */
#define PROMISCUOUS_MODE_OFF  0x00

/* bit of proprietary FCS format that indicates if the CRC is OK */
#define PROPRIETARY_FCS_CRC_OK_BIT  0x80

/* dummy length value for unused entry in lookup table */
#define DUMMY_LEN   0xBE

/* value for rxThresholdIntState */
#define RX_THRESHOLD_INT_STATE_INACTIVE   0
#define RX_THRESHOLD_INT_STATE_ACTIVE     1
#define RX_THRESHOLD_INT_STATE_RESET      2


/* ------------------------------------------------------------------------------------------------
 *                                             Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MEM_ALLOC(x)   macDataRxMemAlloc(x)
#define MEM_FREE(x)    macDataRxMemFree((uint8 **)x)

/*
 *  Macro for encoding frame control information into internal flags format.
 *  Parameter is pointer to the frame.  NOTE!  If either the internal frame
 *  format *or* the specification changes, this macro will need to be modified.
 */
#define INTERNAL_FCF_FLAGS(p)  ((((p)[1] >> 4) & 0x03) | ((p)[0] & 0x78))

/*
 *  The radio replaces the actual FCS with different information.  This proprietary FCS is
 *  the same length as the original and includes:
 *    1) the RSSI value
 *    2) the average correlation value (used for LQI)
 *    3) a CRC passed bit
 *
 *  These macros decode the proprietary FCS.  The macro parameter is a pointer to the two byte FCS.
 */
#define PROPRIETARY_FCS_RSSI(p)                 ((int8)((p)[0]))
#define PROPRIETARY_FCS_CRC_OK(p)               ((p)[1] & PROPRIETARY_FCS_CRC_OK_BIT)
#define PROPRIETARY_FCS_CORRELATION_VALUE(p)    ((p)[1] & ~PROPRIETARY_FCS_CRC_OK_BIT)

/*
 *  Macros for security control field.
 */
#define SECURITY_LEVEL(s)                       (s & 0x07)
#define KEY_IDENTIFIER_MODE(s)                  ((s & 0x18) >> 3)
#define SECURITY_CONTROL_RESERVED(s)            ((s & 0xE0) >> 5)

/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macRxActive;
uint8 macRxFilter;
uint8 macRxOutgoingAckFlag;


/* ------------------------------------------------------------------------------------------------
 *                                       Local Constants
 * ------------------------------------------------------------------------------------------------
 */
static const uint8 CODE macRxAddrLen[] =
{
  0,                                                /* no address */
  DUMMY_LEN,                                        /* reserved */
  MAC_PAN_ID_FIELD_LEN + MAC_SHORT_ADDR_FIELD_LEN,  /* short address + pan id */
  MAC_PAN_ID_FIELD_LEN + MAC_EXT_ADDR_FIELD_LEN     /* extended address + pan id */
};


/* ------------------------------------------------------------------------------------------------
 *                                       Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void rxHaltCleanupFinalStep(void);

static void rxStartIsr(void);
static void rxAddrIsr(void);

#ifdef FEATURE_MAC_SECURITY
  static void rxSecurityHdrIsr(void);
#endif

static void rxPayloadIsr(void);
static void rxDiscardIsr(void);
static void rxFcsIsr(void);

static void rxPrepPayload(void);
static void rxDiscardFrame(void);
static void rxDone(void);
static void rxPostRxUpdates(void);


/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static void    (* pFuncRxState)(void);
static macRx_t  * pRxBuf;

static uint8  rxBuf[MAC_PHY_PHR_LEN + MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN];
static uint8  rxUnreadLen;
static uint8  rxNextLen;
static uint8  rxPayloadLen;
static uint8  rxFilter;
static uint8  rxPromiscuousMode;
static uint8  rxIsrActiveFlag;
static uint8  rxResetFlag;
static uint8  rxFifoOverflowCount;


/**************************************************************************************************
 * @fn          macRxInit
 *
 * @brief       Initialize receive variable states.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxInit(void)
{
  macRxFilter          = RX_FILTER_OFF;
  rxPromiscuousMode    = PROMISCUOUS_MODE_OFF;
  pRxBuf               = NULL; /* required for macRxReset() to function correctly */
  macRxActive          = MAC_RX_ACTIVE_NO_ACTIVITY;
  pFuncRxState         = &rxStartIsr;
  macRxOutgoingAckFlag = 0;
  rxIsrActiveFlag      = 0;
  rxResetFlag          = 0;
  rxFifoOverflowCount  = 0;
}


/**************************************************************************************************
 * @fn          macRxRadioPowerUpInit
 *
 * @brief       Initialization for after radio first powers up.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxRadioPowerUpInit(void)
{
  /* set threshold at initial value */
  MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);

  /* clear any accidental threshold interrupt that happened as part of power up sequence */
  MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  /* enable threshold interrupts */
  MAC_RADIO_ENABLE_RX_THRESHOLD_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macRxTxReset
 *
 * @brief       Reset the receive state.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxTxReset(void)
{
  /* forces receiver off, cleans up by calling macRxHaltCleanup() and macTxHaltCleanup() */
  macRxHardDisable();

  /*
   *   Note : transmit does not require any reset logic
   *          beyond what macRxHardDisable() provides.
   */

  /* restore deault filter mode to off */
  macRxFilter = RX_FILTER_OFF;

  /* return promiscuous mode to default off state */
  macRxPromiscuousMode(MAC_PROMISCUOUS_MODE_OFF);
}


/**************************************************************************************************
 * @fn          macRxHaltCleanup
 *
 * @brief       Cleanup up the receive logic after receiver is forced off.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxHaltCleanup(void)
{
  rxResetFlag = 1;
  if (!rxIsrActiveFlag)
  {
    rxHaltCleanupFinalStep();
    rxResetFlag = 0;
  }
}


/*=================================================================================================
 * @fn          rxHaltCleanupFinalStep
 *
 * @brief       Required cleanup if receiver is halted in the middle of a receive.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxHaltCleanupFinalStep(void)
{
  /* cancel any upcoming ACK transmit complete callback */
  MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();

  /* set start of frame threshold */
  MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);

  /* flush the receive FIFO */
  MAC_RADIO_FLUSH_RX_FIFO();

  /* clear any receive interrupt that happened to squeak through */
  MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  /* if data buffer has been allocated, free it */
  if (pRxBuf != NULL)
  {
    MEM_FREE((uint8 **)&pRxBuf);
  }
  /* MEM_FREE() sets parameter to NULL. */

  pFuncRxState = &rxStartIsr;

  /* if receive was active, perform the post receive updates */
  if (macRxActive || macRxOutgoingAckFlag)
  {
    macRxActive = MAC_RX_ACTIVE_NO_ACTIVITY;
    macRxOutgoingAckFlag = 0;

    rxPostRxUpdates();
  }
}


/**************************************************************************************************
 * @fn          macRxThresholdIsr
 *
 * @brief       Interrupt service routine called when bytes in FIFO reach threshold value.
 *              It implements a state machine for receiving a packet.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxThresholdIsr(void)
{
  /* if currently reseting, do not execute receive ISR logic */
  if (rxResetFlag)
  {
    return;
  }

  /*
   *  Call the function that handles the current receive state.
   *  A flag is set for the duration of the call to indicate
   *  the ISR is executing.  This is necessary for the reset
   *  logic so it does not perform a reset in the middle of
   *  executing the ISR.
   */
  rxIsrActiveFlag = 1;
  (*pFuncRxState)();
  rxIsrActiveFlag = 0;

  /* if a reset occurred during the ISR, peform cleanup here */
  if (rxResetFlag)
  {
    rxHaltCleanupFinalStep();
    rxResetFlag = 0;
  }
}


/*=================================================================================================
 * @fn          rxStartIsr
 *
 * @brief       First ISR state for receiving a packet - compute packet length, allocate
 *              buffer, initialize buffer.  Acknowledgements are handled immediately without
 *              allocating a buffer.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxStartIsr(void)
{
  uint8  addrLen;
  uint8  ackWithPending;
  uint8  dstAddrMode;
  uint8  srcAddrMode;
  uint8  mhrLen = 0;
  
  MAC_ASSERT(!macRxActive); /* receive on top of receive */

  /* indicate rx is active */
  macRxActive = MAC_RX_ACTIVE_STARTED;

  /*
   *  For bullet proof functionality, need to see if the receiver was just turned off.
   *  The logic to request turning off the receiver, disables interrupts and then checks
   *  the value of macRxActive.  If it is TRUE, the receiver will not be turned off.
   *
   *  There is a small hole though.  It's possible to attempt turning off the receiver
   *  in the window from when the receive interrupt fires and the point where macRxActive
   *  is set to TRUE.  To plug this hole, the on/off status must be tested *after*
   *  macRxActive has been set.  If the receiver is off at this point, there is nothing
   *  in the RX fifo and the receive is simply aborted.
   *
   *  Also, there are some considerations in case a hard disable just happened.  Usually,
   *  the receiver will just be off at this point after a hard disable.  The check described
   *  above will account for this case too.  However, if a hard disable were immediately
   *  followed by an enable, the receiver would be on.  To catch this case, the receive
   *  FIFO is also tested to see if it is empty.  Recovery is identical to the other cases.
   */
  if (!macRxOnFlag || MAC_RADIO_RX_FIFO_IS_EMPTY())
  {
    /* reset active flag */
    macRxActive = MAC_RX_ACTIVE_NO_ACTIVITY;

    /*
     *  To be absolutely bulletproof, must make sure no transmit queue'ed up during
     *  the tiny, tiny window when macRxActive was not zero.
     */
    rxPostRxUpdates();

    /* return immediately from here */
    return;
  }

  /*
   *  If interrupts are held off for too long it's possible the previous "transmit done"
   *  callback is pending.  If this is the case, it needs to be completed before
   *  continuing with the receive logic.
   */
  MAC_RADIO_FORCE_TX_DONE_IF_PENDING();

  /*
   *  It's possible receive logic is still waiting for confirmation of an ACK that went out
   *  for the previous receive.  This is OK but the callback needs to be canceled at this point.
   *  That callback execute receive cleanup logic that will run at the completion
   *  of *this* receive.  Also, it is important the flag for the outgoing ACK to be cleared.
   */
  MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();
  macRxOutgoingAckFlag = 0;

  /*
   *  Make a module-local copy of macRxFilter.  This prevents the selected
   *  filter from changing in the middle of a receive.
   */
  rxFilter = macRxFilter;

  /*-------------------------------------------------------------------------------
   *  Read initial frame information from FIFO.
   *
   *   This code is not triggered until the following are in the RX FIFO:
   *     frame length          - one byte containing length of MAC frame (excludes this field)
   *     frame control field   - two bytes defining frame type, addressing fields, control flags
   *     sequence number       - one byte unique sequence identifier
   *     additional two bytes  - these bytes are available in case the received frame is an ACK,
   *                             if so, the frame can be verified and responded to immediately,
   *                             if not an ACK, these bytes will be processed normally
   */

  /* read frame length, frame control field, and sequence number from FIFO */
  MAC_RADIO_READ_RX_FIFO(rxBuf, MAC_PHY_PHR_LEN + MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN);

  /* bytes to read from FIFO equals frame length minus length of MHR fields just read from FIFO */
  rxUnreadLen = (rxBuf[0] & PHY_PACKET_SIZE_MASK) - MAC_FCF_FIELD_LEN - MAC_SEQ_NUM_FIELD_LEN;

  /*
   *  Workaround for chip bug #1547.  The receive buffer can sometimes be corrupted by hardware.
   *  This usually occurs under heavy traffic.  If a corrupted receive buffer is detected
   *  the entire receive buffer is flushed.
   */
  if ((rxUnreadLen > (MAC_A_MAX_PHY_PACKET_SIZE - MAC_FCF_FIELD_LEN - MAC_SEQ_NUM_FIELD_LEN)) ||
      (MAC_FRAME_TYPE(&rxBuf[1]) > MAC_FRAME_TYPE_MAX_VALID))
  {
    MAC_RADIO_FLUSH_RX_FIFO();
    rxDone();
    return;
  }


  /*-------------------------------------------------------------------------------
   *  Process ACKs.
   *
   *  If this frame is an ACK, process it immediately and exit from here.
   *  If this frame is not an ACK and transmit is listening for an ACK, let
   *  the transmit logic know an non-ACK was received so transmit can complete.
   *
   *  In promiscuous mode ACKs are treated like any other frame.
   */
  if ((MAC_FRAME_TYPE(&rxBuf[1]) == MAC_FRAME_TYPE_ACK) && (rxPromiscuousMode == PROMISCUOUS_MODE_OFF))
  {
    halIntState_t  s;
    uint8 fcsBuf[MAC_FCF_FIELD_LEN];
    /*
     *  There are guaranteed to be two unread bytes in the FIFO.  By defintion, for ACK frames
     *  these two bytes will be the FCS.
     */

    /* read FCS from FIFO (threshold set so bytes are guaranteed to be there) */
    MAC_RADIO_READ_RX_FIFO(fcsBuf, MAC_FCS_FIELD_LEN);

    /*
     *  This critical section ensures that the ACK timeout won't be triggered in the
     *  middle of receiving the ACK frame.
     */
    HAL_ENTER_CRITICAL_SECTION(s);

    /* see if transmit is listening for an ACK */
    if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
    {
      MAC_ASSERT(pMacDataTx != NULL); /* transmit buffer must be present */

      /* record link quality metrics for the receive ACK */
      {
        int8 rssiDbm;
        uint8 corr;

        rssiDbm = PROPRIETARY_FCS_RSSI(fcsBuf) + MAC_RADIO_RSSI_OFFSET;
        MAC_RADIO_RSSI_LNA_OFFSET(rssiDbm);
        corr = PROPRIETARY_FCS_CORRELATION_VALUE(fcsBuf);

        pMacDataTx->internal.mpduLinkQuality = macRadioComputeLQI(rssiDbm, corr);
        pMacDataTx->internal.correlation = corr;
        pMacDataTx->internal.rssi= rssiDbm;
      }

      /*
       *  It's okay if the ACK timeout is triggered here. The callbacks for ACK received
       *  or ACK not received will check "macTxActive" flag before taking any actions.
       */
      HAL_EXIT_CRITICAL_SECTION(s);

      /*
       *  An ACK was received so transmit logic needs to know.  If the FCS failed,
       *  the transmit logic still needs to know.  In that case, treat the frame
       *  as a non-ACK to complete the active transmit.
       */
      if (PROPRIETARY_FCS_CRC_OK(fcsBuf))
      {
        /* call transmit logic to indicate ACK was received */
        macTxAckReceivedCallback(MAC_SEQ_NUMBER(&rxBuf[1]), MAC_FRAME_PENDING(&rxBuf[1]));
      }
      else
      {
        macTxAckNotReceivedCallback();
      }
    }
    else
    {
      HAL_EXIT_CRITICAL_SECTION(s);
    }

    /* receive is done, exit from here */
    rxDone();
    return;
  }
  else if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
  {
    macTxAckNotReceivedCallback();
  }

  /*-------------------------------------------------------------------------------
   *  Apply filtering.
   *
   *  For efficiency, see if filtering is even 'on' before processing.  Also test
   *  to make sure promiscuous mode is disabled.  If promiscuous mode is enabled,
   *  do not apply filtering.
   */
  if ((rxFilter != RX_FILTER_OFF) && !rxPromiscuousMode)
  {
    if (/* filter all frames */
         (rxFilter == RX_FILTER_ALL) ||

         /* filter non-beacon frames */
         ((rxFilter == RX_FILTER_NON_BEACON_FRAMES) &&
          (MAC_FRAME_TYPE(&rxBuf[1]) != MAC_FRAME_TYPE_BEACON)) ||

         /* filter non-command frames */
         ((rxFilter == RX_FILTER_NON_COMMAND_FRAMES) &&
          ((MAC_FRAME_TYPE(&rxBuf[1]) != MAC_FRAME_TYPE_COMMAND))))
    {
      /* discard rest of frame */
      rxDiscardFrame();
      return;
    }
  }

  /*-------------------------------------------------------------------------------
   *  Compute length of addressing fields.  Compute payload length.
   */

  /* decode addressing modes */
  dstAddrMode = MAC_DEST_ADDR_MODE(&rxBuf[1]);
  srcAddrMode = MAC_SRC_ADDR_MODE(&rxBuf[1]);

  /*
  *  Workaround for chip bug #1547.  The receive buffer can sometimes be corrupted by hardware.
   *  This usually occurs under heavy traffic.  If a corrupted receive buffer is detected
   *  the entire receive buffer is flushed.
   */
  if (macChipVersion == REV_A)
  {
    if ((srcAddrMode == ADDR_MODE_RESERVERED) || (dstAddrMode == ADDR_MODE_RESERVERED))
    {
      MAC_RADIO_FLUSH_RX_FIFO();
      rxDone();
      return;
    }
  }

  /*
   *  Compute the addressing field length.  A lookup table based on addressing
   *  mode is used for efficiency.  If the source address is present and the
   *  frame is intra-PAN, the PAN Id is not repeated.  In this case, the address
   *  length is adjusted to match the smaller length.
   */
  addrLen = macRxAddrLen[dstAddrMode] + macRxAddrLen[srcAddrMode];
  if ((srcAddrMode != SADDR_MODE_NONE) && MAC_INTRA_PAN(&rxBuf[1]))
  {
    addrLen -= MAC_PAN_ID_FIELD_LEN;
  }

  /*
   *  If there are not enough unread bytes to include the computed address
   *  plus FCS field, the frame is corrupted and must be discarded.
   */
  if ((addrLen + MAC_FCS_FIELD_LEN) > rxUnreadLen)
  {
    /* discard frame and exit */
    rxDiscardFrame();
    return;
  }

  /* aux security header plus payload length is equal to unread bytes minus
   * address length, minus the FCS
   */
  rxPayloadLen = rxUnreadLen - addrLen - MAC_FCS_FIELD_LEN;

  /*-------------------------------------------------------------------------------
   *  Allocate memory for the incoming frame.
   */
  if (MAC_SEC_ENABLED(&rxBuf[1]))
  {
    /* increase the allocation size of MAC header for security */
    mhrLen = MAC_MHR_LEN;
  }

  pRxBuf = (macRx_t *) MEM_ALLOC(sizeof(macRx_t) + mhrLen + rxPayloadLen);
  if (pRxBuf == NULL)
  {
    /* Cancel the outgoing TX ACK */
    MAC_RADIO_CANCEL_TX_ACK();

    /* buffer allocation failed, discard the frame and exit*/
    rxDiscardFrame();
    return;
  }

  /*-------------------------------------------------------------------------------
   *  Set up to process ACK request.  Do not ACK if in promiscuous mode.
   */
  ackWithPending = 0;
  if (!rxPromiscuousMode)
  {
    macRxOutgoingAckFlag = MAC_ACK_REQUEST(&rxBuf[1]);
  }

  /*-------------------------------------------------------------------------------
   *  Process any ACK request.
   */
  if (macRxOutgoingAckFlag)
  {
    halIntState_t  s;

    /*
     *  This critical section ensures that the callback ISR is initiated within time
     *  to guarantee correlation with the strobe.
     */
    HAL_ENTER_CRITICAL_SECTION(s);

    /* Do not ack data packet with pending more data */
    if( MAC_FRAME_TYPE(&rxBuf[1]) == MAC_FRAME_TYPE_COMMAND )
    {
      if( macRxCheckMACPendingCallback())
      {
        /* Check is any mac data pending for end devices */
        ackWithPending = MAC_RX_FLAG_ACK_PENDING;
      }
      else
      {
        if( macSrcMatchIsEnabled )
        {
          /* When autopend is enabled, check if allpending is set to true */
          if( MAC_SrcMatchCheckAllPending() == MAC_AUTOACK_PENDING_ALL_ON )
          {
            ackWithPending = MAC_RX_FLAG_ACK_PENDING;
          }
        }
        else
        {
          /* When autopend is disabled, check the application pending callback */
          if( macRxCheckPendingCallback() )
          {
            ackWithPending = MAC_RX_FLAG_ACK_PENDING;
          }
        }
      }
    }

    if( ackWithPending == MAC_RX_FLAG_ACK_PENDING )
    {
      MAC_RADIO_TX_ACK_PEND();
    }
    else
    {
      MAC_RADIO_TX_ACK();
    }


    /* request a callback to macRxAckTxDoneCallback() when the ACK transmit has finished */
    MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK();
    HAL_EXIT_CRITICAL_SECTION(s);
  }

 /*-------------------------------------------------------------------------------
  *  Populate the receive buffer going up to high-level.
  */

  /* configure the payload buffer
   * save MAC header pointer regardless of security status.
   */
  pRxBuf->mhr.p   = pRxBuf->msdu.p   = (uint8 *) (pRxBuf + 1);
  pRxBuf->mhr.len = pRxBuf->msdu.len =  rxPayloadLen;

  if (MAC_SEC_ENABLED(&rxBuf[1]))
  {
    /* Copy FCF and sequence number to RX buffer */
    pRxBuf->mhr.len = MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN;
    osal_memcpy(pRxBuf->mhr.p, &rxBuf[1], pRxBuf->mhr.len);
    pRxBuf->mhr.p += pRxBuf->mhr.len;
  }

  /* set internal values */
  pRxBuf->mac.srcAddr.addrMode  = srcAddrMode;
  pRxBuf->mac.dstAddr.addrMode  = dstAddrMode;
  pRxBuf->mac.timestamp         = MAC_RADIO_BACKOFF_CAPTURE();
  pRxBuf->mac.timestamp2        = MAC_RADIO_TIMER_CAPTURE();
    
  /* Special Case for Enhanced Beacon Request which has a different
   * frame version
   */
#ifdef FEATURE_ENHANCED_BEACON
  if( MAC_FRAME_VERSION(&rxBuf[1]) == 2 )
  {
    pRxBuf->internal.frameType  = MAC_FRAME_TYPE_INTERNAL_MAC_VERSION_E | \
                                  MAC_FRAME_TYPE(&rxBuf[1]);
  }
  else
#endif
  {
    pRxBuf->internal.frameType  = MAC_FRAME_TYPE(&rxBuf[1]);
  }

  pRxBuf->mac.dsn               = MAC_SEQ_NUMBER(&rxBuf[1]);
  pRxBuf->internal.flags        = INTERNAL_FCF_FLAGS(&rxBuf[1]) | ackWithPending;

  /*-------------------------------------------------------------------------------
   *  If the processing the addressing fields does not require more bytes from
   *  the FIFO go directly address processing function.  Otherwise, configure
   *  interrupt to jump there once bytes are received.
   */
  if (addrLen == 0)
  {
    /* no addressing fields to read, prepare for payload interrupts */
    pFuncRxState = &rxPayloadIsr;
    rxPrepPayload();
  }
  else
  {
    /* need to read and process addressing fields, prepare for address interrupt */
    rxNextLen = addrLen;
    if (MAC_SEC_ENABLED(&rxBuf[1]))
    {
      /* When security is enabled, read off security control field as well */
      MAC_RADIO_SET_RX_THRESHOLD(rxNextLen + MAC_SEC_CONTROL_FIELD_LEN);
    }
    else
    {
      MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
    }
    pFuncRxState = &rxAddrIsr;
  }
}


/*=================================================================================================
 * @fn          rxAddrIsr
 *
 * @brief       Receive ISR state for decoding address.  Reads and stores the address information
 *              from the incoming packet.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxAddrIsr(void)
{
  uint8 buf[MAX_ADDR_FIELDS_LEN];
  uint8 dstAddrMode;
  uint8 srcAddrMode;
#ifdef FEATURE_MAC_SECURITY
  uint8 securityControl;
#endif /* FEATURE_MAC_SECURITY */
  uint8  * p;

  MAC_ASSERT(rxNextLen != 0); /* logic assumes at least one address byte in buffer */

  /*  read out address fields into local buffer in one shot */
  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);

  /* set pointer to buffer with addressing fields */
  p = buf;

  /* destination address */
  dstAddrMode = MAC_DEST_ADDR_MODE(&rxBuf[1]);
  if (dstAddrMode != SADDR_MODE_NONE)
  {
    pRxBuf->mac.srcPanId = pRxBuf->mac.dstPanId = BUILD_UINT16(p[0], p[1]);
    p += MAC_PAN_ID_FIELD_LEN;
    if (dstAddrMode == SADDR_MODE_EXT)
    {
      sAddrExtCpy(pRxBuf->mac.dstAddr.addr.extAddr, p);
      p += MAC_EXT_ADDR_FIELD_LEN;
    }
    else
    {
      pRxBuf->mac.dstAddr.addr.shortAddr = BUILD_UINT16(p[0], p[1]);
      p += MAC_SHORT_ADDR_FIELD_LEN;
    }
  }

  /* sources address */
  srcAddrMode = MAC_SRC_ADDR_MODE(&rxBuf[1]);
  if (srcAddrMode != SADDR_MODE_NONE)
  {
    if (!(pRxBuf->internal.flags & MAC_RX_FLAG_INTRA_PAN))
    {
      pRxBuf->mac.srcPanId = BUILD_UINT16(p[0], p[1]);
      p += MAC_PAN_ID_FIELD_LEN;
    }
    if (srcAddrMode == SADDR_MODE_EXT)
    {
      sAddrExtCpy(pRxBuf->mac.srcAddr.addr.extAddr, p);
    }
    else
    {
      pRxBuf->mac.srcAddr.addr.shortAddr = BUILD_UINT16(p[0], p[1]);
    }
  }

#ifdef FEATURE_MAC_SECURITY
  if (MAC_SEC_ENABLED(&rxBuf[1]))
  {
    uint8 keyIdMode;

    if (MAC_FRAME_VERSION(&rxBuf[1]) == 0)
    {
      /* MAC_UNSUPPORTED_LEGACY - Cancel the outgoing TX ACK.
       * It may be too late but we have to try.
       */
      MAC_RADIO_CANCEL_TX_ACK();

      /* clean up after unsupported security legacy */
      macRxHaltCleanup();
      return;
    }

    /* Copy addressing fields to RX buffer */
    osal_memcpy(pRxBuf->mhr.p, buf, rxNextLen);
    pRxBuf->mhr.p   += rxNextLen;
    pRxBuf->mhr.len += rxNextLen;

    /*-------------------------------------------------------------------------------
     *  Prepare for auxiliary security header interrupts.
     */

    /* read out security control field from FIFO (threshold set so bytes are guaranteed to be there) */
    MAC_RADIO_READ_RX_FIFO(&securityControl, MAC_SEC_CONTROL_FIELD_LEN);

    /* Copy security fields to MHR buffer */
    *pRxBuf->mhr.p   = securityControl;
    pRxBuf->mhr.p   += MAC_SEC_CONTROL_FIELD_LEN;
    pRxBuf->mhr.len += MAC_SEC_CONTROL_FIELD_LEN;

    /* store security level and key ID mode */
    pRxBuf->sec.securityLevel = SECURITY_LEVEL(securityControl);
    pRxBuf->sec.keyIdMode = keyIdMode = KEY_IDENTIFIER_MODE(securityControl);

    /* Corrupted RX frame, should never occur. */
    if ((keyIdMode > MAC_KEY_ID_MODE_8)
    /* Get the next RX length according to AuxLen table minus security control field.
     * The security control length is counted already.
     */
    || ((macKeySourceLen[keyIdMode] + MAC_FRAME_COUNTER_LEN) >= rxPayloadLen)
    /* Security Enabled subfield is one, but the Security Level in the header is zero:
     * MAC_UNSUPPORTED_SECURITY - Cancel the outgoing TX ACK.
     */
    || (pRxBuf->sec.securityLevel == MAC_SEC_LEVEL_NONE))
    {
      /* It may be too late but we have to try. */
      MAC_RADIO_CANCEL_TX_ACK();

      /* clean up after unsupported security or corrupted RX frame. */
      macRxHaltCleanup();
      return;
    }

    /* get the next RX length according to AuxLen table minus security control field.
     * The sceurity control length is counted already.
     */
    rxNextLen = macKeySourceLen[keyIdMode] + MAC_FRAME_COUNTER_LEN;
    MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
    pFuncRxState = &rxSecurityHdrIsr;
  }
  else
#endif /* FEATURE_MAC_SECURITY */
  {
    /* clear security level */
    pRxBuf->sec.securityLevel = MAC_SEC_LEVEL_NONE;

    /*-------------------------------------------------------------------------------
     *  Prepare for payload interrupts.
     */
    pFuncRxState = &rxPayloadIsr;
    rxPrepPayload();
  }
}


#ifdef FEATURE_MAC_SECURITY
/*=================================================================================================
 * @fn          rxSecurityHdrIsr
 *
 * @brief       Receive ISR state for reading out and storing the auxiliary security header.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxSecurityHdrIsr(void)
{
  uint8 buf[MAC_FRAME_COUNTER_LEN + MAC_KEY_ID_8_LEN];

  /* read out frame counter and key ID */
  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);

  /* Incoming frame counter */
  pRxBuf->frameCounter = BUILD_UINT32(buf[0], buf[1], buf[2], buf[3]);
  if (rxNextLen - MAC_FRAME_COUNTER_LEN > 0)
  {
    /* Explicit mode */
    osal_memcpy(pRxBuf->sec.keySource, &buf[MAC_FRAME_COUNTER_LEN], rxNextLen - MAC_FRAME_COUNTER_LEN - 1);
    pRxBuf->sec.keyIndex = buf[rxNextLen - MAC_KEY_INDEX_LEN];
  }

  /* Copy security fields to RX buffer */
  osal_memcpy(pRxBuf->mhr.p, buf, rxNextLen);
  pRxBuf->mhr.p   += rxNextLen;
  pRxBuf->mhr.len += rxNextLen;

  /* Update payload pointer and payload length. The rxPayloadLen includes security header length
   * and SCF byte. The security header and SCF length must be deducted from the rxPayloadLen.
   */
  rxPayloadLen    -= (rxNextLen + MAC_SEC_CONTROL_FIELD_LEN);
  pRxBuf->msdu.len = rxPayloadLen;
  pRxBuf->mhr.len += rxPayloadLen;

  /*-------------------------------------------------------------------------------
   *  Prepare for payload interrupts.
   */
  pFuncRxState = &rxPayloadIsr;
  rxPrepPayload();
}
#endif /* FEATURE_MAC_SECURITY */


/*=================================================================================================
 * @fn          rxPrepPayload
 *
 * @brief       Common code to prepare for the payload ISR.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPrepPayload(void)
{
  if (rxPayloadLen == 0)
  {
    MAC_RADIO_SET_RX_THRESHOLD(MAC_FCS_FIELD_LEN);
    pFuncRxState = &rxFcsIsr;
  }
  else
  {
    rxNextLen = MIN(rxPayloadLen, MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT);
    MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
  }
}


/*=================================================================================================
 * @fn          rxPayloadIsr
 *
 * @brief       Receive ISR state for reading out and storing the packet payload.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPayloadIsr(void)
{
  MAC_RADIO_READ_RX_FIFO(pRxBuf->mhr.p, rxNextLen);
  pRxBuf->mhr.p += rxNextLen;

  rxPayloadLen -= rxNextLen;

  rxPrepPayload();
}


/*=================================================================================================
 * @fn          rxFcsIsr
 *
 * @brief       Receive ISR state for handling the FCS.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxFcsIsr(void)
{
  uint8 crcOK;
  uint8 ackWithPending = 0;

  /* read FCS, rxBuf is now available storage */
  MAC_RADIO_READ_RX_FIFO(rxBuf, MAC_FCS_FIELD_LEN);

  /*
   *  The FCS has actually been replaced within the radio by a proprietary version of the FCS.
   *  This proprietary FCS is two bytes (same length as the real FCS) and contains:
   *    1) the RSSI value
   *    2) the average correlation value (used for LQI)
   *    3) a CRC passed bit
   */

  /* save the "CRC-is-OK" status */
  crcOK = PROPRIETARY_FCS_CRC_OK(rxBuf);

  /*
   *  See if the frame should be passed up to high-level MAC.  If the CRC is OK, the
   *  the frame is always passed up.  Frames with a bad CRC are also passed up *if*
   *  a special variant of promiscuous mode is active.
   */
  if (crcOK || (rxPromiscuousMode == MAC_PROMISCUOUS_MODE_WITH_BAD_CRC))
  {
    int8 rssiDbm;
    uint8 corr;

#ifdef FEATURE_SYSTEM_STATS
    /* Increment diagnostic CRC success counter */
    macLowLevelDiags( MAC_DIAGS_RX_CRC_PASS );
#endif /* FEATURE_SYSTEM_STATS */

    /*
     *  As power saving optimization, set state variable to indicate physical receive
     *  has completed and then request turning of the receiver.  This means the receiver
     *  can be off (if other conditions permit) during execution of the callback function.
     *
     *  The receiver will be requested to turn off once again at the end of the receive
     *  logic.  There is no harm in doing this.
     */
    macRxActive = MAC_RX_ACTIVE_DONE;
    macRxOffRequest();

    /* decode RSSI and correlation values */
    rssiDbm = PROPRIETARY_FCS_RSSI(rxBuf) + MAC_RADIO_RSSI_OFFSET;
    MAC_RADIO_RSSI_LNA_OFFSET(rssiDbm);
    corr = PROPRIETARY_FCS_CORRELATION_VALUE(rxBuf);

    /* record parameters that get passed up to high-level */
    pRxBuf->mac.mpduLinkQuality = macRadioComputeLQI(rssiDbm, corr);
    pRxBuf->mac.rssi = rssiDbm;
    pRxBuf->mac.correlation = corr;

    /* set the MSDU pointer to point at start of data */
    pRxBuf->mhr.p   = (uint8 *) (pRxBuf + 1);
    pRxBuf->msdu.p += (pRxBuf->mhr.len - pRxBuf->msdu.len);

    if ((pRxBuf->internal.flags & MAC_RX_FLAG_ACK_PENDING) && (*pRxBuf->msdu.p != MAC_DATA_REQ_FRAME))
    {
      /* For non-data request commands, cancel the pending bit in the ACK. */
      MAC_RADIO_TX_ACK();
    }

    /* Read the source matching result back */
    if( macSrcMatchIsEnabled && MAC_RADIO_SRC_MATCH_RESULT() )
    {
      /* This result will not overwrite the previously determined pRxBuf->internal.flags */
      ackWithPending = MAC_RX_FLAG_ACK_PENDING;
    }
    pRxBuf->internal.flags |= ( crcOK | ackWithPending );

    /* finally... execute callback function */
    macRxCompleteCallback(pRxBuf);
    pRxBuf = NULL; /* needed to indicate buffer is no longer allocated */
  }
  else
  {
#ifdef FEATURE_SYSTEM_STATS
    /* Increment diagnostic CRC failure counter */
    macLowLevelDiags( MAC_DIAGS_RX_CRC_FAIL );
#endif /* FEATURE_SYSTEM_STATS */

    /*
     *  The CRC is bad so no ACK was sent.  Cancel any callback and clear the flag.
     *  (It's OK to cancel the outgoing ACK even if an ACK was not requested.  It's
     *  slightly more efficient to do so.)
     */
    MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();
    macRxOutgoingAckFlag = 0;

    /* the CRC failed so the packet must be discarded */
    MEM_FREE((uint8 **)&pRxBuf);
    pRxBuf = NULL;  /* needed to indicate buffer is no longer allocated */
  }

  /* reset threshold level, reset receive state, and complete receive logic */
  MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);
  pFuncRxState = &rxStartIsr;
  rxDone();
}


/*=================================================================================================
 * @fn          rxDone
 *
 * @brief       Common exit point for receive.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDone(void)
{
  /* if the receive FIFO has overflowed, flush it here */
  if (MAC_RADIO_RX_FIFO_HAS_OVERFLOWED())
  {
    MAC_RADIO_FLUSH_RX_FIFO();
  }

  /* mark receive as inactive */
  macRxActive = MAC_RX_ACTIVE_NO_ACTIVITY;

  /* if there is no outgoing ACK, run the post receive updates */
  if (!macRxOutgoingAckFlag)
  {
    rxPostRxUpdates();
  }
}


/**************************************************************************************************
 * @fn          macRxAckTxDoneCallback
 *
 * @brief       Function called when the outoing ACK has completed transmitting.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxAckTxDoneCallback(void)
{

  macRxOutgoingAckFlag = 0;

  macTimerCancel(&macTxAckIsrTimer);

  /*
   *  With certain interrupt priorities and timing conditions, it is possible this callback
   *  could be executed before the primary receive logic completes.  To prevent this, the
   *  post updates are only executed if receive logic is no longer active.  In the case the
   *  post updates are not executed here, they will execute when the main receive logic
   *  completes.
   */
  if (!macRxActive)
  {
    rxPostRxUpdates();
  }
}


/*=================================================================================================
 * @fn          rxPostRxUpdates
 *
 * @brief       Updates that need to be performed once receive is complete.
 *
 *              It is not fatal to execute this function if somehow receive is active.  Under
 *              certain timing/interrupt conditions a new receive may have started before this
 *              function executes.  This should happen very rarely (if it happens at all) and
 *              would cause no problems.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPostRxUpdates(void)
{
  /* turn off receiver if permitted */
  macRxOffRequest();

  /* update the transmit power, update may have been blocked by transmit of outgoing ACK */
  macRadioUpdateTxPower();

  /* initiate and transmit that was queued during receive */
  macTxStartQueuedFrame();
}


/*=================================================================================================
 * @fn          rxDiscardFrame
 *
 * @brief       Initializes for discarding a packet.  Must be called before ACK is strobed.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDiscardFrame(void)
{
  MAC_ASSERT(pFuncRxState == &rxStartIsr); /* illegal state for calling discard frame function */

  if (rxUnreadLen == 0)
  {
    rxDone();
  }
  else
  {
    rxNextLen = MIN(rxUnreadLen, MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT);
    MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
    pFuncRxState = &rxDiscardIsr;
  }
}


/*=================================================================================================
 * @fn          rxDiscardIsr
 *
 * @brief       Receive ISR state for discarding a packet.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDiscardIsr(void)
{
  uint8 buf[MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT];

  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);
  rxUnreadLen -= rxNextLen;

  /* read out and discard bytes until all bytes of packet are disposed of */
  if (rxUnreadLen != 0)
  {
    if (rxUnreadLen < MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT)
    {
      rxNextLen = rxUnreadLen;
      MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
    }
  }
  else
  {
    /* reset threshold level, reset receive state, and complete receive logic */
    MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);
    pFuncRxState = &rxStartIsr;
    rxDone();
  }
}


/**************************************************************************************************
 * @fn          macRxFifoOverflowIsr
 *
 * @brief       This interrupt service routine is called when RX FIFO overflow. Note that this
 *              exception does not retrieve the good frames that are trapped in the RX FIFO.
 *              It simply halts and cleanup the RX.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxFifoOverflowIsr(void)
{
  rxFifoOverflowCount++; /* This flag is used for debug purpose only */
  macRxHaltCleanup();
}


/**************************************************************************************************
 * @fn          macRxPromiscuousMode
 *
 * @brief       Sets promiscuous mode - enabling or disabling it.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxPromiscuousMode(uint8 mode)
{
  rxPromiscuousMode = mode;

  if (rxPromiscuousMode == MAC_PROMISCUOUS_MODE_OFF)
  {
    MAC_RADIO_TURN_ON_RX_FRAME_FILTERING();
  }
  else
  {
    MAC_ASSERT((mode == MAC_PROMISCUOUS_MODE_WITH_BAD_CRC)   ||
               (mode == MAC_PROMISCUOUS_MODE_COMPLIANT));  /* invalid mode */

    MAC_RADIO_TURN_OFF_RX_FRAME_FILTERING();
  }
}


/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

/* check for changes to the spec that would affect the source code */
#if ((MAC_A_MAX_PHY_PACKET_SIZE   !=  0x7F )   ||  \
     (MAC_FCF_FIELD_LEN           !=  2    )   ||  \
     (MAC_FCF_FRAME_TYPE_POS      !=  0    )   ||  \
     (MAC_FCF_FRAME_PENDING_POS   !=  4    )   ||  \
     (MAC_FCF_ACK_REQUEST_POS     !=  5    )   ||  \
     (MAC_FCF_INTRA_PAN_POS       !=  6    )   ||  \
     (MAC_FCF_DST_ADDR_MODE_POS   !=  10   )   ||  \
     (MAC_FCF_FRAME_VERSION_POS   !=  12   )   ||  \
     (MAC_FCF_SRC_ADDR_MODE_POS   !=  14   ))
#error "ERROR!  Change to the spec that requires modification of source code."
#endif

/* check for changes to the internal flags format */
#if ((MAC_RX_FLAG_VERSION      !=  0x03)  ||  \
     (MAC_RX_FLAG_ACK_PENDING  !=  0x04)  ||  \
     (MAC_RX_FLAG_SECURITY     !=  0x08)  ||  \
     (MAC_RX_FLAG_PENDING      !=  0x10)  ||  \
     (MAC_RX_FLAG_ACK_REQUEST  !=  0x20)  ||  \
     (MAC_RX_FLAG_INTRA_PAN    !=  0x40))
#error "ERROR!  Change to the internal RX flags format.  Requires modification of source code."
#endif

/* validate CRC OK bit optimization */
#if (MAC_RX_FLAG_CRC_OK != PROPRIETARY_FCS_CRC_OK_BIT)
#error "ERROR!  Optimization relies on these bits having the same position."
#endif

#if (MAC_RX_ACTIVE_NO_ACTIVITY != 0x00)
#error "ERROR! Zero is reserved value of macRxActive. Allows boolean operations, e.g !macRxActive."
#endif

#if (MAC_PROMISCUOUS_MODE_OFF != 0x00)
#error "ERROR! Zero is reserved value of rxPromiscuousMode. Allows boolean operations, e.g !rxPromiscuousMode."
#endif


/**************************************************************************************************
*/
