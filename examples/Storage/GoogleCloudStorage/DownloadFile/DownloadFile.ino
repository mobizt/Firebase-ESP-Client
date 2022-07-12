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

// This example shows how to download file from Firebase and Google Cloud Storage bucket via Google Cloud Storage JSON API.
// The Google Cloud Storage JSON API function required OAuth2.0 authen.

// If SD Card used for storage, assign SD card type and FS used in src/FirebaseFS.h and
// change the config for that card interfaces in src/addons/SDHelper.h

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the SD card interfaces setting and mounting
#include <addons/SDHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

/* 3 The following Service Account credentials required for OAuth2.0 authen in Google Cloud Storage JSON API download */
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

#if defined(ESP8266)
    // required for large file data, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    /* Assign download buffer size in byte */
    // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
    // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
    config.gcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // if use SD card, mount it.
    SD_Card_Mounting(); // See src/addons/SDHelper.h
}

// The Google Cloud Storage download callback function
void gcsDownloadCallback(DownloadStatusInfo info)
{
    if (info.status == fb_esp_gcs_download_status_init)
    {
        Serial.printf("Downloading file %s (%d) to %s\n", info.remoteFileName.c_str(), info.fileSize, info.localFileName.c_str());
    }
    else if (info.status == fb_esp_gcs_download_status_download)
    {
        Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == fb_esp_gcs_download_status_complete)
    {
        Serial.println("Download completed\n");
    }
    else if (info.status == fb_esp_gcs_download_status_error)
    {
        Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
    }
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.println("\nDownload file with Google Cloud Storage JSON API...\n");

        // StorageGetOptions option;
        // For query parameters description of StorageGetOptions, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters
        // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
        if (!Firebase.GCStorage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase or Google Cloud Storage bucket id */, "path/to/file/filename" /* path of remote file stored in the bucket */, "/path/to/save/filename" /* path to local file */, mem_storage_type_sd /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, nullptr /* StorageGetOptions data */, gcsDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
    }
}
