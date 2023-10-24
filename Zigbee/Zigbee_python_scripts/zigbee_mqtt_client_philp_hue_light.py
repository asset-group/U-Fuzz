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
import random

def subscribe_function(client):
     topic_lst = ['zigbee2mqtt/#','zigbee2mqtt/philp_hue_light','zigbee2mqtt/philp_hue_light/availability']
    #  qos_lst = [0,1,2]
    #  topic = topic_lst[random.randint(0,2)]
    #  qos = qos_lst[random.randint(0,2)]
     for topic in topic_lst:
        client.subscribe(topic,0)
        print(Fore.RED+ "client subscibr topic : " + topic +" with qos: 0")


def unsubscribe_function(client):
     topic_lst = ['test/hello','test/asset','test']
     topic = topic_lst[random.randint(0,2)]
     client.unsubscribe(topic)
     print(Fore.GREEN+"client unsubscirbe topic: "+topic)
     
         
def publish_function(client):
    topic = 'zigbee2mqtt/philp_hue_light/set'
    dic_payload_on = {"state":"ON"}
    dic_payload_off = {"state":"OFF"}
    dic_payload_toggle = {"state":"TOGGLE"}
    dic_payload_color_tem_num = gen_color_tem_num_payload()
    dic_payload_color_tem_str = gen_color_tem_str_payload()
    dic_payload_brightness = gen_brightness_payload()
    dic_color_rgb = gen_color_rgb_payload()
    dic_effect = gen_effect_payload()

    payload_on = json.dumps(dic_payload_on)
    payload_off = json.dumps(dic_payload_off)
    payload_toggle = json.dumps(dic_payload_toggle)
    payload_color_tem_num = json.dumps(dic_payload_color_tem_num)
    payload_color_tem_str = json.dumps(dic_payload_color_tem_str)
    payload_brightness = json.dumps(dic_payload_brightness)
    payload_color_rgb = json.dumps(dic_color_rgb)
    payload_effect = json.dumps(dic_effect)
    client.publish(topic,payload_on,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Switch on")
    time.sleep(1.5)
    client.publish(topic,payload_color_tem_num,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Changed color tem with rgb num")
    time.sleep(1.5)
    client.publish(topic,payload_color_tem_str,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Changed color tem with rgb str")
    time.sleep(0.5)
    client.publish(topic,payload_color_rgb,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Changed color with rgb")
    time.sleep(0.5)
    client.publish(topic,payload_brightness,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Changed brightness")
    time.sleep(3)
    client.publish(topic,payload_effect,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Changed effect")
    time.sleep(3)
    client.publish(topic,payload_off,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Switch off")
    time.sleep(1)
    client.publish(topic,payload_toggle,0,retain=False)
    print(Fore.LIGHTGREEN_EX,"Switch TOGGLE")
    time.sleep(3)
    # client.publish(topic,payload_child_unlock,0,retain=False)
    # print("Child lock unlock")
    # time.sleep(3)
    # client.publish(topic,payload_child_lock,0,retain=False)
    # print("Child lock lock")
    # time.sleep(3)
    # print(Fore.BLUE+"published message with topic: "+ topic + " payload" +payload_off)
    # time.sleep(1)

def gen_color_tem_num_payload():
    #  color_temp_lst = ["coolest","cool", "neutral", "warm", "warmest"]
     n = random.randint(100,500)
     payload = {"color_temp":str(n)}
     print(Fore.BLUE,payload)
     return payload

def gen_color_tem_str_payload():
     color_temp_lst = ["coolest","cool", "neutral", "warm", "warmest"]
     n = random.randint(0,4)
    #  v = 
     payload = {"color_temp":color_temp_lst[n]}
     print(Fore.LIGHTBLACK_EX,payload)
     return payload      
         
def gen_color_rgb_payload():
     rr = random.randint(0,255)
     gg = random.randint(0,255)
     bb = random.randint(0,255)
     payload = {"color":{"r":rr,"g":gg,"b":bb}}
     print(Fore.RED,payload)
     return payload

def gen_brightness_payload():
     brit = random.randint(0,254)
     payload = {"brightness":brit}
     print(Fore.LIGHTYELLOW_EX,payload)
     return payload

def gen_effect_payload():
     effect_lst = ["blink", "breathe", "okay", "channel_change", "candle", "fireplace", "colorloop", "finish_effect", "stop_effect", "stop_hue_effect"]
     lengthh = len(effect_lst)-1
     n = random.randint(0,lengthh)
     payload = {"effect":effect_lst[n]}
     print(Fore.MAGENTA,payload)
     return payload
        
# this function will output string with random length N
def randStr(chars = string.ascii_uppercase + string.digits, N=10):
	return ''.join(random.choice(chars) for _ in range(N))
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    # print("Connected with result code "+str(rc))
    global flag_connected
    flag_connected = 1


    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("test/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))


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
# client.loop_start()
# client.loop_forever()
# subscribe_function(client)
while 1:
     publish_function(client)
    #  on_message(client)
     time.sleep(1)

client.loop_forever()

# can change the payload type, length, qos,topic