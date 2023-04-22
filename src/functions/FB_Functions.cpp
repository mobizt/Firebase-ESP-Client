#include "Firebase_Client_Version.h"
#if !FIREBASE_CLIENT_VERSION_CHECK(40310)
#error "Mixed versions compilation."
#endif

/**
 * Google's Cloud Functions class, Functions.cpp version 1.1.22
 *
 * This library supports Espressif ESP8266, ESP32 and RP2040 Pico
 *
 * Created April 5, 2023
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#include "FirebaseFS.h"

#ifdef ENABLE_FB_FUNCTIONS

#ifndef _FB_FUNCTIONS_CPP_
#define _FB_FUNCTIONS_CPP_

#include "FB_Functions.h"

FB_Functions::FB_Functions()
{
}

FB_Functions::~FB_Functions()
{
}

void FB_Functions::makeRequest(struct fb_esp_functions_req_t &req, fb_esp_functions_request_type type,
                               MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
{
    req.requestType = type;
    req.projectId = projectId;
    req.locationId = locationId;
    req.functionId = functionId;
}

bool FB_Functions::mCallFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId,
                                 MB_StringPtr functionId, MB_StringPtr data)
{
    struct fb_esp_functions_req_t req;

    makeRequest(req, fb_esp_functions_request_type_call, projectId, locationId, functionId);

    req.payload = data;
    fbdo->initJson();

    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_pgm_str_67 /* "data" */, req.payload);

    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);

    return sendRequest(fbdo, &req);
}

void FB_Functions::addCreationTask(FirebaseData *fbdo, FunctionsConfig *config, bool patch,
                                   fb_esp_functions_creation_step step, fb_esp_functions_creation_step nextStep,
                                   FunctionsOperationCallback callback, FunctionsOperationStatusInfo *statusInfo)
{
    struct fb_esp_deploy_task_info_t taskInfo;
    taskInfo.step = step;
    taskInfo.nextStep = nextStep;
    taskInfo.fbdo = fbdo;
    taskInfo.config = config;
    taskInfo.projectId = config->_projectId;
    taskInfo.locationId = config->_locationId;
    taskInfo.functionId = config->_name;
    taskInfo.callback = callback;
    taskInfo.patch = patch;
    taskInfo.httpsTriggerUrl = config->_httpsTriggerUrl;
    if (statusInfo)
    {
        taskInfo.statusInfo = statusInfo;
        taskInfo.statusInfo->status = fb_esp_functions_operation_status_deploy_in_progress;
    }
    if (config->_policy)
    {
        taskInfo.setPolicy = true;
        taskInfo.policy = config->_policy->raw();
    }
    _deployTasks.push_back(taskInfo);
    fbdo->session.long_running_task++;

    _creation_task_enable = true;

    // Allow running in FreeRTOS loop task?, add to loop task.
    if (Signer.config->internal.deploy_loop_task_enable)
    {
        // The first task?, running in loop task by default
        if (_deployTasks.size() == 1)
        {

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
            runDeployTask(pgm2Str(fb_esp_func_pgm_str_1 /* "deployTask" */));
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
            runDeployTask();
#endif
        }
    }
    else // User intends to run task manually, run immediately.
        mDeployTasks();
}

bool FB_Functions::createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationStatusInfo *statusInfo)
{
    return createFunctionInt(fbdo, toStringPtr(config->_name), config, false, NULL, statusInfo);
}

bool FB_Functions::createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationCallback callback)
{
    return createFunctionInt(fbdo, toStringPtr(config->_name), config, false, callback, nullptr);
}

bool FB_Functions::mPatchFunction(FirebaseData *fbdo, MB_StringPtr functionId, FunctionsConfig *patchData)
{
    return createFunctionInt(fbdo, functionId, patchData, true, NULL, nullptr);
}

bool FB_Functions::createFunctionInt(FirebaseData *fbdo, MB_StringPtr functionId,
                                     FunctionsConfig *config, bool patch, FunctionsOperationCallback cb,
                                     FunctionsOperationStatusInfo *info)
{
    Signer.config->internal.deploy_loop_task_enable = true;

    if (patch)
        fbdo->session.cfn.cbInfo.functionId = functionId;
    else
    {
        MB_String _functionId = functionId;
        if (_functionId.length() > 0)
            fbdo->session.cfn.cbInfo.functionId = functionId;
        else if (config->_entryPoint.length() > 0)
            fbdo->session.cfn.cbInfo.functionId = config->_entryPoint;
    }

    if (config->_sourceType == functions_sources_type_local_archive ||
        config->_sourceType == functions_sources_type_flash_data)
    {
        if (config->_sourceType == functions_sources_type_local_archive)
        {
            if (config->_uploadArchiveStorageType == mem_storage_type_undefined)
                config->_uploadArchiveStorageType = mem_storage_type_flash;

            fbdo->session.cfn.filepath = config->_uploadArchiveFile;
            fbdo->session.cfn.storageType = config->_uploadArchiveStorageType;

            int sz = Signer.mbfs->open(config->_uploadArchiveFile, mbfs_type config->_uploadArchiveStorageType,
                                       mb_fs_open_mode_read);

            if (sz < 0)
            {
                fbdo->session.response.code = MB_FS_ERROR_FILE_IO_ERROR;
                fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                fbdo->session.cfn.cbInfo.errorMsg = fbdo->errorReason().c_str();
                sendCallback(fbdo, cb, info);
                return false;
            }

            fbdo->session.cfn.fileSize = sz;
        }
        // Fix in ESP32 core 2.0.x
        Signer.mbfs->close(mbfs_type config->_uploadArchiveStorageType);

        addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_gen_upload_url,
                        fb_esp_functions_creation_step_upload_zip_file, cb, info);
        return true;
    }
    else if (config->_sourceType == functions_sources_type_storage_bucket_sources)
    {

        addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_upload_source_files,
                        fb_esp_functions_creation_step_deploy, cb, info);
        return true;
    }
    else
    {
        bool ret = false;
        fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
        fbdo->session.cfn.cbInfo.errorMsg.clear();
        sendCallback(fbdo, cb, info);
        MB_String _functionId = functionId;
        ret = deploy(fbdo, _functionId.c_str(), config, patch);
        fbdo->session.cfn.cbInfo.triggerUrl = config->_httpsTriggerUrl;
        if (info)
            info->triggerUrl = config->_httpsTriggerUrl;

        if (!ret)
        {
            fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
            fbdo->session.cfn.cbInfo.errorMsg = fbdo->session.error.c_str();
            sendCallback(fbdo, cb, info);
        }

        if (ret && config->_policy)
            addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_polling_status,
                            fb_esp_functions_creation_step_set_iam_policy, cb, info);
        else if (ret)
            addCreationTask(fbdo, config, patch, fb_esp_functions_creation_step_polling_status,
                            fb_esp_functions_creation_step_idle, cb, info);
        return ret;
    }

    return true;
}

bool FB_Functions::uploadSources(FirebaseData *fbdo, FunctionsConfig *config)
{
    fbdo->initJson();

    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_2 /* "projectId" */, config->_projectId);
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_3 /* "location" */, config->_locationId);
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_5 /* "zip" */, MB_String(fb_esp_func_pgm_str_8 /* "tmp.zip" */));
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_6 /* "accessToken" */, Signer.getToken());
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_7 /* "path" */, config->_bucketSourcesPath);

    struct fb_esp_functions_req_t req;

    req.requestType = fb_esp_functions_request_type_upload_bucket_sources;

    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);

    URLHelper::addFunctionsHost(req.host, config->_locationId, config->_projectId,
                                MB_String(fb_esp_func_pgm_str_4) /* "autozip" */, false);

    return sendRequest(fbdo, &req);
}

bool FB_Functions::deploy(FirebaseData *fbdo, const char *functionId, FunctionsConfig *config, bool patch)
{

    struct fb_esp_functions_req_t req;

    if (patch)
        req.updateMask = &config->_updateMask;

    makeRequest(req, patch ? fb_esp_functions_request_type_patch : fb_esp_functions_request_type_create,
                toStringPtr(config->_projectId), toStringPtr(config->_locationId),
                toStringPtr(functionId));

    MB_String str;

    fbdo->initJson();

    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_2 /* "projectId" */, config->_projectId);
    URLHelper::host2Url(str, Signer.config->database_url);
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_9 /* "databaseURL" */, str);

    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_10 /* "storageBucket" */, config->_bucketId);
    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_11 /* "locationId" */, config->_locationId);

    str = fb_esp_func_pgm_str_12;  // "environmentVariables"
    str += fb_esp_pgm_str_1;       // "/"
    str += fb_esp_func_pgm_str_13; // "FIREBASE_CONFIG"

    JsonHelper::addString(&config->_funcCfg, str.c_str(), fbdo->session.jsonPtr->raw());
    JsonHelper::clear(fbdo->session.jsonPtr);

    str = fb_esp_func_pgm_str_12;  // "environmentVariables"
    str += fb_esp_pgm_str_1;       // "/"
    str += fb_esp_func_pgm_str_14; // "GCLOUD_PROJECT"

    JsonHelper::addString(&config->_funcCfg, str.c_str(), config->_projectId);

    config->_httpsTriggerUrl.clear();

    if (config->_triggerType == fb_esp_functions_trigger_type_https)
    {
        str.clear();

        URLHelper::addFunctionsHost(str, config->_locationId, config->_projectId, config->_entryPoint, true);
        JsonHelper::addString(&config->_funcCfg, fb_esp_func_pgm_str_15 /* "httpsTrigger/url" */, str);
        config->_httpsTriggerUrl = str;
        str.clear();
    }

    JsonHelper::toString(&config->_funcCfg, req.payload, true);
    return sendRequest(fbdo, &req);
}

void FB_Functions::sendCallback(FirebaseData *fbdo, FunctionsOperationCallback cb, FunctionsOperationStatusInfo *info)
{
    if (fbdo->session.cfn.last_status == fbdo->session.cfn.cbInfo.status)
        return;

    fbdo->session.cfn.last_status = fbdo->session.cfn.cbInfo.status;

    if (cb)
        cb(fbdo->session.cfn.cbInfo);

    if (info)
    {
        info->errorMsg = fbdo->session.cfn.cbInfo.errorMsg;
        info->status = fbdo->session.cfn.cbInfo.status;
        info->functionId = fbdo->session.cfn.cbInfo.functionId;
        info->triggerUrl = fbdo->session.cfn.cbInfo.triggerUrl;
    }
}

bool FB_Functions::mSetIamPolicy(FirebaseData *fbdo, MB_StringPtr projectId,
                                 MB_StringPtr locationId, MB_StringPtr functionId,
                                 PolicyBuilder *policy, MB_StringPtr updateMask)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_set_iam_policy, projectId, locationId, functionId);
    fbdo->initJson();

    if (policy)
        JsonHelper::addObject(fbdo->session.jsonPtr, fb_esp_func_pgm_str_16 /* "policy" */, &policy->json, true);

    JsonHelper::addString(fbdo->session.jsonPtr, fb_esp_pgm_str_70 /* "updateMask" */, MB_String(updateMask));
    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetIamPolicy(FirebaseData *fbdo, MB_StringPtr projectId,
                                 MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr version)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_get_iam_policy, projectId, locationId, functionId);
    req.policyVersion = version;
    fbdo->initJson();
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_get, projectId, locationId, functionId);
    _function_status = fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;
    bool ret = sendRequest(fbdo, &req);
    if (ret)
    {
        fbdo->initJson();
        JsonHelper::setData(fbdo->session.jsonPtr, fbdo->session.cfn.payload, false);
        if (JsonHelper::parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, fb_esp_func_pgm_str_17 /* "status" */))
        {
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), fb_esp_func_pgm_str_18 /* "CLOUD_FUNCTION_STATUS_UNSPECIFIED" */) == 0)
                _function_status = fb_esp_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), fb_esp_func_pgm_str_19 /* "ACTIVE" */) == 0)
                _function_status = fb_esp_functions_status_ACTIVE;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), fb_esp_func_pgm_str_20 /* "OFFLINE" */) == 0)
                _function_status = fb_esp_functions_status_OFFLINE;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), fb_esp_func_pgm_str_21 /* "DEPLOY_IN_PROGRESS" */) == 0)
                _function_status = fb_esp_functions_status_DEPLOY_IN_PROGRESS;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), fb_esp_func_pgm_str_22 /* "DELETE_IN_PROGRESS" */) == 0)
                _function_status = fb_esp_functions_status_DELETE_IN_PROGRESS;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), fb_esp_func_pgm_str_23 /*  "UNKNOWN" */) == 0)
                _function_status = fb_esp_functions_status_UNKNOWN;
        }
    }
    return ret;
}

bool FB_Functions::mListFunctions(FirebaseData *fbdo, MB_StringPtr projectId,
                                  MB_StringPtr locationId, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_list, projectId, locationId, toStringPtr(""));
    req.pageSize = atoi(stringPtr2Str(pageSize));
    req.pageToken = pageToken;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mListOperations(FirebaseData *fbdo, MB_StringPtr filter, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_list_operations;
    req.filter = filter;
    req.pageSize = atoi(stringPtr2Str(pageSize));
    req.pageToken = pageToken;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mDeleteFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_delete, projectId, locationId, functionId);
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateDownloadUrl(FirebaseData *fbdo, MB_StringPtr projectId,
                                        MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr versionId)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_gen_download_url, projectId, locationId, functionId);
    fbdo->initJson();
    JsonHelper::addNumberString(fbdo->session.jsonPtr, fb_esp_func_pgm_str_24 /* "versionId" */, MB_String(versionId));
    JsonHelper::toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateUploadUrl(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId)
{
    struct fb_esp_functions_req_t req;
    makeRequest(req, fb_esp_functions_request_type_gen_upload_url, projectId, locationId, toStringPtr(""));
    return sendRequest(fbdo, &req);
}

bool FB_Functions::uploadFile(FirebaseData *fbdo, const char *uploadUrl, const char *filePath,
                              fb_esp_mem_storage_type storageType)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_upload;
    URLHelper::addPath(req.filePath, MB_String(filePath));
    req.storageType = storageType;

    struct fb_esp_url_info_t info;
    URLHelper::parse(Signer.mbfs, uploadUrl, info);
    req.host = info.host;
    req.uri = fb_esp_pgm_str_1; // "/"
    req.uri += info.uri;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::uploadPGMArchive(FirebaseData *fbdo, const char *uploadUrl, const uint8_t *pgmArc, size_t pgmArcLen)
{
    struct fb_esp_functions_req_t req;
    req.requestType = fb_esp_functions_request_type_pgm_upload;
    req.pgmArc = pgmArc;
    req.pgmArcLen = pgmArcLen;

    struct fb_esp_url_info_t info;
    URLHelper::parse(Signer.mbfs, uploadUrl, info);
    req.host = info.host;
    req.uri = fb_esp_pgm_str_1; // "/"
    req.uri += info.uri;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req)
{
    if (fbdo->tcpClient.reserved)
        return false;

    fbdo->session.http_code = 0;

    if (!Signer.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

#ifdef ENABLE_RTDB
    if (fbdo->session.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect() || !Signer.tokenReady())
        return false;

    if (Signer.config->internal.fb_processing)
        return false;

    Signer.config->internal.fb_processing = true;
    fbdo->clear();
    connect(fbdo, req->host.c_str());

    bool ret = functions_sendRequest(fbdo, req);

    if (!ret)
        fbdo->closeSession();

    return ret;
}

bool FB_Functions::functions_sendRequest(FirebaseData *fbdo, struct fb_esp_functions_req_t *req)
{
    bool post = false;
    fbdo->session.cfn.requestType = req->requestType;

    MB_String header;
    fb_esp_request_method method = http_undefined;
    if (req->requestType == fb_esp_functions_request_type_get_iam_policy ||
        req->requestType == fb_esp_functions_request_type_list_operations ||
        req->requestType == fb_esp_functions_request_type_get ||
        req->requestType == fb_esp_functions_request_type_get_iam_policy ||
        req->requestType == fb_esp_functions_request_type_list)
        method = http_get;
    else if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources ||
             req->requestType == fb_esp_functions_request_type_call ||
             req->requestType == fb_esp_functions_request_type_create ||
             req->requestType == fb_esp_functions_request_type_gen_download_url ||
             req->requestType == fb_esp_functions_request_type_gen_upload_url ||
             req->requestType == fb_esp_functions_request_type_set_iam_policy ||
             req->requestType == fb_esp_functions_request_type_test_iam_policy)
        method = http_post;
    else if (req->requestType == fb_esp_functions_request_type_patch)
        method = http_patch;
    else if (req->requestType == fb_esp_functions_request_type_delete)
        method = http_delete;
    else if (req->requestType == fb_esp_functions_request_type_upload ||
             req->requestType == fb_esp_functions_request_type_pgm_upload)
        method = http_put;

    if (method != http_undefined)
        post = HttpHelper::addRequestHeaderFirst(header, method);

    if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources ||
        req->requestType == fb_esp_functions_request_type_upload ||
        req->requestType == fb_esp_functions_request_type_pgm_upload)
        header += req->uri;
    else if (req->requestType == fb_esp_functions_request_type_list_operations)
    {
        header += fb_esp_func_pgm_str_25; // "/v1/operations"
        bool hasParam = false;

        URLHelper::addParam(header, fb_esp_func_pgm_str_26 /* "filter=" */, req->filter, hasParam);

        if (req->pageSize > 0)
            URLHelper::addParam(header, fb_esp_pgm_str_63 /* "pageSize" */, MB_String(req->pageSize), hasParam);

        URLHelper::addParam(header, fb_esp_pgm_str_65 /* "pageToken" */, req->pageToken, hasParam);
    }
    else
    {
        URLHelper::addGAPIv1Path(header);

        header += req->projectId.length() == 0 ? Signer.config->service_account.data.project_id : req->projectId;

        header += fb_esp_func_pgm_str_27; // "/locations/"
        if (req->locationId.length() > 0)
            header += req->locationId;

        header += fb_esp_func_pgm_str_28; // "/functions"

        if (req->requestType == fb_esp_functions_request_type_list)
        {
            bool hasParam = false;

            if (req->pageSize > 0)
                URLHelper::addParam(header, fb_esp_pgm_str_63 /* "pageSize" */, MB_String(req->pageSize), hasParam);

            URLHelper::addParam(header, fb_esp_pgm_str_65 /* "pageToken" */, req->pageToken, hasParam);
        }

        if (req->requestType == fb_esp_functions_request_type_patch ||
            req->requestType == fb_esp_functions_request_type_get_iam_policy ||
            req->requestType == fb_esp_functions_request_type_gen_download_url ||
            req->requestType == fb_esp_functions_request_type_delete ||
            req->requestType == fb_esp_functions_request_type_get ||
            req->requestType == fb_esp_functions_request_type_call ||
            req->requestType == fb_esp_functions_request_type_set_iam_policy)
        {
            header += fb_esp_pgm_str_1; // "/"
            if (req->functionId.length() > 0)
                header += req->functionId;
            if (req->requestType == fb_esp_functions_request_type_call)
                header += fb_esp_func_pgm_str_29; // ":call"
            else if (req->requestType == fb_esp_functions_request_type_set_iam_policy)
                header += fb_esp_func_pgm_str_30; // ":setIamPolicy"
            else if (req->requestType == fb_esp_functions_request_type_gen_download_url)
                header += fb_esp_func_pgm_str_31; // ":generateDownloadUrl"
            else if (req->requestType == fb_esp_functions_request_type_get_iam_policy)
            {
                header += fb_esp_func_pgm_str_32; // ":getIamPolicy"
                bool hasParam = false;
                URLHelper::addParam(header, fb_esp_func_pgm_str_33 /* "options.requestedPolicyVersion" */,
                                    req->policyVersion, hasParam);
            }
            else if (req->requestType == fb_esp_functions_request_type_patch)
            {
                bool hasParam = false;

                if (req->updateMask->size() > 0)
                {
                    for (size_t i = 0; i < req->updateMask->size(); i++)
                        URLHelper::addParam(header, fb_esp_func_pgm_str_34 /* "updateMask=" */, (*req->updateMask)[i], hasParam);
                }
            }
        }
        else if (req->requestType == fb_esp_functions_request_type_gen_upload_url)
            header += fb_esp_func_pgm_str_35; // :generateUploadUrl"
    }

    HttpHelper::addRequestHeaderLast(header);

    if (post)
    {
        if (req->payload.length() > 0)
            HttpHelper::addContentTypeHeader(header, fb_esp_pgm_str_62 /* "application/json" */);
        HttpHelper::addContentLengthHeader(header, req->payload.length());
    }

    if (req->requestType == fb_esp_functions_request_type_upload ||
        req->requestType == fb_esp_functions_request_type_pgm_upload)
    {
        HttpHelper::addContentTypeHeader(header, fb_esp_func_pgm_str_36 /* "application/zip" */);
        size_t len = 0;
        if (req->requestType == fb_esp_functions_request_type_pgm_upload)
            len = req->pgmArcLen;
        else if (req->requestType == fb_esp_functions_request_type_upload)
            len = fbdo->session.cfn.fileSize;

        HttpHelper::addContentLengthHeader(header, len);
        header += fb_esp_func_pgm_str_37; // "x-goog-content-length-range: 0,104857600"
        HttpHelper::addNewLine(header);
        HttpHelper::addHostHeader(header, req->host.c_str());
    }
    else
    {

        if (req->requestType == fb_esp_functions_request_type_upload_bucket_sources)
            HttpHelper::addHostHeader(header, req->host.c_str());
        else
            HttpHelper::addGAPIsHostHeader(header, fb_esp_func_pgm_str_38 /* "cloudfunctions." */);

        if (req->requestType != fb_esp_functions_request_type_upload_bucket_sources)
        {
            if (!Signer.config->signer.test_mode)
            {
                HttpHelper::addAuthHeaderFirst(header, Signer.getTokenType());
                header += Signer.getToken();
                HttpHelper::addNewLine(header);
            }
        }
    }

    HttpHelper::addUAHeader(header);
    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    HttpHelper::addConnectionHeader(header, keepAlive);
    HttpHelper::getCustomHeaders(header, Signer.config->signer.customHeaders);
    HttpHelper::addNewLine(header);

    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    fbdo->tcpClient.send(header.c_str());
    if (fbdo->session.response.code < 0)
        return false;

    if (fbdo->session.response.code > 0 && req->payload.length() > 0)
        fbdo->tcpClient.send(req->payload.c_str());

    header.clear();
    req->payload.clear();

    if (fbdo->session.response.code > 0)
    {
        fbdo->session.connected = true;
        if (req->requestType == fb_esp_functions_request_type_upload)
        {
            // Fix in ESP32 core 2.0.x
            Signer.mbfs->open(fbdo->session.cfn.filepath, mbfs_type fbdo->session.cfn.storageType, mb_fs_open_mode_read);
            fbdo->session.cfn.filepath.clear();
            int available = Signer.mbfs->available(mbfs_type fbdo->session.cfn.storageType);
            int bufLen = Utils::getUploadBufSize(Signer.config, fb_esp_con_mode_functions);
            uint8_t *buf = MemoryHelper::createBuffer<uint8_t *>(Signer.mbfs, bufLen + 1, false);
            int read = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;

                read = Signer.mbfs->read(mbfs_type fbdo->session.cfn.storageType, buf, available);

                if (fbdo->tcpClient.write(buf, read) != read)
                    break;

                available = Signer.mbfs->available(mbfs_type fbdo->session.cfn.storageType);
            }

            MemoryHelper::freeBuffer(Signer.mbfs, buf);

            Signer.mbfs->close(mbfs_type fbdo->session.cfn.storageType);
        }
        else if (req->requestType == fb_esp_functions_request_type_pgm_upload)
        {
            int len = req->pgmArcLen;
            int available = len;

            int bufLen = Utils::getUploadBufSize(Signer.config, fb_esp_con_mode_functions);
            uint8_t *buf = MemoryHelper::createBuffer<uint8_t *>(Signer.mbfs, bufLen + 1, false);
            size_t pos = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                memcpy_P(buf, req->pgmArc + pos, available);
                if (fbdo->tcpClient.write(buf, available) != available)
                    break;
                pos += available;
                len -= available;
                available = len;
            }

            MemoryHelper::freeBuffer(Signer.mbfs, buf);
        }

        if (handleResponse(fbdo))
        {
            fbdo->closeSession();
            Signer.config->internal.fb_processing = false;
            return true;
        }
    }
    else
        fbdo->session.connected = false;

    Signer.config->internal.fb_processing = false;
    return false;
}

void FB_Functions::rescon(FirebaseData *fbdo, const char *host)
{
    fbdo->_responseCallback = NULL;

    if (fbdo->session.cert_updated || !fbdo->session.connected ||
        millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.con_mode != fb_esp_con_mode_functions || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->session.host = host;
    fbdo->session.con_mode = fb_esp_con_mode_functions;
}

bool FB_Functions::connect(FirebaseData *fbdo, const char *host)
{
    if (strlen(host) > 0)
    {
        rescon(fbdo, host);
        fbdo->tcpClient.begin(host, 443, &fbdo->session.response.code);
    }
    else
    {
        MB_String host;
        HttpHelper::addGAPIsHost(host, fb_esp_func_pgm_str_38 /* "cloudfunctions." */);
        rescon(fbdo, host.c_str());
        fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    }
    fbdo->session.max_payload_length = 0;
    return true;
}

bool FB_Functions::handleResponse(FirebaseData *fbdo)
{
    if (!fbdo->reconnect())
        return false;

    struct server_response_data_t response;
    struct fb_esp_tcp_response_handler_t tcpHandler;

    HttpHelper::initTCPSession(fbdo->session);
    HttpHelper::intTCPHandler(fbdo->tcpClient.client, tcpHandler, 2048, fbdo->session.resp_size, nullptr, false);

    MB_String js;

    if (!fbdo->waitResponse(tcpHandler))
        return false;

    bool complete = false;

    while (tcpHandler.available() > 0 /* data available to read payload */ ||
           tcpHandler.payloadRead < response.contentLen /* incomplete content read  */)
    {
        if (!fbdo->readResponse(&fbdo->session.cfn.payload, tcpHandler, response) && !response.isChunkedEnc)
            break;

        // Last chunk?
        if (Utils::isChunkComplete(&tcpHandler, &response, complete))
            break;
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    // parse the payload
    if (fbdo->session.cfn.payload.length() > 0 &&
        (fbdo->session.cfn.requestType != fb_esp_functions_request_type_upload &&
         fbdo->session.cfn.requestType != fb_esp_functions_request_type_pgm_upload))
    {
        if (fbdo->session.cfn.payload[0] == '{')
        {
            fbdo->initJson();
            int errType = 0;

            JsonHelper::setData(fbdo->session.jsonPtr, fbdo->session.cfn.payload, false);
            if (JsonHelper::parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, fb_esp_storage_ss_pgm_str_16 /* "error/code" */))
                errType = 1;
            else if (JsonHelper::parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, fb_esp_func_pgm_str_39 /* "operations/[0]/error/code" */))
                errType = 2;

            if (errType > 0)
            {
                tcpHandler.error.code = fbdo->session.dataPtr->intValue;

                bool success = false;

                if (errType == 1)
                    success = JsonHelper::parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, fb_esp_storage_ss_pgm_str_17 /* "error/message" */);
                else if (errType == 2)
                    success = JsonHelper::parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, fb_esp_func_pgm_str_40 /* "operations/[0]/error/message" */);

                if (success)
                {
                    fbdo->session.error = fbdo->session.dataPtr->to<const char *>();

                    if (JsonHelper::parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, fb_esp_func_pgm_str_41 /* "error/details" */))
                        fbdo->session.error = fbdo->session.dataPtr->to<const char *>();

                    if (_deployTasks.size() > 0)
                    {
                        fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                        fbdo->session.cfn.cbInfo.errorMsg = fbdo->session.error.c_str();
                        sendCallback(fbdo, _deployTasks[_deployIndex].callback, _deployTasks[_deployIndex].statusInfo);
                    }
                }
            }
            else
            {
                tcpHandler.error.code = 0;
            }

            fbdo->clearJson();
        }
        fbdo->session.content_length = response.payloadLen;
    }

    if (fbdo->session.cfn.requestType == fb_esp_functions_request_type_upload ||
        fbdo->session.cfn.requestType == fb_esp_functions_request_type_pgm_upload)
        return response.httpCode == 200;
    else
        return tcpHandler.error.code == 0;
}

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
void FB_Functions::runDeployTask(const char *taskName)
#else
void FB_Functions::runDeployTask()
#endif
{
#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))

    static FB_Functions *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
        while (_this->_creation_task_enable)
        {

            if (!Signer.config->internal.deploy_loop_task_enable)
                break;

            vTaskDelay(10 / portTICK_PERIOD_MS);

            if (_this->_deployTasks.size() == 0)
                break;

            _this->mDeployTasks();

            yield();
        }

        Signer.config->internal.functions_check_task_handle = NULL;
        vTaskDelete(NULL);
    };

#if defined(ESP32)
    xTaskCreatePinnedToCore(taskCode, taskName, 12000, NULL, 3, &Signer.config->internal.functions_check_task_handle, 1);

#elif defined(MB_ARDUINO_PICO)

    /* Create a task, storing the handle. */
    xTaskCreate(taskCode, taskName, 12000, NULL,
                3, &(Signer.config->internal.functions_check_task_handle));

    /* Define the core affinity mask such that this task can only run on core 0
     * and core 1. */
    UBaseType_t uxCoreAffinityMask = ((1 << 0) | (1 << 1));

    /* Set the core affinity mask for the task. */
    vTaskCoreAffinitySet(Signer.config->internal.functions_check_task_handle, uxCoreAffinityMask);

#endif
#elif defined(ESP8266) || defined(MB_ARDUINO_PICO)
    mDeployTasks();
#endif
}

void FB_Functions::mDeployTasks()
{
    if (_creation_task_running)
        return;

    if (_creation_task_enable)
    {

        delay(10);

        if (_deployTasks.size() == 0)
            return;

        _creation_task_running = true;

        if (_deployIndex < _deployTasks.size() - 1)
            _deployIndex++;
        else
            _deployIndex = 0;

        struct fb_esp_deploy_task_info_t *taskInfo = &_deployTasks[_deployIndex];

        if (!taskInfo->done)
        {

            if (taskInfo->step == fb_esp_functions_creation_step_gen_upload_url)
            {
                taskInfo->done = true;
                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_generate_upload_url;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                bool ret = mGenerateUploadUrl(taskInfo->fbdo,
                                              toStringPtr(taskInfo->config->_projectId),
                                              toStringPtr(taskInfo->config->_locationId));

                if (ret)
                {
                    taskInfo->fbdo->initJson();
                    JsonHelper::setData(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.cfn.payload, false);
                    JsonHelper::parse(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.dataPtr,
                                      fb_esp_func_pgm_str_42 /* "uploadUrl" */);

                    JsonHelper::clear(taskInfo->fbdo->session.jsonPtr);
                    JsonHelper::arrayClear(taskInfo->fbdo->session.arrPtr);
                    taskInfo->fbdo->session.cfn.payload.clear();
                    if (taskInfo->fbdo->session.dataPtr->success)
                    {
                        addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                        taskInfo->nextStep, fb_esp_functions_creation_step_deploy,
                                        taskInfo->callback, taskInfo->statusInfo);
                        _deployTasks[_deployIndex + 1].uploadUrl = taskInfo->fbdo->session.dataPtr->to<const char *>();
                    }
                }
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == fb_esp_functions_creation_step_upload_source_files)
            {
                taskInfo->done = true;

                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_upload_source_file_in_progress;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                bool ret = uploadSources(taskInfo->fbdo, taskInfo->config);

                if (ret)
                {
                    taskInfo->fbdo->initJson();
                    JsonHelper::setData(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.cfn.payload, false);
                    if (JsonHelper::parse(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.dataPtr,
                                          fb_esp_func_pgm_str_43 /* "status/uploadUrl" */))
                    {

                        taskInfo->config->_funcCfg.add(pgm2Str(fb_esp_func_pgm_str_44 /* "sourceUploadUrl" */),
                                                       taskInfo->fbdo->session.dataPtr->to<const char *>());
                        taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_func_pgm_str_44 /* "sourceUploadUrl" */));
                    }

                    JsonHelper::clear(taskInfo->fbdo->session.jsonPtr);

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep,
                                    fb_esp_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    if (taskInfo->fbdo->session.response.code == 302 || taskInfo->fbdo->session.response.code == 403)
                        taskInfo->fbdo->session.error += fb_esp_functions_err_pgm_str_1;
                    // "missing autozip function, please deploy it first"
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == fb_esp_functions_creation_step_upload_zip_file)
            {
                taskInfo->done = true;
                bool ret = false;
                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_upload_source_file_in_progress;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);

                if (taskInfo->config->_sourceType == functions_sources_type_local_archive)
                    ret = uploadFile(taskInfo->fbdo, taskInfo->uploadUrl.c_str(),
                                     taskInfo->config->_uploadArchiveFile.c_str(),
                                     taskInfo->config->_uploadArchiveStorageType);
                else if (taskInfo->config->_sourceType == functions_sources_type_flash_data)
                    ret = uploadPGMArchive(taskInfo->fbdo, taskInfo->uploadUrl.c_str(),
                                           taskInfo->config->_pgmArc, taskInfo->config->_pgmArcLen);

                if (ret)
                {

                    taskInfo->config->_funcCfg.set(pgm2Str(fb_esp_func_pgm_str_44 /* "sourceUploadUrl" */),
                                                   taskInfo->uploadUrl.c_str());
                    taskInfo->config->addUpdateMasks(pgm2Str(fb_esp_func_pgm_str_44 /* "sourceUploadUrl" */));

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                    taskInfo->nextStep, fb_esp_functions_creation_step_polling_status,
                                    taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
                taskInfo->uploadUrl.clear();
            }

            if (taskInfo->step == fb_esp_functions_creation_step_deploy)
            {
                taskInfo->done = true;
                bool ret = false;
                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);

                ret = deploy(taskInfo->fbdo, taskInfo->functionId.c_str(), taskInfo->config, taskInfo->patch);
                taskInfo->fbdo->session.cfn.cbInfo.triggerUrl = taskInfo->config->_httpsTriggerUrl;
                if (ret)
                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                    taskInfo->nextStep, fb_esp_functions_creation_step_set_iam_policy,
                                    taskInfo->callback, taskInfo->statusInfo);
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == fb_esp_functions_creation_step_set_iam_policy)
            {
                taskInfo->done = true;
                bool ret = false;
                static PolicyBuilder pol;
                JsonHelper::setData(&pol.json, taskInfo->policy, false);
                ret = mSetIamPolicy(taskInfo->fbdo, toStringPtr(taskInfo->projectId),
                                    toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId),
                                    &pol, toStringPtr(_EMPTY_STR));
                if (ret)
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == fb_esp_functions_creation_step_delete)
            {
                taskInfo->done = true;
                bool ret = false;
                MB_String t;
                t += fb_esp_func_pgm_str_45; // "project:"
                t += taskInfo->projectId;
                t += fb_esp_func_pgm_str_46; // ",latest:true"
                ret = mListOperations(taskInfo->fbdo, toStringPtr(t), toStringPtr("1"), toStringPtr(_EMPTY_STR));
                if (ret)
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                    taskInfo->fbdo->initJson();
                    JsonHelper::setData(taskInfo->fbdo->session.jsonPtr,
                                        taskInfo->fbdo->session.cfn.payload, false);
                    JsonHelper::parse(taskInfo->fbdo->session.jsonPtr,
                                      taskInfo->fbdo->session.dataPtr, fb_esp_func_pgm_str_40 /* "operations/[0]/error/message" */);

                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->session.dataPtr->to<const char *>();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }

                ret = mDeleteFunction(taskInfo->fbdo, toStringPtr(taskInfo->projectId),
                                      toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId));
            }

            if (taskInfo->step == fb_esp_functions_creation_step_polling_status)
            {
                if (millis() - _lastPollMs > 5000 || _lastPollMs == 0)
                {
                    _lastPollMs = millis();

                    if (!taskInfo->_delete && !taskInfo->active)
                    {

                        bool ret = mGetFunction(taskInfo->fbdo,
                                                toStringPtr(taskInfo->projectId),
                                                toStringPtr(taskInfo->locationId),
                                                toStringPtr(taskInfo->functionId));

                        if (ret)
                        {
                            if (_function_status == fb_esp_functions_status_UNKNOWN ||
                                _function_status == fb_esp_functions_status_OFFLINE)
                                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_error;
                            else if (_function_status == fb_esp_functions_status_DEPLOY_IN_PROGRESS)
                                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_deploy_in_progress;
                            else if (_function_status == fb_esp_functions_status_DELETE_IN_PROGRESS)
                                taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_delete_in_progress;
                            else if (_function_status == fb_esp_functions_status_ACTIVE)
                            {
                                if (taskInfo->setPolicy)
                                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_set_iam_policy_in_progress;
                                else
                                    taskInfo->fbdo->session.cfn.cbInfo.status = fb_esp_functions_operation_status_finished;
                            }

                            if (_function_status != fb_esp_functions_status_UNKNOWN && _function_status != fb_esp_functions_status_OFFLINE)
                            {
                                taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->session.error;
                                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                            }
                        }

                        if (!ret || _function_status == fb_esp_functions_status_ACTIVE ||
                            _function_status == fb_esp_functions_status_UNKNOWN ||
                            _function_status == fb_esp_functions_status_OFFLINE)
                        {
                            taskInfo->done = true;
                            taskInfo->_delete = true;
                            taskInfo->active = _function_status == fb_esp_functions_status_ACTIVE;

                            if (_function_status == fb_esp_functions_status_ACTIVE && taskInfo->setPolicy)
                                addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep,
                                                fb_esp_functions_creation_step_idle, taskInfo->callback, taskInfo->statusInfo);

                            if (_function_status == fb_esp_functions_status_UNKNOWN ||
                                _function_status == fb_esp_functions_status_OFFLINE)
                                addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                                fb_esp_functions_creation_step_delete, fb_esp_functions_creation_step_idle,
                                                taskInfo->callback, taskInfo->statusInfo);
                        }
                    }
                }
            }
        }

        size_t n = 0;
        for (size_t i = 0; i < _deployTasks.size(); i++)
            if (_deployTasks[i].done)
                n++;

#if defined(ESP8266)
        // User intends to run task manually, do not add schedule.
        if (Signer.config->internal.deploy_loop_task_enable && _deployTasks.size() > 0 && n < _deployTasks.size())
            Signer.set_scheduled_callback(std::bind(&FB_Functions::runDeployTask, this));
#endif

        if (n == _deployTasks.size())
        {
            for (size_t i = 0; i < n; i++)
            {
                struct fb_esp_deploy_task_info_t *taskInfo = &_deployTasks[i];
                taskInfo->uploadUrl.clear();
                taskInfo->projectId.clear();
                taskInfo->locationId.clear();
                taskInfo->functionId.clear();
                taskInfo->policy.clear();
                taskInfo->httpsTriggerUrl.clear();
                taskInfo->fbdo->session.long_running_task--;
                taskInfo->fbdo->clear();
                taskInfo->fbdo = nullptr;
                taskInfo->callback = NULL;
                taskInfo->config = nullptr;
            }

            _deployTasks.clear();
            _creation_task_enable = false;
        }
    }

    _creation_task_running = false;
}

void FB_Functions::mRunDeployTasks()
{
    Signer.config->internal.deploy_loop_task_enable = false;

#if defined(ESP32) || (defined(MB_ARDUINO_PICO) && defined(ENABLE_PICO_FREE_RTOS))
    if (Signer.config->internal.functions_check_task_handle)
        return;
#endif

    if (_deployTasks.size() > 0)
        mDeployTasks();
}

#endif

#endif // ENABLE