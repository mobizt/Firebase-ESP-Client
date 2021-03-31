/**
 * Google's Firebase Data class, FB_Session.cpp version 1.0.6
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created April 1, 2021
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
}

bool FirebaseData::init()
{
    if (!Signer.getCfg())
        return false;

    if (!ut)
        ut = new UtilsClass(Signer.getCfg());

    return true;
}

void FirebaseData::clearQueueItem(QueueItem *item)
{
    std::string().swap(item->path);
    std::string().swap(item->filename);
    std::string().swap(item->payload);

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
        std::string().swap(_ss.rtdb.child_nodes[i]);
    _ss.rtdb.child_nodes.clear();
}

void FirebaseData::addNodeList(const String childPath[], size_t size)
{
    clearNodeList();
    for (size_t i = 0; i < size; i++)
        if (childPath[i].length() > 0 && childPath[i] != "/")
            _ss.rtdb.child_nodes.push_back(childPath[i].c_str());
}

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

bool FirebaseData::pauseFirebase(bool pause)
{
    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
        return false;

    if (httpClient.connected() && pause != _ss.rtdb.pause)
    {
        if (httpClient.stream()->connected())
            httpClient.stream()->stop();

        _ss.connected = false;

        if (!httpClient.connected())
        {
            _ss.rtdb.pause = pause;
            return true;
        }
        return false;
    }
    else
    {
        _ss.rtdb.pause = pause;
        return true;
    }
}

void FirebaseData::stopWiFiClient()
{
    if (httpClient.stream())
    {
        if (httpClient.stream()->connected())
            httpClient.stream()->stop();
    }
    _ss.connected = false;
}

WiFiClientSecure *FirebaseData::getWiFiClient()
{
    return httpClient._wcs.get();
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
    std::string res = "";

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
    std::string res = "";

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
    if ((_ss.rtdb.data.length() > 0 && (_ss.rtdb.resp_data_type == fb_esp_data_type::d_integer || _ss.rtdb.resp_data_type == fb_esp_data_type::d_float || _ss.rtdb.resp_data_type == fb_esp_data_type::d_double)))
    {
        if (_ss.rtdb.req_data_type == fb_esp_data_type::d_timestamp)
        {
            double d = atof(_ss.rtdb.data.c_str());
            int ts = d / 1000;
            return ts;
        }
        else
            return atoi(_ss.rtdb.data.c_str());
    }
    else
        return 0;
}

float FirebaseData::floatData()
{
    if (_ss.rtdb.data.length() > 0 && (_ss.rtdb.resp_data_type == fb_esp_data_type::d_integer || _ss.rtdb.resp_data_type == fb_esp_data_type::d_float || _ss.rtdb.resp_data_type == fb_esp_data_type::d_double))
        return atof(_ss.rtdb.data.c_str());
    else
        return 0.0;
}

double FirebaseData::doubleData()
{
    if (_ss.rtdb.data.length() > 0 && (_ss.rtdb.resp_data_type == fb_esp_data_type::d_integer || _ss.rtdb.resp_data_type == fb_esp_data_type::d_float || _ss.rtdb.resp_data_type == fb_esp_data_type::d_double))
        return atof(_ss.rtdb.data.c_str());
    else
        return 0.0;
}

bool FirebaseData::boolData()
{
    if (!init())
        return false;

    bool res = false;
    char *str = ut->boolStr(true);
    if (_ss.rtdb.data.length() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_boolean)
        res = strcmp(_ss.rtdb.data.c_str(), str) == 0;
    ut->delS(str);
    return res;
}

String FirebaseData::stringData()
{
    if (_ss.rtdb.data.length() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_string)
        return _ss.rtdb.data.substr(1, _ss.rtdb.data.length() - 2).c_str();
    else
        return std::string().c_str();
}

String FirebaseData::jsonString()
{
    if (_ss.rtdb.resp_data_type == fb_esp_data_type::d_json)
    {
        String s;
        _ss.json.toString(s);
        return s;
    }
    else
        return String();
}

FirebaseJson *FirebaseData::jsonObjectPtr()
{
    return &_ss.json;
}

FirebaseJson &FirebaseData::jsonObject()
{
    return *jsonObjectPtr();
}

FirebaseJsonArray *FirebaseData::jsonArrayPtr()
{
    if (!init())
        return nullptr;

    if (_ss.rtdb.data.length() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_array)
    {

        std::string().swap(*_ss.arr.int_dbuf());
        std::string().swap(*_ss.arr.int_tbuf());

        char *tmp = ut->strP(fb_json_str_21);
        _ss.arr.int_json()->int_toStdString(*_ss.arr.int_jbuf());
        *_ss.arr.int_rawbuf() = tmp;
        *_ss.arr.int_rawbuf() += _ss.rtdb.data;
        ut->delS(tmp);

        tmp = ut->strP(fb_json_str_26);
        _ss.arr.int_json()->int_parse(tmp, FirebaseJson::PRINT_MODE_PLAIN);
        ut->delS(tmp);

        std::string().swap(*_ss.arr.int_tbuf());
        std::string().swap(*_ss.arr.int_jbuf());
        _ss.arr.int_json()->int_clearPathTk();
        _ss.arr.int_json()->int_clearTokens();

        if (_ss.arr.int_dbuf()->length() > 2)
            *_ss.arr.int_rawbuf() = _ss.arr.int_dbuf()->substr(1, _ss.arr.int_dbuf()->length() - 2);
        _ss.arr.int_set_arr_len(_ss.arr.int_json()->int_get_jsondata_len());
    }
    return &_ss.arr;
}

FirebaseJsonArray &FirebaseData::jsonArray()
{
    return *jsonArrayPtr();
}

FirebaseJsonData &FirebaseData::jsonData()
{
    return _ss.data;
}

FirebaseJsonData *FirebaseData::jsonDataPtr()
{
    return &_ss.data;
}

std::vector<uint8_t> FirebaseData::blobData()
{
    if (_ss.rtdb.blob.size() > 0 && _ss.rtdb.resp_data_type == fb_esp_data_type::d_blob)
    {
        return _ss.rtdb.blob;
    }
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

bool FirebaseData::httpConnected()
{
    return _ss.connected;
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

bool FirebaseData::bufferOverflow()
{
    return _ss.buffer_ovf;
}

size_t FirebaseData::getBackupFileSize()
{
    return _ss.rtdb.backup_file_size;
}

String FirebaseData::getBackupFilename()
{
    return _ss.rtdb.backup_filename.c_str();
}

String FirebaseData::fileTransferError()
{
    return _ss.error.c_str();
}

String FirebaseData::payload()
{
    if (_ss.con_mode == fb_esp_con_mode_rtdb)
        return _ss.rtdb.data.c_str();
    else if (_ss.con_mode == fb_esp_con_mode_firestore)
        return _ss.cfs.payload.c_str();
    else if (_ss.con_mode == fb_esp_con_mode_functions)
        return _ss.cfn.payload.c_str();
    else
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

FileMetaInfo FirebaseData::metaData()
{
    if (_ss.con_mode == fb_esp_con_mode_gc_storage)
        return _ss.gcs.meta;
    else
        return _ss.fcs.meta;
}

FileList *FirebaseData::fileList()
{
    return &_ss.fcs.files;
}

String FirebaseData::downloadURL()
{
    if (!init())
        return "";

    std::string link;
    if (_ss.con_mode == fb_esp_con_mode_storage)
    {
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
    }
    else if (_ss.con_mode == fb_esp_con_mode_gc_storage)
    {
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
    }

    return link.c_str();
}

int FirebaseData::httpCode()
{
    return _ss.http_code;
}

void FirebaseData::closeSession()
{
    if (WiFi.status() == WL_CONNECTED || ut->ethLinkUp())
    {
        //close the socket and free the resources used by the BearSSL data
        if (_ss.connected || httpClient.stream())
        {
            Signer.getCfg()->_int.fb_last_reconnect_millis = millis();
            if (httpClient.stream())
                if (httpClient.stream()->connected())
                    httpClient.stream()->stop();
        }
    }

    if (_ss.con_mode == fb_esp_con_mode_rtdb_stream)
    {
        _ss.rtdb.stream_tmo_Millis = millis();
        _ss.rtdb.data_millis = millis();
        _ss.rtdb.data_tmo = false;
    }
    _ss.connected = false;
}

bool FirebaseData::reconnect(unsigned long dataTime)
{

    bool status = WiFi.status() == WL_CONNECTED || ut->ethLinkUp();

    if (dataTime > 0)
    {
        if (millis() - dataTime > httpClient.timeout && init())
        {
            _ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_READ_TIMEOUT;
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

        _ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST;

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
        httpClient._clockReady = true;
    }
    httpClient._bsslRxSize = _ss.bssl_rx_size;
    httpClient._bsslTxSize = _ss.bssl_tx_size;
#endif

    if (httpClient._certType == -1)
    {

        if (!Signer.getCfg()->_int.fb_clock_rdy && (Signer.getCAFile().length() > 0 || Signer.getCfg()->_int.fb_caCert) && init())
        {

#if defined(ESP8266)
            int retry = 0;
            while (!httpClient._clockReady && retry < 5)
            {
                ut->setClock(Signer.getCfg()->_int.fb_gmt_offset);
                httpClient._clockReady = Signer.getCfg()->_int.fb_clock_rdy;
                retry++;
            }
#endif
        }

        if (Signer.getCAFile().length() == 0)
        {
            if (Signer.getCfg()->_int.fb_caCert)
                httpClient.setCACert(Signer.getCfg()->_int.fb_caCert);
            else
                httpClient.setCACert(nullptr);
        }
        else
        {

#if defined(ESP8266)
            if (Signer.getCfg()->_int.sd_config.ss == -1)
                Signer.getCfg()->_int.sd_config.ss = SD_CS_PIN;
#endif
            httpClient.setCACertFile(Signer.getCAFile().c_str(), Signer.getCAFileStorage(), Signer.getCfg()->_int.sd_config);
        }
    }
}

bool FirebaseData::validRequest(const std::string &path)
{
    if (path.length() == 0 || Signer.getCfg()->host.length() == 0 || (Signer.getToken(token_type_legacy_token).length() == 0 && Signer.getToken(token_type_id_token).length() == 0 && Signer.getToken(token_type_oauth2_access_token).length() == 0))
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
    if (_ss.resp_size < len && !_ss.buffer_ovf)
    {
        if (_ss.rtdb.req_method == fb_esp_method::m_get && !_ss.rtdb.data_tmo && _ss.con_mode != fb_esp_con_mode_fcm && resp.dataType != fb_esp_data_type::d_file && _ss.rtdb.req_method != fb_esp_method::m_download && _ss.rtdb.req_data_type != fb_esp_data_type::d_file)
        {
            _ss.buffer_ovf = true;
            _ss.http_code = FIREBASE_ERROR_BUFFER_OVERFLOW;
        }
    }
}

void FirebaseData::addQueue(struct fb_esp_rtdb_queue_info_t *qinfo)
{
    if (_qMan._queueCollection.size() < _qMan._maxQueue && qinfo->payload.length() <= _ss.rtdb.max_blob_size)
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

        item.stringPtr = qinfo->stringTarget;
        item.intPtr = qinfo->intTarget;
        item.floatPtr = qinfo->floatTarget;
        item.doublePtr = qinfo->doubleTarget;
        item.boolPtr = qinfo->boolTarget;
        item.jsonPtr = qinfo->jsonTarget;
        item.arrPtr = qinfo->arrTarget;
        item.blobPtr = qinfo->blobTarget;
        item.qID = random(100000, 200000);
        item.storageType = qinfo->storageType;
        if (_qMan.add(item))
            _ss.rtdb.queue_ID = item.qID;
        else
            _ss.rtdb.queue_ID = 0;
    }
}

void FirebaseData::clear()
{
    _dataAvailableCallback = NULL;
    _multiPathDataCallback = NULL;
    _timeoutCallback = NULL;
    _queueInfoCallback = NULL;

    if (httpClient.stream())
    {
        if (httpClient.stream()->connected())
            httpClient.stream()->stop();
        _ss.connected = false;
    }
    _ss.cfn.payload.clear();
    std::string()
        .swap(_ss.cfn.payload);
    _ss.json.clear();
    _ss.arr.clear();

    std::string().swap(_ss.rtdb.data);
    std::string().swap(_ss.rtdb.data2);
    std::string().swap(_ss.rtdb.push_name);
    std::string().swap(_ss.rtdb.file_name);
    std::string().swap(_ss.rtdb.redirect_url);
    std::string().swap(_ss.rtdb.event_type);
    std::string().swap(_ss.rtdb.req_etag);
    std::string().swap(_ss.rtdb.resp_etag);
    std::string().swap(_ss.rtdb.priority);

    std::string().swap(_ss.gcs.meta.bucket);
    std::string().swap(_ss.gcs.meta.contentType);
    std::string().swap(_ss.gcs.meta.crc32);
    std::string().swap(_ss.gcs.meta.downloadTokens);
    std::string().swap(_ss.gcs.meta.etag);
    std::string().swap(_ss.gcs.meta.name);

    std::string().swap(_ss.fcs.meta.name);
    std::string().swap(_ss.fcs.meta.bucket);
    std::string().swap(_ss.fcs.meta.contentType);
    std::string().swap(_ss.fcs.meta.etag);
    std::string().swap(_ss.fcs.meta.crc32);
    std::string().swap(_ss.fcs.meta.downloadTokens);
    std::string().swap(_ss.fcs.meta.bucket);
    std::string().swap(_ss.fcs.meta.contentType);
    std::string().swap(_ss.fcs.meta.crc32);
    std::string().swap(_ss.fcs.meta.downloadTokens);
    std::string().swap(_ss.fcs.meta.etag);
    std::string().swap(_ss.fcs.meta.name);
    _ss.fcs.files.items.clear();
    std::string().swap(_ss.cfs.payload);
}

#endif