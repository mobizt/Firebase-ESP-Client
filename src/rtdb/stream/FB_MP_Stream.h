
/**
 * Google's Firebase MultiPathStream class, FB_MP_Stream.h version 1.1.7
 *
 * Created September 9, 2023
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

#include "./FirebaseFS.h"

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)

#ifndef FIREBASE_MULTIPATH_STREAM_SESSION_H
#define FIREBASE_MULTIPATH_STREAM_SESSION_H
#include <Arduino.h>
#include "./FB_Utils.h"
#include "./core/FirebaseCore.h"
#include "FB_Stream.h"

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
    struct firebase_stream_info_t *sif = nullptr;
    void begin(struct firebase_stream_info_t *s);
    void empty();
    bool checkPath(MB_String &root, MB_String &branch);
};

#endif

#endif // ENABLE