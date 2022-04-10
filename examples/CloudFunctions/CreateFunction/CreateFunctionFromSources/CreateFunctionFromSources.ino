
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

/** Prerequisites
 *
 * Cloud Functions deployment requires the pay-as-you-go (Blaze) billing plan.
 *
 * IAM owner permission required for service account used and Cloud Build API must be enabled,
 * https://github.com/mobizt/Firebase-ESP-Client#iam-permission-and-api-enable
 */

/** This example shows how to create (deploy) the Cloud Function from your source code files stored in the Storage data bucket.
 *
 * This operation required OAUth2.0 authentication. The example "Deploy_autozip_Function" should be run prior to use this exanple,
 * please see the details below.
 */

/** Actually the zip archive is required for function deployment via the REST APIs which it may be hosted at the repository or uploaded or
 * stored in the Storage bucket.
 *
 * To deploy the function from sources, all files need to be compressed to a single zip archive.
 *
 * This library provides the special option to deploy the function from sources files instead of a zip archive.
 *
 * Since the implementation of zip compression required the hundreds to megabytes memory for operation and is not suitable for the low memory devices.
 *
 * The zip compression will processs on the cloud then upload to the Cloud Storage via the backend Cloud Function which provided by the library, called autozip.
 *
 * The Cloud function called autozip is used for this backend operation for zip compression and upload.
 *
 * Then you need to deploy the autozip function prior to run this example by running the "Deploy_autozip_Function" example.
 */

/** The pointer, points to the operation info assigned to the create function will provide the progress of deployment that can be accessed later.
 *
 * The Cloud Function source code files to deploy with this example will be compress as a single zip archive.
 *
 * This zip file can be stored in the Firebase Storage data bucket or the repository or in local device storage e.g. flash and SD memory.
 *
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

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/** 2. Define the Service Account credentials (required for token generation)
 *
 * This information can be taken from the service account JSON file.
 *
 * To download service account file, from the Firebase console, goto project settings,
 * select "Service accounts" tab and click at "Generate new private key" button
 */
#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

/* 3. Define the project location e.g. us-central1 or asia-northeast1 */
// https://firebase.google.com/docs/projects/locations
#define PROJECT_LOCATION "PROJECT_LOCATION"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

/* 5. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// We need to define the FunctionsConfig, PolicyBuilder and Binding data to keep the function and triggers configuaration and IAM policy.
// These objects should declare as global objects or static to prevent the stack overflow.
PolicyBuilder policy;
Binding binding;
FunctionsConfig function_config(FIREBASE_PROJECT_ID /* project id */, PROJECT_LOCATION /* location id */, STORAGE_BUCKET_ID /* bucket id */);

bool taskCompleted = false;

unsigned long dataMillis = 0;

/* The function to create and deploy Cloud Function */
void creatFunction();

/* The function to show the Cloud Function deployment status */
void functionCreationCallback(FunctionsOperationStatusInfo info);

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

    /* Assign the Service Account credentials */
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        creatFunction();
        taskCompleted = true;
    }
}

/* The function to create and deploy Cloud Function */
void creatFunction()
{
    // You need to deploy the autozip function prior to run this example by running the "Deploy_autozip_Function.ino"

    function_config.setName("helloWorld");
    function_config.setDescription("test");
    function_config.setEntryPoint("helloWorld");
    function_config.setRuntime("nodejs12");
    function_config.setTimeout(60);
    function_config.setAvailableMemoryMb(256);
    function_config.setMaxInstances(10);

    // If the function source code files stored in the bucket at, "functions/helloWorld"
    function_config.setSource("functions/helloWorld" /* relative path (folder) in the Firebase Storage data bucket that stores source code files */, functions_sources_type_storage_bucket_sources /* source type */);

    function_config.setIngressSettings("ALLOW_ALL");

    // Set up the IAM policy
    binding.setRole("roles/cloudfunctions.invoker");
    binding.addMember("allUsers");
    policy.addBinding(&binding);

    function_config.setIamPolicy(&policy);

    Serial.println("Create the Googgle Cloud Function...");

    Firebase.Functions.createFunction(&fbdo, &function_config /* FunctionsConfig */, functionCreationCallback /* the callback function */);
}

/* The function to show the Cloud Function deployment status */
void functionCreationCallback(FunctionsOperationStatusInfo statusInfo)
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
        Serial.printf("%s: success\n", statusInfo.functionId.c_str());
        Serial.print("Trigger Url: ");
        Serial.println(statusInfo.triggerUrl.c_str());
        Serial.println();
    }
    else if (statusInfo.status == fb_esp_functions_operation_status_error)
    {
        Serial.printf("%s: Error, ", statusInfo.functionId.c_str());
        Serial.println(statusInfo.errorMsg.c_str());
        Serial.println();
    }
}