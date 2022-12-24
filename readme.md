# Surveillance Automobile System 
## Overview
This project uses IOT Protocol MQTT with ESP32 microcontroller to monitor and inspect environments with mobile application

## components used
-  NodeMCU ESP8266 ( wifi embdded microcontroller ), you can use arduino with external wifi module if you want
-  Ultrasonic sensor , LDR , Temperature and Humdity sensor (DHT11)
-  RC car or any body you that moves you can mount the sensors and the microcontroller on it
-  server to deploy the MQTT Broker on it, or you can use third party providers like MQTTX, you can also use any microcontroller that can be used as webserver to deploy the mqtt broker on it and manage the port forwarding part of the network.
## MQTT IOT protocol
MQTT is a communication protocol with features specifically targeted at IoT solutions, it operates a MQTT Broker or web server in simple terms that informs clients/servers that are subscribed to specific topic with the new message provided by the clients/servers publishing to that same topic.
[![N|Solid](https://media.discordapp.net/attachments/804792671356452924/1055977498980122694/Screenshot_1.png)]
## Arduino part
first we use the library `<ESP8266Wifi.h>` to connect the nodemcu with the local wifi network, add your wifi credentials when you call the functions from the library.
second we used normal GPIO pins to connect the sensors including the DHT11 which has a library for nodemcu use `DHTStable.h` , -note: some libraries may not work well with nodemcu as it can work well with arduino-.
third we used the `PubSubClient.h` library to connect the nodemcu with the MQTT Broker
## MQTT part
We used MQTT protocol as its lightweight and easy to implement and have a lot of documentation online,
using the Eclipse Mosquitto docker, we delpoyed the container on raspberry pi webserve,

## Camera feed
we used laptop camera as our feedback for the system ( originally you need a camera mounted on the body ), this is for testing purposes, the python scripts included are used for testing `client.py` is used for publishing the laptop camera feed to the topic `home/camfeed` and the `server.py` is used to publish

## Mobile App
All the sensors and the camera feed are monitored through a mobile appliaction made with flutter framework,
[![N|Solid](https://media.discordapp.net/attachments/818853068018941972/1055915496475721819/image.png?width=377&height=670)


## Notes
you can run the following commands in any linux terminal to test the mqtt broker, may require you to download the `mqtt-clientd` package
##### Subscribe command
mosquitto_sub -h 'mqtt host' -p 'your-port' -u 'your-username' -P 'your password' -t 'your-topic'
##### Publish command
mosquitto_pub -h 'mqtt host' -p 'your-port' -u 'your-username' -P 'your password' -t 'your-topic' -m 'your-message'

Don't forget to add your creds in the following files
#### MQTT_Remote_Sensing_Car:
    #define SSID "your-wifi-network"
    #define SSID_PASSWORD "your-wifi-password"
    #define MQTT_SERVER "your-host"
    #define MQTT_PORT "your-port"
    #define MQTT_NAME "you-mqtt-connection-name" // you can use ESP8266_Sensor
    #define MQTT_USER "your-username"
    #define MQTT_PASS "your-password"
#### Python scripts:
    #MQTT_BROKER = "your-mqttBroker-host" // usually a link , for mqqtx use 'broker.emqx.io' for example
    #MQTT_PORT = "your-mqttBroker-port" // integer, delete ""
    #MQTT_USER = "your-mqttBroker-user"
    #MQTT_PASS = "your-mqttBroker-password"
    #MQTT_SEND = "your-topic" // this is the topic the cam feel will send its data on
    #MQTT_RECEIVE = "your-topic" // this is the topic the client will recieve the camfeed on