# Importing Libraries
import cv2 as cv
import paho.mqtt.client as mqtt
import base64
import time

# Broker connection details
MQTT_BROKER = ""
MQTT_PORT = None
MQTT_USER = ""
MQTT_PASS = ""
# Topic on which frame will be published
MQTT_SEND = ""
# Camera index
camIdx = 0

# Object to capture the frames
cap = cv.VideoCapture(camIdx)

cap.set(cv.CAP_PROP_BUFFERSIZE, 5)
# Paho-MQTT Clinet
client = mqtt.Client()
client.username_pw_set(MQTT_USER, MQTT_PASS)
# Establishing Connection with the Broker
client.connect(MQTT_BROKER, MQTT_PORT)
try:
    while True:
        start = time.time()
        # Read Frame
        _, frame = cap.read()
        # Encoding the Frame
        _, buffer = cv.imencode('.jpeg', frame)
        # Converting into encoded bytes
        jpg_as_text = base64.b64encode(buffer)
        print(jpg_as_text)
        # Publishig the Frame on the Topic home/server
        client.publish(MQTT_SEND, jpg_as_text)
        end = time.time()
        t = end - start
        fps = 1/t
        print(fps)
        time.sleep(1.5)

except:
    cap.release()
    client.disconnect()
    print("\nNow you can restart fresh")
