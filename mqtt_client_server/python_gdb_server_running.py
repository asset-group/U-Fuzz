import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import multiprocessing
import socket
def run_server_gdb(file_name_t,file_name_o):
    # cmd run mqtt mosquitto server
    # localIP = "10.13.210.82"
    # localPort = 8080
    # bufferSize = 1024
    # # Create a datagram socket
    # UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # # Bind to address and ip
    # UDPServerSocket.bind((localIP, localPort))
    # print("-----------------------------------------------------UDP server up and listening for mosquito------------------------------------------------------------")
    # cmd to start the mosquitto broker
    # cmd = "sudo gdb -ex run -ex backtrace --args docker-compose -f /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/coordinator/docker-compose.yml up"
    cmd = "sudo systemctl start emqx"
    p = Popen(cmd.split(),shell=False,stdout=PIPE,stderr=PIPE, stdin=PIPE)
    time.sleep(1)

    while True:
            output = p.stdout.readline()
            # bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
            # message = bytesAddressPair[0]
            # address = bytesAddressPair[1]
            # clientMsg = "Message from Client:{}".format(message)
            # print(clientMsg)
            # if clientMsg:
            #     break
            if output:
                msg = (output).decode('ISO-8859-1')
                if 'Token' in msg:
                     print("Ignore Token")
                     pass
                else:
                    # print("------------------------------------Server Msg-------------------------------------------------")
                    # print(msg)
                    pass
                append_to_file(file_name_o,("[server]: "+msg+"\n"))
            else:
                 continue