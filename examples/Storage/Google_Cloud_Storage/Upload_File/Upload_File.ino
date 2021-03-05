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

//This example shows how to upload file to Firebase storage bucket via the Google Cloud Storage JSON API.
//The file media.mp4 in the data folder should be uploaded to the device flash memory before test.
//The Google Cloud Storage JSON API function required OAuth2.0 authen.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name (required) */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"

/* 3. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

/* 4 The following Service Account credentials required for OAuth2.0 authen in Google Cloud Storage JSON API upload */
#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

String path = "/Test";

//The Google Cloud Storage upload callback function
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

    /* Assign the project host (required) */
    config.host = FIREBASE_HOST;

    /* Assign the Service Account credentials for OAuth2.0 authen */
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo.setBSSLBufferSize(1024, 1024);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo.setResponseSize(1024);

    Serial.println("------------------------------------");
    Serial.println("Upload file via Google Cloud Storage JSON API test...");

    /**
     * The following function uses Google Cloud Storage JSON API to upload the file (object).
     * 
     * The Google Cloud Storage functions required OAuth2.0 authentication.
     * The upload types of methods can be selectable.
     * The gcs_upload_type_simple upload type is used for small file upload in a single request without metadata.
     * gcs_upload_type_multipart upload type is for small file upload in a single reques with metadata.
     * gcs_upload_type_resumable upload type is for medium or large file (larger than or equal to 256 256 KiB) upload with metadata and can be resumable.
     * 
     * The upload with metadata supports allows the library to add the metadata internally for Firebase to request the download access token in Firebase Storage bucket.
     * User also can add custom metadata for the uploading file (object).
   */

    //For query parameters description of UploadOptions, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
    //For request payload properties description of Requestproperties, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    Firebase.GCStorage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase or Google Cloud Storage bucket id */, "/media.mp4" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, gcs_upload_type_resumable /* upload type */, "media.mp4" /* path of remote file stored in the bucket */, "video/mp4" /* mime type */, nullptr /* UploadOptions data */, nullptr /* Requestproperties data */, nullptr /* UploadStatusInfo data to get the status */, gcsUploadCallback /* callback function */);
}

//The Google Cloud Storage upload callback function
void gcsUploadCallback(UploadStatusInfo info)
{
    if (info.status == fb_esp_gcs_upload_status_init)
    {
        Serial.printf("Uploading file %s to %s\n", info.localFileName.c_str(), info.remoteFileName.c_str());
    }
    else if (info.status == fb_esp_gcs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
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
        Serial.printf("Token: %s\n", meta.downloadTokens.c_str());       //only gcs_upload_type_multipart and gcs_upload_type_resumable upload types.
        Serial.printf("Download URL: %s\n", fbdo.downloadURL().c_str()); //only gcs_upload_type_multipart and gcs_upload_type_resumable upload types.
        Serial.println("------------------------------------");
        Serial.println();
    }
    else if (info.status == fb_esp_gcs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void loop()
{
}
