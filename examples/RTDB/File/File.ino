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

//This example shows how to store and read binary data from file on SD card to database.

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

File file;

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

  //Mount SD card
  if (!SD.begin(15))
  {
    Serial.println("SD Card mounted failed");
    return;
  }

  //Delete demo files
  if (SD.exists("/file1.txt"))
    SD.remove("/file1.txt");

  if (SD.exists("/file2.txt"))
    SD.remove("/file2.txt");

  if (SD.exists("/file3.txt"))
    SD.remove("/file3.txt");

  Serial.println("------------------------------------");
  Serial.println("Set file data test...");

  //Write demo data to file
  file = SD.open("/file1.txt", FILE_WRITE);
  uint8_t v = 0;
  for (int i = 0; i < 512; i++)
  {
    file.write(v);
    v++;
  }

  file.close();

  //In case of Root CA was set, set this option to false to disable low memory for secured mode BearSSL to support large file data
  //Firebase.lowMemBSSL(false);

  String Path = path + "/Binary/File/data";

  //Set file (read file from SD card and set to database)
  //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
  if (Firebase.RTDB.setFile(&fbdo, mem_storage_type_sd, Path.c_str(), "/file1.txt"))
  {
    Serial.println("PASSED");
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Get file data test...");

  //Get file (download file to SD card)
  //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
  if (Firebase.RTDB.getFile(&fbdo, mem_storage_type_sd, Path.c_str(), "/file2.txt"))
  {

    Serial.println("PASSED");
    Serial.println("DATA");

    //Readout the downloaded file
    file = SD.open("/file2.txt", FILE_READ);
    int i = 0;

    while (file.available())
    {
      if (i > 0 && i % 16 == 0)
        Serial.println();

      v = file.read();

      if (v < 16)
        Serial.print("0");

      Serial.print(v, HEX);
      Serial.print(" ");
      i++;
    }
    Serial.println();
    Serial.println("------------------------------------");
    Serial.println();
    file.close();
  }
  else
  {

    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Append file data test...");

  if (SD.exists("/file1.txt"))
    SD.remove("/file1.txt");

  //Write demo data to file
  file = SD.open("/file1.txt", FILE_WRITE);
  for (int i = 255; i >= 0; i--)
    file.write((uint8_t)i);

  file.close();

  Path = path + "/Binary/File/Logs";

  //Append file data to database
  //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
  if (Firebase.RTDB.pushFile(&fbdo, mem_storage_type_sd, Path.c_str(), "/file1.txt"))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("PUSH NAME: " + fbdo.pushName());
    Serial.println("------------------------------------");

    Serial.println();

    Serial.println("------------------------------------");
    Serial.println("Get appended file data test...");

    Path = path + "/Binary/File/Logs/" + fbdo.pushName();

    //Get the recently appended file (download file to SD card)
    //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
    if (Firebase.RTDB.getFile(&fbdo, mem_storage_type_sd, Path.c_str(), "/file3.txt"))
    {

      Serial.println("PASSED");
      Serial.println("DATA");

      //Readout the downloaded file
      file = SD.open("/file3.txt", FILE_READ);
      int i = 0;
      int idx = 0;

      while (file.available())
      {
        i = file.read();
        if (i < 16)
          Serial.print("0");

        Serial.print(i, HEX);
        Serial.print(" ");

        if (idx > 0 && (idx + 1) % 16 == 0)
          Serial.println();
        idx++;
      }
      Serial.println();
      Serial.println("------------------------------------");
      Serial.println();
      file.close();
    }
    else
    {

      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.fileTransferError());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void loop()
{
}
