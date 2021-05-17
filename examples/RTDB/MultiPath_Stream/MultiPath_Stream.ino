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

//Define FirebaseESP8266 data object
FirebaseData fbdo1;
FirebaseData fbdo2;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Important information
//In ESP8266 Aruino Core SDK v3.x.x
//The free heap is significantly reduced as much as 5-6 kB from v2.7.4.
//This may lead to out of memory sitation when two Firebase Data objects are used simultaneously (when sessions connected).
//Minimize the reserved memory for BearSSL will gain the free heap a bit but may not enough for your usage.
//You can stay with Core SDK v2.7.4 until this memory issue was solve in the Core SDK.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

String parentPath = "/Test/Stream";
String childPath[2] = {"/node1", "/node2"};
size_t childPathSize = 2;

uint16_t count = 0;

void streamCallback(MultiPathStream stream)
{
  Serial.println();
  Serial.println("Stream Data1 available...");

  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.println("path: " + stream.dataPath + ", type: " + stream.type + ", value: " + stream.value);
    }
  }

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
  fbdo1.setBSSLBufferSize(1024, 512);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo1.setResponseSize(1024);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo2.setBSSLBufferSize(1024, 512);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo2.setResponseSize(1024);


  //The data under the node being stream (parent path) should keep small
  //Large stream payload leads to the parsing error due to memory allocation.

  //The operations is the same as normal stream unless the JSON stream payload will be parsed
  //with the predefined node path (child paths).

  //The changes occurred in any child node that is not in the child paths array will sent to the
  //client as usual.
  if (!Firebase.RTDB.beginMultiPathStream(&fbdo1, parentPath.c_str(), childPath, childPathSize))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.RTDB.setMultiPathStreamCallback(&fbdo1, streamCallback, streamTimeoutCallback);
}

void loop()
{

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    Serial.println("------------------------------------");
    Serial.println("Set JSON...");

    FirebaseJson json;

    for (size_t i = 0; i < 10; i++)
    {
      json.set("node1/data", "hello");
      json.set("node1/num", count);
      json.set("node2/data", "hi");
      json.set("node2/num", count);

      //The response is ignored in this async function, it may return true as long as the connection is established.
      //The purpose for this async function is to set, push and update data instantly.
      Firebase.RTDB.setJSONAsync(&fbdo2, parentPath.c_str(), &json);
      count++;
    }
  }
}
