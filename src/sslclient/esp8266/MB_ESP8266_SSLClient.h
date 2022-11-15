/**
 *
 * The Mobizt ESP8266 SSL Client Class, MB_ESP8266_SSLClient.h v1.0.1
 *
 * Created November 15, 2022
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

#ifndef MB_ESP8266_SSLCLIENT_H
#define MB_ESP8266_SSLCLIENT_H

#ifdef ESP8266

#include <vector>
#include <WiFiClient.h>
#include "MB_BearSSL.h"
#include <StackThunk.h>


#include "ESP8266_SSL_Client.h"

#define WCS_CLASS ESP8266_SSL_Client

class MB_ESP8266_SSLClient : public Client, public WCS_CLASS
{
  friend class ESP8266_TCP_Client;

public:
  MB_ESP8266_SSLClient();
  ~MB_ESP8266_SSLClient();

  /**
   * Set the client.
   * @param client The Client interface.
   */
  void setClient(Client *client);

  /**
   * Connect to server.
   * @param name The server host name to connect.
   * @param port The server port to connecte.
   * @return 1 for success or 0 for error.
   */
  int connect(const char *name, uint16_t port) override;

  /**
   * Connect to server.
   * @param ip The IP address to connect.
   * @param port The server port to connecte.
   * @return 1 for success or 0 for error.
   */
  int connect(IPAddress ip, uint16_t port) override;

  /**
   * Get TCP connection status.
   * @return 1 for connected or 0 for not connected.
   */
  uint8_t connected() override;

  /**
   * Get available data size to read.
   * @return The avaiable data size.
   */
  int available() override;

  /**
   * The TCP data read function.
   * @return A byte data that was successfully read or -1 for error.
   */
  int read() override;

  /**
   * The TCP data read function.
   * @param buf The data buffer.
   * @param size The length of data that read.
   * @return The size of data that was successfully read or -1 for error.
   */
  int read(uint8_t *buf, size_t size) override;

  /**
   * The TCP data write function.
   * @param buf The data to write.
   * @param size The length of data to write.
   * @return The size of data that was successfully written or 0 for error.
   */
  size_t write(const uint8_t *buf, size_t size) override;

  /**
   * The TCP data write function.
   * @param b The data to write.
   * @return 1 for success or 0 for error.
   */
  size_t write(uint8_t b) override { return write(&b, 1); }

  /**
   * Read one byte from stream.
   * @return The data that was successfully read or -1 for error.
   */
  int peek() override;

  /**
   * No certificate chain validation.
   */
  void setInSecure();

  /**
   * Upgrade the current connection by setting up the SSL and perform the SSL handshake.
   *
   * @return operating result.
   */
  bool connectSSL();

  /**
   * Close the connection
   */
  void stop() override;

  /**
   * Set the timeout when waiting for an SSL response.
   * @param timeout The timeout value, in milliseconds.
   */
  void setTimeout(unsigned long timeout);

  /**
   * Force writing the buffered bytes from write to the network.
   */
  void flush() override;

  /**
   *  Sets the requested buffer size for transmit and receive
   *  @param recv The receive buffer size.
   *  @param xmit The transmit buffer size.
   */
  void setBufferSizes(int recv, int xmit);

  /**
   * Equivalent to connected() > 0
   *
   * @returns true if connected, false if not
   */
  operator bool() { return connected() > 0; }

  int availableForWrite();

  void setSession(BearSSL_Session *session);

  void setKnownKey(const BearSSL_PublicKey *pk, unsigned usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN);

  bool setFingerprint(const uint8_t fingerprint[20]);

  bool setFingerprint(const char *fpStr);

  void allowSelfSignedCerts();

  void setTrustAnchors(const BearSSL_X509List *ta);

  void setX509Time(time_t now);

  void setClientRSACert(const BearSSL_X509List *cert, const BearSSL_PrivateKey *sk);

  void setClientECCert(const BearSSL_X509List *cert, const BearSSL_PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type);

  int getMFLNStatus();

  int getLastSSLError(char *dest = NULL, size_t len = 0);

  void setCertStore(BearSSL_CertStoreBase *certStore);

  bool setCiphers(const uint16_t *cipherAry, int cipherCount);

  bool setCiphers(const std::vector<uint16_t> &list);

  bool setCiphersLessSecure();

  bool setSSLVersion(uint32_t min = BR_TLS10, uint32_t max = BR_TLS12);

  bool hasPeekBufferAPI() const { return true; }

  size_t peekAvailable() { return available(); }

  const char *peekBuffer();

  void peekConsume(size_t consume);

private:
  String _host;
  uint16_t _port;

  Client *_basic_client = nullptr;
};

#endif /* ESP8266 */

#endif /* MB_ESP8266_SSLCLIENT_H */
