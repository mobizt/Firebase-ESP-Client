/**
 * Firebase TCP Client v1.1.17
 * 
 * Created January 18, 2022
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#ifdef ESP8266

#include <Arduino.h>

#include "mbfs/MB_FS.h"
#include "FB_Net.h"
#include "FB_Error.h"

class FB_TCP_Client
{

  friend class FirebaseData;
  friend class FB_RTDB;
  friend class FB_CM;
  friend class FB_CloudStorage;
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
  bool connected(void);

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

  void setCACert(const char *caCert);
  void setCACertFile(const char *caCertFile, mb_fs_mem_storage_type storageType);
  bool connect(void);
  void setMBFS(MB_FS *mbfs);

private:
  std::unique_ptr<FB_ESP_SSL_CLIENT> _wcs = std::unique_ptr<FB_ESP_SSL_CLIENT>(new FB_ESP_SSL_CLIENT());
  MB_String _host;
  uint16_t _port = 0;

  //Actually Arduino base Stream (char read) timeout.
  //This will override internally by WiFiClientSecureCtx::_connectSSL
  //to 5000 after SSL handshake was done with success.
  unsigned long timeout = 10 * 1000;

  MB_String _CAFile;
  uint8_t _CAFileStoreageType = 0;
  fb_cert_type _certType = fb_cert_type_undefined;
  bool _clockReady = false;
  uint16_t _bsslRxSize = 512;
  uint16_t _bsslTxSize = 512;
  bool fragmentable = false;
  int chunkSize = 1024;
  bool mflnChecked = false;
  X509List *x509 = nullptr;
  MB_FS *mbfs = nullptr;

  void release();
};

#endif /* ESP8266 */

#endif /* FB_TCP_Client_H */