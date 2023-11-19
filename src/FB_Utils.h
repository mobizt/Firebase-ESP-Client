
/**
 *
 * Created September 5, 2023
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#ifndef FB_UTILS_H
#define FB_UTILS_H

#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "./FirebaseFS.h"

#include <Arduino.h>
#include "./FB_Const.h"
#if defined(ESP8266)
#include <Schedule.h>
#endif

#if defined(ESP8266)
#if __has_include(<core_esp8266_version.h>)
#include <core_esp8266_version.h>
#endif
#endif

using namespace mb_string;

#define stringPtr2Str(p) (MB_String().appendPtr(p).c_str())

namespace FBUtils
{
    inline void idle()
    {
#if defined(ARDUINO_ESP8266_MAJOR) && defined(ARDUINO_ESP8266_MINOR) && defined(ARDUINO_ESP8266_REVISION) && ((ARDUINO_ESP8266_MAJOR == 3 && ARDUINO_ESP8266_MINOR >= 1) || ARDUINO_ESP8266_MAJOR > 3)
        esp_yield();
#else
        delay(0);
#endif
    }
};

class StringHelper
{
public:
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

    size_t getReservedLen(MB_FS *mbfs, size_t len)
    {
        return mbfs->getReservedLen(len);
    }

    void pushTk(const MB_String &str, MB_VECTOR<MB_String> &tk)
    {
        MB_String s = str;
        s.trim();
        if (s.length() > 0)
            tk.push_back(s);
    }

    void splitTk(const MB_String &str, MB_VECTOR<MB_String> &tk, const char *delim)
    {
        size_t current, previous = 0;
        current = str.find(delim, previous);
        while (current != MB_String::npos)
        {
            pushTk(str.substr(previous, current - previous), tk);
            previous = current + strlen(delim);
            current = str.find(delim, previous);
        }
        pushTk(str.substr(previous, current - previous), tk);
    }

    bool find(const MB_String &src, PGM_P token, bool last, size_t offset, int &pos)
    {
        size_t ret = last ? src.find_last_of(pgm2Str(token), offset) : src.find(pgm2Str(token), offset);

        if (ret != MB_String::npos)
        {
            pos = ret;
            return true;
        }
        pos = -1;
        return false;
    }

    bool compare(const MB_String &src, int ofs, PGM_P token, bool caseInSensitive = false)
    {
        MB_String copy;
        src.substr(copy, ofs, strlen_P(token));
        return caseInSensitive ? (strcasecmp(pgm2Str(token), copy.c_str()) == 0) : (strcmp(pgm2Str(token), copy.c_str()) == 0);
    }

    /* convert string to boolean */
    bool str2Bool(const MB_String &v)
    {
        return v.length() > 0 && strcmp(v.c_str(), pgm2Str(firebase_pgm_str_20 /* "true" */)) == 0;
    }

    MB_String intStr2Str(const MB_String &v)
    {
        return MB_String(atoi(v.c_str()));
    }

    MB_String boolStr2Str(const MB_String &v)
    {
        return MB_String(str2Bool(v.c_str()));
    }

    bool tokenSubString(const MB_String &src, MB_String &out, PGM_P token1, PGM_P token2,
                        int &ofs1, int ofs2, bool advanced)
    {
        size_t pos1 = src.find(pgm2Str(token1), ofs1);
        size_t pos2 = MB_String::npos;

        int len1 = strlen_P(token1);
        int len2 = 0;

        if (pos1 != MB_String::npos)
        {
            if (ofs2 > 0)
                pos2 = ofs2;
            else if (ofs2 == 0)
            {
                len2 = strlen_P(token2);
                pos2 = src.find(pgm2Str(token2), pos1 + len1 + 1);
            }
            else if (ofs2 == -1)
                ofs1 = pos1 + len1;

            if (pos2 == MB_String::npos)
                pos2 = src.length();

            if (pos2 != MB_String::npos)
            {
                // advanced the begin position before return
                if (advanced)
                    ofs1 = pos2 + len2;
                out = src.substr(pos1 + len1, pos2 - pos1 - len1);
                return true;
            }
        }

        return false;
    }

    bool tokenSubStringInt(const MB_String &buf, int &out, PGM_P token1, PGM_P token2, int &ofs1, int ofs2, bool advanced)
    {
        MB_String s;
        if (tokenSubString(buf, s, token1, token2, ofs1, ofs2, advanced))
        {
            out = atoi(s.c_str());
            return true;
        }
        return false;
    }
};

class URLHelper
{
public:
    /* Append a parameter to URL */
    bool addParam(MB_String &url, PGM_P key, const MB_String &val, bool &hasParam, bool allowEmptyValue = false)
    {
        if (!allowEmptyValue && val.length() == 0)
            return false;

        MB_String _key(key);

        if (!hasParam && _key[0] == '&')
            _key[0] = '?';
        else if (hasParam && _key[0] == '?')
            _key[0] = '&';

        if (_key[0] != '?' && _key[0] != '&')
            url += !hasParam ? firebase_pgm_str_7 /* "?" */ : firebase_pgm_str_8 /* "&" */;

        if (_key[_key.length() - 1] != '=' && _key.find('=') == MB_String::npos)
            _key += firebase_pgm_str_13; // "="

        url += _key;
        url += val;
        hasParam = true;
        return true;
    }

    /* Append the comma separated tokens as URL parameters */
    void addParamsTokens(StringHelper *sh, MB_String &url, PGM_P key, MB_String val, bool &hasParam)
    {
        if (val.length() == 0)
            return;

        MB_VECTOR<MB_String> tk;
        sh->splitTk(val, tk, ",");
        for (size_t i = 0; i < tk.size(); i++)
            addParam(url, key, tk[i], hasParam);
    }

    /* Append the path to URL */
    void addPath(MB_String &url, const MB_String &path)
    {
        if (path.length() > 0)
        {
            if (path[0] != '/')
                url += firebase_pgm_str_1; // "/"
        }
        else
            url += firebase_pgm_str_1; // "/"

        url += path;
    }

    /* Append the string with google storage URL */
    void addGStorageURL(MB_String &uri, const MB_String &bucketID, const MB_String &storagePath)
    {
        uri += firebase_pgm_str_21; // "gs://"
        uri += bucketID;
        if (storagePath[0] != '/')
            uri += firebase_pgm_str_1; // "/"
        uri += storagePath;
    }

    /* Append the string with cloudfunctions project host */
    void addFunctionsHost(MB_String &uri, const MB_String &locationId, const MB_String &projectId,
                          const MB_String &path, bool url)
    {
#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
        if (url)
            uri = firebase_pgm_str_22; // "https://"
        uri += locationId;
        uri += firebase_pgm_str_14; // "-"
        uri += projectId;
        uri += firebase_func_pgm_str_82; // ".cloudfunctions.net"
        if (path.length() > 0)
        {
            uri += firebase_pgm_str_1; // "/"
            uri += path;
        }
#endif
    }

    void addGAPIv1Path(MB_String &uri)
    {
        uri += firebase_pgm_str_23; // "/v1/projects/"
    }

    void addGAPIv1beta1Path(MB_String &uri)
    {
        uri += firebase_pgm_str_24; // "/v1beta1/projects/"
    }

    void host2Url(MB_String &url, MB_String &host)
    {
        url = firebase_pgm_str_22; // "https://"
        url += host;
    }

    void parse(MB_FS *mbfs, const MB_String &url, struct firebase_url_info_t &info)
    {
        char *host = reinterpret_cast<char *>(mbfs->newP(url.length()));
        char *uri = reinterpret_cast<char *>(mbfs->newP(url.length()));
        char *auth = reinterpret_cast<char *>(mbfs->newP(url.length()));

        int p1 = 0;
        int x = sscanf(url.c_str(), pgm2Str(firebase_pgm_str_25 /* "https://%[^/]/%s" */), host, uri);
        x ? p1 = 8 : x = sscanf(url.c_str(), pgm2Str(firebase_pgm_str_26 /* "http://%[^/]/%s" */), host, uri);
        x ? p1 = 7 : x = sscanf(url.c_str(), pgm2Str(firebase_pgm_str_27 /* "%[^/]/%s" */), host, uri);

        size_t p2 = 0;
        if (x > 0)
        {
            p2 = MB_String(host).find(pgm2Str(firebase_pgm_str_7 /* "?" */), 0);
            if (p2 != MB_String::npos)
                x = sscanf(url.c_str() + p1, pgm2Str(firebase_pgm_str_28 /* "%[^?]?%s" */), host, uri);
        }

        if (strlen(uri) > 0)
        {
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
            p2 = MB_String(uri).find(pgm2Str(firebase_rtdb_pgm_str_19 /* "auth=" */), 0);
            if (p2 != MB_String::npos)
                x = sscanf(uri + p2 + 5, pgm2Str(firebase_pgm_str_29 /* "%[^&]" */), auth);
#endif
        }

        info.uri = uri;
        info.host = host;
        info.auth = auth;
        mbfs->delP(&host);
        mbfs->delP(&uri);
        mbfs->delP(&auth);
    }

    void hexchar(char c, char &hex1, char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 < 10 ? '0' : 'A' - 10;
        hex2 += hex2 < 10 ? '0' : 'A' - 10;
    }

    MB_String encode(const MB_String &s)
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
};

class JsonHelper
{
public:
    /* check for the JSON path or key */
    bool isJsonPath(PGM_P path)
    {
        return MB_String(path).find('/') != MB_String::npos;
    }

    bool parseChunk(MB_String &val, const MB_String &chunk, const MB_String &key, int &pos)
    {
        if (key.length() == 0)
            return false;

        MB_String token = firebase_pgm_str_4; // "\""

        MB_String _key;

        if (key[0] != '"')
            _key += token;
        _key += key;
        if (key[key.length() - 1] != '"')
            _key += token;

        size_t p1 = chunk.find(_key, pos);
        if (p1 != MB_String::npos)
        {
            size_t p2 = chunk.find(MB_String(firebase_pgm_str_2 /* ":" */).c_str(), p1 + _key.length());
            if (p2 != MB_String::npos)
                p2 = chunk.find(token, p2 + 1);
            if (p2 != MB_String::npos)
            {
                size_t p3 = chunk.find(token, p2 + token.length());
                if (p3 != MB_String::npos)
                {
                    pos = p3;
                    val = chunk.substr(p2 + token.length(), p3 - p2 - token.length());
                    return true;
                }
            }
        }
        return false;
    }

    /* convert comma separated tokens into JSON Array and set/add to JSON object */
    void addTokens(StringHelper *sh, FirebaseJson *json, PGM_P key, const MB_String &tokens, const char *pre = "")
    {
        if (json && tokens.length() > 0)
        {
            FirebaseJsonArray arr;
            MB_VECTOR<MB_String> ta;
            sh->splitTk(tokens, ta, ",");
            for (size_t i = 0; i < ta.size(); i++)
            {
                if (strlen(pre))
                {
                    MB_String s = pre;
                    s += ta[i];
                    arr.add(s);
                }
                else
                    arr.add(ta[i]);
            }

            if (ta.size() > 0)
            {
                if (isJsonPath(key))
                    json->set(pgm2Str(key), arr);
                else
                    json->add(pgm2Str(key), arr);
            }
        }
    }

    bool addString(FirebaseJson *json, PGM_P key, const MB_String &val)
    {
        if (json && val.length() > 0)
        {
            if (isJsonPath(key))
                json->set(pgm2Str(key), val);
            else
                json->add(pgm2Str(key), val);
            return true;
        }
        return false;
    }

    bool remove(FirebaseJson *json, PGM_P key)
    {
        if (json)
            return json->remove(pgm2Str(key));
        return false;
    }

    void addString(FirebaseJson *json, PGM_P key, const MB_String &val, bool &flag)
    {
        if (addString(json, key, val))
            flag = true;
    }

    void addBoolString(FirebaseJson *json, PGM_P key, const MB_String &val, bool &flag)
    {
        if (json && val.length() > 0)
        {
            if (isJsonPath(key))
                json->set(pgm2Str(key), strcmp(val.c_str(), pgm2Str(firebase_pgm_str_20 /* "true" */)) == 0 ? true : false);
            else
                json->add(pgm2Str(key), strcmp(val.c_str(), pgm2Str(firebase_pgm_str_20 /* "true" */)) == 0 ? true : false);
            flag = true;
        }
    }

    void addArrayString(FirebaseJson *json, PGM_P key, const MB_String &val, bool &flag)
    {
        if (val.length() > 0)
        {
            static FirebaseJsonArray arr;
            arr.clear();
            arr.setJsonArrayData(val);
            if (isJsonPath(key))
                json->set(pgm2Str(key), arr);
            else
                json->add(pgm2Str(key), arr);
            flag = true;
        }
    }

    void addObject(FirebaseJson *json, PGM_P key, FirebaseJson *val, bool clearAfterAdded)
    {
        if (json)
        {
            FirebaseJson js;

            if (!val)
                val = &js;

            if (isJsonPath(key))
                json->set(pgm2Str(key), *val);
            else
                json->add(pgm2Str(key), *val);

            if (clearAfterAdded && val)
                val->clear();
        }
    }

    void addNumberString(FirebaseJson *json, PGM_P key, const MB_String &val)
    {
        if (json && val.length() > 0)
        {
            if (isJsonPath(key))
                json->set(pgm2Str(key), atoi(val.c_str()));
            else
                json->add(pgm2Str(key), atoi(val.c_str()));
        }
    }

    void arrayAddObjectString(FirebaseJsonArray *arr, MB_String &val, bool clearAfterAdded)
    {
        if (arr && val.length() > 0)
        {
            FirebaseJson json(val);
            arr->add(json);
            if (clearAfterAdded)
                val.clear();
        }
    }

    void arrayAddObject(FirebaseJsonArray *arr, FirebaseJson *val, bool clearAfterAdded)
    {
        if (arr && val)
        {
            arr->add(*val);
            if (clearAfterAdded)
                val->clear();
        }
    }

    void addArray(FirebaseJson *json, PGM_P key, FirebaseJsonArray *val, bool clearAfterAdded)
    {
        if (json && val)
        {
            if (isJsonPath(key))
                json->set(pgm2Str(key), *val);
            else
                json->add(pgm2Str(key), *val);
            if (clearAfterAdded)
                val->clear();
        }
    }

    bool parse(FirebaseJson *json, FirebaseJsonData *result, PGM_P key)
    {
        bool ret = false;
        if (json && result)
        {
            result->clear();
            json->get(*result, pgm2Str(key));
            ret = result->success;
        }
        return ret;
    }

    bool setData(FirebaseJson *json, MB_String &val, bool clearAfterAdded)
    {
        bool ret = false;

        if (json && val.length() > 0)
            ret = json->setJsonData(val);

        if (clearAfterAdded)
            val.clear();

        return ret;
    }

    void toString(FirebaseJson *json, MB_String &out, bool clearSource, bool prettify = false)
    {
        if (json)
        {
            out.clear();
            json->toString(out, prettify);
            if (clearSource)
                json->clear();
        }
    }

    void clear(FirebaseJson *json)
    {
        if (json)
            json->clear();
    }

    void arrayClear(FirebaseJsonArray *arr)
    {
        if (arr)
            arr->clear();
    }
};

class HttpHelper
{
public:
    void addNewLine(MB_String &header)
    {
        header += firebase_pgm_str_30; // "\r\n"
    }

    void addGAPIsHost(MB_String &str, PGM_P sub)
    {
        str += sub;
        if (str[str.length() - 1] != '.')
            str += firebase_pgm_str_5; // "."
        str += firebase_pgm_str_31;    // "googleapis.com"
    }

    void addGAPIsHostHeader(MB_String &header, PGM_P sub)
    {
        header += firebase_pgm_str_32; // "Host: "
        addGAPIsHost(header, sub);
        addNewLine(header);
    }

    void addHostHeader(MB_String &header, PGM_P host)
    {
        header += firebase_pgm_str_32; // "Host: "
        header += host;
        addNewLine(header);
    }

    void addContentTypeHeader(MB_String &header, PGM_P v)
    {
        header += firebase_pgm_str_33; // "Content-Type: "
        header += v;
        header += firebase_pgm_str_30; // "\r\n"
    }

    void addContentLengthHeader(MB_String &header, size_t len)
    {
        header += firebase_pgm_str_34; // "Content-Length: "
        header += len;
        addNewLine(header);
    }

    void addUAHeader(MB_String &header)
    {
        header += firebase_pgm_str_35; // "User-Agent: ESP\r\n"
    }

    void addConnectionHeader(MB_String &header, bool keepAlive)
    {
        header += keepAlive ? firebase_pgm_str_36 /* "Connection: keep-alive\r\n" */
                            : firebase_pgm_str_37 /* "Connection: close\r\n" */;
    }

    /* Append the string with first request line (HTTP method) */
    bool addRequestHeaderFirst(MB_String &header, firebase_request_method method)
    {
        bool post = false;
        switch (method)
        {
        case http_get:
            header += firebase_pgm_str_41; // "GET"
            break;
        case http_post:
            header += firebase_pgm_str_40; // "POST"
            post = true;
            break;

        case http_patch:
            header += firebase_pgm_str_38; // "PATCH"
            post = true;
            break;

        case http_delete:
            header += firebase_pgm_str_42; // "DELETE"
            break;

        case http_put:
            header += firebase_pgm_str_39; // "PUT"
            break;

        default:
            break;
        }

        if (method == http_get || method == http_post || method == http_patch || method == http_delete || method == http_put)
            header += firebase_pgm_str_9; // " "

        return post;
    }

    /* Append the string with last request line (HTTP version) */
    void addRequestHeaderLast(MB_String &header)
    {
        header += firebase_pgm_str_43; // " HTTP/1.1\r\n"
    }

    /* Append the string with first part of Authorization header */
    void addAuthHeaderFirst(MB_String &header, firebase_auth_token_type type)
    {
        header += firebase_pgm_str_44; // "Authorization: "
        if (type == token_type_oauth2_access_token)
            header += firebase_pgm_str_45; // "Bearer "
        else if (type == token_type_id_token || type == token_type_custom_token)
            header += firebase_pgm_str_46; // "Firebase "
        else
            header += firebase_pgm_str_47; // "key="
    }

    void parseRespHeader(StringHelper *sh, const MB_String &src, struct server_response_data_t &response)
    {
        int beginPos = 0;

        MB_String out;

        if (response.httpCode != -1)
        {

            sh->tokenSubString(src, response.connection,
                               firebase_pgm_str_48 /* "Connection: " */,
                               firebase_pgm_str_30 /* "\r\n" */, beginPos, 0, false);
            sh->tokenSubString(src, response.contentType,
                               firebase_pgm_str_33 /* "Content-Type: " */,
                               firebase_pgm_str_30 /* "\r\n" */, beginPos, 0, false);
            sh->tokenSubStringInt(src, response.contentLen,
                                  firebase_pgm_str_34 /* "Content-Length: " */,
                                  firebase_pgm_str_30 /* "\r\n" */, beginPos, 0, false);
            sh->tokenSubString(src, response.etag,
                               firebase_pgm_str_49 /* "ETag: " */,
                               firebase_pgm_str_30 /* "\r\n" */, beginPos, 0, false);
            response.payloadLen = response.contentLen;

            if (sh->tokenSubString(src, response.transferEnc,
                                   firebase_pgm_str_50 /* "Transfer-Encoding: " */,
                                   firebase_pgm_str_30 /* "\r\n" */, beginPos, 0, false) &&
                sh->compare(response.transferEnc, 0, firebase_pgm_str_51 /* "chunked" */))
                response.isChunkedEnc = true;

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK ||
                response.httpCode == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT ||
                response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT ||
                response.httpCode == FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY ||
                response.httpCode == FIREBASE_ERROR_HTTP_CODE_FOUND)
                sh->tokenSubString(src, response.location,
                                   firebase_pgm_str_52 /* "Location: " */,
                                   firebase_pgm_str_30 /* "\r\n" */, beginPos, 0, false);

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                response.noContent = true;
        }
    }

    int getStatusCode(StringHelper *sh, const MB_String &header, int &pos)
    {
        int code = 0;
        sh->tokenSubStringInt(header, code,
                              firebase_pgm_str_53 /* "HTTP/1.1 " */,
                              firebase_pgm_str_9 /* " " */, pos, 0, false);
        return code;
    }

    void setNumDataType(const MB_String &buf, int ofs, struct server_response_data_t &response, bool dec)
    {
        if (ofs < 0)
            return;

        int len = (int)buf.length();

        if (ofs >= len)
            return;

        if (response.payloadLen > 0 && response.payloadLen <= len && ofs < len && ofs + response.payloadLen <= len)
        {
            double d = atof(buf.substr(ofs, response.payloadLen).c_str());

            if (dec)
            {
                if (response.payloadLen <= 7)
                {
                    response.floatData = d;
                    response.dataType = firebase_data_type::d_float;
                }
                else
                {
                    response.doubleData = d;
                    response.dataType = firebase_data_type::d_double;
                }
            }
            else
            {
                if (d > 0x7fffffff)
                {
                    response.doubleData = d;
                    response.dataType = firebase_data_type::d_double;
                }
                else
                {
                    response.intData = (int)d;
                    response.dataType = firebase_data_type::d_integer;
                }
            }
        }
    }

    void parseRespPayload(StringHelper *sh, const MB_String &src, struct server_response_data_t &response, bool getOfs)
    {
        int payloadPos = 0;
        int payloadOfs = 0;

        MB_String out;

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)

        if (!response.isEvent && !response.noEvent)
        {
            if (sh->tokenSubString(src, response.eventType,
                                   firebase_rtdb_pgm_str_12 /* "event: " */,
                                   firebase_pgm_str_12 /* "\n" */, payloadPos, 0, true))
            {
                response.isEvent = true;
                payloadOfs = payloadPos;

                if (sh->tokenSubString(src, out,
                                       firebase_rtdb_pgm_str_13 /* "data: " */,
                                       firebase_pgm_str_12 /* "\n" */, payloadPos, 0, true))
                {
                    payloadOfs += strlen_P(firebase_rtdb_pgm_str_13 /* "data: " */);
                    payloadPos = payloadOfs;
                    response.hasEventData = true;

                    if (sh->tokenSubString(src, response.eventPath,
                                           firebase_pgm_str_54 /* "\"path\":\"" */,
                                           firebase_pgm_str_4 /* "\"" */, payloadPos, 0, true))
                    {
                        payloadOfs = payloadPos;

                        if (sh->tokenSubString(src, response.eventData,
                                               firebase_pgm_str_55 /* "\"data\":" */,
                                               firebase_pgm_str_12 /* "\n" */, payloadPos, 0, true))
                        {
                            response.eventData[response.eventData.length() - 1] = 0;
                            response.payloadLen = response.eventData.length();
                            payloadOfs += strlen_P(firebase_pgm_str_55 /* "\"data\":" */) + 1;
                            response.payloadOfs = payloadOfs;
                        }
                    }
                }
            }
        }

#endif

        if (src.length() < (size_t)payloadOfs)
            return;

        if (response.dataType == 0)
        {
            sh->tokenSubString(src, response.pushName,
                               firebase_pgm_str_56 /* "{\"name\":\"" */,
                               firebase_pgm_str_4 /* "\"" */, payloadPos, 0, true);

            if (sh->tokenSubString(src, out,
                                   firebase_pgm_str_57 /* "\"error\" : " */,
                                   firebase_pgm_str_4 /* "\"" */, payloadPos, 0, true))
            {
                FirebaseJson js;
                FirebaseJsonData d;
                js.setJsonData(src);
                js.get(d, pgm2Str(firebase_pgm_str_58 /* "error" */));
                if (d.success)
                    response.fbError = d.stringValue.c_str();
            }
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
            if (sh->compare(src, payloadOfs, firebase_rtdb_pgm_str_7 /* "\"blob,base64," */, true))
            {
                response.dataType = firebase_data_type::d_blob;
                if ((response.isEvent && response.hasEventData) || getOfs)
                {
                    if (response.eventData.length() > 0)
                    {
                        int dlen = response.eventData.length() - strlen_P(firebase_rtdb_pgm_str_7) - 1;
                        response.payloadLen = dlen;
                    }
                    response.payloadOfs += strlen_P(firebase_rtdb_pgm_str_7);
                    response.eventData.clear();
                }
            }
            else if (sh->compare(src, payloadOfs, firebase_rtdb_pgm_str_8 /* "\"file,base64," */, true))
            {
                response.dataType = firebase_data_type::d_file;
                if ((response.isEvent && response.hasEventData) || getOfs)
                {
                    if (response.eventData.length() > 0)
                    {
                        int dlen = response.eventData.length() - strlen_P(firebase_rtdb_pgm_str_8) - 1;
                        response.payloadLen = dlen;
                    }

                    response.payloadOfs += strlen_P(firebase_rtdb_pgm_str_8);
                    response.eventData.clear();
                }
            }
            else if (sh->compare(src, payloadOfs, firebase_pgm_str_4 /* "\"" */))
                response.dataType = firebase_data_type::d_string;
            else if (sh->compare(src, payloadOfs, firebase_pgm_str_10 /* "{" */))
                response.dataType = firebase_data_type::d_json;
            else if (sh->compare(src, payloadOfs, firebase_pgm_str_6 /* "[" */))
                response.dataType = firebase_data_type::d_array;
            else if (sh->compare(src, payloadOfs, firebase_pgm_str_19 /* "false" */) ||
                     sh->compare(src, payloadOfs, firebase_pgm_str_20 /* "true" */))
            {
                response.dataType = firebase_data_type::d_boolean;
                response.boolData = sh->compare(src, payloadOfs, firebase_pgm_str_20 /* "true" */);
            }
            else if (sh->compare(src, payloadOfs, firebase_pgm_str_59 /* "null" */))
                response.dataType = firebase_data_type::d_null;
            else
                setNumDataType(src, payloadOfs, response, src.find(pgm2Str(firebase_pgm_str_5 /* "." */), payloadOfs) != MB_String::npos);
#endif
        }
    }

    void getCustomHeaders(StringHelper *sh, MB_String &header, const MB_String &tokens)
    {
        if (tokens.length() > 0)
        {
            MB_VECTOR<MB_String> headers;
            sh->splitTk(tokens, headers, ",");
            for (size_t i = 0; i < headers.size(); i++)
            {
                size_t p1 = headers[i].find(F("X-Firebase-"));
                size_t p2 = headers[i].find(':');
                size_t p3 = headers[i].find(F("-ETag"));

                if (p1 != MB_String::npos && p2 != MB_String::npos && p2 > p1 && p3 == MB_String::npos)
                {
                    header += headers[i];
                    addNewLine(header);
                }
                headers[i].clear();
            }
            headers.clear();
        }
    }

    void initTCPSession(struct firebase_session_info_t &session)
    {

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
        if (session.con_mode == firebase_con_mode_rtdb)
            session.rtdb.raw.clear();
#endif

#if defined(ENABLE_FIRESTORE) || defined(FIREBASE_ENABLE_FIRESTORE)
        if (session.con_mode == firebase_con_mode_firestore)
            session.cfs.payload.clear();
#endif

#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
        if (session.con_mode == firebase_con_mode_functions)
            session.cfn.payload.clear();
#endif

#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)
        if (session.con_mode == firebase_con_mode_fcm)
            session.fcm.payload.clear();
#endif

#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
        if (session.con_mode == firebase_con_mode_gc_storage)
            session.gcs.payload.clear();
#endif

#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
        if (session.con_mode == firebase_con_mode_storage)
            session.fcs.files.items.clear();
#endif

        session.response.code = FIREBASE_ERROR_HTTP_CODE_UNDEFINED;
        session.content_length = -1;
        session.payload_length = 0;
        session.chunked_encoding = false;
        session.buffer_ovf = false;
    }

    void intTCPHandler(Client *client, struct firebase_tcp_response_handler_t &tcpHandler,
                       size_t defaultChunkSize, size_t respSize, MB_String *payload, bool isOTA)
    {
        // set the client before calling available
        tcpHandler.client = client;
        tcpHandler.payloadLen = 0;
        tcpHandler.payloadRead = 0;
        tcpHandler.chunkBufSize = tcpHandler.available(); // client must be set before calling
        tcpHandler.defaultChunkSize = respSize;
        tcpHandler.error.code = -1;
        tcpHandler.defaultChunkSize = defaultChunkSize;
        tcpHandler.bufferAvailable = 0;
        tcpHandler.header.clear();
        tcpHandler.dataTime = millis();
        tcpHandler.downloadOTA = isOTA;
        tcpHandler.payload = payload;
    }

    int readLine(Client *client, char *buf, int bufLen)
    {
        if (!client)
            return 0;

        int res = -1;
        char c = 0;
        int idx = 0;
        if (!client)
            return idx;
        while (client->available() && idx < bufLen)
        {
            if (!client)
                break;

            FBUtils::idle();

            res = client->read();
            if (res > -1)
            {
                c = (char)res;
                buf[idx++] = c;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readLine(Client *client, MB_String &buf)
    {
        if (!client)
            return 0;

        int res = -1;
        char c = 0;
        int idx = 0;
        if (!client)
            return idx;
        while (client->available())
        {
            if (!client)
                break;

            FBUtils::idle();

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

    // Returns -1 when complete
    int readChunkedData(StringHelper *sh, MB_FS *mbfs, Client *client, char *out1, MB_String *out2,
                        struct firebase_tcp_response_handler_t &tcpHandler)
    {
        if (!client)
            return 0;

        int bufLen = tcpHandler.chunkBufSize;
        char *buf = nullptr;
        int p1 = 0;
        int olen = 0;

        if (tcpHandler.chunkState.state == 0)
        {
            tcpHandler.chunkState.state = 1;
            tcpHandler.chunkState.chunkedSize = -1;
            tcpHandler.chunkState.dataLen = 0;

            MB_String s;
            int readLen = 0;

            if (out2)
                readLen = readLine(client, s);
            else if (out1)
            {
                buf = reinterpret_cast<char *>(mbfs->newP(bufLen));
                readLen = readLine(client, buf, bufLen);
            }

            if (readLen)
            {
                if (out1)
                    s = buf;

                p1 = sh->strpos(s.c_str(), (const char *)MBSTRING_FLASH_MCR(";"), 0);
                if (p1 == -1)
                    p1 = sh->strpos(s.c_str(), (const char *)MBSTRING_FLASH_MCR("\r\n"), 0);

                if (p1 != -1)
                {
                    if (out2)
                        tcpHandler.chunkState.chunkedSize = hex2int(s.substr(0, p1).c_str());
                    else if (out1)
                    {
                        char *temp = reinterpret_cast<char *>(mbfs->newP(p1 + 1));
                        memcpy(temp, buf, p1);
                        tcpHandler.chunkState.chunkedSize = hex2int(temp);
                        mbfs->delP(&temp);
                    }
                }

                // last chunk
                if (tcpHandler.chunkState.chunkedSize < 1)
                    olen = -1;
            }
            else
                tcpHandler.chunkState.state = 0;

            if (out1)
                mbfs->delP(&buf);
        }
        else
        {
            if (tcpHandler.chunkState.chunkedSize > -1)
            {
                MB_String s;
                int readLen = 0;

                if (out2)
                    readLen = readLine(client, s);
                else if (out1)
                {
                    buf = reinterpret_cast<char *>(mbfs->newP(bufLen));
                    readLen = readLine(client, buf, bufLen);
                }

                if (readLen > 0)
                {
                    // chunk may contain trailing
                    if (tcpHandler.chunkState.dataLen + readLen - 2 < tcpHandler.chunkState.chunkedSize)
                    {
                        tcpHandler.chunkState.dataLen += readLen;
                        if (out2)
                            *out2 += s;
                        else if (out1)
                            memcpy(out1, buf, readLen);

                        olen = readLen;
                    }
                    else
                    {
                        if (tcpHandler.chunkState.chunkedSize - tcpHandler.chunkState.dataLen > 0)
                        {
                            if (out2)
                                *out2 += s;
                            else if (out1)
                                memcpy(out1, buf, tcpHandler.chunkState.chunkedSize - tcpHandler.chunkState.dataLen);
                        }

                        tcpHandler.chunkState.dataLen = tcpHandler.chunkState.chunkedSize;
                        tcpHandler.chunkState.state = 0;
                        olen = readLen;
                    }
                }
                // if all chunks read, returns -1
                else if (tcpHandler.chunkState.dataLen == tcpHandler.chunkState.chunkedSize)
                    olen = -1;

                if (out1)
                    mbfs->delP(&buf);
            }
        }

        return olen;
    }

    bool readStatusLine(StringHelper *sh, MB_FS *mbfs, Client *client, struct firebase_tcp_response_handler_t &tcpHandler,
                        struct server_response_data_t &response)
    {
        tcpHandler.chunkIdx++;

        if (!tcpHandler.isHeader && tcpHandler.chunkIdx > 1)
            tcpHandler.pChunkIdx++;

        if (tcpHandler.chunkIdx > 1)
            return false;

        // the first chunk (line) can be http response status or already connected stream payload
        char *hChunk = reinterpret_cast<char *>(mbfs->newP(tcpHandler.chunkBufSize));
        int readLen = readLine(client, hChunk, tcpHandler.chunkBufSize);
        if (readLen > 0)
            tcpHandler.header += hChunk;

        int pos = 0;
        int status = getStatusCode(sh, hChunk, pos);
        if (status > 0)
        {
            // http response status
            tcpHandler.isHeader = true;
            response.httpCode = status;
        }

        mbfs->delP(&hChunk);
        return true;
    }

    bool readHeader(StringHelper *sh, MB_FS *mbfs, Client *client, struct firebase_tcp_response_handler_t &tcpHandler,
                    struct server_response_data_t &response)
    {
        // do not check of the config here to allow legacy fcm to work

        char *hChunk = reinterpret_cast<char *>(mbfs->newP(tcpHandler.chunkBufSize));
        int readLen = readLine(client, hChunk, tcpHandler.chunkBufSize);

        // check is it the end of http header (\n or \r\n)?
        if ((readLen == 1 && hChunk[0] == '\r') || (readLen == 2 && hChunk[0] == '\r' && hChunk[1] == '\n'))
            tcpHandler.headerEnded = true;

        if (tcpHandler.headerEnded)
        {
            // parse header string to get the header field
            tcpHandler.isHeader = false;
            parseRespHeader(sh, tcpHandler.header, response);
        }
        // accumulate the remaining header field
        else if (readLen > 0)
            tcpHandler.header += hChunk;

        mbfs->delP(&hChunk);
        return tcpHandler.headerEnded;
    }
};

class Base64Helper
{
public:
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

    size_t encodedLength(size_t len)
    {
        return ((len + 2) / 3 * 4) + 1;
    }

    int decodedLen(const char *src)
    {
        int len = strlen(src), i = len - 1, pad = 0;
        if (len < 4)
            return 0;
        while (i > 0 && src[i--] == '=')
        {
            pad++;
        }
        return (3 * (len / 4)) - pad;
    }

    unsigned char *creatBase64EncBuffer(MB_FS *mbfs, bool isURL)
    {
        unsigned char *base64EncBuf = reinterpret_cast<unsigned char *>(mbfs->newP(65));
        strcpy_P((char *)base64EncBuf, (char *)firebase_base64_table);
        if (isURL)
        {
            base64EncBuf[62] = '-';
            base64EncBuf[63] = '_';
        }
        return base64EncBuf;
    }

    bool updateWrite(uint8_t *data, size_t len)
    {
#if (defined(ENABLE_OTA_FIRMWARE_UPDATE) || defined(FIREBASE_ENABLE_OTA_FIRMWARE_UPDATE)) && (defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB) || defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE))
#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)
        return Update.write(data, len) == len;
#endif
#endif
        return false;
    }

    unsigned char *creatBase64DecBuffer(MB_FS *mbfs)
    {
        unsigned char *base64DecBuf = reinterpret_cast<unsigned char *>(mbfs->newP(256, false));
        memset(base64DecBuf, 0x80, 256);
        for (size_t i = 0; i < sizeof(firebase_base64_table) - 1; i++)
            base64DecBuf[firebase_base64_table[i]] = (unsigned char)i;
        base64DecBuf['='] = 0;
        return base64DecBuf;
    }

    template <typename T = uint8_t>
    bool writeOutput(MB_FS *mbfs, firebase_base64_io_t<T> &out)
    {
        size_t write = out.bufWrite;
        out.bufWrite = 0;

        if (write == 0)
            return true;

        if (out.outC && out.outC->write((uint8_t *)out.outT, write) == write)
            return true;
        else if (out.filetype != mb_fs_mem_storage_type_undefined && mbfs->write(mbfs_type out.filetype,
                                                                                 (uint8_t *)out.outT, write) == (int)write)
            return true;
#if defined(OTA_UPDATE_ENABLED)
        else if (out.ota && updateWrite((uint8_t *)out.outT, write))
            return true;
#endif
        return false;
    }

    template <typename T = uint8_t>
    bool setOutput(MB_FS *mbfs, uint8_t val, firebase_base64_io_t<T> &out, T **pos)
    {
        if (out.outT)
        {
            if (out.ota || out.outC || out.filetype != mb_fs_mem_storage_type_undefined)
            {
                out.outT[out.bufWrite++] = val;
                if (out.bufWrite == (int)out.bufLen && !writeOutput(mbfs, out))
                    return false;
            }
            else
                *(*pos)++ = (T)(val);
        }
        else if (out.outL)
            out.outL->push_back(val);

        return true;
    }

    template <typename T>
    bool decode(MB_FS *mbfs, unsigned char *base64DecBuf, const char *src, size_t len, firebase_base64_io_t<T> &out)
    {
        // the maximum chunk size that writes to output is limited by out.bufLen, the minimum is depending on the source length
        bool ret = false;
        unsigned char *block = reinterpret_cast<unsigned char *>(mbfs->newP(4, false));
        unsigned char temp;
        size_t i, count;
        int pad = 0;
        size_t extra_pad;
        T *pos = out.outT ? (T *)&out.outT[0] : nullptr;
        if (len == 0)
            len = strlen(src);

        count = 0;

        for (i = 0; i < len; i++)
        {
            if ((uint8_t)base64DecBuf[(uint8_t)src[i]] != 0x80)
                count++;
        }

        if (count == 0)
            goto skip;

        extra_pad = (4 - count % 4) % 4;
        count = 0;
        for (i = 0; i < len + extra_pad; i++)
        {
            unsigned char val;

            if (i >= len)
                val = '=';
            else
                val = src[i];

            temp = base64DecBuf[val];

            if (temp == 0x80)
                continue;

            if (val == '=')
                pad++;

            block[count] = temp;
            count++;
            if (count == 4)
            {

                setOutput(mbfs, (block[0] << 2) | (block[1] >> 4), out, &pos);

                count = 0;
                if (pad)
                {
                    if (pad == 1)
                        setOutput(mbfs, (block[1] << 4) | (block[2] >> 2), out, &pos);
                    else if (pad > 2)
                        goto skip;

                    break;
                }
                else
                {
                    setOutput(mbfs, (block[1] << 4) | (block[2] >> 2), out, &pos);
                    setOutput(mbfs, (block[2] << 6) | block[3], out, &pos);
                }
            }
        }

        // write remaining
        if (out.bufWrite > 0 && !writeOutput(mbfs, out))
            goto skip;

        ret = true;

    skip:
        mbfs->delP(&block);
        return ret;
    }

    template <typename T>
    bool encodeLast(MB_FS *mbfs, unsigned char *base64EncBuf, const unsigned char *in, size_t len,
                    firebase_base64_io_t<T> &out, T **pos)
    {
        if (len > 2)
            return false;

        if (!setOutput(mbfs, base64EncBuf[in[0] >> 2], out, pos))
            return false;

        if (len == 1)
        {
            if (!setOutput(mbfs, base64EncBuf[(in[0] & 0x03) << 4], out, pos))
                return false;
            if (!setOutput(mbfs, '=', out, pos))
                return false;
        }
        else
        {
            if (!setOutput(mbfs, base64EncBuf[((in[0] & 0x03) << 4) | (in[1] >> 4)], out, pos))
                return false;
            if (!setOutput(mbfs, base64EncBuf[(in[1] & 0x0f) << 2], out, pos))
                return false;
        }

        if (!setOutput(mbfs, '=', out, pos))
            return false;

        return true;
    }

    template <typename T>
    bool encode(MB_FS *mbfs, unsigned char *base64EncBuf, uint8_t *src, size_t len,
                firebase_base64_io_t<T> &out, bool writeAllRemaining = true)
    {
        const unsigned char *end, *in;

        T *pos = out.outT ? (T *)&out.outT[0] : nullptr;
        in = src;
        end = src + len;

        while (end - in >= 3)
        {
            if (!setOutput(mbfs, base64EncBuf[in[0] >> 2], out, &pos))
                return false;
            if (!setOutput(mbfs, base64EncBuf[((in[0] & 0x03) << 4) | (in[1] >> 4)], out, &pos))
                return false;
            if (!setOutput(mbfs, base64EncBuf[((in[1] & 0x0f) << 2) | (in[2] >> 6)], out, &pos))
                return false;
            if (!setOutput(mbfs, base64EncBuf[in[2] & 0x3f], out, &pos))
                return false;
            in += 3;
        }

        if (end - in && !encodeLast(mbfs, base64EncBuf, in, end - in, out, &pos))
            return false;

        if (writeAllRemaining && out.bufWrite > 0 && !writeOutput(mbfs, out))
            return false;

        return true;
    }
    template <typename T>
    bool decodeToArray(MB_FS *mbfs, const MB_String &src, MB_VECTOR<T> &val)
    {
        firebase_base64_io_t<T> out;
        out.outL = &val;
        unsigned char *base64DecBuf = creatBase64DecBuffer(mbfs);
        bool ret = decode<T>(mbfs, base64DecBuf, src.c_str(), src.length(), out);
        mbfs->delP(&base64DecBuf);
        return ret;
    }

    bool decodeToFile(MB_FS *mbfs, const char *src, size_t len, mbfs_file_type type)
    {
        firebase_base64_io_t<uint8_t> out;
        out.filetype = type;
        uint8_t *buf = reinterpret_cast<uint8_t *>(mbfs->newP(out.bufLen));
        out.outT = buf;
        unsigned char *base64DecBuf = creatBase64DecBuffer(mbfs);
        bool ret = decode<uint8_t>(mbfs, base64DecBuf, src, strlen(src), out);
        mbfs->delP(&buf);
        mbfs->delP(&base64DecBuf);
        return ret;
    }

    void encodeUrl(MB_FS *mbfs, char *encoded, unsigned char *string, size_t len)
    {
        size_t i;
        char *p = encoded;
        unsigned char *base64EncBuf = creatBase64EncBuffer(mbfs, true);

        for (i = 0; i < len - 2; i += 3)
        {
            *p++ = base64EncBuf[(string[i] >> 2) & 0x3F];
            *p++ = base64EncBuf[((string[i] & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = base64EncBuf[((string[i + 1] & 0xF) << 2) | ((int)(string[i + 2] & 0xC0) >> 6)];
            *p++ = base64EncBuf[string[i + 2] & 0x3F];
        }

        if (i < len)
        {
            *p++ = base64EncBuf[(string[i] >> 2) & 0x3F];
            if (i == (len - 1))
                *p++ = base64EncBuf[((string[i] & 0x3) << 4)];
            else
            {
                *p++ = base64EncBuf[((string[i] & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
                *p++ = base64EncBuf[((string[i + 1] & 0xF) << 2)];
            }
        }

        *p++ = '\0';

        mbfs->delP(&base64EncBuf);
    }

    MB_String encodeToString(MB_FS *mbfs, uint8_t *src, size_t len)
    {
        MB_String str;
        char *encoded = reinterpret_cast<char *>(mbfs->newP(encodedLength(len) + 1));
        firebase_base64_io_t<char> out;
        out.outT = encoded;
        unsigned char *base64EncBuf = creatBase64EncBuffer(mbfs, false);
        if (encode<char>(mbfs, base64EncBuf, (uint8_t *)src, len, out))
            str = encoded;
        mbfs->delP(&encoded);
        mbfs->delP(&base64EncBuf);
        return str;
    }

    bool encodeToClient(Client *client, MB_FS *mbfs, size_t bufSize, uint8_t *data, size_t len)
    {
        firebase_base64_io_t<uint8_t> out;
        out.outC = client;
        uint8_t *buf = reinterpret_cast<uint8_t *>(mbfs->newP(out.bufLen));
        out.outT = buf;
        unsigned char *base64EncBuf = creatBase64EncBuffer(mbfs, false);
        bool ret = encode<uint8_t>(mbfs, base64EncBuf, (uint8_t *)data, len, out);
        mbfs->delP(&buf);
        mbfs->delP(&base64EncBuf);
        return ret;
    }
};

class OtaHelper
{
public:
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

    bool decodeBase64OTA(Base64Helper *bh, MB_FS *mbfs, const char *src, size_t len, int &code)
    {
        bool ret = true;
        firebase_base64_io_t<uint8_t> out;
        uint8_t *buf = reinterpret_cast<uint8_t *>(mbfs->newP(out.bufLen));
        out.ota = true;
        out.outT = buf;
        unsigned char *base64DecBuf = bh->creatBase64DecBuffer(mbfs);
        if (!bh->decode<uint8_t>(mbfs, base64DecBuf, src, strlen(src), out))
        {
            code = FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED;
            ret = false;
        }
        mbfs->delP(&buf);
        mbfs->delP(&base64DecBuf);
        return ret;
    }
};

class Utils
{
public:
    int ishex(int x)
    {
        return (x >= '0' && x <= '9') ||
               (x >= 'a' && x <= 'f') ||
               (x >= 'A' && x <= 'F');
    }

    char from_hex(char ch)
    {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
    }

    void createDirs(MB_String dirs, firebase_mem_storage_type storageType)
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
    MB_String getBoundary(MB_FS *mbfs, size_t len)
    {
        MB_String temp = firebase_boundary_table;
        char *buf = reinterpret_cast<char *>(mbfs->newP(len));
        if (len)
        {
            --len;
            buf[0] = temp[0];
            buf[1] = temp[1];
            for (size_t n = 2; n < len; n++)
            {
                int key = rand() % (int)(temp.length() - 1);
                buf[n] = temp[key];
            }
            buf[len] = '\0';
        }
        MB_String out = buf;
        mbfs->delP(&buf);
        return out;
    }

    bool boolVal(const MB_String &v)
    {
        return v.find(pgm2Str(firebase_pgm_str_20 /* "true" */)) != MB_String::npos;
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

    uint16_t calCRC(MB_FS *mbfs, const char *buf)
    {
        return mbfs->calCRC(buf);
    }

    void makePath(MB_String &path)
    {
        if (path.length() > 0)
        {
            if (path[0] != '/')
                path.prepend('/');
        }
    }

    MB_String makeFCMMsgPath(PGM_P sub = NULL)
    {
        MB_String path = firebase_pgm_str_60; // "msg"
        path += firebase_pgm_str_1;           // "/"
        if (sub)
            path += sub;
        return path;
    }

#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)

    MB_String makeFCMMessagePath(PGM_P sub = NULL)
    {
        MB_String path = firebase_fcm_pgm_str_68; // "message"
        path += firebase_pgm_str_1;               // "/"
        if (sub)
            path += sub;
        return path;
    }

    void addFCMNotificationPath(MB_String &path, PGM_P sub = NULL)
    {
        path += firebase_fcm_pgm_str_67; // "notification"
        path += firebase_pgm_str_1;      // "/"
        if (sub)
            path += sub;
    }

    void addFCMAndroidPath(MB_String &path, PGM_P sub = NULL)
    {
        path += firebase_fcm_pgm_str_69; // "android"
        path += firebase_pgm_str_1;      // "/"
        if (sub)
            path += sub;
    }

    void addFCMWebpushPath(MB_String &path, PGM_P sub = NULL)
    {
        path += firebase_fcm_pgm_str_70; // "webpush";
        path += firebase_pgm_str_1;      // "/"
        if (sub)
            path += sub;
    }

    void addFCMApnsPath(MB_String &path, PGM_P sub = NULL)
    {
        path += firebase_fcm_pgm_str_71; // "apns";
        path += firebase_pgm_str_1;      // "/"
        if (sub)
            path += sub;
    }

    MB_String makeFCMNotificationPath(PGM_P sub = NULL)
    {
        MB_String path = firebase_fcm_pgm_str_67; // "notification"
        path += firebase_pgm_str_1;               // "/"
        if (sub)
            path += sub;
        return path;
    }

#endif

#if defined(ENABLE_FIRESTORE) || defined(FIREBASE_ENABLE_FIRESTORE)
    MB_String makeDocPath(struct firebase_firestore_req_t &req, const MB_String &projectId)
    {
        MB_String str = firebase_func_pgm_str_47; // "projects/"
        str += req.projectId.length() == 0 ? projectId : req.projectId;
        str += firebase_cfs_pgm_str_32; // "/databases/"
        str += req.databaseId.length() > 0 ? req.databaseId : firebase_cfs_pgm_str_33 /* "(default)" */;
        str += firebase_cfs_pgm_str_21; // "/documents"
        return str;
    }
#endif
    size_t getUploadBufSize(FirebaseConfig *config, firebase_con_mode mode)
    {
        int bufLen = 0;
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
        if (mode == firebase_con_mode_rtdb)
            bufLen = config->rtdb.upload_buffer_size;
#endif
#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
        if (mode == firebase_con_mode_functions)
            bufLen = config->functions.upload_buffer_size;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
        if (mode == firebase_con_mode_gc_storage)
            bufLen = config->gcs.upload_buffer_size;
#endif
#if defined(ENABLE_FB_STORAGE)
        if (mode == firebase_con_mode_storage)
            bufLen = config->fcs.upload_buffer_size;
#endif

        if (bufLen < 512)
            bufLen = 512;

        if (bufLen > 1024 * 16)
            bufLen = 1024 * 16;

        return bufLen;
    }

    bool isNoContent(server_response_data_t *response)
    {
        return !response->isChunkedEnc && response->contentLen == 0;
    }

    bool isResponseTimeout(firebase_tcp_response_handler_t *tcpHandler, bool &complete)
    {
        if (millis() - tcpHandler->dataTime > 5000)
        {
            // Read all remaining data
            tcpHandler->client->flush();
            complete = true;
        }
        return complete;
    }

    bool isResponseComplete(firebase_tcp_response_handler_t *tcpHandler, server_response_data_t *response, bool &complete, bool check = true)
    {
        if (check && !response->isChunkedEnc &&
            (tcpHandler->bufferAvailable < 0 || tcpHandler->payloadRead >= response->contentLen))
        {
            complete = true;
            return true;
        }
        return false;
    }

    bool isChunkComplete(firebase_tcp_response_handler_t *tcpHandler, server_response_data_t *response, bool &complete)
    {
        if (response->isChunkedEnc && tcpHandler->bufferAvailable < 0)
        {
            // Read all remaining data
            tcpHandler->client->flush();
            complete = true;
            return true;
        }
        return false;
    }
};

#endif