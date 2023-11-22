#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

// Temp sensor config
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char *ssid = "INEA-73EF_2.4G";
const char *password = "8689450570";
const char *mqtt_server_ip = "192.168.1.138";
const int mqtt_server_port = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Screen config
#define OLED_SDA 18
#define OLED_SCL 19

Adafruit_SH1106 display(18, 19);

void turnOnBlueLed();
void turnOffBlueLed();

void reconnectToMqtt();

void configure();
void configureWiFi();
void configureMqtt();

float getHumidity();
float getTemperature();

void sendSensorsDataToServer();

void updateScreenWithSensorsData();
void callback(char *topic, byte *message, unsigned int length);

// Assuming a 16x16 monochrome bitmap for the sun
void setup()
{
  Serial.begin(115200);
  delay(100);
  pinMode(2, OUTPUT);  // Initialize GPIO2 pin as an output
  pinMode(21, OUTPUT); // Initialize GPIO2 pin as an output
  configure();
  dht.begin();
  timeClient.begin();
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop()
{
  if (!mqttClient.connected())
  {
    reconnectToMqtt();
  }
  timeClient.forceUpdate();
  updateScreenWithSensorsData();
  sendSensorsDataToServer();
  mqttClient.loop();
}

void turnOnBlueLed()
{
  digitalWrite(2, HIGH); // Turn the LED off by making the voltage HIGH
}

void turnOffBlueLed()
{
  digitalWrite(2, LOW);
}

void configure()
{
  turnOnBlueLed();

  configureWiFi();
  configureMqtt();

  turnOffBlueLed();
}

void configureWiFi()
{

  WiFi.mode(WIFI_STA); // Optional
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to wifi.");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void configureMqtt()
{
  mqttClient.setServer(mqtt_server_ip, mqtt_server_port);
  mqttClient.setCallback(callback);

  reconnectToMqtt();
}

void reconnectToMqtt()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {

    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("Successfully connected to MQTT server");
      mqttClient.subscribe("/home/api/temp");
    }
    else
    {
      Serial.println("failed, rc=");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}

float getTemperature()
{
  float temperature = dht.readTemperature();
  return temperature;
}

float getHumidity()
{
  float humidity = dht.readHumidity();
  return humidity;
}

void updateScreenWithSensorsData()
{
  display.clearDisplay();
  display.setCursor(0, 0);

  display.setTextColor(BLACK, WHITE);

  display.print("Inside\n");
  display.print("\n");

  display.setTextColor(WHITE);

  display.print("Temp -> ");
  display.print(getTemperature());
  display.print("C \n");

  display.print("Hum -> ");
  display.print(getHumidity());
  display.print("% \n");

  display.print("\n");

  display.setTextColor(BLACK, WHITE);

  display.print("Outside\n");
  display.print("\n");

  display.setTextColor(WHITE);

  display.print("Temp -> ");
  display.print(15);
  display.print("C \n");

  display.display();
  delay(100);
}

void sendSensorsDataToServer()
{
  // Serial.println("Reporting sensors data");
  StaticJsonDocument<120> json;
  char sensorsOutput[120];
  json["temperature"] = getTemperature();
  json["humidity"] = getHumidity();
  json["timeStamp"] = timeClient.getEpochTime();
  serializeJson(json, sensorsOutput);
  mqttClient.publish("/home/sensors/temperature", sensorsOutput);
}

void callback(char *topic, byte *message, unsigned int length)
{
  const size_t capacity = JSON_OBJECT_SIZE(10);

  DynamicJsonDocument parsed(capacity);
  DeserializationError error = deserializeJson(parsed, message);

  if (error)
  {
    Serial.println("Failed to parse JSON");
    Serial.println(error.c_str());
    return;
  }

  if (String(topic) == "/home/api/temp")
  {
    if (boolean(parsed["on"]))
    {
      digitalWrite(21, HIGH);
    }
    else if (!boolean(parsed["on"]))
    {
      digitalWrite(21, LOW);
    }
  }
}