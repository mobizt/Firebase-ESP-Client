/**
 * Google's Firebase Util class, Utils.h version 1.1.12
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 1, 2022
 * 
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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

#ifndef FB_UTILS_H
#define FB_UTILS_H

#include <Arduino.h>
#include "common.h"
#include "addons/fastcrc/FastCRC.h"

using namespace mb_string;

class UtilsClass
{
    friend class FirebaseSession;
    friend class Firebase_ESP_Client;

public:
    long default_ts = ESP_DEFAULT_TS;
    uint16_t ntpTimeout = 20;
    callback_function_t _callback_function = nullptr;
    FirebaseConfig *config = nullptr;
    FastCRC16 CRC16;

    UtilsClass(FirebaseConfig *cfg)
    {
        config = cfg;
    };

    ~UtilsClass(){};

    int strposP(const char *buf, PGM_P beginH, int ofs)
    {
        int p = strpos(buf, pgm2Str(beginH), ofs);
        return p;
    }

    bool strcmpP(const char *buf, int ofs, PGM_P beginH)
    {
        
        if (ofs < 0)
        {
            int p = strposP(buf, beginH, 0);
            if (p == -1)
                return false;
            ofs = p;
        }
       
        char *tmp2 = (char *)newP(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcasecmp(pgm2Str(beginH), tmp2) == 0);

        delP(&tmp2);
        return ret;
    }

    char *subStr(const char *buf, PGM_P beginH, PGM_P endH, int beginPos, int endPos)
    {

        char *tmp = nullptr;
        int p1 = strposP(buf, beginH, beginPos);
        if (p1 != -1)
        {
            int p2 = -1;
            if (endPos == 0)
                p2 = strposP(buf, endH, p1 + strlen_P(beginH));

            if (p2 == -1)
                p2 = strlen(buf);

            int len = p2 - p1 - strlen_P(beginH);
            tmp = (char *)newP(len + 1);
            memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
            return tmp;
        }

        return nullptr;
    }

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

    int strpos(const char *haystack, char needle, int offset)
    {
        if (!haystack || needle == 0)
            return -1;

        int hlen = strlen(haystack);

        if (hlen == 0)
            return -1;

        int hidx = offset;
        while ((*(haystack + hidx) != '\0') && hidx < hlen)
        {
            if (needle == *(haystack + hidx))
                return hidx;
            hidx++;
        }

        return -1;
    }

    int rstrpos(const char *haystack, const char *needle, int offset /* start search from this offset to the left string */)
    {
        if (!haystack || !needle)
            return -1;

        int hlen = strlen(haystack);
        int nlen = strlen(needle);

        if (hlen == 0 || nlen == 0)
            return -1;

        int hidx = offset;

        if (hidx >= hlen || offset == -1)
            hidx = hlen - 1;

        int nidx = nlen - 1;

        while (hidx >= 0)
        {
            if (*(needle + nidx) != *(haystack + hidx))
            {
                hidx--;
                nidx = nlen - 1;
            }
            else
            {
                if (nidx == 0)
                    return hidx + nidx;
                nidx--;
                hidx--;
            }
        }

        return -1;
    }

    int rstrpos(const char *haystack, char needle, int offset /* start search from this offset to the left char */)
    {
        if (!haystack || needle == 0)
            return -1;

        int hlen = strlen(haystack);

        if (hlen == 0)
            return -1;

        int hidx = offset;

        if (hidx >= hlen || offset == -1)
            hidx = hlen - 1;

        while (hidx >= 0)
        {
            if (needle == *(haystack + hidx))
                return hidx;
            hidx--;
        }

        return -1;
    }

    void ltrim(MBSTRING &str, const MBSTRING &chars = " ")
    {
        size_t pos = str.find_first_not_of(chars);
        if (pos != MBSTRING::npos)
            str.erase(0, pos);
    }

    void rtrim(MBSTRING &str, const MBSTRING &chars = " ")
    {
        size_t pos = str.find_last_not_of(chars);
        if (pos != MBSTRING::npos)
            str.erase(pos + 1);
    }

    inline MBSTRING trim(const MBSTRING &s)
    {
        MBSTRING chars = " ";
        MBSTRING str = s;
        ltrim(str, chars);
        rtrim(str, chars);
        return str;
    }

    void delP(void *ptr)
    {
        void **p = (void **)ptr;
        if (*p)
        {
            free(*p);
            *p = 0;
        }
    }

    size_t getReservedLen(size_t len)
    {
        int blen = len + 1;

        int newlen = (blen / 4) * 4;

        if (newlen < blen)
            newlen += 4;

        return (size_t)newlen;
    }

    void *newP(size_t len)
    {
        void *p;
        size_t newLen = getReservedLen(len);
#if defined(BOARD_HAS_PSRAM) && defined(FIREBASE_USE_PSRAM)

        if ((p = (void *)ps_malloc(newLen)) == 0)
            return NULL;

#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.setExternalHeap();
#endif

        bool nn = ((p = (void *)malloc(newLen)) > 0);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.resetHeap();
#endif

        if (!nn)
            return NULL;

#endif
        memset(p, 0, newLen);
        return p;
    }

    void substr(MBSTRING &str, const char *s, int offset, size_t len)
    {
        if (!s)
            return;

        int slen = strlen(s);

        if (slen == 0)
            return;

        int last = offset + len;

        if (offset >= slen || len == 0 || last > slen)
            return;

        for (int i = offset; i < last; i++)
            str += s[i];
    }
    void splitString(const char *str, std::vector<MBSTRING> out, const char delim)
    {
        int current = 0, previous = 0;
        current = strpos(str, delim, 0);
        MBSTRING s;
        while (current != -1)
        {
            s.clear();
            substr(s, str, previous, current - previous);
            trim(s);
            if (s.length() > 0)
                out.push_back(s);

            previous = current + 1;
            current = strpos(str, delim, previous);
            delay(0);
        }

        s.clear();

        if (previous > 0 && current == -1)
            substr(s, str, previous, strlen(str) - previous);
        else
            s = str;

        trim(s);
        if (s.length() > 0)
            out.push_back(s);
        s.clear();
    }

    void getUrlInfo(const MBSTRING &url, struct fb_esp_url_info_t &info)
    {
        char *host = (char *)newP(url.length() + 5);
        char *uri = (char *)newP(url.length() + 5);
        char *auth = (char *)newP(url.length() + 5);

        int p1 = 0;
        int x = sscanf(url.c_str(), pgm2Str(fb_esp_pgm_str_441), host, uri);
        x ? p1 = 8 : x = sscanf(url.c_str(), pgm2Str(fb_esp_pgm_str_442), host, uri);
        x ? p1 = 7 : x = sscanf(url.c_str(), pgm2Str(fb_esp_pgm_str_443), host, uri);

        int p2 = 0;
        if (x > 0)
        {
            p2 = strpos(host, pgm2Str(fb_esp_pgm_str_173), 0);
            if (p2 > -1)
            {
                x = sscanf(url.c_str() + p1, pgm2Str(fb_esp_pgm_str_444), host, uri);
            }
        }

        if (strlen(uri) > 0)
        {
            p2 = strpos(uri, pgm2Str(fb_esp_pgm_str_445), 0);
            if (p2 > -1)
            {
                x = sscanf(uri + p2 + 5, pgm2Str(fb_esp_pgm_str_446), auth);
            }
        }

        info.uri = uri;
        info.host = host;
        info.auth = auth;
        delP(&uri);
        delP(&host);
        delP(&auth);
    }

    MBSTRING url_encode(const MBSTRING &s)
    {
        MBSTRING ret;
        ret.reserve(s.length() * 3 + 1);
        for (size_t i = 0, l = s.size(); i < l; i++)
        {
            char c = s[i];
            if ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
                ret += c;
            }
            else
            {
                ret += '%';
                unsigned char d1, d2;
                hexchar(c, d1, d2);
                ret += d1;
                ret += d2;
            }
        }
        ret.shrink_to_fit();
        return ret;
    }

    inline int ishex(int x)
    {
        return (x >= '0' && x <= '9') ||
               (x >= 'a' && x <= 'f') ||
               (x >= 'A' && x <= 'F');
    }

    void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 <= 9 ? '0' : 'a' - 10;
        hex2 += hex2 <= 9 ? '0' : 'a' - 10;
    }

    char from_hex(char ch)
    {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
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

    void parseRespHeader(const char *buf, struct server_response_data_t &response)
    {
        int beginPos = 0, pmax = 0, payloadPos = 0;

        char *tmp = nullptr;

        if (response.httpCode != -1)
        {
            payloadPos = beginPos;
            pmax = beginPos;
            tmp = getHeader(buf, fb_esp_pgm_str_10, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delP(&tmp);
            }
            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_8, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.contentType = tmp;
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_12, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.contentLen = atoi(tmp);
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_167, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.transferEnc = tmp;
                if (stringCompare(tmp, 0, fb_esp_pgm_str_168))
                    response.isChunkedEnc = true;
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_150, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.etag = tmp;
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_10, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_12, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {

                response.payloadLen = atoi(tmp);
                delP(&tmp);
            }

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK || response.httpCode == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY || response.httpCode == FIREBASE_ERROR_HTTP_CODE_FOUND)
            {
                if (pmax < beginPos)
                    pmax = beginPos;
                beginPos = payloadPos;
                tmp = getHeader(buf, fb_esp_pgm_str_95, fb_esp_pgm_str_21, beginPos, 0);
                if (tmp)
                {
                    response.location = tmp;
                    delP(&tmp);
                }
            }

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                response.noContent = true;
        }
    }

    int readLine(WiFiClient *stream, char *buf, int bufLen)
    {
        int res = -1;
        char c = 0;
        int idx = 0;
        if (!stream)
            return idx;
        while (stream->available() && idx <= bufLen)
        {
            if (!stream)
                break;
            res = stream->read();
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

    int readLine(WiFiClient *stream, MBSTRING &buf)
    {
        int res = -1;
        char c = 0;
        int idx = 0;
        if (!stream)
            return idx;
        while (stream->available())
        {
            if (!stream)
                break;
            res = stream->read();
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

    int readChunkedData(WiFiClient *stream, char *out, int &chunkState, int &chunkedSize, int &dataLen, int bufLen)
    {

        char *tmp = nullptr;
        char *buf = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            buf = (char *)newP(bufLen);
            int readLen = readLine(stream, buf, bufLen);
            if (readLen)
            {
                p1 = strpos(buf, pgm2Str(fb_esp_pgm_str_79), 0);
                if (p1 == -1)
                {
                    p1 = strpos(buf, pgm2Str(fb_esp_pgm_str_21), 0);
                }

                if (p1 != -1)
                {
                    tmp = (char *)newP(p1 + 1);
                    memcpy(tmp, buf, p1);
                    chunkedSize = hex2int(tmp);
                    delP(&tmp);
                }

                //last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;

            delP(&buf);
        }
        else
        {

            if (chunkedSize > -1)
            {
                buf = (char *)newP(bufLen);
                int readLen = readLine(stream, buf, bufLen);

                if (readLen > 0)
                {
                    //chunk may contain trailing
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

                delP(&buf);
            }
        }

        return olen;
    }

    int readChunkedData(WiFiClient *stream, MBSTRING &out, int &chunkState, int &chunkedSize, int &dataLen)
    {

        char *tmp = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            MBSTRING s;
            int readLen = readLine(stream, s);
            if (readLen)
            {
                p1 = strpos(s.c_str(), pgm2Str(fb_esp_pgm_str_79), 0);
                if (p1 == -1)
                {
                    p1 = strpos(s.c_str(), pgm2Str(fb_esp_pgm_str_21), 0);
                }

                if (p1 != -1)
                {
                    tmp = (char *)newP(p1 + 1);
                    memcpy(tmp, s.c_str(), p1);
                    chunkedSize = hex2int(tmp);
                    delP(&tmp);
                }

                //last chunk
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
                MBSTRING s;
                int readLen = readLine(stream, s);

                if (readLen > 0)
                {
                    //chunk may contain trailing
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

    char *getHeader(const char *buf, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {
        char *tmp = nullptr;

        int p1 = strpos(buf, pgm2Str(beginH), beginPos);
        int ofs = 0;
        if (p1 != -1)
        {
           
            int p2 = -1;
            if (endPos > 0)
                p2 = endPos;
            else if (endPos == 0)
            {
                ofs = strlen_P(endH);
                p2 = strpos(buf, pgm2Str(endH), p1 + strlen_P(beginH) + 1);
            }
            else if (endPos == -1)
            {
                beginPos = p1 + strlen_P(beginH);
            }

            if (p2 == -1)
                p2 = strlen(buf);

            if (p2 != -1)
            {
                beginPos = p2 + ofs;
                int len = p2 - p1 - strlen_P(beginH);
                tmp = (char *)newP(len + 1);
                memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
                return tmp;
            }
        }

        return nullptr;
    }

    void getHeaderStr(const MBSTRING &in, MBSTRING &out, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {
        MBSTRING _in = in;
        int p1 = strpos(in.c_str(), pgm2Str(beginH), beginPos);
        int ofs = 0;

        if (p1 != -1)
        {

            int p2 = -1;
            if (endPos > 0)
                p2 = endPos;
            else if (endPos == 0)
            {
                ofs = strlen_P(endH);
                p2 = strpos(in.c_str(), pgm2Str(endH), p1 + strlen_P(beginH) + 1);
            }
            else if (endPos == -1)
            {
                beginPos = p1 + strlen_P(beginH);
            }

            if (p2 == -1)
                p2 = in.length();

            if (p2 != -1)
            {
                beginPos = p2 + ofs;
                int len = p2 - p1 - strlen_P(beginH);
                out = _in.substr(p1 + strlen_P(beginH), len);
            }
        }
    }

    void parseRespPayload(const char *buf, struct server_response_data_t &response, bool getOfs)
    {
        int payloadPos = 0;
        int payloadOfs = 0;

        char *tmp = nullptr;

        if (!response.isEvent && !response.noEvent)
        {

            tmp = getHeader(buf, fb_esp_pgm_str_13, fb_esp_pgm_str_180, payloadPos, 0);
            if (tmp)
            {
                response.isEvent = true;
                response.eventType = tmp;
                delP(&tmp);

                payloadOfs = payloadPos;

                tmp = getHeader(buf, fb_esp_pgm_str_14, fb_esp_pgm_str_180, payloadPos, 0);
                if (tmp)
                {
                    payloadOfs += strlen_P(fb_esp_pgm_str_14);
                    payloadPos = payloadOfs;
                    response.hasEventData = true;

                    delP(&tmp);

                    tmp = getHeader(buf, fb_esp_pgm_str_17, fb_esp_pgm_str_3, payloadPos, 0);

                    if (tmp)
                    {
                        payloadOfs = payloadPos;
                        response.eventPath = tmp;
                        delP(&tmp);
                        tmp = getHeader(buf, fb_esp_pgm_str_18, fb_esp_pgm_str_180, payloadPos, 0);

                        if (tmp)
                        {
                            tmp[strlen(tmp) - 1] = 0;
                            response.payloadLen = strlen(tmp);
                            response.eventData = tmp;
                            payloadOfs += strlen_P(fb_esp_pgm_str_18) + 1;
                            response.payloadOfs = payloadOfs;
                            delP(&tmp);
                        }
                    }
                }
            }
        }

        if (strlen(buf) < (size_t)payloadOfs)
            return;

        if (response.dataType == 0)
        {
            tmp = getHeader(buf, fb_esp_pgm_str_20, fb_esp_pgm_str_3, payloadPos, 0);
            if (tmp)
            {
                response.pushName = tmp;
                delP(&tmp);
            }

            tmp = getHeader(buf, fb_esp_pgm_str_102, fb_esp_pgm_str_3, payloadPos, 0);
            if (tmp)
            {
                delP(&tmp);
                FirebaseJson js;
                FirebaseJsonData d;
                js.setJsonData(buf);
                js.get(d, pgm2Str(fb_esp_pgm_str_176));
                if (d.success)
                    response.fbError = d.stringValue.c_str();
            }

            if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_92))
            {
                response.dataType = fb_esp_data_type::d_blob;
                if ((response.isEvent && response.hasEventData) || getOfs)
                {
                    int dlen = response.eventData.length() - strlen_P(fb_esp_pgm_str_92) - 1;
                    response.payloadLen = dlen;
                    response.payloadOfs += strlen_P(fb_esp_pgm_str_92);
                    response.eventData.clear();
                }
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_93))
            {
                response.dataType = fb_esp_data_type::d_file;
                if ((response.isEvent && response.hasEventData) || getOfs)
                {
                    int dlen = response.eventData.length() - strlen_P(fb_esp_pgm_str_93) - 1;
                    response.payloadLen = dlen;
                    response.payloadOfs += strlen_P(fb_esp_pgm_str_93);
                    response.eventData.clear();
                }
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_3))
            {
                response.dataType = fb_esp_data_type::d_string;
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_163))
            {
                response.dataType = fb_esp_data_type::d_json;
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_182))
            {
                response.dataType = fb_esp_data_type::d_array;
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_106) || stringCompare(buf, payloadOfs, fb_esp_pgm_str_107))
            {
                response.dataType = fb_esp_data_type::d_boolean;
                response.boolData = stringCompare(buf, payloadOfs, fb_esp_pgm_str_107);
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_19))
            {
                response.dataType = fb_esp_data_type::d_null;
            }
            else
            {
                int p1 = strpos(buf, pgm2Str(fb_esp_pgm_str_4), payloadOfs);
                setNumDataType(buf, payloadOfs, response, p1 != -1);
            }
        }
    }

    void setNumDataType(const char *buf, int ofs, struct server_response_data_t &response, bool dec)
    {

        if (!buf || ofs < 0)
            return;

        if (ofs >= (int)strlen(buf))
            return;

        if (response.payloadLen > 0 && response.payloadLen <= (int)strlen(buf) && ofs < (int)strlen(buf) && ofs + response.payloadLen <= (int)strlen(buf))
        {
            char *tmp = (char *)newP(response.payloadLen + 1);

            if (!tmp)
                return;

            memcpy(tmp, &buf[ofs], response.payloadLen);
            tmp[response.payloadLen] = 0;
            double d = atof(tmp);
            delP(&tmp);

            if (dec)
            {
                if (response.payloadLen <= 7)
                {
                    response.floatData = d;
                    response.dataType = fb_esp_data_type::d_float;
                }
                else
                {
                    response.doubleData = d;
                    response.dataType = fb_esp_data_type::d_double;
                }
            }
            else
            {
                if (d > 0x7fffffff)
                {
                    response.doubleData = d;
                    response.dataType = fb_esp_data_type::d_double;
                }
                else
                {
                    response.intData = (int)d;
                    response.dataType = fb_esp_data_type::d_integer;
                }
            }
        }
    }

    void createDirs(MBSTRING dirs, fb_esp_mem_storage_type storageType)
    {
#if defined SD_FS
        MBSTRING dir;
        size_t count = 0;
        for (size_t i = 0; i < dirs.length(); i++)
        {
            dir.append(1, dirs[i]);
            count++;
            if (dirs[i] == '/')
            {
                if (dir.length() > 0)
                {
                    if (storageType == mem_storage_type_sd)
                        SD_FS.mkdir(dir.substr(0, dir.length() - 1).c_str());
                }

                count = 0;
            }
        }
        if (count > 0)
        {
            if (storageType == mem_storage_type_sd)
                SD_FS.mkdir(dir.c_str());
        }
        MBSTRING().swap(dir);
#endif
    }

    void closeFileHandle(bool sd)
    {
        if (!config)
            return;

        if (config->_int.fb_file)
            config->_int.fb_file.close();
        if (sd)
        {
            config->_int.fb_sd_used = false;
            config->_int.fb_sd_rdy = false;
#if defined SD_FS
            SD_FS.end();
#endif
        }
    }

    bool decodeBase64Str(const MBSTRING &src, std::vector<uint8_t> &out)
    {
        unsigned char *dtable = (unsigned char *)newP(256);
        memset(dtable, 0x80, 256);
        for (size_t i = 0; i < sizeof(fb_esp_base64_table) - 1; i++)
            dtable[fb_esp_base64_table[i]] = (unsigned char)i;
        dtable['='] = 0;

        unsigned char *block = (unsigned char *)newP(4);
        unsigned char tmp;
        size_t i, count;
        int pad = 0;
        size_t extra_pad;
        size_t len = src.length();

        count = 0;

        for (i = 0; i < len; i++)
        {
            if ((uint8_t)dtable[(uint8_t)src[i]] != 0x80)
                count++;
        }

        if (count == 0)
            goto exit;

        extra_pad = (4 - count % 4) % 4;

        count = 0;
        for (i = 0; i < len + extra_pad; i++)
        {
            unsigned char val;

            if (i >= len)
                val = '=';
            else
                val = src[i];

            tmp = dtable[val];

            if (tmp == 0x80)
                continue;

            if (val == '=')
                pad++;

            block[count] = tmp;
            count++;
            if (count == 4)
            {
                out.push_back((block[0] << 2) | (block[1] >> 4));
                count = 0;
                if (pad)
                {
                    if (pad == 1)
                        out.push_back((block[1] << 4) | (block[2] >> 2));
                    else if (pad > 2)
                        goto exit;

                    break;
                }
                else
                {
                    out.push_back((block[1] << 4) | (block[2] >> 2));
                    out.push_back((block[2] << 6) | block[3]);
                }
            }
        }

        delP(&block);
        delP(&dtable);

        return true;

    exit:
        delP(&block);
        delP(&dtable);
        return false;
    }

    void sendBase64File(size_t bufSize, WiFiClient *client, const MBSTRING &filePath, uint8_t storageType, fs::File &file)
    {

        if (storageType == mem_storage_type_flash)
        {
#if defined FLASH_FS
            file = FLASH_FS.open(filePath.c_str(), "r");
#endif
        }
        else if (storageType == mem_storage_type_sd)
        {
#if defined SD_FS
            file = SD_FS.open(filePath.c_str(), FILE_READ);
#endif
        }

        if (!file)
            return;

        size_t chunkSize = bufSize;
        size_t fbuffSize = 3;
        size_t byteAdd = 0;
        size_t byteSent = 0;

        unsigned char *buff = (unsigned char *)newP(chunkSize);
        memset(buff, 0, chunkSize);

        size_t len = file.size();
        size_t fbuffIndex = 0;
        unsigned char *fbuff = (unsigned char *)newP(3);

        while (file.available())
        {
            memset(fbuff, 0, fbuffSize);
            if (len - fbuffIndex >= 3)
            {
                file.read(fbuff, 3);

                buff[byteAdd++] = fb_esp_base64_table[fbuff[0] >> 2];
                buff[byteAdd++] = fb_esp_base64_table[((fbuff[0] & 0x03) << 4) | (fbuff[1] >> 4)];
                buff[byteAdd++] = fb_esp_base64_table[((fbuff[1] & 0x0f) << 2) | (fbuff[2] >> 6)];
                buff[byteAdd++] = fb_esp_base64_table[fbuff[2] & 0x3f];

                if (byteAdd >= chunkSize - 4)
                {
                    byteSent += byteAdd;
                    client->write(buff, byteAdd);
                    memset(buff, 0, chunkSize);
                    byteAdd = 0;
                }

                fbuffIndex += 3;
            }
            else
            {

                if (len - fbuffIndex == 1)
                {
                    fbuff[0] = file.read();
                }
                else if (len - fbuffIndex == 2)
                {
                    fbuff[0] = file.read();
                    fbuff[1] = file.read();
                }

                break;
            }
        }

        file.close();

        if (byteAdd > 0)
            client->write(buff, byteAdd);

        if (len - fbuffIndex > 0)
        {

            memset(buff, 0, chunkSize);
            byteAdd = 0;

            buff[byteAdd++] = fb_esp_base64_table[fbuff[0] >> 2];
            if (len - fbuffIndex == 1)
            {
                buff[byteAdd++] = fb_esp_base64_table[(fbuff[0] & 0x03) << 4];
                buff[byteAdd++] = '=';
            }
            else
            {
                buff[byteAdd++] = fb_esp_base64_table[((fbuff[0] & 0x03) << 4) | (fbuff[1] >> 4)];
                buff[byteAdd++] = fb_esp_base64_table[(fbuff[1] & 0x0f) << 2];
            }
            buff[byteAdd++] = '=';

            client->write(buff, byteAdd);
        }

        delP(&buff);
        delP(&fbuff);
    }

    bool decodeBase64Stream(const char *src, size_t len, Stream &s)
    {
        unsigned char *dtable = (unsigned char *)newP(256);
        memset(dtable, 0x80, 256);
        for (size_t i = 0; i < sizeof(fb_esp_base64_table) - 1; i++)
            dtable[fb_esp_base64_table[i]] = (unsigned char)i;
        dtable['='] = 0;

        unsigned char *block = (unsigned char *)newP(4);
        unsigned char tmp;
        size_t i, count;
        int pad = 0;
        size_t extra_pad;

        count = 0;

        for (i = 0; i < len; i++)
        {
            if (dtable[(uint8_t)src[i]] != 0x80)
                count++;
        }

        if (count == 0)
            goto exit;

        extra_pad = (4 - count % 4) % 4;

        count = 0;
        for (i = 0; i < len + extra_pad; i++)
        {
            unsigned char val;

            if (i >= len)
                val = '=';
            else
                val = src[i];
            tmp = dtable[val];
            if (tmp == 0x80)
                continue;

            if (val == '=')
                pad++;

            block[count] = tmp;
            count++;
            if (count == 4)
            {
                s.write((block[0] << 2) | (block[1] >> 4));
                count = 0;
                if (pad)
                {
                    if (pad == 1)
                        s.write((block[1] << 4) | (block[2] >> 2));
                    else if (pad > 2)
                        goto exit;

                    break;
                }
                else
                {
                    s.write((block[1] << 4) | (block[2] >> 2));
                    s.write((block[2] << 6) | block[3]);
                }
            }
        }

        delP(&block);
        delP(&dtable);

        return true;

    exit:

        delP(&block);
        delP(&dtable);

        return false;
    }

    //trim double quotes and return pad length
    int trimLastChunkBase64(MBSTRING &s, size_t len)
    {
        int padLen = -1;
        if (len > 1)
        {
            if (s[len - 1] == '"')
            {
                padLen = 0;
                if (len > 2)
                {
                    if (s[len - 2] == '=')
                        padLen++;
                }
                if (len > 3)
                {
                    if (s[len - 3] == '=')
                        padLen++;
                }
                s[len - 1] = 0;
            }
        }
        return padLen;
    }

    bool writeOTA(uint8_t *buf, size_t len, int &code)
    {

#if defined(ENABLE_OTA_FIRMWARE_UPDATE)
        if (Update.write(buf, len) != len)
        {
            code = FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED;
            return false;
        }
        return true;
#else
        return false;
#endif
    }

    bool addBuffer(uint8_t *buf, uint8_t value, int &index, int chunkSize, int &code)
    {
        if (index < chunkSize)
        {
            buf[index] = value;
            index++;
        }
        else
        {
            index = 0;
            if (!writeOTA(buf, chunkSize, code))
                return false;
            memset(buf, 0, chunkSize);
            buf[index] = value;
            index++;
        }

        return true;
    }

    bool decodeBase64OTA(const char *src, size_t len, int &code)
    {

        unsigned char *dtable = (unsigned char *)newP(256);
        memset(dtable, 0x80, 256);
        for (size_t i = 0; i < sizeof(fb_esp_base64_table) - 1; i++)
            dtable[fb_esp_base64_table[i]] = (unsigned char)i;
        dtable['='] = 0;

        unsigned char *block = (unsigned char *)newP(4);
        unsigned char tmp;
        size_t i, count;
        int pad = 0;
        size_t extra_pad;

        int chunkSize = 1024;
        uint8_t *buf = (uint8_t *)newP(chunkSize);
        int index = 0;

        count = 0;

        for (i = 0; i < len; i++)
        {
            if (dtable[(uint8_t)src[i]] != 0x80)
                count++;
        }

        if (count == 0)
            goto exit;

        extra_pad = (4 - count % 4) % 4;

        count = 0;
        for (i = 0; i < len + extra_pad; i++)
        {
            unsigned char val;

            if (i >= len)
                val = '=';
            else
                val = src[i];
            tmp = dtable[val];
            if (tmp == 0x80)
                continue;

            if (val == '=')
                pad++;

            block[count] = tmp;
            count++;
            if (count == 4)
            {
                if (!addBuffer(buf, (block[0] << 2) | (block[1] >> 4), index, chunkSize, code))
                    break;

                count = 0;
                if (pad)
                {
                    if (pad == 1)
                    {
                        if (!addBuffer(buf, (block[1] << 4) | (block[2] >> 2), index, chunkSize, code))
                            break;
                    }
                    else if (pad > 2)
                        goto exit;

                    break;
                }
                else
                {
                    if (!addBuffer(buf, (block[1] << 4) | (block[2] >> 2), index, chunkSize, code))
                        break;

                    if (!addBuffer(buf, (block[2] << 6) | block[3], index, chunkSize, code))
                        break;
                }
            }
        }

        if (index > 0)
            writeOTA(buf, index, code);

        delP(&block);
        delP(&dtable);
        delP(&buf);

        return true;

    exit:

        delP(&block);
        delP(&dtable);
        delP(&buf);

        return false;
    }

    bool stringCompare(const char *buf, int ofs, PGM_P beginH)
    {
        char *tmp2 = (char *)newP(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcmp(pgm2Str(beginH), tmp2) == 0);
        delP(&tmp2);
        return ret;
    }

    bool setClock(float gmtOffset)
    {
        if (!config)
            return false;

        if (time(nullptr) > default_ts && gmtOffset == config->_int.fb_gmt_offset)
            return true;

        if (config->_int.fb_reconnect_wifi)
            reconnect(0);

        time_t now = time(nullptr);

        config->_int.fb_clock_rdy = now > default_ts;

        if (!config->_int.fb_clock_rdy || gmtOffset != config->_int.fb_gmt_offset)
        {
            configTime(gmtOffset * 3600, 0, "pool.ntp.org", "time.nist.gov");

            now = time(nullptr);
            unsigned long timeout = millis();
            while (now < default_ts)
            {
                now = time(nullptr);
                if (now > default_ts || millis() - timeout > ntpTimeout)
                    break;
                delay(10);
            }
        }

        config->_int.fb_clock_rdy = now > default_ts;
        if (config->_int.fb_clock_rdy)
            config->_int.fb_gmt_offset = gmtOffset;

        return config->_int.fb_clock_rdy;
    }

    void encodeBase64Url(char *encoded, unsigned char *string, size_t len)
    {
        size_t i;
        char *p = encoded;

        unsigned char *b64enc = (unsigned char *)newP(65);
        strcpy_P((char *)b64enc, (char *)fb_esp_base64_table);
        b64enc[62] = '-';
        b64enc[63] = '_';

        for (i = 0; i < len - 2; i += 3)
        {
            *p++ = b64enc[(string[i] >> 2) & 0x3F];
            *p++ = b64enc[((string[i] & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = b64enc[((string[i + 1] & 0xF) << 2) | ((int)(string[i + 2] & 0xC0) >> 6)];
            *p++ = b64enc[string[i + 2] & 0x3F];
        }

        if (i < len)
        {
            *p++ = b64enc[(string[i] >> 2) & 0x3F];
            if (i == (len - 1))
            {
                *p++ = b64enc[((string[i] & 0x3) << 4)];
            }
            else
            {
                *p++ = b64enc[((string[i] & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
                *p++ = b64enc[((string[i + 1] & 0xF) << 2)];
            }
        }

        *p++ = '\0';

        delP(&b64enc);
    }

    bool sendBase64(size_t bufSize, uint8_t *data, size_t len, bool flashMem, FB_TCP_Client *client)
    {
        bool ret = false;
        const unsigned char *end, *in;

        end = data + len;
        in = data;

        size_t chunkSize = bufSize;
        size_t byteAdded = 0;
        size_t byteSent = 0;

        unsigned char *buf = (unsigned char *)newP(chunkSize);
        memset(buf, 0, chunkSize);

        unsigned char *tmp = (unsigned char *)newP(3);

        while (end - in >= 3)
        {
            memset(tmp, 0, 3);
            if (flashMem)
                memcpy_P(tmp, in, 3);
            else
                memcpy(tmp, in, 3);

            buf[byteAdded++] = fb_esp_base64_table[tmp[0] >> 2];
            buf[byteAdded++] = fb_esp_base64_table[((tmp[0] & 0x03) << 4) | (tmp[1] >> 4)];
            buf[byteAdded++] = fb_esp_base64_table[((tmp[1] & 0x0f) << 2) | (tmp[2] >> 6)];
            buf[byteAdded++] = fb_esp_base64_table[tmp[2] & 0x3f];

            if (byteAdded >= chunkSize - 4)
            {
                byteSent += byteAdded;

                if (client->send((const char *)buf) != 0)
                    goto ex;
                memset(buf, 0, chunkSize);
                byteAdded = 0;
            }

            in += 3;
        }

        if (byteAdded > 0)
        {
            if (client->send((const char *)buf) != 0)
                goto ex;
        }

        if (end - in)
        {
            memset(buf, 0, chunkSize);
            byteAdded = 0;
            memset(tmp, 0, 3);
            if (flashMem)
            {
                if (end - in == 1)
                    memcpy_P(tmp, in, 1);
                else
                    memcpy_P(tmp, in, 2);
            }
            else
            {
                if (end - in == 1)
                    memcpy(tmp, in, 1);
                else
                    memcpy(tmp, in, 2);
            }

            buf[byteAdded++] = fb_esp_base64_table[tmp[0] >> 2];
            if (end - in == 1)
            {
                buf[byteAdded++] = fb_esp_base64_table[(tmp[0] & 0x03) << 4];
                buf[byteAdded++] = '=';
            }
            else
            {
                buf[byteAdded++] = fb_esp_base64_table[((tmp[0] & 0x03) << 4) | (tmp[1] >> 4)];
                buf[byteAdded++] = fb_esp_base64_table[(tmp[1] & 0x0f) << 2];
            }
            buf[byteAdded++] = '=';

            if (client->send((const char *)buf) != 0)
                goto ex;
            memset(buf, 0, chunkSize);
        }

        ret = true;
    ex:

        delP(&tmp);
        delP(&buf);
        return ret;
    }

    MBSTRING encodeBase64Str(const unsigned char *src, size_t len)
    {
        return encodeBase64Str((uint8_t *)src, len);
    }

    MBSTRING encodeBase64Str(uint8_t *src, size_t len)
    {
        unsigned char *out, *pos;
        const unsigned char *end, *in;

        unsigned char *b64enc = (unsigned char *)newP(65);
        strcpy_P((char *)b64enc, (char *)fb_esp_base64_table);

        size_t olen;

        olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

        MBSTRING outStr;
        outStr.resize(olen);
        out = (unsigned char *)&outStr[0];

        end = src + len;
        in = src;
        pos = out;

        while (end - in >= 3)
        {
            *pos++ = b64enc[in[0] >> 2];
            *pos++ = b64enc[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = b64enc[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
            *pos++ = b64enc[in[2] & 0x3f];
            in += 3;
            delay(0);
        }

        if (end - in)
        {

            *pos++ = b64enc[in[0] >> 2];

            if (end - in == 1)
            {
                *pos++ = b64enc[(in[0] & 0x03) << 4];
                *pos++ = '=';
            }
            else
            {
                *pos++ = b64enc[((in[0] & 0x03) << 4) | (in[1] >> 4)];
                *pos++ = b64enc[(in[1] & 0x0f) << 2];
            }

            *pos++ = '=';
        }

        delP(&b64enc);
        return outStr;
    }

    size_t base64EncLen(size_t len)
    {
        return ((len + 2) / 3 * 4) + 1;
    }

#if defined(CARD_TYPE_SD)
    bool sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
    {
#if defined SD_FS
        if (!config)
            return false;

        if (config)
        {
            config->_int.sd_config.sck = sck;
            config->_int.sd_config.miso = miso;
            config->_int.sd_config.mosi = mosi;
            config->_int.sd_config.ss = ss;
        }
#if defined(ESP32)
        if (ss > -1)
        {
            SPI.begin(sck, miso, mosi, ss);
            return SD_FS.begin(ss, SPI);
        }
        else
            return SD_FS.begin();
#elif defined(ESP8266)
        if (ss > -1)
            return SD_FS.begin(ss);
        else
            return SD_FS.begin(SD_CS_PIN);
#endif
#else
        return false;
#endif
    }
#endif

#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
    bool sdBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
    {
        if (!config)
            return false;

        if (config)
        {
            config->_int.sd_config.sd_mmc_mountpoint = mountpoint;
            config->_int.sd_config.sd_mmc_mode1bit = mode1bit;
            config->_int.sd_config.sd_mmc_format_if_mount_failed = format_if_mount_failed;
        }
        return SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
    }
#endif
#endif

    bool flashTest()
    {
#if defined FLASH_FS
        if (!config)
            return false;
#if defined(ESP32)
        if (FORMAT_FLASH == 1)
            config->_int.fb_flash_rdy = FLASH_FS.begin(true);
        else
            config->_int.fb_flash_rdy = FLASH_FS.begin();
#elif defined(ESP8266)
        config->_int.fb_flash_rdy = FLASH_FS.begin();
#endif
        return config->_int.fb_flash_rdy;
#else
        return false;
#endif
    }

    bool sdTest(fs::File file)
    {
#if defined SD_FS
        if (!config)
            return false;

        MBSTRING filepath = "/sdtest01.txt";
#if defined(CARD_TYPE_SD)
        if (!sdBegin(config->_int.sd_config.ss, config->_int.sd_config.sck, config->_int.sd_config.miso, config->_int.sd_config.mosi))
            return false;
#endif
#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
        if (!sdBegin(config->_int.sd_config.sd_mmc_mountpoint, config->_int.sd_config.sd_mmc_mode1bit, config->_int.sd_config.sd_mmc_format_if_mount_failed))
            return false;
#endif
#endif
        file = SD_FS.open(filepath.c_str(), FILE_WRITE);
        if (!file)
            return false;

        if (!file.write(32))
        {
            file.close();
            return false;
        }

        file.close();

        file = SD_FS.open(filepath.c_str());
        if (!file)
            return false;

        while (file.available())
        {
            if (file.read() != 32)
            {
                file.close();
                return false;
            }
        }
        file.close();

        SD_FS.remove(filepath.c_str());

        MBSTRING().swap(filepath);

        config->_int.fb_sd_rdy = true;

        return true;
#else
        return false;
#endif
    }

#if defined(ESP8266)
    void set_scheduled_callback(callback_function_t callback)
    {
        _callback_function = std::move([callback]()
                                       { schedule_function(callback); });
        _callback_function();
    }
#endif

    void setFloatDigit(uint8_t digit)
    {
        if (!config)
            return;
        config->_int.fb_float_digits = digit;
    }

    void setDoubleDigit(uint8_t digit)
    {
        if (!config)
            return;
        config->_int.fb_double_digits = digit;
    }

    MBSTRING getBoundary(size_t len)
    {
        const char *tmp = pgm2Str(fb_esp_boundary_table);
        char *buf = (char *)newP(len);
        if (len)
        {
            --len;
            buf[0] = tmp[0];
            buf[1] = tmp[1];
            for (size_t n = 2; n < len; n++)
            {
                int key = rand() % (int)(strlen(tmp) - 1);
                buf[n] = tmp[key];
            }
            buf[len] = '\0';
        }
        MBSTRING s = buf;
        delP(&buf);
        return s;
    }

    bool boolVal(const char *v)
    {
        return strposP(v, fb_esp_pgm_str_107, 0) > -1;
    }

    bool waitIdle(int &httpCode)
    {
        if (!config)
            return true;

#if defined(ESP32)
        if (config->_int.fb_multiple_requests)
            return true;

        unsigned long wTime = millis();
        while (config->_int.fb_processing)
        {
            if (millis() - wTime > 3000)
            {
                httpCode = FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED;
                return false;
            }
            delay(0);
        }
#endif
        return true;
    }

    void splitTk(const MBSTRING &str, std::vector<MBSTRING> &tk, const char *delim)
    {
        std::size_t current, previous = 0;
        current = str.find(delim, previous);
        MBSTRING s;
        MBSTRING _str = str;
        while (current != MBSTRING::npos)
        {
            s = _str.substr(previous, current - previous);
            tk.push_back(s);
            previous = current + strlen(delim);
            current = str.find(delim, previous);
        }
        s = _str.substr(previous, current - previous);
        tk.push_back(s);
        MBSTRING().swap(s);
    }

    void replaceAll(MBSTRING &str, const MBSTRING &from, const MBSTRING &to)
    {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != MBSTRING::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    bool validJS(const char *c)
    {
        size_t ob = 0, cb = 0, os = 0, cs = 0;
        for (size_t i = 0; i < strlen(c); i++)
        {
            if (c[i] == '{')
                ob++;
            else if (c[i] == '}')
                cb++;
            else if (c[i] == '[')
                os++;
            else if (c[i] == ']')
                cs++;
        }
        return (ob == cb && os == cs);
    }

    bool ethLinkUp(SPI_ETH_Module *spi_ethernet_module = NULL)
    {
        bool ret = false;
#if defined(ESP32)
        if (strcmp(ETH.localIP().toString().c_str(), pgm2Str(fb_esp_pgm_str_548)) != 0)
        {
            ret = true;
            ETH.linkUp();
        }
#elif defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)

        if (!spi_ethernet_module)
            return ret;

#if defined(INC_ENC28J60_LWIP)
        if (spi_ethernet_module->enc28j60)
            return spi_ethernet_module->enc28j60->status() == WL_CONNECTED;
#endif
#if defined(INC_W5100_LWIP)
        if (spi_ethernet_module->w5100)
            return spi_ethernet_module->w5100->status() == WL_CONNECTED;
#endif
#if defined(INC_W5100_LWIP)
        if (spi_ethernet_module->w5500)
            return spi_ethernet_module->w5500->status() == WL_CONNECTED;
#endif

#endif
        return ret;
    }

    void ethDNSWorkAround(SPI_ETH_Module *spi_ethernet_module, const char *host, int port)
    {
#if defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)

        if (!spi_ethernet_module)
            goto ex;

#if defined(INC_ENC28J60_LWIP)
        if (spi_ethernet_module->enc28j60)
            goto ex;
#endif
#if defined(INC_W5100_LWIP)
        if (spi_ethernet_module->w5100)
            goto ex;
#endif
#if defined(INC_W5100_LWIP)
        if (spi_ethernet_module->w5500)
            goto ex;
#endif
        return;
    ex:
        WiFiClient client;
        client.connect(host, port);
        client.stop();

#endif
    }

    bool reconnect(unsigned long dataTime)
    {

        bool status = WiFi.status() == WL_CONNECTED;

        if (config)
            status |= ethLinkUp(&config->spi_ethernet_module);

        if (dataTime > 0)
        {
            if (config)
            {
                if (config->timeout.serverResponse < MIN_SERVER_RESPONSE_TIMEOUT || config->timeout.serverResponse > MIN_SERVER_RESPONSE_TIMEOUT)
                    config->timeout.serverResponse = DEFAULT_SERVER_RESPONSE_TIMEOUT;

                if (millis() - dataTime > config->timeout.serverResponse)
                    return false;
            }
            else
            {
                if (millis() - dataTime > 10 * 1000)
                    return false;
            }
        }

        if (!status)
        {

            if (config)
            {
                if (config->_int.fb_reconnect_wifi)
                {
                    if (config->timeout.wifiReconnect < MIN_WIFI_RECONNECT_TIMEOUT || config->timeout.wifiReconnect > MAX_WIFI_RECONNECT_TIMEOUT)
                        config->timeout.wifiReconnect = MIN_WIFI_RECONNECT_TIMEOUT;
                    if (millis() - config->_int.fb_last_reconnect_millis > config->timeout.wifiReconnect)
                    {
                        WiFi.reconnect();
                        config->_int.fb_last_reconnect_millis = millis();
                    }
                }
            }

            status = WiFi.status() == WL_CONNECTED;

            if (config)
                status |= ethLinkUp(&config->spi_ethernet_module);
        }

        return status;
    }

    int setTimestamp(time_t ts)
    {
        struct timeval tm = {ts, 0}; // sec, us
        return settimeofday((const timeval *)&tm, 0);
    }

    uint16_t calCRC(const char *buf)
    {
        return CRC16.ccitt((uint8_t *)buf, strlen(buf));
    }

    void idle()
    {
        delay(0);
    }

    int beginUpdate(WiFiClient *tcp, int len, bool verify = true)
    {
        int code = 0;
#if defined(ESP8266)
        if (len > (int)ESP.getFreeSketchSpace())
        {
            code = FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE;
        }
        else if (verify)
        {
            uint8_t buf[4];
            if (tcp->peekBytes(&buf[0], 4) != 4)
                code = FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE;
            else
            {

                // check for valid first magic byte
                if (buf[0] != 0xE9 && buf[0] != 0x1f)
                {
                    code = FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE;
                }
                else if (buf[0] == 0xe9)
                {
                    uint32_t bin_flash_size = ESP.magicFlashChipSize((buf[3] & 0xf0) >> 4);

                    // check if new bin fits to SPI flash
                    if (bin_flash_size > ESP.getFlashChipRealSize())
                    {
                        code = FIREBASE_ERROR_FW_UPDATE_BIN_SIZE_NOT_MATCH_SPI_FLASH_SPACE;
                    }
                }
            }
        }

        if (code == 0)
        {
            if (!Update.begin(len, 0, -1, 0))
            {
                code = FIREBASE_ERROR_FW_UPDATE_BEGIN_FAILED;
            }
        }
#endif
        return code;
    }

private:
};

#endif