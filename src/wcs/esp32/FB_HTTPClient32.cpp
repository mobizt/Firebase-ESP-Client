/**
 * Customized version of ESP32 HTTPClient Library. 
 * Allow custom header and payload
 * 
 * v 1.0.6
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * 
 * HTTPClient Arduino library for ESP32
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

#ifndef FB_HTTPClient32_CPP
#define FB_HTTPClient32_CPP

#ifdef ESP32

#include "FB_HTTPClient32.h"

FB_HTTPClient32::FB_HTTPClient32()
{
}

FB_HTTPClient32::~FB_HTTPClient32()
{
    if (_wcs)
    {
        _wcs->stop();
        _wcs.reset(nullptr);
        _wcs.release();
    }
    std::string().swap(_host);
    std::string().swap(_CAFile);
    _cacert.reset(new char);
    _cacert = nullptr;
}

bool FB_HTTPClient32::begin(const char *host, uint16_t port)
{
    _host = host;
    _port = port;
    return true;
}

bool FB_HTTPClient32::connected()
{
    if (_wcs)
        return (_wcs->connected());
    return false;
}

void FB_HTTPClient32::stop()
{
    if (!connected())
        return;
    return _wcs->stop();
}

bool FB_HTTPClient32::send(const char *header)
{
    if (!connected())
        return false;
    return (_wcs->print(header) == strlen(header));
}

int FB_HTTPClient32::send(const char *header, const char *payload)
{
    size_t size = strlen(payload);
    if (strlen(header) > 0)
    {
        if (!connect())
        {
            return FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_REFUSED;
        }

        if (!send(header))
        {
            return FIREBASE_ERROR_HTTPC_ERROR_SEND_HEADER_FAILED;
        }
    }

    if (size > 0)
    {
        if (_wcs->print(payload) != size)
        {
            return FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED;
        }
    }

    return 0;
}

WiFiClient *FB_HTTPClient32::stream(void)
{
    if (connected())
        return _wcs.get();
    return nullptr;
}

bool FB_HTTPClient32::connect(void)
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

void FB_HTTPClient32::setCACert(const char *caCert)
{
    _wcs->setCACert(caCert);
    if (caCert)
        _certType = 1;
    else
    {

#ifdef CONFIG_ARDUINO_IDF_BRANCH
        size_t len = strlen_P(esp_idf_branch_str);
        char *tmp = new char[len + 1];
        memset(tmp, 0, len + 1);
        std::string s = CONFIG_ARDUINO_IDF_BRANCH;
        size_t p1 = s.find(tmp, 0);
        if (p1 != std::string::npos)
        {
            float v = atof(s.substr(p1 + len, s.length() - p1 - len).c_str());
            if (v >= 3.3f)
                _wcs->setInsecure();
        }
        delete[] tmp;
#endif
        _certType = 0;
    }
    //_wcs->setNoDelay(true);
}

void FB_HTTPClient32::setCACertFile(const char *caCertFile, uint8_t storageType, uint8_t sdPin)
{

    if (strlen(caCertFile) > 0)
    {
        bool t = false;
        _certType = 2;

        if (storageType == 0)
            t = FLASH_FS.begin(true);
        else
            t = SD_FS.begin();
        if (!t)
            return;

        File f;
        if (storageType == 1)
        {
            if (FLASH_FS.exists(caCertFile))
                f = SPIFFS.open(caCertFile, FILE_READ);
        }
        else if (storageType == 2)
        {
            if (SD_FS.exists(caCertFile))
                f = SD_FS.open(caCertFile, FILE_READ);
        }

        if (f)
        {
            size_t len = f.size();
            _cacert.reset(new char);
            _cacert = nullptr;
            _cacert = std::unique_ptr<char>(new char[len]);

            if (f.available())
                f.readBytes(_cacert.get(), len);

            f.close();
            _wcs->setCACert(_cacert.get());
        }
    }
    //_wcs->setNoDelay(true);
}

#endif /* ESP32 */

#endif /* FirebaseESP32HTTPClient_CPP */
