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

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_PROJECT_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data objects
FirebaseData fbdo1;
FirebaseData fbdo2;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis1;

uint16_t count1;

String path = "";

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

  Serial.println("------------------------------------");
  Serial.println("Begin stream 1...");
  String node = path + "/Stream/data1";
  if (!Firebase.RTDB.beginStream(&fbdo2, node.c_str()))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo2.errorReason());
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void loop()
{

  if (!Firebase.ready())
    return;

  if (!Firebase.RTDB.readStream(&fbdo2))
  {
    Serial.println("Can't read stream data");
    Serial.println("REASON: " + fbdo2.errorReason());
    Serial.println();
  }

  if (fbdo2.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (fbdo2.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data Available...");
    Serial.println("STREAM PATH: " + fbdo2.streamPath());
    Serial.println("EVENT PATH: " + fbdo2.dataPath());
    Serial.println("DATA TYPE: " + fbdo2.dataType());
    Serial.println("EVENT TYPE: " + fbdo2.eventType());
    Serial.print("VALUE: ");
    printResult(fbdo2);
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (millis() - sendDataPrevMillis1 > 15000)
  {
    sendDataPrevMillis1 = millis();

    //Create demo data
    uint8_t data[256];
    for (int i = 0; i < 256; i++)
      data[i] = i;
    data[255] = rand();

    Serial.println("------------------------------------");
    Serial.println("Set Blob Data 1...");
    String Path = path + "/Stream/data1";
    if (Firebase.RTDB.setBlob(&fbdo1, Path.c_str(), data, sizeof(data)))
    {
      Serial.println("PASSED");
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo1.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    FirebaseJson json;
    json.add("data1-1", count1).add("data1-2", count1 + 1).add("data1-3", count1 + 2);
    Serial.println("------------------------------------");
    Serial.println("Update Data 1...");
    Path = path + "/Stream/data1";
    if (Firebase.RTDB.updateNode(&fbdo1, Path.c_str(), &json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo1.dataPath());
      Serial.println("TYPE: " + fbdo1.dataType());
      Serial.print("VALUE: ");
      printResult(fbdo1);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo1.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println();

    count1 += 3;
  }
}
