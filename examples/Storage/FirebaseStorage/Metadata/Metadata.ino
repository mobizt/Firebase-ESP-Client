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

// This example shows how to get file Metadata in Firebase Storage bucket.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

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

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    // required
    fbdo.setBSSLBufferSize(1024, 1024);
#endif
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.printf("Get file Metadata... %s\n", Firebase.Storage.getMetadata(&fbdo, STORAGE_BUCKET_ID, "test.dat" /* remote file */) ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
        {
            FileMetaInfo meta = fbdo.metaData();
            Serial.printf("Name: %s\n", meta.name.c_str());
            Serial.printf("Bucket: %s\n", meta.bucket.c_str());
            Serial.printf("contentType: %s\n", meta.contentType.c_str());
            Serial.printf("Size: %d\n", meta.size);
            Serial.printf("Generation: %lu\n", meta.generation);
            Serial.printf("Metageneration: %lu\n", meta.metageneration);
            Serial.printf("ETag: %s\n", meta.etag.c_str());
            Serial.printf("CRC32: %s\n", meta.crc32.c_str());
            Serial.printf("Token: %s\n", meta.downloadTokens.c_str());
            Serial.printf("Download URL: %s\n\n", fbdo.downloadURL().c_str());
        }
    }
}
