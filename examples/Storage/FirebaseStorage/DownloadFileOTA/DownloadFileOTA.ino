/**
 * Created by WoolDoughnut310
 * 
 * Github: https://github.com/WoolDoughnut310
 *
*/

//This example shows how to download firmware file from Firebase Storage bucket and perform an OTA update.
//currently only works for ESP32

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Upload the Blink.bin file to the storage bucket through the Firebase console */

/* 2. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 3. Define the API Key */
#define API_KEY "API_KEY"

/* 4. Define the user Email and password that already registered or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 5. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "BUCKET-NAME.appspot.com"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void setup()
{
    Serial.begin(9600);
    Serial.println();
    Serial.println();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    log_i("Connected with IP: %d", WiFi.localIP());
    Serial.println();

    log_i("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        log_v("Download file... %s\n", Firebase.Storage.downloadOTA(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "Blink.bin" /* path of remote file stored in the bucket */) ? "ok" : fbdo.errorReason().c_str());

        if (!Update.end())
        {
            log_d("Size is %d", Update.size());
            log_e("Error from Update.end(): %s\n", Update.errorString());
            return;
        }

        if (Update.isFinished())
        {
            log_v("Update completed. Restarting. Enter Blink.");
            ESP.restart();
        }
        else
        {
            log_e("Error from Update.isFinished(): %s", Update.errorString());
            return;
        }
    }
}
