#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Firebase TCP Client v1.2.4
 *
 * Created March 5, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#if (defined(ESP8266) || defined(MB_ARDUINO_PICO)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)

#include <ESP8266WiFi.h>
#include "FB_Network.h"
#include "FB_Error.h"
#include "mbfs/MB_FS.h"
#include "./wcs/base/FB_TCP_Client_Base.h"

class FB_TCP_Client : public FB_TCP_Client_Base
{

  friend class FirebaseData;
  friend class FB_RTDB;
  friend class FB_CM;
  friend class FB_CloudStorage;
  friend class UtilsClass;

public:
  FB_TCP_Client();
  ~FB_TCP_Client();

  void setInsecure();

  void setCACert(const char *caCert);

  bool setCertFile(const char *certFile, mb_fs_mem_storage_type storageType);

  void setBufferSizes(int recv, int xmit);

  bool networkReady();

  void networkReconnect();

  void networkDisconnect();

  fb_tcp_client_type type();

  bool isInitialized();

  int hostByName(const char *name, IPAddress &ip);

  void setTimeout(uint32_t timeoutmSec);

  bool begin(const char *host, uint16_t port, int *response_code);

  int beginUpdate(int len, bool verify = true);

  bool ethLinkUp();

  void ethDNSWorkAround();

private:
  std::unique_ptr<FB_ESP_SSL_CLIENT> wcs = std::unique_ptr<FB_ESP_SSL_CLIENT>(new FB_ESP_SSL_CLIENT());

#if defined(ESP8266)
  uint16_t bsslRxSize = 4096;
  uint16_t bsslTxSize = 512;
#elif defined(MB_ARDUINO_PICO)
  uint16_t bsslRxSize = 16384;
  uint16_t bsslTxSize = 1024;
#endif
  X509List *x509 = nullptr;
  void release();
};

#endif /* ESP8266 */

#endif /* FB_TCP_Client_H */