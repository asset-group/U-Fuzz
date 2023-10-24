/**************************************************************************************************
  Filename:       zcl_diagnostic.c
  Revised:        $Date: 2014-03-13 15:57:20 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37682 $

  Description:    Zigbee Cluster Library - Diagnostics.


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

#ifdef ZCL_DIAGNOSTIC

/*********************************************************************
 * INCLUDES
 */
#include "zcl_diagnostic.h"
#include "ZDiags.h"

#if !defined ( FEATURE_SYSTEM_STATS )
#error "ERROR: FEATURE_SYSTEM_STATS shall be defined if ZCL_DIAGNOSTICS is defined."
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
// Attribute record
typedef struct
{
  uint16  zclAttrId;        // Attribute ID as defined by ZCL Diagnostics Cluster
  uint8   dataType;         // Data Type - defined in AF.h
  uint16  ZDiagsAttrId;     // Attribute ID as defined by ZDiags module
} zclDiagnosticAttr_t;



/*********************************************************************
 * GLOBAL VARIABLES
 */
CONST zclDiagnosticAttr_t zclDiagsAttrTable[] =
{
  {
    ATTRID_DIAGNOSTIC_NUMBER_OF_RESETS,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_NUMBER_OF_RESETS
  },
  {
    ATTRID_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_PERSISTENT_MEMORY_WRITES
  },
  {
    ATTRID_DIAGNOSTIC_MAC_RX_BCAST,
    ZCL_DATATYPE_UINT32,
    ZDIAGS_MAC_RX_BCAST
  },
  {
    ATTRID_DIAGNOSTIC_MAC_TX_BCAST,
    ZCL_DATATYPE_UINT32,
    ZDIAGS_MAC_TX_BCAST
  },
  {
    ATTRID_DIAGNOSTIC_MAC_RX_UCAST,
    ZCL_DATATYPE_UINT32,
    ZDIAGS_MAC_RX_UCAST
  },
  {
    ATTRID_DIAGNOSTIC_MAC_TX_UCAST,
    ZCL_DATATYPE_UINT32,
    ZDIAGS_MAC_TX_UCAST
  },
  {
    ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_MAC_TX_UCAST_RETRY
  },
  {
    ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_MAC_TX_UCAST_FAIL
  },
  {
    ATTRID_DIAGNOSTIC_APS_RX_BCAST,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_RX_BCAST
  },
  {
    ATTRID_DIAGNOSTIC_APS_TX_BCAST,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_TX_BCAST
  },
  {
    ATTRID_DIAGNOSTIC_APS_RX_UCAST,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_RX_UCAST
  },
  {
    ATTRID_DIAGNOSTIC_APS_TX_UCAST_SUCCESS,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_TX_UCAST_SUCCESS
  },
  {
    ATTRID_DIAGNOSTIC_APS_TX_UCAST_RETRY,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_TX_UCAST_RETRY
  },
  {
    ATTRID_DIAGNOSTIC_APS_TX_UCAST_FAIL,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_TX_UCAST_FAIL
  },
  {
    ATTRID_DIAGNOSTIC_ROUTE_DISC_INITIATED,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_ROUTE_DISC_INITIATED
  },
  {
    ATTRID_DIAGNOSTIC_NEIGHBOR_ADDED,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_NEIGHBOR_ADDED
  },
  {
    ATTRID_DIAGNOSTIC_NEIGHBOR_REMOVED,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_NEIGHBOR_REMOVED
  },
  {
    ATTRID_DIAGNOSTIC_NEIGHBOR_STALE,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_NEIGHBOR_STALE
  },
  {
    ATTRID_DIAGNOSTIC_JOIN_INDICATION,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_JOIN_INDICATION
  },
  {
    ATTRID_DIAGNOSTIC_CHILD_MOVED,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_CHILD_MOVED
  },
  {
    ATTRID_DIAGNOSTIC_NWK_FC_FAILURE,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_NWK_FC_FAILURE
  },
  {
    ATTRID_DIAGNOSTIC_APS_FC_FAILURE,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_FC_FAILURE
  },
  {
    ATTRID_DIAGNOSTIC_APS_UNAUTHORIZED_KEY,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_UNAUTHORIZED_KEY
  },
  {
    ATTRID_DIAGNOSTIC_NWK_DECRYPT_FAILURES,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_NWK_DECRYPT_FAILURES
  },
  {
    ATTRID_DIAGNOSTIC_APS_DECRYPT_FAILURES,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_APS_DECRYPT_FAILURES
  },
  {
    ATTRID_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES
  },
  {
    ATTRID_DIAGNOSTIC_RELAYED_UCAST,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_RELAYED_UCAST
  },
  {
    ATTRID_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED
  },
  {
    ATTRID_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT,
    ZCL_DATATYPE_UINT16,
    ZDIAGS_PACKET_VALIDATE_DROP_COUNT
  },
};

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclDiagnostic_GetAttribData( uint16 zclAttrId, uint16 *zdiagsAttrId, uint16 *dataLen );

/****************************************************************************
 * @fn          zclDiagnostic_GetAttribData()
 *
 * @brief       Gets the Z-Stack attribute data for a specific ZCL Diagnostics
 *              AttributeID.
 *
 * @param       none.
 *
 * @return      none.
 */
static ZStatus_t zclDiagnostic_GetAttribData( uint16 zclAttrId, uint16 *zdiagsAttrId, uint16 *dataLen )
{
  uint8 i;
  uint8 attrTableSize = sizeof(zclDiagsAttrTable);

  for ( i = 0; i < attrTableSize; i++ )
  {
    if ( zclDiagsAttrTable[i].zclAttrId == zclAttrId )
    {
      *zdiagsAttrId = zclDiagsAttrTable[i].ZDiagsAttrId;
      *dataLen = (uint16)zclGetDataTypeLength( zclDiagsAttrTable[i].dataType );

      return ( ZSuccess );
    }
  }

  return ( ZFailure );
}

/****************************************************************************
 * @fn          zclDiagnostic_InitStats()
 *
 * @brief       Initialize the statistics table.
 *
 * @param       none.
 *
 * @return      ZSuccess - if NV data was initialized successfully.
 *              ZFailure - Otherwise
 */
uint8 zclDiagnostic_InitStats( void )
{
  // Initialize the Diagnostics table in the Lower layer
  return ( ZDiagsInitStats() );
}

/****************************************************************************
 * @fn          zclDiagnostic_ClearStats
 *
 * @brief       Clears the statistics table in RAM and NV if option flag set.
 *
 * @param       clearNV   - Option flag to clear NV data.
 *
 * @return      System Clock.
 */
uint32 zclDiagnostic_ClearStats( bool clearNV )
{
  // calls the diagnostics function to clear statistics and returns the system clock
  return ZDiagsClearStats( clearNV );
}

/****************************************************************************
 * @fn          zclDiagnostic_GetStatsAttr
 *
 * @brief       Reads Diagnostic values based on specific ZCL Diagnostics
 *              attribute ID
 *
 * @param       attributeId  input  - ZCL identifier for the required attribute
 * @param       value       output - value of the specific item
 *
 * NOTE:  the user of this function will have to cast the value
 *        based on the type of the attributeId, the returned value
 *        will allways be uint32
 *
 * @return      ZStatus_t
 */
ZStatus_t zclDiagnostic_GetStatsAttr( uint16 attributeId, uint32 *attrValue, uint16 *dataLen )
{
  uint8 status = ZSuccess;
  uint16 ZDiagsAttrId;

  // this atribute is a calculated value
  if ( attributeId == ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT )
  {
    uint32 macRetriesPerApsTx;
    uint32 apsTxUcastSuccess;
    uint32 apsTxUcastFailure;

    // retrieve each attribute to calculate the requested value
    macRetriesPerApsTx = ZDiagsGetStatsAttr( ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS );

    apsTxUcastSuccess = ZDiagsGetStatsAttr( ZDIAGS_APS_TX_UCAST_SUCCESS );

    apsTxUcastFailure = ZDiagsGetStatsAttr( ZDIAGS_APS_TX_UCAST_FAIL );

    *dataLen = 2;  // this is the lenght of ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT

    if ( ( apsTxUcastSuccess != 0 ) || ( apsTxUcastFailure != 0 ) )
    {
      // This formula considers the total MAC Failures per APS transmitted packet.
      // If MAC PIB element maxFrameRetries is changed from the default value 3, this formula
      // shall be updated and replace 4 with (MAC PIB maxFrameRetries+1) value
      *attrValue = ( macRetriesPerApsTx + ( apsTxUcastFailure * 4 ) ) / ( apsTxUcastSuccess + apsTxUcastFailure );
    }
    else
    {
      *attrValue = 0;
    }
  }
  // look-up for ZDiags attribute ID, based on the ZCL Diagnostics cluster attribute ID
  else if ( zclDiagnostic_GetAttribData( attributeId, &ZDiagsAttrId, dataLen ) == ZSuccess )
  {
    *attrValue = ZDiagsGetStatsAttr( ZDiagsAttrId );
  }
  else
  {
    status = ZFailure;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclDiagnostic_ReadWriteAttrCB
 *
 * @brief   Handle Diagnostics attributes.
 *
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute to be read or written
 * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
 * @param   pValue - pointer to attribute value, OTA endian
 * @param   pLen - length of attribute value read, native endian
 *
 * @return  status
 */
ZStatus_t zclDiagnostic_ReadWriteAttrCB( uint16 clusterId, uint16 attrId, uint8 oper,
                                         uint8 *pValue, uint16 *pLen )
{
  ZStatus_t status = ZSuccess;
  uint16 tempAttr;
  uint32 attrValue;
  afIncomingMSGPacket_t *origPkt;

  origPkt = zcl_getRawAFMsg();

  switch ( oper )
  {
    case ZCL_OPER_LEN:
      if ( ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI ) ||
           ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI ) )
      {
        *pLen = 1;
      }
      else if ( attrId == ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT )
      {
        *pLen = 2;
      }
      // The next function call only returns the length for attributes that are defined
      // in lower layers
      else if ( zclDiagnostic_GetAttribData( attrId, &tempAttr, pLen ) != ZSuccess )
      {
        *pLen = 0;
        status = ZFailure;  // invalid length
      }
      break;

    case ZCL_OPER_READ:
      // Identify if incoming msg is LQI or RSSI attribute
      // and return the LQI and RSSI of the incoming values
      if ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI )
      {
        *pLen = 1;
        attrValue = origPkt->LinkQuality;
      }
      else if ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI )
      {
        //origPkt = zcl_getRawAFMsg();
        *pLen = 1;
        attrValue = origPkt->rssi;
      }
      else if ( zclDiagnostic_GetStatsAttr( attrId, &attrValue, pLen ) == ZSuccess )
      {
        if ( ( attrId == ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY ) ||
             ( attrId == ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL  ) )
        {
          // The lower layer counter is a 32 bit counter, report the higher 16 bit value
          // util the lower layer counter wraps-up
          if ( attrValue > 0x0000FFFF )
          {
            attrValue = 0x0000FFFF;
          }
        }
      }
      else
      {
        *pLen = 0;
        status = ZFailure;  // invalid attribute
      }

      if ( *pLen == 1 )
      {
        pValue[0] = BREAK_UINT32( attrValue, 0 );
      }
      else if ( *pLen == 2 )
      {
        pValue[0] = LO_UINT16( attrValue );
        pValue[1] = HI_UINT16( attrValue );
      }
      else if ( *pLen == 4 )
      {
        pValue[0] = BREAK_UINT32( attrValue, 0 );
        pValue[1] = BREAK_UINT32( attrValue, 1 );
        pValue[2] = BREAK_UINT32( attrValue, 2 );
        pValue[3] = BREAK_UINT32( attrValue, 3 );
      }

      break;

    case ZCL_OPER_WRITE:
      status = ZFailure;  // All attributes in Diagnostics cluster are READ ONLY
      break;
  }

  return ( status );
}

/****************************************************************************
 * @fn          zclDiagnostic_RestoreStatsFromNV
 *
 * @brief       Restores the statistics table from NV into the RAM table.
 *
 * @param       none.
 *
 * @return      ZSuccess - if NV data was restored from NV.
 *              ZFailure - Otherwise, NV_OPER_FAILED for failure.
 */
uint8 zclDiagnostic_RestoreStatsFromNV( void )
{
  return ( ZDiagsRestoreStatsFromNV() );
}

   /****************************************************************************
 * @fn          zclDiagnostic_SaveStatsToNV
 *
 * @brief       Saves the statistics table from RAM to NV.
 *
 * @param       none.
 *
 * @return      System Time.
 */
uint32 zclDiagnostic_SaveStatsToNV( void )
{
   return( ZDiagsSaveStatsToNV() );
}


#endif // ZCL_DIAGNOSTIC
/********************************************************************************************
*********************************************************************************************/

