
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

/** This example will show how to sign in with Email and password and send the verification Email.
 * 
 * You need to enable Email/Password provider.
 * In Firebase console, select Authentication, select Sign-in method tab, 
 * under the Sign-in providers list, enable Email/Password provider.
 * 
 * In the database rules, you can guard the unverified user from access by adding "auth.token.email_verified == true"
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

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/** 2. Define the API key
 * 
 * The API key can be obtained since you created the project and set up 
 * the Authentication in Firebase console.
 * 
 * You may need to enable the Identity provider at https://console.cloud.google.com/customer-identity/providers 
 * Select your project, click at ENABLE IDENTITY PLATFORM button.
 * The API key also available by click at the link APPLICATION SETUP DETAILS.
 * 
*/
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that already registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 4. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 5. Define the Firebase Data object */
FirebaseData fbdo;

/* 6. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 7. Define the FirebaseConfig data for config data */
FirebaseConfig config;

String path = "";
unsigned long dataMillis = 0;
int count = 0;
bool signupOK = false;

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

    /* Assign the API key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.reconnectWiFi(true);

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    String base_path = "/UsersData2/";

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

    /** path for user data is now "/UsersData2/YOUR_USER_UID"
     * The user UID can be taken from auth.token.uid
    */
    path = base_path + auth.token.uid.c_str();

    Serial.println("------------------------------------");
    Serial.println("Send Email verification...");

    /* Send verification Email using internal created id token after Email/Password signed in */
    if (Firebase.sendEmailVerification(&config))
    {
        Serial.printf("Success, the verification link was sent to %s\n\n", auth.user.email.c_str());
    }
    else
    {
        Serial.printf("Failed, %s\n\n", config.signer.verificationError.message.c_str());
    }
    
}

void loop()
{
    if (millis() - dataMillis > 5000 && Firebase.ready())
    {
        dataMillis = millis();

        Serial.println("------------------------------------");
        Serial.println("Set int test...");

        String Path = path + "/int";

        if (Firebase.RTDB.set(&fbdo, Path.c_str(), count++))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.println("ETag: " + fbdo.ETag());
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
    }
}
