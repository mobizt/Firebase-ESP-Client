/**
 *Modified version ssl_client.h version 1.0.2
*/

/** Provide SSL/TLS functions to ESP32 with Arduino IDE
 * by Evandro Copercini - 2017 - Apache 2.0 License
 */

#ifndef FB_SSL_CLIENT32_H
#define FB_SSL_CLIENT32_H

#ifdef ESP32
#include <mbedtls/platform.h>
#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

typedef struct fb_esp_ssl_ctx32
{
    int socket;
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config ssl_conf;

    mbedtls_ctr_drbg_context drbg_ctx;
    mbedtls_entropy_context entropy_ctx;

    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt client_cert;
    mbedtls_pk_context client_key;

    unsigned long handshake_timeout;
    bool is_entropy_ctx;
    bool ssl_freed;
} fb_esp_ssl_ctx32;

void ssl_init(fb_esp_ssl_ctx32 *ssl_client);
int start_ssl_client(fb_esp_ssl_ctx32 *ssl_client, const char *host, uint32_t port, int timeout, const char *rootCABuff, const char *cli_cert, const char *cli_key, const char *pskIdent, const char *psKey);
void stop_ssl_socket(fb_esp_ssl_ctx32 *ssl_client, const char *rootCABuff, const char *cli_cert, const char *cli_key);
int data_to_read(fb_esp_ssl_ctx32 *ssl_client);
int send_ssl_data(fb_esp_ssl_ctx32 *ssl_client, const uint8_t *data, uint16_t len);
int get_ssl_receive(fb_esp_ssl_ctx32 *ssl_client, uint8_t *data, int length);
bool verify_ssl_fingerprint(fb_esp_ssl_ctx32 *ssl_client, const char *fp, const char *domain_name);
bool verify_ssl_dn(fb_esp_ssl_ctx32 *ssl_client, const char *domain_name);


#endif /* ESP32 */

#endif /* FB_SSL_CLIENT32_H */