import os
# import serial
import time
import signal
from subprocess import Popen, PIPE
import multiprocessing
import socket
def listern_to_fuzzer(file_name_t,file_name_o):
    # crash_flag = False
    localIP = "127.0.0.1"
    localPort = 9000
    # localIP = "10.13.210.82"
    # localPort = 8080
    bufferSize = 1024
    # Create a datagram socket
    UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Bind to address and ip
    UDPServerSocket.bind((localIP, localPort))
    print("-----------------------------------------------------UDP server up and listening------------------------------------------------------------")

    while(True):
        bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
        message = bytesAddressPair[0]
        address = bytesAddressPair[1]
        clientMsg = "Message from Client:{}".format(message)
        clientIP  = "Client IP Address:{}".format(address)
        if clientMsg:
            #  crash_flag = True
            print("-------------------------------------------Fuzzer Signal-------------------------------------------------------------")
            # There is a 8 hours difference with the real time, real time = output+8hours
            time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
            crash_reason = clientMsg.decode('utf-8')
            print(crash_reason)
            append_to_file(file_name_t,time_log)
            append_to_file(file_name_o,"[------------------------------Crash------------------------]")
            print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
            return
        # print(clientMsg)
        # print(clientIP)
    # return crash_flag


def create_timestamp_log():
     current_time = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
    #  print(current_time)
    # CoAPthon file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/CoAPthon/time_stamp_log/time_stamp_log_"+current_time+".txt"
    # canopus file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/canpous/time_stamp_log/time_stamp_log_"+current_time+".txt"
    # Jcoap file name
     file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/Jcoap/time_stamp_log/time_stamp_log_"+current_time+".txt"
     fo = open(file_name,'w')
     fo.write("Crash Time Stamp \n")
     fo.close()
     return file_name

def create_output_log():
     current_time = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
    # CoAPthon file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/CoAPthon/output_log/output_log_"+current_time+".txt"
    # canopus file name
    #  file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/coap_log/canpous/output_log/output_log_"+current_time+".txt"
    # Jcoap file name
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
    cmd = "sudo ip netns exec veth5 node client_complete.js"
    # cmd = "sudo ip netns exec veth5 python3 replicate_crash_canpous.py"
    p = Popen(cmd,shell=True,stdout=PIPE,stderr=PIPE)
    while True:
            output = p.stdout.readline()
            if output == '' and p.poll() is not None:
                break
            if output:
                msg = (output.strip()).decode('ISO-8859-1')
                print(msg)
                append_to_file(file_name_o,("[Client]: "+msg+"\n"))
                # if 'DELETE' in msg:
                #      os.killpg(os.getpgid(p.pid), signal.SIGTERM)

def run_server_gdb(file_name_t,file_name_o):
    # cmd = "sudo ip netns exec veth5 gdb -ex run -ex backtrace --args ~/.platformio/packages/framework-espidf2/examples/protocols/coap_server/managed_components/espressif__coap/libcoap/build/coap-server -d 100 -v 7 -r"
    # cmd run CoAPthon server
    # cmd = "gdb -ex run -ex backtrace --args python2 /home/asset/Desktop/work/CoAPthon/coapserver.py"
    # cmd run canopus server
    # cmd = "gdb -ex run -ex backtrace --args /home/asset/canopus/server"
    # cmd run Jcoap server
    cmd = "sudo gdb -ex run -ex backtrace --args sudo /usr/lib/jvm/java-1.11.0-openjdk-amd64/bin/java -javaagent:/home/asset/idea-IC-231.8109.175/lib/idea_rt.jar=43627:/home/asset/idea-IC-231.8109.175/bin -Dfile.encoding=UTF-8 -classpath /home/asset/Desktop/work/jcoap/ws4d-jcoap-examples/target/classes:/root/.m2/repository/org/apache/logging/log4j/log4j-core/2.6.2/log4j-core-2.6.2.jar:/root/.m2/repository/org/apache/logging/log4j/log4j-api/2.6.2/log4j-api-2.6.2.jar:/root/.m2/repository/org/ws4d/jcoap/jcoap-core/1.1.5/jcoap-core-1.1.5.jar org.ws4d.coap.example.basics.Server"
    p = Popen(cmd,shell=True,stdout=PIPE,stderr=PIPE, stdin=PIPE)
    time.sleep(1)
    # p2 = multiprocessing.Process(name='p2',target=run_client,args=(file_name_o,))
    # p2.start()
    # p3 = multiprocessing.Process(name='p3', target=listern_to_fuzzer)
    # p3.start()

    while True:
            # print("do i reach here ??????????????????????????????????????????????????????????")
            # flag = p3.is_alive()
            # print("This is the flag-------------------------------------",flag)
            # if flag == False:
            #     output = p.stdout.readline()
            #     print((output.strip()).decode('ISO-8859-1'))
            #     p.terminate()
            #     print("p termnated")
            #     p2.terminate()
            #     print("p2 termnated")
            #     append_to_file(file_name_o,"[------------------------------Crash------------------------]")
            #     print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
            #     break
            output = p.stdout.readline()
            # if output == '' and p.poll() is not None:
            #     p.terminate()
            #     print("p termnated")
            #     p2.terminate()
            #     print("p2 termnated")
            #     append_to_file(file_name_o,"[------------------------------Crash------------------------]")
            #     print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
            #     break
            if output:
                # print("have output")
                # msg = (output.strip()).decode('ISO-8859-1')
                msg = (output).decode('ISO-8859-1')
                if 'Token' in msg:
                     print("Ignore Token")
                     pass
                else:
                    print("------------------------------------Server Msg-------------------------------------------------")
                # print("after print msg")
                append_to_file(file_name_o,("[server]: "+msg+"\n"))
                # if 'Interrupt' in msg or 'Segmentation' in msg or 'signal' in msg or 'Traceback' in msg or 'panic' in msg or 'error' in msg or 'Warning' in msg or 'github' in msg:
                # # if '[' in msg or 'Acknowledged' in msg or ']' in msg or 'Copyright' in msg or 'http' in msg or 'WARRANTY' in msg or 'warranty' in msg or 'show configuration' in msg or 'GDB' in msg or 'bug reporting' in msg or 'gdb' in msg or 'For help' in msg or 'Type' in 'msg' or 'Reading symbols' in msg or 'Starting' in msg or 'libthread_db library' in msg or 'free software' in msg or 'GNU' in msg:
                # #     pass
                # # else:
                #     time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
                #     append_to_file(file_name_t,time_log)
                #     print("This is the msg:    ",msg)
                #     time.sleep(1)
                #     # while True:
                #         # print('---------------------------------')
                #         # if p.poll() is not None:
                #         #     break
                #         # output = p.stdout.readline()
                #         # print(output)
                #         # if output:
                #         #     msg = (output.strip()).decode('ISO-8859-1')
                #         #     print(msg)
                #     # p.stdin.write(b'bt\n')
                #     # grep_stdout = p.communicate()[0]
                #     # print(grep_stdout.decode())
                #     # p.stdin.close()
                #     # print('INFO Sent')
                #     # p.send_signal(signal.SIGQUIT)
                #     output = p.stdout.readline()
                #     print((output.strip()).decode('ISO-8859-1'))
                #     p.terminate()
                #     print("p termnated")
                #     p2.terminate()
                #     print("p2 termnated")
                #     append_to_file(file_name_o,"[------------------------------Crash------------------------]")
                #     print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
                #     # need to break the while loop
                #     break
            else:
                 print("========================================================no reply?===========================================================")
                 break

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
        p1.start()
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Coap Server Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        p3.start()
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Coap Client Started >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        while p2.is_alive():
            #  print("---------------------Server still alive------------------------")
            if p1.is_alive():
                 continue
            else:
                print("-------------------------------------------P1 Died-------------------------------------------------------------")
                time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
                append_to_file(f_name_timestamp,time_log)
                append_to_file(f_name_output,"[------------------------------ p1 Crash------------------------]")
                print('---------------------------------------------------------RE-STARTING--------------------------------------------------------------')
                break
                 
        # flag 
        # if 
        # p1.join()
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        pid_p3 = p3.pid
        # p3.terminate()
        os.kill(pid_p3,signal.SIGKILL)
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Coap Client Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        pid_p1 = p1.pid
        # p1.terminate()
        os.kill(pid_p1,signal.SIGKILL)
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP Server Terminated >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        print('------------------------------------------------------sleeping--------------------------------------------------------------')
        time.sleep(2)
        print('------------------------------------------------------RE-STARTED------------------------------------------------------------')
    # create_timestamp_log()
    # append_to_file()
    
    
