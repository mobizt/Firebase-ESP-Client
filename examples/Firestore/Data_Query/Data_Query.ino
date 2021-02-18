
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

//This example shows how to run a query. This operation required Email/password, custom or OAUth2.0 authentication.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "PROJECT_ID"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
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

    /* Assign the project host and api key (required) */
    config.host = FIREBASE_HOST;
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo.setBSSLBufferSize(1024, 1024);
#endif
}

void loop()
{

    if (millis() - dataMillis > 60000 || dataMillis == 0)
    {
        dataMillis = millis();

        Serial.println("------------------------------------");
        Serial.println("Query a Firestore database...");

        //If you have run the Create_Documents example, the document b0 (in collection a0) contains the document collection c0, and
        //c0 contains the collections d?.

        //The following query will query at collection c0 to get the 3 documents in the payload result with descending order.

        FirebaseJson query;

        query.set("select/fields/[0]/fieldPath", "count");
        query.set("select/fields/[1]/fieldPath", "random");
        //query.set("select/fields/[2]/fieldPath", "status");

        query.set("from/collectionId", "c0");
        query.set("from/allDescendants", false);
        query.set("orderBy/field/fieldPath", "count");
        query.set("orderBy/direction", "DESCENDING");
        query.set("limit", 3);

        //The consistencyMode and consistency arguments are not assigned
        //The consistencyMode is set to fb_esp_firestore_consistency_mode_undefined by default.
        //The arguments is the consistencyMode value, see the function description at
        //https://github.com/mobizt/Firebase-ESP-Client/tree/main/src#runs-a-query

        if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, "a0/b0" /* The document path */, &query /* The FirebaseJson object holds the StructuredQuery data */))
        {
            Serial.println("PASSED");
            Serial.println("------------------------------------");
            Serial.println(fbdo.payload());
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
