
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

/* This example shows how to re-authenticate using the refresh token generated from other app via OAuth2.0 authentication. */

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

/* 2. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 5. Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

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

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // To refresh the token 5 minutes before expired
    config.signer.preRefreshSeconds = 5 * 60;

    // Use refresh token and force token to refresh immediately
    // Refresh token, Client ID and Client Secret are required for OAuth2.0 token refreshing.
    // The Client ID and Client  Secret can be taken from https://console.developers.google.com/apis/credentials

    Firebase.setAccessToken(&config, "" /* set access token to empty to refresh token immediately */, 3600 /* expiry time */, "<Refresh Token>" /* refresh token */, "<Client ID>" /* client id */, "<Client Secret>" /* client secret */);
    
    // Or refresh token manually
    // Firebase.refreshToken(&config);

    Firebase.begin(&config, &auth);
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (millis() - dataMillis > 5000 && Firebase.ready())
    {
        dataMillis = millis();
        Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "/test/int", count++) ? "ok" : fbdo.errorReason().c_str());
    }
}
