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

// This example shows how to upload file to Firebase storage bucket via the Google Cloud Storage JSON API.
// The file media.mp4 in the data folder should be uploaded to the device flash memory before test.
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

/* 3 The following Service Account credentials required for OAuth2.0 authen in Google Cloud Storage JSON API upload */
#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

// The Google Cloud Storage upload callback function
void gcsUploadCallback(UploadStatusInfo info);

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

    /* Assign upload buffer size in byte */
    // Data to be uploaded will send as multiple chunks with this size, to compromise between speed and memory used for buffering.
    // The memory from external SRAM/PSRAM will not use in the TCP client internal tx buffer.
    config.gcs.upload_buffer_size = 2048;

#if defined(ESP8266)
    // required for large file data, increase Tx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // if use SD card, mount it.
    SD_Card_Mounting(); // See src/addons/SDHelper.h
}

// The Google Cloud Storage upload callback function
void gcsUploadCallback(UploadStatusInfo info)
{
    if (info.status == fb_esp_gcs_upload_status_init)
    {
        Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
    }
    else if (info.status == fb_esp_gcs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == fb_esp_gcs_upload_status_complete)
    {
        Serial.println("Upload completed\n");
        FileMetaInfo meta = fbdo.metaData();
        Serial.printf("Name: %s\n", meta.name.c_str());
        Serial.printf("Bucket: %s\n", meta.bucket.c_str());
        Serial.printf("contentType: %s\n", meta.contentType.c_str());
        Serial.printf("Size: %d\n", meta.size);
        Serial.printf("Generation: %lu\n", meta.generation);
        Serial.printf("ETag: %s\n", meta.etag.c_str());
        Serial.printf("CRC32: %s\n", meta.crc32.c_str());
        Serial.printf("Token: %s\n", meta.downloadTokens.c_str());       // only gcs_upload_type_multipart and gcs_upload_type_resumable upload types.
        Serial.printf("Download URL: %s\n", fbdo.downloadURL().c_str()); // only gcs_upload_type_multipart and gcs_upload_type_resumable upload types.
    }
    else if (info.status == fb_esp_gcs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.println("\nUpload file via Google Cloud Storage JSON API...\n");

        /**
         * The following function uses Google Cloud Storage JSON API to upload the file (object).
         * The Google Cloud Storage functions required OAuth2.0 authentication.
         * The upload types of methods can be selectable.
         *
         * The gcs_upload_type_simple upload type is used for small file upload in a single request without metadata.
         * gcs_upload_type_multipart upload type is for small file upload in a single reques with metadata.
         * gcs_upload_type_resumable upload type is for medium or large file (larger than or equal to 256 256 KiB) upload with metadata and can be resumable.
         *
         * The upload with metadata supports allows the library to add the metadata internally for Firebase to request the download access token in Firebase Storage bucket.
         * User also can add custom metadata for the uploading file (object).
         */

        // For query parameters description of UploadOptions, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
        // For request payload properties description of Requestproperties, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
        // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
        Firebase.GCStorage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase or Google Cloud Storage bucket id */, "/media.mp4" /* path to local file */, mem_storage_type_sd /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, gcs_upload_type_resumable /* upload type */, "media.mp4" /* path of remote file stored in the bucket */, "video/mp4" /* mime type */, nullptr /* UploadOptions data */, nullptr /* Requestproperties data */, nullptr /* UploadStatusInfo data to get the status */, gcsUploadCallback /* callback function */);
    }
}
