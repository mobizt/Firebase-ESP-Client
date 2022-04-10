/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt
 *
 * Copyright (c) 2021 mobizt
 *
 */

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

// This example is for ESP8266 and ESP32

#include <WiFiClientSecure.h>

#include <FirebaseJson.h>

/* Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

unsigned long ms = 0;
int count = 0;

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
}

void loop()
{
    if (millis() - ms > 15000 || ms == 0)
    {
        ms = millis();

        FirebaseJson json;
        FirebaseJsonData result;

        WiFiClientSecure sslClient;
        sslClient.setInsecure(); // skip cert verification

        json.add("name", "esp");
        json.set("data/arr/[0]", count + 1);
        json.set("data/arr/[1]", count + 10);
        json.set("data/arr/[2]", count + 100);

        Serial.print("Connecting to server...");

        if (sslClient.connect("reqres.in", 443))
        {
            Serial.println(" ok");
            Serial.println("Send POST request...");
            sslClient.print("POST /api/users HTTP/1.1\n");
            sslClient.print("Host: reqres.in\n");
            sslClient.print("Content-Type: application/json\n");
            sslClient.print("Content-Length: ");
            sslClient.print(json.serializedBufferLength());
            sslClient.print("\n\n");
            json.toString(sslClient);

            Serial.print("Read response...");

            // Automatically parsing for response (w or w/o header) with chunk encoding supported.
            if (json.readFrom(sslClient))
            {
                Serial.println();
                json.toString(Serial, true);
                Serial.println("\n\nComplete");
            }
            else
                Serial.println(" failed\n");
        }
        else
            Serial.println(" failed\n");

        sslClient.stop();

        count++;
    }
}