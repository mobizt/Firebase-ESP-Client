
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

//This example shows how to dynamic allocated the Firebase Data object and deallocated it for release the memory when no further use.

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

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

FirebaseAuth auth;
FirebaseConfig config;

//Define FirebaseESP8266 data object
FirebaseData *fbdo1 = new FirebaseData();
FirebaseData *fbdo2 = new FirebaseData();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Important information
//In ESP8266 Aruino Core SDK v3.x.x
//The free heap is significantly reduced as much as 5-6 kB from v2.7.4.
//This may lead to out of memory sitation when two Firebase Data objects are used simultaneously (when sessions connected).
//Minimize the reserved memory for BearSSL will gain the free heap a bit but may not enough for your usage.
//You can stay with Core SDK v2.7.4 until this memory issue was solve in the Core SDK.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long sendDataPrevMillis = 0;

String path = "/Test/Stream";

int count = 0;

bool deallocated = false;

void streamCallback(FirebaseStream data)
{

    Serial.println("Stream Data1 available...");
    Serial.println("STREAM PATH: " + data.streamPath());
    Serial.println("EVENT PATH: " + data.dataPath());
    Serial.println("DATA TYPE: " + data.dataType());
    Serial.println("EVENT TYPE: " + data.eventType());
    Serial.print("VALUE: ");
    printResult(data); //see addons/RTDBHelper.h
    Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
    {
        Serial.println();
        Serial.println("Stream timeout, resume streaming...");
        Serial.println();
    }
}

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

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo1->setBSSLBufferSize(1024, 512);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo1->setResponseSize(1024);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo2->setBSSLBufferSize(1024, 512);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo2->setResponseSize(1024);

    if (!Firebase.RTDB.beginStream(fbdo1, path.c_str()))
    {
        Serial.println("------------------------------------");
        Serial.println("Can't begin stream connection...");
        Serial.println("REASON: " + fbdo1->errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }

    Firebase.RTDB.setStreamCallback(fbdo1, streamCallback, streamTimeoutCallback);
}

void loop()
{
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
    {
        count++;
        sendDataPrevMillis = millis();

        if (count == 10)
        {
            deallocated = false;

            Serial.println("Reallocate the Firebase Data objects again after 10 times Firebasse call");

            fbdo1 = new FirebaseData();
            fbdo2 = new FirebaseData();

#if defined(ESP8266)
            //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
            fbdo1->setBSSLBufferSize(1024, 512);
#endif

            //Set the size of HTTP response buffers in the case where we want to work with large data.
            fbdo1->setResponseSize(1024);

#if defined(ESP8266)
            //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
            fbdo2->setBSSLBufferSize(1024, 512);
#endif

            //Set the size of HTTP response buffers in the case where we want to work with large data.
            fbdo2->setResponseSize(1024);

            if (!Firebase.RTDB.beginStream(fbdo1, path.c_str()))
            {
                Serial.println("------------------------------------");
                Serial.println("Can't begin stream connection...");
                Serial.println("REASON: " + fbdo1->errorReason());
                Serial.println("------------------------------------");
                Serial.println();
            }

            Firebase.RTDB.setStreamCallback(fbdo1, streamCallback, streamTimeoutCallback);

            count = 0;
        }

        if (!deallocated)
            Serial.print("Free Heap after allocated the Firebase Data objects: ");
        else
            Serial.print("Free Heap after deallocated the Firebase Data objects: ");

        Serial.println(ESP.getFreeHeap());

        delay(0);

        if (deallocated)
            return;

        Serial.println("------------------------------------");
        Serial.println("Set Int...");
        String Path = path + "/Int";

        if (Firebase.RTDB.set(fbdo2, Path.c_str(), count))
        {
            Serial.println("PASSED");
            Serial.println("------------------------------------");
            Serial.println();
        }
        else
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo2->errorReason());
            Serial.println("------------------------------------");
            Serial.println();
        }

        if (count == 5)
        {
            Serial.println("Deallocate the Firebase Data objects after 3 times Firebasse call");

            deallocated = true;

            //Need to stop the stream to prevent the readStream and callback to be called after objects deallocation
            Firebase.RTDB.endStream(fbdo1);
            Firebase.RTDB.removeStreamCallback(fbdo1);

            //Deallocate
            delete fbdo1;
            delete fbdo2;

            fbdo1 = nullptr;
            fbdo2 = nullptr;
        }
    }
}