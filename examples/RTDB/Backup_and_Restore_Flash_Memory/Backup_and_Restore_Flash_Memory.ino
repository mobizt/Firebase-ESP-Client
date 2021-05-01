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

//This example shows how to backup and restore database data
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"

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

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

#ifdef ESP8266
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);

}

void loop()
{
  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    Serial.println("------------------------------------");
    Serial.println("Backup test...");

    //Download and save data to Flash memory.
    //<target node> is the full path of database to backup and restore.
    //<file name> is file name included path to save to Flash meory
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (!Firebase.RTDB.backup(&fbdo, mem_storage_type_flash, "/<target node>", "/<file name>"))
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.fileTransferError());
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("PASSED");
      Serial.println("BACKUP FILE: " + fbdo.getBackupFilename());
      Serial.printf("FILE SIZE: %d\n", fbdo.getBackupFileSize());
      Serial.println("------------------------------------");
      Serial.println();
    }

    Serial.println("------------------------------------");
    Serial.println("Restore test...");

    //Restore data to defined database path using backup file on Flash memory.
    //<target node> is the full path of database to restore
    //<file name> is file name included path of backed up file.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (!Firebase.RTDB.restore(&fbdo, mem_storage_type_flash, "/<target node>", "/<file name>"))
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.fileTransferError());
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("PASSED");
      Serial.println("BACKUP FILE: " + fbdo.getBackupFilename());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
}
