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

//This example shows how to remove the topic subscription for the IID tokens.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/** 2 Define the Firebase project Server Key which must be taken from
 * https://console.firebase.google.com/u/0/project/_/settings/cloudmessaging
 * 
 * The API, Android, iOS, and browser keys are rejected by FCM
 * 
 */
#define FIREBASE_FCM_SERVER_KEY "FIREBASE_PROJECT_CLOUD_MESSAGING_SERVER_KEY"

/* 3. Define the instance ID tokens to unsubscribe the topic */
#define DEVICE_REGISTRATION_ID_TOKEN_1 "DEVICE_TOKEN_1"
#define DEVICE_REGISTRATION_ID_TOKEN_2 "DEVICE_TOKEN_2"

//Define Firebase Data object
FirebaseData fbdo;

unsigned long lastTime = 0;

int count = 0;

void sendMessage();

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

    Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);

    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo.setBSSLBufferSize(1024, 1024);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo.setResponseSize(1024);

    Serial.println("------------------------------------");
    Serial.println("Unsubscribe the topic...");

    int numToken = 2;
    const char *iid[2];
    iid[0] = DEVICE_REGISTRATION_ID_TOKEN_1;
    iid[1] = DEVICE_REGISTRATION_ID_TOKEN_2;

    if (Firebase.FCM.unsubscibeTopic(&fbdo, "testTopic" /* topic to subscribe */, iid /* IID tokens array */, numToken))
    {

        Serial.println("PASSED");
        Serial.println(Firebase.FCM.payload(&fbdo));
        Serial.println("------------------------------------");
        Serial.println();
    }
    else
    {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }
}

void loop()
{
}
