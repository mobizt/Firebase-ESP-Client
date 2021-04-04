/**
 * Google's Firebase Token Generation class, Signer.cpp version 1.0.8
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created April 4, 2021
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
            config->signer.data = new FirebaseJsonData();
            char *tmp = nullptr;

            size_t len = config->_int.fb_file.size();
            char *buf = ut->newS(len + 10);
            if (config->_int.fb_file.available())
            {
                config->_int.fb_file.readBytes(buf, len);
                config->signer.json->setJsonData(buf);
            }
            config->_int.fb_file.close();
            ut->delS(buf);

            tmp = ut->strP(fb_esp_pgm_str_243);
            config->signer.json->get(*config->signer.data, (const char *)tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
            {
                if (ut->strposP(config->signer.data->stringValue.c_str(), fb_esp_pgm_str_244, 0) > -1)
                {
                    tmp = ut->strP(fb_esp_pgm_str_245);
                    config->signer.json->get(*config->signer.data, (const char *)tmp);
                    ut->delS(tmp);
                    if (config->signer.data->success)
                        config->service_account.data.project_id = config->signer.data->stringValue.c_str();
                    tmp = ut->strP(fb_esp_pgm_str_246);
                    config->signer.json->get(*config->signer.data, (const char *)tmp);
                    ut->delS(tmp);
                    if (config->signer.data->success)
                        config->service_account.data.private_key_id = config->signer.data->stringValue.c_str();
                    tmp = ut->strP(fb_esp_pgm_str_247);
                    config->signer.json->get(*config->signer.data, (const char *)tmp);
                    ut->delS(tmp);
                    if (config->signer.data->success)
                    {
                        tmp = ut->newS(config->signer.data->stringValue.length());
                        size_t c = 0;
                        for (size_t i = 0; i < config->signer.data->stringValue.length(); i++)
                        {
                            if (config->signer.data->stringValue[i] == '\\')
                            {
                                delay(0);
                                tmp[c++] = '\n';
                                i++;
                            }
                            else
                                tmp[c++] = config->signer.data->stringValue[i];
                        }
                        config->signer.pk = tmp;
                        config->signer.data->stringValue.clear();
                        ut->delS(tmp);
                    }

                    tmp = ut->strP(fb_esp_pgm_str_248);
                    config->signer.json->get(*config->signer.data, (const char *)tmp);
                    ut->delS(tmp);
                    if (config->signer.data->success)
                        config->service_account.data.client_email = config->signer.data->stringValue.c_str();
                    tmp = ut->strP(fb_esp_pgm_str_253);
                    config->signer.json->get(*config->signer.data, (const char *)tmp);
                    ut->delS(tmp);
                    if (config->signer.data->success)
                        config->service_account.data.client_id = config->signer.data->stringValue.c_str();

                    delete config->signer.json;
                    delete config->signer.data;
                    return true;
                }
            }

            delete config->signer.json;
            delete config->signer.data;
        }
    }

    return false;
}

void Firebase_Signer::clearSA()
{
    config->service_account.data.private_key = "";
    config->service_account.data.project_id.clear();
    config->service_account.data.private_key_id.clear();
    config->service_account.data.client_email.clear();
    std::string().swap(config->signer.pk);
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

bool Firebase_Signer::handleToken()
{
    if (!config || !auth)
        return false;

    if (config->host.length() == 0)
        return false;

    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;

    if ((config->signer.tokens.token_type == token_type_id_token || config->signer.tokens.token_type == token_type_custom_token || config->signer.tokens.token_type == token_type_oauth2_access_token) && ((unsigned long)time(nullptr) > config->signer.tokens.expires - config->signer.preRefreshSeconds || config->signer.tokens.expires == 0))
    {
        if (config->signer.tokens.expires > 0 && (config->signer.tokens.token_type == token_type_id_token || config->signer.tokens.token_type == token_type_custom_token))
        {
            if (millis() - config->signer.lastReqMillis > config->signer.reqTO || config->signer.lastReqMillis == 0)
            {

                config->signer.lastReqMillis = millis();
                if (config->_int.fb_processing)
                    return false;
#if defined(ESP32)
                return refreshToken();
#elif defined(ESP8266)
                //Due to the response payload of token refreshment request is too big for ESP8266,
                //get the new id token instead.
                return getIdToken(false, "", "");
#endif
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
                        config->signer.tokens.error.message.clear();
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

    static Firebase_Signer *_this = this;

    TaskFunction_t taskCode = [](void *param) {
        _this->config->signer.tokenTaskRunning = true;

        while (_this->_token_processing_task_enable)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);

            if (_this->config->signer.tokens.token_type == token_type_id_token)
            {
                _this->config->signer.lastReqMillis = millis();
                if (_this->getIdToken(false, "", ""))
                {
                    _this->_token_processing_task_enable = false;
                    _this->config->signer.attempts = 0;
                    break;
                }
                else
                {
                    if (_this->config->signer.attempts < _this->config->max_token_generation_retry)
                        _this->config->signer.attempts++;
                    else
                    {
                        _this->config->signer.tokens.error.message.clear();
                        _this->setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
                        _this->sendTokenStatusCB();
                        _this->_token_processing_task_enable = false;
                        _this->config->signer.attempts = 0;
                        break;
                    }
                }
            }
            else
            {
                if (_this->config->signer.step == fb_esp_jwt_generation_step_begin)
                {
                    _this->ut->setClock(_this->config->time_zone);
                    time_t now = time(nullptr);
                    _this->config->_int.fb_clock_rdy = now > _this->ut->default_ts;

                    if (_this->config->_int.fb_clock_rdy)
                        _this->config->signer.step = fb_esp_jwt_generation_step_encode_header_payload;
                }
                else if (_this->config->signer.step == fb_esp_jwt_generation_step_encode_header_payload)
                {
                    if (_this->createJWT())
                        _this->config->signer.step = fb_esp_jwt_generation_step_sign;
                }
                else if (_this->config->signer.step == fb_esp_jwt_generation_step_sign)
                {
                    if (_this->createJWT())
                        _this->config->signer.step = fb_esp_jwt_generation_step_exchange;
                }
                else if (_this->config->signer.step == fb_esp_jwt_generation_step_exchange)
                {
                    if (_this->requestTokens())
                    {
                        _this->config->signer.attempts = 0;
                        _this->_token_processing_task_enable = false;
                        _this->config->signer.step = fb_esp_jwt_generation_step_begin;
                        break;
                    }
                    else
                    {
                        if (_this->config->signer.attempts < _this->config->max_token_generation_retry)
                            _this->config->signer.attempts++;
                        else
                        {
                            _this->config->signer.tokens.error.message.clear();
                            _this->setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
                            _this->sendTokenStatusCB();
                            _this->config->signer.attempts = 0;
                            _this->config->signer.step = fb_esp_jwt_generation_step_begin;
                            break;
                        }
                    }
                }
            }

            yield();
        }
        _this->config->_int.token_processing_task_handle = NULL;
        _this->config->signer.tokenTaskRunning = false;
        vTaskDelete(NULL);
    };

    char *taskname = ut->strP(fb_esp_pgm_str_546);
    xTaskCreatePinnedToCore(taskCode, taskname, 12000, NULL, 3, &config->_int.token_processing_task_handle, 1);
    ut->delS(taskname);

#elif defined(ESP8266)

    if (_token_processing_task_enable)
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
                    config->signer.tokens.error.message.clear();
                    setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
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
            if (config->signer.step == fb_esp_jwt_generation_step_begin)
            {
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
                        config->signer.tokens.error.message.clear();
                        setTokenError(FIREBASE_ERROR_TOKEN_EXCHANGE_MAX_RETRY_REACHED);
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

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
        return false;

    delay(0);

    if (auth == nullptr)
        return false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->_int.fb_processing)
        return false;

    if (config->signer.tokens.legacy_token.length() > 0 || (config->signer.tokens.refresh_token.length() == 0 && config->signer.tokens.id_token.length() == 0 && config->signer.tokens.access_token.length() == 0))
        return false;

    config->signer.tokens.status = token_status_on_refresh;
    config->_int.fb_processing = true;
    config->signer.tokens.error.code = 0;
    config->signer.tokens.error.message.clear();
    sendTokenStatusCB();

    config->signer.tokens.id_token.clear();
    config->signer.tokens.access_token.clear();

#if defined(ESP32)
    config->signer.wcs = new FB_HTTPClient32();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(1024, 512);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.data = new FirebaseJsonData();

    std::string host;
    ut->appendP(host, fb_esp_pgm_str_203);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);
#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_205);
    char *tmp2 = ut->strP(fb_esp_pgm_str_206);
    config->signer.json->add(tmp, (const char *)tmp2);
    ut->delS(tmp);
    ut->delS(tmp2);
    tmp = ut->strP(fb_esp_pgm_str_207);
    config->signer.json->add(tmp, config->signer.tokens.refresh_token.c_str());
    ut->delS(tmp);

    String s;
    config->signer.json->toString(s);

    std::string req;
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
    tmp = ut->intStr(s.length());
    req += tmp;
    ut->delS(tmp);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += s.c_str();
#if defined(ESP32)
    if (config->signer.wcs->send(req.c_str(), "") < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    std::string().swap(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    struct fb_esp_auth_token_error_t error;

    if (handleTokenResponse())
    {
        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.data, tmp);
        ut->delS(tmp);

        if (config->signer.data->success)
        {
            error.code = config->signer.data->intValue;
            config->signer.tokens.status = token_status_error;

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                error.message = config->signer.data->stringValue.c_str();
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {
            if (config->signer.tokens.token_type == token_type_id_token || config->signer.tokens.token_type == token_type_custom_token)
            {
                tmp = ut->strP(fb_esp_pgm_str_208);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.id_token = config->signer.data->stringValue.c_str();

                tmp = ut->strP(fb_esp_pgm_str_209);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.refresh_token = config->signer.data->stringValue.c_str();

                tmp = ut->strP(fb_esp_pgm_str_210);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.expires = time(nullptr) + atoi(config->signer.data->stringValue.c_str());

                tmp = ut->strP(fb_esp_pgm_str_175);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    auth->token.uid = config->signer.data->stringValue.c_str();
            }
            return handleSignerError(0);
        }

        return handleSignerError(4);
    }

    return handleSignerError(3);
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
        case FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED:
            ut->appendP(config->signer.tokens.error.message, fb_esp_pgm_str_42);
            break;
        case FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST:
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

bool Firebase_Signer::handleSignerError(int code)
{

    switch (code)
    {

    case 1:
        config->signer.tokens.error.message.clear();
        setTokenError(FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED);
        sendTokenStatusCB();
        break;
    case 2:
#if defined(ESP32)
        if (config->signer.wcs->stream())
            config->signer.wcs->stream()->stop();
#elif defined(ESP8266)
        config->signer.wcs->stop();
#endif
        config->signer.tokens.error.message.clear();
        setTokenError(FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST);
        sendTokenStatusCB();
        break;
    case 3:
#if defined(ESP32)
        if (config->signer.wcs->stream())
            config->signer.wcs->stream()->stop();
#elif defined(ESP8266)
        config->signer.wcs->stop();
#endif
        config->signer.tokens.error.message.clear();
        setTokenError(FIREBASE_ERROR_HTTP_CODE_REQUEST_TIMEOUT);
        sendTokenStatusCB();
        break;

    default:
        break;
    }

    if (config->signer.wcs)
        delete config->signer.wcs;
    if (config->signer.json)
        delete config->signer.json;
    if (config->signer.data)
        delete config->signer.data;

    config->_int.fb_processing = false;

    if (code > 0 && code < 4)
    {
        config->signer.tokens.status = token_status_error;
        config->signer.tokens.error.code = code;
        return false;
    }
    else if (code == 0)
    {
        config->signer.tokens.error.message.clear();
        config->signer.tokens.status = token_status_ready;
        config->signer.attempts = 0;
        config->signer.step = fb_esp_jwt_generation_step_begin;
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
        config->token_status_callback(tokenInfo);
}

bool Firebase_Signer::handleTokenResponse()
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
        return false;

    struct server_response_data_t response;

    unsigned long dataTime = millis();

    int chunkIdx = 0;
    int chunkBufSize = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    std::string header, payload;
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

        delay(0);
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
                delay(0);

                if (config->_int.fb_reconnect_wifi)
                    ut->reconnect(0);

                if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
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
                            ut->delS(tmp);
                        }
                    }
                    else
                    {
                        if (isHeader)
                        {
                            char *tmp = ut->newS(chunkBufSize);
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
                                std::string().swap(header);
                            }
                            else
                                header += tmp;

                            ut->delS(tmp);
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

                                    char *tmp = ut->newS(chunkBufSize + 1);
                                    int readLen = stream->readBytes(tmp, chunkBufSize);

                                    if (readLen > 0)
                                        payload += tmp;

                                    ut->delS(tmp);
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

    if (payload.length() > 0 && !response.noContent)
    {
        config->signer.json->setJsonData(payload.c_str());
        std::string().swap(payload);
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
        sendTokenStatusCB();

        config->signer.json = new FirebaseJson();
        config->signer.data = new FirebaseJsonData();

        unsigned long now = time(nullptr);

        config->signer.tokens.jwt.clear();

        //header
        char *tmp = ut->strP(fb_esp_pgm_str_239);
        char *tmp2 = ut->strP(fb_esp_pgm_str_242);
        config->signer.json->add(tmp, (const char *)tmp2);
        ut->delS(tmp);
        ut->delS(tmp2);
        tmp2 = ut->strP(fb_esp_pgm_str_234);
        tmp = ut->strP(fb_esp_pgm_str_240);
        config->signer.json->add(tmp, (const char *)tmp2);
        ut->delS(tmp);
        ut->delS(tmp2);

        config->signer.json->int_tostr(config->signer.header);
        size_t len = ut->base64EncLen(config->signer.header.length());
        char *buf = ut->newS(len);
        ut->encodeBase64Url(buf, (unsigned char *)config->signer.header.c_str(), config->signer.header.length());
        config->signer.encHeader = buf;
        ut->delS(buf);
        config->signer.header.clear();
        config->signer.encHeadPayload = config->signer.encHeader;

        //payload
        config->signer.json->clear();
        tmp = ut->strP(fb_esp_pgm_str_212);
        config->signer.json->add(tmp, config->service_account.data.client_email.c_str());
        ut->delS(tmp);
        tmp = ut->strP(fb_esp_pgm_str_213);
        config->signer.json->add(tmp, config->service_account.data.client_email.c_str());
        ut->delS(tmp);
        tmp = ut->strP(fb_esp_pgm_str_214);
        std::string t;
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
        ut->delS(tmp);

        tmp = ut->strP(fb_esp_pgm_str_218);
        config->signer.json->add(tmp, (int)now);
        ut->delS(tmp);

        tmp = ut->strP(fb_esp_pgm_str_215);

        if (config->signer.expiredSeconds > 3600)
            config->signer.json->add(tmp, (int)(now + 3600));
        else
            config->signer.json->add(tmp, (int)(now + config->signer.expiredSeconds));

        ut->delS(tmp);

        if (config->signer.tokens.token_type == token_type_oauth2_access_token)
        {
            std::string buri;
            ut->appendP(buri, fb_esp_pgm_str_112);
            ut->appendP(buri, fb_esp_pgm_str_193);
            ut->appendP(buri, fb_esp_pgm_str_4);
            ut->appendP(buri, fb_esp_pgm_str_120);
            ut->appendP(buri, fb_esp_pgm_str_1);
            ut->appendP(buri, fb_esp_pgm_str_219);
            ut->appendP(buri, fb_esp_pgm_str_1);

            std::string s = buri;
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

            ut->appendP(s, fb_esp_pgm_str_6);
            s += buri;
            ut->appendP(s, fb_esp_pgm_str_451);

            if (config->signer.tokens.scope.length() > 0)
            {
                std::vector<std::string> scopes = std::vector<std::string>();
                ut->splitTk(config->signer.tokens.scope, scopes, ",");
                for (size_t i = 0; i < scopes.size(); i++)
                {
                    ut->appendP(s, fb_esp_pgm_str_6);
                    s += scopes[i];
                    scopes[i].clear();
                    std::string().swap(scopes[i]);
                }
                scopes.clear();
            }

            tmp = ut->strP(fb_esp_pgm_str_220);
            config->signer.json->add(tmp, s.c_str());
            ut->delS(tmp);
        }
        else if (config->signer.tokens.token_type == token_type_custom_token)
        {
            tmp = ut->strP(fb_esp_pgm_str_254);
            config->signer.json->add(tmp, auth->token.uid.c_str());
            ut->delS(tmp);

            std::string s;
            auth->token.claims.int_tostr(s);
            if (s.length() > 2)
            {
                tmp = ut->strP(fb_esp_pgm_str_255);
                config->signer.json->add(tmp, auth->token.claims);
                ut->delS(tmp);
            }
        }

        config->signer.json->int_tostr(config->signer.payload);
        len = ut->base64EncLen(config->signer.payload.length());
        buf = ut->newS(len);
        ut->encodeBase64Url(buf, (unsigned char *)config->signer.payload.c_str(), config->signer.payload.length());
        config->signer.encPayload = buf;
        ut->delS(buf);
        config->signer.payload.clear();

        ut->appendP(config->signer.encHeadPayload, fb_esp_pgm_str_4);
        config->signer.encHeadPayload += config->signer.encPayload;

        std::string().swap(config->signer.encHeader);
        std::string().swap(config->signer.encPayload);

//create message digest from encoded header and payload
#if defined(ESP32)
        config->signer.hash = new uint8_t[config->signer.hashSize];
        int ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (const unsigned char *)config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length(), config->signer.hash);
        if (ret != 0)
        {
            char *tmp = ut->newS(100);
            mbedtls_strerror(ret, tmp, 100);
            config->signer.tokens.error.message = tmp;
            ut->delS(tmp);
            setTokenError(FIREBASE_ERROR_TOKEN_CREATE_HASH);
            sendTokenStatusCB();
            delete[] config->signer.hash;
            return false;
        }
#elif defined(ESP8266)
        config->signer.hash = ut->newS(config->signer.hashSize);
        br_sha256_context mc;
        br_sha256_init(&mc);
        br_sha256_update(&mc, config->signer.encHeadPayload.c_str(), config->signer.encHeadPayload.length());
        br_sha256_out(&mc, config->signer.hash);
#endif

        config->signer.tokens.jwt = config->signer.encHeadPayload;
        ut->appendP(config->signer.tokens.jwt, fb_esp_pgm_str_4);
        std::string().swap(config->signer.encHeadPayload);

        delete config->signer.json;
        delete config->signer.data;
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
            char *tmp = ut->newS(100);
            mbedtls_strerror(ret, tmp, 100);
            config->signer.tokens.error.message = tmp;
            ut->delS(tmp);
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            sendTokenStatusCB();
            mbedtls_pk_free(config->signer.pk_ctx);
            delete[] config->signer.hash;
            delete config->signer.pk_ctx;
            return false;
        }

        //generate RSA signature from private key and message digest
        config->signer.signature = new unsigned char[config->signer.signatureSize];
        size_t retSize = 0;
        config->signer.entropy_ctx = new mbedtls_entropy_context();
        config->signer.ctr_drbg_ctx = new mbedtls_ctr_drbg_context();
        mbedtls_entropy_init(config->signer.entropy_ctx);
        mbedtls_ctr_drbg_init(config->signer.ctr_drbg_ctx);
        mbedtls_ctr_drbg_seed(config->signer.ctr_drbg_ctx, mbedtls_entropy_func, config->signer.entropy_ctx, NULL, 0);

        ret = mbedtls_pk_sign(config->signer.pk_ctx, MBEDTLS_MD_SHA256, (const unsigned char *)config->signer.hash, sizeof(config->signer.hash), config->signer.signature, &retSize, mbedtls_ctr_drbg_random, config->signer.ctr_drbg_ctx);
        if (ret != 0)
        {
            char *tmp = ut->newS(100);
            mbedtls_strerror(ret, tmp, 100);
            config->signer.tokens.error.message = tmp;
            ut->delS(tmp);
            setTokenError(FIREBASE_ERROR_TOKEN_SIGN);
            sendTokenStatusCB();
        }
        else
        {
            config->signer.encSignature.clear();
            size_t len = ut->base64EncLen(config->signer.signatureSize);
            char *buf = ut->newS(len);
            ut->encodeBase64Url(buf, config->signer.signature, config->signer.signatureSize);
            config->signer.encSignature = buf;
            ut->delS(buf);

            config->signer.tokens.jwt += config->signer.encSignature;
            std::string().swap(config->signer.pk);
            std::string().swap(config->signer.encSignature);
        }

        delete[] config->signer.signature;
        delete[] config->signer.hash;
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
        delay(0);
        //parse priv key
        if (config->signer.pk.length() > 0)
            pk = new BearSSL::PrivateKey((const char *)config->signer.pk.c_str());
        else if (strlen_P(config->service_account.data.private_key) > 0)
            pk = new BearSSL::PrivateKey((const char *)config->service_account.data.private_key);

        if (!pk)
        {
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            sendTokenStatusCB();
            return false;
        }

        if (!pk->isRSA())
        {
            setTokenError(FIREBASE_ERROR_TOKEN_PARSE_PK);
            sendTokenStatusCB();
            delete pk;
            return false;
        }

        const br_rsa_private_key *br_rsa_key = pk->getRSA();

        //generate RSA signature from private key and message digest
        config->signer.signature = new unsigned char[config->signer.signatureSize];

        delay(0);
        int ret = br_rsa_i15_pkcs1_sign(BR_HASH_OID_SHA256, (const unsigned char *)config->signer.hash, br_sha256_SIZE, br_rsa_key, config->signer.signature);
        delay(0);
        ut->delS(config->signer.hash);

        size_t len = ut->base64EncLen(config->signer.signatureSize);
        char *buf = ut->newS(len);
        ut->encodeBase64Url(buf, config->signer.signature, config->signer.signatureSize);
        config->signer.encSignature = buf;
        ut->delS(buf);
        delete[] config->signer.signature;
        delete pk;
        //get the signed JWT
        if (ret > 0)
        {
            config->signer.tokens.jwt += config->signer.encSignature;
            std::string().swap(config->signer.pk);
            std::string().swap(config->signer.encSignature);
        }
        else
        {
            setTokenError(FIREBASE_ERROR_TOKEN_SIGN);
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

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
        return false;

    config->signer.signup = false;
    delay(0);

    if (auth == nullptr)
        return false;

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || config->_int.fb_processing)
        return false;

    config->signer.tokens.status = token_status_on_request;
    config->_int.fb_processing = true;
    config->signer.tokens.error.code = 0;
    config->signer.tokens.error.message.clear();
    sendTokenStatusCB();

#if defined(ESP32)
    config->signer.wcs = new FB_HTTPClient32();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(512, 512);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.data = new FirebaseJsonData();

    std::string host;
    if (createUser)
        ut->appendP(host, fb_esp_pgm_str_250);
    else
        ut->appendP(host, fb_esp_pgm_str_193);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_196);
    if (createUser)
    {
        std::string().swap(config->signer.signupError.message);
        if (strlen(email) > 0 && strlen(password) > 0)
            config->signer.json->add(tmp, email);
    }
    else
        config->signer.json->add(tmp, auth->user.email.c_str());
    ut->delS(tmp);
    tmp = ut->strP(fb_esp_pgm_str_197);
    if (createUser)
    {
        if (strlen(email) > 0 && strlen(password) > 0)
            config->signer.json->add(tmp, password);
    }
    else
        config->signer.json->add(tmp, auth->user.password.c_str());
    ut->delS(tmp);
    tmp = ut->strP(fb_esp_pgm_str_198);
    config->signer.json->add(tmp, true);
    ut->delS(tmp);

    String s;
    config->signer.json->toString(s);

    std::string req;
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
    tmp = ut->intStr(s.length());
    req += tmp;
    ut->delS(tmp);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += s.c_str();
#if defined(ESP32)
    if (config->signer.wcs->send(req.c_str(), "") < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    std::string().swap(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    config->signer.json->clear();

    if (handleTokenResponse())
    {
        struct fb_esp_auth_token_error_t error;

        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.data, tmp);
        ut->delS(tmp);

        if (config->signer.data->success)
        {
            error.code = config->signer.data->intValue;
            if (!createUser)
                config->signer.tokens.status = token_status_error;

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                error.message = config->signer.data->stringValue.c_str();
        }

        if (createUser)
            config->signer.signupError = error;
        else
        {
            config->signer.tokens.error = error;
            tokenInfo.status = config->signer.tokens.status;
            tokenInfo.type = config->signer.tokens.token_type;
            tokenInfo.error = config->signer.tokens.error;
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
            }

            tmp = ut->strP(fb_esp_pgm_str_200);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                config->signer.tokens.id_token = config->signer.data->stringValue.c_str();
#if defined(ESP32)
            tmp = ut->strP(fb_esp_pgm_str_201);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                config->signer.tokens.refresh_token = config->signer.data->stringValue.c_str();
#endif

            tmp = ut->strP(fb_esp_pgm_str_202);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                config->signer.tokens.expires = time(nullptr) + atoi(config->signer.data->stringValue.c_str());

            tmp = ut->strP(fb_esp_pgm_str_175);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                auth->token.uid = config->signer.data->stringValue.c_str();

            return handleSignerError(0);
        }

        return handleSignerError(4);
    }

    return handleSignerError(3);
}

bool Firebase_Signer::requestTokens()
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
        return false;

    delay(0);

    if (config->signer.tokens.status == token_status_on_request || config->signer.tokens.status == token_status_on_refresh || time(nullptr) < ut->default_ts || config->_int.fb_processing)
        return false;

    config->signer.tokens.status = token_status_on_request;
    config->_int.fb_processing = true;
    config->signer.tokens.error.code = 0;
    config->signer.tokens.error.message.clear();
    sendTokenStatusCB();

#if defined(ESP32)
    config->signer.wcs = new FB_HTTPClient32();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(512, 512);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.data = new FirebaseJsonData();

    std::string host;
    ut->appendP(host, fb_esp_pgm_str_193);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

    delay(0);
#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    String s;
    std::string req;
    ut->appendP(req, fb_esp_pgm_str_24);
    ut->appendP(req, fb_esp_pgm_str_6);

    if (config->signer.tokens.token_type == token_type_custom_token)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_233);
        config->signer.json->add(tmp, config->signer.tokens.jwt.c_str());
        ut->delS(tmp);
        tmp = ut->strP(fb_esp_pgm_str_198);
        config->signer.json->add(tmp, true);
        ut->delS(tmp);

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
        ut->delS(tmp);
        ut->delS(tmp2);
        tmp = ut->strP(fb_esp_pgm_str_229);
        config->signer.json->add(tmp, config->signer.tokens.jwt.c_str());
        ut->delS(tmp);

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
    config->signer.json->toString(s);
    char *tmp = ut->intStr(s.length());
    req += tmp;
    ut->delS(tmp);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += s.c_str();
#if defined(ESP32)
    config->signer.wcs->setInsecure();
    if (config->signer.wcs->send(req.c_str(), "") < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    std::string().swap(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    struct fb_esp_auth_token_error_t error;

    if (handleTokenResponse())
    {
        std::string().swap(config->signer.tokens.jwt);
        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.data, tmp);
        ut->delS(tmp);
        if (config->signer.data->success)
        {
            error.code = config->signer.data->intValue;
            config->signer.tokens.status = token_status_error;

            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                error.message = config->signer.data->stringValue.c_str();
        }

        config->signer.tokens.error = error;
        tokenInfo.status = config->signer.tokens.status;
        tokenInfo.type = config->signer.tokens.token_type;
        tokenInfo.error = config->signer.tokens.error;
        if (error.code != 0)
            sendTokenStatusCB();

        if (error.code == 0)
        {
            if (config->signer.tokens.token_type == token_type_custom_token)
            {
                tmp = ut->strP(fb_esp_pgm_str_200);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.id_token = config->signer.data->stringValue.c_str();

                tmp = ut->strP(fb_esp_pgm_str_201);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.refresh_token = config->signer.data->stringValue.c_str();

                tmp = ut->strP(fb_esp_pgm_str_202);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.expires = time(nullptr) + atoi(config->signer.data->stringValue.c_str());
            }
            else if (config->signer.tokens.token_type == token_type_oauth2_access_token)
            {
                tmp = ut->strP(fb_esp_pgm_str_235);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.access_token = config->signer.data->stringValue.c_str();

                tmp = ut->strP(fb_esp_pgm_str_236);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.auth_type = config->signer.data->stringValue.c_str();

                tmp = ut->strP(fb_esp_pgm_str_210);
                config->signer.json->get(*config->signer.data, tmp);
                ut->delS(tmp);
                if (config->signer.data->success)
                    config->signer.tokens.expires = time(nullptr) + atoi(config->signer.data->stringValue.c_str());
            }
            return handleSignerError(0);
        }
        return handleSignerError(4);
    }

    return handleSignerError(3);
}

bool Firebase_Signer::handleEmailSending(const char *payload, fb_esp_user_email_sending_type type)
{
    if (config->_int.fb_reconnect_wifi)
        ut->reconnect(0);

    if (WiFi.status() != WL_CONNECTED && !ut->ethLinkUp())
        return false;

    delay(0);

    if (config->_int.fb_processing)
        return false;

    config->_int.fb_processing = true;

#if defined(ESP32)
    config->signer.wcs = new FB_HTTPClient32();
    config->signer.wcs->setCACert(nullptr);
#elif defined(ESP8266)
    config->signer.wcs = new WiFiClientSecure();
    config->signer.wcs->setInsecure();
    config->signer.wcs->setBufferSizes(512, 512);
#endif
    config->signer.json = new FirebaseJson();
    config->signer.data = new FirebaseJsonData();

    std::string host;
    ut->appendP(host, fb_esp_pgm_str_193);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

#if defined(ESP32)
    config->signer.wcs->begin(host.c_str(), 443);
#elif defined(ESP8266)
    int ret = config->signer.wcs->connect(host.c_str(), 443);
    if (ret == 0)
        return handleSignerError(1);
#endif

    char *tmp = ut->strP(fb_esp_pgm_str_260);
    char *tmp2 = nullptr;
    if (type == fb_esp_user_email_sending_type_verify)
    {
        std::string().swap(config->signer.verificationError.message);
        tmp2 = ut->strP(fb_esp_pgm_str_261);
        config->signer.json->add(tmp, (const char *)tmp2);
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        std::string().swap(config->signer.resetPswError.message);
        tmp2 = ut->strP(fb_esp_pgm_str_263);
        config->signer.json->add(tmp, (const char *)tmp2);
    }
    ut->delS(tmp);

    if (type == fb_esp_user_email_sending_type_verify)
    {
        tmp = ut->strP(fb_esp_pgm_str_200);
        if (strlen(payload) > 0)
            config->signer.json->add(tmp, payload);
        else
            config->signer.json->add(tmp, config->signer.tokens.id_token.c_str());
        ut->delS(tmp);
    }
    else if (type == fb_esp_user_email_sending_type_reset_psw)
    {
        tmp = ut->strP(fb_esp_pgm_str_196);
        config->signer.json->add(tmp, payload);
        ut->delS(tmp);
    }

    String s;
    config->signer.json->toString(s);

    std::string req;
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
    tmp = ut->intStr(s.length());
    req += tmp;
    ut->delS(tmp);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_8);
    ut->appendP(req, fb_esp_pgm_str_129);
    ut->appendP(req, fb_esp_pgm_str_21);
    ut->appendP(req, fb_esp_pgm_str_21);

    req += s.c_str();

#if defined(ESP32)
    if (config->signer.wcs->send(req.c_str(), "") < 0)
        return handleSignerError(2);
#elif defined(ESP8266)
    size_t sz = req.length();
    size_t len = config->signer.wcs->print(req.c_str());
    std::string().swap(req);
    if (len != sz)
        return handleSignerError(2);
#endif

    config->signer.json->clear();

    if (handleTokenResponse())
    {

        struct fb_esp_auth_token_error_t error;

        tmp = ut->strP(fb_esp_pgm_str_257);
        config->signer.json->get(*config->signer.data, tmp);
        ut->delS(tmp);

        if (config->signer.data->success)
        {
            error.code = config->signer.data->intValue;
            tmp = ut->strP(fb_esp_pgm_str_258);
            config->signer.json->get(*config->signer.data, tmp);
            ut->delS(tmp);
            if (config->signer.data->success)
                error.message = config->signer.data->stringValue.c_str();
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

    return handleSignerError(3);
}

void Firebase_Signer::checkToken()
{
    if (!config || !auth)
        return;
    if (config->host.length() == 0)
        return;

    if (config->signer.preRefreshSeconds > config->signer.tokens.expires && config->signer.tokens.expires > 0)
        config->signer.preRefreshSeconds = 60;

    if ((config->signer.tokens.token_type == token_type_id_token || config->signer.tokens.token_type == token_type_custom_token || config->signer.tokens.token_type == token_type_oauth2_access_token) && ((unsigned long)time(nullptr) > config->signer.tokens.expires - config->signer.preRefreshSeconds || config->signer.tokens.expires == 0))
        handleToken();
}

bool Firebase_Signer::tokenReady()
{
    if (!auth || !config)
        return false;
    if (config->host.length() == 0)
        return false;
    checkToken();
    return config->signer.tokens.status == token_status_ready;
};

void Firebase_Signer::errorToString(int httpCode, std::string &buff)
{
    buff.clear();

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
    case FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_REFUSED:
        ut->appendP(buff, fb_esp_pgm_str_39);
        return;
    case FIREBASE_ERROR_HTTPC_ERROR_SEND_HEADER_FAILED:
        ut->appendP(buff, fb_esp_pgm_str_40);
        return;
    case FIREBASE_ERROR_HTTPC_ERROR_SEND_PAYLOAD_FAILED:
        ut->appendP(buff, fb_esp_pgm_str_41);
        return;
    case FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED:
        ut->appendP(buff, fb_esp_pgm_str_42);
        return;
    case FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_LOST:
        ut->appendP(buff, fb_esp_pgm_str_43);
        return;
    case FIREBASE_ERROR_HTTPC_ERROR_NO_HTTP_SERVER:
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
    case FIREBASE_ERROR_HTTPC_ERROR_READ_TIMEOUT:
        ut->appendP(buff, fb_esp_pgm_str_69);
        return;
    case FIREBASE_ERROR_DATA_TYPE_MISMATCH:
        ut->appendP(buff, fb_esp_pgm_str_70);
        return;
    case FIREBASE_ERROR_PATH_NOT_EXIST:
        ut->appendP(buff, fb_esp_pgm_str_71);
        return;
    case FIREBASE_ERROR_HTTPC_ERROR_CONNECTION_INUSED:
        ut->appendP(buff, fb_esp_pgm_str_94);
        return;
    case FIREBASE_ERROR_HTTPC_MAX_REDIRECT_REACHED:
        ut->appendP(buff, fb_esp_pgm_str_169);
        return;
    case FIREBASE_ERROR_BUFFER_OVERFLOW:
        ut->appendP(buff, fb_esp_pgm_str_68);
        return;
    case FIREBASE_ERROR_HTTPC_NO_FCM_REGISTRATION_ID_PROVIDED:
        ut->appendP(buff, fb_esp_pgm_str_145);
        return;
    case FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED:
        ut->appendP(buff, fb_esp_pgm_str_146);
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
    case FIREBASE_ERROR_FILE_NOT_FOUND:
        ut->appendP(buff, fb_esp_pgm_str_449);
        return;
    case FIREBASE_ERROR_ARCHIVE_NOT_FOUND:
        ut->appendP(buff, fb_esp_pgm_str_450);
        return;
    case FIREBASE_ERROR_TOKEN_NOT_READY:
        ut->appendP(buff, fb_esp_pgm_str_252);
        return;
    case FIREBASE_ERROR_UNINITIALIZED:
        ut->appendP(buff, fb_esp_pgm_str_256);
        return;
    case FIREBASE_ERROR_HTTPC_FCM_OAUTH2_REQUIRED:
        ut->appendP(buff, fb_esp_pgm_str_328);
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
    default:
        return;
    }
}

fb_esp_auth_token_type Firebase_Signer::getTokenType()
{
    if (!auth || !config)
        return token_type_undefined;
    return config->signer.tokens.token_type;
}

std::string Firebase_Signer::getToken(fb_esp_auth_token_type type)
{
    if (!auth || !config)
        return "";

    if (type == token_type_id_token || type == token_type_custom_token)
        return config->signer.tokens.id_token;
    else if (type == token_type_oauth2_access_token)
        return config->signer.tokens.access_token;
    else if (type == token_type_legacy_token)
        return config->signer.tokens.legacy_token;

    return config->signer.tokens.legacy_token;
}

FirebaseConfig *Firebase_Signer::getCfg()
{
    return config;
}

FirebaseAuth *Firebase_Signer::getAuth()
{
    return auth;
}

std::string Firebase_Signer::getCAFile()
{
    if (!auth || !config)
        return "";
    return config->cert.file;
}
int Firebase_Signer::getCAFileStorage()
{
    if (!auth || !config)
        return 0;
    return config->cert.file_storage;
}

Firebase_Signer Signer = Firebase_Signer();

#endif