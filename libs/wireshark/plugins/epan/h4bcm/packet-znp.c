/* packet-h4bcm.c
 * Routines for Bluetooth H4 Broadcom vendor specific additions
 * Copyright 2019, Jiska Classen / Secure Mobile Networking Lab
 *
 * $Id$
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "config.h"

// #include <epan/dissectors/packet-bthci_acl.h>
#include <epan/packet.h>
#include <epan/prefs.h>
#include <stdio.h>
#include <string.h>

// #include <epan/dissectors/packet-zbee.h>
#include <epan/dissectors/packet-ieee802154.h>
#include <epan/dissectors/packet-zbee-nwk.h>

/* type definitions for Broadcom diagnostics */
// start implement the znp protocol
#define AF_DATA_REQUEST_SREQ 0x2401
#define AF_DATA_REQUEST_SRSP 0x6401
#define AF_DATA_REQUEST_EXT_SREQ 0x2402
#define AF_DATA_CONFIRM_AREQ 0x4480
#define AF_INCOMING_MSG_AREQ 0x4481
#define ZDO_MGMT_PERMIT_JOIN_REQ_SREQ 0x2536
#define ZDO_MGMT_PERMIT_JOIN_REQ_SRSP 0x6536
#define ZDO_MGMT_PERMIT_JOIN_RSP_AREQ 0x45b6
#define STATUS_SUCCESS 0x00
#define STATUS_FAILURE 0x01

// start to implemnt the remaining command but only have command type but no details field
#define SYS_REST_REQ_ARED 0x4100
#define SYS_RESET_IND_AREQ 0x4180
#define SYS_PING_SREQ 0x2101
#define SYS_PING_SRSP 0x6101
#define SYS_VERSION_SREQ 0x2102
#define SYS_VERSION_SRSP 0x6102
#define SYS_OSAL_NV_READ_SREQ 0x2180
#define SYS_OSAL_NV_READ_SRSP 0x6180
#define SYS_OSAL_NV_WRITE_SREQ 0x2190
#define SYS_OSAL_NV_WRITE_SRSP 0x6190
// #define SYS_OSAL_NV_ITEM_INIT_SREQ 0x2170
// #define SYS_OSZB_WRITE_CONFIGURATION_SRSPAL_NV_ITEM_INIT_SRSP 0x6170
#define SYS_OSAL_NV_DELETE_SREQ 0x2112
#define SYS_OSAL_NV_DELETE_SRSP 0x6112
#define SYS_OSAL_NV_LENGTH_SREQ 0x2113
#define SYS_OSAL_NV_LENGTH_SRSP 0x6113
#define SYS_ADC_READ_SREQ 0x210d
#define SYS_ADC_READ_SRSP 0x610d
#define SYS_GPIO_SREQ 0x210e
#define SYS_GPIO_SRSP 0x610e
#define SYS_RANDOM_SREQ 0x210c
#define SYS_RANDOM_SRSP 0x610c
#define SYS_SET_TIME_SREQ 0x2110
#define SYS_SET_TIME_SRSP 0x6110
#define SYS_GET_TIME_SREQ 0x2111
#define SYS_GET_TIME_SRSP 0x6111
#define SYS_SET_TX_POWER_SREQ 0x2114
#define SYS_SET_TX_POWER_SRSP 0x6114
#define ZB_READ_CONFIGURATION_SREQ 0x2604
#define ZB_READ_CONFIGURATION_SRSP 0x6604
#define ZB_WRITE_CONFIGURATION_SREQ 0x2605
#define ZB_WRITE_CONFIGURATION_SRSP 0x6605
#define ZCD_NV_STARTUP_OPTION 0x0003
#define ZCD_NV_LOGICAL_TYPE 0x0087
#define ZCD_NV_ZDO_DIRECT_CB 0x008f
#define ZCD_NV_POLL_RATE 0x0024
#define ZCD_NV_QUEUED_POLL_RATE 0x0025
#define ZCD_NV_RESPONSE_POLL_RATE 0x0026
#define ZCD_NV_POLL_FAILURE_RETRIES 0x0029
#define ZCD_NV_INDIRECT_MSG_TIMEOUT 0x002b
#define ZCD_NV_APS_FRAME_RETRIES 0x0043
#define ZCD_NV_APS_ACK_WAIT_DURATION 0x0044
#define ZCD_NV_BINDING_TIME 0x0046
#define ZCD_NV_USERDESC 0x0081
#define ZCD_NV_PANID 0x0083
#define ZCD_NV_CHANLIST 0x0084
#define ZCD_NV_PRECFGKEY 0x0062
#define ZCD_NV_PRECFGKEYS_ENABLE 0x0063
#define ZCD_NV_SECURITY_MODE 0x0064
#define ZCD_NV_USE_DEFAULT_TCLK 0x006d
#define ZCD_NV_BCAST_RETRIES 0x002e
#define ZCD_NV_PASSIVE_ACK_TIMEOUT 0x002f
#define ZCD_NV_BCAST_DELIVERY_TIME 0x0030
#define ZCD_NV_ROUTE_EXPIRY_TIME 0x002c
#define ZNP_NV_RF_TEST_PARAMS 0x0f07
#define ZB_APP_REGISRTER_REQUEST_SREQ 0x260a
#define ZB_APP_REGISRTER_REQUEST_SRSP 0x660a
#define ZB_START_REQUEST_SREQ 0x2600
#define ZB_START_REQUEST_SRSP 0x6600
#define ZB_START_CONFIRM_AREQ 0x4680
#define ZB_PERMIT_JOINING_REQUEST_SREQ 0x2680
#define ZB_PERMIT_JOINING_REQUEST_SRSP 0x6680
#define ZB_BIND_DEVICE_SREQ 0x2601
#define ZB_BIND_DEVICE_SRSP 0x6601
#define ZB_BIND_DEVICE_CONFIRM_AREQ 0x4681
#define ZB_ALLOW_BIND_SREQ 0x2602
#define ZB_ALLOW_BIND_SRSP 0x6602
#define ZB_ALLOW_BIND_CONFIRM_AREQ 0x4682
#define ZB_SEND_DATA_REQUEST_SREQ 0x2603
#define ZB_SEND_DATA_REQUEST_SRSP 0x6603
#define ZB_SEND_DATA_CONFIRM_AREQ 0x4683
#define ZB_RECEIVE_DATA_INDICATION_AREQ 0x4687
#define ZB_GET_DEVICE_INFO_SREQ 0x2606
#define ZB_GET_DEVICE_INFO_SRSP 0x6606
#define ZB_FIND_DEVICE_REQUEST_SREQ 0x2607
#define ZB_FIND_DEVICE_REQUEST_SRSP 0x6607
#define ZB_FIND_DEVICE_CONFIRM_AREQ 0x4685
#define AF_REGISTER_SREQ 0x2400
#define AF_REGISTER_SRSP 0x6400
// #define AF_DATA_REQUEST_SREQ 0x2401
// #define AF_DATA_REQUEST_SRSP 0x6401
// #define AF_DATA_REQUEST_EXT_SREQ 0x2402
#define AF_DATA_REQUEST_EXT_SRSP 0x6402
#define AF_INTER_PAN_CTL_SREQ 0x2410
#define AF_INTER_PAN_CTL_SRSP 0x6410
#define AF_DATA_STORE_SREQ 0x2411
#define AF_DATA_STORE_SRSP 0x6411
// #define AF_DATA_CONFIRM_AREQ 0x4480
// #define AF_INCOMING_MSG_AREQ 0x4481
#define AF_INCOMING_MSG_EXT_AREQ 0x4482
#define AF_DATA_RETRIEVE_SREQ 0x2412
#define AF_DATA_RETRIEVE_SRSP 0x6412
#define AF_APSF_CONFIG_SET_SREQ 0x2413
#define AF_APSF_CONFIG_SET_SRSP 0x6413
#define ZDO_NWK_ADDR_REQ_SREQ 0x2500
#define ZDO_NWK_ADDR_REQ_SRSP 0x6500
#define ZDO_IEEE_ADDR_REQ_SREQ 0x2501
#define ZDO_IEEE_ADDR_REQ_SRSP 0x6501
#define ZDO_NODE_DESC_REQ_SREQ 0x2502
#define ZDO_NODE_DESC_REQ_SRSP 0x6502
#define ZDO_POWER_DESC_REQ_SREQ 0x2503
#define ZDO_POWER_DESC_REQ_SRSP 0x6503
#define ZDO_SIMPLE_DESC_REQ_SREQ 0x2504
#define ZDO_SIMPLE_DESC_REQ_SRSP 0x6504
#define ZDO_ACTIVE_EP_REQ_SREQ 0x2505
#define ZDO_ACTIVE_EP_REQ_SRSP 0x6505
#define ZDO_MATCH_DESC_REQ_SREQ 0x2506
#define ZDO_MATCH_DESC_REQ_SRSP 0x6506
#define ZDO_COMPLEX_DESC_REQ_SREQ 0x2507
#define ZDO_COMPLEX_DESC_REQ_SRSP 0x6507
#define ZDO_USER_DESC_REQ_SREQ 0x2508
#define ZDO_USER_DESC_REQ_SRSP 0x6508
#define ZDO_DEVICE_ANNCE_SREQ 0x250a
#define ZDO_DEVICE_ANNCE_SRSP 0x650a
#define ZDO_USER_DESC_SET_SREQ 0x250b
#define ZDO_USER_DESC_SET_SRSP 0x650b
#define ZDO_SERVER_DISC_REQ_SREQ 0x250c
#define ZDO_SERVER_DISC_REQ_SRSP 0x650c
#define ZDO_END_DEVICE_BIND_REQ_SREQ 0x2520
#define ZDO_END_DEVICE_BIND_REQ_SRSP 0x6520
#define ZDO_BIND_REQ_SREQ 0x2521
#define ZDO_BIND_REQ_SRSP 0x6521
#define ZDO_UNBIND_REQ_SREQ 0x2522
#define ZDO_UNBIND_REQ_SRSP 0x6522
#define ZDO_MGMT_NWK_DISC_REQ_SREQ 0x2530
#define ZDO_MGMT_NWK_DISC_REQ_SRSP 0x6530
#define ZDO_MGMT_LQI_REQ_SREQ 0x2531
#define ZDO_MGMT_LQI_REQ_SRSP 0x6531
#define ZDO_MGMT_RTG_REQ_SREQ 0x2532
#define ZDO_MGMT_RTG_REQ_SRSP 0x6532
#define ZDO_MGMT_BIND_REQ_SREQ 0x2533
#define ZDO_MGMT_BIND_REQ_SRSP 0x6533
#define ZDO_MGMT_LEAVE_REQ_SREQ 0x2534
#define ZDO_MGMT_LEAVE_REQ_SRSP 0x6534
#define ZDO_MGMT_DIRECT_JOIN_REQ_SREQ 0x2535
#define ZDO_MGMT_DIRECT_JOIN_REQ_SRSP 0x6535
// #define ZDO_MGMT_PERMIT_JOIN_REQ_SREQ 0x2536
// #define ZDO_MGMT_PERMIT_JOIN_REQ_SRSP 0x6536
#define ZDO_MGMT_NWK_UPDATE_REQ_SREQ 0x2537
#define ZDO_MGMT_NWK_UPDATE_REQ_SRSP 0x6537
#define ZDO_STARTUP_FROM_APP_SREQ 0x2540
#define ZDO_STARTUP_FROM_APP_SRSP 0x6540
#define ZDO_AUTO_FIND_DESTINATION_AREQ 0x4541
#define ZDO_SET_LINK_KEY_SREQ 0x2523
#define ZDO_SET_LINK_KEY_SRSP 0x6523
#define ZDO_REMOVE_LINK_KEY_SREQ 0x2524
#define ZDO_REMOVE_LINK_KEY_SRSP 0x6524
#define ZDO_GET_LINK_KEY_SREQ 0x2525
#define ZDO_GET_LINK_KEY_SRSP 0x6525
#define ZDO_NWK_DISCOVERY_REQ_SREQ 0x2526
#define ZDO_NWK_DISCOVERY_REQ_SRSP 0x6526
#define ZDO_JOIN_REQ_SREQ 0x2527
#define ZDO_JOIN_REQ_SRSP 0x6527
#define ZDO_NWK_ADDR_RSP_AREQ 0x4580
#define ZDO_IEEE_ADDR_RSP_AREQ 0x4581
#define ZDO_NODE_DESC_RSP_AREQ 0x4582
#define ZDO_POWER_DESC_RSP_AREQ 0x4583
#define ZDO_SIMPLE_DESC_RSP_AREQ 0x4584
#define ZDO_ACTIVE_EP_RSP_AREQ 0x4585
#define ZDO_MATCH_DESC_RSP_AREQ 0x4586
#define ZDO_COMPLEX_DESC_RSP_AREQ 0x4587
#define ZDO_USER_DESC_RSP_AREQ 0x4588
#define ZDO_USER_DESC_CONF_AREQ 0x4589
#define ZDO_SERVER_DISC_RSP_AREQ 0x458a
#define ZDO_END_DEVICE_BIND_RSP_AREQ 0x45a0
#define ZDO_BIND_RSP_AREQ 0x45a1
#define ZDO_UNBIND_RSP_AREQ 0x45a2
#define ZDO_MGMT_NWK_DISC_RSP_AREQ 0x45b0
#define ZDO_MGMT_LQI_RSP_AREQ 0x45b1
#define ZDO_MGMT_RTG_RSP_AREQ 0x45b2
#define ZDO_MGMT_BIND_RSP_AREQ 0x45b3
#define ZDO_MGMT_LEAVE_RSP_AREQ 0x45b4
#define ZDO_MGMT_DIRECT_JOIN_RSP_AREQ 0x45b5
// #define ZDO_MGMT_PERMIT_JOIN_RSP_AREQ 0x45b6
#define ZDO_STATE_CHANGE_IND_AREQ 0x45c0
#define ZDO_END_DEVICE_ANNCE_IND_AREQ 0x45c1
#define ZDO_MATCH_DESC_RSP_SENT_AREQ 0x45c2
#define ZDO_STATUS_ERROR_RSP_AREQ 0x45c3
#define ZDO_SRC_RTG_IND_AREQ 0x45c4
#define ZDO_LEAVE_IND_AREQ 0x45c9
#define ZDO_MSG_CB_REGISTER_SREQ 0x253e
#define ZDO_MSG_CB_REGISTER_SRSP 0x653e
#define ZDO_MSG_CB_REMOVE_SREQ 0x253f
#define ZDO_MSG_CB_REMOVE_SRSP 0x653f
#define ZDO_MSG_CB_INCOMING_AREQ 0x45ff
#define UTIL_DATA_REQ_SREQ 0x2711
#define UTIL_DATA_REQ_SRSP 0x6711
#define UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SREQ 0x2740
#define UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SRSP 0x6740
#define UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SREQ 0x2741
#define UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SRSP 0x6741
#define UTIL_APSME_LINK_KEY_DATA_GET_SREQ 0x2744
#define UTIL_APSME_LINK_KEY_DATA_GET_SRSP 0x6744
#define UTIL_APSME_LINK_KEY_NV_ID_GET_SREQ 0x2745
#define UTIL_APSME_LINK_KEY_NV_ID_GET_SRSP 0x6745
#define UTIL_APSME_REQUEST_KEY_CMD_SREQ 0x274b
#define UTIL_APSME_REQUEST_KEY_CMD_SRSP 0x674b
#define UTIL_ASSCO_COUNT_SREQ 0x2748
#define UTIL_ASSCO_COUNT_SRSP 0x6748
#define UTIL_ASSOC_FIND_DEVICE_SREQ 0x2749
#define UTIL_ASSOC_FIND_DEVICE_SRSP 0x6749
#define UTIL_ZCL_KEY_EST_INIT_EST_SREQ 0x2780
#define UTIL_ZCL_KEY_EST_INIT_EST_SRSP 0x6780
#define UTIL_ZCL_KEY_EST_SIGN_SREQ 0x2781
#define UTIL_ZCL_KEY_EST_SIGN_SRSP 0x6781
#define UTIL_ZCL_KEY_ESTABLISH_IND_AREQ 0x47e1
#define UTIL_TEST_LOOPBACK_SREQ 0x2710
#define UTIL_TEST_LOOPBACK_SRSP 0x6710

/* function prototypes */
void proto_reg_handoff_znp(void);

/* initialize the protocol and registered fields of znp */
static int proto_znp = -1;
static int len_of_data = -1;
static int hf_znp_fcs = -1;
// static int hf_znp_type = -1;
static int hf_znp_sof = -1;
static int hf_znp_length = -1;
static int hf_znp_command = -1;
// static int hf_znp_data = -1;
// static int hf_znp_checksum = -1;
static int hf_znp_data_request_sreq = -1;
static int hf_znp_data_request_sreq_dstaddr = -1;
static int hf_znp_data_request_sreq_dstendpoint = -1;
static int hf_znp_data_request_sreq_srcendpoint = -1;
static int hf_znp_data_request_sreq_clusterid = -1;
static int hf_znp_data_request_sreq_transid = -1;
static int hf_znp_data_request_sreq_options = -1;
static int hf_znp_data_request_sreq_radius = -1;
static int hf_znp_data_request_sreq_len = -1;
static int hf_znp_data_request_sreq_data = -1;
static int hf_znp_data_request_srsp = -1;
static int hf_znp_data_request_srsp_status = -1;
static int hf_znp_data_request_ext_sreq = -1;
static int hf_znp_data_request_ext_sreq_dstaddrMode = -1;
static int hf_znp_data_request_ext_sreq_dstaddr = -1;
static int hf_znp_data_request_ext_sreq_dstendpoint = -1;
static int hf_znp_data_request_ext_sreq_dstPanid = -1;
static int hf_znp_data_request_ext_sreq_srcendpoint = -1;
static int hf_znp_data_request_ext_sreq_clusterid = -1;
static int hf_znp_data_request_ext_sreq_transid = -1;
static int hf_znp_data_request_ext_sreq_options = -1;
static int hf_znp_data_request_ext_sreq_radius = -1;
static int hf_znp_data_request_ext_sreq_len = -1;
static int hf_znp_data_request_ext_sreq_data = -1;
static int hf_znp_data_confirm_areq = -1;
static int hf_znp_data_confirm_areq_status = -1;
static int hf_znp_data_confirm_areq_endpoint = -1;
static int hf_znp_data_confirm_areq_transid = -1;
// the length of the packet should be included in the header right?
// static int hf_znp_data_confirm_areq_length = -1;
static int hf_znp_incoming_msg_areq = -1;
static int hf_znp_incoming_msg_areq_groupid = -1;
static int hf_znp_incoming_msg_areq_clusterid = -1;
static int hf_znp_incoming_msg_areq_srcaddr = -1;
static int hf_znp_incoming_msg_areq_srcendpoint = -1;
static int hf_znp_incoming_msg_areq_dstendpoint = -1;
static int hf_znp_incoming_msg_areq_wasbroadcast = -1;
static int hf_znp_incoming_msg_areq_linkquality = -1;
static int hf_znp_incoming_msg_areq_securityuse = -1;
static int hf_znp_incoming_msg_areq_timestamp = -1;
static int hf_znp_incoming_msg_areq_transseqnumber = -1;
static int hf_znp_incoming_msg_areq_len = -1;
static int hf_znp_incoming_msg_areq_data = -1;
static int hf_znp_incoming_msg_areq_macsrcaddr = -1;
static int hf_znp_incoming_msg_areq_msgResultRadius = -1;
static int hf_znp_zdo_mgmt_permit_join_req_sreq = -1;
// do we have this field for the permit join request?
// static int hf_znp_zdo_mgmt_permit_join_req_sreq_addrmode = -1;
static int hf_znp_zdo_mgmt_permit_join_req_sreq_addrmode = -1;
static int hf_znp_zdo_mgmt_permit_join_req_sreq_dstaddr = -1;
static int hf_znp_zdo_mgmt_permit_join_req_sreq_duration = -1;
static int hf_znp_zdo_mgmt_permit_join_req_sreq_tcsignificance = -1;
static int hf_znp_zdo_mgmt_permit_join_req_srsp = -1;
static int hf_znp_zdo_mgmt_permit_join_req_srsp_status = -1;
static int hf_znp_zdo_mgmt_permit_join_rsp_areq = -1;
static int hf_znp_zdo_mgmt_permit_join_rsp_areq_status = -1;
static int hf_znp_zdo_mgmt_permit_join_rsp_areq_srcaddr = -1;
// static int hf_znp_zdo_mgmt_permit_join_rsp_areq_duration = -1;
/* initialize the subtree pointers for znp problem */
static gint ett_znp = -1;
// static gint ett_znp_cmd = -1;
// static gint ett_znp_data = -1;
// define a global variable for calling zigbee cluster library
static dissector_handle_t zcl_handle;

// znp type
static const value_string znp_cmd_types_values[] = {
    {AF_DATA_REQUEST_SREQ, "AF data request system request packet"},
    {AF_DATA_REQUEST_SRSP, "AF data request system response packet"},
    {AF_DATA_CONFIRM_AREQ, "AF data confirm asynchronous request packet"},
    {AF_DATA_REQUEST_EXT_SREQ, "AF data request extended system request packet"},
    {AF_INCOMING_MSG_AREQ, "AF incoming message asynchronous request packet"},
    {ZDO_MGMT_PERMIT_JOIN_REQ_SREQ, "ZDO management permit join request system request packet"},
    {ZDO_MGMT_PERMIT_JOIN_REQ_SRSP, "ZDO management permit join request system response packet"},
    {ZDO_MGMT_PERMIT_JOIN_RSP_AREQ, "ZDO management permit join response asynchronous request packet"},
    {SYS_REST_REQ_ARED, "SYS reset request asynchronous request packet"},
    {SYS_RESET_IND_AREQ, "SYS reset indication asynchronous request packet"},
    {SYS_PING_SREQ, "SYS ping system request packet"},
    {SYS_PING_SRSP, "SYS ping system response packet"},
    {SYS_VERSION_SREQ, "SYS version system request packet"},
    {SYS_VERSION_SRSP, "SYS version system response packet"},
    {SYS_OSAL_NV_READ_SREQ, "SYS OSAL NV read system request packet"},
    {SYS_OSAL_NV_READ_SRSP, "SYS OSAL NV read system response packet"},
    {SYS_OSAL_NV_WRITE_SREQ, "SYS OSAL NV write system request packet"},
    {SYS_OSAL_NV_WRITE_SRSP, "SYS OSAL NV write system response packet"},
    {SYS_OSAL_NV_DELETE_SREQ, "SYS OSAL NV delete system request packet"},
    {SYS_OSAL_NV_DELETE_SRSP, "SYS OSAL NV delete system response packet"},
    {SYS_OSAL_NV_LENGTH_SREQ, "SYS OSAL NV length system request packet"},
    {SYS_OSAL_NV_LENGTH_SRSP, "SYS OSAL NV length system response packet"},
    {SYS_ADC_READ_SREQ, "SYS ADC read system request packet"},
    {SYS_ADC_READ_SRSP, "SYS ADC read system response packet"},
    {SYS_GPIO_SREQ, "SYS GPIO system request packet"},
    {SYS_GPIO_SRSP, "SYS GPIO system response packet"},
    {SYS_RANDOM_SREQ, "SYS random system request packet"},
    {SYS_RANDOM_SRSP, "SYS random system response packet"},
    {SYS_SET_TIME_SREQ, "SYS set time system request packet"},
    {SYS_SET_TIME_SRSP, "SYS set time system response packet"},
    {SYS_GET_TIME_SREQ, "SYS get time system request packet"},
    {SYS_GET_TIME_SRSP, "SYS get time system response packet"},
    {SYS_SET_TX_POWER_SREQ, "SYS set TX power system request packet"},
    {SYS_SET_TX_POWER_SRSP, "SYS set TX power system response packet"},
    {ZB_READ_CONFIGURATION_SREQ, "ZB read configuration system request packet"},
    {ZB_READ_CONFIGURATION_SRSP, "ZB read configuration system response packet"},
    {ZB_WRITE_CONFIGURATION_SREQ, "ZB write configuration system request packet"},
    {ZB_WRITE_CONFIGURATION_SRSP, "ZB write configuration system response packet"},
    {ZCD_NV_STARTUP_OPTION, "ZCD NV startup option"},
    {ZCD_NV_LOGICAL_TYPE, "ZCD NV logical type"},
    {ZCD_NV_ZDO_DIRECT_CB, "ZCD NV ZDO direct CB"},
    {ZCD_NV_POLL_RATE, "ZCD NV poll rate"},
    {ZCD_NV_QUEUED_POLL_RATE, "ZCD NV queued poll rate"},
    {ZCD_NV_RESPONSE_POLL_RATE, "ZCD NV response poll rate"},
    {ZCD_NV_POLL_FAILURE_RETRIES, "ZCD NV poll failure retries"},
    {ZCD_NV_INDIRECT_MSG_TIMEOUT, "ZCD NV indirect message timeout"},
    {ZCD_NV_APS_FRAME_RETRIES, "ZCD NV APS frame retries"},
    {ZCD_NV_APS_ACK_WAIT_DURATION, "ZCD NV APS ACK wait duration"},
    {ZCD_NV_BINDING_TIME, "ZCD NV binding time"},
    {ZCD_NV_USERDESC, "ZCD NV user description"},
    {ZCD_NV_PANID, "ZCD NV PAN ID"},
    {ZCD_NV_CHANLIST, "ZCD NV channel list"},
    {ZCD_NV_PRECFGKEY, "ZCD NV preconfigured key"},
    {ZCD_NV_PRECFGKEYS_ENABLE, "ZCD NV preconfigured keys enable"},
    {ZCD_NV_SECURITY_MODE, "ZCD NV security mode"},
    {ZCD_NV_USE_DEFAULT_TCLK, "ZCD NV use default TCLK"},
    {ZCD_NV_BCAST_RETRIES, "ZCD NV broadcast retries"},
    {ZCD_NV_PASSIVE_ACK_TIMEOUT, "ZCD NV passive ACK timeout"},
    {ZCD_NV_BCAST_DELIVERY_TIME, "ZCD NV broadcast delivery time"},
    {ZCD_NV_ROUTE_EXPIRY_TIME, "ZCD NV route expiry time"},
    {ZNP_NV_RF_TEST_PARAMS, "ZNP NV RF test parameters"},
    {ZB_APP_REGISRTER_REQUEST_SREQ, "ZB application register request system request packet"},
    {ZB_APP_REGISRTER_REQUEST_SRSP, "ZB application register request system response packet"},
    {ZB_START_REQUEST_SREQ, "ZB start request system request packet"},
    {ZB_START_REQUEST_SRSP, "ZB start request system response packet"},
    {ZB_START_CONFIRM_AREQ, "ZB start confirm application request packet"},
    {ZB_PERMIT_JOINING_REQUEST_SREQ, "ZB permit joining request system request packet"},
    {ZB_PERMIT_JOINING_REQUEST_SRSP, "ZB permit joining request system response packet"},
    {ZB_BIND_DEVICE_SREQ, "ZB bind device system request packet"},
    {ZB_BIND_DEVICE_SRSP, "ZB bind device system response packet"},
    {ZB_BIND_DEVICE_CONFIRM_AREQ, "ZB bind device confirm application request packet"},
    {ZB_ALLOW_BIND_SREQ, "ZB allow bind system request packet"},
    {ZB_ALLOW_BIND_SRSP, "ZB allow bind system response packet"},
    {ZB_ALLOW_BIND_CONFIRM_AREQ, "ZB allow bind confirm application request packet"},
    {ZB_SEND_DATA_REQUEST_SREQ, "ZB send data request system request packet"},
    {ZB_SEND_DATA_REQUEST_SRSP, "ZB send data request system response packet"},
    {ZB_SEND_DATA_CONFIRM_AREQ, "ZB send data confirm application request packet"},
    {ZB_RECEIVE_DATA_INDICATION_AREQ, "ZB receive data indication application request packet"},
    {ZB_GET_DEVICE_INFO_SREQ, "ZB get device information system request packet"},
    {ZB_GET_DEVICE_INFO_SRSP, "ZB get device information system response packet"},
    {ZB_FIND_DEVICE_REQUEST_SREQ, "ZB find device request system request packet"},
    {ZB_FIND_DEVICE_REQUEST_SRSP, "ZB find device request system response packet"},
    {ZB_FIND_DEVICE_CONFIRM_AREQ, "ZB find device confirm application request packet"},
    {AF_REGISTER_SREQ, "AF register system request packet"},
    {AF_REGISTER_SRSP, "AF register system response packet"},
    // {AF_DATA_REQUEST_SRSP, "AF data request system response packet"},
    {AF_DATA_REQUEST_EXT_SRSP, "AF data request extended system response packet"},
    {AF_INTER_PAN_CTL_SREQ, "AF inter PAN control system request packet"},
    {AF_INTER_PAN_CTL_SRSP, "AF inter PAN control system response packet"},
    {AF_DATA_STORE_SREQ, "AF data store system request packet"},
    {AF_DATA_STORE_SRSP, "AF data store system response packet"},
    {AF_INCOMING_MSG_EXT_AREQ, "AF incoming message application request packet"},
    {AF_DATA_RETRIEVE_SREQ, "AF data retrieve system request packet"},
    {AF_DATA_RETRIEVE_SRSP, "AF data retrieve system response packet"},
    {AF_APSF_CONFIG_SET_SREQ, "AF APSF configuration set system request packet"},
    {AF_APSF_CONFIG_SET_SRSP, "AF APSF configuration set system response packet"},
    {ZDO_NWK_ADDR_REQ_SREQ, "ZDO network address request system request packet"},
    {ZDO_NWK_ADDR_REQ_SRSP, "ZDO network address request system response packet"},
    {ZDO_IEEE_ADDR_REQ_SREQ, "ZDO IEEE address request system request packet"},
    {ZDO_IEEE_ADDR_REQ_SRSP, "ZDO IEEE address request system response packet"},
    {ZDO_NODE_DESC_REQ_SREQ, "ZDO node description request system request packet"},
    {ZDO_NODE_DESC_REQ_SRSP, "ZDO node description request system response packet"},
    {ZDO_POWER_DESC_REQ_SREQ, "ZDO power description request system request packet"},
    {ZDO_POWER_DESC_REQ_SRSP, "ZDO power description request system response packet"},
    {ZDO_SIMPLE_DESC_REQ_SREQ, "ZDO simple description request system request packet"},
    {ZDO_SIMPLE_DESC_REQ_SRSP, "ZDO simple description request system response packet"},
    {ZDO_ACTIVE_EP_REQ_SREQ, "ZDO active endpoint request system request packet"},
    {ZDO_ACTIVE_EP_REQ_SRSP, "ZDO active endpoint request system response packet"},
    {ZDO_MATCH_DESC_REQ_SREQ, "ZDO match description request system request packet"},
    {ZDO_MATCH_DESC_REQ_SRSP, "ZDO match description request system response packet"},
    {ZDO_COMPLEX_DESC_REQ_SREQ, "ZDO complex description request system request packet"},
    {ZDO_COMPLEX_DESC_REQ_SRSP, "ZDO complex description request system response packet"},
    {ZDO_USER_DESC_REQ_SREQ, "ZDO user description request system request packet"},
    {ZDO_USER_DESC_REQ_SRSP, "ZDO user description request system response packet"},
    {ZDO_DEVICE_ANNCE_SREQ, "ZDO device announce system request packet"},
    {ZDO_DEVICE_ANNCE_SRSP, "ZDO device announce system response packet"},
    {ZDO_USER_DESC_SET_SREQ, "ZDO user description set system request packet"},
    {ZDO_USER_DESC_SET_SRSP, "ZDO user description set system response packet"},
    {ZDO_SERVER_DISC_REQ_SREQ, "ZDO server discovery request system request packet"},
    {ZDO_SERVER_DISC_REQ_SRSP, "ZDO server discovery request system response packet"},
    {ZDO_END_DEVICE_BIND_REQ_SREQ, "ZDO end device bind request system request packet"},
    {ZDO_END_DEVICE_BIND_REQ_SRSP, "ZDO end device bind request system response packet"},
    {ZDO_BIND_REQ_SREQ, "ZDO bind request system request packet"},
    {ZDO_BIND_REQ_SRSP, "ZDO bind request system response packet"},
    {ZDO_UNBIND_REQ_SREQ, "ZDO unbind request system request packet"},
    {ZDO_UNBIND_REQ_SRSP, "ZDO unbind request system response packet"},
    {ZDO_MGMT_NWK_DISC_REQ_SREQ, "ZDO management network discovery request system request packet"},
    {ZDO_MGMT_NWK_DISC_REQ_SRSP, "ZDO management network discovery request system response packet"},
    {ZDO_MGMT_LQI_REQ_SREQ, "ZDO management LQI request system request packet"},
    {ZDO_MGMT_LQI_REQ_SRSP, "ZDO management LQI request system response packet"},
    {ZDO_MGMT_RTG_REQ_SREQ, "ZDO management routing request system request packet"},
    {ZDO_MGMT_RTG_REQ_SRSP, "ZDO management routing request system response packet"},
    {ZDO_MGMT_BIND_REQ_SREQ, "ZDO management bind request system request packet"},
    {ZDO_MGMT_BIND_REQ_SRSP, "ZDO management bind request system response packet"},
    {ZDO_MGMT_LEAVE_REQ_SREQ, "ZDO management leave request system request packet"},
    {ZDO_MGMT_LEAVE_REQ_SRSP, "ZDO management leave request system response packet"},
    {ZDO_MGMT_DIRECT_JOIN_REQ_SREQ, "ZDO management direct join request system request packet"},
    {ZDO_MGMT_DIRECT_JOIN_REQ_SRSP, "ZDO management direct join request system response packet"},
    {ZDO_MGMT_NWK_UPDATE_REQ_SREQ, "ZDO management network update request system request packet"},
    {ZDO_MGMT_NWK_UPDATE_REQ_SRSP, "ZDO management network update request system response packet"},
    {ZDO_STARTUP_FROM_APP_SREQ, "ZDO startup from application system request packet"},
    {ZDO_STARTUP_FROM_APP_SRSP, "ZDO startup from application system response packet"},
    {ZDO_AUTO_FIND_DESTINATION_AREQ, "ZDO auto find destination application request packet"},
    {ZDO_SET_LINK_KEY_SREQ, "ZDO set link key system request packet"},
    {ZDO_SET_LINK_KEY_SRSP, "ZDO set link key system response packet"},
    {ZDO_REMOVE_LINK_KEY_SREQ, "ZDO remove link key system request packet"},
    {ZDO_REMOVE_LINK_KEY_SRSP, "ZDO remove link key system response packet"},
    {ZDO_GET_LINK_KEY_SREQ, "ZDO get link key system request packet"},
    {ZDO_GET_LINK_KEY_SRSP, "ZDO get link key system response packet"},
    {ZDO_NWK_DISCOVERY_REQ_SREQ, "ZDO network discovery request system request packet"},
    {ZDO_NWK_DISCOVERY_REQ_SRSP, "ZDO network discovery request system response packet"},
    {ZDO_JOIN_REQ_SREQ, "ZDO join request system request packet"},
    {ZDO_JOIN_REQ_SRSP, "ZDO join request system response packet"},
    {ZDO_NWK_ADDR_RSP_AREQ, "ZDO network address response application request packet"},
    {ZDO_IEEE_ADDR_RSP_AREQ, "ZDO IEEE address response application request packet"},
    {ZDO_NODE_DESC_RSP_AREQ, "ZDO node description response application request packet"},
    {ZDO_POWER_DESC_RSP_AREQ, "ZDO power description response application request packet"},
    {ZDO_SIMPLE_DESC_RSP_AREQ, "ZDO simple description response application request packet"},
    {ZDO_ACTIVE_EP_RSP_AREQ, "ZDO active endpoint response application request packet"},
    {ZDO_MATCH_DESC_RSP_AREQ, "ZDO match description response application request packet"},
    {ZDO_COMPLEX_DESC_RSP_AREQ, "ZDO complex description response application request packet"},
    {ZDO_USER_DESC_RSP_AREQ, "ZDO user description response application request packet"},
    {ZDO_USER_DESC_CONF_AREQ, "ZDO user description confirm application request packet"},
    {ZDO_SERVER_DISC_RSP_AREQ, "ZDO server discovery response application request packet"},
    {ZDO_END_DEVICE_BIND_RSP_AREQ, "ZDO end device bind response application request packet"},
    {ZDO_BIND_RSP_AREQ, "ZDO bind response application request packet"},
    {ZDO_UNBIND_RSP_AREQ, "ZDO unbind response application request packet"},
    {ZDO_MGMT_NWK_DISC_RSP_AREQ, "ZDO management network discovery response application request packet"},
    {ZDO_MGMT_LQI_RSP_AREQ, "ZDO management LQI response application request packet"},
    {ZDO_MGMT_RTG_RSP_AREQ, "ZDO management routing response application request packet"},
    {ZDO_MGMT_BIND_RSP_AREQ, "ZDO management bind response application request packet"},
    {ZDO_MGMT_LEAVE_RSP_AREQ, "ZDO management leave response application request packet"},
    {ZDO_MGMT_DIRECT_JOIN_RSP_AREQ, "ZDO management direct join response application request packet"},
    {ZDO_STATE_CHANGE_IND_AREQ, "ZDO state change indication application request packet"},
    {ZDO_END_DEVICE_ANNCE_IND_AREQ, "ZDO end device announcement indication application request packet"},
    {ZDO_MATCH_DESC_RSP_SENT_AREQ, "ZDO match description response sent application request packet"},
    {ZDO_STATUS_ERROR_RSP_AREQ, "ZDO status error response application request packet"},
    {ZDO_SRC_RTG_IND_AREQ, "ZDO source routing indication application request packet"},
    {ZDO_LEAVE_IND_AREQ, "ZDO leave indication application request packet"},
    {ZDO_MSG_CB_REGISTER_SREQ, "ZDO message callback register system request packet"},
    {ZDO_MSG_CB_REGISTER_SRSP, "ZDO message callback register system response packet"},
    {ZDO_MSG_CB_REMOVE_SREQ, "ZDO message callback remove system request packet"},
    {ZDO_MSG_CB_REMOVE_SRSP, "ZDO message callback remove system response packet"},
    {ZDO_MSG_CB_INCOMING_AREQ, "ZDO message callback incoming application request packet"},
    {UTIL_DATA_REQ_SREQ, "UTIL data request system request packet"},
    {UTIL_DATA_REQ_SRSP, "UTIL data request system response packet"},
    {UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SREQ, "UTIL address manager extended address lookup system request packet"},
    {UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SRSP, "UTIL address manager extended address lookup system response packet"},
    {UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SREQ, "UTIL address manager network address lookup system request packet"},
    {UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SRSP, "UTIL address manager network address lookup system response packet"},
    {UTIL_APSME_LINK_KEY_DATA_GET_SREQ, "UTIL APSME link key data get system request packet"},
    {UTIL_APSME_LINK_KEY_DATA_GET_SRSP, "UTIL APSME link key data get system response packet"},
    {UTIL_APSME_LINK_KEY_NV_ID_GET_SREQ, "UTIL APSME link key NV ID get system request packet"},
    {UTIL_APSME_LINK_KEY_NV_ID_GET_SRSP, "UTIL APSME link key NV ID get system response packet"},
    {UTIL_APSME_REQUEST_KEY_CMD_SREQ, "UTIL APSME request key system request packet"},
    {UTIL_APSME_REQUEST_KEY_CMD_SRSP, "UTIL APSME request key system response packet"},
    {UTIL_ASSCO_COUNT_SREQ, "UTIL association count system request packet"},
    {UTIL_ASSCO_COUNT_SRSP, "UTIL association count system response packet"},
    {UTIL_ASSOC_FIND_DEVICE_SREQ, "UTIL association find device system request packet"},
    {UTIL_ASSOC_FIND_DEVICE_SRSP, "UTIL association find device system response packet"},
    {UTIL_ZCL_KEY_EST_INIT_EST_SREQ, "UTIL ZCL key establishment initialization system request packet"},
    {UTIL_ZCL_KEY_EST_INIT_EST_SRSP, "UTIL ZCL key establishment initialization system response packet"},
    {UTIL_ZCL_KEY_EST_SIGN_SREQ, "UTIL ZCL key establishment sign system request packet"},
    {UTIL_ZCL_KEY_EST_SIGN_SRSP, "UTIL ZCL key establishment sign system response packet"},
    {UTIL_ZCL_KEY_ESTABLISH_IND_AREQ, "UTIL ZCL key establishment indication application request packet"},
    {UTIL_TEST_LOOPBACK_SREQ, "UTIL test loopback system request packet"},
    {UTIL_TEST_LOOPBACK_SRSP, "UTIL test loopback system response packet"},
    {0, NULL}};

// znp status
static const value_string znp_status_values[] = {
    {STATUS_SUCCESS, "Success"},
    {STATUS_FAILURE, "Failure"},
    {0, NULL}};
/* dissect a znp packet*/
static int
dissect_znp(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    proto_item *znp_item;
    proto_tree *znp_tree;
    int offset = 0;
    int znp_cmd_types;
    // declare zcl_tvb
    tvbuff_t *zcl_tvb;
    /* sanity check: length */
    if (tvb_reported_length(tvb) < 1)
        /* bad length: look for a different dissector */
        return 0;
    znp_cmd_types = tvb_get_guint16(tvb, 2, ENC_BIG_ENDIAN);
    // printf("znp_cmd_types: %d", znp_cmd_types);
    // offset += 2;
    if (try_val_to_str(znp_cmd_types, znp_cmd_types_values) == NULL)
        return 0;
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "Zigbee Netwrok Processor");
    col_clear(pinfo->cinfo, COL_INFO);
    col_add_str(pinfo->cinfo, COL_INFO, val_to_str(znp_cmd_types, znp_cmd_types_values, "Unknow Command (%d)"));
    // col_append_str(pinfo->cinfo, COL_INFO, "/");

    col_set_fence(pinfo->cinfo, COL_INFO);

    znp_item = proto_tree_add_item(tree, proto_znp, tvb, offset, -1, ENC_NA);
    znp_tree = proto_item_add_subtree(znp_item, ett_znp);

    uint8_t len = tvb_get_guint8(tvb, offset);

    //
    /* create display subtree for the protocol */
    offset = 0;
    // This is the fe item, which is 1 byte long
    proto_tree_add_item(znp_tree, hf_znp_sof, tvb, offset, 1, ENC_LITTLE_ENDIAN);
    offset += 1;
    proto_tree_add_item(znp_tree, hf_znp_length, tvb, offset, 1, ENC_LITTLE_ENDIAN);
    offset += 1;
    // This is the command type item, which is 2 bytes long
    proto_tree_add_item(znp_tree, hf_znp_command, tvb, offset, 2, ENC_BIG_ENDIAN);
    offset += 2;
    // implement the switch here, we have the command type already, then implement the switch, which all the customized packtets is implented here
    // can we specfy the customised offest here? since every packet has design
    switch (znp_cmd_types) {
    case AF_DATA_REQUEST_SREQ:
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_dstaddr, tvb, offset, 2, ENC_NA);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_dstendpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_srcendpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        uint16_t cluster_id;
        proto_tree_add_item_ret_uint(znp_tree, hf_znp_data_request_sreq_clusterid, tvb, offset, 2, ENC_NA, &cluster_id);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_transid, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_options, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_radius, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_len, tvb, offset, 1, ENC_NA);
        len_of_data = tvb_get_guint8(tvb, offset);
        offset += 1;
        // use zcl dissector to dissect the data
        zcl_tvb = tvb_new_subset_length(tvb, offset, len_of_data);
        // call_dissector(zcl_handle, zcl_tvb, pinfo, znp_tree);
        if (zcl_handle == NULL) {
            // printf("zcl_handle is null\n");
            proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_data, tvb, offset, len_of_data, ENC_NA);
        }
        else {
            // printf("zcl_handle is not null\n");
            zbee_nwk_packet znp_data;
            znp_data.cluster_id = cluster_id;
            // use the zcl_handle to dissect the data
            call_dissector_with_data(zcl_handle, zcl_tvb, pinfo, znp_tree, &znp_data);
            // call_data_dissector(zcl_tvb, pinfo, znp_tree);
            // hf_znp_data_request_sreq_data = call_dissector(zcl_handle, zcl_tvb, pinfo, znp_tree);

            // proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_data, tvb, offset, len_of_data, ENC_NA);
        }
        // hf_znp_data_request_sreq_data = call_dissector(zcl_handle, zcl_tvb, pinfo, znp_tree);
        // proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_data, tvb, offset, len_of_data, ENC_NA);
        offset += len_of_data;
        break;

    case AF_DATA_REQUEST_SRSP:
        proto_tree_add_item(znp_tree, hf_znp_data_request_srsp_status, tvb, offset, 1, ENC_NA);
        offset += 1;
        // can implement more detials about the status here, 0 is success, 1 is failure

        break;
    // This command is responsable for two packets, deal with it later.
    case AF_DATA_REQUEST_EXT_SREQ:
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_dstaddrMode, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_dstaddr, tvb, offset, 8, ENC_NA);
        offset += 8;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_dstendpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_dstPanid, tvb, offset, 2, ENC_NA);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_srcendpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        uint16_t cluster_id_2;
        proto_tree_add_item_ret_uint(znp_tree, hf_znp_data_request_ext_sreq_clusterid, tvb, offset, 2, ENC_NA, &cluster_id_2);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_transid, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_options, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_radius, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_len, tvb, offset, 2, ENC_NA);
        len_of_data = tvb_get_guint8(tvb, offset);
        offset += 2;
        zcl_tvb = tvb_new_subset_length(tvb, offset, len_of_data);

        if (zcl_handle == NULL) {
            // printf("zcl_handle is null");
            proto_tree_add_item(znp_tree, hf_znp_data_request_ext_sreq_data, tvb, offset, len_of_data, ENC_NA);
        }
        else {
            // printf("zcl_handle is not null");
            // need to create the zbee_nwk_packet structure since it is needed by zigbee cluster library dissector to dissect the data
            zbee_nwk_packet znp_data;
            //  the cluster_id is also needed by the zcl disector
            znp_data.cluster_id = cluster_id_2;
            // use the zcl_handle to dissect the data
            call_dissector_with_data(zcl_handle, zcl_tvb, pinfo, znp_tree, &znp_data);
            // call_data_dissector(zcl_tvb, pinfo, znp_tree);
            // hf_znp_data_request_sreq_data = call_dissector(zcl_handle, zcl_tvb, pinfo, znp_tree);

            // proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_data, tvb, offset, len_of_data, ENC_NA);
        }
        offset += len_of_data;
        break;
    case AF_INCOMING_MSG_AREQ:
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_groupid, tvb, offset, 2, ENC_NA);
        offset += 2;
        uint16_t cluster_id_3;
        // this function will return the value of the cluster_id for later use for decoding data
        proto_tree_add_item_ret_uint(znp_tree, hf_znp_incoming_msg_areq_clusterid, tvb, offset, 2, ENC_NA, &cluster_id_3);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_srcaddr, tvb, offset, 2, ENC_NA);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_srcendpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_dstendpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_wasbroadcast, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_linkquality, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_securityuse, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_timestamp, tvb, offset, 4, ENC_NA);
        offset += 4;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_transseqnumber, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_len, tvb, offset, 1, ENC_NA);
        len_of_data = tvb_get_guint8(tvb, offset);
        offset += 1;
        zcl_tvb = tvb_new_subset_length(tvb, offset, len_of_data);
        // call_dissector(zcl_handle, zcl_tvb, pinfo, znp_tree);
        if (zcl_handle == NULL) {
            // printf("zcl_handle is null");
            proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_data, tvb, offset, len_of_data, ENC_NA);
        }
        else {
            // printf("zcl_handle is not null");
            // need to create the zbee_nwk_packet structure since it is needed by zigbee cluster library dissector to dissect the data
            zbee_nwk_packet znp_data;
            //  the cluster_id is also needed by the zcl disector
            znp_data.cluster_id = cluster_id_3;
            // use the zcl_handle to dissect the data
            call_dissector_with_data(zcl_handle, zcl_tvb, pinfo, znp_tree, &znp_data);
            // call_data_dissector(zcl_tvb, pinfo, znp_tree);
            // hf_znp_data_request_sreq_data = call_dissector(zcl_handle, zcl_tvb, pinfo, znp_tree);

            // proto_tree_add_item(znp_tree, hf_znp_data_request_sreq_data, tvb, offset, len_of_data, ENC_NA);
        }
        offset += len_of_data;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_macsrcaddr, tvb, offset, 2, ENC_NA);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_incoming_msg_areq_msgResultRadius, tvb, offset, 1, ENC_NA);
        offset += 1;
        break;
    case AF_DATA_CONFIRM_AREQ:
        proto_tree_add_item(znp_tree, hf_znp_data_confirm_areq_status, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_confirm_areq_endpoint, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_data_confirm_areq_transid, tvb, offset, 1, ENC_NA);
        offset += 1;
        break;
    case ZDO_MGMT_PERMIT_JOIN_REQ_SREQ:
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_req_sreq_addrmode, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_req_sreq_dstaddr, tvb, offset, 2, ENC_NA);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_req_sreq_duration, tvb, offset, 1, ENC_NA);
        offset += 1;
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_req_sreq_tcsignificance, tvb, offset, 1, ENC_NA);
        offset += 1;
        break;
    case ZDO_MGMT_PERMIT_JOIN_REQ_SRSP:
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_req_srsp_status, tvb, offset, 1, ENC_NA);
        offset += 1;
        break;
    case ZDO_MGMT_PERMIT_JOIN_RSP_AREQ:
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_rsp_areq_srcaddr, tvb, offset, 2, ENC_NA);
        offset += 2;
        proto_tree_add_item(znp_tree, hf_znp_zdo_mgmt_permit_join_rsp_areq_status, tvb, offset, 1, ENC_NA);
        offset += 1;
        break;
    case SYS_REST_REQ_ARED:
        break;
    case SYS_RESET_IND_AREQ:
        break;
    case SYS_PING_SREQ:
        break;
    case SYS_PING_SRSP:
        break;
    case SYS_VERSION_SREQ:
        break;
    case SYS_VERSION_SRSP:
        break;
    case SYS_OSAL_NV_READ_SREQ:
        break;
    case SYS_OSAL_NV_READ_SRSP:
        break;
    case SYS_OSAL_NV_WRITE_SREQ:
        break;
    case SYS_OSAL_NV_WRITE_SRSP:
        break;
    case SYS_OSAL_NV_DELETE_SREQ:
        break;
    case SYS_OSAL_NV_DELETE_SRSP:
        break;
    case SYS_OSAL_NV_LENGTH_SREQ:
        break;
    case SYS_OSAL_NV_LENGTH_SRSP:
        break;
    case SYS_ADC_READ_SREQ:
        break;
    case SYS_ADC_READ_SRSP:
        break;
    case SYS_GPIO_SREQ:
        break;
    case SYS_GPIO_SRSP:
        break;
    case SYS_RANDOM_SREQ:
        break;
    case SYS_RANDOM_SRSP:
        break;
    case SYS_SET_TIME_SREQ:
        break;
    case SYS_SET_TIME_SRSP:
        break;
    case SYS_GET_TIME_SREQ:
        break;
    case SYS_GET_TIME_SRSP:
        break;
    case SYS_SET_TX_POWER_SREQ:
        break;
    case SYS_SET_TX_POWER_SRSP:
        break;
    case ZB_READ_CONFIGURATION_SREQ:
        break;
    case ZB_READ_CONFIGURATION_SRSP:
        break;
    case ZB_WRITE_CONFIGURATION_SREQ:
        break;
    case ZB_WRITE_CONFIGURATION_SRSP:
        break;
    case ZCD_NV_STARTUP_OPTION:
        break;
    case ZCD_NV_LOGICAL_TYPE:
        break;
    case ZCD_NV_ZDO_DIRECT_CB:
        break;
    case ZCD_NV_POLL_RATE:
        break;
    case ZCD_NV_QUEUED_POLL_RATE:
        break;
    case ZCD_NV_RESPONSE_POLL_RATE:
        break;
    case ZCD_NV_POLL_FAILURE_RETRIES:
        break;
    case ZCD_NV_INDIRECT_MSG_TIMEOUT:
        break;
    case ZCD_NV_APS_FRAME_RETRIES:
        break;
    case ZCD_NV_APS_ACK_WAIT_DURATION:
        break;
    case ZCD_NV_BINDING_TIME:
        break;
    case ZCD_NV_USERDESC:
        break;
    case ZCD_NV_PANID:
        break;
    case ZCD_NV_CHANLIST:
        break;
    case ZCD_NV_PRECFGKEY:
        break;
    case ZCD_NV_PRECFGKEYS_ENABLE:
        break;
    case ZCD_NV_SECURITY_MODE:
        break;
    case ZCD_NV_USE_DEFAULT_TCLK:
        break;
    case ZCD_NV_BCAST_RETRIES:
        break;
    case ZCD_NV_PASSIVE_ACK_TIMEOUT:
        break;
    case ZCD_NV_BCAST_DELIVERY_TIME:
        break;
    case ZCD_NV_ROUTE_EXPIRY_TIME:
        break;
    case ZNP_NV_RF_TEST_PARAMS:
        break;
    case ZB_APP_REGISRTER_REQUEST_SREQ:
        break;
    case ZB_APP_REGISRTER_REQUEST_SRSP:
        break;
    case ZB_START_REQUEST_SREQ:
        break;
    case ZB_START_REQUEST_SRSP:
        break;
    case ZB_START_CONFIRM_AREQ:
        break;
    case ZB_PERMIT_JOINING_REQUEST_SREQ:
        break;
    case ZB_PERMIT_JOINING_REQUEST_SRSP:
        break;
    case ZB_BIND_DEVICE_SREQ:
        break;
    case ZB_BIND_DEVICE_SRSP:
        break;
    case ZB_BIND_DEVICE_CONFIRM_AREQ:
        break;
    case ZB_ALLOW_BIND_SREQ:
        break;
    case ZB_ALLOW_BIND_SRSP:
        break;
    case ZB_ALLOW_BIND_CONFIRM_AREQ:
        break;
    case ZB_SEND_DATA_REQUEST_SREQ:
        break;
    case ZB_SEND_DATA_REQUEST_SRSP:
        break;
    case ZB_SEND_DATA_CONFIRM_AREQ:
        break;
    case ZB_RECEIVE_DATA_INDICATION_AREQ:
        break;
    case ZB_GET_DEVICE_INFO_SREQ:
        break;
    case ZB_GET_DEVICE_INFO_SRSP:
        break;
    case ZB_FIND_DEVICE_CONFIRM_AREQ:
        break;
    case ZB_FIND_DEVICE_REQUEST_SREQ:
        break;
    case ZB_FIND_DEVICE_REQUEST_SRSP:
        break;
    case AF_REGISTER_SREQ:
        break;
    case AF_REGISTER_SRSP:
        break;
    case AF_DATA_REQUEST_EXT_SRSP:
        break;
    case AF_INTER_PAN_CTL_SREQ:
        break;
    case AF_INTER_PAN_CTL_SRSP:
        break;
    case AF_DATA_STORE_SREQ:
        break;
    case AF_DATA_STORE_SRSP:
        break;
    case AF_INCOMING_MSG_EXT_AREQ:
        break;
    case AF_DATA_RETRIEVE_SREQ:
        break;
    case AF_DATA_RETRIEVE_SRSP:
        break;
    case AF_APSF_CONFIG_SET_SREQ:
        break;
    case AF_APSF_CONFIG_SET_SRSP:
        break;
    case ZDO_NWK_ADDR_REQ_SREQ:
        break;
    case ZDO_NWK_ADDR_REQ_SRSP:
        break;
    case ZDO_IEEE_ADDR_REQ_SREQ:
        break;
    case ZDO_IEEE_ADDR_REQ_SRSP:
        break;
    case ZDO_NODE_DESC_REQ_SREQ:
        break;
    case ZDO_NODE_DESC_REQ_SRSP:
        break;
    case ZDO_POWER_DESC_REQ_SREQ:
        break;
    case ZDO_POWER_DESC_REQ_SRSP:
        break;
    case ZDO_SIMPLE_DESC_REQ_SREQ:
        break;
    case ZDO_SIMPLE_DESC_REQ_SRSP:
        break;
    case ZDO_ACTIVE_EP_REQ_SREQ:
        break;
    case ZDO_ACTIVE_EP_REQ_SRSP:
        break;
    case ZDO_MATCH_DESC_REQ_SREQ:
        break;
    case ZDO_MATCH_DESC_REQ_SRSP:
        break;
    case ZDO_COMPLEX_DESC_REQ_SREQ:
        break;
    case ZDO_COMPLEX_DESC_REQ_SRSP:
        break;
    case ZDO_USER_DESC_REQ_SREQ:
        break;
    case ZDO_USER_DESC_REQ_SRSP:
        break;
    case ZDO_SERVER_DISC_REQ_SREQ:
        break;
    case ZDO_SERVER_DISC_REQ_SRSP:
        break;
    case ZDO_END_DEVICE_BIND_REQ_SREQ:
        break;
    case ZDO_END_DEVICE_BIND_REQ_SRSP:
        break;
    case ZDO_BIND_REQ_SREQ:
        break;
    case ZDO_BIND_REQ_SRSP:
        break;
    case ZDO_UNBIND_REQ_SREQ:
        break;
    case ZDO_UNBIND_REQ_SRSP:
        break;
    case ZDO_MGMT_NWK_DISC_REQ_SREQ:
        break;
    case ZDO_MGMT_NWK_DISC_REQ_SRSP:
        break;
    case ZDO_MGMT_LQI_REQ_SREQ:
        break;
    case ZDO_MGMT_LQI_REQ_SRSP:
        break;
    case ZDO_MGMT_RTG_REQ_SREQ:
        break;
    case ZDO_MGMT_RTG_REQ_SRSP:
        break;
    case ZDO_MGMT_BIND_REQ_SREQ:
        break;
    case ZDO_MGMT_BIND_REQ_SRSP:
        break;
    case ZDO_MGMT_LEAVE_REQ_SREQ:
        break;
    case ZDO_MGMT_LEAVE_REQ_SRSP:
        break;
    case ZDO_MGMT_DIRECT_JOIN_REQ_SREQ:
        break;
    case ZDO_MGMT_DIRECT_JOIN_REQ_SRSP:
        break;
    case ZDO_MGMT_NWK_UPDATE_REQ_SREQ:
        break;
    case ZDO_MGMT_NWK_UPDATE_REQ_SRSP:
        break;
    case ZDO_STARTUP_FROM_APP_SREQ:
        break;
    case ZDO_STARTUP_FROM_APP_SRSP:
        break;
    case ZDO_AUTO_FIND_DESTINATION_AREQ:
        break;
    case ZDO_SET_LINK_KEY_SREQ:
        break;
    case ZDO_SET_LINK_KEY_SRSP:
        break;
    case ZDO_REMOVE_LINK_KEY_SREQ:
        break;
    case ZDO_REMOVE_LINK_KEY_SRSP:
        break;
    case ZDO_GET_LINK_KEY_SREQ:
        break;
    case ZDO_GET_LINK_KEY_SRSP:
        break;
    case ZDO_NWK_DISCOVERY_REQ_SREQ:
        break;
    case ZDO_NWK_DISCOVERY_REQ_SRSP:
        break;
    case ZDO_JOIN_REQ_SREQ:
        break;
    case ZDO_JOIN_REQ_SRSP:
        break;
    case ZDO_NWK_ADDR_RSP_AREQ:
        break;
    case ZDO_IEEE_ADDR_RSP_AREQ:
        break;
    case ZDO_NODE_DESC_RSP_AREQ:
        break;
    case ZDO_POWER_DESC_RSP_AREQ:
        break;
    case ZDO_SIMPLE_DESC_RSP_AREQ:
        break;
    case ZDO_ACTIVE_EP_RSP_AREQ:
        break;
    case ZDO_MATCH_DESC_RSP_AREQ:
        break;
    case ZDO_COMPLEX_DESC_RSP_AREQ:
        break;
    case ZDO_USER_DESC_RSP_AREQ:
        break;
    case ZDO_USER_DESC_CONF_AREQ:
        break;
    case ZDO_SERVER_DISC_RSP_AREQ:
        break;
    case ZDO_END_DEVICE_BIND_RSP_AREQ:
        break;
    case ZDO_BIND_RSP_AREQ:
        break;
    case ZDO_UNBIND_RSP_AREQ:
        break;
    case ZDO_MGMT_NWK_DISC_RSP_AREQ:
        break;
    case ZDO_MGMT_LQI_RSP_AREQ:
        break;
    case ZDO_MGMT_RTG_RSP_AREQ:
        break;
    case ZDO_MGMT_BIND_RSP_AREQ:
        break;
    case ZDO_MGMT_LEAVE_RSP_AREQ:
        break;
    case ZDO_MGMT_DIRECT_JOIN_RSP_AREQ:
        break;
    case ZDO_STATE_CHANGE_IND_AREQ:
        break;
    case ZDO_END_DEVICE_ANNCE_IND_AREQ:
        break;
    case ZDO_MATCH_DESC_RSP_SENT_AREQ:
        break;
    case ZDO_STATUS_ERROR_RSP_AREQ:
        break;
    case ZDO_SRC_RTG_IND_AREQ:
        break;
    case ZDO_LEAVE_IND_AREQ:
        break;
    case ZDO_MSG_CB_REGISTER_SREQ:
        break;
    case ZDO_MSG_CB_REGISTER_SRSP:
        break;
    case ZDO_MSG_CB_REMOVE_SREQ:
        break;
    case ZDO_MSG_CB_REMOVE_SRSP:
        break;
    case ZDO_MSG_CB_INCOMING_AREQ:
        break;
    case UTIL_DATA_REQ_SREQ:
        break;
    case UTIL_DATA_REQ_SRSP:
        break;
    case UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SREQ:
        break;
    case UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SRSP:
        break;
    case UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SREQ:
        break;
    case UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SRSP:
        break;
    case UTIL_APSME_LINK_KEY_DATA_GET_SREQ:
        break;
    case UTIL_APSME_LINK_KEY_DATA_GET_SRSP:
        break;
    case UTIL_APSME_LINK_KEY_NV_ID_GET_SREQ:
        break;
    case UTIL_APSME_LINK_KEY_NV_ID_GET_SRSP:
        break;
    case UTIL_APSME_REQUEST_KEY_CMD_SREQ:
        break;
    case UTIL_APSME_REQUEST_KEY_CMD_SRSP:
        break;
    case UTIL_ASSCO_COUNT_SREQ:
        break;
    case UTIL_ASSCO_COUNT_SRSP:
        break;
    case UTIL_ASSOC_FIND_DEVICE_SREQ:
        break;
    case UTIL_ASSOC_FIND_DEVICE_SRSP:
        break;
    case UTIL_ZCL_KEY_EST_INIT_EST_SREQ:
        break;
    case UTIL_ZCL_KEY_EST_INIT_EST_SRSP:
        break;
    case UTIL_ZCL_KEY_EST_SIGN_SREQ:
        break;
    case UTIL_ZCL_KEY_EST_SIGN_SRSP:
        break;
    case UTIL_ZCL_KEY_ESTABLISH_IND_AREQ:
        break;
    case UTIL_TEST_LOOPBACK_SREQ:
        break;
    case UTIL_TEST_LOOPBACK_SRSP:
        break;
    default:
        break;
    }
    proto_tree_add_item(znp_tree, hf_znp_fcs, tvb, offset, 1, ENC_NA);
    return offset;
};
/* dissect a packet */
/* register the znp protocol with Wireshark */
void proto_register_znp(void)
{
    /* list of fields */
    static hf_register_info hf[] = {
        {&hf_znp_command,
         {"Command", "znp.command",
          FT_UINT16, BASE_HEX,
          VALS(znp_cmd_types_values), 0x0,
          "ZNP Command type", HFILL}},
        {&hf_znp_sof,
         {"SOF", "znp.sof",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Start of the frame", HFILL}},
        {&hf_znp_length, {"Length", "znp.Len", FT_UINT8, BASE_HEX, NULL, 0x0, "Length of the data", HFILL}},
        {&hf_znp_fcs, {"FCS", "znp.FCS", FT_UINT8, BASE_HEX, NULL, 0x0, "Frame Check Sequence", HFILL}},
        {&hf_znp_data_request_sreq_dstaddr,
         {"Destniation Address", "znp.data_request.sreq.destniation_address",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Destniation address of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_dstendpoint,
         {"Destniation Endpoint", "znp.data_request.sreq.destniation_endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Destniation endpoint of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_srcendpoint,
         {"Source Endpoint", "znp.data_request.sreq.source_endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Source endpoint of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_clusterid,
         {"Cluster ID", "znp.data_request.sreq.cluster_id",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Cluster ID of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_transid,
         {"Transaction ID", "znp.data_request.sreq.transaction_id",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Transaction ID of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_options,
         {"Options", "znp.data_request.sreq.options",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Options of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_radius,
         {"Radius", "znp.data_request.sreq.radius",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Radius of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_len,
         {"Data Length", "znp.data_request.sreq.len",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Data length of data request system request packet", HFILL}},
        {&hf_znp_data_request_sreq_data,
         {"Data", "znp.data_request.sreq.data",
          FT_BYTES, BASE_NONE,
          NULL, 0x0,
          "Data payload of data request system request packet", HFILL}},
        {&hf_znp_data_request_srsp_status,
         {"Status", "znp.data_request.srsp.status",
          FT_UINT8, BASE_HEX,
          VALS(znp_status_values), 0x0,
          "Status of data request system response packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_dstaddrMode,
         {"Destniation Address Mode", "znp.data_request_ext.sreq.destniation_address_mode",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Destniation address mode of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_dstaddr,
         {"Destniation Address", "znp.data_request_ext.sreq.destniation_address",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Destniation address of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_dstendpoint,
         {"Destniation Endpoint", "znp.data_request_ext.sreq.destniation_endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Destniation endpoint of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_dstPanid,
         {"Destniation PAN ID", "znp.data_request_ext.sreq.destniation_pan_id",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Destniation PAN ID of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_srcendpoint,
         {"Source Endpoint", "znp.data_request_ext.sreq.source_endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Source endpoint of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_clusterid,
         {"Cluster ID", "znp.data_request_ext.sreq.cluster_id",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Cluster ID of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_transid,
         {"Transaction ID", "znp.data_request_ext.sreq.transaction_id",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Transaction ID of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_options,
         {"Options", "znp.data_request_ext.sreq.options",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Options of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_radius,
         {"Radius", "znp.data_request_ext.sreq.radius",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Radius of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_len,
         {"Data Length", "znp.data_request_ext.sreq.len",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Data length of data request extended system request packet", HFILL}},
        {&hf_znp_data_request_ext_sreq_data,
         {"Data", "znp.data_request_ext.sreq.data",
          FT_BYTES, BASE_NONE,
          NULL, 0x0,
          "Data payload of data request extended system request packet", HFILL}},
        {&hf_znp_data_confirm_areq_endpoint,
         {"Endpoint", "znp.data_confirm.areq.endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Endpoint of data confirm asynchronous response packet", HFILL}},
        {&hf_znp_data_confirm_areq_status,
         {"Status", "znp.data_confirm.areq.status",
          FT_UINT8, BASE_HEX,
          VALS(znp_status_values), 0x0,
          "Status of data confirm asynchronous response packet", HFILL}},
        {&hf_znp_data_confirm_areq_transid,
         {"Transaction ID", "znp.data_confirm.areq.transaction_id",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Transaction ID of data confirm asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_groupid,
         {"Group ID", "znp.incoming_msg.areq.group_id",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Group ID of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_clusterid,
         {"Cluster ID", "znp.incoming_msg.areq.cluster_id",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Cluster ID of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_srcaddr,
         {"Source Address", "znp.incoming_msg.areq.source_address",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Source address of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_srcendpoint,
         {"Source Endpoint", "znp.incoming_msg.areq.source_endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Source endpoint of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_dstendpoint,
         {"Destniation Endpoint", "znp.incoming_msg.areq.destniation_endpoint",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Destniation endpoint of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_wasbroadcast,
         {"Was Broadcast", "znp.incoming_msg.areq.was_broadcast",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "check whether the message is broadcast or not", HFILL}},
        {&hf_znp_incoming_msg_areq_linkquality,
         {"Link Quality", "znp.incoming_msg.areq.link_quality",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Link quality of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_securityuse,
         {"Security Use", "znp.incoming_msg.areq.security_use",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Security use of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_timestamp,
         {"Timestamp", "znp.incoming_msg.areq.timestamp",
          FT_UINT32, BASE_HEX,
          NULL, 0x0,
          "Timestamp of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_transseqnumber,
         {"Transaction Sequence Number", "znp.incoming_msg.areq.transaction_sequence_number",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Transaction sequence number of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_len,
         {"Data Length", "znp.incoming_msg.areq.len",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Data length of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_data,
         {"Data", "znp.incoming_msg.areq.data",
          FT_BYTES, BASE_NONE,
          NULL, 0x0,
          "Data payload of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_macsrcaddr,
         {"MAC Source Address", "znp.incoming_msg.areq.mac_source_address",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "MAC source address of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_incoming_msg_areq_msgResultRadius,
         {"Message Result Radius", "znp.incoming_msg.areq.message_result_radius",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Message result radius of incoming message asynchronous response packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_req_sreq_addrmode,
         {"Address Mode", "znp.zdo_mgmt_permit_join_req.sreq.address_mode",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Address mode of ZDO management permit join request system request packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_req_sreq_dstaddr,
         {"Destination Address", "znp.zdo_mgmt_permit_join_req.sreq.destination_address",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Destination address of ZDO management permit join request system request packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_req_sreq_duration,
         {"Duration", "znp.zdo_mgmt_permit_join_req.sreq.duration",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Duration of ZDO management permit join request system request packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_req_sreq_tcsignificance,
         {"Trust Center Significance", "znp.zdo_mgmt_permit_join_req.sreq.trust_center_significance",
          FT_UINT8, BASE_HEX,
          NULL, 0x0,
          "Trust center significance of ZDO management permit join request system request packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_req_srsp_status,
         {"Status", "znp.zdo_mgmt_permit_join_req.srsp.status",
          FT_UINT8, BASE_HEX,
          VALS(znp_status_values), 0x0,
          "Status of ZDO management permit join request system response packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_rsp_areq_status,
         {"Status", "znp.zdo_mgmt_permit_join_rsp.areq.status",
          FT_UINT8, BASE_HEX,
          VALS(znp_status_values), 0x0,
          "Status of ZDO management permit join response asynchronous response packet", HFILL}},
        {&hf_znp_zdo_mgmt_permit_join_rsp_areq_srcaddr,
         {"Source Address", "znp.zdo_mgmt_permit_join_rsp.areq.source_address",
          FT_UINT16, BASE_HEX,
          NULL, 0x0,
          "Source address of ZDO management permit join response asynchronous response packet", HFILL}},

    };

    /* protocol subtree arrays */
    static gint *ett[] = {
        &ett_znp};

    /* register the protocol name and description */
    proto_znp = proto_register_protocol(
        "Zigbee Network Processor", /* full name */
        "ZNP",                      /* short name */
        "znp"                       /* abbreviation (e.g. for filters) */
    );
    register_dissector("znp", dissect_znp, proto_znp);
    /* register the header fields and subtrees used */
    proto_register_field_array(proto_znp, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_znp(void)
{
    create_dissector_handle(dissect_znp, proto_znp);
    zcl_handle = find_dissector("zbee_zcl");
    if (zcl_handle == NULL) {
        printf("ZCL dissector not found");
        // printf("test");
    }
}
/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
