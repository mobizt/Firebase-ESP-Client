#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase QueueInfo class, QueueInfo.h version 1.0.6
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

#ifndef FIREBASE_QUEUE_INFO_H
#define FIREBASE_QUEUE_INFO_H
#include <Arduino.h>
#include "FB_Utils.h"
#include "QueryFilter.h"

struct QueueItem
{
    fb_esp_data_type dataType = fb_esp_data_type::d_any;
    int subType = 0;
    fb_esp_request_method method = http_get;
    uint32_t qID = 0;
    uint32_t timestamp = 0;
    uint8_t runCount = 0;
    uint8_t runIndex = 0;
#if defined(FIREBASE_ESP_CLIENT)
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
    uint8_t storageType = StorageType::UNDEFINED;
#endif
    MB_String path;
    MB_String payload;
    MB_String filename;
    MB_String etag;
    struct fb_esp_rtdb_address_t address;
    int blobSize = 0;
    bool async = false;
};

class QueueInfo
{
    friend class FB_RTDB;

public:
    QueueInfo();
    ~QueueInfo();
    uint8_t totalQueues();
    uint32_t currentQueueID();
    bool isQueueFull();
    String dataType();
    String firebaseMethod();
    String dataPath();

private:
    void clear();
    uint8_t _totalQueue = 0;
    uint32_t _currentQueueID = 0;
    bool _isQueueFull = false;
    bool _isQueue = false;
    MB_String _dataType;
    MB_String _method;
    MB_String _path;
};

#endif

#endif // ENABLE