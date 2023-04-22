#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Token Management class, Signer.h version 1.3.12
 *
 * This library supports Espressif ESP8266, ESP32 and Raspberry Pi Pico
 *
 * Created April 22, 2023
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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
#include "mbfs/MB_MCU.h"
#include "FB_Utils.h"
#include "./wcs/FB_Clients.h"
#include "./FirebaseFS.h"
#include "./mbfs/MB_FS.h"

using namespace mb_string;

class Firebase_Signer
{
    friend class FIREBASE_CLASS;
#if defined(FIREBASE_ESP_CLIENT)
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
    FirebaseConfig *config = nullptr;
    FirebaseAuth *auth = nullptr;
    MB_FS *mbfs = nullptr;
    uint32_t *mb_ts = nullptr;
    uint32_t *mb_ts_offset = nullptr;
    MB_NTP ntpClient;
    UDP *udp = nullptr;
    fb_esp_wifi_credentials_t wifiCreds;
    float gmtOffset = 0;
#if defined(ESP8266)
    callback_function_t esp8266_cb = nullptr;
#endif
    struct token_info_t tokenInfo;
    bool authenticated = false;
    bool _token_processing_task_enable = false;
    FB_TCP_CLIENT *tcpClient = nullptr;
    bool localTCPClient = false;
    FirebaseJson *jsonPtr = nullptr;
    FirebaseJsonData *resultPtr = nullptr;
    int response_code = 0;
    time_t ts = 0;
    bool autoReconnectWiFi = false;

    // Used as local vars in reconnect
    unsigned long last_reconnect_millis = 0;
    bool net_once_connected = false;
    uint16_t wifi_reconnect_tmo = MIN_WIFI_RECONNECT_TIMEOUT;

    volatile bool networkStatus = false;
    bool networkChecking = false;

    /* intitialize the class */
    void begin(FirebaseConfig *config, FirebaseAuth *auth, MB_FS *mbfs, uint32_t *mb_ts, uint32_t *mb_ts_offset);
    /* free memory */
    void end();
    /* parse service account json file for private key */
    bool parseSAFile();
    /* clear service account credentials */
    void clearServiceAccountCreds();
    /* check for sevice account credentials */
    bool serviceAccountCredsReady();
    /* set token type */
    void setTokenType(fb_esp_auth_token_type type);
    /* check for user account credentials */
    bool userAccountCredsReady();
    /* check for supported tokens */
    bool isAuthToken(bool oauth);
    /* check for time is up or expiry time was reset or unset? */
    bool isExpired();
    /* Adjust the expiry time if system time synched or set. Adjust pre-refresh seconds to not exceed */
    void adjustTime(time_t &now);
    /* auth token was never been request or the last request was timed out */
    bool readyToRequest();
    /* is the time to refresh the token */
    bool readyToRefresh();
    /* is the time to sync clock */
    bool readyToSync();
    /* time synching timed out */
    bool isSyncTimeOut();
    /* error callback timed out */
    bool isErrorCBTimeOut();
    /* handle the auth tokens generation */
    bool handleToken();
    /* init the temp use Json objects */
    void initJson();
    /* free the temp use Json objects */
    void freeJson();
    /* exchane the auth token with the refresh token */
    bool refreshToken();
    /* set the token status by error code */
    void setTokenError(int code);
    /* create new TCP client */
    void newClient(FB_TCP_CLIENT **client);
    /* delete TCP client */
    void freeClient(FB_TCP_CLIENT **client);
    /* handle the token processing task error */
    bool handleTaskError(int code, int httpCode = 0);
    // parse the auth token response
    bool handleTokenResponse(int &httpCode);
    /* process the tokens (generation, signing, request and refresh) */
    void tokenProcessingTask();
    bool checkUDP(UDP *udp, bool &ret, bool &_token_processing_task_enable, float gmtOffset);
    /* encode and sign the JWT token */
    bool createJWT();
    /* verifying the user with email/passwod to get id token */
    bool getIdToken(bool createUser, MB_StringPtr email, MB_StringPtr password);
    /* delete id token */
    bool deleteIdToken(MB_StringPtr idToken);
    /* request or refresh the token */
    bool requestTokens(bool refresh);
    /* check the token ready status and process the token tasks */
    void checkToken();
    /* parse expiry time from string */
    void getExpiration(const char *exp);
    /* send email */
    bool handleEmailSending(MB_StringPtr payload, fb_esp_user_email_sending_type type);
    /* return error string from code */
    void errorToString(int httpCode, MB_String &buff);
    /* check the token ready status and process the token tasks and returns the status */
    bool tokenReady();
    /* error status callback */
    void sendTokenStatusCB();
    /* get auth token */
    const char *getToken();
    /* get refresh token */
    const char *getRefreshToken();
    /* check for authentication type changes */
    bool checkAuthTypeChanged(FirebaseConfig *config, FirebaseAuth *auth);
    /* get token type */
    fb_esp_auth_token_type getTokenType();
    /* get Root CA file name */
    MB_String getCAFile();
    /* get Root CA filesystem type */
    fb_esp_mem_storage_type getCAFileStorage();
    /* get the pointer to user defined config */
    FirebaseConfig *getCfg();
    /* get the pointer to MB_FS class object */
    MB_FS *getMBFS();
    /* get the pointer to internal timestamp var */
    uint32_t *getTS();
    /* get the pointer to user defined auth data*/
    FirebaseAuth *getAuth();
    /* get the pointer to wifi crendentials object */
    fb_esp_wifi_credentials_t *getCred();

    /* prepare or initialize the external/internal TCP client */
    bool initClient(PGM_P subDomain, fb_esp_auth_token_status status = token_status_uninitialized);
    /* resume network connection */
    bool reconnect(FB_TCP_CLIENT *client, fb_esp_session_info_t *session, unsigned long dataTime = 0);
    bool reconnect();
    void resumeWiFi(FB_TCP_CLIENT *client, bool &net_once_connected, unsigned long &last_reconnect_millis, uint16_t &wifi_reconnect_tmo, bool session_connected);
    /* close TCP session */
    void closeSession(FB_TCP_CLIENT *client, fb_esp_session_info_t *session);
    /* set external Client */
    void setTCPClient(FB_TCP_CLIENT *tcpClient);
    void setUDPClient(UDP *client, float gmtOffset);
    /* set the network status acknowledge */
    void setNetworkStatus(bool status);
    /* get system time */
    time_t getTime();
    /* set the system time */
    bool setTime(time_t ts);
    /* set the WiFi (or network) auto reconnection option */
    void setAutoReconnectWiFi(bool reconnect);

#if defined(ESP8266)
    void set_scheduled_callback(callback_function_t callback)
    {
        esp8266_cb = std::move([callback]()
                               { schedule_function(callback); });
        esp8266_cb();
    }
#endif
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
#if __has_include(<WiFiMulti.h>)
#define HAS_WIFIMULTI
    WiFiMulti *multi = nullptr;
#endif
#endif
};

extern Firebase_Signer Signer;

#endif