
/**
 * Google's IAM Policy Builder class, PolicyBuilder.cpp version 1.0.9
 *
 * Created April 5, 2023
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

#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)

#ifndef _FB_IAM_POLICY_BUILDER_CPP_
#define _FB_IAM_POLICY_BUILDER_CPP_

#include "PolicyBuilder.h"

AuditLogConfig::AuditLogConfig()
{
}

AuditLogConfig::~AuditLogConfig()
{
}

void AuditLogConfig::mSetLogType(MB_StringPtr logType)
{
        json.set(pgm2Str(firebase_func_pgm_str_71 /* "logType" */), stringPtr2Str(logType));
}

void AuditLogConfig::mAddExemptedMembers(MB_StringPtr member)
{
        arr.add(stringPtr2Str(member));
        json.set(pgm2Str(firebase_func_pgm_str_72 /* "exemptedMembers" */), arr);
}

void AuditLogConfig::clearExemptedMembers()
{
        arr.clear();
        json.remove(pgm2Str(firebase_func_pgm_str_72 /* "exemptedMembers" */));
}

void AuditLogConfig::clear()
{
        arr.clear();
        json.clear();
}

AuditConfig::AuditConfig()
{
}

AuditConfig::~AuditConfig()
{
}

void AuditConfig::addAuditLogConfig(AuditLogConfig *config, bool clear)
{
        arr.add(config->json);
        json.set(pgm2Str(firebase_func_pgm_str_69 /* "auditConfigs" */), arr);
        if (clear)
                config->clear();
}

void AuditConfig::clearAuditLogConfigs()
{
        arr.clear();
        json.remove(pgm2Str(firebase_func_pgm_str_69 /* "auditConfigs" */));
}

void AuditConfig::clear()
{
        arr.clear();
        json.clear();
}

void AuditConfig::mSetService(MB_StringPtr service)
{
        json.set(pgm2Str(firebase_func_pgm_str_73 /* "service" */), stringPtr2Str(service));
}

Binding::Binding()
{
}
Binding::~Binding()
{
}

void Binding::mAddMember(MB_StringPtr member)
{
        arr.add(stringPtr2Str(member));
        json.set(pgm2Str(firebase_func_pgm_str_75 /* "members" */), arr);
}

void Binding::mSetRole(MB_StringPtr role)
{
        json.set(pgm2Str(firebase_func_pgm_str_74 /* "role" */), stringPtr2Str(role));
}
void Binding::mSetCondition(MB_StringPtr expression, MB_StringPtr title, MB_StringPtr description, MB_StringPtr location)
{

        MB_String b, t;
        b += firebase_func_pgm_str_77; // "condition"
        b += firebase_pgm_str_1;       // "/"

        MB_String _expression = expression, _title = title, _description = description, _location = location;

        if (_expression.length() > 0)
        {
                t = b;
                t += firebase_func_pgm_str_78; // "expression"
                json.set(t.c_str(), _expression);
        }
        if (_title.length() > 0)
        {
                t = b;
                t += firebase_func_pgm_str_79; // "title"
                json.set(t.c_str(), _title);
        }
        if (_description.length() > 0)
        {
                t = b;
                t += firebase_func_pgm_str_80; // "description"
                json.set(t.c_str(), _description);
        }
        if (_location.length() > 0)
        {
                t = b;
                t += firebase_func_pgm_str_3; // "location"
                json.set(t.c_str(), _location);
        }
}

void Binding::clearMembers()
{
        arr.clear();
        json.remove(pgm2Str(firebase_func_pgm_str_75 /* "members" */));
}

void Binding::clear()
{
        arr.clear();
        json.clear();
}

PolicyBuilder::PolicyBuilder()
{
}

PolicyBuilder::~PolicyBuilder()
{
}

void PolicyBuilder::addAuditConfig(AuditConfig *config, bool clear)
{
        arr2.add(config->json);
        json.set(pgm2Str(firebase_func_pgm_str_69 /* "auditConfigs" */), arr2);
        if (clear)
                config->clear();
};

void PolicyBuilder::clearAuditConfigs()
{
        arr2.clear();
        json.remove(pgm2Str(firebase_func_pgm_str_69 /* "auditConfigs" */));
};

void PolicyBuilder::addBinding(Binding *binding, bool clear)
{
        arr.add(binding->json);
        json.set(pgm2Str(firebase_func_pgm_str_76 /* "bindings" */), arr);
        if (clear)
                binding->clear();
}

void PolicyBuilder::mSetVersion(MB_StringPtr v)
{
        MB_String _v = v;
        json.set(pgm2Str(firebase_func_pgm_str_81 /* "version" */), atoi(_v.c_str()));
}
void PolicyBuilder::mSetETag(MB_StringPtr etag)
{
        json.set(pgm2Str(firebase_func_pgm_str_70 /* "etag" */), stringPtr2Str(etag));
}
void PolicyBuilder::clearBindings()
{
        arr.clear();
        json.remove(pgm2Str(firebase_func_pgm_str_76 /* "bindings" */));
}
void PolicyBuilder::clear()
{
        arr2.clear();
        arr.clear();
        json.clear();
}

void PolicyBuilder::toString(String &s, bool prettify)
{
        json.toString(s, prettify);
}

const char *PolicyBuilder::raw()
{
        return json.raw();
}

#endif

#endif // ENABLE