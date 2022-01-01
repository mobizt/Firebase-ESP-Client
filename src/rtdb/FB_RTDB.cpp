/**
 * Google's Firebase Realtime Database class, FB_RTDB.cpp version 1.2.16
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

#include "FirebaseFS.h"

#ifdef ENABLE_RTDB

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

void FB_RTDB::mSetReadTimeout(FirebaseData *fbdo, const char *millisec)
{
    if ((int)millisec <= 900000)
        fbdo->_ss.rtdb.read_tmo = (int)millisec;
}

void FB_RTDB::mSetwriteSizeLimit(FirebaseData *fbdo, const char *size)
{
    fbdo->_ss.rtdb.write_limit = size;
}

bool FB_RTDB::getRules(FirebaseData *fbdo)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path.appendP(fb_esp_pgm_str_103);
    req.method = m_read_rules;
    req.data.type = d_json;
    bool ret = handleRequest(fbdo, &req);
    FirebaseJson *json = fbdo->jsonObjectPtr();
    if (json->errorPosition() > -1)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_INVALID_JSON_RULES;
        ret = false;
    }
    json->clear();
    return ret;
}

bool FB_RTDB::mSetRules(FirebaseData *fbdo, const char *rules)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path.appendP(fb_esp_pgm_str_103);
    req.method = m_set_rules;
    req.payload = rules;
    req.data.type = d_json;
    bool ret = handleRequest(fbdo, &req);
    return ret;
}

void FB_RTDB::storeToken(MBSTRING &atok, const char *databaseSecret)
{
    atok = Signer.config->_int.auth_token;
    Signer.setTokenType(token_type_legacy_token);
    Signer.config->signer.tokens.legacy_token = databaseSecret;
    Signer.config->_int.auth_token = Signer.config->signer.tokens.legacy_token;
    Signer.config->_int.ltok_len = strlen(databaseSecret);
    Signer.config->_int.rtok_len = 0;
    Signer.config->_int.atok_len = 0;
    Signer.handleToken();
}

void FB_RTDB::restoreToken(MBSTRING &atok, fb_esp_auth_token_type tk)
{
    Signer.config->_int.auth_token = atok.c_str();
    atok.clear();
    Signer.config->signer.tokens.legacy_token = "";
    Signer.config->signer.tokens.token_type = tk;
    Signer.config->_int.atok_len = Signer.config->_int.auth_token.length();
    Signer.config->_int.ltok_len = 0;
    Signer.handleToken();
}

bool FB_RTDB::mSetQueryIndex(FirebaseData *fbdo, const char *path, const char *node, const char *databaseSecret)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    MBSTRING s;
    bool ret = false;
    MBSTRING atok;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
        storeToken(atok, databaseSecret);

    if (getRules(fbdo))
    {
        ret = true;
        FirebaseJsonData data;
        FirebaseJson *json = fbdo->jsonObjectPtr();
        if (json->errorPosition() > -1)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_INVALID_JSON_RULES;
            ret = false;
        }
        else
        {
            bool ruleExisted = false;

            s.clear();
            s.appendP(fb_esp_pgm_str_550);
            if (path[0] != '/')
                s += '/';
            s += path;
            s.appendP(fb_esp_pgm_str_551);

            json->get(data, s.c_str());

            if (data.success && strcmp(data.to<const char *>(), node) == 0)
                ruleExisted = true;

            if (strlen(node) == 0)
                json->remove(s.c_str());
            else
                json->set(s.c_str(), node);

            if (!ruleExisted || (ruleExisted && strlen(node) == 0))
            {
                MBSTRING str;
                json->toString(str, true);
                ret = setRules(fbdo, str.c_str());
            }
        }

        json->clear();
    }

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
        restoreToken(atok, tk);

    s.clear();
    return ret;
}

bool FB_RTDB::mSetReadWriteRules(FirebaseData *fbdo, const char *path, const char *var, const char *readVal, const char *writeVal, const char *databaseSecret)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    MBSTRING s;
    bool ret = false;
    MBSTRING atok;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
        storeToken(atok, databaseSecret);

    if (getRules(fbdo))
    {
        ret = true;
        FirebaseJsonData data;
        FirebaseJson &json = fbdo->jsonObject();
        bool rd = false, wr = false;

        MBSTRING s;
        s.appendP(fb_esp_pgm_str_550);
        if (path[0] != '/')
            s.appendP(fb_esp_pgm_str_1);
        s += path;
        s.appendP(fb_esp_pgm_str_1);
        s += var;

        if (strlen(readVal) > 0)
        {
            rd = true;
            MBSTRING r = s;
            r.appendP(fb_esp_pgm_str_1);
            r.appendP(fb_esp_pgm_str_552);
            json.get(data, r.c_str());
            if (data.success)
                if (strcmp(data.to<const char *>(), readVal) == 0)
                    rd = false;
        }

        if (strlen(writeVal) > 0)
        {
            wr = true;
            MBSTRING w = s;
            w.appendP(fb_esp_pgm_str_1);
            w.appendP(fb_esp_pgm_str_553);
            json.get(data, w.c_str());
            if (data.success)
                if (strcmp(data.to<const char *>(), writeVal) == 0)
                    wr = false;
        }

        //modify if the rules changed or does not exist.
        if (wr || rd)
        {
            FirebaseJson js;
            MBSTRING r, w;
            r.appendP(fb_esp_pgm_str_552);
            w.appendP(fb_esp_pgm_str_553);
            if (rd)
                js.add(r.c_str(), readVal);

            if (wr)
                js.add(w.c_str(), writeVal);

            json.set(s.c_str(), js);
            MBSTRING str;
            json.toString(str, true);
            ret = setRules(fbdo, str.c_str());
        }

        json.clear();
    }

    if (strlen(databaseSecret) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
        restoreToken(atok, tk);

    s.clear();
    return ret;
}

bool FB_RTDB::mPathExisted(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return !fbdo->_ss.rtdb.path_not_found;
    else
        return false;
}

String FB_RTDB::mGetETag(FirebaseData *fbdo, const char *path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return fbdo->_ss.rtdb.resp_etag.c_str();
    else
        return "";
}

bool FB_RTDB::mGetShallowData(FirebaseData *fbdo, const char *path)
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

bool FB_RTDB::buildRequest(FirebaseData *fbdo, fb_esp_method method, const char *path, const char *payload, fb_esp_data_type type, int subtype, int value_addr, int query_addr, int priority_addr, const char *etag, bool async, bool queue, size_t blob_size, const char *filename, fb_esp_mem_storage_type storage_type)
{
    ut->idle();

    struct fb_esp_rtdb_request_info_t req;

    MBSTRING tpath, pre, post;

    if (path[0] != '/')
        tpath += '/';

    tpath += path;

    if (method == m_set_priority || method == m_get_priority)
    {
        tpath.appendP(fb_esp_pgm_str_156);
        req.path = tpath.c_str();
    }
    else if (priority_addr > 0 && method != m_get && type != d_blob && type != d_file && type != d_file_ota)
    {
        if (type == d_json)
        {
            //set priority to source json
            FirebaseJson *json = addrTo<FirebaseJson *>(value_addr);
            float *priority = addrTo<float *>(priority_addr);
            if (json && priority)
            {
                json->set(pgm2Str(fb_esp_pgm_str_157), *priority);
            }
            req.path = tpath.c_str();
        }
        else
        {
            //construct json pre and post payloads for priority
            //pre -> {".priority":priority,"subpath":
            //pos -> }

            float *priority = addrTo<float *>(priority_addr);
            if (priority)
            {
                pre.appendP(fb_esp_pgm_str_163);
                pre.appendP(fb_esp_pgm_str_3);
                pre.appendP(fb_esp_pgm_str_157);
                pre.appendP(fb_esp_pgm_str_3);
                pre.appendP(fb_esp_pgm_str_7);
                pre += *priority;
                pre.appendP(fb_esp_pgm_str_132);
                pre.appendP(fb_esp_pgm_str_3);

                if (type == d_string || type == d_std_string || type == d_mb_string)
                    post.appendP(fb_esp_pgm_str_3);
                post.appendP(fb_esp_pgm_str_127);

                int p1 = ut->rstrpos(tpath.c_str(), '/', 0), p2 = 0;
                tpath = path;
                if (p1 > 0)
                {
                    p2 = p1 + 1;
                    req.path = tpath.substr(0, p2).c_str();
                    //subpath
                    pre += tpath.substr(p2, strlen(path) - 1 - p2).c_str();
                }
                else
                {
                    tpath.erase(0, 1);
                    //subpath
                    pre += tpath.c_str();
                    tpath.clear();
                    tpath += '/';
                    req.path = tpath.c_str();
                }

                pre.appendP(fb_esp_pgm_str_3);
                pre.appendP(fb_esp_pgm_str_7);
                if (type == d_string || type == d_std_string || type == d_mb_string)
                    pre.appendP(fb_esp_pgm_str_3);

                req.pre_payload = pre.c_str();
                req.post_payload = post.c_str();
            }
        }
    }
    else
    {
        if (type == d_string || type == d_std_string || type == d_mb_string)
        {
            pre.appendP(fb_esp_pgm_str_3);
            post.appendP(fb_esp_pgm_str_3);
            req.pre_payload = pre.c_str();
            req.post_payload = post.c_str();
        }

        req.path = tpath.c_str();
    }

    if (method == m_get && type == d_blob)
        setBlobRef(fbdo, value_addr);

    req.method = method;
    req.data.type = type;
    req.async = async;
    req.queue = queue;
    method == m_get ? req.data.address.dout = value_addr : req.data.address.din = value_addr;
    req.data.address.priority = priority_addr;
    req.data.address.query = query_addr;
    req.data.etag = etag;
    req.data.value_subtype = subtype;
    req.data.blobSize = blob_size;
    req.payload = payload;
    req.filename = filename;
    req.storageType = storage_type;

    if (type == d_file_ota)
        fbdo->closeSession();

#if defined(ESP8266)
    int rx_size = fbdo->_ss.bssl_rx_size;
    if (type == d_file_ota)
        fbdo->_ss.bssl_rx_size = 16384;
#endif

    bool ret = processRequest(fbdo, &req);

#if defined(ESP8266)
    if (type == d_file_ota)
        fbdo->_ss.bssl_rx_size = rx_size;
#endif

    if (type == d_file_ota)
        fbdo->closeSession();

    return ret;
}

bool FB_RTDB::mDeleteNodesByTimestamp(FirebaseData *fbdo, const char *path, const char *timestampNode, const char *limit, const char *dataRetentionPeriod)
{
    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    time_t current_ts = time(nullptr);

    if (current_ts < ESP_DEFAULT_TS)
        return false;

    bool ret = false;

    int _limit = atoi(limit);

    if (_limit > 30)
        _limit = 30;

    char *pEnd;
    uint32_t pr = strtoull(dataRetentionPeriod, &pEnd, 10);

    QueryFilter query;

    double lastTS = current_ts - pr;

    query.orderBy(timestampNode).startAt(0).endAt(lastTS).limitToLast(_limit);

    if (getJSON(fbdo, path, &query))
    {
        ret = true;
        if (fbdo->_ss.rtdb.resp_data_type == d_json && fbdo->jsonString().length() > 4)
        {
            FirebaseJson *js = fbdo->jsonObjectPtr();
            size_t len = js->iteratorBegin();
            FirebaseJson::IteratorValue value;
            MBSTRING nodes[len];
            for (size_t i = 0; i < len; i++)
            {
                value = js->valueAt(i);
                if (value.type == FirebaseJson::JSON_OBJECT && value.key.length() > 1)
                    nodes[i] = value.key.c_str();
            }
            js->iteratorEnd();
            js->clear();

            for (size_t i = 0; i < len; i++)
            {
                MBSTRING s = path;
                s.appendP(fb_esp_pgm_str_1);
                s += nodes[i];
                deleteNode(fbdo, s.c_str());
            }
        }
    }

    query.clear();
    return ret;
}

bool FB_RTDB::mBeginStream(FirebaseData *fbdo, const char *path)
{
    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    fbdo->_ss.rtdb.pause = false;

    fbdo->_ss.rtdb.new_stream = true;

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

bool FB_RTDB::mBeginMultiPathStream(FirebaseData *fbdo, const char *parentPath)
{
    return mBeginStream(fbdo, parentPath);
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
    if (Signer.getCfg()->timeout.rtdbStreamReconnect < MIN_RTDB_STREAM_RECONNECT_INTERVAL || Signer.getCfg()->timeout.rtdbStreamReconnect > MAX_RTDB_STREAM_RECONNECT_INTERVAL)
        Signer.getCfg()->timeout.rtdbStreamReconnect = MIN_RTDB_STREAM_RECONNECT_INTERVAL;

    if (millis() - Signer.getCfg()->timeout.rtdbStreamReconnect > fbdo->_ss.rtdb.stream_resume_millis)
    {
        reconnectStream = (fbdo->_ss.rtdb.data_tmo && !fbdo->_ss.connected) || fbdo->_ss.http_code >= 400 || fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream;

        if (fbdo->_ss.rtdb.data_tmo)
        {
            fbdo->closeSession();
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
        }
        fbdo->_ss.rtdb.stream_resume_millis = millis();
    }
    else
        ret = true;

    if (fbdo->tcpClient.stream())
    {
        if (!fbdo->tcpClient.stream()->connected())
            reconnectStream = true;
    }
    else
        reconnectStream = true;

    //Stream timed out

    if (Signer.getCfg()->timeout.rtdbKeepAlive < MIN_RTDB_KEEP_ALIVE_TIMEOUT || Signer.getCfg()->timeout.rtdbKeepAlive > MAX_RTDB_KEEP_ALIVE_TIMEOUT)
        Signer.getCfg()->timeout.rtdbKeepAlive = DEFAULT_RTDB_KEEP_ALIVE_TIMEOUT;

    if (millis() - fbdo->_ss.rtdb.data_millis > Signer.getCfg()->timeout.rtdbKeepAlive)
    {
        fbdo->_ss.rtdb.data_millis = millis();
        fbdo->_ss.rtdb.data_tmo = true;
        fbdo->closeSession();
        fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
    }

    if (reconnectStream)
    {
        fbdo->_ss.rtdb.new_stream = true;
        if (!ut->waitIdle(fbdo->_ss.http_code))
            return false;

        fbdo->closeSession();

        if (!fbdo->tokenReady())
            return false;

        MBSTRING path = fbdo->_ss.rtdb.stream_path;
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
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
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
    MBSTRING taskName;
    taskName.appendP(fb_esp_pgm_str_72, true);
    taskName.appendP(fb_esp_pgm_str_113);
    taskName += index;

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
    MBSTRING taskName;
    taskName.appendP(fb_esp_pgm_str_72, true);
    taskName.appendP(fb_esp_pgm_str_113);
    taskName += index;

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
    MBSTRING name = taskName;

    TaskFunction_t taskCode = [](void *param)
    {
        const TickType_t xDelay = Signer.getCfg()->_int.fb_sdo[id].get()._ss.rtdb.stream_task_delay_ms / portTICK_PERIOD_MS;
        while (Signer.getCfg()->_int.fb_sdo[id].get()._ss.rtdb.stream_task_enable)
        {
            if ((Signer.getCfg()->_int.fb_sdo[id].get()._dataAvailableCallback || Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback))
            {

                _this->readStream(&Signer.getCfg()->_int.fb_sdo[id].get());

                if (Signer.getCfg()->_int.fb_sdo[id].get().streamTimeout() && Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback)
                    Signer.getCfg()->_int.fb_sdo[id].get()._timeoutCallback(true);
            }
            vTaskDelay(xDelay);
        }

        Signer.getCfg()->_int.fb_sdo[id].get()._ss.rtdb.stream_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, name.c_str(), fbdo->_ss.rtdb.stream_task_stack_size, NULL, fbdo->_ss.rtdb.stream_task_priority, &fbdo->_ss.rtdb.stream_task_handle, fbdo->_ss.rtdb.stream_task_cpu_core);

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
    ut->idle();

    if (!fbdo->reconnect())
        return;

    if (fbdo->_qMan.size() > 0)
    {

        for (uint8_t i = 0; i < fbdo->_qMan.size(); i++)
        {
            if (!fbdo->_qMan._queueCollection)
                return;

            QueueItem item = fbdo->_qMan._queueCollection->at(i);

            if (item.qID == 0)
            {
                fbdo->clearQueueItem(&item);
                fbdo->_qMan.remove(i);
                continue;
            }

            if (callback)
            {
                QueueInfo qinfo;
                qinfo._isQueue = true;
                qinfo._dataType = fbdo->getDataType(item.dataType);
                qinfo._path = item.path;
                qinfo._currentQueueID = item.qID;
                qinfo._method = fbdo->getMethod(item.method);
                qinfo._totalQueue = fbdo->_qMan.size();
                qinfo._isQueueFull = fbdo->_qMan.size() == fbdo->_qMan._maxQueue;
                callback(qinfo);
            }

            ut->idle();
            if (buildRequest(fbdo, item.method, item.path.c_str(), item.payload.c_str(), item.dataType, item.subType, item.method == m_get ? item.address.dout : item.address.din, item.address.query, item.address.priority, item.etag.c_str(), item.async, _NO_QUEUE, item.blobSize, item.filename.c_str(), item.storageType))
            {
                fbdo->clearQueueItem(&item);
                fbdo->_qMan.remove(i);
            }
        }
    }
}

void FB_RTDB::setBlobRef(FirebaseData *fbdo, int addr)
{
    if (fbdo->_ss.rtdb.blob && fbdo->_ss.rtdb.isBlobPtr)
        delete fbdo->_ss.rtdb.blob;

    fbdo->_ss.rtdb.isBlobPtr = addr == 0;
    addr > 0 ? fbdo->_ss.rtdb.blob = addrTo<std::vector<uint8_t> *>(addr) : fbdo->_ss.rtdb.blob = new std::vector<uint8_t>();
}

bool FB_RTDB::isErrorQueueExisted(FirebaseData *fbdo, uint32_t errorQueueID)
{

    for (uint8_t i = 0; i < fbdo->_qMan.size(); i++)
    {
        if (!fbdo->_qMan._queueCollection)
            return false;

        QueueItem q = fbdo->_qMan._queueCollection->at(i);
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

    MBSTRING taskName;
    taskName.appendP(fb_esp_pgm_str_72);
    taskName.appendP(fb_esp_pgm_str_114);
    taskName += index;

    if (queueTaskStackSize > QUEUE_TASK_STACK_SIZE)
        fbdo->_ss.rtdb.queue_task_stack_size = queueTaskStackSize;
    else
        fbdo->_ss.rtdb.queue_task_stack_size = QUEUE_TASK_STACK_SIZE;

    static FB_RTDB *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
        const TickType_t xDelay = Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_delay_ms / portTICK_PERIOD_MS;
        for (;;)
        {
            if (Signer.getCfg()->_int.fb_sdo[index].get()._queueInfoCallback)
                _this->processErrorQueue(&Signer.getCfg()->_int.fb_sdo[index].get(), Signer.getCfg()->_int.fb_sdo[index].get()._queueInfoCallback);
            else
                _this->processErrorQueue(&Signer.getCfg()->_int.fb_sdo[index].get(), NULL);

            vTaskDelay(xDelay);
        }

        Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName.c_str(), Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_stack_size, NULL, fbdo->_ss.rtdb.queue_task_priority, &Signer.getCfg()->_int.fb_sdo[index].get()._ss.rtdb.queue_task_handle, fbdo->_ss.rtdb.queue_task_cpu_core);

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
    for (uint8_t i = 0; i < fbdo->_qMan.size(); i++)
    {
        if (!fbdo->_qMan._queueCollection)
            return;

        QueueItem item = fbdo->_qMan._queueCollection->at(i);
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

    if (fbdo->_qMan.size() > num)
    {
        for (uint8_t i = fbdo->_qMan.size() - 1; i >= num; i--)
        {
            if (!fbdo->_qMan._queueCollection)
                return;

            QueueItem item = fbdo->_qMan._queueCollection->at(i);
            fbdo->clearQueueItem(&item);
        }
    }
}

bool FB_RTDB::mSaveErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType)
{
    if (storageType == mem_storage_type_sd)
    {
#if defined SD_FS
        if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            return false;

        if (SD_FS.exists(filename))
            SD_FS.remove(filename);

        Signer.getCfg()->_int.fb_file = SD_FS.open(filename, FILE_WRITE);
#endif
    }
    else if (storageType == mem_storage_type_flash)
    {
#if defined FLASH_FS
        if (!Signer.getCfg()->_int.fb_flash_rdy)
            ut->flashTest();

        if (FLASH_FS.exists(filename))
            FLASH_FS.remove(filename);

        Signer.getCfg()->_int.fb_file = FLASH_FS.open(filename, (const char *)FPSTR("w"));
#endif
    }

    if ((storageType == mem_storage_type_flash || storageType == mem_storage_type_sd) && !Signer.getCfg()->_int.fb_file)
        return false;

    File file = Signer.getCfg()->_int.fb_file;
    FirebaseJsonArray arr;

    for (uint8_t i = 0; i < fbdo->_qMan.size(); i++)
    {
        if (!fbdo->_qMan._queueCollection)
            continue;

        QueueItem item = fbdo->_qMan._queueCollection->at(i);
        arr.clear();
        arr.add((uint8_t)item.dataType, (uint8_t)item.subType, (uint8_t)item.method, (uint8_t)item.storageType, (uint8_t)item.async);
        arr.add(item.address.din, item.address.dout, item.address.query, item.address.priority, item.blobSize);
        arr.add(item.path, item.payload, item.etag, item.filename);
        file.write((uint8_t *)arr.raw(), strlen(arr.raw()));
    }

    file.close();
    return true;
}

bool FB_RTDB::mRestoreErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType)
{
    return openErrorQueue(fbdo, filename, storageType, 1) != 0;
}

uint8_t FB_RTDB::mErrorQueueCount(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType)
{
    return openErrorQueue(fbdo, filename, storageType, 0);
}

bool FB_RTDB::mDeleteStorageFile(const char *filename, fb_esp_mem_storage_type storageType)
{

    if (storageType == mem_storage_type_sd)
    {
#if defined SD_FS
        if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            return false;
        return SD_FS.remove(filename);
#else
        return false;
#endif
    }
    else
    {
#if defined FLASH_FS
        if (!Signer.getCfg()->_int.fb_flash_rdy)
            ut->flashTest();
        return FLASH_FS.remove(filename);
#else
        return false;
#endif
    }
}

uint8_t FB_RTDB::openErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType, uint8_t mode)
{

    uint8_t count = 0;

    if (storageType == mem_storage_type_sd)
    {
#if defined SD_FS
        if (!ut->sdTest(Signer.getCfg()->_int.fb_file))
            return 0;
        Signer.getCfg()->_int.fb_file = SD_FS.open(filename, FILE_READ);
#else
        return 0;
#endif
    }
    else if (storageType == mem_storage_type_flash)
    {
#if defined FLASH_FS
        if (!Signer.getCfg()->_int.fb_flash_rdy)
            ut->flashTest();
        Signer.getCfg()->_int.fb_file = FLASH_FS.open(filename, (const char *)FPSTR("r"));
#endif
    }

    if ((storageType == mem_storage_type_flash || storageType == mem_storage_type_sd) && !Signer.getCfg()->_int.fb_file)
        return 0;

    if (!Signer.getCfg()->_int.fb_file.available() || Signer.getCfg()->_int.fb_file.size() < 4)
    {
        Signer.getCfg()->_int.fb_file.close();
        return 0;
    }

    QueueItem item;

    File file = Signer.getCfg()->_int.fb_file;

    FirebaseJsonArray arr;
    FirebaseJsonData result;

    while (file.available())
    {
        ut->idle();
        if (arr.readFrom(file))
        {
            if (mode == 1)
            {
                for (size_t i = 0; i < arr.size(); i++)
                {
                    ut->idle();
                    arr.get(result, i);
                    if (result.success)
                    {
                        switch (i)
                        {
                        case 0:
                            item.dataType = (fb_esp_data_type)result.to<int>();
                            break;
                        case 1:
                            item.subType = result.to<int>();
                            break;
                        case 2:
                            item.method = (fb_esp_method)result.to<int>();
                            break;
                        case 3:
                            item.storageType = (fb_esp_mem_storage_type)result.to<int>();
                            break;
                        case 4:
                            item.async = (bool)result.to<int>();
                            break;
                        case 5:
                            item.address.din = result.to<int>();
                            break;
                        case 6:
                            item.address.dout = result.to<int>();
                            break;
                        case 7:
                            item.address.query = result.to<int>();
                            break;
                        case 8:
                            item.address.priority = result.to<int>();
                            break;
                        case 9:
                            item.blobSize = result.to<int>();
                            break;
                        case 10:
                            item.path = result.to<MBSTRING>();
                            break;
                        case 11:
                            item.payload = result.to<MBSTRING>();
                            break;
                        case 12:
                            item.etag = result.to<MBSTRING>();
                            break;
                        case 13:
                            item.filename = result.to<MBSTRING>();
                            break;
                        default:
                            break;
                        }
                    }
                }
                if (!fbdo->_qMan._queueCollection)
                    fbdo->_qMan._queueCollection = new std::vector<struct QueueItem>();

                fbdo->_qMan._queueCollection->push_back(item);
            }
            count++;
        }
    }
    return count;
}

bool FB_RTDB::isErrorQueueFull(FirebaseData *fbdo)
{
    if (fbdo->_qMan._maxQueue > 0)
        return fbdo->_qMan.size() >= fbdo->_qMan._maxQueue;
    return false;
}

uint8_t FB_RTDB::errorQueueCount(FirebaseData *fbdo)
{
    return fbdo->_qMan.size();
}

bool FB_RTDB::mBackup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName)
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

bool FB_RTDB::mRestore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName)
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

void FB_RTDB::setRefValue(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (req->data.address.dout > 0 && req->method == m_get)
    {

        if (req->data.type != d_file && req->data.type != d_file_ota)
        {
            if (req->data.type == d_boolean)
            {
                bool *ptr = addrTo<bool *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<bool>();
            }
            else if (req->data.type == d_integer)
            {
                int *ptr = addrTo<int *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<int>();
            }
            else if (req->data.type == d_float)
            {
                float *ptr = addrTo<float *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<float>();
            }
            else if (req->data.type == d_double)
            {
                double *ptr = addrTo<double *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<double>();
            }
            else if (req->data.type == d_string)
            {
                String *ptr = addrTo<String *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<String>();
            }
            else if (req->data.type == d_std_string)
            {
                std::string *ptr = addrTo<std::string *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<std::string>();
            }
            else if (req->data.type == d_mb_string)
            {
                MBSTRING *ptr = addrTo<MBSTRING *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<MBSTRING>();
            }
            else if (req->data.type == d_json)
            {
                FirebaseJson *ptr = addrTo<FirebaseJson *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<FirebaseJson>();
            }
            else if (req->data.type == d_array)
            {
                FirebaseJsonArray *ptr = addrTo<FirebaseJsonArray *>(req->data.address.dout);
                if (ptr)
                    *ptr = fbdo->to<FirebaseJsonArray>();
            }
        }
    }
}

void FB_RTDB::addQueueData(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (req->method == m_get || (!req->queue && (req->method == m_put || req->method == m_put_nocontent || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent)))
    {

        struct fb_esp_rtdb_queue_info_t qinfo;

        qinfo.method = req->method;
        qinfo.storageType = req->storageType;
        qinfo.dataType = req->data.type;
        qinfo.subType = req->data.value_subtype;
        qinfo.path = req->path;
        qinfo.address = req->data.address;
        qinfo.filename = req->filename;
        qinfo.payload = req->payload;
        qinfo.etag = req->data.etag;
        qinfo.async = req->async;
        qinfo.blobSize = req->data.blobSize;

        fbdo->addQueue(&qinfo);
    }
}

bool FB_RTDB::processRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    ut->idle();
    if (!Signer.getCfg())
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    if (req->method != m_get)
    {
        if (!fbdo->reconnect())
        {
            addQueueData(fbdo, req);
            return false;
        }
    }

    if (!fbdo->tokenReady())
        return false;

    bool ret = false;

    if (req->data.type == d_file)
    {
        fbdo->_ss.rtdb.file_name = req->filename;

        if (req->method == m_get)
        {
            if (req->storageType == mem_storage_type_flash && !Signer.getCfg()->_int.fb_flash_rdy)
                ut->flashTest();
            else if (req->storageType == mem_storage_type_sd && !Signer.getCfg()->_int.fb_sd_rdy)
                ut->sdTest(Signer.getCfg()->_int.fb_file);

            if (Signer.getCfg()->_int.fb_flash_rdy)
            {
#if defined FLASH_FS
                Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.file_name.c_str(), (const char *)FPSTR("w"));
#endif
            }
            else if (Signer.getCfg()->_int.fb_sd_rdy)
            {
#if defined SD_FS
                Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.file_name.c_str(), FILE_WRITE);
#endif
            }
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

        setRefValue(fbdo, req);

        if (ret)
            break;

        if (fbdo->_ss.rtdb.max_retry > 0)
            if (!ret && connectionError(fbdo))
                errCount++;
    }

    if (!ret && errCount == maxRetry && fbdo->_qMan._maxQueue > 0)
    {
        addQueueData(fbdo, req);
        if (!req->queue && req->method != m_get)
            return ret;
    }

    if (ret)
    {
        if (Signer.getCfg()->rtdb.data_type_stricted && req->method == m_get && req->data.type != d_any)
        {
            if (req->data.type == d_integer || req->data.type == d_float || req->data.type == d_double)
                ret = fbdo->_ss.rtdb.resp_data_type == d_integer || fbdo->_ss.rtdb.resp_data_type == d_float || fbdo->_ss.rtdb.resp_data_type == d_double;
            else if (req->data.type == d_json)
                ret = fbdo->_ss.rtdb.resp_data_type == d_json || fbdo->_ss.rtdb.resp_data_type == d_null;
            else if (req->data.type == d_array)
                ret = fbdo->_ss.rtdb.resp_data_type == d_array || fbdo->_ss.rtdb.resp_data_type == d_null;
            else if (req->data.type != d_file && req->data.type != d_file_ota)
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
        if (strcmp(req->path.c_str(), fbdo->_ss.rtdb.stream_path.c_str()) != 0)
            fbdo->_ss.rtdb.stream_path_changed = true;
        else
            fbdo->_ss.rtdb.stream_path_changed = false;
    }

    if (fbdo->_ss.cert_updated || !fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.rtdb.stream_path_changed || (req->method == m_stream && fbdo->_ss.con_mode != fb_esp_con_mode_rtdb_stream) || (req->method != m_stream && fbdo->_ss.con_mode == fb_esp_con_mode_rtdb_stream) || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
        fbdo->ethDNSWorkAround(&ut->config->spi_ethernet_module, host, 443);
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
    ut->idle();
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

    if ((req->method == m_put || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent || req->method == m_set_rules) && req->payload.length() == 0 && req->data.type != d_string && req->data.type != d_std_string && req->data.type != d_mb_string && req->data.type != d_json && req->data.type != d_array && req->data.type != d_blob && req->data.type != d_file_ota)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_MISSING_DATA;
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
    if (req->data.etag.length() > 0)
        fbdo->_ss.rtdb.req_etag = req->data.etag;
    if (req->data.address.priority > 0)
    {
        float *pri = addrTo<float *>(req->data.address.priority);
        fbdo->_ss.rtdb.priority = *pri;
    }

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
        else if (req->method == m_download || ((req->data.type == d_file || req->data.type == d_file_ota) && req->method == m_get))
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
            fbdo->_ss.rtdb.data_available = fbdo->_ss.rtdb.raw.length() > 0;
            if (fbdo->_ss.rtdb.blob)
                fbdo->_ss.rtdb.data_available |= fbdo->_ss.rtdb.blob->size() > 0;
        }
    }
    else
    {
        fbdo->_ss.http_code = ret;
        if (ret != FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED)
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
        return FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

    if (!fbdo->tokenReady())
        return FIREBASE_ERROR_TOKEN_NOT_READY;

    if (!fbdo->validRequest(req->path))
        return FIREBASE_ERROR_MISSING_CREDENTIALS;

    if (fbdo->_ss.long_running_task > 0)
        return FIREBASE_ERROR_LONG_RUNNING_TASK;

    size_t buffSize = Signer.getCfg()->rtdb.upload_buffer_size;

    if (buffSize < 128)
        buffSize = 128;
#if defined(ESP32)
    if (buffSize > 1024 * 50)
        buffSize = 1024 * 50;
#elif defined(ESP8266)
    if (buffSize > 16384)
        buffSize = 16384;
#endif

    buffSize = ut->getReservedLen(buffSize);

    char *buf = nullptr;

    int len = 0;
    size_t toRead = 0;
    int ret = -1;

    ut->idle();

    rescon(fbdo, Signer.getCfg()->database_url.c_str(), req);

    if (req->method == m_stream)
    {
        fbdo->_ss.rtdb.stream_path.clear();

        if (req->path.length() > 0)
            if (req->path[0] != '/')
                fbdo->_ss.rtdb.stream_path.appendP(fb_esp_pgm_str_1, true);

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
                    fbdo->_ss.rtdb.path.appendP(fb_esp_pgm_str_1, true);
            fbdo->_ss.rtdb.path += req->path;
        }

        fbdo->_ss.rtdb.data_tmo = false;
    }

    fbdo->_ss.max_payload_length = 0;

    fbdo->tcpClient.begin(Signer.getCfg()->database_url.c_str(), FIREBASE_PORT);

    //Prepare request header
    if (req->method != m_download && req->method != m_restore && req->data.type != d_file && req->data.type != d_file_ota)
        ret = sendHeader(fbdo, req);
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
                fbdo->_ss.error.appendP(fb_esp_pgm_str_164, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }
        }
        else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
        {
            if (Signer.getCfg()->_int.fb_sd_used)
            {
                fbdo->_ss.error.appendP(fb_esp_pgm_str_84, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }

            if (!Signer.getCfg()->_int.fb_sd_rdy)
                Signer.getCfg()->_int.fb_sd_rdy = ut->sdTest(Signer.getCfg()->_int.fb_file);

            if (!Signer.getCfg()->_int.fb_sd_rdy)
            {
                fbdo->_ss.error.appendP(fb_esp_pgm_str_85, true);
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
#if defined FLASH_FS
                    FLASH_FS.remove(fbdo->_ss.rtdb.backup_filename.c_str());
                    Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), (const char *)FPSTR("w"));
#endif
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {
#if defined SD_FS
                    SD_FS.remove(fbdo->_ss.rtdb.backup_filename.c_str());
                    Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), FILE_WRITE);
#endif
                }
            }
            else if (req->method == m_restore)
            {

                if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                {
#if defined FLASH_FS
                    if (FLASH_FS.exists(fbdo->_ss.rtdb.backup_filename.c_str()))
                        Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), (const char *)FPSTR("r"));
#endif
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {
#if defined SD_FS
                    if (SD_FS.exists(fbdo->_ss.rtdb.backup_filename.c_str()))
                        Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.backup_filename.c_str(), FILE_READ);
#endif
                }
                else
                {
                    fbdo->_ss.error.appendP(fb_esp_pgm_str_83, true);
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
#if defined FLASH_FS
                    if (FLASH_FS.exists(fbdo->_ss.rtdb.file_name.c_str()))
                        Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.file_name.c_str(), (const char *)FPSTR("r"));
                    else
                    {
                        fbdo->_ss.error.appendP(fb_esp_pgm_str_83, true);
                        return FIREBASE_ERROR_FILE_IO_ERROR;
                    }
#else
                    return FIREBASE_ERROR_FILE_IO_ERROR;
#endif
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {
#if defined SD_FS
                    if (SD_FS.exists(fbdo->_ss.rtdb.file_name.c_str()))
                        Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.file_name.c_str(), FILE_READ);
                    else
                    {
                        fbdo->_ss.error.appendP(fb_esp_pgm_str_83, true);
                        return FIREBASE_ERROR_FILE_IO_ERROR;
                    }
#else
                    return FIREBASE_ERROR_FILE_IO_ERROR;
#endif
                }
                len = (4 * ceil(Signer.getCfg()->_int.fb_file.size() / 3.0)) + strlen_P(fb_esp_pgm_str_93) + 1;
            }
            else if (req->method == m_get)
            {
                size_t p1 = fbdo->_ss.rtdb.file_name.find_last_of(pgm2Str(fb_esp_pgm_str_1));
                MBSTRING folder = (const char *)FPSTR("/");

                if (p1 != MBSTRING::npos && p1 != 0)
                    folder = fbdo->_ss.rtdb.file_name.substr(p1 - 1);

                if (fbdo->_ss.rtdb.storage_type == mem_storage_type_flash)
                {
#if defined FLASH_FS
                    Signer.getCfg()->_int.fb_file = FLASH_FS.open(fbdo->_ss.rtdb.file_name.c_str(), (const char *)FPSTR("w"));
#endif
                }
                else if (fbdo->_ss.rtdb.storage_type == mem_storage_type_sd)
                {
#if defined SD_FS
                    if (!SD_FS.exists(folder.c_str()))
                        ut->createDirs(folder, (fb_esp_mem_storage_type)fbdo->_ss.rtdb.storage_type);

                    SD_FS.remove(fbdo->_ss.rtdb.file_name.c_str());

                    Signer.getCfg()->_int.fb_file = SD_FS.open(fbdo->_ss.rtdb.file_name.c_str(), FILE_WRITE);
#endif
                }
                folder.clear();
            }

            fileChecked = true;
        }

        if (fileChecked)
        {
            if (!Signer.getCfg()->_int.fb_file)
            {
                fbdo->_ss.error.appendP(fb_esp_pgm_str_86, true);
                return FIREBASE_ERROR_FILE_IO_ERROR;
            }
        }

        req->fileLen = len;

        if (req->data.type != d_file && req->data.type != d_file_ota)
            req->path = fbdo->_ss.rtdb.backup_node_path.c_str();
        ret = sendHeader(fbdo, req);
    }

    if (req->method == m_get_nocontent || req->method == m_patch_nocontent || (req->method == m_put_nocontent && req->data.type == d_blob))
        fbdo->_ss.rtdb.no_content_req = true;

    if (req->data.type == d_blob)
    {
        if (fbdo->_ss.rtdb.blob)
            std::vector<uint8_t>().swap(*fbdo->_ss.rtdb.blob);
        else
        {
            fbdo->_ss.rtdb.isBlobPtr = true;
            fbdo->_ss.rtdb.blob = new std::vector<uint8_t>();
        }
    }

    if (!fbdo->reconnect() || ret != 0)
        return FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

    int bufSize = Signer.getCfg()->rtdb.upload_buffer_size;
    if (bufSize < 256)
        bufSize = 256;
#if defined(ESP32)
    if (bufSize > 1024 * 50)
        bufSize = 1024 * 50;
#elif defined(ESP8266)
    if (bufSize > 16384)
        bufSize = 16384;
#endif

    //Send payload
    if (req->data.address.din > 0 && req->data.type == d_json)
    {
        FirebaseJson *json = addrTo<FirebaseJson *>(req->data.address.din);
        if (json)
            ret = fbdo->tcpSend(json->raw());
    }
    else if (req->payload.length() > 0 || (req->data.type == d_array && req->data.address.din > 0))
    {
        if (req->pre_payload.length() > 0)
        {
            ret = fbdo->tcpSend(req->pre_payload.c_str());
            if (ret != 0)
                return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;
        }

        if (req->data.type == d_array && req->data.address.din > 0)
        {
            FirebaseJsonArray *arr = addrTo<FirebaseJsonArray *>(req->data.address.din);
            if (arr)
                ret = fbdo->tcpSend(arr->raw());
            if (ret != 0)
                return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;
        }
        else if (req->payload.length() > 0)
        {
            ret = fbdo->tcpSend(req->payload.c_str());
            if (ret != 0)
                return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;
        }

        if (req->post_payload.length() > 0)
            ret = fbdo->tcpSend(req->post_payload.c_str());
    }
    else if (req->data.address.din > 0 && req->data.blobSize > 0)
    {
        //input blob data is uint8_t array
        uint8_t *blob = addrTo<uint8_t *>(req->data.address.din);
        if (blob)
        {
            MBSTRING s;
            s.appendP(fb_esp_pgm_str_92, true);
            ret = fbdo->tcpSend(s.c_str());
            if (ret == 0)
            {
                if (ut->sendBase64(bufSize, blob, req->data.blobSize, true, &fbdo->tcpClient))
                {
                    s.appendP(fb_esp_pgm_str_3, true);
                    ret = fbdo->tcpSend(s.c_str());
                }
            }
        }
    }
    else if (req->method == m_restore || (req->data.type == d_file && (req->method == m_put_nocontent || req->method == m_post)))
    {

        if (req->data.type == d_file && (req->method == m_put_nocontent || req->method == m_post))
        {
           

            ret = fbdo->tcpSend(pgm2Str(fb_esp_pgm_str_93));
            if (ret == 0)
                ut->sendBase64File(bufSize, fbdo->tcpClient.stream(), fbdo->_ss.rtdb.file_name, fbdo->_ss.rtdb.storage_type, Signer.getCfg()->_int.fb_file);
            else
                return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;

            buf = (char *)ut->newP(2);
            buf[0] = '"';
            buf[1] = '\0';
            ret = fbdo->tcpSend(buf);
            ut->delP(&buf);

            if (ret != 0)
                return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;
        }
        else
        {
            while (len)
            {
                if (!fbdo->reconnect())
                    return FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

                toRead = len;
                if (toRead > buffSize)
                    toRead = buffSize - 1;

                buf = (char *)ut->newP(buffSize);
                Signer.getCfg()->_int.fb_file.read((uint8_t *)buf, toRead);

                buf[toRead] = '\0';

                ret = fbdo->tcpSend(buf);
                ut->delP(&buf);

                if (ret != 0)
                    return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;

                len -= toRead;

                if (len <= 0)
                    break;
            }
        }

        ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
    }

    if (ret != 0)
        return FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED;

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

void FB_RTDB::waitRxReady(FirebaseData *fbdo, unsigned long &dataTime)
{
    if (fbdo->tcpClient.stream()->available() == 0)
        dataTime = millis();

    while (fbdo->tcpClient.stream()->available() == 0 && fbdo->reconnect(dataTime))
    {
        ut->idle();
    }
}

bool FB_RTDB::handleResponse(FirebaseData *fbdo)
{
    ut->idle();

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect())
        return false;

    WiFiClient *stream = fbdo->tcpClient.stream();

    if (!fbdo->_ss.connected || stream == nullptr)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

        if (fbdo->_ss.con_mode == fb_esp_con_mode_rtdb_stream)
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);

        return false;
    }

    unsigned long dataTime = millis();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    MBSTRING header;
    MBSTRING payload;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    //int payloadLen = fbdo->_ss.resp_size;
    int chunkBufSize = stream->available();
    bool redirect = false;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->_ss.resp_size;

    int base64PadLen = 0;

    struct fb_esp_auth_token_error_t error;
    error.code = -1;

    if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_UNDEFINED)
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.payload_length = 0;

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
            if (!fbdo->tcpClient.connected() || stream == nullptr)
            {
                fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;
                fbdo->_ss.connected = false;
            }
            stream->flush();
            return fbdo->_ss.connected;
        }
        else
        {
            while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
            {
                if (!fbdo->reconnect(dataTime) || stream == nullptr || !fbdo->tcpClient.connected())
                {
                    fbdo->_ss.error.clear();
                    fbdo->_ss.http_code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
                    return false;
                }
                chunkBufSize = stream->available();
                ut->idle();
            }
        }
    }

    dataTime = millis();

    while (chunkBufSize > 0)
    {
        ut->idle();

        if (!fbdo->reconnect(dataTime) || stream == nullptr)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

            if (fbdo->_ss.con_mode == fb_esp_con_mode_rtdb_stream)
                fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);

            return false;
        }

        chunkBufSize = stream->available();

        if (response.contentLen > 0 && fbdo->_ss.payload_length < (size_t)response.contentLen)
        {
            waitRxReady(fbdo, dataTime);
            chunkBufSize = stream->available();
        }

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
                ut->idle();
                dataTime = millis();
                if (tmp)
                {
                    //http response header with http response code
                    isHeader = true;
                    response.httpCode = atoi(tmp);
                    fbdo->_ss.http_code = response.httpCode;
                    ut->delP(&tmp);
                }
                else
                {
                    //stream payload data
                    payload = header;
                    fbdo->_ss.payload_length = header.length();
                    if (fbdo->_ss.max_payload_length < fbdo->_ss.payload_length)
                        fbdo->_ss.max_payload_length = fbdo->_ss.payload_length;
                }
            }
            else
            {
                ut->idle();
                dataTime = millis();
                //the next chunk data can be the remaining http header
                if (isHeader)
                {
                    //read one line of next header field until the empty header has found
                    tmp = (char *)ut->newP(chunkBufSize + 10);
                    bool headerEnded = false;
                    int readLen = 0;
                    if (tmp)
                    {
                        if (!stream)
                        {
                            ut->delP(&tmp);
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

                        if (ut->strposP(response.contentType.c_str(), fb_esp_pgm_str_9, 0) > -1)
                        {
                            chunkBufSize = stream->available();

                            if (chunkBufSize == 0)
                            {
                                if (tmp)
                                    ut->delP(&tmp);
                                header.clear();
                                while (chunkBufSize == 0)
                                {
                                    ut->idle();
                                    if (!fbdo->reconnect(dataTime) || stream == nullptr)
                                    {
                                        fbdo->closeSession();
                                        fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT);
                                        break;
                                    }
                                    chunkBufSize = stream->available();
                                }
                                fbdo->_ss.rtdb.new_stream = false;
                                fbdo->_ss.http_code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
                                return false;
                            }
                        }

                        header.clear();

                        if (response.httpCode == 401)
                            Signer.authenticated = false;
                        else if (response.httpCode < 300)
                            Signer.authenticated = true;

                        //error in request or server
                        if (response.httpCode >= 400)
                        {
                            //non-JSON response error handling
                            if (ut->strposP(response.contentType.c_str(), fb_esp_pgm_str_74, 0) < 0)
                            {
                                if (stream)
                                {
                                    fbdo->_ss.error.clear();
                                    while (stream->available())
                                    {
                                        ut->idle();
                                        stream->read();
                                    }
                                    if (tmp)
                                        ut->delP(&tmp);
                                    return false;
                                }
                                else
                                {
                                    if (tmp)
                                        ut->delP(&tmp);
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
                        ut->delP(&tmp);
                }
                else
                {

                    //the next chuunk data is the payload
                    if (!response.noContent && !fbdo->_ss.buffer_ovf)
                    {
                        pChunkIdx++;

                        pChunk = (char *)ut->newP(chunkBufSize + 10);

                        if (!pChunk)
                            break;

                        //read the avilable data
                        int readLen = 0;

                        if (!stream)
                        {
                            ut->delP(&pChunk);
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

                            if (readLen < defaultChunkSize)
                            {
                                if (response.contentLen > 0 && fbdo->_ss.payload_length + readLen < (size_t)response.contentLen)
                                {
                                    fbdo->checkOvf(payload.length() + readLen, response);

                                    if (!fbdo->_ss.buffer_ovf)
                                        payload += pChunk;

                                    if (pChunk)
                                        ut->delP(&pChunk);

                                    waitRxReady(fbdo, dataTime);

                                    chunkBufSize = defaultChunkSize - readLen;

                                    pChunk = (char *)ut->newP(chunkBufSize + 10);

                                    if (!pChunk)
                                    {
                                        fbdo->closeSession();
                                        break;
                                    }

                                    readLen = readLen + stream->readBytes(pChunk, chunkBufSize);
                                }
                            }
                        }

                        if (readLen > 0)
                        {
                            fbdo->_ss.payload_length += readLen;
                            if (fbdo->_ss.max_payload_length < fbdo->_ss.payload_length)
                                fbdo->_ss.max_payload_length = fbdo->_ss.payload_length;

                            fbdo->checkOvf(payload.length() + readLen, response);

                            if (!fbdo->_ss.buffer_ovf)
                                payload += pChunk;
                        }

                        if (!fbdo->_ss.rtdb.data_tmo && !fbdo->_ss.buffer_ovf)
                        {
                            //try to parse the payload
                            if (response.dataType == 0 && !response.isEvent && !response.noContent)
                            {
                                bool getOfs = fbdo->_ss.rtdb.req_data_type == d_blob || fbdo->_ss.rtdb.req_method == m_download || ((fbdo->_ss.rtdb.req_data_type == d_file || fbdo->_ss.rtdb.req_data_type == d_file_ota || fbdo->_ss.rtdb.req_data_type == d_any) && fbdo->_ss.rtdb.req_method == m_get);
                                ut->parseRespPayload(payload.c_str(), response, getOfs);

                                fbdo->_ss.error = response.fbError;
                                if (fbdo->_ss.rtdb.req_method == m_download || fbdo->_ss.rtdb.req_method == m_restore)
                                    fbdo->_ss.error = response.fbError;

                                if (fbdo->_ss.rtdb.req_method == m_download && response.dataType != d_json)
                                {
                                    fbdo->_ss.http_code = FIREBASE_ERROR_EXPECTED_JSON_DATA;

                                    fbdo->_ss.error.appendP(fb_esp_pgm_str_185, true);

                                    header.clear();
                                    payload.clear();
                                    ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
                                    fbdo->closeSession();
                                    return false;
                                }
                            }

                            //in case of the payload data type is file, decode and write stream to temp file
                            if (response.dataType == d_file || (fbdo->_ss.rtdb.req_method == m_download || ((fbdo->_ss.rtdb.req_data_type == d_file || fbdo->_ss.rtdb.req_data_type == d_file_ota) && fbdo->_ss.rtdb.req_method == m_get)))
                            {
                                fbdo->_ss.rtdb.resp_data_type = response.dataType;
                                fbdo->_ss.content_length = response.payloadLen;

                                int ofs = 0;
                                int len = readLen;

                                if (fbdo->_ss.rtdb.req_method == m_download || ((fbdo->_ss.rtdb.req_data_type == d_file || fbdo->_ss.rtdb.req_data_type == d_file_ota) && fbdo->_ss.rtdb.req_method == m_get))
                                {

                                    if (ut->stringCompare(fbdo->_ss.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151))
                                    {
                                        fbdo->_ss.http_code = FIREBASE_ERROR_PATH_NOT_EXIST;
                                        fbdo->_ss.rtdb.path_not_found = true;
                                    }

                                    if (!fbdo->_ss.rtdb.path_not_found)
                                    {
                                        if (fbdo->_ss.rtdb.file_name.length() == 0 && fbdo->_ss.rtdb.req_data_type != d_file_ota)
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
                                                len = payload.size() - response.payloadOfs;
                                                ofs = response.payloadOfs;

                                                if (fbdo->_ss.rtdb.req_data_type == d_file_ota)
                                                {
                                                    //size may include pad which we don't know from the first chunk until the last chunk
                                                    int decodedSize = (3 * (response.contentLen - response.payloadOfs - 1) / 4);

#if defined(ENABLE_OTA_FIRMWARE_UPDATE)
#if defined(ESP32)
                                                    error.code = 0;
                                                    Update.begin(decodedSize);
#elif defined(ESP8266)
                                                    error.code = ut->beginUpdate(stream, decodedSize, false);

#endif
#endif
                                                }
                                            }

                                            base64PadLen = ut->trimLastChunkBase64(payload, len);

                                            if (fbdo->_ss.rtdb.req_data_type == d_file)
                                                ut->decodeBase64Stream(payload.c_str() + ofs, len, Signer.getCfg()->_int.fb_file);
                                            else if (fbdo->_ss.rtdb.req_data_type == d_file_ota)
                                            {
                                                dataTime = millis();

#if defined(ENABLE_OTA_FIRMWARE_UPDATE)
                                                if (error.code == 0)
                                                    ut->decodeBase64OTA(payload.c_str() + ofs, len, error.code);
#endif
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (!Signer.getCfg()->_int.fb_file)
                                    {
#if defined FLASH_FS
                                        fbdo->_ss.rtdb.storage_type = mem_storage_type_flash;

                                        if (!Signer.getCfg()->_int.fb_flash_rdy)
                                            ut->flashTest();

                                        Signer.getCfg()->_int.fb_file = FLASH_FS.open(pgm2Str(fb_esp_pgm_str_184), (const char *)FPSTR("w"));

#endif
                                        readLen = payload.length() - response.payloadOfs;
                                        ofs = response.payloadOfs;
                                    }

                                    if (Signer.getCfg()->_int.fb_file)
                                    {
                                        ut->trimLastChunkBase64(payload, len);
                                        ut->decodeBase64Stream(payload.c_str() + ofs, len, Signer.getCfg()->_int.fb_file);
                                    }
                                }

                                if (response.dataType > 0)
                                {
                                    payload.clear();
                                    readLen = 0;
                                }
                            }
                        }

                        if (pChunk)
                        {
                            if (ut->strposP(pChunk, fb_esp_pgm_str_14, 0) > -1 && response.isEvent)
                            {
                                ut->delP(&pChunk);
                                break;
                            }

                            ut->delP(&pChunk);
                        }

                        if (readLen < 0)
                            break;
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

    header.clear();

    if (fbdo->_ss.rtdb.req_data_type == d_file_ota)
    {
        payload.clear();

#if defined(ENABLE_OTA_FIRMWARE_UPDATE)

        //write extra pad
        if (base64PadLen > 0)
        {
            uint8_t pad[base64PadLen];
            memset(pad, 0, base64PadLen);
            Update.write(pad, base64PadLen);
        }

        if (error.code == 0)
        {
            if (!Update.end())
                error.code = FIREBASE_ERROR_FW_UPDATE_END_FAILED;
        }

        if (error.code != 0)
            fbdo->_ss.http_code = error.code;

        return fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_OK;

#endif
    }

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
                    std::vector<MBSTRING> payloadList = std::vector<MBSTRING>();
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

                payload.clear();

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
                    if (fbdo->_ss.rtdb.req_data_type == d_blob || fbdo->_ss.rtdb.req_method == m_download || ((fbdo->_ss.rtdb.req_data_type == d_file || fbdo->_ss.rtdb.req_data_type == d_file_ota) && fbdo->_ss.rtdb.req_method == m_get))
                        ut->parseRespPayload(payload.c_str(), response, true);
                    else
                        ut->parseRespPayload(payload.c_str(), response, false);

                    fbdo->_ss.error = response.fbError;
                }

                fbdo->_ss.rtdb.resp_data_type = response.dataType;
                fbdo->_ss.content_length = response.payloadLen;

                if (fbdo->_ss.rtdb.resp_data_type == d_blob)
                {
                    if (!fbdo->_ss.rtdb.blob)
                    {
                        fbdo->_ss.rtdb.isBlobPtr = true;
                        fbdo->_ss.rtdb.blob = new std::vector<uint8_t>();
                    }
                    else
                        std::vector<uint8_t>().swap(*fbdo->_ss.rtdb.blob);
                    fbdo->_ss.rtdb.raw.clear();
                    ut->decodeBase64Str((const char *)payload.c_str() + response.payloadOfs, *fbdo->_ss.rtdb.blob);
                }
                else if (fbdo->_ss.rtdb.resp_data_type == d_file || Signer.getCfg()->_int.fb_file)
                {
                    ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
                    fbdo->_ss.rtdb.raw.clear();
                }

                if (fbdo->_ss.rtdb.req_method == m_set_rules)
                {
                    if (ut->stringCompare(payload.c_str(), 0, fb_esp_pgm_str_104))
                        payload.clear();
                }

                if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_OK || fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED)
                {

                    if (fbdo->_ss.rtdb.req_method != m_set_rules)
                    {
                        if (fbdo->_ss.rtdb.resp_data_type != d_blob && fbdo->_ss.rtdb.resp_data_type != d_file && fbdo->_ss.rtdb.resp_data_type != d_file_ota)
                        {
                            handlePayload(fbdo, response, payload.c_str());

                            if (fbdo->_ss.rtdb.priority_val_flag)
                            {
                                char *path = (char *)ut->newP(fbdo->_ss.rtdb.path.length());
                                if (path)
                                {
                                    strncpy(path, fbdo->_ss.rtdb.path.c_str(), fbdo->_ss.rtdb.path.length() - strlen_P(fb_esp_pgm_str_156));
                                    fbdo->_ss.rtdb.path = path;
                                    ut->delP(&path);
                                }
                            }

                            //Push (POST) data?
                            if (fbdo->_ss.rtdb.req_method == m_post)
                            {
                                if (response.pushName.length() > 0)
                                {
                                    fbdo->_ss.rtdb.push_name = response.pushName.c_str();
                                    fbdo->_ss.rtdb.resp_data_type = d_any;
                                    fbdo->_ss.rtdb.raw.clear();
                                }
                            }
                        }
                    }
                }

                if (Signer.getCfg()->rtdb.data_type_stricted && fbdo->_ss.rtdb.req_method == m_get && fbdo->_ss.rtdb.req_data_type != d_timestamp && !response.noContent && response.httpCode < 400)
                {
                    bool _reqType = fbdo->_ss.rtdb.req_data_type == d_integer || fbdo->_ss.rtdb.req_data_type == d_float || fbdo->_ss.rtdb.req_data_type == d_double;
                    bool _respType = fbdo->_ss.rtdb.resp_data_type == d_integer || fbdo->_ss.rtdb.resp_data_type == d_float || fbdo->_ss.rtdb.resp_data_type == d_double;

                    if (fbdo->_ss.rtdb.req_data_type == fbdo->_ss.rtdb.resp_data_type || (_reqType && _respType) || (fbdo->_ss.rtdb.priority > 0 && fbdo->_ss.rtdb.resp_data_type == d_json))
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
        //This issue has been fixed on Google side.
        //https://github.com/mobizt/Firebase-ESP-Client/discussions/165#discussioncomment-1561941
        //#ifdef FIX_FIRERBASE_RTDB_PRINT_SILENT
        //if (ut->stringCompare(fbdo->_ss.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151) && fbdo->_ss.rtdb.no_content_req && fbdo->_ss.rtdb.req_method != m_delete)
        //{
        //fbdo->_ss.http_code = FIREBASE_ERROR_PATH_NOT_EXIST;
        //fbdo->_ss.rtdb.path_not_found = true;
        //}
        //else
        //fbdo->_ss.rtdb.path_not_found = false;
        //#else

        if (ut->stringCompare(fbdo->_ss.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151) && response.noContent)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_PATH_NOT_EXIST;
            fbdo->_ss.rtdb.path_not_found = true;
        }
        else
            fbdo->_ss.rtdb.path_not_found = false;
        //#endif

        if (fbdo->_ss.http_code == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
            fbdo->_ss.rtdb.path.clear();
            fbdo->_ss.rtdb.raw.clear();
            fbdo->_ss.rtdb.push_name.clear();
            fbdo->_ss.rtdb.resp_data_type = d_any;
            fbdo->_ss.rtdb.data_available = false;
        }
    }

    ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);

    payload.clear();

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
            fbdo->_ss.http_code = FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED;
        }
        else
        {
            struct fb_esp_url_info_t uinfo;
            ut->getUrlInfo(fbdo->_ss.rtdb.redirect_url, uinfo);
            struct fb_esp_rtdb_request_info_t _req;
            _req.method = fbdo->_ss.rtdb.req_method;
            _req.data.type = fbdo->_ss.rtdb.req_data_type;
            _req.data.address.priority = toAddr(fbdo->_ss.rtdb.priority);
            _req.path = uinfo.uri.c_str();
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

        if (!fbdo->_ss.jsonPtr)
            fbdo->_ss.jsonPtr = new FirebaseJson();

        if (!fbdo->_ss.arrPtr)
            fbdo->_ss.arrPtr = new FirebaseJsonArray();

        if (fbdo->_ss.rtdb.resp_data_type == d_json)
        {
            fbdo->_ss.jsonPtr->setJsonData(fbdo->_ss.rtdb.raw.c_str());
            fbdo->_ss.arrPtr->clear();
        }

        if (fbdo->_ss.rtdb.resp_data_type == d_array)
        {
            fbdo->_ss.arrPtr->setJsonArrayData(fbdo->_ss.rtdb.raw.c_str());
            fbdo->_ss.jsonPtr->clear();
        }

        s.jsonPtr = fbdo->_ss.jsonPtr;
        s.arrPtr = fbdo->_ss.arrPtr;

        s.sif->stream_path = fbdo->_ss.rtdb.stream_path.c_str();
        s.sif->data = fbdo->_ss.rtdb.raw.c_str();
        s.sif->path = fbdo->_ss.rtdb.path.c_str();
        s.sif->payload_length = fbdo->_ss.payload_length;
        s.sif->max_payload_length = fbdo->_ss.max_payload_length;

        s.sif->data_type = fbdo->_ss.rtdb.resp_data_type;
        fbdo->_ss.rtdb.data_type_str = fbdo->getDataType(s.sif->data_type);
        s.sif->data_type_str = fbdo->_ss.rtdb.data_type_str.c_str();
        s.sif->event_type_str = fbdo->_ss.rtdb.event_type.c_str();

        if (fbdo->_ss.rtdb.resp_data_type == d_blob)
        {
            if (!fbdo->_ss.rtdb.blob)
            {
                fbdo->_ss.rtdb.isBlobPtr = true;
                fbdo->_ss.rtdb.blob = new std::vector<uint8_t>();
            }
            s.sif->blob = fbdo->_ss.rtdb.blob;
        }

        fbdo->_dataAvailableCallback(s);
        fbdo->_ss.rtdb.data_available = false;

        s.empty();
    }
    else if (fbdo->_multiPathDataCallback)
    {
        FIREBASE_MP_STREAM_CLASS s;
        s.begin(ut, &fbdo->_ss.rtdb.stream);
        s.sif->data_type = fbdo->_ss.rtdb.resp_data_type;
        s.sif->path = fbdo->_ss.rtdb.path;
        s.sif->data_type_str = fbdo->getDataType(s.sif->data_type);
        s.sif->event_type_str = fbdo->_ss.rtdb.event_type;
        s.sif->payload_length = fbdo->_ss.payload_length;
        s.sif->max_payload_length = fbdo->_ss.max_payload_length;

        if (!fbdo->_ss.jsonPtr)
            fbdo->_ss.jsonPtr = new FirebaseJson();

        if (fbdo->_ss.rtdb.resp_data_type == d_json)
            fbdo->_ss.jsonPtr->setJsonData(fbdo->_ss.rtdb.raw.c_str());

        if (s.sif->data_type == d_json)
            s.sif->m_json = fbdo->_ss.jsonPtr;
        else
        {
            fbdo->_ss.jsonPtr->clear();
            if (s.sif->data_type == d_string || s.sif->data_type == d_std_string || s.sif->data_type == d_mb_string)
                s.sif->data = fbdo->_ss.rtdb.raw.substr(1, fbdo->_ss.rtdb.raw.length() - 2).c_str();
            else
                s.sif->data = fbdo->_ss.rtdb.raw.c_str();
        }

        fbdo->_multiPathDataCallback(s);
        fbdo->_ss.rtdb.data_available = false;
        s.empty();
    }
}

void FB_RTDB::splitStreamPayload(const char *payloads, std::vector<MBSTRING> &payload)
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
                char *tmp = (char *)ut->newP(len + 10);
                if (tmp)
                {
                    strncpy(tmp, payloads + pos1, len);
                    payload.push_back(tmp);
                    ut->delP(&tmp);
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

    if (fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr->clear();

    if (fbdo->_ss.arrPtr)
        fbdo->_ss.arrPtr->clear();

    if (fbdo->_ss.rtdb.resp_data_type == d_blob)
    {
        if (fbdo->_ss.rtdb.blob)
            std::vector<uint8_t>().swap(*fbdo->_ss.rtdb.blob);
        else
        {
            fbdo->_ss.rtdb.isBlobPtr = true;
            fbdo->_ss.rtdb.blob = new std::vector<uint8_t>();
        }

        fbdo->_ss.rtdb.raw.clear();
        ut->decodeBase64Str((const char *)payload + response.payloadOfs, *fbdo->_ss.rtdb.blob);
    }
    else if (fbdo->_ss.rtdb.resp_data_type == d_file || Signer.getCfg()->_int.fb_file)
    {
        ut->closeFileHandle(fbdo->_ss.rtdb.storage_type == mem_storage_type_sd);
        fbdo->_ss.rtdb.raw.clear();
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

    fbdo->_ss.rtdb.raw.clear();

    if (fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr->clear();

    if (fbdo->_ss.arrPtr)
        fbdo->_ss.arrPtr->clear();

    if (response.isEvent)
    {
        response.eventPathChanged = strcmp(response.eventPath.c_str(), fbdo->_ss.rtdb.path.c_str()) != 0;
        fbdo->_ss.rtdb.path = response.eventPath;
        fbdo->_ss.rtdb.event_type = response.eventType;
    }
    if (fbdo->_ss.rtdb.resp_data_type != d_blob && fbdo->_ss.rtdb.resp_data_type != d_file_ota)
    {
        if (response.isEvent)
            fbdo->_ss.rtdb.raw = response.eventData.c_str();
        else
            fbdo->_ss.rtdb.raw = payload;
        uint16_t crc = ut->calCRC(fbdo->_ss.rtdb.raw.c_str());
        response.dataChanged = fbdo->_ss.rtdb.data_crc != crc;
        fbdo->_ss.rtdb.data_crc = crc;
    }
}

size_t FB_RTDB::getPayloadLen(fb_esp_rtdb_request_info_t *req)
{
    size_t len = 0;
    if (req->method != m_get)
    {
        if (req->data.address.din > 0)
        {
            if (req->data.type == d_blob && req->data.address.priority == 0)
                len = (4 * ceil(req->data.blobSize / 3.0)) + strlen_P(fb_esp_pgm_str_92) + 1;
            else if (req->data.type == d_json)
            {
                FirebaseJson *json = addrTo<FirebaseJson *>(req->data.address.din);
                len = strlen(json->raw());
            }
            else if (req->data.type == d_array)
            {
                FirebaseJsonArray *arr = addrTo<FirebaseJsonArray *>(req->data.address.din);
                len = req->pre_payload.length() + strlen(arr->raw()) + req->post_payload.length();
            }
        }
        else if (req->payload.length() > 0)
            len = req->pre_payload.length() + req->payload.length() + req->post_payload.length();
        else if (req->fileLen > 0 && req->data.address.priority == 0)
            len = req->fileLen;
        else if (req->method == m_restore)
            len = req->fileLen;
    }
    return len;
}

int FB_RTDB::sendHeader(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    fb_esp_method http_method = m_put;
    fbdo->_ss.rtdb.shallow_flag = false;
    fbdo->_ss.rtdb.priority_val_flag = false;

    bool hasServerValue = false;

    if (req->data.type == d_json)
    {
        if (req->data.address.din > 0 && req->data.type == d_json)
        {
            FirebaseJson *json = addrTo<FirebaseJson *>(req->data.address.din);
            hasServerValue = ut->strpos(json->raw(), pgm2Str(fb_esp_pgm_str_166), 0) != -1;
        }
        else
            hasServerValue = ut->strpos(req->payload.c_str(), pgm2Str(fb_esp_pgm_str_166), 0) != -1;
    }

    MBSTRING header;
    int ret = -1;
    if (req->method == m_stream)
        header.appendP(fb_esp_pgm_str_22, true);
    else
    {
        if (req->method == m_put || req->method == m_put_nocontent || req->method == m_set_priority || req->method == m_set_rules)
        {
            http_method = m_put;
            if (fbdo->_ss.classic_request)
                header.appendP(fb_esp_pgm_str_24, true);
            else
                header.appendP(fb_esp_pgm_str_23, true);
        }
        else if (req->method == m_post)
        {
            http_method = m_post;
            header.appendP(fb_esp_pgm_str_24, true);
        }
        else if (req->method == m_get || req->method == m_get_nocontent || req->method == m_get_shallow || req->method == m_get_priority || req->method == m_download || req->method == m_read_rules)
        {
            http_method = m_get;
            header.appendP(fb_esp_pgm_str_25, true);
        }
        else if (req->method == m_patch || req->method == m_patch_nocontent || req->method == m_restore)
        {
            http_method = m_patch;
            header.appendP(fb_esp_pgm_str_26, true);
        }
        else if (req->method == m_delete)
        {
            http_method = m_delete;
            if (fbdo->_ss.classic_request)
                header.appendP(fb_esp_pgm_str_24, true);
            else
                header.appendP(fb_esp_pgm_str_27, true);
        }
        header.appendP(fb_esp_pgm_str_6);
    }

    if (req->path.length() > 0)
        if (req->path[0] != '/')
            header.appendP(fb_esp_pgm_str_1);

    header += req->path;

    if (req->method == m_patch || req->method == m_patch_nocontent)
        header.appendP(fb_esp_pgm_str_1);

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
        if (Signer.getTokenType() == token_type_oauth2_access_token || Signer.getCfg()->signer.test_mode)
            header.appendP(fb_esp_pgm_str_238);
        else
            header.appendP(fb_esp_pgm_str_2);

        ret = fbdo->tcpSend(header.c_str());
        header.clear();

        if (ret < 0)
            return ret;

        if (Signer.getTokenType() != token_type_oauth2_access_token && !Signer.getCfg()->signer.test_mode)
            ret = fbdo->tcpSend(Signer.getCfg()->_int.auth_token.c_str());

        if (ret < 0)
            return ret;
    }

    if (fbdo->_ss.rtdb.read_tmo > 0)
    {
        header.appendP(fb_esp_pgm_str_158);
        header += fbdo->_ss.rtdb.read_tmo;
        header.appendP(fb_esp_pgm_str_159);
    }

    if (fbdo->_ss.rtdb.write_limit.length() > 0)
    {
        header.appendP(fb_esp_pgm_str_160);
        header += fbdo->_ss.rtdb.write_limit;
    }

    if (req->method == m_get_shallow)
    {
        header.appendP(fb_esp_pgm_str_155);
        fbdo->_ss.rtdb.shallow_flag = true;
    }

    QueryFilter *query = req->data.address.query > 0 ? addrTo<QueryFilter *>(req->data.address.query) : nullptr;

    bool hasQuery = false;

    if (req->method == m_get && query)
    {
        if (query->_orderBy.length() > 0)
        {
            hasQuery = true;
            header.appendP(fb_esp_pgm_str_96);
            header += query->_orderBy;

            if (req->method == m_get)
            {
                if (query->_limitToFirst.length() > 0)
                {
                    header.appendP(fb_esp_pgm_str_97);
                    header += query->_limitToFirst;
                }

                if (query->_limitToLast.length() > 0)
                {
                    header.appendP(fb_esp_pgm_str_98);
                    header += query->_limitToLast;
                }

                if (query->_startAt.length() > 0)
                {
                    header.appendP(fb_esp_pgm_str_99);
                    header += query->_startAt;
                }

                if (query->_endAt.length() > 0)
                {
                    header.appendP(fb_esp_pgm_str_100);
                    header += query->_endAt;
                }

                if (query->_equalTo.length() > 0)
                {
                    header.appendP(fb_esp_pgm_str_101);
                    header += query->_equalTo;
                }
            }
        }
    }

    if (req->method == m_download)
    {
        header.appendP(fb_esp_pgm_str_162);
        header.appendP(fb_esp_pgm_str_28);
        MBSTRING filename;

        for (size_t i = 0; i < fbdo->_ss.rtdb.backup_node_path.length(); i++)
        {
            if (fbdo->_ss.rtdb.backup_node_path.c_str()[i] == '/')
                filename.appendP(fb_esp_pgm_str_4);
            else
                filename += fbdo->_ss.rtdb.backup_node_path.c_str()[i];
        }

        header += filename;
        filename.clear();
    }

    if (req->method == m_get && fbdo->_ss.rtdb.file_name.length() > 0)
    {
        header.appendP(fb_esp_pgm_str_28);
        header += fbdo->_ss.rtdb.file_name;
    }

    //This issue has been fixed on Google side.
    //https://github.com/mobizt/Firebase-ESP-Client/discussions/165#discussioncomment-1561941
    //#ifndef FIX_FIRERBASE_RTDB_PRINT_SILENT
    if (req->async || req->method == m_get_nocontent || req->method == m_restore || req->method == m_put_nocontent || req->method == m_patch_nocontent)
        header.appendP(fb_esp_pgm_str_29);
    //#endif

    header.appendP(fb_esp_pgm_str_30);
    header.appendP(fb_esp_pgm_str_31);
    header += Signer.getCfg()->database_url;
    header.appendP(fb_esp_pgm_str_21);
    header.appendP(fb_esp_pgm_str_32);

    if (Signer.getTokenType() == token_type_oauth2_access_token)
    {
        header.appendP(fb_esp_pgm_str_237);
        header += Signer.getCfg()->signer.tokens.auth_type;
        header.appendP(fb_esp_pgm_str_6);

        ret = fbdo->tcpSend(header.c_str());
        header.clear();

        if (ret < 0)
            return ret;

        ret = fbdo->tcpSend(Signer.getCfg()->_int.auth_token.c_str());

        if (ret < 0)
            return ret;

        header.appendP(fb_esp_pgm_str_21);
    }

    //Timestamp cannot use with ETag header, otherwise cases internal server error
    if (!hasServerValue && !hasQuery && req->data.type != d_timestamp && (req->method == m_delete || req->method == m_get || req->method == m_get_nocontent || req->method == m_put || req->method == m_put_nocontent || req->method == m_post))
        header.appendP(fb_esp_pgm_str_148);

    if (fbdo->_ss.rtdb.req_etag.length() > 0 && (req->method == m_put || req->method == m_put_nocontent || req->method == m_delete))
    {
        header.appendP(fb_esp_pgm_str_149);
        header += fbdo->_ss.rtdb.req_etag;
        header.appendP(fb_esp_pgm_str_21);
    }

    if (fbdo->_ss.classic_request && http_method != m_get && http_method != m_post && http_method != m_patch)
    {
        header.appendP(fb_esp_pgm_str_153);

        if (http_method == m_put)
            header.appendP(fb_esp_pgm_str_23);
        else if (http_method == m_delete)
            header.appendP(fb_esp_pgm_str_27);

        header.appendP(fb_esp_pgm_str_21);
    }

    if (req->method == m_stream)
    {
        fbdo->_ss.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        header.appendP(fb_esp_pgm_str_34);
        header.appendP(fb_esp_pgm_str_35);
    }
    else if (req->method == m_download || req->method == m_restore)
    {
        fbdo->_ss.rtdb.http_req_conn_type = fb_esp_http_connection_type_close;
        header.appendP(fb_esp_pgm_str_34);
    }
    else
    {
        fbdo->_ss.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        header.appendP(fb_esp_pgm_str_36);
        header.appendP(fb_esp_pgm_str_37);
    }

    if (req->method != m_download && req->method != m_restore)
        header.appendP(fb_esp_pgm_str_38);

    if (req->method == m_get_priority || req->method == m_set_priority)
        fbdo->_ss.rtdb.priority_val_flag = true;

    if (req->method == m_put || req->method == m_put_nocontent || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent || req->method == m_restore || req->method == m_set_rules || req->method == m_set_priority)
    {
        header.appendP(fb_esp_pgm_str_12);
        header += getPayloadLen(req);
    }
    header.appendP(fb_esp_pgm_str_21);
    header.appendP(fb_esp_pgm_str_21);

    ret = fbdo->tcpSend(header.c_str());
    header.clear();
    return ret;
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
    fbdo->_ss.rtdb.raw.clear();
    fbdo->_ss.rtdb.backup_node_path.clear();
    fbdo->_ss.rtdb.data_type_str.clear();
    fbdo->_ss.rtdb.path.clear();
    fbdo->_ss.rtdb.push_name.clear();

    if (fbdo->_ss.jsonPtr)
        fbdo->_ss.jsonPtr->clear();

    if (fbdo->_ss.arrPtr)
        fbdo->_ss.arrPtr->clear();
}

bool FB_RTDB::connectionError(FirebaseData *fbdo)
{
    return fbdo->_ss.http_code == FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED || fbdo->_ss.http_code == FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST ||
           fbdo->_ss.http_code == FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED || fbdo->_ss.http_code == FIREBASE_ERROR_TCP_ERROR_SEND_HEADER_FAILED ||
           fbdo->_ss.http_code == FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED || fbdo->_ss.http_code == FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
}

bool FB_RTDB::handleStreamRequest(FirebaseData *fbdo, const MBSTRING &path)
{

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady() || !fbdo->validRequest(path))
        return false;

    bool ret = false;
    struct fb_esp_rtdb_request_info_t _req;
    _req.method = m_stream;
    _req.data.type = d_string;
    if (fbdo->_ss.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        ut->getUrlInfo(fbdo->_ss.rtdb.redirect_url, uinfo);
        _req.path = uinfo.uri.c_str();
    }
    else
        _req.path = path.c_str();

    ret = sendRequest(fbdo, &_req) == 0;

    if (!ret)
        fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    return ret;
}

#endif

#endif //ENABLE