/**
 * Firebase TCP Client v1.1.17
 * 
 * Created January 18, 2022
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

#ifndef FB_TCP_Client_H
#define FB_TCP_Client_H

#ifdef ESP32

#include "FB_Net.h"
#include "FB_Error.h"
#include "mbfs/MB_FS.h"


//The derived class to fix the memory leaks issue
//https://github.com/espressif/arduino-esp32/issues/5480
class FB_WCS : public WiFiClientSecure
{
public:
  FB_WCS(){};
  ~FB_WCS(){};

  int _connect(const char *host, uint16_t port, unsigned long timeout)
  {
    _timeout = timeout;

    if (connect(host, port) == 0)
    {
      if (_CA_cert != NULL)
        mbedtls_x509_crt_free(&sslclient->ca_cert);
      return 0;
    }
    _connected = true;
    return 1;
  }
};

class FB_TCP_Client
{

  friend class FirebaseData;
  friend class FB_RTDB;
  friend class FB_CM;
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
  bool connected();

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

  /**
   * Set insecure mode
  */
  void setInsecure();

  void stop();

  bool connect(void);
  void setCACert(const char *caCert);
  void setCACertFile(const char *caCertFile, mb_fs_mem_storage_type storageType);
  void setMBFS(MB_FS *mbfs);

private:
  std::unique_ptr<FB_WCS> _wcs = std::unique_ptr<FB_WCS>(new FB_WCS());
  MB_String _host;
  uint16_t _port = 0;

  //lwIP socket connection and ssl handshake timeout
  unsigned long timeout = 10 * 1000;

  MB_String _CAFile;
  uint8_t _CAFileStoreageType = 0;
  int _certType = fb_cert_type_undefined;
  bool _clockReady = false;
  MB_FS *mbfs = nullptr;
  char *cert = NULL;
  void release();
};

#endif /* ESP32 */

#endif /* FB_TCP_Client_H */
