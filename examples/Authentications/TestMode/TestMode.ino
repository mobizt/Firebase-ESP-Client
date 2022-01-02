
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

/** This example will show how to access the RTDB in Test Mode (no authentication).
*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
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

    /* Assign the certificate file (optional) */
    //config.cert.file = "/cert.cer";
    //config.cert.file_storage = StorageType::FLASH;

    /* Assign the database URL(required) */
    config.database_url = DATABASE_URL;

    config.signer.test_mode = true;

    /**
     Set the database rules to allow public read and write.

       {
	      "rules": {
		      ".read": true,
		      ".write": true
	      }
        }

    */

    Firebase.reconnectWiFi(true);

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

    //Or use legacy authenticate method
    //Firebase.begin(DATABASE_URL, DATABASE_SECRET);
}

void loop()
{
    if (millis() - dataMillis > 5000)
    {
        dataMillis = millis();
        Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "/test/int", count++) ? "ok" : fbdo.errorReason().c_str());
    }
}
