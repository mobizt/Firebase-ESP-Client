#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40318)
#error "Mixed versions compilation."
#endif

/**
 * Firebase TCP Client v1.1.27
 *
 * Created July 10, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 *
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the HTTPClient for Arduino.
 * Port to ESP32 by Evandro Luis Copercini (2017),
 * changed fingerprints to CA verification.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef FB_TCP_Client_CPP
#define FB_TCP_Client_CPP

#if defined(ESP32) && !defined(FB_ENABLE_EXTERNAL_CLIENT)

#include "FB_TCP_Client.h"

FB_TCP_Client::FB_TCP_Client(bool initSSLClient)
{
  if (!wcs && initSSLClient)
  {
    wcs = std::unique_ptr<FB_WCS>(new FB_WCS());
    client = wcs.get();
  }
}

FB_TCP_Client::~FB_TCP_Client()
{
  release();
}

void FB_TCP_Client::setInsecure()
{
  if (!wcs)
    return;

#if __has_include(<esp_idf_version.h>)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(3, 3, 0)
  wcs->setInsecure();
#endif
#endif
}

void FB_TCP_Client::setCACert(const char *caCert)
{
  release();

  wcs = std::unique_ptr<FB_WCS>(new FB_WCS());
  client = wcs.get();

  if (caCert != NULL)
  {
    baseSetCertType(fb_cert_type_data);
    wcs->setCACert(caCert);
  }
  else
  {
    wcs->stop();
    wcs->setCACert(NULL);
    setInsecure();
    baseSetCertType(fb_cert_type_none);
  }
}

bool FB_TCP_Client::setCertFile(const char *caCertFile, mb_fs_mem_storage_type storageType)
{
  if (!wcs)
    return false;

  if (strlen(caCertFile) > 0)
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

      if (storageType == mb_fs_mem_storage_type_flash)
      {
#if defined(MBFS_FLASH_FS)
        fs::File file = mbfs->getFlashFile();
        wcs->loadCACert(file, len);
        baseSetCertType(fb_cert_type_file);
#endif
        mbfs->close(storageType);
      }
      else if (storageType == mb_fs_mem_storage_type_sd)
      {

#if defined(MBFS_ESP32_SDFAT_ENABLED)

        if (cert)
          MemoryHelper::freeBuffer(mbfs, cert);

        cert = MemoryHelper::createBuffer<char *>(mbfs, len);
        if (mbfs->available(storageType))
          mbfs->read(storageType, (uint8_t *)cert, len);

        wcs->setCACert((const char *)cert);
        baseSetCertType(fb_cert_type_file);

#elif defined(MBFS_SD_FS)
        fs::File file = mbfs->getSDFile();
        wcs->loadCACert(file, len);
        baseSetCertType(fb_cert_type_file);
#endif
        mbfs->close(storageType);
      }
    }
  }

  return getCertType() == fb_cert_type_file;
}

bool FB_TCP_Client::networkReady()
{
  return (WiFi.status() == WL_CONNECTED && validIP(WiFi.localIP())) || ethLinkUp();
}

void FB_TCP_Client::networkReconnect()
{
  esp_wifi_connect();
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
  baseSetTimeout(timeoutmSec / 1000);
}

bool FB_TCP_Client::begin(const char *host, uint16_t port, int *response_code)
{
  if (!wcs)
    return false;

  this->host = host;
  this->port = port;
  this->response_code = response_code;
  return true;
}

// override the base connect
bool FB_TCP_Client::connect()
{
  if (!wcs)
    return false;

  if (connected())
  {
    flush();
    return true;
  }

  lastConnMillis = millis();
  if (!wcs->_connect(host.c_str(), port, timeoutMs))
    return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

  wcs->setTimeout(timeoutMs);

#if defined(USE_CONNECTION_KEEP_ALIVE_MODE) // Use TCP KeepAlive (interval connection probing) together with HTTP connection Keep-Alive
  if (isKeepAliveSet())
  {
    if (tcpKeepIdleSeconds == 0 || tcpKeepIntervalSeconds == 0 || tcpKeepCount == 0)
    {
      tcpKeepIdleSeconds = 0;
      tcpKeepIntervalSeconds = 0;
      tcpKeepCount = 0;
    }

    bool success = wcs->setOption(TCP_KEEPIDLE, &tcpKeepIdleSeconds) > -1 &&
                   wcs->setOption(TCP_KEEPINTVL, &tcpKeepIntervalSeconds) > -1 &&
                   wcs->setOption(TCP_KEEPCNT, &tcpKeepCount) > -1;
    if (!success)
      isKeepAlive = false;
  }
#endif

  return connected();
}

bool FB_TCP_Client::ethLinkUp()
{
#if !defined(FB_ENABLE_EXTERNAL_CLIENT)
  if (validIP(ETH.localIP()))
  {
    ETH.linkUp();
    return true;
  }
#endif
  return false;
}

bool FB_TCP_Client::validIP(IPAddress ip)
{
    return strcmp(ip.toString().c_str(), "0.0.0.0") != 0;
}

void FB_TCP_Client::release()
{
  if (wcs)
  {
    wcs->stop();
    wcs.reset(nullptr);
    wcs.release();

    if (cert)
      MemoryHelper::freeBuffer(mbfs, cert);

    baseSetCertType(fb_cert_type_undefined);
  }
}

#endif /* ESP32 */

#endif /* FirebaseESP32HTTPClient_CPP */
