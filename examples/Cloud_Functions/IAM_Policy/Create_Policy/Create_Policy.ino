
/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

/** This example shows how to create IAM Policy used in Cloud Function creation.
 * The helper class PolicyBuilder used in policy JSON object generation
*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

//We need to define the PolicyBuilder, Binding AuditConfig, and AuditLogConfig data to keep the function and triggers configuaration and IAM policy.
//These objects should declare as global objects or static to prevent the stack overflow.
PolicyBuilder policy;
Binding binding;
AuditConfig audit_config;
AuditLogConfig audit_log_config;

void setup()
{

    Serial.begin(115200);

    Firebase.begin(&config, &auth);

    //This will create the policy JSON object as in this document
    //https://cloud.google.com/iam/docs/reference/rest/v1/Policy

    policy.setETag("BwWWja0YfJA=");
    policy.setVersion(3);

    binding.setRole("roles/resourcemanager.organizationAdmin");
    binding.addMember("user:mike@example.com");
    binding.addMember("group:admins@example.com");
    binding.addMember("domain:google.com");
    binding.addMember("serviceAccount:my-project-id@appspot.gserviceaccount.com");
    policy.addBinding(&binding, true);

    binding.setRole("roles/resourcemanager.organizationViewer");
    binding.addMember("user:eve@example.com");
    binding.setCondition("request.time < timestamp('2020-10-01T00:00:00.000Z", "expirable access", "Does not grant access after Sep 2020");
    policy.addBinding(&binding, true);

    audit_config.setService("allServices");

    audit_log_config.setLogType("DATA_READ");
    audit_log_config.addexemptedMembers("user:jose@example.com");
    audit_config.addAuditLogConfig(&audit_log_config, true);

    audit_log_config.setLogType("DATA_WRITE");
    audit_config.addAuditLogConfig(&audit_log_config, true);

    audit_log_config.setLogType("ADMIN_READ");
    audit_config.addAuditLogConfig(&audit_log_config, true);

    policy.addAuditConfig(&audit_config, true);

    audit_config.setService("sampleservice.googleapis.com");

    audit_log_config.setLogType("DATA_READ");
    audit_config.addAuditLogConfig(&audit_log_config, true);

    audit_log_config.setLogType("DATA_WRITE");
    audit_log_config.addexemptedMembers("user:aliya@example.com");
    audit_config.addAuditLogConfig(&audit_log_config, true);

    policy.addAuditConfig(&audit_config, true);

    String policyStr;
    policy.toString(policyStr, true);
    Serial.println(policyStr);
}

void loop()
{
}
