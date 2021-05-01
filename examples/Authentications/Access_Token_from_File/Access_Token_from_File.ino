
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

/** This example will show how to authenticate as admin using 
 * the Service Account file to create the access token to sign in internally.
*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

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


String path = "/Test";
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

    /* Assign the certificate file (optional) */
    config.cert.file = "/gsr2.pem";
    config.cert.file_storage = mem_storage_type_flash; //or mem_storage_type_sd

    
    /* The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h. */

    /* Assign the sevice account JSON file and the file storage type (required) */
    config.service_account.json.path = "/service_account_file.json"; //change this for your json file
    config.service_account.json.storage_type = mem_storage_type_flash;

    /** The user UID set to empty to sign in as admin */
    auth.token.uid = "";

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    /** The scope of the OAuth2.0 authentication 
     * If you wan't this access token for others Google Cloud Services.
    */
    //config.signer.tokens.scope = "Google Scope 1 Url, Google Scope 2 Url,..";

    Firebase.reconnectWiFi(true);

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    /** Assign the maximum retry of token generation */
    config.max_token_generation_retry = 5;

    /** To set system time with the timestamp from RTC
     * The internal NTP server time acquisition
     * of token generation process will be skipped, 
     * if the system time is already set. 
     * 
     * sec is the seconds from midnight Jan 1, 1970
    */
    //Firebase.setSystemTime(sec);

    /* Now we start to signin using access token */

    /** Initialize the library with the Firebase authen and config.
     *  
     * The device time will be set by sending request to the NTP server 
     * befor token generation and exchanging.
     * 
     * The signed RSA256 jwt token will be created and used for access token exchanging.
     * 
     * Theses process may take time to complete.
    */
    Firebase.begin(&config, &auth);

    /* The access token (C++ string) can be accessed from config.signer.tokens.access_token. */
}

void loop()
{
    if (millis() - dataMillis > 5000)
    {
        dataMillis = millis();

        if (Firebase.ready())
        {
            Serial.println("------------------------------------");
            Serial.println("Set int test...");

            String node = path + "/int";

            if (Firebase.RTDB.set(&fbdo, node.c_str(), count++))
            {
                Serial.println("PASSED");
                Serial.println("PATH: " + fbdo.dataPath());
                Serial.println("TYPE: " + fbdo.dataType());
                Serial.println("ETag: " + fbdo.ETag());
                Serial.print("VALUE: ");
                printResult(fbdo); //see addons/RTDBHelper.h
                Serial.println("------------------------------------");
                Serial.println();
            }
            else
            {
                Serial.println("FAILED");
                Serial.println("REASON: " + fbdo.errorReason());
                Serial.println("------------------------------------");
                Serial.println();
            }
        }
    }
}