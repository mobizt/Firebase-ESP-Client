#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Stream class, FB_Stream.h version 1.1.7
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

#ifndef FIREBASE_STREAM_SESSION_H
#define FIREBASE_STREAM_SESSION_H
#include <Arduino.h>
#include "FB_Utils.h"
#include "signer/Signer.h"

#if defined(FIREBASE_ESP_CLIENT)
#define FIREBASE_STREAM_CLASS FirebaseStream
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#define FIREBASE_STREAM_CLASS StreamData
#endif

using namespace mb_string;

class FIREBASE_STREAM_CLASS
{

    friend class FirebaseData;
    friend class FB_RTDB;
    friend class FIREBASE_MP_STREAM_CLASS;

public:
    FIREBASE_STREAM_CLASS();
    ~FIREBASE_STREAM_CLASS();
    String dataPath();
    String streamPath();

    /** Return the integer data of server returned payload.
     *
     * @return integer value.
     */
    int intData();

    /** Return the float data of server returned payload.
     *
     * @return Float value.
     */
    float floatData();

    /** Return the double data of server returned payload.
     *
     * @return double value.
     */
    double doubleData();

    /** Return the Boolean data of server returned payload.
     *
     * @return Boolean value.
     */
    bool boolData();

    /** Return the String data of server returned payload.
     *
     * @return String (String object).
     */
    String stringData();

    /** Return the JSON String data of server returned payload.
     *
     * @return String (String object).
     */
    String jsonString();

    /** Return the Firebase JSON object pointer of server returned payload.
     *
     * @return FirebaseJson object pointer.
     */
    FirebaseJson *jsonObjectPtr();

    /** Return the Firebase JSON object of server returned payload.
     *
     * @return FirebaseJson object.
     */
    FirebaseJson &jsonObject();

    /** Return the Firebase JSON Array object pointer of server returned payload.
     *
     * @return FirebaseJsonArray object pointer.
     */
    FirebaseJsonArray *jsonArrayPtr();

    /** Return the Firebase JSON Array object of server returned payload.
     * @return FirebaseJsonArray object.
     */
    FirebaseJsonArray &jsonArray();

    /** Return the pointer to blob data (uint8_t) array of server returned payload.
     *
     * @return Dynamic array of 8-bit unsigned integer i.e. std::vector<uint8_t>.
     */
    MB_VECTOR<uint8_t> *blobData();

#if defined(ESP32) || defined(ESP8266)
#if defined(MBFS_FLASH_FS)
    /** Return the file stream of server returned payload.
     *
     * @return the file stream.
     */
    File fileStream();
#endif
#endif

    /** Get the data type of payload returned from the server.
     * @return The one of these data type e.g. integer, float, double, boolean, string, JSON and blob.
     */
    String dataType();

    /** Get the data type of payload returned from the server.
     *
     * @return The enumeration value of fb_esp_rtdb_data_type.
     *
     * fb_esp_rtdb_data_type_null or 1,
     * fb_esp_rtdb_data_type_integer or 2,
     * fb_esp_rtdb_data_type_float or 3,
     * fb_esp_rtdb_data_type_double or 4,
     * fb_esp_rtdb_data_type_boolean or 5,
     * fb_esp_rtdb_data_type_string or 6,
     * fb_esp_rtdb_data_type_json or 7,
     * fb_esp_rtdb_data_type_array or 8,
     * fb_esp_rtdb_data_type_blob or 9,
     * fb_esp_rtdb_data_type_file or 10
     */
    uint8_t dataTypeEnum();

    /** Return the event type string.
     *
     * @return the event type String object.
     */
    String eventType();

    /** Return the server's payload data.
     *
     * @return Payload string (String object).
     *
     * @note The returned String will be empty when the response data is File, BLOB, JSON and JSON Array objects.
     *
     * For File data type, call fileStream to get the file stream.
     *
     * For BLOB data type, call blobData to get the dynamic array of unsigned 8-bit data.
     *
     * For JSON object data type, call jsonObject and jsonObjectPtr to get the object and its pointer.
     *
     * For JSON Array data type, call jsonArray and jsonArrayPtr to get the object and its pointer.
     *
     */
    String payload();

    /** Get the HTTP payload length returned from the server.
     * @return integer number of payload length.
     */
    int payloadLength();

    /** Get the maximum size of HTTP payload length returned from the server.
     *
     * @return integer number of payload length.
     */
    int maxPayloadLength();

    /** Clear or empty data in FirebaseStream or StreamData object.
     */
    void empty();

    template <typename T>
    auto to() -> typename enable_if<is_num_int<T>::value || is_num_float<T>::value || is_bool<T>::value, T>::type
    {
        if (sif->data_type == fb_esp_data_type::d_string)
            setRaw(true); // if double quotes string, trim it.

        if (sif->data.length() > 0)
        {
            if (sif->data_type == fb_esp_data_type::d_boolean)
                mSetResBool(strcmp(sif->data.c_str(), num2Str(true, -1)) == 0);
            else if (sif->data_type == fb_esp_data_type::d_integer ||
                     sif->data_type == fb_esp_data_type::d_float ||
                     sif->data_type == fb_esp_data_type::d_double)
            {
                mSetResInt(sif->data.c_str());
                mSetResFloat(sif->data.c_str());
            }
        }

        if (is_bool<T>::value)
            return iVal.uint32 > 0;
        else if (is_num_int8<T>::value)
            return iVal.int8;
        else if (is_num_uint8<T>::value)
            return iVal.uint8;
        else if (is_num_int16<T>::value)
            return iVal.int16;
        else if (is_num_uint16<T>::value)
            return iVal.uint16;
        else if (is_num_int32<T>::value)
            return iVal.int32;
        else if (is_num_uint32<T>::value)
            return iVal.uint32;
        else if (is_num_int64<T>::value)
            return iVal.int64;
        else if (is_num_uint64<T>::value)
            return iVal.uint64;
        else if (is_same<T, float>::value)
            return fVal.f;
        else if (is_same<T, double>::value)
            return fVal.d;
        else
            return 0;
    }

    template <typename T>
    auto to() -> typename enable_if<is_const_chars<T>::value ||
                                        is_std_string<T>::value ||
                                        is_arduino_string<T>::value ||
                                        is_mb_string<T>::value,
                                    T>::type
    {

        if (sif->data_type == fb_esp_data_type::d_string)
            setRaw(true);

        return sif->data.c_str();
    }

    template <typename T>
    auto to() -> typename enable_if<is_same<T, FirebaseJson *>::value, FirebaseJson *>::type
    {
        if (!jsonPtr)
            jsonPtr = new FirebaseJson();

        if (sif->data_type == d_json)
        {
            Utils::idle();
            jsonPtr->clear();
            if (arrPtr)
                arrPtr->clear();
            jsonPtr->setJsonData(sif->data.c_str());
        }

        return jsonPtr;
    }

    template <typename T>
    auto to() -> typename enable_if<is_same<T, FirebaseJson>::value, FirebaseJson &>::type
    {
        return *to<FirebaseJson *>();
    }

    template <typename T>
    auto to() -> typename enable_if<is_same<T, FirebaseJsonArray *>::value, FirebaseJsonArray *>::type
    {
        if (!arrPtr)
            arrPtr = new FirebaseJsonArray();

        if (sif->data_type == d_array)
        {
            if (jsonPtr)
                jsonPtr->clear();
            arrPtr->clear();
            arrPtr->setJsonArrayData(sif->data.c_str());
        }

        return arrPtr;
    }

    template <typename T>
    auto to() -> typename enable_if<is_same<T, FirebaseJsonArray>::value, FirebaseJsonArray &>::type
    {
        return *to<FirebaseJsonArray *>();
    }

    template <typename T>
    auto to() -> typename enable_if<is_same<T, MB_VECTOR<uint8_t> *>::value, MB_VECTOR<uint8_t> *>::type
    {
        return sif->blob;
    }

#if defined(MBFS_FLASH_FS) && defined(ENABLE_RTDB)
    template <typename T>
    auto to() -> typename enable_if<is_same<T, fs::File>::value, fs::File>::type
    {
        if (sif->data_type == fb_esp_data_type::d_file && Signer.config)
        {
            int ret = Signer.mbfs->open(pgm2Str(fb_esp_rtdb_pgm_str_10 /* "/fb_bin_0.tmp" */),
                                        mbfs_type mem_storage_type_flash, mb_fs_open_mode_read);
            if (ret < 0)
                sif->httpCode = ret;
        }

        return Signer.mbfs->getFlashFile();
    }
#endif

    FirebaseJson *jsonPtr = nullptr;
    FirebaseJsonArray *arrPtr = nullptr;

private:
    struct fb_esp_stream_info_t *sif = nullptr;

    union IVal
    {
        uint64_t uint64;
        int64_t int64;
        uint32_t uint32;
        int32_t int32;
        int16_t int16;
        uint16_t uint16;
        int8_t int8;
        uint8_t uint8;
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

    void begin(struct fb_esp_stream_info_t *s);
    void mSetResInt(const char *value);
    void mSetResFloat(const char *value);
    void mSetResBool(bool value);
    void setRaw(bool trim);
};

#endif

#endif // ENABLE