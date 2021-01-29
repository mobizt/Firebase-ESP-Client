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

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data objects
FirebaseData fbdo1;
FirebaseData fbdo2;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis1;

uint16_t count1;

String path = "";

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

  Serial.println("------------------------------------");
  Serial.println("Begin stream 1...");
  String Path = path + "/Stream/data1";
  if (!Firebase.RTDB.beginStream(&fbdo2, Path.c_str()))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo2.errorReason());
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

  if (!Firebase.RTDB.readStream(&fbdo2))
  {
    Serial.println("Can't read stream data");
    Serial.println("REASON: " + fbdo2.errorReason());
    Serial.println();
  }

  if (fbdo2.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (fbdo2.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data Available...");
    Serial.println("STREAM PATH: " + fbdo2.streamPath());
    Serial.println("EVENT PATH: " + fbdo2.dataPath());
    Serial.println("DATA TYPE: " + fbdo2.dataType());
    Serial.println("EVENT TYPE: " + fbdo2.eventType());
    Serial.print("VALUE: ");
    printResult(fbdo2);
    if (fbdo2.dataType() == "blob")
    {
      std::vector<uint8_t> blob = fbdo2.blobData();

      Serial.println();

      for (size_t i = 0; i < blob.size(); i++)
      {
        if (i > 0 && i % 16 == 0)
          Serial.println();

        if (i < 16)
          Serial.print("0");

        Serial.print(blob[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (millis() - sendDataPrevMillis1 > 15000)
  {
    sendDataPrevMillis1 = millis();

    //Create demo data
    uint8_t data[256];
    for (int i = 0; i < 256; i++)
      data[i] = i;
    data[255] = rand();

    Serial.println("------------------------------------");
    Serial.println("Set Blob Data 1...");
    String Path = path + "/Stream/data1";
    if (Firebase.RTDB.setBlob(&fbdo1, Path.c_str(), data, sizeof(data)))
    {
      Serial.println("PASSED");
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo1.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    FirebaseJson json;
    json.add("data1-1", count1).add("data1-2", count1 + 1).add("data1-3", count1 + 2);
    Serial.println("------------------------------------");
    Serial.println("Update Data 1...");
    Path = path + "/Stream/data1";
    if (Firebase.RTDB.updateNode(&fbdo1, Path.c_str(), &json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo1.dataPath());
      Serial.println("TYPE: " + fbdo1.dataType());
      Serial.print("VALUE: ");
      printResult(fbdo1);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo1.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println();

    count1 += 3;
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