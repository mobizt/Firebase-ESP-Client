
/**
 *
 * The Network Upgradable BearSSL Client Class, ESP8266_SSL_Client.cpp v2.0.0
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

#ifndef ESP8266_SSL_Client_CPP
#define ESP8266_SSL_Client_CPP

#if defined(ESP8266)

#include "ESP8266_SSL_Client.h"

void ESP8266_SSL_Client::m_clear()
{

    ssl_client_ctx = nullptr;
    ssl_server_ctx = nullptr;

    eng = nullptr;

    x509_minimal = nullptr;
    x509_insecure = nullptr;
    x509_knownkey = nullptr;

    ssl_iobuf_in = nullptr;
    ssl_iobuf_out = nullptr;

    now = 0; // You can override or ensure time() is correct w/configTime
    ta = nullptr;
    setBufferSizes(16384, 512); // Minimum safe
    handshake_done = false;
    recvapp_buf = nullptr;
    recvapp_len = 0;
    oom_err = false;
    tls_session = nullptr;

    cipher_list = nullptr;

    cipher_cnt = 0;
    tls_version_min = BR_TLS10;
    tls_version_max = BR_TLS12;
}

ESP8266_SSL_Client::ESP8266_SSL_Client()
{
    m_clear();
    m_clearAuthenticationSettings();
    certStore = nullptr; // Don't want to remove cert store on a clear, should be long lived
    sk = nullptr;
    stack_thunk_add_ref();
}

ESP8266_SSL_Client::~ESP8266_SSL_Client()
{
    if (base_client)
    {
        if (base_client->connected())
            base_client->stop();
        base_client = nullptr;
    }
    cipher_list = nullptr;
    m_freeSSL();
    stack_thunk_del_ref();
}

void ESP8266_SSL_Client::setClientRSACert(const BearSSL_X509List *chain, const BearSSL_PrivateKey *sk)
{
    this->chain = chain;
    this->sk = sk;
}

void ESP8266_SSL_Client::setClientECCert(const BearSSL_X509List *chain, const BearSSL_PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type)
{
    this->chain = chain;
    this->sk = sk;
    this->allowed_usages = allowed_usages;
    this->cert_issuer_key_type = cert_issuer_key_type;
}

void ESP8266_SSL_Client::setBufferSizes(int recv, int xmit)
{
    // Following constants taken from bearssl/src/ssl/ssl_engine.c (not exported unfortunately)
    const int ESP8266_SSL_CLIENT_MAX_OUT_OVERHEAD = 85;
    const int ESP8266_SSL_CLIENT_MAX_IN_OVERHEAD = 325;

    // The data buffers must be between 512B and 16KB
    recv = std::max(512, std::min(16384, recv));
    xmit = std::max(512, std::min(16384, xmit));

    // Add in overhead for SSL protocol
    recv += ESP8266_SSL_CLIENT_MAX_IN_OVERHEAD;
    xmit += ESP8266_SSL_CLIENT_MAX_OUT_OVERHEAD;
    ssl_iobuf_in_size = recv;
    ssl_iobuf_out_size = xmit;
}

void ESP8266_SSL_Client::stop()
{

    if (base_client)
        base_client->stop();

    if (!secured)
        return;

    // Only if we've already connected, store session params and clear the connection options
    if (handshake_done)
    {
        if (tls_session)
        {
            br_ssl_engine_get_session_parameters(eng, tls_session->getSession());
        }
    }

    m_freeSSL();
}

void ESP8266_SSL_Client::flush()
{
    if (!base_client)
        return;

    if (secured)
        (void)m_run_until(BR_SSL_SENDAPP);

    base_client->flush();
}

int ESP8266_SSL_Client::connect(IPAddress ip, uint16_t port)
{
    if (!base_client)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF((const char *)FPSTR("Client is not yet initialized\n"));
        return false;
    }

    if (!base_client->connect(ip, port))
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("connect: base client connection failed\n");
        return false;
    }

    return true;
}

bool ESP8266_SSL_Client::connect(const char *name, uint16_t port)
{
    if (!base_client)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF((const char *)FPSTR("Client is not yet initialized\n"));
        return false;
    }

    if (!base_client->connect(name, port))
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("connect: base client connection failed\n");
        return false;
    }

    return true;
}

int ESP8266_SSL_Client::connect(const String &host, uint16_t port)
{
    return connect(host.c_str(), port);
}

void ESP8266_SSL_Client::m_freeSSL()
{
    // These are smart pointers and will free if refcnt==0
    ssl_client_ctx = nullptr;
    ssl_server_ctx = nullptr;

    x509_minimal = nullptr;
    x509_insecure = nullptr;
    x509_knownkey = nullptr;

    ssl_iobuf_in = nullptr;
    ssl_iobuf_out = nullptr;

    // Reset non-allocated ptrs (pointing to bits potentially free'd above)
    recvapp_buf = nullptr;
    recvapp_len = 0;

    // This connection is toast
    handshake_done = false;
    timeout = 15000;
    secured = false;
}

bool ESP8266_SSL_Client::m_clientConnected()
{
    return (base_client && base_client->connected());
}

bool ESP8266_SSL_Client::connected()
{
    if (!base_client)
        return false;

    if (!secured)
        return base_client->connected();

    if (available() || (m_clientConnected() && handshake_done && (br_ssl_engine_current_state(eng) != BR_SSL_CLOSED)))
    {
        return true;
    }
    return false;
}

int ESP8266_SSL_Client::availableForWrite()
{
    if (!base_client || !secured)
        return 0;

    // code taken from ::_write()
    if (!connected() || !handshake_done)
    {
        return 0;
    }
    // Get BearSSL to a state where we can send
    if (m_run_until(BR_SSL_SENDAPP) < 0)
    {
        return 0;
    }
    if (br_ssl_engine_current_state(eng) & BR_SSL_SENDAPP)
    {
        size_t sendapp_len;
        (void)br_ssl_engine_sendapp_buf(eng, &sendapp_len);
        // We want to call br_ssl_engine_sendapp_ack(0) but 0 is forbidden (bssl doc).
        // After checking br_ssl_engine_sendapp_buf() src code,
        // it seems that it is OK to not call ack when the buffer is left untouched.
        // forbidden: br_ssl_engine_sendapp_ack(_eng, 0);
        return (int)sendapp_len;
    }
    return 0;
}

size_t ESP8266_SSL_Client::m_write(const uint8_t *buf, size_t size, bool pmem)
{

    if (!base_client)
        return 0;

    if (!secured)
    {
        if (pmem)
        {
            char dest[size];
            memcpy_P((void *)dest, (PGM_VOID_P)buf, size);
            return base_client->write((uint8_t *)dest, size);
        }

        return base_client->write(buf, size);
    }

    size_t sent_bytes = 0;

    if (!connected() || !size || !handshake_done)
    {
        return 0;
    }

    do
    {
        // Ensure we yield if we need multiple fragments to avoid WDT
        if (sent_bytes)
        {
            optimistic_yield(1000);
        }

        // Get BearSSL to a state where we can send
        if (m_run_until(BR_SSL_SENDAPP) < 0)
        {
            break;
        }

        if (br_ssl_engine_current_state(eng) & BR_SSL_SENDAPP)
        {
            size_t sendapp_len;
            unsigned char *sendapp_buf = br_ssl_engine_sendapp_buf(eng, &sendapp_len);
            int to_send = size > sendapp_len ? sendapp_len : size;

            if (pmem)
            {
                memcpy_P(sendapp_buf, buf, to_send);
            }
            else
            {
                memcpy(sendapp_buf, buf, to_send);
            }

            br_ssl_engine_sendapp_ack(eng, to_send);
            br_ssl_engine_flush(eng, 0);
            flush();
            buf += to_send;
            sent_bytes += to_send;
            size -= to_send;
        }
        else
        {
            break;
        }
    } while (size);

    return sent_bytes;
}

size_t ESP8266_SSL_Client::write(const uint8_t *buf, size_t size)
{
    return m_write(buf, size, false);
}

size_t ESP8266_SSL_Client::write_P(PGM_P buf, size_t size)
{
    return m_write((const uint8_t *)buf, size, true);
}

size_t ESP8266_SSL_Client::write(Stream &stream)
{
    if (!connected() || !handshake_done)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("write: Connect/handshake not completed yet\n");
        return 0;
    }

    size_t dl = stream.available();
    uint8_t buf[dl];
    stream.readBytes(buf, dl);
    return m_write(buf, dl, false);
}

int ESP8266_SSL_Client::read(uint8_t *buf, size_t size)
{
    if (!base_client)
        return 0;

    if (!secured)
        return base_client->read(buf, size);

    if (!handshake_done)
    {
        return -1;
    }

    int avail = available();
    bool conn = connected();
    if (!avail && conn)
    {
        return 0; // We're still connected, but nothing to read
    }
    if (!avail && !conn)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("read: Not connected, none left available\n");
        return -1;
    }

    if (avail)
    {
        // Take data from the recvapp buffer
        int to_copy = recvapp_len < size ? recvapp_len : size;
        memcpy(buf, recvapp_buf, to_copy);
        br_ssl_engine_recvapp_ack(eng, to_copy);
        recvapp_buf = nullptr;
        recvapp_len = 0;
        return to_copy;
    }

    if (!conn)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("read: Not connected\n");
        return -1;
    }
    return 0; // If we're connected, no error but no read.
}

// return a pointer to available data buffer (size = peekAvailable())
// semantic forbids any kind of read() before calling peekConsume()
const char *ESP8266_SSL_Client::peekBuffer()
{
    return (const char *)recvapp_buf;
}

// consume bytes after use (see peekBuffer)
void ESP8266_SSL_Client::peekConsume(size_t consume)
{
    br_ssl_engine_recvapp_ack(eng, consume);
    recvapp_buf = nullptr;
    recvapp_len = 0;
}

int ESP8266_SSL_Client::read()
{
    if (!base_client)
        return -1;

    if (!secured)
        return base_client->read();

    uint8_t c;
    if (1 == read(&c, 1))
    {
        return c;
    }
    MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("read: failed\n");
    return -1;
}

int ESP8266_SSL_Client::available()
{
    if (!base_client)
        return 0;

    if (!secured)
        return base_client->available();

    if (recvapp_buf)
    {
        return recvapp_len; // Anything from last call?
    }
    recvapp_buf = nullptr;
    recvapp_len = 0;
    if (m_run_until(BR_SSL_RECVAPP, false) < 0)
    {
        return 0;
    }

    int st = br_ssl_engine_current_state(eng);
    if (st == BR_SSL_CLOSED)
    {
        return 0; // Nothing leftover, SSL is closed
    }

    if (st & BR_SSL_RECVAPP)
    {
        recvapp_buf = br_ssl_engine_recvapp_buf(eng, &recvapp_len);
        return recvapp_len;
    }

    return 0;
}

int ESP8266_SSL_Client::peek()
{
    if (!available())
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("peek: Not connected, none left available\n");
        return -1;
    }

    if (!secured)
        return base_client->peek();

    if (recvapp_buf && recvapp_len)
    {
        return recvapp_buf[0];
    }
    MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("peek: No data left\n");
    return -1;
}

size_t ESP8266_SSL_Client::peekBytes(uint8_t *buffer, size_t length)
{
    if (!base_client || !secured)
        return 0;

    size_t to_copy = 0;

    startMillis = millis();
    while ((available() < (int)length) && ((millis() - startMillis) < 5000))
    {
        yield();
    }

    to_copy = recvapp_len < length ? recvapp_len : length;
    memcpy(buffer, recvapp_buf, to_copy);
    return to_copy;
}

/* --- Copied almost verbatim from BEARSSL SSL_IO.C ---
   Run the engine, until the specified target state is achieved, or
   an error occurs. The target state is SENDAPP, RECVAPP, or the
   combination of both (the combination matches either). When a match is
   achieved, this function returns 0. On error, it returns -1.
*/

int ESP8266_SSL_Client::m_run_until(unsigned int target, bool blocking)
{

    esp8266::polledTimeout::oneShotMs loopTimeout(timeout);

    for (int no_work = 0; blocking || no_work < 2;)
    {
        optimistic_yield(100);

        if (loopTimeout)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_run_until: Timeout\n");
            return -1;
        }

        int state;
        state = br_ssl_engine_current_state(eng);
        if (state & BR_SSL_CLOSED)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_run_until: BSSL closed\n");
            return -1;
        }

        if (!(base_client->connected()) && !base_client->available())
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_run_until: not connected\n");
            return (state & target) ? 0 : -1;
        }

        /*
           If there is some record data to send, do it. This takes
           precedence over everything else.
        */
        if (state & BR_SSL_SENDREC)
        {
            unsigned char *buf;
            size_t len;
            int wlen;

            buf = br_ssl_engine_sendrec_buf(eng, &len);

#if defined(ESP8266_CORE_SDK_V3_X_X)
            size_t availForWrite = base_client->availableForWrite();

            if (!blocking && len > availForWrite)
            {
                /*
                   writes on WiFiClient will block if len > availableForWrite()
                   this is needed to prevent available() calls from blocking
                   on dropped connections
                */
                len = availForWrite;
            }
#endif
            wlen = base_client->write(buf, len);

            if (wlen <= 0)
            {
                /*
                   If we received a close_notify and we
                   still send something, then we have our
                   own response close_notify to send, and
                   the peer is allowed by RFC 5246 not to
                   wait for it.
                */
                return -1;
            }

            if (wlen > 0)
            {
                br_ssl_engine_sendrec_ack(eng, wlen);
            }

            no_work = 0;
            continue;
        }

        /*
           If we reached our target, then we are finished.
        */
        if (state & target)
        {
            return 0;
        }

        /*
           If some application data must be read, and we did not
           exit, then this means that we are trying to write data,
           and that's not possible until the application data is
           read. This may happen if using a shared in/out buffer,
           and the underlying protocol is not strictly half-duplex.
           This is unrecoverable here, so we report an error.
        */
        if (state & BR_SSL_RECVAPP)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_run_until: Fatal protocol state\n");
            return -1;
        }

        /*
           If we reached that point, then either we are trying
           to read data and there is some, or the engine is stuck
           until a new record is obtained.
        */
        if (state & BR_SSL_RECVREC)
        {
            if (base_client->available())
            {
                unsigned char *buf;
                size_t len;
                int rlen;

                buf = br_ssl_engine_recvrec_buf(eng, &len);

                int read = 0;
                int toRead = len;
                while (toRead > 0 && base_client->available())
                {
                    rlen = base_client->read(buf + read, toRead);
                    read += rlen;
                    toRead = len - read;
                    delay(0);
                    if (loopTimeout)
                    {
                        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_run_until: Timeout\n");
                        return -1;
                    }
                }

                rlen = read;

                if (rlen < 0)
                {
                    return -1;
                }

                if (rlen > 0)
                {
                    br_ssl_engine_recvrec_ack(eng, rlen);
                }

                no_work = 0;
                continue;
            }
        }

        /*
           We can reach that point if the target RECVAPP, and
           the state contains SENDAPP only. This may happen with
           a shared in/out buffer. In that case, we must flush
           the buffered data to "make room" for a new incoming
           record.
        */
        br_ssl_engine_flush(eng, 0);

        no_work++; // We didn't actually advance here
    }
    // We only get here if we ran through the loop without getting anything done
    return -1;
}

bool ESP8266_SSL_Client::m_wait_for_handshake()
{
    handshake_done = false;

    // Change waiting time out to ensure the handshake done
    unsigned long to = timeout;
    timeout = 15000;
    while (!handshake_done && m_clientConnected())
    {
        int ret = m_run_until(BR_SSL_SENDAPP);
        if (ret < 0)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_wait_for_handshake: failed\n");
            break;
        }
        if (br_ssl_engine_current_state(eng) & BR_SSL_SENDAPP)
        {
            handshake_done = true;
        }
        optimistic_yield(1000);
    }

    // Restore waiting time out
    timeout = to;
    return handshake_done;
}

// Set a fingerprint by parsing an ASCII string
bool ESP8266_SSL_Client::setFingerprint(const char *fpStr)
{
    int idx = 0;
    uint8_t c, d;
    uint8_t fp[20];

    while (idx < 20)
    {
        c = pgm_read_byte(fpStr++);
        if (!c)
            break; // String ended, done processing
        d = pgm_read_byte(fpStr++);
        if (!d)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("setFingerprint: FP too short\n");
            return false; // Only half of the last hex digit, error
        }
        c = mb_htoi(c);
        d = mb_htoi(d);
        if ((c > 15) || (d > 15))
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("setFingerprint: Invalid char\n");
            return false; // Error in one of the hex characters
        }
        fp[idx++] = (c << 4) | d;

        // Skip 0 or more spaces or colons
        while (pgm_read_byte(fpStr) && (pgm_read_byte(fpStr) == ' ' || pgm_read_byte(fpStr) == ':'))
        {
            fpStr++;
        }
    }
    if ((idx != 20) || pgm_read_byte(fpStr))
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("setFingerprint: Garbage at end of fp\n");
        return false; // Garbage at EOL or we didn't have enough hex digits
    }
    return setFingerprint(fp);
}

// Set custom list of ciphers
bool ESP8266_SSL_Client::setCiphers(const uint16_t *cipherAry, int cipherCount)
{
    cipher_list = nullptr;
    cipher_list = std::shared_ptr<uint16_t>(new (std::nothrow) uint16_t[cipherCount], std::default_delete<uint16_t[]>());
    if (!cipher_list.get())
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("setCiphers: list empty\n");
        return false;
    }
    memcpy_P(cipher_list.get(), cipherAry, cipherCount * sizeof(uint16_t));
    cipher_cnt = cipherCount;
    return true;
}

bool ESP8266_SSL_Client::setCiphersLessSecure()
{
    return setCiphers(mb_bearssl_faster_suites_P, sizeof(mb_bearssl_faster_suites_P) / sizeof(mb_bearssl_faster_suites_P[0]));
}

bool ESP8266_SSL_Client::setCiphers(const std::vector<uint16_t> &list)
{
    return setCiphers(&list[0], list.size());
}

bool ESP8266_SSL_Client::setSSLVersion(uint32_t min, uint32_t max)
{
    if (((min != BR_TLS10) && (min != BR_TLS11) && (min != BR_TLS12)) ||
        ((max != BR_TLS10) && (max != BR_TLS11) && (max != BR_TLS12)) ||
        (max < min))
    {
        return false; // Invalid options
    }
    tls_version_min = min;
    tls_version_max = max;
    return true;
}

// Installs the appropriate X509 cert validation method for a client connection
bool ESP8266_SSL_Client::m_installClientX509Validator()
{

    if (use_insecure || use_fingerprint || use_self_signed)
    {
        // Use common insecure x509 authenticator
        x509_insecure = std::make_shared<struct mb_br_x509_insecure_context>();
        if (!x509_insecure)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("installClientX509Validator: OOM for x509_insecure\n");
            return false;
        }
        m_br_x509_insecure_init(x509_insecure.get(), use_fingerprint, fingerprint, use_self_signed);
        br_ssl_engine_set_x509(eng, &x509_insecure->vtable);
    }
    else if (knownkey)
    {
        // Simple, pre-known public key authenticator, ignores cert completely.
        x509_knownkey = std::make_shared<br_x509_knownkey_context>();
        if (!x509_knownkey)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("installClientX509Validator: OOM for x509_knownkey\n");
            return false;
        }
        if (knownkey->isRSA())
        {
            br_x509_knownkey_init_rsa(x509_knownkey.get(), knownkey->getRSA(), knownkey_usages);
        }
        else if (knownkey->isEC())
        {
#ifndef BEARSSL_SSL_BASIC
            br_x509_knownkey_init_ec(x509_knownkey.get(), knownkey->getEC(), knownkey_usages);
#else
            (void)knownkey;
            (void)knownkey_usages;
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("installClientX509Validator: Attempting to use EC keys in minimal cipher mode (no EC)\n");
            return false;
#endif
        }
        br_ssl_engine_set_x509(eng, &x509_knownkey->vtable);
    }
    else
    {
        // X509 minimal validator.  Checks dates, cert chain for trusted CA, etc.
        x509_minimal = std::make_shared<br_x509_minimal_context>();
        if (!x509_minimal)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("installClientX509Validator: OOM for x509_minimal\n");
            return false;
        }
        br_x509_minimal_init(x509_minimal.get(), &br_sha256_vtable, ta ? ta->getTrustAnchors() : nullptr, ta ? ta->getCount() : 0);
        br_x509_minimal_set_rsa(x509_minimal.get(), br_ssl_engine_get_rsavrfy(eng));
#ifndef BEARSSL_SSL_BASIC
        br_x509_minimal_set_ecdsa(x509_minimal.get(), br_ssl_engine_get_ec(eng), br_ssl_engine_get_ecdsa(eng));
#endif
        mb_bearssl_br_x509_minimal_install_hashes(x509_minimal.get());
        if (now)
        {
            // Magic constants convert to x509 times
            br_x509_minimal_set_time(x509_minimal.get(), ((uint32_t)now) / 86400 + 719528, ((uint32_t)now) % 86400);
        }
        if (certStore)
        {
            certStore->installCertStore(x509_minimal.get());
        }
        br_ssl_engine_set_x509(eng, &x509_minimal->vtable);
    }
    return true;
}

std::shared_ptr<unsigned char> ESP8266_SSL_Client::m_alloc_iobuf(size_t sz)
{
    // Allocate buffer with preference to IRAM
#if defined(ESP8266_CORE_SDK_V3_X_X)
    HeapSelectIram primary;
    auto sptr = std::shared_ptr<unsigned char>(new (std::nothrow) unsigned char[sz], std::default_delete<unsigned char[]>());
    if (!sptr)
    {
        HeapSelectDram alternate;
        sptr = std::shared_ptr<unsigned char>(new (std::nothrow) unsigned char[sz], std::default_delete<unsigned char[]>());
    }
    return sptr;
#else
    return std::shared_ptr<unsigned char>(new unsigned char[sz], std::default_delete<unsigned char[]>());
#endif
}

// Called by connect() to do the actual SSL setup and handshake.
// Returns if the SSL handshake succeeded.
bool ESP8266_SSL_Client::m_connectSSL(const char *hostName)
{
    if (!base_client)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF((const char *)FPSTR("Client is not yet initialized\n"));
        return false;
    }

    if (!m_clientConnected())
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF((const char *)FPSTR("Client is not connected\n"));
        return false;
    }

    MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSL: start connection\n");

#ifdef MB_ESP8266_SSLCLIENT_ENABLE_DEBUG
    // BearSSL will reject all connections unless an authentication option is set, warn in DEBUG builds
    if (!this->use_insecure && !this->use_fingerprint && !this->use_self_signed && !this->knownkey && !this->certStore && !this->ta)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("Connection *will* fail, no authentication method is setup\n");
    }
#endif

    ssl_client_ctx = std::make_shared<br_ssl_client_context>();
    eng = &ssl_client_ctx->eng; // Allocation/deallocation taken care of by the _sc shared_ptr
    ssl_iobuf_in = m_alloc_iobuf(ssl_iobuf_in_size);
    ssl_iobuf_out = m_alloc_iobuf(ssl_iobuf_out_size);

    if (!ssl_client_ctx || !ssl_iobuf_in || !ssl_iobuf_out)
    {
        this->m_freeSSL();
        oom_err = true;
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSL: OOM error\n");
        return false;
    }
    // If no cipher list yet set, use defaults
    if (!cipher_list)
    {
        mb_br_ssl_client_base_init(ssl_client_ctx.get(), mb_suites_P, sizeof(mb_suites_P) / sizeof(mb_suites_P[0]));
    }
    else
    {
        mb_bearssl_br_ssl_client_base_init(ssl_client_ctx.get(), this->cipher_list.get(), this->cipher_cnt);
    }

    // Only failure possible in the installation is OOM
    if (!m_installClientX509Validator())
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSL: Can't install x509 validator\n");
        return false;
    }

    br_ssl_engine_set_buffers_bidi(eng, ssl_iobuf_in.get(), ssl_iobuf_in_size, ssl_iobuf_out.get(), ssl_iobuf_out_size);
    br_ssl_engine_set_versions(eng, tls_version_min, tls_version_max);

    // Apply any client certificates, if supplied.
    if (this->sk && this->sk->isRSA())
    {
        br_ssl_client_set_single_rsa(ssl_client_ctx.get(), this->chain ? this->chain->getX509Certs() : nullptr, this->chain ? this->chain->getCount() : 0, sk->getRSA(), br_rsa_pkcs1_sign_get_default());
    }
    else if (this->sk && this->sk->isEC())
    {
#ifndef BEARSSL_SSL_BASIC
        br_ssl_client_set_single_ec(ssl_client_ctx.get(), this->chain ? this->chain->getX509Certs() : nullptr, this->chain ? this->chain->getCount() : 0, this->sk->getEC(), allowed_usages, cert_issuer_key_type, br_ec_get_default(), br_ecdsa_sign_asn1_get_default());
#else
        m_freeSSL();
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSL: Attempting to use EC cert in minimal cipher mode (no EC)\n");
        return false;
#endif
    }

    // Restore session from the storage spot, if present
    if (tls_session)
    {
        br_ssl_engine_set_session_parameters(eng, tls_session->getSession());
    }

    if (!br_ssl_client_reset(ssl_client_ctx.get(), hostName, tls_session ? 1 : 0))
    {
        m_freeSSL();
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSL: Can't reset client\n");
        return false;
    }

    auto ret = m_wait_for_handshake();

    // Session is already validated here, there is no need to keep following
    x509_minimal = nullptr;
    x509_insecure = nullptr;
    x509_knownkey = nullptr;

    secured = ret;

    return ret;
}

// Slightly different X509 setup for servers who want to validate client
// certificates, so factor it out as it's used in RSA and EC servers.
bool ESP8266_SSL_Client::m_installServerX509Validator(const BearSSL_X509List *client_CA_ta)
{
    if (client_CA_ta)
    {
        ta = client_CA_ta;
        // X509 minimal validator.  Checks dates, cert chain for trusted CA, etc.
        x509_minimal = std::make_shared<br_x509_minimal_context>();
        if (!x509_minimal)
        {
            m_freeSSL();
            oom_err = true;
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("installServerX509Validator: OOM for x509_minimal\n");
            return false;
        }
        br_x509_minimal_init(x509_minimal.get(), &br_sha256_vtable, ta->getTrustAnchors(), ta->getCount());
        br_ssl_engine_set_default_rsavrfy(eng);
#ifndef BEARSSL_SSL_BASIC
        br_ssl_engine_set_default_ecdsa(eng);
#endif
        br_x509_minimal_set_rsa(x509_minimal.get(), br_ssl_engine_get_rsavrfy(eng));
#ifndef BEARSSL_SSL_BASIC
        br_x509_minimal_set_ecdsa(x509_minimal.get(), br_ssl_engine_get_ec(eng), br_ssl_engine_get_ecdsa(eng));
#endif
        mb_bearssl_br_x509_minimal_install_hashes(x509_minimal.get());
        if (now)
        {
            // Magic constants convert to x509 times
            br_x509_minimal_set_time(x509_minimal.get(), ((uint32_t)now) / 86400 + 719528, ((uint32_t)now) % 86400);
        }
        br_ssl_engine_set_x509(eng, &x509_minimal->vtable);
        br_ssl_server_set_trust_anchor_names_alt(ssl_server_ctx.get(), ta->getTrustAnchors(), ta->getCount());
    }
    return true;
}

// Called by WiFiServerBearSSL when an RSA cert/key is specified.
bool ESP8266_SSL_Client::m_connectSSLServerRSA(const BearSSL_X509List *chain, const BearSSL_PrivateKey *sk, BearSSL_ServerSessions *cache, const BearSSL_X509List *client_CA_ta)
{
    m_freeSSL();
    oom_err = false;

    ssl_server_ctx = std::make_shared<br_ssl_server_context>();
    eng = &ssl_server_ctx->eng; // Allocation/deallocation taken care of by the _sc shared_ptr
    ssl_iobuf_in = m_alloc_iobuf(ssl_iobuf_in_size);
    ssl_iobuf_out = m_alloc_iobuf(ssl_iobuf_out_size);

    if (!ssl_server_ctx || !ssl_iobuf_in || !ssl_iobuf_out)
    {
        m_freeSSL();
        oom_err = true;
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerRSA: OOM error\n");
        return false;
    }

    mb_bearssl_br_ssl_server_base_init(ssl_server_ctx.get(), mb_bearssl_suites_server_rsa_P, sizeof(mb_bearssl_suites_server_rsa_P) / sizeof(mb_bearssl_suites_server_rsa_P[0]));
    br_ssl_server_set_single_rsa(ssl_server_ctx.get(), chain ? chain->getX509Certs() : nullptr, chain ? chain->getCount() : 0,
                                 sk ? sk->getRSA() : nullptr, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN,
                                 br_rsa_private_get_default(), br_rsa_pkcs1_sign_get_default());
    br_ssl_engine_set_buffers_bidi(eng, ssl_iobuf_in.get(), ssl_iobuf_in_size, ssl_iobuf_out.get(), ssl_iobuf_out_size);
    br_ssl_engine_set_versions(eng, tls_version_min, tls_version_max);
    if (cache != nullptr)
        br_ssl_server_set_cache(ssl_server_ctx.get(), cache->getCache());
    if (client_CA_ta && !m_installServerX509Validator(client_CA_ta))
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerRSA: Can't install serverX509check\n");
        return false;
    }
    if (!br_ssl_server_reset(ssl_server_ctx.get()))
    {
        m_freeSSL();
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerRSA: Can't reset server ctx\n");
        return false;
    }

    return m_wait_for_handshake();
}

// Called by WiFiServerBearSSL when an elliptic curve cert/key is specified.
bool ESP8266_SSL_Client::m_connectSSLServerEC(const BearSSL_X509List *chain, unsigned cert_issuer_key_type, const BearSSL_PrivateKey *sk, BearSSL_ServerSessions *cache, const BearSSL_X509List *client_CA_ta)
{
#ifndef BEARSSL_SSL_BASIC
    m_freeSSL();
    oom_err = false;

    ssl_server_ctx = std::make_shared<br_ssl_server_context>();
    eng = &ssl_server_ctx->eng; // Allocation/deallocation taken care of by the _sc shared_ptr
    ssl_iobuf_in = m_alloc_iobuf(ssl_iobuf_in_size);
    ssl_iobuf_out = m_alloc_iobuf(ssl_iobuf_out_size);

    if (!ssl_server_ctx || !ssl_iobuf_in || !ssl_iobuf_out)
    {
        m_freeSSL();
        oom_err = true;
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerEC: OOM error\n");
        return false;
    }

    mb_bearssl_br_ssl_server_base_init(ssl_server_ctx.get(), mb_bearssl_suites_server_ec_P, sizeof(mb_bearssl_suites_server_ec_P) / sizeof(mb_bearssl_suites_server_ec_P[0]));
    br_ssl_server_set_single_ec(ssl_server_ctx.get(), chain ? chain->getX509Certs() : nullptr, chain ? chain->getCount() : 0, sk ? sk->getEC() : nullptr, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, cert_issuer_key_type, br_ssl_engine_get_ec(eng), br_ecdsa_i15_sign_asn1);
    br_ssl_engine_set_buffers_bidi(eng, ssl_iobuf_in.get(), ssl_iobuf_in_size, ssl_iobuf_out.get(), ssl_iobuf_out_size);
    br_ssl_engine_set_versions(eng, tls_version_min, tls_version_max);
    if (cache != nullptr)
        br_ssl_server_set_cache(ssl_server_ctx.get(), cache->getCache());
    if (client_CA_ta && !m_installServerX509Validator(client_CA_ta))
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerEC: Can't install serverX509check\n");
        return false;
    }
    if (!br_ssl_server_reset(ssl_server_ctx.get()))
    {
        m_freeSSL();
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerEC: Can't reset server ctx\n");
        return false;
    }

    return m_wait_for_handshake();
#else
    (void)chain;
    (void)cert_issuer_key_type;
    (void)sk;
    (void)cache;
    (void)client_CA_ta;
    MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("m_connectSSLServerEC: Attempting to use EC cert in minimal cipher mode (no EC)\n");
    return false;
#endif
}

//  Set up the x509 insecure data structures for BearSSL core to use.
void ESP8266_SSL_Client::m_br_x509_insecure_init(mb_br_x509_insecure_context *ctx, int _use_fingerprint, const uint8_t _fingerprint[20], int _allow_self_signed)
{
    static const br_x509_class br_x509_insecure_vtable PROGMEM = {
        sizeof(mb_br_x509_insecure_context),
        mb_insecure_start_chain,
        mb_insecure_start_cert,
        mb_insecure_append,
        mb_insecure_end_cert,
        mb_insecure_end_chain,
        mb_insecure_get_pkey};

    memset(ctx, 0, sizeof *ctx);
    ctx->vtable = &br_x509_insecure_vtable;
    ctx->done_cert = false;
    ctx->match_fingerprint = nullptr;
    ctx->allow_self_signed = 0;
}

void ESP8266_SSL_Client::setTimeout(unsigned long timeout)
{
    this->timeout = timeout;
    if (base_client)
        base_client->setTimeout(timeout);
}

void ESP8266_SSL_Client::m_clearAuthenticationSettings()
{
    use_insecure = false;
    use_fingerprint = false;
    use_self_signed = false;

    knownkey = nullptr;
    ta = nullptr;
}

void ESP8266_SSL_Client::setClient(Client *client)
{
    base_client = client;
}

// Returns an error ID and possibly a string (if dest != null) of the last
// BearSSL reported error.
int ESP8266_SSL_Client::getLastSSLError(char *dest, size_t len)
{
    int err = 0;
    const char *t = PSTR("OK");
    const char *recv_fatal = "";
    const char *send_fatal = "";
    if (ssl_server_ctx)
    {
        err = br_ssl_engine_last_error(eng);
    }
    if (oom_err)
    {
        err = -1000;
    }
    else
    {
        if (err & BR_ERR_RECV_FATAL_ALERT)
        {
            recv_fatal = PSTR("SSL received fatal alert - ");
            err &= ~BR_ERR_RECV_FATAL_ALERT;
        }
        if (err & BR_ERR_SEND_FATAL_ALERT)
        {
            send_fatal = PSTR("SSL sent fatal alert - ");
            err &= ~BR_ERR_SEND_FATAL_ALERT;
        }
    }
    switch (err)
    {
    case -1000:
        t = PSTR("Unable to allocate memory for SSL structures and buffers.");
        break;
    case BR_ERR_BAD_PARAM:
        t = PSTR("Caller-provided parameter is incorrect.");
        break;
    case BR_ERR_BAD_STATE:
        t = PSTR("Operation requested by the caller cannot be applied with the current context state (e.g. reading data while outgoing data is waiting to be sent).");
        break;
    case BR_ERR_UNSUPPORTED_VERSION:
        t = PSTR("Incoming protocol or record version is unsupported.");
        break;
    case BR_ERR_BAD_VERSION:
        t = PSTR("Incoming record version does not match the expected version.");
        break;
    case BR_ERR_BAD_LENGTH:
        t = PSTR("Incoming record length is invalid.");
        break;
    case BR_ERR_TOO_LARGE:
        t = PSTR("Incoming record is too large to be processed, or buffer is too small for the handshake message to send.");
        break;
    case BR_ERR_BAD_MAC:
        t = PSTR("Decryption found an invalid padding, or the record MAC is not correct.");
        break;
    case BR_ERR_NO_RANDOM:
        t = PSTR("No initial entropy was provided, and none can be obtained from the OS.");
        break;
    case BR_ERR_UNKNOWN_TYPE:
        t = PSTR("Incoming record type is unknown.");
        break;
    case BR_ERR_UNEXPECTED:
        t = PSTR("Incoming record or message has wrong type with regards to the current engine state.");
        break;
    case BR_ERR_BAD_CCS:
        t = PSTR("ChangeCipherSpec message from the peer has invalid contents.");
        break;
    case BR_ERR_BAD_ALERT:
        t = PSTR("Alert message from the peer has invalid contents (odd length).");
        break;
    case BR_ERR_BAD_HANDSHAKE:
        t = PSTR("Incoming handshake message decoding failed.");
        break;
    case BR_ERR_OVERSIZED_ID:
        t = PSTR("ServerHello contains a session ID which is larger than 32 bytes.");
        break;
    case BR_ERR_BAD_CIPHER_SUITE:
        t = PSTR("Server wants to use a cipher suite that we did not claim to support. This is also reported if we tried to advertise a cipher suite that we do not support.");
        break;
    case BR_ERR_BAD_COMPRESSION:
        t = PSTR("Server wants to use a compression that we did not claim to support.");
        break;
    case BR_ERR_BAD_FRAGLEN:
        t = PSTR("Server's max fragment length does not match client's.");
        break;
    case BR_ERR_BAD_SECRENEG:
        t = PSTR("Secure renegotiation failed.");
        break;
    case BR_ERR_EXTRA_EXTENSION:
        t = PSTR("Server sent an extension type that we did not announce, or used the same extension type several times in a single ServerHello.");
        break;
    case BR_ERR_BAD_SNI:
        t = PSTR("Invalid Server Name Indication contents (when used by the server, this extension shall be empty).");
        break;
    case BR_ERR_BAD_HELLO_DONE:
        t = PSTR("Invalid ServerHelloDone from the server (length is not 0).");
        break;
    case BR_ERR_LIMIT_EXCEEDED:
        t = PSTR("Internal limit exceeded (e.g. server's public key is too large).");
        break;
    case BR_ERR_BAD_FINISHED:
        t = PSTR("Finished message from peer does not match the expected value.");
        break;
    case BR_ERR_RESUME_MISMATCH:
        t = PSTR("Session resumption attempt with distinct version or cipher suite.");
        break;
    case BR_ERR_INVALID_ALGORITHM:
        t = PSTR("Unsupported or invalid algorithm (ECDHE curve, signature algorithm, hash function).");
        break;
    case BR_ERR_BAD_SIGNATURE:
        t = PSTR("Invalid signature in ServerKeyExchange or CertificateVerify message.");
        break;
    case BR_ERR_WRONG_KEY_USAGE:
        t = PSTR("Peer's public key does not have the proper type or is not allowed for the requested operation.");
        break;
    case BR_ERR_NO_CLIENT_AUTH:
        t = PSTR("Client did not send a certificate upon request, or the client certificate could not be validated.");
        break;
    case BR_ERR_IO:
        t = PSTR("I/O error or premature close on transport stream.");
        break;
    case BR_ERR_X509_INVALID_VALUE:
        t = PSTR("Invalid value in an ASN.1 structure.");
        break;
    case BR_ERR_X509_TRUNCATED:
        t = PSTR("Truncated certificate or other ASN.1 object.");
        break;
    case BR_ERR_X509_EMPTY_CHAIN:
        t = PSTR("Empty certificate chain (no certificate at all).");
        break;
    case BR_ERR_X509_INNER_TRUNC:
        t = PSTR("Decoding error: inner element extends beyond outer element size.");
        break;
    case BR_ERR_X509_BAD_TAG_CLASS:
        t = PSTR("Decoding error: unsupported tag class (application or private).");
        break;
    case BR_ERR_X509_BAD_TAG_VALUE:
        t = PSTR("Decoding error: unsupported tag value.");
        break;
    case BR_ERR_X509_INDEFINITE_LENGTH:
        t = PSTR("Decoding error: indefinite length.");
        break;
    case BR_ERR_X509_EXTRA_ELEMENT:
        t = PSTR("Decoding error: extraneous element.");
        break;
    case BR_ERR_X509_UNEXPECTED:
        t = PSTR("Decoding error: unexpected element.");
        break;
    case BR_ERR_X509_NOT_CONSTRUCTED:
        t = PSTR("Decoding error: expected constructed element, but is primitive.");
        break;
    case BR_ERR_X509_NOT_PRIMITIVE:
        t = PSTR("Decoding error: expected primitive element, but is constructed.");
        break;
    case BR_ERR_X509_PARTIAL_BYTE:
        t = PSTR("Decoding error: BIT STRING length is not multiple of 8.");
        break;
    case BR_ERR_X509_BAD_BOOLEAN:
        t = PSTR("Decoding error: BOOLEAN value has invalid length.");
        break;
    case BR_ERR_X509_OVERFLOW:
        t = PSTR("Decoding error: value is off-limits.");
        break;
    case BR_ERR_X509_BAD_DN:
        t = PSTR("Invalid distinguished name.");
        break;
    case BR_ERR_X509_BAD_TIME:
        t = PSTR("Invalid date/time representation.");
        break;
    case BR_ERR_X509_UNSUPPORTED:
        t = PSTR("Certificate contains unsupported features that cannot be ignored.");
        break;
    case BR_ERR_X509_LIMIT_EXCEEDED:
        t = PSTR("Key or signature size exceeds internal limits.");
        break;
    case BR_ERR_X509_WRONG_KEY_TYPE:
        t = PSTR("Key type does not match that which was expected.");
        break;
    case BR_ERR_X509_BAD_SIGNATURE:
        t = PSTR("Signature is invalid.");
        break;
    case BR_ERR_X509_TIME_UNKNOWN:
        t = PSTR("Validation time is unknown.");
        break;
    case BR_ERR_X509_EXPIRED:
        t = PSTR("Certificate is expired or not yet valid.");
        break;
    case BR_ERR_X509_DN_MISMATCH:
        t = PSTR("Issuer/Subject DN mismatch in the chain.");
        break;
    case BR_ERR_X509_BAD_SERVER_NAME:
        t = PSTR("Expected server name was not found in the chain.");
        break;
    case BR_ERR_X509_CRITICAL_EXTENSION:
        t = PSTR("Unknown critical extension in certificate.");
        break;
    case BR_ERR_X509_NOT_CA:
        t = PSTR("Not a CA, or path length constraint violation.");
        break;
    case BR_ERR_X509_FORBIDDEN_KEY_USAGE:
        t = PSTR("Key Usage extension prohibits intended usage.");
        break;
    case BR_ERR_X509_WEAK_PUBLIC_KEY:
        t = PSTR("Public key found in certificate is too small.");
        break;
    case BR_ERR_X509_NOT_TRUSTED:
        t = PSTR("Chain could not be linked to a trust anchor.");
        break;
    default:
        t = PSTR("Unknown error code.");
        break;
    }
    if (dest)
    {
        // snprintf is PSTR safe and guaranteed to 0-terminate
        snprintf(dest, len, "%s%s%s", recv_fatal, send_fatal, t);
    }
    return err;
}

char *ESP8266_SSL_Client::m_streamLoad(Stream &stream, size_t size)
{
    char *dest = (char *)malloc(size + 1);
    if (!dest)
    {
        return nullptr;
    }
    if (size != stream.readBytes(dest, size))
    {
        free(dest);
        dest = nullptr;
        return nullptr;
    }
    dest[size] = '\0';
    return dest;
}

#endif // ESP8266

#endif // ESP8266_SSL_Client_CPP