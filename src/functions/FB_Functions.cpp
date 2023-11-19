
/**
 * Google's Cloud Functions class, Functions.cpp version 1.1.26
 *
 * Created September 13, 2023
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

#ifndef _FB_FUNCTIONS_CPP_
#define _FB_FUNCTIONS_CPP_

#include "FB_Functions.h"

FB_Functions::FB_Functions()
{
}

FB_Functions::~FB_Functions()
{
}

void FB_Functions::makeRequest(struct firebase_functions_req_t &req, firebase_functions_request_type type,
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
    struct firebase_functions_req_t req;

    makeRequest(req, firebase_functions_request_type_call, projectId, locationId, functionId);

    req.payload = data;
    fbdo->initJson();

    Core.jh.addString(fbdo->session.jsonPtr, firebase_pgm_str_67 /* "data" */, req.payload);

    Core.jh.toString(fbdo->session.jsonPtr, req.payload, true);

    return sendRequest(fbdo, &req);
}

void FB_Functions::addCreationTask(FirebaseData *fbdo, FunctionsConfig *config, bool patch,
                                   firebase_functions_creation_step step, firebase_functions_creation_step nextStep,
                                   FunctionsOperationCallback callback, FunctionsOperationStatusInfo *statusInfo)
{
    struct firebase_deploy_task_info_t taskInfo;
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
        taskInfo.statusInfo->status = firebase_functions_operation_status_deploy_in_progress;
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
    if (Core.internal.deploy_loop_task_enable)
    {
        // The first task?, running in loop task by default
        if (_deployTasks.size() == 1)
            runDeployTask();
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
    Core.internal.deploy_loop_task_enable = true;

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

            int sz = Core.mbfs.open(config->_uploadArchiveFile, mbfs_type config->_uploadArchiveStorageType,
                                       mb_fs_open_mode_read);

            if (sz < 0)
            {
                fbdo->session.response.code = MB_FS_ERROR_FILE_IO_ERROR;
                fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                fbdo->session.cfn.cbInfo.errorMsg = fbdo->errorReason().c_str();
                sendCallback(fbdo, cb, info);
                return false;
            }

            fbdo->session.cfn.fileSize = sz;
        }
        // Fix in ESP32 core 2.0.x
        Core.mbfs.close(mbfs_type config->_uploadArchiveStorageType);

        addCreationTask(fbdo, config, patch, firebase_functions_creation_step_gen_upload_url,
                        firebase_functions_creation_step_upload_zip_file, cb, info);
        return true;
    }
    else if (config->_sourceType == functions_sources_type_storage_bucket_sources)
    {

        addCreationTask(fbdo, config, patch, firebase_functions_creation_step_upload_source_files,
                        firebase_functions_creation_step_deploy, cb, info);
        return true;
    }
    else
    {
        bool ret = false;
        fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_deploy_in_progress;
        fbdo->session.cfn.cbInfo.errorMsg.clear();
        sendCallback(fbdo, cb, info);
        MB_String _functionId = functionId;
        ret = deploy(fbdo, _functionId.c_str(), config, patch);
        fbdo->session.cfn.cbInfo.triggerUrl = config->_httpsTriggerUrl;
        if (info)
            info->triggerUrl = config->_httpsTriggerUrl;

        if (!ret)
        {
            fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
            fbdo->session.cfn.cbInfo.errorMsg = fbdo->session.error.c_str();
            sendCallback(fbdo, cb, info);
        }

        if (ret && config->_policy)
            addCreationTask(fbdo, config, patch, firebase_functions_creation_step_polling_status,
                            firebase_functions_creation_step_set_iam_policy, cb, info);
        else if (ret)
            addCreationTask(fbdo, config, patch, firebase_functions_creation_step_polling_status,
                            firebase_functions_creation_step_idle, cb, info);
        return ret;
    }

    return true;
}

bool FB_Functions::uploadSources(FirebaseData *fbdo, FunctionsConfig *config)
{
    fbdo->initJson();

    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_2 /* "projectId" */, config->_projectId);
    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_3 /* "location" */, config->_locationId);
    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_5 /* "zip" */, MB_String(firebase_func_pgm_str_8 /* "tmp.zip" */));
    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_6 /* "accessToken" */, Core.getToken());
    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_7 /* "path" */, config->_bucketSourcesPath);

    struct firebase_functions_req_t req;

    req.requestType = firebase_functions_request_type_upload_bucket_sources;

    Core.jh.toString(fbdo->session.jsonPtr, req.payload, true);

    Core.uh.addFunctionsHost(req.host, config->_locationId, config->_projectId,
                                MB_String(firebase_func_pgm_str_4) /* "autozip" */, false);

    return sendRequest(fbdo, &req);
}

bool FB_Functions::deploy(FirebaseData *fbdo, const char *functionId, FunctionsConfig *config, bool patch)
{

    struct firebase_functions_req_t req;

    if (patch)
        req.updateMask = &config->_updateMask;

    makeRequest(req, patch ? firebase_functions_request_type_patch : firebase_functions_request_type_create,
                toStringPtr(config->_projectId), toStringPtr(config->_locationId),
                toStringPtr(functionId));

    MB_String str;

    fbdo->initJson();

    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_2 /* "projectId" */, config->_projectId);
    Core.uh.host2Url(str, Core.config->database_url);
    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_9 /* "databaseURL" */, str);

    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_10 /* "storageBucket" */, config->_bucketId);
    Core.jh.addString(fbdo->session.jsonPtr, firebase_func_pgm_str_11 /* "locationId" */, config->_locationId);

    str = firebase_func_pgm_str_12;  // "environmentVariables"
    str += firebase_pgm_str_1;       // "/"
    str += firebase_func_pgm_str_13; // "FIREBASE_CONFIG"

    Core.jh.addString(&config->_funcCfg, str.c_str(), fbdo->session.jsonPtr->raw());
    Core.jh.clear(fbdo->session.jsonPtr);

    str = firebase_func_pgm_str_12;  // "environmentVariables"
    str += firebase_pgm_str_1;       // "/"
    str += firebase_func_pgm_str_14; // "GCLOUD_PROJECT"

    Core.jh.addString(&config->_funcCfg, str.c_str(), config->_projectId);

    config->_httpsTriggerUrl.clear();

    if (config->_triggerType == firebase_functions_trigger_type_https)
    {
        str.clear();

        Core.uh.addFunctionsHost(str, config->_locationId, config->_projectId, config->_entryPoint, true);
        Core.jh.addString(&config->_funcCfg, firebase_func_pgm_str_15 /* "httpsTrigger/url" */, str);
        config->_httpsTriggerUrl = str;
        str.clear();
    }

    Core.jh.toString(&config->_funcCfg, req.payload, true);
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
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_set_iam_policy, projectId, locationId, functionId);
    fbdo->initJson();

    if (policy)
        Core.jh.addObject(fbdo->session.jsonPtr, firebase_func_pgm_str_16 /* "policy" */, &policy->json, true);

    Core.jh.addString(fbdo->session.jsonPtr, firebase_pgm_str_70 /* "updateMask" */, MB_String(updateMask));
    Core.jh.toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetIamPolicy(FirebaseData *fbdo, MB_StringPtr projectId,
                                 MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr version)
{
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_get_iam_policy, projectId, locationId, functionId);
    req.policyVersion = version;
    fbdo->initJson();
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGetFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
{
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_get, projectId, locationId, functionId);
    _function_status = firebase_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;
    bool ret = sendRequest(fbdo, &req);
    if (ret)
    {
        fbdo->initJson();
        Core.jh.setData(fbdo->session.jsonPtr, fbdo->session.cfn.payload, false);
        if (Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, firebase_func_pgm_str_17 /* "status" */))
        {
            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), firebase_func_pgm_str_18 /* "CLOUD_FUNCTION_STATUS_UNSPECIFIED" */) == 0)
                _function_status = firebase_functions_status_CLOUD_FUNCTION_STATUS_UNSPECIFIED;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), firebase_func_pgm_str_19 /* "ACTIVE" */) == 0)
                _function_status = firebase_functions_status_ACTIVE;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), firebase_func_pgm_str_20 /* "OFFLINE" */) == 0)
                _function_status = firebase_functions_status_OFFLINE;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), firebase_func_pgm_str_21 /* "DEPLOY_IN_PROGRESS" */) == 0)
                _function_status = firebase_functions_status_DEPLOY_IN_PROGRESS;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), firebase_func_pgm_str_22 /* "DELETE_IN_PROGRESS" */) == 0)
                _function_status = firebase_functions_status_DELETE_IN_PROGRESS;

            if (strcmp_P(fbdo->session.dataPtr->to<const char *>(), firebase_func_pgm_str_23 /*  "UNKNOWN" */) == 0)
                _function_status = firebase_functions_status_UNKNOWN;
        }
    }
    return ret;
}

bool FB_Functions::mListFunctions(FirebaseData *fbdo, MB_StringPtr projectId,
                                  MB_StringPtr locationId, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_list, projectId, locationId, toStringPtr(""));
    req.pageSize = atoi(stringPtr2Str(pageSize));
    req.pageToken = pageToken;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mListOperations(FirebaseData *fbdo, MB_StringPtr filter, MB_StringPtr pageSize, MB_StringPtr pageToken)
{
    struct firebase_functions_req_t req;
    req.requestType = firebase_functions_request_type_list_operations;
    req.filter = filter;
    req.pageSize = atoi(stringPtr2Str(pageSize));
    req.pageToken = pageToken;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mDeleteFunction(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr functionId)
{
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_delete, projectId, locationId, functionId);
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateDownloadUrl(FirebaseData *fbdo, MB_StringPtr projectId,
                                        MB_StringPtr locationId, MB_StringPtr functionId, MB_StringPtr versionId)
{
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_gen_download_url, projectId, locationId, functionId);
    fbdo->initJson();
    Core.jh.addNumberString(fbdo->session.jsonPtr, firebase_func_pgm_str_24 /* "versionId" */, MB_String(versionId));
    Core.jh.toString(fbdo->session.jsonPtr, req.payload, true);
    return sendRequest(fbdo, &req);
}

bool FB_Functions::mGenerateUploadUrl(FirebaseData *fbdo, MB_StringPtr projectId, MB_StringPtr locationId)
{
    struct firebase_functions_req_t req;
    makeRequest(req, firebase_functions_request_type_gen_upload_url, projectId, locationId, toStringPtr(""));
    return sendRequest(fbdo, &req);
}

bool FB_Functions::uploadFile(FirebaseData *fbdo, const char *uploadUrl, const char *filePath,
                              firebase_mem_storage_type storageType)
{
    struct firebase_functions_req_t req;
    req.requestType = firebase_functions_request_type_upload;
    Core.uh.addPath(req.filePath, MB_String(filePath));
    req.storageType = storageType;

    struct firebase_url_info_t info;
    Core.uh.parse(&Core.mbfs, uploadUrl, info);
    req.host = info.host;
    req.uri = firebase_pgm_str_1; // "/"
    req.uri += info.uri;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::uploadPGMArchive(FirebaseData *fbdo, const char *uploadUrl, const uint8_t *pgmArc, size_t pgmArcLen)
{
    struct firebase_functions_req_t req;
    req.requestType = firebase_functions_request_type_pgm_upload;
    req.pgmArc = pgmArc;
    req.pgmArcLen = pgmArcLen;

    struct firebase_url_info_t info;
    Core.uh.parse(&Core.mbfs, uploadUrl, info);
    req.host = info.host;
    req.uri = firebase_pgm_str_1; // "/"
    req.uri += info.uri;
    return sendRequest(fbdo, &req);
}

bool FB_Functions::sendRequest(FirebaseData *fbdo, struct firebase_functions_req_t *req)
{
    fbdo->session.http_code = 0;

    if (!Core.config)
    {
        fbdo->session.response.code = FIREBASE_ERROR_UNINITIALIZED;
        return false;
    }

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)
    if (fbdo->session.rtdb.pause)
        return true;
#endif

    if (!fbdo->reconnect() || !Core.tokenReady())
        return false;

    if (Core.internal.fb_processing)
        return false;

    Core.internal.fb_processing = true;
    fbdo->clear();
    connect(fbdo, req->host.c_str());

    bool ret = functions_sendRequest(fbdo, req);

    if (!ret)
        fbdo->closeSession();

    return ret;
}

bool FB_Functions::functions_sendRequest(FirebaseData *fbdo, struct firebase_functions_req_t *req)
{
    bool post = false;
    fbdo->session.cfn.requestType = req->requestType;

    MB_String header;
    firebase_request_method method = http_undefined;
    if (req->requestType == firebase_functions_request_type_get_iam_policy ||
        req->requestType == firebase_functions_request_type_list_operations ||
        req->requestType == firebase_functions_request_type_get ||
        req->requestType == firebase_functions_request_type_get_iam_policy ||
        req->requestType == firebase_functions_request_type_list)
        method = http_get;
    else if (req->requestType == firebase_functions_request_type_upload_bucket_sources ||
             req->requestType == firebase_functions_request_type_call ||
             req->requestType == firebase_functions_request_type_create ||
             req->requestType == firebase_functions_request_type_gen_download_url ||
             req->requestType == firebase_functions_request_type_gen_upload_url ||
             req->requestType == firebase_functions_request_type_set_iam_policy ||
             req->requestType == firebase_functions_request_type_test_iam_policy)
        method = http_post;
    else if (req->requestType == firebase_functions_request_type_patch)
        method = http_patch;
    else if (req->requestType == firebase_functions_request_type_delete)
        method = http_delete;
    else if (req->requestType == firebase_functions_request_type_upload ||
             req->requestType == firebase_functions_request_type_pgm_upload)
        method = http_put;

    if (method != http_undefined)
        post = Core.hh.addRequestHeaderFirst(header, method);

    if (req->requestType == firebase_functions_request_type_upload_bucket_sources ||
        req->requestType == firebase_functions_request_type_upload ||
        req->requestType == firebase_functions_request_type_pgm_upload)
        header += req->uri;
    else if (req->requestType == firebase_functions_request_type_list_operations)
    {
        header += firebase_func_pgm_str_25; // "/v1/operations"
        bool hasParam = false;

        Core.uh.addParam(header, firebase_func_pgm_str_26 /* "filter=" */, req->filter, hasParam);

        if (req->pageSize > 0)
            Core.uh.addParam(header, firebase_pgm_str_63 /* "pageSize" */, MB_String(req->pageSize), hasParam);

        Core.uh.addParam(header, firebase_pgm_str_65 /* "pageToken" */, req->pageToken, hasParam);
    }
    else
    {
        Core.uh.addGAPIv1Path(header);

        header += req->projectId.length() == 0 ? Core.config->service_account.data.project_id : req->projectId;

        header += firebase_func_pgm_str_27; // "/locations/"
        if (req->locationId.length() > 0)
            header += req->locationId;

        header += firebase_func_pgm_str_28; // "/functions"

        if (req->requestType == firebase_functions_request_type_list)
        {
            bool hasParam = false;

            if (req->pageSize > 0)
                Core.uh.addParam(header, firebase_pgm_str_63 /* "pageSize" */, MB_String(req->pageSize), hasParam);

            Core.uh.addParam(header, firebase_pgm_str_65 /* "pageToken" */, req->pageToken, hasParam);
        }

        if (req->requestType == firebase_functions_request_type_patch ||
            req->requestType == firebase_functions_request_type_get_iam_policy ||
            req->requestType == firebase_functions_request_type_gen_download_url ||
            req->requestType == firebase_functions_request_type_delete ||
            req->requestType == firebase_functions_request_type_get ||
            req->requestType == firebase_functions_request_type_call ||
            req->requestType == firebase_functions_request_type_set_iam_policy)
        {
            header += firebase_pgm_str_1; // "/"
            if (req->functionId.length() > 0)
                header += req->functionId;
            if (req->requestType == firebase_functions_request_type_call)
                header += firebase_func_pgm_str_29; // ":call"
            else if (req->requestType == firebase_functions_request_type_set_iam_policy)
                header += firebase_func_pgm_str_30; // ":setIamPolicy"
            else if (req->requestType == firebase_functions_request_type_gen_download_url)
                header += firebase_func_pgm_str_31; // ":generateDownloadUrl"
            else if (req->requestType == firebase_functions_request_type_get_iam_policy)
            {
                header += firebase_func_pgm_str_32; // ":getIamPolicy"
                bool hasParam = false;
                Core.uh.addParam(header, firebase_func_pgm_str_33 /* "options.requestedPolicyVersion" */,
                                    req->policyVersion, hasParam);
            }
            else if (req->requestType == firebase_functions_request_type_patch)
            {
                bool hasParam = false;

                if (req->updateMask->size() > 0)
                {
                    for (size_t i = 0; i < req->updateMask->size(); i++)
                        Core.uh.addParam(header, firebase_func_pgm_str_34 /* "updateMask=" */, (*req->updateMask)[i], hasParam);
                }
            }
        }
        else if (req->requestType == firebase_functions_request_type_gen_upload_url)
            header += firebase_func_pgm_str_35; // :generateUploadUrl"
    }

    Core.hh.addRequestHeaderLast(header);

    if (post)
    {
        if (req->payload.length() > 0)
            Core.hh.addContentTypeHeader(header, firebase_pgm_str_62 /* "application/json" */);
        Core.hh.addContentLengthHeader(header, req->payload.length());
    }

    if (req->requestType == firebase_functions_request_type_upload ||
        req->requestType == firebase_functions_request_type_pgm_upload)
    {
        Core.hh.addContentTypeHeader(header, firebase_func_pgm_str_36 /* "application/zip" */);
        size_t len = 0;
        if (req->requestType == firebase_functions_request_type_pgm_upload)
            len = req->pgmArcLen;
        else if (req->requestType == firebase_functions_request_type_upload)
            len = fbdo->session.cfn.fileSize;

        Core.hh.addContentLengthHeader(header, len);
        header += firebase_func_pgm_str_37; // "x-goog-content-length-range: 0,104857600"
        Core.hh.addNewLine(header);
        Core.hh.addHostHeader(header, req->host.c_str());
    }
    else
    {

        if (req->requestType == firebase_functions_request_type_upload_bucket_sources)
            Core.hh.addHostHeader(header, req->host.c_str());
        else
            Core.hh.addGAPIsHostHeader(header, firebase_func_pgm_str_38 /* "cloudfunctions." */);

        if (req->requestType != firebase_functions_request_type_upload_bucket_sources)
        {
            if (!Core.config->signer.test_mode)
            {
                Core.hh.addAuthHeaderFirst(header, Core.getTokenType());
                header += Core.getToken();
                Core.hh.addNewLine(header);
            }
        }
    }

    Core.hh.addUAHeader(header);
    bool keepAlive = false;
#if defined(USE_CONNECTION_KEEP_ALIVE_MODE)
    keepAlive = true;
#endif
    Core.hh.addConnectionHeader(header, keepAlive);
    Core.hh.getCustomHeaders(&Core.sh,header, Core.config->signer.customHeaders);
    Core.hh.addNewLine(header);

    fbdo->session.response.code = FIREBASE_ERROR_TCP_ERROR_NOT_CONNECTED;

    fbdo->tcpSend(header.c_str());
    if (fbdo->session.response.code < 0)
        return false;

    if (fbdo->session.response.code > 0 && req->payload.length() > 0)
        fbdo->tcpSend(req->payload.c_str());

    header.clear();
    req->payload.clear();

    if (fbdo->session.response.code > 0)
    {
        if (req->requestType == firebase_functions_request_type_upload)
        {
            // Fix in ESP32 core 2.0.x
            Core.mbfs.open(fbdo->session.cfn.filepath, mbfs_type fbdo->session.cfn.storageType, mb_fs_open_mode_read);
            fbdo->session.cfn.filepath.clear();
            int available = Core.mbfs.available(mbfs_type fbdo->session.cfn.storageType);
            int bufLen = Core.ut.getUploadBufSize(Core.config, firebase_con_mode_functions);
            uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen + 1, false));
            int read = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;

                read = Core.mbfs.read(mbfs_type fbdo->session.cfn.storageType, buf, available);

                if ((int)fbdo->tcpWrite(buf, read) != read)
                    break;

                available = Core.mbfs.available(mbfs_type fbdo->session.cfn.storageType);
            }

            Core.mbfs.delP(&buf);

            Core.mbfs.close(mbfs_type fbdo->session.cfn.storageType);
        }
        else if (req->requestType == firebase_functions_request_type_pgm_upload)
        {
            int len = req->pgmArcLen;
            int available = len;

            int bufLen = Core.ut.getUploadBufSize(Core.config, firebase_con_mode_functions);
            uint8_t *buf = reinterpret_cast<uint8_t *>(Core.mbfs.newP(bufLen + 1, false));
            size_t pos = 0;

            while (available)
            {
                if (available > bufLen)
                    available = bufLen;
                memcpy_P(buf, req->pgmArc + pos, available);
                if ((int)fbdo->tcpWrite(buf, available) != available)
                    break;
                pos += available;
                len -= available;
                available = len;
            }

            Core.mbfs.delP(&buf);
        }

        if (handleResponse(fbdo))
        {
            fbdo->closeSession();
            Core.internal.fb_processing = false;
            return true;
        }
    }

    Core.internal.fb_processing = false;
    return false;
}

void FB_Functions::rescon(FirebaseData *fbdo, const char *host)
{
    fbdo->_responseCallback = NULL;

    if (fbdo->session.cert_updated || millis() - fbdo->session.last_conn_ms > fbdo->session.conn_timeout ||
        fbdo->session.con_mode != firebase_con_mode_functions || strcmp(host, fbdo->session.host.c_str()) != 0)
    {
        fbdo->session.last_conn_ms = millis();
        fbdo->closeSession();
        fbdo->setSecure();
    }
    fbdo->session.host = host;
    fbdo->session.con_mode = firebase_con_mode_functions;
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
        Core.hh.addGAPIsHost(host, firebase_func_pgm_str_38 /* "cloudfunctions." */);
        rescon(fbdo, host.c_str());
        fbdo->tcpClient.begin(host.c_str(), 443, &fbdo->session.response.code);
    }
    fbdo->tcpClient.setSession(&fbdo->bsslSession);
    fbdo->session.max_payload_length = 0;
    return true;
}

bool FB_Functions::handleResponse(FirebaseData *fbdo)
{
    if (!fbdo->reconnect())
        return false;

    struct server_response_data_t response;
    struct firebase_tcp_response_handler_t tcpHandler;

    Core.hh.initTCPSession(fbdo->session);
    Core.hh.intTCPHandler(&fbdo->tcpClient, tcpHandler, 2048, fbdo->session.resp_size, nullptr, false);

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
        if (Core.ut.isChunkComplete(&tcpHandler, &response, complete))
            break;
    }

    // To make sure all chunks read and
    // ready to send next request
    if (response.isChunkedEnc)
        fbdo->tcpClient.flush();

    // parse the payload
    if (fbdo->session.cfn.payload.length() > 0 &&
        (fbdo->session.cfn.requestType != firebase_functions_request_type_upload &&
         fbdo->session.cfn.requestType != firebase_functions_request_type_pgm_upload))
    {
        if (fbdo->session.cfn.payload[0] == '{')
        {
            fbdo->initJson();
            int errType = 0;

            Core.jh.setData(fbdo->session.jsonPtr, fbdo->session.cfn.payload, false);
            if (Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, firebase_storage_ss_pgm_str_16 /* "error/code" */))
                errType = 1;
            else if (Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, firebase_func_pgm_str_39 /* "operations/[0]/error/code" */))
                errType = 2;

            if (errType > 0)
            {
                tcpHandler.error.code = fbdo->session.dataPtr->intValue;

                bool success = false;

                if (errType == 1)
                    success = Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, firebase_storage_ss_pgm_str_17 /* "error/message" */);
                else if (errType == 2)
                    success = Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, firebase_func_pgm_str_40 /* "operations/[0]/error/message" */);

                if (success)
                {
                    fbdo->session.error = fbdo->session.dataPtr->to<const char *>();

                    if (Core.jh.parse(fbdo->session.jsonPtr, fbdo->session.dataPtr, firebase_func_pgm_str_41 /* "error/details" */))
                        fbdo->session.error = fbdo->session.dataPtr->to<const char *>();

                    if (_deployTasks.size() > 0)
                    {
                        fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
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

    if (fbdo->session.cfn.requestType == firebase_functions_request_type_upload ||
        fbdo->session.cfn.requestType == firebase_functions_request_type_pgm_upload)
        return response.httpCode == 200;
    else
        return tcpHandler.error.code == 0;
}

void FB_Functions::runDeployTask()
{
#if defined(ESP32)

    static FB_Functions *_this = this;
    MB_String taskName = "Deploy_";
    taskName += random(1, 100);

    TaskFunction_t taskCode = [](void *param)
    {
        while (_this->_creation_task_enable)
        {

            if (!Core.internal.deploy_loop_task_enable)
                break;

            vTaskDelay(10 / portTICK_PERIOD_MS);

            if (_this->_deployTasks.size() == 0)
                break;

            _this->mDeployTasks();

            yield();
        }

        Core.internal.functions_check_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, taskName.c_str(), 12000, NULL, 3, &Core.internal.functions_check_task_handle, 1);

#else
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

        struct firebase_deploy_task_info_t *taskInfo = &_deployTasks[_deployIndex];

        if (!taskInfo->done)
        {

            if (taskInfo->step == firebase_functions_creation_step_gen_upload_url)
            {
                taskInfo->done = true;
                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_generate_upload_url;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                bool ret = mGenerateUploadUrl(taskInfo->fbdo,
                                              toStringPtr(taskInfo->config->_projectId),
                                              toStringPtr(taskInfo->config->_locationId));

                if (ret)
                {
                    taskInfo->fbdo->initJson();
                    Core.jh.setData(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.cfn.payload, false);
                    Core.jh.parse(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.dataPtr,
                                      firebase_func_pgm_str_42 /* "uploadUrl" */);

                    Core.jh.clear(taskInfo->fbdo->session.jsonPtr);
                    Core.jh.arrayClear(taskInfo->fbdo->session.arrPtr);
                    taskInfo->fbdo->session.cfn.payload.clear();
                    if (taskInfo->fbdo->session.dataPtr->success)
                    {
                        addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                        taskInfo->nextStep, firebase_functions_creation_step_deploy,
                                        taskInfo->callback, taskInfo->statusInfo);
                        _deployTasks[_deployIndex + 1].uploadUrl = taskInfo->fbdo->session.dataPtr->to<const char *>();
                    }
                }
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == firebase_functions_creation_step_upload_source_files)
            {
                taskInfo->done = true;

                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_upload_source_file_in_progress;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                bool ret = uploadSources(taskInfo->fbdo, taskInfo->config);

                if (ret)
                {
                    taskInfo->fbdo->initJson();
                    Core.jh.setData(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.cfn.payload, false);
                    if (Core.jh.parse(taskInfo->fbdo->session.jsonPtr, taskInfo->fbdo->session.dataPtr,
                                          firebase_func_pgm_str_43 /* "status/uploadUrl" */))
                    {

                        taskInfo->config->_funcCfg.add(pgm2Str(firebase_func_pgm_str_44 /* "sourceUploadUrl" */),
                                                       taskInfo->fbdo->session.dataPtr->to<const char *>());
                        taskInfo->config->addUpdateMasks(pgm2Str(firebase_func_pgm_str_44 /* "sourceUploadUrl" */));
                    }

                    Core.jh.clear(taskInfo->fbdo->session.jsonPtr);

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep,
                                    firebase_functions_creation_step_polling_status, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    if (taskInfo->fbdo->session.response.code == 302 || taskInfo->fbdo->session.response.code == 403)
                        taskInfo->fbdo->session.error += firebase_functions_err_pgm_str_1;
                    // "missing autozip function, please deploy it first"
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == firebase_functions_creation_step_upload_zip_file)
            {
                taskInfo->done = true;
                bool ret = false;
                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_upload_source_file_in_progress;
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

                    taskInfo->config->_funcCfg.set(pgm2Str(firebase_func_pgm_str_44 /* "sourceUploadUrl" */),
                                                   taskInfo->uploadUrl.c_str());
                    taskInfo->config->addUpdateMasks(pgm2Str(firebase_func_pgm_str_44 /* "sourceUploadUrl" */));

                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                    taskInfo->nextStep, firebase_functions_creation_step_polling_status,
                                    taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
                taskInfo->uploadUrl.clear();
            }

            if (taskInfo->step == firebase_functions_creation_step_deploy)
            {
                taskInfo->done = true;
                bool ret = false;
                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_deploy_in_progress;
                taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);

                ret = deploy(taskInfo->fbdo, taskInfo->functionId.c_str(), taskInfo->config, taskInfo->patch);
                taskInfo->fbdo->session.cfn.cbInfo.triggerUrl = taskInfo->config->_httpsTriggerUrl;
                if (ret)
                    addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                    taskInfo->nextStep, firebase_functions_creation_step_set_iam_policy,
                                    taskInfo->callback, taskInfo->statusInfo);
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == firebase_functions_creation_step_set_iam_policy)
            {
                taskInfo->done = true;
                bool ret = false;
                static PolicyBuilder pol;
                Core.jh.setData(&pol.json, taskInfo->policy, false);
                ret = mSetIamPolicy(taskInfo->fbdo, toStringPtr(taskInfo->projectId),
                                    toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId),
                                    &pol, toStringPtr(_EMPTY_STR));
                if (ret)
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_finished;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg.clear();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
                else
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->errorReason().c_str();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }
            }

            if (taskInfo->step == firebase_functions_creation_step_delete)
            {
                taskInfo->done = true;
                bool ret = false;
                MB_String t;
                t += firebase_func_pgm_str_45; // "project:"
                t += taskInfo->projectId;
                t += firebase_func_pgm_str_46; // ",latest:true"
                ret = mListOperations(taskInfo->fbdo, toStringPtr(t), toStringPtr("1"), toStringPtr(_EMPTY_STR));
                if (ret)
                {
                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                    taskInfo->fbdo->initJson();
                    Core.jh.setData(taskInfo->fbdo->session.jsonPtr,
                                        taskInfo->fbdo->session.cfn.payload, false);
                    Core.jh.parse(taskInfo->fbdo->session.jsonPtr,
                                      taskInfo->fbdo->session.dataPtr, firebase_func_pgm_str_40 /* "operations/[0]/error/message" */);

                    taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->session.dataPtr->to<const char *>();
                    sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                }

                ret = mDeleteFunction(taskInfo->fbdo, toStringPtr(taskInfo->projectId),
                                      toStringPtr(taskInfo->locationId), toStringPtr(taskInfo->functionId));
            }

            if (taskInfo->step == firebase_functions_creation_step_polling_status)
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
                            if (_function_status == firebase_functions_status_UNKNOWN ||
                                _function_status == firebase_functions_status_OFFLINE)
                                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_error;
                            else if (_function_status == firebase_functions_status_DEPLOY_IN_PROGRESS)
                                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_deploy_in_progress;
                            else if (_function_status == firebase_functions_status_DELETE_IN_PROGRESS)
                                taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_delete_in_progress;
                            else if (_function_status == firebase_functions_status_ACTIVE)
                            {
                                if (taskInfo->setPolicy)
                                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_set_iam_policy_in_progress;
                                else
                                    taskInfo->fbdo->session.cfn.cbInfo.status = firebase_functions_operation_status_finished;
                            }

                            if (_function_status != firebase_functions_status_UNKNOWN && _function_status != firebase_functions_status_OFFLINE)
                            {
                                taskInfo->fbdo->session.cfn.cbInfo.errorMsg = taskInfo->fbdo->session.error;
                                sendCallback(taskInfo->fbdo, taskInfo->callback, taskInfo->statusInfo);
                            }
                        }

                        if (!ret || _function_status == firebase_functions_status_ACTIVE ||
                            _function_status == firebase_functions_status_UNKNOWN ||
                            _function_status == firebase_functions_status_OFFLINE)
                        {
                            taskInfo->done = true;
                            taskInfo->_delete = true;
                            taskInfo->active = _function_status == firebase_functions_status_ACTIVE;

                            if (_function_status == firebase_functions_status_ACTIVE && taskInfo->setPolicy)
                                addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch, taskInfo->nextStep,
                                                firebase_functions_creation_step_idle, taskInfo->callback, taskInfo->statusInfo);

                            if (_function_status == firebase_functions_status_UNKNOWN ||
                                _function_status == firebase_functions_status_OFFLINE)
                                addCreationTask(taskInfo->fbdo, taskInfo->config, taskInfo->patch,
                                                firebase_functions_creation_step_delete, firebase_functions_creation_step_idle,
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
        if (Core.internal.deploy_loop_task_enable && _deployTasks.size() > 0 && n < _deployTasks.size())
            Core.set_scheduled_callback(std::bind(&FB_Functions::runDeployTask, this));
#endif

        if (n == _deployTasks.size())
        {
            for (size_t i = 0; i < n; i++)
            {
                struct firebase_deploy_task_info_t *taskInfo = &_deployTasks[i];
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
    Core.internal.deploy_loop_task_enable = false;

#if defined(ESP32)
    if (Core.internal.functions_check_task_handle)
        return;
#endif

    if (_deployTasks.size() > 0)
        mDeployTasks();
}

#endif

#endif // ENABLE