/**
 * Google's Firebase QueryFilter class, QueryFilter.h version 1.0.0
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

#ifndef FIREBASE_QUERY_FILTER_H
#define FIREBASE_QUERY_FILTER_H
#include <Arduino.h>
#include "Utils.h"
#include "signer/Signer.h"

class QueryFilter
{
    friend class FirebaseData;
    friend class FB_RTDB;
    friend class FirebaseSession;

public:
    QueryFilter();
    ~QueryFilter();
    QueryFilter &orderBy(const String &);
    QueryFilter &limitToFirst(int);
    QueryFilter &limitToLast(int);
    QueryFilter &startAt(float);
    QueryFilter &endAt(float);
    QueryFilter &startAt(const String &);
    QueryFilter &endAt(const String &);
    QueryFilter &equalTo(int);
    QueryFilter &equalTo(const String &);
    QueryFilter &clear();

private:
    std::string _orderBy = "";
    std::string _limitToFirst = "";
    std::string _limitToLast = "";
    std::string _startAt = "";
    std::string _endAt = "";
    std::string _equalTo = "";
    UtilsClass *ut = nullptr;

    void begin(UtilsClass *u);
};

#endif