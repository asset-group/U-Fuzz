import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import multiprocessing
import socket
def get_pid(cmd):
    pid_lst = []
    proc = Popen(cmd, stdout=PIPE, shell=True)
    # proc = Popen('pgrep mosquitto', stdout=subprocess.PIPE, shell=True)
    (out, err) = proc.communicate()
    out = out.decode('utf-8')
    pid_lst = out.split('\n')
    pid_lst.remove('')
    return pid_lst
def listern_to_fuzzer(file_name_t,file_name_o):
    # crash_flag = False
    # localIP = "10.13.210.82"
    localIP = "127.0.0.1"
    # localPort = 8080
    localPort = 6666
    bufferSize = 1024
    # Create a datagram socket
    UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Bind to address and ip
    UDPServerSocket.bind((localIP, localPort))
    print("-----------------------------------------------------UDP server up and listening for fuzzer------------------------------------------------------------")

    while(True):
        bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
        message = bytesAddressPair[0]
        address = bytesAddressPair[1]
        clientMsg = "Message from Client:{}".format(message)
        clientIP  = "Client IP Address:{}".format(address)
        if clientMsg:
            #  crash_flag = True
            print("-------------------------------------------Fuzzer Signal-------------------------------------------------------------")
            time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
            crash_reason = message.decode('utf-8')
            crash_reason2 = '---------------------------------------------- '+ crash_reason + ' ---------------------------------------------'
            print(message.decode('utf-8'))
            append_to_file(file_name_t,time_log)
            append_to_file(file_name_t,crash_reason2)
            append_to_file(file_name_o,"[------------------------------Crash------------------------]")
            append_to_file(file_name_o,time_log)
            append_to_file(file_name_o,crash_reason2)
            print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
            return clientMsg
        # print(clientMsg)
        # print(clientIP)
    # return crash_flag


def create_timestamp_log():
     current_time = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
    # Mosquitto file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/mqtt_log/mosquitto/time_stamp_log/time_stamp_log_"+current_time+".txt"
    # EMQX file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Philp_hue_light/Time_stamp_log/time_stamp_log_"+current_time+".txt"
     file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Tuya_smartswitch/Time_stamp_log/time_stamp_log_"+current_time+".txt"
    
    # hivemq-ce file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/mqtt_log/hivemq-ce/time_stamp_log/time_stamp_log_"+current_time+".txt"

     fo = open(file_name,'w')
     fo.write("Crash Time Stamp \n")
     fo.close()
     return file_name

def create_output_log():
     current_time = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
    # Mosquitto file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/mqtt_log/mosquitto/output_log/output_log_"+current_time+".txt"
    # EMQX file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Philp_hue_light/output_log/output_log_"+current_time+".txt"
     file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Tuya_smartswitch/output_log/output_log_"+current_time+".txt"
    
    # hivemq-ce file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/mqtt_log/hivemq-ce/output_log/output_log_"+current_time+".txt"

     fo = open(file_name,'w')
     fo.write("Log session starts \n")
     fo.close()
     return file_name
# right now able to run and stop both the server and the client

def append_to_file(file_name,log):
     fo = open(file_name,'a')
     fo.write(log+"\n")
     fo.close()
def run_client(file_name_o):
    # localIP = "10.13.210.82"
    # localPort = 8080
    # bufferSize = 1024
    # # Create a datagram socket
    # UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # # Bind to address and ip
    # UDPServerSocket.bind((localIP, localPort))
    # print("-----------------------------------------------------UDP server up and listening for mqtt client------------------------------------------------------------")
    cmd = "sudo ip netns exec veth5 python3 mqtt_client.py"
    # cmd = "sudo ip netns exec veth5 python3 replicate_crash_canpous.py"
    p = Popen(cmd.split(),shell=False,stdout=PIPE,stderr=PIPE)
    print("client running? ")
    while True:
            output = p.stdout.readline()
            # bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
            # message = bytesAddressPair[0]
            # address = bytesAddressPair[1]
            # clientMsg = "Message from Client:{}".format(message)
            # print(clientMsg)
            # if clientMsg:
            #     break
            if output == '' and p.poll() is not None:
                break
            if output:
                msg = (output.strip()).decode('ISO-8859-1')
                # print(msg)
                append_to_file(file_name_o,("[Client]: "+msg+"\n"))
                if 'disconnection' in msg:
                     time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
                     append_to_file(file_name_o,("[Unexpected Disconnection!!!! Server real Crash!!!!!]"+ '\n' +time_log+'\n'))
                    # print("------------------------------Unexpected Disconnection----------------------------------")
                    # msgFromClient       = "Unexpected Disconnectio"
                    # bytesToSend         = str.encode(msgFromClient)
                    # serverAddressPort = ("127.0.0.1", 9000)
                    # # bufferSize = 1024
                    # UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
                    # UDPClientSocket.sendto(bytesToSend, serverAddressPort)
                # if 'DELETE' in msg:
                #      os.killpg(os.getpgid(p.pid), signal.SIGTERM)
            # if client_msg:
            #      break
            # else:
            #      print("something wrong ??????????")

    # ppid = str(p.pid)
    # os.system('sudo kill -9 '+ppid)

def run_server_gdb(f_name_t,f_name_o):
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
    cmd = "sudo docker-compose -f /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/coordinator/docker-compose.yml up"
    # cmd = "sudo gdb -ex run -ex backtrace --args docker-compose -f /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/zigbee_dongle_connection/coordinator/docker-compose.yml up"
    # cmd to start the emqx broker
    # cmd = "sudo systemctl start emqx"
    # cmd to start the hivemq-ce broker
    # cmd = "sudo /home/asset/Desktop/work/HiveMQ_Community_Edition/hivemq-ce-2023.3/bin/run.sh"
    p = Popen(cmd.split(),shell=False,stdout=PIPE,stderr=PIPE, stdin=PIPE)
    time.sleep(1)
    # seting log level to debug for emqx
    # os.system("sudo emqx_ctl log set-level debug")
    # time.sleep(1)

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
                print(msg)
                append_to_file(f_name_o,msg)
                # if 'stopping' in msg or 'Timeout._onTimeout'in msg or 'Error: Failed to connect to the adapter' in msg:
                #      print("Docker died")
                #      print(msg)
                #      time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
                #      os.system("sudo killall docker-compose")
                #      append_to_file(f_name_o,("[Docker-compose Crashed \n] at "+time_log+'\n'))
                #      break
                # elif 'to Tuya_Smart_plug failed:' in msg:
                #      time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
                #      print('Device hang!!!!!!!!!')
                #      append_to_file(f_name_o,"Device Hang!!"+time_log)
                # else:
                #     # print("------------------------------------Server Msg-------------------------------------------------")
                #     print(msg)
                #     append_to_file(f_name_o,(msg+'\n'))
                #     pass
                # append_to_file(file_name_o,("[server]: "+msg+"\n"))
            else:
                 continue
                #  print("========================================================no reply?===========================================================")
                #  break

    # ppid = str(p.pid)
    # os.system('sudo kill -9 '+ppid)

    # need to handle the braek of the while loop issue
    # p.terminate()
    # print("p termnated due to server not replied")
    # p2.terminate()
    # print("p2 termnated due to server not replied")
    # append_to_file(file_name_o,"[------------------------------Crash server not replied------------------------]")
    # print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
    
if __name__ == '__main__':
    f_name_t = create_timestamp_log()
    print("log file created with name: "+f_name_t+"\n")
    f_name_o = create_output_log()
    print("log file created with name: "+f_name_o + "\n")
    time.sleep(0.5)
    # print("??????????????????????/")
    
    while True:
        p1 = multiprocessing.Process(name='p1', target=listern_to_fuzzer,args=(f_name_t,f_name_o,))
        # p2 = multiprocessing.Process(name='p2',target = run_server_gdb(f_name_t,f_name_o))
        p1.start()
        print("Server running")
        # time.sleep(10)
        # p2.start()
        p2 = multiprocessing.Process(name='p2',target = run_server_gdb,args=(f_name_t,f_name_o,))
        p2.start()
        print("Docker Started")
        while p1.is_alive():
            #  print("Alive")
             continue
        time.sleep(0.5)
        print("serverworks!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        pid_p2 = p2.pid
        os.system("sudo kill -9 "+str(pid_p2))
        print("killed p2? ",p2.is_alive())
        # p2.kill()
        # time.sleep(3)
        # print("reach_here??????????????????????????????????????????????")
        os.system("sudo killall docker-compose")
        time.sleep(1)
    #  while True:
    #       time.sleep(1)
    #       print(p.is_alive())
    # f_name_timestamp = create_timestamp_log()
    # print("log file created with name: "+f_name_timestamp+"\n")
    # f_name_output = create_output_log()
    # print("log file created with name: "+f_name_output + "\n")
    # time.sleep(0.5)
    # test_pid_list = get_pid('pgrep python3')

    # while(True):
    #     print('This is main function id list: ',test_pid_list)
    #     p1 = multiprocessing.Process(name='p1', target=run_server_gdb, args=(f_name_timestamp,f_name_output,))
    #     # p2 = multiprocessing.Process(name='p2',target=run_client)
    #     p2 = multiprocessing.Process(name='p2', target=listern_to_fuzzer,args=(f_name_timestamp,f_name_output,))
    #     p3 = multiprocessing.Process(name='p3',target=run_client,args=(f_name_output,))
    #     # p2.start()
    #     # print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    #     time.sleep(0.5)
    #     p1.start()
    #     print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Mqtt Server Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",p1.is_alive())
    #     time.sleep(3)
    #     p3.start()
    #     print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Mqtt Client Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",p3.is_alive())
    #     p2.start()
    #     print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    #     time.sleep(0.5)
    #     # time.sleep(1)
    #     while p2.is_alive():
    #          continue
    #         #  print("---------------------Server still alive------------------------")
    #         # if p1.is_alive():
    #         #      continue
    #         # else:
    #         # print("-------------------------------------------P1 Died-------------------------------------------------------------")
    #         # time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
    #         # append_to_file(f_name_timestamp,time_log)
    #         # append_to_file(f_name_output,"[------------------------------ p1 Crash------------------------]")
    #         # print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
    #         # break
                 
    #     # flag 
    #     # if 
    #     # p1.join()
    #     # if p3.is_alive():
    #     #     print("---------------------------------P3 is still alive------------------------------")
    #     pid_p3 = p3.pid
    #         # p3.terminate()
    #     # os.kill(pid_p3,signal.SIGKILL)
    #     os.system('sudo kill -9 '+str(pid_p3))
    #     # else:
    #     print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< mqtt Client Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    #     # if p1.is_alive(): 
    #     pid_p1 = p1.pid
    #         # p1.terminate()
    #         # os.kill(pid_p1,signal.SIGKILL)
    #     os.system('sudo kill -9 '+str(pid_p1))
    #     # else:
    #     print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< mqtt Server Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
             
    #     # print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    #     # os.system('sudo killall python3')
    #     client_pid_lst = get_pid('pgrep python3')
    #     print("this is client pid",client_pid_lst)
    #     # for i in range(len(client_pid_lst)-1):
    #     # print('--------------------------------killed the udp server to prevent conflict ------------------------------------')
    #     # os.system('sudo kill -9 '+str(client_pid_lst[0]))
    #     print("-----------------------------------------killed mqtt client-------------------------------------------------")
    #     for ppid in client_pid_lst:
    #          if ppid not in test_pid_list:
    #             print(ppid)
    #             os.system('sudo kill -9 '+ str(ppid))

    #     # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    #     # this part is for killing mosquitto server
    #     # server_pid_lst = []
    #     # server_pid_lst = get_pid('pgrep mosquitto')
    #     # print('This is server pid lst ',server_pid_lst)
    #     # if len(server_pid_lst) != 0:
    #     #     os.system('sudo kill -9 '+str(server_pid_lst[0]))
    #     # else:
    #     #      append_to_file(f_name_output,'----------------------------Server indeed Crashed----------------------------------------------------')
    #     # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    #     # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    #     # this part is for killing emqx
    #     time.sleep(0.5)
    #     os.system('sudo systemctl stop emqx')
    #     # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    #     # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    #     # this part is for killing hivemq-ce server
    #     # time.sleep(0.5)
    #     # server_pid_lst = []
    #     # server_pid_lst = get_pid('pgrep java')
    #     # print('This is server pid lst ',server_pid_lst)
    #     # # if len(server_pid_lst) != 0:
    #     # os.system('sudo kill -9 '+str(server_pid_lst[0]))
    #     # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    #     print('------------------------------------------------------sleeping--------------------------------------------------------------')
    #     print('------------------------------------------------------stoping--------------------------------------------------------------')
    #     time.sleep(2)
    #     # break
    #     print('------------------------------------------------------RE-STARTED------------------------------------------------------------')