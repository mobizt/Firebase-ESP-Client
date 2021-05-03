
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

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

FirebaseJson json;

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
  fbdo.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.RTDB.setReadTimeout(&fbdo, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "tiny");

  //optional, set the decimal places for float and double data to be stored in database
  Firebase.setFloatDigits(2);
  Firebase.setDoubleDigits(6);

  /**
   * This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
   * Firewall that allows only GET and POST requests.
   * 
   * Firebase.RTDB.enableClassicRequest(&fbdo, true);
  */
}

void loop()
{
  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    String path = "/Test";
    String node;

    Serial.println("------------------------------------");
    Serial.println("Set double test...");

    for (uint8_t i = 0; i < 10; i++)
    {
      node = path + "/Double/Data" + String(i + 1);
      //Also can use Firebase.set instead of Firebase.setDouble
      if (Firebase.RTDB.setDoubleAsync(&fbdo, node.c_str(), ((i + 1) * 10) + 0.123456789))
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

    Serial.println("------------------------------------");
    Serial.println("Get double test...");

    for (uint8_t i = 0; i < 10; i++)
    {
      node = path + "/Double/Data" + String(i + 1);
      //Also can use Firebase.get instead of Firebase.setInt
      if (Firebase.RTDB.getInt(&fbdo, node.c_str()))
      {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
        Serial.println("ETag: " + fbdo.ETag());
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
    }

    Serial.println("------------------------------------");
    Serial.println("Push integer test...");

    for (uint8_t i = 0; i < 5; i++)
    {
      node = path + "/Push/Int";
      //Also can use Firebase.push instead of Firebase.pushInt
      if (Firebase.RTDB.pushIntAsync(&fbdo, node.c_str(), (i + 1)))
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

    Serial.println("------------------------------------");
    Serial.println("Push JSON test...");

    for (uint8_t i = 5; i < 10; i++)
    {

      json.clear().add("Data" + String(i + 1), i + 1);

      node = path + "/Push/Int";

      //Also can use Firebase.push instead of Firebase.pushJSON
      //Json string is not support in v 2.6.0 and later, only FirebaseJson object is supported.
      if (Firebase.RTDB.pushJSONAsync(&fbdo, node.c_str(), &json))
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

    Serial.println("------------------------------------");
    Serial.println("Update test...");

    for (uint8_t i = 0; i < 5; i++)
    {

      json.set("Data" + String(i + 1), i + 5.5);

      node = path + "/float";

      if (Firebase.RTDB.updateNodeAsync(&fbdo, node.c_str(), &json))
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
  }
}
