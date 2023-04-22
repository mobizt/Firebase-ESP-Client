#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Stream class, FB_Stream.cpp version 1.1.7
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created December 19, 2022
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

#include "FirebaseFS.h"

#ifdef ENABLE_RTDB

#ifndef FIREBASE_STREAM_SESSION_CPP
#define FIREBASE_STREAM_SESSION_CPP

#include "FB_Stream.h"

FIREBASE_STREAM_CLASS::FIREBASE_STREAM_CLASS()
{
}

FIREBASE_STREAM_CLASS::~FIREBASE_STREAM_CLASS()
{
    empty();
}

void FIREBASE_STREAM_CLASS::begin(struct fb_esp_stream_info_t *s)
{
    sif = s;
}

String FIREBASE_STREAM_CLASS::dataPath()
{
    return sif->path.c_str();
}

String FIREBASE_STREAM_CLASS::streamPath()
{
    return sif->stream_path.c_str();
}

int FIREBASE_STREAM_CLASS::intData()
{
    return to<int>();
}

float FIREBASE_STREAM_CLASS::floatData()
{
    return to<float>();
}

double FIREBASE_STREAM_CLASS::doubleData()
{
    return to<double>();
}

bool FIREBASE_STREAM_CLASS::boolData()
{
    return to<bool>();
}

String FIREBASE_STREAM_CLASS::stringData()
{
    return to<String>();
}

String FIREBASE_STREAM_CLASS::jsonString()
{
    if (sif->data_type == fb_esp_data_type::d_json)
        return sif->data.c_str();
    else
        return MB_String().c_str();
}

FirebaseJson *FIREBASE_STREAM_CLASS::jsonObjectPtr()
{
    return to<FirebaseJson *>();
}

FirebaseJson &FIREBASE_STREAM_CLASS::jsonObject()
{
    return to<FirebaseJson>();
}

FirebaseJsonArray *FIREBASE_STREAM_CLASS::jsonArrayPtr()
{
    return to<FirebaseJsonArray *>();
}

FirebaseJsonArray &FIREBASE_STREAM_CLASS::jsonArray()
{
    return to<FirebaseJsonArray>();
}

MB_VECTOR<uint8_t> *FIREBASE_STREAM_CLASS::blobData()
{
    return to<MB_VECTOR<uint8_t> *>();
}
#if defined(ESP32) || defined(ESP8266)
#if defined(MBFS_FLASH_FS)
File FIREBASE_STREAM_CLASS::fileStream()
{
    return to<File>();
}
#endif
#endif
String FIREBASE_STREAM_CLASS::payload()
{
    if (sif->data_type == fb_esp_data_type::d_string)
        setRaw(false); // if double quotes trimmed string, retain it.
    return sif->data.c_str();
}

String FIREBASE_STREAM_CLASS::dataType()
{
    return sif->data_type_str.c_str();
}

uint8_t FIREBASE_STREAM_CLASS::dataTypeEnum()
{
    return sif->data_type;
}

String FIREBASE_STREAM_CLASS::eventType()
{
    return sif->event_type_str.c_str();
}

void FIREBASE_STREAM_CLASS::empty()
{
    if (jsonPtr)
        jsonPtr->clear();

    if (arrPtr)
        arrPtr->clear();
}

int FIREBASE_STREAM_CLASS::payloadLength()
{
    return sif->payload_length;
}

int FIREBASE_STREAM_CLASS::maxPayloadLength()
{
    return sif->max_payload_length;
}

void FIREBASE_STREAM_CLASS::mSetResInt(const char *value)
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

void FIREBASE_STREAM_CLASS::mSetResFloat(const char *value)
{
    if (strlen(value) > 0)
    {
        char *pEnd;
        fVal.setd(strtod(value, &pEnd));
    }
    else
        fVal.setd(0);
}

void FIREBASE_STREAM_CLASS::mSetResBool(bool value)
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

// Double quotes string trim.
void FIREBASE_STREAM_CLASS::setRaw(bool trim)
{

    if (sif->data.length() > 0)
    {
        if (trim)
        {
            if (sif->data[0] == '"' && sif->data[sif->data.length() - 1] == '"')
            {
                sif->data.pop_back();
                sif->data.erase(0, 1);
            }
        }
        else
        {
            if (sif->data[0] != '"' && sif->data[sif->data.length() - 1] != '"')
            {
                sif->data.insert(0, '"');
                sif->data += '"';
            }
        }
    }
}

#endif

#endif // ENABLE