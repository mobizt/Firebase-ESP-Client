/**
 * Google's Firebase Token Generation class, Signer.cpp version 1.3.2
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 10, 2022
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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
#include "Signer.h"

Firebase_Signer::Firebase_Signer()
{
}

Firebase_Signer::~Firebase_Signer()
{
}

void Firebase_Signer::begin(UtilsClass *utils, MB_FS *mbfs, FirebaseConfig *cfg, FirebaseAuth *authen)
{
    ut = utils;
    this->mbfs = mbfs;
    config = cfg;
    auth = authen;
}

bool Firebase_Signer::parseSAFile()
{
    if (config->signer.pk.length() > 0)
        return false;

    int sz = ut->mbfs->open(config->service_account.json.path, mbfs_type config->service_account.json.storage_type, mb_fs_open_mode_read);

    if (sz >= 0)
    {
        clearSA();
        jsonPtr = new FirebaseJson();
        resultPtr = new FirebaseJsonData();
        char *temp = nullptr;

        size_t len = sz;
        char *buf = (char *)ut->newP(len + 10);
        if (ut->mbfs->available(mbfs_type config->service_account.json.storage_type))
        {
            if ((int)len == ut->mbfs->read(mbfs_type config->service_account.json.storage_type, (uint8_t *)buf, len))
                jsonPtr->setJsonData(buf);
        }
        ut->mbfs->close(mbfs_type config->service_account.json.storage_type);
        ut->delP(&buf);

        if (parseJsonResponse(fb_esp_pgm_str_243))
        {
            if (ut->strposP(resultPtr->to<const char *>(), fb_esp_pgm_str_244, 0) > -1)
            {
                if (parseJsonResponse(fb_esp_pgm_str_245))
                    config->service_account.data.project_id = resultPtr->to<const char *>();

                if (parseJsonResponse(fb_esp_pgm_str_246))
                    config->service_account.data.private_key_id = resultPtr->to<const char *>();

                if (parseJsonResponse(fb_esp_pgm_str_247))
                {
                    temp = (char *)ut->newP(strlen(resultPtr->to<const char *>()));
                    size_t c = 0;
                    for (size_t i = 0; i < strlen(resultPtr->to<const char *>()); i++)
                    {
                        if (resultPtr->to<const char *>()[i] == '\\')
                        {
                            ut->idle();
                            temp[c++] = '\n';
                            i++;
                        }
                        else
                            temp[c++] = resultPtr->to<const char *>()[i];
                    }
                    config->signer.pk = temp;
                    resultPtr->clear();
                    ut->delP(&temp);
                }

                if (parseJsonResponse(fb_esp_pgm_str_248))
                    config->service_account.data.client_email = resultPtr->to<const char *>();
                if (parseJsonResponse(fb_esp_pgm_str_253))
                    config->service_account.data.client_id = resultPtr->to<const char *>();

                delete jsonPtr;
                delete resultPtr;

                jsonPtr = nullptr;
                resultPtr = nullptr;

                return true;
            }
        }

        delete jsonPtr;
        delete resultPtr;

        jsonPtr = nullptr;
        resultPtr = nullptr;
    }

    return false;
}

void Firebase_Signer::clearSA()
{
    config->service_account.data.private_key = "";
    config->service_account.data.project_id.clear();
    config->service_account.data.private_key_id.clear();
    config->service_account.data.client_email.clear();
    config->signer.pk.clear();
}

bool Firebase_Signer::tokenSigninDataReady()
{
    if (!config)
        return false;
    return (strlen_P(config->service_account.data.private_key) > 0 || config->signer.pk.length() > 0) && config->service_account.data.client_email.length() > 0 && config->service_account.data.project_id.length() > 0;
}

void Firebase_Signer::setTokenType(fb_esp_auth_token_type type)
{
    config->signer.tokens.token_type = type;

    switch (type)
    {

    case token_type_custom_token:
    case token_type_oauth2_access_token:
        break;
    case token_type_id_token:
        break;

    case token_type_legacy_token:
        config->signer.tokens.status = token_status_ready;
        break;

    default:
        break;
    }
}

bool Firebase_Signer::userSigninDataReady()
{
    if (!config || !auth)
        return false;

    return config->api_key.length() > 0 && auth->user.email.length() > 0 && auth->user.password.length() > 0;
}

bool Firebase_Signer::isAuthToken(bool admin)
{
    if (!config || !auth)
        return false;

    bool ret = config->signer.tokens.token_type == token_type_id_token || config->signer.tokens.token_type == token_type_custom_token;
    if (admin)
        ret |= config->signer.tokens.token_type == token_type_oauth2_access_token;
    return ret;
}

bool Firebase_Signer::authChanged(FirebaseConfig *config, FirebaseAuth *auth)
{
    bool auth_changed = false;

    if (!config->signer.test_mode)
    {

        if (config->service_account.json.path.length() > 0)
        {
            if (!Signer.parseSAFile())
                config->signer.tokens.status = token_status_uninitialized;
        }

        if (strlen(config->signer.tokens.legacy_token) > 0)
        {
            Signer.setTokenType(token_type_legacy_token);
            config->internal.auth_token = config->signer.tokens.legacy_token;
            config->internal.ltok_len = strlen(config->signer.tokens.legacy_token);
            config->internal.rtok_len = 0;
            config->internal.atok_len = 0;
        }
        else if (Signer.tokenSigninDataReady())
        {
            config->signer.idTokenCustomSet = false;
            config->signer.accessTokenCustomSet = false;
            config->signer.customTokenCustomSet = false;

            if (auth->token.uid.length() == 0)
                Signer.setTokenType(token_type_oauth2_access_token);
            else
                Signer.setTokenType(token_type_custom_token);
        }
        else if (Signer.userSigninDataReady() || config->signer.anonymous)
            Signer.setTokenType(token_type_id_token);

        if (config->signer.tokens.token_type == token_type_id_token && auth->user.email.length() > 0 && auth->user.password.length() > 0)
        {

            uint16_t crc1 = ut->calCRC(auth->user.email.c_str()), crc2 = ut->calCRC(auth->user.password.c_str());

            auth_changed = config->internal.email_crc != crc1 || config->internal.password_crc != crc2;

            config->internal.email_crc = crc1;
            config->internal.password_crc = crc2;
        }
        else if (config->signer.tokens.token_type == token_type_custom_token || config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            uint16_t crc1 = ut->calCRC(config->service_account.data.client_email.c_str()), crc2 = ut->calCRC(config->service_account.data.project_id.c_str());
            uint16_t crc3 = crc2 = ut->calCRC(config->service_account.data.private_key);

            auth_changed = config->internal.client_email_crc != crc1 || config->internal.project_id_crc != crc2 || config->internal.priv_key_crc != crc3;

            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                uint16_t crc4 = auth->token.uid.length() > 0 ? ut->calCRC(auth->token.uid.c_str()) : 0;
                auth_changed |= config->internal.uid_crc != crc4;
                config->internal.uid_crc = crc4;
            }

            config->internal.client_email_crc = crc1;
            config->internal.project_id_crc = crc2;
            config->internal.priv_key_crc = crc3;
        }

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
    return ut->getTime();
}

bool Firebase_Signer::setTime(time_t ts)
{

#if defined(ESP8266) || defined(ESP32)

    if (ut->setTimestamp(ts) == 0)
    {
        this->ts = time(nullptr);
        ut->ts = this->ts;
        return true;
    }

#else
    if (ts > ESP_DEFAULT_TS)
        this->ts = ts - millis() / 1000;
    ut->ts = this->ts;
#endif

    return false;
}

bool Firebase_Signer::isExpired()
{
    if (!config || !auth)
        return false;

    if (config->signer.tokens.token_type == token_type_legacy_token)
        return false;

    time_t now = 0;

    adjustTime(now);

    return (now > (int)(config->signer.tokens.expires - config->signer.preRefreshSeconds) || config->signer.tokens.expires == 0);
}

void Firebase_Signer::adjustTime(time_t &now)
{
    now = getTime();

    // if the time was set (changed) after token has been generated, update its expiration
    if (config->signer.tokens.expires > 0 && config->signer.tokens.expires < ESP_DEFAULT_TS && now > ESP_DEFAULT_TS)
        config->signer.tokens.expires += now - (millis() - config->signer.tokens.last_millis) / 1000 - 60;

    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;
}

bool Firebase_Signer::handleToken()
{

    if (!config || !auth)
        return false;

    if (config->signer.test_mode)
    {
        setTokenError(0);
        return true;
    }

    bool exp = isExpired();

    if (config->signer.customTokenCustomSet)
    {
        if (exp)
        {
            config->signer.tokens.status = token_status_uninitialized;
            config->signer.tokens.token_type = token_type_custom_token;

            bool ret = false;

            if (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0)
            {
                config->signer.lastReqMillis = millis();

                ret = requestTokens(false);

                if (ret)
                    config->signer.customTokenCustomSet = false;
            }

            return ret;
        }

        config->signer.tokens.status = token_status_ready;
        return true;
    }
    else if (config->signer.accessTokenCustomSet && !exp)
    {
        config->signer.tokens.auth_type = fb_esp_pgm_str_209;
        config->signer.tokens.status = token_status_ready;
        return true;
    }

    if (isAuthToken(true) && exp)
    {
        if (config->signer.tokens.expires > 0 && isAuthToken(false))
        {
            if (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0)
            {
                if (config->signer.idTokenCustomSet && config->internal.refresh_token.length() == 0 && auth->user.email.length() == 0 && auth->user.password.length() == 0 && config->signer.anonymous)
                    return true;

                if (config->internal.fb_processing)
                    return false;

                if (millis() - config->internal.fb_last_request_token_cb_millis > 5000)
                    return refreshToken();
            }
            return false;
        }
        else
        {

            if (config->signer.tokens.token_type == token_type_id_token)
            {
                if (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0)
                {
                    _token_processing_task_enable = true;
                    tokenProcessingTask();
                }
                return false;
            }
            else
            {

                if (config->internal.refresh_token.length() > 0)
                    return requestTokens(true);

                if (config->signer.step == fb_esp_jwt_generation_step_begin)
                {

                    if (!config->signer.tokenTaskRunning)
                    {
                        if (config->service_account.json.path.length() > 0 && config->signer.pk.length() == 0)
                        {
                            if (!parseSAFile())
                                config->signer.tokens.status = token_status_uninitialized;
                        }

                        if (config->signer.tokens.status != token_status_on_initialize)
                        {
                            config->signer.tokens.status = token_status_on_initialize;
                            config->signer.tokens.error.code = 0;
                            config->signer.tokens.error.message.clear();
                            config->internal.fb_last_jwt_generation_error_cb_millis = 0;
                            sendTokenStatusCB();
                        }

                        _token_processing_task_enable = true;
                        tokenProcessingTask();
                    }
                }
            }
        }
    }

    if (config->signer.tokens.token_type == token_type_legacy_token)
    {
        setTokenError(0);
        return true;
    }
    else
    {
        if (config->signer.tokens.token_type == token_type_undefined)
        {
            config->signer.tokens.error.message.clear();
            setTokenError(FIREBASE_ERROR_MISSING_CREDENTIALS);
            sendTokenStatusCB();
        }

        return config->signer.tokens.status == token_status_ready;
    }
}

void Firebase_Signer::tokenProcessingTask()
{

    if (config->signer.tokenTaskRunning)
        return;

    bool ret = false;

    config->signer.tokenTaskRunning = true;

    bool sslValidTime = false;

#if defined(ESP8266)
    if (config->cert.data != NULL || config->cert.file.length() > 0)
        sslValidTime = true;
#endif

    while (!ret && config->signer.tokens.status != token_status_ready)
    {
        delay(0);

        // check time if clock synching once set
        if (!config->internal.fb_clock_rdy && (config->internal.fb_clock_synched || sslValidTime))
        {
            if (millis() - config->internal.fb_last_time_sync_millis > FB_TIME_SYNC_INTERVAL)
            {
                config->internal.fb_last_time_sync_millis = millis();

                if (millis() - config->internal.fb_last_ntp_sync_timeout_millis > config->timeout.ntpServerRequest)
                {
                    config->internal.fb_last_ntp_sync_timeout_millis = millis();
                    config->signer.tokens.error.message.clear();
                    setTokenError(FIREBASE_ERROR_NTP_SYNC_TIMED_OUT);
                    sendTokenStatusCB();
                    config->signer.tokens.status = token_status_on_initialize;
                    config->internal.fb_last_jwt_generation_error_cb_millis = 0;
                }

                // set clock again if timed out
                config->internal.fb_clock_synched = false;
            }

            // check or set time again
            ut->syncClock(config->time_zone);

            if (!config->internal.fb_clock_rdy)
            {
                config->signer.tokenTaskRunning = false;
                return;
            }
        }

        if (config->signer.tokens.token_type == token_type_id_token)
        {
            config->signer.lastReqMillis = millis();

            getIdToken(false, toStringPtr(_EMPTY_STR), toStringPtr(_EMPTY_STR));

            _token_processing_task_enable = false;
            ret = true;
        }
        else
        {
            if (config->signer.step == fb_esp_jwt_generation_step_begin && (millis() - config->internal.fb_last_jwt_begin_step_millis > config->timeout.tokenGenerationBeginStep || config->internal.fb_last_jwt_begin_step_millis == 0))
            {
                // time must be set first
                ut->syncClock(config->time_zone);
                config->internal.fb_last_jwt_begin_step_millis = millis();

                if (config->internal.fb_clock_rdy)
                    config->signer.step = fb_esp_jwt_generation_step_encode_header_payload;
            }
            else if (config->signer.step == fb_esp_jwt_generation_step_encode_header_payload)
            {
                if (createJWT())
                    config->signer.step = fb_esp_jwt_generation_step_sign;
            }
            else if (config->signer.step == fb_esp_jwt_generation_step_sign)
            {
                if (createJWT())
                    config->signer.step = fb_esp_jwt_generation_step_exchange;
            }
            else if (config->signer.step == fb_esp_jwt_generation_step_exchange)
            {
                if (millis() - config->internal.fb_last_request_token_cb_millis > 5000)
                {
                    requestTokens(false);

                    _token_processing_task_enable = false;
                    config->signer.step = fb_esp_jwt_generation_step_begin;
                    ret = true;
                }
            }
        }
    }

    config->signer.tokenTaskRunning = false;
}

bool Firebase_Signer::parseJsonResponse(PGM_P key_path)
{
    resultPtr->clear();
    jsonPtr->get(*resultPtr, pgm2Str(key_path));
    return resultPtr->success;
}

bool Firebase_Signer::refreshToken()
{

    if (!config)
        return false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->internal.fb_processing)
        return false;

    if (config->internal.ltok_len > 0 || (config->internal.rtok_len == 0 && config->internal.atok_len == 0))
        return false;

    if (!initClient(fb_esp_pgm_str_203, token_status_on_refresh))
        return false;

    jsonPtr->add(pgm2Str(fb_esp_pgm_str_205), pgm2Str(fb_esp_pgm_str_206));
    jsonPtr->add(pgm2Str(fb_esp_pgm_str_207), config->internal.refresh_token.c_str());

    MB_String s;
    jsonPtr->toString(s);

    MB_String req;
    req += fb_esp_pgm_str_24;
    req += fb_esp_pgm_str_6;

    req += fb_esp_pgm_str_204;
    req += config->api_key;
    req += fb_esp_pgm_str_30;

    req += fb_esp_pgm_str_31;
    req += fb_esp_pgm_str_203;
    req += fb_esp_pgm_str_4;
    req += fb_esp_pgm_str_120;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_32;
    req += fb_esp_pgm_str_12;
    req += s.length();
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_8;
    req += fb_esp_pgm_str_129;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_21;

    req += s.c_str();

    tcpClient->send(req.c_str());

    req.clear();
    if (response_code < 0)
        return handleSignerError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    struct fb_esp_auth_token_error_t error;

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        if (parseJsonResponse(fb_esp_pgm_str_257))
        {
            error.code = resultPtr->to<int>();
            config->signer.tokens.status = token_status_error;

            if (parseJsonResponse(fb_esp_pgm_str_258))
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

            if (parseJsonResponse(fb_esp_pgm_str_208))
            {
                config->internal.auth_token = resultPtr->to<const char *>();
                config->internal.atok_len = strlen(resultPtr->to<const char *>());
                config->internal.ltok_len = 0;
            }

            if (parseJsonResponse(fb_esp_pgm_str_206))
            {
                config->internal.refresh_token = resultPtr->to<const char *>();
                config->internal.rtok_len = strlen(resultPtr->to<const char *>());
            }

            if (parseJsonResponse(fb_esp_pgm_str_210))
                getExpiration(resultPtr->to<const char *>());

            if (parseJsonResponse(fb_esp_pgm_str_187))
                auth->token.uid = resultPtr->to<const char *>();

            return handleSignerError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }

        return handleSignerError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleSignerError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);
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

bool Firebase_Signer::handleSignerError(int code, int httpCode)
{
    // Close TCP connection and unlock use flag
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
            config->signer.tokens.error.message.clear();
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
    if (tcpClient && intTCPClient)
    {
        delete tcpClient;
        tcpClient = nullptr;
        intTCPClient = false;
    }

    if (jsonPtr)
        delete jsonPtr;

    if (resultPtr)
        delete resultPtr;

    jsonPtr = nullptr;
    resultPtr = nullptr;

    

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

    if (config->token_status_callback)
    {
        if (millis() - config->internal.fb_last_jwt_generation_error_cb_millis > config->timeout.tokenGenerationError || config->internal.fb_last_jwt_generation_error_cb_millis == 0)
        {
            config->internal.fb_last_jwt_generation_error_cb_millis = millis();
            config->token_status_callback(tokenInfo);
        }
    }
}

bool Firebase_Signer::handleTokenResponse(int &httpCode)
{

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    tcpClient->networkReady();
#else
    networkStatus = tcpClient->networkReady();
#endif

    if (!networkStatus)
        return false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int chunkBufSize = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    MB_String header, payload;
    bool isHeader = false;

    while (tcpClient->connected() && tcpClient->available() == 0)
    {

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
        tcpClient->networkReady();
#else
        networkStatus = tcpClient->networkReady();
#endif

        if (!networkStatus)
        {
            tcpClient->stop();
            return false;
        }
        ut->idle();
    }

    bool complete = false;
    unsigned long datatime = millis();
    while (!complete)
    {

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
        tcpClient->networkReady();
#else
        networkStatus = tcpClient->networkReady();
#endif

        if (!networkStatus)
        {
            tcpClient->stop();
            return false;
        }

        chunkBufSize = tcpClient->available();

        if (chunkBufSize > 1 || !complete)
        {
            while (!complete)
            {
                ut->idle();

                chunkBufSize = tcpClient->available();

                if (chunkBufSize > 0)
                {
                    if (chunkIdx == 0)
                    {
                        tcpClient->readLine(header);
                        int pos = 0;
                        char *temp = ut->getHeader(header.c_str(), fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                        if (temp)
                        {
                            isHeader = true;
                            response.httpCode = atoi(temp);
                            ut->delP(&temp);
                        }
                    }
                    else
                    {
                        if (isHeader)
                        {
                            char *temp = (char *)ut->newP(chunkBufSize);
                            int readLen = tcpClient->readLine(temp, chunkBufSize);
                            bool headerEnded = false;

                            if (readLen == 1)
                                if (temp[0] == '\r')
                                    headerEnded = true;

                            if (readLen == 2)
                                if (temp[0] == '\r' && temp[1] == '\n')
                                    headerEnded = true;

                            if (headerEnded)
                            {
                                isHeader = false;
                                ut->parseRespHeader(header.c_str(), response);
                                header.clear();
                            }
                            else
                                header += temp;

                            ut->delP(&temp);
                        }
                        else
                        {
                            if (!response.noContent)
                            {
                                if (response.isChunkedEnc)
                                    complete = tcpClient->readChunkedData(payload, chunkedDataState, chunkedDataSize, chunkedDataLen) < 0;
                                else
                                {
                                    chunkBufSize = 1024;
                                    if (tcpClient->available() < chunkBufSize)
                                        chunkBufSize = tcpClient->available();

                                    char *temp = (char *)ut->newP(chunkBufSize + 1);
                                    int readLen = tcpClient->readBytes(temp, chunkBufSize);

                                    if (readLen > 0)
                                        payload += temp;

                                    ut->delP(&temp);
                                    complete = tcpClient->available() <= 0;
                                }
                            }
                            else
                            {
                                tcpClient->flush();
                                break;
                            }
                        }
                    }
                    chunkIdx++;
                }

                if (millis() - datatime > 5000)
                    complete = true;
            }
        }
    }

    if (tcpClient->connected())
        tcpClient->stop();

    httpCode = response.httpCode;

    if (payload.length() > 0 && !response.noContent)
    {

        jsonPtr->setJsonData(payload.c_str());
        payload.clear();
        return true;
    }

    return false;
}

bool Firebase_Signer::createJWT()
{

    if (config->signer.step == fb_esp_jwt_generation_step_encode_header_payload)
    {
        config->signer.tokens.status = token_status_on_signing;
        config->signer.tokens.error.code = 0;
        config->signer.tokens.error.message.clear();
        config->internal.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();

        jsonPtr = new FirebaseJson();
        resultPtr = new FirebaseJsonData();

        time_t now = getTime();

        config->signer.tokens.jwt.clear();

        // header
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_239), pgm2Str(fb_esp_pgm_str_242));
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_240), pgm2Str(fb_esp_pgm_str_234));

        MB_String hdr;
        jsonPtr->toString(hdr);
        size_t len = ut->base64EncLen(hdr.length());
        char *buf = (char *)ut->newP(len);
        ut->encodeBase64Url(buf, (unsigned char *)hdr.c_str(), hdr.length());
        config->signer.encHeader = buf;
        ut->delP(&buf);
        config->signer.encHeadPayload = config->signer.encHeader;
        hdr.clear();

        // payload
        jsonPtr->clear();
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_212), config->service_account.data.client_email.c_str());
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_213), config->service_account.data.client_email.c_str());

        MB_String t = fb_esp_pgm_str_112;
        if (config->signer.tokens.token_type == token_type_custom_token)
        {
            t += fb_esp_pgm_str_250;
            t += fb_esp_pgm_str_4;
            t += fb_esp_pgm_str_120;
            t += fb_esp_pgm_str_231;
        }
        else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            t += fb_esp_pgm_str_251;
            t += fb_esp_pgm_str_4;
            t += fb_esp_pgm_str_120;
            t += fb_esp_pgm_str_1;
            t += fb_esp_pgm_str_233;
        }

        jsonPtr->add(pgm2Str(fb_esp_pgm_str_214), t.c_str());

        jsonPtr->add(pgm2Str(fb_esp_pgm_str_218), (int)now);

        if (config->signer.expiredSeconds > 3600)
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_215), (int)(now + 3600));
        else
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_215), (int)(now + config->signer.expiredSeconds));

        if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            MB_String buri = fb_esp_pgm_str_112;
            buri += fb_esp_pgm_str_193;
            buri += fb_esp_pgm_str_4;
            buri += fb_esp_pgm_str_120;
            buri += fb_esp_pgm_str_1;
            buri += fb_esp_pgm_str_219;
            buri += fb_esp_pgm_str_1;

            MB_String s = buri;
            s += fb_esp_pgm_str_221;

            s += fb_esp_pgm_str_6;
            s += buri;
            s += fb_esp_pgm_str_222;

            s += fb_esp_pgm_str_6;
            s += buri;
            s += fb_esp_pgm_str_223;

            s += fb_esp_pgm_str_6;
            s += buri;
            s += fb_esp_pgm_str_224;

            s += fb_esp_pgm_str_6;
            s += buri;
            s += fb_esp_pgm_str_225;
#if defined(FIREBASE_ESP_CLIENT)
            s += fb_esp_pgm_str_6;
            s += buri;
            s += fb_esp_pgm_str_451;
#endif

            if (config->signer.tokens.scope.length() > 0)
            {
                MB_VECTOR<MB_String> scopes;
                ut->splitTk(config->signer.tokens.scope, scopes, ",");
                for (size_t i = 0; i < scopes.size(); i++)
                {
                    s += fb_esp_pgm_str_6;
                    s += scopes[i];
                    scopes[i].clear();
                }
                scopes.clear();
            }

            jsonPtr->add(pgm2Str(fb_esp_pgm_str_220), s.c_str());
        }
        else if (config->signer.tokens.token_type == token_type_custom_token)
        {
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_254), auth->token.uid.c_str());

            if (auth->token.claims.length() > 2)
            {
                FirebaseJson claims(auth->token.claims.c_str());
                jsonPtr->add(pgm2Str(fb_esp_pgm_str_255), claims);
            }
        }

        MB_String payload;
        jsonPtr->toString(payload);

        len = ut->base64EncLen(payload.length());
        buf = (char *)ut->newP(len);
        ut->encodeBase64Url(buf, (unsigned char *)payload.c_str(), payload.length());
        config->signer.encPayload = buf;
        ut->delP(&buf);
        payload.clear();

        config->signer.encHeadPayload += fb_esp_pgm_str_4;
        config->signer.encHeadPayload += config->signer.encPayload;

        config->signer.encHeader.clear();
        config->signer.encPayload.clear();

// create message digest from encoded header and payload
#if defined(ESP32)
        config->signer.hash = (uint8_t *)ut->newP(config->signer.hashSize);
        int ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (const unsigned char *)config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length(), config->signer.hash);
        if (ret != 0)
        {
            char *temp = (char *)ut->newP(100);
            mbedtls_strerror(ret, temp, 100);
            config->signer.tokens.error.message = temp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_md: "));
            ut->delP(&temp);
            setTokenError(FIREBASE_ERROR_TOKEN_CREATE_HASH);
            sendTokenStatusCB();
            ut->delP(&config->signer.hash);
            return false;
        }
#elif defined(ESP8266)
        config->signer.hash = (char *)ut->newP(config->signer.hashSize);
        br_sha256_context mc;
        br_sha256_init(&mc);
        br_sha256_update(&mc, config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length());
        br_sha256_out(&mc, config->signer.hash);
#endif

        config->signer.tokens.jwt = config->signer.encHeadPayload;
        config->signer.tokens.jwt += fb_esp_pgm_str_4;
        config->signer.encHeadPayload.clear();

        delete jsonPtr;
        delete resultPtr;
        jsonPtr = nullptr;
        resultPtr = nullptr;
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
            ret = mbedtls_pk_parse_key(config->signer.pk_ctx, (const unsigned char *)config->signer.pk.c_str(), config->signer.pk.length() + 1, NULL, 0);
        else if (strlen_P(config->service_account.data.private_key) > 0)
            ret = mbedtls_pk_parse_key(config->signer.pk_ctx, (const unsigned char *)config->service_account.data.private_key, strlen_P(config->service_account.data.private_key) + 1, NULL, 0);

        if (ret != 0)
        {
            char *temp = (char *)ut->newP(100);
            mbedtls_strerror(ret, temp, 100);
            config->signer.tokens.error.message = temp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_pk_parse_key: "));
            ut->delP(&temp);
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            sendTokenStatusCB();
            mbedtls_pk_free(config->signer.pk_ctx);
            ut->delP(&config->signer.hash);
            delete config->signer.pk_ctx;
            config->signer.pk_ctx = nullptr;
            return false;
        }

        // generate RSA signature from private key and message digest
        config->signer.signature = (unsigned char *)ut->newP(config->signer.signatureSize);
        size_t sigLen = 0;
        config->signer.entropy_ctx = new mbedtls_entropy_context();
        config->signer.ctr_drbg_ctx = new mbedtls_ctr_drbg_context();
        mbedtls_entropy_init(config->signer.entropy_ctx);
        mbedtls_ctr_drbg_init(config->signer.ctr_drbg_ctx);
        mbedtls_ctr_drbg_seed(config->signer.ctr_drbg_ctx, mbedtls_entropy_func, config->signer.entropy_ctx, NULL, 0);

        ret = mbedtls_pk_sign(config->signer.pk_ctx, MBEDTLS_MD_SHA256, (const unsigned char *)config->signer.hash, config->signer.hashSize, config->signer.signature, &sigLen, mbedtls_ctr_drbg_random, config->signer.ctr_drbg_ctx);
        if (ret != 0)
        {
            char *temp = (char *)ut->newP(100);
            mbedtls_strerror(ret, temp, 100);
            config->signer.tokens.error.message = temp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_pk_sign: "));
            ut->delP(&temp);
            setTokenError(FIREBASE_ERROR_TOKEN_SIGN);
            sendTokenStatusCB();
        }
        else
        {
            config->signer.encSignature.clear();
            size_t len = ut->base64EncLen(config->signer.signatureSize);
            char *buf = (char *)ut->newP(len);
            ut->encodeBase64Url(buf, config->signer.signature, config->signer.signatureSize);
            config->signer.encSignature = buf;
            ut->delP(&buf);

            config->signer.tokens.jwt += config->signer.encSignature;
            config->signer.pk.clear();
            config->signer.encSignature.clear();
        }

        ut->delP(&config->signer.signature);
        ut->delP(&config->signer.hash);
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
#elif defined(ESP8266)
        // RSA private key
        BearSSL::PrivateKey *pk = nullptr;
        ut->idle();
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

        ut->idle();
        int ret = br_rsa_i15_pkcs1_sign(BR_HASH_OID_SHA256, (const unsigned char *)config->signer.hash, br_sha256_SIZE, br_rsa_key, config->signer.signature);
        ut->idle();
        ut->delP(&config->signer.hash);

        size_t len = ut->base64EncLen(config->signer.signatureSize);
        char *buf = (char *)ut->newP(len);
        ut->encodeBase64Url(buf, config->signer.signature, config->signer.signatureSize);
        config->signer.encSignature = buf;
        ut->delP(&buf);
        ut->delP(&config->signer.signature);
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

    return true;
}

bool Firebase_Signer::getIdToken(bool createUser, MB_StringPtr email, MB_StringPtr password)
{

    config->signer.signup = false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->internal.fb_processing)
        return false;

    if (!initClient(createUser ? fb_esp_pgm_str_250 : fb_esp_pgm_str_193, createUser ? token_status_uninitialized : token_status_on_request))
        return false;

    if (createUser)
    {
        MB_String _email = email, _password = password;
        config->signer.signupError.message.clear();

        if (_email.length() > 0 && _password.length() > 0)
        {
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_196), _email);
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_197), _password);
        }
    }
    else
    {
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_196), auth->user.email.c_str());
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_197), auth->user.password.c_str());
    }

    jsonPtr->add(pgm2Str(fb_esp_pgm_str_198), true);

    MB_String req = fb_esp_pgm_str_24;
    req += fb_esp_pgm_str_6;

    if (createUser)
        req += fb_esp_pgm_str_259;
    else
    {
        req += fb_esp_pgm_str_194;
        req += fb_esp_pgm_str_195;
    }

    req += config->api_key;
    req += fb_esp_pgm_str_30;

    req += fb_esp_pgm_str_31;
    if (createUser)
        req += fb_esp_pgm_str_250;
    else
        req += fb_esp_pgm_str_193;
    req += fb_esp_pgm_str_4;
    req += fb_esp_pgm_str_120;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_32;
    req += fb_esp_pgm_str_12;
    req += strlen(jsonPtr->raw());
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_8;
    req += fb_esp_pgm_str_129;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_21;

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());

    req.clear();

    if (response_code < 0)
    {
        return handleSignerError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);
    }

    jsonPtr->clear();

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        if (parseJsonResponse(fb_esp_pgm_str_257))
        {
            error.code = resultPtr->to<int>();
            if (!createUser)
                config->signer.tokens.status = token_status_error;

            if (parseJsonResponse(fb_esp_pgm_str_258))
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

            if (parseJsonResponse(fb_esp_pgm_str_200))
            {
                config->internal.auth_token = resultPtr->to<const char *>();
                config->internal.atok_len = strlen(resultPtr->to<const char *>());
                config->internal.ltok_len = 0;
            }

            if (parseJsonResponse(fb_esp_pgm_str_201))
            {
                config->internal.refresh_token = resultPtr->to<const char *>();
                config->internal.rtok_len = strlen(resultPtr->to<const char *>());
            }

            if (parseJsonResponse(fb_esp_pgm_str_202))
                getExpiration(resultPtr->to<const char *>());

            if (parseJsonResponse(fb_esp_pgm_str_175))
                auth->token.uid = resultPtr->to<const char *>();

            if (!createUser)
                return handleSignerError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
            else
                return handleSignerError(FIREBASE_ERROR_TOKEN_COMPLETE_UNNOTIFY);
        }
    }

    return handleSignerError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);
}

bool Firebase_Signer::deleteIdToken(MB_StringPtr idToken)
{

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->internal.fb_processing)
        return false;

    if (!initClient(fb_esp_pgm_str_250, token_status_uninitialized))
        return false;

    config->signer.signup = false;

    config->internal.fb_processing = true;

    MB_String _idToken = idToken;
    if (_idToken.length() > 0)
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_200), _idToken);
    else
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_200), config->internal.auth_token);

    MB_String req = fb_esp_pgm_str_24;
    req += fb_esp_pgm_str_6;

    req += fb_esp_pgm_str_582;

    req += config->api_key;
    req += fb_esp_pgm_str_30;

    req += fb_esp_pgm_str_31;
    req += fb_esp_pgm_str_250;

    req += fb_esp_pgm_str_4;
    req += fb_esp_pgm_str_120;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_32;
    req += fb_esp_pgm_str_12;
    req += strlen(jsonPtr->raw());
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_8;
    req += fb_esp_pgm_str_129;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_21;

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());
    req.clear();
    if (response_code < 0)
        return handleSignerError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        if (parseJsonResponse(fb_esp_pgm_str_257))
        {
            error.code = resultPtr->to<int>();
            if (parseJsonResponse(fb_esp_pgm_str_258))
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

    return handleSignerError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);
}

void Firebase_Signer::setAutoReconnectWiFi(bool reconnect)
{
    autoReconnectWiFi = reconnect;
}

void Firebase_Signer::setTCPClient(FB_TCP_CLIENT *tcpClient)
{
    this->tcpClient = tcpClient;
}

void Firebase_Signer::setNetworkStatus(bool status)
{
    networkStatus = status;
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    if (tcpClient)
        tcpClient->setNetworkStatus(networkStatus);
#endif
}

bool Firebase_Signer::initClient(PGM_P subDomain, fb_esp_auth_token_status status)
{

    ut->idle();

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

    if (tcpClient && intTCPClient)
    {
        delete tcpClient;
        tcpClient = nullptr;
        intTCPClient = false;
    }

    // No FirebaseData Object declared globally?
    if (!tcpClient)
    {
        tcpClient = new FB_TCP_CLIENT();

        if (tcpClient)
            intTCPClient = true;

        tcpClient->setMBFS(mbfs);
        tcpClient->setConfig(config);

        // No callbacks i.e., tcpConnectionCB, networkConnectionCB and networkStatusCB for external Client are available
        // when FirebaseData Object was not declared globally
    }

    // stop the TCP session
    tcpClient->stop();

#if !defined(FB_ENABLE_EXTERNAL_CLIENT)
    if (tcpClient->client)
        tcpClient->setCACert(nullptr);
#endif

    if (tcpClient->client)
        networkStatus = tcpClient->networkReady();

    if (!networkStatus)
    {
        if (autoReconnectWiFi)
        {
            if (millis() - last_reconnect_millis > reconnect_tmo)
            {
                tcpClient->networkReconnect();
                last_reconnect_millis = millis();
            }
        }
    }

    if (!tcpClient->isInitialized())
        return handleSignerError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED);

#if defined(ESP8266) && !defined(FB_ENABLE_EXTERNAL_CLIENT)
    tcpClient->setBufferSizes(1024, 1024);
#endif
    jsonPtr = new FirebaseJson();
    resultPtr = new FirebaseJsonData();

    MB_String host = subDomain;
    host += fb_esp_pgm_str_4;
    host += fb_esp_pgm_str_120;

    // use for authentication task
    tcpClient->reserved = true;

    ut->idle();
    tcpClient->begin(host.c_str(), 443, &response_code);

#if defined(FB_ENABLE_EXTERNAL_CLIENT)

    tcpClient->tcp_connection_cb(host.c_str(), 443);

#endif

    return true;
}

bool Firebase_Signer::requestTokens(bool refresh)
{
    time_t now = getTime();

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || ((unsigned long)now < ESP_DEFAULT_TS && !refresh && !config->signer.customTokenCustomSet) || config->internal.fb_processing)
        return false;

    if (!initClient(fb_esp_pgm_str_193, refresh ? token_status_on_refresh : token_status_on_request))
        return false;

    MB_String req = fb_esp_pgm_str_24;
    req += fb_esp_pgm_str_6;

    if (config->signer.tokens.token_type == token_type_custom_token)
    {
        if (config->signer.customTokenCustomSet)
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_233), config->internal.auth_token.c_str());
        else
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_233), config->signer.tokens.jwt.c_str());

        jsonPtr->add(pgm2Str(fb_esp_pgm_str_198), true);

        req += fb_esp_pgm_str_194;
        req += fb_esp_pgm_str_232;
        req += config->api_key;
        req += fb_esp_pgm_str_30;
        req += fb_esp_pgm_str_31;
        req += fb_esp_pgm_str_193;
    }
    else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
    {
        if (refresh)
        {
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_253), config->internal.client_id.c_str());
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_188), config->internal.client_secret.c_str());

            jsonPtr->add(pgm2Str(fb_esp_pgm_str_227), pgm2Str(fb_esp_pgm_str_206));
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_206), config->internal.refresh_token.c_str());
        }
        else
        {

            // rfc 7523, JWT Bearer Token Grant Type Profile for OAuth 2.0
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_227), pgm2Str(fb_esp_pgm_str_228));
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_229), config->signer.tokens.jwt.c_str());
        }

        req += fb_esp_pgm_str_1;
        req += fb_esp_pgm_str_233;
        req += fb_esp_pgm_str_30;
        req += fb_esp_pgm_str_31;
        req += fb_esp_pgm_str_251;
    }

    req += fb_esp_pgm_str_4;
    req += fb_esp_pgm_str_120;

    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_32;
    req += fb_esp_pgm_str_12;
    req += strlen(jsonPtr->raw());
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_8;
    req += fb_esp_pgm_str_129;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_21;

    req += jsonPtr->raw();

    tcpClient->send(req.c_str());

    req.clear();

    if (response_code < 0)
        return handleSignerError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    struct fb_esp_auth_token_error_t error;

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        config->signer.tokens.jwt.clear();
        if (parseJsonResponse(fb_esp_pgm_str_257))
        {
            error.code = resultPtr->to<int>();
            config->signer.tokens.status = token_status_error;

            if (parseJsonResponse(fb_esp_pgm_str_258))
                error.message = resultPtr->to<const char *>();
        }
        else if (parseJsonResponse(fb_esp_pgm_str_549))
        {
            error.code = -1;
            config->signer.tokens.status = token_status_error;

            if (parseJsonResponse(fb_esp_pgm_str_583))
                error.message = resultPtr->to<const char *>();
        }

        if (error.code != 0 && !config->signer.customTokenCustomSet && (config->signer.tokens.token_type == token_type_custom_token || config->signer.tokens.token_type == token_type_oauth2_access_token))
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

                if (parseJsonResponse(fb_esp_pgm_str_200))
                {
                    config->internal.auth_token = resultPtr->to<const char *>();
                    config->internal.atok_len = strlen(resultPtr->to<const char *>());
                    config->internal.ltok_len = 0;
                }

                if (parseJsonResponse(fb_esp_pgm_str_201))
                {
                    config->internal.refresh_token = resultPtr->to<const char *>();
                    config->internal.rtok_len = strlen(resultPtr->to<const char *>());
                }

                if (parseJsonResponse(fb_esp_pgm_str_202))
                    getExpiration(resultPtr->to<const char *>());
            }
            else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
            {
                if (parseJsonResponse(fb_esp_pgm_str_235))
                {
                    config->internal.auth_token = resultPtr->to<const char *>();
                    config->internal.atok_len = strlen(resultPtr->to<const char *>());
                    config->internal.ltok_len = 0;
                }

                if (parseJsonResponse(fb_esp_pgm_str_236))
                    config->signer.tokens.auth_type = resultPtr->to<const char *>();

                if (parseJsonResponse(fb_esp_pgm_str_210))
                    getExpiration(resultPtr->to<const char *>());
            }
            return handleSignerError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }
        return handleSignerError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleSignerError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);
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

    if (config->internal.fb_processing)
        return false;

    if (!initClient(fb_esp_pgm_str_193, token_status_uninitialized))
        return false;

    MB_String _payload = payload;

    config->internal.fb_processing = true;

    if (type == fb_esp_user_email_sending_type_verify)
    {
        config->signer.verificationError.message.clear();
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_260), pgm2Str(fb_esp_pgm_str_261));
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        config->signer.resetPswError.message.clear();
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_260), pgm2Str(fb_esp_pgm_str_263));
    }

    if (type == fb_esp_user_email_sending_type_verify)
    {
        if (_payload.length() > 0)
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_200), _payload);
        else
            jsonPtr->add(pgm2Str(fb_esp_pgm_str_200), config->internal.auth_token.c_str());
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        jsonPtr->add(pgm2Str(fb_esp_pgm_str_196), _payload);
    }

    MB_String s;
    jsonPtr->toString(s);

    MB_String req = fb_esp_pgm_str_24;
    req += fb_esp_pgm_str_6;

    req += fb_esp_pgm_str_194;
    req += fb_esp_pgm_str_262;

    req += config->api_key;
    req += fb_esp_pgm_str_30;

    req += fb_esp_pgm_str_31;

    req += fb_esp_pgm_str_193;
    req += fb_esp_pgm_str_4;
    req += fb_esp_pgm_str_120;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_32;
    req += fb_esp_pgm_str_12;
    req += s.length();
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_8;
    req += fb_esp_pgm_str_129;
    req += fb_esp_pgm_str_21;
    req += fb_esp_pgm_str_21;

    req += s.c_str();

    tcpClient->send(req.c_str());
    req.clear();
    if (response_code < 0)
        return handleSignerError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);

    jsonPtr->clear();

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        if (parseJsonResponse(fb_esp_pgm_str_257))
        {
            error.code = resultPtr->to<int>();
            if (parseJsonResponse(fb_esp_pgm_str_258))
                error.message = resultPtr->to<const char *>();
        }
        if (type == fb_esp_user_email_sending_type_verify)
            config->signer.verificationError = error;
        else if (type == fb_esp_user_email_sending_type_reset_psw)
            config->signer.resetPswError = error;

        if (error.code == 0)
        {
            jsonPtr->clear();
            return handleSignerError(FIREBASE_ERROR_TOKEN_COMPLETE_NOTIFY);
        }

        return handleSignerError(FIREBASE_ERROR_TOKEN_ERROR_UNNOTIFY);
    }

    return handleSignerError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT, httpCode);
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

    return config->signer.tokens.status == token_status_ready;
};

void Firebase_Signer::errorToString(int httpCode, MB_String &buff)
{
    buff.clear();

    if (config)
    {
        if (&config->signer.tokens.error.message != &buff && (config->signer.tokens.status == token_status_error || config->signer.tokens.error.code != 0))
        {
            buff = config->signer.tokens.error.message;
            return;
        }
    }

    switch (httpCode)
    {
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED:
        buff += fb_esp_pgm_str_39;
        return;
    case FIREBASE_ERROR_TCP_ERROR_SEND_REQUEST_FAILED:
        buff += fb_esp_pgm_str_40;
        return;
    case FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED:
        buff += fb_esp_pgm_str_42;
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:
        buff += fb_esp_pgm_str_43;
        return;
    case FIREBASE_ERROR_TCP_ERROR_NO_HTTP_SERVER:
        buff += fb_esp_pgm_str_44;
        return;
    case FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST:
        buff += fb_esp_pgm_str_45;
        return;
    case FIREBASE_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        buff += fb_esp_pgm_str_46;
        return;
    case FIREBASE_ERROR_HTTP_CODE_NO_CONTENT:
        buff += fb_esp_pgm_str_47;
        return;
    case FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY:
        buff += fb_esp_pgm_str_48;
        return;
    case FIREBASE_ERROR_HTTP_CODE_USE_PROXY:
        buff += fb_esp_pgm_str_49;
        return;
    case FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT:
        buff += fb_esp_pgm_str_50;
        return;
    case FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT:
        buff += fb_esp_pgm_str_51;
        return;
    case FIREBASE_ERROR_HTTP_CODE_UNAUTHORIZED:
        buff += fb_esp_pgm_str_52;
        return;
    case FIREBASE_ERROR_HTTP_CODE_FORBIDDEN:
        buff += fb_esp_pgm_str_53;
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_FOUND:
        buff += fb_esp_pgm_str_54;
        return;
    case FIREBASE_ERROR_HTTP_CODE_METHOD_NOT_ALLOWED:
        buff += fb_esp_pgm_str_55;
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_ACCEPTABLE:
        buff += fb_esp_pgm_str_56;
        return;
    case FIREBASE_ERROR_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        buff += fb_esp_pgm_str_57;
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT:
        buff += fb_esp_pgm_str_58;
        return;
    case FIREBASE_ERROR_HTTP_CODE_LENGTH_REQUIRED:
        buff += fb_esp_pgm_str_59;
        return;
    case FIREBASE_ERROR_HTTP_CODE_TOO_MANY_REQUESTS:
        buff += fb_esp_pgm_str_60;
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        buff += fb_esp_pgm_str_61;
        return;
    case FIREBASE_ERROR_HTTP_CODE_INTERNAL_SERVER_ERROR:
        buff += fb_esp_pgm_str_62;
        return;
    case FIREBASE_ERROR_HTTP_CODE_BAD_GATEWAY:
        buff += fb_esp_pgm_str_63;
        return;
    case FIREBASE_ERROR_HTTP_CODE_SERVICE_UNAVAILABLE:
        buff += fb_esp_pgm_str_64;
        return;
    case FIREBASE_ERROR_HTTP_CODE_GATEWAY_TIMEOUT:
        buff += fb_esp_pgm_str_65;
        return;
    case FIREBASE_ERROR_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        buff += fb_esp_pgm_str_66;
        return;
    case FIREBASE_ERROR_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        buff += fb_esp_pgm_str_67;
        return;
    case FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED:
        buff += fb_esp_pgm_str_152;
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT:
        buff += fb_esp_pgm_str_69;
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_READ_FAILED:
        buff += fb_esp_pgm_str_595;
        return;
    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        buff += fb_esp_pgm_str_70;
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        buff += fb_esp_pgm_str_71;
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED:
        buff += fb_esp_pgm_str_94;
        return;
    case FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED:
        buff += fb_esp_pgm_str_169;
        return;
    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        buff += fb_esp_pgm_str_68;
        return;
    case FIREBASE_ERROR_NO_FCM_ID_TOKEN_PROVIDED:
        buff += fb_esp_pgm_str_145;
        return;
    case FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED:
        buff += fb_esp_pgm_str_146;
        return;
    case FIREBASE_ERROR_NO_FCM_TOPIC_PROVIDED:
        buff += fb_esp_pgm_str_542;
        return;
    case FIREBASE_ERROR_FCM_ID_TOKEN_AT_INDEX_NOT_FOUND:
        buff += fb_esp_pgm_str_543;
        return;
    case FIREBASE_ERROR_EXPECTED_JSON_DATA:
        buff += fb_esp_pgm_str_185;
        return;
    case FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE:
        buff += fb_esp_pgm_str_189;
        return;
    case FIREBASE_ERROR_CANNOT_CONFIG_TIME:
        buff += fb_esp_pgm_str_190;
        return;
    case FIREBASE_ERROR_SSL_RX_BUFFER_SIZE_TOO_SMALL:
        buff += fb_esp_pgm_str_191;
        return;
    case MB_FS_ERROR_FILE_IO_ERROR:
        buff += fb_esp_pgm_str_192;
        return;
#if defined(FIREBASE_ESP_CLIENT)
    case FIREBASE_ERROR_ARCHIVE_NOT_FOUND:
        buff += fb_esp_pgm_str_450;
        return;
    case FIREBASE_ERROR_LONG_RUNNING_TASK:
        buff += fb_esp_pgm_str_534;
        return;
    case FIREBASE_ERROR_UPLOAD_TIME_OUT:
        buff += fb_esp_pgm_str_540;
        return;
    case FIREBASE_ERROR_UPLOAD_DATA_ERRROR:
        buff += fb_esp_pgm_str_541;
        return;
    case FIREBASE_ERROR_OAUTH2_REQUIRED:
        buff += fb_esp_pgm_str_328;
        return;
    case FIREBASE_ERROR_FW_UPDATE_INVALID_FIRMWARE:
        buff += fb_esp_pgm_str_584;
        return;
    case FIREBASE_ERROR_FW_UPDATE_TOO_LOW_FREE_SKETCH_SPACE:
        buff += fb_esp_pgm_str_585;
        return;
    case FIREBASE_ERROR_FW_UPDATE_BIN_SIZE_NOT_MATCH_SPI_FLASH_SPACE:
        buff += fb_esp_pgm_str_586;
        return;
    case FIREBASE_ERROR_FW_UPDATE_BEGIN_FAILED:
        buff += fb_esp_pgm_str_587;
        return;
    case FIREBASE_ERROR_FW_UPDATE_WRITE_FAILED:
        buff += fb_esp_pgm_str_588;
        return;
    case FIREBASE_ERROR_FW_UPDATE_END_FAILED:
        buff += fb_esp_pgm_str_589;
        return;
#endif
    case FIREBASE_ERROR_TOKEN_NOT_READY:
        buff += fb_esp_pgm_str_252;
        return;
    case FIREBASE_ERROR_UNINITIALIZED:
        buff += fb_esp_pgm_str_256;
        return;
    case FIREBASE_ERROR_MISSING_DATA:
        buff += fb_esp_pgm_str_579;
        return;
    case FIREBASE_ERROR_MISSING_CREDENTIALS:
        buff += fb_esp_pgm_str_580;
        return;
    case FIREBASE_ERROR_INVALID_JSON_RULES:
        buff += fb_esp_pgm_str_581;
        return;
    case FIREBASE_ERROR_TOKEN_SET_TIME:
        buff += fb_esp_pgm_str_211;
        break;
    case FIREBASE_ERROR_TOKEN_PARSE_PK:
        buff += fb_esp_pgm_str_179;
        break;
    case FIREBASE_ERROR_TOKEN_CREATE_HASH:
        buff += fb_esp_pgm_str_545;
        break;
    case FIREBASE_ERROR_TOKEN_SIGN:
        buff += fb_esp_pgm_str_178;
        break;
    case FIREBASE_ERROR_TOKEN_EXCHANGE:
        buff += fb_esp_pgm_str_177;
        break;

#if defined(MBFS_FLASH_FS) || defined(MBFS_SD_FS)

    case MB_FS_ERROR_FLASH_STORAGE_IS_NOT_READY:
        buff += fb_esp_pgm_str_590;
        return;

    case MB_FS_ERROR_SD_STORAGE_IS_NOT_READY:
        buff += fb_esp_pgm_str_591;
        return;

    case MB_FS_ERROR_FILE_STILL_OPENED:
        buff += fb_esp_pgm_str_592;
        return;

    case MB_FS_ERROR_FILE_NOT_FOUND:
        buff += fb_esp_pgm_str_593;
        return;
#endif

    case FIREBASE_ERROR_SYS_TIME_IS_NOT_READY:
        buff += fb_esp_pgm_str_594;
        return;
    case FIREBASE_ERROR_EXTERNAL_CLIENT_DISABLED:
        buff += fb_esp_pgm_str_596;
        return;
    case FIREBASE_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED:
        buff += fb_esp_pgm_str_597;
        return;
    case FIREBASE_ERROR_NTP_SYNC_TIMED_OUT:
        buff += fb_esp_pgm_str_230;
        return;

    default:
        return;
    }
}

fb_esp_auth_token_type Firebase_Signer::getTokenType()
{
    if (!config)
        return token_type_undefined;
    return config->signer.tokens.token_type;
}

const char *Firebase_Signer::getToken()
{
    if (!config || config->internal.auth_token[0] == '?')
        return "";

    return config->internal.auth_token.c_str();
}

const char *Firebase_Signer::getRefreshToken()
{
    if (!config)
        return "";
    return config->internal.refresh_token.c_str();
}

FirebaseConfig *Firebase_Signer::getCfg()
{
    return config;
}

FirebaseAuth *Firebase_Signer::getAuth()
{
    return auth;
}

MB_FS *Firebase_Signer::getMBFS()
{
    return mbfs;
}

UtilsClass *Firebase_Signer::getUtils()
{
    return ut;
}

MB_String Firebase_Signer::getCAFile()
{
    if (!config)
        return "";
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