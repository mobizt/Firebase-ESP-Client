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

// This example shows how to backup and restore database data
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

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted;

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

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  // required for large file data, increase Rx size as needed.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
#endif
}

// The Firebase download callback function
void rtdbDownloadCallback(RTDB_DownloadStatusInfo info)
{
  if (info.status == fb_esp_rtdb_download_status_init)
  {
    Serial.printf("Downloading file %s (%d) to %s\n", info.remotePath.c_str(), info.size, info.localFileName.c_str());
  }
  else if (info.status == fb_esp_rtdb_download_status_download)
  {
    Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
  }
  else if (info.status == fb_esp_rtdb_download_status_complete)
  {
    Serial.println("Backup completed\n");
  }
  else if (info.status == fb_esp_rtdb_download_status_error)
  {
    Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
  }
}

// The Firebase upload callback function
void rtdbUploadCallback(RTDB_UploadStatusInfo info)
{
  if (info.status == fb_esp_rtdb_upload_status_init)
  {
    Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.size, info.remotePath.c_str());
  }
  else if (info.status == fb_esp_rtdb_upload_status_upload)
  {
    Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
  }
  else if (info.status == fb_esp_rtdb_upload_status_complete)
  {
    Serial.println("Restore completed\n");
  }
  else if (info.status == fb_esp_rtdb_upload_status_error)
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

    // Download and save data to Flash memory.
    //<target node> is the full path of database to backup and restore.
    //<file name> is file name included path to save to Flash meory
    // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.

    if (Firebase.RTDB.backup(&fbdo, mem_storage_type_flash, "/<target node>" /* node path to backup*/, "/<file name>" /* file name included path to save */, rtdbDownloadCallback /* callback function */))
    {
      Serial.printf("backup file, %s\n", fbdo.getBackupFilename().c_str());
      Serial.printf("file size, %d\n", fbdo.getBackupFileSize());
    }
    else
      fbdo.fileTransferError().c_str();

    // Restore data to defined database path using backup file on Flash memory.
    //<target node> is the full path of database to restore
    //<file name> is file name included path of backed up file.
    // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.

    Serial.println("\nRestore... \n");

    if (!Firebase.RTDB.restore(&fbdo, mem_storage_type_flash, "/<target node>" /* node path to restore */, "/<file name>" /* backup file to restore */, rtdbUploadCallback /* callback function */))
      fbdo.fileTransferError().c_str();
  }
}
