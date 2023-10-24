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
    localPort = 9000
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
            append_to_file(file_name_o,"[------------------------------Crash------------------------]")
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
     file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/Jcoap/time_stamp_log/time_stamp_log_"+current_time+".txt"

     fo = open(file_name,'w')
     fo.write("Crash Time Stamp \n")
     fo.close()
     return file_name

def create_output_log():
     current_time = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
    # Mosquitto file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/mqtt_log/mosquitto/output_log/output_log_"+current_time+".txt"
    # EMQX file name
     file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/Jcoap/output_log/output_log_"+current_time+".txt"

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
    cmd = "sudo ip netns exec veth5 node client_complete.js"
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
                # if 'DELETE' in msg:
                #      os.killpg(os.getpgid(p.pid), signal.SIGTERM)
            # if client_msg:
            #      break
            # else:
            #      print("something wrong ??????????")

    # ppid = str(p.pid)
    # os.system('sudo kill -9 '+ppid)

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
    cmd = "sudo gdb -ex run -ex backtrace --args sudo /usr/lib/jvm/java-1.11.0-openjdk-amd64/bin/java -javaagent:/home/asset/idea-IC-231.8109.175/lib/idea_rt.jar=43627:/home/asset/idea-IC-231.8109.175/bin -Dfile.encoding=UTF-8 -classpath /home/asset/Desktop/work/jcoap/ws4d-jcoap-examples/target/classes:/root/.m2/repository/org/apache/logging/log4j/log4j-core/2.6.2/log4j-core-2.6.2.jar:/root/.m2/repository/org/apache/logging/log4j/log4j-api/2.6.2/log4j-api-2.6.2.jar:/root/.m2/repository/org/ws4d/jcoap/jcoap-core/1.1.5/jcoap-core-1.1.5.jar org.ws4d.coap.example.basics.Server"
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
    f_name_timestamp = create_timestamp_log()
    print("log file created with name: "+f_name_timestamp+"\n")
    f_name_output = create_output_log()
    print("log file created with name: "+f_name_output + "\n")
    while(True):
        p1 = multiprocessing.Process(name='p1', target=run_server_gdb, args=(f_name_timestamp,f_name_output,))
        # p2 = multiprocessing.Process(name='p2',target=run_client)
        p2 = multiprocessing.Process(name='p2', target=listern_to_fuzzer,args=(f_name_timestamp,f_name_output,))
        p3 = multiprocessing.Process(name='p3',target=run_client,args=(f_name_output,))
        p2.start()
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        time.sleep(1)
        p1.start()
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Coap Server Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",p1.is_alive())
        time.sleep(2)
        p3.start()
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Coap Client Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",p3.is_alive())
        # time.sleep(1)
        while p2.is_alive():
             continue
            #  print("---------------------Server still alive------------------------")
            # if p1.is_alive():
            #      continue
            # else:
            # print("-------------------------------------------P1 Died-------------------------------------------------------------")
            # time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
            # append_to_file(f_name_timestamp,time_log)
            # append_to_file(f_name_output,"[------------------------------ p1 Crash------------------------]")
            # print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
            # break
                 
        # flag 
        # if 
        # p1.join()
        # if p3.is_alive():
        #     print("---------------------------------P3 is still alive------------------------------")
        pid_p3 = p3.pid
            # p3.terminate()
        # os.kill(pid_p3,signal.SIGKILL)
        os.system('sudo kill -9 '+str(pid_p3))
        # else:
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< coap Client Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        # if p1.is_alive(): 
        pid_p1 = p1.pid
            # p1.terminate()
            # os.kill(pid_p1,signal.SIGKILL)
        os.system('sudo kill -9 '+str(pid_p1))
        # else:
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< coap Server Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
             
        # print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        client_pid_lst = get_pid('pgrep node')
        print("this is client pid",client_pid_lst)
        for id in client_pid_lst:
            os.system('sudo kill -9 '+ str(id))
        print("----------------------------------------------killed all node process-----------------------------------")

        # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        # this part is for killing java server
        server_pid_lst = []
        server_pid_lst = get_pid('pgrep java')
        print('This is server pid lst ',str(server_pid_lst))
        if len(server_pid_lst) != 0:
            #  This works for java
             os.system('sudo killall java')
            # for sid in server_pid_lst:
            #     os.system('sudo killall -9 '+str(sid))
        else:
             append_to_file(f_name_output,'----------------------------Server indeed Crashed----------------------------------------------------')
        print("----------------------------------------------killed all java process-------------------------------------")
        # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        # this part is for killing emqx
        # os.system('sudo systemctl stop emqx')
        # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        print('------------------------------------------------------sleeping--------------------------------------------------------------')
        print('------------------------------------------------------stoping--------------------------------------------------------------')
        time.sleep(2)
        # break
        print('------------------------------------------------------RE-STARTED------------------------------------------------------------')