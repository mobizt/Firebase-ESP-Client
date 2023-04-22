#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase MultiPathStream class, FB_MP_Stream.cpp version 1.1.6
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

#ifndef FIREBASE_MULTIPATH_STREAM_SESSION_CPP
#define FIREBASE_MULTIPATH_STREAM_SESSION_CPP

#include "FB_MP_Stream.h"

FIREBASE_MP_STREAM_CLASS::FIREBASE_MP_STREAM_CLASS()
{
}

FIREBASE_MP_STREAM_CLASS::~FIREBASE_MP_STREAM_CLASS()
{
}

void FIREBASE_MP_STREAM_CLASS::begin(struct fb_esp_stream_info_t *s)
{
    sif = s;
}

bool FIREBASE_MP_STREAM_CLASS::get(const String &path /* child path */)
{
    value.remove(1, value.length());
    type.remove(1, type.length());
    dataPath.remove(1, dataPath.length());
    bool res = false;
    if (sif->data_type == fb_esp_data_type::d_json)
    {
        bool r = strcmp(sif->path.c_str(), pgm2Str(fb_esp_pgm_str_1/* "/" */)) == 0;
        if (r)
        {
            FirebaseJsonData data;
            sif->m_json->get(data, path);
            if (data.success)
            {
                type = data.type;
                if (strcmp(type.c_str(), pgm2Str(fb_esp_rtdb_pgm_str_40/* "object" */)) == 0)
                    type = sif->data_type_str.c_str();
                eventType = sif->event_type_str.c_str();
                value = data.to<const char *>();
                dataPath = path;
                res = true;
            }
        }
        else
        {
            MB_String root = path.c_str();
            MB_String branch = sif->path;
            // check for the steam data path is matched or under the root (child path)
            if (checkPath(root, branch))
            {
                sif->m_json->toString(value, true);
                type = sif->data_type_str.c_str();
                eventType = sif->event_type_str.c_str();
                dataPath = sif->path.c_str();
                res = true;
            }
        }
    }
    else
    {
        MB_String root = path.c_str();
        MB_String branch = sif->path;
        // check for the steam data path is matched or under the root (child path)
        if (checkPath(root, branch))
        {
            value = sif->data.c_str();
            dataPath = sif->path.c_str();
            type = sif->data_type_str.c_str();
            eventType = sif->event_type_str.c_str();
            res = true;
        }
    }
    return res;
}

bool FIREBASE_MP_STREAM_CLASS::checkPath(MB_String &root, MB_String &branch)
{
    if (root[0] != '/')
        root.insert(0, 1, '/');

    if (branch[0] != '/')
        branch.insert(0, 1, '/');

    if (root.length() != branch.length())
    {
        size_t p = branch.find("/", 1);
        if (p != MB_String::npos)
            branch = branch.substr(0, p);
    }

    return strcmp(branch.c_str(), root.c_str()) == 0;
}

void FIREBASE_MP_STREAM_CLASS::empty()
{
    value.remove(1, value.length());
    type.remove(1, type.length());
    dataPath.remove(1, dataPath.length());
    sif->m_json = nullptr;
}

int FIREBASE_MP_STREAM_CLASS::payloadLength()
{
    return sif->payload_length;
}

int FIREBASE_MP_STREAM_CLASS::maxPayloadLength()
{
    return sif->max_payload_length;
}

#endif

#endif // ENABLE