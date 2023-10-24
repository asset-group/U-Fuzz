import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import subprocess
import multiprocessing
import socket

print(len([1,1]))
for i in range(2):
    print(i)

# def get_pid():
#     pid_lst = []
#     # proc = Popen('pgrep python3', stdout=subprocess.PIPE, shell=True)
#     proc = Popen('pgrep mosquitto', stdout=subprocess.PIPE, shell=True)
#     (out, err) = proc.communicate()
#     out = out.decode('utf-8')
#     pid_lst.append(out)
#     print(out)
#     # print("program output:", out)
#     return pid_lst

# process_lst = get_pid()
# for iid in process_lst:
#     print(iid)
#     os.system('sudo kill -9 '+ iid)