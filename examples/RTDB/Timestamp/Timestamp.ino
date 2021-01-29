
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

//This example shows how to set and push timestamp (server time) which is the server variable that suopported by Firebase

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

//Define FirebaseESP8266 data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

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

  /*
  This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
  Firewall that allows only GET and POST requests.
  
  Firebase.enableClassicRequest(&fbdo, true);
  */

  String path = "/Test";
  String Path = path + "/Set/Timestamp";

  Serial.println("------------------------------------");
  Serial.println("Set Timestamp test...");

  if (Firebase.RTDB.setTimestamp(&fbdo, Path.c_str()))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());

    //Timestamp saved in millisecond, get its seconds from intData()
    Serial.print("TIMESTAMP (Seconds): ");
    Serial.println(fbdo.intData());

    //Or print the total milliseconds from doubleData()
    //Due to bugs in Serial.print in Arduino library, use printf to print double instead.
    printf("TIMESTAMP (milliSeconds): %.0lf\n", fbdo.doubleData());

    //Or print it from payload directly
    Serial.print("TIMESTAMP (milliSeconds): ");
    Serial.println(fbdo.payload());

    Path = path + "/Set/Timestamp";

    //Due to some internal server error, ETag cannot get from setTimestamp
    //Try to get ETag manually
    Serial.println("ETag: " + Firebase.RTDB.getETag(&fbdo, Path.c_str()));
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
  Serial.println("Get Timestamp (double of milliseconds) test...");

  Path = path + "/Set/Timestamp";

  if (Firebase.RTDB.getDouble(&fbdo, Path.c_str()))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());

    printf("TIMESTAMP: %.0lf\n", fbdo.doubleData());
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
  Serial.println("Push Timestamp test...");

  Path = path + "/Push/Timestamp";

  if (Firebase.RTDB.pushTimestamp(&fbdo, Path.c_str()))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.print("PUSH NAME: ");
    Serial.println(fbdo.pushName());

    Path = path + "/Push/Timestamp/" + fbdo.pushName();

    //Due to some internal server error, ETag cannot get from pushTimestamp
    //Try to get ETag manually
    Serial.println("ETag: " + Firebase.RTDB.getETag(&fbdo, Path.c_str()));
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

void loop()
{
}