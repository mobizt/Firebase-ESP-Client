
/**
 * Google's Firebase Token Management class, FirebaseCore.cpp version 1.0.2
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

#ifndef FIREBASE_CORE_CPP
#define FIREBASE_CORE_CPP

#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "FirebaseCore.h"

FirebaseCore::FirebaseCore()
{
}

FirebaseCore::~FirebaseCore()
{
    end();
}

void FirebaseCore::begin(FirebaseConfig *cfg, FirebaseAuth *authen)
{
    this->config = cfg;
    this->auth = authen;
}

void FirebaseCore::end()
{
    freeJson();

    wifiCreds.clearAP();
#if defined(HAS_WIFIMULTI)
    if (multi)
        delete multi;

    multi = nullptr;
#endif
    freeClient(&tcpClient);
}

bool FirebaseCore::parseSAFile()
{
    bool ret = false;

    if (!config || config->signer.pk.length() > 0)
        return ret;

    int res = mbfs.open(config->service_account.json.path, mbfs_type config->service_account.json.storage_type, mb_fs_open_mode_read);

    if (res >= 0)
    {
        clearServiceAccountCreds();
        initJson();

        size_t len = res;
        char *buf = reinterpret_cast<char *>(mbfs.newP(len + 10));
        if (mbfs.available(mbfs_type config->service_account.json.storage_type))
        {
            if ((int)len == mbfs.read(mbfs_type config->service_account.json.storage_type, (uint8_t *)buf, len))
                jsonPtr->setJsonData(buf);
        }

        mbfs.close(mbfs_type config->service_account.json.storage_type);

        if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_1)) // type
        {
            ret = true;

            if (resultPtr->to<MB_String>().find(pgm2Str(firebase_auth_pgm_str_2 /* service_account */), 0) != MB_String::npos)
            {
                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_3)) // project_id
                    config->service_account.data.project_id = resultPtr->to<const char *>();

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_5)) // private_key_id
                    config->service_account.data.private_key_id = resultPtr->to<const char *>();

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_6)) // private_key
                {
                    char *buff = reinterpret_cast<char *>(mbfs.newP(strlen(resultPtr->to<const char *>())));
                    size_t c = 0;
                    for (size_t i = 0; i < strlen(resultPtr->to<const char *>()); i++)
                    {
                        if (resultPtr->to<const char *>()[i] == '\\')
                        {
                            FBUtils::idle();
                            buff[c++] = '\n';
                            i++;
                        }
                        else
                            buff[c++] = resultPtr->to<const char *>()[i];
                    }
                    config->signer.pk = buff;
                    resultPtr->clear();
                    mbfs.delP(&buff);
                }

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_7)) // client_email
                    config->service_account.data.client_email = resultPtr->to<const char *>();
                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_8)) // client_id
                    config->service_account.data.client_id = resultPtr->to<const char *>();
            }
        }

        freeJson();
        mbfs.delP(&buf);
    }

    return ret;
}

void FirebaseCore::clearServiceAccountCreds()
{
    if (config)
    {
        config->service_account.data.private_key = "";
        config->service_account.data.project_id.clear();
        config->service_account.data.private_key_id.clear();
        config->service_account.data.client_email.clear();
        config->signer.pk.clear();
    }
}

bool FirebaseCore::serviceAccountCredsReady()
{
    return config && (strlen_P(config->service_account.data.private_key) > 0 || config->signer.pk.length() > 0) && config->service_account.data.client_email.length() > 0 && config->service_account.data.project_id.length() > 0;
}

void FirebaseCore::setTokenType(firebase_auth_token_type type)
{
    if (config)
    {
        config->signer.tokens.token_type = type;
        if (type == token_type_legacy_token)
            config->signer.tokens.status = token_status_ready;
    }
}

bool FirebaseCore::userAccountCredsReady()
{
    return config && auth && config->api_key.length() > 0 && auth->user.email.length() > 0 && auth->user.password.length() > 0;
}

bool FirebaseCore::isAuthToken(bool oauth)
{
    if (!config || !auth)
        return false;

    bool ret = config->signer.tokens.token_type == token_type_id_token || config->signer.tokens.token_type == token_type_custom_token;
    if (oauth)
        ret |= config->signer.tokens.token_type == token_type_oauth2_access_token;
    return ret;
}

bool FirebaseCore::checkAuthTypeChanged(FirebaseConfig *config, FirebaseAuth *auth)
{
    bool auth_changed = false;

    if (!config->signer.test_mode)
    {
        // service account file assigned?
        if (config->service_account.json.path.length() > 0)
        {
            // parse for private key
            if (!parseSAFile())
                config->signer.tokens.status = token_status_uninitialized;
        }

        // set token type to oauth2 or custom token based on service account credentials
        if (serviceAccountCredsReady())
        {
            config->signer.idTokenCustomSet = false;
            config->signer.accessTokenCustomSet = false;
            config->signer.customTokenCustomSet = false;

            if (auth->token.uid.length() == 0)
                setTokenType(token_type_oauth2_access_token);
            else
                setTokenType(token_type_custom_token);
        }
        // set token type to id token based on email/password credentials
        else if (userAccountCredsReady() || config->signer.anonymous)
            setTokenType(token_type_id_token);
        // set token type to legacy token based on database secret
        else if (strlen(config->signer.tokens.legacy_token) > 0)
        {
            setTokenType(token_type_legacy_token);
            internal.auth_token = config->signer.tokens.legacy_token;
            internal.ltok_len = strlen(config->signer.tokens.legacy_token);
            internal.rtok_len = 0;
            internal.atok_len = 0;
        }

        // check if user account credentials changed
        if (config->signer.tokens.token_type == token_type_id_token &&
            auth->user.email.length() > 0 &&
            auth->user.password.length() > 0)
        {
            uint16_t crc1 = mbfs.calCRC(auth->user.email.c_str()),
                     crc2 = mbfs.calCRC(auth->user.password.c_str());

            auth_changed = internal.email_crc != crc1 || internal.password_crc != crc2;

            internal.email_crc = crc1;
            internal.password_crc = crc2;
        }
        // check if service account credentials changed
        else if (config->signer.tokens.token_type == token_type_custom_token ||
                 config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            uint16_t crc1 = mbfs.calCRC(config->service_account.data.client_email.c_str());
            uint16_t crc2 = mbfs.calCRC(config->service_account.data.project_id.c_str());
            uint16_t crc3 = mbfs.calCRC(MB_String(config->service_account.data.private_key).c_str());

            auth_changed = internal.client_email_crc != crc1 || internal.project_id_crc != crc2 || internal.priv_key_crc != crc3;

            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                uint16_t crc4 = auth->token.uid.length() > 0 ? mbfs.calCRC(auth->token.uid.c_str()) : 0;
                auth_changed |= internal.uid_crc != crc4;
                internal.uid_crc = crc4;
            }

            internal.client_email_crc = crc1;
            internal.project_id_crc = crc2;
            internal.priv_key_crc = crc3;
        }

        // reset token status and flags if auth type changed
        if (auth_changed)
        {
            config->signer.tokens.status = token_status_uninitialized;
            config->signer.tokens.expires = 0;
            config->signer.idTokenCustomSet = false;
            config->signer.accessTokenCustomSet = false;
            config->signer.customTokenCustomSet = false;
        }
    }

    return auth_changed;
}

time_t FirebaseCore::getTime()
{
    getBaseTime();
    return baseTs;
}

bool FirebaseCore::setTime(time_t ts)
{
    return setTimestamp(ts);
}

bool FirebaseCore::isExpired()
{

    // never expired for legacy token and test mode
    if (!config || !auth || config->signer.tokens.token_type == token_type_legacy_token || config->signer.test_mode)
        return false;

    time_t now = 0;

    // adjust the expiry time when needed
    adjustTime(now);

    // time is up or expiry time was reset or unset?
    return (now > (int)(config->signer.tokens.expires - config->signer.preRefreshSeconds) || config->signer.tokens.expires == 0);
}

void FirebaseCore::adjustTime(time_t &now)
{
    now = getTime(); // returns timestamp

    // if time has changed after token has been generated, update its expiration
    if (config->signer.tokens.expires > 0 && config->signer.tokens.expires < FIREBASE_DEFAULT_TS && now > FIREBASE_DEFAULT_TS)
        /* new expiry time (timestamp) = current timestamp - total seconds since last token request - 60 */
        config->signer.tokens.expires += now - (millis() - config->signer.tokens.last_millis) / 1000 - 60;

    // pre-refresh seconds should not greater than the expiry time
    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;
}

bool FirebaseCore::readyToRequest()
{
    bool ret = false;
    // To detain the next request using last request millis
    if (config && (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0))
    {
        config->signer.lastReqMillis = millis();
        ret = true;
    }

    return ret;
}

bool FirebaseCore::readyToRefresh()
{
    if (!config)
        return false;
    // To detain the next request using last request millis
    return millis() - internal.fb_last_request_token_cb_millis > 5000;
}

bool FirebaseCore::readyToSync()
{
    bool ret = false;
    // To detain the next synching using last synching millis
    if (config && millis() - internal.fb_last_time_sync_millis > FB_TIME_SYNC_INTERVAL)
    {
        internal.fb_last_time_sync_millis = millis();
        ret = true;
    }

    return ret;
}

bool FirebaseCore::isSyncTimeOut()
{
    bool ret = false;
    // If device time was not synched in time
    if (config && millis() - internal.fb_last_ntp_sync_timeout_millis > config->timeout.ntpServerRequest)
    {
        internal.fb_last_ntp_sync_timeout_millis = millis();
        ret = true;
    }

    return ret;
}

bool FirebaseCore::isErrorCBTimeOut()
{
    bool ret = false;
    // To detain the next error callback
    if (config &&
        (millis() - internal.fb_last_jwt_generation_error_cb_millis > config->timeout.tokenGenerationError ||
         internal.fb_last_jwt_generation_error_cb_millis == 0))
    {
        internal.fb_last_jwt_generation_error_cb_millis = millis();
        ret = true;
    }

    return ret;
}

bool FirebaseCore::handleToken()
{

    // no config?, no auth? or no network?
    if (!config || !auth)
        return false;

    // bypass and set the token ready status for test mode
    if (config->signer.test_mode)
    {
        setTokenError(0);
        return true;
    }

    // time is up or expiey time reset or unset
    bool exp = isExpired();

    // Handle user assigned tokens (custom and access tokens)

    // if custom token was set
    if (config->signer.customTokenCustomSet)
    {
        if (exp)
        {
            // reset state and set type
            config->signer.tokens.status = token_status_uninitialized;
            config->signer.tokens.token_type = token_type_custom_token;

            bool ret = false;

            if (readyToRequest())
            {
                // request auth token using custom token
                ret = requestTokens(false);

                // if success, clear the custom token flag
                if (ret)
                    config->signer.customTokenCustomSet = false;
            }

            // return false for token request failed or true for success
            return ret;
        }

        // auth token is ready to use (unexpired)
        config->signer.tokens.status = token_status_ready;
        return true;
    }
    // if access token was set and unexpired, set the ready status
    else if (config->signer.accessTokenCustomSet && !exp)
    {
        config->signer.tokens.auth_type = firebase_pgm_str_45; // "Bearer "
        config->signer.tokens.status = token_status_ready;
        return true;
    }

    // Handle the signed jwt token generation, request and refresh the token

    // if token type is any and expiry time is up or reset/unset, start the process
    if (isAuthToken(true) && exp)
    {
        // if auth token that has been requested using id and custom token was expired
        if (config->signer.tokens.expires > 0 && isAuthToken(false))
        {
            if (readyToRequest())
            {
                // if id token was set without refresh token, we can't do anything further
                if (config->signer.idTokenCustomSet &&
                    internal.refresh_token.length() == 0 &&
                    auth->user.email.length() == 0 &&
                    auth->user.password.length() == 0 &&
                    config->signer.anonymous)
                    return true;

                // return when tcp client was used by other processes
                if (internal.fb_processing)
                    return false;

                // refresh new auth token
                if (readyToRefresh())
                    return refreshToken();
            }
            return false;
        }
        // if it is new auth request
        else
        {
            // if auth request using id token
            if (config->signer.tokens.token_type == token_type_id_token)
            {
                if (readyToRequest())
                    tokenProcessingTask();

                return false;
            }
            // if auth request using OAuth 2.0 and custom token
            else
            {
                // refresh the token when refresh token was assigned via setCustomToken (with non jwt token)
                // and setAccessToken (fourth argument)
                if (internal.refresh_token.length() > 0)
                    return requestTokens(true);

                // handle the jwt token processing

                // if it is the first step and no task is currently running
                if (!config->signer.tokenTaskRunning)
                {
                    if (config->signer.step == firebase_jwt_generation_step_begin)
                    {
                        // if service account key json file assigned and no private key parsing data
                        if (config->service_account.json.path.length() > 0 && config->signer.pk.length() == 0)
                        {
                            // if fail to parse the private key from service account json file, reset the token status
                            if (!parseSAFile())
                                config->signer.tokens.status = token_status_uninitialized;
                        }

                        // if no token status set, set the states
                        if (config->signer.tokens.status != token_status_on_initialize)
                        {
                            config->signer.tokens.status = token_status_on_initialize;
                            config->signer.tokens.error.code = 0;
                            config->signer.tokens.error.message.clear();
                            internal.fb_last_jwt_generation_error_cb_millis = 0;
                            sendTokenStatusCB();
                        }
                    }

                    tokenProcessingTask();
                }
            }
        }
    }

    // the legacy token has the lowest priority to check
    // and set the token ready status
    if (config->signer.tokens.token_type == token_type_legacy_token)
    {
        setTokenError(0);
        return true;
    }
    else
    {
        // missing credentials or required info
        if (config->signer.tokens.token_type == token_type_undefined)
        {
            config->signer.tokens.error.message.clear();
            setTokenError(FIREBASE_ERROR_MISSING_CREDENTIALS);
            sendTokenStatusCB();
        }

        return config->signer.tokens.status == token_status_ready;
    }

    return config->signer.tokens.status == token_status_ready;
}

void FirebaseCore::initJson()
{
    if (!jsonPtr)
        jsonPtr = new FirebaseJson();
    if (!resultPtr)
        resultPtr = new FirebaseJsonData();
}

void FirebaseCore::freeJson()
{
    if (jsonPtr)
        delete jsonPtr;
    if (resultPtr)
        delete resultPtr;
    jsonPtr = nullptr;
    resultPtr = nullptr;
}

uint32_t FirebaseCore::getTimestamp(int year, int mon, int date, int hour, int mins, int sec)
{
    struct tm timeinfo;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = mon - 1;
    timeinfo.tm_mday = date;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = mins;
    timeinfo.tm_sec = sec;
    uint32_t ts = mktime(&timeinfo);
    return ts;
}

void FirebaseCore::getBaseTime()
{
#if defined(ESP32) || defined(ESP8266) || defined(MB_ARDUINO_PICO)

    if (baseTs < time(nullptr))
        baseTs = time(nullptr);

#elif defined(FIREBASE_HAS_WIFI_TIME)
    if (WiFI_CONNECTED)
        baseTs = WiFi.getTime() > FIREBASE_DEFAULT_TS ? WiFi.getTime() : baseTs;
#else
    baseTs = tsOffset + millis() / 1000;
#endif

    if (baseTs > FIREBASE_DEFAULT_TS)
        tsOffset = baseTs - millis() / 1000;
}

int FirebaseCore::setTimestamp(time_t ts)
{
    internal.fb_base_time_type = ts > FIREBASE_DEFAULT_TS ? firebase_cfg_int_t::base_time_type_user : firebase_cfg_int_t::base_time_type_unset;
#if defined(MB_ARDUINO_ESP)
    struct timeval tm; // sec, us
    tm.tv_sec = ts;
    tm.tv_usec = 0;
    return settimeofday((const struct timeval *)&tm, 0);
#else
    tsOffset = ts - millis() / 1000;
    return 1;
#endif
}

bool FirebaseCore::timeReady()
{
    getBaseTime();
    return baseTs > FIREBASE_DEFAULT_TS;
}

void FirebaseCore::timeBegin()
{
    if (timeReady() && internal.fb_base_time_type == firebase_cfg_int_t::base_time_type_undefined)
        internal.fb_base_time_type = firebase_cfg_int_t::base_time_type_auto;
    else if (internal.fb_base_time_type == firebase_cfg_int_t::base_time_type_undefined)
        internal.fb_base_time_type = firebase_cfg_int_t::base_time_type_unset;

    if ((config->time_zone > FIREBASE_NON_TS && config->time_zone != internal.fb_gmt_offset) || (config->daylight_offset > FIREBASE_NON_TS && config->daylight_offset != internal.fb_daylight_offset))
    {
        // Reset system timestamp when config changed
        baseTs = 0;
        if (config->time_zone > FIREBASE_NON_TS)
            internal.fb_gmt_offset = config->time_zone;
        if (config->daylight_offset > FIREBASE_NON_TS)
            internal.fb_daylight_offset = config->daylight_offset;

        if (internal.fb_base_time_type == firebase_cfg_int_t::base_time_type_unset)
            setTimestamp(millis());
    }
}

void FirebaseCore::readNTPTime()
{

    internal.fb_clock_rdy = timeReady();

    if (!internal.fb_clock_rdy)
    {
        if (WiFI_CONNECTED)
        {

#if defined(FIREBASE_ENABLE_NTP_TIME) || defined(ENABLE_NTP_TIME)
#if (defined(ESP32) || defined(ESP8266))
            configTime(config->time_zone * 3600, config->daylight_offset * 60, "pool.ntp.org", "time.nist.gov");
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
            NTP.begin("pool.ntp.org", "time.nist.gov");
            NTP.waitSet();
#endif
#endif
            unsigned long ms = millis();

            do
            {
#if defined(FIREBASE_HAS_WIFI_TIME)
                baseTs = WiFi.getTime() > FIREBASE_DEFAULT_TS ? WiFi.getTime() : baseTs;
#elif defined(FIREBASE_ENABLE_NTP_TIME) || defined(ENABLE_NTP_TIME)
                baseTs = time(nullptr) > FIREBASE_DEFAULT_TS ? time(nullptr) : baseTs;
#else
                break;
#endif

                FBUtils::idle();
            } while (millis() - ms < 10000 && baseTs < FIREBASE_DEFAULT_TS);
        }
    }

    internal.fb_clock_rdy = baseTs > FIREBASE_DEFAULT_TS;
}

void FirebaseCore::tokenProcessingTask()
{
    // All sessions should be closed
    freeClient(&tcpClient);

    for (size_t i = 0; i < Core.internal.sessions.size(); i++)
    {
        if (Core.internal.sessions[i].status)
            return;
    }

    // return when task is currently running
    if (config->signer.tokenTaskRunning)
        return;

    bool ret = false;

    config->signer.tokenTaskRunning = true;

    timeBegin();

    time_t now = getTime();

    while (!ret && config->signer.tokens.status != token_status_ready)
    {

        FBUtils::idle();
        internal.fb_clock_rdy = timeReady();

        if (!internal.fb_clock_rdy && (config->cert.data != NULL || config->cert.file.length() > 0 || config->signer.tokens.token_type == token_type_oauth2_access_token || config->signer.tokens.token_type == token_type_custom_token))
        {
            int code = FIREBASE_ERROR_NTP_TIMEOUT;

            if (_cli_type == firebase_client_type_external_gsm_client)
            {
                if (!tcpClient)
                    newClient(&tcpClient);

                uint32_t _time = tcpClient->gprsGetTime();
                if (_time > 0)
                {
                    baseTs = _time;
                    setTimestamp(_time);
                }

                freeClient(&tcpClient);
            }
            else
            {
#if defined(FIREBASE_ENABLE_NTP_TIME) || defined(ENABLE_NTP_TIME)
                if (WiFI_CONNECTED)
                    readNTPTime();
                else
                    code = FIREBASE_ERROR_NO_WIFI_TIME;

#else
                code = FIREBASE_ERROR_USER_TIME_SETTING_REQUIRED;
#endif
            }

            if (readyToSync())
            {
                if (isSyncTimeOut())
                {
                    config->signer.tokens.error.message.clear();
                    setTokenError(code);
                    sendTokenStatusCB();
                    config->signer.tokens.status = token_status_on_initialize;
                    internal.fb_last_jwt_generation_error_cb_millis = 0;
                }
            }

            if (!internal.fb_clock_rdy)
            {
                config->signer.tokenTaskRunning = false;
                return;
            }
        }

        if (config->signer.tokens.token_type == token_type_id_token)
        {
            // email/password verification and get id token
            ret = getIdToken(false, toStringPtr(_EMPTY_STR), toStringPtr(_EMPTY_STR));

            // send error cb
            if (!reconnect() && isErrorCBTimeOut())
                handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);
        }
        else
        {
            // create signed JWT token and exchange with auth token
            if (config->signer.step == firebase_jwt_generation_step_begin &&
                (millis() - internal.fb_last_jwt_begin_step_millis > config->timeout.tokenGenerationBeginStep ||
                 internal.fb_last_jwt_begin_step_millis == 0))
            {

                internal.fb_last_jwt_begin_step_millis = millis();

                if (internal.fb_clock_rdy)
                    config->signer.step = firebase_jwt_generation_step_encode_header_payload;
            }
            // encode the JWT token
            else if (config->signer.step == firebase_jwt_generation_step_encode_header_payload)
            {
                if (createJWT())
                    config->signer.step = firebase_jwt_generation_step_sign;
            }
            // sign the JWT token
            else if (config->signer.step == firebase_jwt_generation_step_sign)
            {
                if (createJWT())
                    config->signer.step = firebase_jwt_generation_step_exchange;
            }
            // sending JWT token requst for auth token
            else if (config->signer.step == firebase_jwt_generation_step_exchange)
            {
                if (readyToRefresh())
                {
                    // sending a new request
                    ret = requestTokens(false);
                    config->signer.step = ret || getTime() - now > 3599 ? firebase_jwt_generation_step_begin : firebase_jwt_generation_step_exchange;
                    ret = true;

                    // send error cb
                    if (!reconnect() && isErrorCBTimeOut())
                        handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);
                }
            }
        }
    }

    // reset task running status
    config->signer.tokenTaskRunning = false;
}

bool FirebaseCore::refreshToken()
{
#if !defined(USE_LEGACY_TOKEN_ONLY) && !defined(FIREBASE_USE_LEGACY_TOKEN_ONLY)

    if (!config)
        return false;

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        internal.fb_processing)
        return false;

    if (internal.ltok_len > 0 || (internal.rtok_len == 0 && internal.atok_len == 0))
        return false;

    if (!initClient(firebase_auth_pgm_str_9 /* "securetoken" */, token_status_on_refresh))
        return false;

    jsonPtr->add(pgm2Str(firebase_auth_pgm_str_11 /* "grantType" */), pgm2Str(firebase_auth_pgm_str_12 /* "refresh_token" */));
    jsonPtr->add(pgm2Str(firebase_auth_pgm_str_13 /* "refreshToken" */), internal.refresh_token.c_str());

    MB_String req;
    hh.addRequestHeaderFirst(req, http_post);

    req += firebase_auth_pgm_str_10; // "/v1/token?Key=""
    req += config->api_key;
    hh.addRequestHeaderLast(req);

    hh.addGAPIsHostHeader(req, firebase_auth_pgm_str_9 /* "securetoken" */);
    hh.addUAHeader(req);
    hh.addContentLengthHeader(req, strlen(jsonPtr->raw()));
    hh.addContentTypeHeader(req, firebase_pgm_str_62 /* "application/json" */);
    hh.addNewLine(req);

    req += jsonPtr->raw(); // {"grantType":"refresh_token","refreshToken":"<refresh token>"}

    tcpClient->send(req.c_str());

    req.clear();
    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    struct firebase_auth_token_error_t error;

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            config->signer.tokens.status = token_status_error;

            if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        internal.fb_last_jwt_generation_error_cb_millis = 0;
        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_14 /* "id_token" */))
            {
                internal.auth_token = resultPtr->to<const char *>();
                internal.atok_len = strlen(resultPtr->to<const char *>());
                internal.ltok_len = 0;
            }

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_12 /* "refresh_token" */))
            {
                internal.refresh_token = resultPtr->to<const char *>();
                internal.rtok_len = strlen(resultPtr->to<const char *>());
            }

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_15 /* "expires_in" */))
                getExpiration(resultPtr->to<const char *>());

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_16 /* "user_id" */))
                auth->token.uid = resultPtr->to<const char *>();

            return handleTaskError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }

        return handleTaskError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);

#endif

    return true;
}

void FirebaseCore::newClient(Firebase_TCP_Client **client)
{
    freeClient(client);
    if (!*client)
    {

        *client = new Firebase_TCP_Client();

        if (_cli_type == firebase_client_type_external_generic_client)
            (*client)->setClient(_cli, _net_con_cb, _net_stat_cb);
        else if (_cli_type == firebase_client_type_external_ethernet_client)
        {
            (*client)->setEthernetClient(_cli, _ethernet_mac, _ethernet_cs_pin, _ethernet_reset_pin, _static_ip);
        }
        else if (_cli_type == firebase_client_type_external_gsm_client)
        {
#if defined(FIREBASE_GSM_MODEM_IS_AVAILABLE)
            (*client)->setGSMClient(_cli, _modem, _pin.c_str(), _apn.c_str(), _user.c_str(), _password.c_str());
#endif
        }
        else
            (*client)->_client_type = _cli_type;
    }
}

void FirebaseCore::freeClient(Firebase_TCP_Client **client)
{
    if (*client)
    {

        _cli_type = (*client)->type();
        _cli = (*client)->_basic_client;
        if (_cli_type == firebase_client_type_external_generic_client)
        {
            _net_con_cb = (*client)->_network_connection_cb;
            _net_stat_cb = (*client)->_network_status_cb;
        }
        else if (_cli_type == firebase_client_type_external_ethernet_client)
        {
            _ethernet_mac = (*client)->_ethernet_mac;
            _ethernet_cs_pin = (*client)->_ethernet_cs_pin;
            _ethernet_reset_pin = (*client)->_ethernet_reset_pin;
            _static_ip = (*client)->_static_ip;
        }
        else if (_cli_type == firebase_client_type_external_gsm_client)
        {
#if defined(FIREBASE_GSM_MODEM_IS_AVAILABLE)
            _modem = (*client)->_modem;
            _pin = (*client)->_pin;
            _apn = (*client)->_apn;
            _user = (*client)->_user;
            _password = (*client)->_password;
#endif
        }

        // Only internal client can be deleted
        if (_cli_type == firebase_client_type_internal_basic_client)
            delete *client;
    }

    // Reset pointer in case internal client
    if (_cli_type == firebase_client_type_internal_basic_client)
        *client = nullptr;
}

void FirebaseCore::setTokenError(int code)
{
    if (code != 0)
        config->signer.tokens.status = token_status_error;
    else
    {
        config->signer.tokens.error.message.clear();
        config->signer.tokens.status = token_status_ready;
    }

    config->signer.tokens.error.code = code;

    if (config->signer.tokens.error.message.length() == 0)
    {
        internal.fb_processing = false;
        errorToString(code, config->signer.tokens.error.message);
    }
}

bool FirebaseCore::handleTaskError(int code, int httpCode)
{
    // Close TCP connection and unlock used flag

    if (tcpClient)
        tcpClient->stop();

    internal.fb_processing = false;

    switch (code)
    {

    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:

        // Show error based on connection status
        config->signer.tokens.error.message.clear();
        setTokenError(code);
        internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();
        break;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT:

        // Request time out?
        if (httpCode == 0)
        {
            // Show error based on request time out
            setTokenError(code);
        }
        else
        {
            // Show error from response http code
            errorToString(httpCode, config->signer.tokens.error.message);
            setTokenError(httpCode);
        }

        internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();

        break;

    default:
        break;
    }

    // Free memory
    freeClient(&tcpClient);

    freeJson();

    // reset token processing state
    if (code == FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY || code == FIREBASE_ERROR_TOKEN_COMPLETE_UNNOTIFY)
    {
        config->signer.tokens.error.message.clear();
        config->signer.tokens.status = token_status_ready;
        config->signer.step = firebase_jwt_generation_step_begin;
        internal.fb_last_jwt_generation_error_cb_millis = 0;
        if (code == FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY)
            sendTokenStatusCB();

        return true;
    }

    return false;
}

void FirebaseCore::sendTokenStatusCB()
{
    tokenInfo.status = config->signer.tokens.status;
    tokenInfo.type = config->signer.tokens.token_type;
    tokenInfo.error = config->signer.tokens.error;

    if (config->token_status_callback && isErrorCBTimeOut())
        config->token_status_callback(tokenInfo);
}

bool FirebaseCore::handleTokenResponse(int &httpCode)
{

    if (!reconnect(tcpClient, nullptr))
        return false;

    MB_String header, payload;

    struct server_response_data_t response;
    struct firebase_tcp_response_handler_t tcpHandler;

    hh.intTCPHandler(tcpClient, tcpHandler, 2048, 2048, nullptr, false);

    while (tcpClient->connected() && tcpClient->available() == 0)
    {
        FBUtils::idle();
        if (!reconnect(tcpClient, nullptr, tcpHandler.dataTime))
            return false;
    }

    bool complete = false;

    tcpHandler.chunkBufSize = tcpHandler.defaultChunkSize;

    char *pChunk = reinterpret_cast<char *>(mbfs.newP(tcpHandler.chunkBufSize + 1));

    while (tcpHandler.available() || !complete)
    {
        FBUtils::idle();

        if (!reconnect(tcpClient, nullptr, tcpHandler.dataTime))
            return false;

        if (!hh.readStatusLine(&sh, &mbfs, tcpClient, tcpHandler, response))
        {

            // The next chunk data can be the remaining http header
            if (tcpHandler.isHeader)
            {
                // Read header, complete?
                if (hh.readHeader(&sh, &mbfs, tcpClient, tcpHandler, response))
                {
                    if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                        tcpHandler.error.code = 0;

                    if (ut.isNoContent(&response))
                        break;
                }
            }
            else
            {
                memset(pChunk, 0, tcpHandler.chunkBufSize + 1);

                // Read the avilable data
                // chunk transfer encoding?
                if (response.isChunkedEnc)
                    tcpHandler.bufferAvailable = hh.readChunkedData(&sh, &mbfs, tcpClient,
                                                                    pChunk, nullptr, tcpHandler);
                else
                    tcpHandler.bufferAvailable = hh.readLine(tcpClient,
                                                             pChunk, tcpHandler.chunkBufSize);

                if (tcpHandler.bufferAvailable > 0)
                {
                    tcpHandler.payloadRead += tcpHandler.bufferAvailable;
                    payload += pChunk;
                }

                if (ut.isChunkComplete(&tcpHandler, &response, complete) ||
                    ut.isResponseComplete(&tcpHandler, &response, complete))
                    break;
            }
        }

        if (ut.isResponseTimeout(&tcpHandler, complete))
            break;
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        tcpClient->flush();

    mbfs.delP(&pChunk);

    if (tcpClient->connected())
        tcpClient->stop();

    httpCode = response.httpCode;

    if (jsonPtr && payload.length() > 0 && !response.noContent)
    {
        // Just a simple JSON which is suitable for parsing in low memory device
        jsonPtr->setJsonData(payload.c_str());
        payload.clear();
        return true;
    }

    return false;
}

bool FirebaseCore::handleError(int code, const char *descr, int errNum)
{
    setTokenError(code);
    config->signer.tokens.error.message.insert(0, descr);
    sendTokenStatusCB();
    return false;
}

bool FirebaseCore::createJWT()
{

#if !defined(USE_LEGACY_TOKEN_ONLY) && !defined(FIREBASE_USE_LEGACY_TOKEN_ONLY)

    if (config->signer.step == firebase_jwt_generation_step_encode_header_payload)
    {
        config->signer.tokens.status = token_status_on_signing;
        config->signer.tokens.error.code = 0;
        config->signer.tokens.error.message.clear();
        internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();

        initJson();

        time_t now = getTime();

        config->signer.tokens.jwt.clear();

        // header
        // {"alg":"RS256","typ":"JWT"}
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_17 /* "alg" */), pgm2Str(firebase_auth_pgm_str_29 /* "RS256" */));
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_18 /* "typ" */), pgm2Str(firebase_auth_pgm_str_28 /* "JWT" */));

        size_t len = bh.encodedLength(strlen(jsonPtr->raw()));
        char *buf = reinterpret_cast<char *>(mbfs.newP(len));
        bh.encodeUrl(&mbfs, buf, (unsigned char *)jsonPtr->raw(), strlen(jsonPtr->raw()));
        config->signer.encHeader = buf;
        mbfs.delP(&buf);
        config->signer.encHeadPayload = config->signer.encHeader;

        // payload
        // {"iss":"<email>","sub":"<email>","aud":"<audience>","iat":<timstamp>,"exp":<expire>,"scope":"<scope>"}
        // {"iss":"<email>","sub":"<email>","aud":"<audience>","iat":<timstamp>,"exp":<expire>,"uid":"<uid>","claims":"<claims>"}
        jsonPtr->clear();
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_30 /* "iss" */), config->service_account.data.client_email.c_str());
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_31 /* "sub" */), config->service_account.data.client_email.c_str());

        MB_String t = firebase_pgm_str_22; // "https://"
        if (config->signer.tokens.token_type == token_type_custom_token)
        {
            hh.addGAPIsHost(t, firebase_auth_pgm_str_23 /* "identitytoolkit" */);
            t += firebase_auth_pgm_str_34; // "/google.identity.identitytoolkit.v1.IdentityToolkit"
        }
        else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            hh.addGAPIsHost(t, firebase_auth_pgm_str_35 /* "oauth2" */);
            t += firebase_pgm_str_1;  // "/"
            t += firebase_pgm_str_18; // "token"
        }

        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_32 /* "aud" */), t.c_str());
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_36 /* "iat" */), (int)now);

        if (config->signer.expiredSeconds > 3600)
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_33 /* "exp" */), (int)(now + 3600));
        else
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_33 /* "exp" */), (int)(now + config->signer.expiredSeconds));

        if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            MB_String buri;
            MB_String host;
            hh.addGAPIsHost(host, firebase_pgm_str_61 /* "www" */);
            uh.host2Url(buri, host);
            buri += firebase_pgm_str_1;       // "/"
            buri += firebase_auth_pgm_str_37; // "auth"
            buri += firebase_pgm_str_1;       // "/"

            MB_String s = buri;            // https://www.googleapis.com/auth/
            s += firebase_auth_pgm_str_38; // "devstorage.full_control"

            s += firebase_pgm_str_9;       // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += firebase_auth_pgm_str_39; // "datastore"

            s += firebase_pgm_str_9;       // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += firebase_auth_pgm_str_40; // "userinfo.email"

            s += firebase_pgm_str_9;       // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += firebase_auth_pgm_str_41; // "firebase.database"

            s += firebase_pgm_str_9;       // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += firebase_auth_pgm_str_42; // "cloud-platform"
#if defined(Firebase_TCP_Client)
            s += firebase_pgm_str_9;       // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += firebase_auth_pgm_str_19; // "iam"
#endif

            if (config->signer.tokens.scope.length() > 0)
            {
                MB_VECTOR<MB_String> scopes;
                sh.splitTk(config->signer.tokens.scope, scopes, ",");
                for (size_t i = 0; i < scopes.size(); i++)
                {
                    s += firebase_pgm_str_9; // " "
                    s += scopes[i];
                    scopes[i].clear();
                }
                scopes.clear();
            }

            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_20 /* "scope" */), s.c_str());
        }
        else if (config->signer.tokens.token_type == token_type_custom_token)
        {
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_21 /* "uid" */), auth->token.uid.c_str());

            if (auth->token.claims.length() > 2)
            {
                FirebaseJson claims(auth->token.claims.c_str());
                jsonPtr->add(pgm2Str(firebase_auth_pgm_str_22 /* "claims" */), claims);
            }
        }

        len = bh.encodedLength(strlen(jsonPtr->raw()));
        buf = reinterpret_cast<char *>(mbfs.newP(len));
        bh.encodeUrl(&mbfs, buf, (unsigned char *)jsonPtr->raw(), strlen(jsonPtr->raw()));
        config->signer.encPayload = buf;
        mbfs.delP(&buf);

        config->signer.encHeadPayload += firebase_auth_pgm_str_4; // "."
        config->signer.encHeadPayload += config->signer.encPayload;

        config->signer.encHeader.clear();
        config->signer.encPayload.clear();

        // create message digest from encoded header and payload

        config->signer.hash = reinterpret_cast<char *>(mbfs.newP(config->signer.hashSize));
        br_sha256_context mc;
        br_sha256_init(&mc);
        br_sha256_update(&mc, config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length());
        br_sha256_out(&mc, config->signer.hash);

        config->signer.tokens.jwt = config->signer.encHeadPayload;
        config->signer.tokens.jwt += firebase_auth_pgm_str_4; // "."
        config->signer.encHeadPayload.clear();

        freeJson();
    }
    else if (config->signer.step == firebase_jwt_generation_step_sign)
    {
        config->signer.tokens.status = token_status_on_signing;

        // RSA private key
        PrivateKey *pk = nullptr;
        FBUtils::idle();
        // parse priv key
        if (config->signer.pk.length() > 0)
            pk = new PrivateKey((const char *)config->signer.pk.c_str());
        else if (strlen_P(config->service_account.data.private_key) > 0)
            pk = new PrivateKey((const char *)config->service_account.data.private_key);

        if (!pk)
            return handleError(FIREBASE_ERROR_TOKEN_PARSE_PK, (const char *)FPSTR("BearSSL, PrivateKey: "));

        if (!pk->isRSA())
        {
            delete pk;
            pk = nullptr;
            return handleError(FIREBASE_ERROR_TOKEN_PARSE_PK, (const char *)FPSTR("BearSSL, isRSA: "));
        }

        const br_rsa_private_key *br_rsa_key = pk->getRSA();

        // generate RSA signature from private key and message digest
        config->signer.signature = new unsigned char[config->signer.signatureSize];

        FBUtils::idle();
        int ret = br_rsa_i15_pkcs1_sign(BR_HASH_OID_SHA256, (const unsigned char *)config->signer.hash,
                                        br_sha256_SIZE, br_rsa_key, config->signer.signature);
        FBUtils::idle();
        mbfs.delP(&config->signer.hash);

        size_t len = bh.encodedLength(config->signer.signatureSize);
        char *buf = reinterpret_cast<char *>(mbfs.newP(len));
        bh.encodeUrl(&mbfs, buf, config->signer.signature, config->signer.signatureSize);
        config->signer.encSignature = buf;
        mbfs.delP(&buf);
        mbfs.delP(&config->signer.signature);
        delete pk;
        pk = nullptr;

        // get the signed JWT
        if (ret > 0)
        {
            config->signer.tokens.jwt += config->signer.encSignature;
            config->signer.pk.clear();
            config->signer.encSignature.clear();
        }
        else
            return handleError(FIREBASE_ERROR_TOKEN_SIGN, (const char *)FPSTR("BearSSL, br_rsa_i15_pkcs1_sign: "));
    }

#endif

    return true;
}

bool FirebaseCore::getIdToken(bool createUser, MB_StringPtr email, MB_StringPtr password)
{
#if !defined(USE_LEGACY_TOKEN_ONLY) && !defined(FIREBASE_USE_LEGACY_TOKEN_ONLY)

    config->signer.signup = false;

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        internal.fb_processing)
        return false;

    if (!initClient(createUser
                        ? firebase_auth_pgm_str_23 /* "identitytoolkit" */
                        : firebase_pgm_str_61 /* "www" */,
                    createUser
                        ? token_status_uninitialized
                        : token_status_on_request))
        return false;

    if (createUser)
    {
        MB_String _email = email, _password = password;
        config->signer.signupError.message.clear();

        if (_email.length() > 0 && _password.length() > 0)
        {
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_24 /* "email" */), _email);
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_25 /* "password" */), _password);
        }
    }
    else
    {
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_24 /* "email" */), auth->user.email.c_str());
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_25 /* "password" */), auth->user.password.c_str());
    }

    jsonPtr->add(pgm2Str(firebase_auth_pgm_str_26 /* "returnSecureToken" */), true);

    MB_String req;
    hh.addRequestHeaderFirst(req, http_post);

    if (createUser)
        req += firebase_auth_pgm_str_27; // "/v1/accounts:signUp?key="
    else
    {
        req += firebase_auth_pgm_str_43; // "/identitytoolkit/v3/relyingparty/"
        req += firebase_auth_pgm_str_44; // "verifyPassword?key="
    }

    req += config->api_key;
    hh.addRequestHeaderLast(req);

    if (createUser)
        hh.addGAPIsHostHeader(req, firebase_auth_pgm_str_23 /* "identitytoolkit" */);
    else
        hh.addGAPIsHostHeader(req, firebase_pgm_str_61 /* "www" */);

    hh.addUAHeader(req);
    hh.addContentLengthHeader(req, strlen(jsonPtr->raw())); // {"email":"<email>","password":"<password>","returnSecureToken":true}
    hh.addContentTypeHeader(req, firebase_pgm_str_62 /* "application/json" */);
    hh.addNewLine(req);
    req += jsonPtr->raw();

    tcpClient->send(req.c_str());

    req.clear();

    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        struct firebase_auth_token_error_t error;

        if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            if (!createUser)
                config->signer.tokens.status = token_status_error;

            if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }

        if (createUser)
            config->signer.signupError = error;
        else
        {
            config->signer.tokens.error = error;
            tokenInfo.status = config->signer.tokens.status;
            tokenInfo.type = config->signer.tokens.token_type;
            tokenInfo.error = config->signer.tokens.error;
            internal.fb_last_jwt_generation_error_cb_millis = 0;
            if (error.code != 0)
                sendTokenStatusCB();
        }

        if (error.code == 0)
        {
            if (createUser)
            {
                config->signer.signup = true;
                config->signer.tokens.token_type = token_type_id_token;
                auth->user.email = email;
                auth->user.password = password;
                config->signer.anonymous = auth->user.email.length() == 0 && auth->user.password.length() == 0;
            }

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_45 /* "idToken" */))
            {
                internal.auth_token = resultPtr->to<const char *>();
                internal.atok_len = strlen(resultPtr->to<const char *>());
                internal.ltok_len = 0;
            }

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_46 /* "refreshToken" */))
            {
                internal.refresh_token = resultPtr->to<const char *>();
                internal.rtok_len = strlen(resultPtr->to<const char *>());
            }

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_47 /* "expiresIn" */))
                getExpiration(resultPtr->to<const char *>());

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_48 /* "localId" */))
                auth->token.uid = resultPtr->to<const char *>();

            if (!createUser)
                return handleTaskError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
            else
                return handleTaskError(FIREBASE_ERROR_TOKEN_COMPLETE_UNNOTIFY);
        }
    }

    return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);

#endif
    return true;
}

bool FirebaseCore::deleteIdToken(MB_StringPtr idToken)
{
#if !defined(USE_LEGACY_TOKEN_ONLY) && !defined(FIREBASE_USE_LEGACY_TOKEN_ONLY)

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        internal.fb_processing)
        return false;

    if (!initClient(firebase_auth_pgm_str_23 /* "identitytoolkit" */, token_status_uninitialized))
        return false;

    config->signer.signup = false;
    internal.fb_processing = true;

    MB_String _idToken = idToken;
    if (_idToken.length() > 0)
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_45 /* "idToken" */), _idToken);
    else
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_45 /* "idToken" */), internal.auth_token);

    MB_String req;
    hh.addRequestHeaderFirst(req, http_post);
    req += firebase_auth_pgm_str_49; //"/v1/accounts:delete?key="
    req += config->api_key;
    hh.addRequestHeaderLast(req);
    hh.addGAPIsHostHeader(req, firebase_auth_pgm_str_23 /* "identitytoolkit" */);
    hh.addUAHeader(req);
    hh.addContentLengthHeader(req, strlen(jsonPtr->raw()));
    hh.addContentTypeHeader(req, firebase_pgm_str_62 /* "application/json" */);
    hh.addNewLine(req);

    req += jsonPtr->raw(); // {"idToken":"<id token>"}

    tcpClient->send(req.c_str());
    req.clear();
    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        struct firebase_auth_token_error_t error;

        if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }

        config->signer.deleteError = error;

        if (error.code == 0)
        {
            if (_idToken.length() == 0 || strcmp(internal.auth_token.c_str(), _idToken.c_str()) == 0)
            {
                internal.auth_token.clear();
                internal.atok_len = 0;
                internal.ltok_len = 0;
                config->signer.tokens.expires = 0;
                config->signer.step = firebase_jwt_generation_step_begin;
                internal.fb_last_jwt_generation_error_cb_millis = 0;
                config->signer.tokens.token_type = token_type_undefined;
                config->signer.anonymous = false;
                config->signer.idTokenCustomSet = false;
            }

            return true;
        }
    }

    return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);

#endif

    return true;
}

void FirebaseCore::setAutoReconnectNetwork(bool reconnect)
{
    autoReconnectNetwork = reconnect;
}

void FirebaseCore::setTCPClient(Firebase_TCP_Client *tcpClient)
{
    freeClient(&tcpClient);
    this->tcpClient = tcpClient;
}

void FirebaseCore::setNetworkStatus(bool status)
{
    networkStatus = status;
    if (tcpClient)
        tcpClient->setNetworkStatus(networkStatus);
}

void FirebaseCore::closeSession(Firebase_TCP_Client *client, firebase_session_info_t *session)
{

    // close the socket and free the resources used by the SSL engine
    internal.fb_last_reconnect_millis = millis();

    if (client)
        client->stop();

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (session && session->con_mode == firebase_con_mode_rtdb_stream)
    {
        session->rtdb.stream_tmo_Millis = millis();
        session->rtdb.data_millis = millis();
        session->rtdb.data_tmo = false;
        session->rtdb.new_stream = true;
    }
#endif
}

bool FirebaseCore::reconnect(Firebase_TCP_Client *client, firebase_session_info_t *session, unsigned long dataTime)
{

    if (!client)
        return false;

    client->setConfig(config, &mbfs);

    networkStatus = client->networkReady();

    if (networkStatus && dataTime > 0)
    {
        unsigned long tmo = DEFAULT_SERVER_RESPONSE_TIMEOUT;
        if (config)
        {
            if (config->timeout.serverResponse < MIN_SERVER_RESPONSE_TIMEOUT ||
                config->timeout.serverResponse > MAX_SERVER_RESPONSE_TIMEOUT)
                config->timeout.serverResponse = DEFAULT_SERVER_RESPONSE_TIMEOUT;
            tmo = config->timeout.serverResponse;
        }

        if (millis() - dataTime > tmo)
        {
            if (session)
            {
                session->response.code = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
                errorToString(session->response.code, session->error);
                closeSession(client, session);
            }
            return false;
        }
    }

    if (!networkStatus)
    {
        if (session)
        {
            closeSession(client, session);
            session->response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;
        }

        if (config && config->timeout.wifiReconnect > 0)
            config->timeout.networkReconnect = config->timeout.wifiReconnect;

        resumeNetwork(client, config ? internal.net_once_connected : net_once_connected,
                      config ? internal.fb_last_reconnect_millis : last_reconnect_millis,
                      config ? config->timeout.networkReconnect : net_reconnect_tmo);

        networkStatus = client->networkReady();
    }

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (session)
    {
        if (!networkStatus && session->con_mode == firebase_con_mode_rtdb_stream)
            session->rtdb.new_stream = true;
    }
#endif

    if (networkStatus && config)
        internal.net_once_connected = true;

    if (!networkStatus && session)
        session->response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

    return networkStatus;
}

void FirebaseCore::resumeNetwork(Firebase_TCP_Client *client, bool &net_once_connected, unsigned long &last_reconnect_millis, uint16_t &net_reconnect_tmo)
{

    if (autoReconnectNetwork || !net_once_connected)
    {
        if (net_reconnect_tmo < MIN_NET_RECONNECT_TIMEOUT ||
            net_reconnect_tmo > MAX_NET_RECONNECT_TIMEOUT)
            net_reconnect_tmo = MIN_NET_RECONNECT_TIMEOUT;

        if (millis() - last_reconnect_millis > net_reconnect_tmo)
        {
            client->networkReconnect();
            last_reconnect_millis = millis();
        }
    }
}

bool FirebaseCore::reconnect()
{
    if (networkChecking)
        return networkStatus;

    networkChecking = true;

    bool noClient = tcpClient == nullptr;

    // We need tcpClient for network checking and reconnect here,
    // otherwise the networkStatus will not update
    // and network cannot resume.

    if (noClient)
        newClient(&tcpClient);

    reconnect(tcpClient, nullptr);

    if (noClient)
        freeClient(&tcpClient);

    networkChecking = false;

    if (!networkStatus && config->signer.tokens.status == token_status_on_refresh)
    {
        config->signer.tokens.error.message.clear();
        setTokenError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);
        internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();
    }

    return networkStatus;
}

bool FirebaseCore::initClient(PGM_P subDomain, firebase_auth_token_status status)
{

    FBUtils::idle();

    if (status != token_status_uninitialized)
    {
        config->signer.tokens.status = status;
        internal.fb_processing = true;
        config->signer.tokens.error.code = 0;
        config->signer.tokens.error.message.clear();
        internal.fb_last_jwt_generation_error_cb_millis = 0;
        internal.fb_last_request_token_cb_millis = millis();
        sendTokenStatusCB();
    }

    if (!tcpClient)
        newClient(&tcpClient);

    // Stop TCP session
    tcpClient->stop();

    tcpClient->setCACert(nullptr);

    if (!reconnect(tcpClient, nullptr))
        return false;

    if (!tcpClient->isInitialized())
        return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED);

    tcpClient->setBufferSizes(2048, 1024);

    initJson();

    MB_String host;
    hh.addGAPIsHost(host, subDomain);

    FBUtils::idle();
    tcpClient->setSession(&bsslSession);
    tcpClient->begin(host.c_str(), 443, &response_code);

    return true;
}

bool FirebaseCore::requestTokens(bool refresh)
{

#if !defined(USE_LEGACY_TOKEN_ONLY) && !defined(FIREBASE_USE_LEGACY_TOKEN_ONLY)

    time_t now = getTime();

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        ((unsigned long)now < FIREBASE_DEFAULT_TS && !refresh && !config->signer.customTokenCustomSet) ||
        internal.fb_processing)
        return false;

    if (!initClient(firebase_pgm_str_61 /* "www" */, refresh ? token_status_on_refresh : token_status_on_request))
        return false;

    MB_String req;
    hh.addRequestHeaderFirst(req, http_post);

    if (config->signer.tokens.token_type == token_type_custom_token)
    {
        // {"token":"<sutom or signed jwt token>","returnSecureToken":true}
        if (config->signer.customTokenCustomSet)
            jsonPtr->add(pgm2Str(firebase_pgm_str_18 /* "token" */), internal.auth_token.c_str());
        else
            jsonPtr->add(pgm2Str(firebase_pgm_str_18 /* "token" */), config->signer.tokens.jwt.c_str());

        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_26 /* "returnSecureToken" */), true);

        req += firebase_auth_pgm_str_43; // "/identitytoolkit/v3/relyingparty/"
        req += firebase_auth_pgm_str_50; // "verifyCustomToken?key="
        req += config->api_key;
        hh.addRequestHeaderLast(req);
        hh.addGAPIsHostHeader(req, firebase_pgm_str_61 /* "www" */);
    }
    else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
    {
        if (refresh)
        {
            // {"client_id":"<client id>","client_secret":"<client secret>","grant_type":"refresh_token",
            // "refresh_token":"<refresh token>"}
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_8 /* "client_id" */), internal.client_id.c_str());
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_51 /* "client_secret" */), internal.client_secret.c_str());

            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_52 /* "grant_type" */), pgm2Str(firebase_auth_pgm_str_12 /* "refresh_token" */));
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_12 /* "refresh_token" */), internal.refresh_token.c_str());
        }
        else
        {

            // rfc 7523, JWT Bearer Token Grant Type Profile for OAuth 2.0

            // {"grant_type":"urn:ietf:params:oauth:grant-type:jwt-bearer","assertion":"<signed jwt token>"}
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_52 /* "grant_type" */),
                         pgm2Str(firebase_auth_pgm_str_53 /* "urn:ietf:params:oauth:grant-type:jwt-bearer" */));
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_54 /* "assertion" */), config->signer.tokens.jwt.c_str());
        }

        req += firebase_pgm_str_1;  // "/"
        req += firebase_pgm_str_18; // "token"
        hh.addRequestHeaderLast(req);
        hh.addGAPIsHostHeader(req, firebase_auth_pgm_str_35 /* "aouth2" */);
    }

    hh.addUAHeader(req);
    hh.addContentLengthHeader(req, strlen(jsonPtr->raw()));
    hh.addContentTypeHeader(req, firebase_pgm_str_62 /* "application/json" */);
    hh.addNewLine(req);

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());

    req.clear();

    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    struct firebase_auth_token_error_t error;

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        config->signer.tokens.jwt.clear();
        if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            config->signer.tokens.status = token_status_error;

            if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }
        else if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_55 /* "error" */))
        {
            error.code = -1;
            config->signer.tokens.status = token_status_error;

            if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_56 /* "error_description" */))
                error.message = resultPtr->to<const char *>();
        }

        if (error.code != 0 && !config->signer.customTokenCustomSet &&
            (config->signer.tokens.token_type == token_type_custom_token ||
             config->signer.tokens.token_type == token_type_oauth2_access_token))
        {
            // new jwt needed as it is already cleared
            config->signer.step = firebase_jwt_generation_step_encode_header_payload;
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        internal.fb_last_jwt_generation_error_cb_millis = 0;

        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {
            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_45 /* "idToken" */))
                {
                    internal.auth_token = resultPtr->to<const char *>();
                    internal.atok_len = strlen(resultPtr->to<const char *>());
                    internal.ltok_len = 0;
                }

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_46 /* "refreshToken" */))
                {
                    internal.refresh_token = resultPtr->to<const char *>();
                    internal.rtok_len = strlen(resultPtr->to<const char *>());
                }

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_47 /* "expiresIn" */))
                    getExpiration(resultPtr->to<const char *>());
            }
            else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
            {
                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_57 /* "access_token" */))
                {
                    internal.auth_token = resultPtr->to<const char *>();
                    internal.atok_len = strlen(resultPtr->to<const char *>());
                    internal.ltok_len = 0;
                }

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_58 /* "token_type" */))
                    config->signer.tokens.auth_type = resultPtr->to<const char *>();

                if (jh.parse(jsonPtr, resultPtr, firebase_auth_pgm_str_15 /* "expires_in" */))
                    getExpiration(resultPtr->to<const char *>());
            }
            return handleTaskError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }
        return handleTaskError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);

#endif

    return true;
}

void FirebaseCore::getExpiration(const char *exp)
{
    time_t now = getTime();
    unsigned long ms = millis();
    config->signer.tokens.expires = now + atoi(exp);
    config->signer.tokens.last_millis = ms;
}

bool FirebaseCore::handleEmailSending(MB_StringPtr payload, firebase_user_email_sending_type type)
{
#if !defined(USE_LEGACY_TOKEN_ONLY) && !defined(FIREBASE_USE_LEGACY_TOKEN_ONLY)

    if (internal.fb_processing)
        return false;

    if (!initClient(firebase_pgm_str_61 /* "www" */, token_status_uninitialized))
        return false;

    MB_String _payload = payload;

    internal.fb_processing = true;

    if (type == firebase_user_email_sending_type_verify)
    {
        config->signer.verificationError.message.clear();
        // {"requestType":"VERIFY_EMAIL","idToken":"<id token>"}
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_59 /* "requestType" */), pgm2Str(firebase_auth_pgm_str_60 /* "VERIFY_EMAIL" */));
    }
    else if (type == firebase_user_email_sending_type_reset_psw)
    {
        config->signer.resetPswError.message.clear();
        // {"requestType":"PASSWORD_RESET","email":"<email>"}
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_59 /* "requestType" */), pgm2Str(firebase_auth_pgm_str_62 /* "PASSWORD_RESET" */));
    }

    if (type == firebase_user_email_sending_type_verify)
    {
        if (_payload.length() > 0)
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_45 /* "idToken" */), _payload);
        else
            jsonPtr->add(pgm2Str(firebase_auth_pgm_str_45 /* "idToken" */), internal.auth_token.c_str());
    }
    else if (type == firebase_user_email_sending_type_reset_psw)
    {
        jsonPtr->add(pgm2Str(firebase_auth_pgm_str_24 /* "email" */), _payload);
    }

    MB_String req;
    hh.addRequestHeaderFirst(req, http_post);
    req += firebase_auth_pgm_str_43; // "/identitytoolkit/v3/relyingparty/"
    req += firebase_auth_pgm_str_61; // "getOobConfirmationCode?key="
    req += config->api_key;
    hh.addRequestHeaderLast(req);
    hh.addGAPIsHostHeader(req, firebase_pgm_str_61 /* "www" */);
    hh.addUAHeader(req);
    hh.addContentLengthHeader(req, strlen(jsonPtr->raw()));
    hh.addContentTypeHeader(req, firebase_pgm_str_62 /* "application/json" */);
    hh.addNewLine(req);

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());
    req.clear();
    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        struct firebase_auth_token_error_t error;

        if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            if (jh.parse(jsonPtr, resultPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }
        if (type == firebase_user_email_sending_type_verify)
            config->signer.verificationError = error;
        else if (type == firebase_user_email_sending_type_reset_psw)
            config->signer.resetPswError = error;

        if (error.code == 0)
        {
            jsonPtr->clear();
            return handleTaskError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }

        return handleTaskError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);

#endif

    return true;
}

bool FirebaseCore::checkToken()
{
    if (!config || !auth)
        return false;

    if (isAuthToken(true) && isExpired())
        handleToken();

    return config->signer.tokens.status == token_status_ready;
}

bool FirebaseCore::tokenReady()
{
    if (!config)
        return false;

    checkToken();

    // call checkToken to send callback before checking connection.
    if (!reconnect())
        return false;

    return config->signer.tokens.status == token_status_ready;
};

void FirebaseCore::errorToString(int httpCode, MB_String &buff)
{
    buff.clear();

    if (config)
    {
        if (&config->signer.tokens.error.message != &buff &&
            (config->signer.tokens.status == token_status_error || config->signer.tokens.error.code != 0))
        {
            buff = config->signer.tokens.error.message;
            return;
        }
    }

#if defined(ENABLE_ERROR_STRING) || defined(FIREBASE_ENABLE_ERROR_STRING)

    // The httpcode (session.response.code of FirebaseData) passes to this function matches every case here.
    switch (httpCode)
    {

    case FIREBASE_ERROR_SSL_RX_BUFFER_SIZE_TOO_SMALL:
        buff += firebase_ssl_err_pgm_str_1; // "incomplete SSL client data"
        return;

    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED:
        buff += firebase_client_err_pgm_str_2; // "connection refused"
        return;
    case FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED:
        buff += firebase_client_err_pgm_str_3; // "send request failed"
        return;
    case FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED:
        buff += firebase_client_err_pgm_str_4; // "not connected"
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:
        buff += firebase_client_err_pgm_str_5; // "connection lost"
        return;
    case FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED:
        buff += firebase_client_err_pgm_str_9; // "maximum Redirection reached"
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED:
        buff += firebase_client_err_pgm_str_8; // "http connection was used by other processes"
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT:
        buff += firebase_client_err_pgm_str_1; // "response payload read timed out"
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_READ_FAILED:
        buff += firebase_client_err_pgm_str_7; // "response read failed."
        return;
    case FIREBASE_ERROR_EXTERNAL_CLIENT_DISABLED:
        buff += firebase_client_err_pgm_str_12; // "custom Client is not yet enabled"
        return;
    case FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED:
        buff += firebase_client_err_pgm_str_13; // "Client is not yet initialized"
        return;

    case FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST:
        buff += firebase_http_err_pgm_str_1; // "bad request"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        buff += firebase_http_err_pgm_str_2; // "non-authoriative information"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NO_CONTENT:
        buff += firebase_http_err_pgm_str_3; // "no content"
        return;
    case FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY:
        buff += firebase_http_err_pgm_str_4; // "moved permanently"
        return;
    case FIREBASE_ERROR_HTTP_CODE_USE_PROXY:
        buff += firebase_http_err_pgm_str_5; // "use proxy"
        return;
    case FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT:
        buff += firebase_http_err_pgm_str_6; // "temporary redirect"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT:
        buff += firebase_http_err_pgm_str_7; // "permanent redirect"
        return;
    case FIREBASE_ERROR_HTTP_CODE_UNAUTHORIZED:
        buff += firebase_http_err_pgm_str_8; // "unauthorized"
        return;
    case FIREBASE_ERROR_HTTP_CODE_FORBIDDEN:
        buff += firebase_http_err_pgm_str_9; // "forbidden"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_FOUND:
        buff += firebase_http_err_pgm_str_10; // "not found"
        return;
    case FIREBASE_ERROR_HTTP_CODE_METHOD_NOT_ALLOWED:
        buff += firebase_http_err_pgm_str_11; // "method not allow"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_ACCEPTABLE:
        buff += firebase_http_err_pgm_str_12; // "not acceptable"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        buff += firebase_http_err_pgm_str_13; // "proxy authentication required"
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT:
        buff += firebase_http_err_pgm_str_14; // "request timed out"
        return;
    case FIREBASE_ERROR_HTTP_CODE_LENGTH_REQUIRED:
        buff += firebase_http_err_pgm_str_15; // "length required"
        return;
    case FIREBASE_ERROR_HTTP_CODE_TOO_MANY_REQUESTS:
        buff += firebase_http_err_pgm_str_16; // "too many requests"
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        buff += firebase_http_err_pgm_str_17; // "request header fields too larg"
        return;
    case FIREBASE_ERROR_HTTP_CODE_INTERNAL_SERVER_ERROR:
        buff += firebase_http_err_pgm_str_18; // "internal server error"
        return;
    case FIREBASE_ERROR_HTTP_CODE_BAD_GATEWAY:
        buff += firebase_http_err_pgm_str_19; // "bad gateway"
        return;
    case FIREBASE_ERROR_HTTP_CODE_SERVICE_UNAVAILABLE:
        buff += firebase_http_err_pgm_str_20; // "service unavailable"
        return;
    case FIREBASE_ERROR_HTTP_CODE_GATEWAY_TIMEOUT:
        buff += firebase_http_err_pgm_str_21; // "gateway timeout"
        return;
    case FIREBASE_ERROR_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        buff += firebase_http_err_pgm_str_22; // "http version not support"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        buff += firebase_http_err_pgm_str_23; // "network authentication required"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED:
        buff += firebase_http_err_pgm_str_24; // "precondition failed (ETag does not match)"
        return;

    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        buff += firebase_mem_err_pgm_str_1; // "data buffer overflow"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE:
        buff += firebase_mem_err_pgm_str_2; // "payload too large"
        return;

#if defined(Firebase_TCP_Client)
    case FIREBASE_ERROR_LONG_RUNNING_TASK:
        buff += firebase_general_err_pgm_str_2; // "operation ignored due to long running task is being processed."
        return;
    case FIREBASE_ERROR_UPLOAD_TIME_OUT:
        buff += firebase_client_err_pgm_str_10; // "upload timed out"
        return;
    case FIREBASE_ERROR_UPLOAD_DATA_ERRROR:
        buff += firebase_client_err_pgm_str_11; // "upload data sent error"
        return;
    case FIREBASE_ERROR_OAUTH2_REQUIRED:
        buff += firebase_auth_err_pgm_str_6; // "OAuth2.0 authentication required"
        return;
#endif

    case FIREBASE_ERROR_TOKEN_NOT_READY:
        buff += firebase_auth_err_pgm_str_1; // "token is not ready (revoked or expired)"
        return;
    case FIREBASE_ERROR_UNINITIALIZED:
        buff += firebase_auth_err_pgm_str_2; // "Firebase authentication was not initialized"
        return;
    case FIREBASE_ERROR_MISSING_DATA:
        buff += firebase_general_err_pgm_str_3; // "missing data."
        return;
    case FIREBASE_ERROR_MISSING_CREDENTIALS:
        buff += firebase_auth_err_pgm_str_3; // "missing required credentials."
        return;

    case FIREBASE_ERROR_TOKEN_PARSE_PK:
        buff += firebase_auth_err_pgm_str_4; // "RSA private key parsing failed"
        break;
    case FIREBASE_ERROR_TOKEN_CREATE_HASH:
        buff += firebase_auth_err_pgm_str_8; // "create message digest failed"
        break;
    case FIREBASE_ERROR_TOKEN_SIGN:
        buff += firebase_auth_err_pgm_str_5; // "JWT token signing failed"
        break;
    case FIREBASE_ERROR_TOKEN_EXCHANGE:
        buff += firebase_auth_err_pgm_str_7; // "token exchange failed"
        break;

#if defined(ENABLE_OTA_FIRMWARE_UPDATE) || defined(FIREBASE_ENABLE_OTA_FIRMWARE_UPDATE)
    case FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE:
        buff += firebase_ota_err_pgm_str_5; // "invalid Firmware"
        return;
    case FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE:
        buff += firebase_ota_err_pgm_str_6; // "too low free sketch space"
        return;
    case FIREBASE_ERROR_FW_UPDATE_BIN_SIZE_NOT_MATCH_SPI_FLASH_SPACE:
        buff += firebase_ota_err_pgm_str_1; // "Bin size does not fit the free flash space"
        return;
    case FIREBASE_ERROR_FW_UPDATE_BEGIN_FAILED:
        buff += firebase_ota_err_pgm_str_2; // "Updater begin() failed"
        return;
    case FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED:
        buff += firebase_ota_err_pgm_str_3; // "Updater write() failed."
        return;
    case FIREBASE_ERROR_FW_UPDATE_END_FAILED:
        buff += firebase_ota_err_pgm_str_4; // "Updater end() failed."
        return;
#endif

#if defined(MBFS_FLASH_FS) || defined(MBFS_SD_FS)

    case MB_FS_ERROR_FILE_IO_ERROR:
        buff += firebase_storage_err_pgm_str_1; // "file I/O error"
        return;

    case MB_FS_ERROR_FLASH_STORAGE_IS_NOT_READY:
        buff += firebase_storage_err_pgm_str_2; // "flash Storage is not ready."
        return;

    case MB_FS_ERROR_SD_STORAGE_IS_NOT_READY:
        buff += firebase_storage_err_pgm_str_3; // "SD Storage is not ready."
        return;

    case MB_FS_ERROR_FILE_STILL_OPENED:
        buff += firebase_storage_err_pgm_str_4; // "file is still opened."
        return;

    case MB_FS_ERROR_FILE_NOT_FOUND:
        buff += firebase_storage_err_pgm_str_5; // "file not found."
        return;
#endif

    case FIREBASE_ERROR_NTP_TIMEOUT:
        buff += firebase_time_err_pgm_str_1; // "NTP server time reading timed out"
        break;
    case FIREBASE_ERROR_NO_WIFI_TIME:
        buff += firebase_time_err_pgm_str_2; // "NTP server time reading cannot begin when valid time is required because of no WiFi capability/activity detected."
        buff += firebase_pgm_str_9;          // " "
        buff += firebase_time_err_pgm_str_3; // "Please set the library reference time manually using Firebase.setSystemTime"
        return;
    case FIREBASE_ERROR_USER_TIME_SETTING_REQUIRED:
        buff += firebase_time_err_pgm_str_3; // "Please set the library reference time manually using Firebase.setSystemTime"
        return;
    case FIREBASE_ERROR_SYS_TIME_IS_NOT_READY:

        buff += firebase_time_err_pgm_str_4; // "system time was not set"
        return;
    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        buff += firebase_rtdb_err_pgm_str_3; // "data type mismatch"
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        buff += firebase_rtdb_err_pgm_str_2; // "path not exist"
        return;
    case FIREBASE_ERROR_EXPECTED_JSON_DATA:
        buff += firebase_rtdb_err_pgm_str_1; // "backup data should be the JSON object"
        return;
    case FIREBASE_ERROR_INVALID_JSON_RULES:
        buff += firebase_rtdb_err_pgm_str_4; // "security rules is not a valid JSON"
        return;
    case FIREBASE_ERROR_USER_PAUSE:
        buff += firebase_rtdb_err_pgm_str_5; // "the FirebaseData object was paused"
        break;

    case FIREBASE_ERROR_NO_FCM_ID_TOKEN_PROVIDED:
        buff += firebase_fcm_err_pgm_str_1; // "no ID token or registration token provided"
        return;
    case FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED:
        buff += firebase_fcm_err_pgm_str_2; // "no server key provided"
        return;
    case FIREBASE_ERROR_NO_FCM_TOPIC_PROVIDED:
        buff += firebase_fcm_err_pgm_str_3; // "no topic provided"
        return;
    case FIREBASE_ERROR_FCM_ID_TOKEN_AT_INDEX_NOT_FOUND:
        buff += firebase_fcm_err_pgm_str_4; // "ID token or registration token was not not found at index"
        return;

    default:
        // The default case will never be happened.
        return;
    }

    if (buff.length() == 0)
        buff = firebase_general_err_pgm_str_1; // "unknown error"
#endif
}

firebase_auth_token_type FirebaseCore::getTokenType()
{
    if (!config)
        return token_type_undefined;
    return config->signer.tokens.token_type;
}

const char *FirebaseCore::getToken()
{
    if (!config || internal.auth_token[0] == '?'
        /* return empty string if refresh token assigned when id token is empty via Firebase.setIdToken */)
        return MB_String().c_str();

    return internal.auth_token.c_str();
}

const char *FirebaseCore::getRefreshToken()
{
    if (!config)
        return MB_String().c_str();
    return internal.refresh_token.c_str();
}

FirebaseConfig *FirebaseCore::getCfg()
{
    return config;
}

FirebaseAuth *FirebaseCore::getAuth()
{
    return auth;
}

MB_String FirebaseCore::getCAFile()
{
    if (!config)
        return MB_String();
    return config->cert.file;
}
firebase_mem_storage_type FirebaseCore::getCAFileStorage()
{
    if (!config)
        return mem_storage_type_undefined;
    return (firebase_mem_storage_type)config->cert.file_storage;
}

bool FirebaseCore::waitIdle(int &httpCode)
{
    unsigned long wTime = millis();
    while (internal.fb_processing)
    {
        if (millis() - wTime > 3000)
        {
            httpCode = FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED;
            return false;
        }
        FBUtils::idle();
    }
    return true;
}

FirebaseCore Core = FirebaseCore();

#endif