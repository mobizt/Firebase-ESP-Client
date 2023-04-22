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

#ifndef FB_TCP_Client_CPP
#define FB_TCP_Client_CPP

#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#if (defined(ESP8266) || defined(MB_ARDUINO_PICO)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)

#include "FB_TCP_Client.h"

FB_TCP_Client::FB_TCP_Client()
{
  client = wcs.get();
}

FB_TCP_Client::~FB_TCP_Client()
{
  release();
}

void FB_TCP_Client::setInsecure()
{
  wcs->setInsecure();
}

void FB_TCP_Client::setCACert(const char *caCert)
{

  release();

  wcs = std::unique_ptr<FB_ESP_SSL_CLIENT>(new FB_ESP_SSL_CLIENT());

  client = wcs.get();

  if (caCert)
  {
    x509 = new X509List(caCert);
    wcs->setTrustAnchors(x509);
    baseSetCertType(fb_cert_type_data);
  }
  else
  {
    wcs->setInsecure();
    baseSetCertType(fb_cert_type_none);
  }

  wcs->setBufferSizes(bsslRxSize, bsslTxSize);
}

bool FB_TCP_Client::setCertFile(const char *caCertFile, mb_fs_mem_storage_type storageType)
{

  if (clockReady && strlen(caCertFile) > 0)
  {
    MB_String filename = caCertFile;
    if (filename.length() > 0)
    {
      if (filename[0] != '/')
        filename.prepend('/');
    }

    int len = mbfs->open(filename, storageType, mb_fs_open_mode_read);
    if (len > -1)
    {

      uint8_t *der = MemoryHelper::createBuffer<uint8_t *>(mbfs, len);
      if (mbfs->available(storageType))
        mbfs->read(storageType, der, len);
      mbfs->close(storageType);
      wcs->setTrustAnchors(new X509List(der, len));
      MemoryHelper::freeBuffer(mbfs, der);
      baseSetCertType(fb_cert_type_file);
    }
  }

  wcs->setBufferSizes(bsslRxSize, bsslTxSize);

  return getCertType() == fb_cert_type_file;
}

void FB_TCP_Client::setBufferSizes(int recv, int xmit)
{
  bsslRxSize = recv;
  bsslTxSize = xmit;
}

bool FB_TCP_Client::networkReady()
{
  return WiFi.status() == WL_CONNECTED || ethLinkUp();
}

void FB_TCP_Client::networkReconnect()
{
#if defined(ESP8266)
  WiFi.reconnect();
#else

#endif
}

void FB_TCP_Client::networkDisconnect()
{
  WiFi.disconnect();
}

fb_tcp_client_type FB_TCP_Client::type()
{
  return fb_tcp_client_type_internal;
}

bool FB_TCP_Client::isInitialized() { return true; }

int FB_TCP_Client::hostByName(const char *name, IPAddress &ip)
{
  return WiFi.hostByName(name, ip);
}

void FB_TCP_Client::setTimeout(uint32_t timeoutmSec)
{
  if (wcs)
    wcs->setTimeout(timeoutmSec);

  baseSetTimeout(timeoutmSec / 1000);
}

bool FB_TCP_Client::begin(const char *host, uint16_t port, int *response_code)
{

  this->host = host;
  this->port = port;
  this->response_code = response_code;

  ethDNSWorkAround();

  wcs->setBufferSizes(bsslRxSize, bsslTxSize);

  return true;
}

int FB_TCP_Client::beginUpdate(int len, bool verify)
{
  int code = 0;
#if defined(ESP8266)
  if (len > (int)ESP.getFreeSketchSpace())
  {
    code = FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE;
  }
  else if (verify)
  {
    uint8_t buf[4];
    if (wcs->peekBytes(&buf[0], 4) != 4)
      code = FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE;
    else
    {

      // check for valid first magic byte
      if (buf[0] != 0xE9 && buf[0] != 0x1f)
      {
        code = FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE;
      }
      else if (buf[0] == 0xe9)
      {
        uint32_t bin_flash_size = ESP.magicFlashChipSize((buf[3] & 0xf0) >> 4);

        // check if new bin fits to SPI flash
        if (bin_flash_size > ESP.getFlashChipRealSize())
        {
          code = FIREBASE_ERROR_FW_UPDATE_BIN_SIZE_NOT_MATCH_SPI_FLASH_SPACE;
        }
      }
    }
  }

  if (code == 0)
  {
    if (!Update.begin(len, 0, -1, 0))
    {
      code = FIREBASE_ERROR_FW_UPDATE_BEGIN_FAILED;
    }
  }
#elif defined(MB_ARDUINO_PICO)
  if (!Update.begin(len))
    code = FIREBASE_ERROR_FW_UPDATE_BEGIN_FAILED;
#endif
  return code;
}

bool FB_TCP_Client::ethLinkUp()
{
  if (!eth && config)
    eth = &(config->spi_ethernet_module);

  if (!eth)
    return false;

  bool ret = false;
#if defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)

#if defined(INC_ENC28J60_LWIP)
  if (eth->enc28j60)
  {
    ret = eth->enc28j60->status() == WL_CONNECTED;
    goto ex;
  }
#endif
#if defined(INC_W5100_LWIP)
  if (eth->w5100)
  {
    ret = eth->w5100->status() == WL_CONNECTED;
    goto ex;
  }
#endif
#if defined(INC_W5500_LWIP)
  if (eth->w5500)
  {
    ret = eth->w5500->status() == WL_CONNECTED;
    goto ex;
  }
#endif

#elif defined(MB_ARDUINO_PICO)


#endif

  return ret;

#if defined(INC_ENC28J60_LWIP) || defined(INC_W5100_LWIP) || defined(INC_W5500_LWIP)
ex:
#endif

  // workaround for ESP8266 Ethernet
  delayMicroseconds(0);

  return ret;
}

void FB_TCP_Client::ethDNSWorkAround()
{
  if (!eth)
    return;

#if defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)

#if defined(INC_ENC28J60_LWIP)
  if (eth->enc28j60)
    goto ex;
#endif
#if defined(INC_W5100_LWIP)
  if (eth->w5100)
    goto ex;
#endif
#if defined(INC_W5500_LWIP)
  if (eth->w5500)
    goto ex;
#endif

#elif defined(MB_ARDUINO_PICO)


#endif

  return;

#if defined(INC_ENC28J60_LWIP) || defined(INC_W5100_LWIP) || defined(INC_W5500_LWIP) 
ex:
  WiFiClient _client;
  _client.connect(host.c_str(), port);
  _client.stop();
#endif
}

void FB_TCP_Client::release()
{
  if (wcs)
  {
    wcs->stop();
    wcs.reset(nullptr);
    wcs.release();

    if (x509)
      delete x509;

    baseSetCertType(fb_cert_type_undefined);
  }
  client = nullptr;
}

#endif /* ESP8266 */

#endif /* FB_TCP_Client_CPP */