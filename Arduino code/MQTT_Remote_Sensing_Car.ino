// Base ESP8266
#include <ESP8266WiFi.h>
// MQTT lib
#include <PubSubClient.h>
// dht sensor compatible with Nodemcu
#include "DHTStable.h"

// defined constants

// Debug flags
#define VERBOSE 1
// SSID and its password
#define SSID ""
#define SSID_PASSWORD ""
// MQTT Server and its related constants
#define MQTT_SERVER ""
#define MQTT_PORT 0000
#define MQTT_NAME "ESP8266_Sensor"
#define MQTT_USER ""
#define MQTT_PASS ""
// DHT constants
#define DHT11_PIN D6
#define DHT_TYPE DHT11
#define DHT_INTERVAL 5000 // milliseconds
long unsigned dht_timer;
// LDR constants
#define LDR_PIN A0
#define LDR_RES 330
#define LDR_INTERVAL 1000 // milliseconds
long unsigned ldr_timer;
#define LUX_SKIP 0
int lux_skip_count = 3;
// LUX Threshold
#define LUX_THRESH 10

// MQTT TOPICS
#define TEMP_SENSOR "Sensors/Temp"
#define HUMIDITY_SENSOR "Sensors/Humidity"
#define LUMINOSITY_SENSOR "Sensors/Light"
// Car movement Topic
#define CAR_MOVEMENT_CONTROL "Car/Movement"

// L298 HBridge Ports
#define LEFT_WHEELS_FORWARD_PIN D2
#define LEFT_WHEELS_BACKWARD_PIN D3

#define RIGHT_WHEELS_FORWARD_PIN D4
#define RIGHT_WHEELS_BACKWARD_PIN D5

#define SPEED_CTRL_EN1 D0
#define SPEED_CTRL_EN2 D1

// char buffer for publishing data
char charBuff[128];

// Class initialization
WiFiClient WIFI_CLIENT;
PubSubClient MQTT_CLIENT;
DHTStable DHT;

// previous readings
float prev_temp, prev_hum, prev_lux;

// function definition
void reconnect();
// movement related functions.
void forward();
void backward();
void left_turn();
void right_turn();
void turn_off();
void speed_control(int speed);
void callback(char *topic, byte *payload, unsigned int length);

void setup()
{
  // intialize serial
  Serial.begin(115200);

  // timers initialization
  dht_timer = millis() + DHT_INTERVAL;
  ldr_timer = millis() + LDR_INTERVAL;
  // LDR pin mode
  pinMode(LDR_PIN, INPUT);
  // read temp and humidity
  DHT.read11(DHT11_PIN);

  // L298 HBridge pin definitions
  pinMode(LEFT_WHEELS_FORWARD_PIN, OUTPUT);
  digitalWrite(LEFT_WHEELS_FORWARD_PIN, LOW);

  pinMode(LEFT_WHEELS_BACKWARD_PIN, OUTPUT);
  digitalWrite(LEFT_WHEELS_BACKWARD_PIN, LOW);

  pinMode(RIGHT_WHEELS_BACKWARD_PIN, OUTPUT);
  digitalWrite(RIGHT_WHEELS_BACKWARD_PIN, LOW);

  pinMode(RIGHT_WHEELS_FORWARD_PIN, OUTPUT);
  digitalWrite(RIGHT_WHEELS_FORWARD_PIN, LOW);

  pinMode(SPEED_CTRL_EN1, OUTPUT);
  analogWrite(SPEED_CTRL_EN1, 255);

  pinMode(SPEED_CTRL_EN2, OUTPUT);
  analogWrite(SPEED_CTRL_EN2, 255);

  // wifi connection
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    if (VERBOSE)
      Serial.print(".");
  }
  if (VERBOSE)
  {
    Serial.println("Connected to Wi-Fi");
    Serial.print("Using Ip Address: ");
    Serial.println(WiFi.localIP());
  }

  // predefined callback function for car movement control
  MQTT_CLIENT.setCallback(callback);
  // initialize dummy data of previous readings
  prev_temp = 0;
  prev_hum = 0;
  prev_lux = 0;
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!MQTT_CLIENT.connected())
    reconnect();
  // READ AND PUBLISH DHT11 DATA
  dht_publish();
  // READ AND PUBLISH LDR DATA
  lux_publish();
  // movement control loop
  MQTT_CLIENT.loop();
}
void lux_publish()
{
  if (millis() > ldr_timer)
  {
    float lux = analogRead(LDR_PIN);
    (String(lux)).toCharArray(charBuff, 128);
    if ((lux <= prev_lux - LUX_THRESH || lux >= prev_lux + LUX_THRESH))
    {
      if (VERBOSE)
      {
        Serial.print("LDR Reading: ");
        Serial.println(lux);
      }
      MQTT_CLIENT.publish(LUMINOSITY_SENSOR, charBuff);
      // lux_skip_count = 3;
    }
    // lux_skip_count--;
    ldr_timer = millis() + LDR_INTERVAL;
    prev_lux = lux;
  }
}
void dht_publish()
{
  if (millis() > dht_timer)
  {
    int chk = DHT.read11(DHT11_PIN);
    if (VERBOSE)
    {
      Serial.print("DHT11, \t");
      switch (chk)
      {
      case DHTLIB_OK:
        Serial.print("OK,\n");
        break;
      case DHTLIB_ERROR_CHECKSUM:
        Serial.print("Checksum error,\n");
        break;
      case DHTLIB_ERROR_TIMEOUT:
        Serial.print("Time out error,\n");
        break;
      default:
        Serial.print("Unknown error,\n");
        break;
      }
    }
    // DISPLAY DATA
    (String(DHT.getTemperature())).toCharArray(charBuff, 128);
    if (prev_temp != DHT.getTemperature())
      MQTT_CLIENT.publish(TEMP_SENSOR, charBuff);
    (String(DHT.getHumidity())).toCharArray(charBuff, 128);
    if (prev_hum != DHT.getHumidity())
      MQTT_CLIENT.publish(HUMIDITY_SENSOR, charBuff);
    if (VERBOSE)
    {
      if (prev_temp != DHT.getTemperature())
      {
        Serial.print("\nTemp: ");
        Serial.println(DHT.getTemperature(), 1);
      }
      // Serial.print(",\t");
      if (prev_hum != DHT.getHumidity())
      {
        Serial.print("\nHumidity: ");
        Serial.println(DHT.getHumidity(), 1);
      }
    }
    dht_timer = millis() + DHT_INTERVAL;
    prev_temp = DHT.getTemperature();
    prev_hum = DHT.getHumidity();
  }
}
// This function connects to the MQTT broker
void reconnect()
{
  // Set our MQTT broker address and port
  MQTT_CLIENT.setServer(MQTT_SERVER, MQTT_PORT);
  MQTT_CLIENT.setClient(WIFI_CLIENT);
  // Loop until we're reconnected
  while (!MQTT_CLIENT.connected())
  {
    // Attempt to connect
    Serial.println("Attempt to connect to MQTT broker");
    MQTT_CLIENT.connect(MQTT_NAME, MQTT_USER, MQTT_PASS);

    // Wait some time to space out connection requests
    delay(3000);
  }

  MQTT_CLIENT.subscribe(CAR_MOVEMENT_CONTROL);
  Serial.println("MQTT connected");
}
// car movement
void forward()
{
  // turning of backwards
  digitalWrite(LEFT_WHEELS_BACKWARD_PIN, LOW);
  digitalWrite(RIGHT_WHEELS_BACKWARD_PIN, LOW);
  // turning on forwards
  digitalWrite(LEFT_WHEELS_FORWARD_PIN, HIGH);
  digitalWrite(RIGHT_WHEELS_FORWARD_PIN, HIGH);
}

void backward()
{
  // turning of forwards
  digitalWrite(LEFT_WHEELS_FORWARD_PIN, LOW);
  digitalWrite(RIGHT_WHEELS_FORWARD_PIN, LOW);
  // turning on backwards
  digitalWrite(LEFT_WHEELS_BACKWARD_PIN, HIGH);
  digitalWrite(RIGHT_WHEELS_BACKWARD_PIN, HIGH);
}
void right_turn()
{
  /*
   * making the sides (right wheels and left wheels) rotate in opposite direction of each other
   * in this case -> right wheels goes backward and left wheels rotates forward
   */
  digitalWrite(LEFT_WHEELS_BACKWARD_PIN, LOW);
  digitalWrite(RIGHT_WHEELS_FORWARD_PIN, LOW);
  // turning on forwards
  digitalWrite(LEFT_WHEELS_FORWARD_PIN, HIGH);
  digitalWrite(RIGHT_WHEELS_BACKWARD_PIN, HIGH);
}

void left_turn()
{
  /*
   * making the sides (right wheels and left wheels) rotate in opposite direction of each other
   * in this case -> right wheels goes forawrd and left wheels rotates bakwards
   */
  digitalWrite(LEFT_WHEELS_FORWARD_PIN, LOW);
  digitalWrite(RIGHT_WHEELS_BACKWARD_PIN, LOW);

  digitalWrite(LEFT_WHEELS_BACKWARD_PIN, HIGH);
  digitalWrite(RIGHT_WHEELS_FORWARD_PIN, HIGH);
}

void turn_off()
{
  digitalWrite(LEFT_WHEELS_FORWARD_PIN, LOW);
  digitalWrite(RIGHT_WHEELS_BACKWARD_PIN, LOW);
  digitalWrite(LEFT_WHEELS_BACKWARD_PIN, LOW);
  digitalWrite(RIGHT_WHEELS_FORWARD_PIN, LOW);
}
void speed_control(int speed)
{
  if (VERBOSE)
  {
    Serial.print("Current Mode: ");
    Serial.println(speed);
  }
  analogWrite(SPEED_CTRL_EN1, (speed));
  analogWrite(SPEED_CTRL_EN2, (speed));
}
void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = '\0';
  String value = String((char *)payload);

  if (VERBOSE)
  {
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Recieved String: ");
    Serial.println(value);
  }
  for (auto ch : value)
  {
    switch (ch)
    {
    case 'w':
    {
      if (VERBOSE)
        Serial.println("Moving Forward");
      forward();
      delay(25);
      continue;
    }
    case 's':
    {
      if (VERBOSE)
        Serial.println("Moving Down");
      backward();
      delay(25);
      continue;
    }
    case 'd':
    {
      if (VERBOSE)
        Serial.println("Moving Right");
      right_turn();
      delay(25);
      continue;
    }
    case 'a':
    {
      if (VERBOSE)
        Serial.println("Moving Left");
      left_turn();
      delay(25);
      continue;
    }
    case 'x':
    {
      switch (value[1])
      {
      case '1':
        speed_control(128);
        break;
      case '2':
        speed_control(192);
        break;
      case '3':
        speed_control(224);
        break;
      case '4':
        speed_control(255);
        break;
      default:
        break;
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  turn_off();
}