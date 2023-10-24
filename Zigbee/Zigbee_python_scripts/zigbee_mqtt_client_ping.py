import paho.mqtt.client as mqtt
import struct
import string
import random
import time
import colorama
import socket
import os
import json
from colorama import Fore
from datetime import date

# today = str(date.today())
start_time = time.time()
file_name = "/home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/Zigbee/zigbee_log/Philp_hue_light/timeout_smart_light.txt"
def subscribe_function(client):
     topic_lst = ['zigbee2mqtt/philp_hue_light']
    #  qos_lst = [0,1,2]
    #  topic = topic_lst[random.randint(0,2)]
    #  qos = qos_lst[random.randint(0,2)]
     for topic in topic_lst:
        client.subscribe(topic,0)
        print(Fore.RED+ "client subscibr topic : " + topic +" with qos: 0")


def append_to_file(file_name,log):
     fo = open(file_name,'a')
     fo.write(log+"\n")
     fo.close()    
         
         
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    # print("Connected with result code "+str(rc))
    global flag_connected
    flag_connected = 1

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # time_start = start
    global start_time
    time_end = time.time()
    print(Fore.RED,time_end-start_time)
    if int(time_end-start_time)>15:
         print(Fore.GREEN,time_end-start_time)
        #  time_log = (time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()))+"\n"
        # #  append_to_file(file_name,str(time_log)+"looks like a hang \n")
        #  print(time_log)
         print(Fore.GREEN,"Hange!!!!!!!!!!!!!!!!!!")
    start_time = time.time()
    # print(start_time)
    print(msg.topic+" "+str(msg.payload))

# append_to_file(file_name,today)
client = mqtt.Client(transport='tcp')
client.reinitialise()
client.username_pw_set('admin','admin')
# client.username_pw_set('flashmq')

# may need bind the client to 10.0.47.1 which is the veth5 ip
# test with local
# client.connect("0.0.0.0", 1883, 60)
# run with fuzzer

client.on_connect = on_connect
client.on_message = on_message
# client.on_disconnect = on_disconnect
client.connect("127.0.0.1", 1883, 60)
# client.connect("localhost", 8883, 60)
subscribe_function(client)
# print("hello")
# client.loop_start()
client.loop_forever()
# subscribe_function(client)
# while 1:
    #  publish_function(client)
    #  on_message(client)
    #  time.sleep(1)
    # msg_num = random.randint(0,10)
    # # if msg_num==0:
    # #     publish_function(client)
    # #     time.sleep(1)
    # if msg_num ==1:
    #     subscribe_function(client)
    #     time.sleep(3)
    # elif msg_num == 2:
    #     unsubscribe_function(client)
    #     time.sleep(3)
    # else:
    #     publish_function(client)
    #     time.sleep(3)
        # print("-----------------------------------------Something Wrong--------------------------------------------------")

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
# client.loop_forever()

# can change the payload type, length, qos,topic