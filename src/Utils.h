/**
 * Google's Firebase Util class, Utils.h version 1.0.0
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 12, 2021
 * 
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2021, 2021 K. Suwatchai (Mobizt)
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021, 2021 K. Suwatchai (Mobizt)
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

class UtilsClass
{
    friend class FirebaseSession;
    friend class Firebase_ESP_Client;

public:
    long default_ts = 1510644967;
    uint16_t ntpTimeout = 3000;
    callback_function_t _callback_function = nullptr;
    FirebaseConfig *config = nullptr;

    UtilsClass(FirebaseConfig *cfg)
    {
        config = cfg;
    };

    ~UtilsClass(){};

    char *strP(PGM_P pgm)
    {
        size_t len = strlen_P(pgm) + 1;
        char *buf = newS(len);
        strcpy_P(buf, pgm);
        buf[len - 1] = 0;
        return buf;
    }

    int strposP(const char *buf, PGM_P beginH, int ofs)
    {
        char *tmp = strP(beginH);
        int p = strpos(buf, tmp, ofs);
        delS(tmp);
        return p;
    }

    bool strcmpP(const char *buf, int ofs, PGM_P beginH)
    {
        char *tmp = nullptr;
        if (ofs < 0)
        {
            int p = strposP(buf, beginH, 0);
            if (p == -1)
                return false;
            ofs = p;
        }
        tmp = strP(beginH);
        char *tmp2 = newS(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcasecmp(tmp, tmp2) == 0);
        delS(tmp);
        delS(tmp2);
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
            tmp = newS(len + 1);
            memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
            return tmp;
        }

        return nullptr;
    }

    char *floatStr(float value)
    {
        char *buf = newS(36);
        memset(buf, 0, 36);
        dtostrf(value, 7, config->_int.fb_float_digits, buf);
        return buf;
    }

    char *intStr(int value)
    {
        char *buf = newS(36);
        memset(buf, 0, 36);
        itoa(value, buf, 10);
        return buf;
    }

    char *boolStr(bool value)
    {
        char *buf = nullptr;
        if (value)
            buf = strP(fb_esp_pgm_str_107);
        else
            buf = strP(fb_esp_pgm_str_106);
        return buf;
    }

    char *doubleStr(double value)
    {
        char *buf = newS(36);
        memset(buf, 0, 36);
        dtostrf(value, 12, config->_int.fb_double_digits, buf);
        return buf;
    }

    void appendP(std::string &buf, PGM_P p, bool empty = false)
    {
        if (empty)
            buf.clear();
        char *b = strP(p);
        buf += b;
        delS(b);
    }

    void trimDigits(char *buf)
    {
        size_t i = strlen(buf) - 1;
        while (buf[i] == '0' && i > 0)
        {
            if (buf[i - 1] == '.')
            {
                i--;
                break;
            }
            if (buf[i - 1] != '0')
                break;
            i--;
        }
        if (i < strlen(buf) - 1)
            buf[i] = '\0';
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
        size_t len = strlen(haystack);
        size_t len2 = strlen(needle);
        if (len == 0 || len < len2 || len2 == 0 || offset >= (int)len)
            return -1;
        char *_haystack = newS(len - offset + 1);
        _haystack[len - offset] = 0;
        strncpy(_haystack, haystack + offset, len - offset);
        char *p = strstr(_haystack, needle);
        int r = -1;
        if (p)
            r = p - _haystack + offset;
        delS(_haystack);
        return r;
    }

    char *rstrstr(const char *haystack, const char *needle)
    {
        size_t needle_length = strlen(needle);
        const char *haystack_end = haystack + strlen(haystack) - needle_length;
        const char *p;
        size_t i;
        for (p = haystack_end; p >= haystack; --p)
        {
            for (i = 0; i < needle_length; ++i)
            {
                if (p[i] != needle[i])
                    goto next;
            }
            return (char *)p;
        next:;
        }
        return 0;
    }

    int rstrpos(const char *haystack, const char *needle, int offset)
    {
        size_t len = strlen(haystack);
        size_t len2 = strlen(needle);
        if (len == 0 || len < len2 || len2 == 0 || offset >= (int)len)
            return -1;
        char *_haystack = newS(len - offset + 1);
        _haystack[len - offset] = 0;
        strncpy(_haystack, haystack + offset, len - offset);
        char *p = rstrstr(_haystack, needle);
        int r = -1;
        if (p)
            r = p - _haystack + offset;
        delS(_haystack);
        return r;
    }

    inline std::string trim(const std::string &s)
    {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
        return std::string(wsfront, std::find_if_not(s.rbegin(), std::string::const_reverse_iterator(wsfront), [](int c) { return std::isspace(c); }).base());
    }

    void delS(char *p)
    {
        if (p != nullptr)
            delete[] p;
    }

    char *newS(size_t len)
    {
        char *p = new char[len];
        memset(p, 0, len);
        return p;
    }

    char *newS(char *p, size_t len)
    {
        delS(p);
        p = newS(len);
        return p;
    }

    char *newS(char *p, size_t len, char *d)
    {
        delS(p);
        p = newS(len);
        strcpy(p, d);
        return p;
    }

    std::vector<std::string> splitString(int size, const char *str, const char delim)
    {
        uint16_t index = 0;
        uint16_t len = strlen(str);
        int buffSize = (int)(size * 1.4f);
        char *buf = newS(buffSize);
        std::vector<std::string> out;

        for (uint16_t i = 0; i < len; i++)
        {
            if (str[i] == delim)
            {
                buf = newS(buf, buffSize);
                strncpy(buf, (char *)str + index, i - index);
                buf[i - index] = '\0';
                index = i + 1;
                out.push_back(buf);
            }
        }
        if (index < len + 1)
        {
            buf = newS(buf, buffSize);
            strncpy(buf, (char *)str + index, len - index);
            buf[len - index] = '\0';
            out.push_back(buf);
        }

        delS(buf);
        return out;
    }
    void getUrlInfo(const std::string url, struct fb_esp_url_info_t &info)
    {
        size_t p1;
        size_t p2;
        int scheme = 0;
        char *tmp = nullptr;
        std::string _h;
        tmp = strP(fb_esp_pgm_str_111);
        p1 = url.find(tmp, 0);
        delS(tmp);
        if (p1 == std::string::npos)
        {
            tmp = strP(fb_esp_pgm_str_112);
            p1 = url.find(tmp, 0);
            delS(tmp);
            if (p1 != std::string::npos)
                scheme = 2;
        }
        else
            scheme = 1;

        if (scheme == 1)
            p1 += strlen_P(fb_esp_pgm_str_111);
        else if (scheme == 2)
            p1 += strlen_P(fb_esp_pgm_str_112);
        else
            p1 = 0;

        if (p1 + 3 < url.length())
            if (url[p1] == 'w' && url[p1 + 1] == 'w' && url[p1 + 2] == 'w' && url[p1 + 3] == '.')
                p1 += 4;

        tmp = strP(fb_esp_pgm_str_1);
        p2 = url.find(tmp, p1 + 1);
        delS(tmp);
        if (p2 == std::string::npos)
        {
            tmp = strP(fb_esp_pgm_str_173);
            p2 = url.find(tmp, p1 + 1);
            delS(tmp);
            if (p2 == std::string::npos)
            {
                tmp = strP(fb_esp_pgm_str_174);
                p2 = url.find(tmp, p1 + 1);
                delS(tmp);
                if (p2 == std::string::npos)
                    p2 = url.length();
            }
        }

        _h = url.substr(p1, p2 - p1);

        if (_h[0] == '/')
            info.uri = _h;
        else
            info.host = _h;

        if (config->_int.fb_auth_uri)
        {
            //get uri
            if (p2 != _h.length())
            {
                info.uri = url.substr(p2, url.length() - p2);
                tmp = strP(fb_esp_pgm_str_170);
                p1 = _h.find(tmp, url.length());
                delS(tmp);

                if (p1 == std::string::npos)
                {
                    tmp = strP(fb_esp_pgm_str_171);
                    p1 = _h.find(tmp, url.length());
                    delS(tmp);
                }

                if (p1 != std::string::npos)
                {
                    p1 += 6;
                    tmp = strP(fb_esp_pgm_str_172);
                    p2 = _h.find(tmp, p1);
                    delS(tmp);
                    if (p2 == std::string::npos)
                        p2 = _h.length();
                    info.auth = _h.substr(p1, p2 - p1);
                }
            }
        }
    }

    int url_decode(const char *s, char *dec)
    {
        char *o;
        const char *end = s + strlen(s);
        int c;

        for (o = dec; s <= end; o++)
        {
            c = *s++;
            if (c == '+')
                c = ' ';
            else if (c == '%' && (!ishex(*s++) ||
                                  !ishex(*s++) ||
                                  !sscanf(s - 2, "%2x", &c)))
                return -1;

            if (dec)
                *o = c;
        }

        return o - dec;
    }

    std::string url_encode(std::string s)
    {
        const char *str = s.c_str();
        std::vector<char> v(s.size());
        v.clear();
        for (size_t i = 0, l = s.size(); i < l; i++)
        {
            char c = str[i];
            if ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
                v.push_back(c);
            }
            else if (c == ' ')
            {
                v.push_back('+');
            }
            else
            {
                v.push_back('%');
                unsigned char d1, d2;
                hexchar(c, d1, d2);
                v.push_back(d1);
                v.push_back(d2);
            }
        }

        return std::string(v.cbegin(), v.cend());
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
                delS(tmp);
            }
            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_8, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.contentType = tmp;
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_12, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.contentLen = atoi(tmp);
                delS(tmp);
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
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_150, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.etag = tmp;
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_10, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_esp_pgm_str_12, fb_esp_pgm_str_21, beginPos, 0);
            if (tmp)
            {

                response.payloadLen = atoi(tmp);
                delS(tmp);
            }

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT || response.httpCode == FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY || response.httpCode == FIREBASE_ERROR_HTTP_CODE_FOUND)
            {
                if (pmax < beginPos)
                    pmax = beginPos;
                beginPos = payloadPos;
                tmp = getHeader(buf, fb_esp_pgm_str_95, fb_esp_pgm_str_21, beginPos, 0);
                if (tmp)
                {
                    response.location = tmp;
                    delS(tmp);
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
        while (stream->available() && idx <= bufLen)
        {
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
            buf = newS(bufLen);
            int readLen = readLine(stream, buf, bufLen);
            if (readLen)
            {
                tmp = strP(fb_esp_pgm_str_79);
                p1 = strpos(buf, tmp, 0);
                delS(tmp);
                if (p1 == -1)
                {
                    tmp = strP(fb_esp_pgm_str_21);
                    p1 = strpos(buf, tmp, 0);
                    delS(tmp);
                }

                if (p1 != -1)
                {
                    tmp = newS(p1 + 1);
                    memcpy(tmp, buf, p1);
                    chunkedSize = hex2int(tmp);
                    delS(tmp);
                }

                //last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;

            delS(buf);
        }
        else
        {

            if (chunkedSize > -1)
            {
                buf = newS(bufLen);
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

                delS(buf);
            }
        }

        return olen;
    }

    char *getHeader(const char *buf, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {

        char *tmp = strP(beginH);
        int p1 = strpos(buf, tmp, beginPos);
        int ofs = 0;
        delS(tmp);
        if (p1 != -1)
        {
            tmp = strP(endH);
            int p2 = -1;
            if (endPos > 0)
                p2 = endPos;
            else if (endPos == 0)
            {
                ofs = strlen_P(endH);
                p2 = strpos(buf, tmp, p1 + strlen_P(beginH) + 1);
            }
            else if (endPos == -1)
            {
                beginPos = p1 + strlen_P(beginH);
            }

            if (p2 == -1)
                p2 = strlen(buf);

            delS(tmp);

            if (p2 != -1)
            {
                beginPos = p2 + ofs;
                int len = p2 - p1 - strlen_P(beginH);
                tmp = newS(len + 1);
                memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
                return tmp;
            }
        }

        return nullptr;
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
                delS(tmp);

                payloadOfs = payloadPos;

                tmp = getHeader(buf, fb_esp_pgm_str_14, fb_esp_pgm_str_180, payloadPos, 0);
                if (tmp)
                {
                    payloadOfs += strlen_P(fb_esp_pgm_str_14);
                    payloadPos = payloadOfs;
                    response.hasEventData = true;

                    delS(tmp);

                    tmp = getHeader(buf, fb_esp_pgm_str_17, fb_esp_pgm_str_3, payloadPos, 0);

                    if (tmp)
                    {
                        payloadOfs = payloadPos;
                        response.eventPath = tmp;
                        delS(tmp);
                        tmp = getHeader(buf, fb_esp_pgm_str_18, fb_esp_pgm_str_180, payloadPos, 0);

                        if (tmp)
                        {
                            tmp[strlen(tmp) - 1] = 0;
                            response.payloadLen = strlen(tmp);
                            response.eventData = tmp;
                            payloadOfs += strlen_P(fb_esp_pgm_str_18) + 1;
                            response.payloadOfs = payloadOfs;
                            delS(tmp);
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
                delS(tmp);
            }

            tmp = getHeader(buf, fb_esp_pgm_str_102, fb_esp_pgm_str_3, payloadPos, 0);
            if (tmp)
            {
                delS(tmp);
                FirebaseJson js;
                FirebaseJsonData d;
                js.setJsonData(buf);
                tmp = strP(fb_esp_pgm_str_176);
                js.get(d, tmp);
                delS(tmp);
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
                tmp = strP(fb_esp_pgm_str_4);
                int p1 = strpos(buf, tmp, payloadOfs);
                delS(tmp);
                setNumDataType(buf, payloadOfs, response, p1 != -1);
            }
        }
    }

    void setNumDataType(const char *buf, int ofs, struct server_response_data_t &response, bool dec)
    {

        if (response.payloadLen > 0 && response.payloadLen <= (int)strlen(buf) && ofs < (int)strlen(buf) && ofs + response.payloadLen <= (int)strlen(buf))
        {
            char *tmp = newS(response.payloadLen + 1);
            memcpy(tmp, &buf[ofs], response.payloadLen);
            tmp[response.payloadLen] = 0;
            double d = atof(tmp);
            delS(tmp);

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

    void createDirs(std::string dirs, fb_esp_mem_storage_type storageType)
    {
        std::string dir = "";
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
                        SD.mkdir(dir.substr(0, dir.length() - 1).c_str());
                }

                count = 0;
            }
        }
        if (count > 0)
        {
            if (storageType == mem_storage_type_sd)
                SD.mkdir(dir.c_str());
        }
        std::string().swap(dir);
    }

    void closeFileHandle(bool sd)
    {
        if (config->_int.fb_file)
            config->_int.fb_file.close();
        if (sd)
        {
            config->_int.fb_sd_used = false;
            config->_int.fb_sd_rdy = false;
            SD.end();
        }
    }

    bool decodeBase64Str(const std::string src, std::vector<uint8_t> &out)
    {
        unsigned char *dtable = new unsigned char[256];
        memset(dtable, 0x80, 256);
        for (size_t i = 0; i < sizeof(fb_esp_base64_table) - 1; i++)
            dtable[fb_esp_base64_table[i]] = (unsigned char)i;
        dtable['='] = 0;

        unsigned char *block = new unsigned char[4];
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

        delete[] block;
        delete[] dtable;

        return true;

    exit:
        delete[] block;
        delete[] dtable;
        return false;
    }

    bool decodeBase64Flash(const char *src, size_t len, fs::File &file)
    {
        unsigned char *dtable = new unsigned char[256];
        memset(dtable, 0x80, 256);
        for (size_t i = 0; i < sizeof(fb_esp_base64_table) - 1; i++)
            dtable[fb_esp_base64_table[i]] = (unsigned char)i;
        dtable['='] = 0;

        unsigned char *block = new unsigned char[4];
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
                file.write((block[0] << 2) | (block[1] >> 4));
                count = 0;
                if (pad)
                {
                    if (pad == 1)
                        file.write((block[1] << 4) | (block[2] >> 2));
                    else if (pad > 2)
                        goto exit;
                    break;
                }
                else
                {
                    file.write((block[1] << 4) | (block[2] >> 2));
                    file.write((block[2] << 6) | block[3]);
                }
            }
        }

        delete[] block;
        delete[] dtable;

        return true;

    exit:
        delete[] block;
        delete[] dtable;

        return false;
    }

    void sendBase64Stream(WiFiClient *client, const std::string &filePath, uint8_t storageType, fs::File &file)
    {

        if (storageType == mem_storage_type_flash)
            file = FLASH_FS.open(filePath.c_str(), "r");
        else if (storageType == mem_storage_type_sd)
            file = SD.open(filePath.c_str(), FILE_READ);

        if (!file)
            return;

        size_t chunkSize = 512;
        size_t fbuffSize = 3;
        size_t byteAdd = 0;
        size_t byteSent = 0;

        unsigned char *buff = new unsigned char[chunkSize];
        memset(buff, 0, chunkSize);

        size_t len = file.size();
        size_t fbuffIndex = 0;
        unsigned char *fbuff = new unsigned char[3];

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

        delete[] buff;
        delete[] fbuff;
    }

    bool decodeBase64Stream(const char *src, size_t len, fs::File &file)
    {
        unsigned char *dtable = new unsigned char[256];
        memset(dtable, 0x80, 256);
        for (size_t i = 0; i < sizeof(fb_esp_base64_table) - 1; i++)
            dtable[fb_esp_base64_table[i]] = (unsigned char)i;
        dtable['='] = 0;

        unsigned char *block = new unsigned char[4];
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
                file.write((block[0] << 2) | (block[1] >> 4));
                count = 0;
                if (pad)
                {
                    if (pad == 1)
                        file.write((block[1] << 4) | (block[2] >> 2));
                    else if (pad > 2)
                        goto exit;

                    break;
                }
                else
                {
                    file.write((block[1] << 4) | (block[2] >> 2));
                    file.write((block[2] << 6) | block[3]);
                }
            }
        }

        delete[] block;
        delete[] dtable;

        return true;

    exit:

        delete[] block;
        delete[] dtable;

        return false;
    }

    bool stringCompare(const char *buf, int ofs, PGM_P beginH)
    {
        char *tmp = strP(beginH);
        char *tmp2 = newS(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcmp(tmp, tmp2) == 0);
        delS(tmp);
        delS(tmp2);
        return ret;
    }

    bool setClock(float gmtOffset)
    {
        if (time(nullptr) > default_ts && gmtOffset == config->_int.fb_gmt_offset)
            return true;

        if (WiFi.status() != WL_CONNECTED)
            WiFi.reconnect();

        time_t now = time(nullptr);

        config->_int.fb_clock_rdy = now > default_ts;

        if (!config->_int.fb_clock_rdy || gmtOffset != config->_int.fb_gmt_offset)
        {
            char *server1 = strP(fb_esp_pgm_str_187);
            char *server2 = strP(fb_esp_pgm_str_188);

            configTime(gmtOffset * 3600, 0, server1, server2);

            now = time(nullptr);
            unsigned long timeout = millis();
            while (now < default_ts)
            {
                now = time(nullptr);
                if (now > default_ts || millis() - timeout > ntpTimeout)
                    break;
                delay(200);
            }

            delS(server1);
            delS(server2);
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

        unsigned char *b64enc = new unsigned char[65];
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

        delete[] b64enc;
    }

    std::string encodeBase64Str(const unsigned char *src, size_t len)
    {
        return encodeBase64Str((uint8_t *)src, len);
    }

    std::string encodeBase64Str(uint8_t *src, size_t len)
    {
        unsigned char *out, *pos;
        const unsigned char *end, *in;

        unsigned char *b64enc = new unsigned char[65];
        strcpy_P((char *)b64enc, (char *)fb_esp_base64_table);

        size_t olen;

        olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

        if (olen < len)
        {
            delete[] b64enc;
            return std::string();
        }

        std::string outStr;
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

        delete[] b64enc;
        return outStr;
    }
    size_t base64EncLen(size_t len)
    {
        return ((len + 2) / 3 * 4) + 1;
    }

    bool sdTest(fs::File file)
    {
        std::string filepath = "/sdtest01.txt";

        SD.begin(SD_CS_PIN);

        file = SD.open(filepath.c_str(), FILE_WRITE);
        if (!file)
            return false;

        if (!file.write(32))
        {
            file.close();
            return false;
        }

        file.close();

        file = SD.open(filepath.c_str());
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

        SD.remove(filepath.c_str());

        std::string().swap(filepath);

        return true;
    }

#if defined(ESP8266)
    void set_scheduled_callback(callback_function_t callback)
    {
        _callback_function = std::move([callback]() { schedule_function(callback); });
        _callback_function();
    }
#endif

    void setFloatDigit(uint8_t digit)
    {
        config->_int.fb_float_digits = digit;
    }

    void setDoubleDigit(uint8_t digit)
    {
        config->_int.fb_double_digits = digit;
    }

    std::string getBoundary(size_t len)
    {
        char *tmp = strP(fb_esp_boundary_table);
        char *buf = newS(len);
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
        std::string s = buf;
        delS(buf);
        delS(tmp);
        return s;
    }

    bool boolVal(const char *v)
    {
        return strposP(v, fb_esp_pgm_str_107, 0) > -1;
    }

    bool waitIdle(int &httpCode)
    {
#if defined(ESP32)
        if (config->_int.fb_multiple_requests)
            return true;

        unsigned long wTime = millis();
        while (config->_int.fb_processing)
        {
            if (millis() - wTime > 3000)
            {
                httpCode = FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_INUSED;
                return false;
            }
            delay(0);
        }
#endif
        return true;
    }

private:
};

#endif