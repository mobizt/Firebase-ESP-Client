
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

//This example shows how to set complex json data through FirebaseJson object then read the data back and parse them.

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

        String path = "/Test/Json";

        String jsonStr = "";

        FirebaseJson json1;

        FirebaseJsonData jsonObj;

        json1.set("Hi/myInt", 200);
        json1.set("Hi/myDouble", 0.0023);
        json1.set("Who/are/[0]", "you");
        json1.set("Who/are/[1]", "they");
        json1.set("Who/is/[0]", "she");
        json1.set("Who/is/[1]", "he");
        json1.set("This/is/[0]", false);
        json1.set("This/is/[1]", true);
        json1.set("This/is/[2]", "my house");
        json1.set("This/is/[3]/my", "world");

        Serial.println("------------------------------------");
        Serial.println("JSON Data");
        json1.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println("------------------------------------");

        Serial.println("------------------------------------");
        Serial.println("Set JSON test...");

        if (Firebase.RTDB.set(&fbdo, path.c_str(), &json1))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
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

        Serial.println("------------------------------------");
        Serial.println("Get JSON test...");

        if (Firebase.RTDB.get(&fbdo, path.c_str()))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.print("VALUE: ");
            if (fbdo.dataType() == "json")
            {
                printResult(fbdo); //see addons/RTDBHelper.h
            }

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

        Serial.println("------------------------------------");
        Serial.println("Try to parse return data and get value..");

        json1 = fbdo.jsonObject();

        json1.get(jsonObj, "This/is/[3]/my");

        Serial.println("This/is/[3]/my: " + jsonObj.stringValue);

        json1.get(jsonObj, "Hi/myDouble");
        Serial.print("Hi/myDouble: ");
        Serial.println(jsonObj.doubleValue, 4);

        Serial.println("------------------------------------");
        Serial.println();
    }
}
