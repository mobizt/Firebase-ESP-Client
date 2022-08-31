
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

/** This example will show how to authenticate as user using
 * the Service Account file to create the custom token to sign in internally.
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

/** 4. Define the database secret (optional)
 *
 * This need only for this example to edit the database rules for you and required the
 * authentication bypass.
 *
 * If you edit the database rules manually, this legacy database secret is not required.
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

    /* Assign the certificate file (optional) */
    // config.cert.file = "/cert.cer";
    // config.cert.file_storage = mem_storage_type_flash;

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h. */

    /* Assign the sevice account JSON file and the file storage type (required) */
    config.service_account.json.path = "/service_account_file.json";   // change this for your json file
    config.service_account.json.storage_type = mem_storage_type_flash; // or  mem_storage_type_sd

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

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    /** Assign the custom claims (optional)
     * This uid will be compare to the auth.token.premium_account variable
     * (for this case) in the database rules.
     */
    FirebaseJson claims;
    claims.add("premium_account", true);
    claims.add("admin", true);
    auth.token.claims = claims.raw();

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    /* path for user data is now "/UsersData/Node1" */
    String base_path = "/UsersData/";

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    /** To set system time with the timestamp from RTC
     * The internal NTP server time acquisition
     * of token generation process will be skipped,
     * if the system time is already set.
     *
     * sec is the seconds from midnight Jan 1, 1970
     */
    // Firebase.setSystemTime(sec);

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
    Firebase.RTDB.setReadWriteRules(&fbdo, base_path, var, val, val, DATABASE_SECRET);

    /**
     * The custom token which created internally in this library will use
     * to exchange with the id token returns from the server.
     *
     * The id token is the token which used to sign in as a user.
     *
     * The id token was already saved to the config data (FirebaseConfig data variable) that
     * passed to the Firebase.begin function.
     *
     * The id token can be accessed from Firebase.getToken().
     * 
     * The refresh token can be accessed from Firebase.getRefreshToken().
     */
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (millis() - dataMillis > 5000 && Firebase.ready())
    {
        dataMillis = millis();
        String path = "/UsersData/";
        path += auth.token.uid.c_str(); //<- user uid is "Node1"
        path += "/test/int";
        Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, path, count++) ? "ok" : fbdo.errorReason().c_str());
    }
}
