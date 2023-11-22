# SmartWeatherStation
##### ESP32 IoT device with MQTT
## Features:
- Room temperature sensor
- Room humidity sensor
- MQTT client
- NTP client

## How it works:
- Sensor reads temperature and humidity and displays it on the screen
- Sensor reads temperature and humidity and publishes it to MQTT broker

## Picture

![SmartWeatherStation](https://i.imgur.com/9JWkqB7.jpeg)


## Circuit

![SmartWeatherStation](https://i.imgur.com/0jDzREd.png)

### MQTT Topics:
- `/home/sensors/temperature` - Temperature and
humidity

### Libraries used:
- PubSubClient
- ArduinoJson
- NTPClient
- DHT sensor library
- Adafruit Unified Sensor
- Adafruit GFX Library

### Hardware used:
- ESP32 DevKit
- DHT Temp sensor
- OLED Screen

###### Developed using PlatformIO

