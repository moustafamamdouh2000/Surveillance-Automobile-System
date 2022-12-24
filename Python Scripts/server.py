# Importing libraries
import base64
import cv2 as cv
import numpy as np
import paho.mqtt.client as mqtt

# Broker connection details
MQTT_BROKER = ""
MQTT_PORT = None
MQTT_USER = ""
MQTT_PASS = ""
# Topic from which to receive camera feed
MQTT_RECEIVE = ""

frame = np.zeros((240, 320, 3), np.uint8)


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(MQTT_RECEIVE)


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global frame
    # Decoding the message
    img = base64.b64decode(msg.payload)
    # converting into numpy array from buffer
    npimg = np.frombuffer(img, dtype=np.uint8)
    # Decode to Original Frame
    frame = cv.imdecode(npimg, 1)


# Paho-MQTT Clinet
client = mqtt.Client()
client.username_pw_set(MQTT_USER, MQTT_PASS)
client.on_connect = on_connect
client.on_message = on_message
# Establishing Connection with the Broker
client.connect(MQTT_BROKER, MQTT_PORT)

# Starting thread which will receive the frames
client.loop_start()

while True:
    cv.imshow("Stream", frame)
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# Stop the Thread
client.loop_stop()
