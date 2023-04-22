#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase QueueManager class, QueueManager.cpp version 1.0.5
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 1, 2022
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

#ifndef FIREBASE_QUEUE_MANAGER_CPP
#define FIREBASE_QUEUE_MANAGER_CPP


#include "QueueManager.h"

QueueManager::QueueManager()
{
}
QueueManager::~QueueManager()
{
    clear();
    if (_queueCollection)
        delete _queueCollection;
    _queueCollection = nullptr;
}

void QueueManager::clear()
{
    if (_queueCollection)
    {
        for (uint8_t i = 0; i < _queueCollection->size(); i++)
        {
#if defined(MB_USE_STD_VECTOR)
            QueueItem item = _queueCollection->at(i);
#else
            QueueItem item = (*_queueCollection)[i];
#endif

            item.path.clear();
            item.filename.clear();
            item.payload.clear();
            item.address.dout = 0;
            item.address.din = 0;
            item.blobSize = 0;
            item.address.priority=0;
            item.address.query = 0;
        }
    }
}

bool QueueManager::add(struct QueueItem q)
{
    if (!_queueCollection)
        _queueCollection = new MB_VECTOR<QueueItem>();

    if (_queueCollection->size() < _maxQueue)
    {
        _queueCollection->push_back(q);
        return true;
    }
    return false;
}

void QueueManager::remove(uint8_t index)
{
    if (_queueCollection)
        _queueCollection->erase(_queueCollection->begin() + index);
}

size_t QueueManager::size()
{
    if (_queueCollection)
        return _queueCollection->size();
    return 0;
}

#endif

#endif //ENABLE