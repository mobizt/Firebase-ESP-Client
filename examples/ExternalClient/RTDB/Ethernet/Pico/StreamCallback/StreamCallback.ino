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

/** This example shows the RTDB data changed notification with external Client.
 * This example used Raspberry Pi Pico and WIZnet W5500 module.
 * The Ethernet.h will work with SPI 0 only.
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

// https://github.com/arduino-libraries/Ethernet
#include <Ethernet.h>

// For using other Ethernet library that works with other Ethernet module, 
// the following build flags or macros should be assigned in src/FirebaseFS.h or your CustomFirebaseFS.h.
// FIREBASE_ETHERNET_MODULE_LIB and FIREBASE_ETHERNET_MODULE_CLASS
// See src/FirebaseFS.h for detail.

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 1. Define the API Key */
#define API_KEY "API_KEY"

/* 2. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 4. Defined the Ethernet module connection */
#define WIZNET_RESET_PIN 20       // Connect W5500 Reset pin to GPIO 20 of Raspberry Pi Pico (-1 for no reset pin assigned)
#define WIZNET_CS_PIN PIN_SPI0_SS // Connect W5500 CS pin to SPI 0's SS (GPIO 17) of Raspberry Pi Pico

/* 5. Define MAC */
uint8_t Eth_MAC[] = {0x02, 0xF0, 0x0D, 0xBE, 0xEF, 0x01};

/* 6. Define the static IP (Optional)
IPAddress localIP(192, 168, 1, 104);
IPAddress subnet(255, 255, 0, 0);
IPAddress gateway(192, 168, 1, 1);
IPAddress dnsServer(8, 8, 8, 8);
bool optional = false; // Use this static IP only no DHCP
Firebase_StaticIP staIP(localIP, subnet, gateway, dnsServer, optional);
*/

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

volatile bool dataChanged = false;

EthernetClient eth1;

EthernetClient eth2;

void streamCallback(FirebaseStream data)
{
  Serial_Printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since the stream connected
  // and read once and will not update until stream reconnection takes place.
  // This max value will be zero as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial_Printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial_Printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup()
{

  Serial.begin(115200);
  delay(5000);

  Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  /* Assign the pointer to global defined Ethernet Client object */
  fbdo.setEthernetClient(&eth1, Eth_MAC, WIZNET_CS_PIN, WIZNET_RESET_PIN);   // The staIP can be assigned to the fifth param
  stream.setEthernetClient(&eth2, Eth_MAC, WIZNET_CS_PIN, WIZNET_RESET_PIN); // The staIP can be assigned to the fifth param

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  stream.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.setDoubleDigits(5);

  Firebase.begin(&config, &auth);

  if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
    Serial_Printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

#if !defined(ESP8266) && !defined(ESP32)
  Firebase.RTDB.runStream();
#endif

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    count++;
    FirebaseJson json;
    json.add("data", "hello");
    json.add("num", count);
    Serial_Printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&fbdo, "/test/stream/data/json", &json) ? "ok" : fbdo.errorReason().c_str());
  }

  if (dataChanged)
  {
    dataChanged = false;
    // When stream data is available, do anything here...
  }
}
