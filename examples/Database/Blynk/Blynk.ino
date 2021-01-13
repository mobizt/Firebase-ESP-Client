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

//This example shows the basic usage of Blynk platform and Firebase RTDB.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <BlynkSimpleEsp8266.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data object
FirebaseData fbdo1;
FirebaseData fbdo2;

FirebaseAuth auth;
FirebaseConfig config;

//Debug Blynk to serial port
#define BLYNK_PRINT Serial

//Auth token for your Blynk app project
#define BLYNK_AUTH "YOUR_BLYNK_APP_PROJECT_AUTH_TOKEN"

String path = "/Blynk_Test/Int";

//D4 or GPIO2 on Wemos D1 mini
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

  if (!Firebase.RTDB.beginStream(&fbdo1, path.c_str()))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{
  Blynk.run();

  if (!Firebase.RTDB.readStream(&fbdo1))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (fbdo1.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (fbdo1.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + fbdo1.streamPath());
    Serial.println("EVENT PATH: " + fbdo1.dataPath());
    Serial.println("DATA TYPE: " + fbdo1.dataType());
    Serial.println("EVENT TYPE: " + fbdo1.eventType());
    Serial.print("VALUE: ");
    if (fbdo1.dataType() == "int")
    {

      Serial.println(fbdo1.intData());
      if (fbdo1.intData() == 0)
      {
        digitalWrite(BuiltIn_LED, LOW);
        led.off();
      }
      else if (fbdo1.intData() == 1)
      {
        digitalWrite(BuiltIn_LED, HIGH);
        led.on();
      }
    }
    Serial.println("------------------------------------");
    Serial.println();
  }
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.println("------------------------------------");
  Serial.println("Set integer...");
  //Also can use Firebase.set instead of Firebase.setInt
  if (Firebase.RTDB.setInt(&fbdo2, path.c_str(), pinValue))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo2.dataPath());
    Serial.println("TYPE: " + fbdo2.dataType());
    Serial.print("VALUE: ");
    if (fbdo2.dataType() == "int")
      Serial.println(fbdo2.intData());
    Serial.println("------------------------------------");
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
