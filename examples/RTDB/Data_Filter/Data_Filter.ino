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

//This example shows how to construct queries to filter data.

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

/* This database secret required in this example to get the righs access to database rules */
#define DATABASE_SECRET "DATABASE_SECRET"

//Define Firebase Data object
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

  //Set the size of HTTP response buffers in the case where we want to work with large data in this example.
  fbdo.setResponseSize(1024);
}

void loop()
{
  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    Serial.println("------------------------------------");
    Serial.println("Push JSON test...");

    for (uint8_t i = 0; i < 30; i++)
    {
      json.set("Data1", i + 1);
      json.set("Data2", i + 100);

      //Also can use Firebase.push instead of Firebase.pushJSON
      //JSON string does not support in v 2.6.0 and later, only FirebaseJson object is supported.
      if (Firebase.RTDB.pushJSONAsync(&fbdo, "/Test/Int", &json))
      {
        Serial.println("PASSED");
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

    //Add an index to the node that being query.
    //Update the existing database rules by adding key "Test/Int/.indexOn" and value "Data2"
    //Check your database rules changes after running this function.

    /** If the authentication type is OAuth2.0 which allows the admin right access, 
     * the database secret is not necessary by set this parameter with empty string "".
    */
    Firebase.RTDB.setQueryIndex(&fbdo, "Test/Int", "Data2", DATABASE_SECRET);

    QueryFilter query;

    query.orderBy("Data2");
    query.startAt(105);
    query.endAt(120);
    query.limitToLast(8);

    //Begin the data filtering test
    Serial.println("------------------------------------");
    Serial.println("Data Filtering test...");

    if (Firebase.RTDB.getJSON(&fbdo, "/Test/Int", &query))
    {

      Serial.println("PASSED");
      Serial.println("JSON DATA: ");
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

    //Clear all query parameters
    query.clear();
  }
}
