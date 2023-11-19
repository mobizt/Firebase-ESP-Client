
/**
 * Google's Firebase Data class, FB_Session.cpp version 1.4.1
 *
 * Created September 12, 2023
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
#include "./mbfs/MB_MCU.h"
#include "./FirebaseFS.h"

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

void FirebaseData::setGenericClient(Client *client, FB_NetworkConnectionRequestCallback networkConnectionCB,
                                    FB_NetworkStatusRequestCallback networkStatusCB)
{
    if (client && networkConnectionCB && networkStatusCB)
    {
        _client = client;
        tcpClient.setClient(_client, networkConnectionCB, networkStatusCB);
    }
    // Client type shall be set before calling this.
    Core.setTCPClient(&tcpClient);
}

void FirebaseData::setGSMClient(Client *client, void *modem, const char *pin, const char *apn, const char *user, const char *password)
{
#if defined(FIREBASE_GSM_MODEM_IS_AVAILABLE)

    Core._cli_type = firebase_client_type_external_gsm_client;
    Core._cli = client;
    Core._modem = modem;
    Core._pin = pin;
    Core._apn = apn;
    Core._user = user;
    Core._password = password;

    tcpClient.setGSMClient(client, modem, pin, apn, user, password);
#endif
}

void FirebaseData::setEthernetClient(Client *client, uint8_t macAddress[6], int csPin, int resetPin, Firebase_StaticIP *staticIP)
{
    Core._cli_type = firebase_client_type_external_ethernet_client;
    Core._cli = client;
    Core._ethernet_mac = macAddress;
    Core._ethernet_cs_pin = csPin;
    Core._ethernet_reset_pin = resetPin;
    Core._static_ip = staticIP;

    tcpClient.setEthernetClient(client, macAddress, csPin, resetPin, staticIP);
}

void FirebaseData::setNetworkStatus(bool status)
{
    Core.setNetworkStatus(status);
    tcpClient.setNetworkStatus(status);
}

int FirebaseData::tcpSend(const char *s)
{
    int r = tcpClient.send(s);
    setSession(false, r > 0);
    return r;
}

int FirebaseData::tcpWrite(const uint8_t *data, size_t size)
{
    int r = tcpClient.write(data, size);
    setSession(false, r > 0);
    return r;
}

void FirebaseData::addSession(firebase_con_mode mode)
{
    setSession(true, false);

    if (sessionPtr.ptr == 0)
    {
        sessionPtr.ptr = toAddr(*this);
        Core.internal.sessions.push_back(sessionPtr);
        session.con_mode = mode;
    }
}

void FirebaseData::setSession(bool remove, bool status)
{
    if (sessionPtr.ptr > 0)
    {
        for (size_t i = 0; i < Core.internal.sessions.size(); i++)
        {
            if (sessionPtr.ptr > 0 && Core.internal.sessions[i].ptr == sessionPtr.ptr)
            {
                if (remove)
                {
                    session.con_mode = firebase_con_mode_undefined;
                    Core.internal.sessions.erase(Core.internal.sessions.begin() + i);
                    sessionPtr.ptr = 0;
                }
                else
                {
                    Core.internal.sessions[i].status = status;
                    sessionPtr.status = status;
                }
                break;
            }
        }
    }
}

#if defined(ENABLE_ERROR_QUEUE) || defined(FIREBASE_ENABLE_ERROR_QUEUE)
void FirebaseData::addQueueSession()
{
    if (!Core.config)
        return;

    if (queueSessionPtr.ptr == 0)
    {
        queueSessionPtr.ptr = toAddr(*this);
        Core.internal.queueSessions.push_back(queueSessionPtr);
    }
}
#endif
void FirebaseData::removeQueueSession()
{
    if (!Core.config)
        return;

    if (queueSessionPtr.ptr > 0)
    {
        for (size_t i = 0; i < Core.internal.queueSessions.size(); i++)
        {
            if (queueSessionPtr.ptr > 0 && Core.internal.queueSessions[i].ptr == queueSessionPtr.ptr)
            {
                Core.internal.queueSessions.erase(Core.internal.queueSessions.begin() + i);
                queueSessionPtr.ptr = 0;
                break;
            }
        }
    }
}

// Double quotes string trim.
void FirebaseData::setRaw(bool trim)
{
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
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

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)

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
    case firebase_data_type::d_json:
        res = firebase_rtdb_ss_pgm_str_1; // "json"
        break;
    case firebase_data_type::d_array:
        res = firebase_rtdb_ss_pgm_str_3; // "array"
        break;
    case firebase_data_type::d_string:
        res = firebase_rtdb_ss_pgm_str_2; // "string"
        break;
    case firebase_data_type::d_float:
        res = firebase_rtdb_ss_pgm_str_4; // "float"
        break;
    case firebase_data_type::d_double:
        res = firebase_rtdb_ss_pgm_str_7; // "double"
        break;
    case firebase_data_type::d_boolean:
        res = firebase_rtdb_ss_pgm_str_8; // "boolean"
        break;
    case firebase_data_type::d_integer:
        res = firebase_rtdb_ss_pgm_str_5; // "int"
        break;
    case firebase_data_type::d_blob:
        res = firebase_rtdb_ss_pgm_str_9; // "blob"
        break;
    case firebase_data_type::d_file:
        res = firebase_rtdb_ss_pgm_str_10; // "file"
        break;
    case firebase_data_type::d_null:
        res = firebase_rtdb_ss_pgm_str_6; // "null"
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
        res = firebase_rtdb_ss_pgm_str_11; // "get"
        break;
    case http_put:
    case rtdb_set_nocontent:
        res = firebase_rtdb_ss_pgm_str_12; // "set"
        break;
    case http_post:
        res = firebase_rtdb_ss_pgm_str_13; // "push"
        break;
    case http_patch:
    case rtdb_update_nocontent:
        res = firebase_pgm_str_68; // "update"
        break;
    case http_delete:
        res = firebase_pgm_str_69; // "delete"
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

    if (session.rtdb.req_data_type == firebase_data_type::d_timestamp)
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
    if (session.rtdb.resp_data_type == firebase_data_type::d_json)
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

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
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
    return session.con_mode == firebase_con_mode_rtdb_stream;
}

bool FirebaseData::streamTimeout()
{
    if (session.rtdb.stream_stop || Core.isExpired())
        return false;

    if (!Core.config)
        return false;

    if (Core.config->timeout.rtdbStreamError < MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL ||
        Core.config->timeout.rtdbStreamError > MAX_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL)
        Core.config->timeout.rtdbStreamError = MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL;

    if (millis() - Core.config->timeout.rtdbStreamError > session.rtdb.stream_tmo_Millis ||
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
    bool ret = !session.rtdb.stream_stop && session.rtdb.data_available && session.rtdb.stream_data_changed;
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

#if defined(ENABLE_ERROR_QUEUE) || defined(FIREBASE_ENABLE_ERROR_QUEUE) && (defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB))
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

void FirebaseData::setBSSLBufferSize(uint16_t rx, uint16_t tx)
{
    if (rx >= 512 && rx <= 16384)
        session.bssl_rx_size = rx;
    if (tx >= 512 && tx <= 16384)
        session.bssl_tx_size = tx;
}

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
    Core.mbfs.close(mbfs_type mem_storage_type_flash);
    Core.mbfs.close(mbfs_type mem_storage_type_sd);
}

ESP_SSLClient *FirebaseData::getWiFiClient()
{
    return tcpClient.client();
}

bool FirebaseData::httpConnected()
{
    return tcpClient.connected();
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

void FirebaseData::keepAlive(int tcpKeepIdleSeconds, int tcpKeepIntervalSeconds, int tcpKeepCount)
{
    tcpClient.keepAlive(tcpKeepIdleSeconds, tcpKeepIntervalSeconds, tcpKeepCount);
}

bool FirebaseData::isKeepAlive()
{
    return tcpClient.isKeepAlive();
}

String FirebaseData::payload()
{
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (session.con_mode == firebase_con_mode_rtdb)
    {
        if (session.rtdb.resp_data_type == firebase_data_type::d_string)
            setRaw(false); // if double quotes trimmed string, retain it.
        return session.rtdb.raw.c_str();
    }
#endif
#if defined(FIREBASE_ESP_CLIENT)
#if defined(ENABLE_FIRESTORE) || defined(FIREBASE_ENABLE_FIRESTORE)
    if (session.con_mode == firebase_con_mode_firestore)
        return session.cfs.payload.c_str();
#endif
#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
    if (session.con_mode == firebase_con_mode_functions)
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
        Core.errorToString(session.response.code, buf);
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
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
FileMetaInfo FirebaseData::metaData()
{
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
    if (session.con_mode == firebase_con_mode_gc_storage)
        return session.gcs.meta;
#endif
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
    if (session.con_mode == firebase_con_mode_storage)
        return session.fcs.meta;
#endif
    FileMetaInfo info;
    return info;
}
#endif

#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
FileList *FirebaseData::fileList()
{
    return &session.fcs.files;
}
#endif

#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
String FirebaseData::downloadURL()
{
    MB_String metaName, bucket, token, link;

    if (session.con_mode == firebase_con_mode_storage)
    {
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
        if (session.fcs.meta.downloadTokens.length() > 0)
        {
            metaName = session.fcs.meta.name;
            bucket = session.fcs.meta.bucket;
            token = session.fcs.meta.downloadTokens;
        }

#endif
    }
    else if (session.con_mode == firebase_con_mode_gc_storage)
    {
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
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
        Core.hh.addGAPIsHost(host, firebase_storage_ss_pgm_str_1 /* "firebasestorage." */);
        Core.uh.host2Url(link, host);
        link += firebase_storage_ss_pgm_str_2; // "/v0/b/"
        link += bucket;
        link += firebase_storage_ss_pgm_str_3; // "/o"
        link += firebase_pgm_str_1;            // "/"
        link += Core.uh.encode(metaName);
        link += firebase_pgm_str_7;            // "?"
        link += firebase_storage_ss_pgm_str_4; // "alt=media"
        link += firebase_pgm_str_8;            // "&"
        link += firebase_storage_ss_pgm_str_5; // "token="
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

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
void FirebaseData::sendStreamToCB(int code, bool report)
{
    session.error.clear();
    session.errCode = 0;
    session.rtdb.data_millis = 0;
    session.rtdb.data_tmo = true;
    session.response.code = code;
    if (Core.config)
    {
        if (_timeoutCallback && millis() - Core.internal.fb_last_stream_timeout_cb_millis > 3000)
        {
            Core.internal.fb_last_stream_timeout_cb_millis = millis();
            if (report)
                _timeoutCallback(code < 0);
        }
    }
}
#endif

void FirebaseData::closeSession()
{
    setSession(false, false);
    Core.closeSession(&tcpClient, &session);
}

bool FirebaseData::reconnect(unsigned long dataTime)
{
    return Core.reconnect(&tcpClient, &session, dataTime);
}

void FirebaseData::setTimeout()
{
    if (Core.config)
    {
        if (Core.config->timeout.socketConnection < MIN_SOCKET_CONN_TIMEOUT ||
            Core.config->timeout.socketConnection > MAX_SOCKET_CONN_TIMEOUT)
            Core.config->timeout.socketConnection = DEFAULT_SOCKET_CONN_TIMEOUT;

        tcpClient.setTimeout(Core.config->timeout.socketConnection);
    }
}

void FirebaseData::setSecure()
{
    if (!Core.config)
        return;

    if (sessionPtr.ptr == 0)
        addSession(firebase_con_mode_undefined);

    setTimeout();

    tcpClient.setConfig(Core.config, &Core.mbfs);

    if (!tcpClient.networkReady())
        return;

    if (Core.getTime() > FIREBASE_DEFAULT_TS)
    {
        if (Core.config)
            Core.internal.fb_clock_rdy = true;
        tcpClient.clockReady = true;
    }

    tcpClient.setBufferSizes(session.bssl_rx_size, session.bssl_tx_size);

    if (tcpClient.certType == firebase_cert_type_undefined || session.cert_updated)
    {
        if (!Core.config)
        {
            session.cert_updated = false;
            tcpClient.setCACert(NULL);
            return;
        }

        if (!Core.internal.fb_clock_rdy && (Core.config->cert.file.length() > 0 ||
                                            Core.config->cert.data != NULL || session.cert_ptr > 0))
        {
            Core.timeBegin();
            Core.readNTPTime();
            tcpClient.clockReady = Core.internal.fb_clock_rdy;
        }

        if (Core.config->cert.file.length() == 0)
        {
            if (session.cert_ptr > 0)
                tcpClient.setCACert(reinterpret_cast<const char *>(session.cert_ptr));
            else if (Core.config->cert.data != NULL)
                tcpClient.setCACert(Core.config->cert.data);
            else
                tcpClient.setCACert(NULL);
        }
        else
        {
            if (!tcpClient.setCertFile(Core.config->cert.file.c_str(), mbfs_type Core.getCAFileStorage()))
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
    if (Core.config)
    {
        if (Core.config->signer.test_mode ||
            (Core.config->signer.tokens.token_type == token_type_legacy_token &&
             Core.config->signer.tokens.status == token_status_ready))
            return true;
    }

    if (Core.isExpired())
    {
        closeSession();
        return false;
    }

    if (!Core.tokenReady())
    {
        session.response.code = FIREBASE_ERROR_TOKEN_NOT_READY;
        closeSession();
        return false;
    }
    return true;
};

bool FirebaseData::waitResponse(struct firebase_tcp_response_handler_t &tcpHandler)
{
    while (tcpClient.connected() && tcpHandler.available() <= 0)
    {
        if (!reconnect(tcpHandler.dataTime))
            return false;
        FBUtils::idle();
    }
    return true;
}

bool FirebaseData::isConnected(unsigned long &dataTime)
{
    return reconnect(dataTime) && tcpClient.connected();
}
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
void FirebaseData::createResumableTask(struct fb_gcs_upload_resumable_task_info_t &ruTask,
                                       size_t fileSize, const MB_String &location, const MB_String &local,
                                       const MB_String &remote,
                                       firebase_mem_storage_type type, firebase_gcs_request_type reqType)
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
        FBUtils::idle();
        available = tcpClient.available();
    }
}

bool FirebaseData::readPayload(MB_String *chunkOut, struct firebase_tcp_response_handler_t &tcpHandler,
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

            char *pChunk = reinterpret_cast<char *>(Core.mbfs.newP(tcpHandler.chunkBufSize + 1));

            if (response.isChunkedEnc)
                delay(1);
            // read the avilable data
            // chunk transfer encoding?
            if (response.isChunkedEnc)
                tcpHandler.bufferAvailable = Core.hh.readChunkedData(&Core.sh, &Core.mbfs, &tcpClient, pChunk, nullptr, tcpHandler);
            else
            {

                if (tcpHandler.payloadLen == 0)
                    tcpHandler.bufferAvailable = Core.hh.readLine(&tcpClient, pChunk, tcpHandler.chunkBufSize);
                else
                {
                    // for chunk base64 payload, we need to ensure the size is the multiples of 4 for decoding
                    int readIndex = 0;
                    while (readIndex < tcpHandler.chunkBufSize && tcpHandler.payloadRead + readIndex < tcpHandler.payloadLen)
                    {
                        int r = tcpClient.read();
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

            Core.mbfs.delP(&pChunk);
        }

        return false;
    }

    return true;
}

bool FirebaseData::readResponse(MB_String *payload, struct firebase_tcp_response_handler_t &tcpHandler,
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
        if (Core.hh.readStatusLine(&Core.sh, &Core.mbfs, &tcpClient, tcpHandler, response))
            session.response.code = response.httpCode;
        else
        {
            FBUtils::idle();
            tcpHandler.dataTime = millis();
            // the next line can be the remaining http headers
            if (tcpHandler.isHeader)
            {
                // read header line by line, complete?
                if (Core.hh.readHeader(&Core.sh, &Core.mbfs, &tcpClient, tcpHandler, response))
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
                        Core.authenticated = false;
                    else if (response.httpCode < 300)
                        Core.authenticated = true;
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

bool FirebaseData::prepareDownload(const MB_String &filename, firebase_mem_storage_type type, bool openFileInWrireMode)
{
    if (!Core.config)
        return false;

#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
    // Fix issue in ESP32 core v2.0.x filesystems
    // We can't open file (flash or sd) to write here because of truncated result, only append is ok.
    // We have to remove existing file
    Core.mbfs.remove(filename, mbfs_type type);
#else
    // File need to be opened in case non-RTDB class.
    // In RTDB class, it handles file opening differently.
    if (openFileInWrireMode)
    {
        int ret = Core.mbfs.open(filename, mbfs_type type, mb_fs_open_mode_write);
        if (ret < 0)
        {
            tcpClient.flush();
            session.response.code = ret;
            return false;
        }
    }
#endif
    return true;
}

void FirebaseData::prepareDownloadOTA(struct firebase_tcp_response_handler_t &tcpHandler, struct server_response_data_t &response)
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

void FirebaseData::endDownloadOTA(struct firebase_tcp_response_handler_t &tcpHandler)
{
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))

    if (tcpHandler.error.code == 0 && !Update.end())
        tcpHandler.error.code = FIREBASE_ERROR_FW_UPDATE_END_FAILED;

#endif
}

bool FirebaseData::processDownload(const MB_String &filename, firebase_mem_storage_type type,
                                   uint8_t *buf, int bufLen, struct firebase_tcp_response_handler_t &tcpHandler,
                                   struct server_response_data_t &response, int &stage, bool isOTA)
{
    if (!Core.config)
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
            if (session.con_mode == firebase_con_mode_rtdb)
            {
                MB_String pChunk;

                readPayload(&pChunk, tcpHandler, response);

                // Last chunk?
                if (Core.ut.isChunkComplete(&tcpHandler, &response, complete))
                    return true;

                if (tcpHandler.bufferAvailable > 0 && pChunk.length() > 0)
                {
                    strcpy((char *)buf, pChunk.c_str());
                    bufReady = true;

                    if (tcpHandler.pChunkIdx == 1)
                    {
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
                        // check for the request node path is empty or not found
                        if (Core.sh.compare(session.rtdb.resp_etag, 0, firebase_rtdb_pgm_str_11 /* "null_etag" */))
                        {
                            session.response.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                            tcpHandler.error.code = FIREBASE_ERROR_PATH_NOT_EXIST;
                            session.rtdb.path_not_found = true;
                        }
                        else
                        {
                            // based64 encoded string of file data
                            tcpHandler.isBase64File = Core.sh.compare((char *)buf, 0, firebase_rtdb_pgm_str_8 /* "\"file,base64," */, true);
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
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
            if (session.con_mode == firebase_con_mode_rtdb && session.rtdb.path_not_found)
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

        if (session.con_mode == firebase_con_mode_rtdb)
        {
            payload = (char *)buf;

            if (tcpHandler.isBase64File)
            {
                if (tcpHandler.pChunkIdx == 1)
                    ofs = response.payloadOfs;

                tcpHandler.base64PadLenTail = Core.oh.trimLastChunkBase64(payload, payload.length());
            }
        }

        if (isOTA)
        {
#if defined(OTA_UPDATE_ENABLED) && (defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO))
            if (tcpHandler.error.code == 0)
            {
                bool ret = false;
                if (session.con_mode == firebase_con_mode_rtdb && tcpHandler.isBase64File)
                {
                    if (tcpHandler.pChunkIdx == 1)
                        prepareDownloadOTA(tcpHandler, response);

                    ret = Core.oh.decodeBase64OTA(&Core.bh, &Core.mbfs, payload.c_str() + ofs,
                                                  payload.length() - ofs, tcpHandler.error.code);
                }
                else
                    ret = Core.bh.updateWrite(buf, tcpHandler.bufferAvailable);

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
                ret = Core.mbfs.open(filename, mbfs_type type, mb_fs_open_mode_append);
                if (ret < 0)
                {
                    tcpClient.flush();
                    session.response.code = ret;
                    return false;
                }
#endif
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
                if (session.con_mode == firebase_con_mode_rtdb && tcpHandler.isBase64File)
                    ret = Core.bh.decodeToFile(&Core.mbfs, payload.c_str() + ofs, payload.length() - ofs,
                                               mbfs_type session.rtdb.storage_type);
                else
#endif
                    ret = Core.mbfs.write(mbfs_type type, buf, tcpHandler.bufferAvailable) == (int)tcpHandler.bufferAvailable;
                FBUtils::idle();
                if (!ret)
                    tcpHandler.error.code = MB_FS_ERROR_FILE_IO_ERROR;

#if defined(ESP32_GT_2_0_1_FS_MEMORY_FIX)
                // We close file here after append
                Core.mbfs.close(mbfs_type type);
#else
                if (tcpHandler.error.code == MB_FS_ERROR_FILE_IO_ERROR)
                    Core.mbfs.close(mbfs_type type);
#endif
            }
        }

        tcpHandler.pChunkIdx++;
    }

    return true;
}

#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
bool FirebaseData::getUploadInfo(int type, int &stage, const MB_String &pChunk, bool isList, bool isMeta,
                                 struct firebase_fcs_file_list_item_t *fileitem, int &pos)
{
    if (!isList && !isMeta)
        return false;

    MB_String val;

    switch (stage)
    {
    case 0:
        if (Core.jh.parseChunk(val, pChunk, firebase_pgm_str_66 /* "name" */, pos))
        {
            stage++;
            if (isList)
                fileitem->name = val;
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.name = val;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.name = val;
#endif
        }
        break;
    case 1:
        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_6 /* "bucket" */, pos))
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
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.bucket = val;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.bucket = val;
#endif
        }
        break;

    case 2:
        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_7 /* "generation" */, pos))
        {
            stage++;
            int ts = atoi(val.substr(0, val.length() - 6).c_str());
            if (isList)
                fileitem->generation = ts;
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.generation = ts;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
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

        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_8 /* "metageneration" */, pos))
        {
            stage++;
            int gen = atoi(val.c_str());

#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.metageneration = gen;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.metageneration = gen;
#endif
        }
        break;

    case 4:
        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_9 /* "contentType" */, pos))
        {
            stage++;
            if (isList)
                fileitem->contentType = val;
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.contentType = val;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.contentType = val;
#endif
        }
        break;

    case 5:
        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_10 /* "size" */, pos))
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
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            else if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.size = size;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            else if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.size = size;
#endif
        }
        break;

    case 6:
    case 7:

        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_11 /* "crc32c" */, pos))
        {
            stage++;
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.crc32 = val;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.crc32 = val;
#endif
        }
        else if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_12 /* "etag" */, pos))
        {
            stage++;
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.etag = val;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.etag = val;
#endif
        }
        break;

    case 8:
        if ((type == 0 && Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_13 /* "downloadTokens" */, pos)) ||
            (type == 1 && Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_14 /* "metadata/firebaseStorageDownloadTokens" */,
                                             pos)))
        {
            stage++;
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
            if (isMeta && type == 0 /* fb storage */)
                session.fcs.meta.downloadTokens = val;
#endif
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
            if (isMeta && type == 1 /* cloud storage */)
                session.gcs.meta.downloadTokens = val;
#endif
        }
        break;

    case 9:
        if (Core.jh.parseChunk(val, pChunk, firebase_storage_ss_pgm_str_15 /* "mediaLink" */, pos))
        {
            stage++;
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
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
                                    bool isMeta, struct firebase_fcs_file_list_item_t *fileitem)
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

void FirebaseData::getError(MB_String &payload, struct firebase_tcp_response_handler_t &tcpHandler,
                            struct server_response_data_t &response, bool clearPayload)
{
    if (payload.length() > 0)
    {
        if (payload[0] == '{')
        {
            initJson();
            Core.jh.setData(session.jsonPtr, payload, clearPayload);

            if (Core.jh.parse(session.jsonPtr, session.dataPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
            {
                tcpHandler.error.code = session.dataPtr->to<int>();
                session.errCode = tcpHandler.error.code;
                if (Core.jh.parse(session.jsonPtr, session.dataPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */))
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
#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (session.resp_size < len && !session.buffer_ovf)
    {
        if (session.rtdb.req_method == http_get &&
            !session.rtdb.data_tmo &&
            session.con_mode != firebase_con_mode_fcm &&
            resp.dataType != firebase_data_type::d_file &&
            session.rtdb.req_method != rtdb_backup &&
            session.rtdb.req_data_type != firebase_data_type::d_file &&
            session.rtdb.req_data_type != firebase_data_type::d_file_ota)
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

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)

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
#if defined(ENABLE_GC_STORAGE) || defined(FIREBASE_ENABLE_GC_STORAGE)
    session.gcs.meta.bucket.clear();
    session.gcs.meta.contentType.clear();
    session.gcs.meta.crc32.clear();
    session.gcs.meta.downloadTokens.clear();
    session.gcs.meta.etag.clear();
    session.gcs.meta.name.clear();
#endif
#if defined(ENABLE_FB_STORAGE) || defined(FIREBASE_ENABLE_FB_STORAGE)
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
#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)
    session.cfn.payload.clear();
#endif
#if defined(ENABLE_FIRESTORE) || defined(FIREBASE_ENABLE_FIRESTORE)
    session.cfs.payload.clear();
#endif
#endif
}

#endif