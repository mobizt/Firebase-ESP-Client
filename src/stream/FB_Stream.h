/**
 * Google's Firebase Stream class, FB_Stream.h version 1.0.3
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created June 10, 2021
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

#if defined(FIREBASE_ESP_CLIENT)
#define FIREBASE_STREAM_CLASS FirebaseStream
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#define FIREBASE_STREAM_CLASS StreamData
#endif

class FIREBASE_STREAM_CLASS
{

    friend class FirebaseData;
    friend class FB_RTDB;

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

    /** Return the Firebase JSON Data object pointer that keeps the get(parse) result.
     * 
     * @return FirebaseJsonData object pointer.
    */
    FirebaseJsonData *jsonDataPtr();

    /** Return the Firebase JSON Data object that keeps the get(parse) result.
     * 
     * @return FirebaseJsonData object.
    */
    FirebaseJsonData &jsonData();

    /** Return the blob data (uint8_t) array of server returned payload.
     * 
     * @return Dynamic array of 8-bit unsigned integer i.e. std::vector<uint8_t>.
    */
    std::vector<uint8_t> blobData();

    /** Return the file stream of server returned payload.
     * 
     * @return the file stream.
    */
    File fileStream();

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

    /** Clear or empty data in FirebaseStream or StreamData object.
    */
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