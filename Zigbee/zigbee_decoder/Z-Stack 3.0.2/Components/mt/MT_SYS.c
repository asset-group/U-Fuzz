/******************************************************************************
  Filename:       MT_SYS.c
  Revised:        $Date: 2015-02-09 19:10:05 -0800 (Mon, 09 Feb 2015) $
  Revision:       $Revision: 42469 $

  Description:   MonitorTest functions for SYS commands.

  Copyright 2007-2015 Texas Instruments Incorporated. All rights reserved.

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

 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "ZComDef.h"
#include "MT.h"
#include "MT_SYS.h"
#include "MT_VERSION.h"
#include "OSAL.h"
#include "OSAL_NV.h"
#include "Onboard.h"
#include "OSAL_Clock.h"
#include "mac_low_level.h"
#include "ZMAC.h"
#include "MT_UART.h"

#if !defined( CC26XX )
  #include "hal_adc.h"
#endif
#if !defined( CC253X_MACNP )
  #include "ZGlobals.h"
#endif
#if defined( FEATURE_NVEXID )
  #include "zstackconfig.h"
#endif
#if defined( FEATURE_DUAL_MAC )
  #include "dmmgr.h"
#endif
#if defined( FEATURE_SYSTEM_STATS )
#include "ZDiags.h"
#endif
#if defined( MT_SYS_JAMMER_FEATURE )
  #include "mac_rx.h"
  #include "mac_radio_defs.h"
#endif
#if (defined INCLUDE_REVISION_INFORMATION) && ((defined MAKE_CRC_SHDW) || (defined FAKE_CRC_SHDW)) //built for bootloader
  #include "hal_flash.h"
  #include "sb_shared.h"
#endif

/******************************************************************************
 * MACROS
 *****************************************************************************/

/* RPC_CMD responses for MT_SYS commands */
#define MT_ARSP_SYS ((uint8)MT_RPC_CMD_AREQ | (uint8)MT_RPC_SYS_SYS)
#define MT_SRSP_SYS ((uint8)MT_RPC_CMD_SRSP | (uint8)MT_RPC_SYS_SYS)

/* Max possible MT response length, limited by TX buffer and sizeof uint8 */
#define MT_MAX_RSP_LEN  ( MIN( MT_UART_DEFAULT_MAX_TX_BUFF, 255 ) )

/* Max possible MT response data length, MT protocol overhead */
#define MT_MAX_RSP_DATA_LEN  ( (MT_MAX_RSP_LEN - 1) - SPI_0DATA_MSG_LEN )

#define MT_SYS_DEVICE_INFO_RESPONSE_LEN 14

#if !defined HAL_GPIO || !HAL_GPIO
#define GPIO_DIR_IN(IDX)
#define GPIO_DIR_OUT(IDX)
#define GPIO_TRI(IDX)
#define GPIO_PULL_UP(IDX)
#define GPIO_PULL_DN(IDX)
#define GPIO_SET(IDX)
#define GPIO_CLR(IDX)
#define GPIO_TOG(IDX)
#define GPIO_GET(IDX) 0
#define GPIO_HiD_SET() (val = 0)
#define GPIO_HiD_CLR() (val = 0)
#endif

#if defined ( MT_SYS_SNIFFER_FEATURE )
#if defined ( HAL_MCU_CC2530 ) && !defined ( HAL_BOARD_CC2530USB )
  // This only works with CC253x chips
  #define HAL_BOARD_ENABLE_INTEGRATED_SNIFFER() st         \
  (                                                                                                                                                                                                                                   \
    OBSSEL3 = 0xFD;                                        \
    OBSSEL4 = 0xFC;                                        \
    RFC_OBS_CTRL1 = 0x09; /* 9 - sniff clk */              \
    RFC_OBS_CTRL2 = 0x08; /* 8 - sniff data */             \
    MDMTEST1 |= 0x04;                                      \
  )

  // This only works with CC253x chips
  #define HAL_BOARD_DISABLE_INTEGRATED_SNIFFER() st        \
  (                                                                                                                                                                                                                                   \
    OBSSEL3 &= ~0x80;                                                                                                                                                                             \
    OBSSEL4 &= ~0x80;                                                                                                                                                                             \
    RFC_OBS_CTRL1 = 0x00; /* 0 - constant value 0 to rfc_obs_sigs[1] */                                                                                   \
    RFC_OBS_CTRL2 = 0x00; /* 0 - constant value 0 to rfc_obs_sigs[2] */                                                                                   \
    MDMTEST1 &= ~0x04;                                                                                                                                                         \
  )
#else
  #define HAL_BOARD_ENABLE_INTEGRATED_SNIFFER() { status = FAILURE; }
  #define HAL_BOARD_DISABLE_INTEGRATED_SNIFFER() { status = FAILURE; }
#endif
#endif // MT_SYS_SNIFFER_FEATURE

#define RESET_HARD     0
#define RESET_SOFT     1
#define RESET_SHUTDOWN 2

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/

#if !defined( MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA )
#define MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA  FALSE
#endif

#if defined( MT_SYS_FUNC )
static const uint16 MT_SysOsalEventId[] =
{
  MT_SYS_OSAL_EVENT_0,
  MT_SYS_OSAL_EVENT_1,
  MT_SYS_OSAL_EVENT_2,
  MT_SYS_OSAL_EVENT_3
};
#endif

typedef enum {
  GPIO_DIR,
  GPIO_TRI,
  GPIO_SET,
  GPIO_CLR,
  GPIO_TOG,
  GPIO_GET,
  GPIO_HiD = 0x12
} GPIO_Op_t;

#if defined( MT_SYS_JAMMER_FEATURE )
  #define JAMMER_CHECK_EVT                           0x0001

  #if !defined( JAMMER_DETECT_CONTINUOUS_EVENTS )
    #define JAMMER_DETECT_CONTINUOUS_EVENTS          150
  #endif
  #if !defined( JAMMER_DETECT_PERIOD_TIME )
    #define JAMMER_DETECT_PERIOD_TIME                100  // In milliseconds
  #endif
  #if !defined( JAMMER_HIGH_NOISE_LEVEL )
    #define JAMMER_HIGH_NOISE_LEVEL                  -65
  #endif
#endif // MT_SYS_JAMMER_FEATURE

/******************************************************************************
 * EXTERNAL VARIABLES
 *****************************************************************************/
#if defined( FEATURE_NVEXID )
extern zstack_Config_t *pZStackCfg;
#endif /* FEATURE_NVEXID */

/******************************************************************************
 * LOCAL VARIABLES
 *****************************************************************************/
#if defined( MT_SYS_JAMMER_FEATURE )
static uint8 jammerTaskID;
static uint16 jammerContinuousEvents = JAMMER_DETECT_CONTINUOUS_EVENTS;
static uint16 jammerDetections = JAMMER_DETECT_CONTINUOUS_EVENTS;
static int8 jammerHighNoiseLevel = JAMMER_HIGH_NOISE_LEVEL;
static uint32 jammerDetectPeriodTime = JAMMER_DETECT_PERIOD_TIME;
#endif

#if defined( MT_SYS_SNIFFER_FEATURE )
static uint8 sniffer = FALSE;
#endif

/******************************************************************************
 * LOCAL FUNCTIONS
 *****************************************************************************/
#if defined( MT_SYS_FUNC )
static void MT_SysReset(uint8 *pBuf);
static void MT_SysPing(void);
static void MT_SysVersion(void);
static void MT_SysSetExtAddr(uint8 *pBuf);
static void MT_SysGetExtAddr(void);
static void MT_SysOsalStartTimer(uint8 *pBuf);
static void MT_SysOsalStopTimer(uint8 *pBuf);
static void MT_SysRandom(void);
static void MT_SysGpio(uint8 *pBuf);
static void MT_SysStackTune(uint8 *pBuf);
static void MT_SysSetUtcTime(uint8 *pBuf);
static void MT_SysGetUtcTime(void);
static void MT_SysSetTxPower(uint8 *pBuf);
#if !defined( CC26XX )
static void MT_SysAdcRead(uint8 *pBuf);
#endif /* !CC26xx */
#if !defined( CC253X_MACNP )
static void MT_SysOsalNVItemInit(uint8 *pBuf);
static void MT_SysOsalNVDelete(uint8 *pBuf);
static void MT_SysOsalNVLength(uint8 *pBuf);
static void MT_SysOsalNVRead(uint8 *pBuf);
static void MT_SysOsalNVWrite(uint8 *pBuf);
static uint8 MT_CheckNvId(uint16 nvId);
#if defined( FEATURE_NVEXID )
static void MT_SysNvCompact(uint8 *pBuf);
static void MT_SysNvCreate(uint8 *pBuf);
static void MT_SysNvDelete(uint8 *pBuf);
static void MT_SysNvLength(uint8 *pBuf);
static void MT_SysNvRead(uint8 *pBuf);
static void MT_SysNvWrite(uint8 *pBuf);
static uint8 MT_StackNvExtId( NVINTF_itemID_t *nvId );
static uint8 *MT_ParseNvExtId( uint8 *pBuf, NVINTF_itemID_t *nvId );
#endif /* FEATURE_NVEXID */
#endif /* !CC253X_MACNP */
#if defined( MT_SYS_JAMMER_FEATURE )
static void MT_SysJammerParameters( uint8 *pBuf );
#endif /* MT_SYS_JAMMER_FEATURE */
#if defined( MT_SYS_SNIFFER_FEATURE )
static void MT_SysSnifferParameters( uint8 *pBuf );
#endif /* MT_SYS_SNIFFER_FEATURE */
#if defined( FEATURE_SYSTEM_STATS )
static void MT_SysZDiagsInitStats(void);
static void MT_SysZDiagsClearStats(uint8 *pBuf);
static void MT_SysZDiagsGetStatsAttr(uint8 *pBuf);
static void MT_SysZDiagsRestoreStatsFromNV(void);
static void MT_SysZDiagsSaveStatsToNV(void);
#endif /* FEATURE_SYSTEM_STATS */
#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
static void powerOffSoc(void);
#endif /* ENABLE_MT_SYS_RESET_SHUTDOWN */
#endif /* MT_SYS_FUNC */

#if defined( MT_SYS_FUNC )
/******************************************************************************
 * @fn      MT_SysProcessing
 *
 * @brief   Process all the SYS commands that are issued by test tool
 *
 * @param   pBuf - pointer to the msg buffer
 *
 *          | LEN  | CMD0  | CMD1  |  DATA  |
 *          |  1   |   1   |   1   |  0-255 |
 *
 * @return  status
 *****************************************************************************/
uint8 MT_SysCommandProcessing(uint8 *pBuf)
{
  uint8 status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
    case MT_SYS_RESET_REQ:
      MT_SysReset(pBuf);
      break;

    case MT_SYS_PING:
      MT_SysPing();
      break;

    case MT_SYS_VERSION:
      MT_SysVersion();
      break;

    case MT_SYS_SET_EXTADDR:
      MT_SysSetExtAddr(pBuf);
      break;

    case MT_SYS_GET_EXTADDR:
      MT_SysGetExtAddr();
      break;

    case MT_SYS_OSAL_START_TIMER:
      MT_SysOsalStartTimer(pBuf);
      break;

    case MT_SYS_OSAL_STOP_TIMER:
      MT_SysOsalStopTimer(pBuf);
      break;

    case MT_SYS_RANDOM:
      MT_SysRandom();
      break;

#if !defined( CC26XX )
    case MT_SYS_ADC_READ:
      MT_SysAdcRead(pBuf);
      break;
#endif /* !CC26XX */

    case MT_SYS_GPIO:
      MT_SysGpio(pBuf);
      break;

    case MT_SYS_STACK_TUNE:
      MT_SysStackTune(pBuf);
      break;

    case MT_SYS_SET_TIME:
      MT_SysSetUtcTime(pBuf);
      break;

    case MT_SYS_GET_TIME:
      MT_SysGetUtcTime();
      break;

    case MT_SYS_SET_TX_POWER:
      MT_SysSetTxPower(pBuf);
      break;

// CC253X MAC Network Processor does not have NV support
#if !defined( CC253X_MACNP )
    case MT_SYS_OSAL_NV_DELETE:
      MT_SysOsalNVDelete(pBuf);
      break;

    case MT_SYS_OSAL_NV_ITEM_INIT:
      MT_SysOsalNVItemInit(pBuf);
      break;

    case MT_SYS_OSAL_NV_LENGTH:
      MT_SysOsalNVLength(pBuf);
      break;

    case MT_SYS_OSAL_NV_READ:
    case MT_SYS_OSAL_NV_READ_EXT:
      MT_SysOsalNVRead(pBuf);
      break;

    case MT_SYS_OSAL_NV_WRITE:
    case MT_SYS_OSAL_NV_WRITE_EXT:
      MT_SysOsalNVWrite(pBuf);
      break;

#if defined( FEATURE_NVEXID )
    case MT_SYS_NV_COMPACT:
      MT_SysNvCompact(pBuf);
      break;

    case MT_SYS_NV_CREATE:
      MT_SysNvCreate(pBuf);
      break;

    case MT_SYS_NV_DELETE:
      MT_SysNvDelete(pBuf);
      break;

    case MT_SYS_NV_LENGTH:
      MT_SysNvLength(pBuf);
      break;

    case MT_SYS_NV_READ:
      MT_SysNvRead(pBuf);
      break;

    case MT_SYS_NV_WRITE:
    case MT_SYS_NV_UPDATE:
      MT_SysNvWrite(pBuf);
      break;
#endif  /* FEATURE_NVEXID */
#endif  /* !CC253X_MACNP */

#if !defined( CC26XX )
#if defined( MT_SYS_JAMMER_FEATURE )
    case MT_SYS_JAMMER_PARAMETERS:
      MT_SysJammerParameters( pBuf );
      break;
#endif  /* MT_SYS_JAMMER_FEATURE */

#if defined( MT_SYS_SNIFFER_FEATURE )
    case MT_SYS_SNIFFER_PARAMETERS:
      MT_SysSnifferParameters( pBuf );
      break;
#endif  /* MT_SYS_SNIFFER_FEATURE */
#endif /* !CC26XX */

#if defined( FEATURE_SYSTEM_STATS )
    case MT_SYS_ZDIAGS_INIT_STATS:
      MT_SysZDiagsInitStats();
      break;

    case MT_SYS_ZDIAGS_CLEAR_STATS:
      MT_SysZDiagsClearStats(pBuf);
      break;

    case MT_SYS_ZDIAGS_GET_STATS:
      MT_SysZDiagsGetStatsAttr(pBuf);
       break;

    case MT_SYS_ZDIAGS_RESTORE_STATS_NV:
      MT_SysZDiagsRestoreStatsFromNV();
      break;

    case MT_SYS_ZDIAGS_SAVE_STATS_TO_NV:
      MT_SysZDiagsSaveStatsToNV();
      break;
#endif /* FEATURE_SYSTEM_STATS */

    default:
      status = MT_RPC_ERR_COMMAND_ID;
      break;
  }

  return status;
}

/******************************************************************************
 * @fn      MT_SysReset
 *
 * @brief   Reset the device.
 * @param   typID: 0=reset, 1=serial bootloader,
 *
 * @return  None
 *****************************************************************************/
void MT_SysReset( uint8 *pBuf )
{
  switch( pBuf[MT_RPC_POS_DAT0] )
  {
    case MT_SYS_RESET_HARD:
      SystemReset();
      break;

    case MT_SYS_RESET_SOFT:
#if !defined( HAL_BOARD_F5438 )
      SystemResetSoft();  // Especially useful for CC2531 to not break comm with USB Host.
#endif
      break;

    case MT_SYS_RESET_SHUTDOWN:
      {
#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
        // Disable interrupts and put into deep sleep, use hardware reset to wakeup
        powerOffSoc();
#endif
      }
      break;
  }
}

/******************************************************************************
 * @fn      MT_SysPing
 *
 * @brief   Process the Ping command
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysPing(void)
{
  uint16 tmp16;
  uint8 retArray[2];

  /* Build Capabilities */
  tmp16 = MT_CAP_SYS | MT_CAP_MAC  | MT_CAP_NWK  | MT_CAP_AF    |
          MT_CAP_ZDO | MT_CAP_SAPI | MT_CAP_UTIL | MT_CAP_DEBUG |
          MT_CAP_APP | MT_CAP_GP   | MT_CAP_ZOAD | MT_CAP_APP_CNF;

  /* Convert to high byte first into temp buffer */
  retArray[0] = LO_UINT16( tmp16 );
  retArray[1] = HI_UINT16( tmp16 );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_PING,
                                sizeof(retArray), retArray );
}

/******************************************************************************
 * @fn      MT_SysVersion
 *
 * @brief   Process the Version command
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysVersion(void)
{
#if !defined( INCLUDE_REVISION_INFORMATION )
  /* Build and send back the default response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_VERSION,
                                sizeof(MTVersionString),(uint8*)MTVersionString);
#else
  uint8 verStr[sizeof(MTVersionString) + 4];
  uint8 *pBuf = &verStr[sizeof(MTVersionString)];
#if (defined MAKE_CRC_SHDW) || (defined FAKE_CRC_SHDW)  //built for bootloader
  uint32 sblSig;
  uint32 sblRev;
#endif

  osal_memcpy(verStr, (uint8 *)MTVersionString, sizeof(MTVersionString));

#if (defined MAKE_CRC_SHDW) || (defined FAKE_CRC_SHDW)  //built for bootloader
  HalFlashRead(SBL_SIG_ADDR / HAL_FLASH_PAGE_SIZE,
               SBL_SIG_ADDR % HAL_FLASH_PAGE_SIZE,
               (uint8 *)&sblSig, sizeof(sblSig));

  if (sblSig == SBL_SIGNATURE)
  {
    // SBL is supported and its revision is provided (in a known flash location)
    HalFlashRead(SBL_REV_ADDR / HAL_FLASH_PAGE_SIZE,
                 SBL_REV_ADDR % HAL_FLASH_PAGE_SIZE,
                 (uint8 *)&sblRev, sizeof(sblRev));
  }
  else
  {
    //  SBL is supported but its revision is not provided
    sblRev = 0x00000000;
  }
#else
  // SBL is NOT supported
  sblRev = 0xFFFFFFFF;
#endif

  // Plug the SBL revision indication
  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,sblRev);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_VERSION,
                                sizeof(verStr), verStr);
#endif
}

/******************************************************************************
 * @fn      MT_SysSetExtAddr
 *
 * @brief   Set the Extended Address
 *
 * @param   pBuf
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSetExtAddr(uint8 *pBuf)
{
  uint8 retValue = ZFailure;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if ( ZMacSetReq(ZMacExtAddr, pBuf) == ZMacSuccess )
  {
// CC253X MAC Network Processor does not have NV support
#if defined(CC253X_MACNP)
    retValue = ZSuccess;
#else
    retValue = osal_nv_write(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, pBuf);
#endif
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SET_EXTADDR,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysGetExtAddr
 *
 * @brief   Get the Extended Address
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysGetExtAddr(void)
{
  uint8 extAddr[Z_EXTADDR_LEN];

  ZMacGetReq( ZMacExtAddr, extAddr );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_GET_EXTADDR,
                                sizeof(extAddr), extAddr);
}

#if !defined( CC253X_MACNP )
/******************************************************************************
 * @fn      MT_CheckNvId
 *
 * @brief   Check whether (ZigBee Stack) NV read should be blocked
 *
 * @param   nvId - NV item ID
 *
 * @return  'ZInvalidParameter' if blocked, otherwise 'ZSuccess'
 *****************************************************************************/
static uint8 MT_CheckNvId( uint16 nvId )
{
#if !MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
  if ((ZCD_NV_IMPLICIT_CERTIFICATE == nvId) ||
      (ZCD_NV_CA_PUBLIC_KEY == nvId)        ||
      (ZCD_NV_DEVICE_PRIVATE_KEY == nvId))
  {
    /* Access to Security Certificate Data is denied */
    return( ZInvalidParameter );
  }
#endif  /* MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA */

#if !MT_SYS_KEY_MANAGEMENT
  if ( (nvId == ZCD_NV_NWK_ACTIVE_KEY_INFO) ||
       (nvId == ZCD_NV_NWK_ALTERN_KEY_INFO) ||
      ((nvId >= ZCD_NV_TCLK_TABLE_START) && (nvId <= ZCD_NV_TCLK_TABLE_END)) ||
      ((nvId >= ZCD_NV_APS_LINK_KEY_DATA_START) && (nvId <= ZCD_NV_APS_LINK_KEY_DATA_END)) ||
       (nvId == ZCD_NV_PRECFGKEY) )
  {
    /* Access to Security Key Data is denied */
    return( ZInvalidParameter );
  }
#endif  /* !MT_SYS_KEY_MANAGEMENT */

  return( ZSuccess );
}

/******************************************************************************
 * @fn      MT_SysOsalNVRead
 *
 * @brief   Attempt to read an NV value
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVRead(uint8 *pBuf)
{
  uint8 error;
  uint8 cmdId;
  uint16 nvId;
  uint16 dataLen;
  uint16 dataOfs;
  uint16 nvItemLen;

  /* MT command ID */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* NV item ID */
  nvId = osal_build_uint16( pBuf );

#if defined( ZCD_NV_POLL_RATE_OLD16 )
  if( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    // This ID shouldn't exist anymore, it was converted to the new size and ID
    // then deleted during initialization. But, a read of this item will
    // read the new item and convert the size and return the size expected.
    uint32 pollRate;
    uint16 *pOldPollRate;
    uint8 respBuf[4];
    uint8 respLen;
    // Convert from old uint16 NV item to the new uint32 NV item
    if ( osal_nv_read( ZCD_NV_POLL_RATE, 0, sizeof( uint32 ), &pollRate ) == ZSUCCESS )
    {
      respBuf[0] = ZSuccess;
      respBuf[1] = sizeof( uint16 );
      pOldPollRate = (uint16 *)&respBuf[2];
      *pOldPollRate = (uint16)pollRate;
      respLen = 4;
    }
    else
    {
      respBuf[0] = ZFailure;
      respBuf[1] = 0;
      respLen = 2;
    }
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_READ,
                                  respLen, respBuf );
    return;
  }
#endif

  /* Check whether read-access to this item is allowed */
  error = MT_CheckNvId( nvId );

  /* Get NV data offset */
  if( cmdId == MT_SYS_OSAL_NV_READ )
  {
    /* MT_SYS_OSAL_NV_READ has 1-byte offset */
    dataOfs = (uint16)pBuf[2];
  }
  else
  {
    /* MT_SYS_OSAL_NV_READ_EXT has 2-byte offset */
    dataOfs = osal_build_uint16( pBuf+2 );
  }

  /* Length of entire NV item data */
  nvItemLen = osal_nv_item_len( nvId );
  if( nvItemLen <= dataOfs )
  {
    /* Offset is past end of data */
    error = ZInvalidParameter;
  }

  if( error == ZSuccess )
  {
    uint8 *pRetBuf;
    uint8 respLen = 2;  /* Response header: [0]=status,[1]=length */

    dataLen = nvItemLen - dataOfs;
    if (dataLen > (uint16)(MT_MAX_RSP_DATA_LEN - respLen))
    {
      /* Data length is limited by TX buffer size and MT protocol */
      dataLen = (MT_MAX_RSP_DATA_LEN - respLen);
    }
    respLen += dataLen;

    pRetBuf = osal_mem_alloc(respLen);
    if( pRetBuf != NULL )
    {
      osal_memset(&pRetBuf[2], 0, dataLen);
      if (((osal_nv_read( nvId, dataOfs, dataLen, &pRetBuf[2] )) == ZSUCCESS))
      {
        pRetBuf[0] = ZSuccess;
        pRetBuf[1] = dataLen;
        MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId,
                                      respLen, pRetBuf );
      }
      else
      {
        error = NV_OPER_FAILED;
      }
      osal_mem_free(pRetBuf);
    }
    else
    {
      /* Could not get buffer for NV data */
      error = ZMemError;
    }
  }

  if( error != ZSuccess )
  {
    uint8 tmp[2] = { error, 0 };
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId,
                                  sizeof(tmp), tmp);
  }
}

/******************************************************************************
 * @fn      MT_SysOsalNVWrite
 *
 * @brief   Attempt to write an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVWrite(uint8 *pBuf)
{
  uint8 cmdId;
  uint16 nvId;
  uint16 dataLen;
  uint16 dataOfs;
  uint16 nvItemLen;
  uint8 rtrn = ZSuccess;

  /* MT command ID */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* NV item ID */
  nvId = osal_build_uint16( pBuf );

  /* Get NV data offset & length */
  if ( cmdId == MT_SYS_OSAL_NV_WRITE )
  {
    /* MT_SYS_OSAL_NV_WRITE has 1-byte offset & length */
    dataOfs = (uint16)pBuf[2];
    dataLen = (uint16)pBuf[3];
    pBuf += 4;
  }
  else
  {
    /* MT_SYS_OSAL_NV_WRITE_EXT has 2-byte offset & length */
    dataOfs = osal_build_uint16( pBuf+2 );
    dataLen = osal_build_uint16( pBuf+4 );
    pBuf += 6;
  }

#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    // This ID shouldn't exist anymore, it was converted to the new size and ID
    // then deleted during initialization.  But a write to this item will
    // convert the 16 bits to the new 32 bits and write that value to the new
    // NV item.
    uint32 pollRate;
    uint16 *pOldPollRate = (uint16 *)pBuf;
    uint16 oldPollRate = *pOldPollRate;
    nvId = ZCD_NV_POLL_RATE;
    nvItemLen = sizeof ( uint32 );
    pollRate = (uint32)oldPollRate;
    pBuf = (uint8 *)&pollRate;
  }
#endif

  /* Length of entire NV item data */
  nvItemLen = osal_nv_item_len(nvId);
  if ((dataOfs + dataLen) <= nvItemLen)
  {
    if (dataOfs == 0)
    {
      /* Set the Z-Globals value of this NV item */
      zgSetItem( nvId, dataLen, pBuf );
    }

    if ((osal_nv_write(nvId, dataOfs, dataLen, pBuf)) == ZSUCCESS)
    {
      if (nvId == ZCD_NV_EXTADDR)
      {
        rtrn = ZMacSetReq(ZMacExtAddr, pBuf);
      }
    }
    else
    {
      rtrn = NV_OPER_FAILED;
    }
  }
  else
  {
    /* Bad length or/and offset */
    rtrn = ZInvalidParameter;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId,
                                sizeof(rtrn), &rtrn);
}

/******************************************************************************
 * @fn      MT_SysOsalNVItemInit
 *
 * @brief   Attempt to create an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVItemInit(uint8 *pBuf)
{
  uint8 ret;
  uint8 idLen;
  uint16 nvId;
  uint16 nvLen;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* NV item ID */
  nvId = osal_build_uint16( pBuf );
  /* NV item length */
  nvLen = osal_build_uint16( pBuf+2 );
  /* Initialization data length */
  idLen = pBuf[4];
  pBuf += 5;

#if defined( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    /* This item shouldn't exist anymore.  Read and write will convert
     * to the new NV item, so return Success.
     */
    ret = ZSuccess;
  }
  else
#endif
  {
    if ( idLen < nvLen )
    {
      /* Attempt to create a new NV item */
      ret = osal_nv_item_init( nvId, nvLen, NULL );
      if ( (ret == NV_ITEM_UNINIT) && (idLen > 0) )
      {
        /* Write initialization data to first part of new item */
        (void) osal_nv_write( nvId, 0, (uint16)idLen, pBuf );
      }
    }
    else
    {
      /* Attempt to create/initialize a new NV item */
      ret = osal_nv_item_init( nvId, nvLen, pBuf );
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_ITEM_INIT,
                                sizeof(ret), &ret);
}

/******************************************************************************
 * @fn      MT_SysOsalNVDelete
 *
 * @brief   Attempt to delete an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVDelete(uint8 *pBuf)
{
  uint16 nvId;
  uint16 nvLen;
  uint8 ret;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Get the ID */
  nvId = osal_build_uint16( pBuf );
  /* Get the length */
  nvLen = osal_build_uint16( pBuf+2 );

#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    /* This item shouldn't exist anymore.  Read and write will convert
     * to the new NV item, so return Success.
     */
    ret = ZSuccess;
  }
  else
#endif
  {
    /* Attempt to delete the NV item */
    ret = osal_nv_delete( nvId, nvLen );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_DELETE,
                                sizeof(ret), &ret);
}

/******************************************************************************
 * @fn      MT_SysOsalNVLength
 *
 * @brief   Attempt to get the length to an NV item
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalNVLength(uint8 *pBuf)
{
  uint16 nvId;
  uint16 nvLen;
  uint8 rsp[2];

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Get the ID */
  nvId = osal_build_uint16( pBuf );

#if defined ( ZCD_NV_POLL_RATE_OLD16 )
  if ( nvId == ZCD_NV_POLL_RATE_OLD16 )
  {
    /* Ignore this item and force return  */
    nvLen = 0;
  }
  else
#endif
  {
    /* Attempt to get NV item length */
    nvLen = osal_nv_item_len( nvId );
  }

  rsp[0] = LO_UINT16( nvLen );
  rsp[1] = HI_UINT16( nvLen );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_NV_LENGTH,
                                sizeof(rsp), rsp);
}

#if defined( FEATURE_NVEXID )
/******************************************************************************
 * @fn      MT_ParseNvExtId
 *
 * @brief   Parse the incoming NV ID parameters
 *
 * @param   pBuf - pointer to incoming data
 * @param   nvId - pointer to outgoing NV ID
 *
 * @return  pointer to next incoming data byte
 *****************************************************************************/
static uint8 *MT_ParseNvExtId( uint8 *pBuf, NVINTF_itemID_t *nvId )
{
  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  nvId->systemID = pBuf[0];
  nvId->itemID = osal_build_uint16( pBuf+1 );
  nvId->subID = osal_build_uint16( pBuf+3 );

  return( pBuf + 5 );
}

/******************************************************************************
 * @fn      MT_StackNvExtId
 *
 * @brief   Check whether extended NV ID is from ZigBee Stack
 *
 * @param   nvId - pointer to extended NV ID
 *
 * @return  TRUE if ZigBee Stack NV item, otherwise FALSE
 *****************************************************************************/
static uint8 MT_StackNvExtId( NVINTF_itemID_t *nvId )
{
  return( (nvId->systemID == NVINTF_SYSID_ZSTACK) && (nvId->itemID == 0) );
}

/******************************************************************************
 * @fn      MT_SysNvCompact
 *
 * @brief   Attempt to compact the active NV page
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvCompact(uint8 *pBuf)
{
  uint8 retVal;

  if (( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.compactNV == NULL ))
  {
    /* NV item compact function not available */
    retVal = NVINTF_NOTREADY;
  }
  else
  {
    uint16 minSize;

    /* Skip over RPC header */
    pBuf += MT_RPC_FRAME_HDR_SZ;

    /* Get the remaining size threshold */
    minSize = osal_build_uint16( pBuf );

    /* Attempt to compact the active NV page */
    retVal = pZStackCfg->nvFps.compactNV( minSize );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_COMPACT,
                                sizeof(retVal), &retVal);
}

/******************************************************************************
 * @fn      MT_SysNvCreate
 *
 * @brief   Attempt to create an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvCreate(uint8 *pBuf)
{
  uint8 retVal;

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.createItem == NULL ))
  {
    /* NV item create function not available */
    retVal = NVINTF_NOTREADY;
  }
  else
  {
    uint32 nvLen;
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    pBuf = MT_ParseNvExtId( pBuf, &nvId );

    /* Get the length */
    nvLen = osal_build_uint32( pBuf, sizeof(nvLen) );

    /* Attempt to create the specified item with no initial data */
    retVal = pZStackCfg->nvFps.createItem( nvId, nvLen, NULL );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_CREATE,
                                sizeof(retVal), &retVal);
}

/******************************************************************************
 * @fn      MT_SysNvDelete
 *
 * @brief   Attempt to delete an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvDelete(uint8 *pBuf)
{
  uint8 retVal;

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.deleteItem == NULL ))
  {
    /* NV item delete function not available */
    retVal = NVINTF_NOTREADY;
  }
  else
  {
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    MT_ParseNvExtId( pBuf, &nvId );

    /* Attempt to delete the specified item */
    retVal = pZStackCfg->nvFps.deleteItem( nvId );
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_DELETE,
                                sizeof(retVal), &retVal);
}

/******************************************************************************
 * @fn      MT_SysNvLength
 *
 * @brief   Attempt to delete an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvLength(uint8 *pBuf)
{
  uint32 nvLen;
  uint8 retBuf[4];

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.getItemLen == NULL ))
  {
    /* NV item length function not available */
    nvLen = 0;
  }
  else
  {
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    MT_ParseNvExtId( pBuf, &nvId );

    /* Attempt to get length of the specified item */
    nvLen = pZStackCfg->nvFps.getItemLen( nvId );
  }

  /* Serialize the length bytes */
  osal_buffer_uint32( retBuf, nvLen );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_LENGTH,
                                sizeof(retBuf), retBuf);
}

/******************************************************************************
 * @fn      MT_SysNvRead
 *
 * @brief   Attempt to read an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvRead(uint8 *pBuf)
{
  uint8 error;

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.readItem == NULL ))
  {
    /* NV item length/read function not available */
    error = NVINTF_NOTREADY;
  }
  else
  {
    uint8 dataLen;
    uint16 dataOfs;
    uint8 *pRetBuf;
    uint8 respLen = 2;  /* Response header: [0]=status,[1]=length */
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    pBuf = MT_ParseNvExtId( pBuf, &nvId );

    if( MT_StackNvExtId(&nvId) == TRUE )
    {
      /* Check whether read-access to this ZigBee Stack item is allowed */
      if( MT_CheckNvId( nvId.subID ) != ZSuccess )
      {
        /* Convert to NVINTF error code */
        error = NVINTF_BADSUBID;
      }
    }
    else
    {
      /* It's OK to read this item */
      error = ZSuccess;
    }

    /* Get the read data offset */
    dataOfs = osal_build_uint16( pBuf );

    /* And the read data length */
    dataLen = pBuf[2];

    if( dataLen > (MT_MAX_RSP_DATA_LEN - respLen) )
    {
      /* Data length is limited by TX buffer size and MT protocol */
      dataLen = (MT_MAX_RSP_DATA_LEN - respLen);
    }
    respLen += dataLen;

    pRetBuf = osal_mem_alloc(respLen);
    if( pRetBuf != NULL )
    {
      /* Attempt to read data from the specified item */
      error = pZStackCfg->nvFps.readItem( nvId, dataOfs, dataLen, pRetBuf+2 );
      if( error == NVINTF_SUCCESS )
      {
        pRetBuf[0] = ZSuccess;
        pRetBuf[1] = dataLen;
        MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_READ,
                                      respLen, pRetBuf );
      }
      osal_mem_free(pRetBuf);
    }
    else
    {
      /* Could not get buffer for NV data */
      error = ZMemError;
    }
  }

  if( error != ZSuccess )
  {
    uint8 tmp[2] = { error, 0 };
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_NV_READ,
                                  sizeof(tmp), tmp );
  }
}

/******************************************************************************
 * @fn      MT_SysNvWrite
 *
 * @brief   Attempt to write an NV item (extended item ID)
 *
 * @param   pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysNvWrite(uint8 *pBuf)
{
  uint8 cmdId;
  uint8 error;

  /* MT command ID */
  cmdId = pBuf[MT_RPC_POS_CMD1];

  if(( pZStackCfg == NULL ) || ( pZStackCfg->nvFps.writeItem == NULL ))
  {
    /* NV item length/read function not available */
    error = NVINTF_NOTREADY;
  }
  else
  {
    uint8 dataLen;
    uint16 dataOfs = 0;
    NVINTF_itemID_t nvId;

    /* Get the NV ID parameters */
    pBuf = MT_ParseNvExtId( pBuf, &nvId );

    if( cmdId == MT_SYS_NV_WRITE )
    {
      /* Get data offset for Write command */
      dataOfs = osal_build_uint16( pBuf );
      pBuf += 2;
    }

    /* Get the write data length */
    dataLen = pBuf[0];
    pBuf += 1;

    if( (dataOfs == 0) && (MT_StackNvExtId(&nvId) == TRUE) )
    {
      /* Set the Z-Globals value of this NV item */
      zgSetItem( nvId.subID, dataLen, pBuf );

      if( nvId.subID == ZCD_NV_EXTADDR )
      {
        /* Give MAC the new 64-bit address */
        ZMacSetReq( ZMacExtAddr, pBuf );
      }
    }

    if( cmdId == MT_SYS_NV_UPDATE )
    {
      /* Attempt to update (create) data to the specified item */
      error = pZStackCfg->nvFps.writeItem( nvId, dataLen, pBuf );
    }
    else
    {
      /* Attempt to write data (existing) to the specified item */
      error = pZStackCfg->nvFps.writeItemEx( nvId, dataOfs, dataLen, pBuf );
    }
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, cmdId, sizeof(error), &error);
}
#endif  /* FEATURE_NVEXID */
#endif  /* !CC253X_MACNP */

/******************************************************************************
 * @fn      MT_SysOsalStartTimer
 *
 * @brief
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalStartTimer(uint8 *pBuf)
{
  uint8 retValue;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (*pBuf <= 3)
  {
    uint16 timer = osal_build_uint16( pBuf+1 );
    uint16 eventId = (uint16)MT_SysOsalEventId[pBuf[0]];

    retValue = osal_start_timerEx(MT_TaskID, eventId, timer);
  }
  else
  {
    retValue = ZInvalidParameter;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_START_TIMER,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysOsalStopTimer
 *
 * @brief
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysOsalStopTimer(uint8 *pBuf)
{
  uint16 eventId;
  uint8 retValue = ZFailure;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  if (*pBuf <= 3)
  {
    eventId = (uint16) MT_SysOsalEventId[*pBuf];
    retValue = osal_stop_timerEx(MT_TaskID, eventId);
  }
  else
  {
    retValue = ZInvalidParameter;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_OSAL_STOP_TIMER,
                                sizeof(retValue), &retValue );
}

/******************************************************************************
 * @fn      MT_SysRandom
 *
 * @brief
 *
 * @param   uint8 pData - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysRandom()
{
  uint16 randValue = Onboard_rand();
  uint8 retArray[2];

  retArray[0] = LO_UINT16(randValue);
  retArray[1] = HI_UINT16(randValue);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_RANDOM,
                                sizeof(retArray), retArray );
}

#if !defined( CC26XX )
/******************************************************************************
 * @fn      MT_SysAdcRead
 *
 * @brief   Reading ADC value, temperature sensor and voltage
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysAdcRead(uint8 *pBuf)
{
  uint16 tempValue = 0;
  uint8 retArray[2];

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  {
    uint8 channel = *pBuf++;  /* ADC channel */
    uint8 resolution = *pBuf++;  /* ADC resolution */

    /* Voltage reading */
    switch (channel)
    {
      case HAL_ADC_CHANNEL_TEMP:  /* Temperature sensor */
      case HAL_ADC_CHANNEL_VDD:   /* Voltage reading */
        resolution = HAL_ADC_RESOLUTION_14;

      case HAL_ADC_CHANNEL_0:  /* Analog input channels */
      case HAL_ADC_CHANNEL_1:
      case HAL_ADC_CHANNEL_2:
      case HAL_ADC_CHANNEL_3:
      case HAL_ADC_CHANNEL_4:
      case HAL_ADC_CHANNEL_5:
      case HAL_ADC_CHANNEL_6:
      case HAL_ADC_CHANNEL_7:
        tempValue = HalAdcRead(channel, resolution);
        break;

      default:  /* Undefined channel */
        break;
    }
  }

  retArray[0] = LO_UINT16(tempValue);
  retArray[1] = HI_UINT16(tempValue);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ADC_READ,
                                sizeof(retArray), retArray);
}
#endif /* !CC26XX */

/******************************************************************************
 * @fn      MT_SysGpio
 *
 * @brief   ZAccel RPC interface for controlling the available GPIO pins.
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysGpio(uint8 *pBuf)
{
  uint8 val;
  GPIO_Op_t op;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  op = (GPIO_Op_t)(*pBuf++);
  val = *pBuf;

  switch (op)
  {
    case GPIO_DIR:
      if (val & BV(0)) {GPIO_DIR_OUT(0);} else {GPIO_DIR_IN(0);}
      if (val & BV(1)) {GPIO_DIR_OUT(1);} else {GPIO_DIR_IN(1);}
      if (val & BV(2)) {GPIO_DIR_OUT(2);} else {GPIO_DIR_IN(2);}
      if (val & BV(3)) {GPIO_DIR_OUT(3);} else {GPIO_DIR_IN(3);}
      break;

    case GPIO_TRI:
      if(val & BV(0)) {GPIO_TRI(0);} else if(val & BV(4)) {GPIO_PULL_DN(0);} else {GPIO_PULL_UP(0);}
      if(val & BV(1)) {GPIO_TRI(1);} else if(val & BV(5)) {GPIO_PULL_DN(1);} else {GPIO_PULL_UP(1);}
      if(val & BV(2)) {GPIO_TRI(2);} else if(val & BV(6)) {GPIO_PULL_DN(2);} else {GPIO_PULL_UP(2);}
      if(val & BV(3)) {GPIO_TRI(3);} else if(val & BV(7)) {GPIO_PULL_DN(3);} else {GPIO_PULL_UP(3);}
      break;

    case GPIO_SET:
      if (val & BV(0)) {GPIO_SET(0);}
      if (val & BV(1)) {GPIO_SET(1);}
      if (val & BV(2)) {GPIO_SET(2);}
      if (val & BV(3)) {GPIO_SET(3);}
      break;

    case GPIO_CLR:
      if (val & BV(0)) {GPIO_CLR(0);}
      if (val & BV(1)) {GPIO_CLR(1);}
      if (val & BV(2)) {GPIO_CLR(2);}
      if (val & BV(3)) {GPIO_CLR(3);}
      break;

    case GPIO_TOG:
      if (val & BV(0)) {GPIO_TOG(0);}
      if (val & BV(1)) {GPIO_TOG(1);}
      if (val & BV(2)) {GPIO_TOG(2);}
      if (val & BV(3)) {GPIO_TOG(3);}
      break;

    case GPIO_GET:
      break;

    case GPIO_HiD:
      (val) ? GPIO_HiD_SET() :  GPIO_HiD_CLR();
      break;

    default:
      break;
  }

  val  = (GPIO_GET(0)) ? BV(0) : 0;
  val |= (GPIO_GET(1)) ? BV(1) : 0;
  val |= (GPIO_GET(2)) ? BV(2) : 0;
  val |= (GPIO_GET(3)) ? BV(3) : 0;

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_GPIO,
                                sizeof(val), &val);
}

/******************************************************************************
 * @fn      MT_SysStackTune
 *
 * @brief   RPC interface for tuning the stack parameters to adjust performance
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysStackTune(uint8 *pBuf)
{
  uint8 rtrn;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  switch (*pBuf++)
  {
  case STK_TX_PWR:
    rtrn = ZMacSetReq(ZMacPhyTransmitPowerSigned, pBuf);
    break;

  case STK_RX_ON_IDLE:
    if ((*pBuf != TRUE) && (*pBuf != FALSE))
    {
      (void)ZMacGetReq(ZMacRxOnIdle, &rtrn);
    }
    else
    {
      rtrn = ZMacSetReq(ZMacRxOnIdle, pBuf);
    }
    break;

  default:
    rtrn = ZInvalidParameter;
    break;
  }

  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_STACK_TUNE,
                                sizeof(rtrn), &rtrn);
}

/******************************************************************************
 * @fn      MT_SysSetUtcTime
 *
 * @brief   Set the OSAL UTC Time. UTC rollover is: 06:28:16 02/07/2136
 *
 * @param   pBuf - pointer to time parameters
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSetUtcTime(uint8 *pBuf)
{
  uint8 retStat;
  UTCTime utcSecs;

  /* Skip over RPC header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  utcSecs = osal_build_uint32( pBuf, 4 );
  if ( utcSecs == 0 )
  {
    UTCTimeStruct utc;

    /* Skip past UTC time */
    pBuf += 4;

    /* Get time and date parameters */
    utc.hour = *pBuf++;
    utc.minutes = *pBuf++;
    utc.seconds = *pBuf++;
    utc.month = (*pBuf++) - 1;
    utc.day = (*pBuf++) - 1;
    utc.year = osal_build_uint16 ( pBuf );

    if ((utc.hour < 24) && (utc.minutes < 60) && (utc.seconds < 60) &&
        (utc.month < 12) && (utc.day < 31) && (utc.year > 1999) && (utc.year < 2136))
    {
      /* Got past the course filter, now check for leap year */
      if ((utc.month != 1) || (utc.day < (IsLeapYear( utc.year ) ? 29 : 28)))
      {
        /* Numbers look reasonable, convert to UTC */
        utcSecs = osal_ConvertUTCSecs( &utc );
      }
    }
  }

  if ( utcSecs == 0 )
  {
    /* Bad parameter(s) */
    retStat = ZInvalidParameter;
  }
  else
  {
    /* Parameters accepted, set the time */
    osal_setClock( utcSecs );
    retStat = ZSuccess;
  }

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SET_TIME,
                                sizeof(retStat), &retStat);
}

/******************************************************************************
 * @fn      MT_SysGetUtcTime
 *
 * @brief   Get the OSAL UTC time
 *
 * @param   None
 *
 * @return  32-bit and Parsed UTC time
 *****************************************************************************/
static void MT_SysGetUtcTime(void)
{
  uint8 len;
  uint8 *buf;

  len = sizeof( UTCTime ) + sizeof( UTCTimeStruct );

  buf = osal_mem_alloc( len );
  if ( buf )
  {
    uint8 *pBuf;
    UTCTime utcSecs;
    UTCTimeStruct utcTime;

    // Get current 32-bit UTC time and parse it
    utcSecs = osal_getClock();
    osal_ConvertUTCTime( &utcTime, utcSecs );

    // Start with 32-bit UTC time
    pBuf = osal_buffer_uint32( buf, utcSecs );

    // Concatenate parsed UTC time fields
    *pBuf++ = utcTime.hour;
    *pBuf++ = utcTime.minutes;
    *pBuf++ = utcTime.seconds;
    *pBuf++ = utcTime.month + 1;  // Convert to human numbers
    *pBuf++ = utcTime.day + 1;
    *pBuf++ = LO_UINT16( utcTime.year );
    *pBuf++ = HI_UINT16( utcTime.year );

    /* Build and send back the response */
    MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_GET_TIME,
                                 (uint8)(pBuf-buf), buf);

    osal_mem_free( buf );
  }
}

/******************************************************************************
 * @fn      MT_SysSetTxPower
 *
 * @brief   Set the transmit power.
 *
 * @param   pBuf - MT message containing the ZMacTransmitPower_t power level to set.
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSetTxPower(uint8 *pBuf)
{
  /* A local variable to hold the signed dBm value of TxPower that is being requested. */
  uint8 signed_dBm_of_TxPower_requeseted;

  /*
   * A local variable to hold the signed dBm value of TxPower that can be set which is closest to
   * the requested dBm value of TxPower, but which is also valid according to a complex set of
   * compile-time and run-time configuration which is interpreted by the macRadioSetTxPower()
   * function.
   */
  uint8 signed_dBm_of_TxPower_range_corrected;

  /* Parse the requested dBm from the RPC message. */
  signed_dBm_of_TxPower_requeseted = pBuf[MT_RPC_POS_DAT0];

  /*
   * MAC_MlmeSetReq() will store an out-of-range dBm parameter value into the NIB. So it is not
   * possible to learn the actual dBm value that will be set by invoking MACMlmeGetReq().
   * But this actual dBm value is a required return value in the SRSP to this SREQ. Therefore,
   * it is necessary to make this redundant pre-call to macRadioSetTxPower() here in order to run
   * the code that will properly constrain the requested dBm to a valid range based on both the
   * compile-time and the run-time configurations that affect the available valid ranges
   * (i.e. MAC_MlmeSetReq() itself will invoke for a second time the macRadioSetTxPower() function).
   */
  signed_dBm_of_TxPower_range_corrected = macRadioSetTxPower(signed_dBm_of_TxPower_requeseted);

  /*
   * Call the function to store the requested dBm in the MAC PIB and to set the TxPower as closely
   * as possible within the TxPower range that is valid for the compile-time and run-time
   * configuration.
   */
  (void)MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &signed_dBm_of_TxPower_requeseted);

  /* Send back response that includes the actual dBm TxPower that can be set. */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SET_TX_POWER, 1,
                                &signed_dBm_of_TxPower_range_corrected);
}

#if defined ( FEATURE_SYSTEM_STATS )
/******************************************************************************
 * @fn      MT_SysZDiagsInitStats
 *
 * @brief   Initialize the statistics table in NV or restore values from
 *          NV into the Statistics table in RAM
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsInitStats(void)
{
  uint8 retValue;

  retValue = ZDiagsInitStats();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_INIT_STATS,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysZDiagsClearStats
 *
 * @brief   Clears the statistics table in RAM and NV if option flag set.
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsClearStats(uint8 *pBuf)
{
  uint32 sysClock;
  uint8 retBuf[4];

  /* parse header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* returns the system clock of the time when the statistics were cleared */
  sysClock = ZDiagsClearStats( *pBuf );

  osal_buffer_uint32( retBuf, sysClock );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_CLEAR_STATS,
                                sizeof(retBuf), retBuf);
}

/******************************************************************************
 * @fn      MT_SysZDiagsGetStatsAttr
 *
 * @brief   Reads specific system (attribute) ID statistics and/or metrics.
 *
 * @param   uint8 pBuf - pointer to the data
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsGetStatsAttr(uint8 *pBuf)
{
  uint16 attrId;
  uint32 attrValue;
  uint8 retBuf[4];

  /* parse header */
  pBuf += MT_RPC_FRAME_HDR_SZ;

  /* Get the Attribute ID */
  attrId = osal_build_uint16( pBuf );

  attrValue = ZDiagsGetStatsAttr( attrId );

  osal_buffer_uint32( retBuf, attrValue );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_GET_STATS,
                                sizeof(retBuf), retBuf);
}

/******************************************************************************
 * @fn      MT_SysZDiagsRestoreStatsFromNV
 *
 * @brief   Restores the statistics table from NV into the RAM table.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsRestoreStatsFromNV(void)
{
  uint8 retValue;

  retValue = ZDiagsRestoreStatsFromNV();

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_RESTORE_STATS_NV,
                                sizeof(retValue), &retValue);
}

/******************************************************************************
 * @fn      MT_SysZDiagsSaveStatsToNV
 *
 * @brief   Saves the statistics table from RAM to NV.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void MT_SysZDiagsSaveStatsToNV(void)
{
  uint32 sysClock;
  uint8 retBuf[4];

  /* Returns system clock of the time when the statistics were saved to NV */
  sysClock = ZDiagsSaveStatsToNV();

  osal_buffer_uint32( retBuf, sysClock );

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_ZDIAGS_SAVE_STATS_TO_NV,
                                sizeof(retBuf), retBuf);
}
#endif /* FEATURE_SYSTEM_STATS */
#endif /* MT_SYS_FUNC */

/******************************************************************************
 * SUPPORT
 *****************************************************************************/

/******************************************************************************
 * @fn      MT_SysResetInd()
 *
 * @brief   Sends a ZTOOL "reset response" message.
 *
 * @param   None
 *
 * @return  None
 *
 *****************************************************************************/
void MT_SysResetInd(void)
{
  uint8 retArray[6];

  retArray[0] = ResetReason();   /* Reason */
  osal_memcpy( &retArray[1], MTVersionString, 5 );   /* Revision info */

  /* Send out Reset Response message */
  MT_BuildAndSendZToolResponse( MT_ARSP_SYS, MT_SYS_RESET_IND,
                                sizeof(retArray), retArray);
}

/******************************************************************************
 * @fn      MT_SysOsalTimerExpired()
 *
 * @brief   Sends a SYS Osal Timer Expired
 *
 * @param   None
 *
 * @return  None
 *
 *****************************************************************************/
void MT_SysOsalTimerExpired(uint8 Id)
{
  uint8 retValue = Id;

  MT_BuildAndSendZToolResponse( MT_ARSP_SYS, MT_SYS_OSAL_TIMER_EXPIRED,
                                sizeof(retValue), &retValue);
}

#if defined ( MT_SYS_JAMMER_FEATURE )
/******************************************************************************
 * @fn      MT_SysJammerParameters
 *
 * @brief   Set the Jammer detection parameters.
 *
 * @param   pBuf - MT message containing the parameters.
 *
 * @return  None
 *****************************************************************************/
static void MT_SysJammerParameters( uint8 *pBuf )
{
  uint8 status = SUCCESS;

  // Adjust for the data
  pBuf += MT_RPC_FRAME_HDR_SZ;

  // Number of continuous events needed to detect Jam
  jammerContinuousEvents = osal_build_uint16( pBuf );
  jammerDetections = jammerContinuousEvents;
  pBuf += 2;

  // Noise Level need to be a Jam
  jammerHighNoiseLevel = *pBuf++;

  // The time between each noise level reading
  jammerDetectPeriodTime = osal_build_uint32( pBuf, 4 );

  // Update the timer
  osal_start_reload_timer( jammerTaskID, JAMMER_CHECK_EVT, jammerDetectPeriodTime );

  /* Send out Reset Response message */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_JAMMER_PARAMETERS,
                                sizeof(status), &status );
}

/******************************************************************************
 * @fn      MT_SysJammerInd()
 *
 * @brief   Sends a jammer indication message.
 *
 * @param   jammerInd - TRUE if jammer detected, FALSE if changed to undetected
 *
 * @return  None
 *
 *****************************************************************************/
void MT_SysJammerInd( uint8 jammerInd )
{
  /* Send out Reset Response message */
  MT_BuildAndSendZToolResponse( MT_ARSP_SYS, MT_SYS_JAMMER_IND,
                                sizeof(jammerInd), &jammerInd );
}

/******************************************************************************
 * @fn      jammerInit()
 *
 * @brief   Jammer Detection task initialization function
 *
 * @param   taskId - task ID
 *
 * @return  None
 *
 *****************************************************************************/
void jammerInit( uint8 taskId )
{
  jammerTaskID = taskId;

  // Start the jammer check timer
  osal_start_reload_timer( taskId, JAMMER_CHECK_EVT, jammerDetectPeriodTime );
}

/******************************************************************************
 * @fn      jammerEventLoop()
 *
 * @brief   Jammer Detection task event processing function
 *
 * @param   taskId - task ID
 * @param   events - task events
 *
 * @return  remaining events
 *
 *****************************************************************************/
uint16 jammerEventLoop( uint8 taskId, uint16 events )
{
  osal_event_hdr_t  *pMsg;

  if (events & SYS_EVENT_MSG)
  {
    if ( (pMsg = (osal_event_hdr_t *) osal_msg_receive( taskId )) != NULL )
    {
      switch ( pMsg->event )
      {
        default:
          break;
      }

      osal_msg_deallocate( (byte *)pMsg );
    }

    events ^= SYS_EVENT_MSG;
  }
  else if ( events & JAMMER_CHECK_EVT )
  {
#ifdef FEATURE_DUAL_MAC
    if ( DMMGR_IsDefaultMac() )
#endif /* FEATURE_DUAL_MAC */
    {
    // Make sure we aren't currently receiving a message and radio is active.
    if ( MAC_RX_IS_PHYSICALLY_ACTIVE() == MAC_RX_ACTIVE_NO_ACTIVITY )
    {
      int8 rssiDbm = -128;

      // Read the noise level
      if ( RSSISTAT & 0x01 )
      {
        /* Add the RSSI offset */
        rssiDbm = RSSI + MAC_RADIO_RSSI_OFFSET;

        /* Adjust for external PA/LNA, if any */
        MAC_RADIO_RSSI_LNA_OFFSET( rssiDbm );

        // Check for a noise level that's high
        if ( jammerDetections && (rssiDbm  > jammerHighNoiseLevel) )
        {
          jammerDetections--;
          if ( jammerDetections == 0 )
          {
            // Jam detected
            MT_SysJammerInd( TRUE );
          }
        }
        else if ( rssiDbm <= jammerHighNoiseLevel )
        {
          if ( jammerDetections == 0 )
          {
            // Jam cleared
            MT_SysJammerInd( FALSE );
          }
          jammerDetections = jammerContinuousEvents;
        }
      }
    }
    }
    events ^= JAMMER_CHECK_EVT;
  }
  else
  {
    events = 0;  /* Discard unknown events. */
  }

  return ( events );
}
#endif // MT_SYS_JAMMER_FEATURE

#if defined ( MT_SYS_SNIFFER_FEATURE )
/******************************************************************************
 * @fn      MT_SysSnifferParameters
 *
 * @brief   Set the sniffer parameters.
 *
 * @param   pBuf - MT message containing the parameters.
 *
 * @return  None
 *****************************************************************************/
static void MT_SysSnifferParameters( uint8 *pBuf )
{
  uint8 status = SUCCESS;
  uint8 param;

  // Adjust for the data
  pBuf += MT_RPC_FRAME_HDR_SZ;

  // Noise Level need to be a Jam
  param = *pBuf;

  if ( param == MT_SYS_SNIFFER_DISABLE )
  {
    // Disable Sniffer
    HAL_BOARD_DISABLE_INTEGRATED_SNIFFER();
    sniffer = FALSE;
  }
  else if ( param == MT_SYS_SNIFFER_ENABLE )
  {
    // Enable the Sniffer
    HAL_BOARD_ENABLE_INTEGRATED_SNIFFER();
    sniffer = TRUE;
  }
  else if ( param == MT_SYS_SNIFFER_GET_SETTING )
  {
    status = sniffer; // sniffer setting
  }
  else
  {
    status = INVALIDPARAMETER;
  }

  /* Send back response that includes the actual dBm TxPower that can be set. */
  MT_BuildAndSendZToolResponse( MT_SRSP_SYS, MT_SYS_SNIFFER_PARAMETERS,
                                sizeof(status), &status );
}
#endif // MT_SYS_SNIFFER_FEATURE

#if defined( ENABLE_MT_SYS_RESET_SHUTDOWN )
/******************************************************************************
 * @fn          powerOffSoc
 *
 * @brief   Put the device in lowest power mode infinitely
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
static void powerOffSoc(void)
{
  HAL_DISABLE_INTERRUPTS();

  /* turn off the RF front end device */
  //TBD, based on the rf-front-end being used

  /* turn off the receiver */
  MAC_RADIO_RXTX_OFF();

  /* just in case a receive was about to start, flush the receive FIFO */
  MAC_RADIO_FLUSH_RX_FIFO();

  /* clear any receive interrupt that happened to squeak through */
  MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  /* put MAC timer to sleep */
  MAC_RADIO_TIMER_SLEEP();

  /* power of radio */
  MAC_RADIO_TURN_OFF_POWER();

  STIF = 0; //HAL_SLEEP_TIMER_CLEAR_INT;

  if (ZNP_CFG1_UART == znpCfg1)
  {
    HalUARTSuspend();
  }

  /* Prep CC2530 power mode */
  //HAL_SLEEP_PREP_POWER_MODE(3);
  SLEEPCMD &= ~PMODE; /* clear mode bits */
  SLEEPCMD |= 3;      /* set mode bits  to PM3 */
  while (!(STLOAD & LDRDY));

  while (1) //just in case we wake up for some unknown reason
  {
    /* Execution is supposed to halt at this instruction. Interrupts are
       disabled - the only way to exit this state is from hardware reset. */
    PCON = halSleepPconValue;
    asm("NOP");
  }
}
#endif

/******************************************************************************
 *****************************************************************************/
