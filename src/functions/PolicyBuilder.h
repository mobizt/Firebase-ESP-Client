/**
 * Google's IAM Policy Builder class, PolicyBuilder.h version 1.0.1
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

#ifndef _FB_IAM_POLICY_BUILDER_H_
#define _FB_IAM_POLICY_BUILDER_H_

#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"

/** The class that provides the configuration for logging a type of permissions
 * https://cloud.google.com/functions/docs/reference/rest/v1/Policy#LogType
*/
class AuditLogConfig
{
    friend class AuditConfig;

private:
    UtilsClass *ut = nullptr;
    FirebaseJson json;
    FirebaseJsonArray arr;

public:
    AuditLogConfig();
    ~AuditLogConfig();

    /**
     * Specify the permission types for which logging can be configured.
     * 
     * @param logType The enum of logType string
     * LOG_TYPE_UNSPECIFIED
     * ADMIN_READ
     * DATA_WRITE
     * DATA_READ
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#LogType
     * 
    */
    void setLogType(const char *logType);

    /**
     * Add the identities that do not cause logging for this type of permission. 
     * Follows the same format of Binding.members.
     * 
     * @param member The string of member e.g. allUsers
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#Binding.FIELDS.members
     * 
    */
    void addexemptedMembers(const char *member);

    /**
     * Clear all exempted members. 
    */
    void clearExemptedMembers();

    /**
     * Clear all AuditLogConfig data. 
    */
    void clear();
};

/** The class that provides the audit configuration for a service. 
 * The configuration determines which permission types are logged, and what identities, if any, are exempted from logging. 
 * An AuditConfig must have one or more AuditLogConfigs.\
 * 
 * If there are AuditConfigs for both allServices and a specific service, the union of 
 * the two AuditConfigs is used for that service: the log_types specified in each AuditConfig are enabled, 
 * and the exemptedMembers in each AuditLogConfig are exempted.
 * 
* https://cloud.google.com/functions/docs/reference/rest/v1/Policy#LogType
*/
class AuditConfig
{
    friend class AuditLogConfig;
    friend class Binding;
    friend class PolicyBuilder;

private:
    UtilsClass *ut = nullptr;
    FirebaseJson json;
    FirebaseJsonArray arr;

public:
    AuditConfig();
    ~AuditConfig();

    /**
     * Specifies a service that will be enabled for audit logging. 
     * For example, storage.googleapis.com, cloudsql.googleapis.com. allServices is a special value that covers all services.
     * 
     * @param service The string of service
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#Binding.FIELDS.members
     * 
    */
    void setService(const char *service);

    /** Add AuditLogConfig object 
     * @param config The pointer to the AuditLogConfig class data.
     * @param clear The boolean to clear AuditLogConfig data before adding.
    */
    void addAuditLogConfig(AuditLogConfig *config, bool clear = false);

    /**
     * Clear all AuditLogConfig data that were added. 
    */
    void clearAuditLogConfigs();

    /**
     * Clear all AuditConfig data. 
    */
    void clear();
};

/**
 * The class that provides the configuration to associates members with a role.
 * https://cloud.google.com/functions/docs/reference/rest/v1/Policy#Binding
*/
class Binding
{
    friend class PolicyBuilder;

private:
    UtilsClass *ut = nullptr;
    FirebaseJsonArray arr;
    FirebaseJson json;

public:
    Binding();
    ~Binding();

    /**
     * Add the identities requesting access for a Cloud Platform resource. 
     * 
     * @param member The string of member
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#Binding.FIELDS.members
     * 
    */
    void addMember(const char *member);

    /**
     * Specifies role that is assigned to members. For example, roles/viewer, roles/editor, or roles/owner.
     * 
     * @param role The string of role
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#Binding.FIELDS.role
     * 
    */
    void setRole(const char *role);

    /**
     * Specifies the condition that is associated with this binding.
     * If the condition evaluates to true, then this binding applies to the current request.
     * If the condition evaluates to false, then this binding does not apply to the current request. 
     * However, a different role binding might grant the same role to one or more of the members in this binding.
     * 
     * 
     * @param expression Textual representation of an expression in Common Expression Language syntax.
     * @param title Optional. Title for the expression, i.e. a short string describing its purpose. 
     * This can be used e.g. in UIs which allow to enter the expression.
     * @param description Optional. Description of the expression. 
     * This is a longer text which describes the expression, e.g. when hovered over it in a UI.
     * @param location Optional. String indicating the location of the expression for error reporting, e.g. a file name and a position in the file.
     * 
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#Expr
     * 
    */
    void setCondition(const char *expression = "", const char *title = "", const char *description = "", const char *location = "");

    /**
     * Clear all members that were added. 
    */
    void clearMembers();

    /**
     * Clear all Binding data. 
    */
    void clear();
};

/** The class that provides the Policy which is an Identity and Access Management (IAM) policy, 
 * which specifies access controls for Google Cloud resources.
 * 
 * A Policy is a collection of bindings. 
 * A binding binds one or more members to a single role. 
 * Members can be user accounts, service accounts, Google groups, and domains (such as G Suite). 
 * A role is a named list of permissions; each role can be an IAM predefined role or a user-created custom role.
 * 
 * For some types of Google Cloud resources, a binding can also specify a condition, 
 * which is a logical expression that allows access to a resource only if the expression evaluates to true. 
 * A condition can add constraints based on attributes of the request, the resource, or both.
 * 
 * https://cloud.google.com/functions/docs/reference/rest/v1/Policy
*/
class PolicyBuilder
{
    friend class Binding;
    friend class FB_Functions;
    friend class PolicyInfo;

private:
    UtilsClass *ut = nullptr;
    FirebaseJsonArray arr;
    FirebaseJsonArray arr2;
    FirebaseJson json;
    void _toString(std::string &s, bool prettify = false);

public:
    PolicyBuilder();
    ~PolicyBuilder();

    /**
     * Add the AuditConfig data. 
     * 
     * @param config The pointer to AuditConfig data.
     * @param clear The boolean to clear the AuditConfig data before adding.
     * 
    */
    void addAuditConfig(AuditConfig *config, bool clear = false);

    /**
     * Clear all AuditConfig that added in to the collection. 
     * 
    */
    void clearAuditConfigs();

    /**
     * Add the Binding data. 
     * 
     * @param binding The pointer to Binding class data.
     * @param clear The boolean to clear the Binding data before adding.
     * 
    */
    void addBinding(Binding *binding, bool clear = false);

    /**
     * Specifies the format of the policy.
     * 
     * @param v Valid values are 0, 1, and 3. Requests that specify an invalid value are rejected.
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#FIELDS.version
     * 
    */
    void setVersion(int v);

    /**
     * Set the ETag.
     * etag is used for optimistic concurrency control as a way to help prevent simultaneous updates 
     * of a policy from overwriting each other. 
     * 
     * It is strongly suggested that systems make use of the etag in the read-modify-write cycle to 
     * perform policy updates in order to avoid race conditions: An etag is returned in the response to getIamPolicy, 
     * and systems are expected to put that etag in the request to setIamPolicy to ensure that their change 
     * will be applied to the same version of the policy.
     * 
     * @param etag The string of etag
     * See more at https://cloud.google.com/functions/docs/reference/rest/v1/Policy#FIELDS.etag
     * 
    */
    void setETag(const char *etag);

    /**
     * Clear all Bindings in the collection.
     * 
    */
    void clearBindings();

    /**
     * Clear the Policy data.
     * 
    */
    void clear();

    /**
     * Serialize the Policy to String 
     * 
     * @param s The String object to get the serialized data.
     * @param prettify The boolean to format the serialized data with tabs
     * 
    */
    void toString(String &s, bool prettify = false);
};

#endif