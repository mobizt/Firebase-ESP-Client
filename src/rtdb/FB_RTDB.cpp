/**
 * Google's Firebase Realtime Database class, FB_RTDB.cpp version 2.0.6
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 7, 2022
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
#if defined(ESP32) || defined(ESP8266)
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

bool FB_RTDB::mGetRules(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr filename, RTDB_DownloadProgressCallback callback)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path += fb_esp_pgm_str_103;
    req.method = m_read_rules;
    req.data.type = d_json;
    req.filename = filename;
    req.storageType = storageType;
    req.downloadCallback = callback;

    ut->makePath(req.filename);

    if (req.filename.length() > 0)
        req.task_type = fb_esp_rtdb_task_download_rules;
    else
        req.task_type = fb_esp_rtdb_task_read_rules;

    bool ret = handleRequest(fbdo, &req);

    if (req.filename.length() == 0)
    {
        FirebaseJson *json = fbdo->jsonObjectPtr();
        if (json->errorPosition() > -1)
        {
            fbdo->session.response.code = FIREBASE_ERROR_INVALID_JSON_RULES;
            ret = false;
        }
        json->clear();
    }

    return ret;
}

bool FB_RTDB::mSetRules(FirebaseData *fbdo, MB_StringPtr rules, fb_esp_mem_storage_type storageType, MB_StringPtr filename, RTDB_UploadProgressCallback callback)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path += fb_esp_pgm_str_103;
    req.method = m_set_rules;
    req.payload = rules;
    req.filename = filename;
    ut->makePath(req.filename);
    req.storageType = storageType;

    if (req.filename.length() > 0)
        req.task_type = fb_esp_rtdb_task_upload_rules;
    else
        req.task_type = fb_esp_rtdb_task_store_rules;

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
    Signer.config->internal.auth_token = atok.c_str();
    atok.clear();
    Signer.config->signer.tokens.legacy_token = "";
    Signer.config->signer.tokens.token_type = tk;
    Signer.config->internal.atok_len = Signer.config->internal.auth_token.length();
    Signer.config->internal.ltok_len = 0;
    Signer.handleToken();
}

bool FB_RTDB::mSetQueryIndex(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr node, MB_StringPtr databaseSecret)
{
    if (fbdo->session.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    MB_String s;
    bool ret = false;
    MB_String atok;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
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

            s.clear();
            s += fb_esp_pgm_str_550;

            MB_String _path = path;
            MB_String _node = node;

            ut->replaceFirebasePath(_path);
            ut->makePath(_path);
            s += _path;
            s += fb_esp_pgm_str_551;

            json->get(data, s.c_str());

            if (data.success && strcmp(data.to<const char *>(), _node.c_str()) == 0)
                ruleExisted = true;

            if (_node.length() == 0)
                json->remove(s.c_str());
            else
                json->set(s.c_str(), _node.c_str());

            if (!ruleExisted || (ruleExisted && _node.length() == 0))
            {
                MB_String str;
                json->toString(str, true);
                ret = setRules(fbdo, str.c_str());
            }
        }

        json->clear();
    }

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
            restoreToken(atok, tk);
    }

    s.clear();
    return ret;
}

bool FB_RTDB::mSetReadWriteRules(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr var, MB_StringPtr readVal, MB_StringPtr writeVal, MB_StringPtr databaseSecret)
{
    if (fbdo->session.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

    MB_String s;
    bool ret = false;
    MB_String atok;

    fb_esp_auth_token_type tk = Signer.getTokenType();

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
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

        MB_String s;
        s += fb_esp_pgm_str_550;

        ut->replaceFirebasePath(_path);
        ut->makePath(_path);

        s += _path;
        s += fb_esp_pgm_str_1;
        s += var;

        if (_readVal.length() > 0)
        {
            rd = true;
            MB_String r = s;
            r += fb_esp_pgm_str_1;
            r += fb_esp_pgm_str_552;
            json.get(data, r.c_str());
            if (data.success)
                if (strcmp(data.to<const char *>(), _readVal.c_str()) == 0)
                    rd = false;
        }

        if (_writeVal.length() > 0)
        {
            wr = true;
            MB_String w = s;
            w += fb_esp_pgm_str_1;
            w += fb_esp_pgm_str_553;
            json.get(data, w.c_str());
            if (data.success)
                if (strcmp(data.to<const char *>(), _writeVal.c_str()) == 0)
                    wr = false;
        }

        // modify if the rules changed or does not exist.
        if (wr || rd)
        {
            FirebaseJson js;
            MB_String r, w;
            r += fb_esp_pgm_str_552;
            w += fb_esp_pgm_str_553;
            if (rd)
                js.add(r.c_str(), _readVal.c_str());

            if (wr)
                js.add(w.c_str(), _writeVal.c_str());

            json.set(s.c_str(), js);
            MB_String str;
            json.toString(str, true);
            ret = setRules(fbdo, str.c_str());
        }

        json.clear();
    }

    if (addrTo<const char *>(databaseSecret.address()))
    {
        if (strlen(addrTo<const char *>(databaseSecret.address())) && tk != token_type_oauth2_access_token && tk != token_type_legacy_token)
            restoreToken(atok, tk);
    }

    s.clear();
    return ret;
}

bool FB_RTDB::mPathExisted(FirebaseData *fbdo, MB_StringPtr path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    ut->replaceFirebasePath(req.path);
    req.method = m_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return !fbdo->session.rtdb.path_not_found;
    else
        return false;
}

String FB_RTDB::mGetETag(FirebaseData *fbdo, MB_StringPtr path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    req.method = m_get_nocontent;
    req.data.type = d_string;
    if (handleRequest(fbdo, &req))
        return fbdo->session.rtdb.resp_etag.c_str();
    else
        return "";
}

bool FB_RTDB::mGetShallowData(FirebaseData *fbdo, MB_StringPtr path)
{
    struct fb_esp_rtdb_request_info_t req;
    req.path = path;
    ut->replaceFirebasePath(req.path);
    req.method = m_get_shallow;
    req.data.type = d_string;
    return handleRequest(fbdo, &req);
}

void FB_RTDB::enableClassicRequest(FirebaseData *fbdo, bool enable)
{
    fbdo->session.classic_request = enable;
}

bool FB_RTDB::buildRequest(FirebaseData *fbdo, fb_esp_method method, MB_StringPtr path, MB_StringPtr payload, fb_esp_data_type type, int subtype, uint32_t value_addr, uint32_t query_addr, uint32_t priority_addr, MB_StringPtr etag, bool async, bool queue, size_t blob_size, MB_StringPtr filename, fb_esp_mem_storage_type storage_type, RTDB_DownloadProgressCallback downloadCallback, RTDB_UploadProgressCallback uploadCallback)
{
    ut->idle();

    struct fb_esp_rtdb_request_info_t req;

    MB_String _path, tpath, pre, post;

    tpath = path;
    ut->makePath(tpath);
    ut->replaceFirebasePath(tpath);
    _path = tpath;

    req.downloadCallback = downloadCallback;
    req.uploadCallback = uploadCallback;

    if (method == m_set_priority || method == m_get_priority)
    {
        tpath += fb_esp_pgm_str_156;
        req.path = tpath.c_str();
    }
    else if (priority_addr > 0 && method != m_get && type != d_blob && type != d_file && type != d_file_ota)
    {
        if (type == d_json)
        {
            // set priority to source json
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
            // construct json pre and post payloads for priority
            // pre -> {".priority":priority,"subpath":
            // pos -> }

            float *priority = addrTo<float *>(priority_addr);
            if (priority)
            {
                pre += fb_esp_pgm_str_163;
                pre += fb_esp_pgm_str_3;
                pre += fb_esp_pgm_str_157;
                pre += fb_esp_pgm_str_3;
                pre += fb_esp_pgm_str_7;
                pre += *priority;
                pre += fb_esp_pgm_str_132;
                pre += fb_esp_pgm_str_3;

                if (type == d_string)
                    post += fb_esp_pgm_str_3;
                post += fb_esp_pgm_str_127;

                int p1 = ut->rstrpos(tpath.c_str(), '/', 0), p2 = 0;
                tpath = path;
                if (p1 > 0)
                {
                    p2 = p1 + 1;
                    req.path = tpath.substr(0, p2).c_str();
                    // subpath
                    pre += tpath.substr(p2, _path.length() - 1 - p2).c_str();
                }
                else
                {
                    tpath.erase(0, 1);
                    // subpath
                    pre += tpath.c_str();
                    tpath.clear();
                    tpath += '/';
                    req.path = tpath.c_str();
                }

                pre += fb_esp_pgm_str_3;
                pre += fb_esp_pgm_str_7;
                if (type == d_string)
                    pre += fb_esp_pgm_str_3;

                req.pre_payload = pre.c_str();
                req.post_payload = post.c_str();
            }
        }
    }
    else
    {
        if (type == d_string)
        {
            pre += fb_esp_pgm_str_3;
            post += fb_esp_pgm_str_3;
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
    ut->makePath(req.filename);
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

bool FB_RTDB::mDeleteNodesByTimestamp(FirebaseData *fbdo, MB_StringPtr path, MB_StringPtr timestampNode, MB_StringPtr limit, MB_StringPtr dataRetentionPeriod)
{
    if (fbdo->session.rtdb.pause)
        return true;

    if (!fbdo->reconnect() || !fbdo->tokenReady())
        return false;

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
    uint32_t pr = ut->strtoull_alt(_dataRetentionPeriod.c_str());
#else
    char *pEnd;
    uint32_t pr = strtoull(_dataRetentionPeriod.c_str(), &pEnd, 10);
#endif

    QueryFilter query;

    uint32_t lastTS = current_ts - pr;

    if (strcmp(_timestampNode.c_str(), (const char *)MBSTRING_FLASH_MCR("$key")) == 0)
    {
        MB_String start = 0;
        MB_String end = lastTS;
        query.orderBy(_timestampNode.c_str()).startAt(start).endAt(end).limitToLast(_limit);
    }
    else
        query.orderBy(_timestampNode.c_str()).startAt(0).endAt(lastTS).limitToLast(_limit);

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
                    nodes[i] = value.key.c_str();
            }
            js->iteratorEnd();
            js->clear();

            for (size_t i = 0; i < len; i++)
            {
                MB_String s = path;
                s += fb_esp_pgm_str_1;
                s += nodes[i];
                deleteNode(fbdo, s.c_str());
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
    FirebaseConfig *cfg = Signer.getCfg();

    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    fbdo->session.rtdb.pause = false;

    fbdo->session.rtdb.new_stream = true;

    if (!fbdo->reconnect())
        return false;

    if (!fbdo->tcpClient.reserved)
        fbdo->closeSession();

    fbdo->session.rtdb.stream_stop = false;
    fbdo->session.rtdb.data_tmo = false;

    fbdo->session.rtdb.stream_path = path;

    ut->replaceFirebasePath(fbdo->session.rtdb.stream_path);

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
    // if the client used by authentication task
    if (fbdo->tcpClient.reserved)
        return false;

    // prevent redundant calling
    if (fbdo->session.streaming)
        return false;

    fbdo->session.streaming = true;

    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return exitStream(fbdo, false);
    }

    if (fbdo->session.rtdb.pause || fbdo->session.rtdb.stream_stop)
        return exitStream(fbdo, true);

    if (!fbdo->reconnect())
    {
        fbdo->session.rtdb.data_tmo = true;
        return exitStream(fbdo, true);
    }

    if (!fbdo->tokenReady())
        return exitStream(fbdo, false);

    bool ret = false;
    bool reconnectStream = false;

    // trying to reconnect the stream when required at some interval as running in the loop
    if (cfg->timeout.rtdbStreamReconnect < MIN_RTDB_STREAM_RECONNECT_INTERVAL || cfg->timeout.rtdbStreamReconnect > MAX_RTDB_STREAM_RECONNECT_INTERVAL)
        cfg->timeout.rtdbStreamReconnect = MIN_RTDB_STREAM_RECONNECT_INTERVAL;

    if (millis() - cfg->timeout.rtdbStreamReconnect > fbdo->session.rtdb.stream_resume_millis)
    {
        reconnectStream = (fbdo->session.rtdb.data_tmo && !fbdo->session.connected) || fbdo->session.response.code >= 400 || fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream;

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

    if (cfg->timeout.rtdbKeepAlive < MIN_RTDB_KEEP_ALIVE_TIMEOUT || cfg->timeout.rtdbKeepAlive > MAX_RTDB_KEEP_ALIVE_TIMEOUT)
        cfg->timeout.rtdbKeepAlive = DEFAULT_RTDB_KEEP_ALIVE_TIMEOUT;

    if (millis() - fbdo->session.rtdb.data_millis > cfg->timeout.rtdbKeepAlive)
    {
        fbdo->session.rtdb.data_millis = millis();
        fbdo->session.rtdb.data_tmo = true;
        reconnectStream = true;
        fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
    }

    if (reconnectStream)
    {
        fbdo->session.rtdb.new_stream = true;

        if (!ut->waitIdle(fbdo->session.response.code))
            return exitStream(fbdo, false);

        fbdo->closeSession();

        if (!fbdo->tokenReady())
            return exitStream(fbdo, false);

        MB_String path = fbdo->session.rtdb.stream_path;
        if (fbdo->session.rtdb.redirect_url.length() > 0)
        {
            struct fb_esp_url_info_t uinfo;
            ut->getUrlInfo(fbdo->session.rtdb.redirect_url, uinfo);
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

#if defined(ESP32)
void FB_RTDB::setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize)
{
    fbdo->session.rtdb.stream_task_enable = false;
#elif defined(ESP8266) || defined(FB_ENABLE_EXTERNAL_CLIENT)
void FB_RTDB::setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback)
{
#endif
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    removeMultiPathStreamCallback(fbdo);

    fbdo->_dataAvailableCallback = dataAvailableCallback;
    fbdo->_timeoutCallback = timeoutCallback;

    fbdo->addAddr(fb_esp_con_mode_rtdb_stream);

#if defined(ESP32)
    MB_String taskName = fb_esp_pgm_str_72;
    taskName += fb_esp_pgm_str_113;
    taskName += fbdo->addr;

    if (streamTaskStackSize > STREAM_TASK_STACK_SIZE)
        cfg->internal.stream_task_stack_size = streamTaskStackSize;
    else
        cfg->internal.stream_task_stack_size = STREAM_TASK_STACK_SIZE;

    fbdo->session.rtdb.stream_task_enable = true;
#endif

#if defined(ESP32)
    runStreamTask(fbdo, taskName.c_str());
#elif defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#else
runStreamTask();
#endif
}

#if defined(ESP32)
void FB_RTDB::setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize)
{
    fbdo->session.rtdb.stream_task_enable = false;
#elif defined(ESP8266) || defined(FB_ENABLE_EXTERNAL_CLIENT)
void FB_RTDB::setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback)
{
#endif

    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    removeStreamCallback(fbdo);

    fbdo->_multiPathDataCallback = multiPathDataCallback;
    fbdo->_timeoutCallback = timeoutCallback;

    fbdo->addAddr(fb_esp_con_mode_rtdb_stream);

#if defined(ESP32)
    MB_String taskName = fb_esp_pgm_str_72;
    taskName += fb_esp_pgm_str_113;
    taskName += fbdo->addr;

    if (streamTaskStackSize > STREAM_TASK_STACK_SIZE)
        cfg->internal.stream_task_stack_size = streamTaskStackSize;
    else
        cfg->internal.stream_task_stack_size = STREAM_TASK_STACK_SIZE;

    fbdo->session.rtdb.stream_task_enable = true;
#endif

#if defined(ESP32)
    runStreamTask(fbdo, taskName.c_str());
#elif defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#else
runStreamTask();
#endif
}

void FB_RTDB::removeMultiPathStreamCallback(FirebaseData *fbdo)
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    fbdo->_multiPathDataCallback = NULL;
    fbdo->_timeoutCallback = NULL;
    fbdo->removeAddr();

#if defined(ESP32)
    if (cfg->internal.fbdo_addr_list.size() == 0)
    {
        if (cfg->internal.stream_task_handle)
            vTaskDelete(cfg->internal.stream_task_handle);

        cfg->internal.stream_task_handle = NULL;
    }
#endif
}

#if defined(ESP32)
void FB_RTDB::runStreamTask(FirebaseData *fbdo, const char *taskName)
#elif defined(ESP8266) || defined(FB_ENABLE_EXTERNAL_CLIENT)
void FB_RTDB::runStreamTask()
#endif
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
        return;
#if defined(ESP32)

    static FB_RTDB *_this = this;
    MB_String name = taskName;

    TaskFunction_t taskCode = [](void *param)
    {
        const TickType_t xDelay = Signer.getCfg()->internal.stream_task_delay_ms / portTICK_PERIOD_MS;

        for (;;)
        {
            for (size_t i = 0; i < Signer.getCfg()->internal.fbdo_addr_list.size(); i++)
            {
                FirebaseData *_fbdo = addrTo<FirebaseData *>(Signer.getCfg()->internal.fbdo_addr_list[i]);

                if (_fbdo)
                {
                    if (_fbdo->session.rtdb.stream_task_enable && (_fbdo->_dataAvailableCallback || _fbdo->_timeoutCallback))
                    {

                        _this->readStream(_fbdo);

                        if (_fbdo->streamTimeout() && _fbdo->_timeoutCallback)
                            _fbdo->sendStreamToCB(_fbdo->session.response.code);

                        vTaskDelay(xDelay);
                    }
                }
            }

            vTaskDelay(xDelay);
        }

        Signer.getCfg()->internal.stream_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, name.c_str(), cfg->internal.stream_task_stack_size, NULL, cfg->internal.stream_task_priority, &cfg->internal.stream_task_handle, cfg->internal.stream_task_cpu_core);

#elif defined(ESP8266) || defined(FB_ENABLE_EXTERNAL_CLIENT)
    stream();
#if defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runStreamTask, this));
#endif
#endif
}

void FB_RTDB::stream()
{
#if !defined(ESP32)

    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
        return;

    for (size_t id = 0; id < cfg->internal.fbdo_addr_list.size(); id++)
    {

        FirebaseData *fbdo = addrTo<FirebaseData *>(cfg->internal.fbdo_addr_list[id]);

        if (fbdo)
        {
            if ((fbdo->_dataAvailableCallback || fbdo->_multiPathDataCallback || fbdo->_timeoutCallback))
            {
                readStream(fbdo);

                if (fbdo->streamTimeout() && fbdo->_timeoutCallback)
                    fbdo->sendStreamToCB(fbdo->session.response.code);
            }
        }
    }

#endif
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

#if defined(ESP8266)
void FB_RTDB::runErrorQueueTask()
{
    FirebaseConfig *cfg = Signer.getCfg();

    if (!cfg)
        return;

    for (size_t id = 0; id < cfg->internal.queue_addr_list.size(); id++)
    {
        FirebaseData *fbdo = addrTo<FirebaseData *>(cfg->internal.queue_addr_list[id]);

        if (fbdo)
        {

            if (fbdo->_queueInfoCallback)
                processErrorQueue(fbdo, fbdo->_queueInfoCallback);
            else
                processErrorQueue(fbdo, NULL);
        }
    }

    ut->set_scheduled_callback(std::bind(&FB_RTDB::runErrorQueueTask, this));
}
#endif

uint32_t FB_RTDB::getErrorQueueID(FirebaseData *fbdo)
{
    return fbdo->session.rtdb.queue_ID;
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
            if (buildRequest(fbdo, item.method, MB_StringPtr(toAddr(item.path), mb_string_sub_type_mb_string), MB_StringPtr(toAddr(item.payload), mb_string_sub_type_mb_string), item.dataType, item.subType, item.method == m_get ? item.address.dout : item.address.din, item.address.query, item.address.priority, MB_StringPtr(toAddr(item.etag), mb_string_sub_type_mb_string), item.async, _NO_QUEUE, item.blobSize, MB_StringPtr(toAddr(item.filename), mb_string_sub_type_mb_string), item.storageType))
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

#if defined(ESP32)
void FB_RTDB::beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback, size_t queueTaskStackSize)
#elif defined(ESP8266)
void FB_RTDB::beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback)
#endif
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    if (callback)
        fbdo->_queueInfoCallback = callback;
    else
        fbdo->_queueInfoCallback = NULL;

    fbdo->addQueueAddr();

#if defined(ESP32)

    MB_String taskName = fb_esp_pgm_str_72;
    taskName += fb_esp_pgm_str_114;
    taskName += fbdo->addr;

    if (queueTaskStackSize > QUEUE_TASK_STACK_SIZE)
        cfg->internal.queue_task_stack_size = queueTaskStackSize;
    else
        cfg->internal.queue_task_stack_size = QUEUE_TASK_STACK_SIZE;

    static FB_RTDB *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
        const TickType_t xDelay = Signer.getCfg()->internal.queue_task_delay_ms / portTICK_PERIOD_MS;
        for (;;)
        {
            for (size_t i = 0; i < Signer.getCfg()->internal.queue_addr_list.size(); i++)
            {
                FirebaseData *_fbdo = addrTo<FirebaseData *>(Signer.getCfg()->internal.queue_addr_list[i]);

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

        Signer.getCfg()->internal.queue_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName.c_str(), cfg->internal.queue_task_stack_size, NULL, cfg->internal.queue_task_priority, &cfg->internal.queue_task_handle, cfg->internal.queue_task_cpu_core);

#elif defined(ESP8266)
    ut->set_scheduled_callback(std::bind(&FB_RTDB::runErrorQueueTask, this));
#endif
}

void FB_RTDB::endAutoRunErrorQueue(FirebaseData *fbdo)
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    fbdo->_queueInfoCallback = NULL;
    fbdo->removeQueueAddr();
#if defined(ESP32)
    if (Signer.getCfg()->internal.queue_addr_list.size() == 0)
    {
        if (Signer.getCfg()->internal.queue_task_handle)
            vTaskDelete(Signer.getCfg()->internal.queue_task_handle);
        Signer.getCfg()->internal.queue_task_handle = NULL;
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

    int ret = ut->mbfs->open(_filename, mbfs_type storageType, mb_fs_open_mode_write);

    if (ret < 0)
    {
        fbdo->session.response.code = ret;
        return false;
    }

    // Close file and open later.
    ut->mbfs->close(mbfs_type storageType);

    if ((storageType == mem_storage_type_flash || storageType == mem_storage_type_sd) && !ut->mbfs->ready(mbfs_type storageType))
        return false;

    FirebaseJsonArray arr;

    // This is inefficient unless less memory usage than keep file opened
    // which causes the issue in ESP32 core 2.0.x
    ut->mbfs->open(_filename, mbfs_type storageType, mb_fs_open_mode_write);

    for (uint8_t i = 0; i < fbdo->_qMan.size(); i++)
    {
        if (!fbdo->_qMan._queueCollection)
            continue;

        QueueItem item = fbdo->_qMan._queueCollection->at(i);
        arr.clear();
        arr.add((uint8_t)item.dataType, (uint8_t)item.subType, (uint8_t)item.method, (uint8_t)item.storageType, (uint8_t)item.async);
        arr.add(item.address.din, item.address.dout, item.address.query, item.address.priority, item.blobSize);
        arr.add(item.path, item.payload, item.etag, item.filename);
        ut->mbfs->write(mbfs_type storageType, (uint8_t *)arr.raw(), strlen(arr.raw()));
    }

    ut->mbfs->close(mbfs_type storageType);
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
    MB_String _filename = filename;
    return ut->mbfs->remove(_filename, mbfs_type storageType);
}

uint8_t FB_RTDB::openErrorQueue(FirebaseData *fbdo, MB_StringPtr filename, fb_esp_mem_storage_type storageType, uint8_t mode)
{

    uint8_t count = 0;
    MB_String _filename = filename;

    int ret = ut->mbfs->open(_filename, mbfs_type storageType, mb_fs_open_mode_read);

    if (ret < 0)
    {
        fbdo->session.response.code = ret;
        return 0;
    }

    if (!ut->mbfs->available(mbfs_type storageType) || ut->mbfs->size(mbfs_type storageType) < 4)
    {
        ut->mbfs->close(mbfs_type storageType);
        return 0;
    }

    QueueItem item;

#if defined(MBFS_ESP32_SDFAT_ENABLED)

    if (storageType == mem_storage_type_flash)
    {
#if defined(MBFS_FLASH_FS)
        fs::File file = ut->mbfs->getFlashFile();
        count = readQueueFile(fbdo, file, item, mode);
#endif
    }
    else if (storageType == mem_storage_type_sd)
    {
#if defined(MBFS_SD_FS) && defined(CARD_TYPE_SD)
        MBFS_SD_FILE file = ut->mbfs->getSDFile();
        count = readQueueFileSdFat(fbdo, file, item, mode);
#endif
    }

#else

    if (storageType == mem_storage_type_flash)
    {
#if defined(MBFS_FLASH_FS)
        fs::File file = ut->mbfs->getFlashFile();
        count = readQueueFile(fbdo, file, item, mode);
#endif
    }
    else if (storageType == mem_storage_type_sd)
    {
#if defined(MBFS_SD_FS) && defined(CARD_TYPE_SD)
        MBFS_SD_FILE file = ut->mbfs->getSDFile();
        count = readQueueFile(fbdo, file, item, mode);
#endif
    }

#endif

    return count;
}

#if defined(ESP32) || defined(ESP8266)
uint8_t FB_RTDB::readQueueFile(FirebaseData *fbdo, fs::File &file, QueueItem &item, uint8_t mode)
{
    uint8_t count = 0;
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

bool FB_RTDB::mBackup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr nodePath, MB_StringPtr fileName, RTDB_DownloadProgressCallback callback)
{

    struct fb_esp_rtdb_request_info_t req;
    req.filename = fileName;
    ut->makePath(req.filename);
    req.path = nodePath;
    req.method = m_download;
    req.data.type = d_json;
    req.storageType = storageType;
    req.downloadCallback = callback;

    fbdo->session.rtdb.path = req.path;
    fbdo->session.rtdb.filename = req.filename;

    return handleRequest(fbdo, &req);
}

bool FB_RTDB::mRestore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, MB_StringPtr nodePath, MB_StringPtr fileName, RTDB_UploadProgressCallback callback)
{

    struct fb_esp_rtdb_request_info_t req;
    req.filename = fileName;
    ut->makePath(req.filename);
    req.path = nodePath;
    req.method = m_restore;
    req.data.type = d_json;
    req.storageType = storageType;
    req.uploadCallback = callback;

    fbdo->session.rtdb.path = req.path;
    fbdo->session.rtdb.filename = req.filename;

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

                if (req->data.value_subtype == mb_string_sub_type_arduino_string)
                {
                    String *ptr = addrTo<String *>(req->data.address.dout);
                    if (ptr)
                        *ptr = fbdo->to<String>();
                }
#if !defined(__AVR__)
                else if (req->data.value_subtype == mb_string_sub_type_std_string)
                {
                    std::string *ptr = addrTo<std::string *>(req->data.address.dout);
                    if (ptr)
                        *ptr = fbdo->to<std::string>();
                }
#endif
                else if (req->data.value_subtype == mb_string_sub_type_mb_string)
                {
                    MB_String *ptr = addrTo<MB_String *>(req->data.address.dout);
                    if (ptr)
                        *ptr = fbdo->to<MB_String>();
                }
                else if (req->data.value_subtype == mb_string_sub_type_chars)
                {
                    char *ptr = addrTo<char *>(req->data.address.dout);
                    if (ptr)
                    {
                        strcpy(ptr, fbdo->to<const char *>());
                        ptr[strlen(fbdo->to<const char *>())] = '\0';
                    }
                }
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

bool FB_RTDB::processRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    FirebaseConfig *cfg = Signer.getCfg();

    ut->idle();

    int pc = preRequestCheck(fbdo, req);
    if (pc < 0)
        return false;
    else if (pc == 0)
        return true;

    if (req->method != m_get)
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

    if (req->data.type == d_file && req->method == m_get)
    {
        fbdo->session.rtdb.filename = req->filename;

        size_t p1 = fbdo->session.rtdb.filename.find_last_of(pgm2Str(fb_esp_pgm_str_1));
        MB_String folder = (const char *)MBSTRING_FLASH_MCR("/");

        if (p1 != MB_String::npos && p1 != 0)
            folder = fbdo->session.rtdb.filename.substr(p1 - 1);

        if (fbdo->session.rtdb.storage_type == mem_storage_type_sd)
        {
#if defined(MBFS_SD_FS) && defined(CARD_TYPE_SD)
            if (!MBFS_SD_FS.exists(folder.c_str()))
                ut->createDirs(folder, (fb_esp_mem_storage_type)fbdo->session.rtdb.storage_type);
#endif
        }

        int sz = openFile(fbdo, req, mb_fs_open_mode_write);
        if (sz < 0)
            return false;

        folder.clear();
    }

    fbdo->session.rtdb.queue_ID = 0;

    uint8_t errCount = 0;
    uint8_t maxRetry = fbdo->session.rtdb.max_retry;
    if (maxRetry == 0)
        maxRetry = 1;

    for (int i = 0; i < maxRetry; i++)
    {
        ret = handleRequest(fbdo, req);

        setRefValue(fbdo, req);

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
        if (!req->queue && req->method != m_get)
            return ret;
    }

    if (ret)
    {
        if (cfg->rtdb.data_type_stricted && req->method == m_get && req->data.type != d_any)
        {
            if (req->data.type == d_integer || req->data.type == d_float || req->data.type == d_double)
                ret = fbdo->session.rtdb.resp_data_type == d_integer || fbdo->session.rtdb.resp_data_type == d_float || fbdo->session.rtdb.resp_data_type == d_double;
            else if (req->data.type == d_json)
                ret = fbdo->session.rtdb.resp_data_type == d_json || fbdo->session.rtdb.resp_data_type == d_null;
            else if (req->data.type == d_array)
                ret = fbdo->session.rtdb.resp_data_type == d_array || fbdo->session.rtdb.resp_data_type == d_null;
            else if (req->data.type != d_file && req->data.type != d_file_ota)
                ret = fbdo->session.rtdb.resp_data_type == req->data.type;
        }
    }
    return ret;
}

#if defined(ESP32)
void FB_RTDB::allowMultipleRequests(bool enable)
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
        return;

    cfg->internal.fb_multiple_requests = enable;
}
#endif

void FB_RTDB::rescon(FirebaseData *fbdo, const char *host, fb_esp_rtdb_request_info_t *req)
{
    if (req->method == m_stream)
    {
        if (strcmp(req->path.c_str(), fbdo->session.rtdb.stream_path.c_str()) != 0)
            fbdo->session.rtdb.stream_path_changed = true;
        else
            fbdo->session.rtdb.stream_path_changed = false;
    }

    if (fbdo->session.cert_updated || !fbdo->session.connected || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout || fbdo->session.rtdb.stream_path_changed || (req->method == m_stream && fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream) || (req->method != m_stream && fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream) || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->session.host = host;
    if (req->method == m_stream)
        fbdo->session.con_mode = fb_esp_con_mode_rtdb_stream;
    else
        fbdo->session.con_mode = fb_esp_con_mode_rtdb;

    if (fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream)
        fbdo->session.rtdb.stream_resume_millis = 0;
}

bool FB_RTDB::handleRequest(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    ut->idle();

    int pc = preRequestCheck(fbdo, req);
    if (pc < 0)
        return false;
    else if (pc == 0)
        return true;

    if (!ut->waitIdle(fbdo->session.response.code))
        return false;

    if (!fbdo->session.connected)
        fbdo->session.rtdb.async_count = 0;

    if ((fbdo->session.rtdb.async && !req->async) || fbdo->session.rtdb.async_count > Signer.config->async_close_session_max_request)
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

        if (req->method == m_stream)
        {
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();
                return false;
            }
        }
        else if (req->method == m_read_rules || req->method == m_download || ((req->data.type == d_file || req->data.type == d_file_ota) && req->method == m_get))
        {
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();

                RTDB_DownloadStatusInfo in;
                in.localFileName = req->filename;
                in.remotePath = req->path;
                in.status = fb_esp_rtdb_download_status_error;
                in.progress = 0;
                in.errorMsg = fbdo->errorReason().c_str();
                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                return false;
            }
            else
            {
                RTDB_DownloadStatusInfo in;
                in.localFileName = req->filename;
                in.remotePath = req->path;
                in.status = fb_esp_rtdb_download_status_complete;
                in.size = req->fileSize;
                in.progress = 100;
                sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
            }
        }
        else if (req->method == m_set_rules || req->method == m_restore || (req->data.type == d_file && req->method == m_put_nocontent))
        {
            if (!waitResponse(fbdo, req))
            {
                fbdo->closeSession();

                RTDB_UploadStatusInfo in;
                in.localFileName = req->filename;
                in.remotePath = req->path;
                in.status = fb_esp_rtdb_upload_status_error;
                in.progress = 0;
                in.errorMsg = fbdo->errorReason().c_str();
                sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);

                return false;
            }
            else
            {
                RTDB_UploadStatusInfo in;
                in.localFileName = req->filename;
                in.remotePath = req->path;
                in.status = fb_esp_rtdb_upload_status_complete;
                in.size = req->fileSize;
                in.progress = 100;
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
        RTDB_UploadStatusInfo in;
        in.localFileName = req->filename;
        in.remotePath = req->path;
        in.status = fb_esp_rtdb_upload_status_upload;
        in.progress = p;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
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
        RTDB_DownloadStatusInfo in;
        in.localFileName = req->filename;
        in.remotePath = req->path;
        in.status = fb_esp_rtdb_download_status_download;
        in.progress = p;
        in.size = req->fileSize;
        in.elapsedTime = fbdo->tcpClient.dataTime;
        sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
    }
}

void FB_RTDB::sendUploadCallback(FirebaseData *fbdo, RTDB_UploadStatusInfo &in, RTDB_UploadProgressCallback cb, RTDB_UploadStatusInfo *out)
{

    fbdo->session.rtdb.cbUploadInfo.status = in.status;
    fbdo->session.rtdb.cbUploadInfo.errorMsg = in.errorMsg;
    fbdo->session.rtdb.cbUploadInfo.progress = in.progress;
    fbdo->session.rtdb.cbUploadInfo.localFileName = in.localFileName;
    fbdo->session.rtdb.cbUploadInfo.remotePath = in.remotePath;
    fbdo->session.rtdb.cbUploadInfo.size = in.size;
    fbdo->session.rtdb.cbUploadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.rtdb.cbUploadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.rtdb.cbUploadInfo.errorMsg;
        out->status = fbdo->session.rtdb.cbUploadInfo.status;
        out->progress = fbdo->session.rtdb.cbUploadInfo.progress;
        out->localFileName = fbdo->session.rtdb.cbUploadInfo.localFileName;
    }
}

void FB_RTDB::sendDownloadCallback(FirebaseData *fbdo, RTDB_DownloadStatusInfo &in, RTDB_DownloadProgressCallback cb, RTDB_DownloadStatusInfo *out)
{

    fbdo->session.rtdb.cbDownloadInfo.status = in.status;
    fbdo->session.rtdb.cbDownloadInfo.errorMsg = in.errorMsg;
    fbdo->session.rtdb.cbDownloadInfo.progress = in.progress;
    fbdo->session.rtdb.cbDownloadInfo.localFileName = in.localFileName;
    fbdo->session.rtdb.cbDownloadInfo.remotePath = in.remotePath;
    fbdo->session.rtdb.cbDownloadInfo.size = in.size;
    fbdo->session.rtdb.cbDownloadInfo.elapsedTime = in.elapsedTime;

    if (cb)
        cb(fbdo->session.rtdb.cbDownloadInfo);

    if (out)
    {
        out->errorMsg = fbdo->session.rtdb.cbDownloadInfo.errorMsg;
        out->status = fbdo->session.rtdb.cbDownloadInfo.status;
        out->progress = fbdo->session.rtdb.cbDownloadInfo.progress;
        out->localFileName = fbdo->session.rtdb.cbDownloadInfo.localFileName;
    }
}

int FB_RTDB::preRequestCheck(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{

    fbdo->session.error.clear();
    int code = 1;

    if (!fbdo->reconnect())
        code = fbdo->session.response.code;

    if (!Signer.getCfg())
        code = FIREBASE_ERROR_UNINITIALIZED;
    else if (fbdo->session.rtdb.pause)
        code = 0;
    else if (!fbdo->tokenReady())
        code = FIREBASE_ERROR_TOKEN_NOT_READY;
    else if (req->path.length() == 0 || (Signer.getCfg()->database_url.length() == 0 && Signer.getCfg()->host.length() == 0) || (strlen(Signer.getToken()) == 0 && !Signer.getCfg()->signer.test_mode))
        code = FIREBASE_ERROR_MISSING_CREDENTIALS;
    else if (req->method != m_stream && (req->method == m_put || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent || req->task_type == fb_esp_rtdb_task_store_rules) && req->payload.length() == 0 && req->data.type != d_string && req->data.type != d_json && req->data.type != d_array && req->data.type != d_blob && req->data.type != d_file_ota)
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

    FirebaseConfig *cfg = Signer.getCfg();

    char *buf = nullptr;

    int len = 0;
    size_t toRead = 0;
    bool ret = false;

    ut->idle();

    rescon(fbdo, cfg->database_url.c_str(), req);

    if (req->method == m_stream)
    {
        fbdo->session.rtdb.stream_path.clear();
        ut->makePath(req->path);
        fbdo->session.rtdb.stream_path += req->path;
    }
    else
    {
        fbdo->session.rtdb.path.clear();
        fbdo->session.rtdb.resp_etag.clear();

        if (req->method != m_download && req->method != m_restore)
        {
            fbdo->session.rtdb.path.clear();
            ut->makePath(req->path);
            fbdo->session.rtdb.path += req->path;
        }

        fbdo->session.rtdb.data_tmo = false;
    }

    fbdo->session.max_payload_length = 0;

    fbdo->tcpClient.begin(cfg->database_url.c_str(), FIREBASE_PORT, &fbdo->session.response.code);

    if (req->task_type == fb_esp_rtdb_task_upload_rules)
    {
        int sz = openFile(fbdo, req, mb_fs_open_mode_read);
        if (sz < 0)
            return sz;
        req->fileSize = sz;
        len = sz;
    }

    // Prepare request header
    if (req->method != m_download && req->method != m_restore && req->data.type != d_file && req->data.type != d_file_ota)
        ret = sendHeader(fbdo, req);
    else
    {
        // for file data payload
        if (fbdo->session.rtdb.storage_type == mem_storage_type_flash)
        {
            if (!ut->mbfs->flashReady())
            {
                fbdo->session.error = fb_esp_pgm_str_164;
                fbdo->session.response.code = MB_FS_ERROR_FILE_IO_ERROR;
                return false;
            }
        }
        else if (fbdo->session.rtdb.storage_type == mem_storage_type_sd)
        {
            if (!ut->mbfs->sdReady())
            {
                fbdo->session.error = fb_esp_pgm_str_85;
                fbdo->session.response.code = MB_FS_ERROR_FILE_IO_ERROR;
                return false;
            }
        }

        if (req->method == m_download || req->method == m_restore)
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

        if (req->method == m_put_nocontent || req->method == m_post)
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

        ret = sendHeader(fbdo, req);
    }

    if (req->method == m_get_nocontent || req->method == m_patch_nocontent || (req->method == m_put_nocontent && (req->data.type == d_blob || req->data.type == d_file)))
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

    if (!fbdo->reconnect() || !ret)
        return false;

    int bufSize = cfg->rtdb.upload_buffer_size;
    if (bufSize < 512)
        bufSize = 512;

    if (bufSize > 1024 * 16)
        bufSize = 1024 * 16;

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
            MB_String s;
            s = fb_esp_pgm_str_92;
            fbdo->tcpClient.send(s.c_str());
            if (fbdo->session.response.code > 0)
            {
                if (fbdo->tcpClient.sendBase64(bufSize, blob, req->data.blobSize, true))
                {
                    s = fb_esp_pgm_str_3;
                    fbdo->tcpClient.send(s.c_str());
                }
            }
        }
    }
    else if (req->task_type == fb_esp_rtdb_task_upload_rules || req->method == m_restore || (req->data.type == d_file && (req->method == m_put_nocontent || req->method == m_post)))
    {

        RTDB_UploadStatusInfo in;
        in.localFileName = ut->mbfs->name(mbfs_type req->storageType);
        in.remotePath = req->path;
        in.status = fb_esp_rtdb_upload_status_init;
        in.size = req->fileSize;
        sendUploadCallback(fbdo, in, req->uploadCallback, req->uploadStatusInfo);
        int readLen = 0;

        if (req->data.type == d_file && (req->method == m_put_nocontent || req->method == m_post))
        {
            MB_String s = fb_esp_pgm_str_93;

            // make base64 signature with pad length encoded for later callulated decoded data size
            // used in download progress callback.
            if (ut->getBase64Padding(req->fileSize) == 1)
                s[1] = 'F'; // 1 padding -> "File,base64,
            else if (ut->getBase64Padding(req->fileSize) == 2)
                s[2] = 'I'; // 2 paddings -> "fIle,base64,

            fbdo->tcpClient.send(s.c_str());

            if (fbdo->session.response.code < 0)
                return false;

            sendBase64File(fbdo, bufSize, req->filename, (fb_esp_mem_storage_type)fbdo->session.rtdb.storage_type, req);

            buf = (char *)ut->newP(2);
            buf[0] = '"';
            buf[1] = '\0';
            fbdo->tcpClient.send(buf);
            ut->delP(&buf);

            if (fbdo->session.response.code < 0)
                return false;
        }
        else
        {
            // This is inefficient unless less memory usage than keep file opened
            // which causes the issue in ESP32 core 2.0.x
            MB_String filenme = ut->mbfs->name(mbfs_type req->storageType);
            ut->mbfs->close(mbfs_type req->storageType);
            ut->mbfs->open(filenme.c_str(), mbfs_type req->storageType, mb_fs_open_mode_read);

            while (len)
            {
                if (!fbdo->reconnect())
                {
                    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;
                    return false;
                }

                toRead = len;
                if ((int)toRead > bufSize)
                    toRead = bufSize - 1;

                buf = (char *)ut->newP(bufSize);
                int read = ut->mbfs->read(mbfs_type req->storageType, (uint8_t *)buf, toRead);
                readLen += read;

                reportUploadProgress(fbdo, req, readLen);

                if (read != (int)toRead)
                    break;

                buf[toRead] = '\0';

                fbdo->tcpClient.send(buf);
                ut->delP(&buf);

                if (fbdo->session.response.code < 0)
                    return false;

                len -= toRead;

                if (len <= 0)
                    break;
            }

            reportUploadProgress(fbdo, req, req->fileSize);
        }

        ut->mbfs->close(mbfs_type req->storageType);
    }

    fbdo->tcpClient.dataTime = millis() - ms;

    if (fbdo->session.response.code < 0)
        return false;

    fbdo->session.connected = fbdo->session.response.code > 0;

    return true;
}

void FB_RTDB::sendBase64File(FirebaseData *fbdo, size_t bufSize, const MB_String &filePath, fb_esp_mem_storage_type storageType, struct fb_esp_rtdb_request_info_t *req)
{
    size_t chunkSize = bufSize;
    size_t fbuffSize = 3;
    size_t byteAdd = 0;
    size_t byteSent = 0;

    unsigned char *buff = (unsigned char *)ut->newP(chunkSize);
    memset(buff, 0, chunkSize);

    size_t len = ut->mbfs->size(mbfs_type storageType);
    size_t fbuffIndex = 0;
    unsigned char *fbuff = (unsigned char *)ut->newP(3);
    int readLen = 0;

    // This is inefficient unless less memory usage than keep file opened
    // which causes the issue in ESP32 core 2.0.x
    MB_String filenme = ut->mbfs->name(mbfs_type req->storageType);
    ut->mbfs->close(mbfs_type req->storageType);
    ut->mbfs->open(filenme.c_str(), mbfs_type req->storageType, mb_fs_open_mode_read);

    while (ut->mbfs->available(mbfs_type storageType))
    {
        memset(fbuff, 0, fbuffSize);
        if (len - fbuffIndex >= 3)
        {

            size_t rd = ut->mbfs->read(mbfs_type storageType, fbuff, 3);

            readLen += rd;

            reportUploadProgress(fbdo, req, readLen);

            if (rd != 3)
                break;

            buff[byteAdd++] = fb_esp_base64_table[fbuff[0] >> 2];
            buff[byteAdd++] = fb_esp_base64_table[((fbuff[0] & 0x03) << 4) | (fbuff[1] >> 4)];
            buff[byteAdd++] = fb_esp_base64_table[((fbuff[1] & 0x0f) << 2) | (fbuff[2] >> 6)];
            buff[byteAdd++] = fb_esp_base64_table[fbuff[2] & 0x3f];

            if (byteAdd >= chunkSize - 4)
            {
                byteSent += byteAdd;
                fbdo->tcpClient.write(buff, byteAdd);
                memset(buff, 0, chunkSize);
                byteAdd = 0;
            }

            fbuffIndex += 3;
        }
        else
        {

            if (len - fbuffIndex == 1)
            {
                int r = ut->mbfs->read(mbfs_type storageType);
                if (r > -1)
                    fbuff[0] = (uint8_t)r;
                readLen++;

                reportUploadProgress(fbdo, req, readLen);
            }
            else if (len - fbuffIndex == 2)
            {
                int r = ut->mbfs->read(mbfs_type storageType);
                if (r > -1)
                    fbuff[0] = (uint8_t)r;
                r = ut->mbfs->read(mbfs_type storageType);
                if (r > -1)
                    fbuff[1] = (uint8_t)r;
                readLen += 2;

                reportUploadProgress(fbdo, req, readLen);
            }

            break;
        }
    }

    if (byteAdd > 0)
        fbdo->tcpClient.write(buff, byteAdd);

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

        fbdo->tcpClient.write(buff, byteAdd);
    }

    ut->delP(&buff);
    ut->delP(&fbuff);
}

bool FB_RTDB::waitResponse(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req)
{
#if defined(ESP32)
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    // if currently perform stream payload handling process, skip it.
    if (cfg->internal.fb_processing && fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
        return true;

    // set the blocking flag
    cfg->internal.fb_processing = true;
    bool ret = handleResponse(fbdo, req);
    // reset the blocking flag
    cfg->internal.fb_processing = false;

    return ret;
#else
    return handleResponse(fbdo, req);
#endif
}

void FB_RTDB::waitRxReady(FirebaseData *fbdo, unsigned long &dataTime)
{
    int available = fbdo->tcpClient.available();
    if (available == 0)
        dataTime = millis();

    while (available == 0 && fbdo->reconnect(dataTime))
    {
        ut->idle();
        available = fbdo->tcpClient.available();
    }
}

int FB_RTDB::openFile(FirebaseData *fbdo, fb_esp_rtdb_request_info_t *req, mb_fs_open_mode mode, bool closeSession)
{
    int sz = 0;

    if (req->method == m_download)
        sz = ut->mbfs->open(req->filename, mbfs_type req->storageType, mb_fs_open_mode_write);
    else if (req->method == m_restore)
        sz = ut->mbfs->open(req->filename, mbfs_type req->storageType, mb_fs_open_mode_read);
    else
        sz = ut->mbfs->open(req->filename, mbfs_type req->storageType, mode);

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

    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

    ut->idle();

    if (fbdo->session.rtdb.pause)
        return true;

    if (!fbdo->reconnect())
        return false;

    if (!fbdo->session.connected || !fbdo->tcpClient.connected())
    {
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

        if (fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
            fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);

        return false;
    }

    unsigned long dataTime = millis();

    char *pChunk = nullptr;
    char *temp = nullptr;
    MB_String header;
    MB_String payload;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int chunkBufSize = fbdo->tcpClient.available();
    bool redirect = false;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->session.resp_size;

#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    int base64PadLenTail = 0;      // pad length from tail checking
    int base64PadLenSignature = 0; // pad length from signature checking
#endif

    struct fb_esp_auth_token_error_t error;
    error.code = -1;

    if (fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_UNDEFINED)
        fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->session.content_length = -1;
    fbdo->session.payload_length = 0;

    fbdo->session.rtdb.push_name.clear();
    fbdo->session.rtdb.data_mismatch = false;
    fbdo->session.chunked_encoding = false;
    fbdo->session.buffer_ovf = false;
    int downloadByteLen = 0;

    bool downloadOTA = req->data.type == d_file_ota;

    if (req->task_type == fb_esp_rtdb_task_download_rules)
    {
        if (openFile(fbdo, req, mb_fs_open_mode_write, true) < 0)
            return false;
    }

    if (fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream)
    {
        if (fbdo->session.rtdb.async)
        {

#if defined(ESP32)
            chunkBufSize = fbdo->tcpClient.available();
            if (chunkBufSize > 0)
            {
                char buf[chunkBufSize];
                fbdo->tcpClient.readBytes(buf, chunkBufSize);
            }
#endif

            if (!fbdo->tcpClient.connected() || !fbdo->tcpClient.connected())
            {
                fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;
                fbdo->session.connected = false;
            }

            fbdo->tcpClient.flush();
            return fbdo->session.connected;
        }
        else
        {
            while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
            {
                if (!fbdo->reconnect(dataTime) || !fbdo->tcpClient.connected())
                {
                    fbdo->session.error.clear();
                    fbdo->session.response.code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
                    return false;
                }
                chunkBufSize = fbdo->tcpClient.available();
                ut->idle();
            }
        }
    }

    dataTime = millis();

    if (req->task_type == fb_esp_rtdb_task_download_rules || req->method == m_download)
    {
        // This is inefficient unless less memory usage than keep file opened
        // which causes the issue in ESP32 core 2.0.x
        MB_String filenme = ut->mbfs->name(mbfs_type req->storageType);
        ut->mbfs->close(mbfs_type req->storageType);
        ut->mbfs->open(filenme.c_str(), mbfs_type req->storageType, mb_fs_open_mode_write);
    }

    while (chunkBufSize > 0)
    {
        ut->idle();

        if (!fbdo->reconnect(dataTime) || !fbdo->tcpClient.connected())
        {
            fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

            if (fbdo->session.con_mode == fb_esp_con_mode_rtdb_stream)
                fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);

            return false;
        }

        chunkBufSize = fbdo->tcpClient.available();

        if (response.contentLen > 0 && fbdo->session.payload_length < (size_t)response.contentLen)
        {
            waitRxReady(fbdo, dataTime);
            chunkBufSize = fbdo->tcpClient.available();
        }

        if (chunkBufSize <= 0)
            break;

        if (chunkBufSize > 0)
        {
            if (pChunkIdx == 0)
            {
                if (chunkBufSize > defaultChunkSize + (int)strlen_P(fb_esp_pgm_str_93))
                    chunkBufSize = defaultChunkSize + strlen_P(fb_esp_pgm_str_93); // plus file header length for later base64 decoding
            }
            else
            {
                if (chunkBufSize > defaultChunkSize)
                    chunkBufSize = defaultChunkSize;
            }

            if (chunkIdx == 0)
            {
                if (!fbdo->tcpClient.connected())
                    break;

                // the first chunk can be stream event data (no header) or http response header
                fbdo->tcpClient.readLine(header);
                int pos = 0;

                response.noEvent = fbdo->session.con_mode != fb_esp_con_mode_rtdb_stream;

                temp = ut->getHeader(header.c_str(), fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                ut->idle();
                dataTime = millis();
                if (temp)
                {
                    // http response header with http response code
                    isHeader = true;
                    response.httpCode = atoi(temp);
                    fbdo->session.response.code = response.httpCode;
                    ut->delP(&temp);
                }
                else
                {
                    // stream payload data
                    payload = header;
                    fbdo->session.payload_length = header.length();
                    if (fbdo->session.max_payload_length < fbdo->session.payload_length)
                        fbdo->session.max_payload_length = fbdo->session.payload_length;
                }
            }
            else
            {
                ut->idle();
                dataTime = millis();
                // the next chunk data can be the remaining http header
                if (isHeader)
                {
                    // read one line of next header field until the empty header has found
                    temp = (char *)ut->newP(chunkBufSize + 10);
                    bool headerEnded = false;
                    int readLen = 0;
                    if (temp)
                    {
                        if (!fbdo->tcpClient.connected())
                        {
                            ut->delP(&temp);
                            break;
                        }

                        readLen = fbdo->tcpClient.readLine(temp, chunkBufSize);

                        // check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (temp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (temp[0] == '\r' && temp[1] == '\n')
                                headerEnded = true;
                    }

                    if (headerEnded)
                    {
                        // parse header string to get the header field
                        isHeader = false;
                        ut->parseRespHeader(header.c_str(), response);

                        fbdo->session.http_code = response.httpCode;

                        fbdo->session.rtdb.resp_etag = response.etag;

                        if (response.httpCode == 401)
                            Signer.authenticated = false;
                        else if (response.httpCode < 300)
                            Signer.authenticated = true;

                        if (ut->strposP(response.contentType.c_str(), fb_esp_pgm_str_9, 0) > -1)
                        {
                            chunkBufSize = fbdo->tcpClient.available();

                            if (chunkBufSize == 0)
                            {
                                if (temp)
                                    ut->delP(&temp);
                                header.clear();
                                while (chunkBufSize == 0)
                                {
                                    ut->idle();
                                    if (!fbdo->reconnect(dataTime) || !fbdo->tcpClient.connected())
                                    {
                                        fbdo->closeSession();
                                        fbdo->sendStreamToCB(FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT);
                                        break;
                                    }
                                    chunkBufSize = fbdo->tcpClient.available();
                                }
                                fbdo->session.rtdb.new_stream = false;
                                fbdo->session.response.code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
                                return false;
                            }
                        }

                        header.clear();

                        // error in request or server
                        if (response.httpCode >= 400)
                        {
                            // non-JSON response error handling
                            if (ut->strposP(response.contentType.c_str(), fb_esp_pgm_str_74, 0) < 0)
                            {
                                if (fbdo->tcpClient.connected())
                                {
                                    fbdo->session.error.clear();

                                    fbdo->tcpClient.flush();

                                    if (temp)
                                        ut->delP(&temp);
                                    return false;
                                }
                                else
                                {
                                    if (temp)
                                        ut->delP(&temp);
                                }
                            }
                        }

                        if (fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT || fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY || fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_FOUND)
                        {
                            if (response.location.length() > 0)
                            {
                                fbdo->session.rtdb.redirect_url = response.location;
                                redirect = true;
                                if (fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_FOUND)
                                    fbdo->session.rtdb.redirect = 1;
                                else
                                    fbdo->session.rtdb.redirect = 2;
                            }
                        }

                        if (ut->stringCompare(response.connection.c_str(), 0, fb_esp_pgm_str_11))
                            fbdo->session.rtdb.http_resp_conn_type = fb_esp_http_connection_type_keep_alive;
                        else
                            fbdo->session.rtdb.http_resp_conn_type = fb_esp_http_connection_type_close;

                        fbdo->session.chunked_encoding = response.isChunkedEnc;

                        if (req->method == m_download)
                            fbdo->session.rtdb.file_size = response.contentLen;
                    }
                    else
                    {
                        if (temp)
                        {
                            // accumulate the remaining header field
                            header += temp;
                        }
                    }
                    if (temp)
                        ut->delP(&temp);
                }
                else
                {

                    // the next chuunk data is the payload
                    if (!response.noContent && !fbdo->session.buffer_ovf)
                    {
                        pChunkIdx++;

                        pChunk = (char *)ut->newP(chunkBufSize + 10);

                        if (!pChunk)
                            break;

                        // read the avilable data
                        int readLen = 0;

                        if (!fbdo->tcpClient.connected())
                        {
                            ut->delP(&pChunk);
                            break;
                        }

                        // chunk transfer encoding?
                        if (response.isChunkedEnc)
                            readLen = fbdo->tcpClient.readChunkedData(pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                        else
                        {
                            int avail = fbdo->tcpClient.available();
                            if (avail < chunkBufSize)
                                chunkBufSize = avail;
                            if (avail > 0)
                                readLen = fbdo->tcpClient.readBytes(pChunk, chunkBufSize);

                            if (readLen < defaultChunkSize)
                            {
                                if (response.contentLen > 0 && fbdo->session.payload_length + readLen < (size_t)response.contentLen)
                                {
                                    fbdo->checkOvf(payload.length() + readLen, response);

                                    if (!fbdo->session.buffer_ovf)
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

                                    readLen = readLen + fbdo->tcpClient.readBytes(pChunk, chunkBufSize);
                                }
                            }
                        }

                        if (readLen > 0)
                        {
                            fbdo->session.payload_length += readLen;
                            if (fbdo->session.max_payload_length < fbdo->session.payload_length)
                                fbdo->session.max_payload_length = fbdo->session.payload_length;

                            fbdo->checkOvf(payload.length() + readLen, response);

                            if (!fbdo->session.buffer_ovf)
                                payload += pChunk;
                        }

                        if (!fbdo->session.rtdb.data_tmo && !fbdo->session.buffer_ovf)
                        {
                            // try to parse the payload
                            if (response.dataType == 0 && !response.isEvent && !response.noContent)
                            {
                                bool getOfs = req->data.type == d_blob || req->method == m_download || ((req->data.type == d_file || downloadOTA || req->data.type == d_any) && req->method == m_get);
                                ut->parseRespPayload(payload.c_str(), response, getOfs);

                                fbdo->session.rtdb.resp_data_type = response.dataType;
                                fbdo->session.content_length = response.payloadLen;

                                fbdo->session.error = response.fbError;
                                if (req->method == m_download || req->method == m_restore)
                                    fbdo->session.error = response.fbError;

                                if (req->method == m_download && response.dataType != d_json)
                                {
                                    fbdo->session.response.code = FIREBASE_ERROR_EXPECTED_JSON_DATA;

                                    fbdo->session.error = fb_esp_pgm_str_185;

                                    header.clear();
                                    payload.clear();
                                    ut->mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);
                                    fbdo->closeSession();
                                    return false;
                                }

                                if (req->method == m_get)
                                {
                                    if (ut->stringCompare(fbdo->session.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151))
                                    {
                                        fbdo->session.response.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                                        fbdo->session.rtdb.path_not_found = true;
                                    }
                                }
                            }

                            // in case of the payload data type is file, decode and write stream to temp file
                            if (req->task_type == fb_esp_rtdb_task_download_rules || response.dataType == d_file || (req->method == m_download || ((req->data.type == d_file || downloadOTA) && req->method == m_get)))
                            {

                                int ofs = 0;
                                int len = readLen;

                                if (req && downloadByteLen == 0)
                                {

                                    req->fileSize = response.contentLen;
                                    RTDB_DownloadStatusInfo in;
                                    in.localFileName = ut->mbfs->name(mbfs_type req->storageType);
                                    in.remotePath = req->path;
                                    in.status = fb_esp_rtdb_download_status_init;

                                    if (!fbdo->session.rtdb.path_not_found)
                                        in.size = req->fileSize;
                                    if (req->fileSize > 0 && (req->data.type == d_file || req->data.type == d_file_ota))
                                    {
                                        // decoded data size (with padding)
                                        in.size = (req->fileSize - strlen_P(fb_esp_pgm_str_93)) * 3 / 4;
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
                                        // check for pad length from base64 signature
                                        if (payload[1] == 'F')
                                            base64PadLenSignature = 1;
                                        else if (payload[2] == 'I')
                                            base64PadLenSignature = 2;
                                        // decoded data size (without padding)
                                        in.size -= base64PadLenSignature;
#endif
                                    }
                                    sendDownloadCallback(fbdo, in, req->downloadCallback, req->downloadStatusInfo);
                                }

                                if (req->task_type == fb_esp_rtdb_task_download_rules || req->method == m_download || ((req->data.type == d_file || downloadOTA) && req->method == m_get))
                                {
                                    if (ut->stringCompare(fbdo->session.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151))
                                    {
                                        fbdo->session.response.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                                        error.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                                        fbdo->session.rtdb.path_not_found = true;
                                    }

                                    if (!fbdo->session.rtdb.path_not_found)
                                    {

                                        if (req->task_type == fb_esp_rtdb_task_download_rules || req->method == m_download)
                                        {
                                            int write = ut->mbfs->write(mbfs_type req->storageType, (uint8_t *)payload.c_str(), readLen);
                                            payload.clear();

                                            if (write != readLen)
                                            {
                                                fbdo->session.response.code = MB_FS_ERROR_FILE_IO_ERROR;
                                                fbdo->closeSession();
                                                break;
                                            }

                                            downloadByteLen += len;

                                            reportDownloadProgress(fbdo, req, downloadByteLen);

                                            int avail = fbdo->tcpClient.available();

                                            if (avail == 0)
                                            {
                                                fbdo->closeSession();
                                                break;
                                            }
                                        }
                                        else
                                        {

                                            if (pChunkIdx == 1)
                                            {
                                                len = payload.size() - response.payloadOfs; // payloadOfs must be 13 for signature len
                                                ofs = response.payloadOfs;

                                                if (downloadOTA)
                                                {

#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
                                                    // size may include pad which we don't know from the first chunk until the last chunk
                                                    int decodedSize = (3 * (response.contentLen - response.payloadOfs - 1) / 4);

                                                    if (base64PadLenSignature > 0)
                                                    {
                                                        // known padding from signature
                                                        decodedSize -= base64PadLenSignature;
                                                    }
#if defined(ESP32)
                                                    error.code = 0;
                                                    if (!Update.begin(decodedSize))
                                                        error.code = FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE;
#elif defined(ESP8266)
                                                    error.code = fbdo->tcpClient.beginUpdate(decodedSize, false);

#endif
#endif
                                                }
                                            }
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
                                            base64PadLenTail = ut->trimLastChunkBase64(payload, len);

#endif
                                            downloadByteLen += len + ofs;

                                            reportDownloadProgress(fbdo, req, downloadByteLen);

                                            if (req->data.type == d_file)
                                            {
                                                ut->decodeBase64Stream(payload.c_str() + ofs, len, (fb_esp_mem_storage_type)fbdo->session.rtdb.storage_type);
                                            }
                                            else if (downloadOTA)
                                            {
                                                dataTime = millis();

#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
                                                if (error.code == 0)
                                                    ut->decodeBase64OTA(payload.c_str() + ofs, len, error.code);
#endif
                                            }
                                        }
                                    }
                                }
                                else
                                {
#if defined(MBFS_FLASH_FS)
                                    if (!ut->mbfs->getFlashFile())
                                    {
                                        fbdo->session.rtdb.storage_type = mem_storage_type_flash;

                                        int sz = ut->mbfs->open(pgm2Str(fb_esp_pgm_str_184), mbfs_type fbdo->session.rtdb.storage_type, mb_fs_open_mode_write);
                                        if (sz < 0)
                                            fbdo->session.response.code = sz;

                                        readLen = payload.length() - response.payloadOfs;
                                        ofs = response.payloadOfs;
                                    }

                                    downloadByteLen += len;

                                    reportDownloadProgress(fbdo, req, downloadByteLen);

                                    if (ut->mbfs->getFlashFile())
                                    {
                                        ut->trimLastChunkBase64(payload, len);
                                        ut->decodeBase64Stream(payload.c_str() + ofs, len, (fb_esp_mem_storage_type)fbdo->session.rtdb.storage_type);
                                    }

#endif
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
                        if (!fbdo->tcpClient.connected())
                            break;
                        // read all the rest data
                        fbdo->tcpClient.flush();
                    }
                }
            }

            chunkIdx++;
        }
    }

    header.clear();

    if (downloadByteLen > 0)
        reportDownloadProgress(fbdo, req, response.contentLen);

    if (downloadOTA)
    {
        payload.clear();

#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)

        // write extra pad
        if (base64PadLenTail > 0 && base64PadLenSignature == 0)
        {
            uint8_t pad[base64PadLenTail];
            memset(pad, 0, base64PadLenTail);
            Update.write(pad, base64PadLenTail);
        }

        if (error.code == 0)
        {
            if (!Update.end())
                error.code = FIREBASE_ERROR_FW_UPDATE_END_FAILED;
        }

        if (error.code != 0)
            fbdo->session.response.code = error.code;

        return fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK;

#endif
    }

    if (req->task_type == fb_esp_rtdb_task_download_rules)
    {
        payload.clear();
        ut->mbfs->close(mbfs_type req->storageType);
        return fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK;
    }

    if (!fbdo->session.rtdb.data_tmo && !fbdo->session.buffer_ovf)
    {
        // parse the payload
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
                    MB_VECTOR<MB_String> payloadList;
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
                    fbdo->session.rtdb.data_millis = millis();
                    fbdo->session.rtdb.data_tmo = false;
                }
                else
                {
                    fbdo->session.rtdb.data_millis = 0;
                    fbdo->session.rtdb.data_tmo = true;
                    fbdo->closeSession();
                }

                return valid;
            }
            else
            {

                // the payload ever parsed?
                if (response.dataType == 0 && !response.noContent)
                {
                    if (req->data.type == d_blob || req->method == m_download || ((req->data.type == d_file || downloadOTA) && req->method == m_get))
                        ut->parseRespPayload(payload.c_str(), response, true);
                    else
                        ut->parseRespPayload(payload.c_str(), response, false);

                    fbdo->session.error = response.fbError;
                }

                fbdo->session.rtdb.resp_data_type = response.dataType;
                fbdo->session.content_length = response.payloadLen;

                if (fbdo->session.rtdb.resp_data_type == d_blob)
                {
                    if (!fbdo->session.rtdb.blob)
                    {
                        fbdo->session.rtdb.isBlobPtr = true;
                        fbdo->session.rtdb.blob = new MB_VECTOR<uint8_t>();
                    }
                    else
                        MB_VECTOR<uint8_t>().swap(*fbdo->session.rtdb.blob);
                    fbdo->session.rtdb.raw.clear();
                    ut->decodeBase64Str((const char *)payload.c_str() + response.payloadOfs, *fbdo->session.rtdb.blob);
                }
                else if (fbdo->session.rtdb.resp_data_type == d_file)
                {
                    ut->mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);
                    fbdo->session.rtdb.raw.clear();
                }

                if (req->method == m_set_rules)
                {
                    if (ut->stringCompare(payload.c_str(), 0, fb_esp_pgm_str_104))
                        payload.clear();
                }

                if (fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK || fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED)
                {

                    if (req->method != m_set_rules)
                    {
                        if (fbdo->session.rtdb.resp_data_type != d_blob && fbdo->session.rtdb.resp_data_type != d_file && fbdo->session.rtdb.resp_data_type != d_file_ota)
                        {
                            handlePayload(fbdo, response, payload.c_str());

                            if (fbdo->session.rtdb.priority_val_flag)
                            {
                                char *path = (char *)ut->newP(fbdo->session.rtdb.path.length());
                                if (path)
                                {
                                    strncpy(path, fbdo->session.rtdb.path.c_str(), fbdo->session.rtdb.path.length() - strlen_P(fb_esp_pgm_str_156));
                                    fbdo->session.rtdb.path = path;
                                    ut->delP(&path);
                                }
                            }

                            // Push (POST) data?
                            if (req->method == m_post)
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
                }

                if (cfg->rtdb.data_type_stricted && req->method == m_get && req->data.type != d_timestamp && !response.noContent && response.httpCode < 400)
                {
                    bool _reqType = req->data.type == d_integer || req->data.type == d_float || req->data.type == d_double;
                    bool _respType = fbdo->session.rtdb.resp_data_type == d_integer || fbdo->session.rtdb.resp_data_type == d_float || fbdo->session.rtdb.resp_data_type == d_double;

                    if (req->data.type == fbdo->session.rtdb.resp_data_type || (_reqType && _respType) || (fbdo->session.rtdb.priority > 0 && fbdo->session.rtdb.resp_data_type == d_json))
                        fbdo->session.rtdb.data_mismatch = false;
                    else if (req->data.type != d_any)
                    {
                        fbdo->session.rtdb.data_mismatch = true;
                        fbdo->session.response.code = FIREBASE_ERROR_DATA_TYPE_MISMATCH;
                    }
                }
            }
        }
    }
    if (fbdo->session.rtdb.no_content_req || response.noContent)
    {

        if (ut->stringCompare(fbdo->session.rtdb.resp_etag.c_str(), 0, fb_esp_pgm_str_151) && response.noContent)
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

    ut->mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);

    payload.clear();

    if (!redirect)
    {
        if (fbdo->session.rtdb.redirect == 1 && fbdo->session.rtdb.redirect_count > 1)
            fbdo->session.rtdb.redirect_url.clear();
    }
    else
    {

        fbdo->session.rtdb.redirect_count++;

        if (fbdo->session.rtdb.redirect_count > MAX_REDIRECT)
        {
            fbdo->session.rtdb.redirect = 0;
            fbdo->session.response.code = FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED;
        }
        else
        {
            struct fb_esp_url_info_t uinfo;
            ut->getUrlInfo(fbdo->session.rtdb.redirect_url, uinfo);
            struct fb_esp_rtdb_request_info_t _req;
            _req.method = req->method;
            _req.data.type = req->data.type;
            _req.data.address.priority = toAddr(fbdo->session.rtdb.priority);
            _req.path = uinfo.uri.c_str();

            if (sendRequest(fbdo, &_req))
                return waitResponse(fbdo, &_req);
        }
    }

    if (response.httpCode >= 400)
    {
        fbdo->session.error = response.fbError;
        fbdo->session.response.code = response.httpCode;
    }

    return fbdo->session.response.code == FIREBASE_ERROR_HTTP_CODE_OK;
}

void FB_RTDB::sendCB(FirebaseData *fbdo)
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }

    // prevent the data available and stream data changed flags reset by
    // streamAvailable without stream callbacks assigned.
    if (!fbdo->_dataAvailableCallback && !fbdo->_multiPathDataCallback)
        return;

    if (!fbdo->streamAvailable())
        return;

    // to allow other subsequence request which can be occurred in the user stream
    // callback
    cfg->internal.fb_processing = false;

    if (fbdo->_dataAvailableCallback)
    {
        FIREBASE_STREAM_CLASS s;
        s.begin(ut, &fbdo->session.rtdb.stream);

        if (!fbdo->session.jsonPtr)
            fbdo->session.jsonPtr = new FirebaseJson();

        if (!fbdo->session.arrPtr)
            fbdo->session.arrPtr = new FirebaseJsonArray();

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
        s.begin(ut, &fbdo->session.rtdb.stream);
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

void FB_RTDB::splitStreamPayload(const char *payloads, MB_VECTOR<MB_String> &payload)
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
                char *temp = (char *)ut->newP(len + 10);
                if (temp)
                {
                    strncpy(temp, payloads + pos1, len);
                    MB_String s = temp;
                    ut->delP(&temp);
                    payload.push_back(s);
                }
            }
        }
    }
}

void FB_RTDB::parseStreamPayload(FirebaseData *fbdo, const char *payload)
{
    struct server_response_data_t response;

    ut->parseRespPayload(payload, response, false);

    fbdo->session.rtdb.resp_data_type = response.dataType;
    fbdo->session.content_length = response.payloadLen;

    if (fbdo->session.jsonPtr)
        fbdo->session.jsonPtr->clear();

    if (fbdo->session.arrPtr)
        fbdo->session.arrPtr->clear();

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
        ut->decodeBase64Str((const char *)payload + response.payloadOfs, *fbdo->session.rtdb.blob);
    }
    else if (fbdo->session.rtdb.resp_data_type == d_file)
    {
        ut->mbfs->close(mbfs_type fbdo->session.rtdb.storage_type);
        fbdo->session.rtdb.raw.clear();
    }

    if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_15) || ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_16))
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
        if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_11))
        {
            if (fbdo->_timeoutCallback)
                fbdo->sendStreamToCB(0);
        }

        // Firebase cancel and auth_revoked events
        else if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_109) || ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_110))
        {
            fbdo->session.rtdb.event_type = response.eventType;
            // make stream available status
            fbdo->session.rtdb.stream_data_changed = true;
            fbdo->session.rtdb.data_available = true;

            // We need to close the current session due to the token was already expired.
            if (ut->stringCompare(response.eventType.c_str(), 0, fb_esp_pgm_str_110))
                fbdo->closeSession();
        }
    }
}

void FB_RTDB::handlePayload(FirebaseData *fbdo, struct server_response_data_t &response, const char *payload)
{

    fbdo->session.rtdb.raw.clear();

    if (fbdo->session.jsonPtr)
        fbdo->session.jsonPtr->clear();

    if (fbdo->session.arrPtr)
        fbdo->session.arrPtr->clear();

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

        uint16_t crc = ut->calCRC(fbdo->session.rtdb.raw.c_str());
        response.dataChanged = fbdo->session.rtdb.data_crc != crc;
        fbdo->session.rtdb.data_crc = crc;
    }
}

int FB_RTDB::getPayloadLen(fb_esp_rtdb_request_info_t *req)
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
        else if (req->fileSize > 0 && req->data.address.priority == 0)
        {
            len = req->fileSize;
            if (req->data.type == d_file || req->data.type == d_file_ota || req->data.type == d_blob)
                len = (4 * ceil(req->fileSize / 3.0)) + strlen_P(fb_esp_pgm_str_92) + 1;
        }
        else if (req->method == m_restore)
            len = req->fileSize;
    }
    return len;
}

bool FB_RTDB::sendHeader(FirebaseData *fbdo, struct fb_esp_rtdb_request_info_t *req)
{
    FirebaseConfig *cfg = Signer.getCfg();

    fb_esp_method http_method = m_put;
    fbdo->session.rtdb.shallow_flag = false;
    fbdo->session.rtdb.priority_val_flag = false;

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

    MB_String header;

    if (req->method == m_stream)
        header = fb_esp_pgm_str_22;
    else
    {
        if (req->method == m_put || req->method == m_put_nocontent || req->method == m_set_priority || req->method == m_set_rules)
        {
            http_method = m_put;
            if (fbdo->session.classic_request)
                header = fb_esp_pgm_str_24;
            else
                header = fb_esp_pgm_str_23;
        }
        else if (req->method == m_post)
        {
            http_method = m_post;
            header = fb_esp_pgm_str_24;
        }
        else if (req->method == m_get || req->method == m_get_nocontent || req->method == m_get_shallow || req->method == m_get_priority || req->method == m_download || req->method == m_read_rules)
        {
            http_method = m_get;
            header = fb_esp_pgm_str_25;
        }
        else if (req->method == m_patch || req->method == m_patch_nocontent || req->method == m_restore)
        {
            http_method = m_patch;
            header = fb_esp_pgm_str_26;
        }
        else if (req->method == m_delete)
        {
            http_method = m_delete;
            if (fbdo->session.classic_request)
                header = fb_esp_pgm_str_24;
            else
                header = fb_esp_pgm_str_27;
        }
        header += fb_esp_pgm_str_6;
    }

    ut->makePath(req->path);
    header += req->path;

    if (req->method == m_patch || req->method == m_patch_nocontent)
        header += fb_esp_pgm_str_1;

    bool appendAuth = false;
    bool hasQueryParams = false;

    if (fbdo->session.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        ut->getUrlInfo(fbdo->session.rtdb.redirect_url, uinfo);
        if (uinfo.auth.length() == 0)
            appendAuth = true;
    }
    else
        appendAuth = true;

    if (appendAuth)
    {
        if (Signer.getTokenType() == token_type_oauth2_access_token || cfg->signer.test_mode)
            header += fb_esp_pgm_str_238;
        else
        {
            header += fb_esp_pgm_str_2;
            hasQueryParams = true;
        }

        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        if (Signer.getTokenType() != token_type_oauth2_access_token && !cfg->signer.test_mode)
            fbdo->tcpClient.send(cfg->internal.auth_token.c_str());

        if (fbdo->session.response.code < 0)
            return false;
    }

    if (fbdo->session.rtdb.read_tmo > 0)
    {
        header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
        hasQueryParams = true;

        header += fb_esp_pgm_str_158;
        header += fbdo->session.rtdb.read_tmo;
        header += fb_esp_pgm_str_159;
    }

    if (fbdo->session.rtdb.write_limit.length() > 0)
    {
        header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
        hasQueryParams = true;

        header += fb_esp_pgm_str_160;
        header += fbdo->session.rtdb.write_limit;
    }

    if (req->method == m_get_shallow)
    {
        header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
        hasQueryParams = true;

        header += fb_esp_pgm_str_155;
        fbdo->session.rtdb.shallow_flag = true;
    }

    QueryFilter *query = req->data.address.query > 0 ? addrTo<QueryFilter *>(req->data.address.query) : nullptr;

    bool hasQuery = false;

    if (req->method == m_get && query)
    {
        if (query->_orderBy.length() > 0)
        {
            hasQuery = true;

            header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
            hasQueryParams = true;

            header += fb_esp_pgm_str_96;
            header += query->_orderBy;

            if (req->method == m_get)
            {
                if (query->_limitToFirst.length() > 0)
                {
                    header += fb_esp_pgm_str_97;
                    header += query->_limitToFirst;
                }

                if (query->_limitToLast.length() > 0)
                {
                    header += fb_esp_pgm_str_98;
                    header += query->_limitToLast;
                }

                if (query->_startAt.length() > 0)
                {
                    header += fb_esp_pgm_str_99;
                    header += query->_startAt;
                }

                if (query->_endAt.length() > 0)
                {
                    header += fb_esp_pgm_str_100;
                    header += query->_endAt;
                }

                if (query->_equalTo.length() > 0)
                {
                    header += fb_esp_pgm_str_101;
                    header += query->_equalTo;
                }
            }
        }
    }

    if (req->method == m_download)
    {
        header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
        hasQueryParams = true;

        header += fb_esp_pgm_str_162;
        header += fb_esp_pgm_str_28;
        MB_String filename;

        for (size_t i = 0; i < fbdo->session.rtdb.path.length(); i++)
        {
            if (fbdo->session.rtdb.path.c_str()[i] == '/')
                filename += fb_esp_pgm_str_4;
            else
                filename += fbdo->session.rtdb.path.c_str()[i];
        }

        header += filename;
        filename.clear();
    }

    if (req->method == m_get && req->filename.length() > 0)
    {
        header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
        hasQueryParams = true;

        header += fb_esp_pgm_str_28;
        header += fbdo->session.rtdb.filename;
    }

    if (req->async || req->method == m_get_nocontent || req->method == m_restore || req->method == m_put_nocontent || req->method == m_patch_nocontent)
    {
        header += hasQueryParams ? fb_esp_pgm_str_172 : fb_esp_pgm_str_173;
        hasQueryParams = true;

        header += fb_esp_pgm_str_29;
    }

    header += fb_esp_pgm_str_30;
    header += fb_esp_pgm_str_31;
    header += cfg->database_url;
    header += fb_esp_pgm_str_21;
    header += fb_esp_pgm_str_32;

    ut->getCustomHeaders(header);

    if (Signer.getTokenType() == token_type_oauth2_access_token)
    {
        header += fb_esp_pgm_str_237;
        header += cfg->signer.tokens.auth_type;

        if (cfg->signer.tokens.auth_type.length() > 0)
        {
            if (cfg->signer.tokens.auth_type[cfg->signer.tokens.auth_type.length() - 1] != ' ')
                header += fb_esp_pgm_str_6;
        }

        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpClient.send(cfg->internal.auth_token.c_str());

        if (fbdo->session.response.code < 0)
            return false;

        header += fb_esp_pgm_str_21;
    }

    // Timestamp cannot use with ETag header, otherwise cases internal server error
    if (!hasServerValue && !hasQuery && req->data.type != d_timestamp && (req->method == m_delete || req->method == m_get || req->method == m_get_nocontent || req->method == m_put || req->method == m_put_nocontent || req->method == m_post))
        header += fb_esp_pgm_str_148;

    if (fbdo->session.rtdb.req_etag.length() > 0 && (req->method == m_put || req->method == m_put_nocontent || req->method == m_delete))
    {
        header += fb_esp_pgm_str_149;
        header += fbdo->session.rtdb.req_etag;
        header += fb_esp_pgm_str_21;
    }

    if (fbdo->session.classic_request && http_method != m_get && http_method != m_post && http_method != m_patch)
    {
        header += fb_esp_pgm_str_153;

        if (http_method == m_put)
            header += fb_esp_pgm_str_23;
        else if (http_method == m_delete)
            header += fb_esp_pgm_str_27;

        header += fb_esp_pgm_str_21;
    }

    if (req->method == m_stream)
    {
        fbdo->session.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        header += fb_esp_pgm_str_34;
        header += fb_esp_pgm_str_35;
    }
    else if (req->method == m_download || req->method == m_restore)
    {
        fbdo->session.rtdb.http_req_conn_type = fb_esp_http_connection_type_close;
        header += fb_esp_pgm_str_34;
    }
    else
    {
        fbdo->session.rtdb.http_req_conn_type = fb_esp_http_connection_type_keep_alive;
        header += fb_esp_pgm_str_36;
        header += fb_esp_pgm_str_37;
    }

    if (req->method != m_download && req->method != m_restore)
        header += fb_esp_pgm_str_38;

    if (req->method == m_get_priority || req->method == m_set_priority)
        fbdo->session.rtdb.priority_val_flag = true;

    if (req->method == m_put || req->method == m_put_nocontent || req->method == m_post || req->method == m_patch || req->method == m_patch_nocontent || req->method == m_restore || req->method == m_set_rules || req->method == m_set_priority)
    {
        header += fb_esp_pgm_str_12;
        header += getPayloadLen(req);
    }
    header += fb_esp_pgm_str_21;
    header += fb_esp_pgm_str_21;

    fbdo->tcpClient.send(header.c_str());
    header.clear();

    if (fbdo->session.response.code < 0)
        return false;

    return true;
}

void FB_RTDB::removeStreamCallback(FirebaseData *fbdo)
{
    FirebaseConfig *cfg = Signer.getCfg();
    if (!cfg)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return;
    }
    fbdo->removeAddr();

    fbdo->_dataAvailableCallback = NULL;
    fbdo->_timeoutCallback = NULL;

    if (cfg->internal.fbdo_addr_list.size() == 0)
    {
#if defined(ESP32)
        if (cfg->internal.stream_task_handle)
            vTaskDelete(cfg->internal.stream_task_handle);

        cfg->internal.stream_task_handle = NULL;
#endif
    }
}

void FB_RTDB::clearDataStatus(FirebaseData *fbdo)
{
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

    if (fbdo->session.jsonPtr)
        fbdo->session.jsonPtr->clear();

    if (fbdo->session.arrPtr)
        fbdo->session.arrPtr->clear();
}

bool FB_RTDB::connectionError(FirebaseData *fbdo)
{
    return fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED || fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST ||
           fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED ||
           fbdo->session.response.code == FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED || fbdo->session.response.code == FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
}

bool FB_RTDB::handleStreamRequest(FirebaseData *fbdo, const MB_String &path)
{

    struct fb_esp_rtdb_request_info_t _req;
    _req.method = m_stream;
    _req.data.type = d_string;

    if (fbdo->session.rtdb.redirect_url.length() > 0)
    {
        struct fb_esp_url_info_t uinfo;
        ut->getUrlInfo(fbdo->session.rtdb.redirect_url, uinfo);
        _req.path = uinfo.uri.c_str();
    }
    else
        _req.path = path.c_str();

    int pc = preRequestCheck(fbdo, &_req);
    if (pc < 0)
        return false;
    else if (pc == 0)
        return true;

    if (!sendRequest(fbdo, &_req))
        return false;

    return true;
}

#endif

#endif // ENABLE