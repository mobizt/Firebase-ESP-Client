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

#ifndef FB_HTTPClient32_H
#define FB_HTTPClient32_H

#ifdef ESP32

#include <Arduino.h>
#include <WiFiClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>
#include "wcs/esp32/FB_WCS32.h"
#if __has_include(<WiFiEspAT.h>) || __has_include(<espduino.h>)
#error WiFi UART bridge was not supported.
#endif

#include "wcs/HTTPCode.h"

#define FB_ESP_SSL_CLIENT WiFiClient

class TransportTraits
{
public:
  virtual ~TransportTraits() {}

  virtual std::unique_ptr<WiFiClient> create()
  {
    return std::unique_ptr<WiFiClient>(new WiFiClient());
  }

  virtual bool
  verify(WiFiClient &client, const char *host)
  {
    return true;
  }
};

class TLSTraits : public TransportTraits
{
public:
  TLSTraits(const char *CAcert, const char *clicert = nullptr, const char *clikey = nullptr) : _cacert(CAcert), _clicert(clicert), _clikey(clikey) {}

  std::unique_ptr<WiFiClient> create() override
  {
    return std::unique_ptr<WiFiClient>(new FB_WCS32());
  }

  bool verify(WiFiClient &client, const char *host) override
  {
    FB_WCS32 &wcs = static_cast<FB_WCS32 &>(client);
    wcs.setCACert(_cacert);
    wcs.setCertificate(_clicert);
    wcs.setPrivateKey(_clikey);
    return true;
  }

protected:
  const char *_cacert;
  const char *_clicert;
  const char *_clikey;
};

typedef std::unique_ptr<TransportTraits> FB_ESP_TransportTraitsPtr;

class FB_HTTPClient32
{

  friend class FirebaseData;
  friend class FB_RTDB;
  friend class FB_CM;
  friend class UtilsClass;

public:
  FB_HTTPClient32();
  ~FB_HTTPClient32();

  /**
    * Initialization of new http connection.
    * \param host - Host name without protocols.
    * \param port - Server's port.
    * \return True as default.
    * If no certificate string provided, use (const char*)NULL to CAcert param 
    */
  bool begin(const char *host, uint16_t port);

  /**
    * Check the http connection status.
    * \return True if connected.
    */
  bool connected();

  /**
    * Establish http connection if header provided and send it, send payload if provided.
    * \param header - The header string (constant chars array).
    * \param payload - The payload string (constant chars array), optional.
    * \return http status code, Return zero if new http connection and header and/or payload sent 
    * with no error or no header and payload provided. If obly payload provided, no new http connection was established.
    */
  int send(const char *header, const char *payload);

  /**
    * Send extra header without making new http connection (if send with header has been called)
    * \param header - The header string (constant chars array).
    * \return True if header sending success.
    * Need to call send with header first. 
    */
  bool send(const char *header);

  /**
    * Get the WiFi client pointer.
    * \return WiFi client pointer.
    */
  WiFiClient *stream(void);

  bool connect(void);
  void setCACert(const char *caCert);
  void setCACertFile(const char *caCertFile, uint8_t storageType, uint8_t sdPin);

protected:
  FB_ESP_TransportTraitsPtr transportTraits;
  std::unique_ptr<FB_ESP_SSL_CLIENT> _wcs;
  std::unique_ptr<char> _cacert;
  std::string _host = "";
  uint16_t _port = 0;
  unsigned long timeout = FIREBASE_DEFAULT_TCP_TIMEOUT * 1000;

  std::string _CAFile = "";
  uint8_t _CAFileStoreageType = 0;
  int _certType = -1;
  bool _clockReady = false;
};

#endif /* ESP32 */

#endif /* FirebaseESP32HTTPClient_H_ */
