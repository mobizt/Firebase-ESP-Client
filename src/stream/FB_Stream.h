/**
 * Google's Firebase Stream class, FB_Stream.h version 1.0.2
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

#ifndef FIREBASE_STREAM_SESSION_H
#define FIREBASE_STREAM_SESSION_H
#include <Arduino.h>
#include "Utils.h"
#include "signer/Signer.h"

class FirebaseStream
{

    friend class FirebaseData;
    friend class FB_RTDB;

public:
    FirebaseStream();
    ~FirebaseStream();
    String dataPath();
    String streamPath();
    int intData();
    float floatData();
    double doubleData();
    bool boolData();
    String stringData();
    String jsonString();
    FirebaseJson *jsonObjectPtr();
    FirebaseJson &jsonObject();
    FirebaseJsonArray *jsonArrayPtr();
    FirebaseJsonArray &jsonArray();
    FirebaseJsonData *jsonDataPtr();
    FirebaseJsonData &jsonData();
    std::vector<uint8_t> blobData();
    File fileStream();
    String dataType();
    String eventType();
    void empty();

    FirebaseJson *_json = nullptr;
    FirebaseJsonArray *_jsonArr = nullptr;
    FirebaseJsonData *_jsonData = nullptr;

private:
    UtilsClass *ut = nullptr;
    struct fb_esp_stream_info_t *sif = nullptr;

    void begin(UtilsClass *u, struct fb_esp_stream_info_t *s);
};

#endif