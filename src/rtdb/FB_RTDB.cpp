/**
 * Google's Firebase Realtime Database class, FB_RTDB.cpp version 1.0.18
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created June 10, 2021
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

#ifndef FIREBASE_RTDB_CPP
#define FIREBASE_RTDB_CPP
#include <Arduino.h>
#include "FB_RTDB.h"

FB_RTDB::FB_RTDB()
{
}

FB_RTDB::~FB_RTDB()
{
}

void FB_RTDB::begin(UtilsClass *u)
{
    ut = u;
}

void FB_RTDB::end(FirebaseData *fbdo)
{
    endStream(fbdo);
    removeStreamCallback(fbdo);
    fbdo->clear();
}

void FB_RTDB::setReadTimeout(FirebaseData *fbdo, int millisec)
{
    if (millisec <= 900000)
        fbdo->_ss.rtdb.read_tmo = millisec;
}

void FB_RTDB::setwriteSizeLimit(FirebaseData *fbdo, const char *size)
{
    fbdo->_ss.rtdb.write_limit = size;
}

bool FB_RTDB::getRules(FirebaseData *fbdo)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    ut->appendP(req.path, fb_esp_pgm_str_103, true);
    req.method = m_read_rules;
    req.data.type = d_json;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::setRules(FirebaseData *fbdo, const char *rules)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    ut->appendP(req.path, fb_esp_pgm_str_103, true);
    req.method = m_set_rules;
    req.payload = rules;
    req.data.type = d_json;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::setQueryIndex(FirebaseData *fbdo, const char *path, const char *node, const char *databaseSecret)
{
    return int_setQueryIndex(fbdo, path, node, databaseSecret);
}

bool FB_RTDB::removeQueryIndex(FirebaseData *fbdo, const char *path, const char *databaseSecret)
{
    return int_setQueryIndex(fbdo, path, "", databaseSecret);
}

bool FB_RTDB::int_setQueryIndex(FirebaseData *fbdo, const char *path, const char *node, const char *databaseSecret)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    std::string s;
    bool ret = false;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
    {
        Signer.config->signer.tokens.legacy_token = databaseSecret;
        Signer.config->signer.tokens.token_type = token_type_legacy_token;
        Signer.handleToken();
    }

    if (getRules(fbdo))
    {
        ret = true;
        FirebaseJsonData data;
        FirebaseJson *json = fbdo->jsonObjectPtr();
        bool ruleExisted = false;

        s.clear();
        ut->appendP(s, fb_esp_pgm_str_550);
        if (path[0] != '/')
            s += '/';
        s += path;
        ut->appendP(s, fb_esp_pgm_str_551);

        json->get(data, s.c_str());

        if (data.success && strcmp(data.stringValue.c_str(), node) == 0)
            ruleExisted = true;

        if (strlen(node) == 0)
            json->remove(s.c_str());
        else
            json->set(s.c_str(), node);

        if (!ruleExisted || (ruleExisted && strlen(node) == 0))
        {
            String rules;
            json->toString(rules, true);
            ret = setRules(fbdo, rules.c_str());
        }

        json->clear();
    }

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
    {
        if (tk != token_type_legacy_token)
            Signer.config->signer.tokens.legacy_token.clear();

        Signer.config->signer.tokens.token_type = tk;
        Signer.handleToken();
    }

    std::string().swap(s);

    return ret;
}

bool FB_RTDB::setReadWriteRules(FirebaseData *fbdo, const char *path, const char *var, const char *readVal, const char *writeVal, const char *databaseSecret)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    std::string s;
    bool ret = false;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
    {
        Signer.config->signer.tokens.legacy_token = databaseSecret;
        Signer.config->signer.tokens.token_type = token_type_legacy_token;
        Signer.handleToken();
    }

    if (getRules(fbdo))
    {
        ret = true;
        FirebaseJsonData data;
        FirebaseJson &json = fbdo->jsonObject();
        bool rd = false, wr = false;

        std::string s;
        ut->appendP(s, fb_esp_pgm_str_550);
        if (path[0] != '/')
            ut->appendP(s, fb_esp_pgm_str_1);
        s += path;
        ut->appendP(s, fb_esp_pgm_str_1);
        s += var;

        if (strlen(readVal) > 0)
        {
            rd = true;
            std::string r = s;
            ut->appendP(r, fb_esp_pgm_str_1);
            ut->appendP(r, fb_esp_pgm_str_552);
            json.get(data, r.c_str());
            if (data.success)
                if (strcmp(data.stringValue.c_str(), readVal) == 0)
                    rd = false;
        }

        if (strlen(writeVal) > 0)
        {
            wr = true;
            std::string w = s;
            ut->appendP(w, fb_esp_pgm_str_1);
            ut->appendP(w, fb_esp_pgm_str_553);
            json.get(data, w.c_str());
            if (data.success)
                if (strcmp(data.stringValue.c_str(), writeVal) == 0)
                    wr = false;
        }

        //modify if the rules changed or does not exist.
        if (wr || rd)
        {
            FirebaseJson js;
            std::string r, w;
            ut->appendP(r, fb_esp_pgm_str_552);
            ut->appendP(w, fb_esp_pgm_str_553);
            if (rd)
                js.add(r.c_str(), readVal);

            if (wr)
                js.add(w.c_str(), writeVal);

            json.set(s.c_str(), js);
            String rules;
            json.toString(rules, true);
            ret = setRules(fbdo, rules.c_str());
        }

        json.clear();
    }

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
    {
        if (tk != token_type_legacy_token)
            Signer.config->signer.tokens.legacy_token.clear();

        Signer.config->signer.tokens.token_type = tk;
        Signer.handleToken();
    }

    std::string().swap(s);
    return ret;
}

bool FB_RTDB::pathExisted(FirebaseData *fbdo, const char *path)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return !fbdo->_ss.rtdb.path_not_found;
    else
        return false;
}

String FB_RTDB::getETag(FirebaseData *fbdo, const char *path)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return fbdo->_ss.rtdb.resp_etag.c_str();
    else
        return "";
}

bool FB_RTDB::getShallowData(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get_shallow;
    req.data.type = d_string;
    return handleRequest(fbdo, &req);
}

void FB_RTDB::enableClassicRequest(FirebaseData *fbdo, bool enable)
{
    fbdo->_ss.classic_request = enable;
}

bool FB_RTDB::setPriority(FirebaseData *fbdo, const char *path, float priority)
{
    return int_setPriority(fbdo, path, priority, false);
}

bool FB_RTDB::setPriorityAsync(FirebaseData *fbdo, const char *path, float priority)
{
    return int_setPriority(fbdo, path, priority, true);
}

bool FB_RTDB::int_setPriority(FirebaseData *fbdo, const char *path, float priority, bool async)
{
    char *num = ut->floatStr(priority);
    ut->trimDigits(num);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    ut->appendP(req.path, fb_esp_pgm_str_156);
    req.method = m_set_priority;
    req.data.type = d_float;
    req.async = async;
    req.payload = num;
    req.queue = false;
    bool ret = processRequest(fbdo, &req);
    ut->delS(num);
    return ret;
}

bool FB_RTDB::getPriority(FirebaseData *fbdo, const char *path)
{
    char *tmp = ut->strP(fb_esp_pgm_str_156);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.path += tmp;
    req.method = m_get_priority;
    req.data.type = d_float;
    req.queue = false;
    bool ret = processRequest(fbdo, &req);
    ut->delS(tmp);
    return ret;
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, int intValue)
{
    return pushInt(fbdo, path, intValue);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, int intValue)
{
    return pushIntAsync(fbdo, path, intValue);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    return pushInt(fbdo, path, intValue, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    return pushIntAsync(fbdo, path, intValue, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, float floatValue)
{
    return pushFloat(fbdo, path, floatValue);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, float floatValue)
{
    return pushFloatAsync(fbdo, path, floatValue);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    return pushFloat(fbdo, path, floatValue, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    return pushFloatAsync(fbdo, path, floatValue, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return pushDouble(fbdo, path, doubleValue);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return pushDoubleAsync(fbdo, path, doubleValue);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    return pushDouble(fbdo, path, doubleValue, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    return pushDoubleAsync(fbdo, path, doubleValue, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return pushBool(fbdo, path, boolValue);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return pushBoolAsync(fbdo, path, boolValue);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    return pushBool(fbdo, path, boolValue, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    return pushBoolAsync(fbdo, path, boolValue, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, const char *stringValue)
{
    return pushString(fbdo, path, stringValue);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, const char *stringValue)
{
    return pushStringAsync(fbdo, path, stringValue);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return pushString(fbdo, path, stringValue);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return pushStringAsync(fbdo, path, stringValue);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, const char *stringValue, float priority)
{
    return pushString(fbdo, path, stringValue, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, const char *stringValue, float priority)
{
    return pushStringAsync(fbdo, path, stringValue, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    return pushString(fbdo, path, stringValue, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    return pushStringAsync(fbdo, path, stringValue, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return pushJSON(fbdo, path, json);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return pushJSONAsync(fbdo, path, json);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    return pushJSON(fbdo, path, json, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    return pushJSONAsync(fbdo, path, json, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return pushArray(fbdo, path, arr);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return pushArrayAsync(fbdo, path, arr);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    return pushArray(fbdo, path, arr, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    return pushArrayAsync(fbdo, path, arr, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return pushBlob(fbdo, path, blob, size);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return pushBlobAsync(fbdo, path, blob, size);
}

bool FB_RTDB::push(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    return pushBlob(fbdo, path, blob, size, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    return pushBlobAsync(fbdo, path, blob, size, priority);
}

bool FB_RTDB::push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return pushFile(fbdo, storageType, path, fileName);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return pushFileAsync(fbdo, storageType, path, fileName);
}

bool FB_RTDB::push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    return pushFile(fbdo, storageType, path, fileName, priority);
}

bool FB_RTDB::pushAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    return pushFileAsync(fbdo, storageType, path, fileName, priority);
}

template <typename T>
bool FB_RTDB::push(FirebaseData *fbdo, const char *path, T value)
{
    if (std::is_same<T, int>::value)
        return pushInt(fbdo, path, value);
    else if (std::is_same<T, double>::value)
        return pushDouble(fbdo, path, value);
    else if (std::is_same<T, bool>::value)
        return pushBool(fbdo, path, value);
    else if (std::is_same<T, const char *>::value)
        return pushString(fbdo, path, value);
    else if (std::is_same<T, const String &>::value)
        return pushString(fbdo, path, value);
    else if (std::is_same<T, FirebaseJson &>::value)
        return pushJson(fbdo, path, value);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return pushArray(fbdo, path, value);
}

template <typename T>
bool FB_RTDB::push(FirebaseData *fbdo, const char *path, T value, size_t size)
{
    if (std::is_same<T, uint8_t *>::value)
        return pushBlob(fbdo, path, value, size);
}

template <typename T>
bool FB_RTDB::push(FirebaseData *fbdo, const char *path, T value, float priority)
{
    if (std::is_same<T, int>::value)
        return pushInt(fbdo, path, value, priority);
    else if (std::is_same<T, double>::value)
        return pushDouble(fbdo, path, value, priority);
    else if (std::is_same<T, bool>::value)
        return pushBool(fbdo, path, value, priority);
    else if (std::is_same<T, const char *>::value)
        return pushString(fbdo, path, value, priority);
    else if (std::is_same<T, const String &>::value)
        return pushString(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJson &>::value)
        return pushJson(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return pushArray(fbdo, path, value, priority);
}

template <typename T>
bool FB_RTDB::push(FirebaseData *fbdo, const char *path, T value, size_t size, float priority)
{
    if (std::is_same<T, uint8_t *>::value)
        return pushBlob(fbdo, path, value, size, priority, false);
}

bool FB_RTDB::pushInt(FirebaseData *fbdo, const char *path, int intValue)
{
    return int_pushInt(fbdo, path, intValue, false, "", false);
}

bool FB_RTDB::pushIntAsync(FirebaseData *fbdo, const char *path, int intValue)
{
    return int_pushInt(fbdo, path, intValue, false, "", true);
}

bool FB_RTDB::pushInt(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushInt(fbdo, path, intValue, false, "", false);
    ut->delS(buf);
    return ret;
}
bool FB_RTDB::pushIntAsync(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushInt(fbdo, path, intValue, false, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushInt(FirebaseData *fbdo, const char *path, int intValue, bool queue, const char *priority, bool async)
{
    char *buf = ut->intStr(intValue);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_integer;
    req.payload = buf;
    req.async = async;
    req.queue = queue;
    if (strlen(priority) > 0)
        req.priority = priority;
    req.etag = "";
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushFloat(FirebaseData *fbdo, const char *path, float floatValue)
{
    return int_pushFloat(fbdo, path, floatValue, false, "", false);
}

bool FB_RTDB::pushFloatAsync(FirebaseData *fbdo, const char *path, float floatValue)
{
    return int_pushFloat(fbdo, path, floatValue, false, "", true);
}

bool FB_RTDB::pushFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushFloat(fbdo, path, floatValue, false, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushFloatAsync(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushFloat(fbdo, path, floatValue, false, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushFloat(FirebaseData *fbdo, const char *path, float floatValue, bool queue, const char *priority, bool async)
{
    char *buf = ut->floatStr(floatValue);
    ut->trimDigits(buf);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_float;
    req.async = async;
    req.payload = buf;
    req.queue = queue;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushDouble(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return int_pushDouble(fbdo, path, doubleValue, false, "", false);
}

bool FB_RTDB::pushDoubleAsync(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return int_pushDouble(fbdo, path, doubleValue, false, "", true);
}

bool FB_RTDB::pushDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushDouble(fbdo, path, doubleValue, false, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushDoubleAsync(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushDouble(fbdo, path, doubleValue, false, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushDouble(FirebaseData *fbdo, const char *path, double doubleValue, bool queue, const char *priority, bool async)
{
    char *buf = ut->doubleStr(doubleValue);
    ut->trimDigits(buf);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_double;
    req.async = async;
    req.payload = buf;
    req.queue = queue;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushBool(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return int_pushBool(fbdo, path, boolValue, false, "", false);
}

bool FB_RTDB::pushBoolAsync(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return int_pushBool(fbdo, path, boolValue, false, "", true);
}

bool FB_RTDB::pushBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushBool(fbdo, path, boolValue, false, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushBoolAsync(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushBool(fbdo, path, boolValue, false, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushBool(FirebaseData *fbdo, const char *path, bool boolValue, bool queue, const char *priority, bool async)
{
    char *buf = ut->boolStr(boolValue);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_boolean;
    req.async = async;
    req.payload = buf;
    req.queue = queue;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushString(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return int_pushString(fbdo, path, stringValue, "", false);
}

bool FB_RTDB::pushStringAsync(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return int_pushString(fbdo, path, stringValue, "", true);
}

bool FB_RTDB::pushString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushString(fbdo, path, stringValue, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushStringAsync(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushString(fbdo, path, stringValue, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushString(FirebaseData *fbdo, const char *path, const String &stringValue, const char *priority, bool async)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_string;
    req.async = async;
    req.payload = stringValue.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_pushJSON(fbdo, path, json, "", false);
}

bool FB_RTDB::pushJSONAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_pushJSON(fbdo, path, json, "", true);
}

bool FB_RTDB::pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushJSON(fbdo, path, json, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushJSONAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushJSON(fbdo, path, json, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *priority, bool async)
{
    std::string s;
    json->int_toStdString(s);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_json;
    req.async = async;
    req.payload = s.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    std::string().swap(s);
    return ret;
}

bool FB_RTDB::pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return int_pushArray(fbdo, path, arr, "", false);
}

bool FB_RTDB::pushArrayAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return int_pushArray(fbdo, path, arr, "", true);
}

bool FB_RTDB::pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushArray(fbdo, path, arr, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushArrayAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushArray(fbdo, path, arr, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *priority, bool async)
{
    String s;
    arr->toString(s);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_post;
    req.data.type = d_array;
    req.async = async;
    req.payload = s.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    return ret;
}

bool FB_RTDB::pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return int_pushBlob(fbdo, path, blob, size, false, "", false);
}

bool FB_RTDB::pushBlobAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return int_pushBlob(fbdo, path, blob, size, false, "", true);
}

bool FB_RTDB::pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushBlob(fbdo, path, blob, size, false, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushBlobAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushBlob(fbdo, path, blob, size, false, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, bool queue, const char *priority, bool async)
{
    if (fbdo->_ss.rtdb.max_blob_size < size)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE;
        return false;
    }

    std::string base64Str;
    ut->appendP(base64Str, fb_esp_pgm_str_92, true);
    base64Str += ut->encodeBase64Str((const unsigned char *)blob, size);
    ut->appendP(base64Str, fb_esp_pgm_str_3);
    struct fb_esp_rtdb_request_info_t req;
    req.method = m_post;
    req.data.type = d_blob;
    req.path = path;
    req.async = async;
    req.payload = base64Str.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    std::string().swap(base64Str);
    return ret;
}

bool FB_RTDB::pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return int_pushFile(fbdo, storageType, path, fileName, "", false);
}

bool FB_RTDB::pushFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return int_pushFile(fbdo, storageType, path, fileName, "", true);
}

bool FB_RTDB::pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushFile(fbdo, storageType, path, fileName, buf, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::pushFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_pushFile(fbdo, storageType, path, fileName, buf, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *priority, bool async)
{
    struct fb_esp_rtdb_request_info_t req;
    req.storageType = storageType;
    req.method = m_post;
    req.data.type = d_file;
    req.path = path;
    req.filename = fileName;
    req.queue = false;
    req.async = async;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    return ret;
}

bool FB_RTDB::pushTimestamp(FirebaseData *fbdo, const char *path)
{
    return int_pushTimestamp(fbdo, path, false);
}

bool FB_RTDB::pushTimestampAsync(FirebaseData *fbdo, const char *path)
{
    return int_pushTimestamp(fbdo, path, true);
}

bool FB_RTDB::int_pushTimestamp(FirebaseData *fbdo, const char *path, bool async)
{
    char *tmp = ut->strP(fb_esp_pgm_str_154);
    struct fb_esp_rtdb_request_info_t req;
    req.method = m_post;
    req.data.type = d_timestamp;
    req.path = path;
    req.queue = false;
    req.async = async;
    req.payload = tmp;
    bool ret = processRequest(fbdo, &req);
    ut->delS(tmp);
    return ret;
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, int intValue)
{
    return setInt(fbdo, path, intValue);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, int intValue)
{
    return setIntAsync(fbdo, path, intValue);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    return setInt(fbdo, path, intValue, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    return setIntAsync(fbdo, path, intValue, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, int intValue, const char *ETag)
{
    return setInt(fbdo, path, intValue, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, int intValue, const char *ETag)
{
    return setIntAsync(fbdo, path, intValue, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag)
{
    return setInt(fbdo, path, intValue, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag)
{
    return setIntAsync(fbdo, path, intValue, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, float floatValue)
{
    return setFloat(fbdo, path, floatValue);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, float floatValue)
{
    return setFloatAsync(fbdo, path, floatValue);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    return setFloat(fbdo, path, floatValue, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    return setFloatAsync(fbdo, path, floatValue, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag)
{
    return setFloat(fbdo, path, floatValue, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag)
{
    return setFloatAsync(fbdo, path, floatValue, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag)
{
    return setFloat(fbdo, path, floatValue, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag)
{
    return setFloatAsync(fbdo, path, floatValue, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return setDouble(fbdo, path, doubleValue);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return setDoubleAsync(fbdo, path, doubleValue);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    return setDouble(fbdo, path, doubleValue, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    return setDoubleAsync(fbdo, path, doubleValue, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag)
{
    return setDouble(fbdo, path, doubleValue, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag)
{
    return setDoubleAsync(fbdo, path, doubleValue, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag)
{
    return setDouble(fbdo, path, doubleValue, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag)
{
    return setDoubleAsync(fbdo, path, doubleValue, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return setBool(fbdo, path, boolValue);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return setBoolAsync(fbdo, path, boolValue);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    return setBool(fbdo, path, boolValue, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    return setBoolAsync(fbdo, path, boolValue, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag)
{
    return setBool(fbdo, path, boolValue, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag)
{
    return setBoolAsync(fbdo, path, boolValue, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag)
{
    return setBool(fbdo, path, boolValue, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag)
{
    return setBoolAsync(fbdo, path, boolValue, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const char *stringValue)
{
    return setString(fbdo, path, stringValue);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const char *stringValue)
{
    return setStringAsync(fbdo, path, stringValue);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return setString(fbdo, path, stringValue);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return setStringAsync(fbdo, path, stringValue);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const char *stringValue, float priority)
{
    return setString(fbdo, path, stringValue, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const char *stringValue, float priority)
{
    return setStringAsync(fbdo, path, stringValue, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    return setString(fbdo, path, stringValue, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    return setStringAsync(fbdo, path, stringValue, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const char *stringValue, const char *ETag)
{
    return setString(fbdo, path, stringValue, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const char *stringValue, const char *ETag)
{
    return setStringAsync(fbdo, path, stringValue, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag)
{
    return setString(fbdo, path, stringValue, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag)
{
    return setStringAsync(fbdo, path, stringValue, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const char *stringValue, float priority, const char *ETag)
{
    return setString(fbdo, path, stringValue, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const char *stringValue, float priority, const char *ETag)
{
    return setStringAsync(fbdo, path, stringValue, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag)
{
    return setString(fbdo, path, stringValue, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag)
{
    return setStringAsync(fbdo, path, stringValue, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return setJSON(fbdo, path, json);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return setJSONAsync(fbdo, path, json);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return setArray(fbdo, path, arr);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return setArrayAsync(fbdo, path, arr);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    return setJSON(fbdo, path, json, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    return setJSONAsync(fbdo, path, json, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    return setArray(fbdo, path, arr, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    return setArrayAsync(fbdo, path, arr, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag)
{
    return setJSON(fbdo, path, json, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag)
{
    return setJSONAsync(fbdo, path, json, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag)
{
    return setArray(fbdo, path, arr, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag)
{
    return setArrayAsync(fbdo, path, arr, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag)
{
    return setJSON(fbdo, path, json, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag)
{
    return setJSONAsync(fbdo, path, json, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag)
{
    return setArray(fbdo, path, arr, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag)
{
    return setArrayAsync(fbdo, path, arr, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return setBlob(fbdo, path, blob, size);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return setBlobAsync(fbdo, path, blob, size);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    return setBlob(fbdo, path, blob, size, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    return setBlobAsync(fbdo, path, blob, size, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag)
{
    return setBlob(fbdo, path, blob, size, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag)
{
    return setBlobAsync(fbdo, path, blob, size, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag)
{
    return setBlob(fbdo, path, blob, size, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag)
{
    return setBlobAsync(fbdo, path, blob, size, priority, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return setFile(fbdo, storageType, path, fileName);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return setFileAsync(fbdo, storageType, path, fileName);
}

bool FB_RTDB::set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    return setFile(fbdo, storageType, path, fileName, priority);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    return setFileAsync(fbdo, storageType, path, fileName, priority);
}

bool FB_RTDB::set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag)
{
    return setFile(fbdo, storageType, path, fileName, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag)
{
    return setFileAsync(fbdo, storageType, path, fileName, ETag);
}

bool FB_RTDB::set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag)
{
    return setFile(fbdo, storageType, path, fileName, priority, ETag);
}

bool FB_RTDB::setAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag)
{
    return setFileAsync(fbdo, storageType, path, fileName, priority, ETag);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value);
    else if (std::is_same<T, FirebaseJson *>::value)
        return setJson(fbdo, path, &value);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, size_t size)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(fbdo, path, value, size);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, float priority)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value, priority);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value, priority);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value, priority);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value, priority);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value, priority);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value, priority);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, size_t size, float priority)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(fbdo, path, value, size, priority);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, const char *ETag)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value, ETag);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value, ETag);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value, ETag);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value, ETag);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value, ETag);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value, ETag);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value, ETag);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, size_t size, const char *ETag)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(fbdo, path, value, size, ETag);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, float priority, const char *ETag)
{
    if (std::is_same<T, int>::value)
        return setInt(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, double>::value)
        return setDouble(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, bool>::value)
        return setBool(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, const char *>::value)
        return setString(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, const String &>::value)
        return setString(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, FirebaseJson &>::value)
        return setJson(fbdo, path, value, priority, ETag);
    else if (std::is_same<T, FirebaseJsonArray &>::value)
        return setArray(fbdo, path, value, priority, ETag);
}

template <typename T>
bool FB_RTDB::set(FirebaseData *fbdo, const char *path, T value, size_t size, float priority, const char *ETag)
{
    if (std::is_same<T, uint8_t *>::value)
        return setBlob(fbdo, path, value, size, priority, ETag);
}

bool FB_RTDB::setInt(FirebaseData *fbdo, const char *path, int intValue)
{
    return int_setInt(fbdo, path, intValue, false, "", "", false);
}

bool FB_RTDB::setIntAsync(FirebaseData *fbdo, const char *path, int intValue)
{
    return int_setInt(fbdo, path, intValue, false, "", "", true);
}

bool FB_RTDB::setInt(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setInt(fbdo, path, intValue, false, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setIntAsync(FirebaseData *fbdo, const char *path, int intValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setInt(fbdo, path, intValue, false, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setInt(FirebaseData *fbdo, const char *path, int intValue, const char *ETag)
{
    return int_setInt(fbdo, path, intValue, false, "", ETag, false);
}

bool FB_RTDB::setIntAsync(FirebaseData *fbdo, const char *path, int intValue, const char *ETag)
{
    return int_setInt(fbdo, path, intValue, false, "", ETag, true);
}

bool FB_RTDB::setInt(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setInt(fbdo, path, intValue, false, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setIntAsync(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setInt(fbdo, path, intValue, false, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setInt(FirebaseData *fbdo, const char *path, int intValue, bool queue, const char *priority, const char *etag, bool async)
{
    char *buf = ut->intStr(intValue);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_integer;
    req.async = async;
    req.payload = buf;
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setFloat(FirebaseData *fbdo, const char *path, float floatValue)
{
    return int_setFloat(fbdo, path, floatValue, false, "", "", false);
}

bool FB_RTDB::setFloatAsync(FirebaseData *fbdo, const char *path, float floatValue)
{
    return int_setFloat(fbdo, path, floatValue, false, "", "", true);
}

bool FB_RTDB::setFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFloat(fbdo, path, floatValue, false, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setFloatAsync(FirebaseData *fbdo, const char *path, float floatValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFloat(fbdo, path, floatValue, false, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setFloat(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag)
{
    return int_setFloat(fbdo, path, floatValue, false, "", ETag, false);
}

bool FB_RTDB::setFloatAsync(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag)
{
    return int_setFloat(fbdo, path, floatValue, false, "", ETag, true);
}

bool FB_RTDB::setFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFloat(fbdo, path, floatValue, false, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setFloatAsync(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFloat(fbdo, path, floatValue, false, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setFloat(FirebaseData *fbdo, const char *path, float floatValue, bool queue, const char *priority, const char *etag, bool async)
{
    char *buf = ut->floatStr(floatValue);
    ut->trimDigits(buf);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_float;
    req.async = async;
    req.payload = buf;
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setDouble(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return int_setDouble(fbdo, path, doubleValue, false, "", "", false);
}

bool FB_RTDB::setDoubleAsync(FirebaseData *fbdo, const char *path, double doubleValue)
{
    return int_setDouble(fbdo, path, doubleValue, false, "", "", true);
}

bool FB_RTDB::setDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setDouble(fbdo, path, doubleValue, false, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setDoubleAsync(FirebaseData *fbdo, const char *path, double doubleValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setDouble(fbdo, path, doubleValue, false, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setDouble(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag)
{
    return int_setDouble(fbdo, path, doubleValue, false, "", ETag, false);
}

bool FB_RTDB::setDoubleAsync(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag)
{
    return int_setDouble(fbdo, path, doubleValue, false, "", ETag, true);
}

bool FB_RTDB::setDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setDouble(fbdo, path, doubleValue, false, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setDoubleAsync(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setDouble(fbdo, path, doubleValue, false, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setDouble(FirebaseData *fbdo, const char *path, double doubleValue, bool queue, const char *priority, const char *etag, bool async)
{
    char *buf = ut->doubleStr(doubleValue);
    ut->trimDigits(buf);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_double;
    req.async = async;
    req.payload = buf;
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBool(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return int_setBool(fbdo, path, boolValue, false, "", "", false);
}

bool FB_RTDB::setBoolAsync(FirebaseData *fbdo, const char *path, bool boolValue)
{
    return int_setBool(fbdo, path, boolValue, false, "", "", true);
}

bool FB_RTDB::setBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBool(fbdo, path, boolValue, false, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBoolAsync(FirebaseData *fbdo, const char *path, bool boolValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBool(fbdo, path, boolValue, false, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBool(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag)
{
    return int_setBool(fbdo, path, boolValue, false, "", ETag, false);
}

bool FB_RTDB::setBoolAsync(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag)
{
    return int_setBool(fbdo, path, boolValue, false, "", ETag, true);
}

bool FB_RTDB::setBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBool(fbdo, path, boolValue, false, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBoolAsync(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBool(fbdo, path, boolValue, false, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setBool(FirebaseData *fbdo, const char *path, bool boolValue, bool queue, const char *priority, const char *etag, bool async)
{
    char *buf = ut->boolStr(boolValue);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_boolean;
    req.async = async;
    req.payload = buf;
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setString(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return int_setString(fbdo, path, stringValue.c_str(), "", "", false);
}

bool FB_RTDB::setStringAsync(FirebaseData *fbdo, const char *path, const String &stringValue)
{
    return int_setString(fbdo, path, stringValue.c_str(), "", "", true);
}

bool FB_RTDB::setString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setString(fbdo, path, stringValue.c_str(), buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setStringAsync(FirebaseData *fbdo, const char *path, const String &stringValue, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setString(fbdo, path, stringValue.c_str(), buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setString(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag)
{
    return int_setString(fbdo, path, stringValue.c_str(), "", ETag, false);
}

bool FB_RTDB::setStringAsync(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag)
{
    return int_setString(fbdo, path, stringValue.c_str(), "", ETag, true);
}

bool FB_RTDB::setString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setString(fbdo, path, stringValue.c_str(), buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setStringAsync(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setString(fbdo, path, stringValue.c_str(), buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setString(FirebaseData *fbdo, const char *path, const char *stringValue, const char *priority, const char *etag, bool async)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_string;
    req.async = async;
    req.payload = stringValue;
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    return ret;
}

bool FB_RTDB::setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_setJSON(fbdo, path, json, "", "", false);
}

bool FB_RTDB::setJSONAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_setJSON(fbdo, path, json, "", "", true);
}

bool FB_RTDB::setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setJSON(fbdo, path, json, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setJSONAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setJSON(fbdo, path, json, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag)
{
    return int_setJSON(fbdo, path, json, "", ETag, false);
}

bool FB_RTDB::setJSONAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag)
{
    return int_setJSON(fbdo, path, json, "", ETag, true);
}

bool FB_RTDB::setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setJSON(fbdo, path, json, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setJSONAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setJSON(fbdo, path, json, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *priority, const char *etag, bool async)
{
    std::string s;
    json->int_toStdString(s);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_json;
    req.async = async;
    req.payload = s.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    std::string().swap(s);
    return ret;
}

bool FB_RTDB::setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return int_setArray(fbdo, path, arr, "", "", false);
}

bool FB_RTDB::setArrayAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr)
{
    return int_setArray(fbdo, path, arr, "", "", true);
}

bool FB_RTDB::setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    String arrStr;
    arr->toString(arrStr);
    bool ret = int_setArray(fbdo, path, arr, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setArrayAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    String arrStr;
    arr->toString(arrStr);
    bool ret = int_setArray(fbdo, path, arr, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag)
{
    return int_setArray(fbdo, path, arr, "", ETag, false);
}

bool FB_RTDB::setArrayAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag)
{
    return int_setArray(fbdo, path, arr, "", ETag, true);
}

bool FB_RTDB::setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    String arrStr;
    arr->toString(arrStr);
    bool ret = int_setArray(fbdo, path, arr, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setArrayAsync(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    String arrStr;
    arr->toString(arrStr);
    bool ret = int_setArray(fbdo, path, arr, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *priority, const char *etag, bool async)
{
    String s;
    arr->toString(s);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_array;
    req.async = async;
    req.payload = s.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    return ret;
}

bool FB_RTDB::setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return int_setBlob(fbdo, path, blob, size, false, "", "", false);
}

bool FB_RTDB::setBlobAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size)
{
    return int_setBlob(fbdo, path, blob, size, false, "", "", true);
}

bool FB_RTDB::setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBlob(fbdo, path, blob, size, false, buf, "", false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBlobAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBlob(fbdo, path, blob, size, false, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag)
{
    return int_setBlob(fbdo, path, blob, size, false, "", ETag, false);
}

bool FB_RTDB::setBlobAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag)
{
    return int_setBlob(fbdo, path, blob, size, false, "", ETag, true);
}

bool FB_RTDB::setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBlob(fbdo, path, blob, size, false, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setBlobAsync(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setBlob(fbdo, path, blob, size, false, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, bool queue, const char *priority, const char *etag, bool async)
{
    if (fbdo->_ss.rtdb.max_blob_size < size)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE;
        return false;
    }

    std::string base64Str;
    ut->appendP(base64Str, fb_esp_pgm_str_92, true);
    base64Str += ut->encodeBase64Str((const unsigned char *)blob, size);
    ut->appendP(base64Str, fb_esp_pgm_str_3);

    struct fb_esp_rtdb_request_info_t req;
    req.method = m_put_nocontent;
    req.data.type = d_blob;
    req.async = async;
    req.path = path;
    req.payload = base64Str.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(etag) > 0)
        req.etag = etag;
    bool ret = processRequest(fbdo, &req);
    std::string().swap(base64Str);
    return ret;
}

bool FB_RTDB::setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return int_setFile(fbdo, storageType, path, fileName, "", "", false);
}

bool FB_RTDB::setFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName)
{
    return int_setFile(fbdo, storageType, path, fileName, "", "", true);
}

bool FB_RTDB::setFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFile(fbdo, storageType, path, fileName, buf, "", true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag)
{
    return int_setFile(fbdo, storageType, path, fileName, "", ETag, false);
}

bool FB_RTDB::setFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag)
{
    return int_setFile(fbdo, storageType, path, fileName, "", ETag, true);
}

bool FB_RTDB::setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFile(fbdo, storageType, path, fileName, buf, ETag, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::setFileAsync(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_setFile(fbdo, storageType, path, fileName, buf, ETag, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *priority, const char *ETag, bool async)
{
    struct fb_esp_rtdb_request_info_t req;
    req.storageType = storageType;
    req.method = m_put_nocontent;
    req.data.type = d_file;
    req.path = path;
    req.filename = fileName;
    req.queue = false;
    req.async = async;
    if (strlen(priority) > 0)
        req.priority = priority;
    if (strlen(ETag) > 0)
        req.etag = ETag;
    bool ret = processRequest(fbdo, &req);
    return ret;
}

bool FB_RTDB::setTimestamp(FirebaseData *fbdo, const char *path)
{
    return int_setTimestamp(fbdo, path, false);
}

bool FB_RTDB::setTimestampAsync(FirebaseData *fbdo, const char *path)
{
    return int_setTimestamp(fbdo, path, true);
}

bool FB_RTDB::int_setTimestamp(FirebaseData *fbdo, const char *path, bool async)
{
    char *tmp = ut->strP(fb_esp_pgm_str_154);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_put;
    req.data.type = d_timestamp;
    req.async = async;
    req.payload = tmp;
    req.queue = false;
    bool ret = processRequest(fbdo, &req);
    ut->delS(tmp);
    return ret;
}

bool FB_RTDB::updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_updateNode(fbdo, path, json, "", false, false);
}

bool FB_RTDB::updateNodeAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_updateNode(fbdo, path, json, "", false, true);
}

bool FB_RTDB::updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_updateNode(fbdo, path, json, buf, false, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::updateNodeAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_updateNode(fbdo, path, json, buf, false, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::int_updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *priority, bool silent, bool async)
{
    std::string s;
    json->int_toStdString(s);
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    if (silent)
        req.method = m_patch_nocontent;
    else
        req.method = m_patch;
    req.data.type = d_json;
    req.payload = s.c_str();
    req.queue = false;
    if (strlen(priority) > 0)
        req.priority = priority;
    bool ret = processRequest(fbdo, &req);
    std::string().swap(s);
    return ret;
}

bool FB_RTDB::updateNodeSilent(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_updateNode(fbdo, path, json, "", true, false);
}

bool FB_RTDB::updateNodeSilentAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json)
{
    return int_updateNode(fbdo, path, json, "", true, true);
}

bool FB_RTDB::updateNodeSilent(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_updateNode(fbdo, path, json, buf, true, false);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::updateNodeSilentAsync(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority)
{
    char *buf = ut->floatStr(priority);
    ut->trimDigits(buf);
    bool ret = int_updateNode(fbdo, path, json, buf, true, true);
    ut->delS(buf);
    return ret;
}

bool FB_RTDB::get(FirebaseData *fbdo, const char *path)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get;
    req.data.type = d_any;
    req.storageType = mem_storage_type_undefined;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::getInt(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_integer;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getInt(FirebaseData *fbdo, const char *path, int *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_integer;
    req.data.intPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getFloat(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_float;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getFloat(FirebaseData *fbdo, const char *path, float *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_float;
    req.data.floatPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getDouble(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_double;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getDouble(FirebaseData *fbdo, const char *path, double *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_double;
    req.data.doublePtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getBool(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_boolean;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getBool(FirebaseData *fbdo, const char *path, bool *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_boolean;
    req.data.boolPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getString(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_string;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getString(FirebaseData *fbdo, const char *path, String *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_string;
    req.data.stringPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getJSON(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_json;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getJSON(FirebaseData *fbdo, const char *path, FirebaseJson *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_json;
    req.data.jsonPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getJSON(FirebaseData *fbdo, const char *path, QueryFilter *query)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_json;
    req.data.query = query;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getJSON(FirebaseData *fbdo, const char *path, QueryFilter *query, FirebaseJson *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_json;
    req.data.query = query;
    req.data.jsonPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getArray(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_array;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_array;
    req.data.arrPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getArray(FirebaseData *fbdo, const char *path, QueryFilter *query)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_array;
    req.data.query = query;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getArray(FirebaseData *fbdo, const char *path, QueryFilter *query, FirebaseJsonArray *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_array;
    req.data.arrPtr = target;
    req.data.query = query;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getBlob(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_blob;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getBlob(FirebaseData *fbdo, const char *path, std::vector<uint8_t> *target)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_blob;
    req.data.blobPtr = target;
    req.data.isPtr = true;
    req.path = path;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::getFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName)
{
    struct fb_esp_rtdb_request_info_t req;
    req.data.type = d_file;
    req.storageType = storageType;
    req.filename = fileName;
    req.path = nodePath;
    req.method = m_get;
    return processRequest(fbdo, &req);
}

bool FB_RTDB::deleteNode(FirebaseData *fbdo, const char *path)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_delete;
    req.data.type = d_string;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::deleteNode(FirebaseData *fbdo, const char *path, const char *ETag)
{
    fbdo->queryFilter.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_delete;
    req.data.type = d_string;
    req.etag = ETag;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::deleteNodesByTimestamp(FirebaseData *fbdo, const char *path, const char *timestampNode, size_t limit, unsigned long dataRetentionPeriod)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    time_t current_ts = time(nullptr);

    if (current_ts < ESP_DEFAULT_TS)
        return false;

    bool ret = false;

    if (limit > 30)
        limit = 30;

    QueryFilter query;

    double lastTS = current_ts - dataRetentionPeriod;

    query.orderBy(timestampNode).startAt(0).endAt(lastTS).limitToLast((int)limit);

    if (getJSON(fbdo, path, &query))
    {
        ret = true;
        if (fbdo->_ss.rtdb.resp_data_type == d_json && fbdo->jsonString().length() > 4)
        {
            FirebaseJson *js = fbdo->jsonObjectPtr();
            size_t len = js->iteratorBegin();
            String key, value;
            int otype = 0;
            std::string nodes[len];
            for (size_t i = 0; i < len; i++)
            {
                js->iteratorGet(i, otype, key, value);
                if (otype == FirebaseJson::JSON_OBJECT && key.length() > 1)
                    nodes[i] = key.c_str();
            }
            js->iteratorEnd();
            js->clear();

            for (size_t i = 0; i < len; i++)
            {
                std::string s = path;
                ut->appendP(s, fb_esp_pgm_str_1);
                s += nodes[i];
                deleteNode(fbdo, s.c_str());
            }
        }
    }

    query.clear();
    return ret;
}

bool FB_RTDB::beginStream(FirebaseData *fbdo, const char *path)
{
    return int_beginStream(fbdo, path);
}

bool FB_RTDB::int_beginStream(FirebaseData *fbdo, const char *path, bool clearNodeList)
{
    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    fbdo->_ss.rtdb.pause = false;

    if (clearNodeList)
        fbdo->clearNodeList();

    if (!fbdo->reconnect())
        return false;

    fbdo->closeSession();
    fbdo->_ss.rtdb.stream_stop = false;
    fbdo->_ss.rtdb.data_tmo = false;

    fbdo->_ss.rtdb.stream_path = path;

    if (!handleStreamRequest(fbdo, path))
    {
        if (!fbdo->tokenReady())
            return true;

        return false;
    }

    clearDataStatus(fbdo);

    return waitResponse(fbdo);
}

bool FB_RTDB::beginMultiPathStream(FirebaseData *fbdo, const char *parentPath, const String *childPath, size_t size)
{
    fbdo->addNodeList(childPath, size);
    return int_beginStream(fbdo, parentPath, false);
}

bool FB_RTDB::readStream(FirebaseData *fbdo)
{
    return handleStreamRead(fbdo);
}

bool FB_RTDB::endStream(FirebaseData *fbdo)
{
    fbdo->_ss.rtdb.pause = true;
    fbdo->_ss.rtdb.stream_stop = true;
    fbdo->_ss.con_mode = fb_esp_con_mode_undefined;
    fbdo->closeSession();
    clearDataStatus(fbdo);
    return true;
}

bool FB_RTDB::handleStreamRead(FirebaseData *fbdo)
{

    if (fbdo->_ss.rtdb.pause || fbdo->_ss.rtdb.stream_stop)
        return true;

    if (!fbdo->reconnect())
    {
        fbdo->_ss.rtdb.data_tmo = true;
        return true;
    }

    if (!fbdo->tokenReady())
        return false;

    bool ret = false;
    bool reconnectStream = false;

    //trying to reconnect the stream when required at some interval as running in the loop
    if (millis() - STREAM_RECONNECT_INTERVAL > fbdo->_ss.rtdb.stream_resume_millis)
    {
        reconnectStream = (fbdo->_ss.rtdb.data_tmo && !fbdo->_ss.connected) || fbdo->_ss.http_code >= 400 || fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream;

        if (fbdo->_ss.rtdb.data_tmo)
            fbdo->closeSession();
        fbdo->_ss.rtdb.stream_resume_millis = millis();
    }
    else
        ret = true;

    if (fbdo->httpClient.stream())
    {
        if (!fbdo->httpClient.stream()->connected())
            reconnectStream = true;
    }
    else
        reconnectStream = true;

    //Stream timeout
    if (millis() - fbdo->_ss.rtdb.data_millis > KEEP_ALIVE_TIMEOUT)
    {
        fbdo->_ss.rtdb.data_millis = millis();
        fbdo->_ss.rtdb.data_tmo = true;
        reconnectStream = true;
    }

    if (reconnectStream)
    {
        if (!ut->waitIdle(fbdo->_ss.http_code))
            return false;

        fbdo->closeSession();

        if (!fbdo->tokenReady())
            return false;

        std::string path = fbdo->_ss.rtdb.stream_path;
        if (fbdo->_ss.rtdb.redirect_url.length() > 0)
        {
            struct fb_esp_url_info_t uinfo;
            ut->getUrlInfo(fbdo->_ss.rtdb.redirect_url, uinfo);
            path = uinfo.uri;
        }

        //mode changed, non-stream -> stream, reset the data timeout state to allow data available to be notified.
        fbdo->_ss.rtdb.data_tmo = false;

        if (!handleStreamRequest(fbdo, path))
        {
            fbdo->_ss.rtdb.data_tmo = true;
            return ret;
        }

        fbdo->_ss.con_mode = fb_esp_con_mode_rtdb_stream;
    }

    if (!waitResponse(fbdo))
        return ret;

    return true;
}

#if defined(ESP32)
void FB_RTDB::setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize)
{
    fbdo->_ss.rtdb.stream_task_enable = false;
#elif defined(ESP8266)
void FB_RTDB::setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback)
{
#endif

    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }
    removeMultiPathStreamCallback(fbdo);

    int index = fbdo->_ss.rtdb.Idx;
    bool hasHandle = false;

    if (fbdo->_ss.rtdb.Idx != -1 || fbdo->_ss.rtdb.queue_Idx != -1)
        hasHandle = true;
    else
    {
        index = Signer.getCfg()->_int.fb_stream_idx;
        Signer.getCfg()->_int.fb_stream_idx++;
    }

    fbdo->_ss.rtdb.Idx = index;
    fbdo->_dataAvailableCallback = dataAvailableCallback;
    fbdo->_timeoutCallback = timeoutCallback;

#if defined(ESP32)
    std::string taskName;
    ut->appendP(taskName, fb_esp_pgm_str_72, true);
    char *tmp = ut->intStr(index);
    ut->appendP(taskName, fb_esp_pgm_str_113);
    taskName += tmp;
    ut->delS(tmp);

    if (streamTaskStackSize > STREAM_TASK_STACK_SIZE)
        fbdo->_ss.rtdb.stream_task_stack_size = streamTaskStackSize;
    else
        fbdo->_ss.rtdb.stream_task_stack_size = STREAM_TASK_STACK_SIZE;

    fbdo->_ss.rtdb.stream_task_enable = true;
#endif

    //object created
    if (hasHandle)
        Signer.getCfg()->_int.fb_sdo[index] = *fbdo;
    else
        Signer.getCfg()->_int.fb_sdo.push_back(*fbdo);

#if defined(ESP32)
    runStreamTask(fbdo, taskName.c_str());
#elif defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#endif
}

#if defined(ESP32)
void FB_RTDB::setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize)
{
    fbdo->_ss.rtdb.stream_task_enable = false;
#elif defined(ESP8266)
void FB_RTDB::setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback)
{
#endif

    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    removeStreamCallback(fbdo);

    int index = fbdo->_ss.rtdb.Idx;

    bool hasHandle = false;

    if (fbdo->_ss.rtdb.Idx != -1 || fbdo->_ss.rtdb.queue_Idx != -1)
        hasHandle = true;
    else
    {
        index = Signer.getCfg()->_int.fb_stream_idx;
        Signer.getCfg()->_int.fb_stream_idx++;
    }

    fbdo->_ss.rtdb.Idx = index;
    fbdo->_multiPathDataCallback = multiPathDataCallback;
    fbdo->_timeoutCallback = timeoutCallback;

#if defined(ESP32)
    std::string taskName;
    ut->appendP(taskName, fb_esp_pgm_str_72, true);
    char *tmp = ut->intStr(index);
    ut->appendP(taskName, fb_esp_pgm_str_113);
    taskName += tmp;
    ut->delS(tmp);

    if (streamTaskStackSize > STREAM_TASK_STACK_SIZE)
        fbdo->_ss.rtdb.stream_task_stack_size = streamTaskStackSize;
    else
        fbdo->_ss.rtdb.stream_task_stack_size = STREAM_TASK_STACK_SIZE;

    fbdo->_ss.rtdb.stream_task_enable = true;
#endif

    //object created
    if (hasHandle)
        Signer.getCfg()->_int.fb_sdo[index] = *fbdo;
    else
        Signer.getCfg()->_int.fb_sdo.push_back(*fbdo);
#if defined(ESP32)
    runStreamTask(fbdo, taskName.c_str());
#elif defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#endif
}

void FB_RTDB::removeMultiPathStreamCallback(FirebaseData *fbdo)
{
    int index = fbdo->_ss.rtdb.Idx;

    if (index != -1)
    {
        fbdo->_multiPathDataCallback = NULL;
        fbdo->_timeoutCallback = NULL;

#if defined(ESP32)
        bool hasOherHandles = false;

        if (fbdo->_ss.rtdb.queue_task_handle)
            hasOherHandles = true;

        if (!hasOherHandles)
            fbdo->_ss.rtdb.Idx = -1;

        if (fbdo->_ss.rtdb.stream_task_handle)
            vTaskDelete(fbdo->_ss.rtdb.stream_task_handle);

        fbdo->_ss.rtdb.stream_task_handle = NULL;

        if (!hasOherHandles)
            Signer.getCfg()->_int.fb_sdo.erase(Signer.getCfg()->_int.fb_sdo.begin() + index);

#elif defined(ESP8266)
        fbdo->_ss.rtdb.Idx = -1;
        Signer.getCfg()->_int.fb_sdo.erase(Signer.getCfg()->_int.fb_sdo.begin() + index);
#endif
    }
}

#if defined(ESP32)
void FB_RTDB::runStreamTask(FirebaseData *fbdo, const char *taskName)
#elif defined(ESP8266)
void FB_RTDB::runStreamTask()
#endif
{
#if defined(ESP32)

    static FB_RTDB *_this = this;
    static int id = Signer.getCfg()->_int.fb_stream_idx - 1;

    TaskFunction_t taskCode = [](void *param) {
        while (Signer.getCfg()->_int.fb_sdo[id].get()._ss.rtdb.stream_task_enable)
        {

            if ((Signer.getCfg()->_int.fb_sdo[id].get()._dataAvailableCallback || Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback))
            {

                _this->readStream(&Signer.getCfg()->_int.fb_sdo[id].get());

                if (Signer.getCfg()->_int.fb_sdo[id].get().streamTimeout() && Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback)
                    Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback(true);
            }

            yield();
            vTaskDelay(3 / portTICK_PERIOD_MS);
        }

        Signer.getCfg()->_int.fb_sdo[id].get()._ss.rtdb.stream_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName, fbdo->_ss.rtdb.stream_task_stack_size, NULL, 3, &fbdo->_ss.rtdb.stream_task_handle, 1);

#elif defined(ESP8266)

    for (size_t id = 0; id < Signer.getCfg()->_int.fb_sdo.size(); id++)
    {

        if ((Signer.getCfg()->_int.fb_sdo[id].get()._dataAvailableCallback || Signer.getCfg()->_int.fb_sdo[id].get()._multiPathDataCallback || Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback))
        {
            readStream(&Signer.getCfg()->_int.fb_sdo[id].get());

            if (Signer.getCfg()->_int.fb_sdo[id].get().streamTimeout() && Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback)
                Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback(true);
        }
    }
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#endif
}

#if defined(ESP8266)
void FB_RTDB::runErrorQueueTask()
{
    for (size_t id = 0; id < Signer.getCfg()->_int.fb_sdo.size(); id++)
    {
        if (Signer.getCfg()->_int.fb_sdo[id].get()._queueInfoCallback)
            processErrorQueue(&Signer.getCfg()->_int.fb_sdo[id].get(), Signer.getCfg()->_int.fb_sdo[id].get()._queueInfoCallback);
        else
            processErrorQueue(&Signer.getCfg()->_int.fb_sdo[id].get(), NULL);
    }

    ut->set_scheduled_callback(std::bind(&FB_RTDB::runErrorQueueTask, this));
}
#endif

uint32_t FB_RTDB::getErrorQueueID(FirebaseData *fbdo)
{
    return fbdo->_ss.rtdb.queue_ID;
}

void FB_RTDB::processErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback)
{
    delay(0);

    if (!fbdo->reconnect())
        return;

    if (fbdo->_qMan._queueCollection.size() > 0)
    {

        for (uint8_t i = 0; i < fbdo->_qMan._queueCollection.size(); i++)
        {
            QueueItem item = fbdo->_qMan._queueCollection[i];

            if (callback)
            {
                QueueInfo qinfo;
                qinfo._isQueue = true;
                qinfo._dataType = fbdo->getDataType(item.dataType);
                qinfo._path = item.path;
                qinfo._currentQueueID = item.qID;
                qinfo._method = fbdo->getMethod(item.method);
                qinfo._totalQueue = fbdo->_qMan._queueCollection.size();
                qinfo._isQueueFull = fbdo->_qMan._queueCollection.size() == fbdo->_qMan._maxQueue;
                callback(qinfo);
            }

            if (item.method == m_get)
            {

                switch (item.dataType)
                {

                case d_integer:

                    if (getInt(fbdo, item.path.c_str()))
                    {

                        if (item.intPtr)
                            *item.intPtr = fbdo->intData();

                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;

                case d_float:

                    if (getFloat(fbdo, item.path.c_str()))
                    {

                        if (item.floatPtr)
                            *item.floatPtr = fbdo->floatData();

                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;

                case d_double:
                    if (getDouble(fbdo, item.path.c_str()))
                    {
                        if (item.doublePtr)
                            *item.doublePtr = fbdo->doubleData();

                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;

                case d_boolean:

                    if (getBool(fbdo, item.path.c_str()))
                    {

                        if (item.boolPtr)
                            *item.boolPtr = fbdo->boolData();

                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;

                case d_string:

                    if (getString(fbdo, item.path.c_str()))
                    {

                        if (item.stringPtr)
                            *item.stringPtr = fbdo->stringData();

                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;

                case d_json:

                    if (item.queryFilter._orderBy.length() > 0)
                    {
                        if (getJSON(fbdo, item.path.c_str(), &item.queryFilter))
                        {

                            if (item.jsonPtr)
                                item.jsonPtr = fbdo->jsonObjectPtr();

                            fbdo->clearQueueItem(&item);
                            fbdo->_qMan.remove(i);
                        }
                    }
                    else
                    {
                        if (getJSON(fbdo, item.path.c_str()))
                        {

                            if (item.jsonPtr)
                                item.jsonPtr = fbdo->jsonObjectPtr();

                            fbdo->clearQueueItem(&item);
                            fbdo->_qMan.remove(i);
                        }
                    }

                    break;
                case d_array:

                    if (item.queryFilter._orderBy.length() > 0)
                    {
                        if (getArray(fbdo, item.path.c_str(), &item.queryFilter))
                        {

                            if (item.arrPtr)
                                item.arrPtr = fbdo->jsonArrayPtr();

                            fbdo->clearQueueItem(&item);
                            fbdo->_qMan.remove(i);
                        }
                    }
                    else
                    {
                        if (getArray(fbdo, item.path.c_str()))
                        {
                            if (item.arrPtr)
                                item.arrPtr = fbdo->jsonArrayPtr();

                            fbdo->clearQueueItem(&item);
                            fbdo->_qMan.remove(i);
                        }
                    }

                    break;

                case d_blob:

                    if (getBlob(fbdo, item.path.c_str()))
                    {
                        if (item.blobPtr)
                            *item.blobPtr = fbdo->blobData();

                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }
                    break;

                case d_file:

                    if (getFile(fbdo, item.storageType, item.path.c_str(), item.filename.c_str()))
                    {
                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;
                case d_any:

                    if (get(fbdo, item.path.c_str()))
                    {
                        fbdo->clearQueueItem(&item);
                        fbdo->_qMan.remove(i);
                    }

                    break;

                default:
                    break;
                }
            }
            else if (item.method == m_post || item.method == m_put || item.method == m_put_nocontent || item.method == m_patch || item.method == m_patch_nocontent)
            {
                struct fb_esp_rtdb_request_info_t req;
                req.storageType = item.storageType;
                req.method = item.method;
                req.data.type = d_file;
                req.path = item.path;
                req.queue = true;

                if (item.dataType == d_file)
                    req.filename = item.filename;
                else
                    req.payload = item.payload;

                if (processRequest(fbdo, &req))
                {
                    fbdo->clearQueueItem(&item);
                    fbdo->_qMan.remove(i);
                }
            }
        }
    }
}

bool FB_RTDB::isErrorQueueExisted(FirebaseData *fbdo, uint32_t errorQueueID)
{

    for (uint8_t i = 0; i < fbdo->_qMan._queueCollection.size(); i++)
    {
        QueueItem q = fbdo->_qMan._queueCollection[i];
        if (q.qID == errorQueueID)
            return true;
    }
    return false;
}

#if defined(ESP32)
void FB_RTDB::beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback, size_t queueTaskStackSize)
#elif defined(ESP8266)
void FB_RTDB::beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback)
#endif
{
    static int index = fbdo->_ss.rtdb.queue_Idx;

    bool hasHandle = false;
#if defined(ESP32)
    if (fbdo->_ss.rtdb.stream_task_handle || fbdo->_ss.rtdb.queue_task_handle)
#elif defined(ESP8266)
    if (fbdo->_ss.rtdb.Idx != -1 || fbdo->_ss.rtdb.queue_Idx != -1)
#endif
        hasHandle = true;
    else
    {
        index = Signer.getCfg()->_int.fb_stream_idx;
        Signer.getCfg()->_int.fb_stream_idx++;
    }

    fbdo->_ss.rtdb.queue_Idx = index;
    fbdo->_ss.rtdb.Idx = index;

    if (callback)
        fbdo->_queueInfoCallback = callback;
    else
        fbdo->_queueInfoCallback = NULL;

    //object created
    if (hasHandle)
        Signer.getCfg()->_int.fb_sdo[index] = *fbdo;
    else
        Signer.getCfg()->_int.fb_sdo.push_back(*fbdo);

#if defined(ESP32)

    std::string taskName;
    ut->appendP(taskName, fb_esp_pgm_str_72);
    char *tmp = ut->intStr(index);
    ut->appendP(taskName, fb_esp_pgm_str_114);
    taskName += tmp;
    ut->delS(tmp);

    if (queueTaskStackSize > QUEUE_TASK_STACK_SIZE)
        fbdo->_ss.rtdb.queue_task_stack_size = queueTaskStackSize;
    else
        fbdo->_ss.rtdb.queue_task_stack_size = QUEUE_TASK_STACK_SIZE;

    static FB_RTDB *_this = this;

    TaskFunction_t taskCode = [](void *param) {
        for (;;)
        {
            if (Signer.getCfg()->_int.fb_sdo[index].get()._queueInfoCallback)
                _this->processErrorQueue(&Signer.getCfg()->_int.fb_sdo[index].get(), Signer.getCfg()->_int.fb_sdo[index].get()._queueInfoCallback);
            else
                _this->processErrorQueue(&Signer.getCfg()->_int.fb_sdo[index].get(), NULL);

            yield();
            vTaskDelay(3 / portTICK_PERIOD_MS);
        }

        Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName.c_str(), Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_stack_size, NULL, 1, &Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_handle, 1);

#elif defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runErrorQueueTask, this));
#endif
}

void FB_RTDB::endAutoRunErrorQueue(FirebaseData *fbdo)
{
    int index = fbdo->_ss.rtdb.Idx;

    if (index != -1)
    {
        fbdo->_ss.rtdb.Idx = -1;
        fbdo->_queueInfoCallback = NULL;
        Signer.getCfg()->_int.fb_sdo.erase(Signer.getCfg()->_int.fb_sdo.begin() + index);
    }
}

void FB_RTDB::clearErrorQueue(FirebaseData *fbdo)
{
    for (uint8_t i = 0; i < fbdo->_qMan._queueCollection.size(); i++)
    {
        QueueItem item = fbdo->_qMan._queueCollection[i];
        fbdo->clearQueueItem(&item);
    }
}

void FB_RTDB::setMaxRetry(FirebaseData *fbdo, uint8_t num)
{
    fbdo->_ss.rtdb.max_retry = num;
}

void FB_RTDB::setMaxErrorQueue(FirebaseData *fbdo, uint8_t num)
{
    fbdo->_qMan._maxQueue = num;

    if (fbdo->_qMan._queueCollection.size() > num)
    {
        for (uint8_t i = fbdo->_qMan._queueCollection.size() - 1; i >= num; i--)
        {
            QueueItem item = fbdo->_qMan._queueCollection[i];
            fbdo->clearQueueItem(&item);
        }
    }
}

bool FB_RTDB::saveErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType)
{

    if (storageType == mem_storage_type_sd)
    {
        if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            return false;
        Signer.getCfg()->_int.fb_file = SD_FS.open(filename, FILE_WRITE);
    }
    else if (storageType == mem_storage_type_flash)
    {
        if (!Signer.getCfg()->_int.fb_flash_rdy)
            ut->flashTest();

        Signer.getCfg()->_int.fb_file = FLASH_FS.open(filename, "w");
    }

    if ((storageType == mem_storage_type_flash || storageType == mem_storage_type_sd) && !Signer.getCfg()->_int.fb_file)
        return false;

    uint8_t idx = 0;
    std::string buff = "";
    char *nbuf = nullptr;

    for (uint8_t i = 0; i < fbdo->_qMan._queueCollection.size(); i++)
    {
        QueueItem item = fbdo->_qMan._queueCollection[i];

        if (item.method != m_get)
        {
            if (idx > 0)
                buff.append("\r");

            nbuf = ut->intStr(item.dataType);
            buff.append(nbuf);
            ut->delS(nbuf);
            buff.append("~");

            nbuf = ut->intStr(item.method);
            buff.append(nbuf);
            ut->delS(nbuf);
            buff.append("~");

            buff += item.path.c_str();
            buff.append("~");

            buff += item.payload.c_str();
            buff.append("~");

            for (size_t j = 0; j < item.blob.size(); j++)
            {
                nbuf = ut->intStr(item.blob[j]);
                ut->delS(nbuf);
            }
            buff.append("~");

            buff += item.filename.c_str();

            nbuf = ut->intStr(item.storageType);
            buff.append(nbuf);
            ut->delS(nbuf);
            buff.append("~");

            idx++;
        }
    }

    Signer.getCfg()->_int.fb_file.print(buff.c_str());
    Signer.getCfg()->_int.fb_file.close();

    ut->delS(nbuf);
    std::string().swap(buff);

    return true;
}

bool FB_RTDB::restoreErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType)
{
    return openErrorQueue(fbdo, filename, storageType, 1) != 0;
}

uint8_t FB_RTDB::errorQueueCount(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType)
{
    return openErrorQueue(fbdo, filename, storageType, 0);
}

bool FB_RTDB::deleteStorageFile(const char *filename, fb_esp_mem_storage_type storageType)
{

    if (storageType == mem_storage_type_sd)
    {
        if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            return false;
        return SD_FS.remove(filename);
    }
    else
    {
        if (!Signer.getCfg()->_int.fb_flash_rdy)
            ut->flashTest();
        return FLASH_FS.remove(filename);
    }
}

uint8_t FB_RTDB::openErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType, uint8_t mode)
{

    uint8_t count = 0;

    if (storageType == mem_storage_type_sd)
    {
        if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            return 0;
        Signer.getCfg()->_int.fb_file = SD_FS.open(filename, FILE_READ);
    }
    else if (storageType == mem_storage_type_flash)
    {
        if (!Signer.getCfg()->_int.fb_flash_rdy)
            ut->flashTest();
        Signer.getCfg()->_int.fb_file = FLASH_FS.open(filename, "r");
    }

    if ((storageType == mem_storage_type_flash || storageType == mem_storage_type_sd) && !Signer.getCfg()->_int.fb_file)
        return 0;

    std::string t = "";
    uint8_t c = 0;

    while (Signer.getCfg()->_int.fb_file.available())
    {
        c = Signer.getCfg()->_int.fb_file.read();
        t += (char)c;
    }

    Signer.getCfg()->_int.fb_file.close();

    std::vector<std::string> p = ut->splitString(fbdo->_ss.rtdb.max_blob_size, t.c_str(), '\r');

    for (size_t i = 0; i < p.size(); i++)
    {

        std::vector<std::string> q = ut->splitString(fbdo->_ss.rtdb.max_blob_size, p[i].c_str(), '~');

        if (q.size() == 6)
        {
            count++;

            if (mode == 1)
            {

                //Restore Firebase Error Queues
                QueueItem item;

                item.dataType = (fb_esp_data_type)atoi(q[0].c_str());
                item.method = (fb_esp_method)atoi(q[1].c_str());
                item.path.append(q[2].c_str());
                item.payload.append(q[3].c_str());

                for (size_t j = 0; j < q[4].length(); j++)
                    item.blob.push_back(atoi(q[4].c_str()));

                item.filename.append(q[5].c_str());

                //backwards compatibility to old APIs
                if (q.size() == 7)
                    item.storageType = (fb_esp_mem_storage_type)atoi(q[6].c_str());

                fbdo->_qMan._queueCollection.push_back(item);
            }
        }
    }
    std::string().swap(t);

    return count;
}

bool FB_RTDB::isErrorQueueFull(FirebaseData *fbdo)
{
    if (fbdo->_qMan._maxQueue > 0)
        return fbdo->_qMan._queueCollection.size() >= fbdo->_qMan._maxQueue;
    return false;
}

uint8_t FB_RTDB::errorQueueCount(FirebaseData *fbdo)
{
    return fbdo->_qMan._queueCollection.size();
}

bool FB_RTDB::backup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName)
{
    fbdo->_ss.rtdb.backup_dir.clear();
    fbdo->_ss.rtdb.backup_node_path = nodePath;
    fbdo->_ss.rtdb.backup_filename = fileName;
    fbdo->_ss.rtdb.file_name.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = nodePath;
    req.method = m_download;
    req.data.type = d_json;
    req.storageType = storageType;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::restore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName)
{
    fbdo->_ss.rtdb.backup_dir.clear();
    fbdo->_ss.rtdb.backup_node_path = nodePath;
    fbdo->_ss.rtdb.backup_filename = fileName;
    fbdo->_ss.rtdb.file_name.clear();
    struct fb_esp_rtdb_request_info_t req;
    req.path = nodePath;
    req.method = m_restore;
    req.data.type = d_json;
    req.storageType = storageType;
    bool ret = handleRequest(fbdo, &req);
    return ret;
}

bool FB_RTDB::processRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    if (req->method != m_get)
    {
        if (!fbdo->reconnect())
            return false;
    }

    if (!fbdo->tokenReady())
        return false;

    bool ret = false;
    fbdo->queryFilter.clear();

    if (req->data.query && (req->data.type == d_json || req->data.type == d_array))
    {
        if (req->data.query->_orderBy.length() > 0)
            fbdo->setQuery(req->data.query);
    }
    else if (req->data.type == d_file)
    {
        fbdo->_ss.rtdb.file_name = req->filename;

        if (req->method == m_get)
        {
            if (req->storageType == mem_storage_type_flash && !Signer.getCfg()->_int.fb_flash_rdy)
                ut->flashTest();
            else if (req->storageType == mem_storage_type_sd && !Signer.getCfg()->_int.fb_sd_rdy)
                ut->sdTest(Signer.getCfg()->_int.fb_file);

            if (Signer.getCfg()->_int.fb_flash_rdy)
                Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.file_name.c_str(), "w");
            else if (Signer.getCfg()->_int.fb_sd_rdy)
                Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.file_name.c_str(), FILE_WRITE);
        }
    }

    fbdo->_ss.rtdb.queue_ID = 0;

    uint8_t errCount = 0;
    uint8_t maxRetry = fbdo->_ss.rtdb.max_retry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        ret = handleRequest(fbdo, req);
        if (req->data.isPtr)
        {
            if (req->data.type != d_file)
            {
                if (req->data.type == d_boolean)
                    *req->data.boolPtr = fbdo->boolData();
                else if (req->data.type == d_integer)
                    *req->data.intPtr = fbdo->intData();
                else if (req->data.type == d_float)
                    *req->data.floatPtr = fbdo->floatData();
                else if (req->data.type == d_double)
                    *req->data.doublePtr = fbdo->doubleData();
                else if (req->data.type == d_string)
                    *req->data.stringPtr = fbdo->stringData();
                else if (req->data.type == d_json)
                    req->data.jsonPtr = fbdo->jsonObjectPtr();
                else if (req->data.type == d_array)
                    req->data.arrPtr = fbdo->jsonArrayPtr();
                else if (req->data.type == d_blob)
                    *req->data.blobPtr = fbdo->blobData();
            }
        }

        if (ret)
            break;

        if (fbdo->_ss.rtdb.max_retry > 0)
            if (!ret && connectionError(fbdo))
                errCount++;
    }

    struct fb_esp_rtdb_queue_info_t qinfo;

    qinfo.method = req->method;
    qinfo.storageType = req->storageType;
    qinfo.dataType = req->data.type;
    qinfo.path = req->path;
    if (req->filename.length() > 0)
        qinfo.filename = req->filename;
    if (req->payload.length() > 0)
        qinfo.payload = req->payload;

    if (req->data.isPtr)
    {

        if (req->data.type == d_boolean)
            qinfo.boolPtr = req->data.boolPtr;
        else if (req->data.type == d_integer)
            qinfo.intPtr = req->data.intPtr;
        else if (req->data.type == d_float)
            qinfo.floatPtr = req->data.floatPtr;
        else if (req->data.type == d_double)
            qinfo.doublePtr = req->data.doublePtr;
        else if (req->data.type == d_string)
            qinfo.stringPtr = req->data.stringPtr;
        else if (req->data.type == d_json)
            qinfo.jsonPtr = req->data.jsonPtr;
        else if (req->data.type == d_array)
            qinfo.arrPtr = req->data.arrPtr;
        else if (req->data.type == d_blob)
            qinfo.blobPtr = req->data.blobPtr;
    }

    if (req->method == m_get && !ret && errCount == maxRetry && fbdo->_qMan._maxQueue > 0)
        fbdo->addQueue(&qinfo);
    else if (!req->queue && !ret && errCount == maxRetry && fbdo->_qMan._maxQueue > 0)
    {
        if (req->method == m_put || req->method == m_put_nocontent || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent)
            fbdo->addQueue(&qinfo);
        return ret;
    }

    if (ret)
    {
        if (req->method == m_get)
        {
            if (req->data.type == d_integer || req->data.type == d_float || req->data.type == d_double)
                ret = fbdo->_ss.rtdb.resp_data_type == d_integer || fbdo->_ss.rtdb.resp_data_type == d_float || fbdo->_ss.rtdb.resp_data_type == d_double;
            else if (req->data.type == d_json)
                ret = fbdo->_ss.rtdb.resp_data_type == d_json || fbdo->_ss.rtdb.resp_data_type == d_null;
            else if (req->data.type == d_array)
                ret = fbdo->_ss.rtdb.resp_data_type == d_array || fbdo->_ss.rtdb.resp_data_type == d_null;
            else if (req->data.type != d_file)
                ret = fbdo->_ss.rtdb.resp_data_type == req->data.type;
        }
    }

    return ret;
}

#if defined(ESP32)
void FB_RTDB::allowMultipleRequests(bool enable)
{
    Signer.getCfg()->_int.fb_multiple_requests = enable;
}
#endif

void FB_RTDB::rescon(FirebaseData *fbdo, const char *host, fb_esp_rtdb_request_info_t *req)
{
    if (req->method == m_stream)
    {
        if (req->path != fbdo->_ss.rtdb.stream_path)
            fbdo->_ss.rtdb.stream_path_changed = true;
        else
            fbdo->_ss.rtdb.stream_path_changed = false;
    }

    if (!fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.rtdb.stream_path_changed || (req->method == m_stream && fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream) || (req->method != m_stream && fbdo->_ss.con_mode == fb_esp_con_mode_rtdb_stream) || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->_ss.host = host;
    if (req->method == m_stream)
        fbdo->_ss.con_mode = fb_esp_con_mode_rtdb_stream;
    else
        fbdo->_ss.con_mode = fb_esp_con_mode_rtdb;

    if (fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream)
        fbdo->_ss.rtdb.stream_resume_millis = 0;
}

bool FB_RTDB::handleRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    fbdo->_ss.error.clear();

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady() || !fbdo->validRequest(req->path))
        return false;

    if ((req->method == m_put || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent || req->method == m_set_rules) && req->payload.length() == 0 && req->data.type != d_string && req->data.type != d_blob && req->data.type != d_file)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST;
        return false;
    }

    if (!ut->waitIdle(fbdo->_ss.http_code))
        return false;

    if (!fbdo->_ss.connected)
        fbdo->_ss.rtdb.async_count = 0;

    if ((fbdo->_ss.rtdb.async && !req->async) || fbdo->_ss.rtdb.async_count > Signer.config->async_close_session_max_request)
    {
        fbdo->_ss.rtdb.async_count = 0;
        fbdo->closeSession();
    }

    fbdo->_ss.rtdb.queue_ID = 0;
    fbdo->_ss.rtdb.req_etag = req->etag;
    fbdo->_ss.rtdb.priority = req->priority;
    fbdo->_ss.rtdb.storage_type = req->storageType;

    fbdo->_ss.rtdb.redirect_url.clear();
    fbdo->_ss.rtdb.req_method = req->method;
    fbdo->_ss.rtdb.req_data_type = req->data.type;
    fbdo->_ss.rtdb.data_mismatch = false;
    fbdo->_ss.rtdb.async = req->async;
    if (req->async)
        fbdo->_ss.rtdb.async_count++;
    int ret = sendRequest(fbdo, req);
    if (ret == 0)
    {
        fbdo->_ss.connected = true;

        if (req->method == m_stream)
        {
            if (!waitResponse(fbdo))
            {
                fbdo->closeSession();
                return false;
            }
        }
        else if (req->method == m_download || (req->data.type == d_file && req->method == m_get))
        {
            if (!waitResponse(fbdo))
            {
                fbdo->closeSession();
                return false;
            }
        }
        else if (req->method == m_restore || (req->data.type == d_file && req->method == m_put_nocontent))
        {
            if (!waitResponse(fbdo))
            {
                fbdo->closeSession();
                return false;
            }
        }
        else
        {
            fbdo->_ss.rtdb.path = req->path;
            if (!waitResponse(fbdo))
            {
                fbdo->closeSession();
                return false;
            }
            fbdo->_ss.rtdb.data_available = fbdo->_ss.rtdb.data.length() > 0 || fbdo->_ss.rtdb.blob.size() > 0;
        }
    }
    else
    {
        fbdo->_ss.http_code = ret;
        if (ret != FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_INUSED)
            fbdo->_ss.connected = false;
        return false;
    }

    return true;
}

int FB_RTDB::sendRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{

    fbdo->_ss.error.clear();

    if (fbdo->_ss.rtdb.pause)
        return 0;

    if (!fbdo->reconnect())
        return FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST;

    if (!fbdo->tokenReady())
        return FIREBASE_ERROR_TOKEN_NOT_READY;

    if (!fbdo->validRequest(req->path))
        return FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST;

    if (fbdo->_ss.long_running_task > 0)
    {
        return FIREBASE_ERROR_LONG_RUNNING_TASK;
    }
    uint8_t attempts = 0;
    uint8_t maxRetry = 5;
    size_t buffSize = 128;
    char *tmp = nullptr;
    char *buf = nullptr;

    int len = 0;
    size_t toRead = 0;
    int ret = -1;

    std::string payloadBuf = "";
    std::string header = "";

    rescon(fbdo, Signer.getCfg()->database_url.c_str(), req);

    if (req->method == m_stream)
    {
        fbdo->_ss.rtdb.stream_path.clear();

        if (req->path.length() > 0)
            if (req->path[0] != '/')
                ut->appendP(fbdo->_ss.rtdb.stream_path, fb_esp_pgm_str_1, true);

        fbdo->_ss.rtdb.stream_path += req->path;
    }
    else
    {
        fbdo->_ss.rtdb.path.clear();
        fbdo->_ss.rtdb.resp_etag.clear();

        if (req->method != m_download && req->method != m_restore)
        {
            fbdo->_ss.rtdb.path.clear();
            if (req->path.length() > 0)
                if (req->path[0] != '/')
                    ut->appendP(fbdo->_ss.rtdb.path, fb_esp_pgm_str_1, true);
            fbdo->_ss.rtdb.path += req->path;
        }

        fbdo->_ss.rtdb.data_tmo = false;
    }
#if defined(ESP8266)
    if (fbdo->httpClient._certType > 0 && !Signer.getCfg()->_int.fb_clock_rdy)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_CANNOT_CONFIG_TIME;
        return FIREBASE_ERROR_CANNOT_CONFIG_TIME;
    }
#endif
    fbdo->httpClient.begin(Signer.getCfg()->database_url.c_str(), FIREBASE_PORT);

    //Prepare for string and JSON payloads
    preparePayload(req, payloadBuf);

    //Prepare request header
    if (req->method != m_download && req->method != m_restore && req->data.type != d_file)
    {
        //for non-file data
        bool sv = false;
        if (req->data.type == d_json)
        {
            tmp = ut->strP(fb_esp_pgm_str_166);
            sv = payloadBuf.find(tmp) != std::string::npos;
            ut->delS(tmp);
        }

        prepareHeader(fbdo, req, payloadBuf.length(), header, sv);
    }
    else
    {
        //for file data payload
        bool fileChecked = false;
        if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
        {
            if (!Signer.getCfg()->_int.fb_flash_rdy)
                ut->flashTest();

            if (!Signer.getCfg()->_int.fb_flash_rdy)
            {
                ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_164, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }
        }
        else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
        {
            if (Signer.getCfg()->_int.fb_sd_used)
            {
                ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_84, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }

            if (!Signer.getCfg()->_int.fb_sd_rdy)
                Signer.getCfg()->_int.fb_sd_rdy = ut->sdTest(Signer.getCfg()->_int.fb_file);

            if (!Signer.getCfg()->_int.fb_sd_rdy)
            {
                ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_85, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }

            Signer.getCfg()->_int.fb_sd_used = true;
        }

        if (req->method == m_download || req->method == m_restore)
        {
            if (req->method == m_download)
            {
                if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                {
                    FLASH_FS.remove(fbdo->_ss.rtdb.backup_filename.c_str());
                    Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), "w");
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {
                    SD_FS.remove(fbdo->_ss.rtdb.backup_filename.c_str());
                    Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), FILE_WRITE);
                }
            }
            else if (req->method == m_restore)
            {

                if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash && FLASH_FS.exists(fbdo->_ss.rtdb.backup_filename.c_str()))
                    Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), "r");
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd && SD_FS.exists(fbdo->_ss.rtdb.backup_filename.c_str()))
                    Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), FILE_READ);
                else
                {
                    ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_83, true);
                    return FIREBASE_ERROR_FILE_IO_ERROR;
                }
                len = Signer.getCfg()->_int.fb_file.size();
            }

            fileChecked = true;
        }

        if (req->data.type == d_file)
        {

            if (req->method == m_put_nocontent || req->method == m_post)
            {
                if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                {
                    if (FLASH_FS.exists(fbdo->_ss.rtdb.file_name.c_str()))
                        Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.file_name.c_str(), "r");
                    else
                    {
                        ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_83, true);
                        return FIREBASE_ERROR_FILE_IO_ERROR;
                    }
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {
                    if (SD_FS.exists(fbdo->_ss.rtdb.file_name.c_str()))
                        Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.file_name.c_str(), FILE_READ);
                    else
                    {
                        ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_83, true);
                        return FIREBASE_ERROR_FILE_IO_ERROR;
                    }
                }
                len = (4 * ceil(Signer.getCfg()->_int.fb_file.size() / 3.0)) + strlen_P(fb_esp_pgm_str_93) + 1;
            }
            else if (req->method == m_get)
            {
                tmp = ut->strP(fb_esp_pgm_str_1);
                size_t p1 = fbdo->_ss.rtdb.file_name.find_last_of(tmp);
                ut->delS(tmp);
                std::string folder = "/";

                if (p1 != std::string::npos && p1 != 0)
                    folder = fbdo->_ss.rtdb.file_name.substr(p1 - 1);

                if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                {
                    Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.file_name.c_str(), "w");
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {

                    if (!SD_FS.exists(folder.c_str()))
                        ut->createDirs(folder, (fb_esp_mem_storage_type)fbdo->_ss.rtdb.storage_type);

                    SD_FS.remove(fbdo->_ss.rtdb.file_name.c_str());

                    Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.file_name.c_str(), FILE_WRITE);
                }
                std::string().swap(folder);
            }

            fileChecked = true;
        }

        if (fileChecked)
        {
            if (!Signer.getCfg()->_int.fb_file)
            {
                ut->appendP(fbdo->_ss.error, fb_esp_pgm_str_86, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }
        }

        if (req->data.type == d_file)
            prepareHeader(fbdo, req, len, header, false);
        else
        {
            struct fb_esp_rtdb_request_info_t _req;
            _req.data.type = req->data.type;
            _req.method = req->method;
            _req.path = fbdo->_ss.rtdb.backup_node_path;
            prepareHeader(fbdo, &_req, len, header, false);
        }
    }

    if (req->method == m_get_nocontent || req->method == m_patch_nocontent || (req->method == m_put_nocontent && req->data.type == d_blob))
        fbdo->_ss.rtdb.no_content_req = true;

    if (req->data.type == d_blob)
        std::vector<uint8_t>().swap(fbdo->_ss.rtdb.blob);

    if (!fbdo->reconnect())
        return FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST;

    //Send request
    ret = fbdo->httpClient.send(header.c_str(), payloadBuf.c_str());

    //Retry
    if (req->method != m_stream)
    {
        attempts = 0;
        while (ret != 0)
        {
            attempts++;
            if (attempts > maxRetry)
                break;

            if (!fbdo->reconnect())
                return FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST;

            ret = fbdo->httpClient.send(header.c_str(), payloadBuf.c_str());
        }
    }

    header.clear();
    payloadBuf.clear();
    std::string().swap(header);
    std::string().swap(payloadBuf);

    //handle send file data
    if (req->method == m_restore || (req->data.type == d_file && (req->method == m_put_nocontent || req->method == m_post)))
    {
        if (req->data.type == d_file && (req->method == m_put_nocontent || req->method == m_post))
        {

            buf = ut->strP(fb_esp_pgm_str_93);
            ret = fbdo->httpClient.send("", buf);
            ut->delS(buf);
            if (ret == 0)
                ut->sendBase64Stream(fbdo->httpClient.stream(), fbdo->_ss.rtdb.file_name, fbdo->_ss.rtdb.storage_type, Signer.getCfg()->_int.fb_file);
            else
                return FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED;

            buf = ut->newS(2);
            buf[0] = '"';
            buf[1] = '\0';
            ret = fbdo->httpClient.send("", buf);
            ut->delS(buf);

            if (ret != 0)
                return FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED;
        }
        else
        {
            while (len)
            {
                if (!fbdo->reconnect())
                    return FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST;

                toRead = len;
                if (toRead > buffSize)
                    toRead = buffSize - 1;

                buf = ut->newS(buffSize);
                Signer.getCfg()->_int.fb_file.read((uint8_t *)buf, toRead);

                buf[toRead] = '\0';

                ret = fbdo->httpClient.send("", buf);
                ut->delS(buf);

                if (ret != 0)
                    return FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED;

                len -= toRead;

                if (len <= 0)
                    break;
            }
        }

        ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
    }

    fbdo->_ss.connected = ret == 0;
    return ret;
}

bool FB_RTDB::waitResponse(FirebaseData *fbdo)
{
#if defined(ESP32)

    //if currently perform stream payload handling process, skip it.
    if (Signer.getCfg()->_int.fb_processing && fbdo->_ss.con_mode == fb_esp_con_mode_rtdb_stream)
        return true;

    //set the blocking flag
    Signer.getCfg()->_int.fb_processing = true;
    bool ret = handleResponse(fbdo);
    //reset the blocking flag
    Signer.getCfg()->_int.fb_processing = false;

    return ret;
#elif defined(ESP8266)
    return handleResponse(fbdo);
#endif
}

bool FB_RTDB::handleResponse(FirebaseData *fbdo)
{

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect())
        return false;

    WiFiClient *stream = fbdo->httpClient.stream();

    if (!fbdo->_ss.connected || stream == nullptr)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;
        return false;
    }

    unsigned long dataTime = millis();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    std::string header;
    std::string payload;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->_ss.resp_size;
    int pBufPos = 0;
    int chunkBufSize = stream->available();
    bool redirect = false;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->_ss.resp_size;

    if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_UNDEFINED)
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.rtdb.push_name.clear();
    fbdo->_ss.rtdb.data_mismatch = false;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    if (fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream)
    {
        if (fbdo->_ss.rtdb.async)
        {
#if defined(ESP32)
            chunkBufSize = stream->available();
            if (chunkBufSize > 0)
            {
                char buf[chunkBufSize];
                stream->readBytes(buf, chunkBufSize);
            }
#endif
            return true;
        }
        else
        {
            while (fbdo->httpClient.connected() && chunkBufSize <= 0)
            {
                if (!fbdo->reconnect(dataTime) || stream == nullptr)
                {
                    fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;
                    return false;
                }
                chunkBufSize = stream->available();
                delay(0);
            }
        }
    }

    dataTime = millis();

    while (chunkBufSize > 0)
    {
        if (!fbdo->reconnect(dataTime) || stream == nullptr)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;
            return false;
        }

        chunkBufSize = stream->available();

        if (chunkBufSize <= 0)
            break;

        if (chunkBufSize > 0)
        {
            if (pChunkIdx == 0)
            {
                if (chunkBufSize > defaultChunkSize + (int)strlen_P(fb_esp_pgm_str_93))
                    chunkBufSize = defaultChunkSize + strlen_P(fb_esp_pgm_str_93); //plus file header length for later base64 decoding
            }
            else
            {
                if (chunkBufSize > defaultChunkSize)
                    chunkBufSize = defaultChunkSize;
            }

            if (chunkIdx == 0)
            {
                if (!stream)
                    break;

                //the first chunk can be stream event data (no header) or http response header
                ut->readLine(stream, header);
                int pos = 0;

                response.noEvent = fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream;

                tmp = ut->getHeader(header.c_str(), fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                delay(0);
                dataTime = millis();
                if (tmp)
                {
                    //http response header with http response code
                    isHeader = true;
                    response.httpCode = atoi(tmp);
                    fbdo->_ss.http_code = response.httpCode;
                    ut->delS(tmp);
                }
                else
                {
                    //stream payload data
                    payload = header;
                }
            }
            else
            {
                delay(0);
                dataTime = millis();
                //the next chunk data can be the remaining http header
                if (isHeader)
                {
                    //read one line of next header field until the empty header has found
                    tmp = ut->newS(chunkBufSize + 10);
                    bool headerEnded = false;
                    int readLen = 0;
                    if (tmp)
                    {
                        if (!stream)
                        {
                            ut->delS(tmp);
                            break;
                        }

                        readLen = ut->readLine(stream, tmp, chunkBufSize);

                        //check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (tmp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (tmp[0] == '\r' && tmp[1] == '\n')
                                headerEnded = true;
                    }

                    if (headerEnded)
                    {
                        //parse header string to get the header field
                        isHeader = false;
                        ut->parseRespHeader(header.c_str(), response);
                        fbdo->_ss.rtdb.resp_etag = response.etag;

                        std::string().swap(header);

                        if (response.httpCode == 401)
                            Signer.authenticated = false;
                        else if (response.httpCode < 300)
                            Signer.authenticated = true;

                        //error in request or server
                        if (response.httpCode >= 400)
                        {

                            if (ut->strposP(response.contentType.c_str(), fb_esp_pgm_str_74, 0) < 0)
                            {
                                if (stream)
                                {
                                    size_t len = stream->available();
                                    char buf[len];
                                    stream->readBytes(buf, len);
                                    while (stream->available())
                                    {
                                        delay(0);
                                        stream->read();
                                    }
                                    if (tmp)
                                        ut->delS(tmp);
                                    return false;
                                }
                                else
                                {
                                    if (tmp)
                                        ut->delS(tmp);
                                }
                            }
                        }

                        if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT || fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY || fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_FOUND)
                        {
                            if (response.location.length() > 0)
                            {
                                fbdo->_ss.rtdb.redirect_url = response.location;
                                redirect = true;
                                if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_FOUND)
                                    fbdo->_ss.rtdb.redirect = 1;
                                else
                                    fbdo->_ss.rtdb.redirect = 2;
                            }
                        }

                        if (ut->stringCompare(response.connection.c_str(), 0, fb_esp_pgm_str_11))
                            fbdo->_ss.rtdb.http_resp_conn_type = fb_esp_http_connection_type_keep_alive;
                        else
                            fbdo->_ss.rtdb.http_resp_conn_type = fb_esp_http_connection_type_close;

                        fbdo->_ss.chunked_encoding = response.isChunkedEnc;

                        if (fbdo->_ss.rtdb.req_method == m_download)
                            fbdo->_ss.rtdb.backup_file_size = response.contentLen;
                    }
                    else
                    {
                        if (tmp)
                        {
                            //accumulate the remaining header field
                            header += tmp;
                        }
                    }
                    if (tmp)
                        ut->delS(tmp);
                }
                else
                {

                    //the next chuunk data is the payload
                    if (!response.noContent && !fbdo->_ss.buffer_ovf)
                    {
                        pChunkIdx++;

                        pChunk = ut->newS(chunkBufSize + 10);

                        if (!pChunk)
                            break;

                        //read the avilable data
                        int readLen = 0;

                        if (!stream)
                        {
                            ut->delS(pChunk);
                            break;
                        }

                        //chunk transfer encoding?
                        if (response.isChunkedEnc)
                            readLen = ut->readChunkedData(stream, pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                        else
                        {
                            if (stream->available() < chunkBufSize)
                                chunkBufSize = stream->available();
                            readLen = stream->readBytes(pChunk, chunkBufSize);
                        }

                        if (readLen > 0)
                        {
                            fbdo->checkOvf(payload.length() + readLen, response);

                            if (!fbdo->_ss.buffer_ovf)
                                payload += pChunk;
                        }

                        if (!fbdo->_ss.rtdb.data_tmo && !fbdo->_ss.buffer_ovf)
                        {
                            //try to parse the payload
                            if (response.dataType == 0 && !response.isEvent && !response.noContent)
                            {
                                if (fbdo->_ss.rtdb.req_data_type == d_blob || fbdo->_ss.rtdb.req_method == m_download || (fbdo->_ss.rtdb.req_data_type == d_file && fbdo->_ss.rtdb.req_method == m_get))
                                    ut->parseRespPayload(payload.c_str(), response, true);
                                else
                                    ut->parseRespPayload(payload.c_str(), response, false);

                                fbdo->_ss.error = response.fbError;
                                if (fbdo->_ss.rtdb.req_method == m_download || fbdo->_ss.rtdb.req_method == m_restore)
                                    fbdo->_ss.error = response.fbError;

                                if (fbdo->_ss.rtdb.req_method == m_download && response.dataType != d_json)
                                {
                                    fbdo->_ss.http_code = FIREBASE_ERROR_EXPECTED_JSON_DATA;
                                    tmp = ut->strP(fb_esp_pgm_str_185);
                                    if (tmp)
                                    {
                                        fbdo->_ss.error = tmp;
                                        ut->delS(tmp);
                                    }

                                    std::string().swap(header);
                                    std::string().swap(payload);
                                    ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
                                    fbdo->closeSession();
                                    return false;
                                }
                            }

                            //in case of the payload data type is file, decode and write stream to temp file
                            if (response.dataType == d_file || (fbdo->_ss.rtdb.req_method == m_download || (fbdo->_ss.rtdb.req_data_type == d_file && fbdo->_ss.rtdb.req_method == m_get)))
                            {
                                int ofs = 0;
                                int len = readLen;

                                if (fbdo->_ss.rtdb.req_method == m_download || (fbdo->_ss.rtdb.req_data_type == d_file && fbdo->_ss.rtdb.req_method == m_get))
                                {
                                    if (fbdo->_ss.rtdb.file_name.length() == 0)
                                    {
                                        size_t writeLen = 0;
                                        if (Signer.getCfg()->_int.fb_file)
                                            writeLen = Signer.getCfg()->_int.fb_file.write((uint8_t *)payload.c_str(), readLen);

                                        fbdo->_ss.rtdb.backup_file_size = writeLen;
                                        if (stream->available() == 0)
                                        {
                                            fbdo->closeSession();
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (pChunkIdx == 1)
                                        {
                                            len = payloadLen - response.payloadOfs;
                                            ofs = response.payloadOfs;
                                        }

                                        if (payload[len - 1] == '"')
                                            payload[len - 1] = 0;

                                        if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                                            ut->decodeBase64Flash(payload.c_str() + ofs, len, Signer.getCfg()->_int.fb_file);
                                        else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                                            ut->decodeBase64Stream(payload.c_str() + ofs, len, Signer.getCfg()->_int.fb_file);
                                    }
                                }
                                else
                                {
                                    if (!Signer.getCfg()->_int.fb_file)
                                    {
                                        tmp = ut->strP(fb_esp_pgm_str_184);
                                        if (!Signer.getCfg()->_int.fb_flash_rdy)
                                            ut->flashTest();
                                        if (tmp)
                                        {
                                            Signer.getCfg()->_int.fb_file = FLASH_FS.open(tmp, "w");
                                            ut->delS(tmp);
                                        }
                                        readLen = payloadLen - response.payloadOfs;
                                        ofs = response.payloadOfs;
                                    }

                                    if (Signer.getCfg()->_int.fb_file)
                                    {
                                        if (payload[len - 1] == '"')
                                            payload[len - 1] = 0;

                                        if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                                            ut->decodeBase64Flash(payload.c_str() + ofs, len, Signer.getCfg()->_int.fb_file);
                                        else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                                            ut->decodeBase64Stream(payload.c_str() + ofs, len, Signer.getCfg()->_int.fb_file);
                                    }
                                }

                                if (response.dataType > 0)
                                {
                                    std::string().swap(payload);
                                    pBufPos = 0;
                                    readLen = 0;
                                }
                            }
                        }

                        if (pChunk)
                        {
                            if (ut->strposP(pChunk, fb_esp_pgm_str_14, 0) > -1 && response.isEvent)
                            {
                                ut->delS(pChunk);
                                break;
                            }

                            ut->delS(pChunk);
                        }

                        if (readLen < 0)
                            break;
                        pBufPos += readLen;
                    }
                    else
                    {
                        if (!stream)
                            break;
                        //read all the rest data
                        while (stream->available() > 0)
                            stream->read();
                    }
                }
            }

            chunkIdx++;
        }
    }

    std::string().swap(header);

    if (!fbdo->_ss.rtdb.data_tmo && !fbdo->_ss.buffer_ovf)
    {
        //parse the payload
        if (payload.length() > 0)
        {
            if (response.isEvent)
            {

                int pos = 0, ofs = 0, num = 0;
                bool valid = false;

                while (pos > -1)
                {
                    pos = ut->strposP(payload.c_str(), fb_esp_pgm_str_13, ofs);
                    if (pos > -1)
                    {
                        ofs = pos + 1;
                        num++;
                    }
                }

                if (num > 1)
                {
                    std::vector<std::string> payloadList = std::vector<std::string>();
                    splitStreamPayload(payload.c_str(), payloadList);
                    for (size_t i = 0; i < payloadList.size(); i++)
                    {
                        if (ut->validJS(payloadList[i].c_str()))
                        {
                            valid = true;
                            parseStreamPayload(fbdo, payloadList[i].c_str());
                            sendCB(fbdo);
                        }
                    }
                    payloadList.clear();
                }
                else
                {
                    valid = ut->validJS(payload.c_str());
                    if (valid)
                    {
                        parseStreamPayload(fbdo, payload.c_str());
                        sendCB(fbdo);
                    }
                }

                std::string().swap(payload);

                if (valid)
                {
                    fbdo->_ss.rtdb.data_millis = millis();
                    fbdo->_ss.rtdb.data_tmo = false;
                }
                else
                {
                    fbdo->_ss.rtdb.data_millis = 0;
                    fbdo->_ss.rtdb.data_tmo = true;
                    fbdo->closeSession();
                }

                return valid;
            }
            else
            {

                //the payload ever parsed?
                if (response.dataType == 0 && !response.noContent)
                {
                    if (fbdo->_ss.rtdb.req_data_type == d_blob || fbdo->_ss.rtdb.req_method == m_download || (fbdo->_ss.rtdb.req_data_type == d_file && fbdo->_ss.rtdb.req_method == m_get))
                        ut->parseRespPayload(payload.c_str(), response, true);
                    else
                        ut->parseRespPayload(payload.c_str(), response, false);

                    fbdo->_ss.error = response.fbError;
                }

                fbdo->_ss.rtdb.resp_data_type = response.dataType;
                fbdo->_ss.content_length = response.payloadLen;

                fbdo->_ss.json.clear();
                fbdo->_ss.arr.clear();

                if (fbdo->_ss.rtdb.resp_data_type == d_blob)
                {
                    std::vector<uint8_t>().swap(fbdo->_ss.rtdb.blob);
                    fbdo->_ss.rtdb.data.clear();
                    fbdo->_ss.rtdb.data2.clear();
                    ut->decodeBase64Str((const char *)payload.c_str() + response.payloadOfs, fbdo->_ss.rtdb.blob);
                }
                else if (fbdo->_ss.rtdb.resp_data_type == d_file || Signer.getCfg()->_int.fb_file)
                {
                    ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
                    fbdo->_ss.rtdb.data.clear();
                    fbdo->_ss.rtdb.data2.clear();
                }

                if (fbdo->_ss.rtdb.req_method == m_set_rules)
                {
                    if (ut->stringCompare(payload.c_str(), 0, fb_esp_pgm_str_104))
                        std::string().swap(payload);
                }

                if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_OK || fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED)
                {

                    if (fbdo->_ss.rtdb.req_method != m_set_rules)
                    {
                        if (fbdo->_ss.rtdb.resp_data_type != d_blob && fbdo->_ss.rtdb.resp_data_type != d_file)
                        {
                            handlePayload(fbdo, response, payload.c_str());

                            if (fbdo->_ss.rtdb.priority_val_flag)
                            {
                                char *path = ut->newS(fbdo->_ss.rtdb.path.length());
                                if (path)
                                {
                                    strncpy(path, fbdo->_ss.rtdb.path.c_str(), fbdo->_ss.rtdb.path.length() - strlen_P(fb_esp_pgm_str_156));
                                    fbdo->_ss.rtdb.path = path;
                                    ut->delS(path);
                                }
                            }

                            //Push (POST) data?
                            if (fbdo->_ss.rtdb.req_method == m_post)
                            {
                                if (response.pushName.length() > 0)
                                {
                                    fbdo->_ss.rtdb.push_name = response.pushName;
                                    fbdo->_ss.rtdb.resp_data_type = d_any;
                                    fbdo->_ss.rtdb.data.clear();
                                }
                            }
                        }
                    }
                }

                if (fbdo->_ss.rtdb.resp_data_type != d_null && fbdo->_ss.rtdb.req_data_type != d_timestamp && !response.noContent && fbdo->_ss.rtdb.req_method != m_post && fbdo->_ss.rtdb.req_method != m_get_shallow && response.httpCode < 400)
                {
                    bool _reqType = fbdo->_ss.rtdb.req_data_type == d_integer || fbdo->_ss.rtdb.req_data_type == d_float || fbdo->_ss.rtdb.req_data_type == d_double;
                    bool _respType = fbdo->_ss.rtdb.resp_data_type == d_integer || fbdo->_ss.rtdb.resp_data_type == d_float || fbdo->_ss.rtdb.resp_data_type == d_double;

                    if (fbdo->_ss.rtdb.req_data_type == fbdo->_ss.rtdb.resp_data_type || (_reqType && _respType))
                        fbdo->_ss.rtdb.data_mismatch = false;
                    else if (fbdo->_ss.rtdb.req_data_type != d_any)
                    {
                        fbdo->_ss.rtdb.data_mismatch = true;
                        fbdo->_ss.http_code = FIREBASE_ERROR_DATA_TYPE_MISMATCH;
                    }
                }
            }
        }
    }

    if (fbdo->_ss.rtdb.no_content_req || response.noContent)
    {
        if (ut->stringCompare(fbdo->_ss.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151) && response.noContent)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_PATH_NOT_EXIST;
            fbdo->_ss.rtdb.path_not_found = true;
        }
        else
            fbdo->_ss.rtdb.path_not_found = false;

        if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
            fbdo->_ss.rtdb.path.clear();
            fbdo->_ss.rtdb.data.clear();
            fbdo->_ss.json.clear();
            fbdo->_ss.arr.clear();
            fbdo->_ss.data.stringValue.clear();
            fbdo->_ss.data.success = false;
            fbdo->_ss.data.boolValue = false;
            fbdo->_ss.data.intValue = 0;
            fbdo->_ss.data.floatValue = 0;
            fbdo->_ss.data.doubleValue = 0;
            fbdo->_ss.rtdb.push_name.clear();
            fbdo->_ss.rtdb.resp_data_type = d_any;
            fbdo->_ss.rtdb.data_available = false;
        }
    }

    ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);

    std::string().swap(payload);

    if (!redirect)
    {

        if (fbdo->_ss.rtdb.redirect == 1 && fbdo->_ss.rtdb.redirect_count > 1)
            fbdo->_ss.rtdb.redirect_url.clear();
    }
    else
    {

        fbdo->_ss.rtdb.redirect_count++;

        if (fbdo->_ss.rtdb.redirect_count > MAX_REDIRECT)
        {
            fbdo->_ss.rtdb.redirect = 0;
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_MAX_REDIRECT_REACHED;
        }
        else
        {
            struct fb_esp_url_info_t uinfo;
            ut->getUrlInfo(fbdo->_ss.rtdb.redirect_url, uinfo);
            struct fb_esp_rtdb_request_info_t _req;
            _req.method = fbdo->_ss.rtdb.req_method;
            _req.data.type = fbdo->_ss.rtdb.req_data_type;
            _req.priority = fbdo->_ss.rtdb.priority;
            _req.path = uinfo.uri;
            if (sendRequest(fbdo, &_req) == 0)
                return waitResponse(fbdo);
        }
    }

    return fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_OK;
}

void FB_RTDB::sendCB(FirebaseData *fbdo)
{
    // prevent the data available and stream data changed flags reset by
    // streamAvailable without stream callbacks assigned.
    if (!fbdo->_dataAvailableCallback && !fbdo->_multiPathDataCallback)
        return;

    if (!fbdo->streamAvailable())
        return;

    // to allow other subsequence request which can be occurred in the user stream
    // callback
    Signer.getCfg()->_int.fb_processing = false;

    if (fbdo->_dataAvailableCallback)
    {
        FIREBASE_STREAM_CLASS s;
        s.begin(ut, &fbdo->_ss.rtdb.stream);
        s._json = &fbdo->_ss.json;
        s._jsonArr = &fbdo->_ss.arr;
        s._jsonData = &fbdo->_ss.data;
        s.sif->stream_path = fbdo->_ss.rtdb.stream_path.c_str();
        s.sif->data = fbdo->_ss.rtdb.data.c_str();
        s.sif->path = fbdo->_ss.rtdb.path.c_str();

        s.sif->data_type = fbdo->_ss.rtdb.resp_data_type;
        s.sif->data_type_str = fbdo->getDataType(s.sif->data_type).c_str();
        s.sif->event_type_str = fbdo->_ss.rtdb.event_type.c_str();

        if (fbdo->_ss.rtdb.resp_data_type == d_blob)
            s.sif->blob = &fbdo->_ss.rtdb.blob;
            
        fbdo->_dataAvailableCallback(s);
        fbdo->_ss.rtdb.data_available = false;
        s.empty();
    }
    else if (fbdo->_multiPathDataCallback)
    {
        FIREBASE_MP_STREAM_CLASS mdata;
        mdata.begin(ut, &fbdo->_ss.rtdb.stream);
        mdata.sif->m_type = fbdo->_ss.rtdb.resp_data_type;
        mdata.sif->m_path = fbdo->_ss.rtdb.path.c_str();
        mdata.sif->m_type_str = fbdo->getDataType(mdata.sif->m_type).c_str();
        mdata.sif->m_event_type_str = fbdo->_ss.rtdb.event_type.c_str();

        if (mdata.sif->m_type == d_json)
            mdata.sif->m_json = &fbdo->_ss.json;
        else
        {
            if (mdata.sif->m_type == d_string)
                mdata.sif->m_data = fbdo->_ss.rtdb.data.substr(1, fbdo->_ss.rtdb.data.length() - 2).c_str();
            else
                mdata.sif->m_data = fbdo->_ss.rtdb.data.c_str();
        }

        fbdo->_multiPathDataCallback(mdata);
        fbdo->_ss.rtdb.data_available = false;
        mdata.empty();
    }
}

void FB_RTDB::splitStreamPayload(const char *payloads, std::vector<std::string> &payload)
{
    int ofs = 0;
    int pos1 = 0, pos2 = 0, pos3 = 0;

    while (pos1 > -1)
    {
        pos1 = ut->strposP(payloads, fb_esp_pgm_str_13, ofs);
        if (pos1 > -1)
        {
            ofs = pos1 + 1;
            pos2 = ut->strposP(payloads, fb_esp_pgm_str_14, ofs);
            if (pos2 > -1)
            {
                ofs = pos2 + 1;
                pos3 = ut->strposP(payloads, fb_esp_pgm_str_180, ofs);

                if (pos3 > -1)
                    ofs = pos3 + 1;
                else
                    pos3 = strlen(payloads);

                size_t len = pos3 - pos1;
                char *tmp = ut->newS(len + 10);
                if (tmp)
                {
                    strncpy(tmp, payloads + pos1, len);
                    payload.push_back(tmp);
                    ut->delS(tmp);
                }
            }
        }
    }
}

void FB_RTDB::parseStreamPayload(FirebaseData *fbdo, const char *payload)
{
    struct server_response_data_t response;

    ut->parseRespPayload(payload, response, false);

    fbdo->_ss.rtdb.resp_data_type = response.dataType;
    fbdo->_ss.content_length = response.payloadLen;

    fbdo->_ss.json.clear();
    fbdo->_ss.arr.clear();

    if (fbdo->_ss.rtdb.resp_data_type == d_blob)
    {
        std::vector<uint8_t>().swap(fbdo->_ss.rtdb.blob);
        fbdo->_ss.rtdb.data.clear();
        fbdo->_ss.rtdb.data2.clear();
        ut->decodeBase64Str((const char *)payload + response.payloadOfs, fbdo->_ss.rtdb.blob);
    }
    else if (fbdo->_ss.rtdb.resp_data_type == d_file || Signer.getCfg()->_int.fb_file)
    {
        ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
        fbdo->_ss.rtdb.data.clear();
        fbdo->_ss.rtdb.data2.clear();
    }

    if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_15) || ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_16))
    {

        handlePayload(fbdo, response, payload);

        //Any stream update?
        //based on BLOB or file event data changes (no old data available for comparision or inconvenient for large data)
        //event path changes
        //event data changes without the path changes
        if (fbdo->_ss.rtdb.resp_data_type == d_blob ||
            fbdo->_ss.rtdb.resp_data_type == d_file ||
            response.eventPathChanged ||
            (!response.eventPathChanged && response.dataChanged && !fbdo->_ss.rtdb.stream_path_changed))
        {
            fbdo->_ss.rtdb.stream_data_changed = true;
            fbdo->_ss.rtdb.data2.clear();
            fbdo->_ss.rtdb.data2 = fbdo->_ss.rtdb.data;
        }
        else
            fbdo->_ss.rtdb.stream_data_changed = false;

        fbdo->_ss.rtdb.data_available = true;
        fbdo->_ss.rtdb.stream_path_changed = false;
    }
    else
    {
        //Firebase keep alive event
        if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_11))
        {
            if (fbdo->_timeoutCallback)
                fbdo->_timeoutCallback(false);
        }

        //Firebase cancel and auth_revoked events
        else if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_109) || ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_110))
        {
            fbdo->_ss.rtdb.event_type = response.eventType;
            //make stream available status
            fbdo->_ss.rtdb.stream_data_changed = true;
            fbdo->_ss.rtdb.data_available = true;
        }
    }
}

void FB_RTDB::handlePayload(FirebaseData *fbdo, struct server_response_data_t &response, const char *payload)
{
    String rawArr;
    String rawJson;

    fbdo->_ss.json.toString(rawJson);
    fbdo->_ss.arr.toString(rawArr);

    fbdo->_ss.rtdb.data.clear();
    fbdo->_ss.data.stringValue.clear();
    fbdo->_ss.json.clear();
    fbdo->_ss.arr.clear();

    if (response.isEvent)
    {
        response.eventPathChanged = strcmp(response.eventPath.c_str(), fbdo->_ss.rtdb.path.c_str()) != 0;
        fbdo->_ss.rtdb.path = response.eventPath;
        fbdo->_ss.rtdb.event_type = response.eventType;
    }

    if (fbdo->_ss.rtdb.resp_data_type == d_json)
    {
        if (response.isEvent)
            fbdo->_ss.json.setJsonData(response.eventData.c_str());
        else
            fbdo->_ss.json.setJsonData(payload);
        String test;
        fbdo->_ss.json.toString(test);
        if (test != rawJson)
            response.dataChanged = true;
        test.clear();
    }
    else if (fbdo->_ss.rtdb.resp_data_type == d_array)
    {
        if (response.isEvent)
            fbdo->_ss.arr.setJsonArrayData(response.eventData.c_str());
        else
            fbdo->_ss.arr.setJsonArrayData(payload);
        String test;
        fbdo->_ss.arr.toString(test);
        if (test != rawArr)
            response.dataChanged = true;
        test.clear();
    }
    else if (fbdo->_ss.rtdb.resp_data_type != d_blob && fbdo->_ss.rtdb.resp_data_type != d_file)
    {
        if (response.isEvent)
            fbdo->_ss.rtdb.data = response.eventData;
        else
            fbdo->_ss.rtdb.data = payload;

        response.dataChanged = fbdo->_ss.rtdb.data2 != fbdo->_ss.rtdb.data;
    }

    rawJson.clear();
    rawArr.clear();
    fbdo->_ss.rtdb.data2.clear();
}

void FB_RTDB::prepareHeader(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req, int payloadLength, std::string &header, bool sv)
{
    fb_esp_method http_method = m_put;
    char *tmp = nullptr;
    fbdo->_ss.rtdb.shallow_flag = false;
    fbdo->_ss.rtdb.priority_val_flag = false;

    header.clear();
    if (req->method == m_stream)
        ut->appendP(header, fb_esp_pgm_str_22, true);
    else
    {
        if (req->method == m_put || req->method == m_put_nocontent || req->method == m_set_priority || req->method == m_set_rules)
        {
            http_method = m_put;
            if (fbdo->_ss.classic_request)
                ut->appendP(header, fb_esp_pgm_str_24, true);
            else
                ut->appendP(header, fb_esp_pgm_str_23, true);
        }
        else if (req->method == m_post)
        {
            http_method = m_post;
            ut->appendP(header, fb_esp_pgm_str_24, true);
        }
        else if (req->method == m_get || req->method == m_get_nocontent || req->method == m_get_shallow || req->method == m_get_priority || req->method == m_download || req->method == m_read_rules)
        {
            http_method = m_get;
            ut->appendP(header, fb_esp_pgm_str_25, true);
        }
        else if (req->method == m_patch || req->method == m_patch_nocontent || req->method == m_restore)
        {
            http_method = m_patch;
            ut->appendP(header, fb_esp_pgm_str_26, true);
        }
        else if (req->method == m_delete)
        {
            http_method = m_delete;
            if (fbdo->_ss.classic_request)
                ut->appendP(header, fb_esp_pgm_str_24, true);
            else
                ut->appendP(header, fb_esp_pgm_str_27, true);
        }
        ut->appendP(header, fb_esp_pgm_str_6);
    }

    if (req->path.length() > 0)
        if (req->path[0] != '/')
            ut->appendP(header, fb_esp_pgm_str_1);

    header += req->path;

    if (req->method == m_patch || req->method == m_patch_nocontent)
        ut->appendP(header, fb_esp_pgm_str_1);

    bool appendAuth = false;

    if (fbdo->_ss.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        ut->getUrlInfo(fbdo->_ss.rtdb.redirect_url, uinfo);
        if (uinfo.auth.length() == 0)
            appendAuth = true;
    }
    else
        appendAuth = true;

    if (appendAuth)
    {

        if (Signer.getTokenType() == token_type_oauth2_access_token)
            ut->appendP(header, fb_esp_pgm_str_238);
        else
            ut->appendP(header, fb_esp_pgm_str_2);

        if (Signer.getTokenType() == token_type_legacy_token)
            header += Signer.getToken(token_type_legacy_token);

        if (Signer.getTokenType() == token_type_id_token || Signer.getTokenType() == token_type_custom_token)
            header += Signer.getToken(token_type_id_token);
    }

    if (fbdo->_ss.rtdb.read_tmo > 0)
    {
        ut->appendP(header, fb_esp_pgm_str_158);
        tmp = ut->intStr(fbdo->_ss.rtdb.read_tmo);
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_159);
    }

    if (fbdo->_ss.rtdb.write_limit.length() > 0)
    {
        ut->appendP(header, fb_esp_pgm_str_160);
        header += fbdo->_ss.rtdb.write_limit;
    }

    if (req->method == m_get_shallow)
    {
        ut->appendP(header, fb_esp_pgm_str_155);
        fbdo->_ss.rtdb.shallow_flag = true;
    }

    if (req->method == m_get && fbdo->queryFilter._orderBy.length() > 0)
    {
        ut->appendP(header, fb_esp_pgm_str_96);
        header += fbdo->queryFilter._orderBy;

        if (req->method == m_get)
        {
            if (fbdo->queryFilter._limitToFirst.length() > 0)
            {
                ut->appendP(header, fb_esp_pgm_str_97);
                header += fbdo->queryFilter._limitToFirst;
            }

            if (fbdo->queryFilter._limitToLast.length() > 0)
            {
                ut->appendP(header, fb_esp_pgm_str_98);
                header += fbdo->queryFilter._limitToLast;
            }

            if (fbdo->queryFilter._startAt.length() > 0)
            {
                ut->appendP(header, fb_esp_pgm_str_99);
                header += fbdo->queryFilter._startAt;
            }

            if (fbdo->queryFilter._endAt.length() > 0)
            {
                ut->appendP(header, fb_esp_pgm_str_100);
                header += fbdo->queryFilter._endAt;
            }

            if (fbdo->queryFilter._equalTo.length() > 0)
            {
                ut->appendP(header, fb_esp_pgm_str_101);
                header += fbdo->queryFilter._equalTo;
            }
        }
    }

    if (req->method == m_download)
    {
        ut->appendP(header, fb_esp_pgm_str_162);
        ut->appendP(header, fb_esp_pgm_str_28);
        std::string filename;

        for (size_t i = 0; i < fbdo->_ss.rtdb.backup_node_path.length(); i++)
        {
            if (fbdo->_ss.rtdb.backup_node_path.c_str()[i] == '/')
                ut->appendP(filename, fb_esp_pgm_str_4);
            else
                filename += fbdo->_ss.rtdb.backup_node_path.c_str()[i];
        }

        header += filename;
        std::string().swap(filename);
    }

    if (req->method == m_get && fbdo->_ss.rtdb.file_name.length() > 0)
    {
        ut->appendP(header, fb_esp_pgm_str_28);
        header += fbdo->_ss.rtdb.file_name;
    }

    if (req->async || req->method == m_get_nocontent || req->method == m_restore || req->method == m_put_nocontent || req->method == m_patch_nocontent)
        ut->appendP(header, fb_esp_pgm_str_29);

    ut->appendP(header, fb_esp_pgm_str_30);
    ut->appendP(header, fb_esp_pgm_str_31);
    header += Signer.getCfg()->database_url;
    ut->appendP(header, fb_esp_pgm_str_21);
    ut->appendP(header, fb_esp_pgm_str_32);

    if (Signer.getTokenType() == token_type_oauth2_access_token)
    {
        ut->appendP(header, fb_esp_pgm_str_237);
        header += Signer.getCfg()->signer.tokens.auth_type;
        ut->appendP(header, fb_esp_pgm_str_6);
        header += Signer.getToken(token_type_oauth2_access_token);
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    //Timestamp cannot use with ETag header, otherwise cases internal server error
    if (!sv && fbdo->queryFilter._orderBy.length() == 0 && req->data.type != d_timestamp && (req->method == m_delete || req->method == m_get || req->method == m_get_nocontent || req->method == m_put || req->method == m_put_nocontent || req->method == m_post))
        ut->appendP(header, fb_esp_pgm_str_148);

    if (fbdo->_ss.rtdb.req_etag.length() > 0 && (req->method == m_put || req->method == m_put_nocontent || req->method == m_delete))
    {
        ut->appendP(header, fb_esp_pgm_str_149);
        header += fbdo->_ss.rtdb.req_etag;
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    if (fbdo->_ss.classic_request && http_method != m_get && http_method != m_post && http_method != m_patch)
    {
        ut->appendP(header, fb_esp_pgm_str_153);

        if (http_method == m_put)
            ut->appendP(header, fb_esp_pgm_str_23);
        else if (http_method == m_delete)
            ut->appendP(header, fb_esp_pgm_str_27);

        ut->appendP(header, fb_esp_pgm_str_21);
    }

    if (req->method == m_stream)
    {
        fbdo->_ss.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        ut->appendP(header, fb_esp_pgm_str_34);
        ut->appendP(header, fb_esp_pgm_str_35);
    }
    else if (req->method == m_download || req->method == m_restore)
    {
        fbdo->_ss.rtdb.http_req_conn_type = fb_esp_http_connection_type_close;
        ut->appendP(header, fb_esp_pgm_str_34);
    }
    else
    {
        fbdo->_ss.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        ut->appendP(header, fb_esp_pgm_str_36);
        ut->appendP(header, fb_esp_pgm_str_37);
    }

    if (req->method != m_download && req->method != m_restore)
        ut->appendP(header, fb_esp_pgm_str_38);

    if (req->method == m_get_priority || req->method == m_set_priority)
        fbdo->_ss.rtdb.priority_val_flag = true;

    if (req->method == m_put || req->method == m_put_nocontent || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent || req->method == m_restore || req->method == m_set_rules || req->method == m_set_priority)
    {
        ut->appendP(header, fb_esp_pgm_str_12);
        tmp = ut->intStr(payloadLength);
        header += tmp;
        ut->delS(tmp);
    }
    ut->appendP(header, fb_esp_pgm_str_21);
    ut->appendP(header, fb_esp_pgm_str_21);
}

void FB_RTDB::preparePayload(struct fb_esp_rtdb_request_info_t *req, std::string &buf)
{
    char *tmp = nullptr;

    if (req->method != m_get && req->method != m_read_rules && req->method != m_get_shallow && req->method != m_get_priority && req->method != m_get_nocontent && req->method != m_stream &&
        req->method != m_delete && req->method != m_restore)
    {

        if (req->priority.length() > 0)
        {
            if (req->data.type == d_json)
            {
                if (req->payload.length() > 0)
                {
                    buf.clear();
                    buf = req->payload;
                    tmp = ut->strP(fb_esp_pgm_str_127);
                    size_t x = buf.find_last_of(tmp);
                    ut->delS(tmp);
                    if (x != std::string::npos && x != 0)
                        for (size_t i = 0; i < buf.length() - x; i++)
                            buf.pop_back();

                    ut->appendP(buf, fb_esp_pgm_str_157);
                    buf += req->priority;
                    ut->appendP(buf, fb_esp_pgm_str_127);
                }
            }
            else
            {
                ut->appendP(buf, fb_esp_pgm_str_161, true);
                if (req->data.type == d_string)
                    ut->appendP(buf, fb_esp_pgm_str_3);
                buf += req->payload;
                if (req->data.type == d_string)
                    ut->appendP(buf, fb_esp_pgm_str_3);
                ut->appendP(buf, fb_esp_pgm_str_157);
                buf += req->priority;
                ut->appendP(buf, fb_esp_pgm_str_127);
            }
        }
        else
        {
            buf.clear();
            if (req->data.type == d_string)
                ut->appendP(buf, fb_esp_pgm_str_3);
            buf += req->payload;
            if (req->data.type == d_string)
                ut->appendP(buf, fb_esp_pgm_str_3);
        }
    }
}

void FB_RTDB::removeStreamCallback(FirebaseData *fbdo)
{
    int index = fbdo->_ss.rtdb.Idx;

    if (index != -1)
    {
        fbdo->_dataAvailableCallback = NULL;
        fbdo->_timeoutCallback = NULL;

#if defined(ESP32)
        bool hasOherHandles = false;

        if (fbdo->_ss.rtdb.queue_task_handle)
            hasOherHandles = true;

        if (!hasOherHandles)
            fbdo->_ss.rtdb.Idx = -1;

        if (fbdo->_ss.rtdb.stream_task_handle)
            vTaskDelete(fbdo->_ss.rtdb.stream_task_handle);

        fbdo->_ss.rtdb.stream_task_handle = NULL;

        if (!hasOherHandles)
            Signer.getCfg()->_int.fb_sdo.erase(Signer.getCfg()->_int.fb_sdo.begin() + index);

#elif defined(ESP8266)
        fbdo->_ss.rtdb.Idx = -1;
        Signer.getCfg()->_int.fb_sdo.erase(Signer.getCfg()->_int.fb_sdo.begin() + index);
#endif
    }
}

void FB_RTDB::clearDataStatus(FirebaseData *fbdo)
{
    fbdo->_ss.rtdb.stream_data_changed = false;
    fbdo->_ss.rtdb.stream_path_changed = false;
    fbdo->_ss.rtdb.data_available = false;
    fbdo->_ss.rtdb.data_tmo = false;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.rtdb.resp_data_type = d_any;
    fbdo->_ss.http_code = -1000;
    fbdo->_ss.rtdb.data.clear();
    fbdo->_ss.rtdb.data2.clear();
    fbdo->_ss.rtdb.backup_node_path.clear();
    fbdo->_ss.json.clear();
    fbdo->_ss.arr.clear();
    fbdo->_ss.rtdb.blob.clear();
    fbdo->_ss.rtdb.path.clear();
    fbdo->_ss.rtdb.push_name.clear();
}

bool FB_RTDB::connectionError(FirebaseData *fbdo)
{
    return fbdo->_ss.http_code == FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_REFUSED || fbdo->_ss.http_code == FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST ||
           fbdo->_ss.http_code == FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED || fbdo->_ss.http_code == FIREBASE_ERROR_HTTPC_ERROR_SEND_HEADER_FAILED ||
           fbdo->_ss.http_code == FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED || fbdo->_ss.http_code == FIREBASE_ERROR_HTTPC_ERROR_READ_TIMEOUT;
}

bool FB_RTDB::handleStreamRequest(FirebaseData *fbdo, const std::string &path)
{

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady() || !fbdo->validRequest(path))
        return false;

    bool ret = false;
    struct fb_esp_rtdb_request_info_t _req;
    _req.method = m_stream;
    _req.data.type = d_string;
    _req.priority = "";

    if (fbdo->_ss.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        ut->getUrlInfo(fbdo->_ss.rtdb.redirect_url, uinfo);
        _req.path = uinfo.uri;
    }
    else
        _req.path = path;

    ret = sendRequest(fbdo, &_req) == 0;

    if (!ret)
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;

    return ret;
}

#endif