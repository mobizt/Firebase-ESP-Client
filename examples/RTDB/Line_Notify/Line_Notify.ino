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

//Example showed how to pause Firebase and use shared WiFi Client to send Line message.

/*
 * This shows how to pause the Firbase and send LINE Notify.
 * Install Line Notify Arduino library for ESP8266 and ESP32 https://github.com/mobizt/ESP-Line-Notify
 *
 * More about Line Notify service https://notify-bot.line.me/en/
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

#include <ESP_Line_Notify.h>

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

String path = "/Test";

unsigned long sendDataPrevMillis = 0;

unsigned long sendMessagePrevMillis = 0;

uint16_t count = 0;

/* Define the LineNotifyClient object */
LineNotiFyClient client;

/* Function to print the sending result via Serial (optional) */
void LineNotifyResult(LineNotifySendingResult result);

/* The sending callback function (optional) */
void LineNotifyCallback(LineNotifySendingResult result);

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

  String Path = path + "/Stream/String";

  Serial.println("------------------------------------");
  Serial.println("Begin stream...");
  if (!Firebase.RTDB.beginStream(&fbdo, Path.c_str()))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
    Serial.println("------------------------------------");
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

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    count++;

    Serial.println("------------------------------------");
    Serial.println("Set Data...");
    String Path = path + "/Stream/String";
    String value = "Hello World! " + String(count);
    if (Firebase.RTDB.set(&fbdo, Path.c_str(), value.c_str()))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
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

    //pause Firebase
    fbdo.pauseFirebase(true);

    Serial.println("------------------------------------");

    client.reconnect_wifi = true;

    Serial.println("Sending Line Notify message...");

    client.token = "Your Line Notify Access Token";
    client.message = "Hello world";

    //Assign the Line Notify Sending Callback function.
    client.sendingg_callback = LineNotifyCallback;

    LineNotifySendingResult result = LineNotify.send(client);

    //Print the Line Notify sending result.
    LineNotifyResult(result);

    Serial.println("--------------------------------");
    Serial.println();

    //resume Firebase
    fbdo.pauseFirebase(false);
  }

  if (Firebase.ready())
  {

    if (!Firebase.RTDB.readStream(&fbdo))
    {
      Serial.println("------------------------------------");
      Serial.println("Read stream...");
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    if (fbdo.streamTimeout())
    {
      Serial.println("Stream timeout, resume streaming...");
      Serial.println();
    }

    if (fbdo.streamAvailable())
    {
      Serial.println("------------------------------------");
      Serial.println("Stream Data available...");
      Serial.println("STREAM PATH: " + fbdo.streamPath());
      Serial.println("EVENT PATH: " + fbdo.dataPath());
      Serial.println("DATA TYPE: " + fbdo.dataType());
      Serial.println("EVENT TYPE: " + fbdo.eventType());
      Serial.print("VALUE: ");
      printResult(fbdo); //see addons/RTDBHelper.h
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
}

/* Function to print the sending result via Serial */
void LineNotifyResult(LineNotifySendingResult result)
{
  if (result.status == LineNotify_Sending_Success)
  {
    Serial.printf("Status: %s\n", "success");
    Serial.printf("Text limit: %d\n", result.quota.text.limit);
    Serial.printf("Text remaining: %d\n", result.quota.text.remaining);
    Serial.printf("Image limit: %d\n", result.quota.image.limit);
    Serial.printf("Image remaining: %d\n", result.quota.image.remaining);
    Serial.printf("Reset: %d\n", result.quota.reset);
  }
  else if (result.status == LineNotify_Sending_Error)
  {
    Serial.printf("Status: %s\n", "error");
    Serial.printf("error code: %d\n", result.error.code);
    Serial.printf("error msg: %s\n", result.error.message.c_str());
  }
}

/* The sending callback function (optional) */
void LineNotifyCallback(LineNotifySendingResult result)
{
  if (result.status == LineNotify_Sending_Begin)
  {
    Serial.println("Sending begin");
  }
  else if (result.status == LineNotify_Sending_Upload)
  {
    Serial.printf("Uploaded %s, %d%s\n", result.file_name.c_str(), (int)result.progress, "%");
  }
  else if (result.status == LineNotify_Sending_Success)
  {
    Serial.println("Sending success\n\n");
    Serial.printf("Text limit: %d\n", result.quota.text.limit);
    Serial.printf("Text remaining: %d\n", result.quota.text.remaining);
    Serial.printf("Image limit: %d\n", result.quota.image.limit);
    Serial.printf("Image remaining: %d\n", result.quota.image.remaining);
    Serial.printf("Reset: %d\n", result.quota.reset);
  }
  else if (result.status == LineNotify_Sending_Error)
  {
    Serial.println("Sending failed\n\n");
    Serial.printf("error code: %d\n", result.error.code);
    Serial.printf("error msg: %s\n", result.error.message.c_str());
  }
}
