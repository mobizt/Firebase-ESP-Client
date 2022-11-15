/*
 * The Mobizt ESP32 SSL Client Class, MB_ESP32_SSLClient.h v1.0.2
 *
 * Created November 15, 2022
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 *
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
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

/*
  WiFiClientSecure.h - Base class that provides Client SSL to ESP32
  Copyright (c) 2011 Adrian McEwen.  All right reserved.
  Additions Copyright (C) 2017 Evandro Luis Copercini.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef MB_ESP32_SSLCLIENT_H
#define MB_ESP32_SSLCLIENT_H

#ifdef ESP32

#include "Arduino.h"
#include "IPAddress.h"
#include "ESP32_SSL_Client.h"
#include <WiFiClient.h>
#include <string>


typedef void (*DebugMsgCallback)(PGM_P msg, bool newLine);

#define WCS_CLASS ESP32_SSL_Client
#define WC_CLASS ESP32_SSL_Client

class MB_ESP32_SSLClient : public WCS_CLASS, public Client
{

protected:
    // The mbedTLS last error code.
    int _lastError = 0;

    // milliseconds time out
    int _timeout = 0;

    // Root CA cert verification and authentication option
    bool _use_insecure;

    const char *_CA_cert;
    const char *_cert;
    const char *_private_key;
    const char *_pskIdent; // identity for PSK cipher suites
    const char *_psKey;    // key in hex for PSK cipher suites

public:
    MB_ESP32_SSLClient *next;

    // The default class constructor
    MB_ESP32_SSLClient();

    // The class deconstructor
    ~MB_ESP32_SSLClient();

    /**
     * Set the client.
     * @param client The Client interface.
     */
    void setClient(Client *client);

    /**
     * Connect to server.
     * @param ip The server IP to connect.
     * @param port The server port to connecte.
     * @return 1 for success or 0 for error.
     */
    int connect(IPAddress ip, uint16_t port) override;

    /**
     * Connect to server.
     * @param ip The server IP to connect.
     * @param port The server port to connecte.
     * @param timeout The connection time out in miiliseconds.
     * @return 1 for success or 0 for error.
     */
    int connect(IPAddress ip, uint16_t port, int32_t timeout);

    /**
     * Connect to server.
     * @param host The server host name.
     * @param port The server port to connecte.
     * @return 1 for success or 0 for error.
     */
    int connect(const char *host, uint16_t port) override;

    /**
     * Connect to server.
     * @param host The server host name.
     * @param port The server port to connecte.
     * @param timeout The connection time out in miiliseconds.
     * @return 1 for success or 0 for error.
     */
    int connect(const char *host, uint16_t port, int32_t timeout);

    /**
     * Connect to server.
     * @param ip The server IP to connect.
     * @param port The server port to connecte.
     * @param rootCABuff The server's root CA or CA cert.
     * @param cli_cert The client cert.
     * @param cli_key The private key.
     * @return 1 for success or 0 for error.
     */
    int connect(IPAddress ip, uint16_t port, const char *rootCABuff, const char *cli_cert, const char *cli_key);

    /**
     * Connect to server.
     * @param host The server host name to connect.
     * @param port The server port to connecte.
     * @param rootCABuff The server's root CA or CA cert.
     * @param cli_cert The client cert.
     * @param cli_key The private key.
     * @return 1 for success or 0 for error.
     */
    int connect(const char *host, uint16_t port, const char *rootCABuff, const char *cli_cert, const char *cli_key);

    /**
     * Connect to server.
     * @param ip The server IP to connect.
     * @param port The server port to connecte.
     * @param pskIdent The Pre Shared Key identity.
     * @param psKey The Pre Shared Key.
     * @return 1 for success or 0 for error.
     */
    int connect(IPAddress ip, uint16_t port, const char *pskIdent, const char *psKey);

    /**
     * Connect to server.
     * @param host The server host name to connect.
     * @param port The server port to connecte.
     * @param pskIdent The Pre Shared Key identity.
     * @param psKey The Pre Shared Key.
     * @return 1 for success or 0 for error.
     */
    int connect(const char *host, uint16_t port, const char *pskIdent, const char *psKey);

    /**
     * Read one byte from Stream with time out.
     * @return The byte of data that was successfully read or -1 for timed out.
     */
    int peek() override;

    /**
     * The TCP data write function.
     * @param data The byte of data to write.
     * @return The size of data that was successfully written (1) or 0 for error.
     * @note Send data directly via lwIP for non-secure mode or via mbedTLS to encrypt for secure mode.
     */
    size_t write(uint8_t data) override;

    /**
     * The TCP data write function.
     * @param buf The data to write.
     * @param size The length of data to write.
     * @return The size of data that was successfully written or 0 for error.
     * @note Send data directly via lwIP for non-secure mode or via mbedTLS to encrypt for secure mode.
     */
    size_t write(const uint8_t *buf, size_t size) override;

    /**
     * Get available data size to read.
     * @return The avaiable data size.
     * @note Get available data directly via lwIP for non-secure mode or via mbedTLS for secure mode.
     */
    int available() override;

    /**
     * The TCP data read function.
     * @return A byte data that was successfully read or -1 for error.
     * @note Get data directly via lwIP for non-secure mode or via mbedTLS to deccrypt data for secure mode.
     */
    int read() override;

    /**
     * The TCP data read function.
     * @param buf The data buffer.
     * @param size The length of data that read.
     * @return The size of data that was successfully read or 0 for error.
     * @note Get data directly via lwIP for non-secure mode or via mbedTLS to deccrypt data for secure mode.
     */
    int read(uint8_t *buf, size_t size) override;

    /**
     * Wait for all receive buffer data read.
     */
    void flush() override;

    /**
     * Stop the TCP connection and release resources.
     */
    void stop() override;

    /**
     * Get TCP connection status.
     * @return 1 for connected or 0 for not connected.
     */
    uint8_t connected() override;

    /**
     * Get mbedTLS last error info.
     * @param buf The data buffer to keep error info.
     * @param size The length of buffer data.
     * @return The last mbedTLS error.
     */
    int lastError(char *buf, const size_t size);

    /**
     * Disable certificate verification and ignore the authentication.
     */
    void setInsecure(); // Don't validate the chain, just accept whatever is given.  VERY INSECURE!

    /**
     * Set the Pre Shared Key and its identity.
     * @param pskIdent The Pre Shared Key identity.
     * @param psKey The Pre Shared Key.
     */
    void setPreSharedKey(const char *pskIdent, const char *psKey); // psKey in Hex

    /**
     * Set the Root CA or CA certificate.
     * @param rootCA The Root CA or CA certificate.
     */
    void setCACert(const char *rootCA);

    /**
     * Set client certificate.
     * @param client_ca The public certificate.
     */
    void setCertificate(const char *client_ca);

    /**
     * Set private key.
     * @param private_key The private key.
     */
    void setPrivateKey(const char *private_key);

    /**
     * Read and set CA cert from file (Stream).
     * @param stream The Stream interface.
     * @param size The size of data to read.
     * @return The operating result.
     */
    bool loadCACert(Stream &stream, size_t size);

    /**
     * Read and set client cert from file (Stream).
     * @param stream The Stream interface.
     * @param size The size of data to read.
     * @return The operating result.
     */
    bool loadCertificate(Stream &stream, size_t size);

    /**
     * Read and set private key from file (Stream).
     * @param stream The Stream interface.
     * @param size The size of data to read.
     * @return The operating result.
     */
    bool loadPrivateKey(Stream &stream, size_t size);

    /**
     * Verify certificate's SHA256 fingerprint.
     *
     * @param ssl The pointer to ssl data (context).
     * @param fp The certificate's SHA256 fingerprint data to compare with server certificate's SHA256 fingerprint.
     * @param domain_name The optional domain name to check in server certificate.
     * @return verification result.
     */
    bool verify(const char *fingerprint, const char *domain_name);

    /**
     * Set the SSL handshake timeout in seconds.
     * @param handshake_timeout The SSL handshake timeout in seconds.
     */
    void setHandshakeTimeout(unsigned long handshake_timeout);

    /**
     * Set the TCP timeout in seconds (not implemented).
     * @param seconds The TCP timeout in seconds.
     */
    int setTimeout(uint32_t seconds) { return 0; }

    /**
     * Set the debug callback.
     * @param cb The callback function.
     */
    void setDebugCB(DebugMsgCallback *cb);

    /**
     * Upgrade the current connection by setting up the SSL and perform the SSL handshake.
     *
     * @return operating result.
     */
    bool connectSSL();

    operator bool()
    {
        return connected();
    }

    MB_ESP32_SSLClient &operator=(const MB_ESP32_SSLClient &other);
    bool operator==(const bool value)
    {
        return bool() == value;
    }

    bool operator!=(const bool value)
    {
        return bool() != value;
    }

    bool operator==(const MB_ESP32_SSLClient &);
    
    bool operator!=(const MB_ESP32_SSLClient &rhs)
    {
        return !this->operator==(rhs);
    };

private:
    String _host;
    uint16_t _port;
    ssl_ctx *_ssl;
    bool _withCert = false;
    bool _withKey = false;

    char *_streamLoad(Stream &stream, size_t size);
    int _connect(const char *host, uint16_t port);
};

#endif // ESP32

#endif // MB_ESP32_SSLCLIENT_H