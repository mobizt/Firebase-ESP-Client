#ifndef RTDBHelper_H
#define RTDBHelper_H
#include <Arduino.h>
#include "FirebaseFS.h"

#if defined(FIREBASE_ESP_CLIENT)
#include <Firebase_ESP_Client.h>
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#if defined(ESP32)
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <FirebaseESP8266.h>
#endif
#endif

void printResult(FirebaseData &data)
{
    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
            if (type == FirebaseJson::JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == FirebaseJson::JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == FirebaseJson::JSON_FLOAT)
                Serial.println(jsonData.floatValue);
            else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
                     jsonData.typeNum == FirebaseJson::JSON_NULL ||
                     jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
                     jsonData.typeNum == FirebaseJson::JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
    else if (data.dataType() == "blob")
    {
        std::vector<uint8_t> blob = data.blobData();
        for (size_t i = 0; i < blob.size(); i++)
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();
            if (blob[i] < 16)
                Serial.print("0");
            Serial.print(blob[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    else if (data.dataType() == "file")
    {
        File file = data.fileStream();
        int i = 0;
        while (file.available())
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();

            int v = file.read();

            if (v < 16)
                Serial.print("0");

            Serial.print(v, HEX);
            Serial.print(" ");
            i++;
        }
        Serial.println();
        file.close();
    }
    else
    {
        Serial.println(data.payload());
    }
}

void printResult(FIREBASE_STREAM_CLASS &data)
{

    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string" || data.dataType() == "null")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        FirebaseJson *json = data.jsonObjectPtr();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json->toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json->iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json->iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
            if (type == FirebaseJson::JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json->iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray *arr = data.jsonArrayPtr();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr->toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();

        for (size_t i = 0; i < arr->size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");
            FirebaseJsonData *jsonData = data.jsonDataPtr();
            //Get the result data from FirebaseJsonArray object
            arr->get(*jsonData, i);
            if (jsonData->typeNum == FirebaseJson::JSON_BOOL)
                Serial.println(jsonData->boolValue ? "true" : "false");
            else if (jsonData->typeNum == FirebaseJson::JSON_INT)
                Serial.println(jsonData->intValue);
            else if (jsonData->typeNum == FirebaseJson::JSON_FLOAT)
                Serial.println(jsonData->floatValue);
            else if (jsonData->typeNum == FirebaseJson::JSON_DOUBLE)
                printf("%.9lf\n", jsonData->doubleValue);
            else if (jsonData->typeNum == FirebaseJson::JSON_STRING ||
                     jsonData->typeNum == FirebaseJson::JSON_NULL ||
                     jsonData->typeNum == FirebaseJson::JSON_OBJECT ||
                     jsonData->typeNum == FirebaseJson::JSON_ARRAY)
                Serial.println(jsonData->stringValue);
        }
    }
    else if (data.dataType() == "blob")
    {
        std::vector<uint8_t> blob = data.blobData();
        for (size_t i = 0; i < blob.size(); i++)
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();
            if (blob[i] < 16)
                Serial.print("0");
            Serial.print(blob[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    else if (data.dataType() == "file")
    {
        File file = data.fileStream();
        int i = 0;
        while (file.available())
        {
            if (i > 0 && i % 16 == 0)
                Serial.println();

            int v = file.read();

            if (v < 16)
                Serial.print("0");

            Serial.print(v, HEX);
            Serial.print(" ");
            i++;
        }
        Serial.println();
        file.close();
    }
}

#endif