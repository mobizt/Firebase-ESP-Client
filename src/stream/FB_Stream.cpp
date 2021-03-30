/**
 * Google's Firebase Stream class, FB_Stream.cpp version 1.0.2
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created March 25, 2021
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

#ifndef FIREBASE_STREAM_SESSION_CPP
#define FIREBASE_STREAM_SESSION_CPP
#include "FB_Stream.h"

FirebaseStream::FirebaseStream()
{
}

FirebaseStream::~FirebaseStream()
{
    empty();
}

void FirebaseStream::begin(UtilsClass *u, struct fb_esp_stream_info_t *s)
{
    ut = u;
    sif = s;
}

String FirebaseStream::dataPath()
{
    return sif->path.c_str();
}

String FirebaseStream::streamPath()
{
    return sif->stream_path.c_str();
}

int FirebaseStream::intData()
{
    if (sif->data.length() > 0 && (sif->data_type == fb_esp_data_type::d_integer || sif->data_type == fb_esp_data_type::d_float || sif->data_type == fb_esp_data_type::d_double))
        return atoi(sif->data.c_str());
    else
        return 0;
}

float FirebaseStream::floatData()
{
    if (sif->data.length() > 0 && (sif->data_type == fb_esp_data_type::d_integer || sif->data_type == fb_esp_data_type::d_float || sif->data_type == fb_esp_data_type::d_double))
        return atof(sif->data.c_str());
    else
        return 0;
}

double FirebaseStream::doubleData()
{
    if (sif->data.length() > 0 && (sif->data_type == fb_esp_data_type::d_integer || sif->data_type == fb_esp_data_type::d_float || sif->data_type == fb_esp_data_type::d_double))
        return atof(sif->data.c_str());
    else
        return 0.0;
}

bool FirebaseStream::boolData()
{
    bool res = false;
    char *str = ut->boolStr(true);
    if (sif->data.length() > 0 && sif->data_type == fb_esp_data_type::d_boolean)
        res = strcmp(sif->data.c_str(), str) == 0;
    ut->delS(str);
    return res;
}

String FirebaseStream::stringData()
{
    if (sif->data_type == fb_esp_data_type::d_string)
        return sif->data.substr(1, sif->data.length() - 2).c_str();
    else
        return std::string().c_str();
}

String FirebaseStream::jsonString()
{
    if (sif->data_type == fb_esp_data_type::d_json)
    {
        String res = "";
        _json->toString(res);
        return res;
    }
    else
        return std::string().c_str();
}

FirebaseJson *FirebaseStream::jsonObjectPtr()
{
    return _json;
}

FirebaseJson &FirebaseStream::jsonObject()
{
    return *jsonObjectPtr();
}

FirebaseJsonArray *FirebaseStream::jsonArrayPtr()
{
    if (sif->data.length() > 0 && sif->data_type == fb_esp_data_type::d_array)
    {

        std::string().swap(*_jsonArr->int_dbuf());
        std::string().swap(*_jsonArr->int_tbuf());

        char *tmp = ut->strP(fb_json_str_21);
        _jsonArr->int_json()->int_toStdString(*_jsonArr->int_jbuf());
        *_jsonArr->int_rawbuf() = tmp;
        *_jsonArr->int_rawbuf() += sif->data;
        ut->delS(tmp);

        tmp = ut->strP(fb_json_str_26);
        _jsonArr->int_json()->int_parse(tmp, FirebaseJson::PRINT_MODE_PLAIN);
        ut->delS(tmp);

        std::string().swap(*_jsonArr->int_tbuf());
        std::string().swap(*_jsonArr->int_jbuf());
        _jsonArr->int_json()->int_clearPathTk();
        _jsonArr->int_json()->int_clearTokens();

        if (_jsonArr->int_dbuf()->length() > 2)
            *_jsonArr->int_rawbuf() = _jsonArr->int_dbuf()->substr(1, _jsonArr->int_dbuf()->length() - 2);
        _jsonArr->int_set_arr_len(_jsonArr->int_json()->int_get_jsondata_len());
    }
    return _jsonArr;
}

FirebaseJsonArray &FirebaseStream::jsonArray()
{
    return *jsonArrayPtr();
}

FirebaseJsonData *FirebaseStream::jsonDataPtr()
{
    return _jsonData;
}

FirebaseJsonData &FirebaseStream::jsonData()
{
    return *_jsonData;
}

std::vector<uint8_t> FirebaseStream::blobData()
{
    if (sif->blob.size() > 0 && sif->data_type == fb_esp_data_type::d_blob)
        return sif->blob;
    else
        return std::vector<uint8_t>();
}

File FirebaseStream::fileStream()
{
    if (sif->data_type == fb_esp_data_type::d_file)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_184);
        if (ut->flashTest())
            Signer.getCfg()->_int.fb_file = FLASH_FS.open(tmp, "r");
        ut->delS(tmp);
    }

    return Signer.getCfg()->_int.fb_file;
}

String FirebaseStream::dataType()
{
    return sif->data_type_str.c_str();
}

String FirebaseStream::eventType()
{
    return sif->event_type_str.c_str();
}

void FirebaseStream::empty()
{
    std::string().swap(sif->stream_path);
    std::string().swap(sif->path);
    std::string().swap(sif->data);
    std::string().swap(sif->data_type_str);
    std::string().swap(sif->event_type_str);
    std::vector<uint8_t>().swap(sif->blob);
    if (_json)
        _json->clear();
    if (_jsonArr)
        _jsonArr->clear();
    if (_jsonData)
        _jsonData->stringValue.clear();
}

#endif