#ifndef TOKEN_HElPER_H
#define TOKEN_HElPER_H

#include <Arduino.h>
#if defined(ESP32)
#if defined(FIREBASE_ESP32_CLIENT)
#include <FirebaseESP32.h>
#endif
#elif defined(ESP8266)
#if defined(FIREBASE_ESP8266_CLIENT)
#include <FirebaseESP8266.h>
#endif
#endif

#if defined(FIREBASE_ESP_CLIENT)
#include <Firebase_ESP_Client.h>
#endif

//This header file includes the functions that provide the token generation process info.

/* The helper function to get the token type string */
String getTokenType(struct token_info_t info)
{
    switch (info.type)
    {
    case token_type_undefined:
        return (const char *)FPSTR("undefined");

    case token_type_legacy_token:
        return (const char *)FPSTR("legacy token");

    case token_type_id_token:
        return (const char *)FPSTR("id token");

    case token_type_custom_token:
        return (const char *)FPSTR("custom token");

    case token_type_oauth2_access_token:
        return (const char*)FPSTR("OAuth2.0 access token");

    default:
        break;
    }
    return (const char *)FPSTR("undefined");
}

/* The helper function to get the token status string */
String getTokenStatus(struct token_info_t info)
{
    switch (info.status)
    {
    case token_status_uninitialized:
        return (const char*)FPSTR("uninitialized");

    case token_status_on_initialize:
        return (const char *)FPSTR("on initializing\n** NTP time acquiring **\n** If this takes time too long, set the time manually before calling Firebase.begin **\n");

    case token_status_on_signing:

#if defined(ESP32)
        return (const char*)FPSTR("on signing");
#elif defined(ESP8266)
        return (const char *)FPSTR("on signing\n** wdt reset may be occurred at this stage due to the blocking code in the token generation process **\n");
#endif

    case token_status_on_request:
        return (const char*)FPSTR("on request");

    case token_status_on_refresh:
        return (const char*)FPSTR("on refreshing");

    case token_status_ready:
        return (const char*)FPSTR("ready");

    case token_status_error:
        return (const char*)FPSTR("error");

    default:
        break;
    }
    return (const char *)FPSTR("uninitialized");
}

/* The helper function to get the token error string */
String getTokenError(struct token_info_t info)
{
    String s = "code: ";
    s += String(info.error.code);
    s += ", message: ";
    s += info.error.message.c_str();
    return s;
}

void tokenStatusCallback(TokenInfo info)
{
    /** fb_esp_auth_token_status enum
     * token_status_uninitialized,
     * token_status_on_initialize,
     * token_status_on_signing,
     * token_status_on_request,
     * token_status_on_refresh,
     * token_status_ready,
     * token_status_error
    */
    if (info.status == token_status_error)
    {
        Serial.printf((const char *)FPSTR("Token info: type = %s, status = %s\n"), getTokenType(info).c_str(), getTokenStatus(info).c_str());
        Serial.printf((const char *)FPSTR("Token error: %s\n"), getTokenError(info).c_str());
    }
    else
    {
        Serial.printf((const char *)FPSTR("Token info: type = %s, status = %s\n"), getTokenType(info).c_str(), getTokenStatus(info).c_str());
    }
}

#endif
