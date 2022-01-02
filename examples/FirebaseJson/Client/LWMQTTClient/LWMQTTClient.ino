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

//LW MQTT Client
//https://github.com/256dpi/arduino-mqtt
#include <MQTTClient.h>

//Enable LW MQTT library after include the library and before include the FirebaseJson.
//If you are using the library that built in the FirebaseJson and get the compilation error,
//move #define FBJS_ENABLE_LW_MQTT to the top above that library inclusion.
#define FBJS_ENABLE_LW_MQTT
#include <FirebaseJson.h>

/* Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

WiFiClientSecure ssl;
MQTTClient client;

unsigned long lastMillis = 0;

int count = 0;

void messageReceived(String &topic, String &payload)
{
    Serial.println("Got message, topic: " + topic + ", message: " + payload);
}
void connect()
{
    Serial.print("Connecting to MQTT broker");
    while (!client.connect("arduino", "public", "public"))
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println(", connected.");
    client.subscribe("/hello");
}

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

    ssl.setInsecure();

    client.begin("public.cloud.shiftr.io", 8883, ssl);
    client.onMessage(messageReceived);

    connect();
}

void loop()
{
    client.loop();
    delay(10);

    if (!client.connected())
    {
        connect();
    }

    if (millis() - lastMillis > 1000)
    {
        lastMillis = millis();
        FirebaseJson json;
        json.add("abc", count);
        json.add("def", count % 5 == 0);
        json.toString(client, "/hello");
        count++;
    }
}