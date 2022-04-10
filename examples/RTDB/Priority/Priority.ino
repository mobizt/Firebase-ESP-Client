/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2022 mobizt
 *
 */

// This example shows how set node's priority and filtering the data based on priority of child nodes.
// The priority is virtual node with the key ".priority" and can't see in Console.

// Since data ordering is not supported in Firebase's REST APIs, then the query result will not sorted.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

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

// Define Firebase Data object
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

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    for (int i = 0; i < 15; i++)
    {

      float priority = 15 - i;
      String key = "item_" + String(i + 1);
      String val = "value_" + String(i + 1);
      json.clear();
      json.set(key, val);
      String Path = "/test/items/priority_" + String(15 - i);

      Serial.printf("Set json with proprity... %s\n", Firebase.RTDB.setJSONAsync(&fbdo, Path.c_str(), &json, priority) ? "ok" : fbdo.errorReason().c_str());
    }

    // Qury child nodes under "/test/items" with priority between 3.0 and 8.0
    // Since data ordering is not supported in Firebase's REST APIs, then the query result will not sorted.
    QueryFilter query;
    query.orderBy("$priority").startAt(3.0).endAt(8.0);

    Serial.printf("Set json with proprity... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/items", &query) ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printResult(fbdo); // see addons/RTDBHelper.h
  }
}