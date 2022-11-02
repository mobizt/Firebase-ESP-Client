/**
 *
 * The BearSSL Client Class, MB_BearSSL.h v2.0.0
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

#ifndef MB_BEARSSL_H_
#define MB_BEARSSL_H_

#if defined(ESP8266)

#ifdef __GNUC__
#if __GNUC__ > 4 || __GNUC__ == 10
#if defined(ARDUINO_ESP8266_GIT_VER)
#if ARDUINO_ESP8266_GIT_VER > 0
#define ESP8266_CORE_SDK_V3_X_X
#endif
#endif
#endif
#endif

#include "MB_ESP8266_SSLClient_FS.h"

#include <Arduino.h>
#include <bearssl/bearssl.h>
#include <vector>
#include <StackThunk.h>
#include <sys/time.h>
#include <IPAddress.h>
#include <Client.h>
#include <FS.h>
#include <time.h>
#include <ctype.h>
#include <vector>
#include <algorithm>


#ifdef MB_ESP8266_SSLCLIENT_ENABLE_DEBUG
#define MB_ESP8266_SSLCLIENT_DEBUG_PRINTF Serial.printf
#else
#define MB_ESP8266_SSLCLIENT_DEBUG_PRINTF(...)
#endif

// Stack thunked versions of calls
extern "C"
{
    extern unsigned char *thunk_br_ssl_engine_recvapp_buf(const br_ssl_engine_context *cc, size_t *len);
    extern void thunk_br_ssl_engine_recvapp_ack(br_ssl_engine_context *cc, size_t len);
    extern unsigned char *thunk_br_ssl_engine_recvrec_buf(const br_ssl_engine_context *cc, size_t *len);
    extern void thunk_br_ssl_engine_recvrec_ack(br_ssl_engine_context *cc, size_t len);
    extern unsigned char *thunk_br_ssl_engine_sendapp_buf(const br_ssl_engine_context *cc, size_t *len);
    extern void thunk_br_ssl_engine_sendapp_ack(br_ssl_engine_context *cc, size_t len);
    extern unsigned char *thunk_br_ssl_engine_sendrec_buf(const br_ssl_engine_context *cc, size_t *len);
    extern void thunk_br_ssl_engine_sendrec_ack(br_ssl_engine_context *cc, size_t len);
};

#if !CORE_MOCK

// The BearSSL thunks in use for now
#define br_ssl_engine_recvapp_ack thunk_br_ssl_engine_recvapp_ack
#define br_ssl_engine_recvapp_buf thunk_br_ssl_engine_recvapp_buf
#define br_ssl_engine_recvrec_ack thunk_br_ssl_engine_recvrec_ack
#define br_ssl_engine_recvrec_buf thunk_br_ssl_engine_recvrec_buf
#define br_ssl_engine_sendapp_ack thunk_br_ssl_engine_sendapp_ack
#define br_ssl_engine_sendapp_buf thunk_br_ssl_engine_sendapp_buf
#define br_ssl_engine_sendrec_ack thunk_br_ssl_engine_sendrec_ack
#define br_ssl_engine_sendrec_buf thunk_br_ssl_engine_sendrec_buf

#endif

static const uint16_t mb_suites_P[] PROGMEM = {
#ifndef BEARSSL_SSL_BASIC
    BR_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
    BR_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
    BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
    BR_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
    BR_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
    BR_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
    BR_TLS_ECDHE_ECDSA_WITH_AES_128_CCM,
    BR_TLS_ECDHE_ECDSA_WITH_AES_256_CCM,
    BR_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8,
    BR_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8,
    BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
    BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
    BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
    BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
    BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
    BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
    BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
    BR_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
    BR_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256,
    BR_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384,
    BR_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384,
    BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
    BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,
    BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384,
    BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384,
    BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
    BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
    BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
    BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
    BR_TLS_RSA_WITH_AES_128_GCM_SHA256,
    BR_TLS_RSA_WITH_AES_256_GCM_SHA384,
    BR_TLS_RSA_WITH_AES_128_CCM,
    BR_TLS_RSA_WITH_AES_256_CCM,
    BR_TLS_RSA_WITH_AES_128_CCM_8,
    BR_TLS_RSA_WITH_AES_256_CCM_8,
#endif
    BR_TLS_RSA_WITH_AES_128_CBC_SHA256,
    BR_TLS_RSA_WITH_AES_256_CBC_SHA256,
    BR_TLS_RSA_WITH_AES_128_CBC_SHA,
    BR_TLS_RSA_WITH_AES_256_CBC_SHA,
#ifndef BEARSSL_SSL_BASIC
    BR_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
    BR_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
    BR_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
    BR_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
    BR_TLS_RSA_WITH_3DES_EDE_CBC_SHA
#endif
};

static uint8_t __attribute__((used)) mb_htoi(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    else if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    else
        return 255;
}

// Represents a single server session.
// Use with BearSSL::ServerSessions.
typedef uint8_t BearSSL_ServerSession[100];

// Cache for the TLS sessions of multiple clients.
// Use with BearSSL::WiFiServerSecure::setCache
class BearSSL_ServerSessions
{
    friend class ESP8266_SSL_Client;

public:
    // Uses the given buffer to cache the given number of sessions and initializes it.
    BearSSL_ServerSessions(BearSSL_ServerSession *sessions, uint32_t size) : BearSSL_ServerSessions(sessions, size, false) {}

    // Dynamically allocates a cache for the given number of sessions and initializes it.
    // If the allocation of the buffer wasn't successful, the value
    // returned by size() will be 0.
    BearSSL_ServerSessions(uint32_t size) : BearSSL_ServerSessions(size > 0 ? new BearSSL_ServerSession[size] : nullptr, size, true) {}

    ~BearSSL_ServerSessions()
    {
        if (_isDynamic && _store != nullptr)
            delete _store;
    }

    // Returns the number of sessions the cache can hold.
    uint32_t size() { return _size; }

private:
    BearSSL_ServerSessions(BearSSL_ServerSession *sessions, uint32_t size, bool isDynamic) : _size(sessions != nullptr ? size : 0), _store(sessions), _isDynamic(isDynamic)
    {
        if (_size > 0)
            br_ssl_session_cache_lru_init(&_cache, (uint8_t *)_store, size * sizeof(BearSSL_ServerSession));
    }

    // Returns the cache's vtable or null if the cache has no capacity.
    const br_ssl_session_cache_class **getCache()
    {
        return _size > 0 ? &_cache.vtable : nullptr;
    }

    // Size of the store in sessions.
    uint32_t _size;
    // Store where the information for the sessions are stored.
    BearSSL_ServerSession *_store;
    // Whether the store is dynamically allocated.
    // If this is true, the store needs to be freed in the destructor.
    bool _isDynamic;

    // Cache of the server using the _store.
    br_ssl_session_cache_lru _cache;
};

// Cache for a TLS session with a server
// Use with BearSSL::WiFiClientSecure::setSession
// to accelerate the TLS handshake
class BearSSL_Session
{
    friend class ESP8266_SSL_Client;

public:
    BearSSL_Session() { memset(&_session, 0, sizeof(_session)); }

private:
    br_ssl_session_parameters *getSession() { return &_session; }
    // The actual BearSSL session information
    br_ssl_session_parameters _session;
};

namespace mb_bearssl
{
    // Code here is pulled from brssl sources, with the copyright and license
    // shown below.  I've rewritten things using C++ semantics and removed
    // custom VEC_* calls (std::vector to the rescue) and adjusted things to
    // allow for long-running operation (i.e. some memory issues when DERs
    // passed into the decoders).  Bugs are most likely my fault.

    // Original (c) message follows:
    /*
       Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>

       Permission is hereby granted, free of charge, to any person obtaining
       a copy of this software and associated documentation files (the
       "Software"), to deal in the Software without restriction, including
       without limitation the rights to use, copy, modify, merge, publish,
       distribute, sublicense, and/or sell copies of the Software, and to
       permit persons to whom the Software is furnished to do so, subject to
       the following conditions:

       The above copyright notice and this permission notice shall be
       included in all copies or substantial portions of the Software.

       THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
       EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
       MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
       NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
       BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
       ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
       CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
       SOFTWARE.
    */

    class mb_bearssl_private_key
    {
    public:
        int key_type; /* BR_KEYTYPE_RSA or BR_KEYTYPE_EC */
        union
        {
            br_rsa_private_key rsa;
            br_ec_private_key ec;
        } key;
    };

    class mb_bearssl_public_key
    {
    public:
        int key_type; /* BR_KEYTYPE_RSA or BR_KEYTYPE_EC */
        union
        {
            br_rsa_public_key rsa;
            br_ec_public_key ec;
        } key;
    };

    class mb_bearssl_pem_object
    {
    public:
        char *name;
        unsigned char *data;
        size_t data_len;
    };

    static void __attribute__((used)) mb_bearssl_free_ta_contents(br_x509_trust_anchor *ta)
    {
        if (ta)
        {
            free(ta->dn.data);
            if (ta->pkey.key_type == BR_KEYTYPE_RSA)
            {
                free(ta->pkey.key.rsa.n);
                free(ta->pkey.key.rsa.e);
            }
            else if (ta->pkey.key_type == BR_KEYTYPE_EC)
            {
                free(ta->pkey.key.ec.q);
            }
            memset(ta, 0, sizeof(*ta));
        }
    }

    static void __attribute__((used)) mb_bearssl_free_public_key(mb_bearssl_public_key *pk)
    {
        if (pk)
        {
            if (pk->key_type == BR_KEYTYPE_RSA)
            {
                free(pk->key.rsa.n);
                free(pk->key.rsa.e);
            }
            else if (pk->key_type == BR_KEYTYPE_EC)
            {
                free(pk->key.ec.q);
            }
            free(pk);
        }
    }

    static void __attribute__((used)) mb_bearssl_free_private_key(mb_bearssl_private_key *sk)
    {
        if (sk)
        {
            switch (sk->key_type)
            {
            case BR_KEYTYPE_RSA:
                free(sk->key.rsa.p);
                free(sk->key.rsa.q);
                free(sk->key.rsa.dp);
                free(sk->key.rsa.dq);
                free(sk->key.rsa.iq);
                break;
            case BR_KEYTYPE_EC:
                free(sk->key.ec.x);
                break;
            default:
                // Could be an uninitted key, no sub elements to free
                break;
            }
            free(sk);
        }
    }

    // Checks if a bitstream looks like a valid DER(binary) encoding.
    // Basically tries to verify the length of all included segments
    // matches the length of the input buffer.  Does not actually
    // validate any contents.
    static bool __attribute__((used)) mb_bearssl_looks_like_DER(const unsigned char *buff, size_t len)
    {
        if (len < 2)
        {
            return false;
        }
        if (pgm_read_byte(buff++) != 0x30)
        {
            return false;
        }
        int fb = pgm_read_byte(buff++);
        len -= 2;
        if (fb < 0x80)
        {
            return (size_t)fb == len;
        }
        else if (fb == 0x80)
        {
            return false;
        }
        else
        {
            fb -= 0x80;
            if (len < (size_t)fb + 2)
            {
                return false;
            }
            len -= (size_t)fb;
            size_t dlen = 0;
            while (fb-- > 0)
            {
                if (dlen > (len >> 8))
                {
                    return false;
                }
                dlen = (dlen << 8) + (size_t)pgm_read_byte(buff++);
            }
            return dlen == len;
        }
    }

    static void __attribute__((used)) mb_bearssl_free_pem_object_contents(mb_bearssl_pem_object *po)
    {
        if (po)
        {
            free(po->name);
            free(po->data);
            po->name = nullptr;
            po->data = nullptr;
        }
    }

    // Used as callback multiple places to append a string to a vector
    static void __attribute__((used)) mb_bearssl_byte_vector_append(void *ctx, const void *buff, size_t len)
    {
        std::vector<uint8_t> *vec = static_cast<std::vector<uint8_t> *>(ctx);
        vec->reserve(vec->size() + len); // Allocate extra space all at once
        for (size_t i = 0; i < len; i++)
        {
            vec->push_back(((uint8_t *)buff)[i]);
        }
    }

    // Converts a PEM (~=base64) source into a set of DER-encoded binary blobs.
    // Each blob is named by the ---- BEGIN xxx ---- field, and multiple
    // blobs may be returned.
    static mb_bearssl_pem_object *__attribute__((used)) mb_bearssl_decode_pem(const void *src, size_t len, size_t *num)
    {
        std::vector<mb_bearssl_pem_object> pem_list;
        std::unique_ptr<br_pem_decoder_context> pc(new br_pem_decoder_context); // auto-delete on exit
        if (!pc.get())
        {
            return nullptr;
        }
        mb_bearssl_pem_object po, *pos;
        const unsigned char *buff;
        std::vector<uint8_t> bv;

        *num = 0;
        br_pem_decoder_init(pc.get());
        buff = (const unsigned char *)src;
        po.name = nullptr;
        po.data = nullptr;
        po.data_len = 0;
        bool inobj = false;
        bool extra_nl = true;

        while (len > 0)
        {
            size_t tlen;

            tlen = br_pem_decoder_push(pc.get(), buff, len);
            buff += tlen;
            len -= tlen;
            switch (br_pem_decoder_event(pc.get()))
            {
            case BR_PEM_BEGIN_OBJ:
                po.name = strdup(br_pem_decoder_name(pc.get()));
                br_pem_decoder_setdest(pc.get(), mb_bearssl_byte_vector_append, &bv);
                inobj = true;
                break;

            case BR_PEM_END_OBJ:
                if (inobj)
                {
                    // Stick data into the vector
                    po.data = (uint8_t *)malloc(bv.size());
                    if (po.data)
                    {
                        memcpy(po.data, &bv[0], bv.size());
                        po.data_len = bv.size();
                        pem_list.push_back(po);
                    }
                    // Clean up state for next blob processing
                    bv.clear();
                    po.name = nullptr;
                    po.data = nullptr;
                    po.data_len = 0;
                    inobj = false;
                }
                break;

            case BR_PEM_ERROR:
                free(po.name);
                for (size_t i = 0; i < pem_list.size(); i++)
                {
                    mb_bearssl_free_pem_object_contents(&pem_list[i]);
                }
                return nullptr;

            default:
                // Do nothing here, the parser is still working on things
                break;
            }

            if (len == 0 && extra_nl)
            {
                extra_nl = false;
                buff = (const unsigned char *)"\n";
                len = 1;
            }
        }

        if (inobj)
        {
            free(po.name);
            for (size_t i = 0; i < pem_list.size(); i++)
            {
                mb_bearssl_free_pem_object_contents(&pem_list[i]);
            }
            return nullptr;
        }

        pos = (mb_bearssl_pem_object *)malloc((1 + pem_list.size()) * sizeof(*pos));
        if (pos)
        {
            *num = pem_list.size();
            pem_list.push_back(po); // Null-terminate list
            memcpy(pos, &pem_list[0], pem_list.size() * sizeof(*pos));
        }
        return pos;
    }

    static bool __attribute__((used)) mb_bearssl_certificate_to_trust_anchor_inner(br_x509_trust_anchor *ta, const br_x509_certificate *xc)
    {
        std::unique_ptr<br_x509_decoder_context> dc(new br_x509_decoder_context); // auto-delete on exit
        std::vector<uint8_t> vdn;
        br_x509_pkey *pk;

        // Clear everything in the Trust Anchor
        memset(ta, 0, sizeof(*ta));

        br_x509_decoder_init(dc.get(), mb_bearssl_byte_vector_append, (void *)&vdn, 0, 0);
        br_x509_decoder_push(dc.get(), xc->data, xc->data_len);
        pk = br_x509_decoder_get_pkey(dc.get());
        if (pk == nullptr)
        {
            return false; // No key present, something broken in the cert!
        }

        // Copy the raw certificate data
        ta->dn.data = (uint8_t *)malloc(vdn.size());
        if (!ta->dn.data)
        {
            return false; // OOM, but nothing yet allocated
        }
        memcpy(ta->dn.data, &vdn[0], vdn.size());
        ta->dn.len = vdn.size();
        ta->flags = 0;
        if (br_x509_decoder_isCA(dc.get()))
        {
            ta->flags |= BR_X509_TA_CA;
        }

        // Extract the public key
        switch (pk->key_type)
        {
        case BR_KEYTYPE_RSA:
            ta->pkey.key_type = BR_KEYTYPE_RSA;
            ta->pkey.key.rsa.n = (uint8_t *)malloc(pk->key.rsa.nlen);
            ta->pkey.key.rsa.e = (uint8_t *)malloc(pk->key.rsa.elen);
            if ((ta->pkey.key.rsa.n == nullptr) || (ta->pkey.key.rsa.e == nullptr))
            {
                mb_bearssl_free_ta_contents(ta); // OOM, so clean up
                return false;
            }
            memcpy(ta->pkey.key.rsa.n, pk->key.rsa.n, pk->key.rsa.nlen);
            ta->pkey.key.rsa.nlen = pk->key.rsa.nlen;
            memcpy(ta->pkey.key.rsa.e, pk->key.rsa.e, pk->key.rsa.elen);
            ta->pkey.key.rsa.elen = pk->key.rsa.elen;
            return true;
        case BR_KEYTYPE_EC:
            ta->pkey.key_type = BR_KEYTYPE_EC;
            ta->pkey.key.ec.curve = pk->key.ec.curve;
            ta->pkey.key.ec.q = (uint8_t *)malloc(pk->key.ec.qlen);
            if (ta->pkey.key.ec.q == nullptr)
            {
                mb_bearssl_free_ta_contents(ta); // OOM, so clean up
                return false;
            }
            memcpy(ta->pkey.key.ec.q, pk->key.ec.q, pk->key.ec.qlen);
            ta->pkey.key.ec.qlen = pk->key.ec.qlen;
            return true;
        default:
            mb_bearssl_free_ta_contents(ta); // Unknown key type
            return false;
        }

        // Should never get here, if so there was an unknown error
        return false;
    }

    static br_x509_trust_anchor *__attribute__((used)) mb_bearssl_certificate_to_trust_anchor(const br_x509_certificate *xc)
    {
        br_x509_trust_anchor *ta = (br_x509_trust_anchor *)malloc(sizeof(br_x509_trust_anchor));
        if (!ta)
        {
            return nullptr;
        }

        if (!mb_bearssl_certificate_to_trust_anchor_inner(ta, xc))
        {
            free(ta);
            return nullptr;
        }
        return ta;
    }

    // Parse out DER or PEM encoded certificates from a binary buffer,
    // potentially stored in PROGMEM.
    static br_x509_certificate *__attribute__((used)) mb_bearssl_read_certificates(const char *buff, size_t len, size_t *num)
    {
        std::vector<br_x509_certificate> cert_list;
        mb_bearssl_pem_object *pos;
        size_t u, num_pos;
        br_x509_certificate *xcs;
        br_x509_certificate dummy;

        *num = 0;

        if (mb_bearssl_looks_like_DER((const unsigned char *)buff, len))
        {
            xcs = (br_x509_certificate *)malloc(2 * sizeof(*xcs));
            if (!xcs)
            {
                return nullptr;
            }
            xcs[0].data = (uint8_t *)malloc(len);
            if (!xcs[0].data)
            {
                free(xcs);
                return nullptr;
            }
            memcpy_P(xcs[0].data, buff, len);
            xcs[0].data_len = len;
            xcs[1].data = nullptr;
            xcs[1].data_len = 0;
            *num = 1;
            return xcs;
        }

        pos = mb_bearssl_decode_pem(buff, len, &num_pos);
        if (!pos)
        {
            return nullptr;
        }
        for (u = 0; u < num_pos; u++)
        {
            if (!strcmp_P(pos[u].name, PSTR("CERTIFICATE")) || !strcmp_P(pos[u].name, PSTR("X509 CERTIFICATE")))
            {
                br_x509_certificate xc;
                xc.data = pos[u].data;
                xc.data_len = pos[u].data_len;
                pos[u].data = nullptr; // Don't free the data we moved to the xc vector!
                cert_list.push_back(xc);
            }
        }
        for (u = 0; u < num_pos; u++)
        {
            mb_bearssl_free_pem_object_contents(&pos[u]);
        }
        free(pos);

        if (cert_list.size() == 0)
        {
            return nullptr;
        }
        *num = cert_list.size();
        dummy.data = nullptr;
        dummy.data_len = 0;
        cert_list.push_back(dummy);
        xcs = (br_x509_certificate *)malloc(cert_list.size() * sizeof(*xcs));
        if (!xcs)
        {
            for (size_t i = 0; i < cert_list.size(); i++)
            {
                free(cert_list[i].data); // Clean up any captured data blobs
            }
            return nullptr;
        }
        memcpy(xcs, &cert_list[0], cert_list.size() * sizeof(br_x509_certificate));
        // XCS now has [].data pointing to the previously allocated blobs, so don't
        // want to free anything in cert_list[].
        return xcs;
    }

    static void __attribute__((used)) mb_bearssl_free_certificates(br_x509_certificate *certs, size_t num)
    {
        if (certs)
        {
            for (size_t u = 0; u < num; u++)
            {
                free(certs[u].data);
            }
            free(certs);
        }
    }

    static mb_bearssl_public_key *__attribute__((used)) mb_bearssl_decode_public_key(const unsigned char *buff, size_t len)
    {
        std::unique_ptr<br_pkey_decoder_context> dc(new br_pkey_decoder_context); // auto-delete on exit
        if (!dc.get())
        {
            return nullptr;
        }

        mb_bearssl_public_key *pk = nullptr;

        br_pkey_decoder_init(dc.get());
        br_pkey_decoder_push(dc.get(), buff, len);
        int err = br_pkey_decoder_last_error(dc.get());
        if (err != 0)
        {
            return nullptr;
        }

        const br_rsa_public_key *rk = nullptr;
        const br_ec_public_key *ek = nullptr;
        switch (br_pkey_decoder_key_type(dc.get()))
        {
        case BR_KEYTYPE_RSA:
            rk = br_pkey_decoder_get_rsa(dc.get());
            pk = (mb_bearssl_public_key *)malloc(sizeof *pk);
            if (!pk)
            {
                return nullptr;
            }
            pk->key_type = BR_KEYTYPE_RSA;
            pk->key.rsa.n = (uint8_t *)malloc(rk->nlen);
            pk->key.rsa.e = (uint8_t *)malloc(rk->elen);
            if (!pk->key.rsa.n || !pk->key.rsa.e)
            {
                free(pk->key.rsa.n);
                free(pk->key.rsa.e);
                free(pk);
                return nullptr;
            }
            memcpy(pk->key.rsa.n, rk->n, rk->nlen);
            pk->key.rsa.nlen = rk->nlen;
            memcpy(pk->key.rsa.e, rk->e, rk->elen);
            pk->key.rsa.elen = rk->elen;
            return pk;

        case BR_KEYTYPE_EC:
            ek = br_pkey_decoder_get_ec(dc.get());
            pk = (mb_bearssl_public_key *)malloc(sizeof *pk);
            if (!pk)
            {
                return nullptr;
            }
            pk->key_type = BR_KEYTYPE_EC;
            pk->key.ec.q = (uint8_t *)malloc(ek->qlen);
            if (!pk->key.ec.q)
            {
                free(pk);
                return nullptr;
            }
            memcpy(pk->key.ec.q, ek->q, ek->qlen);
            pk->key.ec.qlen = ek->qlen;
            pk->key.ec.curve = ek->curve;
            return pk;

        default:
            return nullptr;
        }
    }

    static mb_bearssl_private_key *__attribute__((used)) mb_bearssl_decode_private_key(const unsigned char *buff, size_t len)
    {
        std::unique_ptr<br_skey_decoder_context> dc(new br_skey_decoder_context); // auto-delete on exit
        if (!dc.get())
        {
            return nullptr;
        }

        mb_bearssl_private_key *sk = nullptr;

        br_skey_decoder_init(dc.get());
        br_skey_decoder_push(dc.get(), buff, len);
        int err = br_skey_decoder_last_error(dc.get());
        if (err != 0)
        {
            return nullptr;
        }

        const br_rsa_private_key *rk = nullptr;
        const br_ec_private_key *ek = nullptr;
        switch (br_skey_decoder_key_type(dc.get()))
        {
        case BR_KEYTYPE_RSA:
            rk = br_skey_decoder_get_rsa(dc.get());
            sk = (mb_bearssl_private_key *)malloc(sizeof *sk);
            if (!sk)
            {
                return nullptr;
            }
            sk->key_type = BR_KEYTYPE_RSA;
            sk->key.rsa.p = (uint8_t *)malloc(rk->plen);
            sk->key.rsa.q = (uint8_t *)malloc(rk->qlen);
            sk->key.rsa.dp = (uint8_t *)malloc(rk->dplen);
            sk->key.rsa.dq = (uint8_t *)malloc(rk->dqlen);
            sk->key.rsa.iq = (uint8_t *)malloc(rk->iqlen);
            if (!sk->key.rsa.p || !sk->key.rsa.q || !sk->key.rsa.dp || !sk->key.rsa.dq || !sk->key.rsa.iq)
            {
                mb_bearssl_free_private_key(sk);
                return nullptr;
            }
            sk->key.rsa.n_bitlen = rk->n_bitlen;
            memcpy(sk->key.rsa.p, rk->p, rk->plen);
            sk->key.rsa.plen = rk->plen;
            memcpy(sk->key.rsa.q, rk->q, rk->qlen);
            sk->key.rsa.qlen = rk->qlen;
            memcpy(sk->key.rsa.dp, rk->dp, rk->dplen);
            sk->key.rsa.dplen = rk->dplen;
            memcpy(sk->key.rsa.dq, rk->dq, rk->dqlen);
            sk->key.rsa.dqlen = rk->dqlen;
            memcpy(sk->key.rsa.iq, rk->iq, rk->iqlen);
            sk->key.rsa.iqlen = rk->iqlen;
            return sk;

        case BR_KEYTYPE_EC:
            ek = br_skey_decoder_get_ec(dc.get());
            sk = (mb_bearssl_private_key *)malloc(sizeof *sk);
            if (!sk)
            {
                return nullptr;
            }
            sk->key_type = BR_KEYTYPE_EC;
            sk->key.ec.curve = ek->curve;
            sk->key.ec.x = (uint8_t *)malloc(ek->xlen);
            if (!sk->key.ec.x)
            {
                mb_bearssl_free_private_key(sk);
                return nullptr;
            }
            memcpy(sk->key.ec.x, ek->x, ek->xlen);
            sk->key.ec.xlen = ek->xlen;
            return sk;

        default:
            return nullptr;
        }
    }

    static void __attribute__((used)) mb_bearssl_free_pem_object(mb_bearssl_pem_object *pos)
    {
        if (pos != nullptr)
        {
            for (size_t u = 0; pos[u].name; u++)
            {
                mb_bearssl_free_pem_object_contents(&pos[u]);
            }
            free(pos);
        }
    }

    static mb_bearssl_private_key *__attribute__((used)) mb_bearssl_read_private_key(const char *buff, size_t len)
    {
        mb_bearssl_private_key *sk = nullptr;
        mb_bearssl_pem_object *pos = nullptr;

        if (mb_bearssl_looks_like_DER((const unsigned char *)buff, len))
        {
            sk = mb_bearssl_decode_private_key((const unsigned char *)buff, len);
            return sk;
        }

        size_t num;
        pos = mb_bearssl_decode_pem(buff, len, &num);
        if (pos == nullptr)
        {
            return nullptr; // PEM decode error
        }
        for (size_t u = 0; pos[u].name; u++)
        {
            const char *name = pos[u].name;
            if (!strcmp_P(name, PSTR("RSA PRIVATE KEY")) || !strcmp_P(name, PSTR("EC PRIVATE KEY")) || !strcmp_P(name, PSTR("PRIVATE KEY")))
            {
                sk = mb_bearssl_decode_private_key(pos[u].data, pos[u].data_len);
                mb_bearssl_free_pem_object(pos);
                return sk;
            }
        }
        // If we hit here, no match
        mb_bearssl_free_pem_object(pos);
        return nullptr;
    }

    static mb_bearssl_public_key *__attribute__((used)) mb_bearssl_read_public_key(const char *buff, size_t len)
    {
        mb_bearssl_public_key *pk = nullptr;
        mb_bearssl_pem_object *pos = nullptr;

        if (mb_bearssl_looks_like_DER((const unsigned char *)buff, len))
        {
            pk = mb_bearssl_decode_public_key((const unsigned char *)buff, len);
            return pk;
        }
        size_t num;
        pos = mb_bearssl_decode_pem(buff, len, &num);
        if (pos == nullptr)
        {
            return nullptr; // PEM decode error
        }
        for (size_t u = 0; pos[u].name; u++)
        {
            const char *name = pos[u].name;
            if (!strcmp_P(name, PSTR("RSA PUBLIC KEY")) || !strcmp_P(name, PSTR("EC PUBLIC KEY")) || !strcmp_P(name, PSTR("PUBLIC KEY")))
            {
                pk = mb_bearssl_decode_public_key(pos[u].data, pos[u].data_len);
                mb_bearssl_free_pem_object(pos);
                return pk;
            }
        }

        // We hit here == no key found
        mb_bearssl_free_pem_object(pos);
        return pk;
    }

    static uint8_t *__attribute__((used)) mb_bearssl_loadStream(Stream &stream, size_t size)
    {
        uint8_t *dest = (uint8_t *)malloc(size);
        if (!dest)
        {
            return nullptr; // OOM error
        }
        if (size != stream.readBytes(dest, size))
        {
            free(dest); // Error during read
            return nullptr;
        }
        return dest;
    }
};

// Holds either a single public RSA or EC key for use when BearSSL wants a pubkey.
// Copies all associated data so no need to keep input PEM/DER keys.
// All inputs can be either in RAM or PROGMEM.
class BearSSL_PublicKey
{
public:
    BearSSL_PublicKey()
    {
        _key = nullptr;
    }

    BearSSL_PublicKey(const char *pemKey)
    {
        _key = nullptr;
        parse(pemKey);
    }

    BearSSL_PublicKey(const uint8_t *derKey, size_t derLen)
    {
        _key = nullptr;
        parse(derKey, derLen);
    }

    BearSSL_PublicKey(Stream &stream, size_t size)
    {
        _key = nullptr;
        auto buff = mb_bearssl::mb_bearssl_loadStream(stream, size);
        if (buff)
        {
            parse(buff, size);
            free(buff);
        }
    }

    BearSSL_PublicKey(Stream &stream) : BearSSL_PublicKey(stream, stream.available()){};

    ~BearSSL_PublicKey()
    {
        if (_key)
        {
            mb_bearssl::mb_bearssl_free_public_key(_key);
        }
    }

    bool parse(const char *pemKey)
    {
        return parse((const uint8_t *)pemKey, strlen_P(pemKey));
    }

    bool parse(const uint8_t *derKey, size_t derLen)
    {
        if (_key)
        {
            mb_bearssl::mb_bearssl_free_public_key(_key);
            _key = nullptr;
        }
        _key = mb_bearssl::mb_bearssl_read_public_key((const char *)derKey, derLen);
        return _key ? true : false;
    }

    // Accessors for internal use, not needed by apps
    bool isRSA() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_RSA)
        {
            return false;
        }
        return true;
    }

    bool isEC() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_EC)
        {
            return false;
        }
        return true;
    }

    const br_rsa_public_key *getRSA() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_RSA)
        {
            return nullptr;
        }
        return &_key->key.rsa;
    }

    const br_ec_public_key *getEC() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_EC)
        {
            return nullptr;
        }
        return &_key->key.ec;
    }

    // Disable the copy constructor, we're pointer based
    BearSSL_PublicKey(const BearSSL_PublicKey &that) = delete;

private:
    mb_bearssl::mb_bearssl_public_key *_key;
};

// Holds either a single private RSA or EC key for use when BearSSL wants a secretkey.
// Copies all associated data so no need to keep input PEM/DER keys.
// All inputs can be either in RAM or PROGMEM.
class BearSSL_PrivateKey
{
public:
    BearSSL_PrivateKey()
    {
        _key = nullptr;
    }

    BearSSL_PrivateKey(const char *pemKey)
    {
        _key = nullptr;
        parse(pemKey);
    }

    BearSSL_PrivateKey(const uint8_t *derKey, size_t derLen)
    {
        _key = nullptr;
        parse(derKey, derLen);
    }

    BearSSL_PrivateKey(Stream &stream, size_t size)
    {
        _key = nullptr;
        auto buff = mb_bearssl::mb_bearssl_loadStream(stream, size);
        if (buff)
        {
            parse(buff, size);
            free(buff);
        }
    }

    BearSSL_PrivateKey(Stream &stream) : BearSSL_PrivateKey(stream, stream.available()){};

    ~BearSSL_PrivateKey()
    {
        if (_key)
        {
            mb_bearssl::mb_bearssl_free_private_key(_key);
        }
    }

    bool parse(const char *pemKey)
    {
        return parse((const uint8_t *)pemKey, strlen_P(pemKey));
    }

    bool parse(const uint8_t *derKey, size_t derLen)
    {
        if (_key)
        {
            mb_bearssl::mb_bearssl_free_private_key(_key);
            _key = nullptr;
        }
        _key = mb_bearssl::mb_bearssl_read_private_key((const char *)derKey, derLen);
        return _key ? true : false;
    }

    // Accessors for internal use, not needed by apps
    bool isRSA() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_RSA)
        {
            return false;
        }
        return true;
    }

    bool isEC() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_EC)
        {
            return false;
        }
        return true;
    }

    const br_rsa_private_key *getRSA() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_RSA)
        {
            return nullptr;
        }
        return &_key->key.rsa;
    }

    const br_ec_private_key *getEC() const
    {
        if (!_key || _key->key_type != BR_KEYTYPE_EC)
        {
            return nullptr;
        }
        return &_key->key.ec;
    }

    // Disable the copy constructor, we're pointer based
    BearSSL_PrivateKey(const BearSSL_PrivateKey &that) = delete;

private:
    mb_bearssl::mb_bearssl_private_key *_key;
};

// Holds one or more X.509 certificates and associated trust anchors for
// use whenever BearSSL needs a cert or TA.  May want to have multiple
// certs for things like a series of trusted CAs (but check the CertStore class
// for a more memory efficient way).
// Copies all associated data so no need to keep input PEM/DER certs.
// All inputs can be either in RAM or PROGMEM.
class BearSSL_X509List
{
public:
    BearSSL_X509List()
    {
        _count = 0;
        _cert = nullptr;
        _ta = nullptr;
    }

    BearSSL_X509List(const char *pemCert)
    {
        _count = 0;
        _cert = nullptr;
        _ta = nullptr;
        append(pemCert);
    }

    BearSSL_X509List(const uint8_t *derCert, size_t derLen)
    {
        _count = 0;
        _cert = nullptr;
        _ta = nullptr;
        append(derCert, derLen);
    }

    BearSSL_X509List(Stream &stream, size_t size)
    {
        _count = 0;
        _cert = nullptr;
        _ta = nullptr;
        auto buff = mb_bearssl::mb_bearssl_loadStream(stream, size);
        if (buff)
        {
            append(buff, size);
            free(buff);
        }
    }

    BearSSL_X509List(Stream &stream) : BearSSL_X509List(stream, stream.available()){};

    ~BearSSL_X509List()
    {
        mb_bearssl::mb_bearssl_free_certificates(_cert, _count); // also frees cert
        for (size_t i = 0; i < _count; i++)
        {
            mb_bearssl::mb_bearssl_free_ta_contents(&_ta[i]);
        }
        free(_ta);
    }

    bool append(const char *pemCert)
    {
        return append((const uint8_t *)pemCert, strlen_P(pemCert));
    }

    bool append(const uint8_t *derCert, size_t derLen)
    {
        size_t numCerts;
        br_x509_certificate *newCerts = mb_bearssl::mb_bearssl_read_certificates((const char *)derCert, derLen, &numCerts);
        if (!newCerts)
        {
            return false;
        }

        // Add in the certificates
        br_x509_certificate *saveCert = _cert;
        _cert = (br_x509_certificate *)realloc(_cert, (numCerts + _count) * sizeof(br_x509_certificate));
        if (!_cert)
        {
            free(newCerts);
            _cert = saveCert;
            return false;
        }
        memcpy(&_cert[_count], newCerts, numCerts * sizeof(br_x509_certificate));
        free(newCerts);

        // Build TAs for each certificate
        br_x509_trust_anchor *saveTa = _ta;
        _ta = (br_x509_trust_anchor *)realloc(_ta, (numCerts + _count) * sizeof(br_x509_trust_anchor));
        if (!_ta)
        {
            _ta = saveTa;
            return false;
        }
        for (size_t i = 0; i < numCerts; i++)
        {
            br_x509_trust_anchor *newTa = mb_bearssl::mb_bearssl_certificate_to_trust_anchor(&_cert[_count + i]);
            if (newTa)
            {
                _ta[_count + i] = *newTa;
                free(newTa);
            }
            else
            {
                return false; // OOM
            }
        }
        _count += numCerts;

        return true;
    }

    // Accessors
    size_t getCount() const
    {
        return _count;
    }
    const br_x509_certificate *getX509Certs() const
    {
        return _cert;
    }
    const br_x509_trust_anchor *getTrustAnchors() const
    {
        return _ta;
    }

    // Disable the copy constructor, we're pointer based
    BearSSL_X509List(const BearSSL_X509List &that) = delete;

private:
    size_t _count;
    br_x509_certificate *_cert;
    br_x509_trust_anchor *_ta;
};

class BearSSL_CertStoreBase
{
public:
    virtual ~BearSSL_CertStoreBase() {}

    // Installs the cert store into the X509 decoder (normally via static function callbacks)
    virtual void installCertStore(br_x509_minimal_context *ctx) = 0;
};

class BearSSL_CertStore : public BearSSL_CertStoreBase
{
public:
    BearSSL_CertStore(){};
    ~BearSSL_CertStore();

    // Set the file interface instances, do preprocessing
    int initCertStore(fs::FS &fs, const char *indexFileName, const char *dataFileName);

    // Installs the cert store into the X509 decoder (normally via static function callbacks)
    void installCertStore(br_x509_minimal_context *ctx);

protected:
    fs::FS *_fs = nullptr;
    char *_indexName = nullptr;
    char *_dataName = nullptr;
    BearSSL_X509List *_x509 = nullptr;

    // These need to be static as they are callbacks from BearSSL C code
    static const br_x509_trust_anchor *findHashedTA(void *ctx, void *hashed_dn, size_t len);
    static void freeHashedTA(void *ctx, const br_x509_trust_anchor *ta);

    // The binary format of the index file
    class CertInfo
    {
    public:
        uint8_t sha256[32];
        uint32_t offset;
        uint32_t length;
    };
    static CertInfo _preprocessCert(uint32_t length, uint32_t offset, const void *raw);
};

extern "C"
{

    // Some constants uses to init the server/client contexts
    // Note that suites_P needs to be copied to RAM before use w/BearSSL!
    // List copied verbatim from BearSSL/ssl_client_full.c
    /*
     * The "full" profile supports all implemented cipher suites.
     *
     * Rationale for suite order, from most important to least
     * important rule:
     *
     * -- Don't use 3DES if AES or ChaCha20 is available.
     * -- Try to have Forward Secrecy (ECDHE suite) if possible.
     * -- When not using Forward Secrecy, ECDH key exchange is
     *    better than RSA key exchange (slightly more expensive on the
     *    client, but much cheaper on the server, and it implies smaller
     *    messages).
     * -- ChaCha20+Poly1305 is better than AES/GCM (faster, smaller code).
     * -- GCM is better than CCM and CBC. CCM is better than CBC.
     * -- CCM is preferable over CCM_8 (with CCM_8, forgeries may succeed
     *    with probability 2^(-64)).
     * -- AES-128 is preferred over AES-256 (AES-128 is already
     *    strong enough, and AES-256 is 40% more expensive).
     */
    static const uint16_t mb_bearssl_suites_P[] PROGMEM = {
#ifndef BEARSSL_SSL_BASIC
        BR_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
        BR_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CCM,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CCM,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
        BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
        BR_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
        BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
        BR_TLS_RSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_RSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_RSA_WITH_AES_128_CCM,
        BR_TLS_RSA_WITH_AES_256_CCM,
        BR_TLS_RSA_WITH_AES_128_CCM_8,
        BR_TLS_RSA_WITH_AES_256_CCM_8,
#endif
        BR_TLS_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_RSA_WITH_AES_256_CBC_SHA256,
        BR_TLS_RSA_WITH_AES_128_CBC_SHA,
        BR_TLS_RSA_WITH_AES_256_CBC_SHA,
#ifndef BEARSSL_SSL_BASIC
        BR_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_RSA_WITH_3DES_EDE_CBC_SHA
#endif
    };
#ifndef BEARSSL_SSL_BASIC
    // Server w/EC has one set, not possible with basic SSL config
    static const uint16_t mb_bearssl_suites_server_ec_P[] PROGMEM = {
        BR_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CCM,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CCM,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
        BR_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
        BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
        BR_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA};
#endif

    static const uint16_t mb_bearssl_suites_server_rsa_P[] PROGMEM = {
#ifndef BEARSSL_SSL_BASIC
        BR_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
        BR_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
        BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
        BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
        BR_TLS_RSA_WITH_AES_128_GCM_SHA256,
        BR_TLS_RSA_WITH_AES_256_GCM_SHA384,
        BR_TLS_RSA_WITH_AES_128_CCM,
        BR_TLS_RSA_WITH_AES_256_CCM,
        BR_TLS_RSA_WITH_AES_128_CCM_8,
        BR_TLS_RSA_WITH_AES_256_CCM_8,
#endif
        BR_TLS_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_RSA_WITH_AES_256_CBC_SHA256,
        BR_TLS_RSA_WITH_AES_128_CBC_SHA,
        BR_TLS_RSA_WITH_AES_256_CBC_SHA,
#ifndef BEARSSL_SSL_BASIC
        BR_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
        BR_TLS_RSA_WITH_3DES_EDE_CBC_SHA
#endif
    };

    // For apps which want to use less secure but faster ciphers, only
    static const uint16_t mb_bearssl_faster_suites_P[] PROGMEM = {
        BR_TLS_RSA_WITH_AES_256_CBC_SHA256,
        BR_TLS_RSA_WITH_AES_128_CBC_SHA256,
        BR_TLS_RSA_WITH_AES_256_CBC_SHA,
        BR_TLS_RSA_WITH_AES_128_CBC_SHA};

    // BearSSL doesn't define a true insecure decoder, so we make one ourselves
    // from the simple parser.  It generates the issuer and subject hashes and
    // the SHA1 fingerprint, only one (or none!) of which will be used to
    // "verify" the certificate.

    // Private x509 decoder state
    struct mb_br_x509_insecure_context
    {
        const br_x509_class *vtable;
        bool done_cert;
        const uint8_t *match_fingerprint;
        br_sha1_context sha1_cert;
        bool allow_self_signed;
        br_sha256_context sha256_subject;
        br_sha256_context sha256_issuer;
        br_x509_decoder_context ctx;
    };

    // Install hashes into the SSL engine
    static void __attribute__((used)) mb_bearssl_br_ssl_client_install_hashes(br_ssl_engine_context *eng)
    {
        br_ssl_engine_set_hash(eng, br_md5_ID, &br_md5_vtable);
        br_ssl_engine_set_hash(eng, br_sha1_ID, &br_sha1_vtable);
        br_ssl_engine_set_hash(eng, br_sha224_ID, &br_sha224_vtable);
        br_ssl_engine_set_hash(eng, br_sha256_ID, &br_sha256_vtable);
        br_ssl_engine_set_hash(eng, br_sha384_ID, &br_sha384_vtable);
        br_ssl_engine_set_hash(eng, br_sha512_ID, &br_sha512_vtable);
    }

    static void __attribute__((used)) mb_bearssl_br_x509_minimal_install_hashes(br_x509_minimal_context *x509)
    {
        br_x509_minimal_set_hash(x509, br_md5_ID, &br_md5_vtable);
        br_x509_minimal_set_hash(x509, br_sha1_ID, &br_sha1_vtable);
        br_x509_minimal_set_hash(x509, br_sha224_ID, &br_sha224_vtable);
        br_x509_minimal_set_hash(x509, br_sha256_ID, &br_sha256_vtable);
        br_x509_minimal_set_hash(x509, br_sha384_ID, &br_sha384_vtable);
        br_x509_minimal_set_hash(x509, br_sha512_ID, &br_sha512_vtable);
    }

    // Default initializion for our SSL clients
    static void __attribute__((used)) mb_bearssl_br_ssl_client_base_init(br_ssl_client_context *cc, const uint16_t *cipher_list, int cipher_cnt)
    {
        uint16_t suites[cipher_cnt];
        memcpy_P(suites, cipher_list, cipher_cnt * sizeof(cipher_list[0]));
        br_ssl_client_zero(cc);
        br_ssl_engine_add_flags(&cc->eng, BR_OPT_NO_RENEGOTIATION); // forbid SSL renegotiation, as we free the Private Key after handshake
        br_ssl_engine_set_versions(&cc->eng, BR_TLS10, BR_TLS12);
        br_ssl_engine_set_suites(&cc->eng, suites, (sizeof suites) / (sizeof suites[0]));
        br_ssl_client_set_default_rsapub(cc);
        br_ssl_engine_set_default_rsavrfy(&cc->eng);
#ifndef BEARSSL_SSL_BASIC
        br_ssl_engine_set_default_ecdsa(&cc->eng);
#endif
        mb_bearssl_br_ssl_client_install_hashes(&cc->eng);
        br_ssl_engine_set_prf10(&cc->eng, &br_tls10_prf);
        br_ssl_engine_set_prf_sha256(&cc->eng, &br_tls12_sha256_prf);
        br_ssl_engine_set_prf_sha384(&cc->eng, &br_tls12_sha384_prf);
        br_ssl_engine_set_default_aes_cbc(&cc->eng);
#ifndef BEARSSL_SSL_BASIC
        br_ssl_engine_set_default_aes_gcm(&cc->eng);
        br_ssl_engine_set_default_aes_ccm(&cc->eng);
        br_ssl_engine_set_default_des_cbc(&cc->eng);
        br_ssl_engine_set_default_chapol(&cc->eng);
#endif
    }

    // Default initializion for our SSL clients
    static void __attribute__((used)) mb_bearssl_br_ssl_server_base_init(br_ssl_server_context *cc, const uint16_t *cipher_list, int cipher_cnt)
    {
        uint16_t suites[cipher_cnt];
        memcpy_P(suites, cipher_list, cipher_cnt * sizeof(cipher_list[0]));
        br_ssl_server_zero(cc);
        br_ssl_engine_add_flags(&cc->eng, BR_OPT_NO_RENEGOTIATION); // forbid SSL renegotiation, as we free the Private Key after handshake
        br_ssl_engine_set_versions(&cc->eng, BR_TLS10, BR_TLS12);
        br_ssl_engine_set_suites(&cc->eng, suites, (sizeof suites) / (sizeof suites[0]));
#ifndef BEARSSL_SSL_BASIC
        br_ssl_engine_set_default_ec(&cc->eng);
#endif

        mb_bearssl_br_ssl_client_install_hashes(&cc->eng);
        br_ssl_engine_set_prf10(&cc->eng, &br_tls10_prf);
        br_ssl_engine_set_prf_sha256(&cc->eng, &br_tls12_sha256_prf);
        br_ssl_engine_set_prf_sha384(&cc->eng, &br_tls12_sha384_prf);
        br_ssl_engine_set_default_aes_cbc(&cc->eng);
#ifndef BEARSSL_SSL_BASIC
        br_ssl_engine_set_default_aes_ccm(&cc->eng);
        br_ssl_engine_set_default_aes_gcm(&cc->eng);
        br_ssl_engine_set_default_des_cbc(&cc->eng);
        br_ssl_engine_set_default_chapol(&cc->eng);
#endif
    }

    // Callback for the x509_minimal subject DN
    static void __attribute__((used)) mb_bearssl_insecure_subject_dn_append(void *ctx, const void *buf, size_t len)
    {
        mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
        br_sha256_update(&xc->sha256_subject, buf, len);
    }

    // Callback for the x509_minimal issuer DN
    static void __attribute__((used)) mb_bearssl_insecure_issuer_dn_append(void *ctx, const void *buf, size_t len)
    {
        mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
        br_sha256_update(&xc->sha256_issuer, buf, len);
    }
}

static void __attribute__((used)) mb_bearssl_dump_blob(unsigned const char *buf, size_t len)
{

    size_t u;

    for (u = 0; u < len; u++)
    {
        if ((u & 15) == 0)
        {
            MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("\n%08lX  ", (unsigned long)u);
        }
        else if ((u & 7) == 0)
        {
            Serial.print(" ");
        }
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF(" %02x", buf[u]);
    }

    MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("\n");
}

// Callback for the x509_minimal subject DN
static void __attribute__((used)) mb_insecure_subject_dn_append(void *ctx, const void *buf, size_t len)
{
    mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
    br_sha256_update(&xc->sha256_subject, buf, len);
}

// Callback for the x509_minimal issuer DN
static void __attribute__((used)) mb_insecure_issuer_dn_append(void *ctx, const void *buf, size_t len)
{
    mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
    br_sha256_update(&xc->sha256_issuer, buf, len);
}

// Callback on the first byte of any certificate
static void __attribute__((used)) mb_insecure_start_chain(const br_x509_class **ctx, const char *server_name)
{
    mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
    br_x509_decoder_init(&xc->ctx, mb_insecure_subject_dn_append, xc, mb_insecure_issuer_dn_append, xc);
    xc->done_cert = false;
    br_sha1_init(&xc->sha1_cert);
    br_sha256_init(&xc->sha256_subject);
    br_sha256_init(&xc->sha256_issuer);
    (void)server_name;
}

static void __attribute__((used)) mb_insecure_start_cert(const br_x509_class **ctx, uint32_t length)
{
    (void)ctx;
    (void)length;
}

// Callback for each byte stream in the chain.  Only process first cert.
static void __attribute__((used)) mb_insecure_append(const br_x509_class **ctx, const unsigned char *buf, size_t len)
{
    mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
    // Don't process anything but the first certificate in the chain
    if (!xc->done_cert)
    {
        br_sha1_update(&xc->sha1_cert, buf, len);
        br_x509_decoder_push(&xc->ctx, (const void *)buf, len);
    }
}

// Callback on individual cert end.
static void __attribute__((used)) mb_insecure_end_cert(const br_x509_class **ctx)
{
    mb_br_x509_insecure_context *xc = (mb_br_x509_insecure_context *)ctx;
    xc->done_cert = true;
}

// Callback when complete chain has been parsed.
// Return 0 on validation success, !0 on validation error
static unsigned __attribute__((used)) mb_insecure_end_chain(const br_x509_class **ctx)
{
    const mb_br_x509_insecure_context *xc = (const mb_br_x509_insecure_context *)ctx;
    if (!xc->done_cert)
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("insecure_end_chain: No cert seen\n");
        return 1; // error
    }

    // Handle SHA1 fingerprint matching
    char res[20];
    br_sha1_out(&xc->sha1_cert, res);
    if (xc->match_fingerprint && memcmp(res, xc->match_fingerprint, sizeof(res)))
    {
#ifdef MB_ESP8266_SSLCLIENT_ENABLE_DEBUG
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("insecure_end_chain: Received cert FP doesn't match\n");
        char buff[3 * sizeof(res) + 1]; // 3 chars per byte XX_, and null
        buff[0] = 0;
        for (size_t i = 0; i < sizeof(res); i++)
        {
            char hex[4]; // XX_\0
            snprintf(hex, sizeof(hex), "%02x ", xc->match_fingerprint[i] & 0xff);
            strlcat(buff, hex, sizeof(buff));
        }
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("insecure_end_chain: expected %s\n", buff);
        buff[0] = 0;
        for (size_t i = 0; i < sizeof(res); i++)
        {
            char hex[4]; // XX_\0
            snprintf(hex, sizeof(hex), "%02x ", res[i] & 0xff);
            strlcat(buff, hex, sizeof(buff));
        }
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("insecure_end_chain: received %s\n", buff);
#endif
        return BR_ERR_X509_NOT_TRUSTED;
    }

    // Handle self-signer certificate acceptance
    char res_issuer[32];
    char res_subject[32];
    br_sha256_out(&xc->sha256_issuer, res_issuer);
    br_sha256_out(&xc->sha256_subject, res_subject);
    if (xc->allow_self_signed && memcmp(res_subject, res_issuer, sizeof(res_issuer)))
    {
        MB_ESP8266_SSLCLIENT_DEBUG_PRINTF("insecure_end_chain: Didn't get self-signed cert\n");
        return BR_ERR_X509_NOT_TRUSTED;
    }

    // Default (no validation at all) or no errors in prior checks = success.
    return 0;
}

// Return the public key from the validator (set by x509_minimal)
static const __attribute__((used)) br_x509_pkey *mb_insecure_get_pkey(const br_x509_class *const *ctx, unsigned *usages)
{
    const mb_br_x509_insecure_context *xc = (const mb_br_x509_insecure_context *)ctx;
    if (usages != NULL)
    {
        *usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN; // I said we were insecure!
    }
    return &xc->ctx.pkey;
}

// Install hashes into the SSL engine
static void __attribute__((used)) mb_br_ssl_client_install_hashes(br_ssl_engine_context *eng)
{
    br_ssl_engine_set_hash(eng, br_md5_ID, &br_md5_vtable);
    br_ssl_engine_set_hash(eng, br_sha1_ID, &br_sha1_vtable);
    br_ssl_engine_set_hash(eng, br_sha224_ID, &br_sha224_vtable);
    br_ssl_engine_set_hash(eng, br_sha256_ID, &br_sha256_vtable);
    br_ssl_engine_set_hash(eng, br_sha384_ID, &br_sha384_vtable);
    br_ssl_engine_set_hash(eng, br_sha512_ID, &br_sha512_vtable);
}

// Default initializion for our SSL clients
static void __attribute__((used)) mb_br_ssl_client_base_init(br_ssl_client_context *cc, const uint16_t *cipher_list, int cipher_cnt)
{
    uint16_t suites[cipher_cnt];
    memcpy_P(suites, cipher_list, cipher_cnt * sizeof(cipher_list[0]));
    br_ssl_client_zero(cc);
    br_ssl_engine_add_flags(&cc->eng, BR_OPT_NO_RENEGOTIATION); // forbid SSL renegotiation, as we free the Private Key after handshake
    br_ssl_engine_set_versions(&cc->eng, BR_TLS10, BR_TLS12);
    br_ssl_engine_set_suites(&cc->eng, suites, (sizeof suites) / (sizeof suites[0]));
    br_ssl_client_set_default_rsapub(cc);
    br_ssl_engine_set_default_rsavrfy(&cc->eng);
#ifndef BEARSSL_SSL_BASIC
    br_ssl_engine_set_default_ecdsa(&cc->eng);
#endif
    mb_br_ssl_client_install_hashes(&cc->eng);
    br_ssl_engine_set_prf10(&cc->eng, &br_tls10_prf);
    br_ssl_engine_set_prf_sha256(&cc->eng, &br_tls12_sha256_prf);
    br_ssl_engine_set_prf_sha384(&cc->eng, &br_tls12_sha384_prf);
    br_ssl_engine_set_default_aes_cbc(&cc->eng);
#ifndef BEARSSL_SSL_BASIC
    br_ssl_engine_set_default_aes_gcm(&cc->eng);
    br_ssl_engine_set_default_aes_ccm(&cc->eng);
    br_ssl_engine_set_default_des_cbc(&cc->eng);
    br_ssl_engine_set_default_chapol(&cc->eng);
#endif
}


#endif // ESP8266

#endif // MB_BEARSSL_H_