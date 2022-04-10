
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

/**
 * This example shows how to blink the LED programatically.
 *
 * This example used the FireSense, The Programmable Data Logging and IO Control library to
 * control the IOs.
 *
 * The status of IO showed at /{DATA_PATH}/status/Node-ESP-{DEVICE_ID}
 *
 * Where the {DATA_PATH} is the base path (rooth) for all data will be stored.
 *
 * The {DEVICE_ID} is the uniqued id of the device which can get from FireSense.getDeviceId().
 *
 *
 *
 * To manually set the IO or Value channel value, change the value at /{DATA_PATH}/control/Node-ESP-{DEVICE_ID}
 *
 * To control the device, send the test command at /{DATA_PATH}/control/Node-ESP-{DEVICE_ID}/cmd.
 *
 * The supported commands and the descriptions are following,
 * the result of processing is at /{DATA_PATH}/status/Node-ESP-{DEVICE_ID}/terminal
 *
 * time                 To get the device timestamp
 *
 * time {TIMESTAMP}     To set the device timestamp {TIMESTAMP}
 *
 * config               To load the config from database. If no config existed in database,
 *                      the default config may load within the user default config callback function.
 *
 * condition            To load the conditions, the IF THEN ELSE statments's conditions and statements.
 *
 * run                  To run the conditions checking tasks, the condition checking tasks will run automatically by default.
 *                      To disable conditions checking to run at the device startup, call FireSense.enableController(false)
 *                      after FireSense.begin.
 *
 * stop                 to stop the conditions checking tasks.
 *
 * store                To store the channels's status (value) to the database under /{DATA_PATH}/status/Node-ESP-{DEVICE_ID}
 *                      This depends on the the channal's status parameter value.
 *                      The device will store the channels's current values to database when its value changed and the channal's status parameter value is 'true'.
 *
 * restore              To restore (read) the status at /{DATA_PATH}/status/Node-ESP-{DEVICE_ID} and set to the channels's values.
 *
 * clear                To clear all log data at /{DATA_PATH}/logs/Node-ESP-{DEVICE_ID}.
 *                      The device will store the channnels's value to this logs path at the preconfigured interval and the channels's log parameter value is 'true'.
 *
 * ping                 To test the device is running or stream is still connected.
 *
 * restart              To restart the device.
 *
 */

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// The FireSense class used in this example.
#include <addons/FireSense/FireSense.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 5. Define the database secret in case we need to access database rules*/
#define DATABASE_SECRET "DATABASE_SECRET"

// Define Firebase Data object
FirebaseData fbdo1;

FirebaseAuth auth;
FirebaseConfig config;

// The config data for FireSense class
Firesense_Config fsConfig;

int count = 0;
float sum = 0, avg = 0;

void testFunc1(const char *msg)
{
    if (strlen(msg) > 0)
        Serial.println(msg);
}

void testFunc2(const char *msg)
{
    if (strlen(msg) > 0)
        Serial.println(msg);
}

void loadDefaultConfig()
{
    // The config can be set manually from below or read from the stored config file.

    // To store the current config file (read from database and store to device storage),
    // FireSense.backupConfig("/filename.txt" /* path of file to save */, mem_storage_type_flash /* or mem_storage_type_sd */);

    // To restore the current config (read from device storage and add to database),
    // FireSense.restoreConfig("/filename.txt" /* path of back up file to read */, mem_storage_type_flash /* or mem_storage_type_sd */);

    FireSense_Channel channel[1];

    channel[0].id = "LED1";
#if defined(ESP32)
    channel[0].gpio = 5;
#elif defined(ESP8266)
    channel[0].gpio = 2;
#endif
    channel[0].type = Firesense_Channel_Type::Output;
    FireSense.addChannel(channel[0]);

    FireSense_Condition cond[1];

    cond[0].IF = "LED1 == true && millis < 120000"; // blink until 2 minutes reached since boot
    cond[0].THEN = "delay(20), LED1 = false";
    cond[0].ELSE = "delay(980), LED1 = true";
    FireSense.addCondition(cond[0]);
}

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

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // Set up the config
    fsConfig.basePath = "/Blink";
    fsConfig.deviceId = "Node1";
    fsConfig.time_zone = 3; // change for your local time zone
    fsConfig.daylight_offset_in_sec = 0;
    fsConfig.last_seen_interval = 60 * 1000;     // store timestamp
    fsConfig.log_interval = 60 * 1000;           // store log data every 60 seconds
    fsConfig.condition_process_interval = 0;     // check the conditions instantly
    fsConfig.dataRetainingPeriod = 24 * 60 * 60; // keep the log data for 1 day
    fsConfig.shared_fbdo = &fbdo1;               // for store/restore the data

    // This new config added to disable internal command streaming when the fsConfig.stream_fbdo was not assigned.
    // This allows the session to be reused for faster data sending.
    // The seesion will not close and open for usage changed between internal stream and normal data sending.
    fsConfig.disable_command = true;

    fsConfig.debug = true;

    // Initiate the FireSense class
    FireSense.begin(&fsConfig, DATABASE_SECRET); // The database secret can be empty string when using the OAuthen2.0 sign-in method

    // Load the config from database or create the default config
    if (!FireSense.loadConfig())
    {
        loadDefaultConfig(); // The loadDefaultConfig is the function to configure the channels and condition information.
        FireSense.updateConfig();
    }
}

void loop()
{
    FireSense.run();
    // do not use delay or blocking operating code heare
}