/**
 * Firebase TCP Client v1.1.9
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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
  std::string().swap(_host);
  std::string().swap(_CAFile);
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

int FB_TCP_Client::send(const char *data)
{
  if (!connect())
    return FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED;

  if (_wcs->write((const uint8_t *)data, strlen(data)) != strlen(data))
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

  if (!_wcs->connect(_host.c_str(), _port))
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
  _wcs->setCACert(caCert);
  if (caCert)
    _certType = 1;
  else
  {
    setInsecure();
    _certType = 0;
  }
  //_wcs->setNoDelay(true);
}

void FB_TCP_Client::setCACertFile(const char *caCertFile, uint8_t storageType, struct fb_esp_sd_config_info_t sd_config)
{

  if (strlen(caCertFile) > 0)
  {
    _certType = 2;

    File f;
    if (storageType == 1)
    {
      if (FORMAT_FLASH == 1)
        FLASH_FS.begin(true);
      else
        FLASH_FS.begin();
      if (FLASH_FS.exists(caCertFile))
        f = FLASH_FS.open(caCertFile, FILE_READ);
    }
    else if (storageType == 2)
    {
      if (sd_config.ss > -1)
      {
#ifdef CARD_TYPE_SD
        SPI.begin(sd_config.sck, sd_config.miso, sd_config.mosi, sd_config.ss);
        SD_FS.begin(sd_config.ss, SPI);
#endif
#ifdef CARD_TYPE_SD_MMC
        SD_FS.begin(sd_config.sd_mmc_mountpoint, sd_config.sd_mmc_mode1bit, sd_config.sd_mmc_format_if_mount_failed);
#endif
      }
      else
        SD_FS.begin();

      if (SD_FS.exists(caCertFile))
        f = SD_FS.open(caCertFile, FILE_READ);
    }

    if (f)
    {
      size_t len = f.size();
      _wcs->loadCACert(f, len);
      f.close();
    }
  }
}

#endif /* ESP32 */

#endif /* FirebaseESP32HTTPClient_CPP */
