/**
 * Google's Firebase QueryFilter class, QueryFilter.cpp version 1.0.1
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created April 30, 2021
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
    appendP(_orderBy, fb_esp_pgm_str_3, true);
    _orderBy += val.c_str();
    appendP(_orderBy, fb_esp_pgm_str_3);
    return *this;
}
QueryFilter &QueryFilter::limitToFirst(int val)
{

    char *num = intStr(val);
    _limitToFirst = num;
    delS(num);
    return *this;
}

QueryFilter &QueryFilter::limitToLast(int val)
{
    char *num = intStr(val);
    _limitToLast = num;
    delS(num);
    return *this;
}

QueryFilter &QueryFilter::startAt(float val)
{
    char *num = floatStr(val);
    _startAt = num;
    delS(num);
    return *this;
}

QueryFilter &QueryFilter::endAt(float val)
{
    char *num = floatStr(val);
    _endAt = num;
    delS(num);
    return *this;
}

QueryFilter &QueryFilter::startAt(const String &val)
{
    appendP(_startAt, fb_esp_pgm_str_3, true);
    _startAt += val.c_str();
    appendP(_startAt, fb_esp_pgm_str_3);
    return *this;
}

QueryFilter &QueryFilter::endAt(const String &val)
{
    appendP(_endAt, fb_esp_pgm_str_3, true);
    _startAt += val.c_str();
    appendP(_endAt, fb_esp_pgm_str_3);
    return *this;
}

QueryFilter &QueryFilter::equalTo(int val)
{
    char *num = intStr(val);
    _equalTo = num;
    delS(num);
    return *this;
}

QueryFilter &QueryFilter::equalTo(const String &val)
{
    appendP(_equalTo, fb_esp_pgm_str_3, true);
    _equalTo += val.c_str();
    appendP(_equalTo, fb_esp_pgm_str_3);
    return *this;
}

char *QueryFilter::strP(PGM_P pgm)
{
    size_t len = strlen_P(pgm) + 5;
    char *buf = newS(len);
    strcpy_P(buf, pgm);
    buf[strlen_P(pgm)] = 0;
    return buf;
}

char *QueryFilter::newS(size_t len)
{
    char *p = new char[len];
    memset(p, 0, len);
    return p;
}

void QueryFilter::appendP(std::string &buf, PGM_P p, bool empty)
{
    if (empty)
        buf.clear();
    char *b = strP(p);
    buf += b;
    delS(b);
}

void QueryFilter::delS(char *p)
{
    if (p != nullptr)
        delete[] p;
}

char *QueryFilter::floatStr(float value)
{
    char *buf = newS(36);
    dtostrf(value, 7, 9, buf);
    return buf;
}

char *QueryFilter::intStr(int value)
{
    char *buf = newS(36);
    itoa(value, buf, 10);
    return buf;
}

#endif