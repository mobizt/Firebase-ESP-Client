/**
 * Google's Firebase Token Generation class, Signer.cpp version 1.2.6
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created November 5, 2021
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

#ifndef FIREBASE_SIGNER_CPP
#define FIREBASE_SIGNER_CPP
#include "Signer.h"

Firebase_Signer::Firebase_Signer()
{
}

Firebase_Signer::~Firebase_Signer()
{
}

void Firebase_Signer::begin(UtilsClass *utils, FirebaseConfig *cfg, FirebaseAuth *authen)
{
    ut = utils;
    config = cfg;
    auth = authen;
}

bool Firebase_Signer::parseSAFile()
{
    if (config->signer.pk.length() > 0)
        return false;

    if (config->service_account.json.storage_type == mem_storage_type_sd && !config->_int.fb_sd_rdy)
        config->_int.fb_sd_rdy = ut->sdTest(config->_int.fb_file);
    else if (config->service_account.json.storage_type == mem_storage_type_flash && !config->_int.fb_flash_rdy)
        ut->flashTest();

    if (config->_int.fb_sd_rdy || config->_int.fb_flash_rdy)
    {
        if (config->service_account.json.storage_type == mem_storage_type_flash)
        {
            if (FLASH_FS.exists(config->service_account.json.path.c_str()))
                config->_int.fb_file = FLASH_FS.open(config->service_account.json.path.c_str(), "r");
        }
        else
        {
            if (SD_FS.exists(config->service_account.json.path.c_str()))
                config->_int.fb_file = SD_FS.open(config->service_account.json.path.c_str(), "r");
        }

        if (config->_int.fb_file)
        {
            clearSA();
            config->signer.json = new FirebaseJson();
            config->signer.result = new FirebaseJsonData();
            char *tmp = nullptr;

            size_t len = config->_int.fb_file.size();
            char *buf = (char *)ut->newP(len + 10);
            if (config->_int.fb_file.available())
            {
                config->_int.fb_file.readBytes(buf, len);
                config->signer.json->setJsonData(buf);
            }
            config->_int.fb_file.close();
            ut->delP(&buf);

            tmp = ut->strP(fb_esp_pgm_str_243);
            config->signer.json->get(*config->signer.result, (const char *)tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
            {
                if (ut->strposP(config->signer.result->to<const char *>(), fb_esp_pgm_str_244, 0) > -1)
                {
                    tmp = ut->strP(fb_esp_pgm_str_245);
                    config->signer.json->get(*config->signer.result, (const char *)tmp);
                    ut->delP(&tmp);
                    if (config->signer.result->success)
                        config->service_account.data.project_id = config->signer.result->to<const char *>();
                    tmp = ut->strP(fb_esp_pgm_str_246);
                    config->signer.json->get(*config->signer.result, (const char *)tmp);
                    ut->delP(&tmp);
                    if (config->signer.result->success)
                        config->service_account.data.private_key_id = config->signer.result->to<const char *>();
                    tmp = ut->strP(fb_esp_pgm_str_247);
                    config->signer.json->get(*config->signer.result, (const char *)tmp);
                    ut->delP(&tmp);
                    if (config->signer.result->success)
                    {
                        tmp = (char *)ut->newP(strlen(config->signer.result->to<const char *>()));
                        size_t c = 0;
                        for (size_t i = 0; i < strlen(config->signer.result->to<const char *>()); i++)
                        {
                            if (config->signer.result->to<const char *>()[i] == '\\')
                            {
                                ut->idle();
                                tmp[c++] = '\n';
                                i++;
                            }
                            else
                                tmp[c++] = config->signer.result->to<const char *>()[i];
                        }
                        config->signer.pk = tmp;
                        config->signer.result->clear();
                        ut->delP(&tmp);
                    }

                    tmp = ut->strP(fb_esp_pgm_str_248);
                    config->signer.json->get(*config->signer.result, (const char *)tmp);
                    ut->delP(&tmp);
                    if (config->signer.result->success)
                        config->service_account.data.client_email = config->signer.result->to<const char *>();
                    tmp = ut->strP(fb_esp_pgm_str_253);
                    config->signer.json->get(*config->signer.result, (const char *)tmp);
                    ut->delP(&tmp);
                    if (config->signer.result->success)
                        config->service_account.data.client_id = config->signer.result->to<const char *>();

                    delete config->signer.json;
                    delete config->signer.result;
                    return true;
                }
            }

            delete config->signer.json;
            delete config->signer.result;
        }
    }

    return false;
}

void Firebase_Signer::clearSA()
{
    config->service_account.data.private_key = "";
    ut->clearS(config->service_account.data.project_id);
    ut->clearS(config->service_account.data.private_key_id);
    ut->clearS(config->service_account.data.client_email);
    ut->clearS(config->signer.pk);
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

bool Firebase_Signer::isExpired()
{
    if (!config || !auth)
        return false;

    //if the time was set (changed) after token has been generated, update its expiration
    if (config->signer.tokens.expires > 0 && config->signer.tokens.expires < ESP_DEFAULT_TS && time(nullptr) > ESP_DEFAULT_TS)
        config->signer.tokens.expires += time(nullptr) - (millis() - config->signer.tokens.last_millis) / 1000 - 60;

    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;

    return ((unsigned long)time(nullptr) > config->signer.tokens.expires - config->signer.preRefreshSeconds || config->signer.tokens.expires == 0);
}

bool Firebase_Signer::handleToken()
{
    if (!config || !auth)
        return false;

#if defined(ESP8266)
    if ((config->cert.data != NULL || config->cert.file.length() > 0) && !config->_int.fb_clock_rdy)
    {
        ut->idle();
        time_t now = time(nullptr);
        config->_int.fb_clock_rdy = now > ut->default_ts;

        if (!config->_int.fb_clock_rdy)
        {
            ut->setClock(config->time_zone);

            if (config->signer.tokens.status == token_status_uninitialized)
            {
                config->signer.tokens.status = token_status_on_initialize;
                config->signer.tokens.error.code = 0;
                ut->clearS(config->signer.tokens.error.message);
                config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                sendTokenStatusCB();
            }
            return false;
        }
    }
#endif

    if (isAuthToken(true) && isExpired())
    {
        if (config->signer.tokens.expires > 0 && isAuthToken(false))
        {
            if (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0)
            {
                if (config->signer.idTokenCutomSet && auth->user.email.length() == 0 && auth->user.password.length() == 0 && config->signer.anonymous)
                    return true;

                if (config->_int.fb_processing)
                    return false;

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
                if (config->signer.step == fb_esp_jwt_generation_step_begin)
                {

                    if (!config->signer.tokenTaskRunning)
                    {
                        if (config->service_account.json.path.length() > 0 && config->signer.pk.length() == 0)
                        {
                            if (!parseSAFile())
                                config->signer.tokens.status = token_status_uninitialized;
                        }

                        config->signer.tokens.status = token_status_on_initialize;
                        config->signer.tokens.error.code = 0;
                        ut->clearS(config->signer.tokens.error.message);
                        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                        sendTokenStatusCB();

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
            setTokenError(FIREBASE_ERROR_TOKEN_NOT_READY);

        return config->signer.tokens.status == token_status_ready;
    }
}

void Firebase_Signer::tokenProcessingTask()
{

#if defined(ESP32)

    if (config->signer.tokenTaskRunning)
        return;

    bool ret = false;

    config->signer.tokenTaskRunning = true;

    while (!ret && config->signer.tokens.status != token_status_ready)
    {
        delay(0);

        if (config->signer.tokens.token_type == token_type_id_token)
        {
            config->signer.lastReqMillis = millis();

            if (getIdToken(false, "", ""))
            {
                _token_processing_task_enable = false;
                config->signer.attempts = 0;
                ret = true;
            }
            else
            {
                if (config->signer.attempts < config->max_token_generation_retry)
                    config->signer.attempts++;
                else
                {
                    ut->clearS(config->signer.tokens.error.message);
                    setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
                    config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                    sendTokenStatusCB();
                    _token_processing_task_enable = false;
                    config->signer.attempts = 0;
                    ret = true;
                }
            }
        }
        else
        {
            if (config->signer.step == fb_esp_jwt_generation_step_begin && (millis() - config->_int.fb_last_jwt_begin_step_millis > config->timeout.tokenGenerationBeginStep || config->_int.fb_last_jwt_begin_step_millis == 0))
            {
                config->_int.fb_last_jwt_begin_step_millis = millis();
                ut->setClock(config->time_zone);
                time_t now = time(nullptr);
                config->_int.fb_clock_rdy = now > ut->default_ts;

                if (config->_int.fb_clock_rdy)
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
                if (requestTokens())
                {
                    config->signer.attempts = 0;
                    _token_processing_task_enable = false;
                    config->signer.step = fb_esp_jwt_generation_step_begin;
                    ret = true;
                }
                else
                {
                    if (config->signer.attempts < config->max_token_generation_retry)
                        config->signer.attempts++;
                    else
                    {
                        ut->clearS(config->signer.tokens.error.message);
                        setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
                        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                        sendTokenStatusCB();
                        config->signer.attempts = 0;
                        config->signer.step = fb_esp_jwt_generation_step_begin;
                        ret = true;
                    }
                }
            }
        }
    }

    config->signer.tokenTaskRunning = false;

#elif defined(ESP8266)

    if (_token_processing_task_enable && config->signer.tokens.status != token_status_ready)
    {
        if (config->signer.tokens.token_type == token_type_id_token)
        {
            config->signer.tokenTaskRunning = true;

            config->signer.lastReqMillis = millis();
            if (getIdToken(false, "", ""))
            {
                _token_processing_task_enable = false;
                config->signer.attempts = 0;
                config->signer.tokenTaskRunning = false;
                return;
            }
            else
            {
                if (config->signer.attempts < config->max_token_generation_retry)
                    config->signer.attempts++;
                else
                {
                    ut->clearS(config->signer.tokens.error.message);
                    setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
                    config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                    sendTokenStatusCB();
                    _token_processing_task_enable = false;
                    config->signer.attempts = 0;
                    config->signer.tokenTaskRunning = false;
                    return;
                }
            }
        }
        else
        {
            if (config->signer.step == fb_esp_jwt_generation_step_begin && (millis() - config->_int.fb_last_jwt_begin_step_millis > 200 || config->_int.fb_last_jwt_begin_step_millis == 0))
            {
                config->_int.fb_last_jwt_begin_step_millis = millis();
                config->signer.tokenTaskRunning = true;
                ut->setClock(config->time_zone);
                time_t now = time(nullptr);
                config->_int.fb_clock_rdy = now > ut->default_ts;

                if (config->_int.fb_clock_rdy)
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
                if (requestTokens())
                {
                    config->signer.tokenTaskRunning = false;
                    _token_processing_task_enable = false;
                    config->signer.attempts = 0;
                    config->signer.step = fb_esp_jwt_generation_step_begin;
                    return;
                }
                else
                {
                    if (config->signer.attempts < config->max_token_generation_retry)
                        config->signer.attempts++;
                    else
                    {
                        ut->clearS(config->signer.tokens.error.message);
                        setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
                        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                        sendTokenStatusCB();
                        config->signer.tokenTaskRunning = false;
                        _token_processing_task_enable = false;
                        config->signer.attempts = 0;
                        config->signer.step = fb_esp_jwt_generation_step_begin;
                        return;
                    }
                }
            }
        }

        set_scheduled_callback(std::bind(&Firebase_Signer::tokenProcessingTask, this));
    }

#endif
}

bool Firebase_Signer::refreshToken()
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
        return false;

    ut->idle();

    if (auth == nullptr)
        return false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->_int.fb_processing)
        return false;

    if (config->_int.ltok_len > 0 || (config->_int.rtok_len == 0 && config->_int.atok_len == 0))
        return false;

    config->signer.tokens.status = token_status_on_refresh;
    config->_int.fb_processing = true;
    config->signer.tokens.error.code = 0;
    ut->clearS(config->signer.tokens.error.message);
    config->_int.fb_last_jwt_generation_error_cb_millis = 0;
    sendTokenStatusCB();
    ut->clearS(config->_int.auth_token);

#if defined(ESP32)
    config->signer.wcs = new FB_TCP_Client();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(1024, 1024);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.result = new FirebaseJsonData();

    MBSTRING host;
    ut->appendP(host, fb_esp_pgm_str_203);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);
#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)

    ut->ethDNSWorkAround(&ut->config->spi_ethernet_module, host.c_str(), 443);
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_205);
    char *tmp2 = ut->strP(fb_esp_pgm_str_206);
    config->signer.json->add(tmp, (const char *)tmp2);
    ut->delP(&tmp);
    ut->delP(&tmp2);
    tmp = ut->strP(fb_esp_pgm_str_207);
    config->signer.json->add(tmp, config->_int.refresh_token.c_str());
    ut->delP(&tmp);

    MBSTRING s;
    config->signer.json->toString(s);

    MBSTRING req;
    ut->appendP(req, fb_esp_pgm_str_24);
    ut->appendP(req, fb_esp_pgm_str_6);
    ut->appendP(req, fb_esp_pgm_str_204);
    req += config->api_key;
    ut->appendP(req, fb_esp_pgm_str_30);

    ut->appendP(req, fb_esp_pgm_str_31);
    ut->appendP(req, fb_esp_pgm_str_203);
    ut->appendP(req, fb_esp_pgm_str_4);
    ut->appendP(req, fb_esp_pgm_str_120);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_32);
    ut->appendP(req, fb_esp_pgm_str_12);
    req += NUM2S(s.length()).get();
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += s.c_str();
#if defined(ESP32)
    int ret = config->signer.wcs->send(req.c_str());
    ut->clearS(req);
    if (ret < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    ut->clearS(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    struct fb_esp_auth_token_error_t error;

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.result, tmp);
        ut->delP(&tmp);

        if (config->signer.result->success)
        {
            error.code = config->signer.result->to<int>();
            config->signer.tokens.status = token_status_error;

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                error.message = config->signer.result->to<const char *>();
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {
            if (isAuthToken(false))
            {
                tmp = ut->strP(fb_esp_pgm_str_208);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                {
                    ut->storeS(config->_int.auth_token, config->signer.result->to<const char *>(), false);
                    config->_int.atok_len = strlen(config->signer.result->to<const char *>());
                    config->_int.ltok_len = 0;
                }

                tmp = ut->strP(fb_esp_pgm_str_209);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                {
                    ut->storeS(config->_int.refresh_token, config->signer.result->to<const char *>(), false);
                    config->_int.rtok_len = strlen(config->signer.result->to<const char *>());
                }

                tmp = ut->strP(fb_esp_pgm_str_210);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                    getExpiration(config->signer.result->to<const char *>());

                tmp = ut->strP(fb_esp_pgm_str_175);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                    auth->token.uid = config->signer.result->to<const char *>();
            }
            return handleSignerError(0);
        }

        return handleSignerError(4);
    }

    return handleSignerError(3, httpCode);
}

void Firebase_Signer::setTokenError(int code)
{
    if (code != 0)
        config->signer.tokens.status = token_status_error;
    else
    {
        ut->clearS(config->signer.tokens.error.message);
        config->signer.tokens.status = token_status_ready;
    }

    config->signer.tokens.error.code = code;

    if (config->signer.tokens.error.message.length() == 0)
    {
        config->_int.fb_processing = false;
        switch (code)
        {
        case FIREBASE_ERROR_TOKEN_SET_TIME:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_211, true);
            break;
        case FIREBASE_ERROR_TOKEN_PARSE_PK:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_179, true);
            break;
        case FIREBASE_ERROR_TOKEN_CREATE_HASH:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_545, true);
            break;
        case FIREBASE_ERROR_TOKEN_SIGN:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_178, true);
            break;
        case FIREBASE_ERROR_TOKEN_EXCHANGE:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_177, true);
            break;
        case FIREBASE_ERROR_TOKEN_NOT_READY:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_252, true);
            break;
        case FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_547, true);
            break;
        case FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_42);
            break;
        case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_43);
            break;
        case FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_58);
            break;

        default:
            break;
        }
    }
}

bool Firebase_Signer::handleSignerError(int code, int httpCode)
{

    switch (code)
    {

    case 1:
        ut->clearS(config->signer.tokens.error.message);
        setTokenError(FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED);
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();
        break;
    case 2:
#if defined(ESP32)
        if (config->signer.wcs->stream())
            config->signer.wcs->stream()->stop();
#elif defined(ESP8266)
        config->signer.wcs->stop();
#endif
        ut->clearS(config->signer.tokens.error.message);
        setTokenError(FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST);
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();
        break;
    case 3:
#if defined(ESP32)
        if (config->signer.wcs->stream())
            config->signer.wcs->stream()->stop();
#elif defined(ESP8266)
        config->signer.wcs->stop();
#endif

        if (httpCode == 0)
        {
            setTokenError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT);
            ut->clearS(config->signer.tokens.error.message);
        }
        else
        {
            errorToString(httpCode, config->signer.tokens.error.message);
            setTokenError(httpCode);
        }
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();

        break;

    default:
        break;
    }

    if (config->signer.wcs)
        delete config->signer.wcs;
    if (config->signer.json)
        delete config->signer.json;
    if (config->signer.result)
        delete config->signer.result;

    config->_int.fb_processing = false;

    if (code > 0 && code < 4)
    {
        config->signer.tokens.status = token_status_error;
        config->signer.tokens.error.code = code;
        return false;
    }
    else if (code <= 0)
    {
        ut->clearS(config->signer.tokens.error.message);
        config->signer.tokens.status = token_status_ready;
        config->signer.attempts = 0;
        config->signer.step = fb_esp_jwt_generation_step_begin;
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        if (code == 0)
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
        if (millis() - config->_int.fb_last_jwt_generation_error_cb_millis > config->timeout.tokenGenerationError || config->_int.fb_last_jwt_generation_error_cb_millis == 0)
        {
            config->_int.fb_last_jwt_generation_error_cb_millis = millis();
            config->token_status_callback(tokenInfo);
        }
    }
}

bool Firebase_Signer::handleTokenResponse(int &httpCode)
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
        return false;

    struct server_response_data_t response;

    unsigned long dataTime = millis();

    int chunkIdx = 0;
    int chunkBufSize = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    MBSTRING header, payload;
    bool isHeader = false;
#if defined(ESP32)
    WiFiClient *stream = config->signer.wcs->stream();
#elif defined(ESP8266)
    WiFiClient *stream = config->signer.wcs;
#endif
    while (stream->connected() && stream->available() == 0)
    {
        if (!ut->reconnect(dataTime))
        {
            if (stream)
                if (stream->connected())
                    stream->stop();
            return false;
        }

        ut->idle();
    }

    bool complete = false;
    unsigned long datatime = millis();
    while (!complete)
    {

        chunkBufSize = stream->available();

        if (chunkBufSize > 1 || !complete)
        {
            while (!complete)
            {
                ut->idle();

                if (config->_int.fb_reconnect_wifi)
                    ut->reconnect(0);

                if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
                {
                    if (stream)
                        if (stream->connected())
                            stream->stop();
                    return false;
                }
                chunkBufSize = stream->available();

                if (chunkBufSize > 0)
                {
                    if (chunkIdx == 0)
                    {
                        ut->readLine(stream, header);
                        int pos = 0;
                        char *tmp = ut->getHeader(header.c_str(), fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                        if (tmp)
                        {
                            isHeader = true;
                            response.httpCode = atoi(tmp);
                            ut->delP(&tmp);
                        }
                    }
                    else
                    {
                        if (isHeader)
                        {
                            char *tmp = (char *)ut->newP(chunkBufSize);
                            int readLen = ut->readLine(stream, tmp, chunkBufSize);
                            bool headerEnded = false;

                            if (readLen == 1)
                                if (tmp[0] == '\r')
                                    headerEnded = true;

                            if (readLen == 2)
                                if (tmp[0] == '\r' && tmp[1] == '\n')
                                    headerEnded = true;

                            if (headerEnded)
                            {
                                isHeader = false;
                                ut->parseRespHeader(header.c_str(), response);
                                ut->clearS(header);
                            }
                            else
                                header += tmp;

                            ut->delP(&tmp);
                        }
                        else
                        {
                            if (!response.noContent)
                            {
                                if (response.isChunkedEnc)
                                    complete = ut->readChunkedData(stream, payload, chunkedDataState, chunkedDataSize, chunkedDataLen) < 0;
                                else
                                {
                                    chunkBufSize = 1024;
                                    if (stream->available() < chunkBufSize)
                                        chunkBufSize = stream->available();

                                    char *tmp = (char *)ut->newP(chunkBufSize + 1);
                                    int readLen = stream->readBytes(tmp, chunkBufSize);

                                    if (readLen > 0)
                                        payload += tmp;

                                    ut->delP(&tmp);
                                    complete = stream->available() <= 0;
                                }
                            }
                            else
                            {
                                while (stream->available() > 0)
                                    stream->read();
                                if (stream->available() <= 0)
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

    if (stream->connected())
        stream->stop();

    httpCode = response.httpCode;

    if (payload.length() > 0 && !response.noContent)
    {

        config->signer.json->setJsonData(payload.c_str());
        ut->clearS(payload);
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
        ut->clearS(config->signer.tokens.error.message);
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();

        config->signer.json = new FirebaseJson();
        config->signer.result = new FirebaseJsonData();

        unsigned long now = time(nullptr);

        ut->clearS(config->signer.tokens.jwt);

        //header
        char *tmp = ut->strP(fb_esp_pgm_str_239);
        char *tmp2 = ut->strP(fb_esp_pgm_str_242);
        config->signer.json->add(tmp, (const char *)tmp2);
        ut->delP(&tmp);
        ut->delP(&tmp2);
        tmp2 = ut->strP(fb_esp_pgm_str_234);
        tmp = ut->strP(fb_esp_pgm_str_240);
        config->signer.json->add(tmp, (const char *)tmp2);
        ut->delP(&tmp);
        ut->delP(&tmp2);

        MBSTRING hdr;
        config->signer.json->toString(hdr);
        size_t len = ut->base64EncLen(hdr.length());
        char *buf = (char *)ut->newP(len);
        ut->encodeBase64Url(buf, (unsigned char *)hdr.c_str(), hdr.length());
        config->signer.encHeader = buf;
        ut->delP(&buf);
        config->signer.encHeadPayload = config->signer.encHeader;
        ut->clearS(hdr);

        //payload
        config->signer.json->clear();
        tmp = ut->strP(fb_esp_pgm_str_212);
        config->signer.json->add(tmp, config->service_account.data.client_email.c_str());
        ut->delP(&tmp);
        tmp = ut->strP(fb_esp_pgm_str_213);
        config->signer.json->add(tmp, config->service_account.data.client_email.c_str());
        ut->delP(&tmp);
        tmp = ut->strP(fb_esp_pgm_str_214);
        MBSTRING t;
        ut->appendP(t, fb_esp_pgm_str_112);
        if (config->signer.tokens.token_type == token_type_custom_token)
        {
            ut->appendP(t, fb_esp_pgm_str_250);
            ut->appendP(t, fb_esp_pgm_str_4);
            ut->appendP(t, fb_esp_pgm_str_120);
            ut->appendP(t, fb_esp_pgm_str_231);
        }
        else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            ut->appendP(t, fb_esp_pgm_str_251);
            ut->appendP(t, fb_esp_pgm_str_4);
            ut->appendP(t, fb_esp_pgm_str_120);
            ut->appendP(t, fb_esp_pgm_str_1);
            ut->appendP(t, fb_esp_pgm_str_233);
        }

        config->signer.json->add(tmp, t.c_str());
        ut->delP(&tmp);

        tmp = ut->strP(fb_esp_pgm_str_218);
        config->signer.json->add(tmp, (int)now);
        ut->delP(&tmp);

        tmp = ut->strP(fb_esp_pgm_str_215);

        if (config->signer.expiredSeconds > 3600)
            config->signer.json->add(tmp, (int)(now + 3600));
        else
            config->signer.json->add(tmp, (int)(now + config->signer.expiredSeconds));

        ut->delP(&tmp);

        if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            MBSTRING buri;
            ut->appendP(buri, fb_esp_pgm_str_112);
            ut->appendP(buri, fb_esp_pgm_str_193);
            ut->appendP(buri, fb_esp_pgm_str_4);
            ut->appendP(buri, fb_esp_pgm_str_120);
            ut->appendP(buri, fb_esp_pgm_str_1);
            ut->appendP(buri, fb_esp_pgm_str_219);
            ut->appendP(buri, fb_esp_pgm_str_1);

            MBSTRING s = buri;
            ut->appendP(s, fb_esp_pgm_str_221);

            ut->appendP(s, fb_esp_pgm_str_6);
            s += buri;
            ut->appendP(s, fb_esp_pgm_str_222);

            ut->appendP(s, fb_esp_pgm_str_6);
            s += buri;
            ut->appendP(s, fb_esp_pgm_str_223);

            ut->appendP(s, fb_esp_pgm_str_6);
            s += buri;
            ut->appendP(s, fb_esp_pgm_str_224);

            ut->appendP(s, fb_esp_pgm_str_6);
            s += buri;
            ut->appendP(s, fb_esp_pgm_str_225);
#if defined(FIREBASE_ESP_CLIENT)
            ut->appendP(s, fb_esp_pgm_str_6);
            s += buri;
            ut->appendP(s, fb_esp_pgm_str_451);
#endif

            if (config->signer.tokens.scope.length() > 0)
            {
                std::vector<MBSTRING> scopes = std::vector<MBSTRING>();
                ut->splitTk(config->signer.tokens.scope, scopes, ",");
                for (size_t i = 0; i < scopes.size(); i++)
                {
                    ut->appendP(s, fb_esp_pgm_str_6);
                    s += scopes[i];
                    ut->clearS(scopes[i]);
                    ut->clearS(scopes[i]);
                }
                scopes.clear();
            }

            tmp = ut->strP(fb_esp_pgm_str_220);
            config->signer.json->add(tmp, s.c_str());
            ut->delP(&tmp);
        }
        else if (config->signer.tokens.token_type == token_type_custom_token)
        {
            tmp = ut->strP(fb_esp_pgm_str_254);
            config->signer.json->add(tmp, auth->token.uid.c_str());
            ut->delP(&tmp);

            if (auth->token.claims.length() > 2)
            {
                FirebaseJson claims(auth->token.claims.c_str());
                tmp = ut->strP(fb_esp_pgm_str_255);
                config->signer.json->add(tmp, claims);
                ut->delP(&tmp);
            }
        }

        MBSTRING payload;
        config->signer.json->toString(payload);

        len = ut->base64EncLen(payload.length());
        buf = (char *)ut->newP(len);
        ut->encodeBase64Url(buf, (unsigned char *)payload.c_str(), payload.length());
        config->signer.encPayload = buf;
        ut->delP(&buf);
        ut->clearS(payload);

        ut->appendP(config->signer.encHeadPayload, fb_esp_pgm_str_4);
        config->signer.encHeadPayload += config->signer.encPayload;

        ut->clearS(config->signer.encHeader);
        ut->clearS(config->signer.encPayload);

//create message digest from encoded header and payload
#if defined(ESP32)
        config->signer.hash = (uint8_t *)ut->newP(config->signer.hashSize);
        int ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (const unsigned char *)config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length(), config->signer.hash);
        if (ret != 0)
        {
            char *tmp = (char *)ut->newP(100);
            mbedtls_strerror(ret, tmp, 100);
            config->signer.tokens.error.message = tmp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_md: "));
            ut->delP(&tmp);
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
        ut->appendP(config->signer.tokens.jwt, fb_esp_pgm_str_4);
        ut->clearS(config->signer.encHeadPayload);

        delete config->signer.json;
        delete config->signer.result;
    }
    else if (config->signer.step == fb_esp_jwt_generation_step_sign)
    {
        config->signer.tokens.status = token_status_on_signing;

#if defined(ESP32)
        config->signer.pk_ctx = new mbedtls_pk_context();
        mbedtls_pk_init(config->signer.pk_ctx);

        //parse priv key
        int ret = 0;
        if (config->signer.pk.length() > 0)
            ret = mbedtls_pk_parse_key(config->signer.pk_ctx, (const unsigned char *)config->signer.pk.c_str(), config->signer.pk.length() + 1, NULL, 0);
        else if (strlen_P(config->service_account.data.private_key) > 0)
            ret = mbedtls_pk_parse_key(config->signer.pk_ctx, (const unsigned char *)config->service_account.data.private_key, strlen_P(config->service_account.data.private_key) + 1, NULL, 0);

        if (ret != 0)
        {
            char *tmp = (char *)ut->newP(100);
            mbedtls_strerror(ret, tmp, 100);
            config->signer.tokens.error.message = tmp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_pk_parse_key: "));
            ut->delP(&tmp);
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            sendTokenStatusCB();
            mbedtls_pk_free(config->signer.pk_ctx);
            ut->delP(&config->signer.hash);
            delete config->signer.pk_ctx;
            return false;
        }

        //generate RSA signature from private key and message digest
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
            char *tmp = (char *)ut->newP(100);
            mbedtls_strerror(ret, tmp, 100);
            config->signer.tokens.error.message = tmp;
            config->signer.tokens.error.message.insert(0, (const char *)FPSTR("mbedTLS, mbedtls_pk_sign: "));
            ut->delP(&tmp);
            setTokenError(FIREBASE_ERROR_TOKEN_SIGN);
            sendTokenStatusCB();
        }
        else
        {
            ut->clearS(config->signer.encSignature);
            size_t len = ut->base64EncLen(config->signer.signatureSize);
            char *buf = (char *)ut->newP(len);
            ut->encodeBase64Url(buf, config->signer.signature, config->signer.signatureSize);
            config->signer.encSignature = buf;
            ut->delP(&buf);

            config->signer.tokens.jwt += config->signer.encSignature;
            ut->clearS(config->signer.pk);
            ut->clearS(config->signer.encSignature);
        }

        ut->delP(&config->signer.signature);
        ut->delP(&config->signer.hash);
        mbedtls_pk_free(config->signer.pk_ctx);
        mbedtls_entropy_free(config->signer.entropy_ctx);
        mbedtls_ctr_drbg_free(config->signer.ctr_drbg_ctx);
        delete config->signer.pk_ctx;
        delete config->signer.entropy_ctx;
        delete config->signer.ctr_drbg_ctx;

        if (ret != 0)
            return false;
#elif defined(ESP8266)
        //RSA private key
        BearSSL::PrivateKey *pk = nullptr;
        ut->idle();
        //parse priv key
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
            return false;
        }

        const br_rsa_private_key *br_rsa_key = pk->getRSA();

        //generate RSA signature from private key and message digest
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
        //get the signed JWT
        if (ret > 0)
        {
            config->signer.tokens.jwt += config->signer.encSignature;
            ut->clearS(config->signer.pk);
            ut->clearS(config->signer.encSignature);
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

bool Firebase_Signer::getIdToken(bool createUser, const char *email, const char *password)
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
        return false;

    config->signer.signup = false;
    ut->idle();

    if (auth == nullptr)
        return false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->_int.fb_processing)
        return false;

    if (!createUser)
    {
        config->signer.tokens.status = token_status_on_request;
        config->_int.fb_processing = true;
        config->signer.tokens.error.code = 0;
        ut->clearS(config->signer.tokens.error.message);
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        sendTokenStatusCB();
    }

#if defined(ESP32)
    config->signer.wcs = new FB_TCP_Client();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(1024, 1024);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.result = new FirebaseJsonData();

    MBSTRING host;
    if (createUser)
        ut->appendP(host, fb_esp_pgm_str_250);
    else
        ut->appendP(host, fb_esp_pgm_str_193);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)

    ut->ethDNSWorkAround(&ut->config->spi_ethernet_module, host.c_str(), 443);
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_196);
    if (createUser)
    {
        ut->clearS(config->signer.signupError.message);
        if (strlen(email) > 0 && strlen(password) > 0)
            config->signer.json->add(tmp, email);
    }
    else
        config->signer.json->add(tmp, auth->user.email.c_str());
    ut->delP(&tmp);
    tmp = ut->strP(fb_esp_pgm_str_197);
    if (createUser)
    {
        if (strlen(email) > 0 && strlen(password) > 0)
            config->signer.json->add(tmp, password);
    }
    else
        config->signer.json->add(tmp, auth->user.password.c_str());
    ut->delP(&tmp);
    tmp = ut->strP(fb_esp_pgm_str_198);
    config->signer.json->add(tmp, true);
    ut->delP(&tmp);

    MBSTRING req;
    ut->appendP(req, fb_esp_pgm_str_24);
    ut->appendP(req, fb_esp_pgm_str_6);

    if (createUser)
        ut->appendP(req, fb_esp_pgm_str_259);
    else
    {
        ut->appendP(req, fb_esp_pgm_str_194);
        ut->appendP(req, fb_esp_pgm_str_195);
    }

    req += config->api_key;
    ut->appendP(req, fb_esp_pgm_str_30);

    ut->appendP(req, fb_esp_pgm_str_31);
    if (createUser)
        ut->appendP(req, fb_esp_pgm_str_250);
    else
        ut->appendP(req, fb_esp_pgm_str_193);
    ut->appendP(req, fb_esp_pgm_str_4);
    ut->appendP(req, fb_esp_pgm_str_120);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_32);
    ut->appendP(req, fb_esp_pgm_str_12);
    req += NUM2S(strlen(config->signer.json->raw())).get();
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += config->signer.json->raw();

#if defined(ESP32)
    int ret = config->signer.wcs->send(req.c_str());
    ut->clearS(req);
    if (ret < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    ut->clearS(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    config->signer.json->clear();

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.result, tmp);
        ut->delP(&tmp);

        if (config->signer.result->success)
        {
            error.code = config->signer.result->to<int>();
            if (!createUser)
                config->signer.tokens.status = token_status_error;

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                error.message = config->signer.result->to<const char *>();
        }

        if (createUser)
            config->signer.signupError = error;
        else
        {
            config->signer.tokens.error = error;
            tokenInfo.status = config->signer.tokens.status;
            tokenInfo.type = config->signer.tokens.token_type;
            tokenInfo.error = config->signer.tokens.error;
            config->_int.fb_last_jwt_generation_error_cb_millis = 0;
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
                config->signer.anonymous = strlen(email) == 0 && strlen(password) == 0;
            }

            tmp = ut->strP(fb_esp_pgm_str_200);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
            {
                config->_int.auth_token = config->signer.result->to<const char *>();
                config->_int.atok_len = strlen(config->signer.result->to<const char *>());
                config->_int.ltok_len = 0;
            }
#if defined(ESP32)
            tmp = ut->strP(fb_esp_pgm_str_201);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
            {
                ut->storeS(config->_int.refresh_token, config->signer.result->to<const char *>(), false);
                config->_int.rtok_len = strlen(config->signer.result->to<const char *>());
            }
#endif

            tmp = ut->strP(fb_esp_pgm_str_202);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                getExpiration(config->signer.result->to<const char *>());

            tmp = ut->strP(fb_esp_pgm_str_175);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                auth->token.uid = config->signer.result->to<const char *>();

            if (!createUser)
                return handleSignerError(0);
            else
                return handleSignerError(-1);
        }
    }

    return handleSignerError(3, httpCode);
}

bool Firebase_Signer::deleteIdToken(const char *idToken)
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
        return false;

    config->signer.signup = false;
    ut->idle();

    if (auth == nullptr)
        return false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->_int.fb_processing)
        return false;

    config->_int.fb_processing = true;

#if defined(ESP32)
    config->signer.wcs = new FB_TCP_Client();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(1024, 1024);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.result = new FirebaseJsonData();

    MBSTRING host;
    ut->appendP(host, fb_esp_pgm_str_250);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)

    ut->ethDNSWorkAround(&ut->config->spi_ethernet_module, host.c_str(), 443);
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_200);

    if (strlen(idToken) > 0)
        config->signer.json->add(tmp, idToken);
    else
        config->signer.json->add(tmp, config->_int.auth_token);

    ut->delP(&tmp);

    MBSTRING req;
    ut->appendP(req, fb_esp_pgm_str_24);
    ut->appendP(req, fb_esp_pgm_str_6);

    ut->appendP(req, fb_esp_pgm_str_582);

    req += config->api_key;
    ut->appendP(req, fb_esp_pgm_str_30);

    ut->appendP(req, fb_esp_pgm_str_31);
    ut->appendP(req, fb_esp_pgm_str_250);

    ut->appendP(req, fb_esp_pgm_str_4);
    ut->appendP(req, fb_esp_pgm_str_120);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_32);
    ut->appendP(req, fb_esp_pgm_str_12);
    req += NUM2S(strlen(config->signer.json->raw())).get();
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += config->signer.json->raw();

#if defined(ESP32)
    int ret = config->signer.wcs->send(req.c_str());
    ut->clearS(req);
    if (ret < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    ut->clearS(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    config->signer.json->clear();

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        //config->signer.json->toString(Serial, true);

        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.result, tmp);
        ut->delP(&tmp);

        if (config->signer.result->success)
        {
            error.code = config->signer.result->to<int>();

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                error.message = config->signer.result->to<const char *>();
        }

        config->signer.deleteError = error;

        if (error.code == 0)
        {
            if (strlen(idToken) == 0 || strcmp(config->_int.auth_token.c_str(), idToken) == 0)
            {
                config->_int.auth_token.clear();
                config->_int.atok_len = 0;
                config->_int.ltok_len = 0;
                config->signer.tokens.expires = 0;
                config->signer.attempts = 0;
                config->signer.step = fb_esp_jwt_generation_step_begin;
                config->_int.fb_last_jwt_generation_error_cb_millis = 0;
                config->signer.tokens.token_type = token_type_undefined;
                config->signer.anonymous = false;
                config->signer.idTokenCutomSet = false;
            }

            return true;
        }
    }

    return handleSignerError(4, httpCode);
}

bool Firebase_Signer::requestTokens()
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
        return false;

    ut->idle();

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || time(nullptr) < ut->default_ts || config->_int.fb_processing)
        return false;

    config->signer.tokens.status = token_status_on_request;
    config->_int.fb_processing = true;
    config->signer.tokens.error.code = 0;
    ut->clearS(config->signer.tokens.error.message);
    config->_int.fb_last_jwt_generation_error_cb_millis = 0;
    sendTokenStatusCB();

#if defined(ESP32)
    config->signer.wcs = new FB_TCP_Client();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(1024, 1024);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.result = new FirebaseJsonData();

    MBSTRING host;
    ut->appendP(host, fb_esp_pgm_str_193);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

    ut->idle();
#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)

    ut->ethDNSWorkAround(&ut->config->spi_ethernet_module, host.c_str(), 443);
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    MBSTRING req;
    ut->appendP(req, fb_esp_pgm_str_24);
    ut->appendP(req, fb_esp_pgm_str_6);

    if (config->signer.tokens.token_type == token_type_custom_token)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_233);
        config->signer.json->add(tmp, config->signer.tokens.jwt.c_str());
        ut->delP(&tmp);
        tmp = ut->strP(fb_esp_pgm_str_198);
        config->signer.json->add(tmp, true);
        ut->delP(&tmp);

        ut->appendP(req, fb_esp_pgm_str_194);
        ut->appendP(req, fb_esp_pgm_str_232);
        req += config->api_key;
        ut->appendP(req, fb_esp_pgm_str_30);
        ut->appendP(req, fb_esp_pgm_str_31);
        ut->appendP(req, fb_esp_pgm_str_193);
    }
    else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_227);
        char *tmp2 = ut->strP(fb_esp_pgm_str_228);
        config->signer.json->add(tmp, (const char *)tmp2);
        ut->delP(&tmp);
        ut->delP(&tmp2);
        tmp = ut->strP(fb_esp_pgm_str_229);
        config->signer.json->add(tmp, config->signer.tokens.jwt.c_str());
        ut->delP(&tmp);

        ut->appendP(req, fb_esp_pgm_str_1);
        ut->appendP(req, fb_esp_pgm_str_233);
        ut->appendP(req, fb_esp_pgm_str_30);
        ut->appendP(req, fb_esp_pgm_str_31);
        ut->appendP(req, fb_esp_pgm_str_251);
    }

    ut->appendP(req, fb_esp_pgm_str_4);
    ut->appendP(req, fb_esp_pgm_str_120);

    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_32);
    ut->appendP(req, fb_esp_pgm_str_12);
    req += NUM2S(strlen(config->signer.json->raw())).get();
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += config->signer.json->raw();
#if defined(ESP32)
    config->signer.wcs->setInsecure();
    int ret = config->signer.wcs->send(req.c_str());
    ut->clearS(req);
    if (ret < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    ut->clearS(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    struct fb_esp_auth_token_error_t error;

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        ut->clearS(config->signer.tokens.jwt);
        char *tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.result, tmp);
        ut->delP(&tmp);
        if (config->signer.result->success)
        {
            error.code = config->signer.result->to<int>();
            config->signer.tokens.status = token_status_error;

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                error.message = config->signer.result->to<const char *>();
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {
            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                tmp = ut->strP(fb_esp_pgm_str_200);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                {
                    ut->storeS(config->_int.auth_token, config->signer.result->to<const char *>(), false);
                    config->_int.atok_len = strlen(config->signer.result->to<const char *>());
                    config->_int.ltok_len = 0;
                }

                tmp = ut->strP(fb_esp_pgm_str_201);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                {
                    ut->storeS(config->_int.refresh_token, config->signer.result->to<const char *>(), false);
                    config->_int.rtok_len = strlen(config->signer.result->to<const char *>());
                }

                tmp = ut->strP(fb_esp_pgm_str_202);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                    getExpiration(config->signer.result->to<const char *>());
            }
            else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
            {
                tmp = ut->strP(fb_esp_pgm_str_235);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                {
                    ut->storeS(config->_int.auth_token, config->signer.result->to<const char *>(), false);
                    config->_int.atok_len = strlen(config->signer.result->to<const char *>());
                    config->_int.ltok_len = 0;
                }

                tmp = ut->strP(fb_esp_pgm_str_236);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                    config->signer.tokens.auth_type = config->signer.result->to<const char *>();

                tmp = ut->strP(fb_esp_pgm_str_210);
                config->signer.json->get(*config->signer.result, tmp);
                ut->delP(&tmp);
                if (config->signer.result->success)
                    getExpiration(config->signer.result->to<const char *>());
            }
            return handleSignerError(0);
        }
        return handleSignerError(4);
    }

    return handleSignerError(3, httpCode);
}

void Firebase_Signer::getExpiration(const char *exp)
{
    time_t ts = time(nullptr);
    unsigned long ms = millis();
    config->signer.tokens.expires = ts + atoi(exp);
    config->signer.tokens.last_millis = ms;
}

bool Firebase_Signer::handleEmailSending(const char *payload, fb_esp_user_email_sending_type type)
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp(&config->spi_ethernet_module))
        return false;

    ut->idle();

    if (config->_int.fb_processing)
        return false;

    config->_int.fb_processing = true;

#if defined(ESP32)
    config->signer.wcs = new FB_TCP_Client();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(1024, 1024);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.result = new FirebaseJsonData();

    MBSTRING host;
    ut->appendP(host, fb_esp_pgm_str_193);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)

    ut->ethDNSWorkAround(&ut->config->spi_ethernet_module, host.c_str(), 443);
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_260);
    char *tmp2 = nullptr;
    if (type == fb_esp_user_email_sending_type_verify)
    {
        ut->clearS(config->signer.verificationError.message);
        tmp2 = ut->strP(fb_esp_pgm_str_261);
        config->signer.json->add(tmp, (const char *)tmp2);
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        ut->clearS(config->signer.resetPswError.message);
        tmp2 = ut->strP(fb_esp_pgm_str_263);
        config->signer.json->add(tmp, (const char *)tmp2);
    }
    ut->delP(&tmp);

    if (type == fb_esp_user_email_sending_type_verify)
    {
        tmp = ut->strP(fb_esp_pgm_str_200);
        if (strlen(payload) > 0)
            config->signer.json->add(tmp, payload);
        else
            config->signer.json->add(tmp, config->_int.auth_token.c_str());

        ut->delP(&tmp);
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        tmp = ut->strP(fb_esp_pgm_str_196);
        config->signer.json->add(tmp, payload);
        ut->delP(&tmp);
    }

    MBSTRING s;
    config->signer.json->toString(s);

    MBSTRING req;
    ut->appendP(req, fb_esp_pgm_str_24);
    ut->appendP(req, fb_esp_pgm_str_6);

    ut->appendP(req, fb_esp_pgm_str_194);
    ut->appendP(req, fb_esp_pgm_str_262);

    req += config->api_key;
    ut->appendP(req, fb_esp_pgm_str_30);

    ut->appendP(req, fb_esp_pgm_str_31);

    ut->appendP(req, fb_esp_pgm_str_193);
    ut->appendP(req, fb_esp_pgm_str_4);
    ut->appendP(req, fb_esp_pgm_str_120);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_32);
    ut->appendP(req, fb_esp_pgm_str_12);
    req += NUM2S(s.length()).get();
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += s.c_str();

#if defined(ESP32)
    int ret = config->signer.wcs->send(req.c_str());
    ut->clearS(req);
    if (ret < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    ut->clearS(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    config->signer.json->clear();

    int httpCode = 0;
    if (handleTokenResponse(httpCode))
    {
        struct fb_esp_auth_token_error_t error;

        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.result, tmp);
        ut->delP(&tmp);

        if (config->signer.result->success)
        {
            error.code = config->signer.result->to<int>();
            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.result, tmp);
            ut->delP(&tmp);
            if (config->signer.result->success)
                error.message = config->signer.result->to<const char *>();
        }
        if (type == fb_esp_user_email_sending_type_verify)
            config->signer.verificationError = error;
        else if (type == fb_esp_user_email_sending_type_reset_psw)
            config->signer.resetPswError = error;

        if (error.code == 0)
        {
            config->signer.json->clear();
            return handleSignerError(0);
        }

        return handleSignerError(4);
    }

    return handleSignerError(3, httpCode);
}

void Firebase_Signer::checkToken()
{
    if (!config || !auth)
        return;

    //if the time was set (changed) after token has been generated, update its expiration
    if (config->signer.tokens.expires > 0 && config->signer.tokens.expires < ESP_DEFAULT_TS && time(nullptr) > ESP_DEFAULT_TS)
        config->signer.tokens.expires += time(nullptr) - (millis() - config->signer.tokens.last_millis) / 1000 - 60;

    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;

    if (isAuthToken(true) && ((unsigned long)time(nullptr) > config->signer.tokens.expires - config->signer.preRefreshSeconds || config->signer.tokens.expires == 0))
        handleToken();
}

bool Firebase_Signer::tokenReady()
{
    if (!config)
        return false;

    checkToken();
    return config->signer.tokens.status == token_status_ready;
};

void Firebase_Signer::errorToString(int httpCode, MBSTRING &buff)
{
    ut->clearS(buff);

    if (config)
    {
        if (config->signer.tokens.status == token_status_error || config->signer.tokens.error.code != 0)
        {
            buff = config->signer.tokens.error.message;
            return;
        }
    }

    switch (httpCode)
    {
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_REFUSED:
        ut->appendP(buff, fb_esp_pgm_str_39);
        return;
    case FIREBASE_ERROR_TCP_ERROR_SEND_HEADER_FAILED:
        ut->appendP(buff, fb_esp_pgm_str_40);
        return;
    case FIREBASE_ERROR_TCP_ERROR_SEND_PAYLOAD_FAILED:
        ut->appendP(buff, fb_esp_pgm_str_41);
        return;
    case FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED:
        ut->appendP(buff, fb_esp_pgm_str_42);
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_LOST:
        ut->appendP(buff, fb_esp_pgm_str_43);
        return;
    case FIREBASE_ERROR_TCP_ERROR_NO_HTTP_SERVER:
        ut->appendP(buff, fb_esp_pgm_str_44);
        return;
    case FIREBASE_ERROR_HTTP_CODE_BAD_REQUEST:
        ut->appendP(buff, fb_esp_pgm_str_45);
        return;
    case FIREBASE_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        ut->appendP(buff, fb_esp_pgm_str_46);
        return;
    case FIREBASE_ERROR_HTTP_CODE_NO_CONTENT:
        ut->appendP(buff, fb_esp_pgm_str_47);
        return;
    case FIREBASE_ERROR_HTTP_CODE_MOVED_PERMANENTLY:
        ut->appendP(buff, fb_esp_pgm_str_48);
        return;
    case FIREBASE_ERROR_HTTP_CODE_USE_PROXY:
        ut->appendP(buff, fb_esp_pgm_str_49);
        return;
    case FIREBASE_ERROR_HTTP_CODE_TEMPORARY_REDIRECT:
        ut->appendP(buff, fb_esp_pgm_str_50);
        return;
    case FIREBASE_ERROR_HTTP_CODE_PERMANENT_REDIRECT:
        ut->appendP(buff, fb_esp_pgm_str_51);
        return;
    case FIREBASE_ERROR_HTTP_CODE_UNAUTHORIZED:
        ut->appendP(buff, fb_esp_pgm_str_52);
        return;
    case FIREBASE_ERROR_HTTP_CODE_FORBIDDEN:
        ut->appendP(buff, fb_esp_pgm_str_53);
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_FOUND:
        ut->appendP(buff, fb_esp_pgm_str_54);
        return;
    case FIREBASE_ERROR_HTTP_CODE_METHOD_NOT_ALLOWED:
        ut->appendP(buff, fb_esp_pgm_str_55);
        return;
    case FIREBASE_ERROR_HTTP_CODE_NOT_ACCEPTABLE:
        ut->appendP(buff, fb_esp_pgm_str_56);
        return;
    case FIREBASE_ERROR_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        ut->appendP(buff, fb_esp_pgm_str_57);
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT:
        ut->appendP(buff, fb_esp_pgm_str_58);
        return;
    case FIREBASE_ERROR_HTTP_CODE_LENGTH_REQUIRED:
        ut->appendP(buff, fb_esp_pgm_str_59);
        return;
    case FIREBASE_ERROR_HTTP_CODE_TOO_MANY_REQUESTS:
        ut->appendP(buff, fb_esp_pgm_str_60);
        return;
    case FIREBASE_ERROR_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        ut->appendP(buff, fb_esp_pgm_str_61);
        return;
    case FIREBASE_ERROR_HTTP_CODE_INTERNAL_SERVER_ERROR:
        ut->appendP(buff, fb_esp_pgm_str_62);
        return;
    case FIREBASE_ERROR_HTTP_CODE_BAD_GATEWAY:
        ut->appendP(buff, fb_esp_pgm_str_63);
        return;
    case FIREBASE_ERROR_HTTP_CODE_SERVICE_UNAVAILABLE:
        ut->appendP(buff, fb_esp_pgm_str_64);
        return;
    case FIREBASE_ERROR_HTTP_CODE_GATEWAY_TIMEOUT:
        ut->appendP(buff, fb_esp_pgm_str_65);
        return;
    case FIREBASE_ERROR_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        ut->appendP(buff, fb_esp_pgm_str_66);
        return;
    case FIREBASE_ERROR_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        ut->appendP(buff, fb_esp_pgm_str_67);
        return;
    case FIREBASE_ERROR_HTTP_CODE_PRECONDITION_FAILED:
        ut->appendP(buff, fb_esp_pgm_str_152);
        return;
    case FIREBASE_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT:
        ut->appendP(buff, fb_esp_pgm_str_69);
        return;
    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        ut->appendP(buff, fb_esp_pgm_str_70);
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        ut->appendP(buff, fb_esp_pgm_str_71);
        return;
    case FIREBASE_ERROR_TCP_ERROR_CONNECTION_INUSED:
        ut->appendP(buff, fb_esp_pgm_str_94);
        return;
    case FIREBASE_ERROR_TCP_MAX_REDIRECT_REACHED:
        ut->appendP(buff, fb_esp_pgm_str_169);
        return;
    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        ut->appendP(buff, fb_esp_pgm_str_68);
        return;
    case FIREBASE_ERROR_NO_FCM_ID_TOKEN_PROVIDED:
        ut->appendP(buff, fb_esp_pgm_str_145);
        return;
    case FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED:
        ut->appendP(buff, fb_esp_pgm_str_146);
        return;
    case FIREBASE_ERROR_NO_FCM_TOPIC_PROVIDED:
        ut->appendP(buff, fb_esp_pgm_str_542);
        return;
    case FIREBASE_ERROR_FCM_ID_TOKEN_AT_INDEX_NOT_FOUND:
        ut->appendP(buff, fb_esp_pgm_str_543);
        return;
    case FIREBASE_ERROR_EXPECTED_JSON_DATA:
        ut->appendP(buff, fb_esp_pgm_str_185);
        return;
    case FIREBASE_ERROR_HTTP_CODE_PAYLOAD_TOO_LARGE:
        ut->appendP(buff, fb_esp_pgm_str_189);
        return;
    case FIREBASE_ERROR_CANNOT_CONFIG_TIME:
        ut->appendP(buff, fb_esp_pgm_str_190);
        return;
    case FIREBASE_ERROR_SSL_RX_BUFFER_SIZE_TOO_SMALL:
        ut->appendP(buff, fb_esp_pgm_str_191);
        return;
    case FIREBASE_ERROR_FILE_IO_ERROR:
        ut->appendP(buff, fb_esp_pgm_str_192);
        return;
#if defined(FIREBASE_ESP_CLIENT)
    case FIREBASE_ERROR_FILE_NOT_FOUND:
        ut->appendP(buff, fb_esp_pgm_str_449);
        return;
    case FIREBASE_ERROR_ARCHIVE_NOT_FOUND:
        ut->appendP(buff, fb_esp_pgm_str_450);
        return;
    case FIREBASE_ERROR_LONG_RUNNING_TASK:
        ut->appendP(buff, fb_esp_pgm_str_534);
        return;
    case FIREBASE_ERROR_UPLOAD_TIME_OUT:
        ut->appendP(buff, fb_esp_pgm_str_540);
        return;
    case FIREBASE_ERROR_UPLOAD_DATA_ERRROR:
        ut->appendP(buff, fb_esp_pgm_str_541);
        return;
    case FIREBASE_ERROR_OAUTH2_REQUIRED:
        ut->appendP(buff, fb_esp_pgm_str_328);
        return;
#endif
    case FIREBASE_ERROR_TOKEN_NOT_READY:
        ut->appendP(buff, fb_esp_pgm_str_252);
        return;
    case FIREBASE_ERROR_UNINITIALIZED:
        ut->appendP(buff, fb_esp_pgm_str_256);
        return;
    case FIREBASE_ERROR_MISSING_DATA:
        ut->appendP(buff, fb_esp_pgm_str_579);
        return;
    case FIREBASE_ERROR_MISSING_CREDENTIALS:
        ut->appendP(buff, fb_esp_pgm_str_580);
        return;
    case FIREBASE_ERROR_INVALID_JSON_RULES:
        ut->appendP(buff, fb_esp_pgm_str_581);
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
    if (!config)
        return "";
    return config->_int.auth_token.c_str();
}

FirebaseConfig *Firebase_Signer::getCfg()
{
    return config;
}

FirebaseAuth *Firebase_Signer::getAuth()
{
    return auth;
}

MBSTRING Firebase_Signer::getCAFile()
{
    if (!config)
        return "";
    return config->cert.file;
}
int Firebase_Signer::getCAFileStorage()
{
    if (!config)
        return 0;
    return config->cert.file_storage;
}

Firebase_Signer Signer = Firebase_Signer();

#endif