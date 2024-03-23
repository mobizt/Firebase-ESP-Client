
/**
 * The Firebase class, Firebase.cpp v1.2.9
 *
 *  Created March 23, 2024
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

#ifndef Firebase_CPP
#define Firebase_CPP

#include <Arduino.h>
#include "./mbfs/MB_MCU.h"
#include "Firebase.h"

FIREBASE_CLASS::FIREBASE_CLASS()
{
    Core.begin(nullptr, nullptr);
}

FIREBASE_CLASS::~FIREBASE_CLASS()
{
    if (auth)
        delete auth;
    auth = nullptr;

    if (config)
    {
        Core.internal.sessions.clear();
        Core.internal.queueSessions.clear();
        delete config;
        config = nullptr;
    }
}

void FIREBASE_CLASS::begin(FirebaseConfig *config, FirebaseAuth *auth)
{
    init(config, auth);

    if (!config->signer.test_mode)
    {

        firebase_auth_token_type type = config->signer.tokens.token_type;

        bool itoken_set = config->signer.idTokenCustomSet;
        bool atoken_set = config->signer.accessTokenCustomSet;
        bool ctoken_set = config->signer.customTokenCustomSet;
        uint32_t exp = config->signer.tokens.expires;

        Core.checkAuthTypeChanged(config, auth);

        if (Core.internal.fb_rtoken_requested || atoken_set)
            config->signer.tokens.token_type = type;

        if (atoken_set)
        {
            config->signer.accessTokenCustomSet = atoken_set;
            config->signer.tokens.expires = exp;
        }

        if (itoken_set)
        {
            config->signer.idTokenCustomSet = itoken_set;
            config->signer.tokens.expires = exp;
        }

        if (ctoken_set)
        {
            config->signer.customTokenCustomSet = ctoken_set;
        }

        struct firebase_url_info_t uinfo;
        Core.internal.fb_auth_uri = config->signer.tokens.token_type == token_type_legacy_token ||
                                    config->signer.tokens.token_type == token_type_id_token;

        if (config->host.length() > 0)
            config->database_url = config->host;

        if (config->database_url.length() > 0)
        {
            Core.uh.parse(&Core.mbfs, config->database_url.c_str(), uinfo);
            config->database_url = uinfo.host.c_str();
        }
    }

    if (Core.internal.fb_rtoken_requested)
    {
        if (config->signer.tokens.token_type == token_type_oauth2_access_token)
            Core.requestTokens(true);
        else
            Core.refreshToken();

        Core.internal.fb_rtoken_requested = false;
        return;
    }

    Core.handleToken();
}

struct token_info_t FIREBASE_CLASS::authTokenInfo()
{
    return Core.tokenInfo;
}

bool FIREBASE_CLASS::ready()
{
    if (Core.isExpired())
    {
        for (size_t id = 0; id < Core.internal.sessions.size(); id++)
        {
            FirebaseData *fbdo = addrTo<FirebaseData *>(Core.internal.sessions[id].ptr);
            if (fbdo)
                fbdo->closeSession();
        }
    }
    return Core.tokenReady();
}

bool FIREBASE_CLASS::authenticated()
{
    return Core.authenticated;
}

bool FIREBASE_CLASS::mSignUp(FirebaseConfig *config, FirebaseAuth *auth, MB_StringPtr email, MB_StringPtr password)
{
    init(config, auth);
    Core.setTokenType(token_type_id_token);
    return Core.getIdToken(true, email, password);
}

bool FIREBASE_CLASS::msendEmailVerification(FirebaseConfig *config, MB_StringPtr idToken)
{
    init(config, nullptr);
    return Core.handleEmailSending(idToken, firebase_user_email_sending_type_verify);
}

bool FIREBASE_CLASS::mDeleteUser(FirebaseConfig *config, FirebaseAuth *auth, MB_StringPtr idToken)
{
    init(config, auth);
    return Core.deleteIdToken(idToken);
}

bool FIREBASE_CLASS::mSendResetPassword(FirebaseConfig *config, MB_StringPtr email)
{
    init(config, nullptr);
    return Core.handleEmailSending(email, firebase_user_email_sending_type_reset_psw);
}

void FIREBASE_CLASS::mSetAuthToken(FirebaseConfig *config, MB_StringPtr authToken, size_t expire,
                                   MB_StringPtr refreshToken, firebase_auth_token_type type,
                                   MB_StringPtr clientId, MB_StringPtr clientSecret)
{
    if (!config)
        return;

    this->reset(config);

    bool refresh = false;

    MB_String _authToken = authToken;
    Core.internal.refresh_token = refreshToken;
    Core.internal.client_id = clientId;
    Core.internal.client_secret = clientSecret;

    if (Core.internal.refresh_token.length() == 0 && _authToken.length() == 0)
        return;

    if (type == token_type_custom_token)
    {
        if (_authToken.length() > 0)
        {
            size_t p1 = _authToken.find('.');
            if (p1 == MB_String::npos || _authToken.find('.', p1 + 1) == MB_String::npos)
            {
                _authToken.clear();
                Core.internal.refresh_token = authToken;
            }
        }
    }

    // in case refresh token was assigned and id token is empty
    if (_authToken.length() == 0 && Core.internal.refresh_token.length() > 0)
    {
        _authToken.append(1, '?');
        refresh = true;
    }

    if (_authToken.length() == 0 || strcmp(Core.internal.auth_token.c_str(), _authToken.c_str()) == 0)
        return;

    _authToken.clear();

    Core.internal.auth_token = authToken;
    Core.internal.atok_len = Core.internal.auth_token.length();
    Core.internal.rtok_len = Core.internal.refresh_token.length();

    if (expire > 3600)
        expire = 3600;

    if (expire > 0)
        config->signer.tokens.expires += Core.getTime() + expire;
    else
        config->signer.tokens.expires = 0;

    config->signer.tokens.status = token_status_ready;
    config->signer.step = firebase_jwt_generation_step_begin;
    config->signer.tokens.token_type = type;
    config->signer.anonymous = true;

    if (type == token_type_id_token)
        config->signer.idTokenCustomSet = true;
    else if (type == token_type_oauth2_access_token)
        config->signer.accessTokenCustomSet = true;
    else if (type == token_type_custom_token && !refresh)
        config->signer.customTokenCustomSet = true;

    if (refresh)
        this->refreshToken(config);
}

bool FIREBASE_CLASS::isTokenExpired()
{
    return Core.isExpired();
}

void FIREBASE_CLASS::refreshToken(FirebaseConfig *config)
{
    if (config)
    {
        config->signer.lastReqMillis = 0;
        config->signer.tokens.expires = 0;

        if (auth && config)
        {
            Core.internal.fb_rtoken_requested = false;

            if (config->signer.tokens.token_type == token_type_oauth2_access_token)
                Core.requestTokens(true);
            else
                Core.refreshToken();
        }
        else
            Core.internal.fb_rtoken_requested = true;
    }
}

void FIREBASE_CLASS::reset(FirebaseConfig *config)
{
    if (config)
    {
        Core.internal.client_id.clear();
        Core.internal.client_secret.clear();
        Core.internal.auth_token.clear();
        Core.internal.refresh_token.clear();
        Core.internal.atok_len = 0;
        Core.internal.rtok_len = 0;
        Core.internal.ltok_len = 0;
        config->signer.lastReqMillis = 0;
        Core.internal.fb_last_jwt_generation_error_cb_millis = 0;
        config->signer.tokens.expires = 0;
        Core.internal.fb_rtoken_requested = false;
        config->signer.accessTokenCustomSet = false;
        config->signer.idTokenCustomSet = false;
        config->signer.customTokenCustomSet = false;
        config->signer.anonymous = false;

        Core.internal.client_email_crc = 0;
        Core.internal.project_id_crc = 0;
        Core.internal.priv_key_crc = 0;
        Core.internal.email_crc = 0;
        Core.internal.password_crc = 0;

        config->signer.tokens.status = token_status_uninitialized;
    }
}

void FIREBASE_CLASS::init(FirebaseConfig *config, FirebaseAuth *auth)
{
    this->auth = auth;
    this->config = config;

    if (!config)
        config = new FirebaseConfig();

    if (!this->auth)
        this->auth = new FirebaseAuth();

    Core.internal.fb_reconnect_network = Core.autoReconnectNetwork;

    config->signer.lastReqMillis = 0;

    if (!config->signer.anonymous && !config->signer.signup)
        config->signer.tokens.expires = 0;

    config->signer.signup = false;
    Core.begin(config, auth);
    config->signer.tokens.error.message.clear();
}

void FIREBASE_CLASS::reconnectNetwork(bool reconnect)
{
#if defined(FIREBASE_WIFI_IS_AVAILABLE) && (defined(ESP32) || defined(ESP8266))
    WiFi.setAutoReconnect(reconnect);
#endif
    Core.setAutoReconnectNetwork(reconnect);
}

void FIREBASE_CLASS::reconnectWiFi(bool reconnect)
{
    reconnectNetwork(reconnect);
}

time_t FIREBASE_CLASS::getCurrentTime()
{
    return Core.getTime();
}

int FIREBASE_CLASS::getFreeHeap()
{
#if defined(MB_ARDUINO_ESP)
    return ESP.getFreeHeap();
#elif defined(MB_ARDUINO_PICO)
    return rp2040.getFreeHeap();
#else
    return 0;
#endif
}

const char *FIREBASE_CLASS::getToken()
{
    return Core.getToken();
}

const char *FIREBASE_CLASS::getRefreshToken()
{
    return Core.getRefreshToken();
}

void FIREBASE_CLASS::setFloatDigits(uint8_t digits)
{
    if (digits < 7)
        Core.internal.fb_float_digits = digits;
}

void FIREBASE_CLASS::setDoubleDigits(uint8_t digits)
{
    if (digits < 9)
        Core.internal.fb_double_digits = digits;
}

#if defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)

bool FIREBASE_CLASS::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi, uint32_t frequency)
{
    return Core.mbfs.sdBegin(ss, sck, miso, mosi, frequency);
}

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)
bool FIREBASE_CLASS::sdBegin(SDFSConfig *sdFSConfig)
{
    return Core.mbfs.sdFatBegin(sdFSConfig);
}
#endif

#if defined(ESP32)

bool FIREBASE_CLASS::sdBegin(int8_t ss, SPIClass *spiConfig, uint32_t frequency)
{
    return Core.mbfs.sdSPIBegin(ss, spiConfig, frequency);
}
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED) || defined(MBFS_SDFAT_ENABLED)
bool FIREBASE_CLASS::sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
    return Core.mbfs.sdFatBegin(sdFatSPIConfig, ss, sck, miso, mosi);
}

bool FIREBASE_CLASS::sdBegin(SdioConfig *sdFatSDIOConfig)
{
    return Core.mbfs.sdFatBegin(sdFatSDIOConfig);
}
#endif

#endif

#if defined(ESP32) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD_MMC)

bool FIREBASE_CLASS::sdMMCBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
{
    return Core.mbfs.sdMMCBegin(mountpoint, mode1bit, format_if_mount_failed);
}

#endif

bool FIREBASE_CLASS::setSystemTime(time_t ts)
{
    return Core.setTime(ts);
}

FIREBASE_CLASS Firebase = FIREBASE_CLASS();

#endif /* Firebase_CPP */