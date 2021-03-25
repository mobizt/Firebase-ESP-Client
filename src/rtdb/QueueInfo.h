/**
 * Google's Firebase QueueInfo class, QueueInfo.h version 1.0.0
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 12, 2021
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

#ifndef FIREBASE_QUEUE_INFO_H
#define FIREBASE_QUEUE_INFO_H
#include <Arduino.h>
#include "Utils.h"
#include "QueryFilter.h"

typedef struct fb_esp_rtdb_queue_item_info_t
{
    fb_esp_data_type dataType = fb_esp_data_type::d_any;
    fb_esp_method method = fb_esp_method::m_put;
    uint32_t qID = 0;
    uint32_t timestamp = 0;
    uint8_t runCount = 0;
    uint8_t runIndex = 0;
    fb_esp_mem_storage_type storageType = mem_storage_type_undefined;
    std::string path = "";
    std::string payload = "";
    std::vector<uint8_t> blob = std::vector<uint8_t>();
    std::string filename = "";
    QueryFilter queryFilter;
    int *intPtr = nullptr;
    float *floatPtr = nullptr;
    double *doublePtr = nullptr;
    bool *boolPtr = nullptr;
    String *stringPtr = nullptr;
    FirebaseJson *jsonPtr = nullptr;
    FirebaseJsonArray *arrPtr = nullptr;
    std::vector<uint8_t> *blobPtr = nullptr;
} QueueItem;

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
    std::string _dataType = "";
    std::string _method = "";
    std::string _path = "";
};

#endif