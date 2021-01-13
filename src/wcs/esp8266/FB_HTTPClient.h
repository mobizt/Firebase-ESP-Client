/**
 * HTTP Client wrapper v1.1.5
 * 
 * This library provides ESP8266 to perform REST API by GET PUT, POST, PATCH, DELETE data from/to with Google's Firebase database using get, set, update
 * and delete calls. 
 * 
 * The library was test and work well with ESP32s based module and add support for multiple stream event path.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FB_HTTPClient_H
#define FB_HTTPClient_H

#ifdef ESP8266

//ARDUINO_ESP8266_GIT_VER
//2.6.2 0xbc204a9b
//2.6.1 0x482516e3
//2.6.0 0x643ec203
//2.5.2 0x8b899c12
//2.5.1 0xac02aff5
//2.5.0 0x951aeffa
//2.5.0-beta3 0x21db8fc9
//2.5.0-beta2 0x0fd86a07
//2.5.0-beta1 0x9c1e03a1
//2.4.2 0xbb28d4a3
//2.4.1 0x614f7c32
//2.4.0 0x4ceabea9
//2.4.0-rc2 0x0c897c37
//2.4.0-rc1 0xf6d232f1

#include <Arduino.h>
#include <core_version.h>
#include <time.h>

#ifndef ARDUINO_ESP8266_GIT_VER
#error Your ESP8266 Arduino Core SDK is outdated, please update. From Arduino IDE go to Boards Manager and search 'esp8266' then select the latest version.
#endif

//2.6.1 BearSSL bug
#if ARDUINO_ESP8266_GIT_VER == 0x482516e3
#error Due to bugs in BearSSL in ESP8266 Arduino Core SDK version 2.6.1, please update ESP8266 Arduino Core SDK to newer version. The issue was found here https:\/\/github.com/esp8266/Arduino/issues/6811.
#endif

#if ARDUINO_ESP8266_GIT_VER != 0xf6d232f1 && ARDUINO_ESP8266_GIT_VER != 0x0c897c37 && ARDUINO_ESP8266_GIT_VER != 0x4ceabea9 && ARDUINO_ESP8266_GIT_VER != 0x614f7c32 && ARDUINO_ESP8266_GIT_VER != 0xbb28d4a3
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
#define FB_ESP_SSL_CLIENT BearSSL::WiFiClientSecure

#elif ARDUINO_ESP8266_GIT_VER == 0xbb28d4a3
#define USING_AXTLS
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;
#define FB_ESP_SSL_CLIENT axTLS::WiFiClientSecure
#else
#define USING_AXTLS
#include <WiFiClientSecure.h>
#define FB_ESP_SSL_CLIENT WiFiClientSecure
#endif

#define FS_NO_GLOBALS
#include <FS.h>
#include <SD.h>
#include "FirebaseFS.h"

#ifdef USE_LITTLEFS
#include <LittleFS.h>
#define FLASH_FS LittleFS
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define FLASH_FS SPIFFS
#endif

#if __has_include(<WiFiEspAT.h>) || __has_include(<espduino.h>)
#error WiFi UART bridge was not supported.
#endif

#include "wcs/HTTPCode.h"

class FB_HTTPClient
{

  friend class FirebaseData;
  friend class FB_RTDB;
  friend class FB_CM;
  friend class FB_CloudStorage;
  friend class UtilsClass;

public:
  FB_HTTPClient();
  ~FB_HTTPClient();

  bool begin(const char *host, uint16_t port);

  bool connected(void);

  int send(const char *header, const char *payload);

  bool send(const char *header);

  WiFiClient *stream(void);

  void setCACert(const char *caCert);
  void setCACertFile(const char* caCertFile, uint8_t storageType, uint8_t sdPin);
  bool connect(void);


private:
  std::unique_ptr<FB_ESP_SSL_CLIENT> _wcs = std::unique_ptr<FB_ESP_SSL_CLIENT>(new FB_ESP_SSL_CLIENT());
  std::unique_ptr<char> _cacert;
  std::string _host = "";
  uint16_t _port = 0;
  unsigned long timeout = FIREBASE_DEFAULT_TCP_TIMEOUT;

  std::string _CAFile = "";
  uint8_t _CAFileStoreageType = 0;
  int _certType = -1;
  uint8_t _sdPin = 15;
  bool _clockReady = false;
  uint16_t _bsslRxSize = 512;
  uint16_t _bsslTxSize = 512;
  bool fragmentable = false;
  int chunkSize = 1024;
  bool mflnChecked = false;
};

#endif /* ESP8266 */

#endif /* FB_HTTPClient_H */