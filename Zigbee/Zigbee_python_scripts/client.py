# import socket
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

def append_to_file(file_name,log):
     fo = open(file_name,'a')
     fo.write(log+"\n")
     fo.close()

def detect_disconnection(f_name_o):
    #  global flag
    #  cmd = "mosquitto"
    #  p = Popen(cmd.split(),shell=False,stdout=PIPE,stderr=PIPE, stdin=PIPE)
    #  time.sleep(1)
     UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

     while True:
        a = os.system("pgrep docker-compose")
        # print(a)
        if a == 256:
            print("Docker Dead!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
            time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
            append_to_file(f_name_o,"Disconnection!!!!"+time_log)
            # Create a UDP socket at client side
            # Send to server using created UDP socket
            UDPClientSocket.sendto(bytesToSend, serverAddressPort)
            print("----------------Msg to Server Sent-------------")
            UDPClientSocket.sendto(bytesToSend, ("127.0.0.1",6666))
            print("----------------------Msg Sent to Monitor Sent------------------")
            time.sleep(30)
        else:
          time.sleep(1)

                    #  flag = True

if __name__ == '__main__':
     file_name_output = '/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Tuya_smartswitch/timeout_smart_switch.txt'
    #  while True:
    #     p = multiprocessing.Process(name='p1',target=detect_disconnection(file_name_output))
    #     p.start()
    #     print("???????")
     detect_disconnection(file_name_output)