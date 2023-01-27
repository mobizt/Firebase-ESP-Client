
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP8266
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example shows the basic RTDB usage with external Client.
 * This example used Raspberry Pi Pico and WIZnet W5500 (Ethernet) devices which ESP_SSLClient will be used as the external Client.
 * The Ethernet.h will work with SPI 0 only.
 *
 * Even the example for Ethernet that supports ENC28J60 and WIZnet W55xx is available at RTB/BasicEthernet/Pico/Pico.ino,
 * this example will show how to use external SSL Client that supports other network interfaces e.g. GSMClient and especially
 * EthernetClient in this example.
 *
 * Don't gorget to define this in FirebaseFS.h
 * #define FB_ENABLE_EXTERNAL_CLIENT
 */

/**
 *
 * The W5500 Ethernet module and RPI2040 Pico board, SPI 0 port wiring connection.
 *
 * RP2040 (Pico)                           W5500
 *
 * GPIO 16 - SPI 0 MISO                     SO
 * GPIO 19 - SPI 0 MOSI                     SI
 * GPIO 18 - SPI 0 SCK                      SCK
 * GPIO 17 - SPI 0 CS                       CS
 * GPIO 20 - W5500 Reset                    Reset
 * GND                                      GND
 * 3V3                                      VCC
 *
 */

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// https://github.com/mobizt/ESP_SSLClient
#include <ESP_SSLClient.h>

#include <Ethernet.h>

// For NTP time client
#include "MB_NTP.h"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 1. Define the API Key */
#define API_KEY "API_KEY"

/* 2. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 4. Defined the Ethernet module connection */
#define WIZNET_RESET_PIN 20       // Connect W5500 Reset pin to GPIO 20 of Raspberry Pi Pico
#define WIZNET_CS_PIN PIN_SPI0_SS // Connect W5500 CS pin to SPI 0's SS (GPIO 17) of Raspberry Pi Pico

/* 5. Define MAC */
uint8_t Eth_MAC[] = {0x02, 0xF0, 0x0D, 0xBE, 0xEF, 0x01};

/* 6. Define IP (Optional) */
IPAddress Eth_IP(192, 168, 1, 104);

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

// Define the basic client
// The network interface devices that can be used to handle SSL data should
// have large memory buffer up to 1k - 2k or more, otherwise the SSL/TLS handshake
// will fail.
EthernetClient basic_client;

// This is the wrapper client that utilized the basic client for io and
// provides the mean for the data encryption and decryption before sending to or after read from the io.
// The most probable failures are related to the basic client itself that may not provide the buffer
// that large enough for SSL data.
// The SSL client can do nothing for this case, you should increase the basic client buffer memory.
ESP_SSLClient ssl_client;

void ResetEthernet()
{
    Serial.println("Resetting WIZnet W5500 Ethernet Board...  ");
    pinMode(WIZNET_RESET_PIN, OUTPUT);
    digitalWrite(WIZNET_RESET_PIN, HIGH);
    delay(200);
    digitalWrite(WIZNET_RESET_PIN, LOW);
    delay(50);
    digitalWrite(WIZNET_RESET_PIN, HIGH);
    delay(200);
}

void networkConnection()
{
    Ethernet.init(WIZNET_CS_PIN);

    ResetEthernet();

    Serial.println("Starting Ethernet connection...");
    Ethernet.begin(Eth_MAC);

    unsigned long to = millis();

    while (Ethernet.linkStatus() == LinkOFF || millis() - to < 2000)
    {
        delay(100);
    }

    if (Ethernet.linkStatus() == LinkON)
    {
        Serial.print("Connected with IP ");
        Serial.println(Ethernet.localIP());
    }
    else
    {
        Serial.println("Can't connect");
    }
}

// Define the callback function to handle server status acknowledgement
void networkStatusRequestCallback()
{
    // Set the network status
    fbdo.setNetworkStatus(Ethernet.linkStatus() == LinkON);
}

void setup()
{

    Serial.begin(115200);
    delay(5000);

    networkConnection();

    Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the basic Client (Ethernet) pointer to the basic Client */
    ssl_client.setClient(&basic_client);

    /* Similar to WiFiClientSecure */
    ssl_client.setInsecure();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    /* fbdo.setExternalClient and fbdo.setExternalClientCallbacks must be called before Firebase.begin */

    /* Assign the pointer to global defined external SSL Client object */
    fbdo.setExternalClient(&ssl_client);

    /* Assign the required callback functions */
    fbdo.setExternalClientCallbacks(networkConnection, networkStatusRequestCallback);

    // Comment or pass false value when WiFi reconnection will control by your code or third party library
    Firebase.reconnectWiFi(true);

    Firebase.setDoubleDigits(5);

    Firebase.begin(&config, &auth);
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

        Serial_Printf("Set bool... %s\n", Firebase.RTDB.setBool(&fbdo, F("/test/bool"), count % 2 == 0) ? "ok" : fbdo.errorReason().c_str());

        count++;
    }
}
