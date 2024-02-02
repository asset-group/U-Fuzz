import scapy.all as scapy
import time
from binascii import unhexlify

packet_lst_crash_canpous_1 = ["ecf1f8d5476becf1f8d5476b08004500004e044b400040114fc50a2f00010a0dd2528ba11633003a5c8a2a0144ed91762ca65e84760cb568656c6c6f8f000000ff0a20474554203333362072657175657374207061796c6f61643a20"]
packet_lst_crash_canpous_2 = ["ecf1f8d5476becf1f8d5476b08004500004500504000401153c90a2f00010a0dd2528ba1163300318c9ade0244e891762ca1dfea0f3e9d6261736963d10308621410ff68686868686868686868686868686868"]
packet_lst_crash_canpous_3 = ["ecf1f8d5476becf1f8d5476b08004500004f019e4000401152710a2f00010a0dd2528ba11633003bf717ed0267ea91762ca382b19578b568656c6c6fff6868686868686868686868686868686868686868686868686868686868686868"]
packet_lst_crash_canpous_4 = ["ecf1f8d5476becf1f8d5476b080045000045f5b4400040115e640a2f00010a0dd2528ba1163300319a0a48ad44df91762c985da87d17c268656c6c6f9d0308451410ff68686868686868686868686868686868"]
def verify_lst(lst):
    count = 2
    for idx, pkt in enumerate(lst):
        # time.sleep(3)
        print("Packet: ", idx)
        while(count > 1):
            frame = scapy.Ether(unhexlify(pkt))
            frame[scapy.Ether].src ='00:00:00:00:00:00'
            frame[scapy.Ether].dst = '00:00:00:00:00:00'
            # pkt[IP].src = "10.42.0.100"
            frame[scapy.IP].src = '10.13.210.82'
            frame[scapy.IP].dst = '10.13.210.82'
            
            del frame[scapy.IP].chksum
            del frame[scapy.UDP].chksum
            scapy.sendp(frame,iface = 'lo')
            time.sleep(0.5)
            count = count - 1
            print(count)
            # time.sleep(1)
        count = 2
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
def verify(idx):
    count = 10
    while(count>1): 
        frame = scapy.IP(unhexlify(packet_lst_232[idx]))
        # scapy.send(frame,iface = "wlan0")
        # test on fuzzer
        scapy.send(frame,iface = "veth5")
        count = count - 1
        print(count)

def send(pkt):
    count = 100
    while(count>1): 
        # frame = scapy.EthernetII
        frame = scapy.IP(unhexlify(pkt))
        # frame[scapy.IP].dst = '10.42.0.232'
        frame[scapy.IP].chksum = None
        x = frame[scapy.UDP]
        # seems like we need to manually set the chksum to null then update it 
        x[scapy.UDP].chksum = None
        # x.sport = 34599
        # x.dport = 5683
        d = scapy.IP(src='10.42.0.100', dst='10.47.0.1') / scapy.UDP(scapy.raw(x))
        # d = scapy.IP(src='10.42.0.100', dst='10.42.0.232') / scapy.UDP(scapy.raw(x))
        # frame = scapy.IP(frame)
        scapy.send(d)
        count = count - 1
        print(count)
def test_send(pkt):
    count = 10
    while(count>1): 
        frame = scapy.IP(unhexlify(pkt))
        scapy.send(frame,iface = "wlan0")
        count = count - 1
        print(count)
# print(len(paket_lst_156))

# while(count > 1):
#     # time.sleep(1)
#     for idx, pkt in enumerate(packet_lst_crash2):
#         frame = scapy.IP(unhexlify(pkt))
#         scapy.send(frame,iface = "wlan0")
#     count = count -1
#     print(count)


# test replicatble crashed packets to the board
count = 1
while(count>0):
    verify_lst_server(test_lst)
    count = count -1
# count = 1
# while(count>0):
#     verify_lst_server(packet_lst_crash3_verify)
#     count = count -1
