/**
 * Google's Firebase Data class, FB_Session.cpp version 1.1.2
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created July 4, 2021
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

#ifndef FIREBASE_SESSION_CPP
#define FIREBASE_SESSION_CPP
#include "FB_Session.h"

FirebaseData::FirebaseData()
{
}

FirebaseData::~FirebaseData()
{
    if (ut)
        delete ut;

    clear();

    if (_ss.arrPtr)
        delete _ss.arrPtr;

    if (_ss.jsonPtr)
        delete _ss.jsonPtr;
}

bool FirebaseData::init()
{
    if (!Signer.getCfg())
        return false;

    if (!ut)
        ut = new UtilsClass(Signer.getCfg());

    return true;
}
#ifdef ENABLE_RTDB
void FirebaseData::clearQueueItem(QueueItem *item)
{
    ut->clearS(item->path);
    ut->clearS(item->filename);
    ut->clearS(item->payload);

    item->stringPtr = nullptr;
    item->intPtr = nullptr;
    item->floatPtr = nullptr;
    item->doublePtr = nullptr;
    item->boolPtr = nullptr;
    item->blobPtr = nullptr;
    item->queryFilter.clear();
}

void FirebaseData::setQuery(QueryFilter *query)
{
    queryFilter._orderBy = query->_orderBy;
    queryFilter._limitToFirst = query->_limitToFirst;
    queryFilter._limitToLast = query->_limitToLast;
    queryFilter._startAt = query->_startAt;
    queryFilter._endAt = query->_endAt;
    queryFilter._equalTo = query->_equalTo;
}

void FirebaseData::clearNodeList()
{
    for (size_t i = 0; i < _ss.rtdb.child_nodes.size(); i++)
        ut->clearS(_ss.rtdb.child_nodes[i]);
    _ss.rtdb.child_nodes.clear();
}

void FirebaseData::addNodeList(const String childPath[], size_t size)
{
    clearNodeList();
    for (size_t i = 0; i < size; i++)
        if (childPath[i].length() > 0 && childPath[i] != "/")
            _ss.rtdb.child_nodes.push_back(childPath[i].c_str());
}

bool FirebaseData::pauseFirebase(bool pause)
{
    if ((WiFi.status() != WL_CONNECTED && !ut->ethLinkUp()) || !tcpClient.stream())
    {
        _ss.connected = false;
        _ss.rtdb.pause = true;
        return false;
    }

    if (pause == _ss.rtdb.pause)
        return true;

    _ss.rtdb.pause = pause;

    if (pause)
    {
        if (tcpClient.stream()->connected())
            tcpClient.stream()->stop();
        _ss.connected = false;
    }

    return true;
}

bool FirebaseData::isPause()
{
    return _ss.rtdb.pause;
}

String FirebaseData::dataType()
{
    return getDataType(_ss.rtdb.resp_data_type).c_str();
}

String FirebaseData::eventType()
{
    return _ss.rtdb.event_type.c_str();
}

String FirebaseData::ETag()
{
    return _ss.rtdb.resp_etag.c_str();
}

std::string FirebaseData::getDataType(uint8_t type)
{
    if (!init())
        return "";
    std::string res;

    switch (type)
    {
    case fb_esp_data_type::d_json:
        ut->appendP(res, fb_esp_pgm_str_74);
        break;
    case fb_esp_data_type::d_array:
        ut->appendP(res, fb_esp_pgm_str_165);
        break;
    case fb_esp_data_type::d_string:
        ut->appendP(res, fb_esp_pgm_str_75);
        break;
    case fb_esp_data_type::d_float:
        ut->appendP(res, fb_esp_pgm_str_76);
        break;
    case fb_esp_data_type::d_double:
        ut->appendP(res, fb_esp_pgm_str_108);
        break;
    case fb_esp_data_type::d_boolean:
        ut->appendP(res, fb_esp_pgm_str_105);
        break;
    case fb_esp_data_type::d_integer:
        ut->appendP(res, fb_esp_pgm_str_77);
        break;
    case fb_esp_data_type::d_blob:
        ut->appendP(res, fb_esp_pgm_str_91);
        break;
    case fb_esp_data_type::d_file:
        ut->appendP(res, fb_esp_pgm_str_183);
        break;
    case fb_esp_data_type::d_null:
        ut->appendP(res, fb_esp_pgm_str_78);
        break;
    default:
        break;
    }

    return res;
}

std::string FirebaseData::getMethod(uint8_t method)
{
    if (!init())
        return "";
    std::string res;

    switch (method)
    {
    case fb_esp_method::m_get:
        ut->appendP(res, fb_esp_pgm_str_115);
        break;
    case fb_esp_method::m_put:
    case fb_esp_method::m_put_nocontent:
        ut->appendP(res, fb_esp_pgm_str_116);
        break;
    case fb_esp_method::m_post:
        ut->appendP(res, fb_esp_pgm_str_117);
        break;
    case fb_esp_method::m_patch:
    case fb_esp_method::m_patch_nocontent:
        ut->appendP(res, fb_esp_pgm_str_118);
        break;
    case fb_esp_method::m_delete:
        ut->appendP(res, fb_esp_pgm_str_119);
        break;
    default:
        break;
    }
    return res;
}

String FirebaseData::streamPath()
{
    return _ss.rtdb.stream_path.c_str();
}

String FirebaseData::dataPath()
{
    return _ss.rtdb.path.c_str();
}

int FirebaseData::intData()
{
    if ((_ss.rtdb.raw.length() > 0 && (_ss.rtdb.resp_data_type == fb_esp_data_type::d_integer || _ss.rtdb.resp_data_type == fb_esp_data_type::d_float || _ss.rtdb.resp_data_type == fb_esp_data_type::d_double)))
    {
        if (_ss.rtdb.req_data_type == fb_esp_data_type::d_timestamp)
        {
            double d = atof(_ss.rtdb.raw.c_str());
            int ts = d / 1000;
            return ts;
        }
        else
            return atoi(_ss.rtdb.raw.c_str());
    }
    else
        return 0;
}

float FirebaseData::floatData()
{
    if (_ss.rtdb.raw.length() > 0 && (_ss.rtdb.resp_data_type == fb_esp_data_type::d_integer || _ss.rtdb.resp_data_type == fb_esp_data_type::d_float || _ss.rtdb.resp_data_type == fb_esp_data_type::d_double))
        return atof(_ss.rtdb.raw.c_str());
    else
        return 0.0;
}

double FirebaseData::doubleData()
{
    if (_ss.rtdb.raw.length() > 0 && (_ss.rtdb.resp_data_type == fb_esp_data_type::d_integer || _ss.rtdb.resp_data_type == fb_esp_data_type::d_float || _ss.rtdb.resp_data_type == fb_esp_data_type::d_double))
        return atof(_ss.rtdb.raw.c_str());
    else
        return 0.0;
}

bool FirebaseData::boolData()
{
    if (!init())
        return false;

    bool res = false;
    char *str = ut->boolStr(true);
    if (_ss.rtdb.raw.length() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_boolean)
        res = strcmp(_ss.rtdb.raw.c_str(), str) == 0;
    ut->delS(str);
    return res;
}

String FirebaseData::stringData()
{
    if (_ss.rtdb.raw.length() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_string)
        return _ss.rtdb.raw.substr(1, _ss.rtdb.raw.length() - 2).c_str();
    else
        return std::string().c_str();
}

String FirebaseData::jsonString()
{
    if (_ss.rtdb.resp_data_type == fb_esp_data_type::d_json)
        return _ss.rtdb.raw.c_str();
    else
        return String();
}

FirebaseJson *FirebaseData::jsonObjectPtr()
{
    if (!_ss.jsonPtr)
        _ss.jsonPtr = new FirebaseJson();

    if (_ss.rtdb.resp_data_type == d_json)
    {
        _ss.jsonPtr->clear();
        if (_ss.arrPtr)
            _ss.arrPtr->clear();
        _ss.jsonPtr->setJsonData(_ss.rtdb.raw.c_str());
    }

    return _ss.jsonPtr;
}

FirebaseJson &FirebaseData::jsonObject()
{
    return *jsonObjectPtr();
}

FirebaseJsonArray *FirebaseData::jsonArrayPtr()
{
    if (!_ss.arrPtr)
        _ss.arrPtr = new FirebaseJsonArray();

    if (_ss.rtdb.resp_data_type == d_array)
    {
        if (_ss.jsonPtr)
            _ss.jsonPtr->clear();
        _ss.arrPtr->clear();
        _ss.arrPtr->setJsonArrayData(_ss.rtdb.raw.c_str());
    }

    return _ss.arrPtr;
}

FirebaseJsonArray &FirebaseData::jsonArray()
{
    return *jsonArrayPtr();
}

std::vector<uint8_t> FirebaseData::blobData()
{
    if (_ss.rtdb.blob.size() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_blob)
        return _ss.rtdb.blob;
    else
        return std::vector<uint8_t>();
}

fs::File FirebaseData::fileStream()
{

    if (_ss.rtdb.resp_data_type == fb_esp_data_type::d_file && init())
    {
        char *tmp = ut->strP(fb_esp_pgm_str_184);

        ut->flashTest();

        if (Signer.getCfg()->_int.fb_flash_rdy)
            Signer.getCfg()->_int.fb_file = FLASH_FS.open(tmp, "r");
        ut->delS(tmp);
    }

    return Signer.getCfg()->_int.fb_file;
}

String FirebaseData::pushName()
{
    return _ss.rtdb.push_name.c_str();
}

bool FirebaseData::isStream()
{
    return _ss.con_mode == fb_esp_con_mode_rtdb_stream;
}

bool FirebaseData::streamTimeout()
{
    if (_ss.rtdb.stream_stop)
        return false;
    if (millis() - STREAM_ERROR_NOTIFIED_INTERVAL > _ss.rtdb.stream_tmo_Millis || _ss.rtdb.stream_tmo_Millis == 0)
    {
        _ss.rtdb.stream_tmo_Millis = millis();
        if (_ss.rtdb.data_tmo)
            closeSession();
        return _ss.rtdb.data_tmo;
    }
    return false;
}

bool FirebaseData::dataAvailable()
{
    return _ss.rtdb.data_available;
}

uint8_t FirebaseData::dataTypeEnum()
{
    return _ss.rtdb.resp_data_type;
}

bool FirebaseData::streamAvailable()
{
    bool ret = _ss.connected && !_ss.rtdb.stream_stop && _ss.rtdb.data_available && _ss.rtdb.stream_data_changed;
    _ss.rtdb.data_available = false;
    _ss.rtdb.stream_data_changed = false;
    return ret;
}

bool FirebaseData::mismatchDataType()
{
    return _ss.rtdb.data_mismatch;
}

size_t FirebaseData::getBackupFileSize()
{
    return _ss.rtdb.backup_file_size;
}

String FirebaseData::getBackupFilename()
{
    return _ss.rtdb.backup_filename.c_str();
}

void FirebaseData::addQueue(struct fb_esp_rtdb_queue_info_t *qinfo)
{
    if (_qMan.size() < _qMan._maxQueue && qinfo->payload.length() <= _ss.rtdb.max_blob_size)
    {
        QueueItem item;
        item.method = qinfo->method;
        item.dataType = qinfo->dataType;
        item.path = qinfo->path;
        item.filename = qinfo->filename;
        item.payload = qinfo->payload;

        if (qinfo->isQuery)
            item.queryFilter = queryFilter;
        else
            item.queryFilter.clear();

        item.stringPtr = qinfo->stringPtr;
        item.intPtr = qinfo->intPtr;
        item.floatPtr = qinfo->floatPtr;
        item.doublePtr = qinfo->doublePtr;
        item.boolPtr = qinfo->boolPtr;
        item.jsonPtr = qinfo->jsonPtr;
        item.arrPtr = qinfo->arrPtr;
        item.blobPtr = qinfo->blobPtr;
        item.qID = random(100000, 200000);
#if defined(FIREBASE_ESP_CLIENT)
        item.storageType = qinfo->storageType;
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
        item.storageType = (fb_esp_mem_storage_type)qinfo->storageType;
#endif
        if (_qMan.add(item))
            _ss.rtdb.queue_ID = item.qID;
        else
            _ss.rtdb.queue_ID = 0;
    }
}

#endif

#if defined(ESP8266)
void FirebaseData::setBSSLBufferSize(uint16_t rx, uint16_t tx)
{
    if (rx >= 512 && rx <= 16384)
        _ss.bssl_rx_size = rx;
    if (tx >= 512 && tx <= 16384)
        _ss.bssl_tx_size = tx;
}
#endif

void FirebaseData::setResponseSize(uint16_t len)
{
    if (len >= 1024)
        _ss.resp_size = 4 * (1 + (len / 4));
}

void FirebaseData::stopWiFiClient()
{
    if (tcpClient.stream())
    {
        if (tcpClient.stream()->connected())
            tcpClient.stream()->stop();
    }
    _ss.connected = false;
}

WiFiClientSecure *FirebaseData::getWiFiClient()
{
    return tcpClient._wcs.get();
}

bool FirebaseData::httpConnected()
{
    return _ss.connected;
}

bool FirebaseData::bufferOverflow()
{
    return _ss.buffer_ovf;
}

String FirebaseData::fileTransferError()
{
    return _ss.error.c_str();
}

String FirebaseData::payload()
{
#ifdef ENABLE_RTDB
    if (_ss.con_mode == fb_esp_con_mode_rtdb)
        return _ss.rtdb.raw.c_str();
#endif
#if defined(FIREBASE_ESP_CLIENT)
#ifdef ENABLE_FIRESTORE
    if (_ss.con_mode == fb_esp_con_mode_firestore)
        return _ss.cfs.payload.c_str();
#endif
#ifdef ENABLE_FB_FUNCTIONS
    if (_ss.con_mode == fb_esp_con_mode_functions)
        return _ss.cfn.payload.c_str();
#endif
#endif
    return "";
}

String FirebaseData::errorReason()
{
    std::string buf = "";
    if (_ss.error.length() > 0)
        return _ss.error.c_str();
    else
        Signer.errorToString(_ss.http_code, buf);
    return buf.c_str();
}

#if defined(FIREBASE_ESP_CLIENT)
#if defined(ENABLE_GC_STORAGE) || defined(ENABLE_FB_STORAGE)
FileMetaInfo FirebaseData::metaData()
{
#ifdef ENABLE_GC_STORAGE
    if (_ss.con_mode == fb_esp_con_mode_gc_storage)
        return _ss.gcs.meta;
#endif
#ifdef ENABLE_FB_STORAGE
    if (_ss.con_mode == fb_esp_con_mode_storage)
        return _ss.fcs.meta;
#endif
    FileMetaInfo info;
    return info;
}
#endif

#ifdef ENABLE_FB_STORAGE
FileList *FirebaseData::fileList()
{
    return &_ss.fcs.files;
}
#endif

#if defined(ENABLE_FB_STORAGE) || defined(ENABLE_GC_STORAGE)
String FirebaseData::downloadURL()
{
    if (!init())
        return "";

    std::string link;
    if (_ss.con_mode == fb_esp_con_mode_storage)
    {
#ifdef ENABLE_FB_STORAGE
        if (_ss.fcs.meta.downloadTokens.length() > 0)
        {
            ut->appendP(link, fb_esp_pgm_str_112);
            ut->appendP(link, fb_esp_pgm_str_265);
            ut->appendP(link, fb_esp_pgm_str_120);
            ut->appendP(link, fb_esp_pgm_str_266);
            link += _ss.fcs.meta.bucket;
            ut->appendP(link, fb_esp_pgm_str_267);
            ut->appendP(link, fb_esp_pgm_str_1);
            link += ut->url_encode(_ss.fcs.meta.name);
            ut->appendP(link, fb_esp_pgm_str_173);
            ut->appendP(link, fb_esp_pgm_str_269);
            ut->appendP(link, fb_esp_pgm_str_172);
            ut->appendP(link, fb_esp_pgm_str_273);
            link += _ss.fcs.meta.downloadTokens.c_str();
        }
#endif
    }
    else if (_ss.con_mode == fb_esp_con_mode_gc_storage)
    {
#ifdef ENABLE_GC_STORAGE
        if (_ss.gcs.meta.downloadTokens.length() > 0)
        {
            ut->appendP(link, fb_esp_pgm_str_112);
            ut->appendP(link, fb_esp_pgm_str_265);
            ut->appendP(link, fb_esp_pgm_str_120);
            ut->appendP(link, fb_esp_pgm_str_266);
            link += _ss.gcs.meta.bucket;
            ut->appendP(link, fb_esp_pgm_str_267);
            ut->appendP(link, fb_esp_pgm_str_1);
            link += ut->url_encode(_ss.gcs.meta.name);
            ut->appendP(link, fb_esp_pgm_str_173);
            ut->appendP(link, fb_esp_pgm_str_269);
            ut->appendP(link, fb_esp_pgm_str_172);
            ut->appendP(link, fb_esp_pgm_str_273);
            link += _ss.gcs.meta.downloadTokens.c_str();
        }
#endif
    }

    return link.c_str();
}
#endif

#endif

int FirebaseData::httpCode()
{
    return _ss.http_code;
}

void FirebaseData::closeSession()
{
    if (WiFi.status() == WL_CONNECTED || ut->ethLinkUp())
    {
        //close the socket and free the resources used by the BearSSL data
        if (_ss.connected || tcpClient.stream())
        {
            if (Signer.getCfg())
                Signer.getCfg()->_int.fb_last_reconnect_millis = millis();

            if (tcpClient.stream())
                if (tcpClient.stream()->connected())
                    tcpClient.stream()->stop();
        }
    }
#ifdef ENABLE_RTDB
    if (_ss.con_mode == fb_esp_con_mode_rtdb_stream)
    {
        _ss.rtdb.stream_tmo_Millis = millis();
        _ss.rtdb.data_millis = millis();
        _ss.rtdb.data_tmo = false;
    }
#endif
    _ss.connected = false;
}

int FirebaseData::tcpSend(const char *data)
{
    uint8_t attempts = 0;
    uint8_t maxRetry = 5;
    int ret = tcpClient.send(data);

    while (ret != 0)
    {
        attempts++;
        if (attempts > maxRetry)
            break;

        if (!reconnect())
            return FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

        ret = tcpClient.send(data);
    }

    return ret;
}

bool FirebaseData::reconnect(unsigned long dataTime)
{

    bool status = WiFi.status() == WL_CONNECTED || ut->ethLinkUp();

    if (dataTime > 0)
    {
        if (millis() - dataTime > tcpClient.timeout && init())
        {
            _ss.http_code = FIREBASE_ERROR_TCP_ERROR_READ_TIMEOUT;
            char *tmp = ut->strP(fb_esp_pgm_str_69);
            _ss.error = tmp;
            ut->delS(tmp);
            closeSession();
            return false;
        }
    }

    if (!status)
    {
        if (_ss.connected)
            closeSession();

        _ss.http_code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

        if (Signer.getCfg()->_int.fb_reconnect_wifi)
        {
            if (millis() - Signer.getCfg()->_int.fb_last_reconnect_millis > Signer.getCfg()->_int.fb_reconnect_tmo && !_ss.connected)
            {
                WiFi.reconnect();
                Signer.getCfg()->_int.fb_last_reconnect_millis = millis();
            }
        }

        status = WiFi.status() == WL_CONNECTED || ut->ethLinkUp();
    }

    return status;
}

void FirebaseData::setSecure()
{

#if defined(ESP8266)
    if (time(nullptr) > ESP_DEFAULT_TS)
    {
        if (Signer.getCfg())
            Signer.getCfg()->_int.fb_clock_rdy = true;
        tcpClient._clockReady = true;
    }
    tcpClient._bsslRxSize = _ss.bssl_rx_size;
    tcpClient._bsslTxSize = _ss.bssl_tx_size;
#endif

    if (tcpClient._certType == -1)
    {
        if (!Signer.getCfg())
        {
            tcpClient.setCACert(nullptr);
            return;
        }

        if (!Signer.getCfg()->_int.fb_clock_rdy && (Signer.getCAFile().length() > 0 || Signer.getCfg()->_int.fb_caCert) && init())
        {

#if defined(ESP8266)
            int retry = 0;
            while (!tcpClient._clockReady && retry < 5)
            {
                ut->setClock(Signer.getCfg()->_int.fb_gmt_offset);
                tcpClient._clockReady = Signer.getCfg()->_int.fb_clock_rdy;
                retry++;
            }
#endif
        }

        if (Signer.getCAFile().length() == 0)
        {
            if (Signer.getCfg()->_int.fb_caCert)
                tcpClient.setCACert(Signer.getCfg()->_int.fb_caCert);
            else
                tcpClient.setCACert(nullptr);
        }
        else
        {

#if defined(ESP8266)
            if (Signer.getCfg()->_int.sd_config.ss == -1)
                Signer.getCfg()->_int.sd_config.ss = SD_CS_PIN;
#endif
            tcpClient.setCACertFile(Signer.getCAFile().c_str(), Signer.getCAFileStorage(), Signer.getCfg()->_int.sd_config);
        }
    }
}

bool FirebaseData::validRequest(const std::string &path)
{
    if (path.length() == 0 || (Signer.getCfg()->database_url.length() == 0 && Signer.getCfg()->host.length() == 0) || (strlen(Signer.getToken(token_type_legacy_token)) == 0 && strlen(Signer.getToken(token_type_id_token)) == 0 && strlen(Signer.getToken(token_type_oauth2_access_token)) == 0))
    {
        _ss.http_code = FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST;
        return false;
    }
    return true;
}

bool FirebaseData::tokenReady()
{
    if (!Signer.tokenReady())
    {
        _ss.http_code = FIREBASE_ERROR_TOKEN_NOT_READY;
        closeSession();
        return false;
    }
    return true;
};

void FirebaseData::checkOvf(size_t len, struct server_response_data_t &resp)
{
#ifdef ENABLE_RTDB
    if (_ss.resp_size < len && !_ss.buffer_ovf)
    {
        if (_ss.rtdb.req_method == fb_esp_method::m_get && !_ss.rtdb.data_tmo && _ss.con_mode != fb_esp_con_mode_fcm && resp.dataType != fb_esp_data_type::d_file && _ss.rtdb.req_method != fb_esp_method::m_download && _ss.rtdb.req_data_type != fb_esp_data_type::d_file)
        {
            _ss.buffer_ovf = true;
            _ss.http_code = FIREBASE_ERROR_BUFFER_OVERFLOW;
        }
    }
#endif
}

void FirebaseData::clear()
{
    if (tcpClient.stream())
    {
        if (tcpClient.stream()->connected())
            tcpClient.stream()->stop();
        _ss.connected = false;
    }

    if (_ss.arrPtr)
        _ss.arrPtr->clear();
    if (_ss.jsonPtr)
        _ss.jsonPtr->clear();

#ifdef ENABLE_RTDB

    _dataAvailableCallback = NULL;
    _multiPathDataCallback = NULL;
    _timeoutCallback = NULL;
    _queueInfoCallback = NULL;

    ut->clearS(_ss.rtdb.raw);
    ut->clearS(_ss.rtdb.push_name);
    ut->clearS(_ss.rtdb.file_name);
    ut->clearS(_ss.rtdb.redirect_url);
    ut->clearS(_ss.rtdb.event_type);
    ut->clearS(_ss.rtdb.req_etag);
    ut->clearS(_ss.rtdb.resp_etag);
    ut->clearS(_ss.rtdb.priority);
#endif

#if defined(FIREBASE_ESP_CLIENT)
#ifdef ENABLE_GC_STORAGE
    ut->clearS(_ss.gcs.meta.bucket);
    ut->clearS(_ss.gcs.meta.contentType);
    ut->clearS(_ss.gcs.meta.crc32);
    ut->clearS(_ss.gcs.meta.downloadTokens);
    ut->clearS(_ss.gcs.meta.etag);
    ut->clearS(_ss.gcs.meta.name);
#endif
#ifdef ENABLE_FB_STORAGE
    ut->clearS(_ss.fcs.meta.name);
    ut->clearS(_ss.fcs.meta.bucket);
    ut->clearS(_ss.fcs.meta.contentType);
    ut->clearS(_ss.fcs.meta.etag);
    ut->clearS(_ss.fcs.meta.crc32);
    ut->clearS(_ss.fcs.meta.downloadTokens);
    ut->clearS(_ss.fcs.meta.bucket);
    ut->clearS(_ss.fcs.meta.contentType);
    ut->clearS(_ss.fcs.meta.crc32);
    ut->clearS(_ss.fcs.meta.downloadTokens);
    ut->clearS(_ss.fcs.meta.etag);
    ut->clearS(_ss.fcs.meta.name);
    _ss.fcs.files.items.clear();
#endif
#ifdef ENABLE_FB_FUNCTIONS
    ut->clearS(_ss.cfn.payload);
#endif
#ifdef ENABLE_FIRESTORE
    ut->clearS(_ss.cfs.payload);
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

void FCMObject::begin(const String &serverKey)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_577);
    json.set(s.c_str(), serverKey.c_str());
    ut->clearS(raw);
    ut->clearS(s);
    raw = json.raw();
}

void FCMObject::addDeviceToken(const String &deviceToken)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJsonArray arr;
    arr.setJsonArrayData(idTokens.c_str());
    arr.add(deviceToken.c_str());
    ut->clearS(idTokens);
    idTokens = arr.raw();
}

void FCMObject::removeDeviceToken(uint16_t index)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJsonArray arr;
    arr.setJsonArrayData(idTokens.c_str());
    arr.remove(index);
    ut->clearS(idTokens);
    idTokens = arr.raw();
}
void FCMObject::clearDeviceToken()
{
    if (!ut)
        ut = new UtilsClass(nullptr);
    ut->clearS(idTokens);
}

void FCMObject::setNotifyMessage(const String &title, const String &body)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_285);
    json.set(s.c_str(), title);

    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_123);
    json.set(s.c_str(), body);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setNotifyMessage(const String &title, const String &body, const String &icon)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    setNotifyMessage(title, body);
    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_124);
    json.set(s.c_str(), icon);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setNotifyMessage(const String &title, const String &body, const String &icon, const String &click_action)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    setNotifyMessage(title, body, icon);
    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_125);
    json.set(s.c_str(), click_action);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::addCustomNotifyMessage(const String &key, const String &value)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    ut->appendP(s, fb_esp_pgm_str_1);
    s += key.c_str();
    json.set(s.c_str(), value);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::clearNotifyMessage()
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    FirebaseJson json(raw);
    json.remove(s.c_str());
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setDataMessage(const String &jsonString)
{

    if (!ut)
        ut = new UtilsClass(nullptr);

    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_135);
    FirebaseJson js;
    js.setJsonData(jsonString);
    FirebaseJson json(raw);
    json.set(s.c_str(), js);
    js.clear();
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setDataMessage(FirebaseJson &json)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_135);
    FirebaseJson js(raw);
    js.set(s.c_str(), json);
    ut->clearS(s);
    ut->clearS(raw);
    raw = js.raw();
}

void FCMObject::clearDataMessage()
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_135);
    FirebaseJson json(raw);
    json.remove(s.c_str());
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setPriority(const String &priority)
{
    if (!ut)
        ut = new UtilsClass(nullptr);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_136);
    FirebaseJson json(raw);
    json.set(s.c_str(), priority);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setCollapseKey(const String &key)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_138);
    FirebaseJson json(raw);
    json.set(s.c_str(), key);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setTimeToLive(uint32_t seconds)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    if (seconds <= 2419200)
        _ttl = seconds;
    else
        _ttl = -1;
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575, true);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_137);

    FirebaseJson json(raw);
    json.set(s.c_str(), _ttl);
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();
}

void FCMObject::setTopic(const String &topic)
{
    if (!ut)
        ut = new UtilsClass(nullptr);
    FirebaseJson json(raw);
    std::string s, v;
    ut->appendP(s, fb_esp_pgm_str_576);
    ut->appendP(v, fb_esp_pgm_str_134);
    v += topic.c_str();
    json.set(s.c_str(), v.c_str());
    ut->clearS(raw);
    ut->clearS(s);
    ut->clearS(v);
    raw = json.raw();
}

String FCMObject::getSendResult()
{
    return result.c_str();
}

void FCMObject::fcm_begin(FirebaseData &fbdo)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    std::string host;
    ut->appendP(host, fb_esp_pgm_str_249);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);
    rescon(fbdo, host.c_str());
    fbdo.tcpClient.begin(host.c_str(), _port);
}

int FCMObject::fcm_sendHeader(FirebaseData &fbdo, size_t payloadSize)
{
    int ret = -1;
    std::string header;
    if (!ut)
        ut = new UtilsClass(nullptr);
    FirebaseJsonData server_key;
    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_577);
    json.get(server_key, s.c_str());
    ut->clearS(s);

    ut->appendP(header, fb_esp_pgm_str_24, true);
    ut->appendP(header, fb_esp_pgm_str_6);
    ut->appendP(header, fb_esp_pgm_str_121);
    ut->appendP(header, fb_esp_pgm_str_30);

    ut->appendP(header, fb_esp_pgm_str_31);
    ut->appendP(header, fb_esp_pgm_str_249);
    ut->appendP(header, fb_esp_pgm_str_4);
    ut->appendP(header, fb_esp_pgm_str_120);
    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_131);

    ret = fbdo.tcpSend(header.c_str());
    ut->clearS(header);
    if (ret < 0)
        return ret;

    ret = fbdo.tcpSend(server_key.stringValue.c_str());
    if (ret < 0)
        return ret;

    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_32);

    ut->appendP(header, fb_esp_pgm_str_8);
    ut->appendP(header, fb_esp_pgm_str_129);
    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_12);
    char *tmp = ut->intStr(payloadSize);
    header += tmp;
    ut->delS(tmp);
    ut->appendP(header, fb_esp_pgm_str_21);
    ut->appendP(header, fb_esp_pgm_str_36);
    ut->appendP(header, fb_esp_pgm_str_21);

    ret = fbdo.tcpSend(header.c_str());
    ut->clearS(header);
    return ret;
}

void FCMObject::fcm_preparePayload(fb_esp_fcm_msg_type messageType)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJson json(raw);
    if (messageType == fb_esp_fcm_msg_type::msg_single)
    {
        std::string s;
        ut->appendP(s, fb_esp_pgm_str_575, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_128);

        FirebaseJsonArray arr;
        arr.setJsonArrayData(idTokens.c_str());
        FirebaseJsonData data;
        arr.get(data, _index);
        json.set(s.c_str(), data.stringValue.c_str());
        ut->clearS(s);
        ut->clearS(raw);
        raw = json.raw();
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_multicast)
    {
        FirebaseJsonArray arr;
        arr.setJsonArrayData(idTokens.c_str());

        std::string s;
        ut->appendP(s, fb_esp_pgm_str_575, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_130);

        json.set(s.c_str(), arr);
        ut->clearS(s);
        arr.clear();
        ut->clearS(raw);
        raw = json.raw();
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_topic)
    {
        std::string s;
        ut->appendP(s, fb_esp_pgm_str_575, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_128);

        FirebaseJsonData topic;
        std::string s2;
        ut->appendP(s2, fb_esp_pgm_str_576);
        json.get(topic, s2.c_str());
        ut->clearS(s2);
        json.set(s.c_str(), topic.stringValue.c_str());
        ut->clearS(s);
        ut->clearS(raw);
        raw = json.raw();
    }
}

bool FCMObject::waitResponse(FirebaseData &fbdo)
{
    return handleResponse(&fbdo);
}

bool FCMObject::handleResponse(FirebaseData *fbdo)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

#ifdef ENABLE_RTDB
    if (fbdo->_ss.rtdb.pause)
        return true;
#endif
    if (!fbdo->reconnect())
        return false;

    if (!fbdo->_ss.connected)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
        return false;
    }

    ut->clearS(result);

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->tcpClient.stream();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    char *header = nullptr;
    char *payload = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->_ss.resp_size;
    int pBufPos = 0;
    int hBufPos = 0;
    int chunkBufSize = stream->available();
    int hstate = 0;
    int pstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->_ss.resp_size;
    int payloadRead = 0;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;

    fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 768;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        delay(0);
    }

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0)
        {
            if (!fbdo->reconnect())
                return false;

            chunkBufSize = stream->available();

            if (chunkBufSize <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    //the first chunk can be http response header
                    header = ut->newS(chunkBufSize);
                    hstate = 1;
                    int readLen = ut->readLine(stream, header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    delay(0);
                    dataTime = millis();
                    if (tmp)
                    {
                        //http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        fbdo->_ss.http_code = response.httpCode;
                        ut->delS(tmp);
                    }
                    else
                    {
                        payload = ut->newS(payloadLen);
                        pstate = 1;
                        memcpy(payload, header, readLen);
                        pBufPos = readLen;
                        ut->delS(header);
                        hstate = 0;
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
                        tmp = ut->newS(chunkBufSize);
                        int readLen = ut->readLine(stream, tmp, chunkBufSize);
                        bool headerEnded = false;

                        //check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (tmp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (tmp[0] == '\r' && tmp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {
                            //parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            if (hstate == 1)
                                ut->delS(header);
                            hstate = 0;

                            fbdo->_ss.chunked_encoding = response.isChunkedEnc;
                        }
                        else
                        {
                            //accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }

                        ut->delS(tmp);
                    }
                    else
                    {
                        //the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;

                            pChunk = ut->newS(chunkBufSize + 1);

                            if (!payload || pstate == 0)
                            {
                                pstate = 1;
                                payload = ut->newS(payloadLen + 1);
                            }

                            //read the avilable data
                            int readLen = 0;

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
                                payloadRead += readLen;
                                fbdo->checkOvf(pBufPos + readLen, response);

                                if (!fbdo->_ss.buffer_ovf)
                                {
                                    if (pBufPos + readLen <= payloadLen)
                                        memcpy(payload + pBufPos, pChunk, readLen);
                                    else
                                    {
                                        //in case of the accumulated payload size is bigger than the char array
                                        //reallocate the char array

                                        char *buf = ut->newS(pBufPos + readLen + 1);
                                        memcpy(buf, payload, pBufPos);

                                        memcpy(buf + pBufPos, pChunk, readLen);

                                        payloadLen = pBufPos + readLen;
                                        ut->delS(payload);
                                        payload = ut->newS(payloadLen + 1);
                                        memcpy(payload, buf, payloadLen);
                                        ut->delS(buf);
                                    }
                                }
                            }

                            ut->delS(pChunk);
                            if (readLen < 0 && payloadRead >= response.contentLen)
                                break;
                            if (readLen > 0)
                                pBufPos += readLen;
                        }
                        else
                        {
                            //read all the rest data
                            while (stream->available() > 0)
                                stream->read();
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delS(header);

        if (payload)
        {
            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK)
                result = payload;
            else
            {
                std::string t = ut->trim(payload);
                if (t[0] == '{' && t[t.length() - 1] == '}')
                {
                    FirebaseJson json;
                    FirebaseJsonData data;
                    json.setJsonData(t.c_str());

                    char *tmp = ut->strP(fb_esp_pgm_str_257);
                    json.get(data, tmp);
                    ut->delS(tmp);

                    if (data.success)
                    {
                        error.code = data.intValue;
                        tmp = ut->strP(fb_esp_pgm_str_258);
                        json.get(data, tmp);
                        ut->delS(tmp);
                        if (data.success)
                            fbdo->_ss.error = data.stringValue.c_str();
                    }
                    else
                        error.code = 0;
                }
            }
        }

        if (pstate == 1)
            ut->delS(payload);

        return error.code == 0 || response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK;
    }
    else
    {
        while (stream->available() > 0)
            stream->read();
    }

    return false;
}

bool FCMObject::fcm_send(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    if (!ut)
        ut = new UtilsClass(nullptr);

    FirebaseJsonData msg;

    fcm_preparePayload(messageType);

    FirebaseJson json(raw);
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_575);
    json.get(msg, s.c_str());

    int ret = fcm_sendHeader(fbdo, msg.stringValue.length());
    if (ret == 0)
        ret = fbdo.tcpSend(msg.stringValue.c_str());
    json.remove(s.c_str());
    ut->clearS(s);
    ut->clearS(raw);
    raw = json.raw();

    if (ret != 0)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
        fbdo.closeSession();
        if (Signer.getCfg())
            Signer.getCfg()->_int.fb_processing = false;
        return false;
    }
    else
        fbdo._ss.connected = true;

    ret = waitResponse(fbdo);

    if (!ret)
        fbdo.closeSession();

    if (Signer.getCfg())
        Signer.getCfg()->_int.fb_processing = false;

    return ret;
}

void FCMObject::rescon(FirebaseData &fbdo, const char *host)
{
    if (!fbdo._ss.connected || millis() - fbdo._ss.last_conn_ms > fbdo._ss.conn_timeout || fbdo._ss.con_mode != fb_esp_con_mode_fcm || strcmp(host, fbdo._ss.host.c_str()) != 0)
    {
        fbdo._ss.last_conn_ms = millis();
        fbdo.closeSession();
        fbdo.setSecure();
    }
    fbdo._ss.host = host;
    fbdo._ss.con_mode = fb_esp_con_mode_fcm;
}

void FCMObject::clear()
{
    ut->clearS(raw);
    ut->clearS(result);
    _ttl = -1;
    _index = 0;
    clearDeviceToken();

    if (ut)
        delete ut;
}
#endif
#endif

#endif