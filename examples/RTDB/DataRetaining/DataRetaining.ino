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

// This example shows how to retain the past RTDB data within the period from present.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* database secret used in Firebase.RTDB.setQueryIndex function */
#define DATABASE_SECRET "DATABASE_SECRET"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long deleteDataMillis = 0, pushDataMillis = 0;

int count = 0;
bool indexing = false;

void setup()
{

    Serial.begin(115200);
    Serial.println();
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

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // required for large data, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 512 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // NTP time sync
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready())
    {
        if (!indexing)
        {
            indexing = true;
            Serial.print("Set query index in database rules... ");

            // If sign in with OAuth2.0 token, the database secret pass to this function can be empty string
            if (Firebase.RTDB.setQueryIndex(&fbdo, "test/log", "ts", DATABASE_SECRET))
                Serial.println("ok");
            else
                Serial.println(fbdo.errorReason());
        }

        // push data every 30 sec
        if (millis() - pushDataMillis > 30 * 1000)
        {
            pushDataMillis = millis();

            count++;

            FirebaseJson json;

            json.add("ts", (uint32_t)time(nullptr));
            json.add("count", count);

            Serial.print("Push data... ");

            if (Firebase.RTDB.push(&fbdo, "test/log", &json))
                Serial.println("ok");
            else
                Serial.println(fbdo.errorReason());
        }

        // delete old data every 1 min
        if (time(nullptr) > 1618971013 /* timestamp should be valid */ && millis() - deleteDataMillis > 60 * 1000)
        {
            deleteDataMillis = millis();

            Serial.print("Delete history data older than 10 minutes... ");

            if (Firebase.RTDB.deleteNodesByTimestamp(&fbdo, "test/log", "ts", 10 /* delete 10 nodes at once */, 10 * 60 /* retain data within 10 minutes */))
                Serial.println("ok");
            else
                Serial.println(fbdo.errorReason());
        }
    }
}