/***************************************************************************************************
  Filename:       MT_TASK.c
  Revised:        $Date: 2015-01-30 11:15:01 -0800 (Fri, 30 Jan 2015) $
  Revision:       $Revision: 42161 $

  Description:    MonitorTest Task handling routines

  Copyright 2007-2013 Texas Instruments Incorporated. All rights reserved.

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

 ***************************************************************************************************/

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "ZComDef.h"
#include "MT_TASK.h"
#include "MT.h"
#include "MT_DEBUG.h"
#include "MT_UART.h"
#include "MT_UTIL.h"
#include "MT_SYS.h"

#if !defined( NONWK )
#include "MT_ZDO.h"
#include "MT_AF.h"
#endif  /* NONWK */

#include "hal_uart.h"
#include "OSAL_Memory.h"

/***************************************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************************************/

static void MT_ProcessIncomingCommand( mtOSALSerialData_t *msg );
#ifdef MT_SRNG
void MT_ProcessSrngEvent(void); 
#endif

#if defined(NPI)
extern uint8_t npiframe_calcMTFCS(uint8_t *msg_ptr, uint8_t len);
#endif

/***************************************************************************************************
 * GLOBALS
 ***************************************************************************************************/

uint8 MT_TaskID;

/***************************************************************************************************
 * @fn      MT_TaskInit
 *
 * @brief  MonitorTest Task Initialization.  This function is put into the
 *         task table.
 *
 * @param   task_id - task ID of the MT Task
 *
 * @return  void
 ***************************************************************************************************/
void MT_TaskInit(uint8 task_id)
{
  MT_TaskID = task_id;

  /* Initialize the Serial port */
#if !defined(NPI)
  MT_UartInit();

  /* Register taskID - Do this after UartInit() because it will reset the taskID */
  MT_UartRegisterTaskID(task_id);
#endif /* NPI */
  osal_set_event(task_id, MT_SECONDARY_INIT_EVENT);
}

/**************************************************************************************************
 * @fn      MT_ProcessEvent
 *
 * @brief   MonitorTest Task Event Processor.  This task is put into the task table.
 *
 * @param   task_id - task ID of the MT Task
 * @param   events - event(s) for the MT Task
 *
 * @return  Bit mask of the unprocessed MT Task events.
 **************************************************************************************************/
UINT16 MT_ProcessEvent(uint8 task_id, uint16 events)
{
  /* Could be multiple events, so switch won't work */
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *msg_ptr = osal_msg_receive(task_id);

    if (msg_ptr != NULL)
    {
      MT_ProcessIncomingCommand((mtOSALSerialData_t *)msg_ptr);
      osal_msg_deallocate(msg_ptr);
    }

    /* Return unproccessed events */
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & MT_SECONDARY_INIT_EVENT )
  {
    MT_Init(task_id);
    /* Return unproccessed events */
    return (events ^ MT_SECONDARY_INIT_EVENT);
  }

  if ( events & MT_ZTOOL_SERIAL_RCV_BUFFER_FULL )
  {
    /* Return unproccessed events */
    return (events ^ MT_ZTOOL_SERIAL_RCV_BUFFER_FULL);
  }

#if !defined( NONWK )
  if ( events & MT_AF_EXEC_EVT )
  {
    MT_AfExec();
    return (events ^ MT_AF_EXEC_EVT);
  }
#endif  /* NONWK */

  /* Handle MT_SYS_OSAL_START_TIMER callbacks */
#if defined MT_SYS_FUNC
  if ( events & (MT_SYS_OSAL_EVENT_MASK))
  {
    if (events & MT_SYS_OSAL_EVENT_0)
    {
      MT_SysOsalTimerExpired(0x00);
      events ^= MT_SYS_OSAL_EVENT_0;
    }

    if (events & MT_SYS_OSAL_EVENT_1)
    {
      MT_SysOsalTimerExpired(0x01);
      events ^= MT_SYS_OSAL_EVENT_1;
    }

    if (events & MT_SYS_OSAL_EVENT_2)
    {
      MT_SysOsalTimerExpired(0x02);
      events ^= MT_SYS_OSAL_EVENT_2;
    }

    if (events & MT_SYS_OSAL_EVENT_3)
    {
      MT_SysOsalTimerExpired(0x03);
      events ^= MT_SYS_OSAL_EVENT_3;
    }

    return events;
  }
#endif

#ifdef MT_SRNG
  if(events & MT_SRNG_EVENT)
  {
    MT_ProcessSrngEvent(); 
    return (events ^ MT_SRNG_EVENT);
  }
#endif  

  /* Discard or make more handlers */
  return 0;

} /* MT_ProcessEvent() */

#ifdef MT_SRNG
/***************************************************************************************************
 * @fn      MT_ProcessSrngEvent
 *
 * @brief
 *
 *   Process SRNG Event Messages.
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
void MT_ProcessSrngEvent(void)
{
  uint8 * msg;
  osal_event_hdr_t * msg_ptr;
  
  msg_ptr = (osal_event_hdr_t *)osal_msg_allocate( MT_RPC_FRAME_HDR_SZ + 4 + sizeof(osal_event_hdr_t) );
  msg = msg_ptr + sizeof(osal_event_hdr_t) + 4;
  
  if ( msg_ptr )
  {
    msg_ptr->event = CMD_SERIAL_MSG;
    msg_ptr->status = 4;
    if(msg)
    {
      msg[MT_RPC_POS_LEN] = 4 + MT_RPC_FRAME_HDR_SZ;
      msg[MT_RPC_POS_CMD0] = MT_RPC_SYS_UTIL;
      msg[MT_RPC_POS_CMD1] = MT_UTIL_SRNG_GENERATE;
    } 
    osal_memcpy(msg_ptr + sizeof(osal_event_hdr_t), &msg, 4); 
  }
  osal_msg_send( MT_TaskID, (uint8 *)msg_ptr );
}
#endif

/***************************************************************************************************
 * @fn      MT_ProcessIncomingCommand
 *
 * @brief
 *
 *   Process Event Messages.
 *
 * @param   *msg - pointer to event message
 *
 * @return
 ***************************************************************************************************/
static void MT_ProcessIncomingCommand( mtOSALSerialData_t *msg )
{
  uint8 len, *msg_ptr = msg->msg;

  /* Use the first byte of the message as the command ID */
  switch ( msg->hdr.event )
  {
    case CMD_SERIAL_MSG:
      MT_ProcessIncoming(msg_ptr);
      break;

    case CMD_DEBUG_MSG:
      MT_ProcessDebugMsg( (mtDebugMsg_t *)msg );
      break;

    case CB_FUNC:
      /*
        Build SPI message here instead of redundantly calling MT_BuildSPIMsg
        because we have copied data already in the allocated message
      */

      /* msg_ptr is the beginning of the intended SPI message */
      len = SPI_0DATA_MSG_LEN + msg_ptr[DATALEN_FIELD];

      /*
        FCS goes to the last byte in the message and is calculated over all
        the bytes except FCS and SOP
      */
#if !defined(NPI)
      msg_ptr[len-1] = MT_UartCalcFCS(msg_ptr + 1, (uint8)(len-2));
#else
      msg_ptr[len-1] = npiframe_calcMTFCS(msg_ptr + 1, (uint8)(len-2));
#endif

#ifdef MT_UART_DEFAULT_PORT
      HalUARTWrite ( MT_UART_DEFAULT_PORT, msg_ptr, len );
#endif
      break;

    case CMD_DEBUG_STR:
      MT_ProcessDebugStr( (mtDebugStr_t *)msg );
      break;

#if !defined ( NONWK )
    case MT_SYS_APP_RSP_MSG:
      len = SPI_0DATA_MSG_LEN + msg_ptr[DATALEN_FIELD];
      MTProcessAppRspMsg( msg_ptr, len );
      break;
#endif  // NONWK

#if defined (MT_UTIL_FUNC)
#if defined ZCL_KEY_ESTABLISH
    case ZCL_KEY_ESTABLISH_IND:
      MT_UtilKeyEstablishInd((zclKE_StatusInd_t *)msg);
      break;
#endif
#endif
#ifdef MT_ZDO_CB_FUNC
    case ZDO_STATE_CHANGE:
      MT_ZdoStateChangeCB((osal_event_hdr_t *)msg);
      break;
#endif

    default:
      break;
  }
}

#ifdef MT_TASK
/***************************************************************************************************
 * @fn      MT_TransportAlloc
 *
 * @brief   Allocate memory for transport msg
 *
 * @param   uint8 cmd0 - The first byte of the MT command id containing the command type and subsystem.
 *          uint8 len - length
 *
 * @return  pointer the allocated memory or NULL if fail to allocate the memory
 ***************************************************************************************************/
uint8 *MT_TransportAlloc(uint8 cmd0, uint8 len)
{
  uint8 *p;

  (void)cmd0;  // Intentionally unreferenced parameter

  /* Allocate a buffer of data length + SOP+CMD+FCS (5 bytes) */
  p = osal_msg_allocate(len + SPI_0DATA_MSG_LEN);

  if (p)
  {
    p++; /* Save space for SOP_VALUE, msg structure */
    return p;
  }
  else
  {
    return NULL;
  }
}

/***************************************************************************************************
 * @fn      MT_TransportSend
 *
 * @brief   Fill in SOP and FCS then send out the msg
 *
 * @param   uint8 *pBuf - pointer to the message that contains CMD, length, data and FCS
 *
 * @return  None
 ***************************************************************************************************/
void MT_TransportSend(uint8 *pBuf)
{
  uint8 *msgPtr;
  uint8 dataLen = pBuf[0]; /* Data length is on byte #1 from the pointer */

  /* Move back to the SOP */
  msgPtr = pBuf-1;

  /* Insert SOP */
  msgPtr[0] = MT_UART_SOF;

  /* Insert FCS */
  msgPtr[SPI_0DATA_MSG_LEN - 1 + dataLen] = MT_UartCalcFCS (pBuf, (3 + dataLen));

  /* Send to UART */
#ifdef MT_UART_DEFAULT_PORT
  HalUARTWrite(MT_UART_DEFAULT_PORT, msgPtr, dataLen + SPI_0DATA_MSG_LEN);
#endif

  /* Deallocate */
  osal_msg_deallocate(msgPtr);
}
#endif /* MT_TASK */
/***************************************************************************************************
 ***************************************************************************************************/
