
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
#define FIREBASE_AUTH "DATABASE_SECRET"

/* 5. Define the Firebase Data object */
FirebaseData fbdo;

/* 6. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 7. Define the FirebaseConfig data for config data */
FirebaseConfig config;

/* The calback function to print the token generation status */
void tokenStatusCallback(TokenInfo info);

/* The helper function to modify the database rules (optional) */
void prepareDatabaseRules(const char *path, const char *var, const char *readVal, const char *writeVal);

/* The function to print the operating results */
void printResult(FirebaseData &data);

/* The helper function to get the token status string */
String getTokenStatus(struct token_info_t info);

/* The helper function to get the token type string */
String getTokenType(struct token_info_t info);

/* The helper function to get the token error string */
String getTokenError(struct token_info_t info);

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

    /* Assign the certificate data (optional) */
    //config.cert.data = rootCACert;

    /* Assign the project host and api key (required) */
    config.host = FIREBASE_HOST;
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

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    /* path for user data is now "/UsersData/Node1" */
    String base_path = "/UsersData/";

    /** Now modify the database rules (if not yet modified)
     *
     * The user, Node1 in this case will be granted to read and write
     * at the curtain location i.e. "/UsersData/Node1" and we will also check the 
     * custom claims in the rules which must be matched.
     * 
     * To modify the database rules in this exanple, we need the full access rights then 
     * using the database secret in prepareDatabaseRules function to sign in.
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
    prepareDatabaseRules(base_path.c_str(), var.c_str(), val.c_str(), val.c_str());

    /** Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;

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

    path = base_path + auth.token.uid.c_str();
}

void loop()
{
    if (millis() - dataMillis > 5000)
    {
        dataMillis = millis();

        /* Get the token status */
        TokenInfo info = Firebase.authTokenInfo();
        if (info.status == token_status_ready)
        {
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
}

void tokenStatusCallback(TokenInfo info)
{
    /** fb_esp_auth_token_status enum
     * token_status_uninitialized,
     * token_status_on_initialize,
     * token_status_on_signing,
     * token_status_on_request,
     * token_status_on_refresh,
     * token_status_ready,
     * token_status_error
    */
    if (info.status == token_status_error)
    {
        Serial.printf("Token info: type = %s, status = %s\n", getTokenType(info).c_str(), getTokenStatus(info).c_str());
        Serial.printf("Token error: %s\n", getTokenError(info).c_str());
    }
    else
    {
        Serial.printf("Token info: type = %s, status = %s\n", getTokenType(info).c_str(), getTokenStatus(info).c_str());
    }
}

/* The helper function to modify the database rules (optional) */
void prepareDatabaseRules(const char *path, const char *var, const char *readVal, const char *writeVal)
{
    //We will sign in using legacy token (database secret) for full RTDB access
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);

    Serial.println("------------------------------------");
    Serial.println("Read database rules...");
    if (Firebase.RTDB.getRules(&fbdo))
    {
        FirebaseJsonData result;
        FirebaseJson &json = fbdo.jsonObject();
        bool rd = false, wr = false;

        String _path = "rules";
        if (path[0] != '/')
            _path += "/";
        _path += path;
        _path += "/";
        _path += var;

        if (strlen(readVal) > 0)
        {
            rd = true;
            json.get(result, _path + "/.read");
            if (result.success)
                if (strcmp(result.stringValue.c_str(), readVal) == 0)
                    rd = false;
        }

        if (strlen(writeVal) > 0)
        {
            wr = true;
            json.get(result, _path + "/.write");
            if (result.success)
                if (strcmp(result.stringValue.c_str(), writeVal) == 0)
                    wr = false;
        }

        //modify if the rules changed or does not exist.
        if (wr || rd)
        {
            FirebaseJson js;
            std::string s;
            if (rd)
                js.add(".read", readVal);

            if (wr)
                js.add(".write", writeVal);

            Serial.println("Set database rules...");
            json.set(_path, js);
            String rules = "";
            json.toString(rules, true);
            if (!Firebase.RTDB.setRules(&fbdo, rules.c_str()))
            {
                Serial.println("Failed to edit the database rules, " + fbdo.errorReason());
            }
        }

        json.clear();
    }
    else
    {
        Serial.println("Failed to read the database rules, " + fbdo.errorReason());
    }

    //Set to empty string to sign in with other tokens
    config.signer.tokens.legacy_token = "";
}

void printResult(FirebaseData &data)
{

    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
            if (type == FirebaseJson::JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == FirebaseJson::JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == FirebaseJson::JSON_FLOAT)
                Serial.println(jsonData.floatValue);
            else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
                     jsonData.typeNum == FirebaseJson::JSON_NULL ||
                     jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
                     jsonData.typeNum == FirebaseJson::JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
    else if (data.dataType() == "blob")
    {

        Serial.println();

        for (size_t i = 0; i < data.blobData().size(); i++)
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();

            if (i < 16)
                Serial.print("0");

            Serial.print(data.blobData()[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    else if (data.dataType() == "file")
    {

        Serial.println();

        File file = data.fileStream();
        int i = 0;

        while (file.available())
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();

            int v = file.read();

            if (v < 16)
                Serial.print("0");

            Serial.print(v, HEX);
            Serial.print(" ");
            i++;
        }
        Serial.println();
        file.close();
    }
    else
    {
        Serial.println(data.payload());
    }
}

/* The helper function to get the token type string */
String getTokenType(struct token_info_t info)
{
    switch (info.type)
    {
    case token_type_undefined:
        return "undefined";

    case token_type_legacy_token:
        return "legacy token";

    case token_type_id_token:
        return "id token";

    case token_type_custom_token:
        return "custom token";

    case token_type_oauth2_access_token:
        return "OAuth2.0 access token";

    default:
        break;
    }
    return "undefined";
}

/* The helper function to get the token status string */
String getTokenStatus(struct token_info_t info)
{
    switch (info.status)
    {
    case token_status_uninitialized:
        return "uninitialized";

    case token_status_on_initialize:
        return "on initializing";

    case token_status_on_signing:
        return "on signing";

    case token_status_on_request:
        return "on request";

    case token_status_on_refresh:
        return "on refreshing";

    case token_status_ready:
        return "ready";

    case token_status_error:
        return "error";

    default:
        break;
    }
    return "uninitialized";
}

/* The helper function to get the token error string */
String getTokenError(struct token_info_t info)
{
    String s = "code: ";
    s += String(info.error.code);
    s += ", message: ";
    s += info.error.message.c_str();
    return s;
}
