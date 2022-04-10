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

// This example shows how error retry and queues work.

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

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

std::vector<uint8_t> myblob;
double mydouble = 0;

uint32_t queueID[20];
uint8_t qIdx = 0;

int queueCnt = 0;

void callback(QueueInfo queueinfo)
{

  if (queueinfo.isQueueFull())
  {
    Serial.println("Queue is full");
  }

  Serial.print("Remaining queues: ");
  Serial.println(queueinfo.totalQueues());

  Serial.print("Being processed queue ID: ");
  Serial.println(queueinfo.currentQueueID());

  Serial.print("Data type:");
  Serial.println(queueinfo.dataType());

  Serial.print("Method: ");
  Serial.println(queueinfo.firebaseMethod());

  Serial.print("Path: ");
  Serial.println(queueinfo.dataPath());

  Serial.println();
}

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

  // Or use legacy authenticate method
  // Firebase.begin(DATABASE_URL, DATABASE_SECRET);

  Firebase.reconnectWiFi(true);

  // Open and retore Firebase Error Queues from file.
  // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.

  if (Firebase.RTDB.errorQueueCount(&fbdo, "/test.txt", mem_storage_type_flash) > 0)
  {
    Firebase.RTDB.restoreErrorQueue(&fbdo, "/test.txt", mem_storage_type_flash);
    Firebase.RTDB.deleteStorageFile("/test.txt", mem_storage_type_flash);
  }

  // Set maximum Firebase read/store retry operation (0 - 255) in case of
  // network problems and buffer overflow
  Firebase.RTDB.setMaxRetry(&fbdo, 3);

  // Set the maximum Firebase Error Queues in collection (0 - 255).
  // Firebase read/store operation causes by network problems and buffer
  // overflow will be added to Firebase Error Queues collection.
  Firebase.RTDB.setMaxErrorQueue(&fbdo, 10);

  Firebase.RTDB.beginAutoRunErrorQueue(&fbdo, callback);

  // Firebase.RTDB.beginAutoRunErrorQueue(&fbdo);
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    Serial.printf("Set double... %s\n", Firebase.RTDB.setDouble(&fbdo, "/test/double", 340.123456789) ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() != FIREBASE_ERROR_HTTP_CODE_OK && Firebase.RTDB.getErrorQueueID(&fbdo) > 0)
    {
      Serial.printf("Error Queue ID: %d\n", (int)Firebase.RTDB.getErrorQueueID(&fbdo));
      queueID[qIdx] = Firebase.RTDB.getErrorQueueID(&fbdo);
      qIdx++;
    }

    // Create demo data
    uint8_t data[256];
    for (int i = 0; i < 256; i++)
      data[i] = i;

    Serial.printf("Set Blob... %s\n", Firebase.RTDB.setBlob(&fbdo, "/test/blob", data, sizeof(data)) ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() != FIREBASE_ERROR_HTTP_CODE_OK && Firebase.RTDB.getErrorQueueID(&fbdo) > 0)
    {
      Serial.printf("Error Queue ID: %d\n", (int)Firebase.RTDB.getErrorQueueID(&fbdo));
      queueID[qIdx] = Firebase.RTDB.getErrorQueueID(&fbdo);
      qIdx++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("--------------------------------------------------------------------------");
      Serial.println("To test error queue, turn off WiFi AP to make error in the next operation");
      Serial.println("--------------------------------------------------------------------------");
      Serial.println();
      delay(10000);
    }

    Serial.printf("Get double... %s\n", Firebase.RTDB.getDouble(&fbdo, "/test/double", &mydouble) ? "ok" : fbdo.errorReason().c_str());
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printResult(fbdo);
    else
    {
      if (Firebase.RTDB.getErrorQueueID(&fbdo) > 0)
      {
        Serial.printf("Error Queue ID: %d\n", (int)Firebase.RTDB.getErrorQueueID(&fbdo));
        queueID[qIdx] = Firebase.RTDB.getErrorQueueID(&fbdo);
        qIdx++;
      }
    }

    Serial.printf("Get blob... %s\n", Firebase.RTDB.getBlob(&fbdo, "/test/blob", &myblob) ? "ok" : fbdo.errorReason().c_str());
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printResult(fbdo);
    else
    {
      if (Firebase.RTDB.getErrorQueueID(&fbdo) > 0)
      {
        Serial.printf("Error Queue ID: %d\n", (int)Firebase.RTDB.getErrorQueueID(&fbdo));
        queueID[qIdx] = Firebase.RTDB.getErrorQueueID(&fbdo);
        qIdx++;
      }
    }

    if (Firebase.RTDB.errorQueueCount(&fbdo) > 0)
    {
      Serial.println("-----------------------------------------------------------------------------");
      Serial.println("Now turn on WiFi hotspot or router to process these queues");
      Serial.println("-----------------------------------------------------------------------------");
      Serial.println();

      // Save Error Queues to file
      // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
      Firebase.RTDB.saveErrorQueue(&fbdo, "/test.txt", mem_storage_type_flash);
    }

    // Stop error queue auto run process
    // Firebase.RTDB.endAutoRunErrorQueue(&fbdo);

    queueCnt = Firebase.RTDB.errorQueueCount(&fbdo);
  }

  /*

    //if Firebase.RTDB.beginAutoRunErrorQueue was not call,
    //to manaul run the Firebase Error Queues, just call
    //Firebase.RTDB.processErrorQueue in loop


    Firebase.RTDB.processErrorQueue(&fbdo);

    delay(1000);

    if (Firebase.RTDB.isErrorQueueFull(&fbdo))
    {
      Serial.println("Queue is full");
    }

    Serial.print("Remaining queues: ");
    Serial.println(Firebase.RTDB.errorQueueCount(&fbdo));

    for (uint8_t i = 0; i < qIdx; i++)
    {
      Serial.print("Error Queue ");
      Serial.print(queueID[i]);
      if (Firebase.RTDB.isErrorQueueExisted(&fbdo, queueID[i]))
        Serial.println(" is queuing");
      else
        Serial.println(" is done");
    }
    Serial.println();

  */

  if (queueCnt > 0)
  {
    if (mydouble > 0)
    {
      Serial.println("Double Data gets from Queue");
      printf("%.9lf\n", mydouble);
      Serial.println();
      mydouble = 0;
    }

    if (myblob.size() > 0)
    {
      Serial.println("Blob Data gets from Queue");
      for (size_t i = 0; i < myblob.size(); i++)
      {
        if (i > 0 && i % 16 == 0)
          Serial.println();
        if (myblob[i] < 16)
          Serial.print("0");
        Serial.print(myblob[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      Serial.println();
      myblob.clear();
    }
  }
}
