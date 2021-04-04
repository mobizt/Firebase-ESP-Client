/**
 * Google's Firebase ESP Client Main class, Firebase_ESP_Client.h version 2.0.15
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created April 4, 2021
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

#ifndef FIREBASE_ESP_CLIENT_H
#define FIREBASE_ESP_CLIENT_H

#include <Arduino.h>
#include "signer/Signer.h"
#include "rtdb/FB_RTDB.h"
#include "message/FCM.h"
#include "Utils.h"
#include "storage/FCS.h"
#include "gcs/GCS.h"
#include "firestore/FB_Firestore.h"
#include "functions/FB_Functions.h"
#include "functions/FunctionsConfig.h"

class Firebase_ESP_Client
{
    friend class QueryFilter;
    friend class FirebaseSession;

public:
    FB_RTDB RTDB;
    FB_CM FCM;
    FB_Storage Storage;
    FB_Firestore Firestore;
    FB_Functions Functions;
    GG_CloudStorage GCStorage;

    Firebase_ESP_Client();
    ~Firebase_ESP_Client();

    /** Initialize Firebase with the config and Firebase's authentication credentials.
     * 
     * @param config The pointer to FirebaseConfig data.
     * @param auth The pointer to FirebaseAuth data.
     * 
     *  @note For FirebaseConfig and FirebaseAuth data usage, see the examples.
    */
    void begin(FirebaseConfig *config, FirebaseAuth *auth);

    /** Provide the details of token generation.
     * 
     * @return token_info_t The token_info_t structured data that indicates the status.
     * 
     * @note Use type property to get the type enum value.
     * token_type_undefined or 0,
     * token_type_legacy_token or 1,
     * token_type_id_token or 2,
     * token_type_custom_token or 3,
     * token_type_oauth2_access_token or 4
     * 
     * Use status property to get the status enum value.
     * token_status_uninitialized or 0,
     * token_status_on_signing or 1,
     * token_status_on_request or 2,
     * token_status_on_refresh or 3,
     * token_status_ready or 4
     * 
     * In case of token generation and refreshment errors,
     * use error.code property to get the error code number.
     * Use error.message property to get the error message string.
     * 
    */
    struct token_info_t authTokenInfo();

    /** Sign up for a new user.
     * 
     * @param config The pointer to FirebaseConfig data.
     * @param auth The pointer to FirebaseAuth data.
     * @param email The user Email.
     * @param password The user password.
     * @return Boolean type status indicates the success of the operation. 
     * 
     * @note By calling Firebase.begin with config and auth after sign up will be signed in.
     * 
     * This required Email/Password provider to be enabled,
     * From Firebase console, select Authentication, select Sign-in method tab,
     * under the Sign-in providers list, enable Email/Password provider.
     * 
     * If the assigned email and passowrd are empty,
     * the anonymous user will be created if Anonymous provider is enabled.
     * 
     * To enable Anonymous provider,
     * from Firebase console, select Authentication, select Sign-in method tab, 
     * under the Sign-in providers list, enable Anonymous provider.
    */
    bool signUp(FirebaseConfig *config, FirebaseAuth *auth, const char *email, const char *password);

    /** Send a user a verification Email.
     * 
     * @param config The pointer to FirebaseConfig data.
     * @param idToken The id token of user that was already signed in with Email and password (optional).
     * @return Boolean type status indicates the success of the operation.
     * 
     * @note The id token can be obtained from config.signer.tokens.id_token 
     * after begin with config and auth data
     * 
     * If the idToken is not assigned, the internal config.signer.tokens.id_token
     * will be used.
     * 
     * See the Templates of Email address verification in the Firebase console
     * , Authentication.
    */
    bool sendEmailVerification(FirebaseConfig *config, const char *idToken = "");

    /** Send a user a password reset link to Email.
     * 
     * @param config The pointer to FirebaseConfig data.
     * @param email The user Email to send the password resset link.
     * @return Boolean type status indicates the success of the operation.
    */
    bool sendResetPassword(FirebaseConfig *config, const char *email);

    /** Reconnect WiFi if lost connection.
     * 
     *  @param reconnect The boolean to set/unset WiFi AP reconnection.
    */
    void reconnectWiFi(bool reconnect);

    /** Set the decimal places for float value to be stored in database.
     * 
     * @param digits The decimal places.
    */
    void setFloatDigits(uint8_t digits);

    /** Set the decimal places for double value to be stored in database.
     * 
     * @param digits The decimal places.
    */
    void setDoubleDigits(uint8_t digits);

    /** SD card config with GPIO pins.
     * 
     * @param ss -   SPI Chip/Slave Select pin.
     * @param sck -  SPI Clock pin.
     * @param miso - SPI MISO pin.
     * @param mosi - SPI MOSI pin.
     * @return Boolean type status indicates the success of the operation.
    */
    bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);

private:
    UtilsClass *ut = nullptr;
    FirebaseAuth *_auth = nullptr;
    FirebaseConfig *_cfg = nullptr;
    //internal or used by legacy data
    FirebaseAuth _auth_;
    FirebaseConfig _cfg_;
};

extern Firebase_ESP_Client Firebase;

#endif