
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

// This example shows how to set and push timestamp (server time) which is the server variable that suopported by Firebase

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

    Serial.printf("Set timestamp... %s\n", Firebase.RTDB.setTimestamp(&fbdo, "/test/timestamp") ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
    {

      // In setTimestampAsync, the following timestamp will be 0 because the response payload was ignored for all async functions.

      // Timestamp saved in millisecond, get its seconds from int value
      Serial.print("TIMESTAMP (Seconds): ");
      Serial.println(fbdo.to<int>());

      // Or print the total milliseconds from double value
      // Due to bugs in Serial.print in Arduino library, use printf to print double instead.
      printf("TIMESTAMP (milliSeconds): %lld\n", fbdo.to<uint64_t>());
    }

    Serial.printf("Get timestamp... %s\n", Firebase.RTDB.getDouble(&fbdo, "/test/timestamp") ? "ok" : fbdo.errorReason().c_str());
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printf("TIMESTAMP: %lld\n", fbdo.to<uint64_t>());

    // To set and push data with timestamp, requires the JSON data with .sv placeholder
    FirebaseJson json;

    json.set("Data", "Hello");
    // now we will set the timestamp value at Ts
    json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value

    // Set data with timestamp
    Serial.printf("Set data with timestamp... %s\n", Firebase.RTDB.setJSON(&fbdo, "/test/set/data", &json) ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());

    // Push data with timestamp
    Serial.printf("Push data with timestamp... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/test/push/data", &json) ? "ok" : fbdo.errorReason().c_str());

    // Get previous pushed data
    Serial.printf("Get previous pushed data... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/push/data/" + fbdo.pushName()) ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
  }
}