/*
 * FirebaseJson, version 2.6.1
 * 
 * The Easiest Arduino library to parse, create and edit JSON object using a relative path.
 * 
 * November 29, 2021
 * 
 * Features
 * - Using path to access node element in search style e.g. json.get(result,"a/b/c") 
 * - Able to search with key/path and value in JSON object and array
 * - Serializing to writable objects e.g. String, C/C++ string, Client (WiFi and Ethernet), File and Hardware Serial.
 * - Deserializing from const char, char array, string literal and stream e.g. Client (WiFi and Ethernet), File and 
 *   Hardware Serial.
 * - Use managed class, FirebaseJsonData to keep the deserialized result, which can be casted to any primitive data types.
 * 
 * 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
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

#ifndef FirebaseJson_H
#define FirebaseJson_H

#if defined __has_include
#if __has_include(<wirish.h>)
#include <wirish.h>
#undef min
#undef max
#endif
#endif

#include <Arduino.h>
#include <memory>
#include <vector>
#include <string>
#include <stdio.h>
#include <strings.h>
#include <functional>

#if __has_include(<FirebaseFS.h>)
#include <FirebaseFS.h>
#endif

#if defined(FIREBASEJSON_USE_PSRAM) || defined(FIREBASE_USE_PSRAM)
#define MB_STRING_USE_PSRAM
#endif
#include "MB_String.h"

#ifndef USE_MB_STRING
#define USE_MB_STRING
#endif

#ifndef MBSTRING
#define MBSTRING MB_String
#endif

#ifdef Serial_Printf
#undef Serial_Printf
#endif

#if defined(ESP8266) || defined(ESP32)
#include <FS.h>
#define FLASH_MCR FPSTR
#define FILE_SYSTEM fs::File
#define FBJS_ENABLE_FS
#define Serial_Printf Serial.printf

#elif defined(ARDUINO_ARCH_SAMD)
#include <algorithm>
#include <SPI.h>
#include "extras/SD/SD.h"
#define FLASH_MCR PSTR
#define FILE_SYSTEM File
#define FBJS_ENABLE_FS
#define HardwareSerial Serial_

#elif defined(ARDUINO_ARCH_STM32F1) || defined(ARDUINO_ARCH_STM32F4)
#define FLASH_MCR(s) (s)

#elif defined(TEENSYDUINO)
#define FILE_SYSTEM File
#define FLASH_MCR(s) (s)
#define HardwareSerial usb_serial_class
#define Serial_Printf Serial.printf

#endif

#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32F1) || defined(ARDUINO_ARCH_STM32F4)
#include "extras/print/printf.h"

extern "C" __attribute__((weak)) void _putchar(char c)
{
    Serial.print(c);
}

#define Serial_Printf printf

#endif

#include <Client.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "MB_JSON/MB_JSON.h"
#ifdef __cplusplus
}
#endif

#if defined(FBJS_ENABLE_SOFTWARE_SERIAL) || defined(ESP8266)
#include <SoftwareSerial.h>
#define FB_JS_INCLUDE_SW_SERIAL
#endif

#ifdef FBJS_ENABLE_WIFI_CLIENT
#include <WiFiClient.h>
#define FB_JS_INCLUDE_WIFI_CLIENT
#endif

#ifdef FBJS_ENABLE_WIFI_CLIENT_SECURE
#include <WiFiClientSecure.h>
#define FB_JS_INCLUDE_WIFI_CLIENT_SECURE
#endif

#ifdef FBJS_ENABLE_ARDUINO_MQTT
#include <ArduinoMqttClient.h>
#define FB_JS_INCLUDE_ARDUINO_MQTT
#endif

#ifdef FBJS_ENABLE_LW_MQTT
#include <MQTT.h>
#define FB_JS_INCLUDE_LW_MQTT
#endif

#if __has_include(<WiFiClient.h>)
#include <WiFiClient.h>
#define FB_JS_INCLUDE_WIFI_CLIENT
#endif

#if __has_include(<WiFiClientSecure.h>)
#include <WiFiClientSecure.h>
#define FB_JS_INCLUDE_WIFI_CLIENT_SECURE
#endif

#if __has_include(<ArduinoMqttClient.h>)
#include <ArduinoMqttClient.h>
#define FB_JS_INCLUDE_ARDUINO_MQTT
#endif

#if __has_include(<MQTT.h>)
#include <MQTT.h>
#define FB_JS_INCLUDE_LW_MQTT
#endif

/// HTTP codes see RFC7231
#define FBJS_ERROR_HTTP_CODE_OK 200
#define FBJS_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION 203
#define FBJS_ERROR_HTTP_CODE_NO_CONTENT 204
#define FBJS_ERROR_HTTP_CODE_MOVED_PERMANENTLY 301
#define FBJS_ERROR_HTTP_CODE_FOUND 302
#define FBJS_ERROR_HTTP_CODE_USE_PROXY 305
#define FBJS_ERROR_HTTP_CODE_TEMPORARY_REDIRECT 307
#define FBJS_ERROR_HTTP_CODE_PERMANENT_REDIRECT 308

static const char fb_json_str_1[] PROGMEM = "HTTP/1.1 ";
static const char fb_json_str_2[] PROGMEM = " ";
static const char fb_json_str_3[] PROGMEM = "Content-Type: ";
static const char fb_json_str_4[] PROGMEM = "Connection: ";
static const char fb_json_str_5[] PROGMEM = "keep-alive";
static const char fb_json_str_6[] PROGMEM = "Content-Length: ";
static const char fb_json_str_7[] PROGMEM = "\r\n";
static const char fb_json_str_8[] PROGMEM = "Transfer-Encoding: ";
static const char fb_json_str_9[] PROGMEM = "Location: ";

class FirebaseJson;
class FirebaseJsonArray;
class FirebaseJsonData;

static size_t getReservedLen(size_t len)
{
    int blen = len + 1;

    int newlen = (blen / 4) * 4;

    if (newlen < blen)
        newlen += 4;

    return (size_t)newlen;
}

static void *fb_js_malloc(size_t len)
{
    void *p;
    size_t newLen = getReservedLen(len);

#if defined(BOARD_HAS_PSRAM) && defined(FIREBASEJSON_USE_PSRAM)
    if ((p = (void *)ps_malloc(newLen)) == 0)
        return NULL;
#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
    ESP.setExternalHeap();
#endif

    bool nn = ((p = (void *)malloc(newLen)) > 0);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
    ESP.resetHeap();
#endif

    if (!nn)
        return NULL;
#endif
    return p;
}

static void fb_js_free(void *ptr)
{
    if (ptr)
        free(ptr);
}

static void *fb_js_realloc(void *ptr, size_t sz)
{
    size_t newLen = getReservedLen(sz);
#if defined(BOARD_HAS_PSRAM) && defined(FIREBASEJSON_USE_PSRAM)
    ptr = (void *)ps_realloc(ptr, newLen);
#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
    ESP.setExternalHeap();
#endif

    ptr = (void *)realloc(ptr, newLen);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
    ESP.resetHeap();
#endif

#endif
    if (!ptr)
        return NULL;

    return ptr;
}

static MB_JSON_Hooks MB_JSON_hooks __attribute__((used)) = {fb_js_malloc, fb_js_free, fb_js_realloc};

namespace FB_JS
{
    template <bool, typename T = void>
    struct enable_if
    {
    };
    template <typename T>
    struct enable_if<true, T>
    {
        typedef T type;
    };
    template <typename T, typename U>
    struct is_same
    {
        static bool const value = false;
    };
    template <typename T>
    struct is_same<T, T>
    {
        static bool const value = true;
    };

    template <typename T>
    struct is_num_int8
    {
        static bool const value = FB_JS::is_same<T, int8_t>::value || FB_JS::is_same<T, signed char>::value;
    };

    template <typename T>
    struct is_num_uint8
    {
        static bool const value = FB_JS::is_same<T, uint8_t>::value || FB_JS::is_same<T, unsigned char>::value;
    };

    template <typename T>
    struct is_num_int16
    {
        static bool const value = FB_JS::is_same<T, int16_t>::value || FB_JS::is_same<T, signed short>::value;
    };

    template <typename T>
    struct is_num_uint16
    {
        static bool const value = FB_JS::is_same<T, uint16_t>::value || FB_JS::is_same<T, unsigned short>::value;
    };

    template <typename T>
    struct is_num_int32
    {
        static bool const value = FB_JS::is_same<T, signed int>::value || FB_JS::is_same<T, int>::value ||
                                  FB_JS::is_same<T, int32_t>::value || FB_JS::is_same<T, long>::value ||
                                  FB_JS::is_same<T, signed long>::value;
    };

    template <typename T>
    struct is_num_uint32
    {
        static bool const value = FB_JS::is_same<T, unsigned int>::value || FB_JS::is_same<T, uint32_t>::value ||
                                  FB_JS::is_same<T, unsigned long>::value;
    };

    template <typename T>
    struct is_num_int64
    {
        static bool const value = FB_JS::is_same<T, int64_t>::value || FB_JS::is_same<T, signed long long>::value;
    };

    template <typename T>
    struct is_num_uint64
    {
        static bool const value = FB_JS::is_same<T, uint64_t>::value || FB_JS::is_same<T, unsigned long long>::value;
    };

    template <typename T>
    struct is_num_neg_int
    {
        static bool const value = FB_JS::is_num_int8<T>::value || FB_JS::is_num_int16<T>::value ||
                                  FB_JS::is_num_int32<T>::value || FB_JS::is_num_int64<T>::value;
    };

    template <typename T>
    struct is_num_pos_int
    {
        static bool const value = FB_JS::is_num_uint8<T>::value || FB_JS::is_num_uint16<T>::value ||
                                  FB_JS::is_num_uint32<T>::value || FB_JS::is_num_uint64<T>::value;
    };

    template <typename T>
    struct is_num_int
    {
        static bool const value = FB_JS::is_num_pos_int<T>::value || FB_JS::is_num_neg_int<T>::value;
    };

    template <typename T>
    struct is_num_float
    {
        static bool const value = FB_JS::is_same<T, float>::value || FB_JS::is_same<T, double>::value;
    };

    template <typename T>
    struct is_bool
    {
        static bool const value = FB_JS::is_same<T, bool>::value;
    };

    template <typename T>
    struct cs_t
    {
        static bool const value = FB_JS::is_same<T, char *>::value;
    };

    template <typename T>
    struct ccs_t
    {
        static bool const value = FB_JS::is_same<T, const char *>::value;
    };

    template <typename T>
    struct as_t
    {
        static bool const value = FB_JS::is_same<T, String>::value;
    };

    template <typename T>
    struct cas_t
    {
        static bool const value = FB_JS::is_same<T, const String>::value;
    };

    template <typename T>
    struct ss_t
    {
        static bool const value = FB_JS::is_same<T, std::string>::value;
    };

    template <typename T>
    struct css_t
    {
        static bool const value = FB_JS::is_same<T, const std::string>::value;
    };

    template <typename T>
    struct ssh_t
    {
        static bool const value = FB_JS::is_same<T, StringSumHelper>::value;
    };

    template <typename T>
    struct fs_t
    {
        static bool const value = FB_JS::is_same<T, const __FlashStringHelper *>::value;
    };

    template <typename T>
    struct mbs_t
    {
        static bool const value = FB_JS::is_same<T, MBSTRING>::value;
    };

    template <typename T>
    struct cmbs_t
    {
        static bool const value = FB_JS::is_same<T, const MBSTRING>::value;
    };

    template <typename T>
    struct pgm_t
    {
        static bool const value = FB_JS::is_same<T, PGM_P>::value;
    };

    template <typename T>
    struct is_const_chars
    {
        static bool const value = cs_t<T>::value || ccs_t<T>::value;
    };

    template <typename T>
    struct is_arduino_string
    {
        static bool const value = as_t<T>::value || cas_t<T>::value;
    };

    template <typename T>
    struct is_std_string
    {
        static bool const value = ss_t<T>::value || css_t<T>::value;
    };

    template <typename T>
    struct is_mb_string
    {
        static bool const value = mbs_t<T>::value || cmbs_t<T>::value;
    };

    template <typename T>
    struct is_string
    {
        static bool const value = is_const_chars<T>::value || is_arduino_string<T>::value ||
                                  ssh_t<T>::value || fs_t<T>::value ||
                                  is_std_string<T>::value || is_mb_string<T>::value;
    };

    typedef union
    {
        float floatval;
        int32_t int32;
        uint8_t byte[4];
    } intconv;

    struct server_response_data_t
    {
        int httpCode = -1;
        int payloadLen = -1;
        int contentLen = -1;
        int chunkRange = 0;
        int payloadOfs = 0;
        bool isChunkedEnc = false;
        bool noContent = false;
        MBSTRING location;
        MBSTRING contentType;
        MBSTRING connection;
        MBSTRING transferEnc;
    };

    struct serial_data_t
    {
        int pos = -1, start = -1, end = -1;
        int scnt = 0, ecnt = 0;
        MBSTRING buf;
        unsigned long dataTime = 0;
    };
};

class PGM2S
{
public:
    PGM2S() { init(1); }
    PGM2S(PGM_P p) { strP(p); }
    ~PGM2S() { delP(&buf); }
    const char *get() const { return buf; }

private:
    void init(size_t sz)
    {
        delP(&buf);
        buf = (char *)newP(sz + 1);
    }

    void strP(PGM_P pgm)
    {
        size_t len = strlen_P(pgm) + 5;
        init(len);
        strcpy_P(buf, pgm);
        buf[strlen_P(pgm)] = 0;
    }

    void delP(void *ptr)
    {
        void **p = (void **)ptr;
        if (*p)
        {
            free(*p);
            *p = 0;
        }
    }

    size_t getReservedLen(size_t len)
    {
        int blen = len + 1;

        int newlen = (blen / 4) * 4;

        if (newlen < blen)
            newlen += 4;

        return (size_t)newlen;
    }

    void *newP(size_t len)
    {
        void *p;
        size_t newLen = getReservedLen(len);
#if defined(BOARD_HAS_PSRAM) && defined(FIREBASEJSON_USE_PSRAM)

        if ((p = (void *)ps_malloc(newLen)) == 0)
            return NULL;

#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.setExternalHeap();
#endif

        bool nn = ((p = (void *)malloc(len)) > 0);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.resetHeap();
#endif

        if (!nn)
            return NULL;

#endif
        memset(p, 0, newLen);
        return p;
    }

    char *buf = nullptr;
};

class NUM2S
{
public:
    NUM2S() { nullStr(); }
    NUM2S(unsigned long long value) { uint64Str(value); }
    NUM2S(signed long long value) { int64Str(value); }
    NUM2S(unsigned int value) { uint64Str(value); }
    NUM2S(unsigned long value) { uint64Str(value); }
    NUM2S(int value) { int64Str(value); }
    NUM2S(bool value) { boolStr(value); }
    NUM2S(float value, int precision = 5) { floatStr(value, precision); }
    NUM2S(double value, int precision = 9) { doubleStr(value, precision); }
    ~NUM2S() { delP(&buf); }
    const char *get() const { return buf; }

private:
    /*** dtostrf function is taken from 
     * https://github.com/stm32duino/Arduino_Core_STM32/blob/master/cores/arduino/avr/dtostrf.c
    */

    /***
     * dtostrf - Emulation for dtostrf function from avr-libc
     * Copyright (c) 2013 Arduino.  All rights reserved.
     * Written by Cristian Maglie <c.maglie@arduino.cc>
     * This library is free software; you can redistribute it and/or
     * modify it under the terms of the GNU Lesser General Public
     * License as published by the Free Software Foundation; either
     * version 2.1 of the License, or (at your option) any later version.
     * This library is distributed in the hope that it will be useful,
     * but WITHOUT ANY WARRANTY; without even the implied warranty of
     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     * Lesser General Public License for more details.
     * You should have received a copy of the GNU Lesser General Public
     * License along with this library; if not, write to the Free Software
     * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    */

    char *dtostrf(double val, signed char width, unsigned char prec, char *sout)
    {
        //Commented code is the original version
        /***
          char fmt[20];
          sprintf(fmt, "%%%d.%df", width, prec);
          sprintf(sout, fmt, val);
          return sout;
        */

        // Handle negative numbers
        uint8_t negative = 0;
        if (val < 0.0)
        {
            negative = 1;
            val = -val;
        }

        // Round correctly so that print(1.999, 2) prints as "2.00"
        double rounding = 0.5;
        for (int i = 0; i < prec; ++i)
        {
            rounding /= 10.0;
        }

        val += rounding;

        // Extract the integer part of the number
        unsigned long int_part = (unsigned long)val;
        double remainder = val - (double)int_part;

        if (prec > 0)
        {
            // Extract digits from the remainder
            unsigned long dec_part = 0;
            double decade = 1.0;
            for (int i = 0; i < prec; i++)
            {
                decade *= 10.0;
            }
            remainder *= decade;
            dec_part = (int)remainder;

            if (negative)
            {
                sprintf(sout, "-%ld.%0*ld", int_part, prec, dec_part);
            }
            else
            {
                sprintf(sout, "%ld.%0*ld", int_part, prec, dec_part);
            }
        }
        else
        {
            if (negative)
            {
                sprintf(sout, "-%ld", int_part);
            }
            else
            {
                sprintf(sout, "%ld", int_part);
            }
        }
        // Handle minimum field width of the output string
        // width is signed value, negative for left adjustment.
        // Range -128,127

        char *fmt = (char *)newP(129);
        unsigned int w = width;
        if (width < 0)
        {
            negative = 1;
            w = -width;
        }
        else
        {
            negative = 0;
        }

        if (strlen(sout) < w)
        {
            memset(fmt, ' ', 128);
            fmt[w - strlen(sout)] = '\0';
            if (negative == 0)
            {
                char *tmp = (char *)newP(strlen(sout) + 1);
                strcpy(tmp, sout);
                strcpy(sout, fmt);
                strcat(sout, tmp);
                delP(&tmp);
            }
            else
            {
                // left adjustment
                strcat(sout, fmt);
            }
        }

        delP(&fmt);

        return sout;
    }

    void init(size_t sz)
    {
        delP(&buf);
        buf = (char *)newP(sz + 1);
    }

    void delP(void *ptr)
    {
        void **p = (void **)ptr;
        if (*p)
        {
            free(*p);
            *p = 0;
        }
    }

    size_t getReservedLen(size_t len)
    {
        int blen = len + 1;

        int newlen = (blen / 4) * 4;

        if (newlen < blen)
            newlen += 4;

        return (size_t)newlen;
    }

    void *newP(size_t len)
    {
        void *p;
        size_t newLen = getReservedLen(len);
#if defined(BOARD_HAS_PSRAM) && defined(FIREBASEJSON_USE_PSRAM)

        if ((p = (void *)ps_malloc(newLen)) == 0)
            return NULL;

#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.setExternalHeap();
#endif

        bool nn = ((p = (void *)malloc(newLen)) > 0);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.resetHeap();
#endif

        if (!nn)
            return NULL;

#endif
        memset(p, 0, newLen);
        return p;
    }

    char *intStr(int value)
    {
        char *t = (char *)newP(36);
        sprintf(t, (const char *)FLASH_MCR("%d"), value);
        return t;
    }

    void int64Str(signed long long value)
    {
        init(64);
        sprintf(buf, (const char *)FLASH_MCR("%lld"), value);
    }

    void uint64Str(unsigned long long value)
    {
        init(64);
        sprintf(buf, (const char *)FLASH_MCR("%llu"), value);
    }

    void boolStr(bool value)
    {
        init(8);
        value ? strcpy(buf, (const char *)FLASH_MCR("true")) : strcpy(buf, (const char *)FLASH_MCR("false"));
    }

    void floatStr(float value, int precision)
    {
        init(32);
        dtostrf(value, (precision + 2), precision, buf);
        trim();
    }

    void doubleStr(double value, int precision)
    {
        init(64);
        dtostrf(value, (precision + 2), precision, buf);
        trim();
    }

    void nullStr()
    {
        init(6);
        strcpy(buf, (const char *)FLASH_MCR("null"));
    }

    void trim()
    {
        size_t i = strlen(buf) - 1;
        while (buf[i] == '0' && i > 0)
        {
            if (buf[i - 1] == '.')
            {
                i--;
                break;
            }
            if (buf[i - 1] != '0')
                break;
            i--;
        }
        if (i < strlen(buf) - 1)
            buf[i] = '\0';
    }

    char *buf = nullptr;
};

class FirebaseJsonData
{
    friend class FirebaseJsonBase;
    friend class FirebaseJson;
    friend class FirebaseJsonArray;

public:
    FirebaseJsonData();
    ~FirebaseJsonData();

    /**
     * Get array data as FirebaseJsonArray object from FirebaseJsonData object.
     * 
     * @param jsonArray The returning FirebaseJsonArray object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
    bool getArray(FirebaseJsonArray &jsonArray);

    /**
     * Get array data as FirebaseJsonArray object from string.
     * 
     * @param source The JSON array string.
     * @param jsonArray The returning FirebaseJsonArray object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
    template <typename T>
    bool getArray(T source, FirebaseJsonArray &jsonArray) { return mGetArray(getStr(source), jsonArray); }

    /**
     * Get JSON data as FirebaseJson object from FirebaseJsonData object.
     * 
     * @param json The returning FirebaseJson object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
    bool getJSON(FirebaseJson &json);

    /**
     * Get JSON data as FirebaseJson object from string.
     * 
     * @param source The JSON string.
     * @param json The returning FirebaseJson object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
    template <typename T>
    bool getJSON(T source, FirebaseJson &json) { return mGetJSON(getStr(source), json); }

    /**
     * Get the value by specific type from FirebaseJsonData object.
     * This should call after parse or get function.
    */
    template <typename T>
    auto to() -> typename FB_JS::enable_if<FB_JS::is_num_int<T>::value || FB_JS::is_num_float<T>::value || FB_JS::is_bool<T>::value, T>::type
    {
        if (FB_JS::is_bool<T>::value)
            return iVal.uint32 > 0;
        else if (FB_JS::is_num_int8<T>::value)
            return iVal.int8;
        else if (FB_JS::is_num_uint8<T>::value)
            return iVal.uint8;
        else if (FB_JS::is_num_int16<T>::value)
            return iVal.int16;
        else if (FB_JS::is_num_uint16<T>::value)
            return iVal.uint16;
        else if (FB_JS::is_num_int32<T>::value)
            return iVal.int32;
        else if (FB_JS::is_num_uint32<T>::value)
            return iVal.uint32;
        else if (FB_JS::is_num_int64<T>::value)
            return iVal.int64;
        else if (FB_JS::is_num_uint64<T>::value)
            return iVal.uint64;
        else if (FB_JS::is_same<T, float>::value)
            return fVal.f;
        else if (FB_JS::is_same<T, double>::value)
            return fVal.d;
        else
            return 0;
    }

    template <typename T>
    auto to() -> typename FB_JS::enable_if<FB_JS::is_const_chars<T>::value || FB_JS::is_std_string<T>::value || FB_JS::is_arduino_string<T>::value || FB_JS::is_mb_string<T>::value, T>::type
    {
        return stringValue.c_str();
    }

    template <typename T>
    auto get(T &json) -> typename FB_JS::enable_if<FB_JS::is_same<T, FirebaseJson>::value>::type
    {
        getJSON(json);
    }

    template <typename T>
    auto get(T &arr) -> typename FB_JS::enable_if<FB_JS::is_same<T, FirebaseJsonArray>::value>::type
    {
        getArray(arr);
    }

    /**
     * Clear internal buffer.
    */
    void clear();

    /**
     * The String value of parses data.
    */
    String stringValue;

    /**
     * The int value of parses data.
    */
    int intValue = 0;

    /**
     * The float value of parses data.
    */
    float floatValue = 0.0f;

    /**
   * The double value of parses data.
  */
    double doubleValue = 0.0;

    /**
     * The bool value of parses data.
    */
    bool boolValue = false;

    /**
     * The type String of parses data.
    */
    String type;

    /**
     * The type (number) of parses data.
    */
    uint8_t typeNum = 0;

    /**
     * The returning full path of current search.
    */
    String searchPath;

    /**
     * The success flag of parsing data.
    */
    bool success = false;

private:
    union IVal
    {
        std::uint64_t uint64;
        std::int64_t int64;
        std::uint32_t uint32;
        std::int32_t int32;
        std::int16_t int16;
        std::uint16_t uint16;
        std::int8_t int8;
        std::uint8_t uint8;
    };

    struct FVal
    {
        double d = 0;
        float f = 0;
        void setd(double v)
        {
            d = v;
            f = static_cast<float>(v);
        }

        void setf(float v)
        {
            f = v;
            d = static_cast<double>(v);
        }
    };

    IVal iVal = {0};
    FVal fVal;
    uint8_t type_num = 0;

    bool mGetArray(const char *source, FirebaseJsonArray &jsonArray);
    bool mGetJSON(const char *source, FirebaseJson &json);

protected:
    template <typename T>
    auto getStr(const T &val) -> typename FB_JS::enable_if<FB_JS::is_std_string<T>::value || FB_JS::is_arduino_string<T>::value || FB_JS::is_mb_string<T>::value || FB_JS::is_same<T, StringSumHelper>::value, const char *>::type
    {
        return val.c_str();
    }

    template <typename T>
    auto getStr(T val) -> typename FB_JS::enable_if<FB_JS::is_const_chars<T>::value, const char *>::type { return val; }

    template <typename T>
    auto getStr(T val) -> typename FB_JS::enable_if<FB_JS::fs_t<T>::value, const char *>::type { return (const char *)val; }
};

class FirebaseJsonBase
{
    friend class FirebaseJson;
    friend class FirebaseJsonArray;
    friend class FirebaseJsonData;

private:
    typedef enum
    {
        fb_json_serialize_mode_none = -1,
        fb_json_serialize_mode_plain = 0,
        fb_json_serialize_mode_pretty = 1
    } fb_json_serialize_mode;

    enum key_status
    {
        key_status_not_existed = 0,
        key_status_existed = 1,
        key_status_mistype = 2,
        key_status_out_of_range = 3
    };

    typedef enum
    {
        search_mode_none = 0,
        search_mode_once = 1,
        search_mode_all = 2
    } fb_json_search_mode;

    struct search_result_t
    {
        MB_JSON *parent = NULL;
        key_status status = key_status_not_existed;
        int foundIndex = -1;
        int stopIndex = 0;
    };

    struct iterator_result_t
    {
        uint16_t ofs1 = 0;
        uint8_t len1 = 0;
        uint8_t ofs2 = 0;
        uint16_t len2 = 0;
        uint8_t type = 0;
        int16_t depth = -1;
    };

    struct iterator_data_t
    {
        std::vector<struct iterator_result_t> result;
        int buf_offset = 0;
        size_t buf_size = 0;
        int depth = -1;
        int _depth = 0;
        int searchKeyDepth = -1;
        bool searchEnable = false;
        bool searchFinished = false;
        int matchesCount = 0;
        MB_JSON *parent = NULL;
        MB_JSON *parentArr = NULL;
        MBSTRING path;
        std::vector<MBSTRING> *searchKeys = NULL;
        std::vector<MBSTRING> pathList;
    };

    struct fb_js_iterator_value_t
    {
        int type = 0;
        int depth = 0;
        String key;
        String value;
    };

    struct fb_js_search_criteria_t
    {
        int depth = 0;
        int endDepth = -1;
        bool searchAll = false;
        String path;
        String value;
    };

    FirebaseJsonBase &mClear();
    void mIteratorEnd(bool clearBuf = true);
    bool setRaw(const char *raw);
    void prepareRoot();
    MB_JSON *parse(const char *raw);
    void searchElements(std::vector<MBSTRING> &keys, MB_JSON *parent, struct search_result_t &r);
    MB_JSON *getElement(MB_JSON *parent, const char *key, struct search_result_t &r);
    void mAdd(std::vector<MBSTRING> keys, MB_JSON **parent, int beginIndex, MB_JSON *value);
    void makeList(const char *str, std::vector<MBSTRING> &keys, char delim);
    void clearList(std::vector<MBSTRING> &keys);
    bool isArray(MB_JSON *e);
    bool isObject(MB_JSON *e);
    MB_JSON *addArray(MB_JSON *parent, MB_JSON *e, size_t size);
    void appendArray(std::vector<MBSTRING> &keys, struct search_result_t &r, MB_JSON *parent, MB_JSON *value);
    void replaceItem(std::vector<MBSTRING> &keys, struct search_result_t &r, MB_JSON *parent, MB_JSON *value);
    void replace(std::vector<MBSTRING> &keys, struct search_result_t &r, MB_JSON *parent, MB_JSON *item);
    size_t mIteratorBegin(MB_JSON *parent);
    size_t mIteratorBegin(MB_JSON *parent, std::vector<MBSTRING> *keys, struct fb_js_search_criteria_t *criteria);
    void collectResult(MB_JSON *e, const char *key, int arrIndex, struct fb_js_search_criteria_t *criteria);
    void removeDepthPath();
    void mCollectIterator(MB_JSON *e, int type, int &arrIndex, struct fb_js_search_criteria_t *criteria);
    void mIterate(MB_JSON *parent, int &arrIndex, struct fb_js_search_criteria_t *criteria);
    bool checkKeys(struct fb_js_search_criteria_t *criteria);
    int mIteratorGet(size_t index, int &type, String &key, String &value);
    struct fb_js_iterator_value_t mValueAt(size_t index);
    void toBuf(fb_json_serialize_mode mode);
    bool mReadClient(Client *client);
    bool mReadStream(Stream *s, int timeoutMS);
    const char *mRaw();
    bool mRemove(const char *path);
    void mGetPath(MBSTRING &path, std::vector<MBSTRING> paths, int begin = 0, int end = -1);
    size_t mGetSerializedBufferLength(bool prettify);
    void mSetFloatDigits(uint8_t digits);
    void mSetDoubleDigits(uint8_t digits);
    int mResponseCode();
    bool mGet(MB_JSON *parent, FirebaseJsonData *result, const char *path, bool prettify = false);
    void mSetResInt(FirebaseJsonData *data, const char *value);
    void mSetResFloat(FirebaseJsonData *data, const char *value);
    void mSetElementType(FirebaseJsonData *result);
    void mSet(const char *path, MB_JSON *value);
    void mCopy(FirebaseJsonBase &other);
    size_t mSearch(MB_JSON *parent, struct fb_js_search_criteria_t *criteria);
    size_t mSearch(MB_JSON *parent, FirebaseJsonData *result, struct fb_js_search_criteria_t *criteria, bool prettify = false);
    size_t mSearch(MB_JSON *parent, const char *path, bool searchAll = false);
    const char *mGetElementFullPath(MB_JSON *parent, const char *path, bool searchAll = false);

public:
    enum fb_json_root_type
    {
        Root_Type_JSON = 0,
        Root_Type_JSONArray = 1,
    };

    FirebaseJsonBase();
    virtual ~FirebaseJsonBase();

    typedef enum
    {
        fb_json_func_type_undefined = 0,
        fb_json_func_type_set_data,
        fb_json_func_type_add,
        fb_json_func_type_set,
        fb_json_func_type_get,
        fb_json_func_type_remove
    } fb_json_func_type_t;

    enum fb_js_json_data_type
    {
        JSON_UNDEFINED = 0,
        JSON_OBJECT = 1,
        JSON_ARRAY = 2,
        JSON_STRING = 3,
        JSON_INT = 4,
        JSON_FLOAT = 5,
        JSON_DOUBLE = 6,
        JSON_BOOL = 7,
        JSON_NULL = 8
    };

protected:
    uint8_t doubleDigits = 9;
    uint8_t floatDigits = 5;
    int httpCode = 0;
    int errorPos = -1;
    struct FB_JS::serial_data_t serData;
    fb_json_root_type root_type = Root_Type_JSON;
    struct iterator_data_t iterator_data;
    MB_JSON *root = NULL;
    MB_JSON_Hooks *hooks = NULL;
    MBSTRING buf;

    template <typename T>
    auto getStr(const T &val) -> typename FB_JS::enable_if<FB_JS::is_std_string<T>::value || FB_JS::is_arduino_string<T>::value || FB_JS::is_mb_string<T>::value || FB_JS::is_same<T, StringSumHelper>::value, const char *>::type
    {
        return val.c_str();
    }

    template <typename T>
    auto getStr(T val) -> typename FB_JS::enable_if<FB_JS::is_const_chars<T>::value, const char *>::type { return val; }

    template <typename T>
    auto getStr(T val) -> typename FB_JS::enable_if<FB_JS::fs_t<T>::value, const char *>::type { return (const char *)val; }

    template <typename T>
    bool toStringPtrHandler(T *ptr, bool prettify)
    {
        if (!root || !ptr)
            return false;

        if (std::is_same<T, char>::value)
        {
            char *p = prettify ? MB_JSON_Print(root) : MB_JSON_PrintUnformatted(root);
            if (p)
            {
                strcpy(ptr, p);
                MB_JSON_free(p);
                return true;
            }
        }
        return false;
    }

    template <typename T>
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_string<T>::value, bool>::type
    {
        if (!root)
            return false;

        char *p = prettify ? MB_JSON_Print(root) : MB_JSON_PrintUnformatted(root);
        if (p)
        {
            out = p;
            MB_JSON_free(p);
            return true;
        }
        return false;
    }

    template <typename T>
#ifdef FB_JS_INCLUDE_SW_SERIAL
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, HardwareSerial>::value || FB_JS::is_same<T, SoftwareSerial>::value, bool>::type
#else
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, HardwareSerial>::value, bool>::type
#endif
    {
        char *p = prettify ? MB_JSON_Print(root) : MB_JSON_PrintUnformatted(root);
        if (p)
        {
            out.print(p);
            MB_JSON_free(p);
            return true;
        }
        return false;
    }

#ifdef FB_JS_INCLUDE_LW_MQTT
    template <typename T1, typename T2>
    auto toStringHandler(T1 &out, T2 topic) -> typename FB_JS::enable_if<FB_JS::is_same<T1, MQTTClient>::value && FB_JS::is_string<T2>::value, bool>::type
    {
        char *p = MB_JSON_PrintUnformatted(root);
        if (p)
        {
            out.publish(topic, p);
            MB_JSON_free(p);
            return true;
        }
        return false;
    }
#endif

#if defined(FBJS_ENABLE_FS)
    template <typename T>
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, FILE_SYSTEM>::value || FB_JS::is_same<T, File>::value, bool>::type
    {
        return writeHelper(out, prettify);
    }
#endif

    template <typename T>
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, Client>::value, bool>::type
    {
        return writeHelper(out, prettify);
    }

#if defined(FB_JS_INCLUDE_WIFI_CLIENT)
    template <typename T>
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, WiFiClient>::value, bool>::type
    {
        return writeHelper(out, prettify);
    }
#endif

#if defined(FB_JS_INCLUDE_WIFI_CLIENT_SECURE)
    template <typename T>
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, WiFiClientSecure>::value, bool>::type
    {
        return writeHelper(out, prettify);
    }
#endif

#if defined(FB_JS_INCLUDE_ARDUINO_MQTT)
    template <typename T>
    auto toStringHandler(T &out, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_same<T, MqttClient>::value, bool>::type
    {
        return writeHelper(out, prettify);
    }
#endif

    template <typename T>
    bool writeHelper(T &out, bool prettify)
    {
        bool ret = false;

        if (!root)
            return false;

        if (out)
        {
            char *p = prettify ? MB_JSON_Print(root) : MB_JSON_PrintUnformatted(root);
            if (p)
            {
                ret = out.write((const uint8_t *)p, strlen(p)) == strlen(p);
                MB_JSON_free(p);
                return ret;
            }
        }
        return ret;
    }

    void idle()
    {
        yield();
        delay(0);
    }

    void clearS(MBSTRING &s)
    {
        s.clear();
        MBSTRING().swap(s);
    }

    void shrinkS(MBSTRING &s)
    {
#if defined(ESP32)
        s.shrink_to_fit();
#else
        MBSTRING t = s;
        clearS(s);
        s = t;
        clearS(t);
#endif
    }

    void delP(void *ptr)
    {
        void **p = (void **)ptr;
        if (*p)
        {
            free(*p);
            *p = 0;
        }
    }

    size_t getReservedLen(size_t len)
    {
        int blen = len + 1;

        int newlen = (blen / 4) * 4;

        if (newlen < blen)
            newlen += 4;

        return (size_t)newlen;
    }

    void *newP(size_t len)
    {
        void *p;
        size_t newLen = getReservedLen(len);
#if defined(BOARD_HAS_PSRAM) && defined(FIREBASEJSON_USE_PSRAM)

        if ((p = (void *)ps_malloc(newLen)) == 0)
            return NULL;

#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.setExternalHeap();
#endif

        bool nn = ((p = (void *)malloc(newLen)) > 0);

#if defined(ESP8266_USE_EXTERNAL_HEAP)
        ESP.resetHeap();
#endif

        if (!nn)
            return NULL;

#endif
        memset(p, 0, newLen);
        return p;
    }

    void strcat_c(char *str, char c)
    {
        for (; *str; str++)
            ;
        *str++ = c;
        *str++ = 0;
    }

    char *strP(PGM_P pgm)
    {
        size_t len = strlen_P(pgm) + 1;
        char *buf = (char *)newP(len);
        strcpy_P(buf, pgm);
        buf[len - 1] = 0;
        return buf;
    }

    int strpos(const char *haystack, const char *needle, int offset)
    {
        if (!haystack || !needle)
            return -1;

        int hlen = strlen(haystack);
        int nlen = strlen(needle);

        if (hlen == 0 || nlen == 0)
            return -1;

        int hidx = offset, nidx = 0;
        while ((*(haystack + hidx) != '\0') && (*(needle + nidx) != '\0') && hidx < hlen)
        {
            if (*(needle + nidx) != *(haystack + hidx))
            {
                hidx++;
                nidx = 0;
            }
            else
            {
                nidx++;
                hidx++;
                if (nidx == nlen)
                    return hidx - nidx;
            }
        }

        return -1;
    }

    int strpos(const char *haystack, char needle, int offset)
    {
        if (!haystack || needle == 0)
            return -1;

        int hlen = strlen(haystack);

        if (hlen == 0)
            return -1;

        int hidx = offset;
        while ((*(haystack + hidx) != '\0') && hidx < hlen)
        {
            if (needle == *(haystack + hidx))
                return hidx;
            hidx++;
        }

        return -1;
    }

    int rstrpos(const char *haystack, const char *needle, int offset)
    {
        if (!haystack || !needle)
            return -1;

        int hlen = strlen(haystack);
        int nlen = strlen(needle);

        if (hlen == 0 || nlen == 0)
            return -1;

        int hidx = hlen - 1, nidx = nlen - 1;
        while (offset < hidx)
        {
            if (*(needle + nidx) != *(haystack + hidx))
            {
                hidx--;
                nidx = nlen - 1;
            }
            else
            {
                nidx--;
                hidx--;
                if (nidx == 0)
                    return hidx + nidx;
            }
        }

        return -1;
    }

    int rstrpos(const char *haystack, char needle, int offset)
    {
        if (!haystack || needle == 0)
            return -1;

        int hlen = strlen(haystack);

        if (hlen == 0)
            return -1;

        int hidx = hlen - 1;
        while (offset < hidx)
        {
            if (needle == *(haystack + hidx))
                return hidx;
            hidx--;
        }

        return -1;
    }

    void substr(MBSTRING &str, const char *s, int offset, size_t len)
    {
        if (!s)
            return;

        int slen = strlen(s);

        if (slen == 0)
            return;

        int last = offset + len;

        if (offset >= slen || len == 0 || last > slen)
            return;

        for (int i = offset; i < last; i++)
            str += s[i];
    }

    void trimDouble(char *buf)
    {
        size_t i = strlen(buf) - 1;
        while (buf[i] == '0' && i > 0)
        {
            if (buf[i - 1] == '.')
            {
                i--;
                break;
            }
            if (buf[i - 1] != '0')
                break;
            i--;
        }
        if (i < strlen(buf) - 1)
            buf[i] = '\0';
    }

    void storeS(MBSTRING &s, const char *v, bool append)
    {
        if (!append)
            clearS(s);
#if defined(ESP32)
        s += v;
        s.shrink_to_fit();
#else
        if (!append)
            s = v;
        else
        {
            MBSTRING t = s;
            t += v;
            clearS(s);
            s = t;
            clearS(t);
        }
#endif
    }

    void storeS(MBSTRING &s, char v, bool append)
    {
        MBSTRING t;
        t += v;
        storeS(s, t.c_str(), append);
        clearS(t);
    }

    inline int ishex(int x)
    {
        return (x >= '0' && x <= '9') ||
               (x >= 'a' && x <= 'f') ||
               (x >= 'A' && x <= 'F');
    }

    void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 <= 9 ? '0' : 'a' - 10;
        hex2 += hex2 <= 9 ? '0' : 'a' - 10;
    }

    char from_hex(char ch)
    {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
    }

    uint32_t hex2int(const char *hex)
    {
        uint32_t val = 0;
        while (*hex)
        {
            // get current character then increment
            uint8_t byte = *hex++;
            // transform hex character to the 4bit equivalent number, using the ascii table indexes
            if (byte >= '0' && byte <= '9')
                byte = byte - '0';
            else if (byte >= 'a' && byte <= 'f')
                byte = byte - 'a' + 10;
            else if (byte >= 'A' && byte <= 'F')
                byte = byte - 'A' + 10;
            // shift 4 to make space for new digit, and add the 4 bits of the new digit
            val = (val << 4) | (byte & 0xF);
        }
        return val;
    }

    char *getHeader(const char *buf, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {

        char *tmp = strP(beginH);
        int p1 = strpos(buf, tmp, beginPos);
        int ofs = 0;
        delP(&tmp);
        if (p1 != -1)
        {
            tmp = strP(endH);
            int p2 = -1;
            if (endPos > 0)
                p2 = endPos;
            else if (endPos == 0)
            {
                ofs = strlen_P(endH);
                p2 = strpos(buf, tmp, p1 + strlen_P(beginH) + 1);
            }
            else if (endPos == -1)
            {
                beginPos = p1 + strlen_P(beginH);
            }

            if (p2 == -1)
                p2 = strlen(buf);

            delP(&tmp);

            if (p2 != -1)
            {
                beginPos = p2 + ofs;
                int len = p2 - p1 - strlen_P(beginH);
                tmp = (char *)newP(len + 1);
                memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
                return tmp;
            }
        }

        return nullptr;
    }

    void parseRespHeader(const char *buf, struct FB_JS::server_response_data_t &response)
    {
        int beginPos = 0, pmax = 0, payloadPos = 0;

        char *tmp = nullptr;

        if (response.httpCode != -1)
        {
            payloadPos = beginPos;
            pmax = beginPos;
            tmp = getHeader(buf, fb_json_str_1, fb_json_str_7, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delP(&tmp);
            }
            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_json_str_3, fb_json_str_7, beginPos, 0);
            if (tmp)
            {
                response.contentType = tmp;
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_json_str_6, fb_json_str_7, beginPos, 0);
            if (tmp)
            {
                response.contentLen = atoi(tmp);
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_json_str_8, fb_json_str_7, beginPos, 0);
            if (tmp)
            {
                response.transferEnc = tmp;
                if (strcmp(tmp, (const char *)FLASH_MCR("chunked")) == 0)
                    response.isChunkedEnc = true;
                delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, fb_json_str_4, fb_json_str_7, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delP(&tmp);
            }

            if (response.httpCode == FBJS_ERROR_HTTP_CODE_OK || response.httpCode == FBJS_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || response.httpCode == FBJS_ERROR_HTTP_CODE_PERMANENT_REDIRECT || response.httpCode == FBJS_ERROR_HTTP_CODE_MOVED_PERMANENTLY || response.httpCode == FBJS_ERROR_HTTP_CODE_FOUND)
            {
                if (pmax < beginPos)
                    pmax = beginPos;
                beginPos = payloadPos;
                tmp = getHeader(buf, fb_json_str_9, fb_json_str_7, beginPos, 0);
                if (tmp)
                {
                    response.location = tmp;
                    delP(&tmp);
                }
            }

            if (response.httpCode == FBJS_ERROR_HTTP_CODE_NO_CONTENT)
                response.noContent = true;
        }
    }

    int readLine(Client *stream, char *buf, int bufLen)
    {
        int res = -1;
        char c = 0;
        int idx = 0;
        if (!stream)
            return idx;
        while (stream->available() && idx <= bufLen)
        {
            if (!stream)
                break;
            res = stream->read();
            if (res > -1)
            {
                c = (char)res;
                strcat_c(buf, c);
                idx++;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readLine(Client *stream, MBSTRING &buf)
    {
        int res = -1;
        char c = 0;
        int idx = 0;
        if (!stream)
            return idx;
        while (stream->available())
        {
            if (!stream)
                break;
            res = stream->read();
            if (res > -1)
            {
                c = (char)res;
                buf += c;
                idx++;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readChunkedData(Client *stream, char *out, int &chunkState, int &chunkedSize, int &dataLen, int bufLen)
    {
        char *tmp = nullptr;
        char *buf = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            buf = (char *)newP(bufLen);
            int readLen = readLine(stream, buf, bufLen);
            if (readLen)
            {
                p1 = strpos(buf, ';', 0);
                if (p1 == -1)
                {
                    tmp = strP(fb_json_str_7);
                    p1 = strpos(buf, tmp, 0);
                    delP(&tmp);
                }

                if (p1 != -1)
                {
                    tmp = (char *)newP(p1 + 1);
                    memcpy(tmp, buf, p1);
                    chunkedSize = hex2int(tmp);
                    delP(&tmp);
                }

                //last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;

            delP(&buf);
        }
        else
        {

            if (chunkedSize > -1)
            {
                buf = (char *)newP(bufLen);
                int readLen = readLine(stream, buf, bufLen);

                if (readLen > 0)
                {
                    //chunk may contain trailing
                    if (dataLen + readLen - 2 < chunkedSize)
                    {
                        dataLen += readLen;
                        memcpy(out, buf, readLen);
                        olen = readLen;
                    }
                    else
                    {
                        if (chunkedSize - dataLen > 0)
                            memcpy(out, buf, chunkedSize - dataLen);
                        dataLen = chunkedSize;
                        chunkState = 0;
                        olen = readLen;
                    }
                }
                else
                {
                    olen = -1;
                }

                delP(&buf);
            }
        }

        return olen;
    }

    int readChunkedData(Client *stream, MBSTRING &out, int &chunkState, int &chunkedSize, int &dataLen)
    {
        char *tmp = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            MBSTRING s;
            int readLen = readLine(stream, s);
            if (readLen)
            {
                p1 = strpos(s.c_str(), ';', 0);
                if (p1 == -1)
                {
                    tmp = strP(fb_json_str_7);
                    p1 = strpos(s.c_str(), tmp, 0);
                    delP(&tmp);
                }

                if (p1 != -1)
                {
                    tmp = (char *)newP(p1 + 1);
                    memcpy(tmp, s.c_str(), p1);
                    chunkedSize = hex2int(tmp);
                    delP(&tmp);
                }

                //last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;
        }
        else
        {

            if (chunkedSize > -1)
            {
                MBSTRING s;
                int readLen = readLine(stream, s);

                if (readLen > 0)
                {
                    //chunk may contain trailing
                    if (dataLen + readLen - 2 < chunkedSize)
                    {
                        dataLen += readLen;
                        out += s;
                        olen = readLen;
                    }
                    else
                    {
                        if (chunkedSize - dataLen > 0)
                            out += s;
                        dataLen = chunkedSize;
                        chunkState = 0;
                        olen = readLen;
                    }
                }
                else
                {
                    olen = -1;
                }
            }
        }

        return olen;
    }

    int readClient(Client *client, MBSTRING &buf)
    {
        int ret = -1;

        char *pChunk = nullptr;
        char *tmp = nullptr;
        char *header = nullptr;
        bool isHeader = false;

        struct FB_JS::server_response_data_t response;

        int chunkIdx = 0;
        int pChunkIdx = 0;
        int hBufPos = 0;
        int chunkBufSize = client->available();
        int hstate = 0;
        int chunkedDataState = 0;
        int chunkedDataSize = 0;
        int chunkedDataLen = 0;
        int payloadRead = 0;

        int defaultChunkSize = 2048;
        unsigned long dataTime = millis();

        while (client->connected() && chunkBufSize == 0 && millis() - dataTime < 5000)
        {
            chunkBufSize = client->available();
            delay(0);
        }

        if (client->connected())
        {
            int availablePayload = chunkBufSize;

            dataTime = millis();

            if (chunkBufSize > 0)
            {
                while (chunkBufSize > 0 || availablePayload > 0 || payloadRead < response.contentLen)
                {

                    chunkBufSize = client->available();

                    if (chunkBufSize <= 0 && availablePayload <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                        break;

                    if (chunkBufSize > 0)
                    {
                        chunkBufSize = defaultChunkSize;

                        if (chunkIdx == 0)
                        {
                            //the first chunk can be http response header
                            header = (char *)newP(chunkBufSize);
                            hstate = 1;
                            int readLen = readLine(client, header, chunkBufSize);
                            int pos = 0;

                            tmp = getHeader(header, fb_json_str_1, fb_json_str_2, pos, 0);
                            idle();
                            dataTime = millis();
                            if (tmp)
                            {
                                //http response header with http response code
                                isHeader = true;
                                hBufPos = readLen;
                                response.httpCode = atoi(tmp);
                                httpCode = response.httpCode;
                                delP(&tmp);
                            }
                        }
                        else
                        {
                            idle();
                            dataTime = millis();
                            //the next chunk data can be the remaining http header
                            if (isHeader)
                            {
                                //read one line of next header field until the empty header has found
                                tmp = (char *)newP(chunkBufSize);
                                int readLen = readLine(client, tmp, chunkBufSize);
                                bool headerEnded = false;

                                //check is it the end of http header (\n or \r\n)?
                                if (readLen == 1)
                                    if (tmp[0] == '\r')
                                        headerEnded = true;

                                if (readLen == 2)
                                    if (tmp[0] == '\r' && tmp[1] == '\n')
                                        headerEnded = true;

                                if (headerEnded)
                                {
                                    clearS(buf);
                                    //parse header string to get the header field
                                    isHeader = false;
                                    parseRespHeader(header, response);

                                    if (hstate == 1)
                                        delP(&header);
                                    hstate = 0;

                                    if (response.contentLen == 0)
                                    {
                                        delP(&tmp);
                                        break;
                                    }
                                }
                                else
                                {
                                    //accumulate the remaining header field
                                    memcpy(header + hBufPos, tmp, readLen);
                                    hBufPos += readLen;
                                }
                                delP(&tmp);
                            }
                            else
                            {
                                //the next chuunk data is the payload
                                if (!response.noContent)
                                {
                                    pChunkIdx++;
                                    pChunk = (char *)newP(chunkBufSize + 1);
                                    if (response.isChunkedEnc)
                                        delay(10);
                                    //read the avilable data
                                    //chunk transfer encoding?
                                    if (response.isChunkedEnc)
                                        availablePayload = readChunkedData(client, pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                                    else
                                        availablePayload = readLine(client, pChunk, chunkBufSize);

                                    if (availablePayload > 0)
                                    {
                                        payloadRead += availablePayload;
                                        buf += pChunk;
                                    }

                                    delP(&pChunk);

                                    if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                                    {
                                        while (client->available() > 0)
                                            client->read();
                                        break;
                                    }
                                }
                                else
                                {
                                    //read all the rest data
                                    while (client->available() > 0)
                                        client->read();
                                    break;
                                }
                            }
                        }
                        chunkIdx++;
                        if (millis() - dataTime > 5000)
                            return ret;
                    }
                }

                if (hstate == 1)
                    delP(&header);

                if (payloadRead > 0)
                    ret = 200;
            }
            else
            {
                while (client->available() > 0)
                    client->read();
            }
        }

        ret = response.httpCode;

        return ret;
    }

    void clearSerialData(struct FB_JS::serial_data_t &data)
    {
        clearS(data.buf);
        data.start = -1;
        data.end = -1;
        data.pos = -1;
        data.scnt = 0;
        data.ecnt = 0;
        data.dataTime = millis();
    }

    bool readStreamChar(int r, struct FB_JS::serial_data_t &data, MBSTRING &buf, bool isJson)
    {
        bool ret = false;
        if (r > -1)
        {
            data.pos++;

            if (isJson)
            {
                if ((char)r == '{')
                {
                    data.scnt++;
                    data.start = data.pos;
                }
                else if ((char)r == '}')
                {
                    data.ecnt++;
                    data.end = data.pos;
                }
            }
            else
            {
                if ((char)r == '[')
                {
                    data.scnt++;
                    data.start = data.pos;
                }
                else if ((char)r == ']')
                {
                    data.ecnt++;
                    data.end = data.pos;
                }
            }

            if (data.scnt > 0)
                data.buf += r;

            if (data.scnt == data.ecnt && data.scnt > 0)
            {
                bool ret = false;
                if (data.end > data.start)
                {
                    storeS(buf, data.buf.c_str(), false);
                    ret = true;
                }

                clearSerialData(data);
                return ret;
            }

            if (data.ecnt > data.scnt)
                clearSerialData(data);
        }

        return ret;
    }

    bool readStream(Stream *s, struct FB_JS::serial_data_t &data, MBSTRING &buf, bool isJson, int timeoutMS)
    {

        bool ret = false;

        if (timeoutMS > -1)
        {
            if (millis() - data.dataTime > (unsigned long)timeoutMS)
                clearSerialData(data);
        }
        else
            clearSerialData(data);

        while (s->available())
        {
            idle();
            int r = s->read();
            ret = readStreamChar(r, data, buf, isJson);
            if (ret)
            {
                if (timeoutMS == -1)
                    clearSerialData(data);
                return true;
            }
        }

        return ret;
    }

    Stream *toStream(HardwareSerial *ser)
    {
        return reinterpret_cast<Stream *>(ser);
    }
#ifdef FB_JS_INCLUDE_SW_SERIAL
    Stream *toStream(SoftwareSerial *ser)
    {
        return reinterpret_cast<Stream *>(ser);
    }
#endif
#if defined(FBJS_ENABLE_FS)
    Stream *toStream(File *file)
    {
        return reinterpret_cast<Stream *>(file);
    }
#endif

#ifdef FB_JS_INCLUDE_WIFI_CLIENT
    Client *toClient(WiFiClient *client)
    {
        return reinterpret_cast<Client *>(client);
    }
#endif

#ifdef FB_JS_INCLUDE_WIFI_CLIENT_SECURE
    Client *toClient(WiFiClientSecure *client)
    {
        return reinterpret_cast<Client *>(client);
    }
#endif

    void ltrim(MBSTRING &str, const MBSTRING &chars = " ")
    {
        size_t pos = str.find_first_not_of(chars);
        if (pos != MBSTRING::npos)
            str.erase(0, pos);
    }

    void rtrim(MBSTRING &str, const MBSTRING &chars = " ")
    {
        size_t pos = str.find_last_not_of(chars);
        if (pos != MBSTRING::npos)
            str.erase(pos + 1);
    }

    void trim(MBSTRING &str, const MBSTRING &chars = " ")
    {
        ltrim(str, chars);
        rtrim(str, chars);
    }

    bool isArrayKey(int &keyIndex, std::vector<MBSTRING> &keys)
    {
        if (keyIndex < (int)keys.size())
            return keys[keyIndex][0] == '[' && keys[keyIndex][keys[keyIndex].length() - 1] == ']';
        else
            return false;
    }

    bool isArrayKey(const char *key)
    {
        if (strlen(key) > 0)
            return key[0] == '[' && key[strlen(key) - 1] == ']';
        else
            return false;
    }

    int getArrIndex(int &keyIndex, std::vector<MBSTRING> &keys)
    {
        int res = -1;
        if (keyIndex < (int)keys.size())
        {
            res = atoi(keys[keyIndex].substr(1, keys[keyIndex].length() - 2).c_str());
            if (res < 0)
                res = 0;
        }
        return res;
    }

    int getArrIndex(const char *key)
    {
        MBSTRING s = key;
        int res = -1;
        res = atoi(s.substr(1, s.length() - 2).c_str());
        if (res < 0)
            res = 0;
        return res;
    }
};

class FirebaseJsonArray : public FirebaseJsonBase
{

    friend class FirebaseJson;
    friend class FirebaseJsonData;

public:
    typedef struct FirebaseJsonBase::fb_js_iterator_value_t IteratorValue;
    typedef struct FirebaseJsonBase::fb_js_search_criteria_t SearchCriteria;

    FirebaseJsonArray()
    {
        this->root_type = Root_Type_JSONArray;
    }

    template <typename T>
    FirebaseJsonArray(T data)
    {
        this->root_type = Root_Type_JSONArray;
        setRaw(getStr(data));
    }

    FirebaseJsonArray &operator=(FirebaseJsonArray other);
    FirebaseJsonArray(FirebaseJsonArray &other);
    ~FirebaseJsonArray();

    /**
     * Set or deserialize the JSON array data (JSON array literal) as FirebaseJsonArray object.
     * 
     * @param data The JSON array literal string to set or deserialize.
     * @return boolean status of the operation.
     * 
     * Call FirebaseJsonArray.errorPosition to get the error.
    */
    template <typename T>
    bool setJsonArrayData(T data) { return setRaw(getStr(data)); }

    /**
     * Add null to FirebaseJsonArray object.
     * 
     * @return instance of an object.
    */
    FirebaseJsonArray &add() { return nAdd(MB_JSON_CreateNull()); }

    /**
     * Add value to FirebaseJsonArray object.
     * 
     * @param value The value to add.
     * @return instance of an object.
     * 
     * The value that can be added is the following supported types e.g. flash string (PROGMEM and FPSTR/PSTR),
     * String, C/C++ std::string, const char*, char array, string literal, all integer and floating point numbers, 
     * boolean, FirebaseJson object and array.
    */
    template <typename T>
    FirebaseJsonArray &add(T value) { return dataAddHandler(value); }

    FirebaseJsonArray &add(FirebaseJson &value);

    FirebaseJsonArray &add(FirebaseJsonArray &value);

    /**
     * Add multiple values to FirebaseJsonArray object.
     * e.g. add("a","b",1,2)
     * 
     * @param v The value of any type to add.
     * @param n The consecutive values of any type to add.
     * @return instance of an object.
    */
    template <typename First, typename... Next>
    FirebaseJsonArray &add(First v, Next... n)
    {
        dataAddHandler(v);
        return add(n...);
    }

    /**
     * Set JSON array data (Client response) to FirebaseJsonArray object.
     * 
     * @param client The pointer to or instance of Client class.
     * @return boolean status of the operation.
     * 
    */
    bool readFrom(Client *client) { return mReadClient(client); }

    bool readFrom(Client &client) { return mReadClient(&client); }

    /**
     * Set JSON array data (WiFiClient response) to FirebaseJsonArray object.
     * 
     * @param client The pointer to or instance of WiFiClient object.
     * @return boolean status of the operation.
     * 
    */
#if defined(FB_JS_INCLUDE_WIFI_CLIENT)
    bool readFrom(WiFiClient *client)
    {
        return mReadClient(toClient(client));
    }

    bool readFrom(WiFiClient &client) { return mReadClient(&client); }
#endif

    /**
     * Set JSON array data (WiFiClientSecure response) to FirebaseJsonArray object.
     * 
     * @param client The pointer to or instance of WiFiClientSecure object.
     * @return boolean status of the operation.
    */
#if defined(FB_JS_INCLUDE_WIFI_CLIENT_SECURE)
    bool readFrom(WiFiClientSecure *client)
    {
        return mReadClient(toClient(client));
    }

    bool readFrom(WiFiClientSecure &client) { return mReadClient(&client); }
#endif

    /**
     * Set JSON array data (Seral object) to FirebaseJsonArray object.
     * 
     * @param ser The HW or SW Serial object.
     * @param timeoutMS The timeout in millisecond to wait for Serial data to be completed.
     * @return boolean status of the operation.
    */
    bool readFrom(HardwareSerial &ser, uint32_t timeoutMS = 5000) { return mReadStream(toStream(&ser), (int)timeoutMS); }
#ifdef FB_JS_INCLUDE_SW_SERIAL
    bool readFrom(SoftwareSerial &ser, uint32_t timeoutMS = 5000)
    {
        return mReadStream(toStream(&ser), (int)timeoutMS);
    }
#endif

#if defined(FBJS_ENABLE_FS)
    /**
     * Set JSON array data (File object) to FirebaseJsonArray object.
     * 
     * @param file The File object.
     * @return boolean status of the operation.
    */
    bool readFrom(FILE_SYSTEM &file) { return mReadStream(toStream(&file), -1); }
#endif

    /**
     * Get the array value at the specified index or path from the FirebaseJsonArray object.
     * 
     * @param result The reference of FirebaseJsonData object that holds data at the specified index.
     * @param index_or_path Index of data or relative path to data in FirebaseJsonArray object.
     *  @param prettify The text indentation and new line serialization option.
     * @return boolean status of the operation.
     * 
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2
    */
    template <typename T>
    bool get(FirebaseJsonData &result, T index_or_path, bool prettify = false) { return dataGetHandler(index_or_path, result, prettify); }

    /**
     * Search element by key or path in FirebaseJsonArray object.
     * 
     * @param result The reference of FirebaseJsonData that holds the result.
     * @param criteria The FirebaseJson::SearchCriteria data.
     * @param prettify The text indentation and new line serialization option.
     * @return number of elements found from search.
     * 
     * The SearchCriteria data has the properties e.g.
     * path - The key of path to search.
     * Path can be wildcard with * in search path and * should use as key in part and do not mix with any character.
     * value - The value string to search.
     * depth - The begin depth (int) of element to search, default is 0.
     * endDepth - The end depth (int) of element to search, default is -1.
     * searchAll - The boolean option to search all occurrences of elements.
     *  
    */
    size_t search(SearchCriteria &criteria) { return mSearch(root, &criteria); }

    size_t search(FirebaseJsonData &result, SearchCriteria &criteria, bool prettify = false) { return mSearch(root, &result, &criteria, prettify); }

    /**
     * Search element by key or path in FirebaseJsonArray object.
     * 
     * @param path The key or path to search.
     * @param searchAll Search all occurrences.
     * @return number of elements found from search.
     *  
    */
    template <typename T>
    size_t search(T path, bool searchAll = false) { return mSearch(root, getStr(path), searchAll); }

    /**
     * Get the full path to any element in FirebaseJson object.
     * 
     * @param path The key or path to search in to.
     * @param searchAll Search all occurrences.
     * @return full path string in case of found.
     *  
    */
    template <typename T>
    String getPath(T path, bool searchAll = false) { return mGetElementFullPath(root, getStr(path), searchAll); }

    /**
     * Check whether key or path to the child element existed in FirebaseJsonArray or not.
     * 
     * @param path The key or path of child element check.
     * @return boolean status indicated the existence of element.
     *  
    */
    template <typename T>
    bool isMember(T path) { return mGet(root, NULL, getStr(path)); }

    /**
     * Parse and collect all node/array elements in FirebaseJsonArray object.
     * @return number of child/array elements in FirebaseJson object.
    */
    size_t iteratorBegin(const char *data = NULL) { return mIteratorBegin(root); }

    /**
     * Get child/array elements from FirebaseJsonArray objects at specified index.
     * 
     * @param index The element index to get.
     * @param type The integer which holds the type of data i.e. JSON_OBJECT and JSON_ARR
     * @param key The string which holds the key/key of an object, can return empty String if the data type is an array.
     * @param value The string which holds the value for the element key or array.
     * @return depth of element.
    */
    int iteratorGet(size_t index, int &type, String &key, String &value) { return mIteratorGet(index, type, key, value); }

    /**
     * Get child/array elements from FirebaseJsonArray objects at specified index.
     * 
     * @param index The element index to get.
     * @return IteratorValue struct.
     * 
     * This should call after iteratorBegin.
     * 
     * The IteratorValue struct contains the following members.
     * int type
     * String key
     * String value
    */
    IteratorValue valueAt(size_t index) { return mValueAt(index); }

    /**
     * Clear all iterator buffer (should be called since iteratorBegin was called).
    */
    void iteratorEnd() { mIteratorEnd(); }

    /**
     * Get the length of the array in FirebaseJsonArray object.
     * @return length of the array.
    */
    size_t size() { return MB_JSON_GetArraySize(root); }

    /**
     * Get the FirebaseJsonArray object serialized string.
     * 
     * @param out The object e.g. Serial, String, std::string, char array, Stream, File, Client, that accepts the returning string.
     * @param prettify The text indentation and new line serialization option.
    */
    template <typename T>
    bool toString(T &out, bool prettify = false) { return toStringHandler(out, prettify); }

    template <typename T>
    bool toString(T *ptr, bool prettify = false) { return toStringPtrHandler(ptr, prettify); }

    /**
     * Get raw JSON Array
     * @return raw JSON Array string
    */
    const char *raw() { return mRaw(); }

    /**
     * Get the size of serialized JSON array buffer
     * @param prettify The text indentation and new line serialization option.
     * @return size in byte of buffer 
    */
    size_t serializedBufferLength(bool prettify = false) { return mGetSerializedBufferLength(prettify); }

    /**
     * Clear all array in FirebaseJsonArray object.
     * 
     * @return instance of an object.
    */
    FirebaseJsonArray &clear();

    /**
     * Set null to FirebaseJsonArray object at specified index or path.
     * 
     * @param index_or_path The array index or path that null to be set.
    */
    template <typename T>
    void set(T index_or_path) { dataSetHandler(index_or_path, nullptr); }

    /**
     * Set String to FirebaseJsonArray object at the specified index.
     * 
     * @param index_or_path The array index or path that value to be set.
     * @param value The String to set.
    */
    template <typename T1, typename T2>
    void set(T1 index_or_path, T2 value) { dataSetHandler(index_or_path, value); }
    template <typename T>
    void set(T index_or_path, FirebaseJson &value) { return dataSetHandler(index_or_path, value); }
    template <typename T>
    void set(T index_or_path, FirebaseJsonArray &value) { return dataSetHandler(index_or_path, value); }

    /**
     * Remove the array value at the specified index or path from the FirebaseJsonArray object.
     * 
     * @param index_or_path The array index or relative path to array to be removed.
     * @return bool value represents the successful operation.
     * 
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would remove the data of myData key inside the array indexes 2.
    */
    template <typename T1>
    bool remove(T1 index_or_path) { return dataRemoveHandler(index_or_path); }

    /**
     * Get the error position at the JSON object literal from parsing.
     * @return the position of error in JSON object literal
     * Return -1 when for no parsing error.
    */
    int errorPosition() { return errorPos; }

    /**
     * Set the precision for float to JSON Array object
    */
    void setFloatDigits(uint8_t digits) { mSetFloatDigits(digits); }

    /**
     * Set the precision for double to JSON Array object
    */
    void setDoubleDigits(uint8_t digits) { mSetDoubleDigits(digits); }

    /**
     * Get http response code of reading JSON data from WiFi/Ethernet Client.
     * @return the response code of reading JSON data from WiFi/Ethernet Client
    */
    int responseCode() { return mResponseCode(); }

private:
    FirebaseJsonArray &nAdd(MB_JSON *value);
    bool mSetIdx(int index, MB_JSON *value);
    bool mGetIdx(FirebaseJsonData *result, int index, bool prettify);
    bool mRemoveIdx(int index);

    template <typename T>
    auto dataGetHandler(T arg, FirebaseJsonData &result, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_string<T>::value, bool>::type
    {
        return mGet(root, &result, getStr(arg), prettify);
    }

    template <typename T>
    auto dataGetHandler(T arg, FirebaseJsonData &result, bool prettify) -> typename FB_JS::enable_if<FB_JS::is_num_int<T>::value, bool>::type
    {
        return mGetIdx(&result, arg, prettify);
    }

    template <typename T>
    auto dataRemoveHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_string<T>::value, bool>::type
    {
        return mRemove(getStr(arg));
    }

    template <typename T>
    auto dataRemoveHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_num_int<T>::value, bool>::type
    {
        return mRemoveIdx(arg);
    }

    template <typename T>
    auto dataAddHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_bool<T>::value, FirebaseJsonArray &>::type
    {
        nAdd(MB_JSON_CreateBool(arg));
        return *this;
    }

    template <typename T>
    auto dataAddHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_num_int<T>::value, FirebaseJsonArray &>::type
    {
        nAdd(MB_JSON_CreateRaw(NUM2S(arg).get()));
        return *this;
    }

    template <typename T>
    auto dataAddHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_same<T, float>::value, FirebaseJsonArray &>::type
    {
        nAdd(MB_JSON_CreateRaw(NUM2S(arg, floatDigits).get()));
        return *this;
    }

    template <typename T>
    auto dataAddHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_same<T, double>::value, FirebaseJsonArray &>::type
    {
        nAdd(MB_JSON_CreateRaw(NUM2S(arg, doubleDigits).get()));
        return *this;
    }

    template <typename T>
    auto dataAddHandler(T arg) -> typename FB_JS::enable_if<FB_JS::is_string<T>::value, FirebaseJsonArray &>::type
    {
        nAdd(MB_JSON_CreateString(getStr(arg)));
        return *this;
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, std::nullptr_t>::value>::type
    {
        mSet(getStr(arg1), MB_JSON_CreateNull());
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_same<T2, std::nullptr_t>::value>::type
    {
        mSetIdx(arg1, MB_JSON_CreateNull);
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_bool<T2>::value>::type
    {
        mSet(getStr(arg1), MB_JSON_CreateBool(arg2));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_bool<T2>::value>::type
    {
        mSetIdx(arg1, MB_JSON_CreateBool(arg2));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_num_int<T2>::value>::type
    {
        mSet(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2).get()));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_num_int<T2>::value>::type
    {
        mSetIdx(arg1, MB_JSON_CreateRaw(NUM2S(arg2).get()));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, float>::value>::type
    {
        mSet(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2, floatDigits).get()));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_same<T2, float>::value>::type
    {
        mSetIdx(arg1, MB_JSON_CreateRaw(NUM2S(arg2, floatDigits).get()));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, double>::value>::type
    {
        mSet(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2, doubleDigits).get()));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_same<T2, double>::value>::type
    {
        mSetIdx(arg1, MB_JSON_CreateRaw(NUM2S(arg2, doubleDigits).get()));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_string<T2>::value>::type
    {
        mSet(getStr(arg1), MB_JSON_CreateString(getStr(arg2)));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_string<T2>::value>::type
    {
        mSetIdx(arg1, MB_JSON_CreateString(getStr(arg2)));
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 &arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, FirebaseJson>::value>::type
    {
        MB_JSON *e = MB_JSON_Duplicate(arg2.root, true);
        mSet(getStr(arg1), e);
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 &arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_same<T2, FirebaseJson>::value>::type
    {
        MB_JSON *e = MB_JSON_Duplicate(arg2.root, true);
        mSetIdx(arg1, e);
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 &arg2) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, FirebaseJsonArray>::value>::type
    {
        MB_JSON *e = MB_JSON_Duplicate(arg2.root, true);
        mSet(getStr(arg1), e);
    }

    template <typename T1, typename T2>
    auto dataSetHandler(T1 arg1, T2 &arg2) -> typename FB_JS::enable_if<FB_JS::is_num_int<T1>::value && FB_JS::is_same<T2, FirebaseJsonArray>::value>::type
    {
        MB_JSON *e = MB_JSON_Duplicate(arg2.root, true);
        mSetIdx(arg1, e);
    }
};

class FirebaseJson : public FirebaseJsonBase
{
    friend class FirebaseJsonArray;
    friend class FirebaseJsonData;

public:
    typedef enum FirebaseJsonBase::fb_js_json_data_type jsonDataType;
    typedef struct FirebaseJsonBase::fb_js_iterator_value_t IteratorValue;
    typedef struct FirebaseJsonBase::fb_js_search_criteria_t SearchCriteria;

    FirebaseJson() { this->root_type = Root_Type_JSON; }

    template <typename T>
    FirebaseJson(T data)
    {
        this->root_type = Root_Type_JSON;
        setRaw(getStr(data));
    }

    FirebaseJson &operator=(FirebaseJson other);

    FirebaseJson(FirebaseJson &other);

    ~FirebaseJson();

    /**
     * Clear internal buffer of FirebaseJson object.
     * 
     * @return instance of an object.
    */
    FirebaseJson &clear();

    /**
     * Set or deserialize the JSON array data (JSON object literal) as FirebaseJson object.
     * 
     * @param data The JSON object literal string to set or deserialize.
     * @return boolean status of the operation.
     * 
     * Call FirebaseJson.errorPosition to get the error.
    */
    template <typename T>
    bool setJsonData(T data) { return setRaw(getStr(data)); }

    /**
     * Set JSON data (Client response) to FirebaseJson object.
     * 
     * @param client The pointer to or instance of Client object.
     * @return boolean status of the operation.
   */
    bool readFrom(Client *client) { return mReadClient(client); }

    bool readFrom(Client &client) { return mReadClient(&client); }

    /**
     * Set JSON data (WiFiClient response) to FirebaseJson object.
     * 
     * @param client The pointer to or instance of WiFiClient object.
     * @return boolean status of the operation.
    */
#if defined(FB_JS_INCLUDE_WIFI_CLIENT)
    bool readFrom(WiFiClient *client)
    {
        return mReadClient(toClient(client));
    }

    bool readFrom(WiFiClient &client) { return mReadClient(&client); }
#endif

    /**
     * Set JSON data (WiFiClientSecure response) to FirebaseJson object.
     * 
     * @param client The pointer to or instance of WiFiClientSecure object.
     * @return boolean status of the operation.
    */
#if defined(FB_JS_INCLUDE_WIFI_CLIENT_SECURE)
    bool readFrom(WiFiClientSecure *client)
    {
        return mReadClient(toClient(client));
    }

    bool readFrom(WiFiClientSecure &client) { return mReadClient(&client); }
#endif

    /**
     * Set JSON array data (Seral object) to FirebaseJson object.
     * 
     * @param ser The HW or SW Serial object.
     * @param timeoutMS The timeout in millisecond to wait for Serial data to be completed.
     * @return boolean status of the operation.
    */
    bool readFrom(HardwareSerial &ser, uint32_t timeoutMS = 5000) { return mReadStream(toStream(&ser), (int)timeoutMS); }
#ifdef FB_JS_INCLUDE_SW_SERIAL
    bool readFrom(SoftwareSerial &ser, uint32_t timeoutMS = 5000)
    {
        return mReadStream(toStream(&ser), (int)timeoutMS);
    }
#endif

#if defined(FBJS_ENABLE_FS)
    /**
     * Set JSON array data (File object) to FirebaseJson object.
     * 
     * @param file The File object.
     * @return boolean status of the operation.
    */
    bool readFrom(FILE_SYSTEM &file) { return mReadStream(toStream(&file), -1); }
#endif

    /**
     * Add null to FirebaseJson object.
     * 
     * @param key The new key string that null to be added.
     * @return instance of an object.
    */
    template <typename T>
    FirebaseJson &add(T key) { return nAdd(getStr(key), NULL); }

    /**
     * Add value to FirebaseJson object.
     * 
     * @param key The new key string that string value to be added.
     * @param value The value for the new specified key.
     * 
     * @return instance of an object.
    */
    template <typename T1, typename T2>
    FirebaseJson &add(T1 key, T2 value) { return dataHandler(key, value, fb_json_func_type_add); }
    template <typename T>
    FirebaseJson &add(T key, FirebaseJson &value) { return dataHandler(key, value, fb_json_func_type_add); }
    template <typename T>
    FirebaseJson &add(T key, FirebaseJsonArray &value) { return dataHandler(key, value, fb_json_func_type_add); }

    /**
     * Get the FirebaseJson object serialized string.
     * 
     * @param out The writable object e.g. String, std::string, char array, Stream e.g ile, WiFi/Ethernet Client and LWMQTT, that accepts the returning string.
     * @param topic The MQTT topic (LWMQTT).
     * @param prettify The text indentation and new line serialization option.
    */
    template <typename T>
    bool toString(T &out, bool prettify = false) { return toStringHandler(out, prettify); }

    template <typename T1, typename T2>
    auto toString(T1 &out, T2 topic) -> typename FB_JS::enable_if<FB_JS::is_string<T2>::value, bool>::type { return toStringHandler(out, getStr(topic)); }

    template <typename T>
    bool toString(T *ptr, bool prettify = false) { return toStringPtrHandler(ptr, prettify); }

    template <typename T1, typename T2>
    bool toString(T1 *out, T2 topic) { return toStringHandler(out, getStr(topic)); }

    /**
     * Get the value from the specified node path in FirebaseJson object.
     * 
     * @param result The reference of FirebaseJsonData that holds the result.
     * @param path Relative path to the specific node in FirebaseJson object.
     * @param prettify The text indentation and new line serialization option.
     * @return boolean status of the operation.
     * 
     * The FirebaseJsonData object holds the returned data which can be read from the following properties.
     * result.stringValue - contains the returned string.
     * result.intValue - contains the returned signed 32-bit integer value.
     * result.floatValue - contains the returned float value.
     * result.doubleValue - contains the returned double value.
     * result.boolValue - contains the returned boolean value.
     * result.success - used to determine the result of the get operation.
     * result.type - used to determine the type of returned value in string represent
     * the types of value e.g. string, int, double, boolean, array, object, null and undefined.
     * 
     * result.typeNum used to determine the type of returned value is an integer as represented by the following value.
     * FirebaseJson::UNDEFINED = 0
     * FirebaseJson::OBJECT = 1
     * FirebaseJson::ARRAY = 2
     * FirebaseJson::STRING = 3
     * FirebaseJson::INT = 4
     * FirebaseJson::FLOAT = 5
     * FirebaseJson::DOUBLE = 6
     * FirebaseJson::BOOL = 7 and
     * FirebaseJson::NULL = 8
    */
    template <typename T>
    bool get(FirebaseJsonData &result, T path, bool prettify = false) { return mGet(root, &result, getStr(path), prettify); }

    /**
     * Search element by key or path in FirebaseJsonArray object.
     * 
     * @param result The reference of FirebaseJsonData that holds the result.
     * @param criteria The FirebaseJson::SearchCriteria data.
     * @param prettify The text indentation and new line serialization option.
     * @return number of elements found from search.
     * 
     * The SearchCriteria data has the properties e.g.
     * path - The key of path to search.
     * Path can be wildcard with * in search path and * should use as key in part and do not mix with any character.
     * value - The value string to search.
     * depth - The begin depth (int) of element to search, default is 0.
     * endDepth - The end depth (int) of element to search, default is -1.
     * searchAll - The boolean option to search all occurrences of elements.
     *  
    */
    size_t search(SearchCriteria &criteria) { return mSearch(root, &criteria); }

    size_t search(FirebaseJsonData &result, SearchCriteria &criteria, bool prettify = false) { return mSearch(root, &result, &criteria, prettify); }

    /**
     * Search element by key or path in FirebaseJson object.
     * 
     * @param path The key or path to search.
     * @param searchAll Search all occurrences.
     * @return number of elements found from search.
     *  
    */
    template <typename T>
    size_t search(T path, bool searchAll = false) { return mSearch(root, getStr(path), searchAll); }

    /**
     * Get the full path to any element in FirebaseJson object.
     * 
     * @param path The key or path to search in to.
     * @param searchAll Search all occurrences.
     * @return full path string in case of found.
     *  
    */
    template <typename T>
    String getPath(T path, bool searchAll = false) { return mGetElementFullPath(root, getStr(path), searchAll); }

    /**
     * Check whether key or path to the child element existed in FirebaseJson object or not.
     * 
     * @param path The key or path of child element check.
     * @return boolean status indicated the existence of element.
     *  
    */
    template <typename T>
    bool isMember(T path) { return mGet(root, NULL, getStr(path)); }

    /**
     * Parse and collect all node/array elements in FirebaseJson object.
     * 
     * @return number of child/array elements in FirebaseJson object.
    */
    size_t iteratorBegin() { return mIteratorBegin(root); }

    /**
     * Get child/array elements from FirebaseJson objects at specified index.
     * 
     * @param index The element index to get.
     * @param type The integer which holds the type of data i.e. JSON_OBJECT and JSON_ARR
     * @param key The string which holds the key/key of an object, can return empty String if the data type is an array.
     * @param value The string which holds the value for the element key or array.
     * @return depth of element.
    */
    int iteratorGet(size_t index, int &type, String &key, String &value) { return mIteratorGet(index, type, key, value); }

    /**
     * Get child/array elements from FirebaseJson objects at specified index.
     * 
     * @param index The element index to get.
     * @return IteratorValue struct.
     * 
     * This should call after iteratorBegin.
     * 
     * The IteratorValue struct contains the following members.
     * int type
     * String key
     * String value
    */
    IteratorValue valueAt(size_t index) { return mValueAt(index); }

    /**
     * Clear all iterator buffer (should be called since iteratorBegin was called).
    */
    void iteratorEnd() { mIteratorEnd(); }

    /**
     * Set null to FirebaseJson object at the specified node path.
     * 
     * @param path The relative path that null to be set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
    template <typename T>
    void set(T key) { mSet(getStr(key), NULL); }

    /**
     * Set value to FirebaseJson object at the specified node path.
     * 
     * @param path The relative path that string value to be set.
     * @param value The value to set.
     * 
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
     * 
     * The value that can be added is the following supported types e.g. flash string (PROGMEM and FPSTR/PSTR), 
     * String, C/C++ std::string, const char*, char array, string literal, all integer and floating point numbers,
     * boolean, FirebaseJson object and array.
    */
    template <typename T1, typename T2>
    void set(T1 key, T2 value) { dataHandler(key, value, fb_json_func_type_set); }
    template <typename T>
    FirebaseJson &set(T key, FirebaseJson &value) { return dataHandler(key, value, fb_json_func_type_set); }
    template <typename T>
    FirebaseJson &set(T key, FirebaseJsonArray &value) { return dataHandler(key, value, fb_json_func_type_set); }

    /**
     * Remove the specified node and its content.
     * 
     * @param path The relative path to remove its contents/children.
     * @return bool value represents the success operation.
    */
    template <typename T>
    bool remove(T path) { return mRemove(getStr(path)); }

    /**
     * Get raw JSON
     * @return raw JSON string
    */
    const char *raw() { return mRaw(); }

    /**
     * Get the error position at the JSON object literal from parsing.
     * @return the position of error in JSON object literal
     * Return -1 when for no parsing error
    */
    int errorPosition() { return errorPos; }

    /**
     * Get the size of serialized JSON object buffer
     * @param prettify The text indentation and new line serialization option.
     * @return size in byte of buffer 
    */
    size_t serializedBufferLength(bool prettify = false) { return mGetSerializedBufferLength(prettify); }

    /**
     * Set the precision for float to JSON object
     * @param digits The number of decimal places.
    */
    void setFloatDigits(uint8_t digits) { mSetFloatDigits(digits); }

    /**
     * Set the precision for double to JSON object
    */
    void setDoubleDigits(uint8_t digits) { mSetDoubleDigits(digits); }

    /**
     * Get http response code of reading JSON data from WiFi/Ethernet Client.
     * @return the response code of reading JSON data from WiFi/Ethernet Client
    */
    int responseCode() { return mResponseCode(); }

private:
    FirebaseJson &nAdd(const char *key, MB_JSON *value);

    template <typename T1, typename T2>
    auto dataHandler(T1 arg1, T2 arg2, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_bool<T2>::value, FirebaseJson &>::type
    {
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg1), MB_JSON_CreateBool(arg2));
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg1), MB_JSON_CreateBool(arg2));
        return *this;
    }

    template <typename T1, typename T2>
    auto dataHandler(T1 arg1, T2 arg2, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_num_int<T2>::value, FirebaseJson &>::type
    {
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2).get()));
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2).get()));
        return *this;
    }

    template <typename T1, typename T2>
    auto dataHandler(T1 arg1, T2 arg2, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, float>::value, FirebaseJson &>::type
    {
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2, floatDigits).get()));
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2, floatDigits).get()));
        return *this;
    }

    template <typename T1, typename T2>
    auto dataHandler(T1 arg1, T2 arg2, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_same<T2, double>::value, FirebaseJson &>::type
    {
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2, doubleDigits).get()));
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg1), MB_JSON_CreateRaw(NUM2S(arg2, doubleDigits).get()));
        return *this;
    }

    template <typename T1, typename T2>
    auto dataHandler(T1 arg1, T2 arg2, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T1>::value && FB_JS::is_string<T2>::value, FirebaseJson &>::type
    {
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg1), MB_JSON_CreateString(getStr(arg2)));
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg1), MB_JSON_CreateString(getStr(arg2)));
        return *this;
    }

    template <typename T>
    auto dataHandler(T arg, FirebaseJson &json, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T>::value, FirebaseJson &>::type
    {
        MB_JSON *e = MB_JSON_Duplicate(json.root, true);
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg), e);
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg), e);
        return *this;
    }

    template <typename T>
    auto dataHandler(T arg, FirebaseJsonArray &arr, fb_json_func_type_t type) -> typename FB_JS::enable_if<FB_JS::is_string<T>::value, FirebaseJson &>::type
    {
        MB_JSON *e = MB_JSON_Duplicate(arr.root, true);
        if (type == fb_json_func_type_add)
            nAdd(getStr(arg), e);
        else if (type == fb_json_func_type_set)
            mSet(getStr(arg), e);
        return *this;
    }
};

#endif