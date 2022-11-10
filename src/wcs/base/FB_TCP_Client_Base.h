/*
 * TCP Client Base class, version 1.0.8
 *
 * Created November 10, 2022
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

#ifndef FB_TCP_CLIENT_BASE_H
#define FB_TCP_CLIENT_BASE_H

#include <Arduino.h>
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
        char *buf = (char *)mbfs->newP(64);
        sprintf(buf, (const char *)MBSTRING_FLASH_MCR("%d"), data);
        int ret = send(buf);
        mbfs->delP(&buf);
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
        char *buf = (char *)mbfs->newP(64);
        sprintf(buf, (const char *)MBSTRING_FLASH_MCR("%d\r\n"), data);
        int ret = send(buf);
        mbfs->delP(&buf);
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

    int readLine(char *buf, int bufLen)
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int res = -1;
        char c = 0;
        int idx = 0;
        if (!client)
            return idx;
        while (client->available() && idx <= bufLen)
        {
            if (!client)
                break;

            if (mbfs)
                mbfs->feed();

            res = client->read();
            if (res > -1)
            {
                c = (char)res;
                strcat_c(buf, c);
                idx++;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readLine(MB_String &buf)
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int res = -1;
        char c = 0;
        int idx = 0;
        if (!client)
            return idx;
        while (client->available())
        {
            if (!client)
                break;

            if (mbfs)
                mbfs->feed();

            res = client->read();
            if (res > -1)
            {
                c = (char)res;
                buf += c;
                idx++;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readChunkedData(char *out, int &chunkState, int &chunkedSize, int &dataLen, int bufLen)
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        char *temp = nullptr;
        char *buf = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            buf = (char *)mbfs->newP(bufLen);
            int readLen = readLine(buf, bufLen);
            if (readLen)
            {
                p1 = strpos(buf, (const char *)MBSTRING_FLASH_MCR(";"), 0);
                if (p1 == -1)
                {
                    p1 = strpos(buf, (const char *)MBSTRING_FLASH_MCR("\r\n"), 0);
                }

                if (p1 != -1)
                {
                    temp = (char *)mbfs->newP(p1 + 1);
                    memcpy(temp, buf, p1);
                    chunkedSize = hex2int(temp);
                    mbfs->delP(&temp);
                }

                // last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;

            mbfs->delP(&buf);
        }
        else
        {

            if (chunkedSize > -1)
            {
                buf = (char *)mbfs->newP(bufLen);
                int readLen = readLine(buf, bufLen);

                if (readLen > 0)
                {
                    // chunk may contain trailing
                    if (dataLen + readLen - 2 < chunkedSize)
                    {
                        dataLen += readLen;
                        memcpy(out, buf, readLen);
                        olen = readLen;
                    }
                    else
                    {
                        if (chunkedSize - dataLen > 0)
                            memcpy(out, buf, chunkedSize - dataLen);
                        dataLen = chunkedSize;
                        chunkState = 0;
                        olen = readLen;
                    }
                }
                else
                {
                    olen = -1;
                }

                mbfs->delP(&buf);
            }
        }

        return olen;
    }

    bool sendBase64(size_t bufSize, uint8_t *data, size_t len, bool flashMem)
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        bool ret = false;
        const unsigned char *end, *in;

        end = data + len;
        in = data;

        size_t chunkSize = bufSize;
        size_t byteAdded = 0;
        size_t byteSent = 0;

        unsigned char *buf = (unsigned char *)mbfs->newP(chunkSize);
        memset(buf, 0, chunkSize);

        unsigned char *temp = (unsigned char *)mbfs->newP(3);

        while (end - in >= 3)
        {

#if defined(ESP8266)
            delay(0);
#endif

            memset(temp, 0, 3);
            if (flashMem)
                memcpy_P(temp, in, 3);
            else
                memcpy(temp, in, 3);

            buf[byteAdded++] = fb_esp_base64_table[temp[0] >> 2];
            buf[byteAdded++] = fb_esp_base64_table[((temp[0] & 0x03) << 4) | (temp[1] >> 4)];
            buf[byteAdded++] = fb_esp_base64_table[((temp[1] & 0x0f) << 2) | (temp[2] >> 6)];
            buf[byteAdded++] = fb_esp_base64_table[temp[2] & 0x3f];

            if (byteAdded >= chunkSize - 4)
            {
                byteSent += byteAdded;

                size_t sz = strlen((const char *)buf);

                if (client->write((uint8_t *)buf, sz) != sz)
                    goto ex;
                memset(buf, 0, chunkSize);
                byteAdded = 0;
            }

            in += 3;
        }

        if (byteAdded > 0)
        {
            size_t sz = strlen((const char *)buf);
            if (client->write((uint8_t *)buf, sz) != sz)
                goto ex;
        }

        if (end - in)
        {
            memset(buf, 0, chunkSize);
            byteAdded = 0;
            memset(temp, 0, 3);
            if (flashMem)
            {
                if (end - in == 1)
                    memcpy_P(temp, in, 1);
                else
                    memcpy_P(temp, in, 2);
            }
            else
            {
                if (end - in == 1)
                    memcpy(temp, in, 1);
                else
                    memcpy(temp, in, 2);
            }

            buf[byteAdded++] = fb_esp_base64_table[temp[0] >> 2];
            if (end - in == 1)
            {
                buf[byteAdded++] = fb_esp_base64_table[(temp[0] & 0x03) << 4];
                buf[byteAdded++] = '=';
            }
            else
            {
                buf[byteAdded++] = fb_esp_base64_table[((temp[0] & 0x03) << 4) | (temp[1] >> 4)];
                buf[byteAdded++] = fb_esp_base64_table[(temp[1] & 0x0f) << 2];
            }
            buf[byteAdded++] = '=';
            size_t sz = strlen((const char *)buf);
            if (client->write((uint8_t *)buf, sz) != sz)
                goto ex;
            memset(buf, 0, chunkSize);
        }

        ret = true;
    ex:

        mbfs->delP(&temp);
        mbfs->delP(&buf);
        return ret;
    }

    int readChunkedData(MB_String &out, int &chunkState, int &chunkedSize, int &dataLen)
    {
        if (!client)
            return setError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        char *temp = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            MB_String s;
            int readLen = readLine(s);
            if (readLen)
            {
                p1 = strpos(s.c_str(), (const char *)MBSTRING_FLASH_MCR(";"), 0);
                if (p1 == -1)
                {
                    p1 = strpos(s.c_str(), (const char *)MBSTRING_FLASH_MCR("\r\n"), 0);
                }

                if (p1 != -1)
                {
                    temp = (char *)mbfs->newP(p1 + 1);
                    memcpy(temp, s.c_str(), p1);
                    chunkedSize = hex2int(temp);
                    mbfs->delP(&temp);
                }

                // last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;
        }
        else
        {

            if (chunkedSize > -1)
            {
                MB_String s;
                int readLen = readLine(s);

                if (readLen > 0)
                {
                    // chunk may contain trailing
                    if (dataLen + readLen - 2 < chunkedSize)
                    {
                        dataLen += readLen;
                        out += s;
                        olen = readLen;
                    }
                    else
                    {
                        if (chunkedSize - dataLen > 0)
                            out += s;
                        dataLen = chunkedSize;
                        chunkState = 0;
                        olen = readLen;
                    }
                }
                else
                {
                    olen = -1;
                }
            }
        }

        return olen;
    }

    virtual void flush()
    {
        if (!client)
            return;

        while (client->available() > 0)
            client->read();
    }

    fb_cert_type getCertType() { return certType; }

private:
    void strcat_c(char *str, char c)
    {
        for (; *str; str++)
            ;
        *str++ = c;
        *str++ = 0;
    }

    int strpos(const char *haystack, const char *needle, int offset)
    {
        if (!haystack || !needle)
            return -1;

        int hlen = strlen(haystack);
        int nlen = strlen(needle);

        if (hlen == 0 || nlen == 0)
            return -1;

        int hidx = offset, nidx = 0;
        while ((*(haystack + hidx) != '\0') && (*(needle + nidx) != '\0') && hidx < hlen)
        {
            if (*(needle + nidx) != *(haystack + hidx))
            {
                hidx++;
                nidx = 0;
            }
            else
            {
                nidx++;
                hidx++;
                if (nidx == nlen)
                    return hidx - nidx;
            }
        }

        return -1;
    }

    uint32_t hex2int(const char *hex)
    {
        uint32_t val = 0;
        while (*hex)
        {
            // get current character then increment
            uint8_t byte = *hex++;
            // transform hex character to the 4bit equivalent number, using the ascii table indexes
            if (byte >= '0' && byte <= '9')
                byte = byte - '0';
            else if (byte >= 'a' && byte <= 'f')
                byte = byte - 'a' + 10;
            else if (byte >= 'A' && byte <= 'F')
                byte = byte - 'A' + 10;
            // shift 4 to make space for new digit, and add the 4 bits of the new digit
            val = (val << 4) | (byte & 0xF);
        }
        return val;
    }

    void setConfig(FirebaseConfig *config)
    {
        this->config = config;
    }

    int tcpTimeout()
    {
        return timeoutMs;
    }

    void setMBFS(MB_FS *mbfs) { this->mbfs = mbfs; }

    void setSPIEthernet(SPI_ETH_Module *eth) { this->eth = eth; }

    fb_cert_type certType = fb_cert_type_undefined;

protected:
    MB_String host;
    uint16_t port = 0;
    MB_FS *mbfs = nullptr;
    Client *client = nullptr;
    bool reserved = false;
    unsigned long dataStart = 0;
    unsigned long dataTime = 0;

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
