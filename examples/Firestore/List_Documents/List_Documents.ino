
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

//This example shows how to get the documents in a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

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

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "PROJECT_ID"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
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

    //Should run the Create_Documents.ino prior to test this example to create the documents in the collection Id at a0/b0/c0

    //a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id id in the document b0.
    String collectionId = "a0/b0/c0";

    Serial.println("------------------------------------");
    Serial.println("List the documents in a collection...");

    if (Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, collectionId.c_str(), 3 /* The maximum number of documents to return */, "" /* The nextPageToken value returned from a previous List request, if any. */, "" /* The order to sort results by. For example: priority desc, name. */, "count" /* the field name to mask */, false /* showMissing, iIf the list should show missing documents. A missing document is a document that does not exist but has sub-documents. */))
    {
        Serial.println("PASSED");
        Serial.println("------------------------------------");
        Serial.println(fbdo.payload());
        Serial.println("------------------------------------");
        Serial.println();
    }
    else
    {
        //if the showMissing parameter of listDocuments is true, and the error shows "Missing or insufficient permissions."
        //The OAuth2.0 authentication is required to perform this operation.
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }

}

void loop()
{
}
