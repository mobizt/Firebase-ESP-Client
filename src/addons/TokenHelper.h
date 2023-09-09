#ifndef TOKEN_HElPER_H
#define TOKEN_HElPER_H

#include <Arduino.h>
#include "FirebaseFS.h"

#include <Firebase.h>

// This header file includes the functions that provide the token generation process info.

/* The helper function to get the token type string */
const char *getTokenType(struct token_info_t info)
{
    switch (info.type)
    {
    case token_type_undefined:
        return "undefined";

    case token_type_legacy_token:
        return "legacy token";

    case token_type_id_token:
        return "id token (GITKit token)";

    case token_type_custom_token:
        return "custom token";

    case token_type_oauth2_access_token:
        return "OAuth2.0 access token";

    default:
        break;
    }
    return "undefined";
}

/* The helper function to get the token status string */
const char *getTokenStatus(struct token_info_t info)
{
    switch (info.status)
    {
    case token_status_uninitialized:
        return "uninitialized";

    case token_status_on_initialize:
        return "on initializing";

    case token_status_on_signing:
        return "on signing";

    case token_status_on_request:
        return "on request";

    case token_status_on_refresh:
        return "on refreshing";

    case token_status_ready:
        return "ready";

    case token_status_error:
        return "error";

    default:
        break;
    }
    return "uninitialized";
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
    /** firebase_auth_token_status enum
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
        Serial_Printf("Token info: type = %s, status = %s\n", getTokenType(info), getTokenStatus(info));
        Serial_Printf("Token error: %s\n", getTokenError(info).c_str());
    }
    else
    {
        Serial_Printf("Token info: type = %s, status = %s\n", getTokenType(info), getTokenStatus(info));
    }
}

#endif
