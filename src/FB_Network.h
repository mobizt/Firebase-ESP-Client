/**
 * Created July 10, 2022
 */

#ifndef FB_Network_H
#define FB_Network_H

#include "FirebaseFS.h"

#if !defined(ESP32) && !defined(ESP8266)
#ifndef FB_ENABLE_EXTERNAL_CLIENT
#define FB_ENABLE_EXTERNAL_CLIENT
#endif
#endif

#if defined(ESP32)
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ETH.h>
#include <WiFiClientSecure.h>
#if __has_include(<esp_idf_version.h>)
#include <esp_idf_version.h>
#endif
#endif


#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
#include <core_version.h>
#include <time.h>
#define FB_ESP_SSL_CLIENT BearSSL::WiFiClientSecure

//__GNUC__
//__GNUC_MINOR__
//__GNUC_PATCHLEVEL__

#ifdef __GNUC__
#if __GNUC__ > 4 || __GNUC__ == 10
#include <string>
#define ESP8266_CORE_SDK_V3_X_X
#endif
#endif

#ifndef ARDUINO_ESP8266_GIT_VER
#error Your ESP8266 Arduino Core SDK is outdated, please update. From Arduino IDE go to Boards Manager and search 'esp8266' then select the latest version.
#endif

//2.6.1 BearSSL bug
#if ARDUINO_ESP8266_GIT_VER == 0x482516e3
#error Due to bugs in BearSSL in ESP8266 Arduino Core SDK version 2.6.1, please update ESP8266 Arduino Core SDK to newer version. The issue was found here https:\/\/github.com/esp8266/Arduino/issues/6811.
#endif

#if defined __has_include

#if __has_include(<LwipIntfDev.h>)
#include <LwipIntfDev.h>
#endif

#if __has_include(<ENC28J60lwIP.h>)
#define INC_ENC28J60_LWIP
#include <ENC28J60lwIP.h>
#endif


#if __has_include(<W5100lwIP.h>) && defined(ENABLE_ESP8266_W5100_ETH)

#define INC_W5100_LWIP 

// PIO compilation error
#include <W5100lwIP.h>
#endif

#if __has_include(<W5500lwIP.h>)
#define INC_W5500_LWIP
#include <W5500lwIP.h>
#endif

#endif

#endif

#endif