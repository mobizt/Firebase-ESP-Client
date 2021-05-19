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

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

uint32_t idleTimeForStream = 15000;

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

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    //Or use legacy authenticate method
    //config.database_url = DATABASE_URL;
    //config.signer.tokens.legacy_token = "<database secret>";

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    //The data under the node being stream (parent path) should keep small
    //Large stream payload leads to the parsing error due to memory allocation.
    if (!Firebase.RTDB.beginStream(&fbdo, "/test/stream/data"))
        Serial.printf("sream begin error, %s\n\n", fbdo.errorReason().c_str());
}

void loop()
{

    if (Firebase.ready() && (millis() - sendDataPrevMillis > idleTimeForStream || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        count++;
        String Value = "Hello World! " + String(count);
        Serial.printf("Set string... %s\n\n", Firebase.RTDB.setString(&fbdo, "/test/stream/data", Value) ? "ok" : fbdo.errorReason().c_str());
    }

    if (Firebase.ready())
    {

        if (!Firebase.RTDB.readStream(&fbdo))
            Serial.printf("sream read error, %s\n\n", fbdo.errorReason().c_str());

        if (fbdo.streamTimeout())
            Serial.println("stream timeout, resuming...\n");

        if (fbdo.streamAvailable())
        {
            Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\nvalue, %s\n\n",
                          fbdo.streamPath().c_str(),
                          fbdo.dataPath().c_str(),
                          fbdo.dataType().c_str(),
                          fbdo.eventType().c_str(),
                          fbdo.stringData().c_str());
        }
    }
}