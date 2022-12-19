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

/** This example shows the RTDB data changed notification with external Client.
 * This example used ESP32 and WIZnet W5500 (Etherner) devices which ESP_SSLClient will be used as the external Client.
 *
 * Don't gorget to define this in FirebaseFS.h
 * #define FB_ENABLE_EXTERNAL_CLIENT
 */

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

//https://github.com/arduino-libraries/Ethernet
#include <Ethernet.h>

// https://github.com/mobizt/ESP_SSLClient
#include <ESP_SSLClient.h>

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
#define WIZNET_RESET_PIN 26 // Connect W5500 Reset pin to GPIO 26 of ESP32
#define WIZNET_CS_PIN 5     // Connect W5500 CS pin to GPIO 5 of ESP32
#define WIZNET_MISO_PIN 19  // Connect W5500 MISO pin to GPIO 19 of ESP32
#define WIZNET_MOSI_PIN 23  // Connect W5500 MOSI pin to GPIO 23 of ESP32
#define WIZNET_SCLK_PIN 18  // Connect W5500 SCLK pin to GPIO 18 of ESP32

/* 5. Define MAC */
uint8_t Eth_MAC[] = {0x02, 0xF0, 0x0D, 0xBE, 0xEF, 0x01};

/* 6. Define IP (Optional) */
IPAddress Eth_IP(192, 168, 1, 104);

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

volatile bool dataChanged = false;

// Define the basic client
// The network interface devices that can be used to handle SSL data should
// have large memory buffer up to 1k - 2k or more, otherwise the SSL/TLS handshake
// will fail.
EthernetClient basic_client1;

EthernetClient basic_client2;

// This is the wrapper client that utilized the basic client for io and 
// provides the mean for the data encryption and decryption before sending to or after read from the io.
// The most probable failures are related to the basic client itself that may not provide the buffer
// that large enough for SSL data. 
// The SSL client can do nothing for this case, you should increase the basic client buffer memory.
ESP_SSLClient ssl_client1;

ESP_SSLClient ssl_client2;

// For NTP client
EthernetUDP udpClient;

MB_NTP ntpClient(&udpClient, "pool.ntp.org" /* NTP host */, 123 /* NTP port */, 0 /* timezone offset in seconds */);

uint32_t timestamp = 0;

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
    Serial.println("Can't connected");
  }
}

// Define the callback function to handle server status acknowledgement
void networkStatusRequestCallback()
{
  // Set the network status
  fbdo.setNetworkStatus(Ethernet.linkStatus() == LinkON);
  stream.setNetworkStatus(Ethernet.linkStatus() == LinkON);
}

// Define the callback function to handle server connection
void tcpConnectionRequestCallback1(const char *host, int port)
{

  // You may need to set the system timestamp to use for
  // auth token expiration checking.

  if (timestamp == 0)
  {
    timestamp = ntpClient.getTime(2000 /* wait 2000 ms */);

    if (timestamp > 0)
      Firebase.setSystemTime(timestamp);
  }

  Serial.print("Connecting to server via external Client... ");
  if (!ssl_client1.connect(host, port))
  {
    Serial.println("failed.");
    return;
  }
  Serial.println("success.");
}

// Define the callback function to handle server connection
void tcpConnectionRequestCallback2(const char *host, int port)
{

  // You may need to set the system timestamp to use for
  // auth token expiration checking.

  if (timestamp == 0)
  {
    timestamp = ntpClient.getTime(2000 /* wait 2000 ms */);

    if (timestamp > 0)
      Firebase.setSystemTime(timestamp);
  }

  Serial.print("Connecting to server via external Client2... ");
  if (!ssl_client2.connect(host, port))
  {
    Serial.println("failed.");
    return;
  }
  Serial.println("success.");
}

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

  networkConnection();

  Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the basic Client (Ethernet) pointer to the basic Client */
  ssl_client1.setClient(&basic_client1);

  /* Similar to WiFiClientSecure */
  ssl_client1.setInsecure();

  /* Assign the basic Client (Ethernet) pointer to the basic Client */
  ssl_client2.setClient(&basic_client2);

  /* Similar to WiFiClientSecure */
  ssl_client2.setInsecure();

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
  fbdo.setExternalClient(&ssl_client1);

  /* Assign the required callback functions */
  fbdo.setExternalClientCallbacks(tcpConnectionRequestCallback1, networkConnection, networkStatusRequestCallback);

  /* Assign the pointer to global defined  external SSL Client object */
  stream.setExternalClient(&ssl_client2);

  /* Assign the required callback functions */
  stream.setExternalClientCallbacks(tcpConnectionRequestCallback2, networkConnection, networkStatusRequestCallback);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  Firebase.begin(&config, &auth);

  if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
    Serial_Printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

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
