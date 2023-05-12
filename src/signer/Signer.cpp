#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Firebase Token Management class, Signer.cpp version 1.3.12
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

#ifndef FIREBASE_SIGNER_CPP
#define FIREBASE_SIGNER_CPP

#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#include "Signer.h"

Firebase_Signer::Firebase_Signer()
{
}

Firebase_Signer::~Firebase_Signer()
{
    end();
}

void Firebase_Signer::begin(FirebaseConfig *cfg, FirebaseAuth *authen, MB_FS *mbfs, uint32_t *mb_ts, uint32_t *mb_ts_offset)
{
    config = cfg;
    auth = authen;
    this->mbfs = mbfs;
    this->mb_ts = mb_ts;
    this->mb_ts_offset = mb_ts_offset;
}

void Firebase_Signer::end()
{
    freeJson();

    wifiCreds.clearAP();
#if defined(HAS_WIFIMULTI)
    if (multi)
        delete multi;

    multi = nullptr;
#endif

    if (localTCPClient)
        freeClient(&tcpClient);
}

bool Firebase_Signer::parseSAFile()
{
    if (config->signer.pk.length() > 0)
        return false;

    int res = mbfs->open(config->service_account.json.path,
                         mbfs_type config->service_account.json.storage_type,
                         mb_fs_open_mode_read);

    if (res >= 0)
    {
        clearServiceAccountCreds();
        initJson();

        size_t len = res;
        char *buf = MemoryHelper::createBuffer<char *>(mbfs, len + 10);
        if (mbfs->available(mbfs_type config->service_account.json.storage_type))
        {
            if ((int)len == mbfs->read(mbfs_type config->service_account.json.storage_type, (uint8_t *)buf, len))
                jsonPtr->setJsonData(buf);
        }

        mbfs->close(mbfs_type config->service_account.json.storage_type);

        if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_1)) // type
        {
            if (resultPtr->to<MB_String>().find(pgm2Str(fb_esp_signer_pgm_str_2 /* service_account */), 0) != MB_String::npos)
            {
                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_3)) // project_id
                    config->service_account.data.project_id = resultPtr->to<const char *>();

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_5)) // private_key_id
                    config->service_account.data.private_key_id = resultPtr->to<const char *>();

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_6)) // private_key
                {
                    char *temp = MemoryHelper::createBuffer<char *>(mbfs, strlen(resultPtr->to<const char *>()));
                    size_t c = 0;
                    for (size_t i = 0; i < strlen(resultPtr->to<const char *>()); i++)
                    {
                        if (resultPtr->to<const char *>()[i] == '\\')
                        {
                            Utils::idle();
                            temp[c++] = '\n';
                            i++;
                        }
                        else
                            temp[c++] = resultPtr->to<const char *>()[i];
                    }
                    config->signer.pk = temp;
                    resultPtr->clear();
                    MemoryHelper::freeBuffer(mbfs, temp);
                }

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_7)) // client_email
                    config->service_account.data.client_email = resultPtr->to<const char *>();
                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_8)) // client_id
                    config->service_account.data.client_id = resultPtr->to<const char *>();

                freeJson();

                MemoryHelper::freeBuffer(mbfs, buf);

                return true;
            }
        }

        freeJson();

        MemoryHelper::freeBuffer(mbfs, buf);
    }

    return false;
}

void Firebase_Signer::clearServiceAccountCreds()
{
    config->service_account.data.private_key = "";
    config->service_account.data.project_id.clear();
    config->service_account.data.private_key_id.clear();
    config->service_account.data.client_email.clear();
    config->signer.pk.clear();
}

bool Firebase_Signer::serviceAccountCredsReady()
{
    return (strlen_P(config->service_account.data.private_key) > 0 || config->signer.pk.length() > 0) &&
           config->service_account.data.client_email.length() > 0 &&
           config->service_account.data.project_id.length() > 0;
}

void Firebase_Signer::setTokenType(fb_esp_auth_token_type type)
{
    config->signer.tokens.token_type = type;
    if (type == token_type_legacy_token)
        config->signer.tokens.status = token_status_ready;
}

bool Firebase_Signer::userAccountCredsReady()
{
    if (!config || !auth)
        return false;

    return config->api_key.length() > 0 && auth->user.email.length() > 0 && auth->user.password.length() > 0;
}

bool Firebase_Signer::isAuthToken(bool oauth)
{
    if (!config || !auth)
        return false;

    bool ret = config->signer.tokens.token_type == token_type_id_token ||
               config->signer.tokens.token_type == token_type_custom_token;
    if (oauth)
        ret |= config->signer.tokens.token_type == token_type_oauth2_access_token;
    return ret;
}

bool Firebase_Signer::checkAuthTypeChanged(FirebaseConfig *config, FirebaseAuth *auth)
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
            config->internal.auth_token = config->signer.tokens.legacy_token;
            config->internal.ltok_len = strlen(config->signer.tokens.legacy_token);
            config->internal.rtok_len = 0;
            config->internal.atok_len = 0;
        }

        // check if user account credentials changed
        if (config->signer.tokens.token_type == token_type_id_token &&
            auth->user.email.length() > 0 &&
            auth->user.password.length() > 0)
        {
            uint16_t crc1 = Utils::calCRC(mbfs, auth->user.email.c_str()),
                     crc2 = Utils::calCRC(mbfs, auth->user.password.c_str());

            auth_changed = config->internal.email_crc != crc1 || config->internal.password_crc != crc2;

            config->internal.email_crc = crc1;
            config->internal.password_crc = crc2;
        }
        // check if service account credentials changed
        else if (config->signer.tokens.token_type == token_type_custom_token ||
                 config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            uint16_t crc1 = Utils::calCRC(mbfs, config->service_account.data.client_email.c_str()),
                     crc2 = Utils::calCRC(mbfs, config->service_account.data.project_id.c_str());
            uint16_t crc3 = Utils::calCRC(mbfs, config->service_account.data.private_key);

            auth_changed = config->internal.client_email_crc != crc1 ||
                           config->internal.project_id_crc != crc2 ||
                           config->internal.priv_key_crc != crc3;

            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                uint16_t crc4 = auth->token.uid.length() > 0 ? Utils::calCRC(mbfs, auth->token.uid.c_str()) : 0;
                auth_changed |= config->internal.uid_crc != crc4;
                config->internal.uid_crc = crc4;
            }

            config->internal.client_email_crc = crc1;
            config->internal.project_id_crc = crc2;
            config->internal.priv_key_crc = crc3;
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

time_t Firebase_Signer::getTime()
{
    return TimeHelper::getTime(mb_ts, mb_ts_offset);
}

bool Firebase_Signer::setTime(time_t ts)
{
    return TimeHelper::setTime(ts, mb_ts, mb_ts_offset);
}

bool Firebase_Signer::isExpired()
{
    if (!config || !auth)
        return false;

    // never expired for legacy token and test mode
    if (config->signer.tokens.token_type == token_type_legacy_token || config->signer.test_mode)
        return false;

    time_t now = 0;

    // adjust the expiry time when needed
    adjustTime(now);

    // time is up or expiry time was reset or unset?
    return (now > (int)(config->signer.tokens.expires - config->signer.preRefreshSeconds) || config->signer.tokens.expires == 0);
}

void Firebase_Signer::adjustTime(time_t &now)
{
    now = getTime(); // returns timestamp (synched) or millis/1000 (unsynched)

    // if time has changed (synched or manually set) after token has been generated, update its expiration
    if (config->signer.tokens.expires > 0 && config->signer.tokens.expires < ESP_DEFAULT_TS && now > ESP_DEFAULT_TS)
        /* new expiry time (timestamp) = current timestamp - total seconds since last token request - 60 */
        config->signer.tokens.expires += now - (millis() - config->signer.tokens.last_millis) / 1000 - 60;

    // pre-refresh seconds should not greater than the expiry time
    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;
}

bool Firebase_Signer::readyToRequest()
{
    bool ret = false;
    // To detain the next request using lat request millis
    if (config && (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0))
    {
        config->signer.lastReqMillis = millis();
        ret = true;
    }

    return ret;
}

bool Firebase_Signer::readyToRefresh()
{
    if (!config)
        return false;
    // To detain the next request using lat request millis
    return millis() - config->internal.fb_last_request_token_cb_millis > 5000;
}

bool Firebase_Signer::readyToSync()
{
    bool ret = false;
    // To detain the next synching using last synching millis
    if (config && millis() - config->internal.fb_last_time_sync_millis > FB_TIME_SYNC_INTERVAL)
    {
        config->internal.fb_last_time_sync_millis = millis();
        ret = true;
    }

    return ret;
}

bool Firebase_Signer::isSyncTimeOut()
{
    bool ret = false;
    // If device time was not synched in time
    if (config && millis() - config->internal.fb_last_ntp_sync_timeout_millis > config->timeout.ntpServerRequest)
    {
        config->internal.fb_last_ntp_sync_timeout_millis = millis();
        ret = true;
    }

    return ret;
}

bool Firebase_Signer::isErrorCBTimeOut()
{
    bool ret = false;
    // To detain the next error callback
    if (config &&
        (millis() - config->internal.fb_last_jwt_generation_error_cb_millis > config->timeout.tokenGenerationError ||
         config->internal.fb_last_jwt_generation_error_cb_millis == 0))
    {
        config->internal.fb_last_jwt_generation_error_cb_millis = millis();
        ret = true;
    }

    return ret;
}

bool Firebase_Signer::handleToken()
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
        config->signer.tokens.auth_type = fb_esp_pgm_str_45; // "Bearer "
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
                    config->internal.refresh_token.length() == 0 &&
                    auth->user.email.length() == 0 &&
                    auth->user.password.length() == 0 &&
                    config->signer.anonymous)
                    return true;

                // return when tcp client was used by other processes
                if (config->internal.fb_processing)
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
                {
                    // set the token processing task started flag and run the task
                    _token_processing_task_enable = true;
                    tokenProcessingTask();
                }

                return false;
            }
            // if auth request using OAuth 2.0 and custom token
            else
            {
                // refresh the token when refresh token was assigned via setCustomToken (with non jwt token)
                // and setAccessToken (fourth argument)
                if (config->internal.refresh_token.length() > 0)
                    return requestTokens(true);

                // handle the jwt token processing

                // if it is the first step and no task is currently running
                if (!config->signer.tokenTaskRunning)
                {
                    if (config->signer.step == fb_esp_jwt_generation_step_begin)
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
                            config->internal.fb_last_jwt_generation_error_cb_millis = 0;
                            sendTokenStatusCB();
                        }
                    }

                    // set the token processing task started flag and run the task
                    _token_processing_task_enable = true;
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

void Firebase_Signer::initJson()
{
    if (!jsonPtr)
        jsonPtr = new FirebaseJson();
    if (!resultPtr)
        resultPtr = new FirebaseJsonData();
}

void Firebase_Signer::freeJson()
{
    if (jsonPtr)
        delete jsonPtr;
    if (resultPtr)
        delete resultPtr;
    jsonPtr = nullptr;
    resultPtr = nullptr;
}

bool Firebase_Signer::checkUDP(UDP *udp, bool &ret, bool &_token_processing_task_enable, float gmtOffset)
{
#if defined(FB_ENABLE_EXTERNAL_CLIENT)

    if (udp)
        ntpClient.begin(udp, "pool.ntp.org" /* NTP host */, 123 /* NTP port */, gmtOffset * 3600 /* timezone offset in seconds */);
    else
    {
        config->signer.tokens.error.message.clear();
        setTokenError(FIREBASE_ERROR_UDP_CLIENT_REQUIRED);
        sendTokenStatusCB();
        config->signer.tokens.status = token_status_on_initialize;
        config->signer.step = fb_esp_jwt_generation_step_begin;
        _token_processing_task_enable = false;
        ret = true;
        return false;
    }
#endif

    return true;
}

void Firebase_Signer::tokenProcessingTask()
{
    // We don't have to use memory reserved tasks e.g., RTOS task in ESP32 for this JWT
    // All tasks can be processed in a finite loop.

    // return when task is currently running
    if (config->signer.tokenTaskRunning)
        return;

    bool ret = false;

    config->signer.tokenTaskRunning = true;

    // flag set for valid time required
    bool sslValidTime = false;

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)
    // valid time required for SSL handshake using server certificate in ESP8266
    if (config->cert.data != NULL || config->cert.file.length() > 0)
        sslValidTime = true;
#endif

    time_t now = getTime();

    while (!ret && config->signer.tokens.status != token_status_ready)
    {
        Utils::idle();
        // check time if clock synching once set in the JWT token generating process (during beginning step)
        // or valid time required for SSL handshake in ESP8266
        if (!config->internal.fb_clock_rdy && (config->internal.fb_clock_synched || sslValidTime))
        {
            if (readyToSync())
            {
                if (isSyncTimeOut())
                {
                    config->signer.tokens.error.message.clear();
                    setTokenError(FIREBASE_ERROR_NTP_SYNC_TIMED_OUT);
                    sendTokenStatusCB();
                    config->signer.tokens.status = token_status_on_initialize;
                    config->internal.fb_last_jwt_generation_error_cb_millis = 0;
                }

                // reset flag to allow clock synching execution again in ut->syncClock if clocck synching was timed out
                config->internal.fb_clock_synched = false;

                reconnect();
            }

            // check or set time again

            if (!checkUDP(udp, ret, _token_processing_task_enable, config->time_zone))
                continue;

            TimeHelper::syncClock(&ntpClient, mb_ts, mb_ts_offset, config->time_zone, config);

            // exit task immediately if time is not ready synched
            // which handleToken function should run repeatedly to enter this function again.
            if (!config->internal.fb_clock_rdy)
            {
                config->signer.tokenTaskRunning = false;
                return;
            }
        }

        if (config->signer.tokens.token_type == token_type_id_token)
        {
            config->signer.lastReqMillis = millis();

            // email/password verification and get id token

            getIdToken(false, toStringPtr(_EMPTY_STR), toStringPtr(_EMPTY_STR));

            // send error cb
            if (!reconnect())
                handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

            // finish
            _token_processing_task_enable = false;
            ret = true;
        }
        else
        {
            // create signed JWT token and exchange with auth token
            if (config->signer.step == fb_esp_jwt_generation_step_begin &&
                (millis() - config->internal.fb_last_jwt_begin_step_millis > config->timeout.tokenGenerationBeginStep ||
                 config->internal.fb_last_jwt_begin_step_millis == 0))
            {
                // time must be set first
                if (!checkUDP(udp, ret, _token_processing_task_enable, config->time_zone))
                    continue;

                TimeHelper::syncClock(&ntpClient, mb_ts, mb_ts_offset, config->time_zone, config);
                config->internal.fb_last_jwt_begin_step_millis = millis();

                if (config->internal.fb_clock_rdy)
                    config->signer.step = fb_esp_jwt_generation_step_encode_header_payload;
            }
            // encode the JWT token
            else if (config->signer.step == fb_esp_jwt_generation_step_encode_header_payload)
            {
                if (createJWT())
                    config->signer.step = fb_esp_jwt_generation_step_sign;
            }
            // sign the JWT token
            else if (config->signer.step == fb_esp_jwt_generation_step_sign)
            {
                if (createJWT())
                    config->signer.step = fb_esp_jwt_generation_step_exchange;
            }
            // sending JWT token requst for auth token
            else if (config->signer.step == fb_esp_jwt_generation_step_exchange)
            {
                if (readyToRefresh())
                {
                    // sending a new request
                    ret = requestTokens(false);

                    config->signer.step = ret || getTime() - now > 3599 ? fb_esp_jwt_generation_step_begin : fb_esp_jwt_generation_step_exchange;

                    // send error cb
                    if (!reconnect())
                        handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

                    // reset state and exit loop
                    _token_processing_task_enable = false;
                    ret = true;
                }
            }
        }
    }

    // reset task running status
    config->signer.tokenTaskRunning = false;
}

bool Firebase_Signer::refreshToken()
{
#if !defined(USE_LEGACY_TOKEN_ONLY)

    if (!config)
        return false;

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        config->internal.fb_processing)
        return false;

    if (config->internal.ltok_len > 0 || (config->internal.rtok_len == 0 && config->internal.atok_len == 0))
        return false;

    if (!initClient(fb_esp_signer_pgm_str_9 /* "securetoken" */, token_status_on_refresh))
        return false;

    jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_11 /* "grantType" */), pgm2Str(fb_esp_signer_pgm_str_12 /* "refresh_token" */));
    jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_13 /* "refreshToken" */), config->internal.refresh_token.c_str());

    MB_String req;
    HttpHelper::addRequestHeaderFirst(req, http_post);

    req += fb_esp_signer_pgm_str_10; // "/v1/token?Key=""
    req += config->api_key;
    HttpHelper::addRequestHeaderLast(req);

    HttpHelper::addGAPIsHostHeader(req, fb_esp_signer_pgm_str_9 /* "securetoken" */);
    HttpHelper::addUAHeader(req);
    HttpHelper::addContentLengthHeader(req, strlen(jsonPtr->raw()));
    HttpHelper::addContentTypeHeader(req, fb_esp_pgm_str_62 /* "application/json" */);
    HttpHelper::addNewLine(req);

    req += jsonPtr->raw(); // {"grantType":"refresh_token","refreshToken":"<refresh token>"}

    tcpClient->send(req.c_str());

    req.clear();
    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    struct fb_esp_auth_token_error_t error;

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            config->signer.tokens.status = token_status_error;

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_14 /* "id_token" */))
            {
                config->internal.auth_token = resultPtr->to<const char *>();
                config->internal.atok_len = strlen(resultPtr->to<const char *>());
                config->internal.ltok_len = 0;
            }

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_12 /* "refresh_token" */))
            {
                config->internal.refresh_token = resultPtr->to<const char *>();
                config->internal.rtok_len = strlen(resultPtr->to<const char *>());
            }

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_15 /* "expires_in" */))
                getExpiration(resultPtr->to<const char *>());

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_16 /* "user_id" */))
                auth->token.uid = resultPtr->to<const char *>();

            return handleTaskError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }

        return handleTaskError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);

#endif

    return true;
}

void Firebase_Signer::newClient(FB_TCP_CLIENT **client)
{
    freeClient(client);

    if (!*client)
    {
        *client = new FB_TCP_CLIENT();

        if (*client)
        {
            // Set local client
            localTCPClient = true;
            (*client)->setConfig(config, mbfs);
        }
    }
}

void Firebase_Signer::freeClient(FB_TCP_CLIENT **client)
{
    // Local client defined?
    if (localTCPClient)
    {
        if (*client)
            delete *client;
        *client = nullptr;
        localTCPClient = false;
    }
}

void Firebase_Signer::setTokenError(int code)
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
        config->internal.fb_processing = false;
        errorToString(code, config->signer.tokens.error.message);
    }
}

bool Firebase_Signer::handleTaskError(int code, int httpCode)
{
    // Close TCP connection and unlock used flag
    tcpClient->stop();
    tcpClient->reserved = false;
    config->internal.fb_processing = false;

    switch (code)
    {

    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:

        // Show error based on connection status
        config->signer.tokens.error.message.clear();
        setTokenError(code);
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
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

        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
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
        config->signer.step = fb_esp_jwt_generation_step_begin;
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
        if (code == FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY)
            sendTokenStatusCB();

        return true;
    }

    return false;
}

void Firebase_Signer::sendTokenStatusCB()
{
    tokenInfo.status = config->signer.tokens.status;
    tokenInfo.type = config->signer.tokens.token_type;
    tokenInfo.error = config->signer.tokens.error;

    if (config->token_status_callback && isErrorCBTimeOut())
        config->token_status_callback(tokenInfo);
}

bool Firebase_Signer::handleTokenResponse(int &httpCode)
{

    if (!reconnect(tcpClient, nullptr))
        return false;

    MB_String header, payload;

    struct server_response_data_t response;
    struct fb_esp_tcp_response_handler_t tcpHandler;

    HttpHelper::intTCPHandler(tcpClient->client, tcpHandler, 2048, 2048, nullptr, false);

    while (tcpClient->connected() && tcpClient->available() == 0)
    {
        Utils::idle();
        if (!reconnect(tcpClient, nullptr, tcpHandler.dataTime))
            return false;
    }

    bool complete = false;

    tcpHandler.chunkBufSize = tcpHandler.defaultChunkSize;

    char *pChunk = MemoryHelper::createBuffer<char *>(mbfs, tcpHandler.chunkBufSize + 1);

    while (tcpHandler.available() || !complete)
    {
        Utils::idle();

        if (!reconnect(tcpClient, nullptr, tcpHandler.dataTime))
            return false;

        if (!HttpHelper::readStatusLine(mbfs, tcpClient->client, tcpHandler, response))
        {

            // The next chunk data can be the remaining http header
            if (tcpHandler.isHeader)
            {
                // Read header, complete?
                if (HttpHelper::readHeader(mbfs, tcpClient->client, tcpHandler, response))
                {
                    if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_NO_CONTENT)
                        tcpHandler.error.code = 0;

                    if (Utils::isNoContent(&response))
                        break;
                }
            }
            else
            {
                memset(pChunk, 0, tcpHandler.chunkBufSize + 1);

                // Read the avilable data
                // chunk transfer encoding?
                if (response.isChunkedEnc)
                    tcpHandler.bufferAvailable = HttpHelper::readChunkedData(mbfs, tcpClient->client,
                                                                             pChunk, nullptr, tcpHandler);
                else
                    tcpHandler.bufferAvailable = HttpHelper::readLine(tcpClient->client,
                                                                      pChunk, tcpHandler.chunkBufSize);

                if (tcpHandler.bufferAvailable > 0)
                {
                    tcpHandler.payloadRead += tcpHandler.bufferAvailable;
                    payload += pChunk;
                }

                if (Utils::isChunkComplete(&tcpHandler, &response, complete) ||
                    Utils::isResponseComplete(&tcpHandler, &response, complete))
                    break;
            }
        }

        if (Utils::isResponseTimeout(&tcpHandler, complete))
            break;
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        tcpClient->flush();

    MemoryHelper::freeBuffer(mbfs, pChunk);

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

bool Firebase_Signer::createJWT()
{

#if !defined(USE_LEGACY_TOKEN_ONLY)

    if (config->signer.step == fb_esp_jwt_generation_step_encode_header_payload)
    {
        config->signer.tokens.status = token_status_on_signing;
        config->signer.tokens.error.code = 0;
        config->signer.tokens.error.message.clear();
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();

        initJson();

        time_t now = getTime();

        config->signer.tokens.jwt.clear();

        // header
        // {"alg":"RS256","typ":"JWT"}
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_17 /* "alg" */), pgm2Str(fb_esp_signer_pgm_str_29 /* "RS256" */));
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_18 /* "typ" */), pgm2Str(fb_esp_signer_pgm_str_28 /* "JWT" */));

        size_t len = Base64Helper::encodedLength(strlen(jsonPtr->raw()));
        char *buf = MemoryHelper::createBuffer<char *>(mbfs, len);
        Base64Helper::encodeUrl(mbfs, buf, (unsigned char *)jsonPtr->raw(), strlen(jsonPtr->raw()));
        config->signer.encHeader = buf;
        MemoryHelper::freeBuffer(mbfs, buf);
        config->signer.encHeadPayload = config->signer.encHeader;

        // payload
        // {"iss":"<email>","sub":"<email>","aud":"<audience>","iat":<timstamp>,"exp":<expire>,"scope":"<scope>"}
        // {"iss":"<email>","sub":"<email>","aud":"<audience>","iat":<timstamp>,"exp":<expire>,"uid":"<uid>","claims":"<claims>"}
        jsonPtr->clear();
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_30 /* "iss" */), config->service_account.data.client_email.c_str());
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_31 /* "sub" */), config->service_account.data.client_email.c_str());

        MB_String t = fb_esp_pgm_str_22; // "https://"
        if (config->signer.tokens.token_type == token_type_custom_token)
        {
            HttpHelper::addGAPIsHost(t, fb_esp_signer_pgm_str_23 /* "identitytoolkit" */);
            t += fb_esp_signer_pgm_str_34; // "/google.identity.identitytoolkit.v1.IdentityToolkit"
        }
        else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            HttpHelper::addGAPIsHost(t, fb_esp_signer_pgm_str_35 /* "oauth2" */);
            t += fb_esp_pgm_str_1;  // "/"
            t += fb_esp_pgm_str_18; // "token"
        }

        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_32 /* "aud" */), t.c_str());
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_36 /* "iat" */), (int)now);

        if (config->signer.expiredSeconds > 3600)
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_33 /* "exp" */), (int)(now + 3600));
        else
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_33 /* "exp" */), (int)(now + config->signer.expiredSeconds));

        if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            MB_String buri;
            MB_String host;
            HttpHelper::addGAPIsHost(host, fb_esp_pgm_str_61 /* "www" */);
            URLHelper::host2Url(buri, host);
            buri += fb_esp_pgm_str_1;         // "/"
            buri += fb_esp_signer_pgm_str_37; // "auth"
            buri += fb_esp_pgm_str_1;         // "/"

            MB_String s = buri;            // https://www.googleapis.com/auth/
            s += fb_esp_signer_pgm_str_38; // "devstorage.full_control"

            s += fb_esp_pgm_str_9;         // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += fb_esp_signer_pgm_str_39; // "datastore"

            s += fb_esp_pgm_str_9;         // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += fb_esp_signer_pgm_str_40; // "userinfo.email"

            s += fb_esp_pgm_str_9;         // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += fb_esp_signer_pgm_str_41; // "firebase.database"

            s += fb_esp_pgm_str_9;         // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += fb_esp_signer_pgm_str_42; // "cloud-platform"
#if defined(FIREBASE_ESP_CLIENT)
            s += fb_esp_pgm_str_9;         // " "
            s += buri;                     // https://www.googleapis.com/auth/
            s += fb_esp_signer_pgm_str_19; // "iam"
#endif

            if (config->signer.tokens.scope.length() > 0)
            {
                MB_VECTOR<MB_String> scopes;
                StringHelper::splitTk(config->signer.tokens.scope, scopes, ",");
                for (size_t i = 0; i < scopes.size(); i++)
                {
                    s += fb_esp_pgm_str_9; // " "
                    s += scopes[i];
                    scopes[i].clear();
                }
                scopes.clear();
            }

            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_20 /* "scope" */), s.c_str());
        }
        else if (config->signer.tokens.token_type == token_type_custom_token)
        {
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_21 /* "uid" */), auth->token.uid.c_str());

            if (auth->token.claims.length() > 2)
            {
                FirebaseJson claims(auth->token.claims.c_str());
                jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_22 /* "claims" */), claims);
            }
        }

        len = Base64Helper::encodedLength(strlen(jsonPtr->raw()));
        buf = MemoryHelper::createBuffer<char *>(mbfs, len);
        Base64Helper::encodeUrl(mbfs, buf, (unsigned char *)jsonPtr->raw(), strlen(jsonPtr->raw()));
        config->signer.encPayload = buf;
        MemoryHelper::freeBuffer(mbfs, buf);

        config->signer.encHeadPayload += fb_esp_signer_pgm_str_4; // "."
        config->signer.encHeadPayload += config->signer.encPayload;

        config->signer.encHeader.clear();
        config->signer.encPayload.clear();

// create message digest from encoded header and payload
#if defined(ESP32)
        config->signer.hash = MemoryHelper::createBuffer<uint8_t *>(mbfs, config->signer.hashSize);
        int ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                             (const unsigned char *)config->signer.encHeadPayload.c_str(),
                             config->signer.encHeadPayload.length(), config->signer.hash);
        if (ret != 0)
        {
            char *temp = MemoryHelper::createBuffer<char *>(mbfs, 100);
            mbedtls_strerror(ret, temp, 100);
            config->signer.tokens.error.message = temp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_md: "));
            MemoryHelper::freeBuffer(mbfs, temp);
            setTokenError(FIREBASE_ERROR_TOKEN_CREATE_HASH);
            sendTokenStatusCB();
            MemoryHelper::freeBuffer(mbfs, config->signer.hash);
            return false;
        }
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
        config->signer.hash = MemoryHelper::createBuffer<char *>(Signer.mbfs, config->signer.hashSize);
        br_sha256_context mc;
        br_sha256_init(&mc);
        br_sha256_update(&mc, config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length());
        br_sha256_out(&mc, config->signer.hash);
#endif

        config->signer.tokens.jwt = config->signer.encHeadPayload;
        config->signer.tokens.jwt += fb_esp_signer_pgm_str_4; // "."
        config->signer.encHeadPayload.clear();

        freeJson();
    }
    else if (config->signer.step == fb_esp_jwt_generation_step_sign)
    {
        config->signer.tokens.status = token_status_on_signing;

#if defined(ESP32)
        config->signer.pk_ctx = new mbedtls_pk_context();
        mbedtls_pk_init(config->signer.pk_ctx);

        // parse priv key
        int ret = 0;
        if (config->signer.pk.length() > 0)
            ret = mbedtls_pk_parse_key(config->signer.pk_ctx,
                                       (const unsigned char *)config->signer.pk.c_str(),
                                       config->signer.pk.length() + 1, NULL, 0);
        else if (strlen_P(config->service_account.data.private_key) > 0)
            ret = mbedtls_pk_parse_key(config->signer.pk_ctx,
                                       (const unsigned char *)config->service_account.data.private_key,
                                       strlen_P(config->service_account.data.private_key) + 1, NULL, 0);

        if (ret != 0)
        {
            char *temp = MemoryHelper::createBuffer<char *>(mbfs, 100);
            mbedtls_strerror(ret, temp, 100);
            config->signer.tokens.error.message = temp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_pk_parse_key: "));
            MemoryHelper::freeBuffer(mbfs, temp);
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            sendTokenStatusCB();
            mbedtls_pk_free(config->signer.pk_ctx);
            MemoryHelper::freeBuffer(mbfs, config->signer.hash);
            delete config->signer.pk_ctx;
            config->signer.pk_ctx = nullptr;
            return false;
        }

        // generate RSA signature from private key and message digest
        config->signer.signature = MemoryHelper::createBuffer<unsigned char *>(mbfs, config->signer.signatureSize);
        size_t sigLen = 0;
        config->signer.entropy_ctx = new mbedtls_entropy_context();
        config->signer.ctr_drbg_ctx = new mbedtls_ctr_drbg_context();
        mbedtls_entropy_init(config->signer.entropy_ctx);
        mbedtls_ctr_drbg_init(config->signer.ctr_drbg_ctx);
        mbedtls_ctr_drbg_seed(config->signer.ctr_drbg_ctx, mbedtls_entropy_func, config->signer.entropy_ctx, NULL, 0);

        ret = mbedtls_pk_sign(config->signer.pk_ctx, MBEDTLS_MD_SHA256,
                              (const unsigned char *)config->signer.hash, config->signer.hashSize,
                              config->signer.signature, &sigLen,
                              mbedtls_ctr_drbg_random, config->signer.ctr_drbg_ctx);
        if (ret != 0)
        {
            char *temp = MemoryHelper::createBuffer<char *>(mbfs, 100);
            mbedtls_strerror(ret, temp, 100);
            config->signer.tokens.error.message = temp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_pk_sign: "));
            MemoryHelper::freeBuffer(mbfs, temp);
            setTokenError(FIREBASE_ERROR_TOKEN_SIGN);
            sendTokenStatusCB();
        }
        else
        {
            config->signer.encSignature.clear();
            size_t len = Base64Helper::encodedLength(config->signer.signatureSize);
            char *buf = MemoryHelper::createBuffer<char *>(mbfs, len);
            Base64Helper::encodeUrl(mbfs, buf, config->signer.signature, config->signer.signatureSize);
            config->signer.encSignature = buf;
            MemoryHelper::freeBuffer(mbfs, buf);

            config->signer.tokens.jwt += config->signer.encSignature;
            config->signer.pk.clear();
            config->signer.encSignature.clear();
        }

        MemoryHelper::freeBuffer(mbfs, config->signer.signature);
        MemoryHelper::freeBuffer(mbfs, config->signer.hash);
        mbedtls_pk_free(config->signer.pk_ctx);
        mbedtls_entropy_free(config->signer.entropy_ctx);
        mbedtls_ctr_drbg_free(config->signer.ctr_drbg_ctx);
        delete config->signer.pk_ctx;
        delete config->signer.entropy_ctx;
        delete config->signer.ctr_drbg_ctx;

        config->signer.pk_ctx = nullptr;
        config->signer.entropy_ctx = nullptr;
        config->signer.ctr_drbg_ctx = nullptr;

        if (ret != 0)
            return false;
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
        // RSA private key
        BearSSL::PrivateKey *pk = nullptr;
        Utils::idle();
        // parse priv key
        if (config->signer.pk.length() > 0)
            pk = new BearSSL::PrivateKey((const char *)config->signer.pk.c_str());
        else if (strlen_P(config->service_account.data.private_key) > 0)
            pk = new BearSSL::PrivateKey((const char *)config->service_account.data.private_key);

        if (!pk)
        {
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("BearSSL, PrivateKey: "));
            sendTokenStatusCB();
            return false;
        }

        if (!pk->isRSA())
        {
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("BearSSL, isRSA: "));
            sendTokenStatusCB();
            delete pk;
            pk = nullptr;
            return false;
        }

        const br_rsa_private_key *br_rsa_key = pk->getRSA();

        // generate RSA signature from private key and message digest
        config->signer.signature = new unsigned char[config->signer.signatureSize];

        Utils::idle();
        int ret = br_rsa_i15_pkcs1_sign(BR_HASH_OID_SHA256, (const unsigned char *)config->signer.hash,
                                        br_sha256_SIZE, br_rsa_key, config->signer.signature);
        Utils::idle();
        MemoryHelper::freeBuffer(mbfs, config->signer.hash);

        size_t len = Base64Helper::encodedLength(config->signer.signatureSize);
        char *buf = MemoryHelper::createBuffer<char *>(mbfs, len);
        Base64Helper::encodeUrl(mbfs, buf, config->signer.signature, config->signer.signatureSize);
        config->signer.encSignature = buf;
        MemoryHelper::freeBuffer(mbfs, buf);
        MemoryHelper::freeBuffer(mbfs, config->signer.signature);
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
        {
            setTokenError(FIREBASE_ERROR_TOKEN_SIGN);
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("BearSSL, br_rsa_i15_pkcs1_sign: "));
            sendTokenStatusCB();
            return false;
        }
#endif
    }

#endif

    return true;
}

bool Firebase_Signer::getIdToken(bool createUser, MB_StringPtr email, MB_StringPtr password)
{
#if !defined(USE_LEGACY_TOKEN_ONLY)

    config->signer.signup = false;

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        config->internal.fb_processing)
        return false;

    if (!initClient(createUser
                        ? fb_esp_signer_pgm_str_23 /* "identitytoolkit" */
                        : fb_esp_pgm_str_61 /* "www" */,
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
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_24 /* "email" */), _email);
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_25 /* "password" */), _password);
        }
    }
    else
    {
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_24 /* "email" */), auth->user.email.c_str());
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_25 /* "password" */), auth->user.password.c_str());
    }

    jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_26 /* "returnSecureToken" */), true);

    MB_String req;
    HttpHelper::addRequestHeaderFirst(req, http_post);

    if (createUser)
        req += fb_esp_signer_pgm_str_27; // "/v1/accounts:signUp?key="
    else
    {
        req += fb_esp_signer_pgm_str_43; // "/identitytoolkit/v3/relyingparty/"
        req += fb_esp_signer_pgm_str_44; // "verifyPassword?key="
    }

    req += config->api_key;
    HttpHelper::addRequestHeaderLast(req);

    if (createUser)
        HttpHelper::addGAPIsHostHeader(req, fb_esp_signer_pgm_str_23 /* "identitytoolkit" */);
    else
        HttpHelper::addGAPIsHostHeader(req, fb_esp_pgm_str_61 /* "www" */);

    HttpHelper::addUAHeader(req);
    HttpHelper::addContentLengthHeader(req, strlen(jsonPtr->raw())); // {"email":"<email>","password":"<password>","returnSecureToken":true}
    HttpHelper::addContentTypeHeader(req, fb_esp_pgm_str_62 /* "application/json" */);
    HttpHelper::addNewLine(req);
    req += jsonPtr->raw();

    tcpClient->send(req.c_str());

    req.clear();

    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            if (!createUser)
                config->signer.tokens.status = token_status_error;

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */))
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
            config->internal.fb_last_jwt_generation_error_cb_millis = 0;
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

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_45 /* "idToken" */))
            {
                config->internal.auth_token = resultPtr->to<const char *>();
                config->internal.atok_len = strlen(resultPtr->to<const char *>());
                config->internal.ltok_len = 0;
            }

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_46 /* "refreshToken" */))
            {
                config->internal.refresh_token = resultPtr->to<const char *>();
                config->internal.rtok_len = strlen(resultPtr->to<const char *>());
            }

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_47 /* "expiresIn" */))
                getExpiration(resultPtr->to<const char *>());

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_48 /* "localId" */))
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

bool Firebase_Signer::deleteIdToken(MB_StringPtr idToken)
{
#if !defined(USE_LEGACY_TOKEN_ONLY)

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        config->internal.fb_processing)
        return false;

    if (!initClient(fb_esp_signer_pgm_str_23 /* "identitytoolkit" */, token_status_uninitialized))
        return false;

    config->signer.signup = false;
    config->internal.fb_processing = true;

    MB_String _idToken = idToken;
    if (_idToken.length() > 0)
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_45 /* "idToken" */), _idToken);
    else
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_45 /* "idToken" */), config->internal.auth_token);

    MB_String req;
    HttpHelper::addRequestHeaderFirst(req, http_post);
    req += fb_esp_signer_pgm_str_49; //"/v1/accounts:delete?key="
    req += config->api_key;
    HttpHelper::addRequestHeaderLast(req);
    HttpHelper::addGAPIsHostHeader(req, fb_esp_signer_pgm_str_23 /* "identitytoolkit" */);
    HttpHelper::addUAHeader(req);
    HttpHelper::addContentLengthHeader(req, strlen(jsonPtr->raw()));
    HttpHelper::addContentTypeHeader(req, fb_esp_pgm_str_62 /* "application/json" */);
    HttpHelper::addNewLine(req);

    req += jsonPtr->raw(); // {"idToken":"<id token>"}

    tcpClient->send(req.c_str());
    req.clear();
    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }

        config->signer.deleteError = error;

        if (error.code == 0)
        {
            if (_idToken.length() == 0 || strcmp(config->internal.auth_token.c_str(), _idToken.c_str()) == 0)
            {
                config->internal.auth_token.clear();
                config->internal.atok_len = 0;
                config->internal.ltok_len = 0;
                config->signer.tokens.expires = 0;
                config->signer.step = fb_esp_jwt_generation_step_begin;
                config->internal.fb_last_jwt_generation_error_cb_millis = 0;
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

void Firebase_Signer::setAutoReconnectWiFi(bool reconnect)
{
    autoReconnectWiFi = reconnect;
}

void Firebase_Signer::setTCPClient(FB_TCP_CLIENT *tcpClient)
{
    // If local client is alreaddy created, free it
    // and use external client instead.
    if (localTCPClient)
        freeClient(&tcpClient);
    this->tcpClient = tcpClient;
}

void Firebase_Signer::setUDPClient(UDP *client, float gmtOffset)
{
    this->udp = client;
    this->gmtOffset = gmtOffset;
}

void Firebase_Signer::setNetworkStatus(bool status)
{
    networkStatus = status;
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    if (tcpClient)
        tcpClient->setNetworkStatus(networkStatus);
#endif
}

void Firebase_Signer::closeSession(FB_TCP_CLIENT *client, fb_esp_session_info_t *session)
{
    if (!session || !client)
        return;

    bool status = client->networkReady();

    if (status)
    {
        // close the socket and free the resources used by the SSL engine

        if (config)
            config->internal.fb_last_reconnect_millis = millis();

        client->stop();
    }
#ifdef ENABLE_RTDB
    if (session->con_mode == fb_esp_con_mode_rtdb_stream)
    {
        session->rtdb.stream_tmo_Millis = millis();
        session->rtdb.data_millis = millis();
        session->rtdb.data_tmo = false;
        session->rtdb.new_stream = true;
    }
#endif
    session->connected = false;
}

bool Firebase_Signer::reconnect(FB_TCP_CLIENT *client, fb_esp_session_info_t *session, unsigned long dataTime)
{
    if (!client)
        return false;

    if (client->type() == fb_tcp_client_type_external)
    {
#if !defined(FB_ENABLE_EXTERNAL_CLIENT)
        if (session)
            session->response.code = FIREBASE_ERROR_EXTERNAL_CLIENT_DISABLED;
        return false;
#endif
        if (!client->isInitialized())
        {
            if (session)
                session->response.code = FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED;
            return false;
        }
    }

    client->setConfig(config, mbfs);

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    client->networkReady();
#else
    networkStatus = client->networkReady();
#endif

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
                Signer.errorToString(session->response.code, session->error);
                closeSession(client, session);
            }
            return false;
        }
    }

    if (!networkStatus)
    {
        if (session)
        {
            if (session->connected)
                closeSession(client, session);
            session->response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;
        }

        bool s_connected = session ? session->connected : false;

        if (config)
            resumeWiFi(client, config->internal.net_once_connected, config->internal.fb_last_reconnect_millis, config->timeout.wifiReconnect, s_connected);
        else
            resumeWiFi(client, net_once_connected, last_reconnect_millis, wifi_reconnect_tmo, s_connected);

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
        client->networkReady();
#else
        networkStatus = client->networkReady();
#endif
    }

#if defined(ENABLE_RTDB)
    if (session)
    {
        if (!networkStatus && session->con_mode == fb_esp_con_mode_rtdb_stream)
            session->rtdb.new_stream = true;
    }
#endif

    if (networkStatus)
        config->internal.net_once_connected = true;

    if (!networkStatus && session)
        session->response.code = FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST;

    return networkStatus;
}

void Firebase_Signer::resumeWiFi(FB_TCP_CLIENT *client, bool &net_once_connected, unsigned long &last_reconnect_millis, uint16_t &wifi_reconnect_tmo, bool session_connected)
{

    if (autoReconnectWiFi || !net_once_connected)
    {
        if (wifi_reconnect_tmo < MIN_WIFI_RECONNECT_TIMEOUT ||
            wifi_reconnect_tmo > MAX_WIFI_RECONNECT_TIMEOUT)
            wifi_reconnect_tmo = MIN_WIFI_RECONNECT_TIMEOUT;

        if (millis() - last_reconnect_millis > wifi_reconnect_tmo &&
            !session_connected)
        {

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
            client->networkReconnect();
#else

#if defined(ESP32) || defined(ESP8266)
            WiFi.reconnect();
#else
            // If config exists, use wifi credentials from config instead of local wifi creds.
            // The local wifi creds can be accessed trough FCM class in case legacy API used.
            fb_esp_wifi_credentials_t *_wifiCreds = config ? &config->wifi.wifiCreds : &wifiCreds;

            if (_wifiCreds->size() > 0)
            {
#if __has_include(<WiFi.h>) || __has_include(<WiFiNINA.h>) || __has_include(<WiFi101.h>)
                if (!networkStatus)
                {
                    WiFi.disconnect();
#if defined(HAS_WIFIMULTI)
                    if (multi)
                        delete multi;
                    multi = nullptr;

                    multi = new WiFiMulti();
                    for (size_t i = 0; i < _wifiCreds->size(); i++)
                        multi->addAP((*_wifiCreds)[i].ssid.c_str(), (*_wifiCreds)[i].password.c_str());

                    if (_wifiCreds->size() > 0)
                        multi->run();
#else
                    WiFi.begin((*_wifiCreds)[0].ssid.c_str(), (*_wifiCreds)[0].password.c_str());
#endif
                }
#endif
            }

#endif

#endif

            last_reconnect_millis = millis();
        }
    }
}

bool Firebase_Signer::reconnect()
{
    if (networkChecking)
        return networkStatus;

    networkChecking = true;

    bool noClient = tcpClient == nullptr;
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
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();
    }

    return networkStatus;
}

bool Firebase_Signer::initClient(PGM_P subDomain, fb_esp_auth_token_status status)
{

    Utils::idle();

    if (status != token_status_uninitialized)
    {
        config->signer.tokens.status = status;
        config->internal.fb_processing = true;
        config->signer.tokens.error.code = 0;
        config->signer.tokens.error.message.clear();
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
        config->internal.fb_last_request_token_cb_millis = millis();
        sendTokenStatusCB();
    }

    // No external and local client assigned?
    if (!tcpClient && !localTCPClient)
        newClient(&tcpClient);

    // Stop TCP session
    tcpClient->stop();

#if !defined(FB_ENABLE_EXTERNAL_CLIENT)
    if (tcpClient->client)
        tcpClient->setCACert(nullptr);
#endif

    if (!reconnect(tcpClient, nullptr))
        return false;

    if (!tcpClient->isInitialized())
        return handleTaskError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED);

#if defined(ESP8266) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    tcpClient->setBufferSizes(2048, 1024);
#endif

    // Used for authentication task
    tcpClient->reserved = true;

    initJson();

    MB_String host;
    HttpHelper::addGAPIsHost(host, subDomain);

    Utils::idle();
    tcpClient->begin(host.c_str(), 443, &response_code);

    return true;
}

bool Firebase_Signer::requestTokens(bool refresh)
{

#if !defined(USE_LEGACY_TOKEN_ONLY)

    time_t now = getTime();

    if (config->signer.tokens.status == token_status_on_request ||
        config->signer.tokens.status == token_status_on_refresh ||
        ((unsigned long)now < ESP_DEFAULT_TS && !refresh && !config->signer.customTokenCustomSet) ||
        config->internal.fb_processing)
        return false;

    if (!initClient(fb_esp_pgm_str_61 /* "www" */, refresh ? token_status_on_refresh : token_status_on_request))
        return false;

    MB_String req;
    HttpHelper::addRequestHeaderFirst(req, http_post);

    if (config->signer.tokens.token_type == token_type_custom_token)
    {
        // {"token":"<sutom or signed jwt token>","returnSecureToken":true}
        if (config->signer.customTokenCustomSet)
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_18 /* "token" */), config->internal.auth_token.c_str());
        else
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_18 /* "token" */), config->signer.tokens.jwt.c_str());

        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_26 /* "returnSecureToken" */), true);

        req += fb_esp_signer_pgm_str_43; // "/identitytoolkit/v3/relyingparty/"
        req += fb_esp_signer_pgm_str_50; // "verifyCustomToken?key="
        req += config->api_key;
        HttpHelper::addRequestHeaderLast(req);
        HttpHelper::addGAPIsHostHeader(req, fb_esp_pgm_str_61 /* "www" */);
    }
    else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
    {
        if (refresh)
        {
            // {"client_id":"<client id>","client_secret":"<client secret>","grant_type":"refresh_token",
            // "refresh_token":"<refresh token>"}
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_8 /* "client_id" */), config->internal.client_id.c_str());
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_51 /* "client_secret" */), config->internal.client_secret.c_str());

            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_52 /* "grant_type" */), pgm2Str(fb_esp_signer_pgm_str_12 /* "refresh_token" */));
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_12 /* "refresh_token" */), config->internal.refresh_token.c_str());
        }
        else
        {

            // rfc 7523, JWT Bearer Token Grant Type Profile for OAuth 2.0

            // {"grant_type":"urn:ietf:params:oauth:grant-type:jwt-bearer","assertion":"<signed jwt token>"}
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_52 /* "grant_type" */),
                         pgm2Str(fb_esp_signer_pgm_str_53 /* "urn:ietf:params:oauth:grant-type:jwt-bearer" */));
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_54 /* "assertion" */), config->signer.tokens.jwt.c_str());
        }

        req += fb_esp_pgm_str_1;  // "/"
        req += fb_esp_pgm_str_18; // "token"
        HttpHelper::addRequestHeaderLast(req);
        HttpHelper::addGAPIsHostHeader(req, fb_esp_signer_pgm_str_35 /* "aouth2" */);
    }

    HttpHelper::addUAHeader(req);
    HttpHelper::addContentLengthHeader(req, strlen(jsonPtr->raw()));
    HttpHelper::addContentTypeHeader(req, fb_esp_pgm_str_62 /* "application/json" */);
    HttpHelper::addNewLine(req);

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());

    req.clear();

    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    struct fb_esp_auth_token_error_t error;

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        config->signer.tokens.jwt.clear();
        if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            config->signer.tokens.status = token_status_error;

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }
        else if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_55 /* "error" */))
        {
            error.code = -1;
            config->signer.tokens.status = token_status_error;

            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_56 /* "error_description" */))
                error.message = resultPtr->to<const char *>();
        }

        if (error.code != 0 && !config->signer.customTokenCustomSet &&
            (config->signer.tokens.token_type == token_type_custom_token ||
             config->signer.tokens.token_type == token_type_oauth2_access_token))
        {
            // new jwt needed as it is already cleared
            config->signer.step = fb_esp_jwt_generation_step_encode_header_payload;
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;

        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {
            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_45 /* "idToken" */))
                {
                    config->internal.auth_token = resultPtr->to<const char *>();
                    config->internal.atok_len = strlen(resultPtr->to<const char *>());
                    config->internal.ltok_len = 0;
                }

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_46 /* "refreshToken" */))
                {
                    config->internal.refresh_token = resultPtr->to<const char *>();
                    config->internal.rtok_len = strlen(resultPtr->to<const char *>());
                }

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_47 /* "expiresIn" */))
                    getExpiration(resultPtr->to<const char *>());
            }
            else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
            {
                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_57 /* "access_token" */))
                {
                    config->internal.auth_token = resultPtr->to<const char *>();
                    config->internal.atok_len = strlen(resultPtr->to<const char *>());
                    config->internal.ltok_len = 0;
                }

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_58 /* "token_type" */))
                    config->signer.tokens.auth_type = resultPtr->to<const char *>();

                if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_signer_pgm_str_15 /* "expires_in" */))
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

void Firebase_Signer::getExpiration(const char *exp)
{
    time_t now = getTime();
    unsigned long ms = millis();
    config->signer.tokens.expires = now + atoi(exp);
    config->signer.tokens.last_millis = ms;
}

bool Firebase_Signer::handleEmailSending(MB_StringPtr payload, fb_esp_user_email_sending_type type)
{
#if !defined(USE_LEGACY_TOKEN_ONLY)

    if (config->internal.fb_processing)
        return false;

    if (!initClient(fb_esp_pgm_str_61 /* "www" */, token_status_uninitialized))
        return false;

    MB_String _payload = payload;

    config->internal.fb_processing = true;

    if (type == fb_esp_user_email_sending_type_verify)
    {
        config->signer.verificationError.message.clear();
        // {"requestType":"VERIFY_EMAIL","idToken":"<id token>"}
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_59 /* "requestType" */), pgm2Str(fb_esp_signer_pgm_str_60 /* "VERIFY_EMAIL" */));
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        config->signer.resetPswError.message.clear();
        // {"requestType":"PASSWORD_RESET","email":"<email>"}
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_59 /* "requestType" */), pgm2Str(fb_esp_signer_pgm_str_62 /* "PASSWORD_RESET" */));
    }

    if (type == fb_esp_user_email_sending_type_verify)
    {
        if (_payload.length() > 0)
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_45 /* "idToken" */), _payload);
        else
            jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_45 /* "idToken" */), config->internal.auth_token.c_str());
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        jsonPtr->add(pgm2Str(fb_esp_signer_pgm_str_24 /* "email" */), _payload);
    }

    MB_String req;
    HttpHelper::addRequestHeaderFirst(req, http_post);
    req += fb_esp_signer_pgm_str_43; // "/identitytoolkit/v3/relyingparty/"
    req += fb_esp_signer_pgm_str_61; // "getOobConfirmationCode?key="
    req += config->api_key;
    HttpHelper::addRequestHeaderLast(req);
    HttpHelper::addGAPIsHostHeader(req, fb_esp_pgm_str_61 /* "www" */);
    HttpHelper::addUAHeader(req);
    HttpHelper::addContentLengthHeader(req, strlen(jsonPtr->raw()));
    HttpHelper::addContentTypeHeader(req, fb_esp_pgm_str_62 /* "application/json" */);
    HttpHelper::addNewLine(req);

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());
    req.clear();
    if (response_code < 0)
        return handleTaskError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
        {
            error.code = resultPtr->to<int>();
            if (JsonHelper::parse(jsonPtr, resultPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */))
                error.message = resultPtr->to<const char *>();
        }
        if (type == fb_esp_user_email_sending_type_verify)
            config->signer.verificationError = error;
        else if (type == fb_esp_user_email_sending_type_reset_psw)
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

void Firebase_Signer::checkToken()
{
    if (!config || !auth)
        return;

    if (isAuthToken(true) && isExpired())
        handleToken();
}

bool Firebase_Signer::tokenReady()
{
    if (!config)
        return false;

    checkToken();

    // call checkToken to send callback before checking connection.
    if (!reconnect())
        return false;

    return config->signer.tokens.status == token_status_ready;
};

void Firebase_Signer::errorToString(int httpCode, MB_String &buff)
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

#if defined(ENABLE_ERROR_STRING)

    switch (httpCode)
    {

    case FIREBASE_ERROR_SSL_RX_BUFFER_SIZE_TOO_SMALL:
        buff += fb_esp_ssl_err_pgm_str_1; // "incomplete SSL client data"
        return;

    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED:
        buff += fb_esp_client_err_pgm_str_2; // "connection refused"
        return;
    case FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED:
        buff += fb_esp_client_err_pgm_str_3; // "send request failed"
        return;
    case FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED:
        buff += fb_esp_client_err_pgm_str_4; // "not connected"
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:
        buff += fb_esp_client_err_pgm_str_5; // "connection lost"
        return;
    case FIREBASE_ERROR_TCP_ERROR_NO_HTTP_SERVER:
        buff += fb_esp_client_err_pgm_str_6; // "no http server"
        return;
    case FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED:
        buff += fb_esp_client_err_pgm_str_9; // "maximum Redirection reached"
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED:
        buff += fb_esp_client_err_pgm_str_8; // "http connection was used by other processes"
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT:
        buff += fb_esp_client_err_pgm_str_1; // "response payload read timed out"
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_READ_FAILED:
        buff += fb_esp_client_err_pgm_str_7; // "response read failed."
        return;
    case FIREBASE_ERROR_EXTERNAL_CLIENT_DISABLED:
        buff += fb_esp_client_err_pgm_str_12; // "custom Client is not yet enabled"
        return;
    case FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED:
        buff += fb_esp_client_err_pgm_str_13; // "Client is not yet initialized"
        return;

    case FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST:
        buff += fb_esp_http_err_pgm_str_1; // "bad request"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        buff += fb_esp_http_err_pgm_str_2; // "non-authoriative information"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NO_CONTENT:
        buff += fb_esp_http_err_pgm_str_3; // "no content"
        return;
    case FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY:
        buff += fb_esp_http_err_pgm_str_4; // "moved permanently"
        return;
    case FIREBASE_ERROR_HTTP_CODE_USE_PROXY:
        buff += fb_esp_http_err_pgm_str_5; // "use proxy"
        return;
    case FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT:
        buff += fb_esp_http_err_pgm_str_6; // "temporary redirect"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT:
        buff += fb_esp_http_err_pgm_str_7; // "permanent redirect"
        return;
    case FIREBASE_ERROR_HTTP_CODE_UNAUTHORIZED:
        buff += fb_esp_http_err_pgm_str_8; // "unauthorized"
        return;
    case FIREBASE_ERROR_HTTP_CODE_FORBIDDEN:
        buff += fb_esp_http_err_pgm_str_9; // "forbidden"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_FOUND:
        buff += fb_esp_http_err_pgm_str_10; // "not found"
        return;
    case FIREBASE_ERROR_HTTP_CODE_METHOD_NOT_ALLOWED:
        buff += fb_esp_http_err_pgm_str_11; // "method not allow"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_ACCEPTABLE:
        buff += fb_esp_http_err_pgm_str_12; // "not acceptable"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        buff += fb_esp_http_err_pgm_str_13; // "proxy authentication required"
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT:
        buff += fb_esp_http_err_pgm_str_14; // "request timed out"
        return;
    case FIREBASE_ERROR_HTTP_CODE_LENGTH_REQUIRED:
        buff += fb_esp_http_err_pgm_str_15; // "length required"
        return;
    case FIREBASE_ERROR_HTTP_CODE_TOO_MANY_REQUESTS:
        buff += fb_esp_http_err_pgm_str_16; // "too many requests"
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        buff += fb_esp_http_err_pgm_str_17; // "request header fields too larg"
        return;
    case FIREBASE_ERROR_HTTP_CODE_INTERNAL_SERVER_ERROR:
        buff += fb_esp_http_err_pgm_str_18; // "internal server error"
        return;
    case FIREBASE_ERROR_HTTP_CODE_BAD_GATEWAY:
        buff += fb_esp_http_err_pgm_str_19; // "bad gateway"
        return;
    case FIREBASE_ERROR_HTTP_CODE_SERVICE_UNAVAILABLE:
        buff += fb_esp_http_err_pgm_str_20; // "service unavailable"
        return;
    case FIREBASE_ERROR_HTTP_CODE_GATEWAY_TIMEOUT:
        buff += fb_esp_http_err_pgm_str_21; // "gateway timeout"
        return;
    case FIREBASE_ERROR_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        buff += fb_esp_http_err_pgm_str_22; // "http version not support"
        return;
    case FIREBASE_ERROR_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        buff += fb_esp_http_err_pgm_str_23; // "network authentication required"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED:
        buff += fb_esp_http_err_pgm_str_24; // "precondition failed (ETag does not match)"
        return;

    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        buff += fb_esp_mem_err_pgm_str_1; // "data buffer overflow"
        return;
    case FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE:
        buff += fb_esp_mem_err_pgm_str_2; // "payload too large"
        return;

#if defined(FIREBASE_ESP_CLIENT)
    case FIREBASE_ERROR_LONG_RUNNING_TASK:
        buff += fb_esp_general_err_pgm_str_2; // "operation ignored due to long running task is being processed."
        return;
    case FIREBASE_ERROR_UPLOAD_TIME_OUT:
        buff += fb_esp_client_err_pgm_str_10; // "upload timed out"
        return;
    case FIREBASE_ERROR_UPLOAD_DATA_ERRROR:
        buff += fb_esp_client_err_pgm_str_11; // "upload data sent error"
        return;
    case FIREBASE_ERROR_OAUTH2_REQUIRED:
        buff += fb_esp_auth_err_pgm_str_6; // "OAuth2.0 authentication required"
        return;
#endif

    case FIREBASE_ERROR_TOKEN_NOT_READY:
        buff += fb_esp_auth_err_pgm_str_1; // "token is not ready (revoked or expired)"
        return;
    case FIREBASE_ERROR_UNINITIALIZED:
        buff += fb_esp_auth_err_pgm_str_2; // "Firebase authentication was not initialized"
        return;
    case FIREBASE_ERROR_MISSING_DATA:
        buff += fb_esp_general_err_pgm_str_3; // "missing data."
        return;
    case FIREBASE_ERROR_MISSING_CREDENTIALS:
        buff += fb_esp_auth_err_pgm_str_3; // "missing required credentials."
        return;

    case FIREBASE_ERROR_TOKEN_PARSE_PK:
        buff += fb_esp_auth_err_pgm_str_4; // "RSA private key parsing failed"
        break;
    case FIREBASE_ERROR_TOKEN_CREATE_HASH:
        buff += fb_esp_auth_err_pgm_str_8; // "create message digest failed"
        break;
    case FIREBASE_ERROR_TOKEN_SIGN:
        buff += fb_esp_auth_err_pgm_str_5; // "JWT token signing failed"
        break;
    case FIREBASE_ERROR_TOKEN_EXCHANGE:
        buff += fb_esp_auth_err_pgm_str_7; // "token exchange failed"
        break;

#if defined(ENABLE_OTA_FIRMWARE_UPDATE)
    case FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE:
        buff += fb_esp_ota_err_pgm_str_5; // "invalid Firmware"
        return;
    case FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE:
        buff += fb_esp_ota_err_pgm_str_6; // "too low free sketch space"
        return;
    case FIREBASE_ERROR_FW_UPDATE_BIN_SIZE_NOT_MATCH_SPI_FLASH_SPACE:
        buff += fb_esp_ota_err_pgm_str_1; // "Bin size does not fit the free flash space"
        return;
    case FIREBASE_ERROR_FW_UPDATE_BEGIN_FAILED:
        buff += fb_esp_ota_err_pgm_str_2; // "Updater begin() failed"
        return;
    case FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED:
        buff += fb_esp_ota_err_pgm_str_3; // "Updater write() failed."
        return;
    case FIREBASE_ERROR_FW_UPDATE_END_FAILED:
        buff += fb_esp_ota_err_pgm_str_4; // "Updater end() failed."
        return;
#endif

#if defined(MBFS_FLASH_FS) || defined(MBFS_SD_FS)

    case MB_FS_ERROR_FILE_IO_ERROR:
        buff += fb_esp_storage_err_pgm_str_1; // "file I/O error"
        return;

    case MB_FS_ERROR_FLASH_STORAGE_IS_NOT_READY:
        buff += fb_esp_storage_err_pgm_str_2; // "flash Storage is not ready."
        return;

    case MB_FS_ERROR_SD_STORAGE_IS_NOT_READY:
        buff += fb_esp_storage_err_pgm_str_3; // "SD Storage is not ready."
        return;

    case MB_FS_ERROR_FILE_STILL_OPENED:
        buff += fb_esp_storage_err_pgm_str_4; // "file is still opened."
        return;

    case MB_FS_ERROR_FILE_NOT_FOUND:
        buff += fb_esp_storage_err_pgm_str_5; // "file not found."
        return;
#endif

    case FIREBASE_ERROR_TOKEN_SET_TIME:
        buff += fb_esp_time_err_pgm_str_1; // "system time was not set"
        break;
    case FIREBASE_ERROR_CANNOT_CONFIG_TIME:
        buff += fb_esp_time_err_pgm_str_2; // "cannot config time"
        return;
    case FIREBASE_ERROR_SYS_TIME_IS_NOT_READY:
        buff += fb_esp_time_err_pgm_str_3; // "device time was not set"
        return;
    case FIREBASE_ERROR_NTP_SYNC_TIMED_OUT:
        buff += fb_esp_time_err_pgm_str_4; // "NTP server time synching failed"
        return;
    case FIREBASE_ERROR_UDP_CLIENT_REQUIRED:
        buff += fb_esp_time_err_pgm_str_5; // "UDP client is required for NTP server time synching based on your network type e.g. WiFiUDP or EthernetUDP\nPlease call fbdo.setUDPClient(&udpClient); to assing the UDP client."
        return;

    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        buff += fb_esp_rtdb_err_pgm_str_3; // "data type mismatch"
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        buff += fb_esp_rtdb_err_pgm_str_2; // "path not exist"
        return;
    case FIREBASE_ERROR_EXPECTED_JSON_DATA:
        buff += fb_esp_rtdb_err_pgm_str_1; // "backup data should be the JSON object"
        return;
    case FIREBASE_ERROR_INVALID_JSON_RULES:
        buff += fb_esp_rtdb_err_pgm_str_4; // "security rules is not a valid JSON"
        return;

    case FIREBASE_ERROR_NO_FCM_ID_TOKEN_PROVIDED:
        buff += fb_esp_fcm_err_pgm_str_1; // "no ID token or registration token provided"
        return;
    case FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED:
        buff += fb_esp_fcm_err_pgm_str_2; // "no server key provided"
        return;
    case FIREBASE_ERROR_NO_FCM_TOPIC_PROVIDED:
        buff += fb_esp_fcm_err_pgm_str_3; // "no topic provided"
        return;
    case FIREBASE_ERROR_FCM_ID_TOKEN_AT_INDEX_NOT_FOUND:
        buff += fb_esp_fcm_err_pgm_str_4; // "ID token or registration token was not not found at index"
        return;

    default:
        return;
    }

    if (buff.length() == 0)
        buff = fb_esp_general_err_pgm_str_1; // "unknown error"
#endif
}

fb_esp_auth_token_type Firebase_Signer::getTokenType()
{
    if (!config)
        return token_type_undefined;
    return config->signer.tokens.token_type;
}

const char *Firebase_Signer::getToken()
{
    if (!config || config->internal.auth_token[0] == '?'
        /* return empty string if refresh token assigned when id token is empty via Firebase.setIdToken */)
        return MB_String().c_str();

    return config->internal.auth_token.c_str();
}

const char *Firebase_Signer::getRefreshToken()
{
    if (!config)
        return MB_String().c_str();
    return config->internal.refresh_token.c_str();
}

FirebaseConfig *Firebase_Signer::getCfg()
{
    return config;
}

MB_FS *Firebase_Signer::getMBFS()
{
    return mbfs;
}

uint32_t *Firebase_Signer::getTS()
{
    return mb_ts;
}

FirebaseAuth *Firebase_Signer::getAuth()
{
    return auth;
}

MB_String Firebase_Signer::getCAFile()
{
    if (!config)
        return MB_String();
    return config->cert.file;
}
fb_esp_mem_storage_type Firebase_Signer::getCAFileStorage()
{
    if (!config)
        return mem_storage_type_undefined;
    return (fb_esp_mem_storage_type)config->cert.file_storage;
}

Firebase_Signer Signer = Firebase_Signer();

#endif