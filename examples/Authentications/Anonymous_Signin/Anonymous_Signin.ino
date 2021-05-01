
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

/** This example will show how to authenticate as a anonymous user.
 * 
 * You need to enable Anonymous provider.
 * In Firebase console, select Authentication, select Sign-in method tab, 
 * under the Sign-in providers list, enable Anonymous provider.
 * 
 * Warning: this example will create a new anonymous user with 
 * different UID every time you run this example.
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

/* 3. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the Firebase Data object */
FirebaseData fbdo;

/* 5. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 6. Define the FirebaseConfig data for config data */
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

    Serial.println("------------------------------------");
    Serial.println("Sign up new user...");

    /** To sign in as anonymous user, just sign up as anonymous user
     * with blank email and password.
     * 
     * The Anonymous provider must be enabled.
     * 
     * To enable Anonymous provider,
     * from Firebase console, select Authentication, select Sign-in method tab, 
     * under the Sign-in providers list, enable Anonymous provider.
     * 
     * Warning: this will create anonymous user everytime you call this function 
     */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("Success");
        signupOK = true;

        /** if the database rules were set as in the example "Email_Password.ino"
         * This new user can be access the following location.
         * 
         * The new user UID can be taken from auth.token.uid
        */
        path = "/UsersData2/";
        path += auth.token.uid.c_str();
    }
    else
    {
        Serial.printf("Failed, %s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    /** The id token (C++ string) will be available from config.signer.tokens.id_token
     * if the sig-up was successful. 
     * 
     * Now you can initialize the library using the internal created credentials.
     * 
     * If the sign-up was failed, the following function will initialize because 
     * the internal authentication credentials are not created.
    */
    Firebase.begin(&config, &auth);
}

void loop()
{
    if (millis() - dataMillis > 5000 && signupOK && Firebase.ready())
    {
        dataMillis = millis();

        Serial.println("------------------------------------");
        Serial.println("Set int test...");

        String node = path + "/int";

        if (Firebase.RTDB.set(&fbdo, node.c_str(), count++))
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
