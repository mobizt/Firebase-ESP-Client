/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt
 *
 * Copyright (c) 2023 mobizt
 *
 */

// https://github.com/arduino-libraries/ArduinoMqttClient
#include <ArduinoMqttClient.h>

#include <Arduino.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
#include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#endif

// Enable LW MQTT library after include the library and before include the FirebaseJson.

#include <FirebaseJson.h>

/* Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

unsigned long lastMillis = 0;

int count = 0;

const char broker[] = "test.mosquitto.org";
int port = 1883;
const char topic[] = "arduino/simple";

const long interval = 1000;
unsigned long previousMillis = 0;

bool mqttReady = false;

void setup()
{

    Serial.begin(115200);
    Serial.println();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(broker);

    if (!mqttClient.connect(broker, port))
    {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
        return;
    }
    mqttReady = true;

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
}

void loop()
{
    if (!mqttReady)
        return;

    mqttClient.poll();

    if (millis() - lastMillis > 1000)
    {
        lastMillis = millis();

        Serial.print("Sending message to topic: ");

        Serial.println(topic);

        FirebaseJson json;
        json.add("abc", count);
        json.add("def", count % 5 == 0);

        json.toString(Serial);
        Serial.println();

        // send message, the Print interface can be used to set the message contents
        mqttClient.beginMessage(topic);

        json.toString(mqttClient);

        mqttClient.endMessage();
        count++;
    }
}