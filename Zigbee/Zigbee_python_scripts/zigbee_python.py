import serial
import time
import os

SOF = b'\xfe'
DstAddr_1 = b'\x77'
DstAddr_2 = b'\x88'
DstEndpoint = b'\x01'
SrcEndpoint = b'\x01'
ClusterId_1 = b'\x06'
ClusterId_2 = b'\x00'
TransId = b'\x22'
Options = b'\x00'
Radius = b'\x1e'

TimeOut = 0.1
def byte_xor(ba1, ba2):
    return bytes([_a ^ _b for _a, _b in zip(ba1, ba2)])


def computeFCS(data):
    FCS = b'\x00'
    for d in data[1:]:  # See Z-stack API 2.2.1 (XOR over all but SOF)
        FCS = byte_xor(FCS, d)
    return FCS


def sendFrame(f):
    for s in f:
        ser.write(s)
    print(["0x" + i.hex() for i in f])
    time.sleep(TimeOut)

class MT_AF_DATA_REQUEST:

    def __init__(self, payload, payloadLen):
        # Returns a list of bytes that can be interpreted as an AF_DATA_REQUEST
        CMD0 = b'\x24'
        CMD1 = b'\x01'

        header = [DstAddr_1, DstAddr_2, DstEndpoint, SrcEndpoint, ClusterId_1,
                  ClusterId_2, TransId, Options, Radius, payloadLen.to_bytes(1, 'big')]
        headerLen = len(header + payload)
        self.rawbytes = [SOF, headerLen.to_bytes(1, 'big'), CMD0, CMD1] + header + payload
        self.rawbytes.append(computeFCS(self.rawbytes))
if __name__ == '__main__':
    ser = serial.Serial('/dev/pts/11', 921600)
    # sampledata = [b'\x01', b'\x00', b'\x02']
    # # joinFrame = [b'\xff', DstAddr_1, DstAddr_2, b'\x10', b'\x10']
    # frame = MT_AF_DATA_REQUEST(sampledata, len(sampledata))

    # sendFrame(createGeneralFrame(joinFrame, b'\x25', b'\x36'))
    # sendFrame(frame.rawbytes)
    # print(frame.rawbytes)

    bytelist_1 = [SOF, b'\x11', b'\x24', b'\x01', b'\x77', b'\x88', b'\x01', b'\x01', b'\x06', b'\x00', b'\x22', b'\x00',
                b'\x1e', b'\xd4', b'\x10', b'\x1d', b'\x02', b'\x00', b'\x80', b'\x10', b'\x01', b'\x68']
    # bytelist_1 += [computeFCS(bytelist_1)]
    bytelist_2 = [b'\xfd', b'\x0f', b'\x24', b'\x01', b'\x77', b'\x88', b'\x01', b'\x01', b'\x06', b'\x00', b'\x50', b'\x00',
                b'\x1e', b'\x05', b'\x18', b'\x5e', b'\x0b', b'\x0a', b'\x00', b'\xdf']
    bytelist_3 = [b'\xfe', b'\x0f', b'\x24', b'\x01', b'\x77', b'\x88', b'\x01', b'\x01', b'\x06', b'\x00', b'\x59', b'\x00',
                b'\x1e', b'\x01', b'\x18', b'\x69', b'\x0b', b'\x0a', b'\x00', b'\xe1']
    bytelist_4 = [b'\xfe', b'\x0d', b'\x24', b'\x01', b'\x77', b'\x88', b'\x01', b'\x01', b'\x06', b'\x00', b'\x6d', b'\x00',
                b'\x27', b'\x03', b'\x01', b'\x56', b'\x00', b'\xf6']
    bytelist_5 = [b'\xee', b'\x11', b'\x24', b'\x01', b'\x6d', b'\x88', b'\x01', b'\x02', b'\x06', b'\x00', b'\x74', b'\x00',
                b'\x1e', b'\x07', b'\x10', b'\x5b', b'\x02', b'\x00', b'\x80', b'\x10', b'\x00', b'\x79']
    # bytelist_1 += [computeFCS(bytelist_1)]
    i=100
    while i>0:
        sendFrame(bytelist_1)
        print(bytelist_1)
        sendFrame(bytelist_2)
        print(bytelist_2)
        sendFrame(bytelist_3)
        print(bytelist_3)
        sendFrame(bytelist_4)
        print(bytelist_4)
        time.sleep(2)
        i = i-1
