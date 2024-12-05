/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to subscribe the topic for the IID tokens.

// Library allows your ESP device to interact with FCM server through FCM Server protocols.
// https://firebase.google.com/docs/cloud-messaging/server#choose

// This means your device now is not a FCM app clizent and unable to get the notification messages.

// The device registration tokens used in this example were taken from the FCM mobile app (Android or iOS)
// or web app that athenticated to your project.

// For FCM client app quick start
// https://github.com/firebase/quickstart-android/tree/master/messaging
// https://github.com/firebase/quickstart-ios
// https://github.com/firebase/quickstart-js

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
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

/* 3. Define the instance ID tokens to subscribe the topic */
#define DEVICE_REGISTRATION_ID_TOKEN_1 "DEVICE_TOKEN_1"
#define DEVICE_REGISTRATION_ID_TOKEN_2 "DEVICE_TOKEN_2"

// Define Firebase Data object
FirebaseData fbdo;

unsigned long lastTime = 0;

int count = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void sendMessage();

void setup()
{

    Serial.begin(115200);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    Serial.print("Subscribe the topic... ");

    int numToken = 2;
    const char *iid[2];
    iid[0] = DEVICE_REGISTRATION_ID_TOKEN_1;
    iid[1] = DEVICE_REGISTRATION_ID_TOKEN_2;

    if (Firebase.FCM.subscribeTopic(&fbdo, "testTopic" /* topic to subscribe */, iid /* IID tokens array */, numToken))
        Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    else
        Serial.println(fbdo.errorReason());
}

void loop()
{
}
