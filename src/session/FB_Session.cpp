/**
 * Google's Firebase Data class, FB_Session.cpp version 1.2.14
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 21, 2022
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

FirebaseData::~FirebaseData()
{
    if (ut && intCfg)
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

    this->ut = Signer.getUtils();

    this->mbfs = Signer.getMBFS();

    return true;
}

//Double quotes string trim.
void FirebaseData::setRaw(bool trim)
{
    if (_ss.rtdb.raw.length() > 0)
    {
        if (trim)
        {
            if (_ss.rtdb.raw[0] == '"' && _ss.rtdb.raw[_ss.rtdb.raw.length() - 1] == '"')
            {
                _ss.rtdb.raw.pop_back();
                _ss.rtdb.raw.erase(0, 1);
            }
        }
        else
        {
            if (_ss.rtdb.raw[0] != '"' && _ss.rtdb.raw[_ss.rtdb.raw.length() - 1] != '"')
            {
                _ss.rtdb.raw.insert(0, '"');
                _ss.rtdb.raw += '"';
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
        value[0] == '-' ? iVal.int64 = strtoll(value, &pEnd, 10) : iVal.uint64 = strtoull(value, &pEnd, 10);
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

void FirebaseData::ethDNSWorkAround(SPI_ETH_Module *spi_ethernet_module, const char *host, int port)
{
#if defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)

    if (!spi_ethernet_module)
        goto ex;

#if defined(INC_ENC28J60_LWIP)
    if (spi_ethernet_module->enc28j60)
        goto ex;
#endif
#if defined(INC_W5100_LWIP)
    if (spi_ethernet_module->w5100)
        goto ex;
#endif
#if defined(INC_W5100_LWIP)
    if (spi_ethernet_module->w5500)
        goto ex;
#endif
    return;
ex:
    WiFiClient client;
    client.connect(host, port);
    client.stop();

#endif
}

bool FirebaseData::ethLinkUp(SPI_ETH_Module *spi_ethernet_module)
{
    bool ret = false;
#if defined(ESP32)
    if (strcmp(ETH.localIP().toString().c_str(), "0.0.0.0") != 0)
    {
        ret = true;
        ETH.linkUp(); //returns false in core v2.0.x?
    }
#elif defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)

    if (!spi_ethernet_module)
        return ret;

#if defined(INC_ENC28J60_LWIP)
    if (spi_ethernet_module->enc28j60)
        return spi_ethernet_module->enc28j60->status() == WL_CONNECTED;
#endif
#if defined(INC_W5100_LWIP)
    if (spi_ethernet_module->w5100)
        return spi_ethernet_module->w5100->status() == WL_CONNECTED;
#endif
#if defined(INC_W5100_LWIP)
    if (spi_ethernet_module->w5500)
        return spi_ethernet_module->w5500->status() == WL_CONNECTED;
#endif

#endif
    return ret;
}

bool FirebaseData::pauseFirebase(bool pause)
{
    bool status = WiFi.status() == WL_CONNECTED;

    if (init())
    {
        if (_spi_ethernet_module)
            status |= ethLinkUp(_spi_ethernet_module);
        else
            status |= ethLinkUp(&(Signer.getCfg()->spi_ethernet_module));
    }
    else
        status |= ethLinkUp(_spi_ethernet_module);

    if (!status || !tcpClient.stream())
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

MB_String FirebaseData::getDataType(uint8_t type)
{
    if (!init())
        return "";
    MB_String res;

    switch (type)
    {
    case fb_esp_data_type::d_json:
        res.appendP(fb_esp_pgm_str_74);
        break;
    case fb_esp_data_type::d_array:
        res.appendP(fb_esp_pgm_str_165);
        break;
    case fb_esp_data_type::d_string:
        res.appendP(fb_esp_pgm_str_75);
        break;
    case fb_esp_data_type::d_float:
        res.appendP(fb_esp_pgm_str_76);
        break;
    case fb_esp_data_type::d_double:
        res.appendP(fb_esp_pgm_str_108);
        break;
    case fb_esp_data_type::d_boolean:
        res.appendP(fb_esp_pgm_str_105);
        break;
    case fb_esp_data_type::d_integer:
        res.appendP(fb_esp_pgm_str_77);
        break;
    case fb_esp_data_type::d_blob:
        res.appendP(fb_esp_pgm_str_91);
        break;
    case fb_esp_data_type::d_file:
        res.appendP(fb_esp_pgm_str_183);
        break;
    case fb_esp_data_type::d_null:
        res.appendP(fb_esp_pgm_str_78);
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
        res.appendP(fb_esp_pgm_str_115);
        break;
    case fb_esp_method::m_put:
    case fb_esp_method::m_put_nocontent:
        res.appendP(fb_esp_pgm_str_116);
        break;
    case fb_esp_method::m_post:
        res.appendP(fb_esp_pgm_str_117);
        break;
    case fb_esp_method::m_patch:
    case fb_esp_method::m_patch_nocontent:
        res.appendP(fb_esp_pgm_str_118);
        break;
    case fb_esp_method::m_delete:
        res.appendP(fb_esp_pgm_str_119);
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
    if (!init())
        return 0;

    if (_ss.rtdb.req_data_type == fb_esp_data_type::d_timestamp)
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
    if (_ss.rtdb.resp_data_type == fb_esp_data_type::d_json)
        return _ss.rtdb.raw.c_str();
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

std::vector<uint8_t> *FirebaseData::blobData()
{
    return to<std::vector<uint8_t> *>();
}

#if defined(MBFS_FLASH_FS)
fs::File FirebaseData::fileStream()
{
    return to<File>();
}
#endif

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

    if (Signer.getCfg()->timeout.rtdbStreamError < MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL || Signer.getCfg()->timeout.rtdbStreamError > MAX_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL)
        Signer.getCfg()->timeout.rtdbStreamError = MIN_RTDB_STREAM_ERROR_NOTIFIED_INTERVAL;

    if (millis() - Signer.getCfg()->timeout.rtdbStreamError > _ss.rtdb.stream_tmo_Millis || _ss.rtdb.stream_tmo_Millis == 0)
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
    return _ss.rtdb.file_size;
}

String FirebaseData::getBackupFilename()
{
    return _ss.rtdb.filename.c_str();
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
    if (_ss.error.length() > 0)
        return _ss.error.c_str();
    else
        return errorReason();
}

String FirebaseData::payload()
{
#ifdef ENABLE_RTDB
    if (_ss.con_mode == fb_esp_con_mode_rtdb)
    {
        if (_ss.rtdb.resp_data_type == fb_esp_data_type::d_string)
            setRaw(false); //if double quotes trimmed string, retain it.
        return _ss.rtdb.raw.c_str();
    }
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
    if (_ss.error.length() > 0)
        return _ss.error.c_str();
    else
    {
        MB_String buf;
        Signer.errorToString(_ss.http_code, buf);
        if (buf.length() == 0)
            buf.appendP(fb_esp_pgm_str_83);
        return buf.c_str();
    }
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

    MB_String link;
    if (_ss.con_mode == fb_esp_con_mode_storage)
    {
#ifdef ENABLE_FB_STORAGE
        if (_ss.fcs.meta.downloadTokens.length() > 0)
        {
            link.appendP(fb_esp_pgm_str_112);
            link.appendP(fb_esp_pgm_str_265);
            link.appendP(fb_esp_pgm_str_120);
            link.appendP(fb_esp_pgm_str_266);
            link += _ss.fcs.meta.bucket;
            link.appendP(fb_esp_pgm_str_267);
            link.appendP(fb_esp_pgm_str_1);
            link += ut->url_encode(_ss.fcs.meta.name);
            link.appendP(fb_esp_pgm_str_173);
            link.appendP(fb_esp_pgm_str_269);
            link.appendP(fb_esp_pgm_str_172);
            link.appendP(fb_esp_pgm_str_273);
            link += _ss.fcs.meta.downloadTokens.c_str();
        }
#endif
    }
    else if (_ss.con_mode == fb_esp_con_mode_gc_storage)
    {
#ifdef ENABLE_GC_STORAGE
        if (_ss.gcs.meta.downloadTokens.length() > 0)
        {
            link.appendP(fb_esp_pgm_str_112);
            link.appendP(fb_esp_pgm_str_265);
            link.appendP(fb_esp_pgm_str_120);
            link.appendP(fb_esp_pgm_str_266);
            link += _ss.gcs.meta.bucket;
            link.appendP(fb_esp_pgm_str_267);
            link.appendP(fb_esp_pgm_str_1);
            link += ut->url_encode(_ss.gcs.meta.name);
            link.appendP(fb_esp_pgm_str_173);
            link.appendP(fb_esp_pgm_str_269);
            link.appendP(fb_esp_pgm_str_172);
            link.appendP(fb_esp_pgm_str_273);
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

int FirebaseData::payloadLength()
{
    return _ss.payload_length;
}

int FirebaseData::maxPayloadLength()
{
    return _ss.max_payload_length;
}

#ifdef ENABLE_RTDB
void FirebaseData::sendStreamToCB(int code)
{
    _ss.error.clear();
    _ss.rtdb.data_millis = 0;
    _ss.rtdb.data_tmo = true;
    _ss.http_code = code;
    if (_timeoutCallback)
        _timeoutCallback(true);
}
#endif

void FirebaseData::closeSession()
{
    bool status = WiFi.status() == WL_CONNECTED;

    if (init())
    {
        if (_spi_ethernet_module)
            status |= ethLinkUp(_spi_ethernet_module);
        else
            status |= ethLinkUp(&(Signer.getCfg()->spi_ethernet_module));
    }
    else
        status |= ethLinkUp(_spi_ethernet_module);

    if (status)
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
        _ss.rtdb.new_stream = true;
    }
#endif
    _ss.connected = false;
}

int FirebaseData::tcpSend(const char *data)
{

    size_t len = strlen(data);

#if defined(ESP8266)
    if (_ss.bssl_tx_size < 512)
        _ss.bssl_tx_size = 512;
    int chunkSize = _ss.bssl_tx_size;
#else
    int chunkSize = 4096;
#endif

    int sent = 0;
    int ret = 0;

    if (!reconnect())
        return FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

    while (sent < (int)len)
    {
        if (sent + chunkSize > (int)len)
            chunkSize = len - sent;

        ret = tcpClient.send(data + sent, chunkSize);

        if (ret != 0)
            return ret;

        sent += chunkSize;
    }

    return ret;
}

bool FirebaseData::reconnect(unsigned long dataTime)
{

    bool status = WiFi.status() == WL_CONNECTED;

    status |= (init()) ? ((_spi_ethernet_module) ? ethLinkUp(_spi_ethernet_module) : ethLinkUp(&(Signer.getCfg()->spi_ethernet_module))) : ethLinkUp(_spi_ethernet_module);

    if (dataTime > 0)
    {
        unsigned long tmo = DEFAULT_SERVER_RESPONSE_TIMEOUT;
        if (init())
        {
            if (Signer.getCfg()->timeout.serverResponse < MIN_SERVER_RESPONSE_TIMEOUT || Signer.getCfg()->timeout.serverResponse > MIN_SERVER_RESPONSE_TIMEOUT)
                Signer.getCfg()->timeout.serverResponse = DEFAULT_SERVER_RESPONSE_TIMEOUT;
            tmo = Signer.getCfg()->timeout.serverResponse;
        }

        if (millis() - dataTime > tmo)
        {
            _ss.http_code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;

            size_t len = strlen_P(fb_esp_pgm_str_69) + 5;

            if (_ss.con_mode == fb_esp_con_mode_rtdb_stream)
                len += strlen_P(fb_esp_pgm_str_578);

            char *buf = new char[len];
            memset(buf, 0, len);
            strcpy_P(buf, fb_esp_pgm_str_69);

            if (_ss.con_mode == fb_esp_con_mode_rtdb_stream)
                strcat_P(buf, fb_esp_pgm_str_578);

            _ss.error.clear();
            _ss.error.reserve(len);
            _ss.error = buf;
            ut->delP(&buf);

            closeSession();
            return false;
        }
    }

    if (!status)
    {
        if (_ss.connected)
            closeSession();

        _ss.http_code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

        if (init())
        {
            if (Signer.getCfg()->_int.fb_reconnect_wifi)
            {
                if (Signer.getCfg()->timeout.wifiReconnect < MIN_WIFI_RECONNECT_TIMEOUT || Signer.getCfg()->timeout.wifiReconnect > MAX_WIFI_RECONNECT_TIMEOUT)
                    Signer.getCfg()->timeout.wifiReconnect = MIN_WIFI_RECONNECT_TIMEOUT;

                if (millis() - Signer.getCfg()->_int.fb_last_reconnect_millis > Signer.getCfg()->timeout.wifiReconnect && !_ss.connected)
                {
                    WiFi.reconnect();
                    Signer.getCfg()->_int.fb_last_reconnect_millis = millis();
                }
            }
        }
        else
        {
            if (WiFi.getAutoReconnect())
            {
                if (millis() - last_reconnect_millis > reconnect_tmo && !_ss.connected)
                {
                    WiFi.reconnect();
                    last_reconnect_millis = millis();
                }
            }
        }

        status = WiFi.status() == WL_CONNECTED;
        status |= (init()) ? ((_spi_ethernet_module) ? ethLinkUp(_spi_ethernet_module) : ethLinkUp(&(Signer.getCfg()->spi_ethernet_module))) : ethLinkUp(_spi_ethernet_module);
    }

#if defined(ENABLE_RTDB)
    if (!status && _ss.con_mode == fb_esp_con_mode_rtdb_stream)
        _ss.rtdb.new_stream = true;
#endif

    return status;
}

void FirebaseData::setTimeout()
{
    if (Signer.getCfg())
    {
        if (Signer.getCfg()->timeout.socketConnection < MIN_SOCKET_CONN_TIMEOUT || Signer.getCfg()->timeout.socketConnection > MAX_SOCKET_CONN_TIMEOUT)
            Signer.getCfg()->timeout.socketConnection = DEFAULT_SOCKET_CONN_TIMEOUT;

        tcpClient.timeout = Signer.getCfg()->timeout.socketConnection;
    }
}

void FirebaseData::setSecure()
{
    setTimeout();

    tcpClient.setMBFS(mbfs);

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

    if (tcpClient._certType == -1 || _ss.cert_updated)
    {
        if (!Signer.getCfg())
        {
            _ss.cert_updated = false;
            tcpClient.setCACert(NULL);
            return;
        }

        if (!Signer.getCfg()->_int.fb_clock_rdy && (Signer.getCAFile().length() > 0 || Signer.getCfg()->cert.data != NULL || _ss.cert_addr > 0) && init())
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
            if (_ss.cert_addr > 0)
                tcpClient.setCACert(reinterpret_cast<const char *>(_ss.cert_addr));
            else if (Signer.getCfg()->cert.data != NULL)
                tcpClient.setCACert(Signer.getCfg()->cert.data);
            else
                tcpClient.setCACert(NULL);
        }
        else
        {
            tcpClient.setCACertFile(Signer.getCAFile().c_str(), mbfs_type Signer.getCAFileStorage());
        }
        _ss.cert_updated = false;
    }
}

void FirebaseData::setCert(const char *ca)
{
    int addr = reinterpret_cast<int>(ca);
    if (addr != _ss.cert_addr)
    {
        _ss.cert_updated = true;
        _ss.cert_addr = addr;
    }
}

bool FirebaseData::validRequest(const MB_String &path)
{
    if (path.length() == 0 || (Signer.getCfg()->database_url.length() == 0 && Signer.getCfg()->host.length() == 0) || (strlen(Signer.getToken()) == 0 && !Signer.getCfg()->signer.test_mode))
    {
        _ss.http_code = FIREBASE_ERROR_MISSING_CREDENTIALS;
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
        if (_ss.rtdb.req_method == fb_esp_method::m_get && !_ss.rtdb.data_tmo && _ss.con_mode != fb_esp_con_mode_fcm && resp.dataType != fb_esp_data_type::d_file && _ss.rtdb.req_method != fb_esp_method::m_download && _ss.rtdb.req_data_type != fb_esp_data_type::d_file && _ss.rtdb.req_data_type != fb_esp_data_type::d_file_ota)
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

    _ss.rtdb.raw.clear();
    _ss.rtdb.push_name.clear();
    _ss.rtdb.redirect_url.clear();
    _ss.rtdb.event_type.clear();
    _ss.rtdb.req_etag.clear();
    _ss.rtdb.resp_etag.clear();
    _ss.rtdb.priority = 0;

    if (_ss.rtdb.blob && _ss.rtdb.isBlobPtr)
    {
        _ss.rtdb.isBlobPtr = false;
        delete _ss.rtdb.blob;
    }

#endif

#if defined(FIREBASE_ESP_CLIENT)
#ifdef ENABLE_GC_STORAGE
    _ss.gcs.meta.bucket.clear();
    _ss.gcs.meta.contentType.clear();
    _ss.gcs.meta.crc32.clear();
    _ss.gcs.meta.downloadTokens.clear();
    _ss.gcs.meta.etag.clear();
    _ss.gcs.meta.name.clear();
#endif
#ifdef ENABLE_FB_STORAGE
    _ss.fcs.meta.name.clear();
    _ss.fcs.meta.bucket.clear();
    _ss.fcs.meta.contentType.clear();
    _ss.fcs.meta.etag.clear();
    _ss.fcs.meta.crc32.clear();
    _ss.fcs.meta.downloadTokens.clear();
    _ss.fcs.meta.bucket.clear();
    _ss.fcs.meta.contentType.clear();
    _ss.fcs.meta.crc32.clear();
    _ss.fcs.meta.downloadTokens.clear();
    _ss.fcs.meta.etag.clear();
    _ss.fcs.meta.name.clear();
    _ss.fcs.files.items.clear();
#endif
#ifdef ENABLE_FB_FUNCTIONS
    _ss.cfn.payload.clear();
#endif
#ifdef ENABLE_FIRESTORE
    _ss.cfs.payload.clear();
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
    s.appendP(fb_esp_pgm_str_577);
    json->set(s.c_str(), addrTo<const char *>(serverKey.address()));
    raw.clear();
    s.clear();
    raw = json->raw();
    json->clear();
    delete json;
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
}

bool FCMObject::prepareUtil()
{
    if (!ut)
        ut = Signer.getUtils(); //must be initialized in Firebase class constructor

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
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_122);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_285);
    json->set(s.c_str(), _title.c_str());

    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_122);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_123);
    json->set(s.c_str(), _body.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body, MB_StringPtr icon)
{
    prepareUtil();

    MB_String _icon = icon;

    setNotifyMessage(title, body);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_122);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_124);
    json->set(s.c_str(), _icon.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mSetNotifyMessage(MB_StringPtr title, MB_StringPtr body, MB_StringPtr icon, MB_StringPtr click_action)
{
    prepareUtil();

    MB_String _click_action = click_action;

    setNotifyMessage(title, body, icon);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_122);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_125);
    json->set(s.c_str(), _click_action.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mAddCustomNotifyMessage(MB_StringPtr key, MB_StringPtr value)
{
    prepareUtil();

    MB_String _value = value;

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_122);
    s.appendP(fb_esp_pgm_str_1);
    s += key;
    json->set(s.c_str(), _value.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::clearNotifyMessage()
{
    prepareUtil();

    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_122);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->remove(s.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mSetDataMessage(MB_StringPtr jsonString)
{

    prepareUtil();

    MB_String _jsonString = jsonString;

    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_135);
    FirebaseJson *js = new FirebaseJson();
    js->setJsonData(_jsonString.c_str());
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), *js);
    js->clear();
    delete js;
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::setDataMessage(FirebaseJson &json)
{
    prepareUtil();

    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_135);
    FirebaseJson *js = new FirebaseJson();
    js->setJsonData(raw);
    js->set(s.c_str(), json);
    s.clear();
    raw.clear();
    raw = js->raw();
    js->clear();
    delete js;
}

void FCMObject::clearDataMessage()
{
    prepareUtil();

    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_135);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->remove(s.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mSetPriority(MB_StringPtr priority)
{
    prepareUtil();

    MB_String _priority = priority;
    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_136);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), _priority.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mSetCollapseKey(MB_StringPtr key)
{
    prepareUtil();

    MB_String _key = key;

    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_138);
    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), _key.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::setTimeToLive(uint32_t seconds)
{
    prepareUtil();

    if (seconds <= 2419200)
        _ttl = seconds;
    else
        _ttl = -1;
    MB_String s;
    s.appendP(fb_esp_pgm_str_575, true);
    s.appendP(fb_esp_pgm_str_1);
    s.appendP(fb_esp_pgm_str_137);

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    json->set(s.c_str(), _ttl);
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

void FCMObject::mSetTopic(MB_StringPtr topic)
{
    prepareUtil();

    FirebaseJson *json = new FirebaseJson();
    json->setJsonData(raw);
    MB_String s, v;
    s.appendP(fb_esp_pgm_str_576);
    v.appendP(fb_esp_pgm_str_134);
    v += topic;
    json->set(s.c_str(), v.c_str());
    raw.clear();
    s.clear();
    v.clear();
    raw = json->raw();
    json->clear();
    delete json;
}

const char *FCMObject::getSendResult()
{
    return result.c_str();
}

void FCMObject::fcm_begin(FirebaseData &fbdo)
{
    prepareUtil();

    fbdo._spi_ethernet_module = _spi_ethernet_module;

    MB_String host;
    host.appendP(fb_esp_pgm_str_249);
    host.appendP(fb_esp_pgm_str_4);
    host.appendP(fb_esp_pgm_str_120);
    rescon(fbdo, host.c_str());
    fbdo.tcpClient.begin(host.c_str(), _port);
}

int FCMObject::fcm_sendHeader(FirebaseData &fbdo, size_t payloadSize)
{
    int ret = -1;
    MB_String header;

    prepareUtil();

    FirebaseJsonData *server_key = new FirebaseJsonData();

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(raw);
    MB_String s;
    s.appendP(fb_esp_pgm_str_577);
    json->get(*server_key, s.c_str());
    s.clear();
    json->clear();

    header.appendP(fb_esp_pgm_str_24, true);
    header.appendP(fb_esp_pgm_str_6);
    header.appendP(fb_esp_pgm_str_121);
    header.appendP(fb_esp_pgm_str_30);

    header.appendP(fb_esp_pgm_str_31);
    header.appendP(fb_esp_pgm_str_249);
    header.appendP(fb_esp_pgm_str_4);
    header.appendP(fb_esp_pgm_str_120);
    header.appendP(fb_esp_pgm_str_21);

    header.appendP(fb_esp_pgm_str_131);

    ret = fbdo.tcpSend(header.c_str());

    header.clear();
    if (ret < 0)
    {
        server_key->clear();
        delete server_key;
        return ret;
    }

    ret = fbdo.tcpSend(server_key->to<const char *>());
    server_key->clear();
    delete server_key;
    if (ret < 0)
        return ret;

    header.appendP(fb_esp_pgm_str_21);

    header.appendP(fb_esp_pgm_str_32);

    header.appendP(fb_esp_pgm_str_8);
    header.appendP(fb_esp_pgm_str_129);
    header.appendP(fb_esp_pgm_str_21);

    header.appendP(fb_esp_pgm_str_12);
    header += payloadSize;
    header.appendP(fb_esp_pgm_str_21);
    header.appendP(fb_esp_pgm_str_36);
    header.appendP(fb_esp_pgm_str_21);

    ret = fbdo.tcpSend(header.c_str());
    header.clear();

    return ret;
}

void FCMObject::fcm_preparePayload(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    prepareUtil();

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(raw);
    if (messageType == fb_esp_fcm_msg_type::msg_single)
    {
        MB_String s;
        s.appendP(fb_esp_pgm_str_575, true);
        s.appendP(fb_esp_pgm_str_1);
        s.appendP(fb_esp_pgm_str_128);

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
        s.appendP(fb_esp_pgm_str_575, true);
        s.appendP(fb_esp_pgm_str_1);
        s.appendP(fb_esp_pgm_str_130);

        json->set(s.c_str(), *arr);
        s.clear();
        arr->clear();
        raw.clear();
        raw = json->raw();
    }
    else if (messageType == fb_esp_fcm_msg_type::msg_topic)
    {
        MB_String s;
        s.appendP(fb_esp_pgm_str_575, true);
        s.appendP(fb_esp_pgm_str_1);
        s.appendP(fb_esp_pgm_str_128);

        FirebaseJsonData *topic = fbdo.to<FirebaseJsonData *>();
        MB_String s2;
        s2.appendP(fb_esp_pgm_str_576);
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

    result.clear();

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->tcpClient.stream();

    char *pChunk = NULL;
    char *tmp = NULL;
    char *header = NULL;
    char *payload = NULL;
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
    fbdo->_ss.payload_length = 0;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 768;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        ut->idle();
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
                    header = (char *)ut->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = ut->readLine(stream, header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    ut->idle();
                    dataTime = millis();
                    if (tmp)
                    {
                        //http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        fbdo->_ss.http_code = response.httpCode;
                        ut->delP(&tmp);
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
                    //the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        //read one line of next header field until the empty header has found
                        tmp = (char *)ut->newP(chunkBufSize);
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
                                ut->delP(&header);
                            hstate = 0;

                            fbdo->_ss.chunked_encoding = response.isChunkedEnc;
                        }
                        else
                        {
                            //accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }

                        ut->delP(&tmp);
                    }
                    else
                    {
                        //the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;

                            pChunk = (char *)ut->newP(chunkBufSize + 1);

                            if (!payload || pstate == 0)
                            {
                                pstate = 1;
                                payload = (char *)ut->newP(payloadLen + 1);
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
                                fbdo->_ss.payload_length += readLen;
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
                            fbdo->_ss.error = data->to<const char *>();
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
        while (stream->available() > 0)
            stream->read();
    }

    return false;
}

bool FCMObject::fcm_send(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    prepareUtil();

    FirebaseJsonData *msg = fbdo.to<FirebaseJsonData *>();

    fcm_preparePayload(fbdo, messageType);

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(raw);
    MB_String s;
    s.appendP(fb_esp_pgm_str_575);
    json->get(*msg, s.c_str());
    raw = json->raw();
    json->clear();

    int ret = fcm_sendHeader(fbdo, strlen(msg->to<const char *>()));

    if (ret == 0)
        ret = fbdo.tcpSend(msg->to<const char *>());

    json->setJsonData(raw);
    json->remove(s.c_str());
    s.clear();
    raw.clear();
    raw = json->raw();
    json->clear();
    msg->clear();

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
    if (fbdo._ss.cert_updated || !fbdo._ss.connected || millis() - fbdo._ss.last_conn_ms > fbdo._ss.conn_timeout || fbdo._ss.con_mode != fb_esp_con_mode_fcm || strcmp(host, fbdo._ss.host.c_str()) != 0)
    {
        fbdo._ss.last_conn_ms = millis();
        fbdo.closeSession();
        fbdo.setSecure();
        fbdo.ethDNSWorkAround(_spi_ethernet_module, host, 443);
    }
    fbdo._ss.host = host;
    fbdo._ss.con_mode = fb_esp_con_mode_fcm;
}

void FCMObject::clear()
{
    raw.clear();
    result.clear();
    _ttl = -1;
    _index = 0;
    clearDeviceToken();

    if (intUt && ut)
        delete ut;
}
#endif
#endif

#endif