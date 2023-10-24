import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import multiprocessing
import socket


msgFromClient = "Hello UDP Server"
bytesToSend = str.encode(msgFromClient)
# serverAddressPort= ("10.13.210.82", 8080)
serverAddressPort= ("127.0.0.1", 9000)

bufferSize = 1024
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPClientSocket.sendto(bytesToSend, serverAddressPort)
print("----------------Msg to Server Sent-------------")
UDPClientSocket.sendto(bytesToSend, ("127.0.0.1",6666))
print("----------------------Msg Sent to Monitor Sent------------------")