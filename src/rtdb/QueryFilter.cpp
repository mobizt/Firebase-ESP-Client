/**
 * Google's Firebase QueryFilter class, QueryFilter.cpp version 1.0.4
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 1, 2022
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

#include "FirebaseFS.h"

#ifdef ENABLE_RTDB

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
    MBSTRING().swap(_orderBy);
    MBSTRING().swap(_limitToFirst);
    MBSTRING().swap(_limitToLast);
    MBSTRING().swap(_startAt);
    MBSTRING().swap(_endAt);
    MBSTRING().swap(_equalTo);
    return *this;
}

QueryFilter &QueryFilter::mOrderBy(const char *val)
{
    _orderBy = (const char *)FPSTR("\"");
    _orderBy += val;
    _orderBy += (const char *)FPSTR("\"");
    return *this;
}
QueryFilter &QueryFilter::mLimitToFirst(const char *val)
{
    _limitToFirst = val;
    return *this;
}

QueryFilter &QueryFilter::mLimitToLast(const char *val)
{
    _limitToLast = val;
    return *this;
}

QueryFilter &QueryFilter::mStartAt(const char *val, bool isString)
{
    if (isString)
        _startAt = (const char *)FPSTR("\"");
    _startAt += val;
    if (isString)
        _startAt = (const char *)FPSTR("\"");
    return *this;
}

QueryFilter &QueryFilter::mEndAt(const char *val, bool isString)
{
    if (isString)
        _endAt = (const char *)FPSTR("\"");
    _endAt += val;
    if (isString)
        _endAt = (const char *)FPSTR("\"");
    return *this;
}

QueryFilter &QueryFilter::mEqualTo(const char *val, bool isString)
{
    if (isString)
        _equalTo = (const char *)FPSTR("\"");
    _equalTo += val;
    if (isString)
        _equalTo += (const char *)FPSTR("\"");
    return *this;
}

#endif

#endif //ENABLE