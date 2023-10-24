/**************************************************************************************************
  Filename:       mac_pib.c
  Revised:        $Date: 2015-01-09 12:53:20 -0800 (Fri, 09 Jan 2015) $
  Revision:       $Revision: 41702 $

  Description:    This module contains procedures for the MAC PIB.


  Copyright 2006-2015 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "hal_board.h"
#include "mac_api.h"
#include "mac_spec.h"
#include "mac_low_level.h"
#include "mac_radio_defs.h"
#include "mac_main.h"
#include "mac_pib.h"
#include "OSAL.h"
#include <stddef.h>

#include "R2R_FlashJT.h"
#if defined (CC26XX)
#include "R2F_FlashJT.h"
#endif /* CC26XX */

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Attribute index constants, based on attribute ID values from spec */
#define MAC_ATTR_SET1_START       0x40
#define MAC_ATTR_SET1_END         0x5D
#define MAC_ATTR_SET1_OFFSET      0
#define MAC_ATTR_SET2_START       0xE0
#define MAC_ATTR_SET2_END         0xEE
#define MAC_ATTR_SET2_OFFSET      (MAC_ATTR_SET1_END - MAC_ATTR_SET1_START + MAC_ATTR_SET1_OFFSET + 1)

/* frame response values */
#define MAC_MAX_FRAME_RESPONSE_MIN  143
#define MAC_MAX_FRAME_RESPONSE_MAX  25776

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* PIB access and min/max table type */
typedef struct
{
  uint8     offset;
  uint8     len;
  uint8     min;
  uint8     max;
} macPibTbl_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* PIB default values */
static CODE const macPib_t macPibDefaults =
{
  54,                                         /* ackWaitDuration */
  FALSE,                                      /* associationPermit */
  TRUE,                                       /* autoRequest */
  FALSE,                                      /* battLifeExt */
  6,                                          /* battLifeExtPeriods */

  NULL,                                       /* *pMacBeaconPayload */
  0,                                          /* beaconPayloadLength */
  MAC_BO_NON_BEACON,                          /* beaconOrder */
  0,                                          /* beaconTxTime */
  0,                                          /* bsn */

  {0, SADDR_MODE_EXT},                        /* coordExtendedAddress */
  MAC_SHORT_ADDR_NONE,                        /* coordShortAddress */
  0,                                          /* dsn */
  FALSE,                                      /* gtsPermit */
  4,                                          /* maxCsmaBackoffs */

  3,                                          /* minBe */
  0xFFFF,                                     /* panId */
  FALSE,                                      /* promiscuousMode */
  FALSE,                                      /* rxOnWhenIdle */
  MAC_SHORT_ADDR_NONE,                        /* shortAddress */

  MAC_SO_NONE,                                /* superframeOrder */
  0x01F4,                                     /* transactionPersistenceTime */
  FALSE,                                      /* assocciatedPanCoord */
  5,                                          /* maxBe */
  1220,                                       /* maxFrameTotalWaitTime */

  3,                                          /* maxFrameRetries */
  32,                                         /* ResponseWaitTime */
  0,                                          /* syncSymbolOffset */
  TRUE,                                       /* timeStampSupported */
  FALSE,                                      /* securityEnabled */

  /* Proprietary */
  0,                                          /* phyTransmitPower */
  MAC_CHAN_11,                                /* logicalChannel */
  {0, SADDR_MODE_EXT},                        /* extendedAddress */
  1,                                          /* altBe */
  MAC_BO_NON_BEACON,                          /* deviceBeaconOrder */
  0,                                          /* power savings for rf4ce */
  0,                                          /* default is frame Ver is always 0, 
                                                 except for security enabled packets */
  0,                                          /* diagsRxCrcPass */
  0,                                          /* diagsRxCrcFail */
  0,                                          /* diagsRxBcast */
  0,                                          /* diagsTxBcast */
  0,                                          /* diagsRxUcast */
  0,                                          /* diagsTxUcast */
  0,                                          /* diagsTxUcastRetry */
  0                                           /* diagsTxUcastFail */
};


/* PIB access and min/max table.  min/max of 0/0 means not checked; if min/max are
 * equal, element is read-only
 */
#if !defined (CC26XX) || defined (FLASH_ONLY_BUILD)
static CODE const macPibTbl_t macPibTbl[] =
{
  {offsetof(macPib_t, ackWaitDuration), sizeof(uint8), 54, 54},                      /* MAC_ACK_WAIT_DURATION */
  {offsetof(macPib_t, associationPermit), sizeof(bool), FALSE, TRUE},                /* MAC_ASSOCIATION_PERMIT */
  {offsetof(macPib_t, autoRequest), sizeof(bool), FALSE, TRUE},                      /* MAC_AUTO_REQUEST */
  {offsetof(macPib_t, battLifeExt), sizeof(bool), FALSE, TRUE},                      /* MAC_BATT_LIFE_EXT */
  {offsetof(macPib_t, battLifeExtPeriods), sizeof(uint8), 6, 6},                     /* MAC_BATT_LIFE_EXT_PERIODS */

  {offsetof(macPib_t, pBeaconPayload), sizeof(uint8 *), 0, 0},                       /* MAC_BEACON_PAYLOAD */
  {offsetof(macPib_t, beaconPayloadLength), sizeof(uint8), 0, 52},                   /* MAC_BEACON_PAYLOAD_LENGTH */
  {offsetof(macPib_t, beaconOrder), sizeof(uint8), 0, 15},                           /* MAC_BEACON_ORDER */
  {offsetof(macPib_t, beaconTxTime), sizeof(uint32), 1, 1},                          /* MAC_BEACON_TX_TIME */
  {offsetof(macPib_t, bsn), sizeof(uint8), 0x00, 0xFF},                              /* MAC_BSN */

  {offsetof(macPib_t, coordExtendedAddress.addr.extAddr), sizeof(sAddrExt_t), 0, 0}, /* MAC_COORD_EXTENDED_ADDRESS */
  {offsetof(macPib_t, coordShortAddress), sizeof(uint16), 0, 0},                     /* MAC_COORD_SHORT_ADDRESS */
  {offsetof(macPib_t, dsn), sizeof(uint8), 0x00, 0xFF},                              /* MAC_DSN */
  {offsetof(macPib_t, gtsPermit), sizeof(bool), FALSE, TRUE},                        /* MAC_GTS_PERMIT */

  /* Range of maxCsmaBackoffs is between 0 and 5 in IEEE 802.15.4.
   * Such restriction is removed here to allow 802.15.4 non-compliant experimental
   * applications.
   * To be compliant with 802.15.4, application or upper layer must not set this
   * PIB attribute out of range of 802.15.4.
   * The range is still restricted to 254 since value 255 would cause backoff
   * counter to overflow, wrap around and loop infinitely. */
  {offsetof(macPib_t, maxCsmaBackoffs), sizeof(uint8), 0, 254},                      /* MAC_MAX_CSMA_BACKOFFS */

  {offsetof(macPib_t, minBe), sizeof(uint8), 0, 8},                                  /* MAC_MIN_BE */
  {offsetof(macPib_t, panId), sizeof(uint16), 0, 0},                                 /* MAC_PAN_ID */
  {offsetof(macPib_t, promiscuousMode), sizeof(bool), FALSE, TRUE},                  /* MAC_PROMISCUOUS_MODE */
  {offsetof(macPib_t, rxOnWhenIdle), sizeof(bool), FALSE, TRUE},                     /* MAC_RX_ON_WHEN_IDLE */
  {offsetof(macPib_t, shortAddress), sizeof(uint16), 0, 0},                          /* MAC_SHORT_ADDRESS */

  {offsetof(macPib_t, superframeOrder), sizeof(uint8), 0, 15},                       /* MAC_SUPERFRAME_ORDER */
  {offsetof(macPib_t, transactionPersistenceTime), sizeof(uint16), 0, 0},            /* MAC_TRANSACTION_PERSISTENCE_TIME */
  {offsetof(macPib_t, associatedPanCoord), sizeof(bool), FALSE, TRUE},               /* MAC_ASSOCIATED_PAN_COORD */

  /* Range of maxBe is between 3 and 8 in IEEE 802.15.4.
   * Such restriction is removed here to allow 802.15.4 non-compliant experimental
   * applications.
   * To be compliant with 802.15.4, application or upper layer must not set this
   * PIB attribute out of range of 802.15.4. */
  {offsetof(macPib_t, maxBe), sizeof(uint8), 0, 8},                                  /* MAC_MAX_BE */
  {offsetof(macPib_t, maxFrameTotalWaitTime), sizeof(uint16), 0x00, 0xFF},           /* MAC_MAX_FRAME_RESPONSE_TIME */

  {offsetof(macPib_t, maxFrameRetries), sizeof(uint8), 0, 7},                        /* MAC_MAX_FRAME_RETRIES */
  {offsetof(macPib_t, responseWaitTime), sizeof(uint8), 2, 64},                      /* MAC_RESPONSE_WAIT_TIME */
  {offsetof(macPib_t, syncSymbolOffset), sizeof(uint8), 0, 0},                       /* MAC_SYNC_SYMBOL_OFFSET */
  {offsetof(macPib_t, timeStampSupported), sizeof(bool), FALSE, TRUE},               /* MAC_TIMESTAMP_SUPPORTED */
  {offsetof(macPib_t, securityEnabled), sizeof(bool), FALSE, TRUE},                  /* MAC_SECURITY_ENABLED */

  /* Proprietary PIBs */
  {offsetof(macPib_t, phyTransmitPower), sizeof(uint8), 0, 0xFF},                    /* MAC_PHY_TRANSMIT_POWER_SIGNED */
  {offsetof(macPib_t, logicalChannel), sizeof(uint8), MAC_CHAN_11, MAC_CHAN_29},     /* MAC_LOGICAL_CHANNEL */
  {offsetof(macPib_t, extendedAddress.addr.extAddr), sizeof(sAddrExt_t), 0, 0},      /* MAC_EXTENDED_ADDRESS */
  {offsetof(macPib_t, altBe), sizeof(uint8), 0, 8},                                  /* MAC_ALT_BE */
  {offsetof(macPib_t, deviceBeaconOrder), sizeof(uint8), 0, 15},                     /* MAC_DEVICE_BEACON_ORDER */
  {offsetof(macPib_t, rf4cepowerSavings), sizeof(uint8), 0, 1},                      /* MAC_RF4CE_POWER_SAVINGS */
  {offsetof(macPib_t, frameVersionSupport), sizeof(uint8), 0, 0xFF},                 /* MAC_FRAME_VERSION_SUPPORT */
  {offsetof(macPib_t, diagsRxCrcPass), sizeof(uint32), 0, 0},                        /* MAC_DIAGS_RX_CRC_PASS */
  {offsetof(macPib_t, diagsRxCrcFail), sizeof(uint32), 0, 0},                        /* MAC_DIAGS_RX_CRC_FAIL */
  {offsetof(macPib_t, diagsRxBcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_RX_BCAST */
  {offsetof(macPib_t, diagsTxBcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_TX_BCAST */
  {offsetof(macPib_t, diagsRxUcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_RX_UCAST */
  {offsetof(macPib_t, diagsTxUcast), sizeof(uint32), 0, 0},                          /* MAC_DIAGS_TX_UCAST */
  {offsetof(macPib_t, diagsTxUcastRetry), sizeof(uint32), 0, 0},                     /* MAC_DIAGS_TX_UCAST_RETRY */
  {offsetof(macPib_t, diagsTxUcastFail), sizeof(uint32), 0, 0}                       /* MAC_DIAGS_TX_UCAST_FAIL *//* MAC_DIAGS_TX_UCAST_FAIL */

};
#endif /* !CC26XX || FLASH_ONLY_BUILD */

/* Invalid PIB table index used for error code */
#define MAC_PIB_INVALID     ((uint8) (sizeof(macPibTbl) / sizeof(macPibTbl[0])))

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* MAC PIB */
macPib_t macPib;

extern uint8 MAC_MlmeGetReqSize( uint8 pibAttribute );

#if defined( FEATURE_MAC_PIB_PTR )

/* Pointer to the MAC PIB */
macPib_t* pMacPib = &macPib;

/**************************************************************************************************
 * @fn          MAC_MlmeSetActivePib
 *
 * @brief       This function initializes the PIB.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_MlmeSetActivePib( void* pPib )
{
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION(intState);
  pMacPib = (macPib_t *)pPib;
  HAL_EXIT_CRITICAL_SECTION(intState);
}
#endif /* FEATURE_MAC_PIB_PTR */

/**************************************************************************************************
 * @fn          macPibReset
 *
 * @brief       This function initializes the PIB.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macPibReset(void)
{
  /* copy PIB defaults */
#if defined( FEATURE_MAC_PIB_PTR )  
  *pMacPib = macPibDefaults;
#else
  macPib = macPibDefaults;
#endif /* FEATURE_MAC_PIB_PTR */

  /* initialize random sequence numbers */
  pMacPib->dsn = macRadioRandomByte();
  pMacPib->bsn = macRadioRandomByte();
}

/**************************************************************************************************
 * @fn          macPibIndex
 *
 * @brief       This function takes an PIB attribute and returns the index in to
 *              macPibTbl for the attribute.
 *
 * input parameters
 *
 * @param       pibAttribute - PIB attribute to look up.
 *
 * output parameters
 *
 * None.
 *
 * @return      Index in to macPibTbl for the attribute or MAC_PIB_INVALID.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macPibIndex(uint8 pibAttribute)
{
  if ((pibAttribute >= MAC_ATTR_SET1_START) && (pibAttribute <= MAC_ATTR_SET1_END))
  {
    return (pibAttribute - MAC_ATTR_SET1_START + MAC_ATTR_SET1_OFFSET);
  }
  else if ((pibAttribute >= MAC_ATTR_SET2_START) && (pibAttribute <= MAC_ATTR_SET2_END))
  {
    return (pibAttribute - MAC_ATTR_SET2_START + MAC_ATTR_SET2_OFFSET);
  }
  else
  {
    return MAC_PIB_INVALID;
  }
}


/**************************************************************************************************
 * @fn          MAC_MlmeGetReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeGetReq(uint8 pibAttribute, void *pValue)
{
  uint8         i;
  halIntState_t intState;

  if ((i = MAP_macPibIndex(pibAttribute)) == MAC_PIB_INVALID)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;
  }

  HAL_ENTER_CRITICAL_SECTION(intState);
  osal_memcpy(pValue, (uint8 *) pMacPib + macPibTbl[i].offset, macPibTbl[i].len);
  HAL_EXIT_CRITICAL_SECTION(intState);
  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          MAC_MlmeGetReqSize
 *
 * @brief       This direct execute function gets the MAC PIB attribute size
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * None.
 *
 * @return      size in bytes
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeGetReqSize( uint8 pibAttribute )
{
  uint8 index;

  if ((index = MAP_macPibIndex(pibAttribute)) == MAC_PIB_INVALID)
  {
    return 0;
  }

  return ( macPibTbl[index].len );
}

#if !defined (CC26XX) || defined (FLASH_ONLY_BUILD)
/**************************************************************************************************
 * @fn          MAC_MlmeSetReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeSetReq(uint8 pibAttribute, void *pValue)
{
  uint8         i;
  halIntState_t intState;

  if(pibAttribute == MAC_RX_ON_OFF)
  {
    if(*(uint8*)pValue == 1)
    {
      macRxDisable(MAC_RX_WHEN_IDLE);
    }
    else
    {
      macRxEnable(MAC_RX_WHEN_IDLE);
    }
  }
  
  if(pibAttribute == MAC_SUPERFRAME_PAN_COORD)
  {
      macPanCoordinator = *(bool*)pValue;
      macRadioSetPanCoordinator(macPanCoordinator);
  }
  
  if (pibAttribute == MAC_BEACON_PAYLOAD)
  {
    pMacPib->pBeaconPayload = pValue;
    return MAC_SUCCESS;
  }

  /* look up attribute in PIB table */
  if ((i = MAP_macPibIndex(pibAttribute)) == MAC_PIB_INVALID)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;
  }

  /* do range check; no range check if min and max are zero */
  if ((macPibTbl[i].min != 0) || (macPibTbl[i].max != 0))
  {
    /* if min == max, this is a read-only attribute */
    if (macPibTbl[i].min == macPibTbl[i].max)
    {
      return MAC_READ_ONLY;
    }

    /* check for special cases */
    if (pibAttribute == MAC_MAX_FRAME_TOTAL_WAIT_TIME)
    {
      if ((*((uint16 *) pValue) < MAC_MAX_FRAME_RESPONSE_MIN) ||
          (*((uint16 *) pValue) > MAC_MAX_FRAME_RESPONSE_MAX))
      {
        return MAC_INVALID_PARAMETER;
      }
    }

    /* range check for general case */
    if ((*((uint8 *) pValue) < macPibTbl[i].min) || (*((uint8 *) pValue) > macPibTbl[i].max))
    {
      return MAC_INVALID_PARAMETER;
    }

  }

  /* set value in PIB */
  HAL_ENTER_CRITICAL_SECTION(intState);
  osal_memcpy((uint8 *) pMacPib + macPibTbl[i].offset, pValue, macPibTbl[i].len);
  HAL_EXIT_CRITICAL_SECTION(intState);

  /* handle special cases */
  switch (pibAttribute)
  {
    case MAC_PAN_ID:
      /* set pan id in radio */
      macRadioSetPanID(pMacPib->panId);
      break;

    case MAC_SHORT_ADDRESS:
      /* set short address in radio */
      macRadioSetShortAddr(pMacPib->shortAddress);
      break;

    case MAC_RX_ON_WHEN_IDLE:
      /* turn rx on or off */
      if (pMacPib->rxOnWhenIdle)
      {
        macRxEnable(MAC_RX_WHEN_IDLE);
      }
      else
      {
        macRxDisable(MAC_RX_WHEN_IDLE);
      }
      break;

    case MAC_LOGICAL_CHANNEL:
      macRadioSetChannel(pMacPib->logicalChannel);
      break;

    case MAC_EXTENDED_ADDRESS:
      /* set ext address in radio */
      macRadioSetIEEEAddr(pMacPib->extendedAddress.addr.extAddr);
      break;

    case MAC_PHY_TRANSMIT_POWER_SIGNED:
      (void)macRadioSetTxPower(pMacPib->phyTransmitPower);
      break;

    case MAC_RF4CE_POWER_SAVINGS:
      pMacPib->rf4cepowerSavings = *(uint8 *)pValue;
      break;
 
    case MAC_FRAME_VERSION_SUPPORT:
      pMacPib->frameVersionSupport = *(uint8 *)pValue;
      break;

    default:
      break;
  }

  return MAC_SUCCESS;
}
#endif /* !CC26XX || FLASH_ONLY_BUILD */
