/**
 * Google's Firebase Cloud Messaging class, FCM.cpp version 1.0.13
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created September 8, 2021
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

#include "FirebaseFS.h"

#ifdef ENABLE_FCM

#ifndef FIREBASE_FCM_CPP
#define FIREBASE_FCM_CPP
#include "FCM.h"

FB_CM::FB_CM() {}
FB_CM::~FB_CM()
{
    clear();
}

bool FB_CM::init(bool clearInt)
{
    if (clearInt)
    {
        if (ut)
            delete ut;
        if (cfg)
            delete cfg;
        if (auth)
            delete auth;

        ut = nullptr;
        cfg = nullptr;
        auth = nullptr;
    }
    if (!ut)
        ut = new UtilsClass(Signer.getCfg());

    return true;
}

void FB_CM::begin(UtilsClass *u)
{
    init(true);
    ut = u;
}

void FB_CM::mSetServerKey(const char *serverKey, SPI_ETH_Module *spi_ethernet_module)
{
    this->server_key = serverKey;
    this->_spi_ethernet_module = spi_ethernet_module;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_Legacy_HTTP_Message *msg)
{
    if (!Signer.getCfg())
    {
        cfg = new FirebaseConfig();
        auth = new FirebaseAuth();
        ut = new UtilsClass(cfg);
        Signer.begin(ut, cfg, auth);
    }

    if (!init())
        return false;

    if (server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    fcm_prepareLegacyPayload(msg);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_legacy_http, raw.c_str());
    ut->clearS(raw);
    return ret;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_HTTPv1_JSON_Message *msg)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (Signer.getTokenType() != token_type_oauth2_access_token)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_OAUTH2_REQUIRED;
        return false;
    }

    fcm_prepareV1Payload(msg);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_httpv1, raw.c_str());
    ut->clearS(raw);
    return ret;
}

bool FB_CM::mSubscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], const char *numToken)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    fcm_preparSubscriptionPayload(topic, IID, atoi(numToken));
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_subscribe, raw.c_str());
    ut->clearS(raw);
    return ret;
}

bool FB_CM::mUnsubscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], const char *numToken)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    fcm_preparSubscriptionPayload(topic, IID, atoi(numToken));
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_unsubscribe, raw.c_str());
    ut->clearS(raw);
    return ret;
}

bool FB_CM::mAppInstanceInfo(FirebaseData *fbdo, const char *IID)
{
    if (!init())
        return false;

    if (server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    std::string payload = IID;
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_app_instance_info, payload.c_str());
    ut->clearS(payload);
    return ret;
}

bool FB_CM::mRegisAPNsTokens(FirebaseData *fbdo, const char *application, bool sandbox, const char *APNs[], const char *numToken)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    fcm_preparAPNsRegistPayload(application, sandbox, APNs, atoi(numToken));
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_apn_token_registration, raw.c_str());
    ut->clearS(raw);
    return ret;
}

String FB_CM::payload(FirebaseData *fbdo)
{
    return fbdo->_ss.fcm.payload.c_str();
}

void FB_CM::fcm_connect(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode)
{
    fbdo->_spi_ethernet_module = _spi_ethernet_module;

    if (Signer.getCfg())
    {
        if (Signer.getTokenType() != token_type_undefined)
        {
            if (!Signer.tokenReady())
                return;
        }
    }

    std::string host;
    if (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1)
        ut->appendP(host, fb_esp_pgm_str_249);
    else
        ut->appendP(host, fb_esp_pgm_str_329);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

    rescon(fbdo, host.c_str());
    fbdo->tcpClient.begin(host.c_str(), port);
    fbdo->_ss.max_payload_length = 0;
}

int FB_CM::fcm_sendHeader(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, const char *payload)
{
    bool msgMode = (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1);
    int ret = -1;
    std::string header;

    if (mode == fb_esp_fcm_msg_mode_app_instance_info)
        ut->appendP(header, fb_esp_pgm_str_25, true);
    else
        ut->appendP(header, fb_esp_pgm_str_24, true);
    ut->appendP(header, fb_esp_pgm_str_6);

    if (msgMode)
    {
        if (Signer.getTokenType() != token_type_oauth2_access_token || mode == fb_esp_fcm_msg_mode_legacy_http)
            ut->appendP(header, fb_esp_pgm_str_121);
        else
        {
            ut->appendP(header, fb_esp_pgm_str_326);
            header += Signer.getCfg()->service_account.data.project_id;
            ut->appendP(header, fb_esp_pgm_str_327);
        }
    }
    else
    {
        if (mode == fb_esp_fcm_msg_mode_subscribe)
        {
            ut->appendP(header, fb_esp_pgm_str_330);
            ut->appendP(header, fb_esp_pgm_str_331);
        }
        else if (mode == fb_esp_fcm_msg_mode_unsubscribe)
        {
            ut->appendP(header, fb_esp_pgm_str_330);
            ut->appendP(header, fb_esp_pgm_str_332);
        }
        else if (mode == fb_esp_fcm_msg_mode_app_instance_info)
        {
            ut->appendP(header, fb_esp_pgm_str_335);
            header += payload;
            ut->appendP(header, fb_esp_pgm_str_336);
        }
        else if (mode == fb_esp_fcm_msg_mode_apn_token_registration)
        {
            ut->appendP(header, fb_esp_pgm_str_330);
            ut->appendP(header, fb_esp_pgm_str_333);
        }
    }

    ut->appendP(header, fb_esp_pgm_str_30);

    ut->appendP(header, fb_esp_pgm_str_31);
    if (msgMode)
        ut->appendP(header, fb_esp_pgm_str_249);
    else
        ut->appendP(header, fb_esp_pgm_str_329);
    ut->appendP(header, fb_esp_pgm_str_4);
    ut->appendP(header, fb_esp_pgm_str_120);
    ut->appendP(header, fb_esp_pgm_str_21);

    if (Signer.getTokenType() == token_type_oauth2_access_token && mode == fb_esp_fcm_msg_mode_httpv1)
    {
        ut->appendP(header, fb_esp_pgm_str_237);
        ut->appendP(header, fb_esp_pgm_str_271);

        ret = fbdo->tcpSend(header.c_str());
        ut->clearS(header);

        if (ret < 0)
            return ret;

        ret = fbdo->tcpSend(Signer.getToken());

        if (ret < 0)
            return ret;
    }
    else
    {
        ut->appendP(header, fb_esp_pgm_str_131);

        ret = fbdo->tcpSend(header.c_str());
        ut->clearS(header);

        if (ret < 0)
            return ret;

        ret = fbdo->tcpSend(server_key.c_str());

        if (ret < 0)
            return ret;
    }

    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_32);

    if (mode != fb_esp_fcm_msg_mode_app_instance_info)
    {
        ut->appendP(header, fb_esp_pgm_str_8);
        ut->appendP(header, fb_esp_pgm_str_129);
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);
        header += NUM2S(strlen(payload)).get();
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    ut->appendP(header, fb_esp_pgm_str_36);
    ut->appendP(header, fb_esp_pgm_str_21);

    ret = fbdo->tcpSend(header.c_str());
    ut->clearS(header);

    return ret;
}

void FB_CM::fcm_prepareLegacyPayload(FCM_Legacy_HTTP_Message *msg)
{

    std::string s;

    ut->clearS(raw);

    FirebaseJson json;

    if (strlen(msg->targets.to) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_128, true);
        json.add(s.c_str(), msg->targets.to);
    }

    if (strlen(msg->targets.registration_ids) > 0)
    {
        FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->targets.registration_ids);

        ut->appendP(s, fb_esp_pgm_str_130, true);
        json.add(s.c_str(), arr);
    }

    if (strlen(msg->targets.condition) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_282, true);
        json.add(s.c_str(), msg->targets.condition);
    }

    if (strlen(msg->options.collapse_key) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_138, true);
        json.add(s.c_str(), msg->options.collapse_key);
    }

    if (strlen(msg->options.priority) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_136, true);
        json.add(s.c_str(), msg->options.priority);
    }

    if (strlen(msg->options.content_available) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_283, true);
        json.add(s.c_str(), ut->boolVal(msg->options.content_available));
    }

    if (strlen(msg->options.mutable_content) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_284, true);
        json.add(s.c_str(), ut->boolVal(msg->options.mutable_content));
    }

    if (strlen(msg->options.time_to_live) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_137, true);
        json.add(s.c_str(), atoi(msg->options.time_to_live));
    }

    if (strlen(msg->options.restricted_package_name) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_147, true);
        json.add(s.c_str(), msg->options.restricted_package_name);
    }

    if (strlen(msg->options.dry_run) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_281, true);
        json.add(s.c_str(), msg->options.dry_run);
    }

    if (strlen(msg->options.direct_boot_ok) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_323, true);
        json.add(s.c_str(), ut->boolVal(msg->options.direct_boot_ok));
    }

    if (strlen(msg->payloads.data) > 0)
    {
        FirebaseJson js;
        js.setJsonData(msg->payloads.data);
        ut->appendP(s, fb_esp_pgm_str_135, true);
        json.add(s.c_str(), js);
    }

    if (strlen(msg->payloads.notification.title) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_285);
        json.set(s.c_str(), msg->payloads.notification.title);
    }

    if (strlen(msg->payloads.notification.body) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_123);
        json.set(s.c_str(), msg->payloads.notification.body);
    }

    if (strlen(msg->payloads.notification.sound) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_126);
        json.set(s.c_str(), msg->payloads.notification.sound);
    }

    if (strlen(msg->payloads.notification.badge) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_286);
        json.set(s.c_str(), msg->payloads.notification.badge);
    }

    if (strlen(msg->payloads.notification.click_action) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_125);
        json.set(s.c_str(), msg->payloads.notification.click_action);
    }

    if (strlen(msg->payloads.notification.subtitle) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_287);
        json.set(s.c_str(), msg->payloads.notification.subtitle);
    }

    if (strlen(msg->payloads.notification.body_loc_key) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_288);
        json.set(s.c_str(), msg->payloads.notification.body_loc_key);
    }

    if (strlen(msg->payloads.notification.body_loc_args) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_289);
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->payloads.notification.body_loc_args);
        json.set(s.c_str(), arr);
    }

    if (strlen(msg->payloads.notification.title_loc_key) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_290);
        json.set(s.c_str(), msg->payloads.notification.title_loc_key);
    }

    if (strlen(msg->payloads.notification.title_loc_args) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_291);
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->payloads.notification.title_loc_args);
        json.set(s.c_str(), arr);
    }

    if (strlen(msg->payloads.notification.android_channel_id) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_292);
        json.set(s.c_str(), msg->payloads.notification.android_channel_id);
    }

    if (strlen(msg->payloads.notification.icon) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_124);
        json.set(s.c_str(), msg->payloads.notification.icon);
    }

    if (strlen(msg->payloads.notification.tag) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_293);
        json.set(s.c_str(), msg->payloads.notification.tag);
    }

    if (strlen(msg->payloads.notification.color) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_294);
        json.set(s.c_str(), msg->payloads.notification.color);
    }
    ut->clearS(s);
    raw = json.raw();
}

void FB_CM::fcm_preparSubscriptionPayload(const char *topic, const char *IID[], size_t numToken)
{
    std::string base, s;

    ut->clearS(raw);
    FirebaseJson json;

    ut->appendP(base, fb_esp_pgm_str_128, true);

    ut->appendP(s, fb_esp_pgm_str_134);
    s += topic;

    json.add(base.c_str(), s.c_str());

    static FirebaseJsonArray arr;
    arr.clear();
    for (size_t i = 0; i < numToken; i++)
    {
        if (IID[i])
        {
            s = ut->trim(IID[i]);
            if (s.length() > 0)
                arr.add(s.c_str());
        }
    }

    ut->appendP(base, fb_esp_pgm_str_334, true);

    json.add(base.c_str(), arr);

    ut->clearS(base);
    ut->clearS(s);
    raw = json.raw();
}

void FB_CM::fcm_preparAPNsRegistPayload(const char *application, bool sandbox, const char *APNs[], size_t numToken)
{
    std::string base, s;

    ut->clearS(raw);
    FirebaseJson json;

    ut->appendP(base, fb_esp_pgm_str_337, true);

    json.add(base.c_str(), application);

    ut->appendP(base, fb_esp_pgm_str_338, true);
    json.add(base.c_str(), sandbox);

    FirebaseJsonArray arr;
    for (size_t i = 0; i < numToken; i++)
    {
        if (APNs[i])
        {
            s = ut->trim(APNs[i]);
            if (s.length() > 0)
                arr.add(s.c_str());
        }
    }

    ut->appendP(base, fb_esp_pgm_str_339, true);
    json.add(base.c_str(), arr);

    ut->clearS(base);
    ut->clearS(s);
    raw = json.raw();
}

void FB_CM::fcm_prepareV1Payload(FCM_HTTPv1_JSON_Message *msg)
{

    std::string base, _base, s;

    FirebaseJson json;
    ut->clearS(raw);

    ut->appendP(base, fb_esp_pgm_str_295);
    ut->appendP(base, fb_esp_pgm_str_1);
    _base = base;

    if (strlen(msg->token) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_233);
        json.set(s.c_str(), msg->token);
    }
    else if (strlen(msg->topic) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_296);
        json.set(s.c_str(), msg->topic);
    }
    else if (strlen(msg->condition) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_282);
        json.set(s.c_str(), msg->condition);
    }

    if (strlen(msg->data) > 0)
    {
        FirebaseJson js;
        js.setJsonData(msg->data);
        s = base;
        ut->appendP(s, fb_esp_pgm_str_135);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->notification.title) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_285);
        json.set(s.c_str(), msg->notification.title);
    }

    if (strlen(msg->notification.body) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_123);
        json.set(s.c_str(), msg->notification.body);
    }

    if (strlen(msg->notification.image) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_297);
        json.set(s.c_str(), msg->notification.image);
    }

    if (strlen(msg->fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        json.set(s.c_str(), msg->fcm_options.analytics_label);
    }

    ////// AndroidConfig

    if (strlen(msg->android.collapse_key) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_138);
        json.set(s.c_str(), msg->android.collapse_key);
    }

    if (strlen(msg->android.priority) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_136);
        json.set(s.c_str(), msg->android.priority);
    }

    if (strlen(msg->android.ttl) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_303);
        json.set(s.c_str(), msg->android.ttl);
    }

    if (strlen(msg->android.restricted_package_name) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_147);
        json.set(s.c_str(), msg->android.restricted_package_name);
    }

    if (strlen(msg->android.data) > 0)
    {
        FirebaseJson js;
        js.setJsonData(msg->android.data);
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_135);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->android.fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        json.set(s.c_str(), msg->android.fcm_options.analytics_label);
    }

    if (strlen(msg->android.direct_boot_ok) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_323);
        json.set(s.c_str(), msg->android.direct_boot_ok);
    }

    if (strlen(msg->android.notification.title) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_285);
        json.set(s.c_str(), msg->android.notification.title);
    }
    if (strlen(msg->android.notification.body) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_123);
        json.set(s.c_str(), msg->android.notification.body);
    }

    if (strlen(msg->android.notification.icon) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_124);
        json.set(s.c_str(), msg->android.notification.icon);
    }

    if (strlen(msg->android.notification.color) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_294);
        json.set(s.c_str(), msg->android.notification.color);
    }

    if (strlen(msg->android.notification.sound) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_126);
        json.set(s.c_str(), msg->android.notification.sound);
    }
    if (strlen(msg->android.notification.tag) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_293);
        json.set(s.c_str(), msg->android.notification.tag);
    }
    if (strlen(msg->android.notification.click_action) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_125);
        json.set(s.c_str(), msg->android.notification.click_action);
    }

    if (strlen(msg->android.notification.body_loc_key) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_288);
        json.set(s.c_str(), msg->android.notification.body_loc_key);
    }

    if (strlen(msg->android.notification.body_loc_args) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_289);
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->android.notification.body_loc_args);
        json.set(s.c_str(), arr);
    }

    if (strlen(msg->android.notification.title_loc_key) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_290);
        json.set(s.c_str(), msg->android.notification.title_loc_key);
    }

    if (strlen(msg->android.notification.title_loc_args) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_291);
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->android.notification.title_loc_args);
        json.set(s.c_str(), arr);
    }

    if (strlen(msg->android.notification.channel_id) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_304);
        json.set(s.c_str(), msg->android.notification.channel_id);
    }
    if (strlen(msg->android.notification.ticker) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_305);
        json.set(s.c_str(), msg->android.notification.ticker);
    }
    if (strlen(msg->android.notification.sticky) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_306);
        json.set(s.c_str(), ut->boolVal(msg->android.notification.sticky));
    }
    if (strlen(msg->android.notification.event_time) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_307);
        json.set(s.c_str(), msg->android.notification.event_time);
    }
    if (strlen(msg->android.notification.local_only) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_308);
        json.set(s.c_str(), ut->boolVal(msg->android.notification.local_only));
    }
    if (strlen(msg->android.notification.notification_priority) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_309);
        json.set(s.c_str(), msg->android.notification.notification_priority);
    }
    if (strlen(msg->android.notification.default_sound) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_310);
        json.set(s.c_str(), ut->boolVal(msg->android.notification.default_sound));
    }
    if (strlen(msg->android.notification.default_vibrate_timings) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_311);
        json.set(s.c_str(), ut->boolVal(msg->android.notification.default_vibrate_timings));
    }
    if (strlen(msg->android.notification.default_light_settings) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_312);
        json.set(s.c_str(), ut->boolVal(msg->android.notification.default_light_settings));
    }
    if (strlen(msg->android.notification.vibrate_timings) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_313);
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->android.notification.vibrate_timings);
        json.set(s.c_str(), arr);
    }

    if (strlen(msg->android.notification.visibility) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_314);
        json.set(s.c_str(), msg->android.notification.visibility);
    }
    if (strlen(msg->android.notification.notification_count) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_315);
        json.set(s.c_str(), atoi(msg->android.notification.notification_count));
    }

    if (strlen(msg->android.notification.image) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_297);
        json.set(s.c_str(), msg->android.notification.image);
    }

    s = base;
    ut->appendP(s, fb_esp_pgm_str_300);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_122);
    ut->appendP(s, fb_esp_pgm_str_1);
    ut->appendP(s, fb_esp_pgm_str_316);
    ut->appendP(s, fb_esp_pgm_str_1);
    base = s;

    if (strlen(msg->android.notification.light_settings.color.red) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_317);
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.red));
    }
    if (strlen(msg->android.notification.light_settings.color.green) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_318);
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.green));
    }
    if (strlen(msg->android.notification.light_settings.color.blue) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_319);
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.blue));
    }
    if (strlen(msg->android.notification.light_settings.color.alpha) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_320);
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.alpha));
    }
    if (strlen(msg->android.notification.light_settings.light_on_duration) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_321);
        json.set(s.c_str(), msg->android.notification.light_settings.light_on_duration);
    }
    if (strlen(msg->android.notification.light_settings.light_off_duration) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_322);
        json.set(s.c_str(), msg->android.notification.light_settings.light_off_duration);
    }

    ////// WebpushConfig

    FirebaseJson js;
    base = _base;

    if (strlen(msg->webpush.headers) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_324);
        js.setJsonData(msg->webpush.headers);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->webpush.data) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_135);
        js.setJsonData(msg->webpush.data);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->webpush.notification) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        js.setJsonData(msg->webpush.notification);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->webpush.fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        json.set(s.c_str(), msg->webpush.fcm_options.analytics_label);
    }

    if (strlen(msg->webpush.fcm_options.link) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_325);
        json.set(s.c_str(), msg->webpush.fcm_options.link);
    }

    ////// ApnsConfig

    if (strlen(msg->apns.headers) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_324);
        js.setJsonData(msg->apns.headers);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->apns.payload) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_135);
        js.setJsonData(msg->apns.payload);
        json.set(s.c_str(), js);
    }

    if (strlen(msg->apns.fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        json.set(s.c_str(), msg->apns.fcm_options.analytics_label);
    }

    if (strlen(msg->apns.fcm_options.image) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_297);
        json.set(s.c_str(), msg->apns.fcm_options.image);
    }

    ut->clearS(base);
    ut->clearS(_base);
    ut->clearS(s);
    raw = json.raw();
}

bool FB_CM::fcm_send(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, const char *msg)
{

    if (Signer.getCfg())
    {
        if (Signer.getTokenType() != token_type_undefined)
            if (!Signer.tokenReady())
            {
                Signer.getCfg()->_int.fb_processing = false;
                return false;
            }
    }

    int ret = fcm_sendHeader(fbdo, mode, msg);
    if (ret == 0)
        ret = fbdo->tcpClient.send(msg);

    ut->clearS(fbdo->_ss.fcm.payload);
    if (ret != 0)
    {
        fbdo->closeSession();
        Signer.getCfg()->_int.fb_processing = false;
        return false;
    }
    else
        fbdo->_ss.connected = true;

    ret = waitResponse(fbdo);

    if (Signer.getCfg())
        Signer.getCfg()->_int.fb_processing = false;

    bool msgMode = (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1);

    if (!ret || !msgMode)
        fbdo->closeSession();

    return ret;
}

bool FB_CM::waitResponse(FirebaseData *fbdo)
{
    return handleResponse(fbdo);
}

bool FB_CM::handleResponse(FirebaseData *fbdo)
{

#ifdef ENABLE_RTDB
    if (fbdo->_ss.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect())
        return false;

    if (!fbdo->_ss.connected)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
        return false;
    }

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->tcpClient.stream();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    char *header = nullptr;
    char *payload = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->_ss.resp_size;
    int pBufPos = 0;
    int hBufPos = 0;
    int chunkBufSize = stream->available();
    int hstate = 0;
    int pstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->_ss.resp_size;
    int payloadRead = 0;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;

    fbdo->_ss.http_code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->_ss.content_length = -1;
    fbdo->_ss.payload_length = 0;
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 768;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        ut->idle();
    }

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0)
        {
            if (!fbdo->reconnect())
                return false;

            chunkBufSize = stream->available();

            if (chunkBufSize <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    //the first chunk can be http response header
                    header = ut->newS(chunkBufSize);
                    hstate = 1;
                    int readLen = ut->readLine(stream, header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    ut->idle();
                    dataTime = millis();
                    if (tmp)
                    {
                        //http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        fbdo->_ss.http_code = response.httpCode;
                        ut->delS(tmp);
                    }
                    else
                    {
                        payload = ut->newS(payloadLen);
                        pstate = 1;
                        memcpy(payload, header, readLen);
                        pBufPos = readLen;
                        ut->delS(header);
                        hstate = 0;
                    }
                }
                else
                {
                    ut->idle();
                    dataTime = millis();
                    //the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        //read one line of next header field until the empty header has found
                        tmp = ut->newS(chunkBufSize);
                        int readLen = ut->readLine(stream, tmp, chunkBufSize);
                        bool headerEnded = false;

                        //check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (tmp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (tmp[0] == '\r' && tmp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {

                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

                            //parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            if (hstate == 1)
                                ut->delS(header);
                            hstate = 0;

                            fbdo->_ss.chunked_encoding = response.isChunkedEnc;
                        }
                        else
                        {
                            //accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }

                        ut->delS(tmp);
                    }
                    else
                    {
                        //the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;

                            pChunk = ut->newS(chunkBufSize + 1);

                            if (!payload || pstate == 0)
                            {
                                pstate = 1;
                                payload = ut->newS(payloadLen + 1);
                            }

                            //read the avilable data
                            int readLen = 0;

                            //chunk transfer encoding?
                            if (response.isChunkedEnc)
                                readLen = ut->readChunkedData(stream, pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                            else
                            {
                                if (stream->available() < chunkBufSize)
                                    chunkBufSize = stream->available();
                                readLen = stream->readBytes(pChunk, chunkBufSize);
                            }

                            if (readLen > 0)
                            {
                                fbdo->_ss.payload_length += readLen;
                                if (fbdo->_ss.max_payload_length < fbdo->_ss.payload_length)
                                    fbdo->_ss.max_payload_length = fbdo->_ss.payload_length;
                                payloadRead += readLen;
                                fbdo->checkOvf(pBufPos + readLen, response);

                                if (!fbdo->_ss.buffer_ovf)
                                {
                                    if (pBufPos + readLen <= payloadLen)
                                        memcpy(payload + pBufPos, pChunk, readLen);
                                    else
                                    {
                                        //in case of the accumulated payload size is bigger than the char array
                                        //reallocate the char array

                                        char *buf = ut->newS(pBufPos + readLen + 1);
                                        memcpy(buf, payload, pBufPos);

                                        memcpy(buf + pBufPos, pChunk, readLen);

                                        payloadLen = pBufPos + readLen;
                                        ut->delS(payload);
                                        payload = ut->newS(payloadLen + 1);
                                        memcpy(payload, buf, payloadLen);
                                        ut->delS(buf);
                                    }
                                }
                            }

                            ut->delS(pChunk);
                            if (readLen < 0 && payloadRead >= response.contentLen)
                                break;
                            if (readLen > 0)
                                pBufPos += readLen;
                        }
                        else
                        {
                            //read all the rest data
                            while (stream->available() > 0)
                                stream->read();
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delS(header);

        if (payload)
        {
            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK)
                fbdo->_ss.fcm.payload = payload;
            else
            {
                std::string t = ut->trim(payload);
                if (t[0] == '{' && t[t.length() - 1] == '}')
                {
                    FirebaseJson json;
                    FirebaseJsonData data;
                    json.setJsonData(t.c_str());

                    char *tmp = ut->strP(fb_esp_pgm_str_257);
                    json.get(data, tmp);
                    ut->delS(tmp);

                    if (data.success)
                    {
                        error.code = data.to<int>();
                        tmp = ut->strP(fb_esp_pgm_str_258);
                        json.get(data, tmp);
                        ut->delS(tmp);
                        if (data.success)
                            fbdo->_ss.error = data.to<const char *>();
                    }
                    else
                        error.code = 0;
                }
            }
        }

        if (pstate == 1)
            ut->delS(payload);

        return error.code == 0 || response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK;
    }
    else
    {
        while (stream->available() > 0)
            stream->read();
    }

    return false;
}

void FB_CM::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->_ss.cert_updated || !fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.con_mode != fb_esp_con_mode_fcm || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();

        if (Signer.getCfg())
        {
            if (_spi_ethernet_module)
                fbdo->ethDNSWorkAround(_spi_ethernet_module, host, 443);
            else
                fbdo->ethDNSWorkAround(&Signer.getCfg()->spi_ethernet_module, host, 443);
        }
        else
            fbdo->ethDNSWorkAround(_spi_ethernet_module, host, 443);
    }

    fbdo->_ss.host = host;
    fbdo->_ss.con_mode = fb_esp_con_mode_fcm;
}

bool FB_CM::handleFCMRequest(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, const char *payload)
{
    fbdo->_spi_ethernet_module = _spi_ethernet_module;

    if (!fbdo->reconnect())
        return false;

    if (!ut->waitIdle(fbdo->_ss.http_code))
        return false;

#ifdef ENABLE_RTDB
    if (fbdo->_ss.rtdb.pause)
        return true;
#endif
    if (fbdo->_ss.long_running_task > 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.getCfg())
    {
        if (Signer.getCfg()->_int.fb_processing)
            return false;

        Signer.getCfg()->_int.fb_processing = true;
    }

    fcm_connect(fbdo, mode);

    fbdo->_ss.con_mode = fb_esp_con_mode_fcm;

    return fcm_send(fbdo, mode, payload);
}

void FB_CM::clear()
{
    ut->clearS(raw);
    ut->clearS(server_key);
    if (ut)
        delete ut;
    if (cfg)
        delete cfg;
    if (auth)
        delete auth;
}

#endif

#endif //ENABLE