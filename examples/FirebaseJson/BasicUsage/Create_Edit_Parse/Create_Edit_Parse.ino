/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/FirebaseJson
 *
 * Copyright (c) 2023 mobizt
 *
 */

#include <Arduino.h>
#include <FirebaseJson.h>

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // Create, parsing and modify JSON object and JSON array
    // FirebaseJSON supports Arduino String, PROGMEM, flash string, std::string, char array, string literal, and sum of arduino String.

    // All functions supported FPSTR, String, std::string, const char* and char array as parameters.

    FirebaseJson json;       // or constructor with contents e.g. FirebaseJson json("{\"a\":true}");
    FirebaseJsonArray arr;   // or constructor with contents e.g. FirebaseJsonArray arr("[1,2,true,\"test\"]");
    FirebaseJsonData result; // object that keeps the deserializing result

    // To set content
    json.setJsonData("{\"a\":true}");

    arr.setJsonArrayData("[1,2,3]");

    // To add data to json
    json.add("b" /* key or name only */, 123 /* value of any type */);

    // To set data to json
    json.set("a/b/c" /* key or relative path */, "hello" /* value */);

    // To add value to array
    arr.add("hello").add("test").add(99); // or arr.add("hello", "test", 99);

    // To add json into array
    FirebaseJson json2("{\"d\":888,\"e\":false}");
    arr.add(json2);

    // To set the value at index
    arr.set("[0]", 555); // or arr.set(0, 555);

    // To set the value at specific path
    arr.set("/[8]/i", 111);
    arr.set("/[8]/j", 222);
    arr.set("/[8]/k", "hi");

    // To add/set array into json
    json.set("x/y/z", arr);

    // To serialize json to serial
    json.toString(Serial, true /* prettify option */);

    // To serialize array to string
    String str;
    arr.toString(str, true /* prettify option */);

    Serial.println("\n---------");
    Serial.println(str);

    // To remove value from array at index or path
    arr.remove("[6]/d" /* path */);
    arr.remove(7 /* index */);
    Serial.println("\n---------");
    Serial.println(arr.raw()); // print raw string

    // To remove value from json
    json.remove("x/y/z/[6]");
    Serial.println("\n---------");
    Serial.println(json.raw()); // print raw string

    // To get the value from json (deserializing)
    json.get(result /* FirebaseJsonData */, "a/b/c" /* key or path */);

    // To check the deserialized result and get its type and value
    if (result.success)
    {
        Serial.println("\n---------");
        if (result.type == "string") /* or result.typeNum == FirebaseJson::JSON_STRING */
            Serial.println(result.to<String>().c_str());
        else if (result.type == "int") /* or result.typeNum == FirebaseJson::JSON_INT */
            Serial.println(result.to<int>());
        else if (result.type == "float") /* or result.typeNum == FirebaseJson::JSON_FLOAT */
            Serial.println(result.to<float>());
        else if (result.type == "double") /* or result.typeNum == FirebaseJson::JSON_DOUBLE */
            Serial.println(result.to<double>());
        else if (result.type == "bool") /* or result.typeNum == FirebaseJson::JSON_BOOL */
            Serial.println(result.to<bool>());
        else if (result.type == "object") /* or result.typeNum == FirebaseJson::JSON_OBJECT */
            Serial.println(result.to<String>().c_str());
        else if (result.type == "array") /* or result.typeNum == FirebaseJson::JSON_ARRAY */
            Serial.println(result.to<String>().c_str());
        else if (result.type == "null") /* or result.typeNum == FirebaseJson::JSON_NULL */
            Serial.println(result.to<String>().c_str());
    }

    // To get the json object from deserializing result
    json.get(result /* FirebaseJsonData */, "x/y/z/[7]" /* key or path */);
    if (result.success)
    {

        if (result.type == "object") /* or result.typeNum == FirebaseJson::JSON_OBJECT */
        {
            // Use result.get or result.getJSON instead of result.to
            FirebaseJson json3;
            result.get<FirebaseJson /* type e.g. FirebaseJson or FirebaseJsonArray */>(json3 /* object that used to store value */);
            // or result.getJSON(json3);
            Serial.println("\n---------");
            json3.toString(Serial, true); // serialize contents to serial

            // To iterate all values in Json object
            size_t count = json3.iteratorBegin();
            Serial.println("\n---------");
            for (size_t i = 0; i < count; i++)
            {
                FirebaseJson::IteratorValue value = json3.valueAt(i);
                Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(), value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
            }

            Serial.println();
            json3.iteratorEnd(); // required for free the used memory in iteration (node data collection)
        }
    }

    // To get the json array object from deserializing result
    json.get(result /* FirebaseJsonData */, "x/y/z" /* key or path */);
    if (result.success)
    {

        if (result.type == "array") /* or result.typeNum == FirebaseJson::JSON_ARRAY */
        {
            // Use result.get or result.getJSON instead of result.to
            FirebaseJsonArray arr2;
            result.get<FirebaseJsonArray /* type e.g. FirebaseJson or FirebaseJsonArray */>(arr2 /* object that used to store value */);
            // or result.getArray(arr2);
            Serial.println("\n---------");
            arr2.toString(Serial, true); // serialize contents to serial

            // To iterate all values in Json array object
            Serial.println("\n---------");
            FirebaseJsonData result2;
            for (size_t i = 0; i < arr2.size(); i++)
            {
                arr2.get(result2, i);
                if (result2.type == "string" /* result2.typeNum == FirebaseJson::JSON_STRING */)
                    Serial.printf("Array index %d, String Val: %s\n", i, result2.to<String>().c_str());
                else if (result2.type == "int" /* result2.typeNum == FirebaseJson::JSON_INT */)
                    Serial.printf("Array index %d, Int Val: %d\n", i, result2.to<int>());
                else if (result2.type == "float" /* result2.typeNum == FirebaseJson::JSON_FLOAT */)
                    Serial.printf("Array index %d, Float Val: %f\n", i, result2.to<float>());
                else if (result2.type == "double" /* result2.typeNum == FirebaseJson::JSON_DOUBLE */)
                    Serial.printf("Array index %d, Double Val: %f\n", i, result2.to<double>());
                else if (result2.type == "bool" /* result2.typeNum == FirebaseJson::JSON_BOOL */)
                    Serial.printf("Array index %d, Bool Val: %d\n", i, result2.to<bool>());
                else if (result2.type == "object" /* result2.typeNum == FirebaseJson::JSON_OBJECT */)
                    Serial.printf("Array index %d, Object Val: %s\n", i, result2.to<String>().c_str());
                else if (result2.type == "array" /* result2.typeNum == FirebaseJson::JSON_ARRAY */)
                    Serial.printf("Array index %d, Array Val: %s\n", i, result2.to<String>().c_str());
                else if (result2.type == "null" /* result2.typeNum == FirebaseJson::JSON_NULL */)
                    Serial.printf("Array index %d, Null Val: %s\n", i, result2.to<String>().c_str());
            }

            // Or use the same method as iterate the object
            /*
            size_t count = arr2.iteratorBegin();
            Serial.println("\n---------");
            for (size_t i = 0; i < count; i++)
            {
                FirebaseJson::IteratorValue value = arr2.valueAt(i);
                Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(), value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
            }
            */
        }
    }
}

void loop()
{
}