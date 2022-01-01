/**
 * The Firebase class, Firebase.cpp v1.0.14
 * 
 *  Created December 27, 2021
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

#ifndef Firebase_CPP
#define Firebase_CPP

#if defined(ESP8266) || defined(ESP32)

#include "Firebase.h"

#if defined(FIREBASE_ESP_CLIENT)

Firebase_ESP_Client::Firebase_ESP_Client() {}

Firebase_ESP_Client::~Firebase_ESP_Client()
{
    if (ut)
        delete ut;

    if (auth)
        delete auth;

    if (cfg)
        delete cfg;
}

void Firebase_ESP_Client::begin(FirebaseConfig *config, FirebaseAuth *auth)
{
    init(config, auth);
    if (!cfg->signer.test_mode)
    {

        if (cfg->service_account.json.path.length() > 0)
        {
            if (!Signer.parseSAFile())
                cfg->signer.tokens.status = token_status_uninitialized;
        }

        if (strlen(cfg->signer.tokens.legacy_token) > 0)
        {
            Signer.setTokenType(token_type_legacy_token);
            cfg->_int.auth_token = cfg->signer.tokens.legacy_token;
            cfg->_int.ltok_len = strlen(cfg->signer.tokens.legacy_token);
            cfg->_int.rtok_len = 0;
            cfg->_int.atok_len = 0;
        }
        else if (Signer.tokenSigninDataReady())
        {
            cfg->signer.idTokenCutomSet = false;

            if (auth->token.uid.length() == 0)
                Signer.setTokenType(token_type_oauth2_access_token);
            else
                Signer.setTokenType(token_type_custom_token);
        }
        else if (Signer.userSigninDataReady() || cfg->signer.anonymous)
            Signer.setTokenType(token_type_id_token);

        struct fb_esp_url_info_t uinfo;
        cfg->_int.fb_auth_uri = cfg->signer.tokens.token_type == token_type_legacy_token || cfg->signer.tokens.token_type == token_type_id_token;

        if (cfg->host.length() > 0)
            cfg->database_url = cfg->host;

        if (cfg->database_url.length() > 0)
        {
            ut->getUrlInfo(cfg->database_url.c_str(), uinfo);
            cfg->database_url = uinfo.host.c_str();
        }

        if (cfg->cert.file.length() > 0)
        {
            if (cfg->cert.file_storage == mem_storage_type_sd && !cfg->_int.fb_sd_rdy)
                cfg->_int.fb_sd_rdy = ut->sdTest(cfg->_int.fb_file);
            else if ((cfg->cert.file_storage == mem_storage_type_flash) && !cfg->_int.fb_flash_rdy)
                ut->flashTest();
        }
    }

    Signer.handleToken();
}

struct token_info_t Firebase_ESP_Client::authTokenInfo()
{
    return Signer.tokenInfo;
}

bool Firebase_ESP_Client::ready()
{
    return Signer.tokenReady();
}

bool Firebase_ESP_Client::authenticated()
{
    return Signer.authenticated;
}

void Firebase_ESP_Client::setIdToken(FirebaseConfig *config, const char *idToken, size_t expire)
{
    if (!config)
        return;

    if (idToken)
    {
        if (strlen(idToken) == 0 || strcmp(config->_int.auth_token.c_str(), idToken) == 0)
            return;

        MBSTRING copy = idToken;
        config->_int.auth_token = copy;
        copy.clear();
        config->_int.atok_len = config->_int.auth_token.length();
        config->_int.ltok_len = 0;

        if (expire > 3600)
            expire = 3600;

        config->signer.tokens.expires += time(nullptr) + expire;

        config->signer.tokens.status = token_status_ready;
        config->signer.attempts = 0;
        config->signer.step = fb_esp_jwt_generation_step_begin;
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        config->signer.tokens.token_type = token_type_id_token;
        config->signer.anonymous = true;
        config->signer.idTokenCutomSet = true;
    }
}

bool Firebase_ESP_Client::isTokenExpired()
{
    return Signer.isExpired();
}

void Firebase_ESP_Client::init(FirebaseConfig *config, FirebaseAuth *auth)
{
    this->auth = auth;
    cfg = config;

    if (!cfg)
        cfg = new FirebaseConfig();

    if (!this->auth)
        this->auth = new FirebaseAuth();

    if (!ut)
        ut = new UtilsClass(config);

#ifdef ENABLE_RTDB
    RTDB.begin(ut);
#endif
#ifdef ENABLE_FCM
    FCM.begin(ut);
#endif
#ifdef ENABLE_FB_STORAGE
    Storage.begin(ut);
#endif
#ifdef ENABLE_FIRESTORE
    Firestore.begin(ut);
#endif
#ifdef ENABLE_FB_FUNCTIONS
    Functions.begin(ut);
#endif
#ifdef ENABLE_GC_STORAGE
    GCStorage.begin(ut);
#endif

    cfg->_int.fb_reconnect_wifi = WiFi.getAutoReconnect();

    cfg->signer.lastReqMillis = 0;

    //don't clear auth token if anonymous sign in or Email/Password sign up
    if (!cfg->signer.anonymous && !cfg->signer.signup)
    {
        cfg->_int.auth_token.clear();
        cfg->signer.tokens.expires = 0;
    }

    if (auth->user.email.length() > 0 && auth->user.password.length() > 0)
        cfg->signer.idTokenCutomSet = false;

    cfg->signer.signup = false;
    Signer.begin(ut, cfg, auth);
    cfg->signer.tokens.error.message.clear();
}

void Firebase_ESP_Client::reconnectWiFi(bool reconnect)
{
    WiFi.setAutoReconnect(reconnect);
}

const char *Firebase_ESP_Client::getToken()
{
    return Signer.getToken();
}

void Firebase_ESP_Client::setFloatDigits(uint8_t digits)
{
    if (digits < 7 && cfg)
        cfg->_int.fb_float_digits = digits;
}

void Firebase_ESP_Client::setDoubleDigits(uint8_t digits)
{
    if (digits < 9 && cfg)
        cfg->_int.fb_double_digits = digits;
}

bool Firebase_ESP_Client::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
#if defined SD_FS
    if (cfg)
    {
        cfg->_int.sd_config.sck = sck;
        cfg->_int.sd_config.miso = miso;
        cfg->_int.sd_config.mosi = mosi;
        cfg->_int.sd_config.ss = ss;
    }
#if defined(ESP32)
    if (ss > -1)
    {
        SPI.begin(sck, miso, mosi, ss);
#if defined(CARD_TYPE_SD)
        return SD_FS.begin(ss, SPI);
#endif
        return false;
    }
    else
        return SD_FS.begin();
#elif defined(ESP8266)
    if (ss > -1)
        return SD_FS.begin(ss);
    else
        return SD_FS.begin(SD_CS_PIN);
#endif
#endif
    return false;
}

bool Firebase_ESP_Client::sdMMCBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
{
#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
    if (cfg)
    {
        cfg->_int.sd_config.sd_mmc_mountpoint = mountpoint;
        cfg->_int.sd_config.sd_mmc_mode1bit = mode1bit;
        cfg->_int.sd_config.sd_mmc_format_if_mount_failed = format_if_mount_failed;
    }
    return SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
#endif
#endif
    return false;
}

bool Firebase_ESP_Client::setSystemTime(time_t ts)
{
    return ut->setTimestamp(ts) == 0;
}

Firebase_ESP_Client Firebase = Firebase_ESP_Client();

#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)

FIREBASE_CLASS::FIREBASE_CLASS()
{
}

FIREBASE_CLASS::~FIREBASE_CLASS()
{
    if (ut)
        delete ut;

    if (!extConfig)
    {
        if (cfg)
            delete cfg;

        if (auth)
            delete auth;
    }
}

void FIREBASE_CLASS::begin(FirebaseConfig *config, FirebaseAuth *auth)
{
    init(config, auth);

    if (cfg->service_account.json.path.length() > 0)
    {
        if (!Signer.parseSAFile())
            cfg->signer.tokens.status = token_status_uninitialized;
    }

    if (strlen(cfg->signer.tokens.legacy_token) > 0)
    {
        Signer.setTokenType(token_type_legacy_token);
        cfg->_int.auth_token = cfg->signer.tokens.legacy_token;
        cfg->_int.ltok_len = strlen(cfg->signer.tokens.legacy_token);
        cfg->_int.rtok_len = 0;
        cfg->_int.atok_len = 0;
    }
    else if (Signer.tokenSigninDataReady())
    {
        if (auth->token.uid.length() == 0)
            Signer.setTokenType(token_type_oauth2_access_token);
        else
            Signer.setTokenType(token_type_custom_token);
    }
    else if (Signer.userSigninDataReady())
        Signer.setTokenType(token_type_id_token);

    struct fb_esp_url_info_t uinfo;
    cfg->_int.fb_auth_uri = cfg->signer.tokens.token_type == token_type_legacy_token || cfg->signer.tokens.token_type == token_type_id_token;

    if (cfg->host.length() > 0)
        cfg->database_url = cfg->host;

    if (cfg->database_url.length() > 0)
    {
        ut->getUrlInfo(cfg->database_url.c_str(), uinfo);
        cfg->database_url = uinfo.host;
    }
    if (cfg->cert.file.length() > 0)
    {
        if (cfg->cert.file_storage == mem_storage_type_sd && !cfg->_int.fb_sd_rdy)
            cfg->_int.fb_sd_rdy = ut->sdTest(cfg->_int.fb_file);
        else if (cfg->cert.file_storage == mem_storage_type_flash && !cfg->_int.fb_flash_rdy)
            ut->flashTest();
    }

    Signer.handleToken();
}

void FIREBASE_CLASS::end(FirebaseData &fbdo)
{
#ifdef ENABLE_RTDB
    endStream(fbdo);
    removeStreamCallback(fbdo);
#endif
    fbdo.clear();
}

struct token_info_t FIREBASE_CLASS::authTokenInfo()
{
    return Signer.tokenInfo;
}

bool FIREBASE_CLASS::ready()
{
    return Signer.tokenReady();
}

bool FIREBASE_CLASS::authenticated()
{
    return Signer.authenticated;
}

void FIREBASE_CLASS::setIdToken(FirebaseConfig *config, const char *idToken, size_t expire)
{
    if (!config)
        return;

    if (idToken)
    {
        if (strlen(idToken) == 0 || strcmp(config->_int.auth_token.c_str(), idToken) == 0)
            return;

        MBSTRING copy = idToken;
        config->_int.auth_token = copy;
        copy.clear();
        config->_int.atok_len = config->_int.auth_token.length();
        config->_int.ltok_len = 0;

        if (expire > 3600)
            expire = 3600;

        config->signer.tokens.expires += time(nullptr) + expire;

        config->signer.tokens.status = token_status_ready;
        config->signer.attempts = 0;
        config->signer.step = fb_esp_jwt_generation_step_begin;
        config->_int.fb_last_jwt_generation_error_cb_millis = 0;
        config->signer.tokens.token_type = token_type_id_token;
        config->signer.anonymous = true;
        config->signer.idTokenCutomSet = true;
    }
}

bool FIREBASE_CLASS::isTokenExpired()
{
    return Signer.isExpired();
}

void FIREBASE_CLASS::init(FirebaseConfig *config, FirebaseAuth *auth)
{
    if (!this->auth)
        this->auth = auth;

    if (!this->cfg)
        this->cfg = config;

    if (!this->cfg)
        this->cfg = new FirebaseConfig();

    if (!this->auth)
        this->auth = new FirebaseAuth();

    if (ut)
        delete ut;

    ut = new UtilsClass(this->cfg);
#ifdef ENABLE_RTDB
    RTDB.begin(ut);
#endif
    cfg->_int.fb_reconnect_wifi = WiFi.getAutoReconnect();

    cfg->signer.lastReqMillis = 0;

    //don't clear auth token if anonymous sign in or Email/Password sign up
    if (!cfg->signer.anonymous && !cfg->signer.signup)
    {
        cfg->_int.auth_token.clear();
        cfg->signer.tokens.expires = 0;
    }

    if (auth->user.email.length() > 0 && auth->user.password.length() > 0)
        cfg->signer.idTokenCutomSet = false;

    cfg->signer.signup = false;
    Signer.begin(ut, this->cfg, this->auth);
    cfg->signer.tokens.error.message.clear();
}

void FIREBASE_CLASS::reconnectWiFi(bool reconnect)
{
    WiFi.setAutoReconnect(reconnect);
}

const char *FIREBASE_CLASS::getToken()
{
    return Signer.getToken();
}

void FIREBASE_CLASS::setFloatDigits(uint8_t digits)
{
    if (digits < 7)
        cfg->_int.fb_float_digits = digits;
}

void FIREBASE_CLASS::setDoubleDigits(uint8_t digits)
{
    if (digits < 9)
        cfg->_int.fb_double_digits = digits;
}

#ifdef ENABLE_FCM
bool FIREBASE_CLASS::handleFCMRequest(FirebaseData &fbdo, fb_esp_fcm_msg_type messageType)
{
    fbdo._spi_ethernet_module = fbdo.fcm._spi_ethernet_module;

    if (!fbdo.reconnect())
        return false;

    if (!ut)
        ut = new UtilsClass(nullptr);

    if (!ut->waitIdle(fbdo._ss.http_code))
        return false;

    FirebaseJsonData data;

    FirebaseJson *json = fbdo.to<FirebaseJson *>();
    json->setJsonData(fbdo.fcm.raw);

    MBSTRING s;
    s.appendP(fb_esp_pgm_str_577, true);

    json->get(data, s.c_str());

    if (data.stringValue.length() == 0)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    if (fbdo.fcm.idTokens.length() == 0 && messageType == fb_esp_fcm_msg_type::msg_single)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_NO_FCM_ID_TOKEN_PROVIDED;
        return false;
    }

    FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
    arr->setJsonArrayData(fbdo.fcm.idTokens.c_str());

    if (messageType == fb_esp_fcm_msg_type::msg_single && fbdo.fcm.idTokens.length() > 0 && fbdo.fcm._index > arr->size() - 1)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_FCM_ID_TOKEN_AT_INDEX_NOT_FOUND;
        return false;
    }

    s.appendP(fb_esp_pgm_str_576, true);

    json->get(data, s.c_str());

    if (messageType == fb_esp_fcm_msg_type::msg_topic && data.stringValue.length() == 0)
    {
        fbdo._ss.http_code = FIREBASE_ERROR_NO_FCM_TOPIC_PROVIDED;
        return false;
    }

    json->clear();
    arr->clear();

    fbdo.fcm.fcm_begin(fbdo);

    return fbdo.fcm.fcm_send(fbdo, messageType);
}

bool FIREBASE_CLASS::sendMessage(FirebaseData &fbdo, uint16_t index)
{
    fbdo.fcm._index = index;
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_type::msg_single);
}

bool FIREBASE_CLASS::broadcastMessage(FirebaseData &fbdo)
{
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_type::msg_multicast);
}

bool FIREBASE_CLASS::sendTopic(FirebaseData &fbdo)
{
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_type::msg_topic);
}

#endif

#ifdef ESP8266
bool FIREBASE_CLASS::sdBegin(int8_t ss)
{
#if defined SD_FS
    if (cfg)
    {
        cfg->_int.sd_config.sck = -1;
        cfg->_int.sd_config.miso = -1;
        cfg->_int.sd_config.mosi = -1;
        cfg->_int.sd_config.ss = ss;
    }

    if (ss > -1)
        return SD_FS.begin(ss);
    else
        return SD_FS.begin(SD_CS_PIN);
#else
    return false;
#endif
}
#endif

#ifdef ESP32

bool FIREBASE_CLASS::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
#if defined SD_FS
    if (cfg)
    {
        cfg->_int.sd_config.sck = sck;
        cfg->_int.sd_config.miso = miso;
        cfg->_int.sd_config.mosi = mosi;
        cfg->_int.sd_config.ss = ss;
    }
#if defined(ESP32)
    if (ss > -1)
    {
        SPI.begin(sck, miso, mosi, ss);
#if defined(CARD_TYPE_SD)
        return SD_FS.begin(ss, SPI);
#endif
        return false;
    }
    else
        return SD_FS.begin();
#elif defined(ESP8266)
    if (ss > -1)
        return SD_FS.begin(ss);
    else
        return SD_FS.begin(SD_CS_PIN);
#endif
#endif
    return false;
}

bool FIREBASE_CLASS::sdMMCBegin(const String &mountpoint, bool mode1bit, bool format_if_mount_failed)
{
#if defined(SD_FS) && defined(ESP32) && defined(CARD_TYPE_SD_MMC)
    if (cfg)
    {
        cfg->_int.sd_config.sd_mmc_mountpoint = mountpoint;
        cfg->_int.sd_config.sd_mmc_mode1bit = mode1bit;
        cfg->_int.sd_config.sd_mmc_format_if_mount_failed = format_if_mount_failed;
    }
    return SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
#endif
    return false;
}

#endif

fb_esp_mem_storage_type FIREBASE_CLASS::getMemStorageType(uint8_t old_type)
{
    return (fb_esp_mem_storage_type)(old_type);
}

bool FIREBASE_CLASS::setSystemTime(time_t ts)
{
    return ut->setTimestamp(ts) == 0;
}

FIREBASE_CLASS Firebase = FIREBASE_CLASS();

#endif /*  FIREBASE_ESP32_CLIENT || FIREBASE_ESP8266_CLIENT  */

#endif /* ESP8266 || ESP32 */

#endif /* Firebase_CPP */