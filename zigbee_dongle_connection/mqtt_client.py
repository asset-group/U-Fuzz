import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("zigbee2mqtt/0xa4c13852c87d504f/temperature")
    client.subscribe("zigbee2mqtt/0xa4c13852c87d504f/humidity")

def on_message(client, userdata, msg):
    if msg.topic == "zigbee2mqtt/0xa4c13852c87d504f/temperature":
        temperature = float(msg.payload)
        print("Temperature:", temperature)
    elif msg.topic == "zigbee2mqtt/0xa4c13852c87d504f/humidity":
        humidity = float(msg.payload)
        print("Humidity:", humidity)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("127.0.0.1", 1883, 60)

client.loop_forever()