
/**
 * Google's Firebase Cloud Messaging class, FCM.cpp version 1.1.0
 *
 * Created September 5, 2023
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

#include "./FirebaseFS.h"

#if defined(ENABLE_FCM) || defined(FIREBASE_ENABLE_FCM)

#ifndef FIREBASE_FCM_CPP
#define FIREBASE_FCM_CPP

#include "FCM.h"

FB_CM::FB_CM() {}
FB_CM::~FB_CM()
{
    clear();
}

void FB_CM::mSetServerKey(MB_StringPtr serverKey, SPI_ETH_Module *spi_ethernet_module)
{
    this->server_key = serverKey;
    this->_spi_ethernet_module = spi_ethernet_module;
}

bool FB_CM::checkServerKey(FirebaseData *fbdo)
{
    if (server_key.length() > 0)
        return true;
    fbdo->session.response.code = FIREBASE_ERROR_NO_FCM_SERVER_KEY_PROVIDED;
    return false;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_Legacy_HTTP_Message *msg)
{
    if (!checkServerKey(fbdo))
        return false;

    fcm_prepareLegacyPayload(msg);
    bool ret = handleFCMRequest(fbdo, firebase_fcm_msg_mode_legacy_http, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::send(FirebaseData *fbdo, FCM_HTTPv1_JSON_Message *msg)
{
    Core.tokenReady();

    // Core.getTokenType() is required as Core.config is not set in fcm legacy
    if (Core.getTokenType() != token_type_oauth2_access_token)
    {
        fbdo->session.response.code = FIREBASE_ERROR_OAUTH2_REQUIRED;
        return false;
    }

    fcm_prepareV1Payload(msg);
    bool ret = handleFCMRequest(fbdo, firebase_fcm_msg_mode_httpv1, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::mSubscribeTopic(FirebaseData *fbdo, MB_StringPtr topic, const char *IID[], size_t numToken)
{

    Core.tokenReady();

    if (!checkServerKey(fbdo))
        return false;

    MB_String _topic = topic;

    fcm_preparSubscriptionPayload(_topic.c_str(), IID, numToken);
    bool ret = handleFCMRequest(fbdo, firebase_fcm_msg_mode_subscribe, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::mUnsubscribeTopic(FirebaseData *fbdo, MB_StringPtr topic, const char *IID[], size_t numToken)
{
    Core.tokenReady();

    if (!checkServerKey(fbdo))
        return false;

    fcm_preparSubscriptionPayload(stringPtr2Str(topic), IID, numToken);
    bool ret = handleFCMRequest(fbdo, firebase_fcm_msg_mode_unsubscribe, raw.c_str());
    raw.clear();
    return ret;
}

bool FB_CM::mAppInstanceInfo(FirebaseData *fbdo, const char *IID)
{
    if (!checkServerKey(fbdo))
        return false;

    MB_String payload = IID;
    bool ret = handleFCMRequest(fbdo, firebase_fcm_msg_mode_app_instance_info, payload.c_str());
    payload.clear();
    return ret;
}

bool FB_CM::mRegisAPNsTokens(FirebaseData *fbdo, MB_StringPtr application, bool sandbox, const char *APNs[], size_t numToken)
{
    Core.tokenReady();

    if (!checkServerKey(fbdo))
        return false;

    fcm_preparAPNsRegistPayload(stringPtr2Str(application), sandbox, APNs, numToken);
    bool ret = handleFCMRequest(fbdo, firebase_fcm_msg_mode_apn_token_registration, raw.c_str());
    raw.clear();
    return ret;
}

String FB_CM::payload(FirebaseData *fbdo)
{
    return fbdo->session.fcm.payload.c_str();
}

void FB_CM::fcm_connect(FirebaseData *fbdo, firebase_fcm_msg_mode mode)
{
    fbdo->tcpClient.setSPIEthernet(_spi_ethernet_module);

    // Core.getTokenType() is required as Core.config is not set in fcm legacy
    if (Core.getTokenType() != token_type_undefined)
    {
        if (!Core.tokenReady())
            return;
    }

    MB_String host;
    Core.hh.addGAPIsHost(host,
                         (mode == firebase_fcm_msg_mode_legacy_http ||
                          mode == firebase_fcm_msg_mode_httpv1)
                             ? firebase_fcm_pgm_str_1 /* "fcm" */
                             : firebase_fcm_pgm_str_2 /* "iid" */);

    rescon(fbdo, host.c_str());
    fbdo->tcpClient.setSession(&fbdo->bsslSession);
    fbdo->tcpClient.begin(host.c_str(), port, &fbdo->session.response.code);
    fbdo->session.max_payload_length = 0;
}

bool FB_CM::sendHeader(FirebaseData *fbdo, firebase_fcm_msg_mode mode, const char *payload)
{
    bool msgMode = (mode == firebase_fcm_msg_mode_legacy_http || mode == firebase_fcm_msg_mode_httpv1);

    MB_String header;
    if (mode == firebase_fcm_msg_mode_app_instance_info)
        Core.hh.addRequestHeaderFirst(header, http_get);
    else
        Core.hh.addRequestHeaderFirst(header, http_post);

    if (msgMode)
    {
        // Core.getTokenType() is required as Core.config is not set in fcm legacy
        if (Core.getTokenType() != token_type_oauth2_access_token || mode == firebase_fcm_msg_mode_legacy_http)
            header += firebase_fcm_pgm_str_3; // "fcm/send"
        else
        {
            Core.uh.addGAPIv1Path(header);
            header += Core.config->service_account.data.project_id;
            header += firebase_fcm_pgm_str_4; // "/messages:send"
        }
    }
    else
    {
        if (mode == firebase_fcm_msg_mode_subscribe)
        {
            header += firebase_fcm_pgm_str_5; // "/iid/v1"
            header += firebase_fcm_pgm_str_6; // ":batchAdd"
        }
        else if (mode == firebase_fcm_msg_mode_unsubscribe)
        {
            header += firebase_fcm_pgm_str_5; // "/iid/v1"
            header += firebase_fcm_pgm_str_7; // ":batchRemove"
        }
        else if (mode == firebase_fcm_msg_mode_app_instance_info)
        {
            header += firebase_fcm_pgm_str_8; //  "/iid/info/"
            header += payload;
            header += firebase_fcm_pgm_str_9; // "?details=true"
        }
        else if (mode == firebase_fcm_msg_mode_apn_token_registration)
        {
            header += firebase_fcm_pgm_str_5;  // "/iid/v1"
            header += firebase_fcm_pgm_str_15; // ":batchImport"
        }
    }

    Core.hh.addRequestHeaderLast(header);

    Core.hh.addGAPIsHostHeader(header, msgMode ? firebase_fcm_pgm_str_1 /* "fcm" */ : firebase_fcm_pgm_str_2 /* "iid" */);

    // Core.getTokenType() is required as Core.config is not set in fcm legacy
    if (Core.getTokenType() == token_type_oauth2_access_token && mode == firebase_fcm_msg_mode_httpv1)
    {
        Core.hh.addAuthHeaderFirst(header, token_type_oauth2_access_token);

        fbdo->tcpSend(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpSend(Core.getToken());

        if (fbdo->session.response.code < 0)
            return false;
    }
    else
    {
        Core.hh.addAuthHeaderFirst(header, token_type_undefined);

        fbdo->tcpSend(header.c_str());
        header.clear();

        if (fbdo->session.response.code < 0)
            return false;

        fbdo->tcpSend(server_key.c_str());

        if (fbdo->session.response.code < 0)
            return false;
    }

    Core.hh.addNewLine(header);

    Core.hh.addUAHeader(header);

    if (mode != firebase_fcm_msg_mode_app_instance_info)
    {
        Core.hh.addContentTypeHeader(header, firebase_pgm_str_62 /* "application/json" */);
        Core.hh.addContentLengthHeader(header, strlen(payload));
    }

    // required for ESP32 core sdk v2.0.x.
    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    Core.hh.addConnectionHeader(header, keepAlive);
    Core.hh.addNewLine(header);

    fbdo->tcpSend(header.c_str());
    header.clear();

    if (fbdo->session.response.code < 0)
        return false;

    return true;
}

void FB_CM::fcm_prepareLegacyPayload(FCM_Legacy_HTTP_Message *msg)
{
    raw.clear();
    FirebaseJson json;

    if (msg->targets.to.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_10 /* "to" */), msg->targets.to);

    if (msg->targets.registration_ids.length() > 0)
    {
        FirebaseJsonArray arr(msg->targets.registration_ids);
        json.add(firebase_fcm_pgm_str_11 /* "registration_ids" */, arr);
    }

    if (msg->targets.condition.length() > 0)
        json.add(firebase_fcm_pgm_str_16 /* "condition" */, msg->targets.condition);

    if (msg->options.collapse_key.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_14 /* "collapse_key" */), msg->options.collapse_key);

    if (msg->options.priority.length() > 0)
        json.add(firebase_fcm_pgm_str_12 /* "priority" */, msg->options.priority);

    if (msg->options.content_available.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_17 /* "content_available" */), Core.ut.boolVal(msg->options.content_available));

    if (msg->options.mutable_content.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_18 /* "mutable_content" */), Core.ut.boolVal(msg->options.mutable_content));

    if (msg->options.time_to_live.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_13 /* "time_to_live" */), atoi(msg->options.time_to_live.c_str()));

    if (msg->options.restricted_package_name.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_19 /* "restricted_package_name" */), msg->options.restricted_package_name);

    if (msg->options.dry_run.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_20 /* "dry_run" */), msg->options.dry_run);

    if (msg->options.direct_boot_ok.length() > 0)
        json.add(pgm2Str(firebase_fcm_pgm_str_21 /* "direct_boot_ok" */), Core.ut.boolVal(msg->options.direct_boot_ok));

    if (msg->payloads.data.length() > 0)
    {
        FirebaseJson js(msg->payloads.data);
        json.add(pgm2Str(firebase_pgm_str_67 /* "data" */), js);
    }

    if (msg->payloads.notification.title.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_22 /* "title" */), msg->payloads.notification.title);

    if (msg->payloads.notification.body.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_23 /* "body" */), msg->payloads.notification.body);

    if (msg->payloads.notification.sound.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_24 /* "sound" */), msg->payloads.notification.sound);

    if (msg->payloads.notification.badge.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_25 /* "badge" */), msg->payloads.notification.badge);

    if (msg->payloads.notification.click_action.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_33 /* "click_action" */), msg->payloads.notification.click_action);

    if (msg->payloads.notification.subtitle.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_26 /* "subtitle" */), msg->payloads.notification.subtitle);

    if (msg->payloads.notification.body_loc_key.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_27 /* "body_loc_key" */), msg->payloads.notification.body_loc_key);

    if (msg->payloads.notification.body_loc_args.length() > 0)
    {
        FirebaseJsonArray arr(msg->payloads.notification.body_loc_args);
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_28 /* "body_loc_args" */), arr);
    }

    if (msg->payloads.notification.title_loc_key.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_29 /* "title_loc_key" */), msg->payloads.notification.title_loc_key);

    if (msg->payloads.notification.title_loc_args.length() > 0)
    {
        FirebaseJsonArray arr(msg->payloads.notification.title_loc_args);
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_30 /* "title_loc_args" */), arr);
    }

    if (msg->payloads.notification.android_channel_id.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_31 /* "android_channel_id" */), msg->payloads.notification.android_channel_id);

    if (msg->payloads.notification.icon.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_32 /* "icon" */), msg->payloads.notification.icon);

    if (msg->payloads.notification.tag.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_34 /* "tag" */), msg->payloads.notification.tag);

    if (msg->payloads.notification.color.length() > 0)
        json.set(Core.ut.makeFCMNotificationPath(firebase_fcm_pgm_str_35 /* "color" */), msg->payloads.notification.color);
    json.toString(raw);
}

void FB_CM::fcm_preparSubscriptionPayload(const char *topic, const char *IID[], size_t numToken)
{
    MB_String s;
    raw.clear();
    FirebaseJson json;

    s += firebase_fcm_pgm_str_36; // "/topics/"
    s += topic;

    json.add(pgm2Str(firebase_fcm_pgm_str_10 /* "to" */), s);

    FirebaseJsonArray arr;
    for (size_t i = 0; i < numToken; i++)
    {
        if (IID[i])
        {
            s = IID[i];
            s.trim();
            if (s.length() > 0)
                arr.add(s);
        }
    }
    json.add(pgm2Str(firebase_fcm_pgm_str_37 /* "registration_tokens" */), arr);
    json.toString(raw);
}

void FB_CM::fcm_preparAPNsRegistPayload(const char *application, bool sandbox, const char *APNs[], size_t numToken)
{
    MB_String s;
    raw.clear();
    FirebaseJson json;

    json.add(pgm2Str(firebase_fcm_pgm_str_38 /* "application" */), application);
    json.add(pgm2Str(firebase_fcm_pgm_str_39 /* "sandbox" */), sandbox);

    FirebaseJsonArray arr;
    for (size_t i = 0; i < numToken; i++)
    {
        if (APNs[i])
        {
            s = APNs[i];
            s.trim();
            if (s.length() > 0)
                arr.add(s);
        }
    }
    json.add(pgm2Str(firebase_fcm_pgm_str_40 /* "apns_tokens" */), arr);
    json.toString(raw);
}

void FB_CM::fcm_prepareV1Payload(FCM_HTTPv1_JSON_Message *msg)
{

    MB_String s;
    FirebaseJson json;
    raw.clear();

    if (msg->token.length() > 0)
        json.set(Core.ut.makeFCMMessagePath(firebase_pgm_str_18 /* "token" */), msg->token);
    else if (msg->topic.length() > 0)
        json.set(Core.ut.makeFCMMessagePath(firebase_fcm_pgm_str_41 /* "topic" */), msg->topic);
    else if (msg->condition.length() > 0)
        json.set(Core.ut.makeFCMMessagePath(firebase_fcm_pgm_str_16 /* "condition" */), msg->condition);

    if (msg->data.length() > 0)
    {
        FirebaseJson js(msg->data);
        json.set(Core.ut.makeFCMMessagePath(firebase_pgm_str_67 /* "data" */), js);
    }

    if (msg->notification.title.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_22 /* "title" */);
        json.set(s, msg->notification.title);
    }

    if (msg->notification.body.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_23 /* "body" */);
        json.set(s, msg->notification.body);
    }

    if (msg->notification.image.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_42 /* "image" */);
        json.set(s, msg->notification.image);
    }

    if (msg->fcm_options.analytics_label.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        s += firebase_fcm_pgm_str_43; // "fcm_options"
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_44; // "analytics_label"
        json.set(s, msg->fcm_options.analytics_label);
    }

    ////// AndroidConfig

    if (msg->android.collapse_key.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_fcm_pgm_str_14 /* "collapse_key" */);
        json.set(s, msg->android.collapse_key);
    }

    if (msg->android.priority.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_fcm_pgm_str_12 /* "priority" */);
        json.set(s, msg->android.priority);
    }

    if (msg->android.ttl.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_fcm_pgm_str_45 /* "ttl" */);
        json.set(s, msg->android.ttl);
    }

    if (msg->android.restricted_package_name.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_fcm_pgm_str_19 /* "restricted_package_name" */);
        json.set(s, msg->android.restricted_package_name);
    }

    if (msg->android.data.length() > 0)
    {
        FirebaseJson js(msg->android.data);
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_pgm_str_67 /* "data" */);
        json.set(s, js);
    }

    if (msg->android.fcm_options.analytics_label.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_fcm_pgm_str_43 /* "fcm_options" */);
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_44; // "analytics_label"
        json.set(s, msg->android.fcm_options.analytics_label);
    }

    if (msg->android.direct_boot_ok.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s, firebase_fcm_pgm_str_21 /* "direct_boot_ok" */);
        json.set(s, msg->android.direct_boot_ok);
    }

    if (msg->android.notification.title.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_22 /* "title" */);
        json.set(s, msg->android.notification.title);
    }
    if (msg->android.notification.body.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_23 /* "body" */);
        json.set(s, msg->android.notification.body);
    }

    if (msg->android.notification.icon.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_32 /* "icon" */);
        json.set(s, msg->android.notification.icon);
    }

    if (msg->android.notification.color.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_35 /* "color" */);
        json.set(s, msg->android.notification.color);
    }

    if (msg->android.notification.sound.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_24 /* "sound" */);
        json.set(s, msg->android.notification.sound);
    }
    if (msg->android.notification.tag.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_34 /* "tag" */);
        json.set(s, msg->android.notification.tag);
    }
    if (msg->android.notification.click_action.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_33 /* "click_action" */);
        json.set(s, msg->android.notification.click_action);
    }

    if (msg->android.notification.body_loc_key.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_27 /* "body_loc_key" */);
        json.set(s, msg->android.notification.body_loc_key);
    }

    if (msg->android.notification.body_loc_args.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_28 /* "body_loc_args" */);
        static FirebaseJsonArray arr(msg->android.notification.body_loc_args);
        json.set(s, arr);
    }

    if (msg->android.notification.title_loc_key.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_29 /* "title_loc_key" */);
        json.set(s, msg->android.notification.title_loc_key);
    }

    if (msg->android.notification.title_loc_args.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_30 /* "title_loc_args" */);
        FirebaseJsonArray arr(msg->android.notification.title_loc_args);
        json.set(s, arr);
    }

    if (msg->android.notification.channel_id.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_46 /* "channel_id" */);
        json.set(s, msg->android.notification.channel_id);
    }
    if (msg->android.notification.ticker.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_47 /* "ticker" */);
        json.set(s, msg->android.notification.ticker);
    }
    if (msg->android.notification.sticky.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_48 /* "sticky" */);
        json.set(s, Core.ut.boolVal(msg->android.notification.sticky));
    }
    if (msg->android.notification.event_time.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_49 /* "event_time" */);
        json.set(s, msg->android.notification.event_time);
    }
    if (msg->android.notification.local_only.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_50 /* "local_only" */);
        json.set(s, Core.ut.boolVal(msg->android.notification.local_only));
    }
    if (msg->android.notification.notification_priority.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_51 /* "notification_priority" */);
        json.set(s, msg->android.notification.notification_priority);
    }
    if (msg->android.notification.default_sound.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_52 /* "default_sound" */);
        json.set(s, Core.ut.boolVal(msg->android.notification.default_sound));
    }
    if (msg->android.notification.default_vibrate_timings.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_53 /* "default_vibrate_timings" */);
        json.set(s, Core.ut.boolVal(msg->android.notification.default_vibrate_timings));
    }
    if (msg->android.notification.default_light_settings.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_54 /* "default_light_settings" */);
        json.set(s, Core.ut.boolVal(msg->android.notification.default_light_settings));
    }
    if (msg->android.notification.vibrate_timings.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_55 /* "vibrate_timings" */);
        FirebaseJsonArray arr;
        arr.setJsonArrayData(msg->android.notification.vibrate_timings);
        json.set(s, arr);
    }

    if (msg->android.notification.visibility.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_56 /* "visibility" */);
        json.set(s, msg->android.notification.visibility);
    }
    if (msg->android.notification.notification_count.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_57 /* "notification_count" */);
        json.set(s, atoi(msg->android.notification.notification_count.c_str()));
    }

    if (msg->android.notification.image.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMAndroidPath(s);
        Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_42 /* "image" */);
        json.set(s, msg->android.notification.image);
    }

    s = Core.ut.makeFCMMessagePath();
    Core.ut.addFCMAndroidPath(s);
    Core.ut.addFCMNotificationPath(s, firebase_fcm_pgm_str_58 /* "light_settings" */);
    s += firebase_pgm_str_1; // "/"
    MB_String base = s;

    if (msg->android.notification.light_settings.color.red.length() > 0)
    {
        s = base;
        s += firebase_fcm_pgm_str_35; // "color"
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_59; // "red"
        json.set(s, atoi(msg->android.notification.light_settings.color.red.c_str()));
    }
    if (msg->android.notification.light_settings.color.green.length() > 0)
    {
        s = base;
        s += firebase_fcm_pgm_str_35; // "color"
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_60; // "green"
        json.set(s, atoi(msg->android.notification.light_settings.color.green.c_str()));
    }
    if (msg->android.notification.light_settings.color.blue.length() > 0)
    {
        s = base;
        s += firebase_fcm_pgm_str_35; // "color"
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_61; // "blue"
        json.set(s, atoi(msg->android.notification.light_settings.color.blue.c_str()));
    }
    if (msg->android.notification.light_settings.color.alpha.length() > 0)
    {
        s = base;
        s += firebase_fcm_pgm_str_35; // "color"
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_62; // "alpha"
        json.set(s, atoi(msg->android.notification.light_settings.color.alpha.c_str()));
    }
    if (msg->android.notification.light_settings.light_on_duration.length() > 0)
    {
        s = base;
        s += firebase_fcm_pgm_str_63; // "light_on_duration"
        json.set(s, msg->android.notification.light_settings.light_on_duration);
    }
    if (msg->android.notification.light_settings.light_off_duration.length() > 0)
    {
        s = base;
        s += firebase_fcm_pgm_str_64; // "light_off_duration"
        json.set(s, msg->android.notification.light_settings.light_off_duration);
    }

    ////// WebpushConfig

    FirebaseJson js;

    if (msg->webpush.headers.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMWebpushPath(s, firebase_fcm_pgm_str_65 /* "headers" */);
        js.setJsonData(msg->webpush.headers);
        json.set(s, js);
    }

    if (msg->webpush.data.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMWebpushPath(s, firebase_pgm_str_67 /* "data" */);
        js.setJsonData(msg->webpush.data);
        json.set(s, js);
    }

    if (msg->webpush.notification.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMWebpushPath(s, firebase_fcm_pgm_str_67 /* "notification" */);
        js.setJsonData(msg->webpush.notification);
        json.set(s, js);
    }

    if (msg->webpush.fcm_options.analytics_label.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMWebpushPath(s, firebase_fcm_pgm_str_43 /* "fcm_options" */);
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_44; // "analytics_label"
        json.set(s, msg->webpush.fcm_options.analytics_label);
    }

    if (msg->webpush.fcm_options.link.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMWebpushPath(s, firebase_fcm_pgm_str_43 /* "fcm_options" */);
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_66; // "link"
        json.set(s, msg->webpush.fcm_options.link);
    }

    ////// ApnsConfig

    if (msg->apns.headers.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMApnsPath(s, firebase_fcm_pgm_str_65 /* "headers" */);
        js.setJsonData(msg->apns.headers);
        json.set(s, js);
    }

    if (msg->apns.payload.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMApnsPath(s, firebase_pgm_str_67 /* "data" */);
        js.setJsonData(msg->apns.payload);
        json.set(s, js);
    }

    if (msg->apns.fcm_options.analytics_label.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMApnsPath(s, firebase_fcm_pgm_str_43 /* "fcm_options" */);
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_44; // "analytics_label"
        json.set(s, msg->apns.fcm_options.analytics_label);
    }

    if (msg->apns.fcm_options.image.length() > 0)
    {
        s = Core.ut.makeFCMMessagePath();
        Core.ut.addFCMApnsPath(s, firebase_fcm_pgm_str_43 /* "fcm_options" */);
        s += firebase_pgm_str_1;      // "/"
        s += firebase_fcm_pgm_str_42; // "image"
        json.set(s, msg->apns.fcm_options.image);
    }

    json.toString(raw);
}

bool FB_CM::fcm_send(FirebaseData *fbdo, firebase_fcm_msg_mode mode, const char *msg)
{

    if (Core.config)
    {
        // Core.getTokenType() is required as Core.config is not set in fcm legacy
        if (Core.getTokenType() != token_type_undefined)
            if (!Core.tokenReady())
            {
                Core.internal.fb_processing = false;
                return false;
            }
    }

    // in legacy http fcm, the Core.config was not set yet, then no SSL certificate is appliable
    // set the SSL client to skip server SSL certificate verification
    fbdo->tcpClient.setCACert(nullptr);

    bool ret = sendHeader(fbdo, mode, msg);

    if (ret)
        fbdo->tcpSend(msg);

    fbdo->session.fcm.payload.clear();
    if (fbdo->session.response.code < 0)
    {
        fbdo->closeSession();
        Core.internal.fb_processing = false;
        return false;
    }

    ret = waitResponse(fbdo);

    Core.internal.fb_processing = false;

    bool msgMode = (mode == firebase_fcm_msg_mode_legacy_http || mode == firebase_fcm_msg_mode_httpv1);

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
    if (!fbdo->reconnect())
        return false;

    struct server_response_data_t response;
    struct firebase_tcp_response_handler_t tcpHandler;

    Core.hh.initTCPSession(fbdo->session);
    Core.hh.intTCPHandler(&fbdo->tcpClient, tcpHandler, 768, fbdo->session.resp_size, nullptr, false);

    if (!fbdo->waitResponse(tcpHandler))
        return false;

    bool complete = false;

    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */)
    {
        if (!fbdo->readResponse(&fbdo->session.fcm.payload, tcpHandler, response) && !response.isChunkedEnc)
            break;

        // Last chunk?
        if (Core.ut.isChunkComplete(&tcpHandler, &response, complete))
            break;
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    // parse the payload for error
    fbdo->getError(fbdo->session.fcm.payload, tcpHandler, response, false);

    return tcpHandler.error.code == 0 || response.httpCode == FIREBASE_ERROR_HTTP_CODE_OK;
}

void FB_CM::rescon(FirebaseData *fbdo, const char *host)
{
    fbdo->_responseCallback = NULL;

    if (fbdo->session.cert_updated || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.con_mode != firebase_con_mode_fcm ||
        strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }

    fbdo->session.host = host;
    fbdo->session.con_mode = firebase_con_mode_fcm;
}

bool FB_CM::handleFCMRequest(FirebaseData *fbdo, firebase_fcm_msg_mode mode, const char *payload)
{
    fbdo->tcpClient.setSPIEthernet(_spi_ethernet_module);

    fbdo->session.http_code = 0;

    if (!fbdo->reconnect())
        return false;

    if (!Core.waitIdle(fbdo->session.response.code))
        return false;

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (fbdo->session.rtdb.pause)
        return true;
#endif
    if (fbdo->session.long_running_task > 0)
    {
        fbdo->session.response.code = FIREBASE_ERROR_LONG_RUNNING_TASK;
        return false;
    }

    if (Core.internal.fb_processing)
        return false;

    Core.internal.fb_processing = true;

    fcm_connect(fbdo, mode);

    fbdo->session.con_mode = firebase_con_mode_fcm;

    return fcm_send(fbdo, mode, payload);
}

void FB_CM::clear()
{
    raw.clear();
    server_key.clear();
}

#endif

#endif // ENABLE