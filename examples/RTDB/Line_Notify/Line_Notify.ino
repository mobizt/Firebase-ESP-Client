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

//Required Line Notify Library for ESP8266 https://github.com/mobizt/Line-Notify-ESP8266
//or Line Notify Library for ESP32 https://github.com/mobizt/Line-Notify-ESP32

#if defined(ESP32)
#include <WiFi.h>
#include <LineNotifyESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <LineNotifyESP8266.h>
#endif
#include <Firebase_ESP_Client.h>

#define LINE_TOKEN "YOUR_LINE_NOTIFY_TOKEN"

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

LineNotifyHTTPClient net;

String path = "/Test";

unsigned long sendDataPrevMillis = 0;

unsigned long sendMessagePrevMillis = 0;

uint16_t count = 0;

void printResult(FirebaseData &data);

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

    lineNotify.init(&net, LINE_TOKEN);

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

  if (millis() - sendDataPrevMillis > 30000)
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
      printResult(fbdo);
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

    if (fbdo.pauseFirebase(true))
    {

      Serial.println("------------------------------------");
      Serial.println("Send Line Message...");

      //Pause Firebase and use WiFiClient accessed through fbdo.http
      uint8_t status = lineNotify.sendLineMessage("Instant sending message after call!");
      if (status == LineNotifyESP8266::LineStatus::SENT_COMPLETED)
      {
        Serial.println("send Line message completed");
        Serial.println("Text message limit: " + String(lineNotify.textMessageLimit()));
        Serial.println("Text message remaining: " + String(lineNotify.textMessageRemaining()));
        Serial.println("Image message limit: " + String(lineNotify.imageMessageLimit()));
        Serial.println("Image message remaining: " + String(lineNotify.imageMessageRemaining()));
      }
      else if (status == LineNotifyESP8266::LineStatus::SENT_FAILED)
        Serial.println("Send image data was failed!");
      else if (status == LineNotifyESP8266::LineStatus::CONNECTION_FAILED)
        Serial.println("Connection to LINE sevice faild!");
      Serial.println();

      //Unpause Firebase
      fbdo.pauseFirebase(false);
    }
    else
    {
      Serial.println("Could not pause Firebase");
      Serial.println();
    }
  }

  if (millis() - sendMessagePrevMillis > 60000)
  {
    sendMessagePrevMillis = millis();
    if (fbdo.pauseFirebase(true))
    {

      Serial.println("------------------------------------");
      Serial.println("Send Line Message...");

      uint8_t status = lineNotify.sendLineMessage("Schedule message sending!");
      if (status == LineNotifyESP8266::LineStatus::SENT_COMPLETED)
      {
        Serial.println("send Line message completed");
        Serial.println("Text message limit: " + String(lineNotify.textMessageLimit()));
        Serial.println("Text message remaining: " + String(lineNotify.textMessageRemaining()));
        Serial.println("Image message limit: " + String(lineNotify.imageMessageLimit()));
        Serial.println("Image message remaining: " + String(lineNotify.imageMessageRemaining()));
      }
      else if (status == LineNotifyESP8266::LineStatus::SENT_FAILED)
        Serial.println("Send image data was failed!");
      else if (status == LineNotifyESP8266::LineStatus::CONNECTION_FAILED)
        Serial.println("Connection to LINE sevice faild!");
      Serial.println();

      //Unpause Firebase
      fbdo.pauseFirebase(false);
    }
    else
    {
      Serial.println("Could not pause Firebase");
    }
  }

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
    printResult(fbdo);
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_FLOAT)
        Serial.println(jsonData.floatValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
               jsonData.typeNum == FirebaseJson::JSON_NULL ||
               jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
  else
  {
    Serial.println(data.payload());
  }
}
