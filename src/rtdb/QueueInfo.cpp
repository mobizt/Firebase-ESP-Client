#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase QueueInfo class, QueueInfo.cpp version 1.0.6
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created December 25, 2022
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

#ifndef FIREBASE_QUEUE_INFO_CPP
#define FIREBASE_QUEUE_INFO_CPP

#include "QueueInfo.h"

QueueInfo::QueueInfo()
{
}

QueueInfo::~QueueInfo()
{
    clear();
}

uint8_t QueueInfo::totalQueues()
{
    return _totalQueue;
}

uint32_t QueueInfo::currentQueueID()
{
    return _currentQueueID;
}

bool QueueInfo::isQueueFull()
{
    return _isQueueFull;
}

String QueueInfo::dataType()
{
    return _dataType.c_str();
}

String QueueInfo::firebaseMethod()
{
    return _method.c_str();
}

String QueueInfo::dataPath()
{
    return _path.c_str();
}

void QueueInfo::clear()
{
    _dataType.clear();
    _method.clear();
    _path.clear();
}

#endif

#endif //ENABLE