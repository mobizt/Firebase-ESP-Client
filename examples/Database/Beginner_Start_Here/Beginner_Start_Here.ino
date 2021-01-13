
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

/* 4. Define FirebaseESP8266 data object for data sending and receiving */
FirebaseData fbdo;

/* 5. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 6. Define the FirebaseConfig data for config data */
FirebaseConfig config;

void setup()
{

  Serial.begin(115200);

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



  /* 7. Assign the project host and api key (required) */
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;

  /* 8. Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  /* 9. Initialize the library with the autentication data */
  Firebase.begin(&config, &auth);


  /* 10. Enable auto reconnect the WiFi when connection lost */
  Firebase.reconnectWiFi(true);

  /* 11. Try to set int data to Firebase */
  //The set function returns bool for the status of operation
  //fbdo requires for sending the data and pass as the pointer
  if(Firebase.RTDB.setInt(&fbdo, "/LED_Status", 1))
  {
    //Success
     Serial.println("Set int data success");

  }else{
    //Failed?, get the error reason from fbdo

    Serial.print("Error in setInt, ");
    Serial.println(fbdo.errorReason());
  }


  /* 12. Try to get int data from Firebase */
  //The get function returns bool for the status of operation
  //fbdo requires for receiving the data
  if(Firebase.RTDB.getInt(&fbdo, "/LED_Status"))
  {
    //Success
    Serial.print("Get int data success, int = ");
    Serial.println(fbdo.intData());

  }else{
    //Failed?, get the error reason from fbdo

    Serial.print("Error in getInt, ");
    Serial.println(fbdo.errorReason());
  }

  /*

  In case where you want to set other data types i.e. bool, float, double and String, you can use setBool, setFloat, setDouble and setString.
  If you want to get data which you known its type at specific node, use getInt, getBool, getFloat, getDouble, getString.
  If you don't know the data type at specific node, use get and check its type.

  The following shows how to get the variant data

  */

 if(Firebase.RTDB.get(&fbdo, "/LED_Status"))
  {
    //Success
    Serial.print("Get variant data success, type = ");
    Serial.println(fbdo.dataType());

    if(fbdo.dataType() == "int"){
      Serial.print("data = ");
      Serial.println(fbdo.intData());
    }else if(fbdo.dataType() == "bool"){
      if(fbdo.boolData())
        Serial.println("data = true");
      else
        Serial.println("data = false");
    }

  }else{
    //Failed?, get the error reason from fbdo

    Serial.print("Error in get, ");
    Serial.println(fbdo.errorReason());
  }

  /*

  If you want to get the data in realtime instead of using get, see the stream examples.
  If you want to work with JSON, see the FirebaseJson, jsonObject and jsonArray examples.

  If you have questions or found the bugs, feel free to open the issue here https://github.com/mobizt/Firebase-ESP-Client

  */




}

void loop()
{
}

