
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

/** Prerequisites
 *
 * Cloud Functions deployment requires the pay-as-you-go (Blaze) billing plan.
 *
 * IAM owner permission required for service account used and Cloud Build API must be enabled,
 * https://github.com/mobizt/Firebase-ESP-Client#iam-permission-and-api-enable
 */

/** This example shows how to get the IAM Policy for the Cloud Function.
 *
 * This operation required OAUth2.0 authentication.
 */

/** Due to the processing power in ESP8266 is weaker than ESP32, the OAuth2.0 token generation takes time then this example
 * will check for token to be ready in loop prior to get the IAM Policy.
 */

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/** 2. Define the Service Account credentials (required for token generation)
 *
 * This information can be taken from the service account JSON file.
 *
 * To download service account file, from the Firebase console, goto project settings,
 * select "Service accounts" tab and click at "Generate new private key" button
 */
#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

/* 3. Define the project location e.g. us-central1 or asia-northeast1 */
// https://firebase.google.com/docs/projects/locations
#define PROJECT_LOCATION "PROJECT_LOCATION"

/* 4. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

unsigned long dataMillis = 0;

/* The function to get the IAM Policy of the Cloud Function */
void getPolicy();

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

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the Service Account credentials */
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
    if (Firebase.ready() && !taskCompleted)
    {
        getPolicy();
        taskCompleted = true;
    }
}

/* The function to get the IAM Policy of the Cloud Function */
void getPolicy()
{
    // Assumed that the function named helloWorld is already created and deployed for the project.

    Serial.print("Get the IAM Policy of Googgle Cloud Function... ");

    if (Firebase.Functions.getIamPolicy(&fbdo, FIREBASE_PROJECT_ID /* project id */, PROJECT_LOCATION /* project location */, "helloWorld" /* function name */))
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
        Serial.println(fbdo.errorReason());
}
