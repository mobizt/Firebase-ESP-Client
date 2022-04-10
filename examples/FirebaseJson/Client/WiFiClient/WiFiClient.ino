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

#include <WiFiClient.h>

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

        WiFiClient client;

        json.add("name", "esp");
        json.set("data/arr/[0]", count + 1);
        json.set("data/arr/[1]", count + 10);
        json.set("data/arr/[2]", count + 100);

        Serial.print("Connecting to server...");

        if (client.connect("httpbin.org", 80))
        {
            Serial.println(" ok");
            Serial.println("Send POST request...");
            client.print("POST /anything HTTP/1.1\n");
            client.print("Host: httpbin.org\n");
            client.print("Content-Type: application/json\n");
            client.print("Connection: close\n");
            client.print("Content-Length: ");
            client.print(json.serializedBufferLength(true));
            client.print("\n\n");
            json.toString(client, true);

            Serial.print("Read response...");

            // Automatically parsing for response (w or w/o header) with chunk encoding supported.
            if (json.readFrom(client))
            {
                Serial.println();
                json.toString(Serial, true);
                Serial.println("\n\nComplete");
            }
            else
                Serial_Printf(" failed with http code: %d\n", json.responseCode());
        }
        else
            Serial.println(" failed\n");

        client.stop();

        count++;
    }
}