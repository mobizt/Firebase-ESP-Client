/**
 * Google's Firebase Cloud Messaging class, FCM.cpp version 1.0.4
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created March 25, 2021
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
    if (!Signer.getCfg())
        return false;

    if (clearInt)
    {
        if (_ut)
            delete _ut;
        if (_cfg)
            delete _cfg;
        if (_auth)
            delete _auth;

        _ut = nullptr;
        _cfg = nullptr;
        _auth = nullptr;
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

void FB_CM::setServerKey(const char *serverKey)
{
    _server_key = serverKey;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_Legacy_HTTP_Message *msg)
{
    if (!Signer.getCfg())
    {
        _cfg = new FirebaseConfig();
        _auth = new FirebaseAuth();
        _ut = new UtilsClass(_cfg);
        ut = _ut;
        Signer.begin(_ut, _cfg, _auth);
    }

    if (!init())
        return false;

    if (_server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    std::string payload;

    fcm_prepareLegacyPayload(payload, msg);

    return handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_legacy_http, payload);
}

bool FB_CM::send(FirebaseData *fbdo, FCM_HTTPv1_JSON_Message *msg)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (Signer.getTokenType() != token_type_oauth2_access_token)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_FCM_OAUTH2_REQUIRED;
        return false;
    }

    std::string payload;
    fcm_prepareV1Payload(payload, msg);
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_httpv1, payload);
}

bool FB_CM::subscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], size_t numToken)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (_server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    std::string payload;
    fcm_preparSubscriptionPayload(payload, topic, IID, numToken);
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_subscribe, payload);
}

bool FB_CM::unsubscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], size_t numToken)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (_server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }
    std::string payload;
    fcm_preparSubscriptionPayload(payload, topic, IID, numToken);
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_unsubscribe, payload);
}

bool FB_CM::appInstanceInfo(FirebaseData *fbdo, const char *IID)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (_server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }
    std::string payload = IID;
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_app_instance_info, payload);
}

bool FB_CM::regisAPNsTokens(FirebaseData *fbdo, const char *application, bool sandbox, const char *APNs[], size_t numToken)
{
    if (!init())
        return false;

    Signer.tokenReady();

    if (_server_key.length() == 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }
    std::string payload;
    fcm_preparAPNsRegistPayload(payload, application, sandbox, APNs, numToken);
    return handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_apn_token_registration, payload);
}

String FB_CM::payload(FirebaseData *fbdo)
{
    return fbdo->_ss.fcm.payload.c_str();
}

void FB_CM::fcm_connect(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode)
{

    if (Signer.getTokenType() != token_type_undefined)
    {
        if (!Signer.tokenReady())
            return;
    }

    std::string host;
    if (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1)
        ut->appendP(host, fb_esp_pgm_str_249);
    else
        ut->appendP(host, fb_esp_pgm_str_329);
    ut->appendP(host, fb_esp_pgm_str_4);
    ut->appendP(host, fb_esp_pgm_str_120);

    rescon(fbdo, host.c_str());
    fbdo->httpClient.begin(host.c_str(), _port);
}

void FB_CM::fcm_prepareHeader(std::string &header, fb_esp_fcm_msg_mode mode, std::string &payload)
{
    bool msgMode = (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1);

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
        header += Signer.getToken(Signer.getTokenType());
    }
    else
    {
        ut->appendP(header, fb_esp_pgm_str_131);
        header += _server_key;
    }

    ut->appendP(header, fb_esp_pgm_str_21);

    ut->appendP(header, fb_esp_pgm_str_32);

    if (mode != fb_esp_fcm_msg_mode_app_instance_info)
    {
        ut->appendP(header, fb_esp_pgm_str_8);
        ut->appendP(header, fb_esp_pgm_str_129);
        ut->appendP(header, fb_esp_pgm_str_21);

        ut->appendP(header, fb_esp_pgm_str_12);
        char *tmp = ut->intStr(payload.length());
        header += tmp;
        ut->delS(tmp);
        ut->appendP(header, fb_esp_pgm_str_21);
    }

    ut->appendP(header, fb_esp_pgm_str_36);
    ut->appendP(header, fb_esp_pgm_str_21);
}

void FB_CM::fcm_prepareLegacyPayload(std::string &buf, FCM_Legacy_HTTP_Message *msg)
{
    _fcmPayload.clear();

    char *tmp = nullptr;

    if (strlen(msg->targets.to) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_128);
        _fcmPayload.add(tmp, msg->targets.to);
        ut->delS(tmp);
    }

    if (strlen(msg->targets.registration_ids) > 0)
    {
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->targets.registration_ids);
        tmp = ut->strP(fb_esp_pgm_str_130);
        _fcmPayload.add(tmp, arr);
        ut->delS(tmp);
    }

    if (strlen(msg->targets.condition) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_282);
        _fcmPayload.add(tmp, msg->targets.condition);
        ut->delS(tmp);
    }

    if (strlen(msg->options.collapse_key) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_138);
        _fcmPayload.add(tmp, msg->options.collapse_key);
        ut->delS(tmp);
    }

    if (strlen(msg->options.priority) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_136);
        _fcmPayload.add(tmp, msg->options.priority);
        ut->delS(tmp);
    }

    if (strlen(msg->options.content_available) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_283);
        _fcmPayload.add(tmp, ut->boolVal(msg->options.content_available));
        ut->delS(tmp);
    }

    if (strlen(msg->options.mutable_content) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_284);
        _fcmPayload.add(tmp, ut->boolVal(msg->options.mutable_content));
        ut->delS(tmp);
    }

    if (strlen(msg->options.time_to_live) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_137);
        _fcmPayload.add(tmp, atoi(msg->options.time_to_live));
        ut->delS(tmp);
    }

    if (strlen(msg->options.restricted_package_name) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_147);
        _fcmPayload.add(tmp, msg->options.restricted_package_name);
        ut->delS(tmp);
    }

    if (strlen(msg->options.dry_run) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_281);
        _fcmPayload.add(tmp, msg->options.dry_run);
        ut->delS(tmp);
    }

    if (strlen(msg->options.direct_boot_ok) > 0)
    {
        tmp = ut->strP(fb_esp_pgm_str_323);
        _fcmPayload.add(tmp, ut->boolVal(msg->options.direct_boot_ok));
        ut->delS(tmp);
    }

    _fcmPayload.int_tostr(buf);

    std::string s;

    if (strlen(msg->payloads.data) > 0)
    {
        static FirebaseJson js;
        js.clear();
        js.setJsonData(msg->payloads.data);
        tmp = ut->strP(fb_esp_pgm_str_135);
        _fcmPayload.add(tmp, js);
        ut->delS(tmp);
    }

    if (strlen(msg->payloads.notification.title) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_285);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.title);
    }

    if (strlen(msg->payloads.notification.body) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_123);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.body);
    }

    if (strlen(msg->payloads.notification.sound) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_126);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.sound);
    }

    if (strlen(msg->payloads.notification.badge) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_286);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.badge);
    }

    if (strlen(msg->payloads.notification.click_action) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_125);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.click_action);
    }

    if (strlen(msg->payloads.notification.subtitle) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_287);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.subtitle);
    }

    if (strlen(msg->payloads.notification.body_loc_key) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_288);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.body_loc_key);
    }

    if (strlen(msg->payloads.notification.body_loc_args) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_289);
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->payloads.notification.body_loc_args);
        _fcmPayload.add(s.c_str(), arr);
    }

    if (strlen(msg->payloads.notification.title_loc_key) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_290);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.title_loc_key);
    }

    if (strlen(msg->payloads.notification.title_loc_args) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_291);
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->payloads.notification.title_loc_args);
        _fcmPayload.add(s.c_str(), arr);
    }

    if (strlen(msg->payloads.notification.android_channel_id) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_292);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.android_channel_id);
    }

    if (strlen(msg->payloads.notification.icon) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_124);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.icon);
    }

    if (strlen(msg->payloads.notification.tag) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_293);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.tag);
    }

    if (strlen(msg->payloads.notification.color) > 0)
    {
        ut->appendP(s, fb_esp_pgm_str_122, true);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_294);
        _fcmPayload.add(s.c_str(), msg->payloads.notification.color);
    }

    _fcmPayload.int_tostr(buf);

    _fcmPayload.clear();
}

void FB_CM::fcm_preparSubscriptionPayload(std::string &buf, const char *topic, const char *IID[], size_t numToken)
{
    _fcmPayload.clear();
    std::string s;
    ut->appendP(s, fb_esp_pgm_str_134);
    s += topic;
    char *tmp = ut->strP(fb_esp_pgm_str_128);
    _fcmPayload.add(tmp, s.c_str());
    ut->delS(tmp);

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

    tmp = ut->strP(fb_esp_pgm_str_334);
    _fcmPayload.add(tmp, arr);
    ut->delS(tmp);

    _fcmPayload.int_tostr(buf);
    _fcmPayload.clear();
}

void FB_CM::fcm_preparAPNsRegistPayload(std::string &buf, const char *application, bool sandbox, const char *APNs[], size_t numToken)
{
    _fcmPayload.clear();
    std::string s;
    char *tmp = ut->strP(fb_esp_pgm_str_337);
    _fcmPayload.add(tmp, application);
    ut->delS(tmp);

    tmp = ut->strP(fb_esp_pgm_str_338);
    _fcmPayload.add(tmp, sandbox);
    ut->delS(tmp);

    static FirebaseJsonArray arr;
    arr.clear();
    for (size_t i = 0; i < numToken; i++)
    {
        if (APNs[i])
        {
            s = ut->trim(APNs[i]);
            if (s.length() > 0)
                arr.add(s.c_str());
        }
    }

    tmp = ut->strP(fb_esp_pgm_str_339);
    _fcmPayload.add(tmp, arr);
    ut->delS(tmp);

    _fcmPayload.int_tostr(buf);
    _fcmPayload.clear();
}

void FB_CM::fcm_prepareV1Payload(std::string &buf, FCM_HTTPv1_JSON_Message *msg)
{
    _fcmPayload.clear();
    std::string s;
    std::string base;
    std::string _base;
    ut->appendP(base, fb_esp_pgm_str_295);
    ut->appendP(base, fb_esp_pgm_str_1);
    _base = base;

    if (strlen(msg->token) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_233);
        _fcmPayload.set(s.c_str(), msg->token);
    }
    else if (strlen(msg->topic) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_296);
        _fcmPayload.set(s.c_str(), msg->topic);
    }
    else if (strlen(msg->condition) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_282);
        _fcmPayload.set(s.c_str(), msg->condition);
    }

    if (strlen(msg->data) > 0)
    {
        static FirebaseJson js;
        js.clear();
        js.setJsonData(msg->data);
        s = base;
        ut->appendP(s, fb_esp_pgm_str_135);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->notification.title) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_285);
        _fcmPayload.set(s.c_str(), msg->notification.title);
    }

    if (strlen(msg->notification.body) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_123);
        _fcmPayload.set(s.c_str(), msg->notification.body);
    }

    if (strlen(msg->notification.image) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_297);
        _fcmPayload.set(s.c_str(), msg->notification.image);
    }

    if (strlen(msg->fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        _fcmPayload.set(s.c_str(), msg->fcm_options.analytics_label);
    }

    ////// AndroidConfig

    if (strlen(msg->android.collapse_key) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_138);
        _fcmPayload.set(s.c_str(), msg->android.collapse_key);
    }

    if (strlen(msg->android.priority) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_136);
        _fcmPayload.set(s.c_str(), msg->android.priority);
    }

    if (strlen(msg->android.ttl) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_303);
        _fcmPayload.set(s.c_str(), msg->android.ttl);
    }

    if (strlen(msg->android.restricted_package_name) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_147);
        _fcmPayload.set(s.c_str(), msg->android.restricted_package_name);
    }

    if (strlen(msg->android.data) > 0)
    {
        static FirebaseJson js;
        js.clear();
        js.setJsonData(msg->android.data);
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_135);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->android.fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        _fcmPayload.set(s.c_str(), msg->android.fcm_options.analytics_label);
    }

    if (strlen(msg->android.direct_boot_ok) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_323);
        _fcmPayload.set(s.c_str(), msg->android.direct_boot_ok);
    }

    if (strlen(msg->android.notification.title) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_285);
        _fcmPayload.set(s.c_str(), msg->android.notification.title);
    }
    if (strlen(msg->android.notification.body) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_123);
        _fcmPayload.set(s.c_str(), msg->android.notification.body);
    }

    if (strlen(msg->android.notification.icon) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_124);
        _fcmPayload.set(s.c_str(), msg->android.notification.icon);
    }

    if (strlen(msg->android.notification.color) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_294);
        _fcmPayload.set(s.c_str(), msg->android.notification.color);
    }

    if (strlen(msg->android.notification.sound) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_126);
        _fcmPayload.set(s.c_str(), msg->android.notification.sound);
    }
    if (strlen(msg->android.notification.tag) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_293);
        _fcmPayload.set(s.c_str(), msg->android.notification.tag);
    }
    if (strlen(msg->android.notification.click_action) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_125);
        _fcmPayload.set(s.c_str(), msg->android.notification.click_action);
    }

    if (strlen(msg->android.notification.body_loc_key) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_288);
        _fcmPayload.set(s.c_str(), msg->android.notification.body_loc_key);
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
        _fcmPayload.set(s.c_str(), arr);
    }

    if (strlen(msg->android.notification.title_loc_key) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_290);
        _fcmPayload.set(s.c_str(), msg->android.notification.title_loc_key);
    }

    if (strlen(msg->android.notification.title_loc_args) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_291);
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->android.notification.title_loc_args);
        _fcmPayload.set(s.c_str(), arr);
    }

    if (strlen(msg->android.notification.channel_id) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_304);
        _fcmPayload.set(s.c_str(), msg->android.notification.channel_id);
    }
    if (strlen(msg->android.notification.ticker) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_305);
        _fcmPayload.set(s.c_str(), msg->android.notification.ticker);
    }
    if (strlen(msg->android.notification.sticky) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_306);
        _fcmPayload.set(s.c_str(), ut->boolVal(msg->android.notification.sticky));
    }
    if (strlen(msg->android.notification.event_time) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_307);
        _fcmPayload.set(s.c_str(), msg->android.notification.event_time);
    }
    if (strlen(msg->android.notification.local_only) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_308);
        _fcmPayload.set(s.c_str(), ut->boolVal(msg->android.notification.local_only));
    }
    if (strlen(msg->android.notification.notification_priority) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_309);
        _fcmPayload.set(s.c_str(), msg->android.notification.notification_priority);
    }
    if (strlen(msg->android.notification.default_sound) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_310);
        _fcmPayload.set(s.c_str(), ut->boolVal(msg->android.notification.default_sound));
    }
    if (strlen(msg->android.notification.default_vibrate_timings) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_311);
        _fcmPayload.set(s.c_str(), ut->boolVal(msg->android.notification.default_vibrate_timings));
    }
    if (strlen(msg->android.notification.default_light_settings) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_312);
        _fcmPayload.set(s.c_str(), ut->boolVal(msg->android.notification.default_light_settings));
    }
    if (strlen(msg->android.notification.vibrate_timings) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_313);
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->android.notification.vibrate_timings);
        _fcmPayload.set(s.c_str(), arr);
    }

    if (strlen(msg->android.notification.visibility) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_314);
        _fcmPayload.set(s.c_str(), msg->android.notification.visibility);
    }
    if (strlen(msg->android.notification.notification_count) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_315);
        _fcmPayload.set(s.c_str(), atoi(msg->android.notification.notification_count));
    }

    if (strlen(msg->android.notification.image) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_300);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_297);
        _fcmPayload.set(s.c_str(), msg->android.notification.image);
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
        _fcmPayload.set(s.c_str(), atoi(msg->android.notification.light_settings.color.red));
    }
    if (strlen(msg->android.notification.light_settings.color.green) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_318);
        _fcmPayload.set(s.c_str(), atoi(msg->android.notification.light_settings.color.green));
    }
    if (strlen(msg->android.notification.light_settings.color.blue) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_319);
        _fcmPayload.set(s.c_str(), atoi(msg->android.notification.light_settings.color.blue));
    }
    if (strlen(msg->android.notification.light_settings.color.alpha) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_294);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_320);
        _fcmPayload.set(s.c_str(), atoi(msg->android.notification.light_settings.color.alpha));
    }
    if (strlen(msg->android.notification.light_settings.light_on_duration) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_321);
        _fcmPayload.set(s.c_str(), msg->android.notification.light_settings.light_on_duration);
    }
    if (strlen(msg->android.notification.light_settings.light_off_duration) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_322);
        _fcmPayload.set(s.c_str(), msg->android.notification.light_settings.light_off_duration);
    }

    ////// WebpushConfig

    static FirebaseJson js;
    js.clear();
    base = _base;

    if (strlen(msg->webpush.headers) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_324);
        js.clear();
        js.setJsonData(msg->webpush.headers);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->webpush.data) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_135);
        js.clear();
        js.setJsonData(msg->webpush.data);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->webpush.notification) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_122);
        js.clear();
        js.setJsonData(msg->webpush.notification);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->webpush.fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        _fcmPayload.set(s.c_str(), msg->webpush.fcm_options.analytics_label);
    }

    if (strlen(msg->webpush.fcm_options.link) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_301);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_325);
        _fcmPayload.set(s.c_str(), msg->webpush.fcm_options.link);
    }

    ////// ApnsConfig

    if (strlen(msg->apns.headers) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_324);
        js.clear();
        js.setJsonData(msg->apns.headers);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->apns.payload) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_135);
        js.clear();
        js.setJsonData(msg->apns.payload);
        _fcmPayload.set(s.c_str(), js);
    }

    if (strlen(msg->apns.fcm_options.analytics_label) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_299);
        _fcmPayload.set(s.c_str(), msg->apns.fcm_options.analytics_label);
    }

    if (strlen(msg->apns.fcm_options.image) > 0)
    {
        s = base;
        ut->appendP(s, fb_esp_pgm_str_302);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_298);
        ut->appendP(s, fb_esp_pgm_str_1);
        ut->appendP(s, fb_esp_pgm_str_297);
        _fcmPayload.set(s.c_str(), msg->apns.fcm_options.image);
    }

    _fcmPayload.int_tostr(buf);

    _fcmPayload.clear();
}

bool FB_CM::fcm_send(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, std::string &msg)
{

    if (Signer.getTokenType() != token_type_undefined)
        if (!Signer.tokenReady())
        {
            Signer.getCfg()->_int.fb_processing = false;
            return false;
        }

    std::string header = "";
    fcm_prepareHeader(header, mode, msg);
    int ret = fbdo->httpClient.send(header.c_str(), msg.c_str());
    std::string().swap(msg);
    std::string().swap(header);
    fbdo->_ss.fcm.payload.clear();
    if (ret != 0)
    {
        fbdo->closeSession();
        Signer.getCfg()->_int.fb_processing = false;
        return false;
    }
    else
        fbdo->_ss.connected = true;

    ret = waitResponse(fbdo);
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

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (!fbdo->reconnect())
        return false;

    if (!fbdo->_ss.connected)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_HTTPC_ERROR_NOT_CONNECTED;
        return false;
    }

    unsigned long dataTime = millis();

    WiFiClient *stream = fbdo->httpClient.stream();

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
    fbdo->_ss.chunked_encoding = false;
    fbdo->_ss.buffer_ovf = false;

    defaultChunkSize = 768;

    while (fbdo->httpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = stream->available();
        delay(0);
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
                    delay(0);
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
                    delay(0);
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
                    fbdo->_ss.json.setJsonData(t.c_str());

                    char *tmp = ut->strP(fb_esp_pgm_str_257);
                    fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                    ut->delS(tmp);

                    if (fbdo->_ss.data.success)
                    {
                        error.code = fbdo->_ss.data.intValue;
                        tmp = ut->strP(fb_esp_pgm_str_258);
                        fbdo->_ss.json.get(fbdo->_ss.data, tmp);
                        ut->delS(tmp);
                        if (fbdo->_ss.data.success)
                            fbdo->_ss.error = fbdo->_ss.data.stringValue.c_str();
                    }
                    else
                        error.code = 0;
                }

                fbdo->_ss.json.clear();
                fbdo->_ss.arr.clear();
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
    if (!fbdo->_ss.connected || millis() - fbdo->_ss.last_conn_ms > fbdo->_ss.conn_timeout || fbdo->_ss.con_mode != fb_esp_con_mode_fcm || strcmp(host, fbdo->_ss.host.c_str()) != 0)
    {
        fbdo->_ss.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->_ss.host = host;
    fbdo->_ss.con_mode = fb_esp_con_mode_fcm;
}

bool FB_CM::handleFCMRequest(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, std::string &payload)
{

    if (!fbdo->reconnect())
        return false;

    if (!ut->waitIdle(fbdo->_ss.http_code))
        return false;

    if (fbdo->_ss.rtdb.pause)
        return true;

    if (mode == fb_esp_fcm_msg_mode_httpv1)
    {
        if (Signer.config->host.length() == 0)
        {
            fbdo->_ss.http_code = FIREBASE_ERROR_UNINITIALIZED;
            return false;
        }
    }

    if (fbdo->_ss.long_running_task > 0)
    {
        fbdo->_ss.http_code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.getCfg()->_int.fb_processing)
        return false;

    Signer.getCfg()->_int.fb_processing = true;

    fcm_connect(fbdo, mode);

    fbdo->_ss.con_mode = fb_esp_con_mode_fcm;

    return fcm_send(fbdo, mode, payload);
}

void FB_CM::clear()
{
    _fcmPayload.clear();
    std::string().swap(_topic);
    std::string().swap(_server_key);
    _ttl = -1;
    _index = 0;
    _tokens.clear();
    std::vector<std::string>().swap(_tokens);
    if (_ut)
        delete _ut;
    if (_cfg)
        delete _cfg;
    if (_auth)
        delete _auth;
}

#endif