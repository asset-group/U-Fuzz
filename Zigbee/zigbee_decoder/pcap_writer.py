import struct
import time
from binascii import unhexlify, hexlify


PCAP_GLOBAL_HEADER_FMT = '@ I H H i I I I '


# Global Header Values
PCAP_MAGICAL_NUMBER = 2712847316
PCAP_MJ_VERN_NUMBER = 2
PCAP_MI_VERN_NUMBER = 4
PCAP_LOCAL_CORECTIN = 0
PCAP_ACCUR_TIMSTAMP = 0
PCAP_MAX_LENGTH_CAP = 65535
PCAP_DATA_LINK_TYPE = 1


class Pcap:

    def __init__(self, filename, link_type=PCAP_DATA_LINK_TYPE):
        self.pcap_file = open(filename, 'wb')
        print("opened file")
        self.pcap_file.write(struct.pack('@ I H H i I I I ',
                                         PCAP_MAGICAL_NUMBER, PCAP_MJ_VERN_NUMBER,
                                         PCAP_MI_VERN_NUMBER, PCAP_LOCAL_CORECTIN,
                                         PCAP_ACCUR_TIMSTAMP, PCAP_MAX_LENGTH_CAP, link_type))

    def writelist(self, data):
        for i in data:
            self.write(i)
        return

    def write(self, data):
        ts_sec, ts_usec = map(int, str(time.time()).split('.'))
        length = len(data)
        self.pcap_file.write(struct.pack('@ I I I I', ts_sec,
                                         ts_usec, length, length))
      #   print(data)
        data = unhexlify(data)
        print(data)
        self.pcap_file.write(data)

    def close(self):
        self.pcap_file.close()


def start(packets,p):
    # p = Pcap("PcapData.pcap")
    for packet in packets:
        p.write(packet)
    p.pcap_file.flush()
    # p.close()
