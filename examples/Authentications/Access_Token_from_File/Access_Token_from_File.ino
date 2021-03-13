
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

/** This example will show how to authenticate as admin using 
 * the Service Account file to create the access token to sign in internally.
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

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 5. Define the FirebaseConfig data for config data */
FirebaseConfig config;

/* The calback function to print the token generation status */
void tokenStatusCallback(TokenInfo info);

/* The function to print the operating results */
void printResult(FirebaseData &data);

/* The helper function to get the token status string */
String getTokenStatus(struct token_info_t info);

/* The helper function to get the token type string */
String getTokenType(struct token_info_t info);

/* The helper function to get the token error string */
String getTokenError(struct token_info_t info);

String path = "/Test";
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

    /* Assign the certificate file (optional) */
    config.cert.file = "/gsr2.pem";
    config.cert.file_storage = mem_storage_type_flash; //or mem_storage_type_sd

    /* Assign the project host (required) */
    config.host = FIREBASE_HOST;

    /* The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h. */

    /* Assign the sevice account JSON file and the file storage type (required) */
    config.service_account.json.path = "/service_account_file.json"; //change this for your json file
    config.service_account.json.storage_type = mem_storage_type_flash;

    /** The user UID set to empty to sign in as admin */
    auth.token.uid = "";

    /** The scope of the OAuth2.0 authentication 
     * If you wan't this access token for others Google Cloud Services.
    */
    //config.signer.tokens.scope = "Google Scope 1 Url, Google Scope 2 Url,..";

    Firebase.reconnectWiFi(true);

    /** Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;

    /** Assign the maximum retry of token generation */
    config.max_token_generation_retry = 5;

    /* Now we start to signin using access token */

    /** Initialize the library with the Firebase authen and config.
     *  
     * The device time will be set by sending request to the NTP server 
     * befor token generation and exchanging.
     * 
     * The signed RSA256 jwt token will be created and used for access token exchanging.
     * 
     * Theses process may take time to complete.
    */
    Firebase.begin(&config, &auth);

    /* The access token (C++ string) can be accessed from config.signer.tokens.access_token. */
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
