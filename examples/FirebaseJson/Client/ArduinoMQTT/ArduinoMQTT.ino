/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt/FirebaseJson
 * 
 * Copyright (c) 2022 mobizt
 *
*/

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>

/* Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

//https://github.com/arduino-libraries/ArduinoMqttClient
#include <ArduinoMqttClient.h>

//Enable Arduino MQTT library after include the library and before include the FirebaseJson.
//If you are using the library that built in the FirebaseJson and get the compilation error,
//move #define FBJS_ENABLE_ARDUINO_MQTT to the top above that library inclusion.
#define FBJS_ENABLE_ARDUINO_MQTT
#include <FirebaseJson.h>

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

unsigned long lastMillis = 0;
int count = 0;

void onMqttMessage(int messageSize)
{
    Serial.print("Got message, topic: ");
    Serial.print(mqttClient.messageTopic());
    Serial.print(", message: ");
    while (mqttClient.available())
    {
        Serial.print((char)mqttClient.read());
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);

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

    Serial.print("Connecting to MQTT broker");
    if (!mqttClient.connect("test.mosquitto.org", 1883))
    {
        Serial.print(", failed! Error code = ");
        Serial.println(mqttClient.connectError());

        while (1)
        {
            delay(0);
        }
    }
    Serial.println(", connected.");

    mqttClient.onMessage(onMqttMessage);

    mqttClient.subscribe("hello");
}

void loop()
{
    mqttClient.poll();
    if (millis() - lastMillis > 1000)
    {
        lastMillis = millis();
        FirebaseJson json;
        json.add("abc", count);
        json.add("def", count % 5 == 0);
        mqttClient.beginMessage("hello");
        json.toString(mqttClient);
        mqttClient.endMessage();
        count++;
    }
}
