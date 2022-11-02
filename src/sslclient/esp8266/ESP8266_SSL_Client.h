
/**
 *
 * The Network Upgradable BearSSL Client Class, ESP8266_SSL_Client.h v2.0.0
 *
 * Created July 20, 2022
 *
 * This works based on Earle F. Philhower ServerSecure class
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

/*
  WiFiClientBearSSL- SSL client/server for esp8266 using BearSSL libraries
  - Mostly compatible with Arduino WiFi shield library and standard
    WiFiClient/ServerSecure (except for certificate handling).

  Copyright (c) 2018 Earle F. Philhower, III

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

#ifndef ESP8266_SSL_Client_H
#define ESP8266_SSL_Client_H

#if defined(ESP8266)

#include <Arduino.h>
#include <memory>
#include <Client.h>
#include <Stream.h>
#include "MB_BearSSL.h"
#include "PolledTimeout.h"
#if defined(ESP8266_CORE_SDK_V3_X_X)
#include <umm_malloc/umm_heap_select.h>
#endif

class ESP8266_SSL_Client
{

public:
    ESP8266_SSL_Client();
    ESP8266_SSL_Client(const ESP8266_SSL_Client &rhs) = delete;
    ~ESP8266_SSL_Client();

    ESP8266_SSL_Client &operator=(const ESP8266_SSL_Client &) = delete;

    void setClient(Client *client);
    void setTimeout(unsigned long timeout);

    int connect(IPAddress ip, uint16_t port);
    int connect(const String &host, uint16_t port);
    bool connect(const char *name, uint16_t port);

    bool connected();
    size_t write(const uint8_t *buf, size_t size);
    size_t write_P(PGM_P buf, size_t size);
    size_t write(Stream &stream);
    int read(uint8_t *buf, size_t size);
    int read(char *buf, size_t size) { return read((uint8_t *)buf, size); }
    int readBytes(uint8_t *buf, size_t size) { return read(buf, size); }
    int available();
    int read();
    int peek();
    size_t peekBytes(uint8_t *buffer, size_t length);
    void flush();
    void stop();

    int availableForWrite();

    // Allow sessions to be saved/restored automatically to a memory area
    void setSession(BearSSL_Session *session)
    {
        this->tls_session = session;
    }

    // Don't validate the chain, just accept whatever is given.  VERY INSECURE!
    void setInsecure()
    {
        m_clearAuthenticationSettings();
        use_insecure = true;
    }

    // Assume a given public key, don't validate or use cert info at all
    void setKnownKey(const BearSSL_PublicKey *pk, unsigned usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN)
    {
        m_clearAuthenticationSettings();
        knownkey = pk;
        knownkey_usages = usages;
    }

    // Only check SHA1 fingerprint of certificate
    bool setFingerprint(const uint8_t fingerprint[20])
    {
        m_clearAuthenticationSettings();
        use_fingerprint = true;
        memcpy_P(this->fingerprint, fingerprint, 20);
        return true;
    }

    bool setFingerprint(const char *fpStr);

    // Accept any certificate that's self-signed
    void allowSelfSignedCerts()
    {
        m_clearAuthenticationSettings();
        use_self_signed = true;
    }

    // Install certificates of trusted CAs or specific site
    void setTrustAnchors(const BearSSL_X509List *ta)
    {
        m_clearAuthenticationSettings();
        this->ta = ta;
    }

    // In cases when NTP is not used, app must set a time manually to check cert validity
    void setX509Time(time_t now)
    {
        this->now = now;
    }

    // Install a client certificate for this connection, in case the server requires it (i.e. MQTT)
    void setClientRSACert(const BearSSL_X509List *cert, const BearSSL_PrivateKey *sk);
    void setClientECCert(const BearSSL_X509List *cert, const BearSSL_PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type);

    // Sets the requested buffer size for transmit and receive
    void setBufferSizes(int recv, int xmit);

    // Returns whether MFLN negotiation for the above buffer sizes succeeded (after connection)
    int getMFLNStatus()
    {
        return connected() && br_ssl_engine_get_mfln_negotiated(eng);
    }

    // Return an error code and possibly a text string in a passed-in buffer with last SSL failure
    int getLastSSLError(char *dest = NULL, size_t len = 0);

    // Attach a preconfigured certificate store
    void setCertStore(BearSSL_CertStoreBase *certStore)
    {
        this->certStore = certStore;
    }

    // Select specific ciphers (i.e. optimize for speed over security)
    // These may be in PROGMEM or RAM, either will run properly
    bool setCiphers(const uint16_t *cipherAry, int cipherCount);
    bool setCiphers(const std::vector<uint16_t> &list);
    bool setCiphersLessSecure(); // Only use the limited set of RSA ciphers without EC

    // Limit the TLS versions BearSSL will connect with.  Default is
    // BR_TLS10...BR_TLS12
    bool setSSLVersion(uint32_t min = BR_TLS10, uint32_t max = BR_TLS12);

    // peek buffer API is present
    bool hasPeekBufferAPI() const { return true; }

    // return number of byte accessible by peekBuffer()
    size_t peekAvailable() { return available(); }

    // return a pointer to available data buffer (size = peekAvailable())
    // semantic forbids any kind of read() before calling peekConsume()
    const char *peekBuffer();

    // consume bytes after use (see peekBuffer)
    void peekConsume(size_t consume);


protected:
    bool _connectSSL(const char *hostName) { return m_connectSSL(hostName); }

private:

    // Only one of the following two should ever be != nullptr!
    std::shared_ptr<br_ssl_client_context> ssl_client_ctx;
    std::shared_ptr<br_ssl_server_context> ssl_server_ctx;
    inline bool ctx_present()
    {
        return (ssl_client_ctx != nullptr) || (ssl_server_ctx != nullptr);
    }
    br_ssl_engine_context *eng; // &_sc->eng, to allow for client or server contexts
    std::shared_ptr<br_x509_minimal_context> x509_minimal;
    std::shared_ptr<struct mb_br_x509_insecure_context> x509_insecure;
    std::shared_ptr<br_x509_knownkey_context> x509_knownkey;
    std::shared_ptr<unsigned char> ssl_iobuf_in;
    std::shared_ptr<unsigned char> ssl_iobuf_out;
    time_t now;
    const BearSSL_X509List *ta;
    BearSSL_CertStoreBase *certStore;
    int ssl_iobuf_in_size;
    int ssl_iobuf_out_size;
    bool handshake_done;
    bool oom_err;

    // Optional storage space pointer for session parameters
    // Will be used on connect and updated on close
    BearSSL_Session *tls_session;

    bool use_insecure;
    bool use_fingerprint;
    uint8_t fingerprint[20];
    bool use_self_signed;
    const BearSSL_PublicKey *knownkey;
    unsigned knownkey_usages;

    // Custom cipher list pointer or NULL if default
    std::shared_ptr<uint16_t> cipher_list;
    uint8_t cipher_cnt;

    // TLS ciphers allowed
    uint32_t tls_version_min;
    uint32_t tls_version_max;

    unsigned char *recvapp_buf;
    size_t recvapp_len;

    // Optional client certificate
    const BearSSL_X509List *chain;
    const BearSSL_PrivateKey *sk;
    unsigned allowed_usages;
    unsigned cert_issuer_key_type;

    unsigned long timeout = 15000;
    unsigned long startMillis = 0;

    Client *base_client;
    bool secured = false;

    std::shared_ptr<unsigned char> m_alloc_iobuf(size_t sz);
    void m_clear();
    void m_clearAuthenticationSettings();
    bool m_clientConnected();
    void m_freeSSL();
    int m_run_until(unsigned int target, bool blocking = true);
    size_t m_write(const uint8_t *buf, size_t size, bool pmem);
    bool m_wait_for_handshake();

    // RSA keyed server
    bool m_connectSSLServerRSA(const BearSSL_X509List *chain, const BearSSL_PrivateKey *sk, BearSSL_ServerSessions *cache, const BearSSL_X509List *client_CA_ta);
    // EC keyed server
    bool m_connectSSLServerEC(const BearSSL_X509List *chain, unsigned cert_issuer_key_type, const BearSSL_PrivateKey *sk, BearSSL_ServerSessions *cache, const BearSSL_X509List *client_CA_ta);

    // X.509 validators differ from server to client
    bool m_installClientX509Validator();                                     // Set up X509 validator for a client conn.
    bool m_installServerX509Validator(const BearSSL_X509List *client_CA_ta); // Setup X509 client cert validation, if supplied
    void m_br_x509_insecure_init(mb_br_x509_insecure_context *ctx, int _use_fingerprint, const uint8_t _fingerprint[20], int _allow_self_signed);

    bool m_connectSSL(const char *hostName);

    char *m_streamLoad(Stream &stream, size_t size);
};


#endif // ESP8266

#endif // ESP8266_SSL_Client_H
