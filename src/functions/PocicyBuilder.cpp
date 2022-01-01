/**
 * Google's IAM Policy Builder class, PolicyBuilder.cpp version 1.0.5
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created January 1, 2022
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

#ifdef ENABLE_FB_FUNCTIONS

#ifndef _FB_IAM_POLICY_BUILDER_CPP_
#define _FB_IAM_POLICY_BUILDER_CPP_
#include "PolicyBuilder.h"

AuditLogConfig::AuditLogConfig()
{
}

AuditLogConfig::~AuditLogConfig()
{
}

void AuditLogConfig::mSetLogType(const char *logType)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        json.set(pgm2Str(fb_esp_pgm_str_414), logType);
    }
}

void AuditLogConfig::mAddexemptedMembers(const char *member)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.add(member);
        json.set(pgm2Str(fb_esp_pgm_str_415), arr);
    }
}

void AuditLogConfig::clearExemptedMembers()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.remove(pgm2Str(fb_esp_pgm_str_415));
    }
}

void AuditLogConfig::clear()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.clear();
    }
}

AuditConfig::AuditConfig()
{
}

AuditConfig::~AuditConfig()
{
}

void AuditConfig::addAuditLogConfig(AuditLogConfig *config, bool clear)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.add(config->json);
        json.set(pgm2Str(fb_esp_pgm_str_411), arr);
        if(clear)
            config->clear();
    }
}

void AuditConfig::clearAuditLogConfigs()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.remove(pgm2Str(fb_esp_pgm_str_411));
    }
}

void AuditConfig::clear()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.clear();
    }
}

void AuditConfig::mSetService(const char *service)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        json.set(pgm2Str(fb_esp_pgm_str_416), service);
    }
}

Binding::Binding()
{
}
Binding::~Binding()
{
}

void Binding::mAddMember(const char *member)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.add(member);
        json.set(pgm2Str(fb_esp_pgm_str_403), arr);
    }
}

void Binding::mSetRole(const char *role)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        json.set(pgm2Str(fb_esp_pgm_str_402), role);
    }
}
void Binding::mSetCondition(const char *expression, const char *title, const char *description, const char *location)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        MBSTRING b, t;
        b.appendP(fb_esp_pgm_str_405);
        b.appendP(fb_esp_pgm_str_1);

        if (strlen(expression) > 0)
        {
            t = b;
            t.appendP(fb_esp_pgm_str_406);
            json.set(t.c_str(), expression);
        }
        if (strlen(title) > 0)
        {
            t = b;
            t.appendP(fb_esp_pgm_str_407);
            json.set(t.c_str(), title);
        }
        if (strlen(description) > 0)
        {
            t = b;
            t.appendP(fb_esp_pgm_str_408);
            json.set(t.c_str(), description);
        }
        if (strlen(location) > 0)
        {
            t = b;
            t.appendP(fb_esp_pgm_str_409);
            json.set(t.c_str(), location);
        }
    }
}

void Binding::clearMembers()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.remove(pgm2Str(fb_esp_pgm_str_403));
    }
}

void Binding::clear()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.clear();
    }
}

PolicyBuilder::PolicyBuilder()
{
}

PolicyBuilder::~PolicyBuilder()
{
}

void PolicyBuilder::addAuditConfig(AuditConfig *config, bool clear)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr2.add(config->json);
        json.set(pgm2Str(fb_esp_pgm_str_411), arr2);
        if(clear)
            config->clear();
    }
};

void PolicyBuilder::clearAuditConfigs()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr2.clear();
        json.remove(pgm2Str(fb_esp_pgm_str_411));
    }
};

void PolicyBuilder::addBinding(Binding *binding, bool clear)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.add(binding->json);
        json.set(pgm2Str(fb_esp_pgm_str_404), arr);
        if(clear)
            binding->clear();
    }
}

void PolicyBuilder::mSetVersion(const char* v)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        json.set(pgm2Str(fb_esp_pgm_str_410), atoi(v));
    }
}
void PolicyBuilder::mSetETag(const char *etag)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        json.set(pgm2Str(fb_esp_pgm_str_412), etag);
    }
}
void PolicyBuilder::clearBindings()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        json.remove(pgm2Str(fb_esp_pgm_str_404));
    }
}
void PolicyBuilder::clear()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr2.clear();
        arr.clear();
        json.clear();
    }
}

void PolicyBuilder::toString(String &s, bool prettify)
{
    json.toString(s, prettify);
}

const char* PolicyBuilder::raw()
{
    return json.raw();
}

#endif

#endif //ENABLE