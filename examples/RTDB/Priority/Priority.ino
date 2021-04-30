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

//This example shows how set node's priority and filtering the data based on priority of child nodes.
//The priority is virtual node with the key ".priority" and can't see in Console.

//Since data ordering is not supported in Firebase's REST APIs, then the query result will not sorted.

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

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_PROJECT_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define FirebaseESP8266 data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

FirebaseJson json;

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

    /* Assign the project host and api key (required) */
    config.host = FIREBASE_PROJECT_HOST;
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
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

        String path = "/Test";

        Serial.println("------------------------------------");
        Serial.println("Set priority test...");

        for (int i = 0; i < 15; i++)
        {

            float priority = 15 - i;
            String key = "item_" + String(i + 1);
            String val = "value_" + String(i + 1);
            json.clear();
            json.set(key, val);
            String Path = path + "/Items/priority_" + String(15 - i);

            if (Firebase.RTDB.setJSON(&fbdo, Path.c_str(), &json, priority))
            {
                Serial.println("PASSED");
                Serial.println("PATH: " + fbdo.dataPath());
                Serial.println("TYPE: " + fbdo.dataType());
                Serial.println("CURRENT ETag: " + fbdo.ETag());
                Serial.print("VALUE: ");
                printResult(fbdo);
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

        //Qury child nodes under "/Test/Item" with priority between 3.0 and 8.0
        //Since data ordering is not supported in Firebase's REST APIs, then the query result will not sorted.
        QueryFilter query;
        query.orderBy("$priority").startAt(3.0).endAt(8.0);

        Serial.println("------------------------------------");
        Serial.println("Filtering based on priority test...");

        String Path = path + "/Items";

        if (Firebase.RTDB.getJSON(&fbdo, Path.c_str(), &query))
        {

            Serial.println("PASSED");
            Serial.println("JSON DATA: ");
            printResult(fbdo);
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
