#ifndef RTDB_HElPER_H
#define RTDB_HElPER_H

#pragma once

#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#include "FirebaseFS.h"

#ifdef ENABLE_RTDB

#ifndef RTDBHelper_H
#define RTDBHelper_H

#if defined(FIREBASE_ESP_CLIENT)
#include <Firebase_ESP_Client.h>
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#if defined(ESP32)
#include <FirebaseESP32.h>
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
#include <FirebaseESP8266.h>
#endif
#endif

void printResult(FirebaseData &data)
{
    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
        Serial.println(data.to<int>());
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_float)
        Serial.println(data.to<float>(), 5);
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_double)
#if defined(MB_ARDUINO_PICO)
        Serial.printf("%.9lf\n", data.to<double>());
#else
        printf("%.9lf\n", data.to<double>());
#endif
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean)
        Serial.println(data.to<bool>() == 1 ? (const char *)FPSTR("true") : (const char *)FPSTR("false"));
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string)
        Serial.println(data.to<String>());
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json)
    {
        FirebaseJson *json = data.to<FirebaseJson *>();
        // Print all object data
        Serial.println((const char *)FPSTR("Pretty printed JSON data:"));
        json->toString(Serial, true);
        Serial.println();
        Serial.println((const char *)FPSTR("Iterate JSON data:"));
        Serial.println();
        size_t len = json->iteratorBegin();
        FirebaseJson::IteratorValue value;
        for (size_t i = 0; i < len; i++)
        {
            value = json->valueAt(i);
            Serial_Printf((const char *)FPSTR("%d, Type: %s, Name: %s, Value: %s\n"), i, value.type == FirebaseJson::JSON_OBJECT ? (const char *)FPSTR("object") : (const char *)FPSTR("array"), value.key.c_str(), value.value.c_str());
        }
        json->iteratorEnd();
        json->clear();
    }
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_array)
    {
        // get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray *arr = data.to<FirebaseJsonArray *>();
        // Print all array values
        Serial.println((const char *)FPSTR("Pretty printed Array:"));
        arr->toString(Serial, true);
        Serial.println();
        Serial.println((const char *)FPSTR("Iterate array values:"));
        Serial.println();
        for (size_t i = 0; i < arr->size(); i++)
        {
            Serial.print(i);
            Serial.print((const char *)FPSTR(", Value: "));

            FirebaseJsonData result;
            // Get the result data from FirebaseJsonArray object
            arr->get(result, i);
            if (result.typeNum == FirebaseJson::JSON_BOOL)
                Serial.println(result.to<bool>() ? (const char *)FPSTR("true") : (const char *)FPSTR("false"));
            else if (result.typeNum == FirebaseJson::JSON_INT)
                Serial.println(result.to<int>());
            else if (result.typeNum == FirebaseJson::JSON_FLOAT)
                Serial.println(result.to<float>());
            else if (result.typeNum == FirebaseJson::JSON_DOUBLE)
#if defined(MB_ARDUINO_PICO)
                Serial.printf("%.9lf\n", result.to<double>());
#else
                printf("%.9lf\n", result.to<double>());
#endif
            else if (result.typeNum == FirebaseJson::JSON_STRING ||
                     result.typeNum == FirebaseJson::JSON_NULL ||
                     result.typeNum == FirebaseJson::JSON_OBJECT ||
                     result.typeNum == FirebaseJson::JSON_ARRAY)
                Serial.println(result.to<String>());
        }
        arr->clear();
    }
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_blob)
    {
        MB_VECTOR<uint8_t> *blob = data.to<MB_VECTOR<uint8_t> *>();
        for (size_t i = 0; i < blob->size(); i++)
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();
            if ((*blob)[i] < 16)
                Serial.print((const char *)FPSTR("0"));
            Serial.print((*blob)[i], HEX);
            Serial.print((const char *)FPSTR(" "));
        }
        Serial.println();
    }
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_file)
    {
#if defined(DEFAULT_FLASH_FS)
        File file = data.to<File>();
        int i = 0;
        while (file.available())
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();

            int v = file.read();

            if (v < 16)
                Serial.print((const char *)FPSTR("0"));

            Serial.print(v, HEX);
            Serial.print((const char *)FPSTR(" "));
            i++;
        }
        Serial.println();
        data.closeFile();
#endif
    }
    else
    {
        Serial.println(data.payload());
    }
}

void printResult(FIREBASE_STREAM_CLASS &data)
{

    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
        Serial.println(data.to<int>());
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_float)
        Serial.println(data.to<float>(), 5);
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_double)
#if defined(MB_ARDUINO_PICO)
        Serial.printf("%.9lf\n", data.to<double>());
#else
        printf("%.9lf\n", data.to<double>());
#endif
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean)
        Serial.println(data.to<bool>() == 1 ? (const char *)FPSTR("true") : (const char *)FPSTR("false"));
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string || data.dataTypeEnum() == fb_esp_rtdb_data_type_null)
        Serial.println(data.to<String>());
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json)
    {
        FirebaseJson *json = data.to<FirebaseJson *>();
        // Print all object data
        Serial.println((const char *)FPSTR("Pretty printed JSON data:"));
        json->toString(Serial, true);
        Serial.println();
        Serial.println((const char *)FPSTR("Iterate JSON data:"));
        Serial.println();
        size_t len = json->iteratorBegin();
        FirebaseJson::IteratorValue value;
        for (size_t i = 0; i < len; i++)
        {
            value = json->valueAt(i);
            Serial_Printf((const char *)FPSTR("%d, Type: %s, Name: %s, Value: %s\n"), i, value.type == FirebaseJson::JSON_OBJECT ? (const char *)FPSTR("object") : (const char *)FPSTR("array"), value.key.c_str(), value.value.c_str());
        }
        json->iteratorEnd();
        json->clear();
    }
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_array)
    {
        // get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray *arr = data.to<FirebaseJsonArray *>();
        // Print all array values
        Serial.println((const char *)FPSTR("Pretty printed Array:"));
        String arrStr;
        arr->toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println((const char *)FPSTR("Iterate array values:"));
        Serial.println();

        for (size_t i = 0; i < arr->size(); i++)
        {
            Serial.print(i);
            Serial.print((const char *)FPSTR(", Value: "));
            FirebaseJsonData result;
            // Get the result data from FirebaseJsonArray object
            arr->get(result, i);
            if (result.typeNum == FirebaseJson::JSON_BOOL)
                Serial.println(result.to<bool>() ? (const char *)FPSTR("true") : (const char *)FPSTR("false"));
            else if (result.typeNum == FirebaseJson::JSON_INT)
                Serial.println(result.to<int>());
            else if (result.typeNum == FirebaseJson::JSON_FLOAT)
                Serial.println(result.to<float>());
            else if (result.typeNum == FirebaseJson::JSON_DOUBLE)
#if defined(MB_ARDUINO_PICO)
                Serial.printf("%.9lf\n", result.to<double>());
#else
                printf("%.9lf\n", result.to<double>());
#endif
            else if (result.typeNum == FirebaseJson::JSON_STRING ||
                     result.typeNum == FirebaseJson::JSON_NULL ||
                     result.typeNum == FirebaseJson::JSON_OBJECT ||
                     result.typeNum == FirebaseJson::JSON_ARRAY)
                Serial.println(result.to<String>());
        }
        arr->clear();
    }
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_blob)
    {
        MB_VECTOR<uint8_t> *blob = data.to<MB_VECTOR<uint8_t> *>();
        for (size_t i = 0; i < blob->size(); i++)
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();
            if ((*blob)[i] < 16)
                Serial.print((const char *)FPSTR("0"));
            Serial.print((*blob)[i], HEX);
            Serial.print((const char *)FPSTR(" "));
        }
        Serial.println();
    }
    else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_file)
    {
#if defined(FLASH_FS)
        File file = data.to<File>();
        int i = 0;
        while (file.available())
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();

            int v = file.read();

            if (v < 16)
                Serial.print((const char *)FPSTR("0"));

            Serial.print(v, HEX);
            Serial.print((const char *)FPSTR(" "));
            i++;
        }
        Serial.println();
        data.closeFile();
#endif
    }
}

#endif

#endif // ENABLE

#endif // RTDB_HElPER_H