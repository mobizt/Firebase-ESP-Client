/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/FirebaseJson
 *
 * Copyright (c) 2022 mobizt
 *
 */

#include <Arduino.h>
#include <FirebaseJson.h>

FirebaseJson json;

void setup()
{
    Serial.begin(115200);
    Serial.println();
}

void loop()
{
    // No matter what JSON data separation is newline or any.
    if (json.readFrom(Serial))
    {
        Serial.println("JSON Data received...");
        json.toString(Serial, true);
        Serial.println();
    }
}