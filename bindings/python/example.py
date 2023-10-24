#!/usr/bin/env python3

from wdissector import *
import ctypes

wdissector_init("encap:BLUETOOTH_HCI_H4")

print("Version: " + wdissector_version_info().decode())
print("Loaded Profile: " + wdissector_profile_info().decode())

packet_set_direction(1)


pkt = [0x9, 0x76, 0x28, 0x0, 0x0, 0x2b, 0x0, 0x99, 0x1,
       0x4f, 0x0, 0x50, 0xff, 0xff, 0x8f, 0xfe, 0xdb,
       0xff, 0x5b, 0x87, 0x49]

pkt = (ctypes.c_ubyte * len(pkt))(*pkt)

packet_dissect(pkt, len(pkt))

print(packet_summary().decode())
