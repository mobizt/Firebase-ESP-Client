
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

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name (required) */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"

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
#define FIREBASE_AUTH "DATABASE_SECRET"

/* 6. Define the Firebase Data object */
FirebaseData fbdo;

/* 7. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 8. Define the FirebaseConfig data for config data */
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

    /* Assign the project host and api key (required) */
    config.host = FIREBASE_HOST;
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    String base_path = "/UsersData2/";

    /** Modify the database rules if not yet modified (optional)
     *
     * The user in this case will be granted access to read and write
     * at the curtain path i.e. "/UsersData2/xxxx" which xxxx is uid of a user.
     * 
     * Please check your the database rules to see the changes.
     * 
     * To modify the database rules in this exanple, we need the full access rights then 
     * using the database secret in prepareDatabaseRules function to sign in.
     * 
     * The character $ is to make a wildcard variable (can be any name) represents any node key 
     * which located at some level in the rule structure and use as reference variable
     * in .read, .write and .validate rules
     * 
     * If you database rules has been modified, please comment this code out
    */
    String var = "$user";
    String val = "(auth.uid === $user)";
    prepareDatabaseRules(base_path.c_str(), var.c_str(), val.c_str(), val.c_str());

    /** Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;

    /** Assign the maximum retry of token generation */
    config.max_token_generation_retry = 5;

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

    /** path for user data is now "/UsersData2/YOUR_USER_UID"
     * The user UID can be taken from auth.token.uid
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
