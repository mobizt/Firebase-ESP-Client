/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2022 mobizt
 *
 */

// This example shows how to send JSON payload FCM to a recipient via legacy API (requires server key).

// This example is for ESP8266 and ENC28J60 Ethernet module.

/**
 *
 * The ENC28J60 Ethernet module and ESP8266 board, SPI port wiring connection.
 *
 * ESP8266 (Wemos D1 Mini or NodeMCU)        ENC28J60
 *
 * GPIO12 (D6) - MISO                        SO
 * GPIO13 (D7) - MOSI                        SI
 * GPIO14 (D5) - SCK                         SCK
 * GPIO16 (D0) - CS                          CS
 * GND                                       GND
 * 3V3                                       VCC
 *
 */

// Library allows your ESP device to interact with FCM server through FCM Server protocols.
// https://firebase.google.com/docs/cloud-messaging/server#choose

// This means your device now is not a FCM app client and unable to get the notification messages.

// The device registration tokens used in this example were taken from the FCM mobile app (Android or iOS)
// or web app that athenticated to your project.

// For FCM client app quick start
// https://github.com/firebase/quickstart-android/tree/master/messaging
// https://github.com/firebase/quickstart-ios
// https://github.com/firebase/quickstart-js

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

#include <ENC28J60lwIP.h>
//#include <W5100lwIP.h>
//#include <W5500lwIP.h>

/** 1 Define the Firebase project Server Key which must be taken from
 * https://console.firebase.google.com/u/0/project/_/settings/cloudmessaging
 *
 * The API, Android, iOS, and browser keys are rejected by FCM
 *
 */
#define FIREBASE_FCM_SERVER_KEY "FIREBASE_PROJECT_CLOUD_MESSAGING_SERVER_KEY"

/* 2. Define the ID token for client or device to send the message */
#define DEVICE_REGISTRATION_ID_TOKEN "DEVICE_TOKEN"

#define ETH_CS_PIN 16 // GPIO 16 connected to Ethernet module (ENC28J60) CS pin

ENC28J60lwIP eth(ETH_CS_PIN);
// Wiznet5100lwIP eth(ETH_CS_PIN);
// Wiznet5500lwIP eth(ETH_CS_PIN);

SPI_ETH_Module spi_ethernet_module;

// Define Firebase Data object
FirebaseData fbdo;

unsigned long lastTime = 0;

void sendMessage();

void setup()
{

    Serial.begin(115200);

    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz?
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    eth.setDefault(); // use ethernet for default route
    if (!eth.begin())
    {
        Serial.println("ethernet hardware not found ... sleeping");
        while (1)
        {
            delay(1000);
        }
    }
    else
    {
        Serial.print("connecting ethernet");
        while (!eth.connected())
        {
            Serial.print(".");
            delay(1000);
        }
    }
    Serial.println();
    Serial.print("ethernet IP address: ");
    Serial.println(eth.localIP());

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    Firebase.reconnectWiFi(true);

    // required for legacy HTTP API
    spi_ethernet_module.enc28j60 = &eth;
    // spi_ethernet_module.w5100 = &eth;
    // spi_ethernet_module.w5500 = &eth;
    Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY, &spi_ethernet_module);

    sendMessage();
}

void loop()
{

    if (millis() - lastTime > 60 * 1000 || lastTime == 0)
    {
        lastTime = millis();
        sendMessage();
    }
}

void sendMessage()
{

    Serial.print("Send Firebase Cloud Messaging... ");

    // Read more details about legacy HTTP API here https://firebase.google.com/docs/cloud-messaging/http-server-ref
    FCM_Legacy_HTTP_Message msg;

    msg.targets.to = DEVICE_REGISTRATION_ID_TOKEN;

    msg.options.time_to_live = "1000";
    msg.options.priority = "high";

    msg.payloads.notification.title = "Notification title";
    msg.payloads.notification.body = "Notification body";
    msg.payloads.notification.icon = "myicon";
    msg.payloads.notification.click_action = "OPEN_ACTIVITY_1";

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson payload;

    // all data key-values should be string
    payload.add("temp", "28");
    payload.add("unit", "celsius");
    payload.add("timestamp", "1609815454");
    msg.payloads.data = payload.raw();

    if (Firebase.FCM.send(&fbdo, &msg)) // send message to recipient
        Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    else
        Serial.println(fbdo.errorReason());
}
