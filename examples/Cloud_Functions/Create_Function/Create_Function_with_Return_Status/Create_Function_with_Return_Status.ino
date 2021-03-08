
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

/** Prerequisites
 * 
 * Cloud Functions deployment requires the pay-as-you-go (Blaze) billing plan.
 * 
 * IAM owner permission required for service account used and Cloud Build API must be enabled,
 * https://github.com/mobizt/Firebase-ESP-Client#iam-permission-and-api-enable
*/

/* Cloud Functions deployment requires the pay-as-you-go (Blaze) billing plan. */

/** This example shows how to create (deploy) the Cloud Function. 
 * 
 * This operation required OAUth2.0 authentication.
 * 
 * The callback function show the progress of deployment.
*/

/** The Cloud Function source code files to deploy with this example will be compress as a single zip archive.
 * 
 * This zip file can be stored in the Firebase Storage data bucket or the repository or in local device storage e.g. flash and SD memory.
 * In case the archive file in the local memory was choosn, the file will be upload to the Google Cloud Storage bucket automatically in the creation process.
*/

/** Due to the processing power in ESP8266 is weaker than ESP32, the OAuth2.0 token generation takes time then this example
 * will check for token to be ready in loop prior to create the Cloud Function.
 * 
 * The Cloud Function creation (deploy) is the long running operation,
 * the final result may fail due to bugs in the user function, missing dependencies,
 * and incorrect configurations.
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

/* 4. Define the project location e.g. us-central1 or asia-northeast1 */
//https://firebase.google.com/docs/projects/locations
#define PROJECT_LOCATION "PROJECT_LOCATION"

/* 5. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

//We need to define the FunctionsConfig, PolicyBuilder and Binding data to keep the function and triggers configuaration and IAM policy.
//These objects should declare as global objects or static to prevent the stack overflow.
PolicyBuilder policy;
Binding binding;
FunctionsConfig function_config(FIREBASE_PROJECT_ID /* project id */, PROJECT_LOCATION /* location id */, STORAGE_BUCKET_ID /* bucket id */);

int creationStep = 0;

unsigned long dataMillis = 0;

/* Define the FunctionsOperationStatusInfo data to get the Cloud Function creation status */
FunctionsOperationStatusInfo statusInfo;

/* The helper function to get the token status string */
String getTokenStatus(struct token_info_t info);

/* The helper function to get the token type string */
String getTokenType(struct token_info_t info);

/* The helper function to get the token error string */
String getTokenError(struct token_info_t info);

/* The function to create and deploy Cloud Function */
void creatFunction();

/* The function to show the Cloud Function deployment status */
void showFunctionCreationStatus(FunctionsOperationStatusInfo info);

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

    /* Assign the project host (required) */
    config.host = FIREBASE_HOST;

    /* Assign the Service Account credentials */
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo.setBSSLBufferSize(1024, 1024);
#endif
}

void loop()
{

    if (millis() - dataMillis > 10000 || dataMillis == 0)
    {
        dataMillis = millis();

        /* Get the token status */
        struct token_info_t info = Firebase.authTokenInfo();
        if (info.status == token_status_error)
        {
            Serial.println("------------------------------------");
            Serial.printf("Token info: type = %s, status = %s\n", getTokenType(info).c_str(), getTokenStatus(info).c_str());
            Serial.printf("Token error: %s\n\n", getTokenError(info).c_str());
        }
        else
        {
            //Serial.println("------------------------------------");
            //Serial.printf("Token info: type = %s, status = %s\n\n", getTokenType(info).c_str(), getTokenStatus(info).c_str());

            if (creationStep == 0)
            {
                creationStep = 1;
                creatFunction();
            }

            if (creationStep == 1)
                showFunctionCreationStatus(statusInfo);
        }
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

/* The function to create and deploy Cloud Function */
void creatFunction()
{

    function_config.setName("helloWorld");
    function_config.setDescription("test");
    function_config.setEntryPoint("helloWorld");
    function_config.setRuntime("nodejs12");
    function_config.setTimeout(60);
    function_config.setAvailableMemoryMb(256);
    function_config.setMaxInstances(10);

    //If the source code, "helloWorld.zip" is already stored in the Storage bucket
    function_config.setSource("/helloWorld.zip" /* relative file path in the Firebase Storage data bucket */, functions_sources_type_storage_bucket_archive /* source type */);

    //or if it in the local memory storage i.e. flash or SD
    //function_config.setSource("/helloWorld.zip" /* file path */, functions_sources_type_local_archive /* source type */,  mem_storage_type_flash /* type of memory storage */);

    //or the source code archive is hosted in the Cloud Storage repo
    //function_config.setSource("PATH to zip file hosted on the repo", functions_sources_type_repository /* source type */);
    //https://cloud.google.com/functions/docs/reference/rest/v1/projects.locations.functions#sourcerepository

    //or if you want to deploy the function from the source code files stored in the Storage data bucket instead of zip file
    //function_config.setSource("functions/helloWorld" /* relative path (folder) in the Firebase Storage data bucket that stores source code files */, functions_sources_type_storage_bucket_sources /* source type */);

    function_config.setIngressSettings("ALLOW_ALL");

    //Set up the IAM policy
    binding.setRole("roles/cloudfunctions.invoker");
    binding.addMember("allUsers");
    policy.addBinding(&binding);

    function_config.setIamPolicy(&policy);

    Serial.println("------------------------------------");
    Serial.println("Create the Googgle Cloud Function...");

    Firebase.Functions.createFunction(&fbdo, &function_config /* FunctionsConfig */, &statusInfo /* FunctionsOperationStatusInfo data to read the operation status */);
}

/* The function to show the Cloud Function deployment status */
void showFunctionCreationStatus(FunctionsOperationStatusInfo statusInfo)
{
    if (statusInfo.status == fb_esp_functions_operation_status_unknown)
        Serial.printf("%s: Unknown\n", statusInfo.functionId.c_str());
    else if (statusInfo.status == fb_esp_functions_operation_status_generate_upload_url)
        Serial.printf("%s: Generate the upload Url...\n", statusInfo.functionId.c_str());
    else if (statusInfo.status == fb_esp_functions_operation_status_upload_source_file_in_progress)
        Serial.printf("%s: Uploading file...\n", statusInfo.functionId.c_str());
    else if (statusInfo.status == fb_esp_functions_operation_status_deploy_in_progress)
        Serial.printf("%s: Deploying function...\n", statusInfo.functionId.c_str());
    else if (statusInfo.status == fb_esp_functions_operation_status_set_iam_policy_in_progress)
        Serial.printf("%s: Set the IAM policy...\n", statusInfo.functionId.c_str());
    else if (statusInfo.status == fb_esp_functions_operation_status_delete_in_progress)
        Serial.printf("%s: Delete the function...\n", statusInfo.functionId.c_str());
    else if (statusInfo.status == fb_esp_functions_operation_status_finished)
    {
        creationStep = 2;
        Serial.println("Status: success");
        Serial.print("Trigger Url: ");
        Serial.println(statusInfo.triggerUrl.c_str());
        Serial.println();
    }
    else if (statusInfo.status == fb_esp_functions_operation_status_error)
    {
        creationStep = 2;
        Serial.print("Status: ");
        Serial.println(statusInfo.errorMsg.c_str());
        Serial.println();
    }
}
