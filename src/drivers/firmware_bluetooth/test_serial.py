#!/usr/bin/env python3
import serial
import termios
import fcntl
from threading import Thread
from time import sleep
import struct

ser = serial.Serial("/dev/ttyUSB1", 4000000, rtscts=0,
                    dsrdtr=0, timeout=20)


def set_dtr_rts(ser, dtr, rts):
    # Linux only for now
    import termios
    import fcntl
    fd = ser.fileno()

    TIOCMGET = getattr(termios, 'TIOCMGET', 0x5415)
    TIOCMSET = getattr(termios, 'TIOCMSET', 0x5418)
    TIOCM_DTR = getattr(termios, 'TIOCM_DTR', 0x002)
    TIOCM_RTS = getattr(termios, 'TIOCM_RTS', 0x004)
    TIOCM_zero_str = struct.pack('I', 0)

    TIOCM = fcntl.ioctl(fd, TIOCMGET, TIOCM_zero_str)[0]
    TIOCM &= ~(TIOCM_DTR | TIOCM_RTS)
    if dtr:
        TIOCM |= TIOCM_DTR
    if rts:
        TIOCM |= TIOCM_RTS

    TIOCM_str = struct.pack('I', TIOCM)
    fcntl.ioctl(fd, TIOCMSET, TIOCM_str)


def send_msg():
    while True:
        ser.write(b'\xEE')
        print('0xEE sent')
        sleep(1)


Thread(target=send_msg, daemon=True).start()

try:
    while True:
        m = ser.readline()
        print(m)
        if m and len(str(m).split('.')) == 3:
            print('SUCCESS: Version detected, firmware OK!')
        else:
            print('ERROR: Something went wrong, no firmware version detected!')
except:
    print('\n')
