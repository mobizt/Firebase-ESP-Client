/*
 * ESP32 SSL Client v2.0.2
 *
 * Created November 8, 2022
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

/* Provide SSL/TLS functions to ESP32 with Arduino IDE
 *
 * Adapted from the ssl_client1 example of mbedtls.
 *
 * Original Copyright (C) 2006-2015, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) 2017 Evandro Luis Copercini, Apache 2.0 License.
 */

#ifndef ESP32_SSL_Client_CPP
#define ESP32_SSL_Client_CPP

#ifdef ESP32

#include <Arduino.h>
#include <mbedtls/sha256.h>
#include <mbedtls/oid.h>
#include "ESP32_SSL_Client.h"

#if !defined(MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED) && !defined(MBEDTLS_KEY_EXCHANGE_SOME_PSK_ENABLED)
#error "Please configure IDF framework to include mbedTLS -> Enable pre-shared-key ciphersuites and activate at least one cipher"
#endif

const char *custom_str = "esp32-tls";

static int _esp32_ssl_handle_error(int err, const char *file, int line)
{
    if (err == -30848)
    {
        return err;
    }
#ifdef MBEDTLS_ERROR_C
    char error_buf[100];
    mbedtls_strerror(err, error_buf, 100);
    log_e("[%s():%d]: (%d) %s", file, line, err, error_buf);
#else
    log_e("[%s():%d]: code %d", file, line, err);
#endif
    return err;
}

#define esp32_ssl_handle_error(e) _esp32_ssl_handle_error(e, __FUNCTION__, __LINE__)

void ESP32_SSL_Client::ssl_init(ssl_ctx *ssl)
{
    mbedtls_ssl_init(&ssl->ssl_ctx);
    mbedtls_ssl_config_init(&ssl->ssl_conf);
    mbedtls_ctr_drbg_init(&ssl->drbg_ctx);
}

/**
 * \brief          Callback type: send data on the network.
 *
 * \note           That callback may be either blocking or non-blocking.
 *
 * \param ctx      Context for the send callback (typically a file descriptor)
 * \param buf      Buffer holding the data to send
 * \param len      Length of the data to send
 *
 * \return         The callback must return the number of bytes sent if any,
 *                 or a non-zero error code.
 *                 If performing non-blocking I/O, \c MBEDTLS_ERR_SSL_WANT_WRITE
 *                 must be returned when the operation would block.
 *
 * \note           The callback is allowed to send fewer bytes than requested.
 *                 It must always return the number of bytes actually sent.
 */
static int esp_mail_esp32_basic_client_send(void *ctx, const unsigned char *buf, size_t len)
{
    Client *basic_client = (Client *)ctx;

    if (!basic_client)
        return -1;

    int res = basic_client->write(buf, len);

    return res;
}

/**
 * \brief          Callback type: receive data from the network, with timeout
 *
 * \note           That callback must block until data is received, or the
 *                 timeout delay expires, or the operation is interrupted by a
 *                 signal.
 *
 * \param ctx      Context for the receive callback (typically a file descriptor)
 * \param buf      Buffer to write the received data to
 * \param len      Length of the receive buffer
 * \param timeout  Maximum number of milliseconds to wait for data
 *                 0 means no timeout (potentially waiting forever)
 *
 * \return         The callback must return the number of bytes received,
 *                 or a non-zero error code:
 *                 \c MBEDTLS_ERR_SSL_TIMEOUT if the operation timed out,
 *                 \c MBEDTLS_ERR_SSL_WANT_READ if interrupted by a signal.
 *
 * \note           The callback may receive fewer bytes than the length of the
 *                 buffer. It must always return the number of bytes actually
 *                 received and written to the buffer.
 */
static int esp_mail_esp32_basic_client_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    Client *basic_client = (Client *)ctx;

    if (!basic_client)
        return -1;

    int available = basic_client->available();

    int res = 0;

    unsigned long to = millis();
    while (millis() - to < timeout && available < len)
    {
        delay(0);
        available = basic_client->available();
        if (millis() - to >= timeout)
            res = MBEDTLS_ERR_SSL_TIMEOUT;
    };

    res = basic_client->read(buf, len);

    if (!res)
        return MBEDTLS_ERR_SSL_WANT_READ;

    return res;
}

int ESP32_SSL_Client::connect_ssl(ssl_ctx *ssl, const char *host, const char *rootCABuff, const char *cli_cert, const char *cli_key, const char *pskIdent, const char *psKey, bool insecure)
{

    if (!ssl->basic_client)
        return -1;

    if (rootCABuff == NULL && pskIdent == NULL && psKey == NULL && !insecure)
    {
        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_14);
        return -1;
    }

    char buf[512];
    int ret, flags;

    ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_2);

    log_v("Seeding the random number generator");
    mbedtls_entropy_init(&ssl->entropy_ctx);

    ret = mbedtls_ctr_drbg_seed(&ssl->drbg_ctx, mbedtls_entropy_func, &ssl->entropy_ctx, (const unsigned char *)custom_str, strlen(custom_str));
    if (ret < 0)
    {
        ssl_client_send_mbedtls_error_cb(ssl, ret);
        return esp32_ssl_handle_error(ret);
    }

    ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_3);

    log_v("Setting up the SSL/TLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&ssl->ssl_conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ssl_client_send_mbedtls_error_cb(ssl, ret);
        return esp32_ssl_handle_error(ret);
    }

    // MBEDTLS_SSL_VERIFY_REQUIRED if a CA certificate is defined and
    // MBEDTLS_SSL_VERIFY_NONE if not.

    if (insecure)
    {
        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_15);

        mbedtls_ssl_conf_authmode(&ssl->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
        log_i("WARNING: Skipping SSL Verification. INSECURE!");
    }
    else if (rootCABuff != NULL)
    {
        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_4);
        log_v("Loading CA cert");

        mbedtls_x509_crt_init(&ssl->ca_cert);
        mbedtls_ssl_conf_authmode(&ssl->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        ret = mbedtls_x509_crt_parse(&ssl->ca_cert, (const unsigned char *)rootCABuff, strlen(rootCABuff) + 1);
        mbedtls_ssl_conf_ca_chain(&ssl->ssl_conf, &ssl->ca_cert, NULL);
        // mbedtls_ssl_conf_verify(&ssl->ssl_ctx, my_verify, NULL );
        if (ret < 0)
        {
            ssl_client_send_mbedtls_error_cb(ssl, ret);
            // free the ca_cert in the case parse failed, otherwise, the old ca_cert still in the heap memory, that lead to "out of memory" crash.
            mbedtls_x509_crt_free(&ssl->ca_cert);
            return esp32_ssl_handle_error(ret);
        }
    }
    else if (pskIdent != NULL && psKey != NULL)
    {
        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_5);
        log_v("Setting up PSK");
        // convert PSK from hex to binary
        if ((strlen(psKey) & 1) != 0 || strlen(psKey) > 2 * MBEDTLS_PSK_MAX_LEN)
        {
            ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_6);
            log_e("pre-shared key not valid hex or too long");
            return -1;
        }

        unsigned char psk[MBEDTLS_PSK_MAX_LEN];
        size_t psk_len = strlen(psKey) / 2;
        for (int j = 0; j < strlen(psKey); j += 2)
        {
            char c = psKey[j];
            if (c >= '0' && c <= '9')
                c -= '0';
            else if (c >= 'A' && c <= 'F')
                c -= 'A' - 10;
            else if (c >= 'a' && c <= 'f')
                c -= 'a' - 10;
            else
                return -1;
            psk[j / 2] = c << 4;
            c = psKey[j + 1];
            if (c >= '0' && c <= '9')
                c -= '0';
            else if (c >= 'A' && c <= 'F')
                c -= 'A' - 10;
            else if (c >= 'a' && c <= 'f')
                c -= 'a' - 10;
            else
                return -1;
            psk[j / 2] |= c;
        }

        // set mbedtls config
        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_7);

        ret = mbedtls_ssl_conf_psk(&ssl->ssl_conf, psk, psk_len,
                                   (const unsigned char *)pskIdent, strlen(pskIdent));
        if (ret != 0)
        {

            ssl_client_send_mbedtls_error_cb(ssl, ret);

            log_e("mbedtls_ssl_conf_psk returned %d", ret);
            return esp32_ssl_handle_error(ret);
        }
    }
    else
    {
        return -1;
    }

    if (!insecure && cli_cert != NULL && cli_key != NULL)
    {

        mbedtls_x509_crt_init(&ssl->client_cert);
        mbedtls_pk_init(&ssl->client_key);

        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_8);

        log_v("Loading CRT cert");

        ret = mbedtls_x509_crt_parse(&ssl->client_cert, (const unsigned char *)cli_cert, strlen(cli_cert) + 1);
        if (ret < 0)
        {
            ssl_client_send_mbedtls_error_cb(ssl, ret);
            // free the client_cert in the case parse failed, otherwise, the old client_cert still in the heap memory, that lead to "out of memory" crash.
            mbedtls_x509_crt_free(&ssl->client_cert);
            return esp32_ssl_handle_error(ret);
        }

        ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_9);

        log_v("Loading private key");
        ret = mbedtls_pk_parse_key(&ssl->client_key, (const unsigned char *)cli_key, strlen(cli_key) + 1, NULL, 0);

        if (ret != 0)
        {
            ssl_client_send_mbedtls_error_cb(ssl, ret);
            return esp32_ssl_handle_error(ret);
        }

        mbedtls_ssl_conf_own_cert(&ssl->ssl_conf, &ssl->client_cert, &ssl->client_key);
    }

    ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_10);

    log_v("Setting hostname for TLS session...");

    // Hostname set here should match CN in server certificate
    if ((ret = mbedtls_ssl_set_hostname(&ssl->ssl_ctx, host)) != 0)
    {
        ssl_client_send_mbedtls_error_cb(ssl, ret);
        return esp32_ssl_handle_error(ret);
    }

    mbedtls_ssl_conf_rng(&ssl->ssl_conf, mbedtls_ctr_drbg_random, &ssl->drbg_ctx);

    if ((ret = mbedtls_ssl_setup(&ssl->ssl_ctx, &ssl->ssl_conf)) != 0)
    {
        ssl_client_send_mbedtls_error_cb(ssl, ret);
        return esp32_ssl_handle_error(ret);
    }

    mbedtls_ssl_set_bio(&ssl->ssl_ctx, ssl->basic_client, esp_mail_esp32_basic_client_send, NULL, esp_mail_esp32_basic_client_recv_timeout);

    ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_11);

    log_v("Performing the SSL/TLS handshake...");
    unsigned long handshake_start_time = millis();
    while ((ret = mbedtls_ssl_handshake(&ssl->ssl_ctx)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            ssl_client_send_mbedtls_error_cb(ssl, ret);
            return esp32_ssl_handle_error(ret);
        }
        if ((millis() - handshake_start_time) > ssl->handshake_timeout)
            return -1;

        vTaskDelay(2 / portTICK_PERIOD_MS);
    }

    if (cli_cert != NULL && cli_key != NULL)
    {
        log_d("Protocol is %s Ciphersuite is %s", mbedtls_ssl_get_version(&ssl->ssl_ctx), mbedtls_ssl_get_ciphersuite(&ssl->ssl_ctx));
        if ((ret = mbedtls_ssl_get_record_expansion(&ssl->ssl_ctx)) >= 0)
        {
            log_d("Record expansion is %d", ret);
        }
        else
        {
            log_w("Record expansion is unknown (compression)");
        }
    }
    ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_12);

    log_v("Verifying peer X.509 certificate...");

    if ((flags = mbedtls_ssl_get_verify_result(&ssl->ssl_ctx)) != 0)
    {
        memset(buf, 0, sizeof(buf));
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
        log_e("Failed to verify peer certificate! verification info: %s", buf);
        stop_tcp_connection(ssl, rootCABuff, cli_cert, cli_key); // It's not safe continue.
        return esp32_ssl_handle_error(ret);
    }
    else
    {
        log_v("Certificate verified.");
    }

    if (rootCABuff != NULL)
    {
        mbedtls_x509_crt_free(&ssl->ca_cert);
    }

    if (cli_cert != NULL)
    {
        mbedtls_x509_crt_free(&ssl->client_cert);
    }

    if (cli_key != NULL)
    {
        mbedtls_pk_free(&ssl->client_key);
    }

    log_v("Free internal heap after TLS %u", ESP.getFreeHeap());

    return 1;
}

void ESP32_SSL_Client::stop_tcp_connection(ssl_ctx *ssl, const char *rootCABuff, const char *cli_cert, const char *cli_key)
{
    ssl_client_debug_pgm_send_cb(ssl, mb_ssl_client_str_13);

    log_v("Cleaning SSL connection.");

    mbedtls_ssl_free(&ssl->ssl_ctx);
    mbedtls_ssl_config_free(&ssl->ssl_conf);
    mbedtls_ctr_drbg_free(&ssl->drbg_ctx);
    mbedtls_entropy_free(&ssl->entropy_ctx);
}

int ESP32_SSL_Client::data_to_read(ssl_ctx *ssl)
{
    int ret, res;
    ret = mbedtls_ssl_read(&ssl->ssl_ctx, NULL, 0);
    // log_e("RET: %i",ret);   //for low level debug
    res = mbedtls_ssl_get_bytes_avail(&ssl->ssl_ctx);
    // log_e("RES: %i",res);    //for low level debug
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret < 0)
    {
        ssl_client_send_mbedtls_error_cb(ssl, ret);
        return esp32_ssl_handle_error(ret);
    }

    return res;
}

int ESP32_SSL_Client::send_ssl_data(ssl_ctx *ssl, const uint8_t *data, size_t len)
{
    log_v("Writing request with %d bytes...", len); // for low level debug
    int ret = -1;

    while ((ret = mbedtls_ssl_write(&ssl->ssl_ctx, data, len)) <= 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret < 0)
        {
            log_v("Handling error %d", ret); // for low level debug
            return esp32_ssl_handle_error(ret);
        }
        // wait for space to become available
        vTaskDelay(2);
    }

    return ret;
}

int ESP32_SSL_Client::get_ssl_receive(ssl_ctx *ssl, uint8_t *data, int length)
{

    // log_d( "Reading HTTP response...");   //for low level debug
    int ret = -1;

    ret = mbedtls_ssl_read(&ssl->ssl_ctx, data, length);

    // log_v( "%d bytes read", ret);   //for low level debug
    return ret;
}

bool ESP32_SSL_Client::parseHexNibble(char pb, uint8_t *res)
{
    if (pb >= '0' && pb <= '9')
    {
        *res = (uint8_t)(pb - '0');
        return true;
    }
    else if (pb >= 'a' && pb <= 'f')
    {
        *res = (uint8_t)(pb - 'a' + 10);
        return true;
    }
    else if (pb >= 'A' && pb <= 'F')
    {
        *res = (uint8_t)(pb - 'A' + 10);
        return true;
    }
    return false;
}

// Compare a name from certificate and domain name, return true if they match
bool ESP32_SSL_Client::matchName(const std::string &name, const std::string &domainName)
{
    size_t wildcardPos = name.find('*');
    if (wildcardPos == std::string::npos)
    {
        // Not a wildcard, expect an exact match
        return name == domainName;
    }

    size_t firstDotPos = name.find('.');
    if (wildcardPos > firstDotPos)
    {
        // Wildcard is not part of leftmost component of domain name
        // Do not attempt to match (rfc6125 6.4.3.1)
        return false;
    }
    if (wildcardPos != 0 || firstDotPos != 1)
    {
        // Matching of wildcards such as baz*.example.com and b*z.example.com
        // is optional. Maybe implement this in the future?
        return false;
    }
    size_t domainNameFirstDotPos = domainName.find('.');
    if (domainNameFirstDotPos == std::string::npos)
    {
        return false;
    }
    return domainName.substr(domainNameFirstDotPos) == name.substr(firstDotPos);
}

// Verifies certificate provided by the peer to match specified SHA256 fingerprint
bool ESP32_SSL_Client::verify_ssl_fingerprint(ssl_ctx *ssl, const char *fp, const char *domain_name)
{
    // Convert hex string to byte array
    uint8_t fingerprint_local[32];
    int len = strlen(fp);
    int pos = 0;
    for (size_t i = 0; i < sizeof(fingerprint_local); ++i)
    {
        while (pos < len && ((fp[pos] == ' ') || (fp[pos] == ':')))
        {
            ++pos;
        }
        if (pos > len - 2)
        {
            log_d("pos:%d len:%d fingerprint too short", pos, len);
            return false;
        }
        uint8_t high, low;
        if (!parseHexNibble(fp[pos], &high) || !parseHexNibble(fp[pos + 1], &low))
        {
            log_d("pos:%d len:%d invalid hex sequence: %c%c", pos, len, fp[pos], fp[pos + 1]);
            return false;
        }
        pos += 2;
        fingerprint_local[i] = low | (high << 4);
    }

    // Get certificate provided by the peer
    const mbedtls_x509_crt *crt = mbedtls_ssl_get_peer_cert(&ssl->ssl_ctx);

    if (!crt)
    {
        log_d("could not fetch peer certificate");
        return false;
    }

    // Calculate certificate's SHA256 fingerprint
    uint8_t fingerprint_remote[32];
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, false);
    mbedtls_sha256_update(&sha256_ctx, crt->raw.p, crt->raw.len);
    mbedtls_sha256_finish(&sha256_ctx, fingerprint_remote);

    // Check if fingerprints match
    if (memcmp(fingerprint_local, fingerprint_remote, 32))
    {
        log_d("fingerprint doesn't match");
        return false;
    }

    // Additionally check if certificate has domain name if provided
    if (domain_name)
        return verify_ssl_dn(ssl, domain_name);
    else
        return true;
}

// Checks if peer certificate has specified domain in CN or SANs
bool ESP32_SSL_Client::verify_ssl_dn(ssl_ctx *ssl, const char *domain_name)
{
    log_d("domain name: '%s'", (domain_name) ? domain_name : "(null)");
    std::string domain_name_str(domain_name);
    std::transform(domain_name_str.begin(), domain_name_str.end(), domain_name_str.begin(), ::tolower);

    // Get certificate provided by the peer
    const mbedtls_x509_crt *crt = mbedtls_ssl_get_peer_cert(&ssl->ssl_ctx);

    // Check for domain name in SANs
    const mbedtls_x509_sequence *san = &crt->subject_alt_names;
    while (san != nullptr)
    {
        std::string san_str((const char *)san->buf.p, san->buf.len);
        std::transform(san_str.begin(), san_str.end(), san_str.begin(), ::tolower);

        if (matchName(san_str, domain_name_str))
            return true;

        log_d("SAN '%s': no match", san_str.c_str());

        // Fetch next SAN
        san = san->next;
    }

    // Check for domain name in CN
    const mbedtls_asn1_named_data *common_name = &crt->subject;
    while (common_name != nullptr)
    {
        // While iterating through DN objects, check for CN object
        if (!MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &common_name->oid))
        {
            std::string common_name_str((const char *)common_name->val.p, common_name->val.len);

            if (matchName(common_name_str, domain_name_str))
                return true;

            log_d("CN '%s': not match", common_name_str.c_str());
        }

        // Fetch next DN object
        common_name = common_name->next;
    }

    return false;
}

void ESP32_SSL_Client::ssl_client_send_mbedtls_error_cb(ssl_ctx *ssl, int errNo)
{
    if (!ssl->_debugCallback)
        return;

    char *error_buf = new char[100];
    mbedtls_strerror(errNo, error_buf, 100);
    (*ssl->_debugCallback)(error_buf, true);
    delete[] error_buf;
    error_buf = nullptr;
}

void ESP32_SSL_Client::ssl_client_debug_pgm_send_cb(ssl_ctx *ssl, PGM_P info)
{
    if (!ssl->_debugCallback)
        return;

    size_t dbgInfoLen = strlen_P(info) + 1;
    char *dbgInfo = new char[dbgInfoLen];
    memset(dbgInfo, 0, dbgInfoLen);
    strcpy_P(dbgInfo, info);

    (*ssl->_debugCallback)(dbgInfo, true);
    delete[] dbgInfo;
    dbgInfo = nullptr;
}

#endif // ESP32

#endif // ESP32_SSL_Client_CPP