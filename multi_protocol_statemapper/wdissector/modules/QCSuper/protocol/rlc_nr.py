#!/usr/bin/python3
#-*- encoding: utf-8 -*-

# rlc-nr-udp definition:
# - https://github.com/wireshark/wireshark/blob/wireshark-3.6.0/epan/dissectors/packet-gsmtap.h


# rlcMode
RLC_TM_MODE = 1
RLC_UM_MODE = 2
RLC_AM_MODE = 4

# direction 
DIRECTION_UPLINK = 0
DIRECTION_DOWNLINK = 1

# bearerType 
BEARER_TYPE_CCCH = 1
BEARER_TYPE_BCCH_BCH = 2
BEARER_TYPE_PCCH = 3
BEARER_TYPE_SRB = 4
BEARER_TYPE_DRB = 5
BEARER_TYPE_BCCH_DL_SCH = 6

# sequenceNumberLength
TM_SN_LENGTH_0_BITS = 0
UM_SN_LENGTH_6_BITS = 6
UM_SN_LENGTH_12_BITS = 12
AM_SN_LENGTH_12_BITS = 12
AM_SN_LENGTH_18_BITS = 18

# 1 byte */
RLC_NR_DIRECTION_TAG = 0x02

# 2 bytes, network order */
RLC_NR_UEID_TAG = 0x03

# 1 byte */
RLC_NR_BEARER_TYPE_TAG = 0x04

# 1 byte */
RLC_NR_BEARER_ID_TAG = 0x05

# RLC PDU. Following this tag comes the actual RLC PDU (there is no length, the PDU
#   continues until the end of the frame) */
RLC_NR_PAYLOAD_TAG = 0x01

# RLC AM counters
AM_SEQ_NUM = 0
UL_AM_SEQ_NUM = 0
DL_AM_SEQ_NUM = 0

# Reference ipv4 payload with udp magic string (rlc-nr)
pcap_rlc_nr_ip_ref = bytearray(\
    [0x45, 0x0, 0x0, 0x9b, 0x2d, 0x73, 0x40, 0x0, 
    0x40, 0x11, 0x00, 0x00, 0x7f, 0x0, 0x0, 0x1, 
    0x7f, 0x0, 0x0, 0x1, 0xe7, 0xa6, 0x27, 0xf, 
    0x0, 0x87, 0xfe, 0x9a, ord('r'), ord('l'), 
    ord('c'), ord('-'), ord('n'), ord('r')])

def build_rlc_nr_ip(dir, bearer_type, bearer_id, payload):
    global DL_AM_SEQ_NUM, UL_AM_SEQ_NUM
    global AM_SEQ_NUM
    # Create pcap rlc-nr-udp dissector
    # print('bearer type:', rrc_channel)
    # Main rlc dissector meta headers
    seq_number_length = TM_SN_LENGTH_0_BITS 
    rlc_mode = RLC_TM_MODE # Transparent mode

    if (bearer_type == BEARER_TYPE_SRB) or (bearer_type == BEARER_TYPE_DRB):
        # Wireshark cannot directly decode dcch rrc without proper rlc AM headers,
        # so we add a fake rlc + pdcp headers before the actual rrc payload
        rlc_mode = RLC_AM_MODE    # AM
        seq_number_length = AM_SN_LENGTH_12_BITS # always 12 for srb
        # create fake rlc header (2) + pdcp header (2)
        fake_rlc_pdcp_hdrs = b'\xC0\x00'
        # fake_rlc_pdcp_hdrs += AM_SEQ_NUM.to_bytes(2, 'big')
        # AM_SEQ_NUM = (AM_SEQ_NUM + 1) % (2**seq_number_length)
        if dir == DIRECTION_DOWNLINK: # DL sequence number
            fake_rlc_pdcp_hdrs += DL_AM_SEQ_NUM.to_bytes(2, 'big')
            DL_AM_SEQ_NUM = (DL_AM_SEQ_NUM + 1) % (2**seq_number_length)
        else: # UL sequence number
            fake_rlc_pdcp_hdrs += UL_AM_SEQ_NUM.to_bytes(2, 'big')
            UL_AM_SEQ_NUM = (UL_AM_SEQ_NUM + 1) % (2**seq_number_length)

        # add fake rlc header and fake MAC-I bytes
        payload = fake_rlc_pdcp_hdrs + payload + b'\x00\x00\x00\x00'

    if bearer_type > BEARER_TYPE_BCCH_DL_SCH:
        print('Unknown bearer type:', bearer_type)
        return None
        

    # Update reference eth payload with correct lengths for ip and udp
    # so wireshark can correctly decode via rlc-nr-udp dissector
    pcap_buf = pcap_rlc_nr_ip_ref.copy()

    # rlc mode (RLC_TM_MODE)
    pcap_buf += bytearray([rlc_mode])
    # sequence number length
    pcap_buf += bytearray([seq_number_length])
    # RLC_NR_DIRECTION_TAG
    pcap_buf += bytearray([RLC_NR_DIRECTION_TAG, dir])
    # RLC_NR_UEID_TAG
    pcap_buf += bytearray([RLC_NR_UEID_TAG, 0, 0])
    # RLC_NR_BEARER_TYPE_TAG
    pcap_buf += bytearray([RLC_NR_BEARER_TYPE_TAG, bearer_type])
    # RLC_NR_BEARER_ID_TAG
    pcap_buf += bytearray([RLC_NR_BEARER_ID_TAG, bearer_id]) # only relevant for SRB
    # RLC_NR_PAYLOAD_TAG
    pcap_buf += bytearray([RLC_NR_PAYLOAD_TAG])
    pcap_buf += payload

    # Update ip and udp length fields
    # We do this after because the msg length can increase due to fake headers
    rlc_meta_hdrs_length = 12
    udp_len = 8 + len('rlc-nr') + len(payload) + rlc_meta_hdrs_length
    b_udp_len = udp_len.to_bytes(2, byteorder='big')
    ip_len = 20 + udp_len
    b_ip_len = ip_len.to_bytes(2, byteorder='big')
    pcap_buf[2] = b_ip_len[0]
    pcap_buf[3] = b_ip_len[1]
    pcap_buf[24] = b_udp_len[0]
    pcap_buf[25] = b_udp_len[1]

    return bytes(pcap_buf)