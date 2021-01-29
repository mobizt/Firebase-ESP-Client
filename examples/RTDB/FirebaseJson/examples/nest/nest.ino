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
    arr1.set("[0]/int", 123);
    arr1.set("[0]/double", 456.789);
    arr1.set("[0]/string", "Hello World!");
    arr1.set("[0]/bool", true);
    arr1.set("[1]/int", 876);
    arr1.set("[2]/[1]/string", "done");
    arr1.set(4, "What's up");

    FirebaseJson json2;
    json2.set("Hi/myInt", 200);
    json2.set("Hi/myDouble", 0.0023);
    json2.set("This/is/[2]/[3]", "Me");
    json2.set("That/is", false);

    arr1.set(6, json2);

    FirebaseJsonData jsonObj;

    String jsonStr;

    json1.set("anotherNode/data/[0]", "How are you?");
    json1.set("anotherNode/data/[2]", arr1);

    json1.get(jsonObj, "anotherNode/data/[2]", true);

    Serial.println("-------------------------");

    json1.toString(jsonStr, true);
    Serial.println(jsonStr);

    Serial.println("-------------------------");

    Serial.println(jsonObj.stringValue);

    Serial.println("-------------------------");

    json1.remove("anotherNode/data/[0]");

    json1.toString(jsonStr, true);
    Serial.println(jsonStr);

    Serial.println("-------------------------");

    json1.set("anotherNode/data/[6]/That/is", "my house");
    json1.get(jsonObj, "anotherNode/data/[6]/That/is", true);

    Serial.println(jsonObj.stringValue);

    Serial.println("-------------------------");

    json1.toString(jsonStr, true);
    Serial.println(jsonStr);
}

void loop()
{
}