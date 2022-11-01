/**
 * Google's Firebase Cloud Messaging class, FCM.cpp version 1.0.22
 *
 * This library supports Espressif ESP8266 and ESP32
 *
 * Created November 1, 2022
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

bool FB_CM::prepareUtil()
{
    if (!ut)
        ut = Signer.getUtils(); // must be initialized in Firebase class constructor

    if (!ut)
    {
        intUt = true;
        ut = new UtilsClass(Signer.getMBFS());
        ut->setConfig(Signer.getCfg());
    }

    return ut != nullptr;
}

void FB_CM::begin(UtilsClass *u)
{
    ut = u;
}

void FB_CM::mSetServerKey(MB_StringPtr serverKey, SPI_ETH_Module *spi_ethernet_module)
{
    this->server_key = serverKey;
    this->_spi_ethernet_module = spi_ethernet_module;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_Legacy_HTTP_Message *msg)
{
    if (fbdo->tcpClient.reserved)
        return false;

    if (!prepareUtil())
        return false;

    if (server_key.length() == 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    fcm_prepareLegacyPayload(msg);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_legacy_http, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_HTTPv1_JSON_Message *msg)
{
    if (fbdo->tcpClient.reserved)
        return false;

    if (!prepareUtil())
        return false;

    Signer.tokenReady();

    if (Signer.getTokenType() != token_type_oauth2_access_token)
    {
        fbdo->session.response.code = FIREBASE_ERROR_OAUTH2_REQUIRED;
        return false;
    }

    fcm_prepareV1Payload(msg);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_httpv1, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::mSubscribeTopic(FirebaseData *fbdo, MB_StringPtr topic, const char *IID[], size_t numToken)
{
    if (fbdo->tcpClient.reserved)
        return false;

    if (!prepareUtil())
        return false;

    Signer.tokenReady();

    if (server_key.length() == 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    MB_String _topic = topic;

    fcm_preparSubscriptionPayload(_topic.c_str(), IID, numToken);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_subscribe, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::mUnsubscribeTopic(FirebaseData *fbdo, MB_StringPtr topic, const char *IID[], size_t numToken)
{
    if (fbdo->tcpClient.reserved)
        return false;

    if (!prepareUtil())
        return false;

    Signer.tokenReady();

    if (server_key.length() == 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    MB_String _topic = topic;

    fcm_preparSubscriptionPayload(_topic.c_str(), IID, numToken);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_unsubscribe, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::mAppInstanceInfo(FirebaseData *fbdo, const char *IID)
{
    if (fbdo->tcpClient.reserved)
        return false;

    if (!prepareUtil())
        return false;

    if (server_key.length() == 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    MB_String payload = IID;
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_app_instance_info, payload.c_str());
    payload.clear();
    return ret;
}

bool FB_CM::mRegisAPNsTokens(FirebaseData *fbdo, MB_StringPtr application, bool sandbox, const char *APNs[], size_t numToken)
{
    if (fbdo->tcpClient.reserved)
        return false;

    if (!prepareUtil())
        return false;

    Signer.tokenReady();

    if (server_key.length() == 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
        return false;
    }

    MB_String _application = application;

    fcm_preparAPNsRegistPayload(_application.c_str(), sandbox, APNs, numToken);
    bool ret = handleFCMRequest(fbdo, fb_esp_fcm_msg_mode_apn_token_registration, raw.c_str());
    raw.clear();
    return ret;
}

String FB_CM::payload(FirebaseData *fbdo)
{
    return fbdo->session.fcm.payload.c_str();
}

void FB_CM::fcm_connect(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode)
{
    fbdo->tcpClient.setSPIEthernet(_spi_ethernet_module);

    if (Signer.getCfg())
    {
        if (Signer.getTokenType() != token_type_undefined)
        {
            if (!Signer.tokenReady())
                return;
        }
    }

    MB_String host;
    if (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1)
        host += fb_esp_pgm_str_249;
    else
        host += fb_esp_pgm_str_329;
    host += fb_esp_pgm_str_4;
    host += fb_esp_pgm_str_120;

    rescon(fbdo, host.c_str());
    fbdo->tcpClient.begin(host.c_str(), port, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
}

bool FB_CM::sendHeader(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, const char *payload)
{
    bool msgMode = (mode == fb_esp_fcm_msg_mode_legacy_http || mode == fb_esp_fcm_msg_mode_httpv1);

    MB_String header;

    if (mode == fb_esp_fcm_msg_mode_app_instance_info)
        header = fb_esp_pgm_str_25;
    else
        header = fb_esp_pgm_str_24;
    header += fb_esp_pgm_str_6;

    if (msgMode)
    {
        if (Signer.getTokenType() != token_type_oauth2_access_token || mode == fb_esp_fcm_msg_mode_legacy_http)
            header += fb_esp_pgm_str_121;
        else
        {
            header += fb_esp_pgm_str_326;
            header += Signer.getCfg()->service_account.data.project_id;
            header += fb_esp_pgm_str_327;
        }
    }
    else
    {
        if (mode == fb_esp_fcm_msg_mode_subscribe)
        {
            header += fb_esp_pgm_str_330;
            header += fb_esp_pgm_str_331;
        }
        else if (mode == fb_esp_fcm_msg_mode_unsubscribe)
        {
            header += fb_esp_pgm_str_330;
            header += fb_esp_pgm_str_332;
        }
        else if (mode == fb_esp_fcm_msg_mode_app_instance_info)
        {
            header += fb_esp_pgm_str_335;
            header += payload;
            header += fb_esp_pgm_str_336;
        }
        else if (mode == fb_esp_fcm_msg_mode_apn_token_registration)
        {
            header += fb_esp_pgm_str_330;
            header += fb_esp_pgm_str_333;
        }
    }

    header += fb_esp_pgm_str_30;

    header += fb_esp_pgm_str_31;
    if (msgMode)
        header += fb_esp_pgm_str_249;
    else
        header += fb_esp_pgm_str_329;
    header += fb_esp_pgm_str_4;
    header += fb_esp_pgm_str_120;
    header += fb_esp_pgm_str_21;

    if (Signer.getTokenType() == token_type_oauth2_access_token && mode == fb_esp_fcm_msg_mode_httpv1)
    {
        header += fb_esp_pgm_str_237;
        header += fb_esp_pgm_str_209;

        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpClient.send(Signer.getToken());

        if (fbdo->session.response.code < 0)
            return false;
    }
    else
    {
        header += fb_esp_pgm_str_131;

        fbdo->tcpClient.send(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpClient.send(server_key.c_str());

        if (fbdo->session.response.code < 0)
            return false;
    }

    header += fb_esp_pgm_str_21;

    header += fb_esp_pgm_str_32;

    if (mode != fb_esp_fcm_msg_mode_app_instance_info)
    {
        header += fb_esp_pgm_str_8;
        header += fb_esp_pgm_str_129;
        header += fb_esp_pgm_str_21;

        header += fb_esp_pgm_str_12;
        header += strlen(payload);
        header += fb_esp_pgm_str_21;
    }

    header += fb_esp_pgm_str_36;
    header += fb_esp_pgm_str_21;

    fbdo->tcpClient.send(header.c_str());
    header.clear();

    if (fbdo->session.response.code < 0)
        return false;

    return true;
}

void FB_CM::fcm_prepareLegacyPayload(FCM_Legacy_HTTP_Message *msg)
{

    MB_String s;

    raw.clear();

    FirebaseJson json;

    if (msg->targets.to.length() > 0)
    {
        s = fb_esp_pgm_str_128;
        json.add(s.c_str(), msg->targets.to);
    }

    if (msg->targets.registration_ids.length() > 0)
    {
        FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->targets.registration_ids);

        s = fb_esp_pgm_str_130;
        json.add(s.c_str(), arr);
    }

    if (msg->targets.condition.length() > 0)
    {
        s = fb_esp_pgm_str_282;
        json.add(s.c_str(), msg->targets.condition);
    }

    if (msg->options.collapse_key.length() > 0)
    {
        s = fb_esp_pgm_str_138;
        json.add(s.c_str(), msg->options.collapse_key);
    }

    if (msg->options.priority.length() > 0)
    {
        s = fb_esp_pgm_str_136;
        json.add(s.c_str(), msg->options.priority);
    }

    if (msg->options.content_available.length() > 0)
    {
        s = fb_esp_pgm_str_283;
        json.add(s.c_str(), ut->boolVal(msg->options.content_available.c_str()));
    }

    if (msg->options.mutable_content.length() > 0)
    {
        s = fb_esp_pgm_str_284;
        json.add(s.c_str(), ut->boolVal(msg->options.mutable_content.c_str()));
    }

    if (msg->options.time_to_live.length() > 0)
    {
        s = fb_esp_pgm_str_137;
        json.add(s.c_str(), atoi(msg->options.time_to_live.c_str()));
    }

    if (msg->options.restricted_package_name.length() > 0)
    {
        s = fb_esp_pgm_str_147;
        json.add(s.c_str(), msg->options.restricted_package_name);
    }

    if (msg->options.dry_run.length() > 0)
    {
        s = fb_esp_pgm_str_281;
        json.add(s.c_str(), msg->options.dry_run);
    }

    if (msg->options.direct_boot_ok.length() > 0)
    {
        s = fb_esp_pgm_str_323;
        json.add(s.c_str(), ut->boolVal(msg->options.direct_boot_ok.c_str()));
    }

    if (msg->payloads.data.length() > 0)
    {
        FirebaseJson js;
        js.setJsonData(msg->payloads.data);
        s = fb_esp_pgm_str_135;
        json.add(s.c_str(), js);
    }

    if (msg->payloads.notification.title.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_285;
        json.set(s.c_str(), msg->payloads.notification.title);
    }

    if (msg->payloads.notification.body.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_123;
        json.set(s.c_str(), msg->payloads.notification.body);
    }

    if (msg->payloads.notification.sound.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_126;
        json.set(s.c_str(), msg->payloads.notification.sound);
    }

    if (msg->payloads.notification.badge.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_286;
        json.set(s.c_str(), msg->payloads.notification.badge);
    }

    if (msg->payloads.notification.click_action.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_125;
        json.set(s.c_str(), msg->payloads.notification.click_action);
    }

    if (msg->payloads.notification.subtitle.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_287;
        json.set(s.c_str(), msg->payloads.notification.subtitle);
    }

    if (msg->payloads.notification.body_loc_key.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_288;
        json.set(s.c_str(), msg->payloads.notification.body_loc_key);
    }

    if (msg->payloads.notification.body_loc_args.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_289;
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->payloads.notification.body_loc_args);
        json.set(s.c_str(), arr);
    }

    if (msg->payloads.notification.title_loc_key.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_290;
        json.set(s.c_str(), msg->payloads.notification.title_loc_key);
    }

    if (msg->payloads.notification.title_loc_args.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_291;
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->payloads.notification.title_loc_args);
        json.set(s.c_str(), arr);
    }

    if (msg->payloads.notification.android_channel_id.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_292;
        json.set(s.c_str(), msg->payloads.notification.android_channel_id);
    }

    if (msg->payloads.notification.icon.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_124;
        json.set(s.c_str(), msg->payloads.notification.icon);
    }

    if (msg->payloads.notification.tag.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_293;
        json.set(s.c_str(), msg->payloads.notification.tag);
    }

    if (msg->payloads.notification.color.length() > 0)
    {
        s = fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_294;
        json.set(s.c_str(), msg->payloads.notification.color);
    }
    s.clear();
    raw = json.raw();
}

void FB_CM::fcm_preparSubscriptionPayload(const char *topic, const char *IID[], size_t numToken)
{
    MB_String base, s;

    raw.clear();
    FirebaseJson json;

    base = fb_esp_pgm_str_128;

    s += fb_esp_pgm_str_134;
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

    base = fb_esp_pgm_str_334;

    json.add(base.c_str(), arr);

    base.clear();
    s.clear();
    raw = json.raw();
}

void FB_CM::fcm_preparAPNsRegistPayload(const char *application, bool sandbox, const char *APNs[], size_t numToken)
{
    MB_String base, s;

    raw.clear();
    FirebaseJson json;

    base = fb_esp_pgm_str_337;

    json.add(base.c_str(), application);

    base = fb_esp_pgm_str_338;
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

    base = fb_esp_pgm_str_339;
    json.add(base.c_str(), arr);

    base.clear();
    s.clear();
    raw = json.raw();
}

void FB_CM::fcm_prepareV1Payload(FCM_HTTPv1_JSON_Message *msg)
{

    MB_String base, _base, s;

    FirebaseJson json;
    raw.clear();

    base = fb_esp_pgm_str_295;
    base += fb_esp_pgm_str_1;
    _base = base;

    if (msg->token.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_233;
        json.set(s.c_str(), msg->token);
    }
    else if (msg->topic.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_296;
        json.set(s.c_str(), msg->topic);
    }
    else if (msg->condition.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_282;
        json.set(s.c_str(), msg->condition);
    }

    if (msg->data.length() > 0)
    {
        FirebaseJson js;
        js.setJsonData(msg->data);
        s = base;
        s += fb_esp_pgm_str_135;
        json.set(s.c_str(), js);
    }

    if (msg->notification.title.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_285;
        json.set(s.c_str(), msg->notification.title);
    }

    if (msg->notification.body.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_123;
        json.set(s.c_str(), msg->notification.body);
    }

    if (msg->notification.image.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_297;
        json.set(s.c_str(), msg->notification.image);
    }

    if (msg->fcm_options.analytics_label.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_298;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_299;
        json.set(s.c_str(), msg->fcm_options.analytics_label);
    }

    ////// AndroidConfig

    if (msg->android.collapse_key.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_138;
        json.set(s.c_str(), msg->android.collapse_key);
    }

    if (msg->android.priority.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_136;
        json.set(s.c_str(), msg->android.priority);
    }

    if (msg->android.ttl.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_303;
        json.set(s.c_str(), msg->android.ttl);
    }

    if (msg->android.restricted_package_name.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_147;
        json.set(s.c_str(), msg->android.restricted_package_name);
    }

    if (msg->android.data.length() > 0)
    {
        FirebaseJson js;
        js.setJsonData(msg->android.data);
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_135;
        json.set(s.c_str(), js);
    }

    if (msg->android.fcm_options.analytics_label.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_298;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_299;
        json.set(s.c_str(), msg->android.fcm_options.analytics_label);
    }

    if (msg->android.direct_boot_ok.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_323;
        json.set(s.c_str(), msg->android.direct_boot_ok);
    }

    if (msg->android.notification.title.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_285;
        json.set(s.c_str(), msg->android.notification.title);
    }
    if (msg->android.notification.body.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_123;
        json.set(s.c_str(), msg->android.notification.body);
    }

    if (msg->android.notification.icon.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_124;
        json.set(s.c_str(), msg->android.notification.icon);
    }

    if (msg->android.notification.color.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_294;
        json.set(s.c_str(), msg->android.notification.color);
    }

    if (msg->android.notification.sound.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_126;
        json.set(s.c_str(), msg->android.notification.sound);
    }
    if (msg->android.notification.tag.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_293;
        json.set(s.c_str(), msg->android.notification.tag);
    }
    if (msg->android.notification.click_action.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_125;
        json.set(s.c_str(), msg->android.notification.click_action);
    }

    if (msg->android.notification.body_loc_key.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_288;
        json.set(s.c_str(), msg->android.notification.body_loc_key);
    }

    if (msg->android.notification.body_loc_args.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_289;
        static FirebaseJsonArray arr;
        arr.clear();
        arr.setJsonArrayData(msg->android.notification.body_loc_args);
        json.set(s.c_str(), arr);
    }

    if (msg->android.notification.title_loc_key.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_290;
        json.set(s.c_str(), msg->android.notification.title_loc_key);
    }

    if (msg->android.notification.title_loc_args.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_291;
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->android.notification.title_loc_args);
        json.set(s.c_str(), arr);
    }

    if (msg->android.notification.channel_id.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_304;
        json.set(s.c_str(), msg->android.notification.channel_id);
    }
    if (msg->android.notification.ticker.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_305;
        json.set(s.c_str(), msg->android.notification.ticker);
    }
    if (msg->android.notification.sticky.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_306;
        json.set(s.c_str(), ut->boolVal(msg->android.notification.sticky.c_str()));
    }
    if (msg->android.notification.event_time.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_307;
        json.set(s.c_str(), msg->android.notification.event_time);
    }
    if (msg->android.notification.local_only.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_308;
        json.set(s.c_str(), ut->boolVal(msg->android.notification.local_only.c_str()));
    }
    if (msg->android.notification.notification_priority.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_309;
        json.set(s.c_str(), msg->android.notification.notification_priority);
    }
    if (msg->android.notification.default_sound.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_310;
        json.set(s.c_str(), ut->boolVal(msg->android.notification.default_sound.c_str()));
    }
    if (msg->android.notification.default_vibrate_timings.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_311;
        json.set(s.c_str(), ut->boolVal(msg->android.notification.default_vibrate_timings.c_str()));
    }
    if (msg->android.notification.default_light_settings.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_312;
        json.set(s.c_str(), ut->boolVal(msg->android.notification.default_light_settings.c_str()));
    }
    if (msg->android.notification.vibrate_timings.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_313;
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->android.notification.vibrate_timings);
        json.set(s.c_str(), arr);
    }

    if (msg->android.notification.visibility.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_314;
        json.set(s.c_str(), msg->android.notification.visibility);
    }
    if (msg->android.notification.notification_count.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_315;
        json.set(s.c_str(), atoi(msg->android.notification.notification_count.c_str()));
    }

    if (msg->android.notification.image.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_300;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_297;
        json.set(s.c_str(), msg->android.notification.image);
    }

    s = base;
    s += fb_esp_pgm_str_300;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_122;
    s += fb_esp_pgm_str_1;
    s += fb_esp_pgm_str_316;
    s += fb_esp_pgm_str_1;
    base = s;

    if (msg->android.notification.light_settings.color.red.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_294;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_317;
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.red.c_str()));
    }
    if (msg->android.notification.light_settings.color.green.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_294;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_318;
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.green.c_str()));
    }
    if (msg->android.notification.light_settings.color.blue.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_294;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_319;
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.blue.c_str()));
    }
    if (msg->android.notification.light_settings.color.alpha.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_294;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_320;
        json.set(s.c_str(), atoi(msg->android.notification.light_settings.color.alpha.c_str()));
    }
    if (msg->android.notification.light_settings.light_on_duration.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_321;
        json.set(s.c_str(), msg->android.notification.light_settings.light_on_duration);
    }
    if (msg->android.notification.light_settings.light_off_duration.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_322;
        json.set(s.c_str(), msg->android.notification.light_settings.light_off_duration);
    }

    ////// WebpushConfig

    FirebaseJson js;
    base = _base;

    if (msg->webpush.headers.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_301;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_324;
        js.setJsonData(msg->webpush.headers);
        json.set(s.c_str(), js);
    }

    if (msg->webpush.data.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_301;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_135;
        js.setJsonData(msg->webpush.data);
        json.set(s.c_str(), js);
    }

    if (msg->webpush.notification.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_301;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_122;
        js.setJsonData(msg->webpush.notification);
        json.set(s.c_str(), js);
    }

    if (msg->webpush.fcm_options.analytics_label.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_301;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_298;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_299;
        json.set(s.c_str(), msg->webpush.fcm_options.analytics_label);
    }

    if (msg->webpush.fcm_options.link.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_301;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_298;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_325;
        json.set(s.c_str(), msg->webpush.fcm_options.link);
    }

    ////// ApnsConfig

    if (msg->apns.headers.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_302;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_324;
        js.setJsonData(msg->apns.headers);
        json.set(s.c_str(), js);
    }

    if (msg->apns.payload.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_302;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_135;
        js.setJsonData(msg->apns.payload);
        json.set(s.c_str(), js);
    }

    if (msg->apns.fcm_options.analytics_label.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_302;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_298;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_299;
        json.set(s.c_str(), msg->apns.fcm_options.analytics_label);
    }

    if (msg->apns.fcm_options.image.length() > 0)
    {
        s = base;
        s += fb_esp_pgm_str_302;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_298;
        s += fb_esp_pgm_str_1;
        s += fb_esp_pgm_str_297;
        json.set(s.c_str(), msg->apns.fcm_options.image);
    }

    base.clear();
    _base.clear();
    s.clear();
    raw = json.raw();
}

bool FB_CM::fcm_send(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, const char *msg)
{

    if (Signer.getCfg())
    {
        if (Signer.getTokenType() != token_type_undefined)
            if (!Signer.tokenReady())
            {
                Signer.getCfg()->internal.fb_processing = false;
                return false;
            }
    }

    bool ret = sendHeader(fbdo, mode, msg);

    if (ret)
        fbdo->tcpClient.send(msg);

    fbdo->session.fcm.payload.clear();
    if (fbdo->session.response.code < 0)
    {
        fbdo->closeSession();
        Signer.getCfg()->internal.fb_processing = false;
        return false;
    }
    else
        fbdo->session.connected = true;

    ret = waitResponse(fbdo);

    if (Signer.getCfg())
        Signer.getCfg()->internal.fb_processing = false;

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
    if (fbdo->session.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect())
        return false;

    if (!fbdo->session.connected)
    {
        fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;
        return false;
    }

    unsigned long dataTime = millis();

    char *pChunk = nullptr;
    char *temp = nullptr;
    char *header = nullptr;
    char *payload = nullptr;
    bool isHeader = false;

    struct server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = fbdo->session.resp_size;
    int pBufPos = 0;
    int hBufPos = 0;
    int chunkBufSize = fbdo->tcpClient.available();
    int hstate = 0;
    int pstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = fbdo->session.resp_size;
    int payloadRead = 0;
    struct fb_esp_auth_token_error_t error;
    error.code = -1;

    fbdo->session.response.code = FIREBASE_ERROR_HTTP_CODE_OK;
    fbdo->session.content_length = -1;
    fbdo->session.payload_length = 0;
    fbdo->session.chunked_encoding = false;
    fbdo->session.buffer_ovf = false;

    defaultChunkSize = 768;

    while (fbdo->tcpClient.connected() && chunkBufSize <= 0)
    {
        if (!fbdo->reconnect(dataTime))
            return false;
        chunkBufSize = fbdo->tcpClient.available();
        ut->idle();
    }

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0)
        {
            if (!fbdo->reconnect())
                return false;

            chunkBufSize = fbdo->tcpClient.available();

            if (chunkBufSize <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    // the first chunk can be http response header
                    header = (char *)ut->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = fbdo->tcpClient.readLine(header, chunkBufSize);
                    int pos = 0;

                    temp = ut->getHeader(header, fb_esp_pgm_str_5, fb_esp_pgm_str_6, pos, 0);
                    ut->idle();
                    dataTime = millis();
                    if (temp)
                    {
                        // http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(temp);
                        fbdo->session.response.code = response.httpCode;
                        ut->delP(&temp);
                    }
                    else
                    {
                        payload = (char *)ut->newP(payloadLen);
                        pstate = 1;
                        memcpy(payload, header, readLen);
                        pBufPos = readLen;
                        ut->delP(&header);
                        hstate = 0;
                    }
                }
                else
                {
                    ut->idle();
                    dataTime = millis();
                    // the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        // read one line of next header field until the empty header has found
                        temp = (char *)ut->newP(chunkBufSize);
                        int readLen = fbdo->tcpClient.readLine(temp, chunkBufSize);
                        bool headerEnded = false;

                        // check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (temp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (temp[0] == '\r' && temp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {

                            if (response.httpCode == 401)
                                Signer.authenticated = false;
                            else if (response.httpCode < 300)
                                Signer.authenticated = true;

                            // parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            fbdo->session.http_code = response.httpCode;

                            if (hstate == 1)
                                ut->delP(&header);
                            hstate = 0;

                            fbdo->session.chunked_encoding = response.isChunkedEnc;
                        }
                        else
                        {
                            // accumulate the remaining header field
                            memcpy(header + hBufPos, temp, readLen);
                            hBufPos += readLen;
                        }

                        ut->delP(&temp);
                    }
                    else
                    {
                        // the next chuunk data is the payload
                        if (!response.noContent)
                        {
                            pChunkIdx++;

                            pChunk = (char *)ut->newP(chunkBufSize + 1);

                            if (!payload || pstate == 0)
                            {
                                pstate = 1;
                                payload = (char *)ut->newP(payloadLen + 1);
                            }

                            // read the avilable data
                            int readLen = 0;

                            // chunk transfer encoding?
                            if (response.isChunkedEnc)
                                readLen = fbdo->tcpClient.readChunkedData(pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                            else
                            {
                                if (fbdo->tcpClient.available() < chunkBufSize)
                                    chunkBufSize = fbdo->tcpClient.available();
                                readLen = fbdo->tcpClient.readBytes(pChunk, chunkBufSize);
                            }

                            if (readLen > 0)
                            {
                                fbdo->session.payload_length += readLen;
                                if (fbdo->session.max_payload_length < fbdo->session.payload_length)
                                    fbdo->session.max_payload_length = fbdo->session.payload_length;
                                payloadRead += readLen;
                                fbdo->checkOvf(pBufPos + readLen, response);

                                if (!fbdo->session.buffer_ovf)
                                {
                                    if (pBufPos + readLen <= payloadLen)
                                        memcpy(payload + pBufPos, pChunk, readLen);
                                    else
                                    {
                                        // in case of the accumulated payload size is bigger than the char array
                                        // reallocate the char array

                                        char *buf = (char *)ut->newP(pBufPos + readLen + 1);
                                        memcpy(buf, payload, pBufPos);

                                        memcpy(buf + pBufPos, pChunk, readLen);

                                        payloadLen = pBufPos + readLen;
                                        ut->delP(&payload);
                                        payload = (char *)ut->newP(payloadLen + 1);
                                        memcpy(payload, buf, payloadLen);
                                        ut->delP(&buf);
                                    }
                                }
                            }

                            ut->delP(&pChunk);
                            if (readLen < 0 && payloadRead >= response.contentLen)
                                break;
                            if (readLen > 0)
                                pBufPos += readLen;
                        }
                        else
                        {
                            // read all the rest data
                            fbdo->tcpClient.flush();
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delP(&header);

        if (payload)
        {
            if (response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK)
                fbdo->session.fcm.payload = payload;
            else
            {

                MB_String t = payload;
                t.trim();

                if (t[0] == '{' && t[t.length() - 1] == '}')
                {
                    FirebaseJson json;
                    FirebaseJsonData data;
                    json.setJsonData(t.c_str());

                    json.get(data, pgm2Str(fb_esp_pgm_str_257));

                    if (data.success)
                    {
                        error.code = data.to<int>();
                        json.get(data, pgm2Str(fb_esp_pgm_str_258));
                        if (data.success)
                            fbdo->session.error = data.to<const char *>();
                    }
                    else
                        error.code = 0;
                }
            }
        }

        if (pstate == 1)
            ut->delP(&payload);

        return error.code == 0 || response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK;
    }
    else
    {
        fbdo->tcpClient.flush();
    }

    return false;
}

void FB_CM::rescon(FirebaseData *fbdo, const char *host)
{
    if (fbdo->session.cert_updated || !fbdo->session.connected || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout || fbdo->session.con_mode != fb_esp_con_mode_fcm || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->session.host = host;
    fbdo->session.con_mode = fb_esp_con_mode_fcm;
}

bool FB_CM::handleFCMRequest(FirebaseData *fbdo, fb_esp_fcm_msg_mode mode, const char *payload)
{
    fbdo->tcpClient.setSPIEthernet(_spi_ethernet_module);

    fbdo->session.http_code = 0;

    if (!fbdo->reconnect())
        return false;

    if (!ut->waitIdle(fbdo->session.response.code))
        return false;

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif
    if (fbdo->session.long_running_task > 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Signer.getCfg())
    {
        if (Signer.getCfg()->internal.fb_processing)
            return false;

        Signer.getCfg()->internal.fb_processing = true;
    }

    fcm_connect(fbdo, mode);

    fbdo->session.con_mode = fb_esp_con_mode_fcm;

    return fcm_send(fbdo, mode, payload);
}

void FB_CM::clear()
{
    raw.clear();
    server_key.clear();
    if (ut && intUt)
    {
        delete ut;
        ut = nullptr;
    }
}

#endif

#endif // ENABLE