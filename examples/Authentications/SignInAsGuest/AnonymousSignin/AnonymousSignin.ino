
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example will show how to authenticate as a anonymous user and then delete this user from project.
 *
 * You need to enable Anonymous provider.
 * In Firebase console, select Authentication, select Sign-in method tab,
 * under the Sign-in providers list, enable Anonymous provider.
 *
 * Warning: this example will create a new anonymous user with
 * different UID every time you run this example.
 *
 */
#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

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

unsigned long dataMillis = 0;
int count = 0;
bool signupOK = false;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void setup()
{

    Serial.begin(115200);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the API key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    /** To sign in as anonymous user, just sign up as anonymous user
     * with blank email and password.
     *
     * The Anonymous provider must be enabled.
     *
     * To enable Anonymous provider,
     * from Firebase console, select Authentication, select Sign-in method tab,
     * under the Sign-in providers list, enable Anonymous provider.
     *
     * Warning: this will create anonymous user everytime you called this function and your user list
     * will grow up and the anonymous users stay in the user list after it created, we recommend to delete
     * this user after used.
     *
     * https://stackoverflow.com/questions/38694015/what-happens-to-firebase-anonymous-users
     * https://stackoverflow.com/questions/39640574/how-to-bulk-delete-firebase-anonymous-users
     */

    Serial.print("Sign up new user... ");

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;

        /** if the database rules were set as in the example "EmailPassword.ino"
         * This new user can be access the following location.
         *
         * "/UserData/<user uid>"
         *
         * The new user UID or <user uid> can be taken from auth.token.uid
         */
    }
    else
        Serial.printf("%s\n", config.signer.signupError.message.c_str());

    // If the signupError.message showed "ADMIN_ONLY_OPERATION", you need to enable Anonymous provider in project's Authentication.

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

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

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (millis() - dataMillis > 5000 && signupOK && Firebase.ready())
    {
        dataMillis = millis();
        String path = auth.token.uid.c_str(); //<- user uid
        path += "/test/int";
        Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, path, count++) ? "ok" : fbdo.errorReason().c_str());

        if (count == 10)
        {
            Serial.print("Delete user... ");
            if (Firebase.deleteUser(&config, &auth /* third argument can be the id token of active user to delete or leave it blank to delete current user */))
            {
                Serial.println("ok");
            }
            else
                Serial.printf("%s\n", config.signer.deleteError.message.c_str());
        }
    }
}
