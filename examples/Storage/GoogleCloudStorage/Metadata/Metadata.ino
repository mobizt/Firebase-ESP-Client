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

// This example shows how to get file Metadata in Firebase and Google Cloud Storage bucket via Google Cloud Storage JSON API.
// The Google Cloud Storage JSON API function required OAuth2.0 authen.

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

/* 2. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

/* 3 The following Service Account credentials required for OAuth2.0 authen in Google Cloud Storage JSON API upload */
#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

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

    /* Assign the Service Account credentials for OAuth2.0 authen */
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.print("Get file Metadata with Google Cloud Storage JSON API... ");

        // StorageGetOptions option;
        // For query parameters description of StorageGetOptions, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters

        if (Firebase.GCStorage.getMetadata(&fbdo, STORAGE_BUCKET_ID /* The Firebase or Google Cloud Storage bucket id */, "path/to/file/filename" /* The remote filename stored in the Storage bucket */, nullptr /* StorageGetOptions data */))
        {
            Serial.println("ok");
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
            Serial.printf("Media Link: %s\n", meta.mediaLink.c_str());
            // No download url is available for file uploaded with gcs_upload_type_simple upload.
            Serial.printf("Download URL: %s\n", fbdo.downloadURL().c_str());
        }
        else
            Serial.println(fbdo.errorReason());
    }
}
