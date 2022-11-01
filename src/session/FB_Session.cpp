/**
 * Google's Firebase Data class, FB_Session.cpp version 1.3.0
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 1, 2022
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
    if (ut && intCfg)
    {
        delete ut;
        ut = nullptr;
    }

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

void FirebaseData::setExternalClientCallbacks(FB_TCPConnectionRequestCallback tcpConnectionCB, FB_NetworkConnectionRequestCallback networkConnectionCB, FB_NetworkStatusRequestCallback networkStatusCB)
{
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    tcpClient.tcpConnectionRequestCallback(tcpConnectionCB);
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

void FirebaseData::addAddr(fb_esp_con_mode mode)
{
    if (!Signer.getCfg())
        return;

    removeAddr();

    if (addr == 0)
    {
        addr = toAddr(*this);
        Signer.getCfg()->internal.fbdo_addr_list.push_back(addr);
        session.con_mode = mode;
    }
}

void FirebaseData::removeAddr()
{
    if (!Signer.getCfg())
        return;

    if (addr > 0)
    {
        for (size_t i = 0; i < Signer.getCfg()->internal.fbdo_addr_list.size(); i++)
        {
            if (addr > 0 && Signer.getCfg()->internal.fbdo_addr_list[i] == addr)
            {
                session.con_mode = fb_esp_con_mode_undefined;
                Signer.getCfg()->internal.fbdo_addr_list.erase(Signer.getCfg()->internal.fbdo_addr_list.begin() + i);
                addr = 0;
                break;
            }
        }
    }
}

void FirebaseData::addQueueAddr()
{
    if (queue_addr == 0)
    {
        queue_addr = toAddr(*this);
        Signer.getCfg()->internal.queue_addr_list.push_back(queue_addr);
    }
}

void FirebaseData::removeQueueAddr()
{
    if (!Signer.getCfg())
        return;

    if (queue_addr > 0)
    {
        for (size_t i = 0; i < Signer.getCfg()->internal.queue_addr_list.size(); i++)
        {
            if (queue_addr > 0 && Signer.getCfg()->internal.queue_addr_list[i] == queue_addr)
            {
                Signer.getCfg()->internal.queue_addr_list.erase(Signer.getCfg()->internal.queue_addr_list.begin() + i);
                queue_addr = 0;
                break;
            }
        }
    }
}

bool FirebaseData::init()
{
    if (!Signer.getCfg())
        return false;

    this->ut = Signer.getUtils();

    this->mbfs = Signer.getMBFS();

    return true;
}

// Double quotes string trim.
void FirebaseData::setRaw(bool trim)
{
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
}

#ifdef ENABLE_RTDB

void FirebaseData::mSetResBool(bool value)
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

void FirebaseData::mSetResInt(const char *value)
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

void FirebaseData::mSetResFloat(const char *value)
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
    init();

    bool status = tcpClient.networkReady();

    if (!status || !tcpClient.connected())
    {
        session.connected = false;
        session.rtdb.pause = true;
        return false;
    }

    if (pause == session.rtdb.pause)
        return true;

    session.rtdb.pause = pause;

    if (pause)
    {
        closeSession();
    }

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
    if (!init())
        return "";
    MB_String res;

    switch (type)
    {
    case fb_esp_data_type::d_json:
        res += fb_esp_pgm_str_74;
        break;
    case fb_esp_data_type::d_array:
        res += fb_esp_pgm_str_165;
        break;
    case fb_esp_data_type::d_string:
        res += fb_esp_pgm_str_75;
        break;
    case fb_esp_data_type::d_float:
        res += fb_esp_pgm_str_76;
        break;
    case fb_esp_data_type::d_double:
        res += fb_esp_pgm_str_108;
        break;
    case fb_esp_data_type::d_boolean:
        res += fb_esp_pgm_str_105;
        break;
    case fb_esp_data_type::d_integer:
        res += fb_esp_pgm_str_77;
        break;
    case fb_esp_data_type::d_blob:
        res += fb_esp_pgm_str_91;
        break;
    case fb_esp_data_type::d_file:
        res += fb_esp_pgm_str_183;
        break;
    case fb_esp_data_type::d_null:
        res += fb_esp_pgm_str_78;
        break;
    default:
        break;
    }

    return res;
}

MB_String FirebaseData::getMethod(uint8_t method)
{
    if (!init())
        return "";
    MB_String res;

    switch (method)
    {
    case fb_esp_method::m_get:
        res += fb_esp_pgm_str_115;
        break;
    case fb_esp_method::m_put:
    case fb_esp_method::m_put_nocontent:
        res += fb_esp_pgm_str_116;
        break;
    case fb_esp_method::m_post:
        res += fb_esp_pgm_str_117;
        break;
    case fb_esp_method::m_patch:
    case fb_esp_method::m_patch_nocontent:
        res += fb_esp_pgm_str_118;
        break;
    case fb_esp_method::m_delete:
        res += fb_esp_pgm_str_119;
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
    if (!init())
        return 0;

    if (session.rtdb.req_data_type == fb_esp_data_type::d_timestamp)
        return to<uint64_t>() / 1000;
    else
        return to<int>();
}

float FirebaseData::floatData()
{
    if (!init())
        return 0;

    return to<float>();
}

double FirebaseData::doubleData()
{
    if (!init())
        return 0;

    return to<double>();
}

bool FirebaseData::boolData()
{
    if (!init())
        return false;

    return to<bool>();
}

String FirebaseData::stringData()
{
    if (!init())
        return String();

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
    if (session.rtdb.stream_stop)
        return false;

    if (Signer.getCfg()->timeout.rtdbStreamError < MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL || Signer.getCfg()->timeout.rtdbStreamError > MAX_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL)
        Signer.getCfg()->timeout.rtdbStreamError = MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL;

    if (millis() - Signer.getCfg()->timeout.rtdbStreamError > session.rtdb.stream_tmo_Millis || session.rtdb.stream_tmo_Millis == 0)
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
    bool ret = session.connected && !session.rtdb.stream_stop && session.rtdb.data_available && session.rtdb.stream_data_changed;
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

void FirebaseData::addQueue(struct fb_esp_rtdb_queue_info_t *qinfo)
{
    if (_qMan.size() < _qMan._maxQueue && qinfo->payload.length() <= session.rtdb.max_blob_size)
    {
        QueueItem item;
        item.method = qinfo->method;
        item.dataType = qinfo->dataType;
        item.path = qinfo->path;
        item.filename = qinfo->filename;
        item.payload = qinfo->payload;

        item.address.query = qinfo->address.query;
        item.address.din = qinfo->address.din;
        item.address.dout = qinfo->address.dout;
        item.blobSize = qinfo->blobSize;
        item.qID = random(100000, 200000);
#if defined(FIREBASE_ESP_CLIENT)
        item.storageType = qinfo->storageType;
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
        item.storageType = (fb_esp_mem_storage_type)qinfo->storageType;
#endif
        if (_qMan.add(item))
            session.rtdb.queue_ID = item.qID;
        else
            session.rtdb.queue_ID = 0;
    }
}

#endif

#if defined(ESP8266)
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

#if (defined(ESP32) || defined(ESP8266)) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
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
    return "";
}

String FirebaseData::errorReason()
{
    if (session.error.length() > 0)
        return session.error.c_str();
    else
    {
        MB_String buf;
        Signer.errorToString(session.response.code, buf);
        if (buf.length() == 0)
            buf += fb_esp_pgm_str_83;
        return buf.c_str();
    }
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
    if (!init())
        return "";

    MB_String link;
    if (session.con_mode == fb_esp_con_mode_storage)
    {
#ifdef ENABLE_FB_STORAGE
        if (session.fcs.meta.downloadTokens.length() > 0)
        {
            link += fb_esp_pgm_str_112;
            link += fb_esp_pgm_str_265;
            link += fb_esp_pgm_str_120;
            link += fb_esp_pgm_str_266;
            link += session.fcs.meta.bucket;
            link += fb_esp_pgm_str_267;
            link += fb_esp_pgm_str_1;
            link += ut->url_encode(session.fcs.meta.name);
            link += fb_esp_pgm_str_173;
            link += fb_esp_pgm_str_269;
            link += fb_esp_pgm_str_172;
            link += fb_esp_pgm_str_273;
            link += session.fcs.meta.downloadTokens.c_str();
        }
#endif
    }
    else if (session.con_mode == fb_esp_con_mode_gc_storage)
    {
#ifdef ENABLE_GC_STORAGE
        if (session.gcs.meta.downloadTokens.length() > 0)
        {
            link += fb_esp_pgm_str_112;
            link += fb_esp_pgm_str_265;
            link += fb_esp_pgm_str_120;
            link += fb_esp_pgm_str_266;
            link += session.gcs.meta.bucket;
            link += fb_esp_pgm_str_267;
            link += fb_esp_pgm_str_1;
            link += ut->url_encode(session.gcs.meta.name);
            link += fb_esp_pgm_str_173;
            link += fb_esp_pgm_str_269;
            link += fb_esp_pgm_str_172;
            link += fb_esp_pgm_str_273;
            link += session.gcs.meta.downloadTokens.c_str();
        }
#endif
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
    session.rtdb.data_millis = 0;
    session.rtdb.data_tmo = true;
    session.response.code = code;
    if (Signer.getCfg())
    {
        if (_timeoutCallback && millis() - Signer.getCfg()->internal.fb_last_stream_timeout_cb_millis > 3000)
        {
            Signer.getCfg()->internal.fb_last_stream_timeout_cb_millis = millis();
            _timeoutCallback(code < 0);
        }
    }
}
#endif

void FirebaseData::closeSession()
{

    init();
    bool status = tcpClient.networkReady();

    if (status)
    {
        // close the socket and free the resources used by the SSL engine
        if (session.connected || tcpClient.connected())
        {
            if (Signer.getCfg())
                Signer.getCfg()->internal.fb_last_reconnect_millis = millis();

            if (tcpClient.connected())
                tcpClient.stop();
        }
    }
#ifdef ENABLE_RTDB
    if (session.con_mode == fb_esp_con_mode_rtdb_stream)
    {
        session.rtdb.stream_tmo_Millis = millis();
        session.rtdb.data_millis = millis();
        session.rtdb.data_tmo = false;
        session.rtdb.new_stream = true;
    }
#endif
    session.connected = false;
}

bool FirebaseData::reconnect(unsigned long dataTime)
{
    if (tcpClient.type() == fb_tcp_client_type_external)
    {
#if !defined(FB_ENABLE_EXTERNAL_CLIENT)
        session.response.code = FIREBASE_ERROR_EXTERNAL_CLIENT_DISABLED;
        return false;
#endif
        if (!tcpClient.isInitialized())
        {
            session.response.code = FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED;
            return false;
        }
    }

    init();

    tcpClient.setConfig(Signer.getCfg());
    tcpClient.setMBFS(Signer.getMBFS());

    bool status = tcpClient.networkReady();

    if (dataTime > 0)
    {
        unsigned long tmo = DEFAULT_SERVER_RESPONSE_TIMEOUT;
        if (init())
        {
            if (Signer.getCfg()->timeout.serverResponse < MIN_SERVER_RESPONSE_TIMEOUT || Signer.getCfg()->timeout.serverResponse > MAX_SERVER_RESPONSE_TIMEOUT)
                Signer.getCfg()->timeout.serverResponse = DEFAULT_SERVER_RESPONSE_TIMEOUT;
            tmo = Signer.getCfg()->timeout.serverResponse;
        }

        if (millis() - dataTime > tmo)
        {
            session.response.code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;

            size_t len = strlen_P(fb_esp_pgm_str_69) + 5;

#if defined(ESP32) || defined(ESP8266)
            if (session.con_mode == fb_esp_con_mode_rtdb_stream)
                len += strlen_P(fb_esp_pgm_str_578);
#endif

            char *buf = new char[len];
            memset(buf, 0, len);
            strcpy_P(buf, fb_esp_pgm_str_69);

#if defined(ESP32) || defined(ESP8266)
            if (session.con_mode == fb_esp_con_mode_rtdb_stream)
                strcat_P(buf, fb_esp_pgm_str_578);
#endif

            session.error.clear();
            session.error.reserve(len);
            session.error = buf;
            ut->delP(&buf);

            closeSession();
            return false;
        }
    }

    if (!status)
    {
        if (session.connected)
            closeSession();

        session.response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

        if (init())
        {
            if (Signer.autoReconnectWiFi)
            {
                if (Signer.getCfg()->timeout.wifiReconnect < MIN_WIFI_RECONNECT_TIMEOUT || Signer.getCfg()->timeout.wifiReconnect > MAX_WIFI_RECONNECT_TIMEOUT)
                    Signer.getCfg()->timeout.wifiReconnect = MIN_WIFI_RECONNECT_TIMEOUT;

                if (millis() - Signer.getCfg()->internal.fb_last_reconnect_millis > Signer.getCfg()->timeout.wifiReconnect && !session.connected)
                {

#if defined(ESP32) || defined(ESP8266)
                    WiFi.reconnect();
#else
                    tcpClient.networkReconnect();
#endif
                    Signer.getCfg()->internal.fb_last_reconnect_millis = millis();
                }
            }
        }
        else
        {

            if (Signer.autoReconnectWiFi)
            {
                if (millis() - last_reconnect_millis > reconnect_tmo && !session.connected)
                {
#if defined(ESP32) || defined(ESP8266)
                    WiFi.reconnect();
#else
                    tcpClient.networkReconnect();
#endif
                    last_reconnect_millis = millis();
                }
            }
        }
        status = tcpClient.networkReady();
    }

#if defined(ENABLE_RTDB)
    if (!status && session.con_mode == fb_esp_con_mode_rtdb_stream)
        session.rtdb.new_stream = true;
#endif

    return status;
}

void FirebaseData::setTimeout()
{
    if (Signer.getCfg())
    {
        if (Signer.getCfg()->timeout.socketConnection < MIN_SOCKET_CONN_TIMEOUT || Signer.getCfg()->timeout.socketConnection > MAX_SOCKET_CONN_TIMEOUT)
            Signer.getCfg()->timeout.socketConnection = DEFAULT_SOCKET_CONN_TIMEOUT;

        tcpClient.setTimeout(Signer.getCfg()->timeout.socketConnection);
    }
}

void FirebaseData::setSecure()
{
    if (addr == 0)
        addAddr(fb_esp_con_mode_undefined);

    setTimeout();

    tcpClient.setMBFS(mbfs);
    tcpClient.setConfig(Signer.getCfg());

    if (!tcpClient.networkReady())
        return;

#if defined(ESP8266) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    if (Signer.getTime() > ESP_DEFAULT_TS)
    {
        if (Signer.getCfg())
            Signer.getCfg()->internal.fb_clock_rdy = true;
        tcpClient.clockReady = true;
    }
    tcpClient.bsslRxSize = session.bssl_rx_size;
    tcpClient.bsslTxSize = session.bssl_tx_size;
#endif

    if (tcpClient.certType == fb_cert_type_undefined || session.cert_updated)
    {
        if (!Signer.getCfg())
        {
            session.cert_updated = false;
            tcpClient.setCACert(NULL);
            return;
        }

        if (!Signer.getCfg()->internal.fb_clock_rdy && (Signer.getCAFile().length() > 0 || Signer.getCfg()->cert.data != NULL || session.cert_addr > 0) && init())
        {
            ut->syncClock(Signer.getCfg()->internal.fb_gmt_offset);
            tcpClient.clockReady = Signer.getCfg()->internal.fb_clock_rdy;
        }

        if (Signer.getCAFile().length() == 0)
        {
            if (session.cert_addr > 0)
                tcpClient.setCACert(reinterpret_cast<const char *>(session.cert_addr));
            else if (Signer.getCfg()->cert.data != NULL)
                tcpClient.setCACert(Signer.getCfg()->cert.data);
            else
                tcpClient.setCACert(NULL);
        }
        else
        {
            if (!tcpClient.setCertFile(Signer.getCAFile().c_str(), mbfs_type Signer.getCAFileStorage()))
                tcpClient.setCACert(NULL);
        }
        session.cert_updated = false;
    }
}

void FirebaseData::setCert(const char *ca)
{
    int addr = reinterpret_cast<int>(ca);
    if (addr != session.cert_addr)
    {
        session.cert_updated = true;
        session.cert_addr = addr;
    }
}

bool FirebaseData::tokenReady()
{
    if (Signer.getCfg())
    {
        if (Signer.getCfg()->signer.test_mode || (Signer.getCfg()->signer.tokens.token_type == token_type_legacy_token && Signer.getCfg()->signer.tokens.status == token_status_ready))
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

void FirebaseData::checkOvf(size_t len, struct server_response_data_t &resp)
{
#ifdef ENABLE_RTDB
    if (session.resp_size < len && !session.buffer_ovf)
    {
        if (session.rtdb.req_method == fb_esp_method::m_get && !session.rtdb.data_tmo && session.con_mode != fb_esp_con_mode_fcm && resp.dataType != fb_esp_data_type::d_file && session.rtdb.req_method != fb_esp_method::m_download && session.rtdb.req_data_type != fb_esp_data_type::d_file && session.rtdb.req_data_type != fb_esp_data_type::d_file_ota)
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

    if (session.arrPtr)
        session.arrPtr->clear();

    if (session.jsonPtr)
        session.jsonPtr->clear();

    if (session.dataPtr)
        session.dataPtr->clear();

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
    prepareUtil();

    _spi_ethernet_module = spi_ethernet_module;

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s += fb_esp_pgm_str_577;
    json->set(s.c_str(), addrTo<const char *>(serverKey.address()));
    raw.clear();
    s.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mAddDeviceToken(MB_StringPtr deviceToken)
{
    prepareUtil();

    MB_String _deviceToken = deviceToken;

    FirebaseJsonArray *arr = new FirebaseJsonArray();
    arr->setJsonArrayData(idTokens.c_str());
    arr->add(_deviceToken.c_str());
    idTokens.clear();
    idTokens = arr->raw();
    arr->clear();
    delete arr;
    arr = nullptr;
}

void FCMObject::removeDeviceToken(uint16_t index)
{
    prepareUtil();

    FirebaseJsonArray *arr = new FirebaseJsonArray();
    arr->setJsonArrayData(idTokens.c_str());
    arr->remove(index);
    idTokens.clear();
    idTokens = arr->raw();
    arr->clear();
    delete arr;
    arr = nullptr;
}

bool FCMObject::prepareUtil()
{
    if (!ut)
        ut = Signer.getUtils(); // must be initialized in Firebase class constructor

    if (!ut)
    {
        intUt = true;
        ut = new UtilsClass(Signer.getMBFS());
        ut->setConfig(Signer.getCfg());
    }
    return ut != nullptr;
}

void FCMObject::clearDeviceToken()
{
    prepareUtil();

    idTokens.clear();
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body)
{
    prepareUtil();

    MB_String _title = title, _body = body;

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_285;
    json->set(s.c_str(), _title.c_str());

    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_123;
    json->set(s.c_str(), _body.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body, MB_StringPtr icon)
{
    prepareUtil();

    MB_String _icon = icon;

    setNotifyMessage(title, body);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_124;
    json->set(s.c_str(), _icon.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body, MB_StringPtr icon, MB_StringPtr click_action)
{
    prepareUtil();

    MB_String _click_action = click_action;

    setNotifyMessage(title, body, icon);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_125;
    json->set(s.c_str(), _click_action.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mAddCustomNotifyMessage(MB_StringPtr key, MB_StringPtr value)
{
    prepareUtil();

    MB_String _value = value;

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    s += fb_esp_pgm_str_1;
    s += key;
    json->set(s.c_str(), _value.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::clearNotifyMessage()
{
    prepareUtil();

    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->remove(s.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mSetDataMessage(MB_StringPtr jsonString)
{

    prepareUtil();

    MB_String _jsonString = jsonString;

    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_135;
    FirebaseJson *js = new FirebaseJson();
    js->setJsonData(_jsonString.c_str());
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), *js);
    js->clear();
    delete js;
    js = nullptr;
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::setDataMessage(FirebaseJson &json)
{
    prepareUtil();

    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_135;
    FirebaseJson *js = new FirebaseJson();
    js->setJsonData(raw);
    js->set(s.c_str(), json);
    s.clear();
    raw.clear();
    raw = js->raw();
    js->clear();
    delete js;
    js = nullptr;
}

void FCMObject::clearDataMessage()
{
    prepareUtil();

    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_135;
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->remove(s.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mSetPriority(MB_StringPtr priority)
{
    prepareUtil();

    MB_String _priority = priority;
    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_136;
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), _priority.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mSetCollapseKey(MB_StringPtr key)
{
    prepareUtil();

    MB_String _key = key;

    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_138;
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), _key.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::setTimeToLive(uint32_t seconds)
{
    prepareUtil();

    if (seconds <= 2419200)
        _ttl = seconds;
    else
        _ttl = -1;
    MB_String s;
    s = fb_esp_pgm_str_575;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_137;

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), _ttl);
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

void FCMObject::mSetTopic(MB_StringPtr topic)
{
    prepareUtil();

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s, v;
    s += fb_esp_pgm_str_576;
    v += fb_esp_pgm_str_134;
    v += topic;
    json->set(s.c_str(), v.c_str());
    raw.clear();
    s.clear();
    v.clear();
    raw = json->raw();
    json->clear();
    delete json;
    json = nullptr;
}

const char *FCMObject::getSendResult()
{
    return result.c_str();
}

void FCMObject::fcm_begin(FirebaseData &fbdo)
{
    prepareUtil();

    fbdo.tcpClient.setSPIEthernet(_spi_ethernet_module);

    if (!fbdo.tcpClient.networkReady())
        return;

    MB_String host;
    host += fb_esp_pgm_str_249;
    host += fb_esp_pgm_str_4;
    host += fb_esp_pgm_str_120;
    rescon(fbdo, host.c_str());
    fbdo.tcpClient.begin(host.c_str(), _port, &fbdo.session.response.code);
}

bool FCMObject::fcm_sendHeader(FirebaseData &fbdo, size_t payloadSize)
{

    MB_String header;

    prepareUtil();

    FirebaseJsonData *server_key = new FirebaseJsonData();

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(raw);
    MB_String s = fb_esp_pgm_str_577;
    json->get(*server_key, s.c_str());
    s.clear();
    json->clear();

    header = fb_esp_pgm_str_24;
    header += fb_esp_pgm_str_6;
    header += fb_esp_pgm_str_121;
    header += fb_esp_pgm_str_30;

    header += fb_esp_pgm_str_31;
    header += fb_esp_pgm_str_249;
    header += fb_esp_pgm_str_4;
    header += fb_esp_pgm_str_120;
    header += fb_esp_pgm_str_21;

    header += fb_esp_pgm_str_131;

    fbdo.tcpClient.send(header.c_str());

    header.clear();
    if (fbdo.session.response.code < 0)
    {
        server_key->clear();
        delete server_key;
        server_key = nullptr;
        return false;
    }

    fbdo.tcpClient.send(server_key->to<const char *>());
    server_key->clear();
    delete server_key;
    server_key = nullptr;

    if (fbdo.session.response.code < 0)
        return false;

    header += fb_esp_pgm_str_21;

    header += fb_esp_pgm_str_32;

    header += fb_esp_pgm_str_8;
    header += fb_esp_pgm_str_129;
    header += fb_esp_pgm_str_21;

    header += fb_esp_pgm_str_12;
    header += payloadSize;
    header += fb_esp_pgm_str_21;
    header += fb_esp_pgm_str_36;
    header += fb_esp_pgm_str_21;

    fbdo.tcpClient.send(header.c_str());
    header.clear();
    if (fbdo.session.response.code < 0)
        return false;

    return true;
}

void FCMObject::fcm_preparePayload(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    prepareUtil();

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(raw);
    if (messageType == fb_esp_fcm_msg_type::msg_single)
    {
        MB_String s;
        s = fb_esp_pgm_str_575;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_128;

        FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
        arr->setJsonArrayData(idTokens.c_str());
        FirebaseJsonData *data = fbdo.to<FirebaseJsonData *>();
        arr->get(*data, _index);
        json->set(s.c_str(), data->to<const char *>());
        s.clear();
        raw.clear();
        raw = json->raw();
        arr->clear();
        data->clear();
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_multicast)
    {
        FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
        arr->setJsonArrayData(idTokens.c_str());

        MB_String s;
        s = fb_esp_pgm_str_575;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_130;

        json->set(s.c_str(), *arr);
        s.clear();
        arr->clear();
        raw.clear();
        raw = json->raw();
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_topic)
    {
        MB_String s;
        s = fb_esp_pgm_str_575;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_128;

        FirebaseJsonData *topic = fbdo.to<FirebaseJsonData *>();
        MB_String s2;
        s2 += fb_esp_pgm_str_576;
        json->get(*topic, s2.c_str());
        s2.clear();
        json->set(s.c_str(), topic->to<const char *>());
        s.clear();
        raw.clear();
        raw = json->raw();
        topic->clear();
    }
    json->clear();
}

bool FCMObject::waitResponse(FirebaseData &fbdo)
{
    return handleResponse(&fbdo);
}

bool FCMObject::handleResponse(FirebaseData *fbdo)
{
    prepareUtil();

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif
    if (!fbdo->reconnect())
        return false;

    if (!fbdo->session.connected)
    {
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
        return false;
    }

    result.clear();

    unsigned long dataTime = millis();

    char *pChunk = NULL;
    char *temp = NULL;
    char *header = NULL;
    char *payload = NULL;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->session.resp_size;
    int pBufPos = 0;
    int hBufPos = 0;
    int chunkBufSize = fbdo->tcpClient.available();
    int hstate = 0;
    int pstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->session.resp_size;
    int payloadRead = 0;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;

    fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->session.content_length = -1;
    fbdo->session.payload_length = 0;
    fbdo->session.chunked_encoding = false;
    fbdo->session.buffer_ovf = false;

    defaultChunkSize = 768;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = fbdo->tcpClient.available();
        ut->idle();
    }

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0)
        {
            if (!fbdo->reconnect())
                return false;

            chunkBufSize = fbdo->tcpClient.available();

            if (chunkBufSize <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    // the first chunk can be http response header
                    header = (char *)ut->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = fbdo->tcpClient.readLine(header, chunkBufSize);
                    int pos = 0;

                    temp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    ut->idle();
                    dataTime = millis();
                    if (temp)
                    {
                        // http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(temp);
                        fbdo->session.response.code = response.httpCode;
                        ut->delP(&temp);
                    }
                    else
                    {
                        payload = (char *)ut->newP(payloadLen);
                        pstate = 1;
                        memcpy(payload, header, readLen);
                        pBufPos = readLen;
                        ut->delP(&header);
                        hstate = 0;
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
                        temp = (char *)ut->newP(chunkBufSize);
                        int readLen = fbdo->tcpClient.readLine(temp, chunkBufSize);
                        bool headerEnded = false;

                        // check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (temp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (temp[0] == '\r' && temp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {
                            // parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            fbdo->session.http_code = response.httpCode;

                            if (hstate == 1)
                                ut->delP(&header);
                            hstate = 0;

                            fbdo->session.chunked_encoding = response.isChunkedEnc;
                        }
                        else
                        {
                            // accumulate the remaining header field
                            memcpy(header + hBufPos, temp, readLen);
                            hBufPos += readLen;
                        }

                        ut->delP(&temp);
                    }
                    else
                    {
                        // the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;

                            pChunk = (char *)ut->newP(chunkBufSize + 1);

                            if (!payload || pstate == 0)
                            {
                                pstate = 1;
                                payload = (char *)ut->newP(payloadLen + 1);
                            }

                            // read the avilable data
                            int readLen = 0;

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
                            }

                            if (readLen > 0)
                            {
                                fbdo->session.payload_length += readLen;
                                payloadRead += readLen;
                                fbdo->checkOvf(pBufPos + readLen, response);

                                if (!fbdo->session.buffer_ovf)
                                {
                                    if (pBufPos + readLen <= payloadLen)
                                        memcpy(payload + pBufPos, pChunk, readLen);
                                    else
                                    {
                                        // in case of the accumulated payload size is bigger than the char array
                                        // reallocate the char array

                                        char *buf = (char *)ut->newP(pBufPos + readLen + 1);
                                        memcpy(buf, payload, pBufPos);

                                        memcpy(buf + pBufPos, pChunk, readLen);

                                        payloadLen = pBufPos + readLen;
                                        ut->delP(&payload);
                                        payload = (char *)ut->newP(payloadLen + 1);
                                        memcpy(payload, buf, payloadLen);
                                        ut->delP(&buf);
                                    }
                                }
                            }

                            ut->delP(&pChunk);
                            if (readLen < 0 && payloadRead >= response.contentLen)
                                break;
                            if (readLen > 0)
                                pBufPos += readLen;
                        }
                        else
                        {
                            // read all the rest data
                            fbdo->tcpClient.flush();
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delP(&header);

        if (payload)
        {
            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK)
                result = payload;
            else
            {
                MB_String t = ut->trim(payload);
                if (t[0] == '{' && t[t.length() - 1] == '}')
                {
                    FirebaseJson *json = fbdo->to<FirebaseJson *>();
                    FirebaseJsonData *data = fbdo->to<FirebaseJsonData *>();
                    json->setJsonData(t.c_str());

                    json->get(*data, pgm2Str(fb_esp_pgm_str_257));

                    if (data->success)
                    {
                        error.code = data->to<int>();
                        json->get(*data, pgm2Str(fb_esp_pgm_str_258));
                        if (data->success)
                            fbdo->session.error = data->to<const char *>();
                    }
                    else
                        error.code = 0;
                    json->clear();
                    data->clear();
                }
            }
        }

        if (pstate == 1)
            ut->delP(&payload);

        return error.code == 0 || response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK;
    }
    else
    {
        fbdo->tcpClient.flush();
    }

    return false;
}

bool FCMObject::fcm_send(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    if (fbdo.tcpClient.reserved)
        return false;

    prepareUtil();

    FirebaseJsonData *msg = fbdo.to<FirebaseJsonData *>();

    fcm_preparePayload(fbdo, messageType);

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(raw);
    MB_String s;
    s += fb_esp_pgm_str_575;
    json->get(*msg, s.c_str());
    raw = json->raw();
    json->clear();

    fcm_sendHeader(fbdo, strlen(msg->to<const char *>()));

    if (fbdo.session.response.code < 0)
        return false;

    fbdo.tcpClient.send(msg->to<const char *>());

    json->setJsonData(raw);
    json->remove(s.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    msg->clear();

    if (fbdo.session.response.code < 0)
    {
        fbdo.closeSession();
        if (Signer.getCfg())
            Signer.getCfg()->internal.fb_processing = false;
        return false;
    }
    else
        fbdo.session.connected = true;

    bool ret = waitResponse(fbdo);

    if (!ret)
        fbdo.closeSession();

    if (Signer.getCfg())
        Signer.getCfg()->internal.fb_processing = false;

    return ret;
}

void FCMObject::rescon(FirebaseData &fbdo, const char *host)
{
    if (fbdo.session.cert_updated || !fbdo.session.connected || millis() - fbdo.session.last_conn_ms > fbdo.session.conn_timeout || fbdo.session.con_mode != fb_esp_con_mode_fcm || strcmp(host, fbdo.session.host.c_str()) != 0)
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

    if (intUt && ut)
    {
        delete ut;
        ut = nullptr;
    }
}
#endif
#endif

#endif