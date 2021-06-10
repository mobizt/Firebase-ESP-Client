/**
 * Google's Firebase MultiPathStream class, FB_MP_Stream.cpp version 1.0.4
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

#ifndef FIREBASE_MULTIPATH_STREAM_SESSION_CPP
#define FIREBASE_MULTIPATH_STREAM_SESSION_CPP
#include "FB_MP_Stream.h"

FIREBASE_MP_STREAM_CLASS::FIREBASE_MP_STREAM_CLASS()
{
}

FIREBASE_MP_STREAM_CLASS::~FIREBASE_MP_STREAM_CLASS()
{
}

void FIREBASE_MP_STREAM_CLASS::begin(UtilsClass *u, struct fb_esp_stream_info_t *s)
{
    ut = u;
    sif = s;
}

bool FIREBASE_MP_STREAM_CLASS::get(const String &path)
{
    value.clear();
    type.clear();
    dataPath.clear();
    bool res = false;
    if (sif->m_type == fb_esp_data_type::d_json)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_1);
        bool r = strcmp(sif->m_path, tmp) == 0;
        ut->delS(tmp);
        if (r)
        {
            FirebaseJsonData data;
            sif->m_json->get(data, path);
            if (data.success)
            {
                type = data.type;
                char *buf = ut->strP(fb_esp_pgm_str_186);
                if (strcmp(type.c_str(), buf) == 0)
                    type = sif->m_type_str;
                eventType = sif->m_event_type_str;
                value = data.stringValue;
                dataPath = path;
                ut->delS(buf);
                res = true;
            }
        }
        else
        {
            std::string p1 = sif->m_path;
            if (path.length() < p1.length())
                p1 = p1.substr(0, path.length());
            std::string p2 = path.c_str();
            if (p2[0] != '/')
                p2 = "/" + p2;
            if (strcmp(p1.c_str(), p2.c_str()) == 0)
            {
                sif->m_json->toString(value, true);
                type = sif->m_type_str;
                eventType = sif->m_event_type_str;
                dataPath = sif->m_path;
                res = true;
            }
            std::string().swap(p1);
            std::string().swap(p2);
        }
    }
    else
    {
        std::string p1 = sif->m_path;
        if (path.length() < p1.length())
            p1 = p1.substr(0, path.length());
        std::string p2 = path.c_str();
        if (p2[0] != '/')
            p2 = "/" + p2;
        if (strcmp(p1.c_str(), p2.c_str()) == 0)
        {
            value = sif->m_data;
            dataPath = sif->m_path;
            type = sif->m_type_str;
            eventType = sif->m_event_type_str;
            res = true;
        }
        std::string().swap(p1);
        std::string().swap(p2);
    }
    return res;
}

void FIREBASE_MP_STREAM_CLASS::empty()
{
    dataPath.clear();
    value.clear();
    type.clear();
    sif->m_json = nullptr;
}

#endif