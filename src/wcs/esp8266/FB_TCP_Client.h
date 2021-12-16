/**
 * Firebase TCP Client v1.1.16
 * 
 * Created November 23, 2021
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

#ifndef FB_TCP_Client_H
#define FB_TCP_Client_H

#ifdef ESP8266

#include <Arduino.h>
#include <core_version.h>
#include <time.h>

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

#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
#define FB_ESP_SSL_CLIENT BearSSL::WiFiClientSecure

#define FS_NO_GLOBALS
#include <FS.h>
#include "FirebaseFS.h"

#if defined DEFAULT_FLASH_FS
#define FLASH_FS DEFAULT_FLASH_FS
#endif

#if defined DEFAULT_SD_FS
#define SD_FS DEFAULT_SD_FS
#endif

#if defined(FIREBASE_USE_PSRAM)
#define FIREBASEJSON_USE_PSRAM
#endif

#include "./json/FirebaseJson.h"

#if defined __has_include

#if __has_include(<LwipIntfDev.h>)
#include <LwipIntfDev.h>
#endif

#if __has_include(<ENC28J60lwIP.h>)
#define INC_ENC28J60_LWIP
#include <ENC28J60lwIP.h>
#endif

#if __has_include(<W5100lwIP.h>)
#define INC_W5100_LWIP
#include <W5100lwIP.h>
#endif

#if __has_include(<W5500lwIP.h>)
#define INC_W5500_LWIP
#include <W5500lwIP.h>
#endif

#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "wcs/HTTPCode.h"

struct fb_esp_sd_config_info_t
{
  int sck = -1;
  int miso = -1;
  int mosi = -1;
  int ss = -1;
};

class FB_TCP_Client
{

  friend class FirebaseData;
  friend class FB_RTDB;
  friend class FB_CM;
  friend class FB_CloudStorage;
  friend class UtilsClass;

public:
  FB_TCP_Client();
  ~FB_TCP_Client();

  /**
   * Initialization of new http connection.
   * \param host - Host name without protocols.
   * \param port - Server's port.
   * \return True as default.
   * If no certificate string provided, use (const char*)NULL to CAcert param 
  */
  bool begin(const char *host, uint16_t port);

  /**
   *  Check the http connection status.
   * \return True if connected.
  */
  bool connected(void);

  /**
    * Establish TCP connection when required and send data.
    * \param data - The data to send.
    * \param len - The length of data to send.
    * 
    * \return TCP status code, Return zero if new TCP connection and data sent.
    */
  int send(const char *data, size_t len = 0);

  /**
   * Get the WiFi client pointer.
   * \return WiFi client pointer.
  */
  WiFiClient *stream(void);

  void setCACert(const char *caCert);
  void setCACertFile(const char *caCertFile, uint8_t storageType, struct fb_esp_sd_config_info_t sd_config);
  bool connect(void);

private:
  std::unique_ptr<FB_ESP_SSL_CLIENT> _wcs = std::unique_ptr<FB_ESP_SSL_CLIENT>(new FB_ESP_SSL_CLIENT());
  MBSTRING _host;
  uint16_t _port = 0;

  //Actually Arduino base Stream (char read) timeout.
  //This will override internally by WiFiClientSecureCtx::_connectSSL
  //to 5000 after SSL handshake was done with success.
  unsigned long timeout = 10 * 1000;

  MBSTRING _CAFile;
  uint8_t _CAFileStoreageType = 0;
  int _certType = -1;
  uint8_t _sdPin = 15;
  bool _clockReady = false;
  uint16_t _bsslRxSize = 512;
  uint16_t _bsslTxSize = 512;
  bool fragmentable = false;
  int chunkSize = 1024;
  bool mflnChecked = false;
  X509List *x509 = nullptr;

  void release();
};

#endif /* ESP8266 */

#endif /* FB_TCP_Client_H */