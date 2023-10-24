import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import subprocess
import multiprocessing
import socket
# this fucntion will get all the pid of running process then output them as a list, then can use os.system to kill them all
def get_pid(cmd):
    pid_lst = []
    proc = Popen(cmd, stdout=subprocess.PIPE, shell=True)
    # proc = Popen('pgrep mosquitto', stdout=subprocess.PIPE, shell=True)
    (out, err) = proc.communicate()
    out = out.decode('utf-8')
    pid_lst = out.split('\n')
    pid_lst.remove('')
    return pid_lst
def run_client():
    # crash_flag = False
    localIP = "10.13.210.82"
    localPort = 8080
    bufferSize = 1024
    # Create a datagram socket
    UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Bind to address and ip
    UDPServerSocket.bind((localIP, localPort))
    print("-----------------------------------------------------UDP server up and listening------------------------------------------------------------")

    # while(True):
    #     bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
    #     message = bytesAddressPair[0]
    #     address = bytesAddressPair[1]
    #     clientMsg = "Message from Client:{}".format(message)
    #     clientIP  = "Client IP Address:{}".format(address)
    #     if clientMsg:
    #         break
    # cmd2 = "sudo gdb -ex run -ex backtrace --args docker-compose -f /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/coordinator/docker-compose.yml up"
    cmd = "sudo ip netns exec veth5 python3 mqtt_client.py"
    # cmd= "sudo gdb -ex run -ex backtrace --args docker-compose -f /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/coordinator/docker-compose.yml up"
    # cmd = "sudo ip netns exec veth5 python3 replicate_crash_canpous.py"
    p = Popen(cmd.split(),shell=False,stdout=PIPE,stderr=PIPE)
    while True:
        bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
        message = bytesAddressPair[0]
        address = bytesAddressPair[1]
        clientMsg = "Message from Client:{}".format(message)
        print(clientMsg)
        if clientMsg:
            break
    ppid = str(p.pid)
    os.system('sudo kill -9 '+ppid)
    # p.terminate()

run_client()
time.sleep(2)
client_process_lst = get_pid('pgrep python3')
iii = client_process_lst[1]
os.system('sudo kill -9 '+ iii)
time.sleep(3)
server_process_lst = get_pid('pgrep mosquitto')
ii2 = server_process_lst[0]
os.system('sudo kill -9 '+ii2)
