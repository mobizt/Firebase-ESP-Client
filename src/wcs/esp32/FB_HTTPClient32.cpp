/**
 * Customized version of ESP32 HTTPClient Library. 
 * Allow custom header and payload
 * 
 * v 1.0.5
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
    transportTraits = FB_ESP_TransportTraitsPtr(new TLSTraits(nullptr));
    _wcs = transportTraits->create();
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
    transportTraits.reset(nullptr);
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

bool FB_HTTPClient32::send(const char *header)
{
    if (!connected())
        return false;
    return (_wcs->write(header, strlen(header)) == strlen(header));
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
        if (_wcs->write(&payload[0], size) != size)
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

    if (!transportTraits)
        return false;

    transportTraits->verify(*_wcs, _host.c_str());
    if (!_wcs->connect(_host.c_str(), _port))
        return false;

    return connected();
}

void FB_HTTPClient32::setCACert(const char *caCert)
{
    if (caCert)
    {
        transportTraits.reset(nullptr);
        transportTraits = FB_ESP_TransportTraitsPtr(new TLSTraits(caCert));
        _certType = 1;
    }
    else
        _certType = 0;
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
            t = SD.begin();
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
            if (SD.exists(caCertFile))
                f = SD.open(caCertFile, FILE_READ);
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

            transportTraits.reset(nullptr);
            transportTraits = FB_ESP_TransportTraitsPtr(new TLSTraits(_cacert.get()));
        }
    }
}

#endif /* ESP32 */

#endif /* FirebaseESP32HTTPClient_CPP */
