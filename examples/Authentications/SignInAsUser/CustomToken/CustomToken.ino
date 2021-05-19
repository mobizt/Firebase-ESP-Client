
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

/** This example will show how to authenticate as user using 
 * the hard coded Service Account to create the custom token to sign in.
 * 
 * From this example, the user will be granted to access the specific location that matches 
 * the unique user ID (uid) assigned in the token.
 * 
 * The anonymous user with user UID will be created if not existed.
 * 
 * This example will modify the database rules to set up the security rule which need to 
 * guard the unauthorized access with the uid and custom claims assigned in the token.
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

/** 3. Define the Service Account credentials (required for token generation)
 * 
 * This information can be taken from the service account JSON file.
 * 
 * To download service account file, from the Firebase console, goto project settings, 
 * select "Service accounts" tab and click at "Generate new private key" button
*/
#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

/* 4. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* This is Google root CA certificate */
/*
const char rootCACert[] PROGMEM = "-----BEGIN CERTIFICATE-----\n"
                                  "MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"
                                  "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n"
                                  "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n"
                                  "MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n"
                                  "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"
                                  "hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n"
                                  "v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n"
                                  "eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n"
                                  "tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"
                                  "C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n"
                                  "zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n"
                                  "mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n"
                                  "V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n"
                                  "bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n"
                                  "3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n"
                                  "J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n"
                                  "291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n"
                                  "ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"
                                  "AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n"
                                  "TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"
                                  "-----END CERTIFICATE-----\n";
*/

/** 4. Define the database secret (optional)
 * 
 * This database secret needed only for this example to modify the database rules
 * 
 * If you edit the database rules yourself, this is not required.
*/
#define DATABASE_SECRET "DATABASE_SECRET"

/* 5. Define the Firebase Data object */
FirebaseData fbdo;

/* 6. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 7. Define the FirebaseConfig data for config data */
FirebaseConfig config;

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

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the certificate data (optional) */
    //config.cert.data = rootCACert;

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the sevice account credentials and private key (required) */
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    /** Assign the unique user ID (uid) (required)
     * This uid will be compare to the auth.uid variable in the database rules.
     * 
     * If the assigned uid (user UID) was not existed, the new user will be created.
     * 
     * If the uid is empty or not assigned, the library will create the OAuth2.0 access token 
     * instead.
     * 
     * With OAuth2.0 access token, the device will be signed in as admin which has 
     * the full ggrant access and no database rules and custom claims are applied.
     * This similar to sign in using the database secret but no admin rights.
    */
    auth.token.uid = "Node1";

    /** Assign the custom claims (optional)
     * This uid will be compare to the auth.token.premium_account variable
     * (for this case) in the database rules.
    */
    auth.token.claims.add("premium_account", true);
    auth.token.claims.add("admin", true);

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    /* path for user data is now "/UsersData/Node1" */
    String base_path = "/UsersData/";

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    /** Assign the maximum retry of token generation */
    config.max_token_generation_retry = 5;

    /* Now we start to signin using custom token */

    /** Initialize the library with the Firebase authen and config.
     *  
     * The device time will be set by sending request to the NTP server 
     * befor token generation and exchanging.
     * 
     * The signed RSA256 jwt token will be created and used for id token exchanging.
     * 
     * Theses process may take time to complete.
    */
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
     * For this case $userId represents any <user uid> node that places under UsersData node i.e.
     * /UsersData/<user uid> which <user uid> is user UID or "Node1" in this case.
     * 
     * Please check your the database rules to see the changes after run the below code.
    */
    String var = "$userId";
    String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
    Firebase.RTDB.setReadWriteRules(&fbdo, base_path.c_str(), var.c_str(), val.c_str(), val.c_str(), DATABASE_SECRET);

    /** 
     * The custom token which created internally in this library will use 
     * to exchange with the id token returns from the server.
     * 
     * The id token is the token which used to sign in as a user.
     * 
     * The id token was already saved to the config data (FirebaseConfig data variable) that 
     * passed to the Firebase.begin function.
     *  
     * The id token (C++ string) can be accessed from config.signer.tokens.id_token.
    */
}

void loop()
{
    if (millis() - dataMillis > 5000 && Firebase.ready())
    {
        dataMillis = millis();
        String path = "/UsersData/";
        path += auth.token.uid.c_str(); //<- user uid or "Node1"
        path += "/test/int";
        Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, path.c_str(), count++) ? "ok" : fbdo.errorReason().c_str());
    }
}