#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase MultiPathStream class, FB_MP_Stream.h version 1.1.6
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

#ifndef FIREBASE_MULTIPATH_STREAM_SESSION_H
#define FIREBASE_MULTIPATH_STREAM_SESSION_H
#include <Arduino.h>
#include "FB_Utils.h"
#include "signer/Signer.h"
#include "FB_Stream.h"

#if defined(FIREBASE_ESP_CLIENT)
#define FIREBASE_MP_STREAM_CLASS MultiPathStream
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#define FIREBASE_MP_STREAM_CLASS MultiPathStreamData
#endif

using namespace mb_string;

class FIREBASE_MP_STREAM_CLASS
{
    friend class FB_RTDB;

public:
    FIREBASE_MP_STREAM_CLASS();
    ~FIREBASE_MP_STREAM_CLASS();
    bool get(const String &path);
    int payloadLength();
    int maxPayloadLength();
    String dataPath;
    String value;
    String type;
    String eventType;

private:
    struct fb_esp_stream_info_t *sif = nullptr;
    void begin(struct fb_esp_stream_info_t *s);
    void empty();
    bool checkPath(MB_String &root, MB_String &branch);
};

#endif

#endif // ENABLE