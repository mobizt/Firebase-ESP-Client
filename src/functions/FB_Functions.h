/**
 * Google's Cloud Functions class, Functions.h version 1.0.4
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

#ifndef _FB_FUNCTIONS_H_
#define _FB_FUNCTIONS_H_

#include <Arduino.h>
#include "Utils.h"
#include "FunctionsConfig.h"

class FB_Functions
{
    friend class Firebase_ESP_Client;

public:
    FB_Functions();
    ~FB_Functions();

    /** Synchronously invokes a deployed Cloud Function. 
     * To be used for testing purposes as very limited traffic is allowed. 
     * For more information on the actual limits, refer to Rate Limits.
     * https://cloud.google.com/functions/quotas#rate_limits
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param functionId The name of function.
     * @param data The Input to be passed to the function.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool callFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *data);

    /** Creates a new function. 
     * If a function with the given name already exists in the specified project, 
     * the long running operation will return ALREADY_EXISTS error.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param config The pointer to FunctionsConfig object that encapsulates the function and triggers configurationston.
     * @param callback The callback function to get the Cloud Function creation status.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationCallback callback = NULL);

    /** Creates a new function. 
     * If a function with the given name already exists in the specified project, 
     * the long running operation will return ALREADY_EXISTS error.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param config The pointer to FunctionsConfig object that encapsulates the function and triggers configurationston.
     * @param statusInfo The pointer to FunctionsOperationStatusInfo data to get the Cloud Function creation status later.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationStatusInfo *statusInfo);

    /** Updates existing function.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param functionId The name of function.
     * @param patchData The pointer to FunctionsConfig object that encapsulates the function and triggers configurationston.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool patchFunction(FirebaseData *fbdo, const char *functionId, FunctionsConfig *patchData);

    /** Sets the IAM access control policy on the specified function. Replaces any existing policy.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param functionId The name of function.
     * @param policy The pointer to PolicyBuilder data concapsulates the policy configuration.
     * The complete policy to be applied to the resource. 
     * @param updateMask A FieldMask specifying which fields of the policy to modify. Only the fields in the mask will be modified. If no mask is provided, the following default mask is used:
     * paths: "bindings, etag"
     * A comma-separated list of fully qualified names of fields. Example: "user.displayName,photo"
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool setIamPolicy(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, PolicyBuilder *policy, const char *updateMask = "");

    /** Gets the IAM access control policy for a function. 
     * Returns an empty policy if the function exists and does not have a policy set.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param functionId The name of function.
     * @param version Optional. The policy format version to be returned.
     * Valid values are 0, 1, and 3. Requests specifying an invalid value will be rejected.
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool getIamPolicy(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *version = "");

    /** Returns a function with the given name from the requested project.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param functionId The name of function.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool getFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId);

    /** Deletes a function with the given name from the specified project. 
     * If the given function is used by some trigger, the trigger will be updated to remove this function.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param functionId The name of function.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool deleteFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId);

    /** Returns a signed URL for downloading deployed function source code. 
     * The URL is only valid for a limited period and should be used within minutes after generation. 
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param functionId The name of function.
     * @param versionId The optional version of function. If not set, default, current version is used.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool generateDownloadUrl(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *versionId = "");

    /** Returns a signed URL for uploading a function source code.  
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool generateUploadUrl(FirebaseData *fbdo, const char *projectId, const char *locationId);

    /** Returns a list of functions that belong to the requested project.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param projectId The Firebase project id (only the name without the firebaseio.com).
     * @param locationId The project location.
     * @param pageSize Maximum number of functions to return per call.
     * @param pageToken The value returned by the last ListFunctionsResponse; indicates that this is a continuation 
     * of a prior functions.list call, and that the system should return the next page of data.
     * 
     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool listFunctions(FirebaseData *fbdo, const char *projectId, const char *locationId, size_t pageSize, const char *pageToken = "");

    /** Returns a function with the given name from the requested project.
     * 
     * @param fbdo The pointer to Firebase Data Object.
     * @param filter The Firebase project id (only the name without the firebaseio.com).
     * A filter for matching the requested operations.
     * The supported formats of filter are: 
     * To query for a specific function:
     * project:*,location:*,function:*
     * To query for all of the latest operations for a project:
     * project:*,latest:true
     * @param pageSize The maximum number of records that should be returned.
     * Requested page size cannot exceed 100. If not set, the default page size is 100
     * @param pageToken Token identifying which result to start with, which is returned by a previous list call.

     * @return Boolean value, indicates the success of the operation. 
     * 
     * @note Use FirebaseData.payload() to get the returned payload.
     * 
     * This function requires OAuth2.0 authentication.
     * 
    */
    bool listOperations(FirebaseData *fbdo, const char *filter, int pageSize, const char *pageToken);

private:
    fb_esp_functions_status _function_status = fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;
    UtilsClass *ut = nullptr;
    unsigned long _lasPollMs = 0;
#if defined(ESP32)
    TaskHandle_t function_check_task_handle = NULL;
#endif
    bool _creation_task_enable = false;
    size_t _deployIndex = 0;
    std::vector<fb_esp_deploy_task_info_t> _deployTasks = std::vector<fb_esp_deploy_task_info_t>();

    void begin(UtilsClass *u);
    void rescon(FirebaseData *fbdo, const char *host);
    bool connect(FirebaseData *fbdo, const char *host = "");
    void addCreationTask(FirebaseData *fbdo, FunctionsConfig *config, bool patch, fb_esp_functions_creation_step step, fb_esp_functions_creation_step nextStep, FunctionsOperationCallback callback, FunctionsOperationStatusInfo *statusInfo);
    bool sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req);
    bool functions_sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req);
    bool handleResponse(FirebaseData *fbdo);
    bool uploadSources(FirebaseData *fbdo, FunctionsConfig *config);
    bool deploy(FirebaseData *fbdo, const char *functionId, FunctionsConfig *config, bool patch);
    bool createFunctionInt(FirebaseData *fbdo, const char *functionId, FunctionsConfig *config, bool patch, FunctionsOperationCallback cb = NULL, FunctionsOperationStatusInfo *info = nullptr);
    bool uploadFile(FirebaseData *fbdo, const char *uploadUrl, const char *filePath, fb_esp_mem_storage_type storageType);
    bool uploadPGMArchive(FirebaseData *fbdo, const char *uploadUrl, const uint8_t *pgmArc, size_t pgmArcLen);
    void sendCallback(FirebaseData *fbdo, fb_esp_functions_operation_status status, const char *message, FunctionsOperationCallback cb, FunctionsOperationStatusInfo *info);

#if defined(ESP32)
    void runDeployTask(const char *taskName);
#elif defined(ESP8266)
    void runDeployTask();
#endif
};

#endif