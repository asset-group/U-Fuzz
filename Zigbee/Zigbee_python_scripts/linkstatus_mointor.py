import os
# import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import multiprocessing
from colorama import Fore


def append_to_file(file_name,log):
     fo = open(file_name,'a')
     fo.write(log+"\n")
     fo.close()
def run_server_gdb():
    file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Philp_hue_light/no_ping_for_15.txt"
    file_name_2 = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Philp_hue_light/sniff_tshark.txt"
    # cmd = "sudo whsniff -c 11 | tshark -Y \'zbee_nwk.src == 0x8877\' -r -"
    # print(cmd.split())
    # when there is special characters or multiple commands involved, need to use /bin/bash -c for the process.open to work
#     philp hue
    # cmd_split = ['/bin/bash', '-c', "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/sniffer/whsniff -c 11 | tshark -l -Y \'zbee_nwk.src==0xae8c\' -r -"]
#     smart switch
    # cmd_split = ['/bin/bash', '-c', "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/sniffer/whsniff -c 11 | tshark -l -Y \'zbee_nwk.src==0x8877\' -r -"]
    cmd_split = ['/bin/bash', '-c', "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/sniffer/whsniff -c 11 | tshark -l -Y \'zbee_nwk.src==0x84eb\' -r -"]

    
    p = Popen(cmd_split,shell=False,stdout=PIPE,stderr=PIPE, stdin=PIPE)
    # time.sleep(1)
    start_time = time.time()
    while True:
            output = p.stdout.readline()
            if output:
                received_time = time.time()
                msg = (output).decode('ISO-8859-1')
               #  print(Fore.WHITE,msg, end='')
                append_to_file(file_name_2,msg+"\n")
                if "Link Status" in msg:
                     print(Fore.WHITE,msg, end='')
                     received_time = time.time()
                     time_diff = received_time-start_time
                     start_time = time.time()
                     print(Fore.GREEN,time_diff)
                     if time_diff>20:
                          time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
                          append_to_file(file_name,time_log+"hardware dead +\n"+str(time_diff)+"\n")
                          print(Fore.RED,"Device No Ping for 15s, check whether its alive or not!!!!!!!!!!!!!!!!!!!!")
                
            else:
                 print("no out put")
                 continue
if __name__ == '__main__':
    # f_name_t = create_timestamp_log()
    # print("log file created with name: "+f_name_t+"\n")
    # f_name_o = create_output_log()
    # print("log file created with name: "+f_name_o + "\n")
    time.sleep(0.5)
    # print("??????????????????????/")
    
    # while True:
    p1 = multiprocessing.Process(name='p1', target=run_server_gdb())
    # p2 = multiprocessing.Process(name='p2',target = run_server_gdb(f_name_t,f_name_o))
    p1.start()