/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows the basic usage of Blynk platform and Firebase RTDB.

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
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

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

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

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
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

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

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
