/**
 * Google's Firebase Util class, FB_Utils.h version 1.1.19
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created July 12, 2022
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#ifndef FB_UTILS_H
#define FB_UTILS_H

#include <Arduino.h>
#include "FB_Const.h"
#if defined(ESP8266)
#include <Schedule.h>
#endif
using namespace mb_string;

class UtilsClass
{
    friend class FirebaseSession;
    friend class Firebase_ESP_Client;

public:
#if defined(ESP8266)
    callback_function_t _callback_function = nullptr;
#endif
    FirebaseConfig *config = nullptr;
    MB_FS *mbfs = nullptr;
    time_t ts = 0;

    ~UtilsClass(){};

    UtilsClass(MB_FS *mbfs)
    {
        this->mbfs = mbfs;
    }

    void setConfig(FirebaseConfig *config)
    {
        this->config = config;
    }

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

    void ltrim(MB_String &str, const MB_String &chars = " ")
    {
        size_t pos = str.find_first_not_of(chars);
        if (pos != MB_String::npos)
            str.erase(0, pos);
    }

    void rtrim(MB_String &str, const MB_String &chars = " ")
    {
        size_t pos = str.find_last_not_of(chars);
        if (pos != MB_String::npos)
            str.erase(pos + 1);
    }

    inline MB_String trim(const MB_String &s)
    {
        MB_String chars = " ";
        MB_String str = s;
        ltrim(str, chars);
        rtrim(str, chars);
        return str;
    }

    void delP(void *ptr)
    {
        if (!mbfs)
            return;
        mbfs->delP(ptr);
    }

    size_t getReservedLen(size_t len)
    {
        if (!mbfs)
            return 0;
        return mbfs->getReservedLen(len);
    }

    void *newP(size_t len, bool clear = true)
    {
        if (!mbfs)
            return NULL;
        return mbfs->newP(len, clear);
    }

    void substr(MB_String &str, const char *s, int offset, size_t len)
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
    
    void splitString(const char *str, MB_VECTOR<MB_String> out, const char delim)
    {
        int current = 0, previous = 0;
        current = strpos(str, delim, 0);
        MB_String s;
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

    void getUrlInfo(const MB_String &url, struct fb_esp_url_info_t &info)
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

    MB_String url_encode(const MB_String &s)
    {
        MB_String ret;
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
                char d1, d2;
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

    void hexchar(char c, char &hex1, char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 < 10 ? '0' : 'A' - 10;
        hex2 += hex2 < 10 ? '0' : 'A' - 10;
    }

    char from_hex(char ch)
    {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
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

    void getHeaderStr(const MB_String &in, MB_String &out, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {
        MB_String _in = in;
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

            if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_92, true))
            {
                response.dataType = fb_esp_data_type::d_blob;
                if ((response.isEvent && response.hasEventData) || getOfs)
                {
                    if (response.eventData.length() > 0)
                    {
                        int dlen = response.eventData.length() - strlen_P(fb_esp_pgm_str_92) - 1;
                        response.payloadLen = dlen;
                    }
                    response.payloadOfs += strlen_P(fb_esp_pgm_str_92);
                    response.eventData.clear();
                }
            }
            else if (stringCompare(buf, payloadOfs, fb_esp_pgm_str_93, true))
            {
                response.dataType = fb_esp_data_type::d_file;
                if ((response.isEvent && response.hasEventData) || getOfs)
                {
                    if (response.eventData.length() > 0)
                    {
                        int dlen = response.eventData.length() - strlen_P(fb_esp_pgm_str_93) - 1;
                        response.payloadLen = dlen;
                    }

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

    void createDirs(MB_String dirs, fb_esp_mem_storage_type storageType)
    {
#if defined(SD_FS)
        MB_String dir;
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
        dir.clear();
#endif
    }

#if defined(__AVR__)
    unsigned long long strtoull_alt(const char *s)
    {
        unsigned long long sum = 0;
        while (*s)
        {
            sum = sum * 10 + (*s++ - '0');
        }
        return sum;
    }
#endif

    bool decodeBase64Str(const MB_String &src, MB_VECTOR<uint8_t> &out)
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
                uint8_t v = (block[0] << 2) | (block[1] >> 4);
                out.push_back(v);
                count = 0;
                if (pad)
                {
                    if (pad == 1)
                    {
                        v = (block[1] << 4) | (block[2] >> 2);
                        out.push_back(v);
                    }
                    else if (pad > 2)
                        goto exit;

                    break;
                }
                else
                {
                    v = (block[1] << 4) | (block[2] >> 2);
                    out.push_back(v);
                    v = (block[2] << 6) | block[3];
                    out.push_back(v);
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

    bool decodeBase64Stream(const char *src, size_t len, fb_esp_mem_storage_type type)
    {
        if (!mbfs)
            return false;

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
                mbfs->write(mbfs_type type, (block[0] << 2) | (block[1] >> 4));
                count = 0;
                if (pad)
                {
                    if (pad == 1)
                        mbfs->write(mbfs_type type, (block[1] << 4) | (block[2] >> 2));
                    else if (pad > 2)
                        goto exit;

                    break;
                }
                else
                {
                    mbfs->write(mbfs_type type, (block[1] << 4) | (block[2] >> 2));
                    mbfs->write(mbfs_type type, (block[2] << 6) | block[3]);
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

#if defined(ESP32) || defined(ESP8266)

    // trim double quotes and return pad length
    int trimLastChunkBase64(MB_String &s, int len)
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

#if defined(OTA_UPDATE_ENABLED)
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

#endif

    bool stringCompare(const char *buf, int ofs, PGM_P beginH, bool caseInSensitive = false)
    {
        char *tmp2 = (char *)newP(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;

        bool ret = caseInSensitive ? (strcasecmp(pgm2Str(beginH), tmp2) == 0) : (strcmp(pgm2Str(beginH), tmp2) == 0);
        delP(&tmp2);
        return ret;
    }

    time_t getTime()
    {

        time_t tm = ts;

#if defined(ESP8266) || defined(ESP32)
        if (tm < ESP_DEFAULT_TS)
            tm = time(nullptr);
#else
        tm += millis() / 1000;
#endif

        return tm;
    }

    bool syncClock(float gmtOffset)
    {

        if (!config)
            return false;

        time_t now = getTime();

        config->internal.fb_clock_rdy = (unsigned long)now > ESP_DEFAULT_TS;

        if (config->internal.fb_clock_rdy && gmtOffset == config->internal.fb_gmt_offset)
            return true;

        if (!config->internal.fb_clock_rdy || gmtOffset != config->internal.fb_gmt_offset)
        {
            if (config->internal.fb_clock_rdy && gmtOffset != config->internal.fb_gmt_offset)
                config->internal.fb_clock_synched = false;

#if defined(ESP32) || defined(ESP8266)
            if (!config->internal.fb_clock_synched)
            {
                config->internal.fb_clock_synched = true;
                configTime(gmtOffset * 3600, 0, "pool.ntp.org", "time.nist.gov");
            }
#endif
        }

        now = getTime();

        config->internal.fb_clock_rdy = (unsigned long)now > ESP_DEFAULT_TS;
        if (config->internal.fb_clock_rdy)
            config->internal.fb_gmt_offset = gmtOffset;

        return config->internal.fb_clock_rdy;
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

    MB_String encodeBase64Str(const unsigned char *src, size_t len)
    {
        return encodeBase64Str((uint8_t *)src, len);
    }

    MB_String encodeBase64Str(uint8_t *src, size_t len)
    {
        unsigned char *out, *pos;
        const unsigned char *end, *in;

        unsigned char *b64enc = (unsigned char *)newP(65);
        strcpy_P((char *)b64enc, (char *)fb_esp_base64_table);

        size_t olen;

        olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

        MB_String outStr;
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
        config->internal.fb_float_digits = digit;
    }

    void setDoubleDigit(uint8_t digit)
    {
        if (!config)
            return;
        config->internal.fb_double_digits = digit;
    }

    MB_String getBoundary(size_t len)
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
        MB_String s = buf;
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
        if (config->internal.fb_multiple_requests)
            return true;

        unsigned long wTime = millis();
        while (config->internal.fb_processing)
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

    void splitTk(const MB_String &str, MB_VECTOR<MB_String> &tk, const char *delim)
    {
        size_t current, previous = 0;
        current = str.find(delim, previous);
        MB_String s;
        MB_String _str = str;
        while (current != MB_String::npos)
        {
            s = _str.substr(previous, current - previous);
            tk.push_back(s);
            previous = current + strlen(delim);
            current = str.find(delim, previous);
        }
        s = _str.substr(previous, current - previous);
        tk.push_back(s);
        s.clear();
    }

    void getCustomHeaders(MB_String &header)
    {
        if (!config)
            return;

        if (config->signer.customHeaders.length() > 0)
        {
            MB_VECTOR<MB_String> headers;
            splitTk(config->signer.customHeaders, headers, ",");
            for (size_t i = 0; i < headers.size(); i++)
            {
                size_t p1 = headers[i].find(F("X-Firebase-"));
                size_t p2 = headers[i].find(':');
                size_t p3 = headers[i].find(F("-ETag"));

                if (p1 != MB_String::npos && p2 != MB_String::npos && p2 > p1 && p3 == MB_String::npos)
                {
                    header += headers[i];
                    header += fb_esp_pgm_str_21;
                }
                headers[i].clear();
            }
            headers.clear();
        }
    }

    void replaceAll(MB_String &str, const MB_String &from, const MB_String &to)
    {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != MB_String::npos)
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

    int setTimestamp(time_t ts)
    {
#if defined(ESP32) || defined(ESP8266)
        struct timeval tm = {ts, 0}; // sec, us
        return settimeofday((const timeval *)&tm, 0);
#endif
        return -1;
    }

    uint16_t calCRC(const char *buf)
    {
        if (!mbfs)
            return 0;
        return mbfs->calCRC(buf);
    }

    void idle()
    {
        delay(0);
    }

    void replaceFirebasePath(MB_String &path)
    {

        path.replaceAll(F("."), F(""));
        path.replaceAll(F("$"), F(""));
        path.replaceAll(F("#"), F(""));
        path.replaceAll(F("["), F(""));
        path.replaceAll(F("]"), F(""));

        size_t limit = 768;

        for (size_t i = 0; i < path.length(); i++)
        {
            if (path[i] < 32 || path[i] == 127)
            {
                limit = i;
                break;
            }
        }

        if (path.length() > limit)
        {
            path[limit] = 0;
            path.shrink_to_fit();
        }
    }

    void makePath(MB_String &path)
    {
        if (path.length() > 0)
        {
            if (path[0] != '/')
                path.prepend('/');
        }
    }

    int getBase64Len(int n)
    {
        int len = (4 * ceil(n / 3.0));
        return len;
    }

    int getBase64Padding(int n)
    {
        int pLen = getBase64Len(n);
        int uLen = ceil(4.0 * n / 3.0);
        return pLen - uLen;
    }
};

#endif