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

// This example shows the basic usage of Blynk platform and Firebase RTDB.

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

#include <BlynkSimpleEsp8266.h>

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
FirebaseData stream;

FirebaseAuth auth;
FirebaseConfig config;

// Debug Blynk to serial port
#define BLYNK_PRINT Serial

// Auth token for your Blynk app project
#define BLYNK_AUTH "YOUR_BLYNK_APP_PROJECT_AUTH_TOKEN"

String path = "/Blynk_Test/Int";

// D4 or GPIO2 on Wemos D1 mini
uint8_t BuiltIn_LED = 2;

/**
 * Blynk app Widget setup
 * **********************
 *
 * 1. Button Widget (Switch type), Output -> Virtual pin V1
 * 2. LED Widget, Input -> Virtual pin V2
 */
WidgetLED led(V2);

void setup()
{

  Serial.begin(115200);

  pinMode(BuiltIn_LED, OUTPUT);

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

  if (!Firebase.RTDB.beginStream(&stream, "/test/blynk/int"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{

  Blynk.run();

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready())
  {
    if (!Firebase.RTDB.readStream(&stream))
      Serial.printf("sream read error, %s\n\n", stream.errorReason().c_str());

    if (stream.streamTimeout())
      Serial.println("stream timeout, resuming...\n");

    if (stream.streamAvailable())
    {

      Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\nvalue, %d\n\n",
                    stream.streamPath().c_str(),
                    stream.dataPath().c_str(),
                    stream.dataType().c_str(),
                    stream.eventType().c_str(),
                    stream.intData());

      if (stream.dataType() == "int")
      {
        if (stream.intData() == 1)
        {
          digitalWrite(BuiltIn_LED, HIGH);
          led.on();
        }
        else
        {
          digitalWrite(BuiltIn_LED, LOW);
          led.off();
        }
      }
    }
  }
}

BLYNK_WRITE(V1)
{
  Serial.printf("Set int... %s\n\n", Firebase.RTDB.setInt(&fbdo, "/test/blynk/int", param.asInt()) ? "ok" : fbdo.errorReason().c_str());
}
