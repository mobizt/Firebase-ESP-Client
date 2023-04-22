#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Realtime Database class, FB_RTDB.cpp version 2.0.14
 *
 * This library supports Espressif ESP8266, ESP32 and RP2040 Pico
 *
 * Created April 5, 2023
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
#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#include "FirebaseFS.h"

#ifdef ENABLE_RTDB

#ifndef FIREBASE_RTDB_CPP
#define FIREBASE_RTDB_CPP

#include "FB_RTDB.h"

FB_RTDB::FB_RTDB()
{
}

FB_RTDB::~FB_RTDB()
{
}

void FB_RTDB::end(FirebaseData *fbdo)
{
    endStream(fbdo);
#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)
    removeStreamCallback(fbdo);
#endif
    fbdo->clear();
}

void FB_RTDB::mSetReadTimeout(FirebaseData *fbdo, MB_StringPtr millisec)
{
    MB_String _millisec = millisec;
    if (atoi(_millisec.c_str()) <= 900000)
        fbdo->session.rtdb.read_tmo = atoi(_millisec.c_str());
}

void FB_RTDB::mSetwriteSizeLimit(FirebaseData *fbdo, MB_StringPtr size)
{
    fbdo->session.rtdb.write_limit = size;
}

bool FB_RTDB::mGetRules(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr filename,
                        RTDB_DownloadProgressCallback callback)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path += fb_esp_rtdb_pgm_str_1; // "/.settings/rules"
    req.method = rtdb_get_rules;
    req.data.type = d_json;
    req.filename = filename;
    req.storageType = storageType;
    req.downloadCallback = callback;

    Utils::makePath(req.filename);
    req.task_type = req.filename.length() ? fb_esp_rtdb_task_download_rules : fb_esp_rtdb_task_read_rules;

    bool ret = handleRequest(fbdo, &req);

    if (req.filename.length() == 0)
    {
        if (fbdo->jsonObjectPtr()->errorPosition() > -1)
        {
            fbdo->session.response.code = FIREBASE_ERROR_INVALID_JSON_RULES;
            ret = false;
        }
        fbdo->jsonObjectPtr()->clear();
    }

    return ret;
}

bool FB_RTDB::mSetRules(FirebaseData *fbdo, MB_StringPtr rules, fb_esp_mem_storage_type storageType,
                        MB_StringPtr filename, RTDB_UploadProgressCallback callback)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path += fb_esp_rtdb_pgm_str_1; // "/.settings/rules"
    req.method = rtdb_set_rules;
    req.payload = rules;
    req.filename = filename;
    Utils::makePath(req.filename);
    req.storageType = storageType;
    req.task_type = req.filename.length() ? fb_esp_rtdb_task_upload_rules : fb_esp_rtdb_task_store_rules;
    req.data.type = d_json;
    req.uploadCallback = callback;
    bool ret = handleRequest(fbdo, &req);
    return ret;
}

void FB_RTDB::storeToken(MB_String &atok, const char *databaseSecret)
{
    atok = Signer.config->internal.auth_token;
    Signer.setTokenType(token_type_legacy_token);
    Signer.config->signer.tokens.legacy_token = databaseSecret;
    Signer.config->internal.auth_token = Signer.config->signer.tokens.legacy_token;
    Signer.config->internal.ltok_len = strlen(databaseSecret);
    Signer.config->internal.rtok_len = 0;
    Signer.config->internal.atok_len = 0;
    Signer.handleToken();
}

void FB_RTDB::restoreToken(MB_String &atok, fb_esp_auth_token_type tk)
{
    Signer.config->internal.auth_token = atok;
    atok.clear();
    Signer.config->signer.tokens.legacy_token = "";
    Signer.config->signer.tokens.token_type = tk;
    Signer.config->internal.atok_len = Signer.config->internal.auth_token.length();
    Signer.config->internal.ltok_len = 0;
    Signer.handleToken();
}

bool FB_RTDB::mSetQueryIndex(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr node,
                             MB_StringPtr databaseSecret)
{
    if (fbdo->session.rtdb.pause)
        return true;

    MB_String s;
    bool ret = false;
    MB_String atok;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) &&
            tk != token_type_oauth2_access_token &&
            tk != token_type_legacy_token)
            storeToken(atok, addrTo<const char *>(databaseSecret.address()));
    }

    if (getRules(fbdo))
    {
        ret = true;
        FirebaseJsonData data;
        FirebaseJson *json = fbdo->jsonObjectPtr();
        if (json->errorPosition() > -1)
        {
            fbdo->session.response.code = FIREBASE_ERROR_INVALID_JSON_RULES;
            ret = false;
        }
        else
        {
            bool ruleExisted = false;

            s = fb_esp_rtdb_pgm_str_3; // "rules"

            MB_String _path = path;
            MB_String _node = node;
            Utils::makePath(_path);
            s += _path;
            s += fb_esp_rtdb_pgm_str_4; // "/.indexOn"

            if (JsonHelper::parse(json, &data, s.c_str()) &&
                strcmp(data.to<const char *>(), _node.c_str()) == 0)
                ruleExisted = true;

            if (_node.length() == 0)
                JsonHelper::remove(json, s.c_str());
            else
                JsonHelper::addString(json, s.c_str(), _node);

            if (!ruleExisted || (ruleExisted && _node.length() == 0))
            {
                MB_String str;
                JsonHelper::toString(json, str, false, true);
                ret = setRules(fbdo, str);
            }
        }

        json->clear();
    }

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) &&
            tk != token_type_oauth2_access_token &&
            tk != token_type_legacy_token)
            restoreToken(atok, tk);
    }

    s.clear();
    return ret;
}

bool FB_RTDB::mSetReadWriteRules(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr var,
                                 MB_StringPtr readVal, MB_StringPtr writeVal, MB_StringPtr databaseSecret)
{
    if (fbdo->session.rtdb.pause)
        return true;

    MB_String s;
    bool ret = false;
    MB_String atok;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) &&
            tk != token_type_oauth2_access_token &&
            tk != token_type_legacy_token)
            storeToken(atok, addrTo<const char *>(databaseSecret.address()));
    }

    if (getRules(fbdo))
    {
        MB_String _readVal = readVal;
        MB_String _writeVal = writeVal;
        MB_String _path = path;

        ret = true;
        FirebaseJsonData data;
        FirebaseJson &json = fbdo->jsonObject();
        bool rd = false, wr = false;

        MB_String s = fb_esp_rtdb_pgm_str_3; // "rules"
        Utils::makePath(_path);

        s += _path;
        s += fb_esp_pgm_str_1; // "/"
        s += var;

        if (_readVal.length() > 0)
        {
            rd = true;
            MB_String r = s;
            r += fb_esp_pgm_str_1;      // "/"
            r += fb_esp_rtdb_pgm_str_5; // ".read"
            if (JsonHelper::parse(&json, &data, r.c_str()) &&
                strcmp(data.to<const char *>(), _readVal.c_str()) == 0)
                rd = false;
        }

        if (_writeVal.length() > 0)
        {
            wr = true;
            MB_String w = s;
            w += fb_esp_pgm_str_1;      // "/"
            w += fb_esp_rtdb_pgm_str_6; // ".write"
            if (JsonHelper::parse(&json, &data, w.c_str()) &&
                strcmp(data.to<const char *>(), _writeVal.c_str()) == 0)
                wr = false;
        }

        // modify if the rules changed or does not exist.
        if (wr || rd)
        {
            FirebaseJson js;
            if (rd)
                JsonHelper::addString(&js, fb_esp_rtdb_pgm_str_5 /* ".read"*/, _readVal);
            if (wr)
                JsonHelper::addString(&js, fb_esp_rtdb_pgm_str_6 /* ".write" */, _writeVal);

            JsonHelper::addObject(&json, s.c_str(), &js, false);
            MB_String str;
            JsonHelper::toString(&json, str, false, true);
            ret = setRules(fbdo, str);
        }

        json.clear();
    }

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) &&
            tk != token_type_oauth2_access_token &&
            tk != token_type_legacy_token)
            restoreToken(atok, tk);
    }

    s.clear();
    return ret;
}

bool FB_RTDB::mPathExisted(FirebaseData *fbdo, MB_StringPtr path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = rtdb_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return !fbdo->session.rtdb.path_not_found;
    return false;
}

String FB_RTDB::mGetETag(FirebaseData *fbdo, MB_StringPtr path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = rtdb_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return fbdo->session.rtdb.resp_etag.c_str();
    return String();
}

bool FB_RTDB::mGetShallowData(FirebaseData *fbdo, MB_StringPtr path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = rtdb_get_shallow;
    req.data.type = d_string;
    return handleRequest(fbdo, &req);
}

void FB_RTDB::enableClassicRequest(FirebaseData *fbdo, bool enable)
{
    fbdo->session.classic_request = enable;
}

bool FB_RTDB::buildRequest(FirebaseData *fbdo, fb_esp_request_method method, MB_StringPtr path,
                           MB_StringPtr payload, fb_esp_data_type type, int subtype, uint32_t value_addr,
                           uint32_t query_addr, uint32_t priority_addr, MB_StringPtr etag, bool async,
                           bool queue, size_t blob_size, MB_StringPtr filename, fb_esp_mem_storage_type storage_type,
                           RTDB_DownloadProgressCallback downloadCallback, RTDB_UploadProgressCallback uploadCallback)
{
    Utils::idle();

#if defined(MB_ARDUINO_PICO)
    if (!Utils::waitIdle(fbdo->session.response.code, Signer.config))
        return false;
#endif

    struct fb_esp_rtdb_request_info_t req;

    MB_String _path, tpath, pre, post;

    tpath = path;
    Utils::makePath(tpath);
    _path = tpath;

    req.downloadCallback = downloadCallback;
    req.uploadCallback = uploadCallback;

    if (method == rtdb_set_priority || method == rtdb_get_priority)
    {
        tpath += fb_esp_pgm_str_1;      // "/"
        tpath += fb_esp_rtdb_pgm_str_2; // "/.priority"
        req.path = tpath;
    }
    else if (priority_addr > 0 && method != http_get && type != d_blob && type != d_file && type != d_file_ota)
    {
        if (type == d_json)
        {
            // set priority to source json
            FirebaseJson *json = addrTo<FirebaseJson *>(value_addr);
            float *priority = addrTo<float *>(priority_addr);
            if (json && priority)
                json->set(pgm2Str(fb_esp_rtdb_pgm_str_2) /* ".priority" */, *priority);
            req.path = tpath;
        }
        else
        {
            // construct json pre and post payloads for priority
            // pre -> {".priority":priority,"subpath":
            // pos -> }

            float *priority = addrTo<float *>(priority_addr);
            if (priority)
            {
                pre += fb_esp_pgm_str_10;     // "{"
                pre += fb_esp_pgm_str_4;      // "\""
                pre += fb_esp_rtdb_pgm_str_2; // ".priority"
                pre += fb_esp_pgm_str_4;      // "\""
                pre += fb_esp_pgm_str_2;      // ":"
                pre += *priority;
                pre += fb_esp_pgm_str_3; // ","
                pre += fb_esp_pgm_str_4; // "\""

                if (type == d_string)
                    post += fb_esp_pgm_str_4; // "\""
                post += fb_esp_pgm_str_11;    // "}"

                int p1 = 0, p2 = 0;
                tpath = path;
                if (StringHelper::find(tpath, fb_esp_pgm_str_1 /* "/" */, true, 0, p1))
                {
                    if (p1 > 0)
                    {
                        p2 = p1 + 1;
                        tpath.substr(req.path, 0, p2);
                        // subpath
                        pre += tpath.substr(p2, _path.length() - 1 - p2);
                    }
                    else
                    {
                        tpath.erase(0, 1);
                        // subpath
                        pre += tpath;
                        tpath = '/';
                        req.path = tpath;
                    }
                }

                pre += fb_esp_pgm_str_4; // "\""
                pre += fb_esp_pgm_str_2; // ":"
                if (type == d_string)
                    pre += fb_esp_pgm_str_4; // "\""

                req.pre_payload = pre;
                req.post_payload = post;
            }
        }
    }
    else
    {
        if (type == d_string)
        {
            pre += fb_esp_pgm_str_4;  // "\""
            post += fb_esp_pgm_str_4; // "\""
            req.pre_payload = pre;
            req.post_payload = post;
        }

        req.path = tpath;
    }

    if (method == http_get && type == d_blob)
        setBlobRef(fbdo, value_addr);

    req.method = method;
    req.data.type = type;
    req.async = async;
    req.queue = queue;
    method == http_get ? req.data.address.dout = value_addr : req.data.address.din = value_addr;
    req.data.address.priority = priority_addr;
    req.data.address.query = query_addr;
    req.data.etag = etag;
    req.data.value_subtype = subtype;
    req.data.blobSize = blob_size;
    req.payload = payload;
    req.filename = filename;
    Utils::makePath(req.filename);
    fbdo->session.rtdb.filename = req.filename;
    req.storageType = storage_type;

    if (type == d_file_ota)
        fbdo->closeSession();

#if defined(ESP8266)
    int rx_size = fbdo->session.bssl_rx_size;
    if (type == d_file_ota)
        fbdo->session.bssl_rx_size = 16384;
#endif

    bool ret = processRequest(fbdo, &req);

#if defined(ESP8266)
    if (type == d_file_ota)
        fbdo->session.bssl_rx_size = rx_size;
#endif

    if (type == d_file_ota)
        fbdo->closeSession();

    return ret;
}

bool FB_RTDB::mDeleteNodesByTimestamp(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr timestampNode,
                                      MB_StringPtr limit, MB_StringPtr dataRetentionPeriod)
{
    if (fbdo->session.rtdb.pause)
        return true;

    time_t current_ts = Signer.getTime();

    if (current_ts < ESP_DEFAULT_TS)
    {
        fbdo->session.response.code = FIREBASE_ERROR_SYS_TIME_IS_NOT_READY;
        return false;
    }

    bool ret = false;

    MB_String lm = limit;
    MB_String _timestampNode = timestampNode;
    MB_String _dataRetentionPeriod = dataRetentionPeriod;

    int _limit = atoi(lm.c_str());

    if (_limit > 30)
        _limit = 30;

#if defined(__AVR__)
    uint32_t pr = Utils::strtoull_alt(_dataRetentionPeriod.c_str());
#else
    char *pEnd;
    uint32_t pr = strtoull(_dataRetentionPeriod.c_str(), &pEnd, 10);
#endif

    QueryFilter query;

    uint32_t lastTS = current_ts - pr;

    if (strcmp(_timestampNode.c_str(), (const char *)MBSTRING_FLASH_MCR("$key")) == 0)
        query.orderBy(_timestampNode).startAt(MB_String(0)).endAt(MB_String((int)lastTS)).limitToLast(_limit);
    else
        query.orderBy(_timestampNode).startAt(0).endAt(lastTS).limitToLast(_limit);

    if (getJSON(fbdo, path, &query))
    {
        ret = true;
        if (fbdo->session.rtdb.resp_data_type == d_json && fbdo->jsonString().length() > 4)
        {
            FirebaseJson *js = fbdo->jsonObjectPtr();
            size_t len = js->iteratorBegin();
            FirebaseJson::IteratorValue value;
            MB_String *nodes = new MB_String[len];

            for (size_t i = 0; i < len; i++)
            {
                value = js->valueAt(i);
                if (value.type == FirebaseJson::JSON_OBJECT && value.key.length() > 1)
                    nodes[i] = value.key;
            }
            js->iteratorEnd();
            js->clear();

            for (size_t i = 0; i < len; i++)
            {
                MB_String s = path;
                s += fb_esp_pgm_str_1; // "/"
                s += nodes[i];
                deleteNode(fbdo, s);
            }

            delete[] nodes;
            nodes = nullptr;
        }
    }

    query.clear();
    return ret;
}

bool FB_RTDB::mBeginStream(FirebaseData *fbdo, MB_StringPtr path)
{

    if (!Signer.config)
        return false;

#if defined(MB_ARDUINO_PICO)
    if (!Utils::waitIdle(fbdo->session.response.code, Signer.config))
        return false;
#endif

    fbdo->session.rtdb.pause = false;

    fbdo->session.rtdb.new_stream = true;

    if (!fbdo->tcpClient.reserved)
        fbdo->closeSession();

    fbdo->session.rtdb.stream_stop = false;
    fbdo->session.rtdb.data_tmo = false;
    fbdo->session.rtdb.stream_path = path;

    if (!fbdo->tcpClient.reserved && !handleStreamRequest(fbdo, fbdo->session.rtdb.stream_path))
    {
        if (!fbdo->tokenReady())
            return true;

        return false;
    }

    clearDataStatus(fbdo);

    fb_esp_rtdb_request_info_t req;
    return waitResponse(fbdo, &req);
}

bool FB_RTDB::mBeginMultiPathStream(FirebaseData *fbdo, MB_StringPtr parentPath)
{
    return mBeginStream(fbdo, parentPath);
}

bool FB_RTDB::readStream(FirebaseData *fbdo)
{
    return handleStreamRead(fbdo);
}

bool FB_RTDB::endStream(FirebaseData *fbdo)
{
    fbdo->session.rtdb.pause = true;
    fbdo->session.rtdb.stream_stop = true;
    fbdo->session.con_mode = fb_esp_con_mode_undefined;
    fbdo->closeSession();
    clearDataStatus(fbdo);
    return true;
}

bool FB_RTDB::handleStreamRead(FirebaseData *fbdo)
{

    if (Signer.isExpired())
        return false;

#if defined(MB_ARDUINO_PICO)
    if (!Utils::waitIdle(fbdo->session.response.code, Signer.config))
        return false;
#endif

    // if the client used by the authentication task
    if (fbdo->tcpClient.reserved)
        return false;

    // prevent redundant calling
    if (fbdo->session.streaming)
        return false;

    fbdo->session.streaming = true;

    if (fbdo->session.rtdb.pause || fbdo->session.rtdb.stream_stop)
        return exitStream(fbdo, true);

    if (!fbdo->tokenReady())
        return exitStream(fbdo, false);

    bool ret = false;
    bool reconnectStream = false;

    // trying to reconnect the stream when required at some interval as running in the loop
    if (Signer.config->timeout.rtdbStreamReconnect < MIN_RTDB_STREAM_RECONNECT_INTERVAL ||
        Signer.config->timeout.rtdbStreamReconnect > MAX_RTDB_STREAM_RECONNECT_INTERVAL)
        Signer.config->timeout.rtdbStreamReconnect = MIN_RTDB_STREAM_RECONNECT_INTERVAL;

    if (millis() - Signer.config->timeout.rtdbStreamReconnect > fbdo->session.rtdb.stream_resume_millis)
    {
        reconnectStream = (fbdo->session.rtdb.data_tmo && !fbdo->session.connected) ||
                          fbdo->session.response.code >= 400 ||
                          fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream;

        if (fbdo->session.rtdb.data_tmo)
        {
            reconnectStream = true;
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
        }
        fbdo->session.rtdb.stream_resume_millis = millis();
    }
    else
        ret = true;

    // Stream timed out
    if (Signer.config->timeout.rtdbKeepAlive < MIN_RTDB_KEEP_ALIVE_TIMEOUT ||
        Signer.config->timeout.rtdbKeepAlive > MAX_RTDB_KEEP_ALIVE_TIMEOUT)
        Signer.config->timeout.rtdbKeepAlive = DEFAULT_RTDB_KEEP_ALIVE_TIMEOUT;

    if (millis() - fbdo->session.rtdb.data_millis > Signer.config->timeout.rtdbKeepAlive)
    {
        fbdo->session.rtdb.data_millis = millis();
        fbdo->session.rtdb.data_tmo = true;
        reconnectStream = true;
        fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
    }

    if (reconnectStream)
    {
        fbdo->session.rtdb.new_stream = true;

        if (!Utils::waitIdle(fbdo->session.response.code, Signer.config))
            return exitStream(fbdo, false);

        fbdo->closeSession();

        if (!fbdo->tokenReady())
            return exitStream(fbdo, false);

        MB_String path = fbdo->session.rtdb.stream_path;
        if (fbdo->session.rtdb.redirect_url.length() > 0)
        {
            struct fb_esp_url_info_t uinfo;
            URLHelper::parse(Signer.mbfs, fbdo->session.rtdb.redirect_url, uinfo);
            path = uinfo.uri;
        }

        // mode changed, non-stream -> stream, reset the data timeout state to allow data available to be notified.
        fbdo->session.rtdb.data_tmo = false;

        if (!handleStreamRequest(fbdo, path))
        {
            fbdo->session.rtdb.data_tmo = true;
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
            return exitStream(fbdo, ret);
        }

        fbdo->session.con_mode = fb_esp_con_mode_rtdb_stream;
    }

    fb_esp_rtdb_request_info_t req;

    if (!waitResponse(fbdo, &req))
        return exitStream(fbdo, ret);

    return exitStream(fbdo, true);
}

bool FB_RTDB::exitStream(FirebaseData *fbdo, bool status)
{
    fbdo->session.streaming = false;
    return status;
}

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
void FB_RTDB::setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback,
                                FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize)
{
    fbdo->session.rtdb.stream_loop_task_enable = false;
#else
void FB_RTDB::setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback,
                                FirebaseData::StreamTimeoutCallback timeoutCallback)
{
#endif

    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    removeMultiPathStreamCallback(fbdo);

    fbdo->_dataAvailableCallback = dataAvailableCallback;
    fbdo->_timeoutCallback = timeoutCallback;

    fbdo->addSession(fb_esp_con_mode_rtdb_stream);
    Signer.config->internal.stream_loop_task_enable = true;

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
    runStreamTask(fbdo, fbdo->getTaskName(streamTaskStackSize, true));
#elif defined(ESP8266)
    Signer.set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#else
runStreamTask();
#endif
}

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
void FB_RTDB::setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback,
                                         FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize)
{
    fbdo->session.rtdb.stream_loop_task_enable = false;
#else
void FB_RTDB::setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback,
                                         FirebaseData::StreamTimeoutCallback timeoutCallback)
{
#endif
    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    removeStreamCallback(fbdo);

    fbdo->_multiPathDataCallback = multiPathDataCallback;
    fbdo->_timeoutCallback = timeoutCallback;

    fbdo->addSession(fb_esp_con_mode_rtdb_stream);
    Signer.config->internal.stream_loop_task_enable = true;

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
    runStreamTask(fbdo, fbdo->getTaskName(streamTaskStackSize, true));
#elif defined(ESP8266)
    Signer.set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#else
runStreamTask();
#endif
}

void FB_RTDB::removeMultiPathStreamCallback(FirebaseData *fbdo)
{
    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    fbdo->_multiPathDataCallback = NULL;
    fbdo->_timeoutCallback = NULL;
    fbdo->removeSession();

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
    if (Signer.config->internal.sessions.size() == 0)
    {
        if (Signer.config->internal.stream_task_handle)
            vTaskDelete(Signer.config->internal.stream_task_handle);

        Signer.config->internal.stream_task_handle = NULL;
    }
#endif
}

#if defined(ESP32) || defined(ENABLE_PICO_FREE_RTOS)
void FB_RTDB::runStreamTask(FirebaseData *fbdo, const char *taskName)
#else
void FB_RTDB::runStreamTask()
#endif
{
    if (!Signer.config || !Signer.config->internal.stream_loop_task_enable)
        return;

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))

    static FB_RTDB *_this = this;
    MB_String name = taskName;

    TaskFunction_t taskCode = [](void *param)
    {
        FirebaseConfig *config = (FirebaseConfig *)param;
        const TickType_t xDelay = config->internal.stream_task_delay_ms / portTICK_PERIOD_MS;
        for (;;)
        {
            if (!config->internal.stream_loop_task_enable)
                break;
            _this->mRunStream();
            vTaskDelay(xDelay);
        }

        config->internal.stream_task_handle = NULL;

        vTaskDelete(NULL);
    };

#if defined(ESP32)
    xTaskCreatePinnedToCore(taskCode, name.c_str(), Signer.config->internal.stream_task_stack_size,
                            Signer.config, Signer.config->internal.stream_task_priority,
                            &Signer.config->internal.stream_task_handle,
                            Signer.config->internal.stream_task_cpu_core);
#elif defined(MB_ARDUINO_PICO)

    /* Create a task, storing the handle. */
    xTaskCreate(taskCode, name.c_str(), Signer.config->internal.stream_task_stack_size, Signer.config,
                Signer.config->internal.stream_task_priority, &(Signer.config->internal.stream_task_handle));

    /* Define the core affinity mask such that this task can only run on core 0
     * and core 1. */
    UBaseType_t uxCoreAffinityMask = ((1 << 0) | (1 << 1));

    /* Set the core affinity mask for the task. */
    vTaskCoreAffinitySet(Signer.config->internal.stream_task_handle, uxCoreAffinityMask);

#endif

#else
    mRunStream();
#if defined(ESP8266)
    Signer.set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#endif
#endif
}

void FB_RTDB::mStopStreamLoopTask()
{
    if (Signer.config)
        Signer.config->internal.stream_loop_task_enable = false;
}

void FB_RTDB::mRunStream()
{
    FirebaseData *fbdo = nullptr;

    if (!Signer.config)
        return;

    if (Signer.isExpired())
        return;

    for (size_t id = 0; id < Signer.config->internal.sessions.size(); id++)
    {

        fbdo = addrTo<FirebaseData *>(Signer.config->internal.sessions[id]);

        if (fbdo)
        {
            if ((fbdo->_dataAvailableCallback || fbdo->_multiPathDataCallback || fbdo->_timeoutCallback))
            {
                if (Signer.isExpired())
                {
                    fbdo->session.rtdb.stream_tmo_Millis = millis();
                    fbdo->session.rtdb.data_tmo = false;
                    return;
                }

                readStream(fbdo);

                if (fbdo->streamTimeout() && fbdo->_timeoutCallback)
                    fbdo->sendStreamToCB(fbdo->session.response.code);
            }
        }
    }
}

void FB_RTDB::setMaxRetry(FirebaseData *fbdo, uint8_t num)
{
    fbdo->session.rtdb.max_retry = num;
}

void FB_RTDB::setBlobRef(FirebaseData *fbdo, int addr)
{
    if (fbdo->session.rtdb.blob && fbdo->session.rtdb.isBlobPtr)
    {
        delete fbdo->session.rtdb.blob;
        fbdo->session.rtdb.blob = nullptr;
    }

    fbdo->session.rtdb.isBlobPtr = addr == 0;
    addr > 0 ? fbdo->session.rtdb.blob = addrTo<MB_VECTOR<uint8_t> *>(addr) : fbdo->session.rtdb.blob = new MB_VECTOR<uint8_t>();
}

#if defined(ENABLE_ERROR_QUEUE)

void FB_RTDB::addQueueData(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (req->method == http_get || (!req->queue && (req->method == http_put ||
                                                    req->method == rtdb_set_nocontent ||
                                                    req->method == http_post ||
                                                    req->method == http_patch ||
                                                    req->method == rtdb_update_nocontent)))
    {
        QueueItem qItem;
        qItem.method = req->method;
        qItem.storageType = req->storageType;
        qItem.dataType = req->data.type;
        qItem.subType = req->data.value_subtype;
        qItem.path = req->path;
        qItem.address = req->data.address;
        qItem.filename = req->filename;
        qItem.payload = req->payload;
        qItem.etag = req->data.etag;
        qItem.async = req->async;
        qItem.blobSize = req->data.blobSize;
        fbdo->addQueue(&qItem);
    }
}

#if defined(ESP8266)
void FB_RTDB::runErrorQueueTask()
{
    if (!Signer.config)
        return;

    for (size_t id = 0; id < Signer.config->internal.queueSessions.size(); id++)
    {
        FirebaseData *fbdo = addrTo<FirebaseData *>(Signer.config->internal.queueSessions[id]);

        if (fbdo)
        {
            if (fbdo->_queueInfoCallback)
                processErrorQueue(fbdo, fbdo->_queueInfoCallback);
            else
                processErrorQueue(fbdo, NULL);
        }
    }

    Signer.set_scheduled_callback(std::bind(&FB_RTDB::runErrorQueueTask, this));
}
#endif

uint32_t FB_RTDB::getErrorQueueID(FirebaseData *fbdo)
{
    return fbdo->session.rtdb.queue_ID;
}

void FB_RTDB::processErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback)
{
    Utils::idle();

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

            Utils::idle();
            if (buildRequest(fbdo, item.method, MB_StringPtr(toAddr(item.path), mb_string_sub_type_mb_string),
                             MB_StringPtr(toAddr(item.payload), mb_string_sub_type_mb_string), item.dataType,
                             item.subType, item.method == http_get ? item.address.dout : item.address.din, item.address.query,
                             item.address.priority, MB_StringPtr(toAddr(item.etag), mb_string_sub_type_mb_string),
                             item.async, _NO_QUEUE, item.blobSize,
                             MB_StringPtr(toAddr(item.filename), mb_string_sub_type_mb_string),
                             (fb_esp_mem_storage_type)item.storageType))
            {
                fbdo->clearQueueItem(&item);
                fbdo->_qMan.remove(i);
            }
        }
    }
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

#if defined(ESP32) || defined(MB_ARDUINO_PICO) || defined(ESP8266)
#if defined(ESP32) || defined(MB_ARDUINO_PICO)
void FB_RTDB::beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback,
                                     size_t queueTaskStackSize)
#else
void FB_RTDB::beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback)
#endif
{
    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    if (callback)
        fbdo->_queueInfoCallback = callback;
    else
        fbdo->_queueInfoCallback = NULL;

    fbdo->addQueueSession();

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))

    static FB_RTDB *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
        FirebaseConfig *config = (FirebaseConfig *)param;
        const TickType_t xDelay = config->internal.queue_task_delay_ms / portTICK_PERIOD_MS;
        for (;;)
        {
            for (size_t i = 0; i < config->internal.queueSessions.size(); i++)
            {
                FirebaseData *_fbdo = addrTo<FirebaseData *>(config->internal.queueSessions[i]);

                if (_fbdo)
                {

                    if (_fbdo->_queueInfoCallback)
                        _this->processErrorQueue(_fbdo, _fbdo->_queueInfoCallback);
                    else
                        _this->processErrorQueue(_fbdo, NULL);
                }

                vTaskDelay(xDelay);
            }

            vTaskDelay(xDelay);
        }

        config->internal.queue_task_handle = NULL;
        vTaskDelete(NULL);
    };
#if defined(ESP32)
    xTaskCreatePinnedToCore(taskCode, fbdo->getTaskName(queueTaskStackSize, false),
                            Signer.config->internal.queue_task_stack_size,
                            Signer.config,
                            Signer.config->internal.queue_task_priority,
                            &Signer.config->internal.queue_task_handle,
                            Signer.config->internal.queue_task_cpu_core);

#elif defined(MB_ARDUINO_PICO)

    /* Create a task, storing the handle. */
    xTaskCreate(taskCode, fbdo->getTaskName(queueTaskStackSize, false), Signer.config->internal.queue_task_stack_size, Signer.config,
                Signer.config->internal.queue_task_priority, &(Signer.config->internal.queue_task_handle));

    /* Define the core affinity mask such that this task can only run on core 0
     * and core 1. */
    UBaseType_t uxCoreAffinityMask = ((1 << 0) | (1 << 1));

    /* Set the core affinity mask for the task. */
    vTaskCoreAffinitySet(Signer.config->internal.queue_task_handle, uxCoreAffinityMask);

#endif

#elif defined(ESP8266)
    Signer.set_scheduled_callback(std::bind(&FB_RTDB::runErrorQueueTask, this));
#endif
}
#endif

void FB_RTDB::endAutoRunErrorQueue(FirebaseData *fbdo)
{
    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    fbdo->_queueInfoCallback = NULL;
    fbdo->removeQueueSession();
#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
    if (Signer.config->internal.queueSessions.size() == 0)
    {
        if (Signer.config->internal.queue_task_handle)
            vTaskDelete(Signer.config->internal.queue_task_handle);
        Signer.config->internal.queue_task_handle = NULL;
    }
#endif
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

bool FB_RTDB::mSaveErrorQueue(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType)
{

    MB_String _filename = filename;

    int ret = Signer.mbfs->open(_filename, mbfs_type storageType, mb_fs_open_mode_write);

    if (ret < 0)
    {
        fbdo->session.response.code = ret;
        return false;
    }

    // Close file and open later.
    Signer.mbfs->close(mbfs_type storageType);

    if ((storageType == mem_storage_type_flash || storageType == mem_storage_type_sd) &&
        !Signer.mbfs->ready(mbfs_type storageType))
        return false;

    FirebaseJsonArray arr;

    // required for ESP32 core 2.0.x
    Signer.mbfs->open(_filename, mbfs_type storageType, mb_fs_open_mode_write);

    for (uint8_t i = 0; i < fbdo->_qMan.size(); i++)
    {
        if (!fbdo->_qMan._queueCollection)
            continue;

        QueueItem item = fbdo->_qMan._queueCollection->at(i);
        arr.clear();
        arr.add((uint8_t)item.dataType, (uint8_t)item.subType, (uint8_t)item.method,
                (uint8_t)item.storageType, (uint8_t)item.async);
        arr.add(item.address.din, item.address.dout, item.address.query,
                item.address.priority, item.blobSize);
        arr.add(item.path, item.payload, item.etag, item.filename);
        Signer.mbfs->write(mbfs_type storageType, (uint8_t *)arr.raw(), strlen(arr.raw()));
    }

    Signer.mbfs->close(mbfs_type storageType);
    return true;
}

bool FB_RTDB::mRestoreErrorQueue(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType)
{
    return openErrorQueue(fbdo, filename, storageType, 1) != 0;
}

uint8_t FB_RTDB::mErrorQueueCount(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType)
{
    return openErrorQueue(fbdo, filename, storageType, 0);
}

bool FB_RTDB::mDeleteStorageFile(MB_StringPtr filename, fb_esp_mem_storage_type storageType)
{
    if (!Signer.mbfs)
        return false;
    return Signer.mbfs->remove(MB_String(filename), mbfs_type storageType);
}

uint8_t FB_RTDB::openErrorQueue(FirebaseData *fbdo, MB_StringPtr filename,
                                fb_esp_mem_storage_type storageType, uint8_t mode)
{
    uint8_t count = 0;
    MB_String _filename = filename;

    int ret = Signer.mbfs->open(_filename, mbfs_type storageType, mb_fs_open_mode_read);

    if (ret < 0)
    {
        fbdo->session.response.code = ret;
        return 0;
    }

    if (!Signer.mbfs->available(mbfs_type storageType) || Signer.mbfs->size(mbfs_type storageType) < 4)
    {
        Signer.mbfs->close(mbfs_type storageType);
        return 0;
    }

    QueueItem item;

#if defined(MBFS_ESP32_SDFAT_ENABLED)

    if (storageType == mem_storage_type_flash)
    {
#if defined(MBFS_FLASH_FS)
        fs::File file = Signer.mbfs->getFlashFile();
        count = readQueueFile(fbdo, file, item, mode);
#endif
    }
    else if (storageType == mem_storage_type_sd)
    {
#if defined(MBFS_SD_FS) && defined(CARD_TYPE_SD)
        MBFS_SD_FILE file = Signer.mbfs->getSDFile();
        count = readQueueFileSdFat(fbdo, file, item, mode);
#endif
    }

#else

    if (storageType == mem_storage_type_flash)
    {
#if defined(MBFS_FLASH_FS)
        fs::File file = Signer.mbfs->getFlashFile();
        count = readQueueFile(fbdo, file, item, mode);
#endif
    }
    else if (storageType == mem_storage_type_sd)
    {
#if defined(MBFS_SD_FS) && defined(CARD_TYPE_SD)
        MBFS_SD_FILE file = Signer.mbfs->getSDFile();
        count = readQueueFile(fbdo, file, item, mode);
#endif
    }

#endif

    return count;
}

#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)
uint8_t FB_RTDB::readQueueFile(FirebaseData *fbdo, fs::File &file, QueueItem &item, uint8_t mode)
{

    uint8_t count = 0;
    FirebaseJsonArray arr;
    FirebaseJsonData result;

    while (file.available())
    {
        Utils::idle();
        if (arr.readFrom(file))
        {
            if (mode == 1)
            {
                for (size_t i = 0; i < arr.size(); i++)
                {
                    Utils::idle();
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
                            item.method = (fb_esp_request_method)result.to<int>();
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
                            item.path = result.to<MB_String>();
                            break;
                        case 11:
                            item.payload = result.to<MB_String>();
                            break;
                        case 12:
                            item.etag = result.to<MB_String>();
                            break;
                        case 13:
                            item.filename = result.to<MB_String>();
                            break;
                        default:
                            break;
                        }
                    }
                }
                if (!fbdo->_qMan._queueCollection)
                    fbdo->_qMan._queueCollection = new MB_VECTOR<struct QueueItem>();

                fbdo->_qMan._queueCollection->push_back(item);
            }
            count++;
        }
    }

    return count;
}
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED)
uint8_t FB_RTDB::readQueueFileSdFat(FirebaseData *fbdo, MBFS_SD_FILE &file, QueueItem &item, uint8_t mode)
{
    uint8_t count = 0;
    FirebaseJsonArray arr;
    FirebaseJsonData result;

    while (file.available())
    {
        Utils::idle();
        if (arr.readFrom(file))
        {
            if (mode == 1)
            {
                for (size_t i = 0; i < arr.size(); i++)
                {
                    Utils::idle();
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
                            item.method = (fb_esp_request_method)result.to<int>();
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
                            item.path = result.to<MB_String>();
                            break;
                        case 11:
                            item.payload = result.to<MB_String>();
                            break;
                        case 12:
                            item.etag = result.to<MB_String>();
                            break;
                        case 13:
                            item.filename = result.to<MB_String>();
                            break;
                        default:
                            break;
                        }
                    }
                }
                if (!fbdo->_qMan._queueCollection)
                    fbdo->_qMan._queueCollection = new MB_VECTOR<struct QueueItem>();

                fbdo->_qMan._queueCollection->push_back(item);
            }
            count++;
        }
    }

    return count;
}

#endif

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

#endif

bool FB_RTDB::mBackup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr nodePath,
                      MB_StringPtr fileName, RTDB_DownloadProgressCallback callback)
{
    struct fb_esp_rtdb_request_info_t req;
    req.filename = fileName;
    Utils::makePath(req.filename);
    req.path = nodePath;
    req.method = rtdb_backup;
    req.data.type = d_json;
    req.storageType = storageType;
    req.downloadCallback = callback;
    fbdo->session.rtdb.path = req.path;
    fbdo->session.rtdb.filename = req.filename;
    return handleRequest(fbdo, &req);
}

bool FB_RTDB::mRestore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr nodePath,
                       MB_StringPtr fileName, RTDB_UploadProgressCallback callback)
{
    struct fb_esp_rtdb_request_info_t req;
    req.filename = fileName;
    Utils::makePath(req.filename);
    req.path = nodePath;
    req.method = rtdb_restore;
    req.data.type = d_json;
    req.storageType = storageType;
    req.uploadCallback = callback;
    fbdo->session.rtdb.path = req.path;
    fbdo->session.rtdb.filename = req.filename;
    return handleRequest(fbdo, &req);
}

void FB_RTDB::setPtrValue(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (req->data.address.dout > 0 && req->method == http_get)
    {
        if (req->data.type != d_file && req->data.type != d_file_ota)
        {
            if (req->data.type == d_boolean)
                fbdo->restoreValue<bool>(req->data.address.dout);
            else if (req->data.type == d_integer)
                fbdo->restoreValue<int>(req->data.address.dout);
            else if (req->data.type == d_float)
                fbdo->restoreValue<float>(req->data.address.dout);
            else if (req->data.type == d_double)
                fbdo->restoreValue<double>(req->data.address.dout);
            else if (req->data.type == d_string)
            {
                if (req->data.value_subtype == mb_string_sub_type_arduino_string)
                    fbdo->restoreValue<String>(req->data.address.dout);
#if !defined(__AVR__)
                else if (req->data.value_subtype == mb_string_sub_type_std_string)
                    fbdo->restoreValue<std::string>(req->data.address.dout);
#endif
                else if (req->data.value_subtype == mb_string_sub_type_mb_string)
                    fbdo->restoreValue<MB_String>(req->data.address.dout);
                else if (req->data.value_subtype == mb_string_sub_type_chars)
                    fbdo->restoreCString(req->data.address.dout);
            }
            else if (req->data.type == d_json)
                fbdo->restoreValue<FirebaseJson>(req->data.address.dout);
            else if (req->data.type == d_array)
                fbdo->restoreValue<FirebaseJsonArray>(req->data.address.dout);
        }
    }
}

bool FB_RTDB::processRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{

    Utils::idle();

    if (!preRequestCheck(fbdo, req))
        return false;

    if (req->method != http_get)
    {
        if (!fbdo->reconnect())
        {
#if defined(ENABLE_ERROR_QUEUE)
            addQueueData(fbdo, req);
#endif
            return false;
        }
    }

    bool ret = false;

    if (req->data.type == d_file && req->method == http_get)
    {
        fbdo->session.rtdb.filename = req->filename;

        int p1 = 0;
        MB_String folder = fb_esp_pgm_str_1; // "/"

        if (StringHelper::find(fbdo->session.rtdb.filename, fb_esp_pgm_str_1 /* "/" */, true, 0, p1) && p1 != 0)
            fbdo->session.rtdb.filename.substr(folder, p1 - 1);

        if (fbdo->session.rtdb.storage_type == mem_storage_type_sd)
        {
#if defined(MBFS_SD_FS) && defined(CARD_TYPE_SD)
            if (!MBFS_SD_FS.exists(folder.c_str()))
                Utils::createDirs(folder, (fb_esp_mem_storage_type)fbdo->session.rtdb.storage_type);
#endif
        }

        if (openFile(fbdo, req, mb_fs_open_mode_write) < 0)
            return false;

        folder.clear();
    }

    fbdo->session.rtdb.queue_ID = 0;

    uint8_t errCount = 0;
    uint8_t maxRetry = fbdo->session.rtdb.max_retry > 0 ? fbdo->session.rtdb.max_retry : 1;

    for (int i = 0; i < maxRetry; i++)
    {
        ret = handleRequest(fbdo, req);
        setPtrValue(fbdo, req);
        if (ret)
            break;

        if (fbdo->session.rtdb.max_retry > 0)
            if (!ret && connectionError(fbdo))
                errCount++;
    }

    if (!ret && errCount == maxRetry && fbdo->_qMan._maxQueue > 0)
    {
#if defined(ENABLE_ERROR_QUEUE)
        addQueueData(fbdo, req);
#endif
        if (!req->queue && req->method != http_get)
            return ret;
    }

    if (ret)
    {
        if (Signer.config->rtdb.data_type_stricted && req->method == http_get && req->data.type != d_any)
        {
            if (req->data.type == d_integer ||
                req->data.type == d_float ||
                req->data.type == d_double)
                ret = fbdo->session.rtdb.resp_data_type == d_integer ||
                      fbdo->session.rtdb.resp_data_type == d_float ||
                      fbdo->session.rtdb.resp_data_type == d_double;
            else if (req->data.type == d_json)
                ret = fbdo->session.rtdb.resp_data_type == d_json ||
                      fbdo->session.rtdb.resp_data_type == d_null;
            else if (req->data.type == d_array)
                ret = fbdo->session.rtdb.resp_data_type == d_array ||
                      fbdo->session.rtdb.resp_data_type == d_null;
            else if (req->data.type != d_file && req->data.type != d_file_ota)
                ret = fbdo->session.rtdb.resp_data_type == req->data.type;
        }
    }
    return ret;
}

#if defined(ESP32) || defined(MB_ARDUINO_PICO)
void FB_RTDB::allowMultipleRequests(bool enable)
{
    if (!Signer.config)
        return;
    Signer.config->internal.fb_multiple_requests = enable;
}
#endif

void FB_RTDB::rescon(FirebaseData *fbdo, const char *host, fb_esp_rtdb_request_info_t *req)
{
    fbdo->_responseCallback = NULL;

    if (req->method == rtdb_stream)
        fbdo->session.rtdb.stream_path_changed = strcmp(req->path.c_str(), fbdo->session.rtdb.stream_path.c_str()) != 0
                                                     ? true
                                                     : false;

    if (fbdo->session.cert_updated ||
        !fbdo->session.connected ||
        millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.rtdb.stream_path_changed ||
        (req->method == rtdb_stream && fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream) ||
        (req->method != rtdb_stream && fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream) ||
        strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->session.host = host;
    fbdo->session.con_mode = req->method == rtdb_stream ? fb_esp_con_mode_rtdb_stream : fb_esp_con_mode_rtdb;

    if (fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream)
        fbdo->session.rtdb.stream_resume_millis = 0;
}

bool FB_RTDB::handleRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    Utils::idle();

    if (!preRequestCheck(fbdo, req))
        return false;

#if defined(MB_ARDUINO_PICO)
    if (!Utils::waitIdle(fbdo->session.response.code, Signer.config))
        return false;
#endif

    if (!fbdo->session.connected)
        fbdo->session.rtdb.async_count = 0;

    if ((fbdo->session.rtdb.async && !req->async) ||
        fbdo->session.rtdb.async_count > Signer.config->async_close_session_max_request)
    {
        fbdo->session.rtdb.async_count = 0;
        fbdo->closeSession();
    }

    fbdo->session.rtdb.queue_ID = 0;
    if (req->data.etag.length() > 0)
        fbdo->session.rtdb.req_etag = req->data.etag;
    if (req->data.address.priority > 0)
    {
        float *pri = addrTo<float *>(req->data.address.priority);
        fbdo->session.rtdb.priority = *pri;
    }

    fbdo->session.rtdb.storage_type = req->storageType;

    fbdo->session.rtdb.redirect_url.clear();
    fbdo->session.rtdb.req_method = req->method;
    fbdo->session.rtdb.req_data_type = req->data.type;
    fbdo->session.rtdb.data_mismatch = false;
    fbdo->session.rtdb.async = req->async;
    if (req->async)
        fbdo->session.rtdb.async_count++;

    if (sendRequest(fbdo, req))
    {
        fbdo->session.connected = true;

        if (req->method == rtdb_stream)
        {
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();
                return false;
            }
        }
        else if (req->method == rtdb_get_rules ||
                 req->method == rtdb_backup ||
                 ((req->data.type == d_file || req->data.type == d_file_ota) && req->method == http_get))
        {
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();
                if (req->downloadCallback)
                {
                    RTDB_DownloadStatusInfo in;
                    makeDownloadStatus(in, req->filename, req->path, fb_esp_rtdb_download_status_error,
                                       0, 0, 0, fbdo->errorReason());
                    sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                }

                return false;
            }
            else if (req->downloadCallback)
            {
                RTDB_DownloadStatusInfo in;
                makeDownloadStatus(in, req->filename, req->path, fb_esp_rtdb_download_status_complete,
                                   100, req->fileSize, 0, "");
                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
            }
        }
        else if (req->method == rtdb_set_rules ||
                 req->method == rtdb_restore ||
                 (req->data.type == d_file && req->method == rtdb_set_nocontent))
        {
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();

                if (req->uploadCallback)
                {
                    RTDB_UploadStatusInfo in;
                    makeUploadStatus(in, req->filename, req->path, fb_esp_rtdb_upload_status_error,
                                     0, 0, 0, fbdo->errorReason());
                    sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
                }

                return false;
            }
            else if (req->uploadCallback)
            {
                RTDB_UploadStatusInfo in;
                makeUploadStatus(in,
                                 req->filename,
                                 req->path,
                                 fb_esp_rtdb_upload_status_complete,
                                 100,
                                 req->fileSize,
                                 0,
                                 "");
                sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
            }
        }
        else
        {
            fbdo->session.rtdb.path = req->path;
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();
                return false;
            }
            fbdo->session.rtdb.data_available = fbdo->session.rtdb.raw.length() > 0;
            if (fbdo->session.rtdb.blob)
                fbdo->session.rtdb.data_available |= fbdo->session.rtdb.blob->size() > 0;
        }
    }
    else
    {
        if (fbdo->session.response.code != FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED)
            fbdo->session.connected = false;
        return false;
    }

    return true;
}

void FB_RTDB::reportUploadProgress(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req, size_t readBytes)
{
    if (!req)
        return;

    if (req->fileSize == 0)
        return;

    int p = (float)readBytes / req->fileSize * 100;

    if (readBytes == 0)
        fbdo->tcpClient.dataStart = millis();

    if (req->progress != p && (p == 0 || p == 100 || req->progress + ESP_REPORT_PROGRESS_INTERVAL <= p))
    {
        fbdo->tcpClient.dataTime = millis() - fbdo->tcpClient.dataStart;

        req->progress = p;

        fbdo->session.rtdb.cbUploadInfo.status = fb_esp_rtdb_upload_status_upload;
        if (req->uploadCallback)
        {
            RTDB_UploadStatusInfo in;
            makeUploadStatus(in,
                             req->filename,
                             req->path,
                             fb_esp_rtdb_upload_status_upload,
                             p,
                             0,
                             fbdo->tcpClient.dataTime,
                             "");
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }
    }
}

void FB_RTDB::reportDownloadProgress(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req, size_t readBytes)
{
    if (!req)
        return;

    if (req->fileSize == 0)
        return;

    int p = (float)readBytes / req->fileSize * 100;

    if (readBytes == 0)
        fbdo->tcpClient.dataStart = millis();

    if (req->progress != p && (p == 0 || p == 100 || req->progress + ESP_REPORT_PROGRESS_INTERVAL <= p))
    {
        req->progress = p;

        fbdo->tcpClient.dataTime = millis() - fbdo->tcpClient.dataStart;

        fbdo->session.rtdb.cbDownloadInfo.status = fb_esp_rtdb_download_status_download;
        if (req->downloadCallback)
        {
            RTDB_DownloadStatusInfo in;
            makeDownloadStatus(in, req->filename,
                               req->path,
                               fb_esp_rtdb_download_status_download,
                               p, req->fileSize,
                               fbdo->tcpClient.dataTime,
                               "");
            sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
        }
    }
}

void FB_RTDB::makeUploadStatus(RTDB_UploadStatusInfo &info, const MB_String &local, const MB_String &remote,
                               fb_esp_rtdb_upload_status status, size_t progress, size_t size, int elapsedTime,
                               const MB_String &msg)
{
    info.localFileName = local;
    info.remotePath = remote;
    info.status = status;
    info.size = size;
    info.progress = progress;
    info.elapsedTime = elapsedTime;
    info.errorMsg = msg;
}

void FB_RTDB::sendUploadCallback(FirebaseData *fbdo, RTDB_UploadStatusInfo &in, RTDB_UploadProgressCallback cb,
                                 RTDB_UploadStatusInfo *out)
{
    fbdo->session.rtdb.cbUploadInfo = in;
    if (cb)
        cb(fbdo->session.rtdb.cbUploadInfo);
    if (out)
        out = &fbdo->session.rtdb.cbUploadInfo;
}

void FB_RTDB::makeDownloadStatus(RTDB_DownloadStatusInfo &info, const MB_String &local, const MB_String &remote,
                                 fb_esp_rtdb_download_status status, size_t progress, size_t size,
                                 int elapsedTime, const MB_String &msg)
{
    info.localFileName = local;
    info.remotePath = remote;
    info.status = status;
    info.size = size;
    info.progress = progress;
    info.elapsedTime = elapsedTime;
    info.errorMsg = msg;
}

void FB_RTDB::sendDownloadCallback(FirebaseData *fbdo, RTDB_DownloadStatusInfo &in,
                                   RTDB_DownloadProgressCallback cb, RTDB_DownloadStatusInfo *out)
{
    fbdo->session.rtdb.cbDownloadInfo = in;
    if (cb)
        cb(fbdo->session.rtdb.cbDownloadInfo);
    if (out)
        out = &fbdo->session.rtdb.cbDownloadInfo;
}

int FB_RTDB::preRequestCheck(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    fbdo->session.error.clear();
    int code = 1;

    // don't check the network connection here which it ignores the
    // error queues

    if (!Signer.config)
        code = FIREBASE_ERROR_UNINITIALIZED;
    else if (fbdo->session.rtdb.pause)
        code = 0;
    else if (!fbdo->tokenReady())
        code = FIREBASE_ERROR_TOKEN_NOT_READY;
    else if (req->path.length() == 0 ||
             (Signer.config->database_url.length() == 0 && Signer.config->host.length() == 0) ||
             (strlen(Signer.getToken()) == 0 && !Signer.config->signer.test_mode))
        code = FIREBASE_ERROR_MISSING_CREDENTIALS;
    else if (req->method != rtdb_stream &&
             (req->method == http_put || req->method == http_post || req->method == http_patch ||
              req->method == rtdb_update_nocontent || req->task_type == fb_esp_rtdb_task_store_rules) &&
             req->payload.length() == 0 && req->data.type != d_string && req->data.type != d_json &&
             req->data.type != d_array && req->data.type != d_blob && req->data.type != d_file_ota)
        code = FIREBASE_ERROR_MISSING_DATA;
    else if (fbdo->session.long_running_task > 0)
        code = FIREBASE_ERROR_LONG_RUNNING_TASK;

    if (code < 0)
        fbdo->session.response.code = code;

    return code;
}

bool FB_RTDB::sendRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    if (fbdo->tcpClient.reserved)
        return false;

    fbdo->session.http_code = 0;

    char *buf = nullptr;
    int len = 0;
    size_t toRead = 0;
    bool ret = false;

    rescon(fbdo, Signer.config->database_url.c_str(), req);

    if (req->method == rtdb_stream)
    {
        fbdo->session.rtdb.stream_path.clear();
        Utils::makePath(req->path);
        fbdo->session.rtdb.stream_path += req->path;
    }
    else
    {
        fbdo->session.rtdb.resp_etag.clear();

        if (req->method != rtdb_backup && req->method != rtdb_restore)
        {
            fbdo->session.rtdb.path.clear();
            Utils::makePath(req->path);
            fbdo->session.rtdb.path += req->path;
        }

        fbdo->session.rtdb.data_tmo = false;
    }

    fbdo->session.max_payload_length = 0;

    fbdo->tcpClient.begin(Signer.config->database_url.c_str(), FIREBASE_PORT, &fbdo->session.response.code);

    if (req->task_type == fb_esp_rtdb_task_upload_rules)
    {
        int sz = openFile(fbdo, req, mb_fs_open_mode_read);
        if (sz < 0)
            return sz;
        req->fileSize = sz;
        len = sz;
    }

    // Prepare request header
    if (req->method != rtdb_backup &&
        req->method != rtdb_restore &&
        req->data.type != d_file &&
        req->data.type != d_file_ota)
        ret = sendRequestHeader(fbdo, req);
    else
    {

        if (req->method == rtdb_backup || req->method == rtdb_restore)
        {
            int sz = openFile(fbdo, req, mb_fs_open_mode_undefined);
            if (sz < 0)
            {
                fbdo->session.response.code = sz;
                return false;
            }

            req->fileSize = sz;
            len = sz;
        }
        else if (req->method == rtdb_set_nocontent || req->method == http_post)
        {
            if (req->data.type == d_file)
            {
                int sz = openFile(fbdo, req, mb_fs_open_mode_read);
                if (sz < 0)
                {
                    fbdo->session.response.code = sz;
                    return false;
                }
                req->fileSize = sz;
            }
        }

        ret = sendRequestHeader(fbdo, req);
    }

    if (req->method == rtdb_get_nocontent ||
        req->method == rtdb_update_nocontent ||
        (req->method == rtdb_set_nocontent && (req->data.type == d_blob || req->data.type == d_file)))
        fbdo->session.rtdb.no_content_req = true;

    if (req->data.type == d_blob)
    {
        if (fbdo->session.rtdb.blob)
            MB_VECTOR<uint8_t>().swap(*fbdo->session.rtdb.blob);
        else
        {
            fbdo->session.rtdb.isBlobPtr = true;
            fbdo->session.rtdb.blob = new MB_VECTOR<uint8_t>();
        }
    }

    if (!ret)
    {
        fbdo->closeSession();
        return false;
    }

    int bufSize = Utils::getUploadBufSize(Signer.config, fb_esp_con_mode_rtdb);
    unsigned long ms = millis();
    fbdo->tcpClient.dataTime = 0;

    // Send payload
    if (req->data.address.din > 0 && req->data.type == d_json)
    {
        FirebaseJson *json = addrTo<FirebaseJson *>(req->data.address.din);
        if (json)
            fbdo->tcpClient.send(json->raw());
    }
    else if (req->payload.length() > 0 || (req->data.type == d_array && req->data.address.din > 0))
    {
        if (req->pre_payload.length() > 0)
        {
            fbdo->tcpClient.send(req->pre_payload.c_str());
            if (fbdo->session.response.code < 0)
                return false;
        }

        if (req->data.type == d_array && req->data.address.din > 0)
        {
            FirebaseJsonArray *arr = addrTo<FirebaseJsonArray *>(req->data.address.din);
            if (arr)
                fbdo->tcpClient.send(arr->raw());

            if (fbdo->session.response.code < 0)
                return false;
        }
        else if (req->payload.length() > 0)
        {
            fbdo->tcpClient.send(req->payload.c_str());
            if (fbdo->session.response.code < 0)
                return false;
        }

        if (req->post_payload.length() > 0)
            fbdo->tcpClient.send(req->post_payload.c_str());
    }
    else if (req->data.address.din > 0 && req->data.blobSize > 0)
    {
        // input blob data is uint8_t array
        uint8_t *blob = addrTo<uint8_t *>(req->data.address.din);
        if (blob)
        {
            // send first payload (file base64 signature)
            fbdo->tcpClient.send(pgm2Str(fb_esp_rtdb_pgm_str_7 /* "\"blob,base64," */));
            if (fbdo->session.response.code > 0)
            {
                // convert blob to base64 string before sending
                if (Base64Helper::encodeToClient(fbdo->tcpClient.client, Signer.mbfs, bufSize, blob, req->data.blobSize))
                    fbdo->tcpClient.send(pgm2Str(fb_esp_pgm_str_4) /* "\"" */); // send last payload
            }
        }
    }
    else if (req->task_type == fb_esp_rtdb_task_upload_rules ||
             req->method == rtdb_restore ||
             (req->data.type == d_file && (req->method == rtdb_set_nocontent || req->method == http_post)))
    {

        if (req->uploadCallback)
        {
            RTDB_UploadStatusInfo in;
            makeUploadStatus(in,
                             Signer.mbfs->name(mbfs_type req->storageType),
                             req->path, fb_esp_rtdb_upload_status_init,
                             0,
                             req->fileSize,
                             0,
                             "");
            sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        }

        int readLen = 0;

        if (req->data.type == d_file && (req->method == rtdb_set_nocontent || req->method == http_post))
        {
            MB_String s = fb_esp_rtdb_pgm_str_8; // "\"file,base64,"

            // make base64 signature with pad length encoded for later callulated decoded data size
            // used in download progress callback.
            if (Base64Helper::getBase64Padding(req->fileSize) == 1)
                s[1] = 'F'; // 1 padding -> "File,base64,
            else if (Base64Helper::getBase64Padding(req->fileSize) == 2)
                s[2] = 'I'; // 2 paddings -> "fIle,base64,

            // send first payload (file base64 signature)
            fbdo->tcpClient.send(s.c_str());

            if (fbdo->session.response.code < 0)
                return false;

            // read file and convert to base64 string before sending
            if (encodeFileToClient(fbdo, bufSize, req->filename, (fb_esp_mem_storage_type)fbdo->session.rtdb.storage_type, req))
                fbdo->tcpClient.send(pgm2Str(fb_esp_pgm_str_4) /* "\"" */); // send last payload

            if (fbdo->session.response.code < 0)
                return false;
        }
        else
        {
            // required for ESP32 core 2.0.x
            MB_String filenme = Signer.mbfs->name(mbfs_type req->storageType);
            Signer.mbfs->close(mbfs_type req->storageType);
            Signer.mbfs->open(filenme, mbfs_type req->storageType, mb_fs_open_mode_read);

            while (len)
            {
                toRead = len;
                if ((int)toRead > bufSize)
                    toRead = bufSize - 1;

                buf = MemoryHelper::createBuffer<char *>(Signer.mbfs, bufSize, false);
                int read = Signer.mbfs->read(mbfs_type req->storageType, (uint8_t *)buf, toRead);
                readLen += read;

                reportUploadProgress(fbdo, req, readLen);

                if (read != (int)toRead)
                    break;

                buf[toRead] = '\0';

                fbdo->tcpClient.send(buf);

                MemoryHelper::freeBuffer(Signer.mbfs, buf);

                if (fbdo->session.response.code < 0)
                    return false;

                len -= toRead;

                if (len <= 0)
                    break;
            }

            reportUploadProgress(fbdo, req, req->fileSize);
        }

        Signer.mbfs->close(mbfs_type req->storageType);
    }

    fbdo->tcpClient.dataTime = millis() - ms;

    if (fbdo->session.response.code < 0)
        return false;

    fbdo->session.connected = fbdo->session.response.code > 0;

    return true;
}

bool FB_RTDB::encodeFileToClient(FirebaseData *fbdo, size_t bufSize, const MB_String &filePath,
                                 fb_esp_mem_storage_type storageType, struct fb_esp_rtdb_request_info_t *req)
{

    MB_String filenme = Signer.mbfs->name(mbfs_type req->storageType);
    Signer.mbfs->close(mbfs_type req->storageType);
    int size = Signer.mbfs->open(filenme, mbfs_type req->storageType, mb_fs_open_mode_read);
    if (!size)
        return false;

    int total = 0;
    req->fileSize = size;

    reportUploadProgress(fbdo, req, total);

    fb_esp_base64_io_t<uint8_t> out;
    out.outC = fbdo->tcpClient.client;
    out.bufLen = bufSize;
    uint8_t *outBuf = MemoryHelper::createBuffer<uint8_t *>(Signer.mbfs, out.bufLen);
    out.outT = outBuf;
    unsigned char *base64EncBuf = Base64Helper::creatBase64EncBuffer(Signer.mbfs, false);
    uint8_t *data = MemoryHelper::createBuffer<uint8_t *>(Signer.mbfs, 3);

    while (Signer.mbfs->available(mbfs_type storageType))
    {
        // read 3 bytes from stream
        int read = Signer.mbfs->read(mbfs_type storageType, data, 3);
        if (read)
        {
            if (!Base64Helper::encode<uint8_t>(Signer.mbfs, base64EncBuf, data, read, out, read != 3 /* write remaining */))
                break;
            total += read;
            reportUploadProgress(fbdo, req, total);
        }
        else
            break;
    }

    // remaing data to wrire? write it
    if (size == total && out.bufLen > 0)
        Base64Helper::writeOutput(Signer.mbfs, out);

    MemoryHelper::freeBuffer(Signer.mbfs, data);
    MemoryHelper::freeBuffer(Signer.mbfs, base64EncBuf);
    MemoryHelper::freeBuffer(Signer.mbfs, outBuf);

    return size == total;
}

bool FB_RTDB::waitResponse(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req)
{
#if defined(ESP32) || defined(MB_ARDUINO_PICO)

    // if currently perform stream payload handling process, skip it.
    if (Signer.config->internal.fb_processing && fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
        return true;

    // set the blocking flag
    Signer.config->internal.fb_processing = true;
    bool ret = handleResponse(fbdo, req);
    // reset the blocking flag
    Signer.config->internal.fb_processing = false;

    return ret;
#else
    return handleResponse(fbdo, req);
#endif
}

int FB_RTDB::openFile(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, mb_fs_open_mode mode, bool closeSession)
{
    int sz = 0;

    if (req->method == rtdb_backup)
        sz = Signer.mbfs->open(req->filename, mbfs_type req->storageType, mb_fs_open_mode_write);
    else if (req->method == rtdb_restore)
        sz = Signer.mbfs->open(req->filename, mbfs_type req->storageType, mb_fs_open_mode_read);
    else
        sz = Signer.mbfs->open(req->filename, mbfs_type req->storageType, mode);

    if (sz < 0)
    {
        fbdo->session.response.code = sz;
        if (closeSession)
            fbdo->closeSession();
        return sz;
    }

    return sz;
}

bool FB_RTDB::handleResponse(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req)
{

    if (fbdo->session.rtdb.pause)
        return true;

    if (!fbdo->session.connected || !fbdo->tcpClient.connected())
    {
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

        if (fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);

        return false;
    }

    MB_String payload;
    struct server_response_data_t response;
    struct fb_esp_tcp_response_handler_t tcpHandler;

    int pChunkSize = 1024;

    HttpHelper::initTCPSession(fbdo->session);
    HttpHelper::intTCPHandler(fbdo->tcpClient.client, tcpHandler, 2048 + strlen_P(fb_esp_rtdb_pgm_str_8 /* "\"file,base64," */),
                              fbdo->session.resp_size, &payload, req->data.type == d_file_ota);

waits:

    if (fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream)
    {
        if (fbdo->session.rtdb.async)
            return fbdo->tcpClient.connected();
        else if (!fbdo->waitResponse(tcpHandler))
            return false;
    }

    if ((req->task_type == fb_esp_rtdb_task_download_rules || req->method == rtdb_backup) &&
        !fbdo->prepareDownload(req->filename, (fb_esp_mem_storage_type)req->storageType))
        return false;

    bool complete = false;

    // data available to read?
    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */)
    {
        if (fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;

        // still session connected?
        if (!fbdo->isConnected(tcpHandler.dataTime))
        {
            fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
            if (fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
                fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
            return false;
        }

        // read available responses (only http headers or first line of stream payload)
        if (!fbdo->readResponse(nullptr, tcpHandler, response))
            break;

        // header buffer contains complete http headers? handle the headers
        if (tcpHandler.headerEnded && tcpHandler.header.length() > 0)
        {
            // clear header buffer
            tcpHandler.header.clear();

            // redirect required?
            int redirect = handleRedirect(fbdo, req, tcpHandler, response);
            if (redirect == -1)
                break;
            else if (redirect == 1)
                goto waits;

            // stream response header?
            if (response.contentType.find(pgm2Str(fb_esp_rtdb_pgm_str_9 /* "text/event-stream" */)) != MB_String::npos)
                fbdo->session.rtdb.new_stream = false; // reset new stream connection status

            // check connection types
            fbdo->session.rtdb.http_resp_conn_type = StringHelper::compare(response.connection,
                                                                           0, fb_esp_pgm_str_15 /* "keep-alive" */)
                                                         ? fb_esp_http_connection_type_keep_alive
                                                         : fb_esp_http_connection_type_close;

            // store download size for file function
            if (req->method == rtdb_backup)
                fbdo->session.rtdb.file_size = response.contentLen;

            if (response.httpCode >= 400)
            {
                fbdo->session.error = response.fbError;
                fbdo->session.response.code = response.httpCode;
            }

            fbdo->session.rtdb.resp_etag = response.etag;
        }
        // not http header received, stream payload received?
        else if (!tcpHandler.isHeader && tcpHandler.header.length() > 0)
        {
            // keep it as a first payload
            payload += tcpHandler.header;

            // clear header buffer
            tcpHandler.header.clear();
        }
        // payloads available? handle the rest of stream payloads or http response payloads
        else if (tcpHandler.pChunkIdx > 0)
        {
            bool downloadRequired = req->task_type == fb_esp_rtdb_task_download_rules ||
                                    req->method == rtdb_backup ||
                                    ((req->data.type == d_file || tcpHandler.downloadOTA) && req->method == http_get);

            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK && downloadRequired)
            {
                tcpHandler.dataTime = millis();

                req->fileSize = response.contentLen;
                tcpHandler.error.code = 0;

                int bufLen = tcpHandler.chunkBufSize;
                uint8_t *buf = MemoryHelper::creatDownloadBuffer<uint8_t *>(Signer.mbfs, bufLen, false);

                int stage = 0;
                bool downloaded = false;

                tcpHandler.chunkBufSize = tcpHandler.pChunkIdx == 1 ? pChunkSize +
                                                                          strlen_P(fb_esp_rtdb_pgm_str_8 /* "\"file,base64," */)
                                                                    : pChunkSize;

                while (fbdo->processDownload(req->filename, (fb_esp_mem_storage_type)req->storageType, buf, bufLen,
                                             tcpHandler, response, stage, tcpHandler.downloadOTA))
                {
                    downloaded = true;
                    if (stage)
                    {
                        // based64 encoded string of file data
                        if (tcpHandler.pChunkIdx == 1 && tcpHandler.isBase64File)
                        {
                            req->fileSize = tcpHandler.payloadLen;

                            if (req->downloadCallback)
                            {
                                RTDB_DownloadStatusInfo in;
                                makeDownloadStatus(in, Signer.mbfs->name(mbfs_type req->storageType), req->path, fb_esp_rtdb_download_status_init,
                                                   0, tcpHandler.decodedPayloadLen, 0, "");
                                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                            }
                        }

                        reportDownloadProgress(fbdo, req, tcpHandler.payloadRead);
                    }

                    tcpHandler.chunkBufSize = tcpHandler.pChunkIdx == 1 ? pChunkSize +
                                                                              strlen_P(fb_esp_rtdb_pgm_str_8 /* "\"file,base64," */)
                                                                        : pChunkSize;
                }

                MemoryHelper::freeBuffer(Signer.mbfs, buf);

                if (downloaded)
                    reportDownloadProgress(fbdo, req, tcpHandler.payloadRead);

                if (tcpHandler.downloadOTA && !endDownloadOTA(fbdo, req, tcpHandler, response))
                    goto skip;

                if (tcpHandler.error.code != 0)
                    fbdo->session.response.code = tcpHandler.error.code;

                goto skip;
            }
            else
            {

                MB_String pChunk;
                tcpHandler.chunkBufSize = tcpHandler.pChunkIdx == 1 ? pChunkSize +
                                                                          strlen_P(fb_esp_rtdb_pgm_str_8 /* "\"file,base64," */)
                                                                    : pChunkSize;
                fbdo->readPayload(&pChunk, tcpHandler, response);

                // Last chunk?
                if (Utils::isChunkComplete(&tcpHandler, &response, complete))
                    goto skip;

                if (tcpHandler.bufferAvailable > 0 && pChunk.length() > 0)
                {

                    Utils::idle();
                    payload += pChunk;

                    // early parsing currently available http response for data types, event types, and event data
                    // which these information will be used for download task
                    if (!parseTCPResponse(fbdo, req, tcpHandler, response))
                        goto skip;

                    // stream data complete?
                    int ofs = payload[payload.length() - 1] == '\r' || payload[payload.length() - 1] == '\n' ? 3 : 2;

                    __attribute__((unused)) bool streamDataComplete = payload.length() > 2 &&
                                                                      payload[payload.length() - ofs] == '"' &&
                                                                      payload[payload.length() - ofs + 1] == '}';

                    if (response.dataType == d_file)
                    {
#if defined(MBFS_FLASH_FS)

                        // In case file is available in stream with no download request,
                        // we store this file data to temp file (/fb_bin_0.tmp) that user can read from stream data

                        Signer.mbfs->remove(pgm2Str(fb_esp_rtdb_pgm_str_10 /* "/fb_bin_0.tmp" */), mb_fs_mem_storage_type_flash);
                        int sz = Signer.mbfs->open(pgm2Str(fb_esp_rtdb_pgm_str_10 /* "/fb_bin_0.tmp" */),
                                                   mb_fs_mem_storage_type_flash, mb_fs_open_mode_append);
                        if (sz < 0)
                            fbdo->session.response.code = sz;

                        // we keep the first part of JSON for parsing later with parsePayload()
                        MB_String stream = payload.substr(0, response.payloadOfs);
                        // append " and } to make a valid JSON
                        stream += fb_esp_pgm_str_4; // "\""
                        stream += fb_esp_pgm_str_11; // "}"

                        payload.erase(0, response.payloadOfs);

                        // stream event data complete
                        if (streamDataComplete)
                        {
                            trimEndJson(payload);
                            Base64Helper::decodeToFile(Signer.mbfs, payload.c_str(),
                                                       payload.length(), mb_fs_mem_storage_type_flash);
                        }
                        // stream event data is not complete, try to read incoming data as a chunk (multiples of 4 bytes length)
                        // base64 decoded and store in temp file
                        else
                        {
                            unsigned long time = millis();
                            while (!streamDataComplete && millis() - time < 1000 /* something wrongs if time out */)
                                // keep reading until the end of JSON tag found
                                readBase64FileChunk(fbdo, payload, tcpHandler, response, pChunkSize, streamDataComplete);
                        }

                        // Save the payload
                        payload = stream;
                        payload.shrink_to_fit();
                        Signer.mbfs->close(mb_fs_mem_storage_type_flash);
                        goto skip;
#endif
                    }
                }

                if (Utils::isResponseComplete(&tcpHandler, &response, complete, fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream))
                    goto skip;
            }
        }

        tcpHandler.dataTime = millis();
    }

skip:

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    endDownload(fbdo, req, tcpHandler, response);

    parsePayload(fbdo, req, response, payload);

    handleNoContent(fbdo, response);

    return fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK ||
           (fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream && fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_UNDEFINED);
}

void FB_RTDB::trimEndJson(MB_String &payload)
{
    size_t p = 0;
    while (p < payload.length() && payload[payload.length() - p] != '}') // look for }
        p++;
    p++; // forward for "
    payload.erase(payload.length() - p, p);
}

void FB_RTDB::readBase64FileChunk(FirebaseData *fbdo, MB_String &payload, struct fb_esp_tcp_response_handler_t &tcpHandler,
                                  struct server_response_data_t &response, int chunkSize, bool &streamDataComplete)
{
    chunkSize = chunkSize * 2;
    MB_String pChunk;
    fbdo->readPayload(&pChunk, tcpHandler, response);
    streamDataComplete = pChunk.length() > 2 && pChunk[pChunk.length() - 3] == '"' && pChunk[pChunk.length() - 2] == '}';

    if (pChunk.length() > 0)
    {
        if (streamDataComplete)
        {
            payload += pChunk;
            trimEndJson(payload);
            Base64Helper::decodeToFile(Signer.mbfs, payload.c_str(), payload.length(), mb_fs_mem_storage_type_flash);
        }
        else
        {
            int total = payload.length() + pChunk.length();
            if (total > chunkSize)
            {
                int toLen = chunkSize - payload.length();
                payload += pChunk.substr(0, toLen);
                Base64Helper::decodeToFile(Signer.mbfs, payload.c_str(), payload.length(), mb_fs_mem_storage_type_flash);
                payload = pChunk.substr(toLen, total - chunkSize);
            }
            else
                payload += pChunk;
        }
    }
}

void FB_RTDB::handleNoContent(FirebaseData *fbdo, struct server_response_data_t &response)
{
    if (fbdo->session.rtdb.no_content_req || response.noContent)
    {

        if (StringHelper::compare(fbdo->session.rtdb.resp_etag, 0, fb_esp_rtdb_pgm_str_11 /* "null_etag" */) && response.noContent)
        {
            fbdo->session.response.code = FIREBASE_ERROR_PATH_NOT_EXIST;
            fbdo->session.rtdb.path_not_found = true;
        }
        else
            fbdo->session.rtdb.path_not_found = false;

        if (fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
        {
            fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
            fbdo->session.rtdb.path.clear();
            fbdo->session.rtdb.raw.clear();
            fbdo->session.rtdb.push_name.clear();
            fbdo->session.rtdb.resp_data_type = d_any;
            fbdo->session.rtdb.data_available = false;
        }
    }
}

bool FB_RTDB::parseTCPResponse(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req,
                               struct fb_esp_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response)
{

    if (response.dataType == 0 && !response.isEvent)
    {
        bool getOfs = req->data.type == d_blob || req->method == rtdb_backup ||
                      ((req->data.type == d_file || tcpHandler.downloadOTA || req->data.type == d_any) && req->method == http_get);
        HttpHelper::parseRespPayload(*tcpHandler.payload, response, getOfs);

        fbdo->session.rtdb.resp_data_type = response.dataType;
        fbdo->session.content_length = response.payloadLen;

        fbdo->session.error = response.fbError;
        if (req->method == rtdb_backup || req->method == rtdb_restore)
            fbdo->session.error = response.fbError;

        if (req->method == rtdb_backup && response.dataType != d_json)
        {
            fbdo->session.response.code = FIREBASE_ERROR_EXPECTED_JSON_DATA;

            Signer.errorToString(fbdo->session.response.code, fbdo->session.error);

            tcpHandler.payload->clear();
            Signer.mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);
            fbdo->closeSession();
            return false;
        }

        if (req->method == http_get)
        {
            if (StringHelper::compare(fbdo->session.rtdb.resp_etag, 0, fb_esp_rtdb_pgm_str_11 /* "null_etag" */))
            {
                fbdo->session.response.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                fbdo->session.rtdb.path_not_found = true;
            }
        }
    }

    return true;
}

void FB_RTDB::endDownload(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req,
                          struct fb_esp_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response)
{
    if (tcpHandler.downloadByteLen > 0)
        reportDownloadProgress(fbdo, req, response.contentLen);

    if (tcpHandler.payload->length() > 0 && fbdo->session.rtdb.resp_data_type == d_blob)
    {
        // decode base64 string and store to temporary vector
        if (!fbdo->session.rtdb.blob)
        {
            fbdo->session.rtdb.isBlobPtr = true;
            fbdo->session.rtdb.blob = new MB_VECTOR<uint8_t>();
        }
        else
            MB_VECTOR<uint8_t>().swap(*fbdo->session.rtdb.blob);

        fbdo->session.rtdb.raw.clear();

        MB_String &payload = *tcpHandler.payload;

        if (payload[payload.length() - 1] == '"') // http response payload
            payload.erase(payload.length() - 1, 1);
        else // stream data
            trimEndJson(payload);

        payload.erase(0, response.payloadOfs);

        Base64Helper::decodeToArray<uint8_t>(Signer.mbfs, payload, *fbdo->session.rtdb.blob);
    }
    else if (fbdo->session.rtdb.resp_data_type == d_file)
    {
        // to make sure the file was closed
        Signer.mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);
        fbdo->session.rtdb.raw.clear();
    }
}

bool FB_RTDB::endDownloadOTA(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req,
                             struct fb_esp_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response)
{
    if (tcpHandler.downloadByteLen > 0)
        reportDownloadProgress(fbdo, req, response.contentLen);

    if (tcpHandler.downloadOTA)
    {

#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))

        // write extra pad
        if (tcpHandler.base64PadLenTail > 0 && tcpHandler.base64PadLenSignature == 0)
        {
            uint8_t pad[tcpHandler.base64PadLenTail];
            memset(pad, 0, tcpHandler.base64PadLenTail);
            Base64Helper::updateWrite(pad, tcpHandler.base64PadLenTail);
        }

        if (tcpHandler.error.code == 0)
        {
            if (!Update.end())
                tcpHandler.error.code = FIREBASE_ERROR_FW_UPDATE_END_FAILED;
        }

        if (tcpHandler.error.code != 0)
            fbdo->session.response.code = tcpHandler.error.code;

        return fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK;

#endif
    }

    return false;
}

int FB_RTDB::handleRedirect(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req,
                            struct fb_esp_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response)
{
    if (response.redirect && response.location.length() > 0)
    {

        fbdo->session.rtdb.redirect_count++;

        if (fbdo->session.rtdb.redirect_count > MAX_REDIRECT)
        {
            fbdo->session.response.code = FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED;
            fbdo->session.rtdb.redirect_url.clear();
            fbdo->session.rtdb.redirect_count = 0;
            return -1;
        }
        else
        {

            struct fb_esp_url_info_t uinfo;
            URLHelper::parse(Signer.mbfs, response.location, uinfo);
            struct fb_esp_rtdb_request_info_t newReq;
            newReq.method = req->method;
            newReq.data.type = req->data.type;
            newReq.data.address.priority = toAddr(fbdo->session.rtdb.priority);
            newReq.path = uinfo.uri;
            fbdo->session.rtdb.redirect_url = response.location;

            fbdo->closeSession();
            sendRequest(fbdo, &newReq);
            return 1;
        }
    }

    return 0;
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
    Signer.config->internal.fb_processing = false;

    if (fbdo->_dataAvailableCallback)
    {
        FIREBASE_STREAM_CLASS s;
        s.begin(&fbdo->session.rtdb.stream);

        fbdo->initJson();

        if (fbdo->session.rtdb.resp_data_type == d_json)
        {
            fbdo->session.jsonPtr->setJsonData(fbdo->session.rtdb.raw.c_str());
            fbdo->session.arrPtr->clear();
        }

        if (fbdo->session.rtdb.resp_data_type == d_array)
        {
            fbdo->session.arrPtr->setJsonArrayData(fbdo->session.rtdb.raw.c_str());
            fbdo->session.jsonPtr->clear();
        }

        s.jsonPtr = fbdo->session.jsonPtr;
        s.arrPtr = fbdo->session.arrPtr;

        s.sif->stream_path = fbdo->session.rtdb.stream_path.c_str();
        s.sif->data = fbdo->session.rtdb.raw.c_str();
        s.sif->path = fbdo->session.rtdb.path.c_str();
        s.sif->payload_length = fbdo->session.payload_length;
        s.sif->max_payload_length = fbdo->session.max_payload_length;

        s.sif->data_type = fbdo->session.rtdb.resp_data_type;
        fbdo->session.rtdb.data_type_str = fbdo->getDataType(s.sif->data_type);
        s.sif->data_type_str = fbdo->session.rtdb.data_type_str.c_str();
        s.sif->event_type_str = fbdo->session.rtdb.event_type.c_str();

        if (fbdo->session.rtdb.resp_data_type == d_blob)
        {
            if (!fbdo->session.rtdb.blob)
            {
                fbdo->session.rtdb.isBlobPtr = true;
                fbdo->session.rtdb.blob = new MB_VECTOR<uint8_t>();
            }
            s.sif->blob = fbdo->session.rtdb.blob;
        }

        fbdo->_dataAvailableCallback(s);
        fbdo->session.rtdb.data_available = false;

        s.empty();
    }
    else if (fbdo->_multiPathDataCallback)
    {
        FIREBASE_MP_STREAM_CLASS s;
        s.begin(&fbdo->session.rtdb.stream);
        s.sif->data_type = fbdo->session.rtdb.resp_data_type;
        s.sif->path = fbdo->session.rtdb.path;
        s.sif->data_type_str = fbdo->getDataType(s.sif->data_type);
        s.sif->event_type_str = fbdo->session.rtdb.event_type;
        s.sif->payload_length = fbdo->session.payload_length;
        s.sif->max_payload_length = fbdo->session.max_payload_length;

        if (!fbdo->session.jsonPtr)
            fbdo->session.jsonPtr = new FirebaseJson();

        if (fbdo->session.rtdb.resp_data_type == d_json)
            fbdo->session.jsonPtr->setJsonData(fbdo->session.rtdb.raw.c_str());

        if (s.sif->data_type == d_json)
            s.sif->m_json = fbdo->session.jsonPtr;
        else
        {
            fbdo->session.jsonPtr->clear();
            s.sif->data = fbdo->session.rtdb.raw.c_str();
        }

        fbdo->_multiPathDataCallback(s);
        fbdo->session.rtdb.data_available = false;
        s.empty();
    }
}

void FB_RTDB::splitStreamPayload(const MB_String &payloads, MB_VECTOR<MB_String> &payload)
{
    int ofs = 0;
    int pos1 = 0, pos2 = 0, pos3 = 0;

    while (pos1 > -1)
    {
        if (StringHelper::find(payloads, fb_esp_rtdb_pgm_str_12 /* "event: " */, false, ofs, pos1))
        {
            ofs = pos1 + 1;
            if (StringHelper::find(payloads, fb_esp_rtdb_pgm_str_13 /* "data: " */, false, ofs, pos2))
            {
                ofs = pos2 + 1;
                if (StringHelper::find(payloads, fb_esp_pgm_str_12 /* "\n" */, false, ofs, pos3))
                    ofs = pos3 + 1;
                else
                    pos3 = payloads.length();

                payload.push_back(payloads.substr(pos1, pos3 - pos1));
            }
        }
    }
}

void FB_RTDB::parseStreamPayload(FirebaseData *fbdo, const MB_String &payload)
{
    struct server_response_data_t response;

    HttpHelper::parseRespPayload(payload, response, false);

    fbdo->session.rtdb.resp_data_type = response.dataType;
    fbdo->session.content_length = response.payloadLen;

    fbdo->clearJson();

    if (fbdo->session.rtdb.resp_data_type == d_blob)
    {
        if (fbdo->session.rtdb.blob)
            MB_VECTOR<uint8_t>().swap(*fbdo->session.rtdb.blob);
        else
        {
            fbdo->session.rtdb.isBlobPtr = true;
            fbdo->session.rtdb.blob = new MB_VECTOR<uint8_t>();
        }

        fbdo->session.rtdb.raw.clear();
        Base64Helper::decodeToArray<uint8_t>(Signer.mbfs, payload.c_str() + response.payloadOfs, *fbdo->session.rtdb.blob);
    }
    else if (fbdo->session.rtdb.resp_data_type == d_file)
    {
        Signer.mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);
        fbdo->session.rtdb.raw.clear();
    }

    if (StringHelper::compare(response.eventType, 0, fb_esp_pgm_str_16 /* "put" */) ||
        StringHelper::compare(response.eventType, 0, fb_esp_pgm_str_17 /* "patch" */))
    {

        handlePayload(fbdo, response, payload);

        // Any stream update?
        // based on BLOB or file event data changes (no old data available for comparision or inconvenient for large data)
        // event path changes
        // event data changes without the path changes
        if (fbdo->session.rtdb.resp_data_type == d_blob ||
            fbdo->session.rtdb.resp_data_type == d_file ||
            response.eventPathChanged ||
            (!response.eventPathChanged && response.dataChanged && !fbdo->session.rtdb.stream_path_changed))
        {
            fbdo->session.rtdb.stream_data_changed = true;
        }
        else
            fbdo->session.rtdb.stream_data_changed = false;

        fbdo->session.rtdb.data_available = true;
        fbdo->session.rtdb.stream_path_changed = false;
    }
    else
    {
        // Firebase keep alive event
        if (StringHelper::compare(response.eventType, 0, fb_esp_pgm_str_15 /* "keep-alive" */))
        {
            if (fbdo->_timeoutCallback)
                fbdo->sendStreamToCB(0);
        }

        // Firebase cancel and auth_revoked events
        else if (StringHelper::compare(response.eventType, 0, fb_esp_rtdb_pgm_str_14 /* "cancel" */) ||
                 StringHelper::compare(response.eventType, 0, fb_esp_rtdb_pgm_str_15 /* "auth_revoked" */))
        {
            fbdo->session.rtdb.event_type = response.eventType;
            // make stream available status
            fbdo->session.rtdb.stream_data_changed = true;
            fbdo->session.rtdb.data_available = true;

            // We need to close the current session due to the token was already expired.
            if (StringHelper::compare(response.eventType, 0, fb_esp_rtdb_pgm_str_15 /* "auth_revoked" */))
                fbdo->closeSession();
        }
    }
}

void FB_RTDB::parsePayload(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req,
                           struct server_response_data_t &response, MB_String payload)
{
    // parse the payload
    if (payload.length() > 0)
    {
        // stream data?
        if (response.isEvent)
        {
            int pos = 0, ofs = 0, jsonCount = 0;
            bool validJson = false;

            // counting the occurences of JSON sets
            while (pos > -1)
            {
                if (StringHelper::find(payload, fb_esp_rtdb_pgm_str_12 /* "event: " */, false, ofs, pos))
                {
                    ofs = pos + 1;
                    jsonCount++;
                }
            }

            // we need to spit the tream data if stream data contains multiple sets of JSON
            // that happens in case simultaneously children data changes.
            // {json1}{json..}{json n}
            // Then we pase each JSON and send to callback function.
            MB_VECTOR<MB_String> payloadList;
            splitStreamPayload(payload, payloadList);
            for (size_t i = 0; i < payloadList.size(); i++)
            {
                if (Utils::validJS(payloadList[i].c_str()))
                {
                    validJson = true;
                    parseStreamPayload(fbdo, payloadList[i].c_str());
                    sendCB(fbdo);
                }
            }
            payloadList.clear(); // clear payload after splitting
            payload.clear();

            if (validJson)
            {
                fbdo->session.rtdb.data_millis = millis();
                fbdo->session.rtdb.data_tmo = false;
            }
            else
            {
                fbdo->session.rtdb.data_millis = 0;
                fbdo->session.rtdb.data_tmo = true;
                fbdo->closeSession();
            }

            // stream data handling ends here
        }
        // http response payload
        else
        {

            fbdo->session.rtdb.resp_data_type = response.dataType;
            fbdo->session.content_length = response.payloadLen;

            if (req->method == rtdb_set_rules)
            {
                if (StringHelper::compare(payload, 0, fb_esp_rtdb_pgm_str_16 /* "{\"status\":\"ok\"}" */))
                    payload.clear();
            }

            if (fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK ||
                fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED)
            {

                if (req->method != rtdb_set_rules && fbdo->session.rtdb.resp_data_type != d_blob &&
                    fbdo->session.rtdb.resp_data_type != d_file &&
                    fbdo->session.rtdb.resp_data_type != d_file_ota)
                {
                    handlePayload(fbdo, response, payload.c_str());

                    if (fbdo->session.rtdb.priority_val_flag)
                        fbdo->session.rtdb.path =
                            fbdo->session.rtdb.path.substr(0, fbdo->session.rtdb.path.length() -
                                                                  strlen_P(fb_esp_rtdb_pgm_str_2 /* ".priority" */) - 1);

                    // Push (POST) data? set push name
                    if (req->method == http_post)
                    {
                        if (response.pushName.length() > 0)
                        {
                            fbdo->session.rtdb.push_name = response.pushName.c_str();
                            fbdo->session.rtdb.resp_data_type = d_any;
                            fbdo->session.rtdb.raw.clear();
                        }
                    }
                }
            }

            // mismatch data type check
            if (Signer.config->rtdb.data_type_stricted && req->method == http_get &&
                req->data.type != d_timestamp &&
                !response.noContent && response.httpCode < 400)
            {
                bool _reqType = req->data.type == d_integer ||
                                req->data.type == d_float ||
                                req->data.type == d_double;
                bool _respType = fbdo->session.rtdb.resp_data_type == d_integer ||
                                 fbdo->session.rtdb.resp_data_type == d_float ||
                                 fbdo->session.rtdb.resp_data_type == d_double;

                if (req->data.type == fbdo->session.rtdb.resp_data_type ||
                    (_reqType && _respType) ||
                    (fbdo->session.rtdb.priority > 0 && fbdo->session.rtdb.resp_data_type == d_json))
                    fbdo->session.rtdb.data_mismatch = false;
                else if (req->data.type != d_any)
                {
                    fbdo->session.rtdb.data_mismatch = true;
                    fbdo->session.response.code = FIREBASE_ERROR_DATA_TYPE_MISMATCH;
                }
            }
        }
    }

    payload.clear();
}

void FB_RTDB::handlePayload(FirebaseData *fbdo, struct server_response_data_t &response, const MB_String &payload)
{

    fbdo->session.rtdb.raw.clear();
    fbdo->clearJson();

    if (response.isEvent)
    {
        response.eventPathChanged = strcmp(response.eventPath.c_str(), fbdo->session.rtdb.path.c_str()) != 0;
        fbdo->session.rtdb.path = response.eventPath;
        fbdo->session.rtdb.event_type = response.eventType;
    }

    if (fbdo->session.rtdb.resp_data_type != d_blob && fbdo->session.rtdb.resp_data_type != d_file_ota)
    {
        if (response.isEvent)
            fbdo->session.rtdb.raw = response.eventData.c_str();
        else
            fbdo->session.rtdb.raw = payload;

        if (fbdo->session.rtdb.resp_data_type == d_string)
            fbdo->setRaw(true); // if double quotes string, trim it.

        uint16_t crc = Utils::calCRC(Signer.mbfs, fbdo->session.rtdb.raw.c_str());
        response.dataChanged = fbdo->session.rtdb.data_crc != crc;
        fbdo->session.rtdb.data_crc = crc;
    }
}

int FB_RTDB::getPayloadLen(fb_esp_rtdb_request_info_t *req)
{
    size_t len = 0;
    if (req->method != http_get)
    {
        if (req->data.address.din > 0)
        {
            if (req->data.type == d_blob && req->data.address.priority == 0)
                len = (4 * ceil(req->data.blobSize / 3.0)) + strlen_P(fb_esp_rtdb_pgm_str_7 /* "\"blob,base64," */) + 1;
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
        else if (req->fileSize > 0 && req->data.address.priority == 0)
        {
            len = req->fileSize;
            if (req->data.type == d_file || req->data.type == d_file_ota || req->data.type == d_blob)
                len = (4 * ceil(req->fileSize / 3.0)) + strlen_P(fb_esp_rtdb_pgm_str_7 /* "\"blob,base64," */) + 1;
        }
        else if (req->method == rtdb_restore)
            len = req->fileSize;
    }
    return len;
}

fb_esp_request_method FB_RTDB::getHTTPMethod(fb_esp_rtdb_request_info_t *req)
{
    if (req->method == http_post)
        return http_post;
    else if (req->method == http_put ||
             req->method == rtdb_set_nocontent ||
             req->method == rtdb_set_priority ||
             req->method == rtdb_set_rules)
        return http_put;
    else if (req->method == rtdb_stream ||
             req->method == http_get ||
             req->method == rtdb_get_nocontent ||
             req->method == rtdb_get_shallow ||
             req->method == rtdb_get_priority ||
             req->method == rtdb_backup ||
             req->method == rtdb_get_rules)
        return http_get;
    else if (req->method == http_patch ||
             req->method == rtdb_update_nocontent ||
             req->method == rtdb_restore)
        return http_patch;
    else if (req->method == http_delete)
        return http_delete;

    return req->method;
}

bool FB_RTDB::hasPayload(struct fb_esp_rtdb_request_info_t *req)
{
    return getHTTPMethod(req) == http_put || getHTTPMethod(req) == http_post || getHTTPMethod(req) == http_patch;
}

bool FB_RTDB::sendRequestHeader(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    fb_esp_request_method http_method = getHTTPMethod(req);
    fbdo->session.rtdb.shallow_flag = false;
    fbdo->session.rtdb.priority_val_flag = false;

    bool hasServerValue = false;

    if (req->data.type == d_json)
    {
        int p;
        if (req->data.address.din > 0 && req->data.type == d_json)
            hasServerValue = StringHelper::find(addrTo<FirebaseJson *>(req->data.address.din)->raw(),
                                                fb_esp_rtdb_pgm_str_17 /* "\".sv\"" */, false, 0, p);
        else
            hasServerValue = StringHelper::find(req->payload, fb_esp_rtdb_pgm_str_17 /* "\".sv\"" */, false, 0, p);
    }

    MB_String header;

    HttpHelper::addRequestHeaderFirst(header, fbdo->session.classic_request &&
                                                      (http_method == http_put || http_method == http_delete)
                                                  ? http_post
                                                  : http_method);

    Utils::makePath(req->path);
    header += req->path;

    if (req->method == http_patch || req->method == rtdb_update_nocontent)
        header += fb_esp_pgm_str_1; // "/"

    bool appendAuth = false;
    bool hasQueryParams = false;

    if (fbdo->session.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        URLHelper::parse(Signer.mbfs, fbdo->session.rtdb.redirect_url, uinfo);
        if (uinfo.auth.length() == 0)
            appendAuth = true;
    }
    else
        appendAuth = true;

    if (appendAuth)
    {
        header += fb_esp_rtdb_pgm_str_18; // ".json"
        if (Signer.getTokenType() != token_type_oauth2_access_token && !Signer.config->signer.test_mode)
            URLHelper::addParam(header, fb_esp_rtdb_pgm_str_19 /* "auth=" */, "", hasQueryParams, true);

        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        if (Signer.getTokenType() != token_type_oauth2_access_token && !Signer.config->signer.test_mode)
            fbdo->tcpClient.send(Signer.config->internal.auth_token.c_str());

        if (fbdo->session.response.code < 0)
            return false;
    }

    if (fbdo->session.rtdb.read_tmo > 0)
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_20 /* "timeout=" */,
                            MB_String(fbdo->session.rtdb.read_tmo) + fb_esp_rtdb_pgm_str_21 /* "ms" */, hasQueryParams);

    URLHelper::addParam(header, fb_esp_rtdb_pgm_str_22 /* "writeSizeLimit=" */,
                        fbdo->session.rtdb.write_limit, hasQueryParams);

    if (req->method == rtdb_get_shallow)
    {
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_23 /* "shallow=true" */, "", hasQueryParams, true);
        fbdo->session.rtdb.shallow_flag = true;
    }

    QueryFilter *query = req->data.address.query > 0 ? addrTo<QueryFilter *>(req->data.address.query) : nullptr;
    bool hasQuery = false;
    if (req->method == http_get && query && query->_orderBy.length() > 0)
    {
        hasQuery = true;
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_24 /* "orderBy=" */, query->_orderBy, hasQueryParams);
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_25 /* "&limitToFirst=" */, query->_limitToFirst, hasQueryParams);
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_26 /* "&limitToLast=" */, query->_limitToLast, hasQueryParams);
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_27 /* "&startAt=" */, query->_startAt, hasQueryParams);
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_30 /* "&endAt=" */, query->_endAt, hasQueryParams);
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_31 /* "&equalTo=" */, query->_equalTo, hasQueryParams);
    }

    if (req->method == rtdb_backup)
    {
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_32 /* "format=export" */, "", hasQueryParams, true);
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_28 /* "download=" */, fbdo->session.rtdb.filename, hasQueryParams);
    }

    if (req->method == http_get && req->filename.length() > 0)
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_28 /* "download=" */, fbdo->session.rtdb.filename, hasQueryParams);

    if (req->async || req->method == rtdb_get_nocontent ||
        req->method == rtdb_restore || req->method == rtdb_set_nocontent ||
        req->method == rtdb_update_nocontent)
        URLHelper::addParam(header, fb_esp_rtdb_pgm_str_29 /* "print=silent" */, "", hasQueryParams, true);

    HttpHelper::addRequestHeaderLast(header);
    HttpHelper::addHostHeader(header, Signer.config->database_url.c_str());
    HttpHelper::addUAHeader(header);
    HttpHelper::getCustomHeaders(header, Signer.config->signer.customHeaders);

    if (Signer.getTokenType() == token_type_oauth2_access_token)
    {
        HttpHelper::addAuthHeaderFirst(header, token_type_oauth2_access_token);

        if (Signer.config->signer.tokens.auth_type.length() > 0 &&
            Signer.config->signer.tokens.auth_type[Signer.config->signer.tokens.auth_type.length() - 1] != ' ')
            header += fb_esp_pgm_str_9; // " "

        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpClient.send(Signer.config->internal.auth_token.c_str());

        if (fbdo->session.response.code < 0)
            return false;

        HttpHelper::addNewLine(header);
    }

    // Timestamp cannot use with ETag header, due to internal server error
    if (!hasServerValue && !hasQuery && req->data.type != d_timestamp &&
        (req->method == http_delete || req->method == http_get ||
         req->method == rtdb_get_nocontent || req->method == http_put ||
         req->method == rtdb_set_nocontent || req->method == http_post))
        header += fb_esp_rtdb_pgm_str_33; // "X-Firebase-ETag: true\r\n"

    if (fbdo->session.rtdb.req_etag.length() > 0 &&
        (req->method == http_put || req->method == rtdb_set_nocontent || req->method == http_delete))
    {
        header += fb_esp_rtdb_pgm_str_34; // "if-match: "
        header += fbdo->session.rtdb.req_etag;
        HttpHelper::addNewLine(header);
    }

    if (fbdo->session.classic_request && http_method != http_get && http_method != http_post && http_method != http_patch)
    {
        header += fb_esp_rtdb_pgm_str_36; // "X-HTTP-Method-Override: "
        if (http_method == http_put || http_method == http_delete)
            HttpHelper::addRequestHeaderFirst(header, http_method);
        HttpHelper::addNewLine(header);
    }

    if (req->method == rtdb_stream)
    {
        fbdo->session.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        HttpHelper::addConnectionHeader(header, false);
        header += fb_esp_rtdb_pgm_str_35; //  "Accept: text/event-stream\r\n"
    }
    else
    {
        // required for ESP32 core sdk v2.0.x.
        fbdo->session.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
        keepAlive = true;
#endif
        HttpHelper::addConnectionHeader(header, keepAlive);
        header += fb_esp_rtdb_pgm_str_37; // "Keep-Alive: timeout=30, max=100\r\n"
    }

    if (req->method != rtdb_backup && req->method != rtdb_restore)
        header += fb_esp_rtdb_pgm_str_38; // "Accept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\n"

    if (req->method == rtdb_get_priority || req->method == rtdb_set_priority)
        fbdo->session.rtdb.priority_val_flag = true;

    if (hasPayload(req))
        HttpHelper::addContentLengthHeader(header, getPayloadLen(req));

    HttpHelper::addNewLine(header);

    fbdo->tcpClient.send(header.c_str());
    header.clear();

    if (fbdo->session.response.code < 0)
        return false;

    return true;
}

void FB_RTDB::removeStreamCallback(FirebaseData *fbdo)
{
    fbdo->removeSession();

    fbdo->_dataAvailableCallback = NULL;
    fbdo->_timeoutCallback = NULL;

    if (Signer.config->internal.sessions.size() == 0)
    {
#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
        if (Signer.config->internal.stream_task_handle)
            vTaskDelete(Signer.config->internal.stream_task_handle);

        Signer.config->internal.stream_task_handle = NULL;
#endif
    }
}

void FB_RTDB::clearDataStatus(FirebaseData *fbdo)
{
    fbdo->clearJson();
    fbdo->session.rtdb.stream_data_changed = false;
    fbdo->session.rtdb.stream_path_changed = false;
    fbdo->session.rtdb.data_available = false;
    fbdo->session.rtdb.data_tmo = false;
    fbdo->session.content_length = -1;
    fbdo->session.rtdb.resp_data_type = d_any;
    fbdo->session.response.code = -1000;
    fbdo->session.rtdb.file_size = 0;
    fbdo->session.rtdb.raw.clear();
    fbdo->session.rtdb.data_type_str.clear();
    fbdo->session.rtdb.path.clear();
    fbdo->session.rtdb.push_name.clear();
}

bool FB_RTDB::connectionError(FirebaseData *fbdo)
{
    return fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED ||
           fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST ||
           fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED ||
           fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED ||
           fbdo->session.response.code == FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
}

bool FB_RTDB::handleStreamRequest(FirebaseData *fbdo, const MB_String &path)
{
    if (Signer.isExpired())
        return false;

    struct fb_esp_rtdb_request_info_t req;
    req.method = rtdb_stream;
    req.data.type = d_string;

    if (fbdo->session.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        URLHelper::parse(Signer.mbfs, fbdo->session.rtdb.redirect_url, uinfo);
        req.path = uinfo.uri.c_str();
    }
    else
        req.path = path.c_str();

    if (!preRequestCheck(fbdo, &req))
        return false;

    if (!sendRequest(fbdo, &req))
        return false;

    return true;
}

#endif

#endif // ENABLE