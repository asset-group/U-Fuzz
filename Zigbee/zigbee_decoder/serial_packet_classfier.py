from binascii import hexlify, unhexlify
from ZNP_decoder import *


def classifier(data):
    # result = None
    data_decoded = hexlify(data).decode('utf8')
    command = data_decoded[4:8]
    if command == "2401":
        print("AF_DATA_REQUEST_SREQ: ", data_decoded)
        AF_DATA_REQUEST = ZNP(data)
        AF_DATA_REQUEST.Data = ZigbeeClusterLibrary(AF_DATA_REQUEST.Data)
        AF_DATA_REQUEST.show()
    elif command == "4100":
        # result = "SYS_RESET_REQ: ",data
        print("SYS_RESET_REQ_AREQ: ", data_decoded)
    elif command == "4180":
        print("SYS_RESET_IND_AREQ: ", data_decoded)
    elif command == "2102":
        print("SYS_VERSION_SREQ: ", data_decoded)
    elif command == "6102":
        print("SYS_VERSION_SRSP: ", data_decoded)
    elif command == "2108":
        print("SYS_OSAL_NV_READ_SREQ: ", data_decoded)
    elif command == "6108":
        print("SYS_OSAL_NV_READ_SRSP: ", data_decoded)
    elif command == "2109":
        print("SYS_OSAL_NV_WRITE_SREQ: ", data_decoded)
    elif command == "6109":
        print("SYS_OSAL_NV_WRITE_SRSP: ", data_decoded)
    elif command == "2107":
        print("SYS_OSAL_NV_ITEM_INIT_SREQ: ", data_decoded)
    elif command == "6107":
        print("SYS_OSAL_NV_ITEM_INIT_SRSP: ", data_decoded)
    elif command == "2112":
        print("SYS_OSAL_NV_DELETE_SREQ: ", data_decoded)
    elif command == "6112":
        print("SYS_OSAL_NV_DELETE_SRSP: ", data_decoded)
    elif command == "2113":
        print("SYS_OSAL_NV_LENGTH_SREQ: ", data_decoded)
    elif command == "6113":
        print("SYS_OSAL_NV_LENGTH_SRSP: ", data_decoded)
    elif command == "210d":
        print("SYS_ADC_READ_SREQ: ", data_decoded)
    elif command == "610d":
        print("SYS_ADC_READ_SRSP: ", data_decoded)
    elif command == "210e":
        print("SYS_GPIO_SREQ: ", data_decoded)
    elif command == "610e":
        print("SYS_GPIO_SRSP: ", data_decoded)
    elif command == "210c":
        print("SYS_RANDOM_SREQ: ", data_decoded)
    elif command == "610c":
        print("SYS_RANDOM_SRSP: ", data_decoded)
    elif command == "2110":
        print("SYS_SET_TIME_SREQ: ", data_decoded)
    elif command == "6110":
        print("SYS_SET_TIME_SRSP: ", data_decoded)
    elif command == "2111":
        print("SYS_GET_TIME_SREQ: ", data_decoded)
    elif command == "6111":
        print("SYS_GET_TIME_SRSP: ", data_decoded)
    elif command == "2114":
        print("SYS_SET_TX_POWER_SREQ: ", data_decoded)
    elif command == "6114":
        print("SYS_SET_TX_POWER_SRSP: ", data_decoded)
    elif command == "2604":
        print("ZB_READ_CONFIGURATION_SREQ: ", data_decoded)
    elif command == "6604":
        print("ZB_READ_CONFIGURATION_SRSP: ", data_decoded)
    elif command == "2605":
        print("ZB_WRITE_CONFIGURATION_SREQ: ", data_decoded)
    elif command == "6605":
        print("ZB_WRITE_CONFIGURATION_SRSP: ", data_decoded)
    elif command == "0003":
        print("ZCD_NV_STARTUP_OPTION: ", data_decoded)
    elif command == "0087":
        print("ZCD_NV_LOGICAL_TYPE: ", data_decoded)
    elif command == "008f":
        print("ZCD_NV_ZDO_DIRECT_CB: ", data_decoded)
    elif command == "0024":
        print("ZCD_NV_POLL_RATE: ", data_decoded)
    elif command == "0025":
        print("ZCD_NV_QUEUED_POLL_RATE: ", data_decoded)
    elif command == "0026":
        print("ZCD_NV_RESPONSE_POLL_RATE: ", data_decoded)
    elif command == "0029":
        print("ZCD_NV_POLL_FAILURE_RETRIES: ", data_decoded)
    elif command == "002b":
        print("ZCD_NV_INDIRECT_MSG_TIMEOUT: ", data_decoded)
    elif command == "0043":
        print("ZCD_NV_APS_FRAME_RETRIES: ", data_decoded)
    elif command == "0044":
        print("ZCD_NV_APS_ACK_WAIT_DURATION: ", data_decoded)
    elif command == "0046":
        print("ZCD_NV_BINDING_TIME: ", data_decoded)
    elif command == "0081":
        print("ZCD_NV_USERDESC: ", data_decoded)
    elif command == "0083":
        print("ZCD_NV_PANID: ", data_decoded)
    elif command == "0084":
        print("ZCD_NV_CHANLIST: ", data_decoded)
    elif command == "0062":
        print("ZCD_NV_PRECFGKEY: ", data_decoded)
    elif command == "0063":
        print("ZCD_NV_PRECFGKEYS_ENABLE: ", data_decoded)
    elif command == "0064":
        print("ZCD_NV_SECURITY_MODE: ", data_decoded)
    elif command == "006d":
        print("ZCD_NV_USE_DEFAULT_TCLK: ", data_decoded)
    elif command == "002e":
        print("ZCD_NV_BCAST_RETRIES: ", data_decoded)
    elif command == "002f":
        print("ZCD_NV_PASSIVE_ACK_TIMEOUT: ", data_decoded)
    elif command == "0030":
        print("ZCD_NV_BCAST_DELIVERY_TIME: ", data_decoded)
    elif command == "002c":
        print("ZCD_NV_ROUTE_EXPIRY_TIME: ", data_decoded)
    elif command == "0f07":
        print("ZNP_NV_RF_TEST_PARAMS: ", data_decoded)
    elif command == "260a":
        print("ZB_APP_REGISRTER_REQUEST_SREQ: ", data_decoded)
    elif command == "660a":
        print("ZB_APP_REGISRTER_REQUEST_SRSP: ", data_decoded)
    elif command == "2600":
        print("ZB_START_REQUEST_SREQ: ", data_decoded)
    elif command == "6600":
        print("ZB_START_REQUEST_SRSP: ", data_decoded)
    elif command == "4680":
        print("ZB_START_CONFIRM_AREQ: ", data_decoded)
    elif command == "2608":
        print("ZB_PERMIT_JOINING_REQUEST_SREQ: ", data_decoded)
    elif command == "6608":
        print("ZB_PERMIT_JOINING_REQUEST_SRSP: ", data_decoded)
    elif command == "2601":
        print("ZB_BIND_DEVICE_SREQ: ", data_decoded)
    elif command == "6601":
        print("ZB_BIND_DEVICE_SRSP: ", data_decoded)
    elif command == "4681":
        print("ZB_BIND_CONFIRM_AREQ: ", data_decoded)
    elif command == "2602":
        print("ZB_ALLOW_BIND_SREQ: ", data_decoded)
    elif command == "6602":
        print("ZB_ALLOW_BIND_SRSP: ", data_decoded)
    elif command == "4682":
        print("ZB_ALLOW_BIND_CONFIRM_AREQ: ", data_decoded)
    elif command == "2603":
        print("ZB_SEND_DATA_REQUEST_SREQ: ", data_decoded)
    elif command == "6603":
        print("ZB_SEND_DATA_REQUEST_SRSP: ", data_decoded)
    elif command == "4683":
        print("ZB_SEND_DATA_CONFIRM:AREQ: ", data_decoded)
    elif command == "4687":
        print("ZB_RECEIVE_DATA_INDICATION_AREQ: ", data_decoded)
    elif command == "2606":
        print("ZB_GET_DEVICE_INFO_SREQ: ", data_decoded)
    elif command == "6606":
        print("ZB_GET_DEVICE_INFO_SRSP: ", data_decoded)
    elif command == "2607":
        print("ZB_FIND_DEVICE_REQUEST_SREQ: ", data_decoded)
    elif command == "6607":
        print("ZB_FIND_DEVICE_REQUEST_SRSP: ", data_decoded)
    elif command == "4685":
        print("ZB_FIND_DEVICE_CONFIRM_AREQ: ", data_decoded)
    elif command == "2400":
        print("AF_REGISTER_SREQ: ", data_decoded)
    elif command == "6400":
        print("AF_REGISTER_SRSP: ", data_decoded)
    elif command == "6401":
        print("AF_DATA_REQUEST_SRSP: ", data_decoded)
        AF_DATA_REQUEST_SRSP_packet = ZNP(data)
        AF_DATA_REQUEST_SRSP_packet.show()
    elif command == "2402":
        print('AF_DATA_REQUEST_EXT_SREQ/AF_DATA_REQUEST_SRC_RTG_SREQ(differentiate by len field): ', data_decoded)
        AF_DATA_REQUEST_EXT = ZNP(data)
        AF_DATA_REQUEST_EXT.Data = ZigbeeClusterLibrary(
            AF_DATA_REQUEST_EXT.Data)
        AF_DATA_REQUEST_EXT.show()
    elif command == "6402":
        print("AF_DATA_REQUEST_EXT_SRSP/AF_DATA_REQUEST_SRC_RTG_SRSP(differentiate by len field): ", data_decoded)
    elif command == "2410":
        print("AF_INTER_PAN_CTL_SREQ: ", data_decoded)
    elif command == "6410":
        print("AF_INTER_PAN_CTL_SRSP: ", data_decoded)
    elif command == "2411":
        print("AF_DATA_STORE_SREQ: ", data_decoded)
    elif command == "6411":
        print("AF_DATA_STORE_SRSP: ", data_decoded)
    elif command == "4480":
        print("AF_DATA_CONFIRM_AREQ: ", data_decoded)
        AF_DATA_CONFIRM_AREQ_packet = ZNP(data)
        AF_DATA_CONFIRM_AREQ_packet.show()
    elif command == "4481":
        print("AF_INCOMING_MSG_AREQ: ", data_decoded)
        AF_INCOMING_MSG_packet = ZNP(data)
        AF_INCOMING_MSG_packet.show()
    elif command == "4482":
        print("AF_INCOMING_MSG_EXT_AREQ: ", data_decoded)
    elif command == "2412":
        print("AF_DATA_RETRIEVE_SREQ: ", data_decoded)
    elif command == "6412":
        print("AF_DATA_RETRIEVE_SRSP: ", data_decoded)
    elif command == "2413":
        print("AF_APSF_CONFIG_SET_SREQ: ", data_decoded)
    elif command == "6413":
        print("AF_APSF_CONFIG_SET_SRSP: ", data_decoded)
    elif command == "2500":
        print("ZDO_NWK_ADDR_REQ_SREQ: ", data_decoded)
    elif command == "6500":
        print("ZDO_NWK_ADDR_REQ_SRSP: ", data_decoded)
    elif command == "2501":
        print("ZDO_IEEE_ADDR_REQ_SREQ: ", data_decoded)
    elif command == "6501":
        print("ZDO_IEEE_ADDR_REQ_SRSP: ", data_decoded)
    elif command == "2502":
        print("ZDO_NODE_DESC_REQ_SREQ: ", data_decoded)
    elif command == "6502":
        print("ZDO_NODE_DESC_REQ_SRSP: ", data_decoded)
    elif command == "2503":
        print("ZDO_POWER_DESC_REQ_SREQ: ", data_decoded)
    elif command == "6503":
        print("ZDO_POWER_DESC_REQ_SRSP: ", data_decoded)
    elif command == "2504":
        print("ZDO_SIMPLE_DESC_REQ_SREQ: ", data_decoded)
    elif command == "6504":
        print("ZDO_SIMPLE_DESC_REQ_SRSP: ", data_decoded)
    elif command == "2505":
        print("ZDO_ACTIVE_EP_REQ_SREQ: ", data_decoded)
    elif command == "6505":
        print("ZDO_ACTIVE_EP_REQ__SRSP: ", data_decoded)
    elif command == "2506":
        print("ZDO_MATCH_DESC_REQ_SREQ: ", data_decoded)
    elif command == "6506":
        print("ZDO_MATCH_DESC_REQ_SRSP: ", data_decoded)
    elif command == "2507":
        print("ZDO_COMPLEX_DESC_REQ_SREQ: ", data_decoded)
    elif command == "6507":
        print("ZDO_COMPLEX_DESC_REQ_SRSP: ", data_decoded)
    elif command == "2508":
        print("ZDO_USER_DESC_REQ_SREQ: ", data_decoded)
    elif command == "6508":
        print("ZDO_USER_DESC_REQ_SRSP: ", data_decoded)
    elif command == "250a":
        print("ZDO_DEVICE_ANNCE_SREQ: ", data_decoded)
    elif command == "650a":
        print("ZDO_DEVICE_ANNCE_SRSP: ", data_decoded)
    elif command == "250b":
        print("ZDO_USER_DESC_SET_SREQ: ", data_decoded)
    elif command == "650b":
        print("ZDO_USER_DESC_SET_SRSP: ", data_decoded)
    elif command == "250c":
        print("ZDO_SERVER_DISC_REQ_SREQ: ", data_decoded)
    elif command == "650c":
        print("ZDO_SERVER_DISC_REQ_SRSP: ", data_decoded)
    elif command == "2520":
        print("ZDO_END_DEVICE_BIND_REQ_SREQ: ", data_decoded)
    elif command == "6520":
        print("ZDO_END_DEVICE_BIND_REQ_SRSP: ", data_decoded)
    elif command == "2521":
        print("ZDO_BIND_REQ_SREQ: ", data_decoded)
    elif command == "6521":
        print("ZDO_BIND_REQ_SRSP: ", data_decoded)
    elif command == "2522":
        print("ZDO_UNBIND_REQ_SREQ: ", data_decoded)
    elif command == "6522":
        print("ZDO_UNBIND_REQ_SRSP: ", data_decoded)
    elif command == "2530":
        print("ZDO_MGMT_NWK_DISC_REQ_SREQ: ", data_decoded)
    elif command == "6530":
        print("ZDO_MGMT_NWK_DISC_REQ_SRSP: ", data_decoded)
    elif command == "2531":
        print("ZDO_MGMT_LQI_REQ_SREQ: ", data_decoded)
    elif command == "6531":
        print("ZDO_MGMT_LQI_REQ_SRSP: ", data_decoded)
    elif command == "2532":
        print("ZDO_MGMT_RTG_REQ_SREQ: ", data_decoded)
    elif command == "6532":
        print("ZDO_MGMT_RTG_REQ_SRSP: ", data_decoded)
    elif command == "2533":
        print("ZDO_MGMT_BIND_REQ_SREQ: ", data_decoded)
    elif command == "6533":
        print("ZDO_MGMT_BIND_REQ_SRSP: ", data_decoded)
    elif command == "2534":
        print("ZDO_MGMT_LEAVE_REQ_SREQ: ", data_decoded)
    elif command == "6534":
        print("ZDO_MGMT_LEAVE_REQ_SRSP: ", data_decoded)
    elif command == "2535":
        print("ZDO_MGMT_DIRECT_JOIN_REQ_SREQ: ", data_decoded)
    elif command == "6535":
        print("ZDO_MGMT_DIRECT_JOIN_REQ_SRSP: ", data_decoded)
    elif command == "2536":
        print("ZDO_MGMT_PERMIT_JOIN_REQ_SREQ: ", data_decoded)
        ZDO_MGMT_PERMIT_JOIN_REQ_SREQ_packet = ZNP(data)
        ZDO_MGMT_PERMIT_JOIN_REQ_SREQ_packet.show()
    elif command == "6536":
        print("ZDO_MGMT_PERMIT_JOIN_REQ_SRSP: ", data_decoded)
        ZDO_MGMT_PERMIT_JOIN_REQ_SRSP_packet = ZNP(data)
        ZDO_MGMT_PERMIT_JOIN_REQ_SRSP_packet.show()
    elif command == "2537":
        print("ZDO_MGMT_NWK_UPDATE_REQ_SREQ: ", data_decoded)
    elif command == "6537":
        print("ZDO_MGMT_NWK_UPDATE_REQ_SRSP: ", data_decoded)
    elif command == "2540":
        print("ZDO_STARTUP_FROM_APP_SREQ: ", data_decoded)
    elif command == "6540":
        print("ZDO_STARTUP_FROM_APP_SRSP: ", data_decoded)
    elif command == "4541":
        print("ZDO_AUTO_FIND_DESTINATION_AREQ: ", data_decoded)
    elif command == "2523":
        print("ZDO_SET_LINK_KEY_SREQ: ", data_decoded)
    elif command == "6523":
        print("ZDO_SET_LINK_KEY_SRSP: ", data_decoded)
    elif command == "2524":
        print("ZDO_REMOVE_LINK_KEY_SREQ: ", data_decoded)
    elif command == "6524":
        print("ZDO_REMOVE_LINK_KEY_SRSP: ", data_decoded)
    elif command == "2525":
        print("ZDO_GET_LINK_KEY_SREQ: ", data_decoded)
    elif command == "6525":
        print("ZDO_GET_LINK_KEY_SRSP: ", data_decoded)
    elif command == "2526":
        print("ZDO_NWK_DISCOVERY_REQ_SREQ: ", data_decoded)
    elif command == "6526":
        print("ZDO_NWK_DISCOVERY_REQ_SRSP: ", data_decoded)
    elif command == "2527":
        print("ZDO_JOIN_REQ_SREQ: ", data_decoded)
    elif command == "6527":
        print("ZDO_JOIN_REQ_SRSP: ", data_decoded)
    elif command == "4580":
        print("ZDO_NWK_ADDR_RSP_AREQ: ", data_decoded)
    elif command == "4581":
        print("ZDO_IEEE_ADDR_RSP_AREQ: ", data_decoded)
    elif command == "4582":
        print("ZDO_NODE_DESC_RSP_AREQ: ", data_decoded)
    elif command == "4583":
        print("ZDO_POWER_DESC_RSP_AREQ: ", data_decoded)
    elif command == "4584":
        print("ZDO_SIMPLE_DESC_RSP_AREQ: ", data_decoded)
    elif command == "4585":
        print("ZDO_ACTIVE_EP_RSP_AREQ: ", data_decoded)
    elif command == "4586":
        print("ZDO_MATCH_DESC_RSP_AREQ: ", data_decoded)
    elif command == "4587":
        print("ZDO_COMPLEX_DESC_RSP_AREQ: ", data_decoded)
    elif command == "4588":
        print("ZDO_USER_DESC_RSP_AREQ: ", data_decoded)
    elif command == "4589":
        print("ZDO_USER_DESC_CONF_AREQ: ", data_decoded)
    elif command == "458a":
        print("ZDO_SERVER_DISC_RSP_AREQ: ", data_decoded)
    elif command == "45a0":
        print("ZDO_END_DEVICE_BIND_RSP_AREQ: ", data_decoded)
    elif command == "45a1":
        print("ZDO_BIND_RSP_AREQ: ", data_decoded)
    elif command == "45a2":
        print("ZDO_UNBIND_RSP_AREQ: ", data_decoded)
    elif command == "45b0":
        print("ZDO_MGMT_NWK_DISC_RSP_AREQ: ", data_decoded)
    elif command == "45b1":
        print("ZDO_MGMT_LQI_RSP_AREQ: ", data_decoded)
    elif command == "45b2":
        print("ZDO_MGMT_RTG_RSP_AREQ: ", data_decoded)
    elif command == "45b3":
        print("ZDO_MGMT_BIND_RSP_AREQ: ", data_decoded)
    elif command == "45b4":
        print("ZDO_MGMT_LEAVE_RSP_AREQ: ", data_decoded)
    elif command == "45b5":
        print("ZDO_MGMT_DIRECT_JOIN_RSP_AREQ: ", data_decoded)
    elif command == "45b6":
        print("ZDO_MGMT_PERMIT_JOIN_RSP_AREQ: ", data_decoded)
        ZDO_MGMT_PERMIT_JOIN_RSP_AREQ_packet = ZNP(data)
        ZDO_MGMT_PERMIT_JOIN_RSP_AREQ_packet.show()
    elif command == "45c0":
        print("ZDO_STATE_CHANGE_IND_AREQ: ", data_decoded)
    elif command == "45c1":
        print("ZDO_END_DEVICE_ANNCE_IND_AREQ: ", data_decoded)
    elif command == "45c2":
        print("ZDO_MATCH_DESC_RSP_SENT_AREQ: ", data_decoded)
    elif command == "45c3":
        print("ZDO_STATUE_ERROR_RSP_AREQ: ", data_decoded)
    elif command == "45c4":
        print("ZDO_SRC_RTG_IND_AREQ: ", data_decoded)
    elif command == "45c9":
        print("ZDO_LEAVE_IND_AREQ: ", data_decoded)
    elif command == "253e":
        print("ZDO_MSG_CB_REGISTER_SREQ: ", data_decoded)
    elif command == "653e":
        print("ZDO_MSG_CB_REGISTER_SRSP: ", data_decoded)
    elif command == "253f":
        print("ZDO_MSG_CB_REMOVE_SREQ: ", data_decoded)
    elif command == "653f":
        print("ZDO_MSG_CB_REMOVE_SRSP: ", data_decoded)
    elif command == "45ff":
        print("ZDO_MSG_CB_INCOMING_AREQ: ", data_decoded)
    # UTIL interface
    elif command == "2711":
        print("UTIL_DATA_REQ_SREQ: ", data_decoded)
    elif command == "6711":
        print("UTIL_DATA_REQ_SRSP: ", data_decoded)
    elif command == "2740":
        print("UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SREQ: ", data_decoded)
    elif command == "6740":
        print("UTIL_ADDRMGR_EXT_ADDR_LOOKUP_SRSP: ", data_decoded)
    elif command == "2741":
        print("UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SREQ: ", data_decoded)
    elif command == "6741":
        print("UTIL_ADDRMGR_NWK_ADDR_LOOKUP_SRSP: ", data_decoded)
    elif command == "2744":
        print("UTIL_APSME_LINK_KEY_DATA_GET_SREQ: ", data_decoded)
    elif command == "6744":
        print("UTIL_APSME_LINK_KEY_DATA_GET_SRSP: ", data_decoded)
    elif command == "2745":
        print("UTIL_APSME_LINK_KEY_NV_ID_GET_SREQ: ", data_decoded)
    elif command == "6745":
        print("UTIL_APSME_LINK_KEY_NV_ID_GET_SRSP: ", data_decoded)
    elif command == "274b":
        print("UTIL_APSME_REQUEST_KEY_CMD_SREQ: ", data_decoded)
    elif command == "674b":
        print("UTIL_APSME_REQUEST_KEY_CMD_SRSP: ", data_decoded)
    elif command == "2748":
        print("UTIL_ASSCO_COUNT_SREQ: ", data_decoded)
    elif command == "6748":
        print("UTIL_ASSCO_COUNT_SRSP: ", data_decoded)
    elif command == "2749":
        print("UTIL_ASSOC_FIND_DEVICE_SREQ: ", data_decoded)
    elif command == "6749":
        print("UTIL_ASSOC_FIND_DEVICE_SRSP: ", data_decoded)
    elif command == "2780":
        print("UTIL_ZCL_KEY_EST_INIT_EST_SREQ: ", data_decoded)
    elif command == "6780":
        print("UTIL_ZCL_KEY_EST_INIT_EST_SRSP: ", data_decoded)
    elif command == "2781":
        print("UTIL_ZCL_KEY_EST_SIGN_SREQ: ", data_decoded)
    elif command == "6781":
        print("UTIL_ZCL_KEY_EST_SIGN_SRSP: ", data_decoded)
    elif command == "47e1":
        print("UTIL_ZCL_KEY_ESTABLISH_IND_AREQ: ", data_decoded)
    elif command == "2710":
        print("UTIL_TEST_LOOPBACK_SREQ: ", data_decoded)
    elif command == "6710":
        print("UTIL_TEST_LOOPBACK_SRSP: ", data_decoded)
    else:
        print("command not find", command, " ", data_decoded)
    return None


# data1 = b'\xfe\x17\x24\x01\x55\xd4\x08\x01\x04\x06\x04\x00\x1e\x0d\x10\x03\x02\x4d\x00\x42\x06\x68\x65\x6c\x6c\x6f\x31\xe4'
# print(hexlify(data1).decode('utf8')[4:8])

# classifier(data1)
