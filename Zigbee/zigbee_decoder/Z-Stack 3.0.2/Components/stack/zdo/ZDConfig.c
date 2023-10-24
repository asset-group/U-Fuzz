/**************************************************************************************************
  Filename:       ZDConfig.c
  Revised:        $Date: 2013-05-07 13:02:49 -0700 (Tue, 07 May 2013) $
  Revision:       $Revision: 34167 $


  Description:    This file contains the configuration attributes for the Zigbee Device Object.
                  These are references to Configuration items that MUST be defined in ZDApp.c.
                  The names mustn't change.


  Copyright 2004-2013 Texas Instruments Incorporated. All rights reserved.

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
#include "ZDObject.h"
#include "ZDConfig.h"

 /*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
NodeDescriptorFormat_t ZDO_Config_Node_Descriptor;
NodePowerDescriptorFormat_t ZDO_Config_Power_Descriptor;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * @fn      ZDConfig_InitDescriptors()
 *
 * @brief   Setup the ZDO descriptors
 *             Node, Power
 *
 * @param   none
 *
 * @return  none
 */
void ZDConfig_InitDescriptors( void )
{
  ZDConfig_UpdateNodeDescriptor();
  ZDConfig_UpdatePowerDescriptor();
}

/*********************************************************************
 * @fn      ZDConfig_UpdateNodeDescriptor()
 *
 * @brief   Update the ZDO Node Descriptor
 *
 * @param   none
 *
 * @return  none
 */
void ZDConfig_UpdateNodeDescriptor( void )
{
  // Build the Node Descriptor
  if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_COORDINATOR;
  else if ( ZSTACK_ROUTER_BUILD )
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_ROUTER;
  else if ( ZSTACK_END_DEVICE_BUILD )
    ZDO_Config_Node_Descriptor.LogicalType = NODETYPE_DEVICE;

  ZDO_Config_Node_Descriptor.ComplexDescAvail = FALSE;      // set elsewhere
  ZDO_Config_Node_Descriptor.UserDescAvail = FALSE;         // set elsewhere
  ZDO_Config_Node_Descriptor.Reserved = 0;                  // Reserved
  ZDO_Config_Node_Descriptor.APSFlags = 0;                  // NO APS flags
  ZDO_Config_Node_Descriptor.FrequencyBand = NODEFREQ_2400; // Frequency Band

  // MAC Capabilities
  if ( ZSTACK_ROUTER_BUILD )
  {
    ZDO_Config_Node_Descriptor.CapabilityFlags
              = (CAPINFO_DEVICETYPE_FFD | CAPINFO_POWER_AC |
                 CAPINFO_RCVR_ON_IDLE | CAPINFO_ALLOC_ADDR);

    if ( ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE )
      ZDO_Config_Node_Descriptor.CapabilityFlags |= CAPINFO_ALTPANCOORD;
  }
  else if ( ZSTACK_END_DEVICE_BUILD )
  {
    ZDO_Config_Node_Descriptor.CapabilityFlags = (CAPINFO_DEVICETYPE_RFD
  #if ( RFD_RCVC_ALWAYS_ON == TRUE)
            | CAPINFO_RCVR_ON_IDLE
  #endif
        | CAPINFO_ALLOC_ADDR);
  }

  // Manufacturer Code - *YOU FILL IN*
  ZDO_Config_Node_Descriptor.ManufacturerCode[0] = 0;
  ZDO_Config_Node_Descriptor.ManufacturerCode[1] = 0;

  // Maximum Buffer Size
  ZDO_Config_Node_Descriptor.MaxBufferSize = MAX_BUFFER_SIZE;

  // Maximum Incoming Transfer Size Field
  ZDO_Config_Node_Descriptor.MaxInTransferSize[0] = LO_UINT16( MAX_TRANSFER_SIZE );
  ZDO_Config_Node_Descriptor.MaxInTransferSize[1] = HI_UINT16( MAX_TRANSFER_SIZE );

  // Maximum Outgoing Transfer Size Field
  ZDO_Config_Node_Descriptor.MaxOutTransferSize[0] = LO_UINT16( MAX_TRANSFER_SIZE );
  ZDO_Config_Node_Descriptor.MaxOutTransferSize[1] = HI_UINT16( MAX_TRANSFER_SIZE );

#ifdef TP2_LEGACY_ZC
  //Do not set the current stack revision
#else
  // Set the current stack revision
  ZDO_Config_Node_Descriptor.ServerMask |= (STACK_COMPLIANCE_CURRENT_REV << STACK_COMPLIANCE_CURRENT_REV_POS);
#endif
  
  // Descriptor Capability Field - extended active endpoint list and
  // extended simple descriptor are not supported.
  ZDO_Config_Node_Descriptor.DescriptorCapability = 0;
}

/*********************************************************************
 * @fn      ZDConfig_UpdatePowerDescriptor()
 *
 * @brief   Update the ZDO Power Descriptor
 *
 * @param   none
 *
 * @return  none
 */
void ZDConfig_UpdatePowerDescriptor( void )
{
  // Build the Power Descriptor
  if ( ZSTACK_ROUTER_BUILD )
  {
    ZDO_Config_Power_Descriptor.PowerMode = NODECURPWR_RCVR_ALWAYS_ON;
    ZDO_Config_Power_Descriptor.AvailablePowerSources = NODEAVAILPWR_MAINS;
    ZDO_Config_Power_Descriptor.CurrentPowerSource = NODEAVAILPWR_MAINS;
    ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_100;
  }
  else if ( ZSTACK_END_DEVICE_BUILD )
  {
    if ( zgPollRate )
      ZDO_Config_Power_Descriptor.PowerMode = NODECURPWR_RCVR_AUTO;
    else
      ZDO_Config_Power_Descriptor.PowerMode = NODECURPWR_RCVR_STIM;

    ZDO_Config_Power_Descriptor.AvailablePowerSources = NODEAVAILPWR_RECHARGE;
    ZDO_Config_Power_Descriptor.CurrentPowerSource = NODEAVAILPWR_RECHARGE;
    ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_66;
  }
}

/*********************************************************************
*********************************************************************/


