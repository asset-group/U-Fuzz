import scapy.all as scapy
from binascii import unhexlify
frame = scapy.IP(unhexlify("450000454b8440004011da2e0a2f00010a2a009c9fa016330031f3d24802371adc8b9cdf06e76413b568656c6c6fd103f6d11410ffe6686868686868686868686868686868"))
scapy.send(frame,iface = "wlan0")

