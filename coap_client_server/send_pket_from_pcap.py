from scapy.all import *
from scapy.utils import rdpcap

pkts = rdpcap("/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/06_04_2023coap_crash_ref.pcapng")
pkts = pkts[57650:57665]
for idx, pkt in enumerate(pkts):
    if IP in pkt:
        if pkt[IP].src == '10.47.0.1':
            print("Packet: ",idx)
            pkt[Ether].src ='00:D4:9E:83:48:D9'
            pkt[Ether].dst = 'a8:03:2a:eb:f5:20'
            # pkt[IP].src = "10.42.0.100"
            del pkt[IP].chksum
            del pkt[UDP].chksum
            # pkt.show2()
            # pkt[Ether].dst = getmacbyip("10.42.0.232")
            srp(pkt)
            # ip_pkt = pkt[IP]
            # print(ip_pkt.summary())
            # sr1(ip_pkt, timeout=0.3)
            # sendp(pkt)