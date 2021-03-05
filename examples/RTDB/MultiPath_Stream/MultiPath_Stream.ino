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

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define FirebaseESP8266 data object
FirebaseData fbdo1;
FirebaseData fbdo2;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

String parentPath = "/Test/Stream";
String childPath[2] = {"/node1", "/node2"};
size_t childPathSize = 2;

uint16_t count = 0;

void printResult(FirebaseData &data);

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
  fbdo1.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo1.setResponseSize(1024);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo2.setBSSLBufferSize(1024, 1024);
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

  if (millis() - sendDataPrevMillis > 15000)
  {
    sendDataPrevMillis = millis();
    count++;

    Serial.println("------------------------------------");
    Serial.println("Set JSON...");

    FirebaseJson json;
    json.set("node1/data", "hello");
    json.set("node1/num", count);
    json.set("node2/data", "hi");
    json.set("node2/num", count);
    if (Firebase.RTDB.setJSON(&fbdo2, parentPath.c_str(), &json))
    {
      Serial.println("PASSED");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo2.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    //This will trig the another stream event.

    Serial.println("------------------------------------");
    Serial.println("Set string...");

    String Path = parentPath + "/node2/new/data";

    if (Firebase.RTDB.setString(&fbdo2, Path.c_str(), "test"))
    {
      Serial.println("PASSED");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo2.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    //This will trig the another stream event.

    Serial.println("------------------------------------");
    Serial.println("Set int...");

    Path = parentPath + "/node1/new/data";

    if (Firebase.RTDB.setInt(&fbdo2, Path.c_str(), count))
    {
      Serial.println("PASSED");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo2.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
}
