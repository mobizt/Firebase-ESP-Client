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

//This example shows how to store and read binary data from memory to database.

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

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

String path = "/Test";

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
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);



  Serial.println("------------------------------------");
  Serial.println("Set BLOB data test...");

  //Create demo data
  uint8_t data[256];
  for (int i = 0; i < 256; i++)
    data[i] = i;

  String Path = path + "/Binary/Blob/data";
  //Set binary data to database (also can use Firebase.set)
  if (Firebase.RTDB.setBlob(&fbdo, Path.c_str(), data, sizeof(data)))
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

  Serial.println("------------------------------------");
  Serial.println("Get BLOB data test...");

  //Get binary data from database (also can use Firebase.get)
  if (Firebase.RTDB.getBlob(&fbdo, Path.c_str()))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
    Serial.print("VALUE: ");
    if (fbdo.dataType() == "blob")
    {

      std::vector<uint8_t> blob = fbdo.blobData();

      Serial.println();

      for (size_t i = 0; i < blob.size(); i++)
      {
        if (i > 0 && i % 16 == 0)
          Serial.println();

        if (blob[i] < 16)
          Serial.print("0");

        Serial.print(blob[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
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

  Serial.println("------------------------------------");
  Serial.println("Append BLOB data test...");

  //Create demo data
  for (int i = 0; i < 256; i++)
    data[i] = 255 - i;

  Path = path + "/Binary/Blob/Logs";

  //Append binary data to database (also can use Firebase.push)
  if (Firebase.RTDB.pushBlob(&fbdo, Path.c_str(), data, sizeof(data)))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("PUSH NAME: " + fbdo.pushName());
    Serial.println("------------------------------------");
    Serial.println();

    Serial.println("------------------------------------");
    Serial.println("Get appended BLOB data test...");

    Path = path + "/Binary/Blob/Logs" + fbdo.pushName();

    //Get appended binary data from database (also can use Firebase.get)
    if (Firebase.RTDB.getBlob(&fbdo, Path.c_str()))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.print("VALUE: ");
      if (fbdo.dataType() == "blob")
      {

        std::vector<uint8_t> blob = fbdo.blobData();
        Serial.println();
        for (size_t i = 0; i < blob.size(); i++)
        {
          if (i > 0 && i % 16 == 0)
            Serial.println();
          if (blob[i] < 16)
            Serial.print("0");
          Serial.print(blob[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
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
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void loop()
{
}
