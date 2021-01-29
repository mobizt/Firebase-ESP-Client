
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

//This example shows how to dynamic allocated the Firebase Data object and deallocated it for release the memory when no further use.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

FirebaseAuth auth;
FirebaseConfig config;

//Define FirebaseESP8266 data object
FirebaseData *fbdo1 = new FirebaseData();
FirebaseData *fbdo2 = new FirebaseData();

unsigned long sendDataPrevMillis = 0;

String path = "/Test/Stream";

int count = 0;

bool deallocated = false;

void printResult(FirebaseData &data);
void printResult(FirebaseStream &data);

void streamCallback(FirebaseStream data)
{

    Serial.println("Stream Data1 available...");
    Serial.println("STREAM PATH: " + data.streamPath());
    Serial.println("EVENT PATH: " + data.dataPath());
    Serial.println("DATA TYPE: " + data.dataType());
    Serial.println("EVENT TYPE: " + data.eventType());
    Serial.print("VALUE: ");
    printResult(data);
    Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
    {
        Serial.println();
        Serial.println("Stream timeout, resume streaming...");
        Serial.println();
    }
}

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    /* Assign the project host and api key (required) */
    config.host = FIREBASE_HOST;
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo1.setBSSLBufferSize(1024, 1024);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo1->setResponseSize(1024);

#if defined(ESP8266)
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo2.setBSSLBufferSize(1024, 1024);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo2->setResponseSize(1024);

    if (!Firebase.RTDB.beginStream(fbdo1, path.c_str()))
    {
        Serial.println("------------------------------------");
        Serial.println("Can't begin stream connection...");
        Serial.println("REASON: " + fbdo1->errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }

    Firebase.RTDB.setStreamCallback(fbdo1, streamCallback, streamTimeoutCallback);
}

void loop()
{

    if (millis() - sendDataPrevMillis > 5000)
    {
        count++;
        sendDataPrevMillis = millis();

        if (count == 10)
        {
            deallocated = false;

            Serial.println("Reallocate the Firebase Data objects again after 10 times Firebasse call");

            fbdo1 = new FirebaseData();
            fbdo2 = new FirebaseData();

#if defined(ESP8266)
            //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
            fbdo1.setBSSLBufferSize(1024, 1024);
#endif

            //Set the size of HTTP response buffers in the case where we want to work with large data.
            fbdo1->setResponseSize(1024);

#if defined(ESP8266)
            //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
            fbdo2.setBSSLBufferSize(1024, 1024);
#endif

            //Set the size of HTTP response buffers in the case where we want to work with large data.
            fbdo2->setResponseSize(1024);

            if (!Firebase.RTDB.beginStream(fbdo1, path.c_str()))
            {
                Serial.println("------------------------------------");
                Serial.println("Can't begin stream connection...");
                Serial.println("REASON: " + fbdo1->errorReason());
                Serial.println("------------------------------------");
                Serial.println();
            }

            Firebase.RTDB.setStreamCallback(fbdo1, streamCallback, streamTimeoutCallback);

            count = 0;
        }

        if (!deallocated)
            Serial.print("Free Heap after allocated the Firebase Data objects: ");
        else
            Serial.print("Free Heap after deallocated the Firebase Data objects: ");

        Serial.println(ESP.getFreeHeap());

        delay(0);

        if (deallocated)
            return;

        Serial.println("------------------------------------");
        Serial.println("Set Int...");
        String Path = path + "/Int";

        if (Firebase.RTDB.set(fbdo2, Path.c_str(), count))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo2->dataPath());
            Serial.println("TYPE: " + fbdo2->dataType());
            Serial.print("VALUE: ");
            printResult(*fbdo2);
            Serial.println("------------------------------------");
            Serial.println();
        }
        else
        {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo2->errorReason());
            Serial.println("------------------------------------");
            Serial.println();
        }

        if (count == 5)
        {
            Serial.println("Deallocate the Firebase Data objects after 3 times Firebasse call");

            deallocated = true;

            //Need to stop the stream to prevent the readStream and callback to be called after objects deallocation
            Firebase.RTDB.endStream(fbdo1);
            Firebase.RTDB.removeStreamCallback(fbdo1);

            //Deallocate
            delete fbdo1;
            delete fbdo2;

            fbdo1 = nullptr;
            fbdo2 = nullptr;
        }
    }
}

//Helper function for printing the response/received data
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
        Serial.println();
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
        Serial.println();
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
    else
    {
        Serial.println(data.payload());
    }
}

//Helper function for printing the response/received stream data
void printResult(FirebaseStream &data)
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
        Serial.println();
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
        Serial.println();
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
}