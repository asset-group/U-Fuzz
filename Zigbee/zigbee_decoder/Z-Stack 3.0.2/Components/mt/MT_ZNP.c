/**************************************************************************************************
  Filename:       MT_ZNP.c
  Revised:        $Date: 2011-08-22 16:18:12 -0700 (Mon, 22 Aug 2011) $
  Revision:       $Revision: 27245 $

  Description:    Definitions for the ZNP sub-module of the MT API.

  Copyright 2011-2015 Texas Instruments Incorporated. All rights reserved.

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

#include "comdef.h"
#include "MT.h"
#include "MT_RPC.h"
#include "MT_ZNP.h"
#include "OnBoard.h"
#include "OSAL_Nv.h"
#include "znp_app.h"

#if defined (MT_ZNP_FUNC)
/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Global Variables
 * ------------------------------------------------------------------------------------------------
 */

uint32 MT_PeriodicMsgRate;

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void znpBasicCfg(uint8 *pBuf);
static void znpZCL_Cfg(uint8 *pBuf);
static void znpSE_Cfg(uint8 *pBuf);

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          MT_ZnpCommandProcessing
 *
 * @brief       Process all MT ZNP commands.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing a ZNP command.
 *
 * output parameters
 *
 * None.
 *
 * @return      A 1-byte SRSP value or MT_RPC_SRSP_SENT or MT_RPC_SRSP_SENT;
 */
uint8 MT_ZnpCommandProcessing(uint8 *pBuf)
{
  const uint8 cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  switch (cmd1)
  {
    case MT_ZNP_BASIC_CFG:
      znpBasicCfg(pBuf);
      break;

    case MT_ZNP_ZCL_CFG:
      znpZCL_Cfg(pBuf);
      break;

    case MT_ZNP_SE_CFG:
      znpSE_Cfg(pBuf);
      break;

    default:
      return MT_RPC_ERR_COMMAND_ID;
  }

#if defined MT_RPC_SRSP_SENT
  return MT_RPC_SRSP_SENT;
#else
  return MT_RPC_SUCCESS;
#endif
}

/**************************************************************************************************
 * @fn          MT_ZnpBasicRsp
 *
 * @brief       Build and send the ZNP Basic Response to the ZAP.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      true if message built and sent; false otherwise.
 */
bool MT_ZnpBasicRsp(void)
{
  uint8 *pBuf = osal_mem_alloc(sizeof(mt_znp_basic_rsp_t));

  if (pBuf == NULL)
  {
    return false;
  }

  osal_buffer_uint32( &pBuf[0], MT_PeriodicMsgRate );

  osal_buffer_uint32( &pBuf[4], zgDefaultChannelList );

  pBuf[8] = LO_UINT16(zgConfigPANID);
  pBuf[9] = HI_UINT16(zgConfigPANID);

  osal_nv_read(ZCD_NV_STARTUP_OPTION, 0, 1, pBuf+10);
  pBuf[11] = zgDeviceLogicalType;

  pBuf[12] = LO_UINT16(_NIB.nwkDevAddress);
  pBuf[13] = HI_UINT16(_NIB.nwkDevAddress);

  pBuf[14] = LO_UINT16(_NIB.nwkCoordAddress);
  pBuf[15] = HI_UINT16(_NIB.nwkCoordAddress);

  pBuf[16] = LO_UINT16(_NIB.nwkPanId);
  pBuf[17] = HI_UINT16(_NIB.nwkPanId);

  pBuf[18] = _NIB.nwkLogicalChannel;
  pBuf[19] = _NIB.nwkState;

  (void)osal_memcpy(pBuf+20, _NIB.nwkCoordExtAddress, Z_EXTADDR_LEN);
  (void)osal_memcpy(pBuf+28, aExtendedAddress, Z_EXTADDR_LEN);

  pBuf[36] = devState;
#if defined INTER_PAN
  extern uint8 appEndPoint;
  pBuf[37] = appEndPoint;
  //rsp->spare1[2];
#else
  //rsp->spare1[3];
#endif

  // Initialize list with invalid EndPoints.
  (void)osal_memset(pBuf+40, AF_BROADCAST_ENDPOINT, (MT_ZNP_EP_ID_LIST_MAX * 3));
  uint8 idx = 40;
  epList_t *epItem = epList;

  for (uint8 cnt = 0; cnt < MT_ZNP_EP_ID_LIST_MAX; cnt++)
  {
    if (epItem == NULL)
    {
      break;
    }
    if ((epItem->epDesc->simpleDesc != NULL) && (epItem->epDesc->simpleDesc->EndPoint != ZDO_EP))
    {
      pBuf[idx++] = epItem->epDesc->simpleDesc->EndPoint;
      pBuf[idx++] = LO_UINT16(epItem->epDesc->simpleDesc->AppProfId);
      pBuf[idx++] = HI_UINT16(epItem->epDesc->simpleDesc->AppProfId);
    }
    epItem = epItem->nextDesc;
  }
  idx = 40 + (MT_ZNP_EP_ID_LIST_MAX * 3);

  // Initialize list with invalid Cluster Id's.
  (void)osal_memset(pBuf+idx, 0xFF, (MT_ZNP_ZDO_MSG_CB_LIST_MAX * 2));
  typedef struct
  {
    void *next;
    uint8 taskID;
    uint16 clusterID;
  } ZDO_MsgCB_t;
  extern ZDO_MsgCB_t *zdoMsgCBs;
  ZDO_MsgCB_t *pItem = zdoMsgCBs;

  for (uint8 cnt = 0; cnt < MT_ZNP_ZDO_MSG_CB_LIST_MAX; cnt++)
  {
    if (pItem == NULL)
    {
      break;
    }
    else if (pItem->taskID == MT_TaskID)
    {
      pBuf[idx++] = LO_UINT16(pItem->clusterID);
      pBuf[idx++] = HI_UINT16(pItem->clusterID);
    }
    pItem = pItem->next;
  }
  idx = 40 + (MT_ZNP_EP_ID_LIST_MAX * 3) + (MT_ZNP_ZDO_MSG_CB_LIST_MAX * 2);

  extern pfnZdoCb zdoCBFunc[MAX_ZDO_CB_FUNC];
  for (uint8 cnt = 0; cnt < MAX_ZDO_CB_FUNC; cnt++)
  {
    pBuf[idx++] = (zdoCBFunc[cnt] == NULL) ? 0 : 1;
  }

  MT_BuildAndSendZToolResponse(((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_ZNP), MT_ZNP_BASIC_RSP,
      40 + (MT_ZNP_EP_ID_LIST_MAX * 3) + (MT_ZNP_ZDO_MSG_CB_LIST_MAX * 2) + MAX_ZDO_CB_FUNC, pBuf);
  (void)osal_mem_free(pBuf);

  return true;
}

/**************************************************************************************************
 * @fn          znpBasicCfg
 *
 * @brief       Process the Conglomerate Basic Configuration command.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing the conglomerated configuration.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void znpBasicCfg(uint8 *pBuf)
{
  uint32 t32 = osal_build_uint32( &pBuf[0], 4 );
  if (MT_PeriodicMsgRate != t32)
  {
    MT_PeriodicMsgRate = t32;
    (void)osal_start_reload_timer(MT_TaskID, MT_PERIODIC_MSG_EVENT, t32);
  }

  t32 = osal_build_uint32( &pBuf[4], 4 );
  if (osal_memcmp(&zgDefaultChannelList, &t32, 4) == FALSE)
  {
    (void)osal_nv_write(ZCD_NV_CHANLIST, 0, 4, &t32);
  }

  uint16 t16 = osal_build_uint16( &pBuf[8] );
  if (osal_memcmp(&zgConfigPANID, &t16, 2) == FALSE)
  {
    (void)osal_nv_write(ZCD_NV_PANID, 0, 2, &t16);
  }

  if (zgDeviceLogicalType != pBuf[10])
  {
    (void)osal_nv_write(ZCD_NV_LOGICAL_TYPE, 0, 1, pBuf+10);
  }

  if (pBuf[11] & MT_ZNP_CMD_DISC_RESET_NWK)
  {
    pBuf[0] = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
    (void)osal_nv_write(ZCD_NV_STARTUP_OPTION, 0, 1, pBuf);
#if defined CC2531ZNP
    SystemResetSoft();
#else
    SystemReset();
#endif
  }
  else if (pBuf[11] & MT_ZNP_CMD_DISC_ZDO_START)
  {
    if (devState == DEV_HOLD)
    {
      ZDOInitDevice(0);
    }
  }
}

/**************************************************************************************************
 * @fn          znpZCL_Cfg
 *
 * @brief       Process the Conglomerate ZCL Configuration command.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing the conglomerated configuration.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void znpZCL_Cfg(uint8 *pBuf)
{
}

/**************************************************************************************************
 * @fn          znpSE_Cfg
 *
 * @brief       Process the Conglomerate SE Configuration command.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing the conglomerated configuration.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void znpSE_Cfg(uint8 *pBuf)
{
}

#endif
/**************************************************************************************************
*/
