
/**
 * Google's Firebase Token Management class, FirebaseCore.h version 1.0.2
 * 
 * Created December 27, 2023
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

#ifndef FIREBASE_CORE_H
#define FIREBASE_CORE_H

#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "./FB_Utils.h"
#include "./client/FB_TCP_Client.h"
#include "./FirebaseFS.h"
#include "./mbfs/MB_FS.h"

using namespace mb_string;

class FirebaseCore
{
    friend class FIREBASE_CLASS;

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

    friend class FIREBASE_STREAM_CLASS;
    friend class FIREBASE_MP_STREAM_CLASS;
    friend class UtilsClass;
    friend class FB_RTDB;
    friend class FirebaseData;
    friend class QueryFilter;

public:
    FirebaseCore();
    ~FirebaseCore();

private:
    MB_FS mbfs;
    Utils ut;
    StringHelper sh;
    URLHelper uh;
    JsonHelper jh;
    HttpHelper hh;
    Base64Helper bh;
    OtaHelper oh;
    uint32_t baseTs = 0;
    uint32_t tsOffset = 0;
    struct firebase_cfg_int_t internal;
    BearSSL_Session bsslSession;
    FirebaseConfig *config = nullptr;
    FirebaseAuth *auth = nullptr;
    firebase_wifi wifiCreds;
    float gmtOffset = 0;
#if defined(ESP8266)
    callback_function_t esp8266_cb = nullptr;
#endif
    struct token_info_t tokenInfo;
    bool authenticated = false;
    Firebase_TCP_Client *tcpClient = nullptr;
    FirebaseJson *jsonPtr = nullptr;
    FirebaseJsonData *resultPtr = nullptr;
    int response_code = 0;
    time_t ts = 0;
    bool autoReconnectNetwork = false;

    // Used as local vars in reconnect
    unsigned long last_reconnect_millis = 0;
    bool net_once_connected = false;
    uint16_t net_reconnect_tmo = MIN_NET_RECONNECT_TIMEOUT;

    volatile bool networkStatus = false;
    bool networkChecking = false;

    /* intitialize the class */
    void begin(FirebaseConfig *config, FirebaseAuth *auth);
    /* free memory */
    void end();
    /* parse service account json file for private key */
    bool parseSAFile();
    /* clear service account credentials */
    void clearServiceAccountCreds();
    /* check for sevice account credentials */
    bool serviceAccountCredsReady();
    /* set token type */
    void setTokenType(firebase_auth_token_type type);
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

    /* time functions */
    uint32_t getTimestamp(int year, int mon, int date, int hour, int mins, int sec);
    void getBaseTime();
    int setTimestamp(time_t ts);
    bool timeReady();
    void timeBegin();
    void readNTPTime();

    /* exchane the auth token with the refresh token */
    bool refreshToken();
    /* set the token status by error code */
    void setTokenError(int code);
    /* create new TCP client */
    void newClient(Firebase_TCP_Client **client);
    /* delete TCP client */
    void freeClient(Firebase_TCP_Client **client);
    /* handle the token processing task error */
    bool handleTaskError(int code, int httpCode = 0);
    // parse the auth token response
    bool handleTokenResponse(int &httpCode);
    /* process the tokens (generation, signing, request and refresh) */
    void tokenProcessingTask();
    bool handleError(int code, const char *descr, int errNum = 0);
    /* encode and sign the JWT token */
    bool createJWT();
    /* verifying the user with email/passwod to get id token */
    bool getIdToken(bool createUser, MB_StringPtr email, MB_StringPtr password);
    /* delete id token */
    bool deleteIdToken(MB_StringPtr idToken);
    /* request or refresh the token */
    bool requestTokens(bool refresh);
    /* check the token ready status and process the token tasks */
    bool checkToken();
    /* parse expiry time from string */
    void getExpiration(const char *exp);
    /* send email */
    bool handleEmailSending(MB_StringPtr payload, firebase_user_email_sending_type type);
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
    firebase_auth_token_type getTokenType();
    /* get Root CA file name */
    MB_String getCAFile();
    /* get Root CA filesystem type */
    firebase_mem_storage_type getCAFileStorage();
    /* get the pointer to user defined config */
    FirebaseConfig *getCfg();
    /* get the pointer to user defined auth data*/
    FirebaseAuth *getAuth();

    bool waitIdle(int &httpCode);

    /* prepare or initialize the external/internal TCP client */
    bool initClient(PGM_P subDomain, firebase_auth_token_status status = token_status_uninitialized);
    /* resume network connection */
    bool reconnect(Firebase_TCP_Client *client, firebase_session_info_t *session, unsigned long dataTime = 0);
    bool reconnect();
    void resumeNetwork(Firebase_TCP_Client *client, bool &net_once_connected, unsigned long &last_reconnect_millis, uint16_t &net_reconnect_tmo);
    /* close TCP session */
    void closeSession(Firebase_TCP_Client *client, firebase_session_info_t *session);
    /* set external Client */
    void setTCPClient(Firebase_TCP_Client *tcpClient);
    /* set the network status acknowledge */
    void setNetworkStatus(bool status);
    /* get system time */
    time_t getTime();
    /* set the system time */
    bool setTime(time_t ts);
    /* set the WiFi (or network) auto reconnection option */
    void setAutoReconnectNetwork(bool reconnect);

#if defined(ESP8266)
    void set_scheduled_callback(callback_function_t callback)
    {
        esp8266_cb = std::move([callback]()
                               { schedule_function(callback); });
        esp8266_cb();
    }
#endif

#if defined(FIREBASE_HAS_WIFIMULTI)
    WiFiMulti *multi = nullptr;
#endif

    firebase_client_type _cli_type = firebase_client_type_undefined;
    FB_NetworkConnectionRequestCallback _net_con_cb = NULL;
    FB_NetworkStatusRequestCallback _net_stat_cb = NULL;
    Client *_cli = nullptr;

    int _ethernet_reset_pin = -1;
    int _ethernet_cs_pin = -1;
    uint8_t *_ethernet_mac = nullptr;
    Firebase_StaticIP *_static_ip = nullptr;

#if defined(FIREBASE_GSM_MODEM_IS_AVAILABLE)
    MB_String _pin, _apn, _user, _password;
    void *_modem = nullptr;
#endif
};

extern FirebaseCore Core;

#endif