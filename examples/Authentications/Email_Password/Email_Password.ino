
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

/** This example will show how to authenticate as a user with Email and password.
 * 
 * You need to enable Email/Password provider.
 * In Firebase console, select Authentication, select Sign-in method tab, 
 * under the Sign-in providers list, enable Email/Password provider.
 * 
 * From this example, the user will be granted to access the specific location that matches 
 * the user uid.
 * 
 * This example will modify the database rules to set up the security rule which need to 
 * guard the unauthorized access with the user Email.
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

/* 2. Define the Firebase project host name (required) */
#define FIREBASE_PROJECT_HOST "PROJECT_ID.firebaseio.com"

/** 3. Define the API key
 * 
 * The API key (required) can be obtained since you created the project and set up 
 * the Authentication in Firebase console. Then you will get the API key from
 * Firebase project Web API key in Project settings, on General tab should show the
 * Web API Key.
 * 
 * You may need to enable the Identity provider at https://console.cloud.google.com/customer-identity/providers 
 * Select your project, click at ENABLE IDENTITY PLATFORM button.
 * The API key also available by click at the link APPLICATION SETUP DETAILS.
 * 
*/
#define API_KEY "API_KEY"

/* 4. Define the user Email and password that already registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/** 5. Define the database secret (optional)
 * 
 * This database secret needed only for this example to modify the database rules
 * 
 * If you edit the database rules yourself, this is not required.
*/
#define DATABASE_SECRET "DATABASE_SECRET"

/* 6. Define the Firebase Data object */
FirebaseData fbdo;

/* 7. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 8. Define the FirebaseConfig data for config data */
FirebaseConfig config;

String path = "";
unsigned long dataMillis = 0;
int count = 0;

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
    config.host = FIREBASE_PROJECT_HOST;
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    String base_path = "/UsersData2/";

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;

    /** Assign the maximum retry of token generation */
    config.max_token_generation_retry = 5;

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

    /** Now modify the database rules (if not yet modified)
     *
     * The user, Node1 in this case will be granted to read and write
     * at the curtain location i.e. "/UsersData/Node1" and we will also check the 
     * custom claims in the rules which must be matched.
     * 
     * If you database rules has been modified, please comment this code out.
     * 
     * The character $ is to make a wildcard variable (can be any name) represents any node key 
     * which located at some level in the rule structure and use as reference variable
     * in .read, .write and .validate rules
     * 
     * For this case $userId represents any xxxx node that places under UsersData node i.e.
     * /UsersData/xxxxx which xxxx is user UID or "Node1" in this case.
     * 
     * Please check your the database rules to see the changes after run the below code.
    */
    String var = "$userId";
    String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
    Firebase.RTDB.setReadWriteRules(&fbdo, base_path.c_str(), var.c_str(), val.c_str(), val.c_str(), DATABASE_SECRET);

    /** path for user data is now "/UsersData2/YOUR_USER_UID"
     * The user UID can be taken from auth.token.uid
    */
    path = base_path + auth.token.uid.c_str();
}

void loop()
{
    if (millis() - dataMillis > 5000 && Firebase.ready())
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
            printResult(fbdo);
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
