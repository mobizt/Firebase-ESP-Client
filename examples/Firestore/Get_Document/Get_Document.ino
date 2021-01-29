
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

//This example shows how to get a document from a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the project ID */
#define FIREBASE_PROJECT_ID "PROJECT_ID"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
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

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo.setBSSLBufferSize(1024, 1024);
#endif

    String content;

    FirebaseJson js;

    js.set("fields/Japan/mapValue/fields/time_zone/integerValue", "9");
    js.set("fields/Japan/mapValue/fields/population/integerValue", "125570000");

    js.set("fields/Belgium/mapValue/fields/time_zone/integerValue", "1");
    js.set("fields/Belgium/mapValue/fields/population/integerValue", "11492641");

    js.set("fields/Singapore/mapValue/fields/time_zone/integerValue", "8");
    js.set("fields/Singapore/mapValue/fields/population/integerValue", "5703600");

    js.toString(content);

    //info is the collection id, countries is the document id in collection info.
    String documentPath = "info/countries";

    Serial.println("------------------------------------");
    Serial.println("Create document...");

    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.c_str()))
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

void loop()
{

    if (millis() - dataMillis > 60000 || dataMillis == 0)
    {
        dataMillis = millis();

        String documentPath = "info/countries";
        String mask = "Singapore";

        Serial.println("------------------------------------");
        Serial.println("Get a document...");

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str()))
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
