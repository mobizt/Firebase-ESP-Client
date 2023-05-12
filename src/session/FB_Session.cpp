#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Data class, FB_Session.cpp version 1.3.7
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

#ifndef FIREBASE_SESSION_CPP
#define FIREBASE_SESSION_CPP

#include "FB_Session.h"

FirebaseData::FirebaseData()
{
}

FirebaseData::FirebaseData(Client *client)
{
    setExternalClient(client);
}

FirebaseData::~FirebaseData()
{

    clear();

    if (session.dataPtr)
    {
        delete session.dataPtr;
        session.dataPtr = nullptr;
    }

    if (session.arrPtr)
    {
        delete session.arrPtr;
        session.arrPtr = nullptr;
    }

    if (session.jsonPtr)
    {
        delete session.jsonPtr;
        session.jsonPtr = nullptr;
    }
}

void FirebaseData::setExternalClient(Client *client)
{
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    this->tcpClient.setClient(client);
    Signer.setTCPClient(&(this->tcpClient));
#endif
}

void FirebaseData::setExternalClientCallbacks(FB_TCPConnectionRequestCallback tcpConnectionCB,
                                              FB_NetworkConnectionRequestCallback networkConnectionCB,
                                              FB_NetworkStatusRequestCallback networkStatusCB)
{
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    Serial_Printf("\nWarning: The TCP Connection Request Callback is deprecated\nWarning: Do not assign this to setExternalClientCallbacks\n\n");
    tcpClient.networkConnectionRequestCallback(networkConnectionCB);
    tcpClient.networkStatusRequestCallback(networkStatusCB);
#endif
}

void FirebaseData::setExternalClientCallbacks(FB_NetworkConnectionRequestCallback networkConnectionCB,
                                              FB_NetworkStatusRequestCallback networkStatusCB)
{
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    tcpClient.networkConnectionRequestCallback(networkConnectionCB);
    tcpClient.networkStatusRequestCallback(networkStatusCB);
#endif
}

void FirebaseData::setNetworkStatus(bool status)
{
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    Signer.setNetworkStatus(status);
    tcpClient.setNetworkStatus(status);
#endif
}

void FirebaseData::addSession(fb_esp_con_mode mode)
{
    if (!Signer.config)
        return;

    removeSession();

    if (sessionPtr == 0)
    {
        sessionPtr = toAddr(*this);
        Signer.config->internal.sessions.push_back(sessionPtr);
        session.con_mode = mode;
    }
}

void FirebaseData::removeSession()
{
    if (!Signer.config)
        return;

    if (sessionPtr > 0)
    {
        for (size_t i = 0; i < Signer.config->internal.sessions.size(); i++)
        {
            if (sessionPtr > 0 && Signer.config->internal.sessions[i] == sessionPtr)
            {
                session.con_mode = fb_esp_con_mode_undefined;
                Signer.config->internal.sessions.erase(Signer.config->internal.sessions.begin() + i);
                sessionPtr = 0;
                break;
            }
        }
    }
}
#if defined(ENABLE_ERROR_QUEUE)
void FirebaseData::addQueueSession()
{
    if (!Signer.config)
        return;

    if (queueSessionPtr == 0)
    {
        queueSessionPtr = toAddr(*this);
        Signer.config->internal.queueSessions.push_back(queueSessionPtr);
    }
}
#endif
void FirebaseData::removeQueueSession()
{
    if (!Signer.config)
        return;

    if (queueSessionPtr > 0)
    {
        for (size_t i = 0; i < Signer.config->internal.queueSessions.size(); i++)
        {
            if (queueSessionPtr > 0 && Signer.config->internal.queueSessions[i] == queueSessionPtr)
            {
                Signer.config->internal.queueSessions.erase(Signer.config->internal.queueSessions.begin() + i);
                queueSessionPtr = 0;
                break;
            }
        }
    }
}

// Double quotes string trim.
void FirebaseData::setRaw(bool trim)
{
#if defined(ENABLE_RTDB)
    if (session.rtdb.raw.length() > 0)
    {
        if (trim)
        {
            if (session.rtdb.raw[0] == '"' && session.rtdb.raw[session.rtdb.raw.length() - 1] == '"')
            {
                session.rtdb.raw.pop_back();
                session.rtdb.raw.erase(0, 1);
            }
        }
        else
        {
            if (session.rtdb.raw[0] != '"' && session.rtdb.raw[session.rtdb.raw.length() - 1] != '"')
            {
                session.rtdb.raw.insert(0, '"');
                session.rtdb.raw += '"';
            }
        }
    }
#endif
}

#ifdef ENABLE_RTDB

void FirebaseData::mSetBoolValue(bool value)
{
    if (value)
    {
        iVal = {1};
        fVal.setd(1);
    }
    else
    {
        iVal = {0};
        fVal.setd(0);
    }
}

void FirebaseData::mSetIntValue(const char *value)
{
    if (strlen(value) > 0)
    {
        char *pEnd;
#if defined(__AVR__)
        value[0] == '-' ? iVal.int64 = strtol(value, &pEnd, 10) : iVal.uint64 = ut->strtoull_alt(value);
#else
        value[0] == '-' ? iVal.int64 = strtoll(value, &pEnd, 10) : iVal.uint64 = strtoull(value, &pEnd, 10);
#endif
    }
    else
        iVal = {0};
}

void FirebaseData::mSetFloatValue(const char *value)
{
    if (strlen(value) > 0)
    {
        char *pEnd;
        fVal.setd(strtod(value, &pEnd));
    }
    else
        fVal.setd(0);
}

void FirebaseData::clearQueueItem(QueueItem *item)
{
    item->path.clear();
    item->filename.clear();
    item->payload.clear();
    item->address.din = 0;
    item->address.dout = 0;
    item->blobSize = 0;
    item->address.priority = 0;
    item->address.query = 0;
}

bool FirebaseData::pauseFirebase(bool pause)
{

    if (pause == session.rtdb.pause)
        return true;

    session.rtdb.pause = pause;

    if (pause)
        closeSession();

    return true;
}

bool FirebaseData::isPause()
{
    return session.rtdb.pause;
}

String FirebaseData::dataType()
{
    return getDataType(session.rtdb.resp_data_type).c_str();
}

String FirebaseData::eventType()
{
    return session.rtdb.event_type.c_str();
}

String FirebaseData::ETag()
{
    return session.rtdb.resp_etag.c_str();
}

MB_String FirebaseData::getDataType(uint8_t type)
{

    MB_String res;

    switch (type)
    {
    case fb_esp_data_type::d_json:
        res = fb_esp_rtdb_ss_pgm_str_1; // "json"
        break;
    case fb_esp_data_type::d_array:
        res = fb_esp_rtdb_ss_pgm_str_3; // "array"
        break;
    case fb_esp_data_type::d_string:
        res = fb_esp_rtdb_ss_pgm_str_2; // "string"
        break;
    case fb_esp_data_type::d_float:
        res = fb_esp_rtdb_ss_pgm_str_4; // "float"
        break;
    case fb_esp_data_type::d_double:
        res = fb_esp_rtdb_ss_pgm_str_7; // "double"
        break;
    case fb_esp_data_type::d_boolean:
        res = fb_esp_rtdb_ss_pgm_str_8; // "boolean"
        break;
    case fb_esp_data_type::d_integer:
        res = fb_esp_rtdb_ss_pgm_str_5; // "int"
        break;
    case fb_esp_data_type::d_blob:
        res = fb_esp_rtdb_ss_pgm_str_9; // "blob"
        break;
    case fb_esp_data_type::d_file:
        res = fb_esp_rtdb_ss_pgm_str_10; // "file"
        break;
    case fb_esp_data_type::d_null:
        res = fb_esp_rtdb_ss_pgm_str_6; // "null"
        break;
    default:
        break;
    }

    return res;
}

MB_String FirebaseData::getMethod(uint8_t method)
{

    MB_String res;

    switch (method)
    {
    case http_get:
        res = fb_esp_rtdb_ss_pgm_str_11; // "get"
        break;
    case http_put:
    case rtdb_set_nocontent:
        res = fb_esp_rtdb_ss_pgm_str_12; // "set"
        break;
    case http_post:
        res = fb_esp_rtdb_ss_pgm_str_13; // "push"
        break;
    case http_patch:
    case rtdb_update_nocontent:
        res = fb_esp_pgm_str_68; // "update"
        break;
    case http_delete:
        res = fb_esp_pgm_str_69; // "delete"
        break;
    default:
        break;
    }
    return res;
}

String FirebaseData::streamPath()
{
    return session.rtdb.stream_path.c_str();
}

String FirebaseData::dataPath()
{
    return session.rtdb.path.c_str();
}

int FirebaseData::intData()
{

    if (session.rtdb.req_data_type == fb_esp_data_type::d_timestamp)
        return to<uint64_t>() / 1000;
    else
        return to<int>();
}

float FirebaseData::floatData()
{
    return to<float>();
}

double FirebaseData::doubleData()
{
    return to<double>();
}

bool FirebaseData::boolData()
{
    return to<bool>();
}

String FirebaseData::stringData()
{
    return to<const char *>();
}

String FirebaseData::jsonString()
{
    if (session.rtdb.resp_data_type == fb_esp_data_type::d_json)
        return session.rtdb.raw.c_str();
    else
        return String();
}

FirebaseJson *FirebaseData::jsonObjectPtr()
{
    return to<FirebaseJson *>();
}

FirebaseJson &FirebaseData::jsonObject()
{
    return to<FirebaseJson>();
}

FirebaseJsonArray *FirebaseData::jsonArrayPtr()
{
    return to<FirebaseJsonArray *>();
}

FirebaseJsonArray &FirebaseData::jsonArray()
{
    return to<FirebaseJsonArray>();
}

#ifdef ENABLE_RTDB
FirebaseJsonData &FirebaseData::jsonData()
{
    if (!session.dataPtr)
        session.dataPtr = new FirebaseJsonData();

    return *session.dataPtr;
}

FirebaseJsonData *FirebaseData::jsonDataPtr()
{
    if (!session.dataPtr)
        session.dataPtr = new FirebaseJsonData();

    return session.dataPtr;
}
#endif

MB_VECTOR<uint8_t> *FirebaseData::blobData()
{
    return to<MB_VECTOR<uint8_t> *>();
}

#if defined(MBFS_FLASH_FS)
fs::File FirebaseData::fileStream()
{
    return to<File>();
}
#endif

String FirebaseData::pushName()
{
    return session.rtdb.push_name.c_str();
}

bool FirebaseData::isStream()
{
    return session.con_mode == fb_esp_con_mode_rtdb_stream;
}

bool FirebaseData::streamTimeout()
{
    if (session.rtdb.stream_stop || Signer.isExpired())
        return false;

    if (!Signer.config)
        return false;

    if (Signer.config->timeout.rtdbStreamError < MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL ||
        Signer.config->timeout.rtdbStreamError > MAX_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL)
        Signer.config->timeout.rtdbStreamError = MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL;

    if (millis() - Signer.config->timeout.rtdbStreamError > session.rtdb.stream_tmo_Millis ||
        session.rtdb.stream_tmo_Millis == 0)
    {
        session.rtdb.stream_tmo_Millis = millis();
        if (session.rtdb.data_tmo)
            closeSession();
        return session.rtdb.data_tmo;
    }
    return false;
}

bool FirebaseData::dataAvailable()
{
    return session.rtdb.data_available;
}

uint8_t FirebaseData::dataTypeEnum()
{
    return session.rtdb.resp_data_type;
}

bool FirebaseData::streamAvailable()
{
    bool ret = session.connected && !session.rtdb.stream_stop &&
               session.rtdb.data_available && session.rtdb.stream_data_changed;
    session.rtdb.data_available = false;
    session.rtdb.stream_data_changed = false;
    return ret;
}

bool FirebaseData::mismatchDataType()
{
    return session.rtdb.data_mismatch;
}

size_t FirebaseData::getBackupFileSize()
{
    return session.rtdb.file_size;
}

String FirebaseData::getBackupFilename()
{
    return session.rtdb.filename.c_str();
}

#if defined(ENABLE_ERROR_QUEUE) && defined(ENABLE_RTDB)
void FirebaseData::addQueue(QueueItem *qItem)
{
    if (_qMan.size() < _qMan._maxQueue && qItem->payload.length() <= session.rtdb.max_blob_size)
    {
        qItem->qID = random(100000, 200000);
        if (_qMan.add(*qItem))
            session.rtdb.queue_ID = qItem->qID;
        else
            session.rtdb.queue_ID = 0;
    }
}
#endif

#endif

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)
void FirebaseData::setBSSLBufferSize(uint16_t rx, uint16_t tx)
{
    if (rx >= 512 && rx <= 16384)
        session.bssl_rx_size = rx;
    if (tx >= 512 && tx <= 16384)
        session.bssl_tx_size = tx;
}
#endif

void FirebaseData::setResponseSize(uint16_t len)
{
    if (len >= 1024)
        session.resp_size = 4 * (1 + (len / 4));
}

void FirebaseData::stopWiFiClient()
{
    closeSession();
}

void FirebaseData::closeFile()
{
    if (!Signer.mbfs)
        return;

    Signer.mbfs->close(mbfs_type mem_storage_type_flash);
    Signer.mbfs->close(mbfs_type mem_storage_type_sd);
}

#if (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
WiFiClientSecure *FirebaseData::getWiFiClient()
{
    return tcpClient.wcs.get();
}
#endif

bool FirebaseData::httpConnected()
{
    return session.connected;
}

bool FirebaseData::bufferOverflow()
{
    return session.buffer_ovf;
}

String FirebaseData::fileTransferError()
{
    if (session.error.length() > 0)
        return session.error.c_str();
    else
        return errorReason();
}

String FirebaseData::payload()
{
#ifdef ENABLE_RTDB
    if (session.con_mode == fb_esp_con_mode_rtdb)
    {
        if (session.rtdb.resp_data_type == fb_esp_data_type::d_string)
            setRaw(false); // if double quotes trimmed string, retain it.
        return session.rtdb.raw.c_str();
    }
#endif
#if defined(FIREBASE_ESP_CLIENT)
#ifdef ENABLE_FIRESTORE
    if (session.con_mode == fb_esp_con_mode_firestore)
        return session.cfs.payload.c_str();
#endif
#ifdef ENABLE_FB_FUNCTIONS
    if (session.con_mode == fb_esp_con_mode_functions)
        return session.cfn.payload.c_str();
#endif
#endif
    return String();
}

String FirebaseData::errorReason()
{
    if (session.error.length() > 0)
        return session.error.c_str();
    else
    {
        MB_String buf;
        Signer.errorToString(session.response.code, buf);
        return buf.c_str();
    }
}

int FirebaseData::errorCode()
{
    if (session.error.length() > 0)
        return session.errCode;
    else
        return session.response.code;
}

#if defined(FIREBASE_ESP_CLIENT)
#if defined(ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE)
FileMetaInfo FirebaseData::metaData()
{
#ifdef ENABLE_GC_STORAGE
    if (session.con_mode == fb_esp_con_mode_gc_storage)
        return session.gcs.meta;
#endif
#ifdef ENABLE_FB_STORAGE
    if (session.con_mode == fb_esp_con_mode_storage)
        return session.fcs.meta;
#endif
    FileMetaInfo info;
    return info;
}
#endif

#ifdef ENABLE_FB_STORAGE
FileList *FirebaseData::fileList()
{
    return &session.fcs.files;
}
#endif

#if defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE)
String FirebaseData::downloadURL()
{
    MB_String metaName, bucket, token, link;

    if (session.con_mode == fb_esp_con_mode_storage)
    {
#ifdef ENABLE_FB_STORAGE
        if (session.fcs.meta.downloadTokens.length() > 0)
        {
            metaName = session.fcs.meta.name;
            bucket = session.fcs.meta.bucket;
            token = session.fcs.meta.downloadTokens;
        }

#endif
    }
    else if (session.con_mode == fb_esp_con_mode_gc_storage)
    {
#ifdef ENABLE_GC_STORAGE
        if (session.gcs.meta.downloadTokens.length() > 0)
        {
            metaName = session.gcs.meta.name;
            bucket = session.gcs.meta.bucket;
            token = session.gcs.meta.downloadTokens;
        }
#endif
    }

    if (bucket.length() > 0)
    {
        MB_String host;
        HttpHelper::addGAPIsHost(host, fb_esp_storage_ss_pgm_str_1/* "firebasestorage." */);
        URLHelper::host2Url(link, host);
        link += fb_esp_storage_ss_pgm_str_2; // "/v0/b/"
        link += bucket;
        link += fb_esp_storage_ss_pgm_str_3; // "/o"
        link += fb_esp_pgm_str_1;            // "/"
        link += URLHelper::encode(metaName);
        link += fb_esp_pgm_str_7;            // "?"
        link += fb_esp_storage_ss_pgm_str_4; // "alt=media"
        link += fb_esp_pgm_str_8;            // "&"
        link += fb_esp_storage_ss_pgm_str_5; // "token="
        link += token;
    }

    return link.c_str();
}
#endif

#endif

int FirebaseData::httpCode()
{
    // in case error, return error code
    if (session.http_code == 0 || session.response.code < 0)
        return session.response.code;

    // if no error, return http code
    return session.http_code;
}

int FirebaseData::payloadLength()
{
    return session.payload_length;
}

int FirebaseData::maxPayloadLength()
{
    return session.max_payload_length;
}

#ifdef ENABLE_RTDB
void FirebaseData::sendStreamToCB(int code)
{
    session.error.clear();
    session.errCode = 0;
    session.rtdb.data_millis = 0;
    session.rtdb.data_tmo = true;
    session.response.code = code;
    if (Signer.config)
    {
        if (_timeoutCallback && millis() - Signer.config->internal.fb_last_stream_timeout_cb_millis > 3000)
        {
            Signer.config->internal.fb_last_stream_timeout_cb_millis = millis();
            _timeoutCallback(code < 0);
        }
    }
}
#endif

void FirebaseData::closeSession()
{
    Signer.closeSession(&tcpClient, &session);
}

bool FirebaseData::reconnect(unsigned long dataTime)
{
    return Signer.reconnect(&tcpClient, &session, dataTime);
}

void FirebaseData::setTimeout()
{
    if (Signer.config)
    {
        if (Signer.config->timeout.socketConnection < MIN_SOCKET_CONN_TIMEOUT ||
            Signer.config->timeout.socketConnection > MAX_SOCKET_CONN_TIMEOUT)
            Signer.config->timeout.socketConnection = DEFAULT_SOCKET_CONN_TIMEOUT;

        tcpClient.setTimeout(Signer.config->timeout.socketConnection);
    }
}

void FirebaseData::setSecure()
{
    if (!Signer.config)
        return;

    if (sessionPtr == 0)
        addSession(fb_esp_con_mode_undefined);

    setTimeout();

    tcpClient.setConfig(Signer.config, Signer.mbfs);

    if (!tcpClient.networkReady())
        return;

#if (defined(ESP8266) || defined(MB_ARDUINO_PICO)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    if (Signer.getTime() > ESP_DEFAULT_TS)
    {
        if (Signer.config)
            Signer.config->internal.fb_clock_rdy = true;
        tcpClient.clockReady = true;
    }
    tcpClient.bsslRxSize = session.bssl_rx_size;
    tcpClient.bsslTxSize = session.bssl_tx_size;
#endif

    if (tcpClient.certType == fb_cert_type_undefined || session.cert_updated)
    {
        if (!Signer.config)
        {
            session.cert_updated = false;
            tcpClient.setCACert(NULL);
            return;
        }

        if (!Signer.config->internal.fb_clock_rdy && (Signer.config->cert.file.length() > 0 ||
                                                      Signer.config->cert.data != NULL || session.cert_ptr > 0))
        {
            TimeHelper::syncClock(&Signer.ntpClient, Signer.mb_ts, Signer.mb_ts_offset, Signer.config->internal.fb_gmt_offset, Signer.config);
            tcpClient.clockReady = Signer.config->internal.fb_clock_rdy;
        }

        if (Signer.config->cert.file.length() == 0)
        {
            if (session.cert_ptr > 0)
                tcpClient.setCACert(reinterpret_cast<const char *>(session.cert_ptr));
            else if (Signer.config->cert.data != NULL)
                tcpClient.setCACert(Signer.config->cert.data);
            else
                tcpClient.setCACert(NULL);
        }
        else
        {
            if (!tcpClient.setCertFile(Signer.config->cert.file.c_str(), mbfs_type Signer.getCAFileStorage()))
                tcpClient.setCACert(NULL);
        }
        session.cert_updated = false;
    }
}

void FirebaseData::setCert(const char *ca)
{
    int ptr = reinterpret_cast<int>(ca);
    if (ptr != session.cert_ptr)
    {
        session.cert_updated = true;
        session.cert_ptr = ptr;
    }
}

bool FirebaseData::tokenReady()
{
    if (Signer.config)
    {
        if (Signer.config->signer.test_mode ||
            (Signer.config->signer.tokens.token_type == token_type_legacy_token &&
             Signer.config->signer.tokens.status == token_status_ready))
            return true;
    }

    if (Signer.isExpired())
    {
        closeSession();
        return false;
    }

    if (!Signer.tokenReady())
    {
        session.response.code = FIREBASE_ERROR_TOKEN_NOT_READY;
        closeSession();
        return false;
    }
    return true;
};

bool FirebaseData::waitResponse(struct fb_esp_tcp_response_handler_t &tcpHandler)
{
    while (tcpClient.connected() && tcpHandler.available() <= 0)
    {
        if (!reconnect(tcpHandler.dataTime))
            return false;
        Utils::idle();
    }
    return true;
}

bool FirebaseData::isConnected(unsigned long &dataTime)
{
    return reconnect(dataTime) && tcpClient.connected();
}
#if defined(ENABLE_GC_STORAGE)
void FirebaseData::createResumableTask(struct fb_gcs_upload_resumable_task_info_t &ruTask,
                                       size_t fileSize, const MB_String &location, const MB_String &local,
                                       const MB_String &remote,
                                       fb_esp_mem_storage_type type, fb_esp_gcs_request_type reqType)
{
    ruTask.req.fileSize = fileSize;
    ruTask.req.location = location;
    ruTask.req.localFileName = local;
    ruTask.req.remoteFileName = remote;
    ruTask.req.storageType = type;
    ruTask.fbdo = this;
    ruTask.req.requestType = reqType;
}
#endif

void FirebaseData::waitRxReady()
{
    int available = tcpClient.available();
    unsigned long dataTime = 0;
    if (available == 0)
        dataTime = millis();

    while (available == 0 && reconnect(dataTime))
    {
        Utils::idle();
        available = tcpClient.available();
    }
}

bool FirebaseData::readPayload(MB_String *chunkOut, struct fb_esp_tcp_response_handler_t &tcpHandler,
                               struct server_response_data_t &response)
{
    // do not check of the config here to allow legacy fcm to work

    if (tcpHandler.pChunkIdx > 0)
    {

        // the next chunk data is the payload
        if (!response.noContent)
        {
            if (!chunkOut)
                return true;

            char *pChunk = MemoryHelper::createBuffer<char *>(Signer.mbfs, tcpHandler.chunkBufSize + 1);

            if (response.isChunkedEnc)
                delay(1);
            // read the avilable data
            // chunk transfer encoding?
            if (response.isChunkedEnc)
                tcpHandler.bufferAvailable = HttpHelper::readChunkedData(Signer.mbfs, tcpClient.client, pChunk, nullptr, tcpHandler);
            else
            {

                if (tcpHandler.payloadLen == 0)
                    tcpHandler.bufferAvailable = HttpHelper::readLine(tcpClient.client, pChunk, tcpHandler.chunkBufSize);
                else
                {
                    // for chunk base64 payload, we need to ensure the size is the multiples of 4 for decoding
                    int readIndex = 0;
                    while (readIndex < tcpHandler.chunkBufSize && tcpHandler.payloadRead + readIndex < tcpHandler.payloadLen)
                    {
                        int r = tcpClient.client->read();
                        if (r > -1)
                            pChunk[readIndex++] = (char)r;
                        if (!reconnect(tcpHandler.dataTime))
                            break;
                    }
                    tcpHandler.bufferAvailable = readIndex;
                }
            }

            if (tcpHandler.bufferAvailable > 0)
            {
                session.payload_length += tcpHandler.bufferAvailable;
                if (session.max_payload_length < session.payload_length)
                    session.max_payload_length = session.payload_length;
                tcpHandler.payloadRead += tcpHandler.bufferAvailable;

                if (_responseCallback)
                    _responseCallback(pChunk);

                if (chunkOut)
                {
                    checkOvf(chunkOut->length() + tcpHandler.bufferAvailable, response);
                    if (!session.buffer_ovf)
                        *chunkOut += pChunk;
                }
            }

            MemoryHelper::freeBuffer(Signer.mbfs, pChunk);
        }

        return false;
    }

    return true;
}

bool FirebaseData::readResponse(MB_String *payload, struct fb_esp_tcp_response_handler_t &tcpHandler,
                                struct server_response_data_t &response)
{
    // Do not check for the config here to allow legacy fcm to work

    // Return false for time out and network issue otherwise return true

    if (!reconnect(tcpHandler.dataTime))
        return false;

    if (tcpHandler.available() <= 0 && tcpHandler.payloadRead >= response.contentLen && response.contentLen > 0)
        return false;

    if (tcpHandler.available() > 0)
    {
        tcpHandler.chunkBufSize = tcpHandler.defaultChunkSize;

        // status line or data?
        if (HttpHelper::readStatusLine(Signer.mbfs, tcpClient.client, tcpHandler, response))
            session.response.code = response.httpCode;
        else
        {
            Utils::idle();
            tcpHandler.dataTime = millis();
            // the next line can be the remaining http headers
            if (tcpHandler.isHeader)
            {
                // read header line by line, complete?
                if (HttpHelper::readHeader(Signer.mbfs, tcpClient.client, tcpHandler, response))
                {
                    session.http_code = response.httpCode;
                    session.chunked_encoding = response.isChunkedEnc;
                    tcpHandler.payloadLen = response.contentLen;

                    if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK ||
                        response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT ||
                        response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT)
                        tcpHandler.error.code = 0;

                    response.redirect = response.httpCode == FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT ||
                                        response.httpCode == FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT ||
                                        response.httpCode == FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY ||
                                        response.httpCode == FIREBASE_ERROR_HTTP_CODE_FOUND;

                    if (response.httpCode == 401)
                        Signer.authenticated = false;
                    else if (response.httpCode < 300)
                        Signer.authenticated = true;
                }
            }
            else // the next line is the payload
            {

                if (!response.noContent)
                {
                    if (!payload)
                        return true;

                    MB_String pChunk;

                    readPayload(&pChunk, tcpHandler, response);

                    if (pChunk.length() > 0 && !_responseCallback)
                        *payload += pChunk;
                }
            }
        }
    }

    return true;
}

bool FirebaseData::prepareDownload(const MB_String &filename, fb_esp_mem_storage_type type)
{
    if (!Signer.config)
        return false;

#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
    // Fix issue in ESP32 core v2.0.x filesystems
    // We can't open file (flash or sd) to write here because of truncated result, only append is ok.
    // We have to remove existing file
    Signer.mbfs->remove(filename, mbfs_type type);
#else
    int ret = Signer.mbfs->open(filename, mbfs_type type, mb_fs_open_mode_write);
    if (ret < 0)
    {
        tcpClient.flush();
        session.response.code = ret;
        return false;
    }
#endif
    return true;
}

void FirebaseData::prepareDownloadOTA(struct fb_esp_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))
    int size = tcpHandler.decodedPayloadLen > 0 ? tcpHandler.decodedPayloadLen : response.contentLen;
#if defined(ESP32) || defined(MB_ARDUINO_PICO)
    tcpHandler.error.code = 0;
    if (!Update.begin(size))
        tcpHandler.error.code = FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE;
#elif defined(ESP8266)
    // tcpHandler.error.code = tcpClient.beginUpdate(size, false);
    Update.begin(size);

#endif
#endif
}

void FirebaseData::endDownloadOTA(struct fb_esp_tcp_response_handler_t &tcpHandler)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))

    if (tcpHandler.error.code == 0 && !Update.end())
        tcpHandler.error.code = FIREBASE_ERROR_FW_UPDATE_END_FAILED;

#endif
}

bool FirebaseData::processDownload(const MB_String &filename, fb_esp_mem_storage_type type,
                                   uint8_t *buf, int bufLen, struct fb_esp_tcp_response_handler_t &tcpHandler,
                                   struct server_response_data_t &response, int &stage, bool isOTA)
{
    if (!Signer.config)
        return false;

    size_t available = tcpClient.available();
    tcpHandler.dataTime = millis();

    bool complete = false;

    if (stage == 0 /* read stage */)
    {
        while (available == 0 && reconnect(tcpHandler.dataTime) && tcpClient.connected() &&
               tcpHandler.payloadRead < response.contentLen)
        {
            available = tcpClient.available();
        }

        if (available)
        {
            session.payload_length += available;
            if (session.max_payload_length < session.payload_length)
                session.max_payload_length = session.payload_length;

            tcpHandler.dataTime = millis();

            if ((int)available > bufLen)
                available = bufLen;

            bool bufReady = false;

            // for RTDB, read payload string instead
            if (session.con_mode == fb_esp_con_mode_rtdb)
            {
                MB_String pChunk;

                readPayload(&pChunk, tcpHandler, response);

                // Last chunk?
                if (Utils::isChunkComplete(&tcpHandler, &response, complete))
                    return true;

                if (tcpHandler.bufferAvailable > 0 && pChunk.length() > 0)
                {
                    strcpy((char *)buf, pChunk.c_str());
                    bufReady = true;

                    if (tcpHandler.pChunkIdx == 1)
                    {
#if defined(ENABLE_RTDB)
                        // check for the request node path is empty or not found
                        if (StringHelper::compare(session.rtdb.resp_etag, 0, fb_esp_rtdb_pgm_str_11 /* "null_etag" */))
                        {
                            session.response.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                            tcpHandler.error.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                            session.rtdb.path_not_found = true;
                        }
                        else
                        {
                            // based64 encoded string of file data
                            tcpHandler.isBase64File = StringHelper::compare((char *)buf, 0, fb_esp_rtdb_pgm_str_8/* "\"file,base64," */, true);
                        }

                        if (tcpHandler.isBase64File)
                        {
                            session.rtdb.resp_data_type = tcpHandler.downloadOTA ? d_file_ota : d_file;
                            response.payloadOfs = 13; // payloadOfs must be 13 for signature len "\"file,base64,"

                            // decoded data size may include pad which we don't know
                            // from the first chunk until the last chunk
                            if (response.contentLen > 0)
                                tcpHandler.decodedPayloadLen = (3 * (response.contentLen - response.payloadOfs - 1) / 4);

                            // check for pad length from base64 signature
                            if (pChunk[1] == 'F')
                                tcpHandler.base64PadLenSignature = 1;
                            else if (pChunk[2] == 'I')
                                tcpHandler.base64PadLenSignature = 2;

                            if (tcpHandler.base64PadLenSignature > 0 && tcpHandler.decodedPayloadLen > 0)
                            {
                                // known padding from signature
                                // then decoded data size (without padding)
                                tcpHandler.decodedPayloadLen -= tcpHandler.base64PadLenSignature;
                            }
                        }
#endif
                    }
                }

                pChunk.clear();
            }
            else
            {
                tcpHandler.bufferAvailable = tcpClient.readBytes(buf, available);
                bufReady = tcpHandler.bufferAvailable == (int)available;
                tcpHandler.payloadRead += tcpHandler.bufferAvailable;
            }
#if defined(ENABLE_RTDB)
            if (session.con_mode == fb_esp_con_mode_rtdb && session.rtdb.path_not_found)
                return false;
#endif
            if (bufReady)
            {
                stage = 1; // set stage to write
                return true;
            }
        }

        return false;
    }
    else /* write stage */
    {
        stage = 0; // set stage to read

        MB_String payload;
        int ofs = 0;
        (void)ofs;

        if (session.con_mode == fb_esp_con_mode_rtdb)
        {
            payload = (char *)buf;

            if (tcpHandler.isBase64File)
            {
                if (tcpHandler.pChunkIdx == 1)
                    ofs = response.payloadOfs;

                tcpHandler.base64PadLenTail = OtaHelper::trimLastChunkBase64(payload, payload.length());
            }
        }

        if (isOTA)
        {
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))
            if (tcpHandler.error.code == 0)
            {
                bool ret = false;
                if (session.con_mode == fb_esp_con_mode_rtdb && tcpHandler.isBase64File)
                {
                    if (tcpHandler.pChunkIdx == 1)
                        prepareDownloadOTA(tcpHandler, response);

                    ret = OtaHelper::decodeBase64OTA(Signer.mbfs, payload.c_str() + ofs,
                                                     payload.length() - ofs, tcpHandler.error.code);
                }
                else
                    ret = Base64Helper::updateWrite(buf, tcpHandler.bufferAvailable);

                if (!ret)
                    tcpHandler.error.code = FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED;
            }
#endif
        }
        else
        {

            if (tcpHandler.error.code == 0)
            {
                int ret = 0;
#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
                // We open file to append here
                ret = Signer.mbfs->open(filename, mbfs_type type, mb_fs_open_mode_append);
                if (ret < 0)
                {
                    tcpClient.flush();
                    session.response.code = ret;
                    return false;
                }
#endif
#if defined(ENABLE_RTDB)
                if (session.con_mode == fb_esp_con_mode_rtdb && tcpHandler.isBase64File)
                    ret = Base64Helper::decodeToFile(Signer.mbfs, payload.c_str() + ofs, payload.length() - ofs,
                                                     mbfs_type session.rtdb.storage_type);
                else
#endif
                    ret = Signer.mbfs->write(mbfs_type type, buf, tcpHandler.bufferAvailable) == (int)tcpHandler.bufferAvailable;
                Utils::idle();
                if (!ret)
                    tcpHandler.error.code = MB_FS_ERROR_FILE_IO_ERROR;

#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
                // We close file here after append
                Signer.mbfs->close(mbfs_type type);
#endif
            }
        }

        tcpHandler.pChunkIdx++;
    }

    return true;
}

#if defined(ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE)
bool FirebaseData::getUploadInfo(int type, int &stage, const MB_String &pChunk, bool isList, bool isMeta,
                                 struct fb_esp_fcs_file_list_item_t *fileitem, int &pos)
{
    if (!isList && !isMeta)
        return false;

    MB_String val;

    switch (stage)
    {
    case 0:
        if (JsonHelper::parseChunk(val, pChunk, fb_esp_pgm_str_66 /* "name" */, pos))
        {
            stage++;
            if (isList)
                fileitem->name = val;
#if defined(ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.name = val;
#endif
#if defined(ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.name = val;
#endif
        }
        break;
    case 1:
        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_6 /* "bucket" */, pos))
        {
            stage++;
            if (isList)
            {
                fileitem->bucket = val;
                if (type == 0)
                {
                    stage = 0; // reset stage
                    return true;
                }
            }
#if defined(ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.bucket = val;
#endif
#if defined(ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.bucket = val;
#endif
        }
        break;

    case 2:
        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_7 /* "generation" */, pos))
        {
            stage++;
            int ts = atoi(val.substr(0, val.length() - 6).c_str());
            if (isList)
                fileitem->generation = ts;
#if defined(ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.generation = ts;
#endif
#if defined(ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.generation = ts;

#endif
        }
        break;

    case 3:

        if (isList)
        {
            stage++;
            return false;
        }

        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_8 /* "metageneration" */, pos))
        {
            stage++;
            int gen = atoi(val.c_str());

#if defined(ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.metageneration = gen;
#endif
#if defined(ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.metageneration = gen;
#endif
        }
        break;

    case 4:
        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_9 /* "contentType" */, pos))
        {
            stage++;
            if (isList)
                fileitem->contentType = val;
#if defined(ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.contentType = val;
#endif
#if defined(ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.contentType = val;
#endif
        }
        break;

    case 5:
        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_10 /* "size" */, pos))
        {
            stage++;
            int size = atoi(val.c_str());
            if (isList)
            {
                fileitem->size = size;
                if (type == 1)
                {
                    stage = 0; // reset stage
                    return true;
                }
            }
#if defined(ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.size = size;
#endif
#if defined(ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.size = size;
#endif
        }
        break;

    case 6:
    case 7:

        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_11 /* "crc32c" */, pos))
        {
            stage++;
#if defined(ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.crc32 = val;
#endif
#if defined(ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.crc32 = val;
#endif
        }
        else if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_12 /* "etag" */, pos))
        {
            stage++;
#if defined(ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.etag = val;
#endif
#if defined(ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.etag = val;
#endif
        }
        break;

    case 8:
        if ((type == 0 && JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_13 /* "downloadTokens" */, pos)) ||
            (type == 1 && JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_14 /* "metadata/firebaseStorageDownloadTokens" */,
                                                 pos)))
        {
            stage++;
#if defined(ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.downloadTokens = val;
#endif
#if defined(ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.downloadTokens = val;
#endif
        }
        break;

    case 9:
        if (JsonHelper::parseChunk(val, pChunk, fb_esp_storage_ss_pgm_str_15 /* "mediaLink" */, pos))
        {
            stage++;
#if defined(ENABLE_GC_STORAGE)
            if (isMeta)
                session.gcs.meta.mediaLink = val;

#endif
        }
        break;

    default:
        break;
    }

    return false;
}

void FirebaseData::getAllUploadInfo(int type, int &currentStage, const MB_String &payload, bool isList,
                                    bool isMeta, struct fb_esp_fcs_file_list_item_t *fileitem)
{
    if (isList || isMeta)
    {
        int pos = 0;
        unsigned long time = millis();
        while (currentStage < 9 && millis() - time < 1000 /* something wrongs if time out */)
        {
            int stage = currentStage;
            if (getUploadInfo(type, currentStage, payload, isList, isMeta, fileitem, pos))
                session.fcs.files.items.push_back(*fileitem);

            if (currentStage == stage)
                currentStage++;
        };
    }
}
#endif

#if (defined(ESP32) || defined(MB_ARDUINO_PICO)) && defined(ENABLE_RTDB)
const char *FirebaseData::getTaskName(size_t taskStackSize, bool isStream)
{
    MB_String taskName = fb_esp_rtdb_ss_pgm_str_14; // "task"
    taskName += isStream ? fb_esp_rtdb_ss_pgm_str_15 /* "_stream" */ : fb_esp_rtdb_ss_pgm_str_16 /* "_error_queue" */;
    taskName += sessionPtr;
    if (isStream)
    {
        Signer.config->internal.stream_task_stack_size = taskStackSize > STREAM_TASK_STACK_SIZE
                                                             ? taskStackSize
                                                             : STREAM_TASK_STACK_SIZE;
    }
    else
        Signer.config->internal.queue_task_stack_size = taskStackSize > QUEUE_TASK_STACK_SIZE
                                                            ? taskStackSize
                                                            : QUEUE_TASK_STACK_SIZE;
    return taskName.c_str();
}
#endif

void FirebaseData::getError(MB_String &payload, struct fb_esp_tcp_response_handler_t &tcpHandler,
                            struct server_response_data_t &response, bool clearPayload)
{
    if (payload.length() > 0)
    {
        if (payload[0] == '{')
        {
            initJson();
            JsonHelper::setData(session.jsonPtr, payload, clearPayload);

            if (JsonHelper::parse(session.jsonPtr, session.dataPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
            {
                tcpHandler.error.code = session.dataPtr->to<int>();
                session.errCode = tcpHandler.error.code;
                if (JsonHelper::parse(session.jsonPtr, session.dataPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */))
                    session.error = session.dataPtr->to<const char *>();
            }
            else
                tcpHandler.error.code = 0;

            clearJson();
        }
        // JSON Array payload
        else if (payload[0] == '[')
            tcpHandler.error.code = 0;
        else
        {
            session.response.code = response.httpCode;
            tcpHandler.error.code = response.httpCode;
        }

        session.content_length = response.payloadLen;
    }
}

void FirebaseData::clearJson()
{
    if (session.jsonPtr)
        session.jsonPtr->clear();
    if (session.arrPtr)
        session.arrPtr->clear();
    if (session.dataPtr)
        session.dataPtr->clear();
}

void FirebaseData::freeJson()
{
    if (session.jsonPtr)
        delete session.jsonPtr;
    if (session.arrPtr)
        delete session.arrPtr;
    if (session.dataPtr)
        delete session.dataPtr;
}

void FirebaseData::initJson()
{
    if (!session.jsonPtr)
        session.jsonPtr = new FirebaseJson();

    if (!session.arrPtr)
        session.arrPtr = new FirebaseJsonArray();

    if (!session.dataPtr)
        session.dataPtr = new FirebaseJsonData();

    clearJson();
}

void FirebaseData::checkOvf(size_t len, struct server_response_data_t &resp)
{
#ifdef ENABLE_RTDB
    if (session.resp_size < len && !session.buffer_ovf)
    {
        if (session.rtdb.req_method == http_get &&
            !session.rtdb.data_tmo &&
            session.con_mode != fb_esp_con_mode_fcm &&
            resp.dataType != fb_esp_data_type::d_file &&
            session.rtdb.req_method != rtdb_backup &&
            session.rtdb.req_data_type != fb_esp_data_type::d_file &&
            session.rtdb.req_data_type != fb_esp_data_type::d_file_ota)
        {
            session.buffer_ovf = true;
            session.response.code = FIREBASE_ERROR_BUFFER_OVERFLOW;
        }
    }
#endif
}

void FirebaseData::clear()
{
    closeSession();
    clearJson();

#ifdef ENABLE_RTDB

    _dataAvailableCallback = NULL;
    _multiPathDataCallback = NULL;
    _timeoutCallback = NULL;
    _queueInfoCallback = NULL;

    session.rtdb.raw.clear();
    session.rtdb.push_name.clear();
    session.rtdb.redirect_url.clear();
    session.rtdb.event_type.clear();
    session.rtdb.req_etag.clear();
    session.rtdb.resp_etag.clear();
    session.rtdb.priority = 0;

    if (session.rtdb.blob && session.rtdb.isBlobPtr)
    {
        session.rtdb.isBlobPtr = false;
        delete session.rtdb.blob;
        session.rtdb.blob = nullptr;
    }

#endif

#if defined(FIREBASE_ESP_CLIENT)
#ifdef ENABLE_GC_STORAGE
    session.gcs.meta.bucket.clear();
    session.gcs.meta.contentType.clear();
    session.gcs.meta.crc32.clear();
    session.gcs.meta.downloadTokens.clear();
    session.gcs.meta.etag.clear();
    session.gcs.meta.name.clear();
#endif
#ifdef ENABLE_FB_STORAGE
    session.fcs.meta.name.clear();
    session.fcs.meta.bucket.clear();
    session.fcs.meta.contentType.clear();
    session.fcs.meta.etag.clear();
    session.fcs.meta.crc32.clear();
    session.fcs.meta.downloadTokens.clear();
    session.fcs.meta.bucket.clear();
    session.fcs.meta.contentType.clear();
    session.fcs.meta.crc32.clear();
    session.fcs.meta.downloadTokens.clear();
    session.fcs.meta.etag.clear();
    session.fcs.meta.name.clear();
    session.fcs.files.items.clear();
#endif
#ifdef ENABLE_FB_FUNCTIONS
    session.cfn.payload.clear();
#endif
#ifdef ENABLE_FIRESTORE
    session.cfs.payload.clear();
#endif
#endif
}

#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)

#ifdef ENABLE_FCM
FCMObject::FCMObject()
{
}
FCMObject::~FCMObject()
{
    clear();
}

void FCMObject::mBegin(MB_StringPtr serverKey, SPI_ETH_Module *spi_ethernet_module)
{
    _spi_ethernet_module = spi_ethernet_module;
    FirebaseJson json(raw);
    json.set(pgm2Str(esp_fb_legacy_fcm_pgm_str_1 /* "server_key" */), addrTo<const char *>(serverKey.address()));
    raw.clear();
    json.toString(raw);
}

void FCMObject::mAddDeviceToken(MB_StringPtr deviceToken)
{
    FirebaseJsonArray arr(idTokens);
    arr.add(MB_String(deviceToken).c_str());
    arr.toString(idTokens);
}

void FCMObject::removeDeviceToken(uint16_t index)
{
    FirebaseJsonArray arr(idTokens);
    arr.remove(index);
    arr.toString(idTokens);
}

void FCMObject::clearDeviceToken()
{
    idTokens.clear();
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body)
{
    MB_String s = Utils::makeFCMMsgPath();
    Utils::addFCMNotificationPath(s, esp_fb_legacy_fcm_pgm_str_3 /* "title" */);
    FirebaseJson json(raw);
    json.set(s, stringPtr2Str(title));

    s = Utils::makeFCMMsgPath();
    Utils::addFCMNotificationPath(s, esp_fb_legacy_fcm_pgm_str_4 /* "body" */);
    json.set(s, stringPtr2Str(body));
    json.toString(raw);
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body, MB_StringPtr icon)
{
    setNotifyMessage(title, body);
    MB_String s = Utils::makeFCMMsgPath();
    Utils::addFCMNotificationPath(s, esp_fb_legacy_fcm_pgm_str_5 /* "icon" */);
    FirebaseJson json(raw);
    json.set(s, stringPtr2Str(icon));
    json.toString(raw);
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body, MB_StringPtr icon, MB_StringPtr click_action)
{
    setNotifyMessage(title, body, icon);
    MB_String s = Utils::makeFCMMsgPath();
    Utils::addFCMNotificationPath(s, esp_fb_legacy_fcm_pgm_str_6 /* "click_action" */);
    FirebaseJson json(raw);
    json.set(s, stringPtr2Str(click_action));
    json.toString(raw);
}

void FCMObject::mAddCustomNotifyMessage(MB_StringPtr key, MB_StringPtr value)
{
    MB_String s = Utils::makeFCMMsgPath();
    Utils::addFCMNotificationPath(s);
    s += key;
    FirebaseJson json(raw);
    json.set(s, stringPtr2Str(value));
    json.toString(raw);
}

void FCMObject::clearNotifyMessage()
{
    MB_String s = Utils::makeFCMMsgPath();
    Utils::addFCMNotificationPath(s);
    FirebaseJson json(raw);
    json.remove(s);
    json.toString(raw);
}

void FCMObject::mSetDataMessage(MB_StringPtr jsonString)
{
    FirebaseJson js(stringPtr2Str(jsonString));
    FirebaseJson json(raw);
    json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_7 /*  "data" */), js);
    json.toString(raw);
}

void FCMObject::setDataMessage(FirebaseJson &json)
{
    FirebaseJson js(raw);
    js.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_7 /*  "data" */), json);
    js.toString(raw);
}

void FCMObject::clearDataMessage()
{
    FirebaseJson json(raw);
    json.remove(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_7 /*  "data" */));
    json.toString(raw);
}

void FCMObject::mSetPriority(MB_StringPtr priority)
{
    FirebaseJson json(raw);
    json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_8 /* "priority" */), stringPtr2Str(priority));
    json.toString(raw);
}

void FCMObject::mSetCollapseKey(MB_StringPtr key)
{
    FirebaseJson json(raw);
    json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_9 /* "collapse_key" */), stringPtr2Str(key));
    json.toString(raw);
}

void FCMObject::setTimeToLive(uint32_t seconds)
{
    _ttl = (seconds <= 2419200) ? seconds : -1;
    FirebaseJson json(raw);
    json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_10 /* "time_to_live" */), _ttl);
    json.toString(raw);
}

void FCMObject::mSetTopic(MB_StringPtr topic)
{
    MB_String s, v;
    s += esp_fb_legacy_fcm_pgm_str_2;  // "topic"
    v += esp_fb_legacy_fcm_pgm_str_11; // "/topics/"
    v += topic;
    FirebaseJson json(raw);
    json.set(s, v);
    json.toString(raw);
}
const char *FCMObject::getSendResult()
{
    return result.c_str();
}

void FCMObject::fcm_begin(FirebaseData &fbdo)
{
    fbdo.tcpClient.setSPIEthernet(_spi_ethernet_module);

    if (!fbdo.tcpClient.networkReady())
        return;

    MB_String host;
    HttpHelper::addGAPIsHost(host, esp_fb_legacy_fcm_pgm_str_12 /* "fcm" */);
    rescon(fbdo, host.c_str());
    fbdo.tcpClient.begin(host.c_str(), _port, &fbdo.session.response.code);
    if (Signer.config)
    {
        fbdo.setSecure();
        return;
    }
    // Without config, no sever certificate is available
#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)
    fbdo.tcpClient.setInsecure();
#endif
}

bool FCMObject::fcm_sendHeader(FirebaseData &fbdo, size_t payloadSize)
{
    MB_String header;
    FirebaseJsonData server_key;
    FirebaseJson json(raw);
    json.get(server_key, pgm2Str(esp_fb_legacy_fcm_pgm_str_1 /* "server_key" */));
    HttpHelper::addRequestHeaderFirst(header, http_post);
    header += esp_fb_legacy_fcm_pgm_str_13; // "/fcm/send"
    HttpHelper::addRequestHeaderLast(header);
    HttpHelper::addGAPIsHostHeader(header, esp_fb_legacy_fcm_pgm_str_12 /* "fcm" */);
    HttpHelper::addAuthHeaderFirst(header, token_type_undefined);

    fbdo.tcpClient.send(header.c_str());
    header.clear();

    if (fbdo.session.response.code < 0)
        return false;

    fbdo.tcpClient.send(server_key.to<const char *>());

    if (fbdo.session.response.code < 0)
        return false;

    HttpHelper::addNewLine(header);
    HttpHelper::addUAHeader(header);
    HttpHelper::addContentTypeHeader(header, fb_esp_pgm_str_62 /* "application/json" */);
    HttpHelper::addContentLengthHeader(header, payloadSize);
    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    HttpHelper::addConnectionHeader(header, keepAlive);
    HttpHelper::addNewLine(header);

    fbdo.tcpClient.send(header.c_str());
    header.clear();
    if (fbdo.session.response.code < 0)
        return false;

    return true;
}

void FCMObject::fcm_preparePayload(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{

    FirebaseJson json(raw);
    if (messageType == fb_esp_fcm_msg_type::msg_single)
    {
        FirebaseJsonArray arr(idTokens);
        FirebaseJsonData data;
        arr.get(data, _index);
        json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_14 /* "to" */), data.to<const char *>());
        json.toString(raw);
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_multicast)
    {
        FirebaseJsonArray arr(idTokens);
        Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_15 /* "registration_ids" */);
        json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_15 /* "registration_ids" */), arr);
        json.toString(raw);
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_topic)
    {
        FirebaseJsonData topic;
        json.get(topic, pgm2Str(esp_fb_legacy_fcm_pgm_str_2 /* "topic" */));
        json.set(Utils::makeFCMMsgPath(esp_fb_legacy_fcm_pgm_str_14 /* "to" */), topic.to<const char *>());
        json.toString(raw);
    }
}

bool FCMObject::waitResponse(FirebaseData &fbdo)
{
    return handleResponse(&fbdo);
}

bool FCMObject::handleResponse(FirebaseData *fbdo)
{

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif
    if (!fbdo->reconnect())
        return false;

    bool isOTA = false;
    MB_String payload;
    struct server_response_data_t response;
    struct fb_esp_tcp_response_handler_t tcpHandler;

    HttpHelper::initTCPSession(fbdo->session);
    HttpHelper::intTCPHandler(fbdo->tcpClient.client, tcpHandler, 2048, fbdo->session.resp_size, &payload, isOTA);

    if (!fbdo->waitResponse(tcpHandler))
        return false;

    if (!fbdo->tcpClient.connected())
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */)
    {

        if (!fbdo->readResponse(nullptr, tcpHandler, response))
            break;

        if (tcpHandler.pChunkIdx > 0)
        {
            MB_String pChunk;
            fbdo->readPayload(&pChunk, tcpHandler, response);
            if (tcpHandler.bufferAvailable > 0 && pChunk.length() > 0)
            {
                Utils::idle();
                payload += pChunk;
            }
        }
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    result = payload;
    // parse payload for error
    fbdo->getError(payload, tcpHandler, response, true);

    return tcpHandler.error.code == 0;
}

bool FCMObject::fcm_send(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    if (fbdo.tcpClient.reserved)
        return false;

    fcm_preparePayload(fbdo, messageType);

    FirebaseJson json(raw);
    FirebaseJsonData msg;
    json.get(msg, pgm2Str(esp_fb_legacy_fcm_pgm_str_16 /*  "msg" */));
    json.toString(raw);

    fcm_sendHeader(fbdo, strlen(msg.to<const char *>()));

    if (fbdo.session.response.code < 0)
    {
        fbdo.closeSession();
        return false;
    }

    fbdo.tcpClient.send(msg.to<const char *>());

    json.setJsonData(raw);
    json.remove(pgm2Str(esp_fb_legacy_fcm_pgm_str_16 /*  "msg" */));
    json.toString(raw);

    if (fbdo.session.response.code < 0)
    {
        fbdo.closeSession();
        if (Signer.config)
            Signer.config->internal.fb_processing = false;
        return false;
    }
    else
        fbdo.session.connected = true;

    bool ret = waitResponse(fbdo);

    if (!ret)
        fbdo.closeSession();

    if (Signer.config)
        Signer.config->internal.fb_processing = false;

    return ret;
}

void FCMObject::rescon(FirebaseData &fbdo, const char *host)
{
    fbdo._responseCallback = NULL;

    if (fbdo.session.cert_updated || !fbdo.session.connected ||
        millis() - fbdo.session.last_conn_ms > fbdo.session.conn_timeout ||
        fbdo.session.con_mode != fb_esp_con_mode_fcm ||
        strcmp(host, fbdo.session.host.c_str()) != 0)
    {
        fbdo.session.last_conn_ms = millis();
        fbdo.closeSession();
        fbdo.setSecure();
    }
    fbdo.session.host = host;
    fbdo.session.con_mode = fb_esp_con_mode_fcm;
}

void FCMObject::clear()
{
    raw.clear();
    result.clear();
    _ttl = -1;
    _index = 0;
    clearDeviceToken();
}
#endif
#endif

#endif