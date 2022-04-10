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

// This example shows how to construct queries to filter data.

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

/* This database secret required in this example to get the righs access to database rules */
#define DATABASE_SECRET "DATABASE_SECRET"

// Define Firebase Data object
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

  // Or use legacy authenticate method
  // Firebase.begin(DATABASE_URL, DATABASE_SECRET);

  Firebase.reconnectWiFi(true);
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && !taskCompleted)
  {

    taskCompleted = true;

    FirebaseJson json;

    for (uint8_t i = 0; i < 30; i++)
    {
      json.set("Data1", i + 1);
      json.set("Data2", i + 100);
      Serial.printf("Push json... %s\n", Firebase.RTDB.pushAsync(&fbdo, "/test/push", &json) ? "ok" : fbdo.errorReason().c_str());
    }
    Serial.println();

    // Add an index to the node that is being queried.
    // https://firebase.google.com/docs/database/security/indexing-data

    // Update the existing database rules by adding key "test/push/.indexOn" and value "Data2"
    // Check your database rules changes after running this function.

    /** If the authentication type is OAuth2.0 which allows the admin right access,
     * the database secret is not necessary by set this parameter with empty string "".
     */
    Firebase.RTDB.setQueryIndex(&fbdo, "/test/push" /* parent path of child's node that is being queried */, "Data2" /* the child node key that is being queried */, DATABASE_SECRET);

    QueryFilter query;

    query.orderBy("Data2");
    query.startAt(105);
    query.endAt(120);
    // get only last 8 results
    query.limitToLast(8);

    // Get filtered data
    Serial.printf("Get json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/push", &query) ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printResult(fbdo); // see addons/RTDBHelper.h

    // Clear all query parameters
    query.clear();
  }
}