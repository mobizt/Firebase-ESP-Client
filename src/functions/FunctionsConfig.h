/**
 * Google's Cloud Functions Config class, FunctionsConfig.h version 1.0.0
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created February 17, 2021
 * 
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2020, 2021 K. Suwatchai (Mobizt)
 * 
 * The MIT License (MIT)
 * Copyright (c) 2020, 2021 K. Suwatchai (Mobizt)
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

#ifndef _FB_FUNCTIONS_CONFIG_H_
#define _FB_FUNCTIONS_CONFIG_H_

#include <Arduino.h>
#include "Utils.h"
#include "session/FB_Session.h"
#include "PolicyBuilder.h"

class FunctionsConfig
{
    friend class FB_Functions;

private:
    FirebaseJson _funcCfg;
    std::string _projectId, _locationId, _bucketId, _entryPoint, _name, _httpsTriggerUrl;
    std::vector<std::string> _updateMask = std::vector<std::string>();
    UtilsClass *ut = nullptr;
    std::string _bucketSourcesPath = "";
    std::string _uploadArchiveFile = "";
    const uint8_t *_pgmArc = nullptr;
    size_t _pgmArcLen = 0;
    fb_esp_mem_storage_type _uploadArchiveStorageType = mem_storage_type_undefined;
    fb_esp_functions_sources_type _sourceType = functions_sources_type_undefined;
    fb_esp_functions_trigger_type _triggerType = fb_esp_functions_trigger_type_undefined;
    PolicyBuilder *_policy = nullptr;

    void addUpdateMasks(const char *key);
    void removeUpdateMasks(const char *key);

public:
    /**
     * Class object constructor.
     * This name excluded the project, location and function.
     * 
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param bucketId The Firebase storage bucket ID in the project.
    */
    FunctionsConfig(const char *projectId, const char *locationId, const char *bucketId);

    ~FunctionsConfig();

    /**
     * Set the Firebase project id
     * 
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
    */
    void setProjectId(const char *projectId);

    /**
     * Set the location of project.
     * 
     * @param locationId The project location.
    */
    void setLocationId(const char *locationId);

    /**
     * Set the Storage data bucket.
     * 
     * @param bucketId The Firebase storage bucket ID in the project.
    */
    void setBucketId(const char *bucketId);

    /**
     * Set a user-defined name of the function. Function names must be unique globally.
     * This name excluded the project, location and function.
     * 
     * @param name The string of function name.
    */
    void setName(const char *name);

    /**
     * Set User-provided description of a function.
     * 
     * @param description The string of function description.
    */
    void setDescription(const char *description);

    /**
     * Set the name of the function (as defined in source code) that will be executed. 
     * Defaults to the resource name suffix, if not specified. 
     * 
     * For backward compatibility, if function with given name is not found, then the system will try to use function named "function".
     * 
     * @param entry The string of function entry.
    */
    void setEntryPoint(const char *entry);

    /**
     * Set the runtime in which to run the function. 
     * Required when deploying a new function, optional when updating an existing function. 
     * For a complete list of possible choices, see the gcloud command reference.
     * https://cloud.google.com/sdk/gcloud/reference/functions/deploy#--runtime
     * 
     * @param runtime The string of function runtime.
    */
    void setRuntime(const char *runtime);

    /**
     * Set the function execution timeout. 
     * Execution is considered failed and can be terminated if the function is not completed at the end of the timeout period. 
     * Defaults to 60 seconds.
     * 
     * A duration in seconds with up to nine fractional digits, terminated by 's'. Example: "3.5s".
     * 
     * @param seconds The number of seconds for timeout.
    */
    void setTimeout(size_t seconds);

    /**
     * Set the amount of memory in MB available for a function.
     * 128, 256, 512, 1024, 2048 and 4096 MB 
     * Defaults to 256MB.
     * 
     * @param mb The number of MB.
    */
    void setAvailableMemoryMb(size_t mb);

    /**
     * Set the limit on the maximum number of function instances that may coexist at a given time.
     * In some cases, such as rapid traffic surges, Cloud Functions may, for a short period of time, create more instances 
     * than the specified max instances limit. 
     * If your function cannot tolerate this temporary behavior, you may want to factor in a safety margin and set 
     * a lower max instances value than your function can tolerate.
     * 
     * @param maxInstances The number of instances.
    */
    void setMaxInstances(size_t maxInstances);

    /**
     * Set the location of the function source code.
     * 
     * @param path The path of source code depends on the sourceType
     * The path is the relative path of zip archive in Firebase Storage bucket when sourceType is functions_sources_type_storage_bucket_archive
     * The path is the relative path of source code files in Firebase Storage bucket when sourceType is functions_sources_type_storage_bucket_sources
    *  The path is the local zip archive file path when sourceType is functions_sources_type_local_archive.
    *  The path is the source repository where a function is hosted when sourceType is functions_sources_type_repository.
     * @param sourceType The source types enum
     * functions_sources_type_storage_bucket_archive
     * functions_sources_type_storage_bucket_sources
     * functions_sources_type_local_archive
     * functions_sources_type_repository
     * @param storageType The local storage types enum
     * mem_storage_type_flash
     * mem_storage_type_sd
    */
    void setSource(const char *path, fb_esp_functions_sources_type sourceType, fb_esp_mem_storage_type storageType = mem_storage_type_undefined);

    /**
     * Set the location of the function source code to the flash zip archive data.
     * 
     * @param pgmArchiveData The zip archive data array.
     * @param len The size of data in bytes.
    */
    void setSource(const uint8_t *pgmArchiveData, size_t len);

    /**
     * Add Labels associated with this Cloud Function.
     * 
     * @param key The string of label key.
     * @param value The string of label value.
    */
    void addLabel(const char *key, const char *value);

    /**
     * Clear the Labels
    */
    void clearLabels();

    /**
     * Add the Environment variables that shall be available during function execution.
     * 
     * @param key The string of var key.
     * @param value The string of var value.
     * 
    */
    void addEnvironmentVariable(const char *key, const char *value);

    /**
     * Clear the Envireonmebt variables.
    */
    void clearEnvironmentVariables();

    /**
    * Add the Build environment variables that shall be available during build time.
    * 
    * @param key The string of var key.
    * @param value The string of var value.
   */
    void addBuildEnvironmentVariable(const char *key, const char *value);

    /**
     * Clear the Build envireonmebt variables.
    */
    void clearBuildEnvironmentVariables();

    /**
     * Set the VPC Network that this cloud function can connect to. 
     * It can be either the fully-qualified URI, or the short name of the network resource. 
     * If the short network name is used, the network must belong to the same project. 
     * Otherwise, it must belong to a project within the same organization. 
     * The format of this field is either projects/{project}/global/networks/{network} or {network}, 
     * where {project} is a project id where the network is defined, and {network} is the short name of the network.
     * 
     * @param network The string of network.
    */
    void setNetwork(const char *network);

    /**
     * The VPC Network Connector that this cloud function can connect to. 
     * It can be either the fully-qualified URI, or the short name of the network connector resource. 
     * The format of this field is projects/{project-id}/locations/{location-id}/connectors/{connector}
     * 
     * This field is mutually exclusive with network field and will eventually replace it.
     * See the VPC documentation for more information on connecting Cloud projects.
     * https://cloud.google.com/compute/docs/vpc
     * 
     * @param vpcConnector The string of vpcConnector.
    */
    void setVpcConnector(const char *vpcConnector);

    /**
     * The egress settings for the connector, controlling what traffic is diverted through it.
     * 
     * @param e The VpcConnectorEgressSettings enum.
     * VPC_CONNECTOR_EGRESS_SETTINGS_UNSPECIFIED
     * PRIVATE_RANGES_ONLY
     * ALL_TRAFFIC
    */
    void setVpcConnectorEgressSettings(const char *e);

    /**
     * The ingress settings for the function, controlling what traffic can reach it.
     * 
     * @param settings The IngressSettings enum.
     * INGRESS_SETTINGS_UNSPECIFIED
     * ALLOW_ALL
     * ALLOW_INTERNAL_ONLY
     * ALLOW_INTERNAL_AND_GCLB
    */
    void setIngressSettings(const char *settings);

    /**
     * Set a source that fires events in response to a condition in another service.
     * 
     * @param eventType Required. The type of event to observe. 
     * For example: providers/cloud.storage/eventTypes/object.change and providers/cloud.pubsub/eventTypes/topic.publish.
     * Event types match pattern providers/{provider}/eventTypes /{?}.{?}. The pattern contains: 
     * 1. namespace: For example, cloud.storage and google.firebase.analytics.
     * 2. resource type: The type of resource on which event occurs. For example, the Google Cloud Storage API includes the type object.
     * 3. action: The action that generates the event. For example, action for a Google Cloud Storage Object is 'change'. These parts are lower case.
     * 
     * @param resource Required. The resource(s) from which to observe events, for example, projects/_/buckets/myBucket.
     * Not all syntactically correct values are accepted by all services. For example:
     * 1. The authorization model must support it. Google Cloud Functions only allows EventTriggers to be deployed that observe resources in the same project as the CloudFunction.
     * 2. The resource type must match the pattern expected for an eventType. For example, an EventTrigger that has an eventType of "google.pubsub.topic.publish" should have a resource that matches Google Cloud Pub/Sub topics.
     * Additionally, some services may support short names when creating an EventTrigger. These will always be returned in the normalized "long" format.
     * 
     * @param service The hostname of the service that should be observed. If no string is provided, the default service implementing the API will be used. 
     * For example, storage.googleapis.com is the default for all event types in the google.storage namespace.
     * @param failurePolicy Specifies policy for failed executions.
    */
    void setEventTrigger(const char *eventType, const char *resource, const char *service = "", const char *failurePolicy = "");

    /**
     * Set the IAM policy.
     * 
     * @param policy The PolicyBuilder class data.
     * For Policy, see https://cloud.google.com/functions/docs/reference/rest/v1/Policy
    */
    void setIamPolicy(PolicyBuilder *policy);

    /**
     * Get the HTTPS trigger Url of Cloud Function.
    */
    String getTriggerUrl();

    /**
     * Clear the FunctionsConfig class data.
    */
    void clear();
};

#endif