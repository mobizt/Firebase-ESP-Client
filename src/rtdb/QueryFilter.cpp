/**
 * Google's Firebase QueryFilter class, QueryFilter.cpp version 1.0.0
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

#ifndef FIREBASE_QUERY_FILTER_CPP
#define FIREBASE_QUERY_FILTER_CPP
#include "QueryFilter.h"

QueryFilter::QueryFilter()
{
}

QueryFilter::~QueryFilter()
{
    clear();
}
void QueryFilter::begin(UtilsClass *u)
{
    ut = u;
}

QueryFilter &QueryFilter::clear()
{
    std::string().swap(_orderBy);
    std::string().swap(_limitToFirst);
    std::string().swap(_limitToLast);
    std::string().swap(_startAt);
    std::string().swap(_endAt);
    std::string().swap(_equalTo);
    return *this;
}

QueryFilter &QueryFilter::orderBy(const String &val)
{
    ut->appendP(_orderBy, fb_esp_pgm_str_3, true);
    _orderBy += val.c_str();
    ut->appendP(_orderBy, fb_esp_pgm_str_3);
    return *this;
}
QueryFilter &QueryFilter::limitToFirst(int val)
{

    char *num = ut->intStr(val);
    _limitToFirst = num;
    ut->delS(num);
    return *this;
}

QueryFilter &QueryFilter::limitToLast(int val)
{
    char *num = ut->intStr(val);
    _limitToLast = num;
    ut->delS(num);
    return *this;
}

QueryFilter &QueryFilter::startAt(float val)
{
    char *num = ut->floatStr(val);
    _startAt = num;
    ut->delS(num);
    return *this;
}

QueryFilter &QueryFilter::endAt(float val)
{
    char *num = ut->floatStr(val);
    _endAt = num;
    ut->delS(num);
    return *this;
}

QueryFilter &QueryFilter::startAt(const String &val)
{
    ut->appendP(_startAt, fb_esp_pgm_str_3, true);
    _startAt += val.c_str();
    ut->appendP(_startAt, fb_esp_pgm_str_3);
    return *this;
}

QueryFilter &QueryFilter::endAt(const String &val)
{
    ut->appendP(_endAt, fb_esp_pgm_str_3, true);
    _startAt += val.c_str();
    ut->appendP(_endAt, fb_esp_pgm_str_3);
    return *this;
}

QueryFilter &QueryFilter::equalTo(int val)
{
    char *num = ut->intStr(val);
    _equalTo = num;
    ut->delS(num);
    return *this;
}

QueryFilter &QueryFilter::equalTo(const String &val)
{
    ut->appendP(_equalTo, fb_esp_pgm_str_3, true);
    _equalTo += val.c_str();
    ut->appendP(_equalTo, fb_esp_pgm_str_3);
    return *this;
}

#endif