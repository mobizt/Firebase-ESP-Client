/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

#include <Arduino.h>
#include <FirebaseJson.h>

    void setup()
{

    Serial.begin(115200);

    Serial.println();
    Serial.println();

    FirebaseJson json1;
    FirebaseJsonArray arr1;
    FirebaseJsonData jsonData;
    FirebaseJsonArray arr2;

    arr1.add("Val 1");
    arr1.add(777);
    arr1.add();
    arr1.add(0.274);
    String t = "";
    String arrStr, jsonStr;

    json1.add("key1", "to dance with my father again");
    json1.add("key2", 222.493);
    json1.add("key3", arr1);

    Serial.println("-----------------------------------------------");
    Serial.println("Array 1 data");
    Serial.println("-----------------------------------------------");
    arr1.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("JSON object with nested Array 1 data");
    Serial.println("-----------------------------------------------");
    json1.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("Get nested Array 1 data back from JSON object");
    Serial.println("-----------------------------------------------");

    json1.get(jsonData, "key3");

    Serial.println("Data type: " + jsonData.type);
    Serial.println("Data type Num: " + String(jsonData.typeNum));

    //Check the type of returned data and success
    if (jsonData.type == "array" && jsonData.success)
    {

        jsonData.getArray(arr2);
        Serial.println("Array Size: " + String(arr2.size()));
        for (size_t i = 0; i < arr2.size(); i++)
        {
            arr2.get(jsonData, i);
            if (jsonData.type == "string" /* jsonData.typeNum == FirebaseJson::JSON_STRING */)
                Serial.println("Array index " + String(i) + ", String Val: " + jsonData.stringValue);
            else if (jsonData.type == "int" /* jsonData.typeNum == FirebaseJson::JSON_INT */)
                Serial.println("Array index " + String(i) + ", Int Val: " + jsonData.intValue);
            else if (jsonData.type == "float" /* jsonData.typeNum == FirebaseJson::JSON_FLOAT */)
                Serial.println("Array index " + String(i) + ", Float Val: " + jsonData.floatValue);
            else if (jsonData.type == "double" /* jsonData.typeNum == FirebaseJson::JSON_DOUBLE */)
                Serial.println("Array index " + String(i) + ", Double Val: " + jsonData.doubleValue);
            else if (jsonData.type == "bool" /* jsonData.typeNum == FirebaseJson::JSON_BOOL */)
                Serial.println("Array index " + String(i) + ", Bool Val: " + jsonData.boolValue);
            else if (jsonData.type == "object" /* jsonData.typeNum == FirebaseJson::JSON_OBJECT */)
                Serial.println("Array index " + String(i) + ", Object Val: " + jsonData.stringValue);
            else if (jsonData.type == "array" /* jsonData.typeNum == FirebaseJson::JSON_ARRAY */)
                Serial.println("Array index " + String(i) + ", Array Val: " + jsonData.stringValue);
            else if (jsonData.type == "null" /* jsonData.typeNum == FirebaseJson::JSON_NULL */)
                Serial.println("Array index " + String(i) + ", Null Val: " + jsonData.stringValue);
        }
    }
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("Remove nested Array 1 value at index 2 from JSON");
    Serial.println("-----------------------------------------------");
    json1.remove("key3/[2]");
    json1.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("Remove Array 1 value at index 0 from JSON Array");
    Serial.println("-----------------------------------------------");
    arr1.remove(0);
    arr1.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("Set or update JSON object at key3");
    Serial.println("-----------------------------------------------");
    json1.set("key3", "This is replaced text instead of array");
    json1.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("Plain JSON object string");
    Serial.println("-----------------------------------------------");
    json1.toString(jsonStr);
    Serial.println(jsonStr);
    Serial.println();

    Serial.println("-----------------------------------------------");
    Serial.println("Iterate all JSMN tokens");
    Serial.println("-----------------------------------------------");

    FirebaseJsonData jsonParseResult;

    size_t count = json1.iteratorBegin();
    String key;
    String value;
    int type = 0;

    for (size_t i = 0; i < count; i++)
    {
        json1.iteratorGet(i, type, key, value);

        Serial.print("KEY: ");
        Serial.print(key);
        Serial.print(", ");
        Serial.print("VALUE: ");
        Serial.print(value);
        Serial.print(", ");
        Serial.print("TYPE: ");
        Serial.println(type == FirebaseJson::JSMN_OBJECT ? "object" : "array");
    }

    Serial.println();

    json1.clear();
    arr1.clear();
    arr2.clear();
}

void loop()
{
}