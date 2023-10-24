from scapy.all import *
from scapy.layers.zigbee import *
from binascii import unhexlify

# the normal way to do a data request for znp will be AF_DATA_REQUEST -> SRSP(synchronous response) -> AF_DATA_CONFIRM -> AF_INCOMING_MSG


class ZNP(Packet):
    name = "ZNP"
    # This field has two bytes and is called "len1"
    fields_desc = [XByteField("SOF", 0xfe), XByteField("Len", 0), ShortEnumField("cmd", 0x00, {
        0x2401: "AF_DATA_REQUEST_SREQ", 0x2402: "AF_DATA_REQUEST_EXT", 0x4480: "AF_DATA_CONFIRM", 0x4481: "AF_INCOMING_MSG", 0x2536: "ZDO_MGMT_PERMIT_JOIN_REQ_SREQ", 0x4100: "SYS_RESET_REQ", 0x4180: "SYS_RESET_IND", 0x6401: "AF_DATA_REQUEST_SRSP", 0x6536: "ZDO_MGMT_PERMIT_JOIN_REQ_SRSP"})]


class AF_DATA_REQUEST_SREQ(Packet):
    name = "AF_DATA_REQUEST_SREQ"
    fields_desc = [XShortField("DstAddr", 0x1899), XByteField("DstEndpoint", 0x01), XByteField("SrcEndpoint", 0x01), XLEShortField("ClusterId", 0x0000), XByteField(
        "TransId", 0x00), XByteField("Options", 0x00), XByteField("Radius", 0x00), XByteField("len", None), StrLenField("Data", "", length_from=lambda pkt:pkt.len), XByteField("FCS", 0x00)]


class AF_DATA_REQUEST_SRSP(Packet):
    name = "AF_DATA_REQUEST_SRSP"
    fields_desc = [XByteEnumField(
        "status", 0x00, {0x00: "Success", 0x01: "Failure"}), XByteField("FCS", 0x00)]


class AF_DATA_REQUEST_EXT(Packet):
    name = "AF_DATA_REQUEST_EXT"
    fields_desc = [XByteField("DstAddrMode", 0x02), XLongField("DstAddr", 0x0000000000000000), XByteField("DstEndpoint", 0x01), XShortField("PanId", 0x00), XByteField("SrcEndpoint", 0x01), XShortField(
        "ClusterId", 0x0000), XByteField("TransId", 0x00), XByteField("Options", 0x00), XByteField("Radius", 0x00), XShortField("len", 0x0000), StrLenField("Data", "", length_from=lambda pkt:pkt.len), XByteField("FCS", 0x00)]


class AF_DATA_CONFIRM(Packet):
    name = "AF_DATA_CONFIRM"
    fields_desc = [XByteEnumField("status", 0x00, {0x00: "Success", 0x01: "Failure"}), XByteField(
        "Endpoint", 0x00), XByteField("TransId", 0x00), XByteField("FCS", 0x00)]


class AF_INCOMING_MSG(Packet):
    name = "AF_INCOMING_MSG"
    fields_desc = [XShortField("GroupID", 0x0000), XShortField("ClusterID", 0x0000), XShortField("SrcAddr", 0x0000), XByteField("SrcEndpoint", 0x00), XByteField("DstEndpoint", 0x00), XByteField("WasBroadcast", 0x00), XByteField("LinkQuality", 0x00), XByteField(
        "SecurityUse", 0x00), XIntField("Timestamp", 0x00000000), XByteField("TransSeqNumber", 0x00), XByteField("len", 0x00), XStrLenField("Data", "", length_from=lambda pkt:pkt.len), XShortField("MacSrcAddr", 0x0000), XByteField("MsgResultRadius", 0x00), XByteField("FCS", 0x00)]


class ZDO_MGMT_PERMIT_JOIN_REQ_SREQ(Packet):
    name = "ZDO_MGMT_PERMIT_JOIN_REQ_SREQ"
    fields_desc = [XByteEnumField("addMode", 0x00, {0x00: "AddrNotPresent", 0x01: "AddGroup", 0x02: "Addr16Bit", 0x03: "Addr64Bit", 0x0f: "Broadcast"}), XShortField(
        "DstAddr", 0x0000), XByteField("Duration", 0x00), XByteField("TCSignificance", 0x00), XByteField("FCS", 0x00)]


class ZDO_MGMT_PERMIT_JOIN_REQ_SRSP(Packet):
    name = "ZDO_MGMT_PERMIT_JOIN_REQ_SRSP"
    fields_desc = [XByteEnumField(
        "status", 0x00, {0x00: "Success", 0x01: "Failure"}), XByteField("FCS", 0x00)]


class ZDO_MGMT_PERMIT_JOIN_RSP(Packet):
    name = "ZDO_MGMT_PERMIT_JOIN_RSP"
    fields_desc = [XShortField("SrcAddr", 0x0000), XByteEnumField(
        "status", 0x00, {0x00: "Success", 0x01: "Failure"}), XByteField("FCS", 0x00)]


# need to create the other layers then use this bind_layers to bind them
bind_layers(ZNP, AF_DATA_REQUEST_SREQ, cmd=0x2401)
bind_layers(ZNP, AF_DATA_REQUEST_EXT, cmd=0x2402)
bind_layers(ZNP, AF_DATA_REQUEST_SRSP, cmd=0x6401)
bind_layers(ZNP, AF_DATA_CONFIRM, cmd=0x4480)
bind_layers(ZNP, AF_INCOMING_MSG, cmd=0x4481)
bind_layers(ZNP, ZDO_MGMT_PERMIT_JOIN_REQ_SREQ, cmd=0x2536)

# x1 = ZNP(b'\xfe\x0d\x24\x01\x18\x99\x0b\x01\x06\x00\x04\x00\x1e\x03\x01\x05\x01\x00\x1e\x03\x01\x05\x01\x00\x1e\x03\x01\x05\x01\x00\x1e\x03\x01\x05\x01\xb9')
# hello message
# x2 = ZNP(b'\xfe\x16\x24\x01\x55\xd4\x08\x01\x04\x06\x0a\x00\x1e\x0c\x10\x07\x02\x4d\x00\x42\x05\x68\x65\x6c\x6c\x6f\xdc')
# hello1 message
AF_DATA_REQUEST_SREQ_packet = ZNP(
    b'\xfe\x17\x24\x01\x55\xd4\x08\x01\x04\x06\x13\x00\x1e\x0d\x10\x08\x02\x4d\x00\x42\x06\x68\x65\x6c\x6c\x6f\x31\xf8')
Synchronous_Response_packet = ZNP(b'\xfe\x01\x64\x01\x00\x64')
AF_DATA_CONFIRM_packet = ZNP(b'\xfe\x03\x44\x80\x00\x01\x1a\xdc')
AF_INCOMING_MSG_packet = ZNP(
    b'\xfe\x1b\x44\x81\x00\x00\x06\x00\x55\xd4\x08\x01\x00\x73\x00\xa8\xf4\xa8\x00\x00\x07\x00\x3c\x0a\x00\x00\x20\x00\x55\xd4\x1d\x5a')
fake_packet = ZigbeeNWK(
    b'\x00\x08\x04\x06\x04\x01\x01\x5b\x4d\x00\x42\x06\x68\x65\x6c\x6c\x6f\x31')
# ZCL_WRITE_ATTRIBUTE_packet = ZigbeeClusterLibrary(b'\x10\x03\x02\x4d\x00\x42\x06\x68\x65\x6c\x6c\x6f\x31')
# AF_DATA_REQUEST_packet.show()
# Synchronous_Response_packet.show()
# AF_DATA_CONFIRM_packet.show()
# AF_INCOMING_MSG_packet.show()
# pkt = ZigbeeAppDataPayload(unhexlify('00080406040101251003024d00420668656c6c6f31'))
# serial_packet = ZNP(b'\xfe\x17\x24\x01\x55\xd4\x08\x01\x04\x06\x04\x00\x1e\x0d\x10\x03\x02\x4d\x00\x42\x06\x68\x65\x6c\x6c\x6f\x31\xe4')

# new_pkt = ZigbeeAppDataPayload(frame_control=0,delivery_mode = 0,dst_endpoint=serial_packet.DstEndpoint, src_endpoint=serial_packet.SrcEndpoint, cluster=serial_packet.ClusterId, profile=0x0104,counter=0x25) / ZigbeeClusterLibrary(serial_packet.Data)
# write_hello1_packet.show()
# fake_packet.show()
# ZCL_WRITE_ATTRIBUTE_packet.show()
# new_pkt.show()

write_hello1_packet = ZNP(
    b'\xfe\x16\x24\x01\x55\xd4\x08\x01\x02\x06\x05\x00\x1e\x0c\x10\x04\x02\x4d\x00\x42\x05\x68\x65\x6c\x6c\x6f\xd6\xe4')
write_hello1_packet.Data = ZigbeeClusterLibrary(write_hello1_packet.Data)
write_hello1_packet.show()

ZCL_GREEN_POWER_AF_DATA_EXIT_packet = ZNP(
    b'\xfe\x1a\x24\x02\x02\xfd\xff\x00\x00\x00\x00\x00\x00\xf2\x00\x00\xf2\x21\x00\x0f\x00\x1e\x00\x06\x19\x0b\x02\x0b\xfe\x00\xef')
ZCL_GREEN_POWER_AF_DATA_EXIT_packet.Data = ZigbeeClusterLibrary(
    ZCL_GREEN_POWER_AF_DATA_EXIT_packet.Data)
# ZCL_GREEN_POWER_AF_DATA_EXIT_packet.show()

ZDO_MGMT_PERMIT_JOIN_REQ_SREQ_packet = ZNP(
    b'\xfe\x05\x25\x36\x0f\xfc\xff\xfe\x00\xe4')
ZDO_MGMT_PERMIT_JOIN_REQ_SREQ_packet.show()
# data_ZCL_Green_Power = ZigbeeClusterLibrary(b'\x19\x0b\x02\x0b\xfe\x00')
# data_ZCL_Green_Power.show()
# Step1: Create a algorithm to characterize the packetfrom the serial port
# Step2: as for the AF_DATA_REQUEST packet, the type of the data filed need to be packetField, research on that, then it will directly call ZigbeeClusterLibrary to parse the data field
# Step3: trying to test the algorithm for all AF_DATA_REQUEST packet we can intercept from the serial port
# Step4: extend the decoding for other packet type, need to find the zigbee layer whcih scapy implemented that can decode the data field for the other type of packets
