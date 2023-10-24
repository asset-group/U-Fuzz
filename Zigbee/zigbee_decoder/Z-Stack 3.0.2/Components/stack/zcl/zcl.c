/**************************************************************************************************
  Filename:       zcl.c
  Revised:        $Date: 2015-09-09 11:51:49 -0700 (Wed, 09 Sep 2015) $
  Revision:       $Revision: 44489 $

  Description:    This file contains the Zigbee Cluster Library Foundation functions.


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

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "AF.h"
#include "APS.h"
#include "zcl.h"
#include "zcl_general.h"

#if defined ( INTER_PAN )
  #include "stub_aps.h"
#endif
   
#ifdef BDB_REPORTING
  #include "bdb_Reporting.h"
#endif
#include "bdb_interface.h"

#include "zcl_green_power.h"

   
/*********************************************************************
 * MACROS
 */
/*** Frame Control ***/
#define zcl_FCType( a )               ( (a) & ZCL_FRAME_CONTROL_TYPE )
#define zcl_FCManuSpecific( a )       ( (a) & ZCL_FRAME_CONTROL_MANU_SPECIFIC )
#define zcl_FCDirection( a )          ( (a) & ZCL_FRAME_CONTROL_DIRECTION )
#define zcl_FCDisableDefaultRsp( a )  ( (a) & ZCL_FRAME_CONTROL_DISABLE_DEFAULT_RSP )

/*** Attribute Access Control ***/
#define zcl_AccessCtrlRead( a )       ( (a) & ACCESS_CONTROL_READ )
#define zcl_AccessCtrlWrite( a )      ( (a) & ACCESS_CONTROL_WRITE )
#define zcl_AccessCtrlCmd( a )        ( (a) & ACCESS_CONTROL_CMD )
#define zcl_AccessCtrlAuthRead( a )   ( (a) & ACCESS_CONTROL_AUTH_READ )
#define zcl_AccessCtrlAuthWrite( a )  ( (a) & ACCESS_CONTROL_AUTH_WRITE )
#define zcl_AccessClient( a )         ( (a) & ACCESS_CLIENT )

#define zclParseCmd( a, b )           zclCmdTable[(a)].pfnParseInProfile( (b) )
#define zclProcessCmd( a, b )         zclCmdTable[(a)].pfnProcessInProfile( (b) )

#define zcl_DefaultRspCmd( zclHdr )   ( zcl_ProfileCmd( (zclHdr).fc.type )     && \
                                        (zclHdr).fc.manuSpecific == 0          && \
                                        (zclHdr).commandID == ZCL_CMD_DEFAULT_RSP )

// Commands that have corresponding responses (ZCL_CMD_WRITE_NO_RSP, does not have response, but must not send default response)
#define CMD_HAS_RSP( cmd )            ( (cmd) == ZCL_CMD_READ                   || \
                                        (cmd) == ZCL_CMD_WRITE                  || \
                                        (cmd) == ZCL_CMD_WRITE_UNDIVIDED        || \
                                        (cmd) == ZCL_CMD_WRITE_NO_RSP           || \
                                        (cmd) == ZCL_CMD_CONFIG_REPORT          || \
                                        (cmd) == ZCL_CMD_READ_REPORT_CFG        || \
                                        (cmd) == ZCL_CMD_DISCOVER_ATTRS         || \
                                        (cmd) == ZCL_CMD_DISCOVER_CMDS_RECEIVED || \
                                        (cmd) == ZCL_CMD_DISCOVER_CMDS_GEN      || \
                                        (cmd) == ZCL_CMD_DISCOVER_ATTRS_EXT     || \
                                        (cmd) == ZCL_CMD_DEFAULT_RSP ) // exception
#define  ZCL_VALID_MIN_HEADER_LEN  3

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclLibPlugin
{
  struct zclLibPlugin *next;
  uint16              startClusterID;    // starting cluster ID
  uint16              endClusterID;      // ending cluster ID
  zclInHdlr_t         pfnIncomingHdlr;    // function to handle incoming message
} zclLibPlugin_t;

// Command record list
typedef struct zclCmdRecsList
{
  struct zclCmdRecsList *pNext;
  uint8                 endpoint;
  uint8                 numCommands;
  CONST zclCommandRec_t *pCmdRecs;
} zclCmdRecsList_t;


// Cluster option list item
typedef struct zclClusterOptionList
{
  struct zclClusterOptionList *next;
  uint8                       endpoint;   // Used to link it into the endpoint descriptor
  uint8                       numOptions; // Number of the following records
  zclOptionRec_t              *options;   // option records
} zclClusterOptionList;

typedef void *(*zclParseInProfileCmd_t)( zclParseCmd_t *pCmd );
typedef uint8 (*zclProcessInProfileCmd_t)( zclIncoming_t *pInMsg );

typedef struct
{
  zclParseInProfileCmd_t   pfnParseInProfile;
  zclProcessInProfileCmd_t pfnProcessInProfile;
} zclCmdItems_t;


// List record for external handler for unhandled ZCL Foundation commands/rsps
typedef struct zclExternalFoundationHandlerList
{
    struct zclExternalFoundationHandlerList *next;
    uint8 zcl_ExternalTaskID;
    uint8 zcl_ExternalEndPoint;
} zclExternalFoundationHandlerList;


/*********************************************************************
 * GLOBAL VARIABLES
 */

#if !defined ( ZCL_STANDALONE )
  uint8 zcl_TaskID;
#endif

// The Application should register its attribute data validation function
zclValidateAttrData_t zcl_ValidateAttrDataCB = (zclValidateAttrData_t)NULL;

// ZCL Sequence number
//uint8 zcl_SeqNum = 0x00;  //Not longer used, refer to bdb_getZCLFrameCounter() in bdb_interface.h
uint8 zcl_InSeqNum = 0x00;

uint8 zcl_radius = AF_DEFAULT_RADIUS;

static uint8 savedZCLTransSeqNum = 0;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclLibPlugin_t *plugins = (zclLibPlugin_t *)NULL;

#if defined ( ZCL_DISCOVER )
  static zclCmdRecsList_t *gpCmdList = (zclCmdRecsList_t *)NULL;
#endif

static zclAttrRecsList *attrList = (zclAttrRecsList *)NULL;
static zclClusterOptionList *clusterOptionList = (zclClusterOptionList *)NULL;

static afIncomingMSGPacket_t *rawAFMsg = (afIncomingMSGPacket_t *)NULL;

#if !defined ( ZCL_STANDALONE )
static zclExternalFoundationHandlerList *externalEndPointHandlerList = (zclExternalFoundationHandlerList *)NULL;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 *zclBuildHdr( zclFrameHdr_t *hdr, uint8 *pData );
static uint8 zclCalcHdrSize( zclFrameHdr_t *hdr );
static zclLibPlugin_t *zclFindPlugin( uint16 clusterID, uint16 profileID );

#if !defined ( ZCL_STANDALONE )
static uint8 zcl_addExternalFoundationHandler( uint8 taskId, uint8 endPointId );
static uint8 zcl_getExternalFoundationHandler( afIncomingMSGPacket_t *pInMsg );
#endif // !defined ( ZCL_STANDALONE )

#if defined ( ZCL_DISCOVER )
  static zclCmdRecsList_t *zclFindCmdRecsList( uint8 endpoint );
#endif

zclAttrRecsList *zclFindAttrRecsList( uint8 endpoint );
static zclOptionRec_t *zclFindClusterOption( uint8 endpoint, uint16 clusterID );
static uint8 zclGetClusterOption( uint8 endpoint, uint16 clusterID );
static void zclSetSecurityOption( uint8 endpoint, uint16 clusterID, uint8 enable );

static uint8 zcl_DeviceOperational( uint8 srcEP, uint16 clusterID, uint8 frameType, uint8 cmd, uint16 profileID );

#if defined ( ZCL_READ ) || defined ( ZCL_WRITE )
static zclReadWriteCB_t zclGetReadWriteCB( uint8 endpoint );
static zclAuthorizeCB_t zclGetAuthorizeCB( uint8 endpoint );
#endif // ZCL_READ || ZCL_WRITE

#ifdef ZCL_READ
ZStatus_t zclReadAttrData( uint8 *pAttrData, zclAttrRec_t *pAttr, uint16 *pDataLen );
static uint16 zclGetAttrDataLengthUsingCB( uint8 endpoint, uint16 clusterID, uint16 attrId );
static ZStatus_t zclReadAttrDataUsingCB( uint8 endpoint, uint16 clusterId, uint16 attrId,
                                         uint8 *pAttrData, uint16 *pDataLen );
static ZStatus_t zclAuthorizeRead( uint8 endpoint, afAddrType_t *srcAddr, zclAttrRec_t *pAttr );
static void *zclParseInReadRspCmd( zclParseCmd_t *pCmd );
static uint8 zclProcessInReadCmd( zclIncoming_t *pInMsg );
#endif // ZCL_READ

#ifdef ZCL_WRITE
static ZStatus_t zclWriteAttrData( uint8 endpoint, afAddrType_t *srcAddr,
                                   zclAttrRec_t *pAttr, zclWriteRec_t *pWriteRec );
static ZStatus_t zclWriteAttrDataUsingCB( uint8 endpoint, afAddrType_t *srcAddr,
                                          zclAttrRec_t *pAttr, uint8 *pAttrData );
static ZStatus_t zclAuthorizeWrite( uint8 endpoint, afAddrType_t *srcAddr, zclAttrRec_t *pAttr );
static void *zclParseInWriteRspCmd( zclParseCmd_t *pCmd );
static uint8 zclProcessInWriteCmd( zclIncoming_t *pInMsg );
static uint8 zclProcessInWriteUndividedCmd( zclIncoming_t *pInMsg );
#endif // ZCL_WRITE

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
static void *zclParseInConfigReportRspCmd( zclParseCmd_t *pCmd );
static void *zclParseInReadReportCfgRspCmd( zclParseCmd_t *pCmd );
#endif

static void *zclParseInDefaultRspCmd( zclParseCmd_t *pCmd );

#ifdef ZCL_DISCOVER
static uint8 zclFindNextCmdRec( uint8 endpoint, uint16 clusterID, uint8 commandID, uint8 direction, uint8 *pCmdID, zclCommandRec_t *pCmd );
static uint8 zclFindNextAttrRec( uint8 endpoint, uint16 clusterID, uint8 direction, uint16 *attrId, zclAttrRec_t *pAttr );
static void *zclParseInDiscCmdsRspCmd( zclParseCmd_t *pCmd );
static void *zclParseInDiscAttrsRspCmd( zclParseCmd_t *pCmd );
static void *zclParseInDiscAttrsExtRspCmd( zclParseCmd_t *pCmd );
static uint8 zclProcessInDiscCmd( zclIncoming_t *pInMsg );
static uint8 zclProcessInDiscAttrs( zclIncoming_t *pInMsg );
static void zclProcessInDiscAttrsCmd( zclIncoming_t *pInMsg, zclDiscoverAttrsCmd_t *pDiscoverCmd, uint8 attrLenBuf );
static void zclProcessInDiscAttrsExtCmd( zclIncoming_t *pInMsg, zclDiscoverAttrsCmd_t *pDiscoverCmd, uint8 attrLenBuf );
#endif // ZCL_DISCOVER

/*********************************************************************
 * Parse Profile Command Function Table
 */

static CONST zclCmdItems_t zclCmdTable[] =
{
#ifdef ZCL_READ
  /* ZCL_CMD_READ */                { zclParseInReadCmd,             zclProcessInReadCmd             },
  /* ZCL_CMD_READ_RSP */            { zclParseInReadRspCmd,          zcl_HandleExternal              },
#else
  /* ZCL_CMD_READ */                { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_READ_RSP */            { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif // ZCL_READ

#ifdef ZCL_WRITE
  /* ZCL_CMD_WRITE */               { zclParseInWriteCmd,            zclProcessInWriteCmd            },
  /* ZCL_CMD_WRITE_UNDIVIDED */     { zclParseInWriteCmd,            zclProcessInWriteUndividedCmd   },
  /* ZCL_CMD_WRITE_RSP */           { zclParseInWriteRspCmd,         zcl_HandleExternal              },
  /* ZCL_CMD_WRITE_NO_RSP */        { zclParseInWriteCmd,            zclProcessInWriteCmd            },
#else
  /* ZCL_CMD_WRITE */               { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_WRITE_UNDIVIDED */     { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_WRITE_RSP */           { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_WRITE_NO_RSP */        { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif // ZCL_WRITE

#ifdef ZCL_REPORTING_DEVICE
    /* ZCL_CMD_CONFIG_REPORT */       { zclParseInConfigReportCmd,     zcl_HandleExternal              },
#else
    /* ZCL_CMD_CONFIG_REPORT */       { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
    /* ZCL_CMD_CONFIG_REPORT_RSP */   { zclParseInConfigReportRspCmd,  zcl_HandleExternal              },
#else
    /* ZCL_CMD_CONFIG_REPORT_RSP */   { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif

#ifdef ZCL_REPORTING_DEVICE
    /* ZCL_CMD_READ_REPORT_CFG */     { zclParseInReadReportCfgCmd,    zcl_HandleExternal              },
#else
    /* ZCL_CMD_READ_REPORT_CFG */     { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
    /* ZCL_CMD_READ_REPORT_CFG_RSP */ { zclParseInReadReportCfgRspCmd, zcl_HandleExternal              },
#else
    /* ZCL_CMD_READ_REPORT_CFG_RSP */ { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif

#ifdef ZCL_REPORT_DESTINATION_DEVICE
    /* ZCL_CMD_REPORT */              { zclParseInReportCmd,           zcl_HandleExternal              },
#else
    /* ZCL_CMD_REPORT */              { (zclParseInProfileCmd_t)NULL,  (zclProcessInProfileCmd_t)NULL  },
#endif

  /* ZCL_CMD_DEFAULT_RSP */         { zclParseInDefaultRspCmd,       zcl_HandleExternal              },

#ifdef ZCL_DISCOVER
  /* ZCL_CMD_DISCOVER_ATTRS */                { zclParseInDiscAttrsCmd,         zclProcessInDiscAttrs           },
  /* ZCL_CMD_DISCOVER_ATTRS_RSP */            { zclParseInDiscAttrsRspCmd,      zcl_HandleExternal              },
  /* *not supported* READ_ATTRS_STRCT */      { NULL,                           (zclProcessInProfileCmd_t)NULL  },
  /* *not supported* WRITE_ATTRS_STRCT */     { NULL,                           (zclProcessInProfileCmd_t)NULL  },
  /* *not supported* WRITE_ATTRS_STRCT_RSP */ { NULL,                           (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_CMDS_RECEIVED */        { zclParseInDiscCmdsCmd,          zclProcessInDiscCmd             },
  /* ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP */    { zclParseInDiscCmdsRspCmd,       zcl_HandleExternal              },
  /* ZCL_CMD_DISCOVER_CMDS_GEN */             { zclParseInDiscCmdsCmd,          zclProcessInDiscCmd             },
  /* ZCL_CMD_DISCOVER_CMDS_GEN_RSP */         { zclParseInDiscCmdsRspCmd,       zcl_HandleExternal              },
  /* ZCL_CMD_DISCOVER_ATTRS_EXT */            { zclParseInDiscAttrsCmd,         zclProcessInDiscAttrs           },
  /* ZCL_CMD_DISCOVER_ATTRS_EXT_RSP */        { zclParseInDiscAttrsExtRspCmd,   zcl_HandleExternal              },
#else
  /* ZCL_CMD_DISCOVER_ATTRS */                { (zclParseInProfileCmd_t)NULL,    (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_ATTRS_RSP */            { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* *not supported* READ_ATTRS_STRCT */      { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* *not supported* WRITE_ATTRS_STRCT */     { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* *not supported* WRITE_ATTRS_STRCT_RSP */ { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_CMDS_RECEIVED */        { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP */    { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_CMDS_GEN */             { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_CMDS_GEN_RSP */         { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_ATTRS_EXT */            { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
  /* ZCL_CMD_DISCOVER_ATTRS_EXT_RSP */        { (zclParseInProfileCmd_t)NULL,   (zclProcessInProfileCmd_t)NULL  },
#endif // ZCL_DISCOVER
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zcl_Init
 *
 * @brief       Initialization function for the zcl layer.
 *
 * @param       task_id - ZCL task id
 *
 * @return      none
 */
void zcl_Init( uint8 task_id )
{
  zcl_TaskID = task_id;
}
#endif

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn          zcl_event_loop
 *
 * @brief       Event Loop Processor for zcl.
 *
 * @param       task_id - task id
 * @param       events - event bitmap
 *
 * @return      unprocessed events
 */
uint16 zcl_event_loop( uint8 task_id, uint16 events )
{
  uint8 *msgPtr;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    msgPtr = osal_msg_receive( zcl_TaskID );
    while ( msgPtr != NULL )
    {
      uint8 dealloc = TRUE;

      if ( *msgPtr == AF_INCOMING_MSG_CMD )
      {
        zcl_ProcessMessageMSG( (afIncomingMSGPacket_t *)msgPtr );
      }
      else
      {
        uint8 taskID;
        taskID = zcl_getExternalFoundationHandler( (afIncomingMSGPacket_t *)msgPtr );

        if ( taskID != TASK_NO_TASK )
        {
          // send it to another task to process.
          osal_msg_send( taskID, msgPtr );
          dealloc = FALSE;
        }
      }

      // Release the memory
      if ( dealloc )
      {
        osal_msg_deallocate( msgPtr );
      }

      // Next
      msgPtr = osal_msg_receive( zcl_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  if ( events & ZCL_DATABUF_SEND )
  {
    gpNotificationMsg_t *gpNotification = NULL;
    gpCmdPayloadMsg_t *pCmdPayloadMsgCurr = NULL;
    gpd_ID_t gpd_ID;
    uint8 *pgpdid;
    uint8 entry[PROXY_TBL_ENTRY_LEN];
    uint8 appId;
    uint8 nwkSeqNum;
    uint8 apsSecNum;
    uint16 nwkAddr;
    
    gpNotification = gp_GetHeadNotificationMsg( );
    
    if ( gpNotification == NULL )
    {
      return 0;
    }
    
    pCmdPayloadMsgCurr = ( gpCmdPayloadMsg_t* ) gpNotification->pMsg ;
    appId = PROXY_TBL_GET_APPLICTION_ID((uint16)*pCmdPayloadMsgCurr->pMsg);
    
    // To save the NIB nwk sequense number and use the GP alias nwk sequence number
    // for the GP notification
    nwkSeqNum = _NIB.SequenceNum;
    _NIB.SequenceNum = pCmdPayloadMsgCurr->secNum;
    
    // To save the NIB nwk address and use the GP alias nwk address for the GP notification
    nwkAddr = _NIB.nwkDevAddress;
    
    if( appId == GP_OPT_APP_ID_GPD )
    {
      pgpdid = pCmdPayloadMsgCurr->pMsg + sizeof( uint16 );
      
      osal_memcpy( &_NIB.nwkDevAddress, pgpdid, sizeof(uint16) );
      gpd_ID.AppID = GP_OPT_APP_ID_GPD;
      osal_memcpy( &gpd_ID.GPDId.SrcID, pgpdid, sizeof( uint32 ) );
    }
    else if( appId == GP_OPT_APP_ID_IEEE )
    {
      pgpdid = pCmdPayloadMsgCurr->pMsg + Z_EXTADDR_LEN;
      
      osal_revmemcpy( &_NIB.nwkDevAddress,(pCmdPayloadMsgCurr->pMsg + Z_EXTADDR_LEN), sizeof(uint16) );
      
      gpd_ID.AppID = GP_OPT_APP_ID_IEEE;

      pgpdid = pCmdPayloadMsgCurr->pMsg + sizeof( uint16 );
      osal_memcpy( gpd_ID.GPDId.GPDExtAddr, pgpdid, Z_EXTADDR_LEN );
    }
    
    gp_getProxyTableByGpId(&gpd_ID, entry, NULL);

    if ( PROXY_TBL_GET_ASSIGNED_ALIAS( entry[PROXY_TBL_ENTRY_OPT + 1] ) )
    {
      _NIB.nwkDevAddress = 0;
      osal_memcpy( (uint8*)&_NIB.nwkDevAddress, &entry[PROXY_TBL_ENTRY_ALIAS], sizeof(uint16));
    }
    
    // save aps sequence number
    apsSecNum = APS_Counter;
    APS_Counter = pCmdPayloadMsgCurr->secNum;
    
    // Set nwk radius for zcl frame
    if ( entry[PROXY_TBL_ENTRY_GRP_RAD] != 0xFF)
    {
      zcl_radius =  entry[PROXY_TBL_ENTRY_GRP_RAD];
    }
    
    zcl_SendCommand( GREEN_POWER_INTERNAL_ENDPOINT, &gpNotification->addr, ZCL_CLUSTER_ID_GREEN_POWER,
                          COMMAND_GP_NOTIFICATION, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR,
                          TRUE, 0,  bdb_getZCLFrameCounter(), pCmdPayloadMsgCurr->lenght, pCmdPayloadMsgCurr->pMsg );
    
    // restore nwk radius
    zcl_radius = AF_DEFAULT_RADIUS;
    
    // restore aps sequence number
    APS_Counter = apsSecNum;
     
    // Restore the NIB nwk sequence number
    _NIB.SequenceNum = nwkSeqNum;
    
    // Restore the NIB nwk address
    _NIB.nwkDevAddress = nwkAddr;
    
    gp_NotificationMsgClean( gp_GetPHeadNotification ( ) );
    
    if ( gp_GetHeadNotificationMsg ( ) != NULL )
    {
      osal_start_timerEx( zcl_TaskID, ZCL_DATABUF_SEND, 50 );
    }
  }
#endif

  // Discard unknown events
  return 0;
}
#endif

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zcl_registerForMsg
 *
 * @brief   The ZCL is setup to send all incoming Foundation Command/Response
 *          messages that aren't processed to one task (if a task is
 *          registered).
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 *
 * @return  TRUE if task registeration successful, FALSE otherwise
 *********************************************************************/
uint8 zcl_registerForMsg( uint8 taskId )
{
  return zcl_addExternalFoundationHandler( taskId, AF_BROADCAST_ENDPOINT );
}

/*********************************************************************
 * @fn      zcl_registerForMsgExt
 *
 * @brief   This function enables a Task to register to recieve all
 *          incoming Foundation Command/Response messages, for a particular
 *          End Point, that aren't processed by ZCL.
 *
 *          NOTE: Any Task registered for a unique end point will take
 *          priority over any Task registered with the AF_BROADCAST_ENDPOINT
 *          value.  ie. If task A registers for End Point 1, task B registers
 *          for AF_BROADCAST_ENDPOINT,  commands addressed to End Point 1 will be
 *          sent to Task A and NOT Task B.
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 * @param   endPointId - end point Id of interest
 *
 * @return  TRUE if task registeration successful, FALSE otherwise
 *********************************************************************/
uint8 zcl_registerForMsgExt( uint8 taskId, uint8 endPointId  )
{
  return ( zcl_addExternalFoundationHandler( taskId, endPointId  ) );
}

/*********************************************************************
 * @fn      zcl_addExternalFoundationHandler
 *
 * @brief   This function adds a record to the internal list of external
 *          handlers of unhandled incoming Foundation Command/Response messages.
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 * @param   endPointId - end point Id of interest
 *
 * @return  TRUE if task registeration successful, FALSE otherwise
 *********************************************************************/
uint8 zcl_addExternalFoundationHandler( uint8 taskId, uint8 endPointId  )
{
  zclExternalFoundationHandlerList *pNewItem;
  zclExternalFoundationHandlerList *pLoop;
  zclExternalFoundationHandlerList *pLoopPrev;

  // Fill in the new endpoint registrant list
  pNewItem = zcl_mem_alloc( sizeof( zclExternalFoundationHandlerList ) );
  if ( pNewItem == NULL )
  {
    return ( false );
  }

  pNewItem->zcl_ExternalEndPoint = endPointId;
  pNewItem->zcl_ExternalTaskID = taskId;
  pNewItem->next = NULL;

  // Add to the list
  if ( externalEndPointHandlerList == NULL )
  {
    externalEndPointHandlerList = pNewItem;
  }
  else
  {
    // make sure no one else tried to register for this endpoint
    pLoop = externalEndPointHandlerList;
    pLoopPrev = externalEndPointHandlerList;

    while ( pLoop != NULL )
    {
      if ( ( pLoop->zcl_ExternalEndPoint ) == endPointId )
      {
        zcl_mem_free(pNewItem);
        return ( false );
      }
      pLoopPrev = pLoop;
      pLoop = pLoop->next;
    }

    if ( endPointId == AF_BROADCAST_ENDPOINT )
    {
      // put new registration at the end of the list
      pLoopPrev->next = pNewItem;
      pNewItem->next = NULL;
    }
    else
    {
      // put new registration at the front of the list
      zclExternalFoundationHandlerList *temp = externalEndPointHandlerList;
      externalEndPointHandlerList = pNewItem;
      pNewItem->next = temp;
    }
  }

  return ( true );

}

/*********************************************************************
 * @fn      zcl_getExternalFoundationHandler
 *
 * @brief   This function retrieves the Task ID of the task registered
 *          to received unhandled incoming Foundation Command/Response messages
 *          for a particular End Point ID.
 *
 * @param   pInMsg - recevied ZCL command
 *
 * @return  TASK ID of registered task.  If no task is reigistered, it returns
 *          TASK_NO_TASK.
 *********************************************************************/
static uint8 zcl_getExternalFoundationHandler( afIncomingMSGPacket_t *pInMsg )
{
  zclExternalFoundationHandlerList *pLoop;
  uint8 addressedEndPointId = pInMsg->endPoint;

  // make sure no one else tried to register for this endpoint
  pLoop = externalEndPointHandlerList;

  while ( pLoop != NULL )
  {
    if ( ( ( pLoop->zcl_ExternalEndPoint ) == addressedEndPointId ) ||
         ( ( pLoop->zcl_ExternalEndPoint ) == AF_BROADCAST_ENDPOINT ) )
    {
      return ( pLoop->zcl_ExternalTaskID );
    }
    pLoop = pLoop->next;
  }

  return ( TASK_NO_TASK );
}
#endif

#if !defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zcl_HandleExternal
 *
 * @brief
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE
 */
uint8 zcl_HandleExternal( zclIncoming_t *pInMsg )
{
  zclIncomingMsg_t *pCmd;
  uint8 taskID;

  taskID = zcl_getExternalFoundationHandler( pInMsg->msg );

  if ( taskID == TASK_NO_TASK )
  {
    return ( TRUE );
  }

  pCmd = (zclIncomingMsg_t *)osal_msg_allocate( sizeof ( zclIncomingMsg_t ) );
  if ( pCmd != NULL )
  {
    // fill in the message
    pCmd->hdr.event = ZCL_INCOMING_MSG;
    pCmd->zclHdr    = pInMsg->hdr;
    pCmd->clusterId = pInMsg->msg->clusterId;
    pCmd->srcAddr   = pInMsg->msg->srcAddr;
    pCmd->endPoint  = pInMsg->msg->endPoint;
    pCmd->attrCmd   = pInMsg->attrCmd;
    
#ifdef BDB_REPORTING
    if(pCmd->zclHdr.commandID == ZCL_CMD_CONFIG_REPORT)
    {
      bdb_ProcessInConfigReportCmd(pCmd);
      osal_msg_deallocate((uint8*)pCmd);
      return TRUE;          
    }
    if(pCmd->zclHdr.commandID == ZCL_CMD_READ_REPORT_CFG)
    {
      bdb_ProcessInReadReportCfgCmd(pCmd);
      osal_msg_deallocate((uint8*)pCmd);
      return TRUE;
    }
#endif
    // Application will free the attrCmd buffer
    pInMsg->attrCmd = NULL;

    /* send message through task message */
    osal_msg_send( taskID, (uint8 *)pCmd );
  }

  return ( TRUE );
}
#endif


/*********************************************************************
 * @fn          zcl_getRawAFMsg
 *
 * @brief       Call to get original unprocessed AF message
 *              (not parsed by ZCL).
 *
 *   NOTE:  This function can only be called during a ZCL callback function
 *          and the calling function must NOT change any data in the message.
 *
 * @param       none
 *
 * @return      pointer to original AF message, NULL if not processing
 *              AF message.
 */
afIncomingMSGPacket_t *zcl_getRawAFMsg( void )
{
  return ( rawAFMsg );
}

/*********************************************************************
 * @fn          zcl_getParsedTransSeqNum
 *
 * @brief       Call to the get the transaction sequence number from
 *              the incoming message.
 *
 *   NOTE:  This function can only be called during a ZCL callback function
 *          and the calling function must NOT change any data in the message.
 *
 * @param       none
 *
 * @return      transaction sequence number.
 */
uint8 zcl_getParsedTransSeqNum( void )
{
  return ( savedZCLTransSeqNum );
}

/*********************************************************************
 * @fn          zcl_registerPlugin
 *
 * @brief       Add a Cluster Library handler
 *
 * @param       startClusterID - starting cluster ID
 * @param       endClusterID - ending cluster ID
 * @param       pfnHdlr - function pointer to incoming message handler
 *
 * @return      ZSuccess if OK
 */
ZStatus_t zcl_registerPlugin( uint16 startClusterID,
          uint16 endClusterID, zclInHdlr_t pfnIncomingHdlr )
{
  zclLibPlugin_t *pNewItem;
  zclLibPlugin_t *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclLibPlugin_t ) );
  if ( pNewItem == NULL )
  {
    return (ZMemError);
  }

  // Fill in the plugin record.
  pNewItem->next = (zclLibPlugin_t *)NULL;
  pNewItem->startClusterID = startClusterID;
  pNewItem->endClusterID = endClusterID;
  pNewItem->pfnIncomingHdlr = pfnIncomingHdlr;

  // Find spot in list
  if (  plugins == NULL )
  {
    plugins = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = plugins;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn          zcl_registerCmdList
 *
 * @brief       Register a Command List with ZCL Foundation
 *
 * @param       endpoint - endpoint the attribute list belongs to
 * @param       newCmdList - array of command records
 *
 * @return      ZSuccess if OK
 */
ZStatus_t zcl_registerCmdList( uint8 endpoint, CONST uint8 cmdListSize, CONST zclCommandRec_t newCmdList[] )
{
  zclCmdRecsList_t *pNewItem;
  zclCmdRecsList_t *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclCmdRecsList_t ) );
  if ( pNewItem == NULL )
  {
    return (ZMemError);
  }

  pNewItem->pNext = (zclCmdRecsList_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->numCommands = cmdListSize;
  pNewItem->pCmdRecs = newCmdList;

  // Find spot in list
  if ( gpCmdList == NULL )
  {
    gpCmdList = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = gpCmdList;
    while ( pLoop->pNext != NULL )
    {
      pLoop = pLoop->pNext;
    }

    // Put new item at end of list
    pLoop->pNext = pNewItem;
  }

  return ( ZSuccess );
}
#endif  // ZCL_DISCOVER

/*********************************************************************
 * @fn          zcl_registerAttrList
 *
 * @brief       Register an Attribute List with ZCL Foundation
 *
 * @param       endpoint - endpoint the attribute list belongs to
 * @param       numAttr - number of attributes in list
 * @param       newAttrList - array of Attribute records.
 *                            NOTE: THE ATTRIBUTE IDs (FOR A CLUSTER) MUST BE IN
 *                            ASCENDING ORDER. OTHERWISE, THE DISCOVERY RESPONSE
 *                            COMMAND WILL NOT HAVE THE RIGHT ATTRIBUTE INFO
 *
 * @return      ZSuccess if OK
 */
ZStatus_t zcl_registerAttrList( uint8 endpoint, uint8 numAttr, CONST zclAttrRec_t newAttrList[] )
{
  zclAttrRecsList *pNewItem;
  zclAttrRecsList *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclAttrRecsList ) );
  if ( pNewItem == NULL )
  {
    return (ZMemError);
  }

  pNewItem->next = (zclAttrRecsList *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->pfnReadWriteCB = NULL;
  pNewItem->numAttributes = numAttr;
  pNewItem->attrs = newAttrList;

  // Find spot in list
  if ( attrList == NULL )
  {
    attrList = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = attrList;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn          zcl_registerClusterOptionList
 *
 * @brief       Register a Cluster Option List with ZCL Foundation
 *
 * @param       endpoint - endpoint the option list belongs to
 * @param       numOption - number of options in list
 * @param       optionList - array of cluster option records.
 *
 *              NOTE: This API should be called to enable 'Application
 *                    Link Key' security and/or 'APS ACK' for a specific
 *                    Cluster. The 'Application Link Key' is discarded
 *                    if security isn't enabled on the device.
 *                    The default behavior is 'Network Key' when security
 *                    is enabled and no 'APS ACK' for the ZCL messages.
 *
 * @return      ZSuccess if OK
 */
ZStatus_t zcl_registerClusterOptionList( uint8 endpoint, uint8 numOption, zclOptionRec_t optionList[] )
{
  zclClusterOptionList *pNewItem;
  zclClusterOptionList *pLoop;

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclClusterOptionList ) );
  if ( pNewItem == NULL )
  {
    return (ZMemError);
  }

  pNewItem->next = (zclClusterOptionList *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->numOptions = numOption;
  pNewItem->options = optionList;

  // Find spot in list
  if ( clusterOptionList == NULL )
  {
    clusterOptionList = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = clusterOptionList;
    while ( pLoop->next != NULL )
    {
      pLoop = pLoop->next;
    }

    // Put new item at end of list
    pLoop->next = pNewItem;
  }

  return ( ZSuccess );
}

/*********************************************************************
 * @fn          zcl_registerValidateAttrData
 *
 * @brief       Add a validation function for attribute data
 *
 * @param       pfnValidateAttrData - function pointer to validate routine
 *
 * @return      ZSuccess if OK
 */
ZStatus_t zcl_registerValidateAttrData( zclValidateAttrData_t pfnValidateAttrData )
{
  zcl_ValidateAttrDataCB = pfnValidateAttrData;

  return ( ZSuccess );
}

/*********************************************************************
 * @fn          zcl_registerReadWriteCB
 *
 * @brief       Register the application's callback function to read/write
 *              attribute data, and authorize read/write operation.
 *
 *              Note: The pfnReadWriteCB callback function is only required
 *                    when the attribute data format is unknown to ZCL. The
 *                    callback function gets called when the pointer 'dataPtr'
 *                    to the attribute value is NULL in the attribute database
 *                    registered with the ZCL.
 *
 *              Note: The pfnAuthorizeCB callback function is only required
 *                    when the Read/Write operation on an attribute requires
 *                    authorization (i.e., attributes with ACCESS_CONTROL_AUTH_READ
 *                    or ACCESS_CONTROL_AUTH_WRITE access permissions).
 *
 * @param       endpoint - application's endpoint
 * @param       pfnReadWriteCB - function pointer to read/write routine
 * @param       pfnAuthorizeCB - function pointer to authorize read/write operation
 *
 * @return      ZSuccess if successful. ZFailure, otherwise.
 */
ZStatus_t zcl_registerReadWriteCB( uint8 endpoint, zclReadWriteCB_t pfnReadWriteCB,
                                   zclAuthorizeCB_t pfnAuthorizeCB )
{
  zclAttrRecsList *pRec = zclFindAttrRecsList( endpoint );

  if ( pRec != NULL )
  {
    pRec->pfnReadWriteCB = pfnReadWriteCB;
    pRec->pfnAuthorizeCB = pfnAuthorizeCB;

    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      zcl_DeviceOperational
 *
 * @brief   Used to see whether or not the device can send or respond
 *          to application level commands.
 *
 * @param   srcEP - source endpoint
 * @param   clusterID - cluster ID
 * @param   frameType - command type
 * @param   cmd - command ID
 *
 * @return  TRUE if device is operational, FALSE otherwise
 */
static uint8 zcl_DeviceOperational( uint8 srcEP, uint16 clusterID,
                                    uint8 frameType, uint8 cmd, uint16 profileID )
{
  zclAttrRec_t attrRec;
  uint8 deviceEnabled = DEVICE_ENABLED; // default value

  (void)profileID;  // Intentionally unreferenced parameter

  // If the device is Disabled (DeviceEnabled attribute is set to Disabled), it
  // cannot send or respond to application level commands, other than commands
  // to read or write attributes. Note that the Identify cluster cannot be
  // disabled, and remains functional regardless of this setting.
  if ( zcl_ProfileCmd( frameType ) && cmd <= ZCL_CMD_WRITE_NO_RSP )
  {
    return ( TRUE );
  }

  if ( clusterID == ZCL_CLUSTER_ID_GEN_IDENTIFY )
  {
    return ( TRUE );
  }

  // Is device enabled?
  if ( zclFindAttrRec( srcEP, ZCL_CLUSTER_ID_GEN_BASIC,
                       ATTRID_BASIC_DEVICE_ENABLED, &attrRec ) )
  {
#ifdef ZCL_READ
    zclReadAttrData( &deviceEnabled, &attrRec, NULL );
#endif
  }

  return ( deviceEnabled == DEVICE_ENABLED ? TRUE : FALSE );
}

/*********************************************************************
 * @fn      zcl_SendCommand
 *
 * @brief   Used to send Profile and Cluster Specific Command messages.
 *
 *          NOTE: The calling application is responsible for incrementing
 *                the Sequence Number.
 *
 * @param   srcEp - source endpoint
 * @param   destAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cmd - command ID
 * @param   specific - whether the command is Cluster Specific
 * @param   direction - client/server direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   manuCode - manufacturer code for proprietary extensions to a profile
 * @param   seqNumber - identification number for the transaction
 * @param   cmdFormatLen - length of the command to be sent
 * @param   cmdFormat - command to be sent
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendCommand( uint8 srcEP, afAddrType_t *destAddr,
                           uint16 clusterID, uint8 cmd, uint8 specific, uint8 direction,
                           uint8 disableDefaultRsp, uint16 manuCode, uint8 seqNum,
                           uint16 cmdFormatLen, uint8 *cmdFormat )
{
  endPointDesc_t *epDesc;
  zclFrameHdr_t hdr;
  uint8 *msgBuf;
  uint16 msgLen;
  uint8 *pBuf;
  uint8 options;
  ZStatus_t status;

  epDesc = afFindEndPointDesc( srcEP );
  if ( epDesc == NULL )
  {
    return ( ZInvalidParameter ); // EMBEDDED RETURN
  }

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( destAddr->panId, destAddr->endPoint ) )
  {
    options = AF_TX_OPTIONS_NONE;
  }
  else
#endif
  {
    options = zclGetClusterOption( srcEP, clusterID );

    // The cluster might not have been defined to use security but if this message
    // is in response to another message that was using APS security this message
    // will be sent with APS security
    if ( !( options & AF_EN_SECURITY ) )
    {
      afIncomingMSGPacket_t *origPkt = zcl_getRawAFMsg();

      if ( ( origPkt != NULL ) && ( origPkt->SecurityUse == TRUE ) )
      {
        options |= AF_EN_SECURITY;
      }
    }
  }

  zcl_memset( &hdr, 0, sizeof( zclFrameHdr_t ) );

  // Not Profile wide command (like READ, WRITE)
  if ( specific )
  {
    hdr.fc.type = ZCL_FRAME_TYPE_SPECIFIC_CMD;
  }
  else
  {
    hdr.fc.type = ZCL_FRAME_TYPE_PROFILE_CMD;
  }

  if ( ( epDesc->simpleDesc == NULL ) ||
       ( zcl_DeviceOperational( srcEP, clusterID, hdr.fc.type,
                                cmd, epDesc->simpleDesc->AppProfId ) == FALSE ) )
  {
    return ( ZFailure ); // EMBEDDED RETURN
  }

  // Fill in the Maufacturer Code
  if ( manuCode != 0 )
  {
    hdr.fc.manuSpecific = 1;
    hdr.manuCode = manuCode;
  }

  // Set the Command Direction
  if ( direction )
  {
    hdr.fc.direction = ZCL_FRAME_SERVER_CLIENT_DIR;
  }
  else
  {
    hdr.fc.direction = ZCL_FRAME_CLIENT_SERVER_DIR;
  }

  // Set the Disable Default Response field
  if ( disableDefaultRsp )
  {
    hdr.fc.disableDefaultRsp = 1;
  }
  else
  {
    hdr.fc.disableDefaultRsp = 0;
  }

  // Fill in the Transaction Sequence Number
  hdr.transSeqNum = seqNum;

  // Fill in the command
  hdr.commandID = cmd;

  // calculate the needed buffer size
  msgLen = zclCalcHdrSize( &hdr );
  msgLen += cmdFormatLen;

  // Allocate the buffer needed
  msgBuf = zcl_mem_alloc( msgLen );
  if ( msgBuf != NULL )
  {
    // Fill in the ZCL Header
    pBuf = zclBuildHdr( &hdr, msgBuf );

    // Fill in the command frame
    zcl_memcpy( pBuf, cmdFormat, cmdFormatLen );

    status = AF_DataRequest( destAddr, epDesc, clusterID, msgLen, msgBuf,
                             &APS_Counter, options, zcl_radius );
    zcl_mem_free ( msgBuf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zcl_SendRead
 *
 * @brief   Send a Read command
 *
 * @param   srcEP - Application's endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readCmd - read command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendRead( uint8 srcEP, afAddrType_t *dstAddr,
                        uint16 clusterID, zclReadCmd_t *readCmd,
                        uint8 direction, uint8 disableDefaultRsp, uint8 seqNum)
{
  uint16 dataLen;
  uint8 *buf;
  uint8 *pBuf;
  ZStatus_t status;

  dataLen = readCmd->numAttr * 2; // Attribute ID

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    uint8 i;

    // Load the buffer - serially
    pBuf = buf;
    for (i = 0; i < readCmd->numAttr; i++)
    {
      *pBuf++ = LO_UINT16( readCmd->attrID[i] );
      *pBuf++ = HI_UINT16( readCmd->attrID[i] );
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_READ, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendReadRsp
 *
 * @brief   Send a Read Response command.
 *
 * @param   srcEP - Application's endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readRspCmd - read response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendReadRsp( uint8 srcEP, afAddrType_t *dstAddr,
                           uint16 clusterID, zclReadRspCmd_t *readRspCmd,
                           uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint16 len = 0;
  ZStatus_t status;
  uint8 i;

  // calculate the size of the command
  for ( i = 0; i < readRspCmd->numAttr; i++ )
  {
    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);

    len += 2 + 1; // Attribute ID + Status

    if ( statusRec->status == ZCL_STATUS_SUCCESS )
    {
      len++; // Attribute Data Type length

      // Attribute Data length
      if ( statusRec->data != NULL )
      {
        len += zclGetAttrDataLength( statusRec->dataType, statusRec->data );
      }
      else
      {
        len += zclGetAttrDataLengthUsingCB( srcEP, clusterID, statusRec->attrID );
      }
    }
  }

  buf = zcl_mem_alloc( len );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;

    for ( i = 0; i < readRspCmd->numAttr; i++ )
    {
      zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);

      *pBuf++ = LO_UINT16( statusRec->attrID );
      *pBuf++ = HI_UINT16( statusRec->attrID );
      *pBuf++ = statusRec->status;

      if ( statusRec->status == ZCL_STATUS_SUCCESS )
      {
        *pBuf++ = statusRec->dataType;

        if ( statusRec->data != NULL )
        {
          // Copy attribute data to the buffer to be sent out
          pBuf = zclSerializeData( statusRec->dataType, statusRec->data, pBuf );
        }
        else
        {
          uint16 dataLen;

          // Read attribute data directly into the buffer to be sent out
          zclReadAttrDataUsingCB( srcEP, clusterID, statusRec->attrID, pBuf, &dataLen );
          pBuf += dataLen;
        }
      }
    } // for loop

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_READ_RSP, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, len, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      sendWriteRequest
 *
 * @brief   Send a Write command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   writeCmd - write command to be sent
 * @param   cmd - ZCL_CMD_WRITE, ZCL_CMD_WRITE_UNDIVIDED or ZCL_CMD_WRITE_NO_RSP
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendWriteRequest( uint8 srcEP, afAddrType_t *dstAddr, uint16 clusterID,
                                zclWriteCmd_t *writeCmd, uint8 cmd, uint8 direction,
                                uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint16 dataLen = 0;
  ZStatus_t status;
  uint8 i;

  for ( i = 0; i < writeCmd->numAttr; i++ )
  {
    zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

    dataLen += 2 + 1; // Attribute ID + Attribute Type

    // Attribute Data
    dataLen += zclGetAttrDataLength( statusRec->dataType, statusRec->attrData );
  }

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    for ( i = 0; i < writeCmd->numAttr; i++ )
    {
      zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

      *pBuf++ = LO_UINT16( statusRec->attrID );
      *pBuf++ = HI_UINT16( statusRec->attrID );
      *pBuf++ = statusRec->dataType;

      pBuf = zclSerializeData( statusRec->dataType, statusRec->attrData, pBuf );
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, cmd, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status);
}

/*********************************************************************
 * @fn      zcl_SendWriteRsp
 *
 * @brief   Send a Write Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   wrtieRspCmd - write response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendWriteRsp( uint8 srcEP, afAddrType_t *dstAddr,
                            uint16 clusterID, zclWriteRspCmd_t *writeRspCmd,
                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen;
  uint8 *buf;
  ZStatus_t status;

  dataLen = writeRspCmd->numAttr * ( 1 + 2 ); // status + attribute id

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 i;
    uint8 *pBuf = buf;
    for ( i = 0; i < writeRspCmd->numAttr; i++ )
    {
      *pBuf++ = writeRspCmd->attrList[i].status;
      *pBuf++ = LO_UINT16( writeRspCmd->attrList[i].attrID );
      *pBuf++ = HI_UINT16( writeRspCmd->attrList[i].attrID );
    }

    // If there's only a single status record and its status field is set to
    // SUCCESS then omit the attribute ID field.
    if ( writeRspCmd->numAttr == 1 && writeRspCmd->attrList[0].status == ZCL_STATUS_SUCCESS )
    {
      dataLen = 1;
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_WRITE_RSP, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif // ZCL_WRITE

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*********************************************************************
 * @fn      zcl_SendConfigReportCmd
 *
 * @brief   Send a Configure Reporting command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cfgReportCmd - configure reporting command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendConfigReportCmd( uint8 srcEP, afAddrType_t *dstAddr,
                          uint16 clusterID, zclCfgReportCmd_t *cfgReportCmd,
                          uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint16 dataLen = 0;
  ZStatus_t status;
  uint8 i;

  // Find out the data length
  for ( i = 0; i < cfgReportCmd->numAttr; i++ )
  {
    zclCfgReportRec_t *reportRec = &(cfgReportCmd->attrList[i]);

    dataLen += 1 + 2; // Direction + Attribute ID

    if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
    {
      dataLen += 1 + 2 + 2; // Data Type + Min + Max Reporting Intervals

      // Find out the size of the Reportable Change field (for Analog data types)
      if ( zclAnalogDataType( reportRec->dataType ) )
      {
        dataLen += zclGetDataTypeLength( reportRec->dataType );
      }
    }
    else
    {
      dataLen += 2; // Timeout Period
    }
  }

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;

    for ( i = 0; i < cfgReportCmd->numAttr; i++ )
    {
      zclCfgReportRec_t *reportRec = &(cfgReportCmd->attrList[i]);

      *pBuf++ = reportRec->direction;
      *pBuf++ = LO_UINT16( reportRec->attrID );
      *pBuf++ = HI_UINT16( reportRec->attrID );

      if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        *pBuf++ = reportRec->dataType;
        *pBuf++ = LO_UINT16( reportRec->minReportInt );
        *pBuf++ = HI_UINT16( reportRec->minReportInt );
        *pBuf++ = LO_UINT16( reportRec->maxReportInt );
        *pBuf++ = HI_UINT16( reportRec->maxReportInt );

        if ( zclAnalogDataType( reportRec->dataType ) )
        {
          pBuf = zclSerializeData( reportRec->dataType, reportRec->reportableChange, pBuf );
        }
      }
      else
      {
        *pBuf++ = LO_UINT16( reportRec->timeoutPeriod );
        *pBuf++ = HI_UINT16( reportRec->timeoutPeriod );
      }
    } // for loop

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_CONFIG_REPORT, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif

#ifdef ZCL_REPORTING_DEVICE
/*********************************************************************
 * @fn      zcl_SendConfigReportRspCmd
 *
 * @brief   Send a Configure Reporting Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cfgReportRspCmd - configure reporting response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendConfigReportRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                    uint16 clusterID, zclCfgReportRspCmd_t *cfgReportRspCmd,
                    uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen;
  uint8 *buf;
  ZStatus_t status;

  // Atrribute list (Status, Direction and Attribute ID)
  dataLen = cfgReportRspCmd->numAttr * ( 1 + 1 + 2 );

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    uint8 i;

    for ( i = 0; i < cfgReportRspCmd->numAttr; i++ )
    {
      *pBuf++ = cfgReportRspCmd->attrList[i].status;
      *pBuf++ = cfgReportRspCmd->attrList[i].direction;
      *pBuf++ = LO_UINT16( cfgReportRspCmd->attrList[i].attrID );
      *pBuf++ = HI_UINT16( cfgReportRspCmd->attrList[i].attrID );
    }

    // If there's only a single status record and its status field is set to
    // SUCCESS then omit the attribute ID field.
    if ( cfgReportRspCmd->numAttr == 1 && cfgReportRspCmd->attrList[0].status == ZCL_STATUS_SUCCESS )
    {
      dataLen = 1;
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID,
                              ZCL_CMD_CONFIG_REPORT_RSP, FALSE, direction,
                              disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*********************************************************************
 * @fn      zcl_SendReadReportCfgCmd
 *
 * @brief   Send a Read Reporting Configuration command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readReportCfgCmd - read reporting configuration command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendReadReportCfgCmd( uint8 srcEP, afAddrType_t *dstAddr,
                  uint16 clusterID, zclReadReportCfgCmd_t *readReportCfgCmd,
                  uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen;
  uint8 *buf;
  ZStatus_t status;

  dataLen = readReportCfgCmd->numAttr * ( 1 + 2 ); // Direction + Atrribute ID

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    uint8 i;

    for ( i = 0; i < readReportCfgCmd->numAttr; i++ )
    {
      *pBuf++ = readReportCfgCmd->attrList[i].direction;
      *pBuf++ = LO_UINT16( readReportCfgCmd->attrList[i].attrID );
      *pBuf++ = HI_UINT16( readReportCfgCmd->attrList[i].attrID );
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_READ_REPORT_CFG, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif

#ifdef ZCL_REPORTING_DEVICE
/*********************************************************************
 * @fn      zcl_SendReadReportCfgRspCmd
 *
 * @brief   Send a Read Reporting Configuration Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readReportCfgRspCmd - read reporting configuration response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendReadReportCfgRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
             uint16 clusterID, zclReadReportCfgRspCmd_t *readReportCfgRspCmd,
             uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint16 dataLen = 0;
  ZStatus_t status;
  uint8 i;

  // Find out the data length
  for ( i = 0; i < readReportCfgRspCmd->numAttr; i++ )
  {
    zclReportCfgRspRec_t *reportRspRec = &(readReportCfgRspCmd->attrList[i]);

    dataLen += 1 + 1 + 2 ; // Status, Direction and Atrribute ID

    if ( reportRspRec->status == ZCL_STATUS_SUCCESS )
    {
      if ( reportRspRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        dataLen += 1 + 2 + 2; // Data Type + Min + Max Reporting Intervals

        // Find out the size of the Reportable Change field (for Analog data types)
        if ( zclAnalogDataType( reportRspRec->dataType ) )
        {
          dataLen += zclGetDataTypeLength( reportRspRec->dataType );
        }
      }
      else
      {
        dataLen += 2; // Timeout Period
      }
    }
  }

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;

    for ( i = 0; i < readReportCfgRspCmd->numAttr; i++ )
    {
      zclReportCfgRspRec_t *reportRspRec = &(readReportCfgRspCmd->attrList[i]);

      *pBuf++ = reportRspRec->status;
      *pBuf++ = reportRspRec->direction;
      *pBuf++ = LO_UINT16( reportRspRec->attrID );
      *pBuf++ = HI_UINT16( reportRspRec->attrID );

      if ( reportRspRec->status == ZCL_STATUS_SUCCESS )
      {
        if ( reportRspRec->direction == ZCL_SEND_ATTR_REPORTS )
        {
          *pBuf++ = reportRspRec->dataType;
          *pBuf++ = LO_UINT16( reportRspRec->minReportInt );
          *pBuf++ = HI_UINT16( reportRspRec->minReportInt );
          *pBuf++ = LO_UINT16( reportRspRec->maxReportInt );
          *pBuf++ = HI_UINT16( reportRspRec->maxReportInt );

          if ( zclAnalogDataType( reportRspRec->dataType ) )
          {
            pBuf = zclSerializeData( reportRspRec->dataType,
                                     reportRspRec->reportableChange, pBuf );
          }
        }
        else
        {
          *pBuf++ = LO_UINT16( reportRspRec->timeoutPeriod );
          *pBuf++ = HI_UINT16( reportRspRec->timeoutPeriod );
        }
      }
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID,
                              ZCL_CMD_READ_REPORT_CFG_RSP, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendReportCmd
 *
 * @brief   Send a Report command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   reportCmd - report command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendReportCmd( uint8 srcEP, afAddrType_t *dstAddr,
                             uint16 clusterID, zclReportCmd_t *reportCmd,
                             uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen = 0;
  uint8 *buf;
  ZStatus_t status;
  uint8 i;

  // calculate the size of the command
  for ( i = 0; i < reportCmd->numAttr; i++ )
  {
    zclReport_t *reportRec = &(reportCmd->attrList[i]);

    dataLen += 2 + 1; // Attribute ID + data type

    // Attribute Data
    dataLen += zclGetAttrDataLength( reportRec->dataType, reportRec->attrData );
  }

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;

    for ( i = 0; i < reportCmd->numAttr; i++ )
    {
      zclReport_t *reportRec = &(reportCmd->attrList[i]);

      *pBuf++ = LO_UINT16( reportRec->attrID );
      *pBuf++ = HI_UINT16( reportRec->attrID );
      *pBuf++ = reportRec->dataType;

      pBuf = zclSerializeData( reportRec->dataType, reportRec->attrData, pBuf );
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_REPORT, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif

/*********************************************************************
 * @fn      zcl_SendDefaultRspCmd
 *
 * @brief   Send a Default Response command
 *
 *          Note: The manufacturer code field should be set if this
 *          command is being sent in response to a manufacturer specific
 *          command.
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   defaultRspCmd - default response command to be sent
 * @param   direction - direction of the command
 * @param   manuCode - manufacturer code for proprietary extensions to a profile
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDefaultRspCmd( uint8 srcEP, afAddrType_t *dstAddr, uint16 clusterID,
                                 zclDefaultRspCmd_t *defaultRspCmd, uint8 direction,
                                 uint8 disableDefaultRsp, uint16 manuCode, uint8 seqNum )
{
  uint8 buf[2]; // Command ID and Status;

  // Load the buffer - serially
  buf[0] = defaultRspCmd->commandID;
  buf[1] = defaultRspCmd->statusCode;

  return ( zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DEFAULT_RSP, FALSE,
                            direction, disableDefaultRsp, manuCode, seqNum, 2, buf ) );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zcl_SendDiscoverCmdsCmd
 *
 * @brief   Send a Discover Commands command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cmdType - requesting command ID
 * @param   pDiscoverCmd - discover command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDiscoverCmdsCmd( uint8 srcEP, afAddrType_t *dstAddr, uint16 clusterID,
                                  uint8 cmdType, zclDiscoverCmdsCmd_t *pDiscoverCmd,
                                  uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payload[2]; // size of startCmdID and maxCmdID
  ZStatus_t status;

  payload[0] = pDiscoverCmd->startCmdID;
  payload[1] = pDiscoverCmd->maxCmdID;

  // Send message for either commands received or generated
  if ( cmdType == ZCL_CMD_DISCOVER_CMDS_RECEIVED )
  {
    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_CMDS_RECEIVED, FALSE,
                                direction, disableDefaultRsp, 0, seqNum, sizeof(payload), payload );
  }
  else  // generated
  {
    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_CMDS_GEN, FALSE,
                                direction, disableDefaultRsp, 0, seqNum, sizeof(payload), payload );
  }

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendDiscoverCmdsRspCmd
 *
 * @brief   Send a Discover Commands Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   pDiscoverRspCmd - response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDiscoverCmdsRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                                      uint16 clusterID, zclDiscoverCmdsCmdRsp_t *pDiscoverRspCmd,
                                      uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 payloadSize = ( 1 + pDiscoverRspCmd->numCmd );  // size of discovery complete field plus cmds
  uint8 *pCmdBuf;
  uint8 i;
  ZStatus_t status = ZSuccess;

  // allocate memory
  pCmdBuf = zcl_mem_alloc( payloadSize );
  if ( pCmdBuf != NULL )
  {
    uint8 *pBuf = pCmdBuf;

    // Load the buffer - serially
    *pBuf++ = pDiscoverRspCmd->discComplete;
    for ( i = 0; i < pDiscoverRspCmd->numCmd; i++ )
    {
      *pBuf++ = pDiscoverRspCmd->pCmdID[i];
    }

    // Send response message for either commands received or generated
    if( pDiscoverRspCmd->cmdType == ZCL_CMD_DISCOVER_CMDS_RECEIVED )
    {
      status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP, FALSE,
                                direction, disableDefaultRsp, 0, seqNum, payloadSize, pCmdBuf );
    }
    else if ( pDiscoverRspCmd->cmdType == ZCL_CMD_DISCOVER_CMDS_GEN )
    {
      status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_CMDS_GEN_RSP, FALSE,
                                direction, disableDefaultRsp, 0, seqNum, payloadSize, pCmdBuf );
    }

    zcl_mem_free( pCmdBuf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendDiscoverAttrsCmd
 *
 * @brief   Send a Discover Attributes command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   pDiscoverCmd - discover command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDiscoverAttrsCmd( uint8 srcEP, afAddrType_t *dstAddr,
                            uint16 clusterID, zclDiscoverAttrsCmd_t *pDiscoverCmd,
                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 dataLen = 2 + 1; // Start Attribute ID and Max Attribute IDs
  uint8 *buf;
  ZStatus_t status;

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    *pBuf++ = LO_UINT16(pDiscoverCmd->startAttr);
    *pBuf++ = HI_UINT16(pDiscoverCmd->startAttr);
    *pBuf++ = pDiscoverCmd->maxAttrIDs;

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_ATTRS, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendDiscoverAttrsRspCmd
 *
 * @brief   Send a Discover Attributes Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   reportRspCmd - report response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDiscoverAttrsRspCmd( uint8 srcEP, afAddrType_t *dstAddr,
                          uint16 clusterID, zclDiscoverAttrsRspCmd_t *pDiscoverRspCmd,
                          uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen = 1; // Discovery complete
  uint8 *buf;
  ZStatus_t status;

  // calculate the size of the command
  dataLen += pDiscoverRspCmd->numAttr * (2 + 1); // Attribute ID and Data Type

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 i;
    uint8 *pBuf = buf;

    *pBuf++ = pDiscoverRspCmd->discComplete;

    for ( i = 0; i < pDiscoverRspCmd->numAttr; i++ )
    {
      *pBuf++ = LO_UINT16(pDiscoverRspCmd->attrList[i].attrID);
      *pBuf++ = HI_UINT16(pDiscoverRspCmd->attrList[i].attrID);
      *pBuf++ = pDiscoverRspCmd->attrList[i].dataType;
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_ATTRS_RSP, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendDiscoverAttrsExt
 *
 * @brief   Send a Discover Attributes Extended command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   pDiscoverAttrsExt:
 *            - startAttrID: the first attribute to be selected
 *            - maxAttrIDs: maximum number of returned attributes
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDiscoverAttrsExt( uint8 srcEP, afAddrType_t *dstAddr,
                            uint16 clusterID, zclDiscoverAttrsCmd_t *pDiscoverAttrsExt,
                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 buf[3];  // Buffer size equal to Start Attribute ID and Max Attribute IDs
  ZStatus_t status;

  // Load the buffer - serially
  buf[0] = LO_UINT16(pDiscoverAttrsExt->startAttr);
  buf[1] = HI_UINT16(pDiscoverAttrsExt->startAttr);
  buf[2] = pDiscoverAttrsExt->maxAttrIDs;

  status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_ATTRS_EXT, FALSE,
                            direction, disableDefaultRsp, 0, seqNum, sizeof( buf ), buf );

  return ( status );
}

/*********************************************************************
 * @fn      zcl_SendDiscoverAttrsExtRsp
 *
 * @brief   Send a Discover Attributes Extended Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   pDiscoverRspCmd:
 *            - discComplete: indicates whether all requested attributes returned
 *            - attrID: attribute ID
 *            - attrDataType: data type of the given attribute
 *            - attrAccessControl: access control of the given attribute
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendDiscoverAttrsExtRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                      uint16 clusterID, zclDiscoverAttrsExtRsp_t *pDiscoverRspCmd,
                                      uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint8 i;
  uint16 dataLen = 1; // Discovery complete
  ZStatus_t status;

  // calculate the size of the command
  dataLen += pDiscoverRspCmd->numAttr * (2 + 1 + 1); // Attribute ID, Data Type, and Access Control

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    *pBuf++ = pDiscoverRspCmd->discComplete;
    for ( i = 0; i < pDiscoverRspCmd->numAttr; i++ )
    {
      *pBuf++ = LO_UINT16(pDiscoverRspCmd->aExtAttrInfo[i].attrID);
      *pBuf++ = HI_UINT16(pDiscoverRspCmd->aExtAttrInfo[i].attrID);
      *pBuf++ = pDiscoverRspCmd->aExtAttrInfo[i].attrDataType;
      *pBuf++ = pDiscoverRspCmd->aExtAttrInfo[i].attrAccessControl;
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_DISCOVER_ATTRS_EXT_RSP, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif // ZCL_DISCOVER

/*********************************************************************
 * @fn      zcl_ProcessMessageMSG
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   pkt - incoming message
 *
 * @return  zclProcMsgStatus_t
 */
zclProcMsgStatus_t zcl_ProcessMessageMSG( afIncomingMSGPacket_t *pkt )
{
  endPointDesc_t *epDesc;
  zclIncoming_t inMsg;
  zclLibPlugin_t *pInPlugin;
  zclDefaultRspCmd_t defautlRspCmd;
  uint8 options;
  uint8 securityEnable;
  uint8 interPanMsg;
  ZStatus_t status = ZFailure;
  uint8 defaultResponseSent = FALSE;

  if ( pkt->cmd.DataLength < ZCL_VALID_MIN_HEADER_LEN  )
  {
    return ( ZCL_PROC_INVALID );   // Error, ignore the message
  }

  // Initialize
  rawAFMsg = (afIncomingMSGPacket_t *)pkt;
  inMsg.msg = pkt;
  inMsg.attrCmd = NULL;
  inMsg.pData = NULL;
  inMsg.pDataLen = 0;

  inMsg.pData = zclParseHdr( &(inMsg.hdr), pkt->cmd.Data );
  inMsg.pDataLen = pkt->cmd.DataLength;
  inMsg.pDataLen -= (uint16)(inMsg.pData - pkt->cmd.Data);

  // Temporary workaround to allow callback functions access to the
  // transaction sequence number.  Callback functions will call
  // zcl_getParsedTransSeqNum() to retrieve this number.
  savedZCLTransSeqNum = inMsg.hdr.transSeqNum;

  // Find the wanted endpoint
  epDesc = afFindEndPointDesc( pkt->endPoint );
  if ( epDesc == NULL )
  {
    rawAFMsg = NULL;
    return ( ZCL_PROC_EP_NOT_FOUND );   // Error, ignore the message
  }

  if ( ( epDesc->simpleDesc == NULL ) ||
       ( zcl_DeviceOperational( pkt->endPoint, pkt->clusterId, inMsg.hdr.fc.type,
                                inMsg.hdr.commandID, epDesc->simpleDesc->AppProfId ) == FALSE ) )
  {
    rawAFMsg = NULL;
    return ( ZCL_PROC_NOT_OPERATIONAL ); // Error, ignore the message
  }

#if defined ( INTER_PAN )
  if ( StubAPS_InterPan( pkt->srcAddr.panId, pkt->srcAddr.endPoint ) )
  {
    // No foundation command is supported thru Inter-PAN communication.
    // But the Light Link cluster uses a different Frame Control format
    // for it's Inter-PAN messages, where the messages could be confused
    // with the foundation commands.
    if ( zcl_ProfileCmd( inMsg.hdr.fc.type ) )
    {
      rawAFMsg = NULL;
      return ( ZCL_PROC_INTERPAN_FOUNDATION_CMD );
    }

    interPanMsg = TRUE;
    options = AF_TX_OPTIONS_NONE;
  }
  else
#endif
  {
    interPanMsg = FALSE;
    options = zclGetClusterOption( pkt->endPoint, pkt->clusterId );
  }

  // Find the appropriate plugin
  pInPlugin = zclFindPlugin( pkt->clusterId, epDesc->simpleDesc->AppProfId );

  // Local and remote Security options must match except for Default Response command
  if ( ( pInPlugin != NULL ) && !zcl_DefaultRspCmd( inMsg.hdr ) )
  {
    securityEnable = ( options & AF_EN_SECURITY ) ? TRUE : FALSE;

    // Make sure that Clusters specifically defined to use security are received secure,
    // any other cluster that wants to use APS security will be allowed
    if ( ( securityEnable == TRUE ) && ( pkt->SecurityUse == FALSE ) )
    {
      if ( UNICAST_MSG( inMsg.msg ) )
      {
        // Send a Default Response command back with no Application Link Key security
        zclSetSecurityOption( pkt->endPoint, pkt->clusterId, FALSE );

        defautlRspCmd.statusCode = status;
        defautlRspCmd.commandID = inMsg.hdr.commandID;
        zcl_SendDefaultRspCmd( inMsg.msg->endPoint, &(inMsg.msg->srcAddr),
                               inMsg.msg->clusterId, &defautlRspCmd,
                               !inMsg.hdr.fc.direction, true,
                               inMsg.hdr.manuCode, inMsg.hdr.transSeqNum );

        zclSetSecurityOption( pkt->endPoint, pkt->clusterId, TRUE );
      }

      rawAFMsg = NULL;
      return ( ZCL_PROC_NOT_SECURE );   // Error, ignore the message
    }
  }

  // Is this a foundation type message
  if ( !interPanMsg && zcl_ProfileCmd( inMsg.hdr.fc.type ) )
  {
    if ( inMsg.hdr.fc.manuSpecific )
    {
      // We don't support any manufacturer specific command
      status = ZCL_STATUS_UNSUP_MANU_GENERAL_COMMAND;
    }
    else if ( ( inMsg.hdr.commandID <= ZCL_CMD_MAX ) &&
              ( zclCmdTable[inMsg.hdr.commandID].pfnParseInProfile != NULL ) )
    {
      zclParseCmd_t parseCmd;

      parseCmd.endpoint = pkt->endPoint;
      parseCmd.dataLen = inMsg.pDataLen;
      parseCmd.pData = inMsg.pData;

      // Parse the command, remember that the return value is a pointer to allocated memory
      inMsg.attrCmd = zclParseCmd( inMsg.hdr.commandID, &parseCmd );
      if ( (inMsg.attrCmd != NULL) && (zclCmdTable[inMsg.hdr.commandID].pfnProcessInProfile != NULL) )
      {
        // Process the command
        if ( zclProcessCmd( inMsg.hdr.commandID, &inMsg ) == FALSE )
        {
          // Couldn't find attribute in the table.
        }
      }

      // Free the buffer
      if ( inMsg.attrCmd )
      {
        zcl_mem_free( inMsg.attrCmd );
      }

      if ( CMD_HAS_RSP( inMsg.hdr.commandID ) )
      {
        rawAFMsg = NULL;
        return ( ZCL_PROC_SUCCESS ); // We're done
      }

      status = ZSuccess;
    }
    else
    {
      // Unsupported message
      status = ZCL_STATUS_UNSUP_GENERAL_COMMAND;
    }
  }
  else  // Not a foundation type message, so it must be specific to the cluster ID.
  {
    if ( pInPlugin && pInPlugin->pfnIncomingHdlr )
    {
      // The return value of the plugin function will be
      //  ZSuccess - Supported and need default response
      //  ZFailure - Unsupported
      //  ZCL_STATUS_CMD_HAS_RSP - Supported and do not need default rsp
      //  ZCL_STATUS_INVALID_FIELD - Supported, but the incoming msg is wrong formatted
      //  ZCL_STATUS_INVALID_VALUE - Supported, but the request not achievable by the h/w
      //  ZCL_STATUS_SOFTWARE_FAILURE - Supported but ZStack memory allocation fails
      status = pInPlugin->pfnIncomingHdlr( &inMsg );
      if ( status == ZCL_STATUS_CMD_HAS_RSP || ( interPanMsg && status == ZSuccess ) )
      {
        rawAFMsg = NULL;
        return ( ZCL_PROC_SUCCESS ); // We're done
      }
    }

    if ( status == ZFailure )
    {
      // Unsupported message
      if ( inMsg.hdr.fc.manuSpecific )
      {
        status = ZCL_STATUS_UNSUP_MANU_CLUSTER_COMMAND;
      }
      else
      {
        status = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
      }
    }
  }

  if ( UNICAST_MSG( inMsg.msg ) && inMsg.hdr.fc.disableDefaultRsp == 0 )
  {
    // Send a Default Response command back
    defautlRspCmd.statusCode = status;
    defautlRspCmd.commandID = inMsg.hdr.commandID;
    zcl_SendDefaultRspCmd( inMsg.msg->endPoint, &(inMsg.msg->srcAddr),
                           inMsg.msg->clusterId, &defautlRspCmd,
                           !inMsg.hdr.fc.direction, true,
                           inMsg.hdr.manuCode, inMsg.hdr.transSeqNum );
    defaultResponseSent = TRUE;
  }

  rawAFMsg = NULL;
  if ( status == ZSuccess )
  {
    return ( ZCL_PROC_SUCCESS );
  }
  else if ( status == ZCL_STATUS_UNSUP_MANU_GENERAL_COMMAND )
  {
    if ( defaultResponseSent )
    {
      return ( ZCL_PROC_MANUFACTURER_SPECIFIC_DR );
    }
    else
    {
      return ( ZCL_PROC_MANUFACTURER_SPECIFIC );
    }
  }
  else
  {
    if ( defaultResponseSent )
    {
      return ( ZCL_PROC_NOT_HANDLED_DR );
    }
    else
    {
      return ( ZCL_PROC_NOT_HANDLED );
    }
  }
}

/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      zclParseHdr
 *
 * @brief   Parse header of the ZCL format
 *
 * @param   hdr - place to put the frame control information
 * @param   pData - incoming buffer to parse
 *
 * @return  pointer past the header
 */
uint8 *zclParseHdr( zclFrameHdr_t *hdr, uint8 *pData )
{
  // Clear the header
  zcl_memset( (uint8 *)hdr, 0, sizeof ( zclFrameHdr_t ) );

  // Parse the Frame Control
  hdr->fc.type = zcl_FCType( *pData );
  hdr->fc.manuSpecific = zcl_FCManuSpecific( *pData ) ? 1 : 0;
  if ( zcl_FCDirection( *pData ) )
  {
    hdr->fc.direction = ZCL_FRAME_SERVER_CLIENT_DIR;
  }
  else
  {
    hdr->fc.direction = ZCL_FRAME_CLIENT_SERVER_DIR;
  }

  hdr->fc.disableDefaultRsp = zcl_FCDisableDefaultRsp( *pData ) ? 1 : 0;
  pData++;  // move past the frame control field

  // parse the manfacturer code
  if ( hdr->fc.manuSpecific )
  {
    hdr->manuCode = BUILD_UINT16( pData[0], pData[1] );
    pData += 2;
  }

  // parse the Transaction Sequence Number
  hdr->transSeqNum = *pData++;

  // parse the Cluster's command ID
  hdr->commandID = *pData++;

  // Should point to the frame payload
  return ( pData );
}

/*********************************************************************
 * @fn      zclBuildHdr
 *
 * @brief   Build header of the ZCL format
 *
 * @param   hdr - outgoing header information
 * @param   pData - outgoing header space
 *
 * @return  pointer past the header
 */
static uint8 *zclBuildHdr( zclFrameHdr_t *hdr, uint8 *pData )
{
  // Build the Frame Control byte
  *pData = hdr->fc.type;
  *pData |= hdr->fc.manuSpecific << 2;
  *pData |= hdr->fc.direction << 3;
  *pData |= hdr->fc.disableDefaultRsp << 4;
  pData++;  // move past the frame control field

  // Add the manfacturer code
  if ( hdr->fc.manuSpecific )
  {
    *pData++ = LO_UINT16( hdr->manuCode );
    *pData++ = HI_UINT16( hdr->manuCode );
  }

  // Add the Transaction Sequence Number
  *pData++ = hdr->transSeqNum;

  // Add the Cluster's command ID
  *pData++ = hdr->commandID;

  // Should point to the frame payload
  return ( pData );
}

/*********************************************************************
 * @fn      zclCalcHdrSize
 *
 * @brief   Calculate the number of bytes needed for an outgoing
 *          ZCL header.
 *
 * @param   hdr - outgoing header information
 *
 * @return  returns the number of bytes needed
 */
static uint8 zclCalcHdrSize( zclFrameHdr_t *hdr )
{
  uint8 needed = (1 + 1 + 1); // frame control + transaction seq num + cmd ID

  // Add the manfacturer code
  if ( hdr->fc.manuSpecific )
  {
    needed += 2;
  }

  return ( needed );
}

/*********************************************************************
 * @fn      zclFindPlugin
 *
 * @brief   Find the right plugin for a cluster ID
 *
 * @param   clusterID - cluster ID to look for
 * @param   profileID - profile ID
 *
 * @return  pointer to plugin, NULL if not found
 */
static zclLibPlugin_t *zclFindPlugin( uint16 clusterID, uint16 profileID )
{
  zclLibPlugin_t *pLoop = plugins;

  (void)profileID;  // Intentionally unreferenced parameter

  while ( pLoop != NULL )
  {
    if ( ( clusterID >= pLoop->startClusterID ) && ( clusterID <= pLoop->endClusterID ) )
    {
      return ( pLoop );
    }

    pLoop = pLoop->next;
  }

  return ( (zclLibPlugin_t *)NULL );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclFindCmdRecsList
 *
 * @brief   Find the right command record list for an endpoint
 *
 * @param   endpoint - endpoint to look for
 *
 * @return  pointer to record list, NULL if not found
 */
static zclCmdRecsList_t *zclFindCmdRecsList( uint8 endpoint )
{
  zclCmdRecsList_t *pLoop = gpCmdList;

  while ( pLoop != NULL )
  {
    if ( pLoop->endpoint == endpoint )
    {
      return ( pLoop );
    }

    pLoop = pLoop->pNext;
  }

  return ( NULL );
}

/*********************************************************************
 * @fn      zclFindCmdRec
 *
 * @brief   Find the command record that matchs the parameters
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID
 * @param   attrId - attribute looking for
 * @param   pAttr - attribute record to be returned
 *
 * @return  TRUE if record found. FALSE, otherwise.
 */
uint8 zclFindCmdRec( uint8 endpoint, uint16 clusterID, uint8 cmdID, zclCommandRec_t *pCmd )
{
  uint8 i;
  zclCmdRecsList_t *pRec = zclFindCmdRecsList( endpoint );

  if ( pRec != NULL )
  {
    for ( i = 0; i < pRec->numCommands; i++ )
    {
      if ( pRec->pCmdRecs[i].clusterID == clusterID && pRec->pCmdRecs[i].cmdID == cmdID )
      {
        *pCmd = pRec->pCmdRecs[i];

        return ( TRUE ); // EMBEDDED RETURN
      }
    }
  }

  return ( FALSE );
}
#endif // ZCL_DISCOVER

/*********************************************************************
 * @fn      zclFindAttrRecsList
 *
 * @brief   Find the right attribute record list for an endpoint
 *
 * @param   clusterID - endpointto look for
 *
 * @return  pointer to record list, NULL if not found
 */
zclAttrRecsList *zclFindAttrRecsList( uint8 endpoint )
{
  zclAttrRecsList *pLoop = attrList;

  while ( pLoop != NULL )
  {
    if ( pLoop->endpoint == endpoint )
    {
      return ( pLoop );
    }

    pLoop = pLoop->next;
  }

  return ( NULL );
}

/*********************************************************************
 * @fn      zclFindAttrRec
 *
 * @brief   Find the attribute record that matchs the parameters
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID
 * @param   attrId - attribute looking for
 * @param   pAttr - attribute record to be returned
 *
 * @return  TRUE if record found. FALSE, otherwise.
 */
uint8 zclFindAttrRec( uint8 endpoint, uint16 clusterID, uint16 attrId, zclAttrRec_t *pAttr )
{
  uint8 x;
  zclAttrRecsList *pRec = zclFindAttrRecsList( endpoint );

  if ( pRec != NULL )
  {
    for ( x = 0; x < pRec->numAttributes; x++ )
    {
      if ( pRec->attrs[x].clusterID == clusterID && pRec->attrs[x].attr.attrId == attrId )
      {
        *pAttr = pRec->attrs[x];

        return ( TRUE ); // EMBEDDED RETURN
      }
    }
  }

  return ( FALSE );
}

#if defined ( ZCL_STANDALONE )
/*********************************************************************
 * @fn      zclSetAttrRecList
 *
 * @brief   Set attribute record list for end point
 *
 * @param   endpoint - endpoint the attribute list belongs to
 * @param   numAttr - number of attributes in list
 * @param   attrList - array of attribute records.
 *                     NOTE: THE ATTRIBUTE IDs (FOR A CLUSTER) MUST BE
 *                     IN ASCENDING ORDER. OTHERWISE, THE DISCOVERY
 *                     RESPONSE COMMAND WILL NOT HAVE THE RIGHT
 *                     ATTRIBUTE INFO
 *
 * @return  TRUE if successful, FALSE otherwise.
 */
uint8 zclSetAttrRecList( uint8 endpoint, uint8 numAttr, CONST zclAttrRec_t attrList[] )
{
  zclAttrRecsList *pRecsList = zclFindAttrRecsList( endpoint );

  if ( pRecsList != NULL )
  {
    pRecsList->numAttributes = numAttr;
    pRecsList->attrs = attrList;
    return ( TRUE );
  }

  return ( FALSE );
}

#endif // ZCL_STANDALONE

#if defined ( ZCL_READ ) || defined ( ZCL_WRITE )
/*********************************************************************
 * @fn      zclGetReadWriteCB
 *
 * @brief   Get the Read/Write callback function pointer for a given endpoint.
 *
 * @param   endpoint - Application's endpoint
 *
 * @return  Read/Write CB, NULL if not found
 */
static zclReadWriteCB_t zclGetReadWriteCB( uint8 endpoint )
{
  zclAttrRecsList *pRec = zclFindAttrRecsList( endpoint );

  if ( pRec != NULL )
  {
    return ( pRec->pfnReadWriteCB );
  }

  return ( NULL );
}

/*********************************************************************
 * @fn      zclGetAuthorizeCB
 *
 * @brief   Get the Read/Write Authorization callback function pointer
 *          for a given endpoint.
 *
 * @param   endpoint - Application's endpoint
 *
 * @return  Authorization CB, NULL if not found
 */
static zclAuthorizeCB_t zclGetAuthorizeCB( uint8 endpoint )
{
  zclAttrRecsList *pRec = zclFindAttrRecsList( endpoint );

  if ( pRec != NULL )
  {
    return ( pRec->pfnAuthorizeCB );
  }

  return ( NULL );
}
#endif // ZCL_READ || ZCL_WRITE

/*********************************************************************
 * @fn      zclFindClusterOption
 *
 * @brief   Find the option record that matchs the cluster id
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID looking for
 *
 * @return  pointer to clutser option, NULL if not found
 */
static zclOptionRec_t *zclFindClusterOption( uint8 endpoint, uint16 clusterID )
{
  zclClusterOptionList *pLoop;

  pLoop = clusterOptionList;
  while ( pLoop != NULL )
  {
    if ( pLoop->endpoint == endpoint )
    {
      uint8 x;
      for ( x = 0; x < pLoop->numOptions; x++ )
      {
        if ( pLoop->options[x].clusterID == clusterID )
        {
          return ( &(pLoop->options[x]) ); // EMBEDDED RETURN
        }
      }
    }

    pLoop = pLoop->next;
  }

  return ( NULL );
}

/*********************************************************************
 * @fn      zclGetClusterOption
 *
 * @brief   Get the option record that matchs the cluster id
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID looking for
 *
 * @return  clutser option, AF_TX_OPTIONS_NONE if not found
 */
static uint8 zclGetClusterOption( uint8 endpoint, uint16 clusterID )
{
  uint8 option;
  zclOptionRec_t *pOption;

  pOption = zclFindClusterOption( endpoint, clusterID );
  if ( pOption != NULL )
  {
    option = pOption->option;
    if ( !ZG_SECURE_ENABLED )
    {
      option &= (AF_EN_SECURITY ^ 0xFF); // make sure Application Link Key security is off
    }

    return ( option ); // EMBEDDED RETURN
  }

  return ( AF_TX_OPTIONS_NONE );
}

/*********************************************************************
 * @fn      zclSetSecurityOption
 *
 * @brief   Set the security option for the cluster id
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID looking for
 * @param   enable - whether to enable (TRUE) or disable (FALSE) security option
 *
 * @return  none
 */
static void zclSetSecurityOption( uint8 endpoint, uint16 clusterID, uint8 enable )
{
  zclOptionRec_t *pOption;

  pOption = zclFindClusterOption( endpoint, clusterID );
  if ( pOption != NULL )
  {
    if ( enable )
    {
      pOption->option |= AF_EN_SECURITY;
    }
    else
    {
      pOption->option &= (AF_EN_SECURITY ^ 0xFF);
    }
  }
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclFindNextCmdRec
 *
 * @brief   Find the command (or next) record that matchs the parameters
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID
 * @param   commandID - command ID from requesting command
 * @param   direction- direction of received command
 * @param   pCmdID - command looking for
 * @param   pCmd - command information within command record list
 *
 * @return  pointer to command record, NULL no more records of this cluster
 */
static uint8 zclFindNextCmdRec( uint8 endpoint, uint16 clusterID, uint8 commandID,
                                uint8 direction, uint8 *pCmdID, zclCommandRec_t *pCmd )
{
  zclCmdRecsList_t *pRec = zclFindCmdRecsList( endpoint );
  uint8 i;

  if ( pRec != NULL )
  {
    for ( i = 0; i < pRec->numCommands; i++ )
    {
      if ( ( pRec->pCmdRecs[i].clusterID == clusterID ) &&
          ( pRec->pCmdRecs[i].cmdID >= *pCmdID ) )
      {
        if ( commandID == ZCL_CMD_DISCOVER_CMDS_RECEIVED )
        {
          if ( ( direction == ZCL_FRAME_SERVER_CLIENT_DIR ) && ( pRec->pCmdRecs[i].flag & CMD_DIR_CLIENT_RECEIVED ) )
          {
            *pCmd = pRec->pCmdRecs[i];

            // Update command ID
            *pCmdID = pCmd->cmdID;

            return ( TRUE ); // EMBEDDED RETURN
          }
          else if ( ( direction == ZCL_FRAME_CLIENT_SERVER_DIR ) && ( pRec->pCmdRecs[i].flag & CMD_DIR_SERVER_RECEIVED ) )
          {
            *pCmd = pRec->pCmdRecs[i];

            // Update command ID
            *pCmdID = pCmd->cmdID;

            return ( TRUE ); // EMBEDDED RETURN
          }
        }
        else if ( commandID == ZCL_CMD_DISCOVER_CMDS_GEN )
        {
          if ( ( direction == ZCL_FRAME_CLIENT_SERVER_DIR ) && ( pRec->pCmdRecs[i].flag & CMD_DIR_SERVER_GENERATED ) )
          {
            *pCmd = pRec->pCmdRecs[i];

            // Update command ID
            *pCmdID = pCmd->cmdID;

            return ( TRUE ); // EMBEDDED RETURN
          }
          else if ( ( direction == ZCL_FRAME_SERVER_CLIENT_DIR ) && ( pRec->pCmdRecs[i].flag & CMD_DIR_CLIENT_GENERATED ) )
          {
            *pCmd = pRec->pCmdRecs[i];

            // Update command ID
            *pCmdID = pCmd->cmdID;

            return ( TRUE ); // EMBEDDED RETURN
          }
        }
        else
        {
          return ( FALSE ); // Incorrect Command ID
        }
      }
    }
  }

  return ( FALSE );
}

/*********************************************************************
 * @fn      zclFindNextAttrRec
 *
 * @brief   Find the attribute (or next) record that matchs the parameters
 *
 * @param   endpoint - Application's endpoint
 * @param   clusterID - cluster ID
 * @param   attr - attribute looking for
 *
 * @return  pointer to attribute record, NULL if not found
 */
static uint8 zclFindNextAttrRec( uint8 endpoint, uint16 clusterID, uint8 direction,
                                 uint16 *attrId, zclAttrRec_t *pAttr )
{
  zclAttrRecsList *pRec = zclFindAttrRecsList( endpoint );
  uint8 attrDir;

  if ( pRec != NULL )
  {
    uint16 x;

    for ( x = 0; x < pRec->numAttributes; x++ )
    {
      if ( ( pRec->attrs[x].clusterID == clusterID ) &&
           ( pRec->attrs[x].attr.attrId >= *attrId ) )
      {
        // also make sure direction is right
        attrDir = (pRec->attrs[x].attr.accessControl & ACCESS_CLIENT) ? 1 : 0;
        if ( (attrDir == direction) || (pRec->attrs[x].attr.accessControl & ACCESS_GLOBAL))
        {
          // return attribute and found attribute ID
          *pAttr = pRec->attrs[x];
          *attrId = pAttr->attr.attrId;

          return ( TRUE ); // EMBEDDED RETURN
        }
      }
    }
  }

  return ( FALSE );
}
#endif // ZCL_DISCOVER

/*********************************************************************
 * @fn      zclSerializeData
 *
 * @brief   Builds a buffer from the attribute data to sent out over
 *          the air.
 *          NOTE - Not compatible with application's attributes callbacks.
 *
 * @param   dataType - data types defined in zcl.h
 * @param   attrData - pointer to the attribute data
 * @param   buf - where to put the serialized data
 *
 * @return  pointer to end of destination buffer
 */
uint8 *zclSerializeData( uint8 dataType, void *attrData, uint8 *buf )
{
  uint8 *pStr;
  uint16 len;

  if ( attrData == NULL )
  {
    return ( buf );
  }

  switch ( dataType )
  {
    case ZCL_DATATYPE_DATA8:
    case ZCL_DATATYPE_BOOLEAN:
    case ZCL_DATATYPE_BITMAP8:
    case ZCL_DATATYPE_INT8:
    case ZCL_DATATYPE_UINT8:
    case ZCL_DATATYPE_ENUM8:
      *buf++ = *((uint8 *)attrData);
       break;

    case ZCL_DATATYPE_DATA16:
    case ZCL_DATATYPE_BITMAP16:
    case ZCL_DATATYPE_UINT16:
    case ZCL_DATATYPE_INT16:
    case ZCL_DATATYPE_ENUM16:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_CLUSTER_ID:
    case ZCL_DATATYPE_ATTR_ID:
      *buf++ = LO_UINT16( *((uint16*)attrData) );
      *buf++ = HI_UINT16( *((uint16*)attrData) );
      break;

    case ZCL_DATATYPE_DATA24:
    case ZCL_DATATYPE_BITMAP24:
    case ZCL_DATATYPE_UINT24:
    case ZCL_DATATYPE_INT24:
      *buf++ = BREAK_UINT32( *((uint32*)attrData), 0 );
      *buf++ = BREAK_UINT32( *((uint32*)attrData), 1 );
      *buf++ = BREAK_UINT32( *((uint32*)attrData), 2 );
      break;

    case ZCL_DATATYPE_DATA32:
    case ZCL_DATATYPE_BITMAP32:
    case ZCL_DATATYPE_UINT32:
    case ZCL_DATATYPE_INT32:
    case ZCL_DATATYPE_SINGLE_PREC:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
    case ZCL_DATATYPE_BAC_OID:
      buf = zcl_buffer_uint32( buf, *((uint32*)attrData) );
      break;

    case ZCL_DATATYPE_BITMAP40:
    case ZCL_DATATYPE_UINT40:
    case ZCL_DATATYPE_INT40:
      pStr = (uint8*)attrData;
      buf = zcl_memcpy( buf, pStr, 5 );
      break;

    case ZCL_DATATYPE_BITMAP48:
    case ZCL_DATATYPE_UINT48:
    case ZCL_DATATYPE_INT48:
      pStr = (uint8*)attrData;
      buf = zcl_memcpy( buf, pStr, 6 );
      break;

    case ZCL_DATATYPE_BITMAP56:
    case ZCL_DATATYPE_UINT56:
    case ZCL_DATATYPE_INT56:
      pStr = (uint8*)attrData;
      buf = zcl_memcpy( buf, pStr, 7 );
      break;

    case ZCL_DATATYPE_BITMAP64:
    case ZCL_DATATYPE_DOUBLE_PREC:
    case ZCL_DATATYPE_IEEE_ADDR:
    case ZCL_DATATYPE_UINT64:
    case ZCL_DATATYPE_INT64:
      pStr = (uint8*)attrData;
      buf = zcl_memcpy( buf, pStr, 8 );
      break;

    case ZCL_DATATYPE_CHAR_STR:
    case ZCL_DATATYPE_OCTET_STR:
      pStr = (uint8*)attrData;
      len = *pStr;
      buf = zcl_memcpy( buf, pStr, len+1 ); // Including length field
      break;

    case ZCL_DATATYPE_LONG_CHAR_STR:
    case ZCL_DATATYPE_LONG_OCTET_STR:
      pStr = (uint8*)attrData;
      len = BUILD_UINT16( pStr[0], pStr[1] );
      buf = zcl_memcpy( buf, pStr, len+2 ); // Including length field
      break;

    case ZCL_DATATYPE_128_BIT_SEC_KEY:
      pStr = (uint8*)attrData;
      buf = zcl_memcpy( buf, pStr, SEC_KEY_LEN );
      break;

    case ZCL_DATATYPE_NO_DATA:
    case ZCL_DATATYPE_UNKNOWN:
      // Fall through

    default:
      break;
  }

  return ( buf );
}

#if defined ZCL_REPORTING_DEVICE || defined ZCL_REPORT_CONFIGURING_DEVICE
/*********************************************************************
 * @fn      zclAnalogDataType
 *
 * @brief   Checks to see if Data Type is Analog
 *
 * @param   dataType - data type
 *
 * @return  TRUE if data type is analog
 */
uint8 zclAnalogDataType( uint8 dataType )
{
  uint8 analog;

  switch ( dataType )
  {
    case ZCL_DATATYPE_UINT8:
    case ZCL_DATATYPE_UINT16:
    case ZCL_DATATYPE_UINT24:
    case ZCL_DATATYPE_UINT32:
    case ZCL_DATATYPE_UINT40:
    case ZCL_DATATYPE_UINT48:
    case ZCL_DATATYPE_UINT56:
    case ZCL_DATATYPE_UINT64:
    case ZCL_DATATYPE_INT8:
    case ZCL_DATATYPE_INT16:
    case ZCL_DATATYPE_INT24:
    case ZCL_DATATYPE_INT32:
    case ZCL_DATATYPE_INT40:
    case ZCL_DATATYPE_INT48:
    case ZCL_DATATYPE_INT56:
    case ZCL_DATATYPE_INT64:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_SINGLE_PREC:
    case ZCL_DATATYPE_DOUBLE_PREC:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
      analog = TRUE;
      break;

    default:
      analog = FALSE;
      break;
  }

  return ( analog );
}

/*********************************************************************
 * @fn      zclIsLittleEndianMachine
 *
 * @brief   Verifies endianness in system.
 *
 * @param   none
 *
 * @return  MSB-00 or LSB-01 depending on endianness in the system
 */
static int zclIsLittleEndianMachine(void)
{
  uint16 test = 0x0001;

  return (*((uint8 *)(&test)));
}

/*********************************************************************
 * @fn      zcl_BuildAnalogData
 *
 * @brief   Build an analog arribute out of sequential bytes.
 *
 * @param   dataType - type of data
 * @param   pData - pointer to data
 * @param   pBuf - where to put the data
 *
 * @return  none
 */
static void zcl_BuildAnalogData( uint8 dataType, uint8 *pData, uint8 *pBuf )
{
  int current_byte_index;
  int remaining_bytes;
  int step;

  remaining_bytes = zclGetAttrDataLength(dataType, pData);

  // decide if move forward or backwards to copy data
  if ( zclIsLittleEndianMachine() )
  {
    step = 1;
    current_byte_index = 0;
  }
  else
  {
    step = -1;
    current_byte_index = remaining_bytes - 1;
  }

  while ( remaining_bytes-- )
  {
    pData[current_byte_index] = *(pBuf++);
    current_byte_index += step;
  }
}
#endif

/*********************************************************************
 * @fn      zclGetDataTypeLength
 *
 * @brief   Return the length of the datatype in octet.
 *
 *          NOTE: Should not be called for ZCL_DATATYPE_OCTECT_STR or
 *                ZCL_DATATYPE_CHAR_STR data types.
 *
 * @param   dataType - data type
 *
 * @return  length of data
 */
uint8 zclGetDataTypeLength( uint8 dataType )
{
  uint8 len;

  switch ( dataType )
  {
    case ZCL_DATATYPE_DATA8:
    case ZCL_DATATYPE_BOOLEAN:
    case ZCL_DATATYPE_BITMAP8:
    case ZCL_DATATYPE_INT8:
    case ZCL_DATATYPE_UINT8:
    case ZCL_DATATYPE_ENUM8:
      len = 1;
      break;

    case ZCL_DATATYPE_DATA16:
    case ZCL_DATATYPE_BITMAP16:
    case ZCL_DATATYPE_UINT16:
    case ZCL_DATATYPE_INT16:
    case ZCL_DATATYPE_ENUM16:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_CLUSTER_ID:
    case ZCL_DATATYPE_ATTR_ID:
      len = 2;
      break;

    case ZCL_DATATYPE_DATA24:
    case ZCL_DATATYPE_BITMAP24:
    case ZCL_DATATYPE_UINT24:
    case ZCL_DATATYPE_INT24:
      len = 3;
      break;

    case ZCL_DATATYPE_DATA32:
    case ZCL_DATATYPE_BITMAP32:
    case ZCL_DATATYPE_UINT32:
    case ZCL_DATATYPE_INT32:
    case ZCL_DATATYPE_SINGLE_PREC:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
    case ZCL_DATATYPE_BAC_OID:
      len = 4;
      break;

   case ZCL_DATATYPE_UINT40:
   case ZCL_DATATYPE_INT40:
       len = 5;
       break;

   case ZCL_DATATYPE_UINT48:
   case ZCL_DATATYPE_INT48:
       len = 6;
       break;

   case ZCL_DATATYPE_UINT56:
   case ZCL_DATATYPE_INT56:
       len = 7;
       break;

   case ZCL_DATATYPE_DOUBLE_PREC:
   case ZCL_DATATYPE_IEEE_ADDR:
   case ZCL_DATATYPE_UINT64:
   case ZCL_DATATYPE_INT64:
     len = 8;
     break;

    case ZCL_DATATYPE_128_BIT_SEC_KEY:
     len = SEC_KEY_LEN;
     break;

    case ZCL_DATATYPE_NO_DATA:
    case ZCL_DATATYPE_UNKNOWN:
      // Fall through

    default:
      len = 0;
      break;
  }

  return ( len );
}

/*********************************************************************
 * @fn      zclGetAttrDataLength
 *
 * @brief   Return the length of the attribute.
 *
 * @param   dataType - data type
 * @param   pData - pointer to data
 *
 * @return  returns atrribute length
 */
uint16 zclGetAttrDataLength( uint8 dataType, uint8 *pData )
{
  uint16 dataLen = 0;

  if ( dataType == ZCL_DATATYPE_LONG_CHAR_STR || dataType == ZCL_DATATYPE_LONG_OCTET_STR )
  {
    dataLen = BUILD_UINT16( pData[0], pData[1] ) + 2; // long string length + 2 for length field
  }
  else if ( dataType == ZCL_DATATYPE_CHAR_STR || dataType == ZCL_DATATYPE_OCTET_STR )
  {
    dataLen = *pData + 1; // string length + 1 for length field
  }
  else
  {
    dataLen = zclGetDataTypeLength( dataType );
  }

  return ( dataLen );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclReadAttrData
 *
 * @brief   Read the attribute's current value into pAttrData.
 *          NOTE - Not compatible with application's attributes callbacks.
 *
 * @param   pAttrData - where to put attribute data
 * @param   pAttr - pointer to attribute
 * @param   pDataLen - where to put attribute data length
 *
 * @return Success
 */
ZStatus_t zclReadAttrData( uint8 *pAttrData, zclAttrRec_t *pAttr, uint16 *pDataLen )
{
  uint16 dataLen;

  if ( pAttr->attr.dataPtr == NULL )
  {
    return ( ZCL_STATUS_FAILURE );
  }

  dataLen = zclGetAttrDataLength( pAttr->attr.dataType, (uint8*)(pAttr->attr.dataPtr) );
  zcl_memcpy( pAttrData, pAttr->attr.dataPtr, dataLen );

  if ( pDataLen != NULL )
  {
    *pDataLen = dataLen;
  }

  return ( ZCL_STATUS_SUCCESS );
}

/*********************************************************************
 * @fn      zcl_ReadAttrData
 *
 * @brief   Read the attribute's current value into pAttrData.
 *          Use application's callback function if assigned to this attribute.
 *
 * @param   endpoint - application's endpoint
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute id
 * @param   pAttrData - where to put attribute data
 * @param   pDataLen - where to put attribute data length
 *
 * @return  Successful if data was read
 */
ZStatus_t zcl_ReadAttrData( uint8 endpoint, uint16 clusterId, uint16 attrId,
                                         uint8 *pAttrData, uint16 *pDataLen )
{
  zclAttrRec_t attrRec;

  if ( zclFindAttrRec( endpoint, clusterId, attrId, &attrRec ) == FALSE )
  {
    return ( ZCL_STATUS_FAILURE );
  }

  if ( attrRec.attr.dataPtr != NULL )
  {
    return zclReadAttrData( pAttrData, &attrRec, pDataLen );
  }
  else
  {
    return zclReadAttrDataUsingCB( endpoint, clusterId, attrId, pAttrData, pDataLen );
  }
}

/*********************************************************************
 * @fn      zclGetAttrDataLengthUsingCB
 *
 * @brief   Use application's callback to get the length of the attribute's
 *          current value stored in the database.
 *
 * @param   endpoint - application's endpoint
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute id
 *
 * @return  returns attribute length
 */
static uint16 zclGetAttrDataLengthUsingCB( uint8 endpoint, uint16 clusterId, uint16 attrId )
{
  uint16 dataLen = 0;
  zclReadWriteCB_t pfnReadWriteCB = zclGetReadWriteCB( endpoint );

  if ( pfnReadWriteCB != NULL )
  {
    // Only get the attribute length
    (*pfnReadWriteCB)( clusterId, attrId, ZCL_OPER_LEN, NULL, &dataLen );
  }

  return ( dataLen );
}

/*********************************************************************
 * @fn      zclReadAttrDataUsingCB
 *
 * @brief   Use application's callback to read the attribute's current
 *          value stored in the database.
 *
 * @param   endpoint - application's endpoint
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute id
 * @param   pAttrData - where to put attribute data
 * @param   pDataLen - where to put attribute data length
 *
 * @return  Successful if data was read
 */
static ZStatus_t zclReadAttrDataUsingCB( uint8 endpoint, uint16 clusterId, uint16 attrId,
                                         uint8 *pAttrData, uint16 *pDataLen )
{
  zclReadWriteCB_t pfnReadWriteCB = zclGetReadWriteCB( endpoint );

  if ( pDataLen != NULL )
  {
    *pDataLen = 0; // Always initialize it to 0
  }

  if ( pfnReadWriteCB != NULL )
  {
    // Read the attribute value and its length
    return ( (*pfnReadWriteCB)( clusterId, attrId, ZCL_OPER_READ, pAttrData, pDataLen ) );
  }

  return ( ZCL_STATUS_SOFTWARE_FAILURE );
}

/*********************************************************************
 * @fn      zclAuthorizeRead
 *
 * @brief   Use application's callback to authorize a Read operation
 *          on a given attribute.
 *
 * @param   endpoint - application's endpoint
 * @param   srcAddr - source Address
 * @param   pAttr - pointer to attribute
 *
 * @return  ZCL_STATUS_SUCCESS: Operation authorized
 *          ZCL_STATUS_NOT_AUTHORIZED: Operation not authorized
 */
static ZStatus_t zclAuthorizeRead( uint8 endpoint, afAddrType_t *srcAddr, zclAttrRec_t *pAttr )
{
  if ( zcl_AccessCtrlAuthRead( pAttr->attr.accessControl ) )
  {
    zclAuthorizeCB_t pfnAuthorizeCB = zclGetAuthorizeCB( endpoint );

    if ( pfnAuthorizeCB != NULL )
    {
      return ( (*pfnAuthorizeCB)( srcAddr, pAttr, ZCL_OPER_READ ) );
    }
  }

  return ( ZCL_STATUS_SUCCESS );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclWriteAttrData
 *
 * @brief   Write the received data.
 *
 * @param   endpoint - application's endpoint
 * @param   pAttr - where to write data to
 * @param   pWriteRec - data to be written
 *
 * @return  Successful if data was written
 */
static ZStatus_t zclWriteAttrData( uint8 endpoint, afAddrType_t *srcAddr,
                                   zclAttrRec_t *pAttr, zclWriteRec_t *pWriteRec )
{
  uint8 status;

  if ( zcl_AccessCtrlWrite( pAttr->attr.accessControl ) )
  {
    status = zclAuthorizeWrite( endpoint, srcAddr, pAttr );
    if ( status == ZCL_STATUS_SUCCESS )
    {
      if ( ( zcl_ValidateAttrDataCB == NULL ) || zcl_ValidateAttrDataCB( pAttr, pWriteRec ) )
      {
        // Write the attribute value
        uint16 len = zclGetAttrDataLength( pAttr->attr.dataType, pWriteRec->attrData );
        zcl_memcpy( pAttr->attr.dataPtr, pWriteRec->attrData, len );

        status = ZCL_STATUS_SUCCESS;
      }
      else
      {
        status = ZCL_STATUS_INVALID_VALUE;
      }
    }
  }
  else
  {
    status = ZCL_STATUS_READ_ONLY;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclWriteAttrDataUsingCB
 *
 * @brief   Use application's callback to write the attribute's current
 *          value stored in the database.
 *
 * @param   endpoint - application's endpoint
 * @param   pAttr - where to write data to
 * @param   pAttrData - data to be written
 *
 * @return  Successful if data was written
 */
static ZStatus_t zclWriteAttrDataUsingCB( uint8 endpoint, afAddrType_t *srcAddr,
                                          zclAttrRec_t *pAttr, uint8 *pAttrData )
{
  uint8 status;

  if ( zcl_AccessCtrlWrite( pAttr->attr.accessControl ) )
  {
    status = zclAuthorizeWrite( endpoint, srcAddr, pAttr );
    if ( status == ZCL_STATUS_SUCCESS )
    {
      zclReadWriteCB_t pfnReadWriteCB = zclGetReadWriteCB( endpoint );
      if ( pfnReadWriteCB != NULL )
      {
        // Write the attribute value
        status = (*pfnReadWriteCB)( pAttr->clusterID, pAttr->attr.attrId,
                                    ZCL_OPER_WRITE, pAttrData, NULL );
      }
      else
      {
        status = ZCL_STATUS_SOFTWARE_FAILURE;
      }
    }
  }
  else
  {
    status = ZCL_STATUS_READ_ONLY;
  }

  return ( status );
}

/*********************************************************************
 * @fn      zclAuthorizeWrite
 *
 * @brief   Use application's callback to authorize a Write operation
 *          on a given attribute.
 *
 * @param   endpoint - application's endpoint
 * @param   srcAddr - source Address
 * @param   pAttr - pointer to attribute
 *
 * @return  ZCL_STATUS_SUCCESS: Operation authorized
 *          ZCL_STATUS_NOT_AUTHORIZED: Operation not authorized
 */
static ZStatus_t zclAuthorizeWrite( uint8 endpoint, afAddrType_t *srcAddr, zclAttrRec_t *pAttr )
{
  if ( zcl_AccessCtrlAuthWrite( pAttr->attr.accessControl ) )
  {
    zclAuthorizeCB_t pfnAuthorizeCB = zclGetAuthorizeCB( endpoint );

    if ( pfnAuthorizeCB != NULL )
    {
      return ( (*pfnAuthorizeCB)( srcAddr, pAttr, ZCL_OPER_WRITE ) );
    }
  }

  return ( ZCL_STATUS_SUCCESS );
}
#endif // ZCL_WRITE

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclParseInReadCmd
 *
 * @brief   Parse the "Profile" Read Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInReadCmd( zclParseCmd_t *pCmd )
{
  zclReadCmd_t *readCmd;
  uint8 *pBuf = pCmd->pData;

  readCmd = (zclReadCmd_t *)zcl_mem_alloc( sizeof ( zclReadCmd_t ) + pCmd->dataLen );
  if ( readCmd != NULL )
  {
    uint8 i;
    readCmd->numAttr = pCmd->dataLen / 2; // Atrribute ID
    for ( i = 0; i < readCmd->numAttr; i++ )
    {
      readCmd->attrID[i] = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
    }
  }

  return ( (void *)readCmd );
}

/*********************************************************************
 * @fn      zclParseInReadRspCmd
 *
 * @brief   Parse the "Profile" Read Response Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
static void *zclParseInReadRspCmd( zclParseCmd_t *pCmd )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 *dataPtr;
  uint8 numAttr = 0;
  uint8 hdrLen;
  uint16 dataLen = 0;
  uint16 attrDataLen;

  // find out the number of attributes and the length of attribute data
  while ( pBuf < ( pCmd->pData + pCmd->dataLen ) )
  {
    uint8 status;

    numAttr++;
    pBuf += 2; // move pass attribute id

    status = *pBuf++;
    if ( status == ZCL_STATUS_SUCCESS )
    {
      uint8 dataType = *pBuf++;

      attrDataLen = zclGetAttrDataLength( dataType, pBuf );
      pBuf += attrDataLen; // move pass attribute data

      // add padding if needed
      if ( PADDING_NEEDED( attrDataLen ) )
      {
        attrDataLen++;
      }

      dataLen += attrDataLen;
    }
  }

  // calculate the length of the response header
  hdrLen = sizeof( zclReadRspCmd_t ) + ( numAttr * sizeof( zclReadRspStatus_t ) );

  readRspCmd = (zclReadRspCmd_t *)zcl_mem_alloc( hdrLen + dataLen );
  if ( readRspCmd != NULL )
  {
    uint8 i;
    pBuf = pCmd->pData;
    dataPtr = (uint8 *)( (uint8 *)readRspCmd + hdrLen );

    readRspCmd->numAttr = numAttr;
    for ( i = 0; i < numAttr; i++ )
    {
      zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);

      statusRec->attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      statusRec->status = *pBuf++;
      if ( statusRec->status == ZCL_STATUS_SUCCESS )
      {
        statusRec->dataType = *pBuf++;

        attrDataLen = zclGetAttrDataLength( statusRec->dataType, pBuf );
        zcl_memcpy( dataPtr, pBuf, attrDataLen);
        statusRec->data = dataPtr;

        pBuf += attrDataLen; // move pass attribute data

        // advance attribute data pointer
        if ( PADDING_NEEDED( attrDataLen ) )
        {
          attrDataLen++;
        }

        dataPtr += attrDataLen;
      }
    }
  }

  return ( (void *)readRspCmd );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclParseInWriteCmd
 *
 * @brief   Parse the "Profile" Write, Write Undivided and Write No
 *          Response Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInWriteCmd( zclParseCmd_t *pCmd )
{
  zclWriteCmd_t *writeCmd;
  uint8 *pBuf = pCmd->pData;
  uint16 attrDataLen;
  uint8 *dataPtr;
  uint8 numAttr = 0;
  uint8 hdrLen;
  uint16 dataLen = 0;

  // find out the number of attributes and the length of attribute data
  while ( pBuf < ( pCmd->pData + pCmd->dataLen ) )
  {
    uint8 dataType;

    numAttr++;
    pBuf += 2; // move pass attribute id

    dataType = *pBuf++;

    attrDataLen = zclGetAttrDataLength( dataType, pBuf );
    pBuf += attrDataLen; // move pass attribute data

    // add padding if needed
    if ( PADDING_NEEDED( attrDataLen ) )
    {
      attrDataLen++;
    }

    dataLen += attrDataLen;
  }

  // calculate the length of the response header
  hdrLen = sizeof( zclWriteCmd_t ) + ( numAttr * sizeof( zclWriteRec_t ) );

  writeCmd = (zclWriteCmd_t *)zcl_mem_alloc( hdrLen + dataLen );
  if ( writeCmd != NULL )
  {
    uint8 i;
    pBuf = pCmd->pData;
    dataPtr = (uint8 *)( (uint8 *)writeCmd + hdrLen );

    writeCmd->numAttr = numAttr;
    for ( i = 0; i < numAttr; i++ )
    {
      zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

      statusRec->attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      statusRec->dataType = *pBuf++;

      attrDataLen = zclGetAttrDataLength( statusRec->dataType, pBuf );
      zcl_memcpy( dataPtr, pBuf, attrDataLen);
      statusRec->attrData = dataPtr;

      pBuf += attrDataLen; // move pass attribute data

      // advance attribute data pointer
      if ( PADDING_NEEDED( attrDataLen ) )
      {
        attrDataLen++;
      }

      dataPtr += attrDataLen;
    }
  }

  return ( (void *)writeCmd );
}

/*********************************************************************
 * @fn      zclParseInWriteRspCmd
 *
 * @brief   Parse the "Profile" Write Response Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
static void *zclParseInWriteRspCmd( zclParseCmd_t *pCmd )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 i = 0;

  writeRspCmd = (zclWriteRspCmd_t *)zcl_mem_alloc( sizeof ( zclWriteRspCmd_t ) + pCmd->dataLen );
  if ( writeRspCmd != NULL )
  {
    if ( pCmd->dataLen == 1 )
    {
      // special case when all writes were successfull
      writeRspCmd->attrList[i++].status = *pBuf;
    }
    else
    {
      while ( pBuf < ( pCmd->pData + pCmd->dataLen ) )
      {
        writeRspCmd->attrList[i].status = *pBuf++;
        writeRspCmd->attrList[i++].attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;
      }
    }

    writeRspCmd->numAttr = i;
  }

  return ( (void *)writeRspCmd );
}
#endif // ZCL_WRITE

#ifdef ZCL_REPORTING_DEVICE
/*********************************************************************
 * @fn      zclParseInConfigReportCmd
 *
 * @brief   Parse the "Profile" Configure Reporting Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInConfigReportCmd( zclParseCmd_t *pCmd )
{
  zclCfgReportCmd_t *cfgReportCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 *dataPtr;
  uint8 numAttr = 0;
  uint8 dataType;
  uint8 hdrLen;
  uint16 dataLen = 0;
  uint8 reportChangeLen; // length of Reportable Change field

  // Calculate the length of the Request command
  while ( pBuf < ( pCmd->pData + pCmd->dataLen ) )
  {
    uint8 direction;

    numAttr++;
    direction = *pBuf++;
    pBuf += 2; // move pass the attribute ID

    // Is there a Reportable Change field?
    if ( direction == ZCL_SEND_ATTR_REPORTS )
    {
      dataType = *pBuf++;
      pBuf += 4; // move pass the Min and Max Reporting Intervals

      // For attributes of 'discrete' data types this field is omitted
      if ( zclAnalogDataType( dataType ) )
      {
        reportChangeLen = zclGetDataTypeLength( dataType );
        pBuf += reportChangeLen;

        // add padding if needed
        if ( PADDING_NEEDED( reportChangeLen ) )
        {
          reportChangeLen++;
        }

        dataLen += reportChangeLen;
      }
      else
      {
        pBuf++; // move past reportable change field
      }
    }
    else
    {
      pBuf += 2; // move pass the Timeout Period
    }
  } // while loop

  hdrLen = sizeof( zclCfgReportCmd_t ) + ( numAttr * sizeof( zclCfgReportRec_t ) );

  cfgReportCmd = (zclCfgReportCmd_t *)zcl_mem_alloc( hdrLen + dataLen );
  if ( cfgReportCmd != NULL )
  {
    uint8 i;
    pBuf = pCmd->pData;
    dataPtr = (uint8 *)( (uint8 *)cfgReportCmd + hdrLen );

    cfgReportCmd->numAttr = numAttr;
    for ( i = 0; i < numAttr; i++ )
    {
      zclCfgReportRec_t *reportRec = &(cfgReportCmd->attrList[i]);

      zcl_memset( reportRec, 0, sizeof( zclCfgReportRec_t ) );

      reportRec->direction = *pBuf++;
      reportRec->attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        // Attribute to be reported
        reportRec->dataType = *pBuf++;
        reportRec->minReportInt = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;
        reportRec->maxReportInt = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;

        // For attributes of 'discrete' data types this field is omitted
        if ( zclAnalogDataType( reportRec->dataType ) )
        {
          zcl_BuildAnalogData( reportRec->dataType, dataPtr, pBuf);
          reportRec->reportableChange = dataPtr;

          reportChangeLen = zclGetDataTypeLength( reportRec->dataType );
          pBuf += reportChangeLen;

          // advance attribute data pointer
          if ( PADDING_NEEDED( reportChangeLen ) )
          {
            reportChangeLen++;
          }

          dataPtr += reportChangeLen;
        }
      }
      else
      {
        // Attribute reports to be received
        reportRec->timeoutPeriod = BUILD_UINT16( pBuf[0], pBuf[1] );
        pBuf += 2;
      }
    } // while loop
  }

  return ( (void *)cfgReportCmd );
}
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*********************************************************************
 * @fn      zclParseInConfigReportRspCmd
 *
 * @brief   Parse the "Profile" Configure Reporting Response Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
static void *zclParseInConfigReportRspCmd( zclParseCmd_t *pCmd )
{
  zclCfgReportRspCmd_t *cfgReportRspCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 numAttr;

  numAttr = pCmd->dataLen / ( 1 + 1 + 2 ); // Status + Direction + Attribute ID

  cfgReportRspCmd = (zclCfgReportRspCmd_t *)zcl_mem_alloc( sizeof( zclCfgReportRspCmd_t )
                                            + ( numAttr * sizeof( zclCfgReportStatus_t ) ) );
  if ( cfgReportRspCmd != NULL )
  {
    uint8 i;
    cfgReportRspCmd->numAttr = numAttr;
    for ( i = 0; i < cfgReportRspCmd->numAttr; i++ )
    {
      cfgReportRspCmd->attrList[i].status = *pBuf++;
      cfgReportRspCmd->attrList[i].direction = *pBuf++;
      cfgReportRspCmd->attrList[i].attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
    }
  }

  return ( (void *)cfgReportRspCmd );
}
#endif

#ifdef ZCL_REPORTING_DEVICE
/*********************************************************************
 * @fn      zclParseInReadReportCfgCmd
 *
 * @brief   Parse the "Profile" Read Reporting Configuration Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInReadReportCfgCmd( zclParseCmd_t *pCmd )
{
  zclReadReportCfgCmd_t *readReportCfgCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 numAttr;

  numAttr = pCmd->dataLen / ( 1 + 2 ); // Direction + Attribute ID

  readReportCfgCmd = (zclReadReportCfgCmd_t *)zcl_mem_alloc( sizeof( zclReadReportCfgCmd_t )
                                                  + ( numAttr * sizeof( zclReadReportCfgRec_t ) ) );
  if ( readReportCfgCmd != NULL )
  {
    uint8 i;
    readReportCfgCmd->numAttr = numAttr;
    for ( i = 0; i < readReportCfgCmd->numAttr; i++)
    {
      readReportCfgCmd->attrList[i].direction = *pBuf++;;
      readReportCfgCmd->attrList[i].attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
    }
  }

  return ( (void *)readReportCfgCmd );
}
#endif

#ifdef ZCL_REPORT_CONFIGURING_DEVICE
/*********************************************************************
 * @fn      zclParseInReadReportCfgRspCmd
 *
 * @brief   Parse the "Profile" Read Reporting Configuration Response Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
static void *zclParseInReadReportCfgRspCmd( zclParseCmd_t *pCmd )
{
  zclReadReportCfgRspCmd_t *readReportCfgRspCmd;
  uint8 reportChangeLen;
  uint8 *pBuf = pCmd->pData;
  uint8 *dataPtr;
  uint8 numAttr = 0;
  uint8 hdrLen;
  uint16 dataLen = 0;

  // Calculate the length of the response command
  while ( pBuf < ( pCmd->pData + pCmd->dataLen ) )
  {
    uint8 status;
    uint8 direction;

    numAttr++;
    status = *pBuf++;
    direction = *pBuf++;
    pBuf += 2; // move pass the attribute ID

    if ( status == ZCL_STATUS_SUCCESS )
    {
      if ( direction == ZCL_SEND_ATTR_REPORTS )
      {
        uint8 dataType = *pBuf++;
        pBuf += 4; // move pass the Min and Max Reporting Intervals

        // For attributes of 'discrete' data types this field is omitted
        if ( zclAnalogDataType( dataType ) )
        {
          reportChangeLen = zclGetDataTypeLength( dataType );
          pBuf += reportChangeLen;

          // add padding if needed
          if ( PADDING_NEEDED( reportChangeLen ) )
          {
            reportChangeLen++;
          }

          dataLen += reportChangeLen;
        }
      }
      else
      {
        pBuf += 2; // move pass the Timeout field
      }
    }
  } // while loop

  hdrLen = sizeof( zclReadReportCfgRspCmd_t ) + ( numAttr * sizeof( zclReportCfgRspRec_t ) );

  readReportCfgRspCmd = (zclReadReportCfgRspCmd_t *)zcl_mem_alloc( hdrLen + dataLen );
  if ( readReportCfgRspCmd != NULL )
  {
    uint8 i;
    pBuf = pCmd->pData;
    dataPtr = (uint8 *)( (uint8 *)readReportCfgRspCmd + hdrLen );

    readReportCfgRspCmd->numAttr = numAttr;
    for ( i = 0; i < numAttr; i++ )
    {
      zclReportCfgRspRec_t *reportRspRec = &(readReportCfgRspCmd->attrList[i]);

      reportRspRec->status = *pBuf++;
      reportRspRec->direction = *pBuf++;
      reportRspRec->attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;

      if ( reportRspRec->status == ZCL_STATUS_SUCCESS )
      {
        if ( reportRspRec->direction == ZCL_SEND_ATTR_REPORTS )
        {
          reportRspRec->dataType = *pBuf++;
          reportRspRec->minReportInt = BUILD_UINT16( pBuf[0], pBuf[1] );
          pBuf += 2;
          reportRspRec->maxReportInt = BUILD_UINT16( pBuf[0], pBuf[1] );
          pBuf += 2;

          if ( zclAnalogDataType( reportRspRec->dataType ) )
          {
            zcl_BuildAnalogData( reportRspRec->dataType, dataPtr, pBuf);
            reportRspRec->reportableChange = dataPtr;

            reportChangeLen = zclGetDataTypeLength( reportRspRec->dataType );
            pBuf += reportChangeLen;

            // advance attribute data pointer
            if ( PADDING_NEEDED( reportChangeLen ) )
            {
              reportChangeLen++;
            }

            dataPtr += reportChangeLen;
          }
        }
        else
        {
          reportRspRec->timeoutPeriod = BUILD_UINT16( pBuf[0], pBuf[1] );
          pBuf += 2;
        }
      }
    }
  }

  return ( (void *)readReportCfgRspCmd );
}
#endif

#ifdef ZCL_REPORT_DESTINATION_DEVICE
/*********************************************************************
 * @fn      zclParseInReportCmd
 *
 * @brief   Parse the "Profile" Report Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInReportCmd( zclParseCmd_t *pCmd )
{
  zclReportCmd_t *reportCmd;
  uint8 *pBuf = pCmd->pData;
  uint16 attrDataLen;
  uint8 *dataPtr;
  uint8 numAttr = 0;
  uint8 hdrLen;
  uint16 dataLen = 0;

  // find out the number of attributes and the length of attribute data
  while ( pBuf < ( pCmd->pData + pCmd->dataLen ) )
  {
    uint8 dataType;

    numAttr++;
    pBuf += 2; // move pass attribute id

    dataType = *pBuf++;

    attrDataLen = zclGetAttrDataLength( dataType, pBuf );
    pBuf += attrDataLen; // move pass attribute data

    // add padding if needed
    if ( PADDING_NEEDED( attrDataLen ) )
    {
      attrDataLen++;
    }

    dataLen += attrDataLen;
  }

  hdrLen = sizeof( zclReportCmd_t ) + ( numAttr * sizeof( zclReport_t ) );

  reportCmd = (zclReportCmd_t *)zcl_mem_alloc( hdrLen + dataLen );
  if (reportCmd != NULL )
  {
    uint8 i;
    pBuf = pCmd->pData;
    dataPtr = (uint8 *)( (uint8 *)reportCmd + hdrLen );

    reportCmd->numAttr = numAttr;
    for ( i = 0; i < numAttr; i++ )
    {
      zclReport_t *reportRec = &(reportCmd->attrList[i]);

      reportRec->attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      reportRec->dataType = *pBuf++;

      attrDataLen = zclGetAttrDataLength( reportRec->dataType, pBuf );
      zcl_memcpy( dataPtr, pBuf, attrDataLen );
      reportRec->attrData = dataPtr;

      pBuf += attrDataLen; // move pass attribute data

      // advance attribute data pointer
      if ( PADDING_NEEDED( attrDataLen ) )
      {
        attrDataLen++;
      }

      dataPtr += attrDataLen;
    }
  }

  return ( (void *)reportCmd );
}
#endif

/*********************************************************************
 * @fn      zclParseInDefaultRspCmd
 *
 * @brief   Parse the "Profile" Default Response Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
static void *zclParseInDefaultRspCmd( zclParseCmd_t *pCmd )
{
  zclDefaultRspCmd_t *defaultRspCmd;
  uint8 *pBuf = pCmd->pData;

  defaultRspCmd = (zclDefaultRspCmd_t *)zcl_mem_alloc( sizeof ( zclDefaultRspCmd_t ) );
  if ( defaultRspCmd != NULL )
  {
    defaultRspCmd->commandID = *pBuf++;
    defaultRspCmd->statusCode = *pBuf;
  }

  return ( (void *)defaultRspCmd );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclParseInDiscAttrsCmd
 *
 * @brief   Parse the "Profile" Discovery Attributes and Attributes Extended Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInDiscAttrsCmd( zclParseCmd_t *pCmd )
{
  zclDiscoverAttrsCmd_t *pDiscoverCmd;
  uint8 *pBuf = pCmd->pData;

  pDiscoverCmd = (zclDiscoverAttrsCmd_t *)zcl_mem_alloc( sizeof ( zclDiscoverAttrsCmd_t ) );
  if ( pDiscoverCmd != NULL )
  {
    pDiscoverCmd->startAttr = BUILD_UINT16( pBuf[0], pBuf[1] );
    pBuf += 2;
    pDiscoverCmd->maxAttrIDs = *pBuf;
  }

  return ( (void *)pDiscoverCmd );
}

/*********************************************************************
 * @fn      zclParseInDiscAttrsRspCmd
 *
 * @brief   Parse the "Profile" Discovery Response Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
#define ZCLDISCRSPCMD_DATALEN(a)  ((a)-1) // data len - Discovery Complete
static void *zclParseInDiscAttrsRspCmd( zclParseCmd_t *pCmd )
{
  zclDiscoverAttrsRspCmd_t *pDiscoverRspCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 numAttr = ZCLDISCRSPCMD_DATALEN(pCmd->dataLen) / ( 2 + 1 ); // Attr ID + Data Type

  pDiscoverRspCmd = (zclDiscoverAttrsRspCmd_t *)zcl_mem_alloc( sizeof ( zclDiscoverAttrsRspCmd_t ) +
                    ( numAttr * sizeof(zclDiscoverAttrInfo_t) ) );

  if ( pDiscoverRspCmd != NULL )
  {
    uint8 i;

    pDiscoverRspCmd->discComplete = *pBuf++;
    pDiscoverRspCmd->numAttr = numAttr;

    for ( i = 0; i < numAttr; i++ )
    {
      pDiscoverRspCmd->attrList[i].attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      pDiscoverRspCmd->attrList[i].dataType = *pBuf++;
    }
  }

  return ( (void *)pDiscoverRspCmd );
}

/*********************************************************************
 * @fn      zclParseInDiscCmdsCmd
 *
 * @brief   Parse the "Profile" Discovery Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
void *zclParseInDiscCmdsCmd( zclParseCmd_t *pCmd )
{
  zclDiscoverCmdsCmd_t *pDiscoverCmd;
  uint8 *pBuf = pCmd->pData;

  pDiscoverCmd = (zclDiscoverCmdsCmd_t *)zcl_mem_alloc( sizeof ( zclDiscoverCmdsCmd_t ) );
  if ( pDiscoverCmd != NULL )
  {
    pDiscoverCmd->startCmdID = *pBuf++;
    pDiscoverCmd->maxCmdID = *pBuf++;
  }

  return ( (void *)pDiscoverCmd );
}

/*********************************************************************
 * @fn      zclParseInDiscCmdsRspCmd
 *
 * @brief   Parse the Discover Commands Response Command
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
#define ZCLDISCRSPCMD_DATALEN(a)  ((a)-1) // data len - Discovery Complete
static void *zclParseInDiscCmdsRspCmd( zclParseCmd_t *pCmd )
{
  zclDiscoverCmdsCmdRsp_t *pDiscoverRspCmd;
  uint8 *pBuf = pCmd->pData;
  uint8 numCmds = ZCLDISCRSPCMD_DATALEN(pCmd->dataLen);  // length of command ID variable array

    // allocate memory for size of structure plus variable array
  pDiscoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)zcl_mem_alloc( sizeof ( zclDiscoverCmdsCmdRsp_t ) +
                    ( numCmds * sizeof(uint8) ) );
  if ( pDiscoverRspCmd != NULL )
  {
    uint8 i;
    pDiscoverRspCmd->discComplete = *pBuf++;
    pDiscoverRspCmd->numCmd = numCmds;

    for ( i = 0; i < numCmds; i++ )
    {
      pDiscoverRspCmd->pCmdID[i] = *pBuf++;
    }
  }

  return ( (void *)pDiscoverRspCmd );
}

/*********************************************************************
 * @fn      zclParseInDiscAttrsExtRspCmd
 *
 * @brief   Parse the "Profile" Discovery Extended Attributes Response Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
#define ZCLDISCRSPCMD_DATALEN(a)  ((a)-1) // data len - Discovery Complete
static void *zclParseInDiscAttrsExtRspCmd( zclParseCmd_t *pCmd )
{
  zclDiscoverAttrsExtRsp_t *pDiscoverRspCmd;
  uint8 i;
  uint8 *pBuf = pCmd->pData;
  uint8 numAttrs = ZCLDISCRSPCMD_DATALEN(pCmd->dataLen) / ( 2 + 1 + 1 ); // Attr ID + Data Type + Access Control

  pDiscoverRspCmd = (zclDiscoverAttrsExtRsp_t *)zcl_mem_alloc( sizeof ( zclDiscoverAttrsExtRsp_t ) +
                    ( numAttrs * sizeof(zclExtAttrInfo_t) ) );

  if ( pDiscoverRspCmd != NULL )
  {
    pDiscoverRspCmd->discComplete = *pBuf++;
    pDiscoverRspCmd->numAttr = numAttrs;

    for ( i = 0; i < numAttrs; i++ )
    {
      pDiscoverRspCmd->aExtAttrInfo[i].attrID = BUILD_UINT16( pBuf[0], pBuf[1] );
      pBuf += 2;
      pDiscoverRspCmd->aExtAttrInfo[i].attrDataType = *pBuf++;
      pDiscoverRspCmd->aExtAttrInfo[i].attrAccessControl = *pBuf++;
    }
  }

  return ( (void *)pDiscoverRspCmd );
}
#endif // ZCL_DISCOVER

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclProcessInReadCmd
 *
 * @brief   Process the "Profile" Read Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if command processed. FALSE, otherwise.
 */
static uint8 zclProcessInReadCmd( zclIncoming_t *pInMsg )
{
  zclReadCmd_t *readCmd;
  zclReadRspCmd_t *readRspCmd;
  zclAttrRec_t attrRec;
  uint16 len;
  uint8 i;
  uint8 attrFound;

  readCmd = (zclReadCmd_t *)pInMsg->attrCmd;

  // calculate the length of the response status record
  len = sizeof( zclReadRspCmd_t ) + (readCmd->numAttr * sizeof( zclReadRspStatus_t ));

  readRspCmd = zcl_mem_alloc( len );
  if ( readRspCmd == NULL )
  {
    return FALSE; // EMBEDDED RETURN
  }

  readRspCmd->numAttr = readCmd->numAttr;
  for ( i = 0; i < readCmd->numAttr; i++ )
  {
    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);

    statusRec->attrID = readCmd->attrID[i];
    
    attrFound = zclFindAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, readCmd->attrID[i], &attrRec );
    
    //Validate the attribute is found and the access control
    if ( ( attrFound == TRUE ) && 
         (  (attrRec.attr.accessControl & ACCESS_GLOBAL) || 
            (GET_BIT( &attrRec.attr.accessControl, ACCESS_CONTROL_MASK ) == pInMsg->hdr.fc.direction ) ) )
    {
      if ( zcl_AccessCtrlRead( attrRec.attr.accessControl ) )
      {
        statusRec->status = zclAuthorizeRead( pInMsg->msg->endPoint,
                                              &(pInMsg->msg->srcAddr), &attrRec );
        if ( statusRec->status == ZCL_STATUS_SUCCESS )
        {
          statusRec->data = attrRec.attr.dataPtr;
          statusRec->dataType = attrRec.attr.dataType;
        }
      }
      else
      {
        statusRec->status = ZCL_STATUS_WRITE_ONLY;
      }
    }
    else
    {
      statusRec->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
  }

  // Build and send Read Response command
  zcl_SendReadRsp( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr), pInMsg->msg->clusterId,
                   readRspCmd, !pInMsg->hdr.fc.direction,
                   true, pInMsg->hdr.transSeqNum );
  zcl_mem_free( readRspCmd );

  return TRUE;
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      processInWriteCmd
 *
 * @brief   Process the "Profile" Write and Write No Response Commands
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if command processed. FALSE, otherwise.
 */
static uint8 zclProcessInWriteCmd( zclIncoming_t *pInMsg )
{
  zclWriteCmd_t *writeCmd;
  zclWriteRspCmd_t *writeRspCmd;
  uint8 sendRsp = FALSE;
  uint8 j = 0;
  uint8 i;

  writeCmd = (zclWriteCmd_t *)pInMsg->attrCmd;
  if ( pInMsg->hdr.commandID == ZCL_CMD_WRITE )
  {
    // We need to send a response back - allocate space for it
    writeRspCmd = (zclWriteRspCmd_t *)zcl_mem_alloc( sizeof( zclWriteRspCmd_t )
            + sizeof( zclWriteRspStatus_t ) * writeCmd->numAttr );
    if ( writeRspCmd == NULL )
    {
      return FALSE; // EMBEDDED RETURN
    }

    sendRsp = TRUE;
  }

  for ( i = 0; i < writeCmd->numAttr; i++ )
  {
    zclAttrRec_t attrRec;
    zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

    if ( zclFindAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                         statusRec->attrID, &attrRec ) )
    {
      if ( GET_BIT( &attrRec.attr.accessControl, ACCESS_CONTROLEXT_MASK ) != pInMsg->hdr.fc.direction )
      {
        writeRspCmd->attrList[j].status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        writeRspCmd->attrList[j++].attrID = statusRec->attrID;
        break;
      }
      if ( statusRec->dataType == attrRec.attr.dataType )
      {
        uint8 status;

        // Write the new attribute value
        if ( attrRec.attr.dataPtr != NULL )
        {
          //Handle special case for Identify
          if((pInMsg->msg->clusterId == ZCL_CLUSTER_ID_GEN_IDENTIFY) && (statusRec->attrID == ATTRID_IDENTIFY_TIME))
          {
            uint16 identifyTime; 
                  
            osal_memcpy((uint8*)&identifyTime,statusRec->attrData,sizeof(uint16));
                        
            bdb_ZclIdentifyCmdInd(identifyTime, pInMsg->msg->endPoint);
            
            status = ZCL_STATUS_SUCCESS;
          }          
          else
          {                
            status = zclWriteAttrData( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                       &attrRec, statusRec );
          }
        }
        else // Use CB
        {
          status = zclWriteAttrDataUsingCB( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                            &attrRec, statusRec->attrData );
        }

        // If successful, a write attribute status record shall NOT be generated
        if ( sendRsp && status != ZCL_STATUS_SUCCESS )
        {
          // Attribute is read only - move on to the next write attribute record
          writeRspCmd->attrList[j].status = status;
          writeRspCmd->attrList[j++].attrID = statusRec->attrID;
        }
      }
      else
      {
        // Attribute data type is incorrect - move on to the next write attribute record
        if ( sendRsp )
        {
          writeRspCmd->attrList[j].status = ZCL_STATUS_INVALID_DATA_TYPE;
          writeRspCmd->attrList[j++].attrID = statusRec->attrID;
        }
      }
    }
    else
    {
      // Attribute is not supported - move on to the next write attribute record
      if ( sendRsp )
      {
        writeRspCmd->attrList[j].status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        writeRspCmd->attrList[j++].attrID = statusRec->attrID;
      }
    }
  } // for loop

  if ( sendRsp )
  {
    writeRspCmd->numAttr = j;
    if ( writeRspCmd->numAttr == 0 )
    {
      // Since all records were written successful, include a single status record
      // in the resonse command with the status field set to SUCCESS and the
      // attribute ID field omitted.
      writeRspCmd->attrList[0].status = ZCL_STATUS_SUCCESS;
      writeRspCmd->numAttr = 1;
    }

    zcl_SendWriteRsp( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                      pInMsg->msg->clusterId, writeRspCmd, !pInMsg->hdr.fc.direction,
                      true, pInMsg->hdr.transSeqNum );
    zcl_mem_free( writeRspCmd );
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclRevertWriteUndividedCmd
 *
 * @brief   Revert the "Profile" Write Undevided Command
 *
 * @param   pInMsg - incoming message to process
 * @param   curWriteRec - old data
 * @param   numAttr - number of attributes to be reverted
 *
 * @return  none
 */
static void zclRevertWriteUndividedCmd( zclIncoming_t *pInMsg,
                                    zclWriteRec_t *curWriteRec, uint16 numAttr )
{
  uint8 i;

  for ( i = 0; i < numAttr; i++ )
  {
    zclAttrRec_t attrRec;
    zclWriteRec_t *statusRec = &(curWriteRec[i]);

    if ( !zclFindAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                          statusRec->attrID, &attrRec ) )
    {
      break; // should never happen
    }

    if ( attrRec.attr.dataPtr != NULL )
    {
      // Just copy the old data back - no need to validate the data
      uint16 dataLen = zclGetAttrDataLength( attrRec.attr.dataType, statusRec->attrData );
      zcl_memcpy( attrRec.attr.dataPtr, statusRec->attrData, dataLen );
    }
    else // Use CB
    {
      // Write the old data back
      zclWriteAttrDataUsingCB( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                               &attrRec, statusRec->attrData );
    }
  } // for loop
}

/*********************************************************************
 * @fn      zclProcessInWriteUndividedCmd
 *
 * @brief   Process the "Profile" Write Undivided Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if command processed. FALSE, otherwise.
 */
static uint8 zclProcessInWriteUndividedCmd( zclIncoming_t *pInMsg )
{
  zclWriteCmd_t *writeCmd;
  zclWriteRspCmd_t *writeRspCmd;
  zclAttrRec_t attrRec;
  uint16 dataLen;
  uint16 curLen = 0;
  uint8 j = 0;
  uint8 i;

  writeCmd = (zclWriteCmd_t *)pInMsg->attrCmd;

  // Allocate space for Write Response Command
  writeRspCmd = (zclWriteRspCmd_t *)zcl_mem_alloc( sizeof( zclWriteRspCmd_t )
                   + sizeof( zclWriteRspStatus_t )* writeCmd->numAttr );
  if ( writeRspCmd == NULL )
  {
    return FALSE; // EMBEDDED RETURN
  }

  // If any attribute cannot be written, no attribute values are changed. Hence,
  // make sure all the attributes are supported and writable
  for ( i = 0; i < writeCmd->numAttr; i++ )
  {
    zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

    if ( !zclFindAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                          statusRec->attrID, &attrRec ) )
    {
      // Attribute is not supported - stop here
      writeRspCmd->attrList[j].status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
      writeRspCmd->attrList[j++].attrID = statusRec->attrID;
      break;
    }

    if ( statusRec->dataType != attrRec.attr.dataType )
    {
      // Attribute data type is incorrect - stope here
      writeRspCmd->attrList[j].status = ZCL_STATUS_INVALID_DATA_TYPE;
      writeRspCmd->attrList[j++].attrID = statusRec->attrID;
      break;
    }

    if ( !zcl_AccessCtrlWrite( attrRec.attr.accessControl ) )
    {
      // Attribute is not writable - stop here
      writeRspCmd->attrList[j].status = ZCL_STATUS_READ_ONLY;
      writeRspCmd->attrList[j++].attrID = statusRec->attrID;
      break;
    }

    if ( zcl_AccessCtrlAuthWrite( attrRec.attr.accessControl ) )
    {
      // Not authorized to write - stop here
      writeRspCmd->attrList[j].status = ZCL_STATUS_NOT_AUTHORIZED;
      writeRspCmd->attrList[j++].attrID = statusRec->attrID;
      break;
    }

    // Attribute Data length
    if ( attrRec.attr.dataPtr != NULL )
    {
      dataLen = zclGetAttrDataLength( attrRec.attr.dataType, attrRec.attr.dataPtr );
    }
    else // Use CB
    {
      dataLen = zclGetAttrDataLengthUsingCB( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                                             statusRec->attrID );
    }

    // add padding if needed
    if ( PADDING_NEEDED( dataLen ) )
    {
      dataLen++;
    }

    curLen += dataLen;
  } // for loop

  writeRspCmd->numAttr = j;
  if ( writeRspCmd->numAttr == 0 ) // All attributes can be written
  {
    uint8 *curDataPtr;
    zclWriteRec_t *curWriteRec;

    // calculate the length of the current data header
    uint8 hdrLen = j * sizeof( zclWriteRec_t );

    // Allocate space to keep a copy of the current data
    curWriteRec = (zclWriteRec_t *) zcl_mem_alloc( hdrLen + curLen );
    if ( curWriteRec == NULL )
    {
      zcl_mem_free(writeRspCmd );
      return FALSE; // EMBEDDED RETURN
    }

    curDataPtr = (uint8 *)((uint8 *)curWriteRec + hdrLen);

    // Write the new data over
    for ( i = 0; i < writeCmd->numAttr; i++ )
    {
      uint8 status;
      zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);
      zclWriteRec_t *curStatusRec = &(curWriteRec[i]);

      if ( !zclFindAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                            statusRec->attrID, &attrRec ) )
      {
        break; // should never happen
      }

      // Keep a copy of the current data before before writing the new data over
      curStatusRec->attrID = statusRec->attrID;
      curStatusRec->attrData = curDataPtr;

      if ( attrRec.attr.dataPtr != NULL )
      {
        // Read the current value
        zclReadAttrData( curDataPtr, &attrRec, &dataLen );

        // Write the new attribute value
        status = zclWriteAttrData( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                   &attrRec, statusRec );
      }
      else // Use CBs
      {
        // Read the current value
        zclReadAttrDataUsingCB( pInMsg->msg->endPoint, pInMsg->msg->clusterId,
                                statusRec->attrID, curDataPtr, &dataLen );
        // Write the new attribute value
        status = zclWriteAttrDataUsingCB( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                                          &attrRec, statusRec->attrData );
      }

      // If successful, a write attribute status record shall NOT be generated
      if ( status != ZCL_STATUS_SUCCESS )
      {
        writeRspCmd->attrList[j].status = status;
        writeRspCmd->attrList[j++].attrID = statusRec->attrID;

        // Since this write failed, we need to revert all the pervious writes
        zclRevertWriteUndividedCmd( pInMsg, curWriteRec, i);
        break;
      }

      // add padding if needed
      if ( PADDING_NEEDED( dataLen ) )
      {
        dataLen++;
      }

      curDataPtr += dataLen;
    } // for loop

    writeRspCmd->numAttr = j;
    if ( writeRspCmd->numAttr  == 0 )
    {
      // Since all records were written successful, include a single status record
      // in the resonse command with the status field set to SUCCESS and the
      // attribute ID field omitted.
      writeRspCmd->attrList[0].status = ZCL_STATUS_SUCCESS;
      writeRspCmd->numAttr = 1;
    }

    zcl_mem_free( curWriteRec );
  }

  zcl_SendWriteRsp( pInMsg->msg->endPoint, &(pInMsg->msg->srcAddr),
                    pInMsg->msg->clusterId, writeRspCmd, !pInMsg->hdr.fc.direction,
                    true, pInMsg->hdr.transSeqNum );
  zcl_mem_free( writeRspCmd );

  return TRUE;
}
#endif // ZCL_WRITE

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclProcessInDiscAttrs
 *
 * @brief   Process the "Profile" Discover Attributes Commands
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if command processed. FALSE, otherwise.
 */
static uint8 zclProcessInDiscAttrs( zclIncoming_t *pInMsg )
{
  zclDiscoverAttrsCmd_t *pDiscoverCmd;
  zclAttrRec_t attrRec;
  uint16 attrID;
  uint8 numAttrs;
  uint8 i;

  pDiscoverCmd = (zclDiscoverAttrsCmd_t *)pInMsg->attrCmd;

  // Find out the number of attributes supported within the specified range
  for ( i = 0, attrID = pDiscoverCmd->startAttr; i < pDiscoverCmd->maxAttrIDs; i++, attrID++ )
  {
    // finds the next attribute on this endpoint/cluster after the range.
    // attributes must be in numerical order in the list.
    if ( !zclFindNextAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.fc.direction, &attrID, &attrRec ) )
    {
      break;
    }
  }

  numAttrs = i;  // store range of attributes in buffer

    // Process message for either attributes or extended attributes
  if( pInMsg->hdr.commandID == ZCL_CMD_DISCOVER_ATTRS )
  {
    zclProcessInDiscAttrsCmd( pInMsg, pDiscoverCmd, numAttrs );
  }
  else if ( pInMsg->hdr.commandID == ZCL_CMD_DISCOVER_ATTRS_EXT )
  {
    zclProcessInDiscAttrsExtCmd( pInMsg, pDiscoverCmd, numAttrs );
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclProcessInDiscAttrsCmd
 *
 * @brief   Process the Discover Attributes Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @param   pDiscoverCmd - structure from requesting command
 *
 * @param   attrLenBuf - describes the amount of attributes to be processed
 *
 * @return  none
 */
static void zclProcessInDiscAttrsCmd( zclIncoming_t *pInMsg, zclDiscoverAttrsCmd_t *pDiscoverCmd, uint8 numAttrs )
{
  zclDiscoverAttrsRspCmd_t *pDiscoverRsp;
  uint8 discComplete = TRUE;
  zclAttrRec_t attrRec;
  uint16 attrID;
  uint8 i;

  // Allocate space for the response command
  pDiscoverRsp = (zclDiscoverAttrsRspCmd_t *)zcl_mem_alloc( sizeof (zclDiscoverAttrsRspCmd_t)
                                                          + sizeof ( zclDiscoverAttrInfo_t ) * numAttrs );
  if ( pDiscoverRsp == NULL )
  {
    return; // EMBEDDED RETURN
  }

  if ( numAttrs != 0 )
  {
    for ( i = 0, attrID = pDiscoverCmd->startAttr; i < numAttrs; i++, attrID++ )
    {
      if ( !zclFindNextAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.fc.direction, &attrID, &attrRec ) )
      {
        break; // should not happen, as numAttrs already calculated
      }

      pDiscoverRsp->attrList[i].attrID = attrRec.attr.attrId;
      pDiscoverRsp->attrList[i].dataType = attrRec.attr.dataType;
    }

    // Are there more attributes to be discovered?
    if ( zclFindNextAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.fc.direction, &attrID, &attrRec ) )
    {
      discComplete = FALSE;
    }
  }

  pDiscoverRsp->discComplete = discComplete;
  pDiscoverRsp->numAttr = numAttrs;

  zcl_SendDiscoverAttrsRspCmd( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                               pInMsg->msg->clusterId, pDiscoverRsp, !(pInMsg->hdr.fc.direction),
                               true, pInMsg->hdr.transSeqNum );
  zcl_mem_free( pDiscoverRsp );

  return;
}

/*********************************************************************
 * @fn      zclProcessInDiscAttrsExtCmd
 *
 * @brief   Process the Discover Attributes Extended Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @param   pDiscoverCmd - structure from requesting command
 *
 * @param   attrLenBuf - describes the amount of attributes to be processed
 *
 * @return  none
 */
static void zclProcessInDiscAttrsExtCmd( zclIncoming_t *pInMsg, zclDiscoverAttrsCmd_t *pDiscoverCmd, uint8 numAttrs )
{
  zclDiscoverAttrsExtRsp_t *pDiscoverExtRsp;
  uint8 discComplete = TRUE;
  zclAttrRec_t attrRec;
  uint16 attrID;
  uint8 i;

    // Allocate space for the response command
  pDiscoverExtRsp = (zclDiscoverAttrsExtRsp_t *)zcl_mem_alloc( sizeof (zclDiscoverAttrsExtRsp_t)
                                                         + sizeof ( zclExtAttrInfo_t ) * numAttrs );
  if ( pDiscoverExtRsp == NULL )
  {
    return; // EMBEDDED RETURN
  }


  if ( numAttrs != 0 )
  {
    for ( i = 0, attrID = pDiscoverCmd->startAttr; i < numAttrs; i++, attrID++ )
    {
      if ( !zclFindNextAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.fc.direction, &attrID, &attrRec ) )
      {
        break; // Should not happen, as numAttrs already calculated
      }

      pDiscoverExtRsp->aExtAttrInfo[i].attrID = attrRec.attr.attrId;
      pDiscoverExtRsp->aExtAttrInfo[i].attrDataType = attrRec.attr.dataType;
      pDiscoverExtRsp->aExtAttrInfo[i].attrAccessControl = attrRec.attr.accessControl & ACCESS_CONTROLEXT_MASK;
    }

    // Are there more attributes to be discovered?
    if ( zclFindNextAttrRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.fc.direction, &attrID, &attrRec ) )
    {
      discComplete = FALSE;
    }
  }

  pDiscoverExtRsp->discComplete = discComplete;
  pDiscoverExtRsp->numAttr = numAttrs;

  zcl_SendDiscoverAttrsExtRsp( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                               pInMsg->msg->clusterId, pDiscoverExtRsp, !(pInMsg->hdr.fc.direction),
                               true, pInMsg->hdr.transSeqNum );

  zcl_mem_free( pDiscoverExtRsp );

  return;
}

/*********************************************************************
 * @fn      zclProcessInDiscCmd
 *
 * @brief   Process the "Profile" Discover Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE if command processed. FALSE, otherwise.
 */
static uint8 zclProcessInDiscCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverCmdsCmd_t *pDiscoverCmd;
  zclDiscoverCmdsCmdRsp_t cmdRsp;
  ZStatus_t status;
  zclCommandRec_t cmdRec;
  uint8 cmdID;
  uint8 i;
  uint8 j;

  pDiscoverCmd = (zclDiscoverCmdsCmd_t *)pInMsg->attrCmd;

  // Find out the number of commands supported within the specified range
  for ( i = 0, cmdID = pDiscoverCmd->startCmdID; i < pDiscoverCmd->maxCmdID; i++, cmdID++ )
  {
    if ( !zclFindNextCmdRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.commandID, pInMsg->hdr.fc.direction, &cmdID, &cmdRec ) )
    {
      break;  // Command not supported
    }
  }

  // Allocate space for the response command
  cmdRsp.pCmdID = zcl_mem_alloc( i ); // size of number of commands returned

  if ( cmdRsp.pCmdID == NULL )
  {
    return FALSE; // EMBEDDED RETURN
  }

  if ( i != 0 )
  {
    for ( j = 0, cmdID = pDiscoverCmd->startCmdID; j < i; j++, cmdID++ )
    {
      if ( !zclFindNextCmdRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.commandID, pInMsg->hdr.fc.direction, &cmdID, &cmdRec ) )
      {
        break; // Attribute not supported
      }

      cmdRsp.pCmdID[j] = cmdRec.cmdID;
    }
  }

  // Are there more commands to be discovered?
  if ( zclFindNextCmdRec( pInMsg->msg->endPoint, pInMsg->msg->clusterId, pInMsg->hdr.commandID, pInMsg->hdr.fc.direction, &cmdID, &cmdRec ) )
  {
    cmdRsp.discComplete = FALSE;
  }
  else
  {
    cmdRsp.discComplete = TRUE;
  }

  // pass the command requested
  cmdRsp.cmdType = pInMsg->hdr.commandID;

  // store number of commands returned
  cmdRsp.numCmd = j;

  status = zcl_SendDiscoverCmdsRspCmd( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                      pInMsg->msg->clusterId, &cmdRsp, !(pInMsg->hdr.fc.direction),
                                      true, pInMsg->hdr.transSeqNum );

  zcl_mem_free( cmdRsp.pCmdID );

  if ( status == ZSuccess )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

#endif // ZCL_DISCOVER


/*********************************************************************
*********************************************************************/
