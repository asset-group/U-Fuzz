/**************************************************************************************************
  Filename:       znp_app.c
  Revised:        $Date: 2014-11-24 18:26:24 -0800 (Mon, 24 Nov 2014) $
  Revision:       $Revision: 41234 $

  Description:    This file is the Application implementation for the ZNP.


  Copyright 2009-2014 Texas Instruments Incorporated. All rights reserved.

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

#include "hal_board_cfg.h"
#include "mac_radio_defs.h"
#include "MT.h"
#include "MT_AF.h"
#include "MT_SYS.h"
#include "MT_UART.h"
#include "MT_UTIL.h"
#include "MT_ZDO.h"
#if defined MT_ZNP_FUNC
#include "MT_ZNP.h"
#endif
#include "OSAL.h"
#include "OSAL_Nv.h"
#if defined POWER_SAVING || defined CC2531ZNP
#include "OSAL_PwrMgr.h"
#endif
#include "ZComDef.h"
#include "ZMAC.h"
#include "znp_app.h"
#include "znp_spi.h"

#if defined ( TC_LINKKEY_JOIN ) || defined ( ZCL_KEY_ESTABLISH )
  #include "zcl.h"
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void npInit(void);
static void npInitNV(void);

#if defined MT_ZNP_FUNC
static void npBasicRsp(void);
#endif

static void npUartCback(uint8 port, uint8 event);
static void npUartTxReady(void);
static uint8* npMtUartAlloc(uint8 cmd0, uint8 len);
static void npMtUartSend(uint8 *pBuf);

#if !defined CC2531ZNP
static uint8* npMtSpiAlloc(uint8 cmd0, uint8 len);
static void npMtSpiSend(uint8 *pBuf);
uint8* npSpiPollCallback(void);
bool npSpiReadyCallback(void);
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

static osal_msg_q_t npTxQueue;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

uint8 znpCfg1;
uint8 znpCfg0;

#if defined TC_LINKKEY_JOIN
extern uint8 zcl_TaskID;
#endif

/**************************************************************************************************
 * @fn          znpInit
 *
 * @brief       This function is the OSAL task initialization callback.
 *
 * input parameters
 *
 * @param taskId - The task ID assigned to this task by the OSAL.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void znpInit(uint8 taskId)
{
  znpTaskId = taskId;
  (void)osal_set_event(taskId, ZNP_SECONDARY_INIT_EVENT);
#if defined MT_ZNP_FUNC
  znpBasicRspRate = ZNP_BASIC_RSP_RATE;
  (void)osal_start_reload_timer(taskId, ZNP_BASIC_RSP_EVENT, ZNP_BASIC_RSP_RATE);
#endif
}

/**************************************************************************************************
 * @fn          znpEventLoop
 *
 * @brief       This function processes the OSAL events and messages for the application.
 *
 * input parameters
 *
 * @param taskId - The task ID assigned to this application by OSAL at system initialization.
 * @param events - A bit mask of the pending event(s).
 *
 * output parameters
 *
 * None.
 *
 * @return      The events bit map received via parameter with the bits cleared which correspond to
 *              the event(s) that were processed on this invocation.
 **************************************************************************************************
 */
uint16 znpEventLoop(uint8 taskId, uint16 events)
{
  osal_event_hdr_t *pMsg;
#if !defined CC2531ZNP
  uint8 *pBuf;
#endif

  if (events & SYS_EVENT_MSG)
  {
    while ((pMsg = (osal_event_hdr_t *) osal_msg_receive(znpTaskId)) != NULL)
    {
      switch (pMsg->event)
      {
      /* incoming message from UART transport */
      case CMD_SERIAL_MSG:
        MT_ProcessIncoming(((mtOSALSerialData_t *)pMsg)->msg);
        break;

#if defined ZCL_KEY_ESTABLISH
#if defined (MT_UTIL_FUNC)
      case ZCL_KEY_ESTABLISH_IND:
        MT_UtilKeyEstablishInd((zclKE_StatusInd_t *)pMsg);
        break;
#endif
#endif

      case AF_INCOMING_MSG_CMD:
#if defined ZCL_KEY_ESTABLISH
        if (ZCL_KE_ENDPOINT == (((afIncomingMSGPacket_t *)pMsg)->endPoint))
        {
          zcl_ProcessMessageMSG((afIncomingMSGPacket_t *)pMsg);
        }
        else
#endif
        {
          MT_AfIncomingMsg((afIncomingMSGPacket_t *)pMsg);
        }
        break;

#ifdef MT_ZDO_FUNC
      case ZDO_STATE_CHANGE:
        MT_ZdoStateChangeCB(pMsg);
        break;

      case ZDO_CB_MSG:
        MT_ZdoSendMsgCB((zdoIncomingMsg_t *)pMsg);
        break;
#endif

      case AF_DATA_CONFIRM_CMD:
        MT_AfDataConfirm((afDataConfirm_t *)pMsg);
        break;

      default:
        break;
      }

      osal_msg_deallocate((byte *)pMsg);
    }

    events ^= SYS_EVENT_MSG;
  }
#if !defined CC2531ZNP
  else if (events & ZNP_SPI_RX_AREQ_EVENT)
  {
    if ((pBuf = npSpiGetReqBuf()) != NULL )
    {
      MT_ProcessIncoming(pBuf);
      npSpiAReqComplete();
    }

    events ^= ZNP_SPI_RX_AREQ_EVENT;
  }
  else if (events & ZNP_SPI_RX_SREQ_EVENT)
  {
    if ((pBuf = npSpiGetReqBuf()) != NULL)
    {
      MT_ProcessIncoming(pBuf);
    }

    events ^= ZNP_SPI_RX_SREQ_EVENT;
  }
#endif
  else if (events & ZNP_UART_TX_READY_EVENT)
  {
    npUartTxReady();
    events ^= ZNP_UART_TX_READY_EVENT;
  }
#if defined MT_SYS_FUNC
  else if (events & MT_SYS_OSAL_EVENT_0)
  {
    MT_SysOsalTimerExpired(0x00);
    events ^= MT_SYS_OSAL_EVENT_0;
  }
  else if (events & MT_SYS_OSAL_EVENT_1)
  {
    MT_SysOsalTimerExpired(0x01);
    events ^= MT_SYS_OSAL_EVENT_1;
  }
  else if (events & MT_SYS_OSAL_EVENT_2)
  {
    MT_SysOsalTimerExpired(0x02);
    events ^= MT_SYS_OSAL_EVENT_2;
  }
  else if (events & MT_SYS_OSAL_EVENT_3)
  {
    MT_SysOsalTimerExpired(0x03);
    events ^= MT_SYS_OSAL_EVENT_3;
  }
#endif
#if defined POWER_SAVING
  else if (events & ZNP_PWRMGR_CONSERVE_EVENT)
  {
    (void)osal_pwrmgr_task_state(znpTaskId, PWRMGR_CONSERVE);
    events ^= ZNP_PWRMGR_CONSERVE_EVENT;
  }
#endif
  else if (events & ZNP_SECONDARY_INIT_EVENT)
  {
    npInit();
    events ^= ZNP_SECONDARY_INIT_EVENT;
  }
#if defined MT_ZNP_FUNC
  else if (events & ZNP_BASIC_RSP_EVENT)
  {
    npBasicRsp();
    events ^= ZNP_BASIC_RSP_EVENT;
  }
#endif
  else if (events & MT_AF_EXEC_EVT)
  {
    MT_AfExec();
    events ^= MT_AF_EXEC_EVT;
  }
  else
  {
    events = 0;  /* Discard unknown events. */
  }

  return ( events );
}

/**************************************************************************************************
 * @fn          MT_TransportAlloc
 *
 * @brief       This function is the definition of the physical transport API for allocation a msg.
 *
 * input parameters
 *
 * @param cmd0 - The RPC command byte 0.
 * @param len - The RPC data length.
 *
 * output parameters
 *
 * @param uint8 * - Pointer to the buffer to use build and send the RPC message.
 *
 * @return      None.
 **************************************************************************************************
 */
uint8 *MT_TransportAlloc(uint8 cmd0, uint8 len)
{
#if !defined CC2531ZNP
  if (ZNP_CFG1_UART == znpCfg1)
#endif
  {
    return npMtUartAlloc(cmd0, len);
  }
#if !defined CC2531ZNP
  else
  {
    return npMtSpiAlloc(cmd0, len);
  }
#endif
}

/**************************************************************************************************
 * @fn          MT_TransportSend
 *
 * @brief       This function is the definition of the physical transport API for sending a message.
 *
 * input parameters
 *
 * @param pBuf - Pointer to the buffer created with MT_TransportAlloc.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MT_TransportSend(uint8 *pBuf)
{
#if !defined CC2531ZNP
  if (ZNP_CFG1_UART == znpCfg1)
#endif
  {
    npMtUartSend(pBuf);
  }
#if !defined CC2531ZNP
  else
  {
    npMtSpiSend(pBuf);
  }
#endif
}

/**************************************************************************************************
 * @fn         npInit
 *
 * @brief      This function is the secondary initialization that resolves conflicts during
 *             osalInitTasks(). For example, since ZNP is the highest priority task, and
 *             specifically because the ZNP task is initialized before the ZDApp task, if znpInit()
 *             registers anything with ZDO_RegisterForZdoCB(), it is wiped out when ZDApp task
 *             initialization invokes ZDApp_InitZdoCBFunc().
 *             There may be other existing or future such races, so try to do all possible
 *             NP initialization here vice in znpInit().
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npInit(void)
{
  if (ZNP_CFG1_UART == znpCfg1)
  {
    halUARTCfg_t uartConfig;

    uartConfig.configured           = TRUE;
    uartConfig.baudRate             = ZNP_UART_BAUD;
#ifdef ZNP_ALT
    uartConfig.flowControl          = FALSE;
#else
    uartConfig.flowControl          = TRUE;
#endif
    uartConfig.flowControlThreshold = HAL_UART_FLOW_THRESHOLD;
    uartConfig.rx.maxBufSize        = HAL_UART_RX_BUF_SIZE;
    uartConfig.tx.maxBufSize        = HAL_UART_TX_BUF_SIZE;
    uartConfig.idleTimeout          = HAL_UART_IDLE_TIMEOUT;
    uartConfig.intEnable            = TRUE;
    uartConfig.callBackFunc         = npUartCback;
    HalUARTOpen(HAL_UART_PORT, &uartConfig);
    MT_UartRegisterTaskID(znpTaskId);

#ifdef HAL_PA_LNA_CC2592
   ZMacSetTransmitPower(TX_PWR_PLUS_19);
#else
   ZMacSetTransmitPower(TX_PWR_PLUS_4);
#endif 
  }
  else
  {
    /* npSpiInit() is called by hal_spi.c: HalSpiInit().*/
  }

  npInitNV();
#if defined (MT_ZDO_FUNC)
  MT_ZdoInit();
#endif
  MT_SysResetInd();
#if defined ZCL_KEY_ESTABLISH
#if defined TC_LINKKEY_JOIN
  zcl_TaskID = znpTaskId;
#endif
#endif
#if LQI_ADJUST
  ZMacLqiAdjustMode(LQI_ADJ_MODE1);
#endif
#if defined CC2531ZNP
  (void)osal_pwrmgr_task_state(znpTaskId, PWRMGR_HOLD);
#endif
}

/**************************************************************************************************
 * @fn         npInitNV
 *
 * @brief
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npInitNV(void)
{
  /* 4 x 2 bytes ZNP_NV_APP_ITEM_X */
  osal_nv_item_init(ZNP_NV_APP_ITEM_1, 2, NULL);
  osal_nv_item_init(ZNP_NV_APP_ITEM_2, 2, NULL);
  osal_nv_item_init(ZNP_NV_APP_ITEM_3, 2, NULL);
  osal_nv_item_init(ZNP_NV_APP_ITEM_4, 2, NULL);

  /* 2 x 16 bytes ZNP_NV_APP_ITEM_X */
  osal_nv_item_init(ZNP_NV_APP_ITEM_5, 16, NULL);
  osal_nv_item_init(ZNP_NV_APP_ITEM_6, 16, NULL);
}

#if defined MT_ZNP_FUNC
/**************************************************************************************************
 * @fn         npBasicRsp
 *
 * @brief      Generate the ZNP Basic Response message to the ZAP.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npBasicRsp(void)
{
  if (MT_ZnpBasicRsp() == false)
  {
    (void)osal_set_event(znpTaskId, ZNP_BASIC_RSP_EVENT);
  }
}
#endif

/**************************************************************************************************
 * @fn          npUartCback
 *
 * @brief       This function is the UART callback processor.
 *
 * input parameters
 *
 * @param port - The port being used for UART.
 * @param event - The reason for the callback.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npUartCback(uint8 port, uint8 event)
{
  switch (event) {
  case HAL_UART_RX_FULL:
  case HAL_UART_RX_ABOUT_FULL:
  case HAL_UART_RX_TIMEOUT:
    MT_UartProcessZToolData(port, znpTaskId);
    break;

  case HAL_UART_TX_EMPTY:
    osal_set_event(znpTaskId, ZNP_UART_TX_READY_EVENT);
    break;

  default:
    break;
  }
}

/**************************************************************************************************
 * @fn          npUartTxReady
 *
 * @brief       This function gets and writes the next chunk of data to the UART.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npUartTxReady(void)
{
  static uint16 npUartTxCnt = 0;
  static uint8 *npUartTxMsg = NULL;
  static uint8 *pMsg = NULL;

  if (!npUartTxMsg)
  {
    if ((pMsg = npUartTxMsg = osal_msg_dequeue(&npTxQueue)))
    {
      /* | SOP | Data Length | CMD |  DATA   | FSC |
       * |  1  |     1       |  2  | as dLen |  1  |
       */
      npUartTxCnt = pMsg[1] + MT_UART_FRAME_OVHD + MT_RPC_FRAME_HDR_SZ;
    }
  }

  if (npUartTxMsg)
  {
    uint16 len = HalUARTWrite(HAL_UART_PORT, pMsg, npUartTxCnt);
    npUartTxCnt -= len;

    if (npUartTxCnt == 0)
    {
      osal_msg_deallocate(npUartTxMsg);
      npUartTxMsg = NULL;
    }
    else
    {
      pMsg += len;
    }
  }
}

/**************************************************************************************************
 * @fn          npMtUartAlloc
 *
 * @brief       This function allocates a buffer for Txing on UART.
 *
 * input parameters
 *
 * @param cmd0 - The first byte of the MT command id containing the command type and subsystem.
 * @param len - Data length required.
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to the buffer obtained; possibly NULL if an allocation failed.
 **************************************************************************************************
 */
static uint8* npMtUartAlloc(uint8 cmd0, uint8 len)
{
  uint8 *p;

  if ((p = osal_msg_allocate(len + MT_RPC_FRAME_HDR_SZ + MT_UART_FRAME_OVHD)) != NULL)
  {
    return p + 1;
  }

  return NULL;
}

/**************************************************************************************************
 * @fn          npMtUartSend
 *
 * @brief       This function transmits or enqueues the buffer for transmitting on UART.
 *
 * input parameters
 *
 * @param pBuf - Pointer to the buffer to transmit on the UART.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npMtUartSend(uint8 *pBuf)
{
  uint8 len = pBuf[0] + MT_RPC_FRAME_HDR_SZ;

  pBuf[len] = MT_UartCalcFCS(pBuf, len);
  pBuf--;
  pBuf[0] = MT_UART_SOF;

  osal_msg_enqueue(&npTxQueue, pBuf);
  osal_set_event(znpTaskId, ZNP_UART_TX_READY_EVENT);
}

#if !defined CC2531ZNP
/**************************************************************************************************
 * @fn          npMtSpiAlloc
 *
 * @brief       This function gets or allocates a buffer for Txing on SPI.
 *
 * input parameters
 *
 * @param cmd0 - The first byte of the MT command id containing the command type and subsystem.
 * @param len - Data length required.
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to the buffer obtained; possibly NULL if an allocation failed.
 **************************************************************************************************
 */
static uint8* npMtSpiAlloc(uint8 cmd0, uint8 len)
{
  if ((cmd0 & MT_RPC_CMD_TYPE_MASK) == MT_RPC_CMD_SRSP)
  {
    return npSpiSRspAlloc(len);
  }
  else
  {
    return npSpiAReqAlloc(len);
  }
}

/**************************************************************************************************
 * @fn          npMtSpiSend
 *
 * @brief       This function transmits or enqueues the buffer for transmitting on SPI.
 *
 * input parameters
 *
 * @param pBuf - Pointer to the buffer to transmit on the SPI.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npMtSpiSend(uint8 *pBuf)
{
  if ((pBuf[1] & MT_RPC_CMD_TYPE_MASK) == MT_RPC_CMD_SRSP)
  {
    npSpiSRspReady(pBuf);
  }
  else
  {
    osal_msg_enqueue(&npTxQueue, pBuf);
    npSpiAReqReady();
  }
}

/**************************************************************************************************
 * @fn          npSpiPollCallback
 *
 * @brief       This function is called by the SPI driver when a POLL frame is received.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      A pointer to an OSAL message buffer containing the next AREQ frame to transmit,
 *              if any; NULL otherwise.
 **************************************************************************************************
 */
uint8* npSpiPollCallback(void)
{
  return osal_msg_dequeue(&npTxQueue);
}

/**************************************************************************************************
 * @fn          npSpiReadyCallback
 *
 * @brief       This function is called by the SPI driver to check if any data is ready to send.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if data is ready to send; FALSE otherwise.
 **************************************************************************************************
 */
bool npSpiReadyCallback(void)
{
  return !OSAL_MSG_Q_EMPTY(&npTxQueue);
}
#endif

/**************************************************************************************************
*/
