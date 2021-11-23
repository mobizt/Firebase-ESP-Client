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

#ifndef FB_TCP_Client_CPP
#define FB_TCP_Client_CPP

#ifdef ESP8266

#include "FB_TCP_Client.h"

FB_TCP_Client::FB_TCP_Client()
{
}

FB_TCP_Client::~FB_TCP_Client()
{
  release();
  MBSTRING().swap(_host);
  MBSTRING().swap(_CAFile);
}

bool FB_TCP_Client::begin(const char *host, uint16_t port)
{
  if (strcmp(_host.c_str(), host) != 0)
    mflnChecked = false;

  _host = host;
  _port = port;

  //probe for fragmentation support at the specified size
  if (!mflnChecked)
  {
    fragmentable = _wcs->probeMaxFragmentLength(_host.c_str(), _port, chunkSize);
    if (fragmentable)
    {
      _bsslRxSize = chunkSize;
      _bsslTxSize = chunkSize;
      _wcs->setBufferSizes(_bsslRxSize, _bsslTxSize);
    }
    mflnChecked = true;
  }

  if (!fragmentable)
    _wcs->setBufferSizes(_bsslRxSize, _bsslTxSize);

  return true;
}

bool FB_TCP_Client::connected()
{
  if (_wcs)
    return (_wcs->connected());
  return false;
}

int FB_TCP_Client::send(const char *data, size_t len)
{
  if (!connect())
    return FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED;

  if (len == 0)
    len = strlen(data);

  if (len == 0)
    return 0;

  if (_wcs->write((const uint8_t *)data, len) != len)
    return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;

  return 0;
}

WiFiClient *FB_TCP_Client::stream(void)
{
  if (connected())
    return _wcs.get();
  return nullptr;
}

bool FB_TCP_Client::connect(void)
{
  if (connected())
  {
    while (_wcs->available() > 0)
      _wcs->read();
    return true;
  }

  _wcs->setTimeout(timeout);

  if (!_wcs->connect(_host.c_str(), _port))
    return false;

  return connected();
}

void FB_TCP_Client::release()
{
  if (_wcs)
  {
    _wcs->stop();
    _wcs.reset(nullptr);
    _wcs.release();
  }
  if (x509)
    delete x509;
}

void FB_TCP_Client::setCACert(const char *caCert)
{

  release();

  _wcs = std::unique_ptr<FB_ESP_SSL_CLIENT>(new FB_ESP_SSL_CLIENT());

  _wcs->setBufferSizes(_bsslRxSize, _bsslTxSize);

  if (caCert)
  {
    x509 = new X509List(caCert);
    _wcs->setTrustAnchors(x509);
    _certType = 1;
  }
  else
  {
    _wcs->setInsecure();
    _certType = 0;
  }

  _wcs->setNoDelay(true);
}

void FB_TCP_Client::setCACertFile(const char *caCertFile, uint8_t storageType, struct fb_esp_sd_config_info_t sd_config)
{
  _sdPin = sd_config.ss;
  _wcs->setBufferSizes(_bsslRxSize, _bsslTxSize);

  if (_clockReady && strlen(caCertFile) > 0)
  {
    fs::File f;
    if (storageType == 1)
    {
#if defined FLASH_FS
      FLASH_FS.begin();
      if (FLASH_FS.exists(caCertFile))
        f = FLASH_FS.open(caCertFile, "r");
#endif
    }
    else if (storageType == 2)
    {
#if defined SD_FS
      SD_FS.begin(_sdPin);
      if (SD_FS.exists(caCertFile))
        f = SD_FS.open(caCertFile, FILE_READ);
#endif
    }
    if (f)
    {
      size_t len = f.size();
      uint8_t *der = new uint8_t[len];
      if (f.available())
        f.read(der, len);
      f.close();
      _wcs->setTrustAnchors(new X509List(der, len));
      delete[] der;
    }
    _certType = 2;
  }
  _wcs->setNoDelay(true);
}

#endif /* ESP8266 */

#endif /* FB_TCP_Client_CPP */