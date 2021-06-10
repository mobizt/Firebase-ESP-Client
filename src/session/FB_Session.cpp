/**
 * Google's Firebase Data class, FB_Session.cpp version 1.0.11
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
    if ((WiFi.status() != WL_CONNECTED && !ut->ethLinkUp()) || !httpClient.stream())
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
        if (httpClient.stream()->connected())
            httpClient.stream()->stop();
        _ss.connected = false;
    }

    return true;
}

bool FirebaseData::isPause()
{
    return _ss.rtdb.pause;
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

uint8_t FirebaseData::dataTypeEnum()
{
    return _ss.rtdb.data_type;
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
#if defined(FIREBASE_ESP_CLIENT)
    else if (_ss.con_mode == fb_esp_con_mode_firestore)
        return _ss.cfs.payload.c_str();
    else if (_ss.con_mode == fb_esp_con_mode_functions)
        return _ss.cfn.payload.c_str();
#endif
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

#if defined(FIREBASE_ESP_CLIENT)
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
    if (path.length() == 0 || (Signer.getCfg()->database_url.length() == 0 && Signer.getCfg()->host.length() == 0) || (Signer.getToken(token_type_legacy_token).length() == 0 && Signer.getToken(token_type_id_token).length() == 0 && Signer.getToken(token_type_oauth2_access_token).length() == 0))
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
#if defined(FIREBASE_ESP_CLIENT)
    _ss.cfn.payload.clear();
    std::string().swap(_ss.cfn.payload);
#endif
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
#if defined(FIREBASE_ESP_CLIENT)
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
#endif
}

#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)

FCMObject::FCMObject()
{
}
FCMObject::~FCMObject()
{
    clear();
}

bool FCMObject::init()
{
    if (!Signer.getCfg())
        return false;

    if (_ut == nullptr)
        _ut = new UtilsClass(Signer.getCfg());

    return true;
}

void FCMObject::begin(UtilsClass *u)
{
    _ut = u;
}

void FCMObject::begin(const String &serverKey)
{
    if (!init())
    {
        if (_ut)
            delete _ut;

        _ut = new UtilsClass(&_cfg_);

        Signer.begin(_ut, &_cfg_, &_auth_);
    }
    _server_key = serverKey.c_str();
}

void FCMObject::addDeviceToken(const String &deviceToken)
{
    _deviceToken.push_back(deviceToken.c_str());
}
void FCMObject::removeDeviceToken(uint16_t index)
{
    if (_deviceToken.size() > 0)
    {
        std::string().swap(_deviceToken[index]);
        _deviceToken.erase(_deviceToken.begin() + index);
    }
}
void FCMObject::clearDeviceToken()
{
    for (size_t i = 0; i < _deviceToken.size(); i++)
    {
        std::string().swap(_deviceToken[i]);
        _deviceToken.erase(_deviceToken.begin() + i);
    }
}

void FCMObject::setNotifyMessage(const String &title, const String &body)
{
    if (!init())
        return;
    std::string s;
    _ut->appendP(s, fb_esp_pgm_str_122, true);
    _ut->appendP(s, fb_esp_pgm_str_1);
    _ut->appendP(s, fb_esp_pgm_str_285);
    _fcmPayload.set(s.c_str(), title);

    _ut->appendP(s, fb_esp_pgm_str_122, true);
    _ut->appendP(s, fb_esp_pgm_str_1);
    _ut->appendP(s, fb_esp_pgm_str_123);
    _fcmPayload.set(s.c_str(), body);
}

void FCMObject::setNotifyMessage(const String &title, const String &body, const String &icon)
{
    if (!init())
        return;
    setNotifyMessage(title, body);

    std::string s;
    _ut->appendP(s, fb_esp_pgm_str_122, true);
    _ut->appendP(s, fb_esp_pgm_str_1);
    _ut->appendP(s, fb_esp_pgm_str_124);
    _fcmPayload.set(s.c_str(), icon);
}

void FCMObject::setNotifyMessage(const String &title, const String &body, const String &icon, const String &click_action)
{
    if (!init())
        return;
    setNotifyMessage(title, body, icon);
    std::string s;
    _ut->appendP(s, fb_esp_pgm_str_122, true);
    _ut->appendP(s, fb_esp_pgm_str_1);
    _ut->appendP(s, fb_esp_pgm_str_125);
    _fcmPayload.set(s.c_str(), click_action);
}

void FCMObject::addCustomNotifyMessage(const String &key, const String &value)
{
    if (!init())
        return;
    std::string s;
    _ut->appendP(s, fb_esp_pgm_str_122, true);
    _ut->appendP(s, fb_esp_pgm_str_1, false);
    s += key.c_str();
    _fcmPayload.set(s.c_str(), value);
    std::string().swap(s);
}

void FCMObject::clearNotifyMessage()
{
    if (!init())
        return;
    char *key = _ut->strP(fb_esp_pgm_str_122);
    _fcmPayload.remove(key);
    _ut->delS(key);
}

void FCMObject::setDataMessage(const String &jsonString)
{
    if (!init())
        return;

    char *key = _ut->strP(fb_esp_pgm_str_135);
    FirebaseJson js;
    js.setJsonData(jsonString);
    _fcmPayload.set(key, js);
    _ut->delS(key);
    js.clear();
}

void FCMObject::setDataMessage(FirebaseJson &json)
{
    if (!init())
        return;
    char *key = _ut->strP(fb_esp_pgm_str_135);
    _fcmPayload.set(key, json);
    _ut->delS(key);
}

void FCMObject::clearDataMessage()
{
    if (!init())
        return;
    char *key = _ut->strP(fb_esp_pgm_str_135);
    _fcmPayload.remove(key);
    _ut->delS(key);
}

void FCMObject::setPriority(const String &priority)
{
    if (!init())
        return;
    char *key = _ut->strP(fb_esp_pgm_str_136);
    _fcmPayload.set(key, priority);
    _ut->delS(key);
}

void FCMObject::setCollapseKey(const String &key)
{
    if (!init())
        return;
    char *_key = _ut->strP(fb_esp_pgm_str_138);
    _fcmPayload.set(_key, key);
    _ut->delS(_key);
}

void FCMObject::setTimeToLive(uint32_t seconds)
{
    if (!init())
        return;
    if (seconds <= 2419200)
        _ttl = seconds;
    else
        _ttl = -1;
    char *key = _ut->strP(fb_esp_pgm_str_137);
    _fcmPayload.set(key, _ttl);
    _ut->delS(key);
}

void FCMObject::setTopic(const String &topic)
{
    if (!init())
        return;
    _ut->appendP(_topic, fb_esp_pgm_str_134, true);
    _topic += topic.c_str();
}

String FCMObject::getSendResult()
{
    return _sendResult.c_str();
}

void FCMObject::fcm_begin(FirebaseData &fbdo)
{
    if (!init())
        return;
    std::string host;
    _ut->appendP(host, fb_esp_pgm_str_249);
    _ut->appendP(host, fb_esp_pgm_str_4);
    _ut->appendP(host, fb_esp_pgm_str_120);
    rescon(fbdo, host.c_str());
    fbdo.httpClient.begin(host.c_str(), _port);
}

void FCMObject::fcm_prepareHeader(std::string &header, size_t payloadSize)
{
    if (!init())
        return;

    char *len = nullptr;

    _ut->appendP(header, fb_esp_pgm_str_24, true);
    _ut->appendP(header, fb_esp_pgm_str_6);
    _ut->appendP(header, fb_esp_pgm_str_121);
    _ut->appendP(header, fb_esp_pgm_str_30);

    _ut->appendP(header, fb_esp_pgm_str_31);
    _ut->appendP(header, fb_esp_pgm_str_249);
    _ut->appendP(header, fb_esp_pgm_str_4);
    _ut->appendP(header, fb_esp_pgm_str_120);
    _ut->appendP(header, fb_esp_pgm_str_21);

    _ut->appendP(header, fb_esp_pgm_str_131);
    header += _server_key;
    _ut->appendP(header, fb_esp_pgm_str_21);

    _ut->appendP(header, fb_esp_pgm_str_32);

    _ut->appendP(header, fb_esp_pgm_str_8);
    _ut->appendP(header, fb_esp_pgm_str_129);
    _ut->appendP(header, fb_esp_pgm_str_21);

    _ut->appendP(header, fb_esp_pgm_str_12);
    len = _ut->intStr(payloadSize);
    header += len;
    _ut->delS(len);
    _ut->appendP(header, fb_esp_pgm_str_21);
    _ut->appendP(header, fb_esp_pgm_str_36);
    _ut->appendP(header, fb_esp_pgm_str_21);
}

void FCMObject::fcm_preparePayload(std::string &msg, fb_esp_fcm_msg_type messageType)
{
    if (!init())
        return;
    char *tmp = nullptr;
    if (messageType == fb_esp_fcm_msg_type::msg_single)
    {
        tmp = _ut->strP(fb_esp_pgm_str_128);
        _fcmPayload.add(tmp, _deviceToken[_index].c_str());
        _ut->delS(tmp);
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_multicast)
    {
        FirebaseJsonArray arr;
        for (uint16_t i = 0; i < _deviceToken.size(); i++)
            arr.add(_deviceToken[i].c_str());
        tmp = _ut->strP(fb_esp_pgm_str_130);
        _fcmPayload.add(tmp, arr);
        _ut->delS(tmp);
        arr.clear();
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_topic)
    {
        tmp = _ut->strP(fb_esp_pgm_str_128);
        _fcmPayload.add(tmp, _topic.c_str());
        _ut->delS(tmp);
    }

    String s;
    _fcmPayload.toString(s);
    msg = s.c_str();
    s.clear();
}

bool FCMObject::waitResponse(FirebaseData &fbdo)
{
    return handleResponse(&fbdo);
}

bool FCMObject::handleResponse(FirebaseData *fbdo)
{
    if (!init())
        return false;

    UtilsClass *ut = _ut;

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect())
        return false;

    if (!fbdo->_ss.connected)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;
        return false;
    }

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->httpClient.stream();

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

    while (fbdo->httpClient.connected() && chunkBufSize <= 0)
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
                _sendResult = payload;
            else
            {
                std::string t = ut->trim(payload);
                if (t[0] == '{' && t[t.length() - 1] == '}')
                {
                    fbdo->_ss.json.setJsonData(t.c_str());

                    char *tmp = ut->strP(fb_esp_pgm_str_257);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);

                    if (fbdo->_ss.data.success)
                    {
                        error.code = fbdo->_ss.data.intValue;
                        tmp = ut->strP(fb_esp_pgm_str_258);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.error = fbdo->_ss.data.stringValue.c_str();
                    }
                    else
                        error.code = 0;
                }

                fbdo->_ss.json.clear();
                fbdo->_ss.arr.clear();
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
    if (!init())
        return false;

    std::string msg = "";
    std::string header = "";

    fcm_preparePayload(msg, messageType);
    fcm_prepareHeader(header, msg.length());

    int ret = fbdo.httpClient.send(header.c_str(), msg.c_str());
    std::string().swap(msg);
    std::string().swap(header);
    _fcmPayload.clear();
    if (ret != 0)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;
        fbdo.closeSession();
        Signer.getCfg()->_int.fb_processing = false;
        return false;
    }
    else
        fbdo._ss.connected = true;

    ret = waitResponse(fbdo);

    if (!ret)
        fbdo.closeSession();
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
    _fcmPayload.clear();
    std::string().swap(_topic);
    std::string().swap(_server_key);
    std::string().swap(_sendResult);
    _ttl = -1;
    _index = 0;
    clearDeviceToken();
    std::vector<std::string>().swap(_deviceToken);
}
#endif

#endif