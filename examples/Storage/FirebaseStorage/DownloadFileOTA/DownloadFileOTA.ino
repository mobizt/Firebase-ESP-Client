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

//This example shows how to update firmware (bin file) from Firebase Storage bucket.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
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

//Define Firebase Data object
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
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

#if defined(ESP8266)
    //required for large file data, increase Rx size as needed.
    fbdo.setBSSLBufferSize(1024 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    /* Assign download buffer size in byte */
    //Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
    //The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
    config.fcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        //If you want to get download url to use with your own OTA update process using core update library,
        //see Metadata.ino example

        Serial.println("\nWait for file downloading...");

        //In ESP8266, this function will allocate 16k+ memory for internal SSL client.
        Serial.printf("Download firmware file... %s\n", Firebase.Storage.downloadOTA(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "<firmware.bin>" /* path of firmware file stored in the bucket */) ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == 200)
        {
            Serial.println("Update completed. Restarting...");
            Serial.println();
            delay(2000);
            ESP.restart();
        }
    }
}
