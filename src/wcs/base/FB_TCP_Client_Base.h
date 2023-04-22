#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/*
 * TCP Client Base class, version 1.0.9
 *
 * Created March 5, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#ifndef FB_TCP_CLIENT_BASE_H
#define FB_TCP_CLIENT_BASE_H

#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#include "FB_Utils.h"
#include <IPAddress.h>
#include <Client.h>

typedef enum
{
    fb_cert_type_undefined = -1,
    fb_cert_type_none = 0,
    fb_cert_type_data,
    fb_cert_type_file

} fb_cert_type;

typedef enum
{
    fb_tcp_client_type_undefined,
    fb_tcp_client_type_internal,
    fb_tcp_client_type_external

} fb_tcp_client_type;

class FB_TCP_Client_Base
{
    friend class FirebaseData;
    friend class FB_CM;
    friend class Firebase_Signer;
    friend class FirebaseESP32;
    friend class FirebaseESP8266;
    friend class FCMObject;
    friend class FB_Functions;
    friend class FB_Storage;
    friend class GG_CloudStorage;
    friend class FB_RTDB;
    friend class FB_Firestore;
    friend class FIREBASE_CLASS;
    friend class Firebase_ESP_Client;

public:
    FB_TCP_Client_Base()
    {
        certType = fb_cert_type_undefined;
    };
    virtual ~FB_TCP_Client_Base(){};

    virtual void ethDNSWorkAround(){};

    virtual bool networkReady() { return false; }

    virtual void networkReconnect(){};

    virtual void disconnect(){};

    virtual fb_tcp_client_type type() { return fb_tcp_client_type_undefined; }

    virtual bool isInitialized() { return false; }

    virtual int hostByName(const char *name, IPAddress &ip) { return 0; }

    int virtual setError(int code)
    {
        if (!response_code)
            return -1000;

        *response_code = code;
        return *response_code;
    }

    virtual bool begin(const char *host, uint16_t port, int *response_code)
    {

        this->host = host;
        this->port = port;
        this->response_code = response_code;

        return true;
    }

    virtual bool connect()
    {
        if (!client)
            return false;

        if (connected())
            return true;

        if (!client->connect(host.c_str(), port))
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        client->setTimeout(timeoutMs);

        return connected();
    }

    virtual void stop()
    {
        if (!client)
            return;

        return client->stop();
    };

    virtual bool connected()
    {
        if (client)
            return client->connected();

        return false;
    }

    virtual int write(uint8_t *data, int len)
    {       
        if (!data || !client)
            return setError(FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED);

        if (len == 0)
            return setError(FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED);

        if (!networkReady())
            return setError(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);

        // call base or derived connect.
        if (!connected() && !connect())
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int res = client->write(data, len);

        if (res != len)
            return setError(FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED);

        setError(FIREBASE_ERROR_HTTP_CODE_OK);

        return len;
    }

    virtual int send(const char *data, int len = 0)
    {
        if (len == 0)
            len = strlen(data);
        return write((uint8_t *)data, len);
    }

    virtual int print(const char *data)
    {
        return send(data);
    }

    virtual int print(int data)
    {
        char *buf = MemoryHelper::createBuffer<char *>(mbfs, 64);
        sprintf(buf, (const char *)MBSTRING_FLASH_MCR("%d"), data);
        int ret = send(buf);
        MemoryHelper::freeBuffer(mbfs, buf);
        return ret;
    }

    virtual int println(const char *data)
    {
        int len = send(data);
        if (len < 0)
            return len;
        int sz = send((const char *)MBSTRING_FLASH_MCR("\r\n"));
        if (sz < 0)
            return sz;
        return len + sz;
    }

    virtual int println(int data)
    {
        char *buf = MemoryHelper::createBuffer<char *>(mbfs, 64);
        sprintf(buf, (const char *)MBSTRING_FLASH_MCR("%d\r\n"), data);
        int ret = send(buf);
        MemoryHelper::freeBuffer(mbfs, buf);
        return ret;
    }

    virtual int available()
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        return client->available();
    }

    virtual int read()
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int r = client->read();

        if (r < 0)
            return setError(FIREBASE_ERROR_TCP_RESPONSE_READ_FAILED);

        return r;
    }

    virtual int readBytes(uint8_t *buf, int len)
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int r = client->readBytes(buf, len);

        if (r != len)
            return setError(FIREBASE_ERROR_TCP_RESPONSE_READ_FAILED);

        setError(FIREBASE_ERROR_HTTP_CODE_OK);

        return r;
    }

    virtual int readBytes(char *buf, int len) { return readBytes((uint8_t *)buf, len); }

    void baseSetCertType(fb_cert_type type) { certType = type; }

    void baseSetTimeout(uint32_t timeoutSec) { timeoutMs = timeoutSec * 1000; }

    virtual void flush()
    {
        if (!client)
            return;

        while (client->available() > 0)
            client->read();
    }

    fb_cert_type getCertType() { return certType; }

private:
    void setConfig(FirebaseConfig *config, MB_FS *mbfs)
    {
        this->config = config;
        this->mbfs = mbfs;
    }

    int tcpTimeout()
    {
        return timeoutMs;
    }

    void setSPIEthernet(SPI_ETH_Module *eth) { this->eth = eth; }

    fb_cert_type certType = fb_cert_type_undefined;

protected:
    MB_String host;
    uint16_t port = 0;
    Client *client = nullptr;
    bool reserved = false;
    unsigned long dataStart = 0;
    unsigned long dataTime = 0;
    MB_FS *mbfs = nullptr;

    // In esp8266, this is actually Arduino base Stream (char read) timeout.
    //  This will override internally by WiFiClientSecureCtx::_connectSSL
    //  to 5000 after SSL handshake was done with success.
    int timeoutMs = 120000; // 120 sec
    bool clockReady = false;
    time_t now = 0;
    int *response_code = nullptr;
    SPI_ETH_Module *eth = NULL;
    FirebaseConfig *config = nullptr;
};

#endif
