/**
 * Google's Firebase Token Generation class, Signer.h version 1.0.12
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created May 11, 2021
 * 
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2020, 2021 K. Suwatchai (Mobizt)
 * 
 * The MIT License (MIT)
 * Copyright (c) 2020, 2021 K. Suwatchai (Mobizt)
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

#ifndef FIREBASE_SIGNER_H
#define FIREBASE_SIGNER_H

#include <Arduino.h>
#include "Utils.h"

class Firebase_Signer
{
#if defined(FIREBASE_ESP_CLIENT)
    friend class Firebase_ESP_Client;
    friend class FB_CM;
    friend class FB_Storage;
    friend class GG_CloudStorage;
    friend class FB_Firestore;
    friend class FB_Functions;
    friend class Binding;
    friend class PolicyBuilder;
    friend class AuditLogConfig;
    friend class AuditConfig;
    friend class FunctionsConfig;

#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#if defined(ESP32)
    friend class FirebaseESP32;
#elif defined(ESP8266)
    friend class FirebaseESP8266;
#endif
    friend class FCMObject;
#endif
    friend class FIREBASE_STREAM_CLASS;
    friend class FIREBASE_MP_STREAM_CLASS;
    friend class UtilsClass;
    friend class FB_RTDB;
    friend class FirebaseData;
    friend class QueryFilter;
  
public:
    Firebase_Signer();
    ~Firebase_Signer();

private:
    UtilsClass *ut = nullptr;
    FirebaseConfig *config = nullptr;
    FirebaseAuth *auth = nullptr;
    callback_function_t _cb = nullptr;
    struct token_info_t tokenInfo;
    bool authenticated = false;
    bool _token_processing_task_enable = false;
    unsigned long unauthen_millis = 0;
    unsigned long unauthen_pause_duration = 3000;

    void begin(UtilsClass *ut, FirebaseConfig *config, FirebaseAuth *auth);
    bool parseSAFile();
    void clearSA();
    bool tokenSigninDataReady();
    void setTokenType(fb_esp_auth_token_type type);
    bool userSigninDataReady();
    bool handleToken();
    bool refreshToken();
    void setTokenError(int code);
    bool handleSignerError(int code);
    bool handleTokenResponse();
    void tokenProcessingTask();
    bool createJWT();
    bool getIdToken(bool createUser, const char *email, const char *password);
    bool requestTokens();
    void checkToken();
    void getExpiration(const String &exp);
    bool handleEmailSending(const char *payload, fb_esp_user_email_sending_type type);
    void errorToString(int httpCode, std::string &buff);
    bool tokenReady();
    void sendTokenStatusCB();
    std::string getToken(fb_esp_auth_token_type type);
    fb_esp_auth_token_type getTokenType();
    std::string getCAFile();
    int getCAFileStorage();
    FirebaseConfig *getCfg();
    FirebaseAuth *getAuth();

#if defined(ESP8266)
    void set_scheduled_callback(callback_function_t callback)
    {
        _cb = std::move([callback]() { schedule_function(callback); });
        _cb();
    }
#endif
};

extern Firebase_Signer Signer;

#endif