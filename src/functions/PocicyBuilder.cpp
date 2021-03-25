/**
 * Google's IAM Policy Builder class, PolicyBuilder.cpp version 1.0.1
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

#ifndef _FB_IAM_POLICY_BUILDER_CPP_
#define _FB_IAM_POLICY_BUILDER_CPP_
#include "PolicyBuilder.h"

AuditLogConfig::AuditLogConfig()
{
}

AuditLogConfig::~AuditLogConfig()
{
}

void AuditLogConfig::setLogType(const char *logType)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_414);
        json.set(tmp, logType);
        ut->delS(tmp);
    }
}

void AuditLogConfig::addexemptedMembers(const char *member)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_415);
        arr.add(member);
        json.set(tmp, arr);
        ut->delS(tmp);
    }
}

void AuditLogConfig::clearExemptedMembers()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        char *tmp = ut->strP(fb_esp_pgm_str_415);
        json.remove(tmp);
        ut->delS(tmp);
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
        char *tmp = ut->strP(fb_esp_pgm_str_411);
        arr.add(config->json);
        json.set(tmp, arr);
        ut->delS(tmp);
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
        char *tmp = ut->strP(fb_esp_pgm_str_411);
        json.remove(tmp);
        ut->delS(tmp);
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

void AuditConfig::setService(const char *service)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_416);
        json.set(tmp, service);
        ut->delS(tmp);
    }
}

Binding::Binding()
{
}
Binding::~Binding()
{
}

void Binding::addMember(const char *member)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_403);
        arr.add(member);
        json.set(tmp, arr);
        ut->delS(tmp);
    }
}

void Binding::setRole(const char *role)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_402);
        json.set(tmp, role);
        ut->delS(tmp);
    }
}
void Binding::setCondition(const char *expression, const char *title, const char *description, const char *location)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        std::string b, t;
        ut->appendP(b, fb_esp_pgm_str_405);
        ut->appendP(b, fb_esp_pgm_str_1);

        if (strlen(expression) > 0)
        {
            t = b;
            ut->appendP(t, fb_esp_pgm_str_406);
            json.set(t.c_str(), expression);
        }
        if (strlen(title) > 0)
        {
            t = b;
            ut->appendP(t, fb_esp_pgm_str_407);
            json.set(t.c_str(), title);
        }
        if (strlen(description) > 0)
        {
            t = b;
            ut->appendP(t, fb_esp_pgm_str_408);
            json.set(t.c_str(), description);
        }
        if (strlen(location) > 0)
        {
            t = b;
            ut->appendP(t, fb_esp_pgm_str_409);
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
        char *tmp = ut->strP(fb_esp_pgm_str_403);
        json.remove(tmp);
        ut->delS(tmp);
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
        char *tmp = ut->strP(fb_esp_pgm_str_411);
        json.set(tmp, arr2);
        ut->delS(tmp);
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
        char *tmp = ut->strP(fb_esp_pgm_str_411);
        json.remove(tmp);
        ut->delS(tmp);
    }
};

void PolicyBuilder::addBinding(Binding *binding, bool clear)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.add(binding->json);
        char *tmp = ut->strP(fb_esp_pgm_str_404);
        json.set(tmp, arr);
        ut->delS(tmp);
        if(clear)
            binding->clear();
    }
}

void PolicyBuilder::setVersion(int v)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_410);
        json.set(tmp, v);
        ut->delS(tmp);
    }
}
void PolicyBuilder::setETag(const char *etag)
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        char *tmp = ut->strP(fb_esp_pgm_str_412);
        json.set(tmp, etag);
        ut->delS(tmp);
    }
}
void PolicyBuilder::clearBindings()
{
    if (!ut)
        ut = Signer.ut;
    if (ut)
    {
        arr.clear();
        char *tmp = ut->strP(fb_esp_pgm_str_404);
        json.remove(tmp);
        ut->delS(tmp);
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

void PolicyBuilder::_toString(std::string &s, bool prettify)
{
    json.int_tostr(s, prettify);
}

#endif