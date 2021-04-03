
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

/** This example will show how to send the reset password link to user Email.
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

/* 2. Define the Firebase project host name (required) */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"

/** 3. Define the API key
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

/* 4. Define the user Email to reset the password */
#define USER_EMAIL "USER_EMAIL"


/* 5. Define the FirebaseConfig data for config data */
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

    /* Assign the project host and API key (required) */
    config.host = FIREBASE_HOST;
    config.api_key = API_KEY;

    Firebase.reconnectWiFi(true);

    Serial.println("------------------------------------");
    Serial.println("Send Email reset password link...");

    /* Send password reset link to user Email */
    if (Firebase.sendResetPassword(&config, USER_EMAIL))
    {
        Serial.printf("Success, the reset password link was sent to %s\n\n", USER_EMAIL);
    }
    else
    {
        Serial.printf("Failed, %s\n\n", config.signer.resetPswError.message.c_str());
    }
}

void loop()
{
    
}
