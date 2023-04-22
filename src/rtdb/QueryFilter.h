#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase QueryFilter class, QueryFilter.h version 1.0.7
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created December 19, 2022
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

#ifndef FIREBASE_QUERY_FILTER_H
#define FIREBASE_QUERY_FILTER_H
#include <Arduino.h>
#include "FB_Utils.h"
#include "signer/Signer.h"

using namespace mb_string;

class QueryFilter
{
    friend class FirebaseData;
    friend class FB_RTDB;
    friend class FirebaseSession;

public:
    QueryFilter();
    ~QueryFilter();

    template <typename T = const char *>
    QueryFilter &orderBy(T val) { return mOrderBy(toStringPtr(val)); }

    template <typename T = int>
    QueryFilter &limitToFirst(T val) { return mLimitToFirst(toStringPtr(val, -1)); }

    template <typename T = int>
    QueryFilter &limitToLast(T val) { return mLimitToLast(toStringPtr(val, -1)); }

    template <typename T = int>
    auto startAt(T val) -> typename enable_if<is_same<T, float>::value || is_same<T, double>::value ||
                                                  is_num_int<T>::value,
                                              QueryFilter &>::type { return mStartAt(toStringPtr(val, -1), false); }

    template <typename T = int>
    auto endAt(T val) -> typename enable_if<is_same<T, float>::value || is_same<T, double>::value ||
                                                is_num_int<T>::value,
                                            QueryFilter &>::type { return mEndAt(toStringPtr(val, -1), false); }

    template <typename T = const char *>
    auto startAt(T val) -> typename enable_if<is_string<T>::value, QueryFilter &>::type
    {
        return mStartAt(toStringPtr(val), true);
    }

    template <typename T = const char *>
    auto endAt(T val) -> typename enable_if<is_string<T>::value, QueryFilter &>::type
    {
        return mEndAt(toStringPtr(val), true);
    }

    template <typename T = int>
    auto equalTo(T val) -> typename enable_if<is_num_int<T>::value, QueryFilter &>::type
    {
        return mEqualTo(toStringPtr(val), false);
    }

    template <typename T = const char *>
    auto equalTo(T val) -> typename enable_if<is_string<T>::value, QueryFilter &>::type
    {
        return mEqualTo(toStringPtr(val), true);
    }

    QueryFilter &clear();

private:
    MB_String _orderBy;
    MB_String _limitToFirst;
    MB_String _limitToLast;
    MB_String _startAt;
    MB_String _endAt;
    MB_String _equalTo;

    QueryFilter &mOrderBy(MB_StringPtr val);
    QueryFilter &mLimitToFirst(MB_StringPtr val);
    QueryFilter &mLimitToLast(MB_StringPtr val);
    QueryFilter &mStartAt(MB_StringPtr val, bool isString);
    QueryFilter &mEndAt(MB_StringPtr val, bool isString);
    QueryFilter &mEqualTo(MB_StringPtr val, bool isString);
};

#endif

#endif // ENABLE