/**
 * Firebase TCP Client v1.1.17
 * 
 * Created Created January 18, 2022
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#ifdef ESP32

#include "FB_TCP_Client.h"

FB_TCP_Client::FB_TCP_Client() {}

FB_TCP_Client::~FB_TCP_Client()
{
  if (_wcs)
  {
    _wcs->stop();
    _wcs.reset(nullptr);
    _wcs.release();
  }
  _host.clear();
  _CAFile.clear();

  if (cert)
    mbfs->delP(&cert);
}

bool FB_TCP_Client::begin(const char *host, uint16_t port)
{
  _host = host;
  _port = port;
  return true;
}

bool FB_TCP_Client::connected()
{
  if (_wcs)
    return (_wcs->connected());
  return false;
}

void FB_TCP_Client::stop()
{
  if (!connected())
    return;
  return _wcs->stop();
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

  if (!_wcs->_connect(_host.c_str(), _port, timeout))
    return false;

  return connected();
}

void FB_TCP_Client::setInsecure()
{
#if __has_include(<esp_idf_version.h>)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(3, 3, 0)
  _wcs->setInsecure();
#endif
#endif
}

void FB_TCP_Client::setCACert(const char *caCert)
{
  release();

  _wcs = std::unique_ptr<FB_WCS>(new FB_WCS());

  if (caCert != NULL)
  {
    _certType = fb_cert_type_data;
    _wcs->setCACert(caCert);
  }
  else
  {
    _wcs->stop();
    _wcs->setCACert(NULL);
    setInsecure();
    _certType = fb_cert_type_none;
  }
  //_wcs->setNoDelay(true);
}

void FB_TCP_Client::setCACertFile(const char *caCertFile, mb_fs_mem_storage_type storageType)
{
  if (!mbfs)
    return;

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
        fs::File file = mbfs->getFlashFile();
        _wcs->loadCACert(file, len);
        mbfs->close(storageType);
      }
      else if (storageType == mb_fs_mem_storage_type_sd)
      {

#if defined(MBFS_ESP32_SDFAT_ENABLED)

        if (cert)
          mbfs->delP(&cert);

        cert = (char *)mbfs->newP(len);
        if (mbfs->available(storageType))
          mbfs->read(storageType, (uint8_t *)cert, len);

        mbfs->close(storageType);
        _wcs->setCACert((const char *)cert);
        _certType = fb_cert_type_file;

#elif defined(MBFS_SD_FS)
        fs::File file = mbfs->getSDFile();
        _wcs->loadCACert(file, len);
        mbfs->close(storageType);
        _certType = fb_cert_type_file;

#endif
      }
    }
  }
}

void FB_TCP_Client::release()
{
  if (_wcs)
  {
    _wcs->stop();
    _wcs.reset(nullptr);
    _wcs.release();

    if (cert)
      mbfs->delP(&cert);

    _certType = fb_cert_type_undefined;
  }
}

void FB_TCP_Client::setMBFS(MB_FS *mbfs)
{
  this->mbfs = mbfs;
}

#endif /* ESP32 */

#endif /* FirebaseESP32HTTPClient_CPP */
